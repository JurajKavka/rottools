#include <wx/wx.h>

#include "TextEditorPanel.h"

class TextEditorPanelApp final : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(TextEditorPanelApp);

bool TextEditorPanelApp::OnInit() {
    auto* mainFrame = new wxFrame(nullptr, wxID_ANY, "Test: Text Editor", wxDefaultPosition, wxSize(800, 600));
    auto* editorPanel = new TextEditorPanel(mainFrame);

    auto* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(editorPanel, 1, wxEXPAND | wxALL, 0);
    mainFrame->SetSizer(frameSizer);
    mainFrame->Layout();
    mainFrame->Show(true);

    editorPanel->FocusEditor();
    return true;
}
