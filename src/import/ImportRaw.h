/**********************************************************************

  Audacium: A Digital Audio Editor

  ImportRaw.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __AUDACITY_IMPORT_RAW__
#define __AUDACITY_IMPORT_RAW__

#include <memory>

class AudaciumProject;
class WaveTrackFactory;
class WaveTrack;
class wxString;
class wxWindow;

#include <vector>

// Newly constructed WaveTracks that are not yet owned by a TrackList
// are held in unique_ptr not shared_ptr
using NewChannelGroup = std::vector< std::shared_ptr<WaveTrack> >;
using TrackHolders = std::vector< NewChannelGroup >;


void ImportRaw(const AudaciumProject &project, wxWindow *parent, const wxString &fileName,
   WaveTrackFactory *trackFactory, TrackHolders &outTracks);

#endif
