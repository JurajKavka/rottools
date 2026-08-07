#include "MainFrame.h"

#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include <functional>
#ifndef __WXOSX__
#include "AppIcon.h"
#include "AppIconData.h"
#endif
#include "FileBrowserTreePanel.h"
#include "HelperFunctions.h"
#include "TextEditorPanel.h"

namespace {
constexpr auto kTextFileWildcard =
    "Supported text files (*.txt;*.json;*.csv;*.md;*.sql)|*.txt;*.json;*.csv;*.md;*.sql|All files (*.*)|*.*";
}

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent) {
#ifndef __WXOSX__
    // macOS takes the window and Dock icon from AppIcon.icns in the .app bundle,
    // where SetIcons does nothing. Windows and X11 need it set explicitly.
    SetIcons(rottools::MakeIconBundle(kAppIconPngs, kAppIconPngCount));
#endif

    Bind(wxEVT_MENU, &MainFrame::HandleOpenFileMenuItemClick, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainFrame::HandleSaveMenuItemClick, this, wxID_SAVE);
    Bind(wxEVT_MENU, &MainFrame::HandleSaveAsMenuItemClick, this, wxID_SAVEAS);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleFileBrowserMenuItemClick, this, wxID_TOGGLE_FILE_BROWSER_MENU_ITEM);
    Bind(wxEVT_TOOL, &MainFrame::HandleOpenFileMenuItemClick, this, m_fileOpenTool->GetId());
    Bind(wxEVT_TOOL, &MainFrame::HandleSaveMenuItemClick, this, m_saveTool->GetId());
    Bind(wxEVT_TOOL, &MainFrame::HandleSaveAsMenuItemClick, this, m_saveAsTool->GetId());

    // The browser and editor are siblings in one splitter, matching rotreader's
    // top-level layout. FileBrowserTreePanel shows every ordinary file by default.
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_fileBrowserPanel =
        new FileBrowserTreePanel(m_mainSplitter, std::bind_front(&MainFrame::OpenTextFile, this), nullptr);
    m_textEditorPanel = new TextEditorPanel(m_mainSplitter);

    m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_textEditorPanel, m_fileBrowserWidth);
    m_mainSplitter->SetMinimumPaneSize(100);

    wxSizer* mainSizer = GetSizer();
    mainSizer->Add(m_mainSplitter, 1, wxEXPAND | wxALL, 0);

    wxFileName initialDirectory;
    initialDirectory.AssignHomeDir();
    m_fileBrowserPanel->ListDir(initialDirectory);

    Layout();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleOpenFileMenuItemClick(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "Open Text File", "", "", kTextFileWildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    OpenTextFile(wxFileName(openFileDialog.GetPath()));
}

void MainFrame::HandleSaveMenuItemClick(wxCommandEvent& event) {
    if (!m_currentFile.IsOk()) {
        HandleSaveAsMenuItemClick(event);
        return;
    }

    SaveTextFile(m_currentFile);
}

void MainFrame::HandleSaveAsMenuItemClick(wxCommandEvent& event) {
    wxString defaultDirectory;
    wxString defaultFileName;
    if (m_currentFile.IsOk()) {
        defaultDirectory = m_currentFile.GetPath();
        defaultFileName = m_currentFile.GetFullName();
    }

    wxFileDialog saveFileDialog(this, "Save Text File", defaultDirectory, defaultFileName, kTextFileWildcard,
                                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    SaveTextFile(wxFileName(saveFileDialog.GetPath()));
}

void MainFrame::HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event) {
    if (m_mainSplitter->IsSplit()) {
        // Restore the user's last sash position rather than resetting the
        // browser to its initial width every time it is shown.
        int sashPosition = m_mainSplitter->GetSashPosition();
        if (sashPosition > 0) {
            m_fileBrowserWidth = sashPosition;
        }
        m_mainSplitter->Unsplit(m_fileBrowserPanel);
    } else {
        m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_textEditorPanel, m_fileBrowserWidth);
    }

    m_textEditorPanel->FocusEditor();
}

bool MainFrame::SaveTextFile(const wxFileName& filePath) {
    wxFileName absolutePath(filePath);
    absolutePath.MakeAbsolute();

    if (absolutePath.FileExists() && !absolutePath.IsFileWritable()) {
        wxMessageBox(wxString("No permission to write file: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return false;
    }

    if (!WriteFileUtf8(absolutePath, m_textEditorPanel->GetText())) {
        wxMessageBox(wxString("Could not save file: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return false;
    }

    const bool pathChanged = !m_currentFile.IsOk() || !m_currentFile.SameAs(absolutePath);
    m_currentFile = absolutePath;
    m_textEditorPanel->MarkSaved();
    statusBar->SetStatusText(wxString("Saved ") + absolutePath.GetFullPath());

    if (pathChanged) {
        if (m_fileBrowserPanel->IsShowingDir(absolutePath.GetPath())) {
            m_fileBrowserPanel->ReloadCurrentDir();
        } else {
            m_fileBrowserPanel->ListDir(absolutePath.GetPath());
        }
    }

    m_textEditorPanel->FocusEditor();
    return true;
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

    // Keep the browser in sync for files opened by the desktop shell, the Open
    // dialog, or another component rather than from the browser itself.
    if (!m_fileBrowserPanel->IsShowingDir(absolutePath.GetPath())) {
        m_fileBrowserPanel->ListDir(absolutePath.GetPath());
    }

    m_currentFile = absolutePath;
    m_textEditorPanel->LoadText(text);
    statusBar->SetStatusText(absolutePath.GetFullPath());
    m_textEditorPanel->FocusEditor();
}
