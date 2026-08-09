#pragma once

#include <wx/filename.h>
#include <wx/panel.h>
#include <wx/string.h>

#include <functional>
#include <memory>

class FsWatcher;
class ScintillaTextEditorPanel;
class wxWindow;

/**
 * Owns ROT Reader's editable Markdown document, including its disk lifecycle.
 *
 * MainFrame remains responsible for presenting the document elsewhere (for
 * example, in the rendered preview and file browser) through the callbacks.
 */
class MarkdownEditorPanel final : public wxPanel {
   public:
    enum class ChangeReason {
        Opened,
        Reloaded,
        Saved,
    };

    struct DocumentChange {
        ChangeReason reason = ChangeReason::Opened;
        bool diskEntryChanged = false;
        wxString markdown;
        wxFileName filePath;
    };

    struct StatusChange {
        wxString text;
        bool replaceOnPreviewReady = false;
    };

    using OnDocumentChangedCallback = std::function<void(const DocumentChange&)>;
    using OnStatusChangedCallback = std::function<void(const StatusChange&)>;

    MarkdownEditorPanel(wxWindow* parent, OnDocumentChangedCallback onDocumentChanged,
                        OnStatusChangedCallback onStatusChanged);
    ~MarkdownEditorPanel() override;

    [[nodiscard]] bool ShowOpenDialog();
    void OpenFile(const wxFileName& filePath);
    [[nodiscard]] bool Save();
    [[nodiscard]] bool SaveAs();
    [[nodiscard]] bool ConfirmSaveBeforeDiscard();

    [[nodiscard]] bool ContainsFocus() const;
    void FocusEditor();

    void Undo();
    void Redo();
    void Copy();
    void Cut();
    void Paste();
    [[nodiscard]] bool CanUndo() const;
    [[nodiscard]] bool CanRedo() const;
    [[nodiscard]] bool CanCopy() const;
    [[nodiscard]] bool CanCut() const;
    [[nodiscard]] bool CanPaste() const;

    void SetWordWrap(bool enabled);
    [[nodiscard]] bool IsWordWrapEnabled() const;
    void ShowFontDialog();

   private:
    [[nodiscard]] wxWindow* GetDialogParent() const;
    [[nodiscard]] bool OpenFileInternal(const wxFileName& filePath);
    [[nodiscard]] bool SaveFile(const wxFileName& filePath, bool missingFileRecreationConfirmed = false);
    [[nodiscard]] bool ConfirmOverwriteExternalChanges(const wxFileName& filePath,
                                                       bool missingFileRecreationConfirmed = false);
    void RefreshDocumentWatcher();
    void HandleDocumentWatcherChange();
    void NotifyDocumentChanged(ChangeReason reason, const wxString& markdown, bool diskEntryChanged = false) const;
    void NotifyStatusChanged(const wxString& status, bool replaceOnPreviewReady = false) const;

    ScintillaTextEditorPanel* m_editor = nullptr;
    OnDocumentChangedCallback m_onDocumentChanged;
    OnStatusChangedCallback m_onStatusChanged;
    std::unique_ptr<FsWatcher> m_documentWatcher;
    wxFileName m_currentFile;
    wxString m_loadedText;
};
