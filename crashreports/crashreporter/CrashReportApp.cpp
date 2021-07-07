#include "CrashReportApp.h"

#include <sstream>
#include <memory>

#include <wx/cmdline.h>
#include <wx/chartype.h>
#include <wx/artprov.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "google_breakpad/processor/basic_source_line_resolver.h"
#include "google_breakpad/processor/minidump_processor.h"
#include "google_breakpad/processor/process_state.h"
#include "google_breakpad/processor/minidump.h"
#include "processor/stackwalk_common.h"

#include "warning.xpm"

//Temporary solution until lib-strings is added
#define XC(msg, ctx) (wxGetTranslation(msg, wxEmptyString, ctx))

#if defined(_WIN32)
#include <locale>
#include <codecvt>
#include "client/windows/sender/crash_report_sender.h"

namespace
{
    std::wstring ToPlatformString(const std::string& utf8)
    {
        return std::wstring_convert<std::codecvt_utf8<std::wstring::traits_type::char_type>, std::wstring::traits_type::char_type>().from_bytes(utf8);
    }

    bool SendMinidump(const std::string& url, const wxString& minidumpPath, const std::map<std::string, std::string>& arguments, const wxString& commentsFilePath)
    {
        std::map<std::wstring, std::wstring> files;
        files[L"upload_file_minidump"] = minidumpPath.wc_str();
        if (!commentsFilePath.empty())
        {
            files[wxFileName(commentsFilePath).GetFullName().wc_str()] = commentsFilePath.wc_str();
        }

        std::map<std::wstring, std::wstring> parameters;
        for (auto& p : arguments)
        {
            parameters[ToPlatformString(p.first)] = ToPlatformString(p.second);
        }

        google_breakpad::CrashReportSender sender(L"");

        auto result = sender.SendCrashReport(
            ToPlatformString(url),
            parameters,
            files,
            nullptr
        );
        return result == google_breakpad::RESULT_SUCCEEDED;
    }
}
#else

#include "common/linux/http_upload.h"

namespace
{
    bool SendMinidump(const std::string& url, const wxString& minidumpPath, const std::map<std::string, std::string>& arguments, const wxString& commentsFilePath)
    {
        std::map<std::string, std::string> files;
        files["upload_file_minidump"] = minidumpPath.ToStdString();
        if (!commentsFilePath.empty())
        {
            files["comments.txt"] = commentsFilePath.ToStdString();
        }

        std::string response, error;
        bool success = google_breakpad::HTTPUpload::SendRequest(
            url,
            arguments,
            files,
            std::string(),
            std::string(),
            std::string(),
            &response,
            NULL,
            &error);
            
        return success;
    }
}

#endif

IMPLEMENT_APP(CrashReportApp);
namespace
{
    std::map<std::string, std::string> parseArguments(const std::string& str)
    {
        int TOKEN_IDENTIFIER{ 0 };
        constexpr int TOKEN_EQ{ 1 };
        constexpr int TOKEN_COMMA{ 2 };
        constexpr int TOKEN_VALUE{ 3 };

        int i = 0;

        std::string key;
        int state = TOKEN_COMMA;
        std::map<std::string, std::string> result;
        while (true)
        {
            if (str[i] == 0)
                break;
            else if (isspace(str[i]))
                ++i;
            else if (isalpha(str[i]))
            {
                if (state != TOKEN_COMMA)
                    throw std::logic_error("malformed parameters string: unexpected identifier");

                int begin = i;
                while (isalnum(str[i]))
                    ++i;

                key = str.substr(begin, i - begin);
                state = TOKEN_IDENTIFIER;
            }
            else if (str[i] == '=')
            {
                if (state != TOKEN_IDENTIFIER)
                    throw std::logic_error("malformed parameters string: unexpected '=' symbol");
                ++i;
                state = TOKEN_EQ;
            }
            else if (str[i] == '\"')
            {
                if (state != TOKEN_EQ)
                    throw std::logic_error("malformed parameters string: unexpected '\"' symbol");

                int begin = ++i;
                while (true)
                {
                    if (str[i] == 0)
                        throw std::logic_error("unterminated string literal");
                    else if (str[i] == '\"')
                    {
                        if (i > begin)
                            result[key] = str.substr(begin, i - begin);
                        else
                            result[key] = std::string();
                        ++i;
                        state = TOKEN_VALUE;
                        break;
                    }
                    ++i;
                }
            }
            else if (str[i] == ',')
            {
                if (state != TOKEN_VALUE)
                    throw std::logic_error("malformed parameters string: unexpected ',' symbol");
                state = TOKEN_COMMA;
                ++i;
            }
            else
                throw std::logic_error("malformed parameters string");
        }
        if (state != TOKEN_VALUE)
            throw std::logic_error("malformed parameters string");

        return result;
    }

