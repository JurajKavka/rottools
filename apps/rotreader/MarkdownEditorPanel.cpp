#include "MarkdownEditorPanel.h"

#include <wx/filedlg.h>
#include <wx/fontdlg.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stockitem.h>
#include <wx/window.h>

#include <functional>
#include <utility>

#include "FsWatcher.h"
#include "HelperFunctions.h"
#include "ScintillaTextEditorPanel.h"

namespace {
constexpr auto kMarkdownFileWildcard = "Markdown files (*.md;*.markdown)|*.md;*.markdown";
}

MarkdownEditorPanel::MarkdownEditorPanel(wxWindow* parent, OnDocumentChangedCallback onDocumentChanged,
                                         OnStatusChangedCallback onStatusChanged)
    : wxPanel(parent),
      m_onDocumentChanged(std::move(onDocumentChanged)),
      m_onStatusChanged(std::move(onStatusChanged)) {
    m_editor =
        new ScintillaTextEditorPanel(this, {.syntax = ScintillaTextEditorPanel::Syntax::Markdown, .lineNumbers = true});
    m_editor->SetEditorFont(LoadEditorFont(m_editor->GetEditorFont()));
    m_editor->SetWordWrap(true);

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_editor, 1, wxEXPAND);
    SetSizer(sizer);
}

MarkdownEditorPanel::~MarkdownEditorPanel() {
    // FsWatcher callbacks refer to this panel. Destroy it before wx tears down
    // the child editor and its native controls.
    m_documentWatcher.reset();
}

bool MarkdownEditorPanel::ShowOpenDialog() {
    wxFileDialog dialog(GetDialogParent(), "Open Markdown File", {}, {}, kMarkdownFileWildcard,
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK) {
        return false;
    }

    return OpenFileInternal(wxFileName(dialog.GetPath()));
}

void MarkdownEditorPanel::OpenFile(const wxFileName& filePath) {
    static_cast<void>(OpenFileInternal(filePath));
}

bool MarkdownEditorPanel::OpenFileInternal(const wxFileName& filePath) {
    wxFileName absolutePath(filePath);
    absolutePath.MakeAbsolute();

    if (!absolutePath.FileExists()) {
        wxMessageBox(wxString("File does not exist: ") + absolutePath.GetFullPath(), "Error", wxOK | wxICON_ERROR,
                     GetDialogParent());
        return false;
    }
    if (!absolutePath.IsFileReadable()) {
        wxMessageBox(wxString("No permission to read file: ") + absolutePath.GetFullPath(), "Error",
                     wxOK | wxICON_ERROR, GetDialogParent());
        return false;
    }
    if (!ConfirmSaveBeforeDiscard()) {
        return false;
    }

    wxString markdown;
    if (!ReadFileUtf8(absolutePath, markdown)) {
        wxMessageBox(wxString("Could not read file: ") + absolutePath.GetFullPath(), "Error", wxOK | wxICON_ERROR,
                     GetDialogParent());
        return false;
    }

    m_currentFile = absolutePath;
    m_loadedText = markdown;
    m_editor->LoadText(markdown);
    RefreshDocumentWatcher();
    NotifyStatusChanged("Loading ...", true);
    NotifyDocumentChanged(ChangeReason::Opened, markdown);
    return true;
}

bool MarkdownEditorPanel::Save() {
    return m_currentFile.IsOk() ? SaveFile(m_currentFile) : SaveAs();
}

bool MarkdownEditorPanel::SaveAs() {
    wxString defaultDirectory;
    wxString defaultFileName;
    if (m_currentFile.IsOk()) {
        defaultDirectory = m_currentFile.GetPath();
        defaultFileName = m_currentFile.GetFullName();
    }

    wxFileDialog dialog(GetDialogParent(), "Save Markdown File", defaultDirectory, defaultFileName,
                        kMarkdownFileWildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK) {
        return false;
    }

    return SaveFile(wxFileName(dialog.GetPath()));
}

