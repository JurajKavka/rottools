#include "MainFrame.h"

#include <wx/aboutdlg.h>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/fontdlg.h>
#include <wx/msgdlg.h>
#include <wx/stockitem.h>
#include <wx/textdlg.h>
#include <wx/utils.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <utility>
#ifndef __WXOSX__
#include "AppIcon.h"
#include "AppIconData.h"
#endif
#include "AppConfigFunctions.h"
#include "FileBrowserTreePanel.h"
#include "FsWatcher.h"
#include "HelperFunctions.h"
#include "TextEditorPanel.h"
#include "version.h"

namespace {
constexpr auto kTextFileWildcard =
    "Supported text files (*.txt;*.json;*.csv;*.md;*.sql)|*.txt;*.json;*.csv;*.md;*.sql|All files (*.*)|*.*";
constexpr auto kApplicationTitle = "ROT Pad";

TextSearchOptions GetTextSearchOptions(const wxFindDialogEvent& event) {
    return {.backwards = (event.GetFlags() & wxFR_DOWN) == 0,
            .matchCase = (event.GetFlags() & wxFR_MATCHCASE) != 0,
            .wholeWord = (event.GetFlags() & wxFR_WHOLEWORD) != 0};
}

bool ReadDocument(const wxFileName& filePath, wxString& text) {
    return ReadTextFileUtf8(filePath, text, std::numeric_limits<std::size_t>::max()) == TextFileReadResult::Ok;
}
}  // namespace

MainFrame::MainFrame(wxWindow* parent, const wxFileName& initialDirectory) : MainFrameWx(parent) {
    SetTitle(kApplicationTitle);

#ifndef __WXOSX__
    // macOS takes the window and Dock icon from AppIcon.icns in the .app bundle,
    // where SetIcons does nothing. Windows and X11 need it set explicitly.
    SetIcons(rottools::MakeIconBundle(kAppIconPngs, kAppIconPngCount));
#endif

    Bind(wxEVT_MENU, &MainFrame::HandleNewFileMenuItemClick, this, wxID_NEW);
    Bind(wxEVT_MENU, &MainFrame::HandleNewWindowMenuItemClick, this, wxID_NEW_WINDOW_MENU_ITEM);
    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::HandleCloseWindow, this);
    Bind(wxEVT_MENU, &MainFrame::HandleCloseMenuItemClick, this, wxID_CLOSE);
    Bind(wxEVT_MENU, &MainFrame::HandleAboutMenuItemClick, this, wxID_ABOUT);
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
    Bind(wxEVT_MENU, &MainFrame::HandleSelectAllMenuItemClick, this, wxID_SELECTALL);
    Bind(wxEVT_MENU, &MainFrame::HandleFindMenuItemClick, this, wxID_FIND);
    Bind(wxEVT_MENU, &MainFrame::HandleReplaceMenuItemClick, this, wxID_REPLACE);
    Bind(wxEVT_FIND, &MainFrame::HandleFindDialogFind, this);
    Bind(wxEVT_FIND_NEXT, &MainFrame::HandleFindDialogFind, this);
    Bind(wxEVT_FIND_REPLACE, &MainFrame::HandleFindDialogReplace, this);
    Bind(wxEVT_FIND_REPLACE_ALL, &MainFrame::HandleFindDialogReplaceAll, this);
    Bind(wxEVT_FIND_CLOSE, &MainFrame::HandleFindDialogClose, this);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateEditorMenuItem, this, wxID_SELECTALL);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateEditorMenuItem, this, wxID_FIND);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateEditorMenuItem, this, wxID_REPLACE);
    Bind(wxEVT_TEXT, &MainFrame::HandleEditorTextChanged, this);
#ifdef __WXOSX__
    Bind(wxEVT_MENU_HIGHLIGHT, &MainFrame::HandleMenuHighlight, this);
