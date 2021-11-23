/*!********************************************************************

 Audacity: A Digital Audio Editor

 @file CodeConversions.h
 @brief Declare functions to preform UTF-8 to std::wstring conversions.

 Dmitry Vedenko
 **********************************************************************/

#pragma once

#include <string>
#include <wx/string.h>

namespace audacity
{

STRINGS_API std::string ToUTF8 (const std::wstring& wstr);
STRINGS_API std::string ToUTF8 (const wchar_t* wstr);
STRINGS_API std::string ToUTF8 (const wxString& wstr);

// std::wstring is UTF16 on windows and UTF32 elsewhere.
STRINGS_API std::wstring ToWString (const std::string& str);
STRINGS_API std::wstring ToWString (const char* str);
STRINGS_API std::wstring ToWString (const wxString& str);

STRINGS_API wxString ToWXString (const std::string& str);
STRINGS_API wxString ToWXString (const std::wstring& str);

}
