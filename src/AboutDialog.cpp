/**********************************************************************

  Audacium: A Digital Audio Editor

  AboutDialog.cpp

  Dominic Mazzoni
  Vaughan Johnson
  James Crook

********************************************************************//**

\class AboutDialog
\brief The AboutDialog shows the program version and developer credits.

It is a simple scrolling window with an 'OK... Audacious!' button to
close it.

*//*****************************************************************//**

\class AboutDialogCreditItem
\brief AboutDialogCreditItem is a structure used by the AboutDialog to
hold information about one contributor to Audacium.

*//********************************************************************/



#include "AboutDialog.h"



#include <wx/dialog.h>
#include <wx/html/htmlwin.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/intl.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>

#include "FileNames.h"
#include "HelpText.h"
#include "ShuttleGui.h"
#include "widgets/HelpSystem.h"

#include "AllThemeResources.h"
#include "Theme.h"

#include "../images/AudacityLogoWithName.xpm"

// To substitute into many other translatable strings
static const auto ProgramName =
   //XO("Audacium");
   Verbatim("Audacium");

void AboutDialog::CreateCreditsList()
{
   const auto developerFormat =
   /* i18n-hint: For "About Audacium..." credits, substituting a person's proper name */
      XO("%s, developer");
   const auto developerAndSupprtFormat =
   /* i18n-hint: For "About Audacium..." credits, substituting a person's proper name */
      XO("%s, developer and support");
   const auto documentationAndSupportFormat =
   /* i18n-hint: For "About Audacium..." credits, substituting a person's proper name */
      XO("%s, documentation and support");
   const auto testerFormat =
   /* i18n-hint: For "About Audacium..." credits, substituting a person's proper name */
      XO("%s, tester");
   const auto webDeveloperFormat =
   /* i18n-hint: For "About Audacium..." credits, substituting a person's proper name */
      XO("%s, web developer");
   const auto graphicsFormat =
   /* i18n-hint: For "About Audacium..." credits, substituting a person's proper name */
      XO("%s, graphics");

   // The Audacium Team: developers and support
   AddCredit(wxT("@AnErrupTion"), developerAndSupprtFormat, roleTeamMember);
   AddCredit(wxT("@Semisol"), testerFormat, roleTeamMember);

   // Contributors
   AddCredit(wxT("@PlavorSeol"), documentationAndSupportFormat, roleContributor);
   AddCredit(wxT("@matt-hu"), developerFormat, roleContributor);
   AddCredit(wxT("@bl-ue"), documentationAndSupportFormat, roleContributor);

   // Website and Graphics
   AddCredit(wxT("@realseb1g"), webDeveloperFormat, roleGraphics);
   AddCredit(wxT("@bascurtiz"), graphicsFormat, roleGraphics);

   // Libraries
   AddCredit(wxT("[[https://libexpat.github.io/|expat]]"), roleLibrary);
   AddCredit(wxT("[[https://xiph.org/flac/|FLAC]]"), roleLibrary);
   AddCredit(wxT("[[http://lame.sourceforge.net/|LAME]]"), roleLibrary);
   AddCredit(wxT("[[https://www.underbit.com/products/mad/|libmad]]"), roleLibrary);
   AddCredit(wxT("[[http://www.mega-nerd.com/libsndfile/|libsndfile]]"), roleLibrary);
   AddCredit(wxT("[[https://sourceforge.net/p/soxr/wiki/Home/|libsoxr]]"), roleLibrary);
   AddCredit(
      XO("%s (incorporating %s, %s, %s, %s and %s)")
         .Format(
            "[[http://lv2plug.in/|lv2]]",
            "lilv",
            "msinttypes",
            "serd",
            "sord",
            "sratom"
         ).Translation(),
      roleLibrary);
   AddCredit(wxT("[[https://www.cs.cmu.edu/~music/nyquist/|Nyquist]]"), roleLibrary);
   AddCredit(wxT("[[https://xiph.org/vorbis/|Ogg Vorbis]]"), roleLibrary);
   AddCredit(wxT("[[http://www.portaudio.com/|PortAudio]]"), roleLibrary);
   AddCredit(wxT("[[http://www.portmedia.sourceforge.net/portmidi/|PortMidi]]"), roleLibrary);
   AddCredit(wxT("[[https://sourceforge.net/p/portmedia/wiki/portsmf/|portsmf]]"), roleLibrary);
   AddCredit(wxT("[[http://sbsms.sourceforge.net/|sbsms]]"), roleLibrary);
   AddCredit(wxT("[[http://www.twolame.org/|TwoLAME]]"), roleLibrary);
   AddCredit(wxT("[[http://www.vamp-plugins.org/|Vamp]]"), roleLibrary);
   AddCredit(wxT("[[https://wxwidgets.org/|wxWidgets]]"), roleLibrary);

   // Thanks
   AddCredit(wxT("The Audacity Team"), roleThanks);
}

// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(AboutDialog, wxDialogWrapper)
   EVT_BUTTON(wxID_OK, AboutDialog::OnOK)
END_EVENT_TABLE()

IMPLEMENT_CLASS(AboutDialog, wxDialogWrapper)

namespace {
   AboutDialog *sActiveInstance{};
}

AboutDialog *AboutDialog::ActiveIntance()
{
   return sActiveInstance;
}

AboutDialog::AboutDialog(wxWindow * parent)
   /* i18n-hint: information about the program */
   :  wxDialogWrapper(parent, -1, XO("About %s").Format( ProgramName ),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
   wxASSERT(!sActiveInstance);
   sActiveInstance = this;

   SetName();
   this->SetBackgroundColour(theTheme.Colour( clrAboutBoxBackground ));
   //this->SetBackgroundColour(theTheme.Colour( clrMedium ));
   icon = NULL;
   ShuttleGui S( this, eIsCreating );
   S.StartNotebook();
   {
      PopulateAudacityPage( S );
   }
   S.EndNotebook();

   S.Id(wxID_OK)
      .Prop(0)
      .AddButton(XXO("OK"), wxALIGN_CENTER, true);

   Fit();
   this->Centre();
}

#define ABOUT_DIALOG_WIDTH 506

void AboutDialog::PopulateAudacityPage( ShuttleGui & S )
{
   CreateCreditsList();

   /* i18n-hint: The translation of "translator_credits" will appear
    *  in the credits in the About Audacium window.  Use this to add
    *  your own name(s) to the credits.
    *
    *  For example:  "English translation by Dominic Mazzoni." */
   auto translatorCreditsMsgid = XO("translator_credits");
   auto translatorCredits = translatorCreditsMsgid.Translation();
   if ( translatorCredits == translatorCreditsMsgid.MSGID().GET() )
      // We're in an English locale
      translatorCredits.clear();
   else
      translatorCredits += wxT("<br>");

   wxStringOutputStream o;
   wxTextOutputStream informationStr( o );   // string to build up list of information in
   informationStr
      << wxT("<center>")
      << XO("<h3>")
      << ProgramName
      << wxT(" ")
      << wxString(AUDACITY_VERSION_STRING)
      << wxT("</center></h3>")
      /* i18n-hint: The program's name substitutes for %s */
      << XO("%s is the free, open source and cross-platform software for recording and editing sounds.")
      .Format(ProgramName)

      << wxT("<h3>")
      << XO("Donation")
      << wxT("</h3>")
      << wxT("<p>Litecoin: LfA7A3wVdGysc4QXpsUvYaVUYvhcZnVE7j</p>")

      << wxT("<br><h3>")
      << XO("Credits")
      << wxT("</h3>")
      << wxT("<p>")

      << wxT("<p><b>")
      /* i18n-hint: The program's name substitutes for %s */
      << XO("%s Team Members").Format( ProgramName )
      << wxT("</b><br>")
      << GetCreditsByRole(roleTeamMember)

      << wxT("<p><b>")
      << XO("Contributors")
      << wxT("</b><br>")
      << GetCreditsByRole(roleContributor)

      << wxT("<p><b>")
      << XO("Website and Graphics")
      << wxT("</b><br>")
      << GetCreditsByRole(roleGraphics)
   ;

   if(!translatorCredits.empty()) informationStr
      << wxT("<p><b>")
      << XO("Translators")
      << wxT("</b><br>")
      << translatorCredits
   ;

   informationStr
      << wxT("<p><b>")
      << XO("Libraries")
      << wxT("</b><br>")
      /* i18n-hint: The program's name substitutes for %s */
      << XO("%s includes code from the following projects:").Format( ProgramName )
      << wxT("<br><br>")
      << GetCreditsByRole(roleLibrary)

      << wxT("<p><b>")
      << XO("Special thanks:")
      << wxT("</b><br>")
      << GetCreditsByRole(roleThanks)

      << wxT("<p><br>")
      /* i18n-hint: The program's name substitutes for %s */
      << XO("%s website: ").Format( ProgramName )
      << wxT("[[https://audacium.xyz/|https://audacium.xyz/]]")

      << wxT("<p><br>&nbsp; &nbsp; ")
      /* i18n-hint Audacium's name substitutes for first and third %s,
       and a "copyright" symbol for the second */
      << XO("%s software is copyleft.")
         .Format( ProgramName )
      << wxT("<br>")

      << wxT("&nbsp; &nbsp; ")
      /* i18n-hint Audacium's name substitutes for %s */
      << XO("The name %s is not a registered trademark.")
         .Format( Verbatim("<b>%s</b>").Format( ProgramName ) )
      << wxT("<br><br>")

      << wxT("</center>")
   ;

   auto pPage = S.StartNotebookPage( ProgramName );
   S.StartVerticalLay(1);
   {
      //v For now, change to AudacityLogoWithName via old-fashioned way, not Theme.
      wxBitmap logo(AudacityLogoWithName_xpm); //v

      icon =
          safenew wxStaticBitmap(S.GetParent(), -1,
              //*logo, //v
              //v theTheme.Bitmap(bmpAudacityLogo), wxPoint(93, 10), wxSize(215, 190));
              //v theTheme.Bitmap(bmpAudacityLogoWithName),
              logo,
              wxDefaultPosition);
   }
   S.Prop(0).AddWindow( icon );

   HtmlWindow *html = safenew LinkingHtmlWindow(S.GetParent(), -1,
                                         wxDefaultPosition,
                                         wxSize(ABOUT_DIALOG_WIDTH, 359),
                                         wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER);
   html->SetPage( FormatHtmlText( o.GetString() ) );

   /* locate the html renderer where it fits in the dialogue */
   S.Prop(1).Position( wxEXPAND ).Focus()
      .AddWindow( html );

   S.EndVerticalLay();
   S.EndNotebookPage();
}

