#[[
A small library defining various constants and utilities

No dependencies on wxWidgets.  Very few if any symbols with linkage.

One very important utility is finally() as defined in The C++ Programming
Language, fourth edition, for describing ad-hoc RAII actions.

Historically, MemoryX.h began as a wrapper of C++11 <memory>, which by itself
did not yet provide std::make_unique.  That explains the name.

Audacity now uses C++14, and yet, need arose for other anticipations of
future standards, such as with optional.  It also provides other pervasively
used utilities that don't correspond to things in the standard.

]]#

set( TARGET lib-utility )
set( TARGET_ROOT ${CMAKE_CURRENT_SOURCE_DIR} )

def_vars()

set( SOURCES
   MemoryX.cpp
   MemoryX.h
   ModuleConstants.h
   Uuid.h
   Uuid.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set( DEFINES PRIVATE USE_UUID_CREATE )
    set( LIBRARIES PRIVATE Rpcrt4 )
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    find_library(CORE_FOUNDATION CoreFoundation)

    set( DEFINES PRIVATE USE_CFUUID )
    set( LIBRARIES PRIVATE ${CORE_FOUNDATION})
else()
    set( DEFINES PRIVATE USE_LIBUUID)
    set( LIBRARIES PRIVATE libuuid::libuuid)
endif()

list( APPEND LIBRARIES PRIVATE lib-strings)

audacity_library( ${TARGET} "${SOURCES}" "${LIBRARIES}" "${DEFINES}" "" )
