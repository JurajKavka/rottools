#include <wx/wx.h>

#include "MarkdownPreviewPanel.h"

class MarkdownPreviewPanelApp : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MarkdownPreviewPanelApp);

bool MarkdownPreviewPanelApp::OnInit() {
    // 1. Create a Top-Level Window (Frame) to hold your panel
    wxFrame* mainFrame = new wxFrame(nullptr, wxID_ANY, "Test: Markdown Preview", wxDefaultPosition, wxSize(700, 500));

    // 2. Instantiate your custom panel, passing the mainFrame as its parent
    MarkdownPreviewPanel* sourcePanel = new MarkdownPreviewPanel(mainFrame);

    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(sourcePanel, 1, wxEXPAND | wxALL, 0);
    mainFrame->SetSizer(frameSizer);
    mainFrame->Layout();

    // 3. Show the frame (which automatically shows the child panel)
    mainFrame->Show(true);

    wxFileName file = wxFileName("README.md");

    sourcePanel->LoadFile(file);

    return true;
}
