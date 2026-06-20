#include "MainFrame.h"

#include <wx/filedlg.h>  // Required for wxFileDialog
#include <wx/msgdlg.h>   // Required for wxMessageBox
#include <wx/dnd.h>      // Required for wxFileDropTarget

#include <fstream>  // For opening the file
#include <sstream>  // For reading the file content

#include "HelperFunctions.h"
#include "MarkdownToHtml.h"
#include "WebViewPanel/WebViewPanel.h"
#include "FileDropTarget/FileDropTarget.h"

//-----------------------------------------------------------------------------
// MainFrame implementation
//-----------------------------------------------------------------------------

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent) {
    Bind(wxEVT_MENU, &MainFrame::HandleOpenFileMenuItemClick, this, wxID_OPEN);
    Bind(wxEVT_TOOL, &MainFrame::HandleOpenFileMenuItemClick, this, fileOpenTool->GetId());

    // 1. Instantiate the WebViewPanel, setting this frame as its parent
    m_webViewPanel = new WebViewPanel(this);

    // 2. Vypýtame si od okna ten sizer, ktorý vygeneroval wxFormBuilder (s tlačidlom)
    wxSizer* mainSizer = this->GetSizer();

    // 3. Bezpečne pridáme WebViewPanel do tohto sizeru
    if (mainSizer) {
        // Zabezpečíme, že WebView vyplní zvyšný priestor (proporcia 1, wxEXPAND)
        mainSizer->Add(m_webViewPanel, 1, wxEXPAND | wxALL, 0);
    }
    Layout();

    // 4. Register drag and drop targets
    this->SetDropTarget(new FileDropTarget([this](const wxString& filePath) {
        this->OpenFile(filePath);
    }));
    m_webViewPanel->SetDropTarget(new FileDropTarget([this](const wxString& filePath) {
        this->OpenFile(filePath);
    }));
}

MainFrame::~MainFrame() {}

void MainFrame::OpenFile(const wxString& filePath) {
    std::ifstream file(filePath.ToStdString());
    if (!file.is_open()) {
        wxMessageBox("Could not open the selected file.", "Error", wxICON_ERROR);
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string markdownStr = buffer.str();

    wxString rawMarkdown = wxString::FromUTF8(markdownStr);

    wxString htmlContent = ConvertMarkdownToHtml(rawMarkdown);

    if (m_webViewPanel) {
        m_webViewPanel->LoadHtml(htmlContent);
    }

    if (statusBar) {
        statusBar->SetStatusText(filePath);
    }
}

void MainFrame::HandleOpenFileMenuItemClick(wxCommandEvent& event) {
    printLog("Click open file");

    wxFileDialog openFileDialog(
        this,                  // Parent window
        "Open Markdown File",  // Dialog Title
        "",                    // Default directory (empty means current)
        "",                    // Default filename
        "Markdown files (*.md;*.markdown)|*.md;*.markdown|All files (*.*)|*.*",  // File extensions filter
        wxFD_OPEN | wxFD_FILE_MUST_EXIST  // Flags: Open mode & force file existence
    );

    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    OpenFile(openFileDialog.GetPath());
}
