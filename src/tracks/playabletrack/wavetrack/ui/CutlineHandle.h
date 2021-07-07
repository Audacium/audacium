/**********************************************************************

Audacium: A Digital Audio Editor

CutlineHandle.h

Paul Licameli

**********************************************************************/

#ifndef __AUDACITY_CUTLINE_HANDLE__
#define __AUDACITY_CUTLINE_HANDLE__

#include "../../../../UIHandle.h"
#include "../../../../WaveTrackLocation.h"

class wxMouseEvent;
class wxMouseState;
class WaveTrack;

class CutlineHandle final : public UIHandle
{
   CutlineHandle(const CutlineHandle&) = delete;
   static HitTestPreview HitPreview(bool cutline, bool unsafe);

public:
   explicit CutlineHandle
      ( const std::shared_ptr<WaveTrack> &pTrack,
        WaveTrackLocation location );

   CutlineHandle &operator=(const CutlineHandle&) = default;

   static UIHandlePtr HitAnywhere
      (const AudaciumProject *pProject, bool cutline, UIHandlePtr ptr);
   static UIHandlePtr HitTest
      (std::weak_ptr<CutlineHandle> &holder,
       const wxMouseState &state, const wxRect &rect,
       const AudaciumProject *pProject,
       const std::shared_ptr<WaveTrack> &pTrack);

   virtual ~CutlineHandle();

   const WaveTrackLocation &GetLocation() { return mLocation; }
   std::shared_ptr<WaveTrack> GetTrack() { return mpTrack; }

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
   std::shared_ptr<WaveTrack> mpTrack{};
   enum Operation { Merge, Expand, Remove };
   Operation mOperation{ Merge };
   double mStartTime{}, mEndTime{};
   WaveTrackLocation mLocation {};
};

#endif
