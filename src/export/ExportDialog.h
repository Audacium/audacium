#include "ShuttleGui.h"
#include "export/Export.h"

class AUDACITY_DLL_API ExportDialog final : public wxDialogWrapper
{
public:
	ExportDialog(AudacityProject* parent);
	ExportDialog(AudacityProject* parent, NotifyingSelectedRegion &region);
	ExportDialog(AudacityProject* parent, wxString extension);

	virtual ~ExportDialog();

	int ShowModal();

private:
	void PopulateOrExchange(ShuttleGui& S);

	void OnIndexChanged(wxCommandEvent& evt);
	void OnOK(wxCommandEvent& evt);

	void OnClose(wxCloseEvent& evt);

	AudacityProject* mProject;

	Exporter mExporter;

	std::vector<ExportPlugin*> mPlugins;

	double mTrackStart = 0.0, mTrackEnd = 0.0;

	wxSimplebook* mBook = NULL;
	wxComboBox* mFormat;
	wxButton* mOK;

	wxString mFileExtension = wxEmptyString;

	DECLARE_EVENT_TABLE()
};