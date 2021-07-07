/**********************************************************************

Audacium: A Digital Audio Editor

SampleHandle.h

Paul Licameli

**********************************************************************/

#ifndef __AUDACITY_SAMPLE_HANDLE__
#define __AUDACITY_SAMPLE_HANDLE__

#include "../../../../UIHandle.h"
#include "audacium/Types.h"

class wxMouseEvent;
class wxMouseState;

class Track;
class ViewInfo;
class WaveTrack;

class SampleHandle final : public UIHandle
{
   SampleHandle(const SampleHandle&) = delete;
   static HitTestPreview HitPreview
      (const wxMouseState &state, const AudaciumProject *pProject, bool unsafe);

public:
   explicit SampleHandle( const std::shared_ptr<WaveTrack> &pTrack );

   SampleHandle &operator=(const SampleHandle&) = default;

   static UIHandlePtr HitAnywhere
      (std::weak_ptr<SampleHandle> &holder,
       const wxMouseState &state, const std::shared_ptr<WaveTrack> &pTrack);
   static UIHandlePtr HitTest
      (std::weak_ptr<SampleHandle> &holder,
       const wxMouseState &state, const wxRect &rect,
       const AudaciumProject *pProject, const std::shared_ptr<WaveTrack> &pTrack);

   virtual ~SampleHandle();

   std::shared_ptr<WaveTrack> GetTrack() const { return mClickedTrack; }

   void Enter(bool forward, AudaciumProject *) override;

   Result Click
      (const TrackPanelMouseEvent &event, AudaciumProject *pProject) override;

   Result Drag
      (const TrackPanelMouseEvent &event, AudaciumProject *pProject) override;

   HitTestPreview Preview
      (const TrackPanelMouseState &state, AudaciumProject *pProject)
      override;

   Result Release
      (const TrackPanelMouseEvent &event, AudaciumProject *pProject,
       wxWindow *pParent) override;

   Result Cancel(AudaciumProject *pProject) override;

   bool StopsOnKeystroke() override { return true; }

private:
   float FindSampleEditingLevel
      (const wxMouseEvent &event, const ViewInfo &viewInfo, double t0);

   std::shared_ptr<WaveTrack> mClickedTrack;
   wxRect mRect{};

   sampleCount mClickedStartSample{};
   sampleCount mLastDragSample{};
   float mLastDragSampleValue{};
   bool mAltKey{};
};

#endif
