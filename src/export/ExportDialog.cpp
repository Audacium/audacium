#include "ExportDialog.h"

#include "../ViewInfo.h"

#include "../widgets/AudacityMessageBox.h"
#include "../widgets/FileHistory.h"

#include <wx/simplebook.h>

enum
{
    ComboID = 20001
};

BEGIN_EVENT_TABLE(ExportDialog, wxEvtHandler)
    EVT_COMBOBOX(ComboID, ExportDialog::OnIndexChanged)
    EVT_BUTTON(wxID_OK, ExportDialog::OnOK)
    EVT_CLOSE(ExportDialog::OnClose)
END_EVENT_TABLE()

ExportDialog::ExportDialog(AudacityProject* parent) :
    wxDialogWrapper(&GetProjectFrame(*parent), wxID_ANY, XO("Export Audio")), mExporter{*parent}
{
	SetName();

	mProject = parent;

	for (const auto& plugin : mExporter.GetPlugins())
		mPlugins.push_back(plugin.get());

	ShuttleGui S(this, eIsCreatingFromPrefs);
	PopulateOrExchange(S);

	Layout();
	SetMinSize(GetSize());
	Center();
}

ExportDialog::ExportDialog(AudacityProject* parent, NotifyingSelectedRegion &region) :
    wxDialogWrapper(&GetProjectFrame(*parent), wxID_ANY, XO("Export Selected Audio")), mExporter{ *parent }
{
    SetName();

    mProject = parent;

    mTrackStart = region.t0();
    mTrackEnd = region.t1();

    for (const auto& plugin : mExporter.GetPlugins())
        mPlugins.push_back(plugin.get());

    ShuttleGui S(this, eIsCreatingFromPrefs);
    PopulateOrExchange(S);

    Layout();
    SetMinSize(GetSize());
    Center();
}

ExportDialog::ExportDialog(AudacityProject* parent, wxString extension) :
    wxDialogWrapper(&GetProjectFrame(*parent), wxID_ANY, XO("Export Audio")), mExporter{ *parent }
{
    SetName();

    mProject = parent;
    mFileExtension = extension;

    for (const auto& plugin : mExporter.GetPlugins())
        mPlugins.push_back(plugin.get());

    ShuttleGui S(this, eIsCreatingFromPrefs);
    PopulateOrExchange(S);

    Layout();
    SetMinSize(GetSize());
    Center();
}

ExportDialog::~ExportDialog() {}

void ExportDialog::PopulateOrExchange(ShuttleGui &S)
{
    wxArrayStringEx choices;
    wxString defaultChoice;

    for (const auto &type : mExporter.GetAllFileTypes())
    {
        wxString str = type.description.Translation();

        if (mFileExtension != wxEmptyString && type.extensions.Index(mFileExtension, false) != wxNOT_FOUND)
            defaultChoice = str;

        choices.push_back(str);
    }

    if (mFileExtension == wxEmptyString)
        defaultChoice = choices.Item(0);

    S.StartStatic(XO("File Format"), 1);
    {
        S.StartHorizontalLay(wxEXPAND, false);
        {
            mFormat = S.Position(wxEXPAND).Id(ComboID).AddCombo(XO("Select file format:"), defaultChoice, choices);
            mFormat->SetStringSelection(defaultChoice);
        }
        S.EndHorizontalLay();
    }
    S.EndStatic();

    S.StartStatic(XO("Format Options"), 1);
    {
        S.StartHorizontalLay(wxEXPAND);
        {
            mBook = S.Position(wxEXPAND).StartSimplebook();
            {
                for (const auto &pPlugin : mPlugins)
                    for (int j = 0; j < pPlugin->GetFormatCount(); j++)
                    {
                        // Name of simple book page is not displayed
                        S.StartNotebookPage({});
                        {
                            pPlugin->OptionsCreate(S, j);
                        }
                        S.EndNotebookPage();
                    }
            }
            S.EndSimplebook();
        }
        S.EndHorizontalLay();
    }
    S.EndStatic();

    mOK = S.Position(wxCENTER).Id(wxID_OK).AddButton(XO("OK"));
}

int ExportDialog::ShowModal()
{
	return wxDialogWrapper::ShowModal();
}

void ExportDialog::OnIndexChanged(wxCommandEvent& WXUNUSED(event))
{
    mBook->ChangeSelection(mFormat->GetSelection());
}

void ExportDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
    FileNames::FileType fileType;
    FileExtension extension;

    for (const auto& type : mExporter.GetAllFileTypes())
        if (type.description.Translation() == mFormat->GetStringSelection())
        {
            fileType = type;
            extension = type.extensions.Item(0);

            break;
        }

    FileNames::FileTypes fileTypes;
    fileTypes.push_back(fileType);

    auto &tracks = TrackList::Get(*mProject);

    if (mTrackStart == 0.0 && mTrackEnd == 0.0)
        mTrackEnd = tracks.GetEndTime();

    mExporter.SetFileTypes(fileTypes);
    mExporter.SetDefaultFormat(extension, mFormat->GetCurrentSelection() - 1);

    if (mExporter.Ask(false, mTrackStart, mTrackEnd) && !mProject->mBatchMode)
    {
        for (int i = 0; i < mBook->GetPageCount(); i++)
            mBook->GetPage(i)->TransferDataFromWindow();

        if (mExporter.Export())
        {
            FileHistory::Global().Append(mExporter.GetAutoExportFileName().GetFullPath());
            EndModal(0);
        } else AudacityMessageBox(XO("Exporting audio failed."));
    } else AudacityMessageBox(XO("Pre-exporting failed."));
}

void ExportDialog::OnClose(wxCloseEvent& WXUNUSED(event))
{
    EndModal(0);
}