#endif
    Bind(wxEVT_MENU, &MainFrame::HandleToggleFileBrowserMenuItemClick, this, wxID_TOGGLE_FILE_BROWSER_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleWordWrapMenuItemClick, this, wxID_WORDWRAP);
    Bind(wxEVT_MENU, &MainFrame::HandleFontMenuItemClick, this, wxID_FONT);
    Bind(wxEVT_TOOL, &MainFrame::HandleNewFileMenuItemClick, this, m_newFileTool->GetId());
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
    m_editMenu->FindItem(wxID_SELECTALL)->SetItemLabel(wxGetStockLabel(wxID_SELECTALL, stockLabelFlags));
    m_editMenu->FindItem(wxID_FIND)->SetItemLabel(wxGetStockLabel(wxID_FIND, stockLabelFlags));
    m_editMenu->FindItem(wxID_REPLACE)->SetItemLabel(wxGetStockLabel(wxID_REPLACE, stockLabelFlags));

    // The browser and editor are siblings in one splitter, matching rotreader's
    // top-level layout. Start with the file types from the Open dialog.
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_fileBrowserPanel = new FileBrowserTreePanel(
        m_mainSplitter,
        {.onFileOpened = std::bind_front(&MainFrame::HandleOpenTextFile, this),
         .onDirectoryChanged = std::bind_front(&MainFrame::HandleDirectoryChanged, this),
         .onHomeRequested = std::bind_front(&MainFrame::HandleFileBrowserHomeRequested, this),
         .onCloseRequested = std::bind_front(&MainFrame::HandleFileBrowserCloseRequested, this),
         .selectNewFolderName = std::bind_front(&MainFrame::HandleSelectBrowserFolderName, this),
         .onCreateFolderError = std::bind_front(&MainFrame::HandleBrowserCreateFolderError, this),
         .onConfirmMoveToTrash = std::bind_front(&MainFrame::HandleConfirmBrowserMoveToTrash, this),
         .onConfirmPermanentlyDelete = std::bind_front(&MainFrame::HandleConfirmBrowserPermanentlyDelete, this),
         .onPathDeleted = std::bind_front(&MainFrame::HandleBrowserPathDeleted, this),
         .onDeleteError = std::bind_front(&MainFrame::HandleBrowserDeleteError, this),
         .onMoveError = std::bind_front(&MainFrame::HandleFileTreeBrowserMoveError, this),
         .onBeforeMove = std::bind_front(&MainFrame::HandleFileTreeBrowserBeforeMove, this),
         .onPathMoved = std::bind_front(&MainFrame::HandleFileTreeBrowserPathMoved, this),
         .onCopyError = std::bind_front(&MainFrame::HandleFileTreeBrowserCopyError, this),
         .onConfirmOverwrite = std::bind_front(&MainFrame::HandleFileTreeBrowserConfirmOverwrite, this),
         .onPathCopied = std::bind_front(&MainFrame::HandleFileTreeBrowserDestinationUpdated, this)},
        {{_("Supported text files"), {"txt", "json", "csv", "md", "sql"}},
         {_("All files"), {FileBrowserTreePanel::kFilterAllFiles}}});
    m_textEditorPanel = new TextEditorPanel(m_mainSplitter);
    m_textEditorPanel->SetEditorFont(rottools::LoadEditorFont(m_textEditorPanel->GetEditorFont()));
    m_textEditorPanel->SetWordWrap(true);
    m_viewMenu->Check(wxID_WORDWRAP, true);

    m_mainSplitter->SetMinimumPaneSize(100);
    m_mainSplitter->Initialize(m_textEditorPanel);
    m_fileBrowserPanel->Hide();
    m_viewMenu->Check(wxID_TOGGLE_FILE_BROWSER_MENU_ITEM, false);

    wxSizer* mainSizer = GetSizer();
    mainSizer->Add(m_mainSplitter, 1, wxEXPAND | wxALL, 0);

    wxFileName directory = initialDirectory;
    if (!directory.IsOk() || !directory.DirExists()) {
        directory.AssignHomeDir();
    }
    m_fileBrowserPanel->ListDir(directory);

    Layout();
    UpdateDocumentTitle();
    m_textEditorPanel->FocusEditor();
}

MainFrame::~MainFrame() {
    // Watchers must stop before their callback targets are destroyed.
    m_documentWatcher.reset();
    m_browserWatcher.reset();
    if (m_findDialog != nullptr) {
        m_findDialog->Destroy();
    }
}

void MainFrame::HandleNewFileMenuItemClick(wxCommandEvent&) {
    if (ConfirmSaveBeforeDiscard()) {
        ResetDocument();
    }
}

void MainFrame::HandleNewWindowMenuItemClick(wxCommandEvent&) {
    (new MainFrame(nullptr, m_fileBrowserPanel->GetCurrentDirectory()))->Show(true);
}

void MainFrame::HandleCloseWindow(wxCloseEvent& event) {
    if (event.CanVeto() && !ConfirmSaveBeforeDiscard()) {
        event.Veto();
        return;
    }
    event.Skip();
}

void MainFrame::HandleCloseMenuItemClick(wxCommandEvent&) {
    Close();
}