bool MarkdownEditorPanel::ConfirmSaveBeforeDiscard() {
    const bool fileMissing = m_currentFile.IsOk() && !m_currentFile.FileExists();
    if (!m_editor->HasUnsavedChanges() && !fileMissing) {
        return true;
    }

    const wxString documentName = m_currentFile.IsOk() ? m_currentFile.GetFullName() : wxString("Untitled");
    const wxString message = fileMissing ? wxString("The file \"") + documentName +
                                               "\" was removed outside ROT Reader.\n\nRecreate it before continuing?"
                                         : wxString("Save changes to \"") + documentName + "\" before continuing?";
    wxMessageDialog dialog(GetDialogParent(), message, fileMissing ? "File Removed" : "Unsaved Changes",
                           wxYES_NO | wxCANCEL | wxCANCEL_DEFAULT | wxICON_WARNING);
    dialog.SetYesNoCancelLabels(fileMissing ? wxString("Recreate") : wxGetStockLabel(wxID_SAVE), "Don't Save",
                                wxGetStockLabel(wxID_CANCEL));

    const int result = dialog.ShowModal();
    if (result == wxID_NO) {
        return true;
    }
    if (result != wxID_YES) {
        return false;
    }

    return m_currentFile.IsOk() ? SaveFile(m_currentFile, fileMissing) : SaveAs();
}

bool MarkdownEditorPanel::ContainsFocus() const {
    wxWindow* focusedWindow = wxWindow::FindFocus();
    return focusedWindow != nullptr && (focusedWindow == this || IsDescendant(focusedWindow));
}

void MarkdownEditorPanel::FocusEditor() {
    m_editor->FocusEditor();
}

void MarkdownEditorPanel::Undo() {
    m_editor->Undo();
    FocusEditor();
}

void MarkdownEditorPanel::Redo() {
    m_editor->Redo();
    FocusEditor();
}

void MarkdownEditorPanel::Copy() {
    m_editor->Copy();
    FocusEditor();
}

void MarkdownEditorPanel::Cut() {
    m_editor->Cut();
    FocusEditor();
}

void MarkdownEditorPanel::Paste() {
    m_editor->Paste();
    FocusEditor();
}

bool MarkdownEditorPanel::CanUndo() const {
    return m_editor->CanUndo();
}

bool MarkdownEditorPanel::CanRedo() const {
    return m_editor->CanRedo();
}

bool MarkdownEditorPanel::CanCopy() const {
    return m_editor->CanCopy();
}

bool MarkdownEditorPanel::CanCut() const {
    return m_editor->CanCut();
}

bool MarkdownEditorPanel::CanPaste() const {
    return m_editor->CanPaste();
}

void MarkdownEditorPanel::SetWordWrap(bool enabled) {
    const bool editorHadFocus = ContainsFocus();
    m_editor->SetWordWrap(enabled);
    if (editorHadFocus) {
        FocusEditor();
    }
}

bool MarkdownEditorPanel::IsWordWrapEnabled() const {
    return m_editor->IsWordWrapEnabled();
}

void MarkdownEditorPanel::ShowFontDialog() {
    const bool editorHadFocus = ContainsFocus();
    wxFontData fontData;
    fontData.EnableEffects(false);
    fontData.SetInitialFont(m_editor->GetEditorFont());

    wxFontDialog dialog(GetDialogParent(), fontData);
    if (dialog.ShowModal() == wxID_OK) {
        const wxFont chosenFont = dialog.GetFontData().GetChosenFont();
        if (chosenFont.IsOk()) {
            m_editor->SetEditorFont(chosenFont);
            SaveEditorFont(chosenFont);
        }
    }

    if (editorHadFocus) {
        FocusEditor();
    }
}

wxWindow* MarkdownEditorPanel::GetDialogParent() const {
    wxWindow* topLevelParent = wxGetTopLevelParent(const_cast<MarkdownEditorPanel*>(this));
    return topLevelParent != nullptr ? topLevelParent : const_cast<MarkdownEditorPanel*>(this);
}

bool MarkdownEditorPanel::SaveFile(const wxFileName& filePath, bool missingFileRecreationConfirmed) {
    wxFileName absolutePath(filePath);
    absolutePath.MakeAbsolute();
    const bool targetWasMissing = !absolutePath.FileExists();

    if (!targetWasMissing && !absolutePath.IsFileWritable()) {
        wxMessageBox(wxString("No permission to write file: ") + absolutePath.GetFullPath(), "Error",
                     wxOK | wxICON_ERROR, GetDialogParent());
        return false;
    }
    if (!ConfirmOverwriteExternalChanges(absolutePath, missingFileRecreationConfirmed)) {
        return false;
    }

    const wxString markdown = m_editor->GetText();
    if (!WriteFileUtf8(absolutePath, markdown)) {
        wxMessageBox(wxString("Could not save file: ") + absolutePath.GetFullPath(), "Error", wxOK | wxICON_ERROR,
                     GetDialogParent());
        return false;
    }

    const bool pathChanged = !m_currentFile.IsOk() || !m_currentFile.SameAs(absolutePath);
    m_currentFile = absolutePath;
    m_loadedText = markdown;
    m_editor->MarkSaved();
    if (pathChanged || targetWasMissing) {
        RefreshDocumentWatcher();
    }

    NotifyStatusChanged(wxString("Saved ") + absolutePath.GetFullPath());
    NotifyDocumentChanged(ChangeReason::Saved, markdown, pathChanged || targetWasMissing);
    return true;
}