void AboutDialog::AddCredit( const wxString &name, Role role )
{
   AddCredit( name, {}, role );
}

void AboutDialog::AddCredit(
   const wxString &name, TranslatableString format, Role role )
{
   auto str = format.empty()
      ? Verbatim( name )
      : TranslatableString{ format }.Format( name );
   creditItems.emplace_back(std::move(str), role);
}

wxString AboutDialog::GetCreditsByRole(AboutDialog::Role role)
{
   wxString s;

   for (const auto &item : creditItems)
   {
      if (item.role == role)
      {
         s += item.description.Translation();
         s += wxT("<br>");
      }
   }

   // Strip last <br>, if any
   if (s.Right(4) == wxT("<br>"))
      s = s.Left(s.length() - 4);

   return s;
}

/** \brief Add a table row saying if a library is used or not
 *
 * Used when creating the build information tab to show if each optional
 * library is enabled or not, and what it does */
void AboutDialog::AddBuildinfoRow(
   wxTextOutputStream *str, const wxChar * libname,
   const TranslatableString &libdesc, const TranslatableString &status)
{
   *str
      << wxT("<tr><td>")
      << libname
      << wxT("</td><td>(")
      << libdesc
      << wxT(")</td><td>")
      << status
      << wxT("</td></tr>");
}

/** \brief Add a table row saying if a library is used or not
 *
 * Used when creating the build information tab to show build dates and
 * file paths */
void AboutDialog::AddBuildinfoRow(
   wxTextOutputStream *str,
   const TranslatableString &description, const wxChar *spec)
{
   *str
      << wxT("<tr><td>")
      << description
      << wxT("</td><td>")
      << spec
      << wxT("</td></tr>");
}

AboutDialog::~AboutDialog()
{
   sActiveInstance = {};
}

void AboutDialog::OnOK(wxCommandEvent & WXUNUSED(event))
{
#ifdef __WXMAC__
   Destroy();
#else
   EndModal(wxID_OK);
#endif
}
