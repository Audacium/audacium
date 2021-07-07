/**********************************************************************

  Audacium: A Digital Audio Editor

  AudaciumLogger.cpp

******************************************************************//**

\class AudaciumLogger
\brief AudaciumLogger is a thread-safe logger class

Provides thread-safe logging based on the wxWidgets log facility.

*//*******************************************************************/



#include "AudaciumLogger.h"



#include "FileNames.h"
#include "Internat.h"
#include "ShuttleGui.h"

#include <mutex>
#include <wx/filedlg.h>
#include <wx/log.h>
#include <wx/ffile.h>
#include <wx/frame.h>
#include <wx/icon.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>

#include "../images/AudaciumLogoAlpha.xpm"
#include "widgets/AudaciumMessageBox.h"

//
// AudaciumLogger class
//
// Two reasons for this class instead of the wxLogWindow class (or any WX GUI logging class)
//
// 1)  If wxLogWindow is used and initialized before the Mac's "root" window, then
//     Audacium may crash when terminating.  It's not fully understood why this occurs
//     but it probably has to do with the order of deletion.  However, deferring the
//     creation of the log window until it is actually shown circumvents the problem.
// 2)  By providing an Audacium specific logging class, it can be made thread-safe and,
//     as such, can be used by the ever growing threading within Audacium.
//
enum
{
   LoggerID_Save = wxID_HIGHEST + 1,
   LoggerID_Clear,
   LoggerID_Close
};

AudaciumLogger *AudaciumLogger::Get()
{
   static std::once_flag flag;
   std::call_once( flag, []{
      // wxWidgets will clean up the logger for the main thread, so we can say
      // safenew.  See:
      // http://docs.wxwidgets.org/3.0/classwx_log.html#a2525bf54fa3f31dc50e6e3cd8651e71d
      std::unique_ptr < wxLog > // DELETE any previous logger
         { wxLog::SetActiveTarget(safenew AudaciumLogger) };
   } );

   // Use dynamic_cast so that we get a NULL ptr in case our logger
   // is no longer the target.
   return dynamic_cast<AudaciumLogger *>(wxLog::GetActiveTarget());
}

AudaciumLogger::AudaciumLogger()
:  wxEvtHandler(),
   wxLog()
{
   mText = NULL;
   mUpdated = false;
}

AudaciumLogger::~AudaciumLogger()  = default;

void AudaciumLogger::Flush()
{
   if (mUpdated && mFrame && mFrame->IsShown()) {
      mUpdated = false;
      mText->ChangeValue(mBuffer);
   }
}

void AudaciumLogger::DoLogText(const wxString & str)
{
   if (!wxIsMainThread()) {
      wxMutexGuiEnter();
   }

   if (mBuffer.empty()) {
      wxString stamp;

      TimeStamp(&stamp);

      mBuffer << stamp << _TS("Audacium ") << AUDACITY_VERSION_STRING << wxT("\n");
   }

   mBuffer << str << wxT("\n");

   mUpdated = true;

   Flush();

   if (!wxIsMainThread()) {
      wxMutexGuiLeave();
   }
}

bool AudaciumLogger::SaveLog(const wxString &fileName) const
{
   wxFFile file(fileName, wxT("w"));

   if (file.IsOpened()) {
      file.Write(mBuffer);
      file.Close();
      return true;
   }

   return false;
}

bool AudaciumLogger::ClearLog()
{
   mBuffer = wxEmptyString;
   DoLogText(wxT("Log Cleared."));

   return true;
}