bool MarkdownEditorPanel::ConfirmOverwriteExternalChanges(const wxFileName& filePath,
                                                          bool missingFileRecreationConfirmed) {
    if (!m_currentFile.IsOk() || !m_currentFile.SameAs(filePath)) {
        return true;
    }

    wxString message;
    wxString actionLabel;
    if (!filePath.FileExists()) {
        if (missingFileRecreationConfirmed) {
            return true;
        }
        message = wxString("The file was removed outside ROT Reader:\n") + filePath.GetFullPath() +
                  "\n\nRecreate it with your editor contents?";
        actionLabel = "Recreate";
    } else {
        wxString onDisk;
        if (!ReadFileUtf8(filePath, onDisk)) {
            wxMessageBox(wxString("Could not check the current file before saving: ") + filePath.GetFullPath(), "Error",
                         wxOK | wxICON_ERROR, GetDialogParent());
            return false;
        }
        if (onDisk == m_loadedText) {
            return true;
        }

        message = wxString("The file changed outside ROT Reader:\n") + filePath.GetFullPath() +
                  "\n\nOverwrite those external changes with your editor contents?";
        actionLabel = "Overwrite";
    }

    wxMessageDialog dialog(GetDialogParent(), message, "File Changed on Disk",
                           wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_WARNING);
    dialog.SetOKCancelLabels(actionLabel, wxGetStockLabel(wxID_CANCEL));
    return dialog.ShowModal() == wxID_OK;
}

void MarkdownEditorPanel::RefreshDocumentWatcher() {
    m_documentWatcher.reset();
    if (m_currentFile.IsOk()) {
        m_documentWatcher = std::make_unique<FsWatcher>(
            m_currentFile, std::bind_front(&MarkdownEditorPanel::HandleDocumentWatcherChange, this));
    }
}

void MarkdownEditorPanel::HandleDocumentWatcherChange() {
    if (!m_currentFile.IsOk()) {
        return;
    }
    if (!m_currentFile.FileExists()) {
        NotifyStatusChanged(wxString("File was removed on disk - the editor copy was kept: ") +
                            m_currentFile.GetFullPath());
        return;
    }

    wxString onDisk;
    if (!ReadFileUtf8(m_currentFile, onDisk)) {
        printError("[Watcher] Could not read changed file: {}", m_currentFile.GetFullPath());
        return;
    }

    // Ignore an echo of our own write, or a repeated event for text already
    // accepted as the new disk baseline.
    if (onDisk == m_loadedText) {
        return;
    }

    if (m_editor->HasUnsavedChanges()) {
        NotifyStatusChanged(wxString("File changed on disk - your unsaved edits were kept: ") +
                            m_currentFile.GetFullPath());
        return;
    }

    printLog("[Watcher] Reloading changed file: {}", m_currentFile.GetFullPath());
    m_loadedText = onDisk;
    m_editor->LoadText(onDisk, ScintillaTextEditorPanel::LoadBehavior::KeepPosition);
    NotifyStatusChanged("Reloading ...", true);
    NotifyDocumentChanged(ChangeReason::Reloaded, onDisk);
}

void MarkdownEditorPanel::NotifyDocumentChanged(ChangeReason reason, const wxString& markdown,
                                                bool diskEntryChanged) const {
    if (m_onDocumentChanged) {
        m_onDocumentChanged(
            {.reason = reason, .diskEntryChanged = diskEntryChanged, .markdown = markdown, .filePath = m_currentFile});
    }
}

void MarkdownEditorPanel::NotifyStatusChanged(const wxString& status, bool replaceOnPreviewReady) const {
    if (m_onStatusChanged) {
        m_onStatusChanged({.text = status, .replaceOnPreviewReady = replaceOnPreviewReady});
    }
}
