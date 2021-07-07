/**********************************************************************

  Audacium: A Digital Audio Editor

  AudaciumLogger.h

  Dominic Mazzoni

  This is the main source file for Audacium which handles
  initialization and termination by subclassing wxApp.

**********************************************************************/

#ifndef __AUDACITY_LOGGER__
#define __AUDACITY_LOGGER__





#include "MemoryX.h"
#include "Prefs.h"
#include <wx/log.h> // to inherit
#include <wx/event.h> // to inherit wxEvtHandler

class wxFrame;
class wxTextCtrl;

class AUDACITY_DLL_API AudaciumLogger final : public wxEvtHandler,
                             public wxLog,
                             public PrefsListener
{
 public:

   ~AudaciumLogger() override;
 
   // Get the singleton instance or null
   static AudaciumLogger *Get();

   void Show(bool show = true);

   bool SaveLog(const wxString &fileName) const;
   bool ClearLog();

   wxString GetLog(int count = 0);

 protected:
   void Flush()  override;
   void DoLogText(const wxString & msg) override;

 private:
   AudaciumLogger();

   void OnCloseWindow(wxCloseEvent & e);
   void OnClose(wxCommandEvent & e);
   void OnClear(wxCommandEvent & e);
   void OnSave(wxCommandEvent & e);

   // PrefsListener implementation
   void UpdatePrefs() override;

   Destroy_ptr<wxFrame> mFrame;
   wxTextCtrl *mText;
   wxString mBuffer;
   bool mUpdated;
};

#endif
