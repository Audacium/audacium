/*!********************************************************************

 Audacium: A Digital Audio Editor

 @file DateTimeConversions.h
 @brief Declare functions to work with date and time string representations.

 Dmitry Vedenko
 **********************************************************************/

#pragma once

#include <chrono>
#include <string>

namespace audacity
{

using SystemTime = std::chrono::system_clock::time_point;

STRINGS_API bool ParseRFC822Date (const std::string& dateString, SystemTime* time);
STRINGS_API std::string SerializeRFC822Date (SystemTime timePoint);

}
