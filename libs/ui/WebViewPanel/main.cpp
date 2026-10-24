#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/stdpaths.h>
#include <wx/wx.h>

#include "WebViewPanel.h"

class WebViewPanelApp final : public wxApp {
   public:
    bool OnInit() override {
        auto* frame =
            new wxFrame(nullptr, wxID_ANY, "WebViewPanel: Neon Asteroids", wxDefaultPosition, wxSize(1024, 768));
        auto* webViewPanel = new WebViewPanel(frame);

        auto* frameSizer = new wxBoxSizer(wxVERTICAL);
        frameSizer->Add(webViewPanel, 1, wxEXPAND);
        frame->SetSizer(frameSizer);

        wxFileName gameFile(wxStandardPaths::Get().GetExecutablePath());
        gameFile.SetFullName("neon-asteroids.html");
        webViewPanel->LoadUrl(wxFileSystem::FileNameToURL(gameFile));
        frame->Show();
        webViewPanel->FocusContent();
        return true;
    }
};

wxIMPLEMENT_APP(WebViewPanelApp);