    void PrintMinidump(google_breakpad::Minidump& minidump)
    {
        google_breakpad::BasicSourceLineResolver resolver;
        google_breakpad::MinidumpProcessor minidumpProcessor(nullptr, &resolver);
        google_breakpad::MinidumpThreadList::set_max_threads(std::numeric_limits<uint32_t>::max());
        google_breakpad::MinidumpMemoryList::set_max_regions(std::numeric_limits<uint32_t>::max());
        
        google_breakpad::ProcessState processState;
        
        if (minidumpProcessor.Process(&minidump, &processState) != google_breakpad::PROCESS_OK)
        {
            printf("Failed to process minidump");
        }
        else
        {
            google_breakpad::PrintProcessState(processState, true, &resolver);
        }
    }

    wxString MakeDumpString(google_breakpad::Minidump& minidump, const wxString& temp)
    {
#if _WIN32
        auto stream = _wfreopen(temp.wc_str(), L"w+", stdout);
#else
        auto stream = freopen(temp.utf8_str().data(), "w+", stdout);
#endif
        if (stream == NULL)
            throw std::runtime_error("Failed to print minidump: cannot open temp file");
        PrintMinidump(minidump);
        fflush(stdout);

        auto length = ftell(stream);
        std::vector<char> bytes(length);
        fseek(stream, 0, SEEK_SET);
        fread(&bytes[0], 1, length, stream);
        fclose(stream);
        
#if _WIN32
        _wremove(temp.wc_str());
#else
        remove(temp.utf8_str().data());
#endif

        return wxString::From8BitData(&bytes[0], bytes.size());
    }

    wxString MakeHeaderString(google_breakpad::Minidump& minidump)
    {
        if (auto exception = minidump.GetException())
        {
            if (auto rawException = exception->exception())
            {
                // i18n-hint C++ programming assertion
                return wxString::Format(_("Exception code 0x%x"), rawException->exception_record.exception_code);
            }
            else
            {
                // i18n-hint C++ programming assertion
                return _("Unknown exception");
            }
        }
        else if (auto assertion = minidump.GetAssertion())
        {
            auto expression = assertion->expression();
            if (!expression.empty())
            {
                return expression;
            }
        }
        return _("Unknown error");
    }

    void DoShowCrashReportFrame(const wxString& header, const wxString& dump, const std::function<bool(const wxString& comment)>& onSend)
    {
        static constexpr int MaxUserCommentLength = 2000;

        auto frame = new wxFrame(
            nullptr, 
            wxID_ANY, 
            _("Problem Report for Audacium"),
            wxDefaultPosition, 
            wxDefaultSize, 
            wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)//disable frame resize
        );
        frame->SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
        
        auto mainLayout = new wxBoxSizer(wxVERTICAL);
        
        auto headerText = new wxStaticText(frame, wxID_ANY, header);
        headerText->SetFont(wxFont(wxFontInfo().Bold()));

        auto headerLayout = new wxBoxSizer(wxHORIZONTAL);
        headerLayout->Add(new wxStaticBitmap(frame, wxID_ANY, wxIcon(warning)));
        headerLayout->AddSpacer(5);
        headerLayout->Add(headerText, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));

        auto buttonsLayout = new wxBoxSizer(wxHORIZONTAL);
        
        wxTextCtrl* commentCtrl = nullptr;
        if (onSend != nullptr)
        {
            commentCtrl = new wxTextCtrl(frame, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, 100), wxTE_MULTILINE);
            commentCtrl->SetMaxLength(MaxUserCommentLength);
        }

        if (onSend != nullptr)
        {
            auto okButton = new wxButton(frame, wxID_ANY, XC("&Don't send", "crash reporter button"));
            auto sendButton = new wxButton(frame, wxID_ANY, XC("&Send", "crash reporter button"));

            okButton->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
                {
                    frame->Close(true);
                });
            sendButton->Bind(wxEVT_BUTTON, [frame, commentCtrl, onSend](wxCommandEvent&)
                { 
                    if (onSend(commentCtrl->GetValue()))
                    {
                        frame->Close(true);
                    }
                });

            buttonsLayout->Add(okButton);
            buttonsLayout->AddSpacer(5);
            buttonsLayout->Add(sendButton);
        }
        else
        {
            auto okButton = new wxButton(frame, wxID_OK, wxT("OK"));
            okButton->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
                {
                    frame->Close(true);
                });
            buttonsLayout->Add(okButton);
        }

        mainLayout->Add(headerLayout, wxSizerFlags().Border(wxALL));
        if (onSend != nullptr)
        {
            mainLayout->AddSpacer(5);
            mainLayout->Add(new wxStaticText(frame, wxID_ANY, _("Click \"Send\" to submit the report to Audacium. This information is collected anonymously.")), wxSizerFlags().Border(wxALL));
        }
        mainLayout->AddSpacer(10);
        mainLayout->Add(new wxStaticText(frame, wxID_ANY, _("Problem details")), wxSizerFlags().Border(wxALL));
        
        auto dumpTextCtrl = new wxTextCtrl(frame, wxID_ANY, dump, wxDefaultPosition, wxSize(500, 300), wxTE_RICH | wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP);
        dumpTextCtrl->SetFont(wxFont(wxFontInfo().Family(wxFONTFAMILY_TELETYPE)));
        dumpTextCtrl->ShowPosition(0);//scroll to top
        mainLayout->Add(dumpTextCtrl, wxSizerFlags().Border(wxALL).Expand());

        if (onSend != nullptr)
        {
            mainLayout->AddSpacer(10);
            mainLayout->Add(new wxStaticText(frame, wxID_ANY, _("Comments")), wxSizerFlags().Border(wxALL));
            mainLayout->Add(commentCtrl, wxSizerFlags().Border(wxALL).Expand());
        }

        mainLayout->Add(buttonsLayout, wxSizerFlags().Border(wxALL).Align(wxALIGN_RIGHT));
        frame->SetSizerAndFit(mainLayout);

        frame->Show(true);
    }
}