void MainFrame::HandleAboutMenuItemClick(wxCommandEvent&) {
    wxAboutDialogInfo info;
    info.SetName(kApplicationTitle);
    info.SetVersion(ROTPAD_VERSION_STRING);
    info.SetDescription(_("A simple native plain-text editor") + wxS("\n\n") +
                        wxString::Format(_("Built with %s"), wxGetLibraryVersionInfo().GetVersionString()));
    wxAboutBox(info, this);
}

void MainFrame::HandleOpenFileMenuItemClick(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "Open Text File", "", "", kTextFileWildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    HandleOpenTextFile(wxFileName(openFileDialog.GetPath()));
}

void MainFrame::HandleSaveMenuItemClick(wxCommandEvent& event) {
    (void)SaveDocument();
}

void MainFrame::HandleSaveAsMenuItemClick(wxCommandEvent& event) {
    (void)SaveDocumentAs();
}

bool MainFrame::SaveDocument() {
    return m_currentFile.IsOk() ? SaveTextFile(m_currentFile) : SaveDocumentAs();
}

bool MainFrame::SaveDocumentAs() {
    wxString defaultDirectory;
    wxString defaultFileName;
    if (m_currentFile.IsOk()) {
        defaultDirectory = m_currentFile.GetPath();
        defaultFileName = m_currentFile.GetFullName();
    } else {
        defaultDirectory = m_fileBrowserPanel->GetCurrentDirectory().GetFullPath();
        defaultFileName = _("Untitled.txt");
    }

    wxFileDialog saveFileDialog(this, "Save Text File", defaultDirectory, defaultFileName, kTextFileWildcard,
                                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return false;
    }

    return SaveTextFile(wxFileName(saveFileDialog.GetPath()));
}

