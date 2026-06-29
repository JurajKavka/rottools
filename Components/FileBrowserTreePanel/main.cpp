#include <wx/wx.h>

#include "FileBrowserTreePanel.h"

class FileBrowserTreePanelApp : public wxApp {
   public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(FileBrowserTreePanelApp);

bool FileBrowserTreePanelApp::OnInit() {
    // 1. Create a Top-Level Window (Frame) to hold your panel
    // We pass 'nullptr' because this frame has no parent; it is the main app window.
    wxFrame* mainFrame = new wxFrame(nullptr, wxID_ANY, "Test: File Browser", wxDefaultPosition, wxSize(400, 600));

    // 2. Instantiate your custom panel, passing the mainFrame as its parent!
    FileBrowserTreePanel* treePanel = new FileBrowserTreePanel(mainFrame);

    // Optional: If your panel doesn't auto-expand, force the frame to fit it via a sizer
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(treePanel, 1, wxEXPAND | wxALL, 0);
    mainFrame->SetSizer(frameSizer);
    mainFrame->Layout();

    // 3. Show the frame (which automatically shows the child panel)
    mainFrame->Show(true);

    std::filesystem::path currentDir = std::filesystem::current_path();
    treePanel->ListDir(currentDir);

    return true;
}
