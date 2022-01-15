/*!********************************************************************

 Audacium: A Digital Audio Editor

 @file UrlDecode.h
 @brief Declare a function to decode an URL encode string.

 Dmitry Vedenko
 **********************************************************************/

#pragma once

#include <string>

namespace audacity
{

STRINGS_API std::string UrlDecode (const std::string& url);

}