void AudaciumLogger::Show(bool show)
{
   // Hide the frame if created, otherwise do nothing
   if (!show) {
      if (mFrame) {
         mFrame->Show(false);
      }
      return;
   }

   // If the frame already exists, refresh its contents and show it
   if (mFrame) {
      if (!mFrame->IsShown()) {
         mText->ChangeValue(mBuffer);
         mText->SetInsertionPointEnd();
         mText->ShowPosition(mText->GetLastPosition());
      }
      mFrame->Show();
      mFrame->Raise();
      return;
   }

   // This is the first use, so create the frame
   Destroy_ptr<wxFrame> frame
      { safenew wxFrame(NULL, wxID_ANY, _("Audacium Log")) };
   frame->SetName(frame->GetTitle());
   frame->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));

   // loads either the XPM or the windows resource, depending on the platform
   {
#if !defined(__WXMAC__) && !defined(__WXX11__)
#if defined(__WXMSW__)
      wxIcon ic{wxICON(AudaciumLogo)};
#elif defined(__WXGTK__)
      wxIcon ic{wxICON(AudaciumLogoAlpha)};
#else
      wxIcon ic{};
      ic.CopyFromBitmap(theTheme.Bitmap(bmpAudaciumLogo48x48));
#endif
      frame->SetIcon(ic);
#endif
   }

   // Log text
   ShuttleGui S(frame.get(), eIsCreating);

   S.Style(wxNO_BORDER | wxTAB_TRAVERSAL).Prop(true).StartPanel();
   {
      S.StartVerticalLay(true);
      {
         mText = S.Style(wxTE_MULTILINE | wxHSCROLL | wxTE_READONLY | wxTE_RICH)
            .AddTextWindow(mBuffer);

         S.AddSpace(0, 5);
         S.StartHorizontalLay(wxALIGN_CENTER, 0);
         {
            S.AddSpace(10, 0);
            S.Id(LoggerID_Save).AddButton(XXO("&Save..."));
            S.Id(LoggerID_Clear).AddButton(XXO("Cl&ear"));
            S.Id(LoggerID_Close).AddButton(XXO("&Close"));
            S.AddSpace(10, 0);
         }
         S.EndHorizontalLay();
         S.AddSpace(0, 3);
      }
      S.EndVerticalLay();
   }
   S.EndPanel();

   // Give a place for the menu help text to go
   // frame->CreateStatusBar();

   frame->Layout();

   // Hook into the frame events
   frame->Bind(wxEVT_CLOSE_WINDOW,
                  wxCloseEventHandler(AudaciumLogger::OnCloseWindow),
                  this);

   frame->Bind(   wxEVT_COMMAND_MENU_SELECTED,
                  &AudaciumLogger::OnSave,
                  this, LoggerID_Save);
   frame->Bind(   wxEVT_COMMAND_MENU_SELECTED,
                  &AudaciumLogger::OnClear,
                  this, LoggerID_Clear);
   frame->Bind(   wxEVT_COMMAND_MENU_SELECTED,
                  &AudaciumLogger::OnClose,
                  this, LoggerID_Close);
   frame->Bind(   wxEVT_COMMAND_BUTTON_CLICKED,
                  &AudaciumLogger::OnSave,
                  this, LoggerID_Save);
   frame->Bind(   wxEVT_COMMAND_BUTTON_CLICKED,
                  &AudaciumLogger::OnClear,
                  this, LoggerID_Clear);
   frame->Bind(   wxEVT_COMMAND_BUTTON_CLICKED,
                  &AudaciumLogger::OnClose,
                  this, LoggerID_Close);

   mFrame = std::move( frame );

   mFrame->Show();

   Flush();
}

wxString AudaciumLogger::GetLog(int count)
{
   if (count == 0)
   {
      return mBuffer;
   }

   wxString buffer;

   auto lines = wxStringTokenize(mBuffer, wxT("\r\n"), wxTOKEN_RET_DELIMS);
   for (int index = lines.size() - 1; index >= 0 && count > 0; --index, --count)
   {
      buffer.Prepend(lines[index]);
   }

   return buffer;
}

void AudaciumLogger::OnCloseWindow(wxCloseEvent & WXUNUSED(e))
{
#if defined(__WXMAC__)
   // On the Mac, destroy the window rather than hiding it since the
   // log menu will override the root windows menu if there is no
   // project window open.
   mFrame.reset();
#else
   Show(false);
#endif
}

void AudaciumLogger::OnClose(wxCommandEvent & WXUNUSED(e))
{
   wxCloseEvent dummy;
   OnCloseWindow(dummy);
}

void AudaciumLogger::OnClear(wxCommandEvent & WXUNUSED(e))
{
   ClearLog();
}

void AudaciumLogger::OnSave(wxCommandEvent & WXUNUSED(e))
{
   wxString fName = _("log.txt");

   fName = FileNames::SelectFile(FileNames::Operation::Export,
      XO("Save log to:"),
      wxEmptyString,
      fName,
      wxT("txt"),
      { FileNames::TextFiles },
      wxFD_SAVE | wxFD_OVERWRITE_PROMPT | wxRESIZE_BORDER,
      mFrame.get());

   if (fName.empty()) {
      return;
   }

   if (!mText->SaveFile(fName)) {
      AudaciumMessageBox(
         XO("Couldn't save log to file: %s").Format( fName ),
         XO("Warning"),
         wxICON_EXCLAMATION,
         mFrame.get());
      return;
   }
}

void AudaciumLogger::UpdatePrefs()
{
   if (mFrame) {
      bool shown = mFrame->IsShown();
      if (shown) {
         Show(false);
      }
      mFrame.reset();
      if (shown) {
         Show(true);
      }
   }
}
