#include <wx/wx.h>

#include "ScintillaTextEditorPanel.h"

class ScintillaTextEditorPanelApp final : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(ScintillaTextEditorPanelApp);

bool ScintillaTextEditorPanelApp::OnInit() {
    auto* mainFrame =
        new wxFrame(nullptr, wxID_ANY, "Test: Scintilla Text Editor", wxDefaultPosition, wxSize(800, 600));
    auto* editorPanel = new ScintillaTextEditorPanel(mainFrame);

    auto* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(editorPanel, 1, wxEXPAND | wxALL, 0);
    mainFrame->SetSizer(frameSizer);
    mainFrame->Layout();
    mainFrame->Show(true);

    editorPanel->FocusEditor();
    return true;
}
