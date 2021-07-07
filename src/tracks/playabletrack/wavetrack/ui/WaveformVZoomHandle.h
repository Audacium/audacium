/**********************************************************************

Audacium: A Digital Audio Editor

WaveformVZoomHandle.h

Paul Licameli split from WaveTrackVZoomHandle.h

**********************************************************************/

#ifndef __AUDACITY_WAVEFORM_VZOOM_HANDLE__
#define __AUDACITY_WAVEFORM_VZOOM_HANDLE__

#include "../../../../UIHandle.h" // to inherit
#include "WaveTrackViewConstants.h"

class WaveTrack;

class WaveformVZoomHandle final : public UIHandle
{
   WaveformVZoomHandle(const WaveformVZoomHandle&);

public:
   explicit WaveformVZoomHandle
      (const std::shared_ptr<WaveTrack> &pTrack, const wxRect &rect, int y);

   WaveformVZoomHandle &operator=(const WaveformVZoomHandle&) = default;

   static void DoZoom(
      AudaciumProject *pProject, WaveTrack *pTrack,
      WaveTrackViewConstants::ZoomActions ZoomKind,
      const wxRect &rect, int zoomStart, int zoomEnd,
      bool fixedMousePoint);

   ~WaveformVZoomHandle() override;

   std::shared_ptr<WaveTrack> GetTrack() const { return mpTrack.lock(); }

   void Enter( bool forward, AudaciumProject * ) override;

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

private:

   // TrackPanelDrawable implementation
   void Draw(
      TrackPanelDrawingContext &context,
      const wxRect &rect, unsigned iPass ) override;

   wxRect DrawingArea(
      TrackPanelDrawingContext &,
      const wxRect &rect, const wxRect &panelRect, unsigned iPass ) override;

   std::weak_ptr<WaveTrack> mpTrack;

   int mZoomStart{}, mZoomEnd{};
   wxRect mRect{};
};

#include "WaveTrackVZoomHandle.h" // to inherit

class WaveformVRulerMenuTable : public WaveTrackVRulerMenuTable
{
   WaveformVRulerMenuTable()
      : WaveTrackVRulerMenuTable{ "WaveFormVRuler" }
   {}
   virtual ~WaveformVRulerMenuTable() {}
   DECLARE_POPUP_MENU(WaveformVRulerMenuTable);

public:
   static PopupMenuTable &Instance();

private:
   void OnWaveformScaleType(wxCommandEvent &evt);
};

#endif
