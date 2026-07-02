#include "MainFrame.h"

#include <wx/dnd.h>      // Required for wxFileDropTarget
#include <wx/filedlg.h>  // Required for wxFileDialog
#include <wx/msgdlg.h>   // Required for wxMessageBox

#include <fstream>  // For opening the file
#include <sstream>  // For reading the file content

#include "FileDropTarget/FileDropTarget.h"
#include "HelperFunctions.h"
#include "WebViewPanel/WebViewPanel.h"

//-----------------------------------------------------------------------------
// MainFrame implementation
//-----------------------------------------------------------------------------

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent) {
    Bind(wxEVT_MENU, &MainFrame::HandleOpenFileMenuItemClick, this, wxID_OPEN);
    Bind(wxEVT_TOOL, &MainFrame::HandleOpenFileMenuItemClick, this, fileOpenTool->GetId());

    Bind(EVT_MARKDOWN_READY, &MainFrame::OnMarkdownReady, this);
    Bind(EVT_MARKDOWN_ERROR, &MainFrame::OnMarkdownError, this);

    m_parserThread = std::make_shared<MarkdownToHtmlAsync>(this);

    // 1. Instantiate the WebViewPanel, setting this frame as its parent
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_fileBrowserPanel = new FileBrowserTreePanel(m_mainSplitter, std::bind_front(&MainFrame::HandleFileOpened, this));
    m_webViewPanel = new WebViewPanel(m_mainSplitter);

    m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_webViewPanel, 300);
    m_mainSplitter->SetMinimumPaneSize(120);  // Prevents hiding the sidebar entirely

    // 2. Vypýtame si od okna ten sizer, ktorý vygeneroval wxFormBuilder (s tlačidlom)
    wxSizer* mainSizer = this->GetSizer();

    // 3. Bezpečne pridáme WebViewPanel do tohto sizeru
    if (mainSizer) {
        wxBoxSizer* horizontalSizer = new wxBoxSizer(wxHORIZONTAL);
        // Zabezpečíme, že WebView vyplní zvyšný priestor (proporcia 1, wxEXPAND)
        mainSizer->Add(m_mainSplitter, 1, wxEXPAND | wxALL, 0);

        // inital directory list
        wxFileName initialDirectory;
        initialDirectory.AssignHomeDir();

        m_fileBrowserPanel->ListDir(initialDirectory);
    }
    Layout();

    // 4. Register drag and drop targets
    this->SetDropTarget(
        new FileDropTarget([this](const wxFileName& filePath) { m_parserThread->ParseFile(filePath); }));
}

MainFrame::~MainFrame() {}

void MainFrame::HandleOpenFileMenuItemClick(wxCommandEvent& event) {
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

    if (statusBar) {
        statusBar->SetStatusText(wxString("Loading ..."));
    }

    m_parserThread->ParseFile(openFileDialog.GetPath());
}

void MainFrame::OnMarkdownReady(MarkdownToHtmlAsyncEvent& event) {
    if (m_webViewPanel) {
        m_webViewPanel->LoadHtml(event.html);
    }
    if (statusBar) {
        statusBar->SetStatusText(event.filePath.GetAbsolutePath());
    }
    m_fileBrowserPanel->ListDir(event.filePath.GetPath());
}

void MainFrame::OnMarkdownError(MarkdownToHtmlAsyncEvent& event) {
    wxMessageBox("Error parsing markdown!", "Error", wxICON_ERROR);
    if (statusBar) {
        statusBar->SetStatusText(wxString(""));
    }
}

void MainFrame::HandleFileOpened(const wxFileName& filePath) {
    m_parserThread->ParseFile(filePath);
}