void MainFrame::HandleUndoMenuItemClick(wxCommandEvent& event) {
    m_textEditorPanel->Undo();
    UpdateDocumentTitle();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleRedoMenuItemClick(wxCommandEvent& event) {
    m_textEditorPanel->Redo();
    UpdateDocumentTitle();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleCopyMenuItemClick(wxCommandEvent& event) {
    if (ContainsFocus(m_fileBrowserPanel)) {
        m_fileBrowserPanel->Copy();
    } else {
        m_textEditorPanel->Copy();
        m_textEditorPanel->FocusEditor();
    }
}

void MainFrame::HandleCutMenuItemClick(wxCommandEvent& event) {
    if (ContainsFocus(m_fileBrowserPanel)) {
        m_fileBrowserPanel->Cut();
    } else {
        m_textEditorPanel->Cut();
        m_textEditorPanel->FocusEditor();
    }
}

void MainFrame::HandlePasteMenuItemClick(wxCommandEvent& event) {
    if (ContainsFocus(m_fileBrowserPanel)) {
        m_fileBrowserPanel->Paste();
    } else {
        m_textEditorPanel->Paste();
        m_textEditorPanel->FocusEditor();
    }
}

void MainFrame::HandleUpdateUndoMenuItem(wxUpdateUIEvent& event) {
    event.Enable(ContainsFocus(m_textEditorPanel) && m_textEditorPanel->CanUndo());
}

void MainFrame::HandleUpdateRedoMenuItem(wxUpdateUIEvent& event) {
    event.Enable(ContainsFocus(m_textEditorPanel) && m_textEditorPanel->CanRedo());
}

void MainFrame::HandleUpdateCopyMenuItem(wxUpdateUIEvent& event) {
    event.Enable(ContainsFocus(m_fileBrowserPanel) ? m_fileBrowserPanel->CanCopy() : m_textEditorPanel->CanCopy());
}

void MainFrame::HandleUpdateCutMenuItem(wxUpdateUIEvent& event) {
    event.Enable(ContainsFocus(m_fileBrowserPanel) ? m_fileBrowserPanel->CanCut() : m_textEditorPanel->CanCut());
}

void MainFrame::HandleUpdatePasteMenuItem(wxUpdateUIEvent& event) {
    event.Enable(ContainsFocus(m_fileBrowserPanel) ? m_fileBrowserPanel->CanPaste() : m_textEditorPanel->CanPaste());
}

void MainFrame::HandleSelectAllMenuItemClick(wxCommandEvent&) {
    m_textEditorPanel->SelectAll();
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleFindMenuItemClick(wxCommandEvent&) {
    ShowFindDialog(false);
}

void MainFrame::HandleReplaceMenuItemClick(wxCommandEvent&) {
    ShowFindDialog(true);
}

void MainFrame::ShowFindDialog(bool replace) {
    if (m_findDialog != nullptr) {
        if (replace == m_findDialogIsReplace) {
            m_findDialog->Raise();
            m_findDialog->SetFocus();
            return;
        }
        m_findDialog->Destroy();
        m_findDialog = nullptr;
    }
    const wxString selection = m_textEditorPanel->GetSelectedText();
    if (!selection.IsEmpty() && selection.length() <= 150 && selection.Find('\n') == wxNOT_FOUND &&
        selection.Find('\r') == wxNOT_FOUND) {
        m_findData.SetFindString(selection);
    }
    m_findDialogIsReplace = replace;
    m_findDialog = new wxFindReplaceDialog(this, &m_findData, replace ? _("Replace") : _("Find"),
                                           replace ? wxFR_REPLACEDIALOG : 0);
    m_findDialog->Show();
}

void MainFrame::HandleFindDialogFind(wxFindDialogEvent& event) {
    if (event.GetDialog() == m_findDialog &&
        !m_textEditorPanel->FindText(event.GetFindString(), GetTextSearchOptions(event))) {
        wxBell();
    }
}

void MainFrame::HandleFindDialogReplace(wxFindDialogEvent& event) {
    if (event.GetDialog() == m_findDialog &&
        !m_textEditorPanel->ReplaceText(event.GetFindString(), event.GetReplaceString(), GetTextSearchOptions(event))) {
        wxBell();
    }
}

void MainFrame::HandleFindDialogReplaceAll(wxFindDialogEvent& event) {
    if (event.GetDialog() == m_findDialog &&
        m_textEditorPanel->ReplaceAllText(event.GetFindString(), event.GetReplaceString(),
                                          GetTextSearchOptions(event)) == 0) {
        wxBell();
    }
}

void MainFrame::HandleFindDialogClose(wxFindDialogEvent& event) {
    if (event.GetDialog() == m_findDialog) {
        m_findDialog->Destroy();
        m_findDialog = nullptr;
        m_textEditorPanel->FocusEditor();
    }
}

void MainFrame::HandleUpdateEditorMenuItem(wxUpdateUIEvent& event) {
    event.Enable(ContainsFocus(m_textEditorPanel) || m_findDialog != nullptr);
}

void MainFrame::HandleEditorTextChanged(wxCommandEvent& event) {
    if (m_textEditorPanel != nullptr) {
        UpdateDocumentTitle();
    }
    event.Skip();
}

void MainFrame::UpdateDocumentTitle() {
    const wxString name = m_currentFile.IsOk() ? m_currentFile.GetFullName() : _("Untitled");
    SetTitle((m_textEditorPanel->HasUnsavedChanges() ? wxString("*") : wxString{}) + name + " - " + kApplicationTitle);
}

void MainFrame::ResetDocument() {
    m_documentWatcher.reset();
    m_currentFile = wxFileName();
    m_loadedText.clear();
    m_textEditorPanel->LoadText(wxString{});
    statusBar->SetStatusText(wxString{});
    UpdateDocumentTitle();
    m_textEditorPanel->FocusEditor();
}

bool MainFrame::ConfirmSaveBeforeDiscard() {
    const bool missing = m_currentFile.IsOk() && !m_currentFile.FileExists();
    if (!m_textEditorPanel->HasUnsavedChanges() && !missing) {
        return true;
    }
    const wxString name = m_currentFile.IsOk() ? m_currentFile.GetFullName() : _("Untitled");
    const wxString message =
        missing
            ? wxString::Format(_("\"%s\" was removed on disk. Save your editor copy before continuing?"), name.c_str())
            : wxString::Format(_("Save changes to \"%s\" before continuing?"), name.c_str());
    wxMessageDialog dialog(this, message, kApplicationTitle, wxYES_NO | wxCANCEL | wxICON_QUESTION);
    dialog.SetYesNoCancelLabels(wxGetStockLabel(wxID_SAVE), _("Don't Save"), wxGetStockLabel(wxID_CANCEL));
    switch (dialog.ShowModal()) {
        case wxID_YES:
            return missing ? SaveTextFile(m_currentFile, true) : SaveDocument();
        case wxID_NO:
            return true;
        default:
            return false;
    }
}

bool MainFrame::ConfirmOverwriteExternalChanges(const wxFileName& filePath, bool recreationConfirmed) {
    if (!IsSameFilePath(m_currentFile, filePath)) {
        return true;
    }
    const bool missing = !filePath.FileExists();
    if (missing && recreationConfirmed) {
        return true;
    }
    if (!missing) {
        wxString onDisk;
        if (!ReadDocument(filePath, onDisk)) {
            wxMessageBox(_("Could not read the current file to check for external changes. The file was not saved."),
                         _("Save File"), wxOK | wxICON_ERROR, this);
            return false;
        }
        if (onDisk == m_loadedText) {
            return true;
        }
    }
    const wxString message =
        missing ? _("The file was removed on disk. Recreate it with your editor contents?")
                : _("The file changed on disk. Overwrite those external changes with your editor contents?");
    wxMessageDialog dialog(this, message + "\n\n" + filePath.GetFullPath(), _("File Changed on Disk"),
                           wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_WARNING);
    dialog.SetOKCancelLabels(missing ? _("Recreate") : _("Overwrite"), wxGetStockLabel(wxID_CANCEL));
    return dialog.ShowModal() == wxID_OK;
}

void MainFrame::HandleDirectoryChanged(const wxFileName& directory) {
    const wxFileName path = wxFileName::DirName(directory.GetFullPath());
    if (IsSameFilePath(path, m_watchedDirectory)) {
        return;
    }
    m_browserWatcher.reset();
    m_watchedDirectory = path;
    if (path.DirExists()) {
        m_browserWatcher =
            std::make_unique<FsWatcher>(path, std::bind_front(&MainFrame::HandleBrowserWatcherChange, this));
    }
}

void MainFrame::HandleBrowserWatcherChange() {
    m_fileBrowserPanel->ReloadCurrentDir();
}

void MainFrame::RefreshDocumentWatcher() {
    m_documentWatcher.reset();
    if (m_currentFile.IsOk()) {
        m_documentWatcher =
            std::make_unique<FsWatcher>(m_currentFile, std::bind_front(&MainFrame::HandleDocumentWatcherChange, this));
    }
}

void MainFrame::HandleDocumentWatcherChange() {
    if (!m_currentFile.FileExists()) {
        statusBar->SetStatusText(_("File was removed on disk; your editor copy was kept."));
        return;
    }
    wxString onDisk;
    if (!ReadDocument(m_currentFile, onDisk)) {
        statusBar->SetStatusText(_("Could not read the changed file; your editor copy was kept."));
        return;
    }
    if (onDisk == m_loadedText) {
        return;
    }
    if (m_textEditorPanel->HasUnsavedChanges()) {
        statusBar->SetStatusText(_("File changed on disk; your unsaved edits were kept."));
        return;
    }
    m_loadedText = onDisk;
    m_textEditorPanel->LoadText(onDisk, true);
    UpdateDocumentTitle();
    statusBar->SetStatusText(_("Reloaded ") + m_currentFile.GetFullPath());
}

void MainFrame::HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event) {
    if (!event.IsChecked()) {
        HideFileBrowser();
    } else if (!m_mainSplitter->IsSplit()) {
        m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_textEditorPanel, m_fileBrowserWidth);
        m_textEditorPanel->FocusEditor();
    }
}

void MainFrame::HandleFileBrowserHomeRequested() {
    wxFileName homeDirectory;
    homeDirectory.AssignHomeDir();
    m_fileBrowserPanel->ListDir(homeDirectory);
}

void MainFrame::HandleFileBrowserCloseRequested() {
    HideFileBrowser();
}

std::optional<wxString> MainFrame::HandleSelectBrowserFolderName(const std::optional<wxString>& previousName) {
    wxTextEntryDialog dialog(this, _("Name for the new folder:"), _("New Folder"),
                             previousName.value_or(_("New Folder")));
    if (dialog.ShowModal() != wxID_OK) {
        return std::nullopt;
    }
    return dialog.GetValue();
}

void MainFrame::HandleBrowserCreateFolderError(const FileBrowserTreePanel::CreateFolderError& error) {
    const wxString detail = wxString::FromUTF8(error.error.message().c_str());
    const wxString message =
        wxString::Format(_("Could not create folder \"%s\": %s"), error.name.c_str(), detail.c_str());
    wxMessageBox(message, _("New Folder"), wxOK | wxICON_ERROR, this);
}

bool MainFrame::HandleConfirmBrowserMoveToTrash(const std::vector<FileBrowserTreePanel::DeletePrompt>& paths) {
    const wxString trashName = FileBrowserTreePanel::GetTrashName();
    wxString message;
    if (paths.size() == 1) {
        const auto& path = paths.front();
        const wxString name = path.isDirectory ? GetLastDirectoryName(path.path) : path.path.GetFullName();
        message = wxString::Format(_("Move \"%s\" to %s?"), name.c_str(), trashName.c_str());
    } else {
        message = wxString::Format(_("Move %zu selected items to %s?"), paths.size(), trashName.c_str());
    }
    const wxString action = wxString::Format(_("Move to %s"), trashName.c_str());
    return ConfirmBrowserDelete(paths, std::move(message), action);
}

bool MainFrame::HandleConfirmBrowserPermanentlyDelete(const std::vector<FileBrowserTreePanel::DeletePrompt>& paths) {
    wxString message;
    if (paths.size() == 1) {
        const auto& path = paths.front();
        const wxString name = path.isDirectory ? GetLastDirectoryName(path.path) : path.path.GetFullName();
        message = path.isDirectory
                      ? wxString::Format(_("Permanently delete the folder \"%s\" and all its contents?"), name.c_str())
                      : wxString::Format(_("Permanently delete the file \"%s\"?"), name.c_str());
        message += _("\n\nThis action cannot be undone.");
    } else {
        message = wxString::Format(_("Permanently delete %zu selected items and all folder contents?"
                                     "\n\nThis action cannot be undone."),
                                   paths.size());
    }
    const wxString action = _("Delete Permanently");
    return ConfirmBrowserDelete(paths, std::move(message), action);
}

bool MainFrame::ConfirmBrowserDelete(const std::vector<FileBrowserTreePanel::DeletePrompt>& paths, wxString message,
                                     const wxString& action) {
    message +=
        _("\n\nThis operation runs synchronously and may take some time. The application will be "
          "unresponsive until it finishes.");
    const bool affectsCurrentDocument =
        std::ranges::any_of(paths, [this](const auto& path) { return IsCurrentDocumentAffectedByDelete(path); });
    if (affectsCurrentDocument && m_textEditorPanel->HasUnsavedChanges()) {
        message += _("\n\nUnsaved changes in the open document will be lost.");
    }
    wxMessageDialog dialog(this, message, action, wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_WARNING);
    dialog.SetOKCancelLabels(action, wxGetStockLabel(wxID_CANCEL));
    return dialog.ShowModal() == wxID_OK;
}

bool MainFrame::IsCurrentDocumentAffectedByDelete(const FileBrowserTreePanel::DeletePrompt& deleted) const {
    return IsSameFilePath(m_currentFile, deleted.path) ||
           (deleted.isDirectory && IsFileWithinDirectory(m_currentFile, deleted.path));
}

void MainFrame::HandleBrowserPathDeleted(const FileBrowserTreePanel::DeletePrompt& deleted) {
    if (IsCurrentDocumentAffectedByDelete(deleted)) {
        ResetDocument();
    }
}

void MainFrame::HandleBrowserDeleteError(const std::vector<FileBrowserTreePanel::DeleteError>& errors,
                                         FileBrowserTreePanel::DeleteMode mode) {
    const bool trash = mode == FileBrowserTreePanel::DeleteMode::Trash;
    const wxString trashName = FileBrowserTreePanel::GetTrashName();
    wxString message;
    if (errors.size() == 1) {
        const auto& error = errors.front();
        const wxString name = error.isDirectory ? GetLastDirectoryName(error.path) : error.path.GetFullName();
        const wxString detail = wxString::FromUTF8(error.error.message().c_str());
        message = trash ? wxString::Format(_("Could not move \"%s\" to %s: %s"), name.c_str(), trashName.c_str(),
                                           detail.c_str())
                        : wxString::Format(_("Could not completely delete \"%s\": %s"), name.c_str(), detail.c_str());
    } else {
        message = trash ? wxString::Format(_("Could not move %zu items to %s:"), errors.size(), trashName.c_str())
                        : wxString::Format(_("Could not completely delete %zu items:"), errors.size());
        for (const auto& error : errors) {
            const wxString name = error.isDirectory ? GetLastDirectoryName(error.path) : error.path.GetFullName();
            const wxString detail = wxString::FromUTF8(error.error.message().c_str());
            message += wxString::Format("\n%s: %s", name.c_str(), detail.c_str());
        }
    }
    const wxString title = trash ? wxString::Format(_("Move to %s"), trashName.c_str()) : _("Delete Permanently");
    wxMessageBox(message, title, wxOK | wxICON_ERROR, this);
}

void MainFrame::HandleFileTreeBrowserMoveError(const std::vector<FileBrowserTreePanel::MoveError>& errors) {
    wxString message = _("Could not move the following items:") + "\n\n";
    for (const auto& error : errors) {
        const wxString detail = wxString::FromUTF8(error.error.message().c_str());
        message += wxString::Format("%s: %s\n", error.path.GetFullPath().c_str(), detail.c_str());
    }
    wxMessageBox(message, _("Move Files"), wxOK | wxICON_ERROR, this);
}

void MainFrame::HandleFileTreeBrowserCopyError(const std::vector<FileBrowserTreePanel::CopyError>& errors) {
    wxString message = _("Could not copy the following items:") + "\n\n";
    for (const auto& error : errors) {
        const wxString detail = wxString::FromUTF8(error.error.message().c_str());
        message += wxString::Format("%s: %s\n", error.path.GetFullPath().c_str(), detail.c_str());
    }
    wxMessageBox(message, _("Copy Files"), wxOK | wxICON_ERROR, this);
}

bool MainFrame::HandleFileTreeBrowserConfirmOverwrite(const FileBrowserTreePanel::OverwritePrompt& prompt) {
    if (IsSameFilePath(m_currentFile, prompt.destination) && m_textEditorPanel->HasUnsavedChanges()) {
        wxMessageBox(_("The destination is open with unsaved changes. Save or close it before replacing it."),
                     _("Replace File"), wxOK | wxICON_WARNING, this);
        return false;
    }
    const wxString message =
        wxString::Format(_("A file already exists at:\n%s\n\nReplace it with:\n%s?"),
                         prompt.destination.GetFullPath().c_str(), prompt.source.GetFullPath().c_str());
    wxMessageDialog dialog(this, message, _("Replace File"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    dialog.SetYesNoLabels(_("Replace"), _("Skip"));
    return dialog.ShowModal() == wxID_YES;
}

void MainFrame::HandleFileTreeBrowserDestinationUpdated(const wxFileName& destination) {
    if (IsSameFilePath(m_currentFile, destination) && !m_textEditorPanel->HasUnsavedChanges()) {
        wxString text;
        if (ReadDocument(destination, text)) {
            m_loadedText = text;
            m_textEditorPanel->LoadText(text, true);
            UpdateDocumentTitle();
        } else {
            wxMessageBox(wxString::Format(_("Could not read file: %s"), destination.GetFullPath().c_str()),
                         _("Reload File"), wxOK | wxICON_ERROR, this);
        }
    }
}

bool MainFrame::IsCurrentDocumentAffectedByMove(const FileBrowserTreePanel::MovePrompt& moved) const {
    return IsSameFilePath(m_currentFile, moved.path) ||
           (moved.isDirectory && IsFileWithinDirectory(m_currentFile, moved.path));
}

bool MainFrame::HandleFileTreeBrowserBeforeMove(const std::vector<FileBrowserTreePanel::MovePrompt>& paths) {
    const bool affectsCurrentDocument =
        std::ranges::any_of(paths, [this](const auto& path) { return IsCurrentDocumentAffectedByMove(path); });
    if (affectsCurrentDocument && m_textEditorPanel->HasUnsavedChanges()) {
        wxMessageBox(_("The open document has unsaved changes. Save or close it before moving it or its folder."),
                     _("Move Files"), wxOK | wxICON_WARNING, this);
        return false;
    }
    return true;
}

void MainFrame::HandleFileTreeBrowserPathMoved(const FileBrowserTreePanel::MovePrompt& moved,
                                               const wxFileName& destination) {
    if (IsCurrentDocumentAffectedByMove(moved)) {
        wxFileName relocated = destination;
        if (moved.isDirectory) {
            // Preserve the document's path within the moved folder.
            const wxArrayString& directories = m_currentFile.GetDirs();
            for (std::size_t index = moved.path.GetDirCount(); index < directories.size(); ++index) {
                relocated.AppendDir(directories[index]);
            }
            relocated.SetFullName(m_currentFile.GetFullName());
        }
        // The buffer is already the moved file's contents. Relocate its path
        // without clearing it or risking a second read after a successful move.
        m_currentFile = relocated;
        RefreshDocumentWatcher();
        UpdateDocumentTitle();
        statusBar->SetStatusText(relocated.GetFullPath());
        m_fileBrowserPanel->ShowFile(relocated);
    } else {
        // A move can replace the open destination without moving its path.
        HandleFileTreeBrowserDestinationUpdated(destination);
    }
}

#ifdef __WXOSX__
void MainFrame::HandleMenuHighlight(wxMenuEvent& event) {
    if (event.GetMenuId() == wxID_NONE) {
        // Cocoa unhighlights keyboard shortcuts later, without a menu-close
        // event. Dismiss help instead of showing empty help over a newly opened
        // file's status. wxFrame preserves text changed since help was shown.
        MainFrameWx::DoGiveHelp(wxString{}, false);
        return;
    }
    event.Skip();
}
#endif

void MainFrame::HideFileBrowser() {
    m_viewMenu->Check(wxID_TOGGLE_FILE_BROWSER_MENU_ITEM, false);
    if (m_mainSplitter->IsSplit()) {
        // Restore the user's last sash position rather than resetting the
        // browser to its initial width every time it is shown.
        const int sashPosition = m_mainSplitter->GetSashPosition();
        if (sashPosition > 0) {
            m_fileBrowserWidth = sashPosition;
        }
        m_mainSplitter->Unsplit(m_fileBrowserPanel);
    }
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleWordWrapMenuItemClick(wxCommandEvent& event) {
    m_textEditorPanel->SetWordWrap(event.IsChecked());
    m_textEditorPanel->FocusEditor();
}

void MainFrame::HandleFontMenuItemClick(wxCommandEvent& event) {
    wxFontData fontData;
    fontData.EnableEffects(false);
    fontData.SetInitialFont(m_textEditorPanel->GetEditorFont());

    wxFontDialog fontDialog(this, fontData);
    if (fontDialog.ShowModal() == wxID_OK) {
        const wxFont chosenFont = fontDialog.GetFontData().GetChosenFont();
        if (chosenFont.IsOk()) {
            m_textEditorPanel->SetEditorFont(chosenFont);
            rottools::SaveEditorFont(chosenFont);
        }
    }

    m_textEditorPanel->FocusEditor();
}

bool MainFrame::SaveTextFile(const wxFileName& filePath, bool recreationConfirmed) {
    wxFileName absolutePath(filePath);
    absolutePath.MakeAbsolute();

    if (absolutePath.FileExists() && !absolutePath.IsFileWritable()) {
        wxMessageBox(wxString("No permission to write file: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return false;
    }

    if (!ConfirmOverwriteExternalChanges(absolutePath, recreationConfirmed)) {
        return false;
    }

    // wxTextCtrl normalizes line endings. Retain the opened document's format
    // when writing it back, and use the system convention for new documents.
    wxString text = m_textEditorPanel->GetText();
    text.Replace("\r\n", "\n");
    text.Replace("\r", "\n");
    bool useCrLf = m_loadedText.Find("\r\n") != wxNOT_FOUND;
#ifdef __WXMSW__
    if (!m_currentFile.IsOk()) {
        useCrLf = true;
    }
#endif
    if (useCrLf) {
        text.Replace("\n", "\r\n");
    } else if (m_loadedText.Find('\r') != wxNOT_FOUND && m_loadedText.Find('\n') == wxNOT_FOUND) {
        text.Replace("\n", "\r");
    }
    wxTempFile output;
    if (!output.Open(absolutePath.GetFullPath()) || !output.Write(text) || !output.Flush() || !output.Commit()) {
        wxMessageBox(wxString("Could not save file: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return false;
    }

    const bool pathChanged = !m_currentFile.IsOk() || !m_currentFile.SameAs(absolutePath);
    m_currentFile = absolutePath;
    m_loadedText = text;
    m_textEditorPanel->MarkSaved();
    RefreshDocumentWatcher();
    UpdateDocumentTitle();
    statusBar->SetStatusText(wxString("Saved ") + absolutePath.GetFullPath());

    if (pathChanged) {
        m_fileBrowserPanel->ShowFile(absolutePath);
    }

    m_textEditorPanel->FocusEditor();
    return true;
}

void MainFrame::HandleOpenTextFile(const wxFileName& filePath) {
    wxFileName absolutePath(filePath);
    absolutePath.MakeAbsolute();

    if (IsSameFilePath(m_currentFile, absolutePath)) {
        m_textEditorPanel->FocusEditor();
        return;
    }

    if (!absolutePath.FileExists()) {
        wxMessageBox(wxString("File does not exist: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return;
    }
    if (!absolutePath.IsFileReadable()) {
        wxMessageBox(wxString("No permission to read file: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return;
    }

    wxString text;
    if (!ReadDocument(absolutePath, text)) {
        wxMessageBox(_("Could not read the file as UTF-8 text: ") + absolutePath.GetFullPath(), _("Open File"),
                     wxOK | wxICON_ERROR, this);
        return;
    }

    if (!ConfirmSaveBeforeDiscard()) {
        return;
    }
    // Saving the old document may run a modal dialog. Read again afterwards
    // so changes made while it was open are not lost on the next save.
    if (!ReadDocument(absolutePath, text)) {
        wxMessageBox(_("Could not read the file as UTF-8 text: ") + absolutePath.GetFullPath(), _("Open File"),
                     wxOK | wxICON_ERROR, this);
        return;
    }

    m_currentFile = absolutePath;
    m_loadedText = text;
    m_textEditorPanel->LoadText(text);
    RefreshDocumentWatcher();
    UpdateDocumentTitle();
    m_fileBrowserPanel->ShowFile(absolutePath);
    statusBar->SetStatusText(absolutePath.GetFullPath());
    m_textEditorPanel->FocusEditor();
}
