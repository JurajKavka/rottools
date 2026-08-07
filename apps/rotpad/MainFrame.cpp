#include "MainFrame.h"

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/stockitem.h>

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
    Bind(wxEVT_MENU, &MainFrame::HandleUndoMenuItemClick, this, wxID_UNDO);
    Bind(wxEVT_MENU, &MainFrame::HandleRedoMenuItemClick, this, wxID_REDO);
    Bind(wxEVT_MENU, &MainFrame::HandleCopyMenuItemClick, this, wxID_COPY);
    Bind(wxEVT_MENU, &MainFrame::HandleCutMenuItemClick, this, wxID_CUT);
    Bind(wxEVT_MENU, &MainFrame::HandlePasteMenuItemClick, this, wxID_PASTE);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateUndoMenuItem, this, wxID_UNDO);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateRedoMenuItem, this, wxID_REDO);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateCopyMenuItem, this, wxID_COPY);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateCutMenuItem, this, wxID_CUT);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdatePasteMenuItem, this, wxID_PASTE);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleFileBrowserMenuItemClick, this, wxID_TOGGLE_FILE_BROWSER_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleWordWrapMenuItemClick, this, wxID_WORDWRAP);
    Bind(wxEVT_TOOL, &MainFrame::HandleOpenFileMenuItemClick, this, m_fileOpenTool->GetId());
    Bind(wxEVT_TOOL, &MainFrame::HandleSaveMenuItemClick, this, m_saveTool->GetId());
    Bind(wxEVT_TOOL, &MainFrame::HandleSaveAsMenuItemClick, this, m_saveAsTool->GetId());

    // Let the active wxWidgets port supply its standard labels and accelerators.
    const long stockLabelFlags = wxSTOCK_WITH_MNEMONIC | wxSTOCK_WITH_ACCELERATOR;
    m_editMenu->FindItem(wxID_UNDO)->SetItemLabel(wxGetStockLabel(wxID_UNDO, stockLabelFlags));
    m_editMenu->FindItem(wxID_REDO)->SetItemLabel(wxGetStockLabel(wxID_REDO, stockLabelFlags));
    m_editMenu->FindItem(wxID_COPY)->SetItemLabel(wxGetStockLabel(wxID_COPY, stockLabelFlags));
    m_editMenu->FindItem(wxID_CUT)->SetItemLabel(wxGetStockLabel(wxID_CUT, stockLabelFlags));
    m_editMenu->FindItem(wxID_PASTE)->SetItemLabel(wxGetStockLabel(wxID_PASTE, stockLabelFlags));

    // The browser and editor are siblings in one splitter, matching rotreader's
    // top-level layout. FileBrowserTreePanel shows every ordinary file by default.
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_fileBrowserPanel =
        new FileBrowserTreePanel(m_mainSplitter, std::bind_front(&MainFrame::OpenTextFile, this), nullptr);
    m_textEditorPanel = new TextEditorPanel(m_mainSplitter);
    m_textEditorPanel->SetWordWrap(true);
    m_viewMenu->Check(wxID_WORDWRAP, true);

    m_mainSplitter->SetMinimumPaneSize(100);
    m_mainSplitter->Initialize(m_textEditorPanel);
    m_fileBrowserPanel->Hide();

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

void MainFrame::HandleUndoMenuItemClick(wxCommandEvent& event) {
    m_textEditorPanel->Undo();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleRedoMenuItemClick(wxCommandEvent& event) {
    m_textEditorPanel->Redo();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleCopyMenuItemClick(wxCommandEvent& event) {
    m_textEditorPanel->Copy();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleCutMenuItemClick(wxCommandEvent& event) {
    m_textEditorPanel->Cut();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandlePasteMenuItemClick(wxCommandEvent& event) {
    m_textEditorPanel->Paste();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleUpdateUndoMenuItem(wxUpdateUIEvent& event) {
    event.Enable(m_textEditorPanel->CanUndo());
}

void MainFrame::HandleUpdateRedoMenuItem(wxUpdateUIEvent& event) {
    event.Enable(m_textEditorPanel->CanRedo());
}

void MainFrame::HandleUpdateCopyMenuItem(wxUpdateUIEvent& event) {
    event.Enable(m_textEditorPanel->CanCopy());
}

void MainFrame::HandleUpdateCutMenuItem(wxUpdateUIEvent& event) {
    event.Enable(m_textEditorPanel->CanCut());
}

void MainFrame::HandleUpdatePasteMenuItem(wxUpdateUIEvent& event) {
    event.Enable(m_textEditorPanel->CanPaste());
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

void MainFrame::HandleWordWrapMenuItemClick(wxCommandEvent& event) {
    m_textEditorPanel->SetWordWrap(event.IsChecked());
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
