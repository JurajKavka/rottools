#include "MainFrame.h"

#include <wx/msgdlg.h>

#include <functional>
#ifndef __WXOSX__
#include "AppIcon.h"
#include "AppIconData.h"
#endif
#include "FileBrowserTreePanel.h"
#include "HelperFunctions.h"
#include "TextEditorPanel.h"

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent) {
#ifndef __WXOSX__
    // macOS takes the window and Dock icon from AppIcon.icns in the .app bundle,
    // where SetIcons does nothing. Windows and X11 need it set explicitly.
    SetIcons(rottools::MakeIconBundle(kAppIconPngs, kAppIconPngCount));
#endif

    // The browser and editor are siblings in one splitter, matching rotreader's
    // top-level layout. FileBrowserTreePanel shows every ordinary file by default.
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_fileBrowserPanel = new FileBrowserTreePanel(m_mainSplitter, std::bind_front(&MainFrame::OpenTextFile, this),
                                                  nullptr);
    m_textEditorPanel = new TextEditorPanel(m_mainSplitter);

    m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_textEditorPanel, 200);
    m_mainSplitter->SetMinimumPaneSize(100);

    wxSizer* mainSizer = GetSizer();
    mainSizer->Add(m_mainSplitter, 1, wxEXPAND | wxALL, 0);

    wxFileName initialDirectory;
    initialDirectory.AssignHomeDir();
    m_fileBrowserPanel->ListDir(initialDirectory);

    Layout();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::OpenTextFile(const wxFileName& filePath) {
    wxFileName absolutePath(filePath);
    absolutePath.MakeAbsolute();

    if (!absolutePath.FileExists()) {
        wxMessageBox(wxString("File does not exist: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return;
    }
    if (!absolutePath.IsFileReadable()) {
        wxMessageBox(wxString("No permission to read file: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return;
    }

    wxString text;
    if (!ReadFileUtf8(absolutePath, text)) {
        wxMessageBox(wxString("Could not read file: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return;
    }

    m_textEditorPanel->LoadText(text);
    statusBar->SetStatusText(absolutePath.GetFullPath());
    m_textEditorPanel->FocusEditor();
}