bool CrashReportApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;
    
    if (mSilent)
    {
        if (!mURL.empty())
            SendMinidump(mURL, mMinidumpPath, mArguments, wxEmptyString);
    }
    else
    {
        static std::unique_ptr<wxLocale> sLocale(new wxLocale(wxLANGUAGE_DEFAULT));
#if defined(__WXOSX__)
        sLocale->AddCatalogLookupPathPrefix(wxT("../Resources"));
#elif defined(__WXMSW__)
        sLocale->AddCatalogLookupPathPrefix(wxT("Languages"));
#elif defined(__WXGTK__)
        sLocale->AddCatalogLookupPathPrefix(wxT("./locale"));
        sLocale->AddCatalogLookupPathPrefix(wxString::Format(wxT("%s/share/locale"), wxT(INSTALL_PREFIX)));
#endif
        sLocale->AddCatalog("audacium");
        sLocale->AddCatalog("wxstd");
        
        google_breakpad::Minidump minidump(mMinidumpPath.ToStdString(), false);
        if (minidump.Read())
        {
            SetExitOnFrameDelete(true);

            wxFileName temp(mMinidumpPath);
            temp.SetExt("tmp");

            try
            {
                ShowCrashReport(MakeHeaderString(minidump), MakeDumpString(minidump, temp.GetFullPath()));
            }
            catch (std::exception& e)
            {
                wxMessageBox(e.what());
                return false;
            }
            return true;
        }
    }
    return false;
}

void CrashReportApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    static const wxCmdLineEntryDesc cmdLineEntryDesc[] =
    {
         { wxCMD_LINE_SWITCH, "h", "help", "Display help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
         { wxCMD_LINE_SWITCH, "s", "silent", "Send without displaying the confirmation dialog" },
         { wxCMD_LINE_OPTION, "u", "url", "Crash report server URL", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
         { wxCMD_LINE_OPTION, "a", "args", "A set of arguments to send", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
         { wxCMD_LINE_PARAM,  NULL, NULL, "path to minidump file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
         { wxCMD_LINE_NONE }
    };

    parser.SetDesc(cmdLineEntryDesc);
    
    wxApp::OnInitCmdLine(parser);
}

bool CrashReportApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    wxString url;
    wxString arguments;
    if (parser.Found("u", &url))
    {
        mURL = url.ToStdString();
    }
    if (parser.Found("a", &arguments))
    {
        try
        {
            mArguments = parseArguments(arguments.ToStdString());
        }
        catch (std::exception& e)
        {
            wxMessageBox(e.what());
            return false;
        }
    }
    mMinidumpPath = parser.GetParam(0);
    mSilent = parser.Found("s");
    
    return wxApp::OnCmdLineParsed(parser);
}

void CrashReportApp::ShowCrashReport(const wxString& header, const wxString& text)
{
    if (mURL.empty())
    {
        DoShowCrashReportFrame(header, text, nullptr);
    }
    else
    {
        DoShowCrashReportFrame(header, text, [this](const wxString& comments)
            {
                wxString commentsFilePath;
                if (!comments.empty())
                {
                    wxFileName temp(mMinidumpPath);
                    temp.SetName(temp.GetName() + "-comments");
                    temp.SetExt("txt");
                    commentsFilePath = temp.GetFullPath();
                    wxFile file;
                    if (file.Open(commentsFilePath, wxFile::write))
                    {
                        file.Write(comments);
                        file.Close();
                    }
                }

                auto result = SendMinidump(mURL, mMinidumpPath, mArguments, commentsFilePath);
                if (!commentsFilePath.empty())
                    wxRemoveFile(commentsFilePath);

                if (!result)
                {
                    wxMessageBox(_("Failed to send crash report"));
                }
                return result;
            });
    }
}
