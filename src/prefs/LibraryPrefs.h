/**********************************************************************

  Audacium: A Digital Audio Editor

  FileFormatPrefs.h

  Joshua Haberman
  Dominic Mazzoni
  James Crook

**********************************************************************/

#ifndef __AUDACITY_FILE_FORMAT_PREFS__
#define __AUDACITY_FILE_FORMAT_PREFS__

#include <wx/defs.h>

#include "PrefsPanel.h"

class wxStaticText;
class wxTextCtrl;
class ReadOnlyText;
class ShuttleGui;

#define LIBRARY_PREFS_PLUGIN_SYMBOL ComponentInterfaceSymbol{ XO("Library") }

class LibraryPrefs final : public PrefsPanel
{
 public:
   LibraryPrefs(wxWindow * parent, wxWindowID winid);
   ~LibraryPrefs();
   ComponentInterfaceSymbol GetSymbol() override;
   TranslatableString GetDescription() override;

   bool Commit() override;
   ManualPageID HelpPageName() override;
   void PopulateOrExchange(ShuttleGui & S) override;

 private:
   void Populate();
   void SetMP3VersionText(bool prompt = false);

   void OnMP3FindButton(wxCommandEvent & e);
   void OnMP3DownButton(wxCommandEvent & e);

   ReadOnlyText *mMP3Version;

   DECLARE_EVENT_TABLE()
};

#endif
