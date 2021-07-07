/*!********************************************************************

 Audacium: A Digital Audio Editor

 @file HeadersList.cpp
 @brief Define HTTP headers list class.

 Dmitry Vedenko
 **********************************************************************/

/*!********************************************************************

 @class Header
 @brief A string pair, representing HTTP header.

 **********************************************************************/

/*!********************************************************************

 @class HeadersList
 @brief A class, representing a list of HTTP headers.

 **********************************************************************/

#include "HeadersList.h"

#include <algorithm>
#include <cctype>

namespace audacium
{
namespace network_manager
{

bool Header::hasSameName (const Header& header) const
{
    return hasSameName (header.Name);
}

bool Header::hasSameName (const std::string& name) const
{
    return std::equal (
        name.begin (), name.end (),
        Name.begin (), Name.end (),
        [](const char leftChar, const char rightChar) {
            return std::tolower (leftChar) == std::tolower (rightChar);
    });
}

Header Header::Parse (const std::string& header)
{
    const size_t colonPosition = header.find (": ");

    if (colonPosition == std::string::npos) // This can happen when we receieve the first line of the response
        return { header, std::string () };

    return {
        header.substr (0, colonPosition),
        header.substr (colonPosition + 2)
    };
}

void HeadersList::setHeader (const Header& header)
{
    setHeader (header.Name, header.Value);
}

void HeadersList::setHeader (const std::string& headerName, std::string headerValue)
{
    Header* item = getHeader (headerName);

    if (item != nullptr)
        item->Value = std::move (headerValue);
    else
        mHeaders.push_back ({ headerName, std::move (headerValue) });
}

void HeadersList::addHeader (Header header)
{
    addHeader (std::move (header.Name), std::move (header.Value));
}

void HeadersList::addHeader (std::string headerName, std::string headerValue)
{
    mHeaders.push_back ({ std::move (headerName), std::move (headerValue) });
}

bool HeadersList::hasHeader (const std::string& headerName) const noexcept
{
    return getHeader (headerName) != nullptr;
}

std::string HeadersList::getHeaderValue (const std::string& headerName) const
{
    const Header* header = getHeader (headerName);

    if (header != nullptr)
        return header->Value;

    return {};
}

const Header* HeadersList::getHeader (size_t idx) const noexcept
{
    return const_cast<HeadersList*>(this)->getHeader (idx);
}

const Header* HeadersList::getHeader (const std::string& name) const noexcept
{
    return const_cast<HeadersList*>(this)->getHeader (name);
}

size_t HeadersList::getHeadersCount () const noexcept
{
    return mHeaders.size ();
}

HeadersList::HeadersIterator HeadersList::begin () noexcept
{
    return mHeaders.begin ();
}

HeadersList::HeadersIterator HeadersList::end () noexcept
{
    return mHeaders.end ();
}

HeadersList::HeadersConstIterator HeadersList::begin () const noexcept
{
    return mHeaders.begin ();
}

HeadersList::HeadersConstIterator HeadersList::end () const noexcept
{
    return mHeaders.end ();
}

Header* HeadersList::getHeader (size_t idx) noexcept
{
    if (idx < mHeaders.size ())
        return &mHeaders[idx];

    return nullptr;
}

Header* HeadersList::getHeader (const std::string& headerName) noexcept
{
    for (Header& header : mHeaders)
    {
        if (header.hasSameName (headerName))
            return &header;
    }

    return nullptr;
}

}
}
