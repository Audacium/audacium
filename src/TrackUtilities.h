/**********************************************************************
 
 Audacium: A Digital Audio Editor
 
 TrackUtilities.h
 
 Paul Licameli split from TrackMenus.h
 
 **********************************************************************/

#ifndef __AUDACITY_TRACK_UTILITIES__
#define __AUDACITY_TRACK_UTILITIES__

class AudaciumProject;
class Track;

namespace TrackUtilities {

   enum MoveChoice {
      OnMoveUpID, OnMoveDownID, OnMoveTopID, OnMoveBottomID
   };
   /// Move a track up, down, to top or to bottom.
   AUDACITY_DLL_API void DoMoveTrack(
      AudaciumProject &project, Track* target, MoveChoice choice );
   // "exclusive" mute means mute the chosen track and unmute all others.
   AUDACITY_DLL_API
   void DoTrackMute( AudaciumProject &project, Track *pTrack, bool exclusive );
   // Type of solo (standard or simple) follows the set preference, unless
   // exclusive == true, which causes the opposite behavior.
   AUDACITY_DLL_API
   void DoTrackSolo( AudaciumProject &project, Track *pTrack, bool exclusive );
   AUDACITY_DLL_API
   void DoRemoveTrack( AudaciumProject &project, Track * toRemove );
   AUDACITY_DLL_API
   void DoRemoveTracks( AudaciumProject & );

}

#endif
