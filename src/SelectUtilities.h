/**********************************************************************
 
 Audacium: A Digital Audio Editor
 
 SelectUtilities.h
 
 Paul Licameli split from SelectMenus.h
 
 **********************************************************************/

#ifndef __AUDACITY_SELECT_UTILITIES__
#define __AUDACITY_SELECT_UTILITIES__

class AudaciumProject;
class Track;

/// Namespace for functions for Select menu
namespace SelectUtilities {

AUDACITY_DLL_API void DoSelectTimeAndTracks(
   AudaciumProject &project, bool bAllTime, bool bAllTracks);
AUDACITY_DLL_API void SelectAllIfNone( AudaciumProject &project );
AUDACITY_DLL_API bool SelectAllIfNoneAndAllowed( AudaciumProject &project );
AUDACITY_DLL_API void SelectNone( AudaciumProject &project );
AUDACITY_DLL_API void DoListSelection(
   AudaciumProject &project, Track *t,
   bool shift, bool ctrl, bool modifyState );
AUDACITY_DLL_API void DoSelectAll( AudaciumProject &project );
AUDACITY_DLL_API void DoSelectAllAudio( AudaciumProject &project );
AUDACITY_DLL_API void DoSelectSomething( AudaciumProject &project );

}

#endif
