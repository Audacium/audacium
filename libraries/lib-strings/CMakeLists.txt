#[[
String wrapping types and internationalization support.

Identifier and specializations of TaggedIdentifier generate types of strings
used for different internal purposes, not meant for users to see (except
sometimes in macro programming), and the types do not implicitly interconvert.

TranslatableString holds the English msgid of a string meant for a user to see,
and can also bind formatting arguments.  It can generate a translation later,
even if the global choice of locale changes during its lifetime.  It does not
implicitly interconvert with wxString.

This library depends only on the wxBase subset of wxWidgets.
]]#

set(TARGET lib-strings)
set(TARGET_ROOT ${CMAKE_CURRENT_SOURCE_DIR} )

def_vars()

set( SOURCES
   FutureStrings.h

   Identifier.cpp
   Identifier.h

   Internat.cpp
   Internat.h

   Languages.cpp
   Languages.h

   TranslatableString.cpp
   TranslatableString.h

   wxArrayStringEx.h
   wxArrayStringEx.cpp

   HexHelpers.h
    
   UrlEncode.h
   UrlEncode.cpp

   UrlDecode.h
   UrlDecode.cpp

   CodeConversions.h
   CodeConversions.cpp

   DateTimeConversions.h
   DateTimeConversions.cpp
)

set( LIBRARIES
   PRIVATE
   wxBase
   wxwidgets::base
)

audacity_library( ${TARGET} "${SOURCES}" "${LIBRARIES}" "" "" )