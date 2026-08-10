#pragma once

#include <wx/filename.h>
#include <wx/string.h>

#include <functional>

#include "ScintillaTextEditorPanelWx.h"

// Forward declarations keep the Scintilla headers out of consumers.
class wxFont;
class wxStyledTextCtrl;
class wxStyledTextEvent;
class wxSysColourChangedEvent;

class ScintillaTextEditorPanel : public ScintillaTextEditorPanelWx {
   public:
    enum class Syntax {
        None,
        Markdown,
    };

    struct Options {
        Syntax syntax = Syntax::None;
        bool lineNumbers = false;
        bool wordWrap = false;
        wxString fileWildcard = "All files (*.*)|*.*";
        wxString openDialogTitle = "Open File";
        wxString saveDialogTitle = "Save File";
    };

    enum class LoadBehavior {
        ResetToTop,
        KeepPosition,
    };

    enum class ChangeReason {
        Opened,
        Reloaded,
        Saved,
    };

    struct DocumentChange {
        ChangeReason reason = ChangeReason::Opened;
        bool diskEntryChanged = false;
        wxString text;
        wxFileName filePath;
    };

    enum class Status {
        Loading,
        Reloading,
        Saved,
        FileRemoved,
        FileChangedWithUnsavedEdits,
    };

    enum class ErrorCode {
        /** The path passed to OpenFile does not identify an existing file. */
        FileDoesNotExist,
        /** The file passed to OpenFile exists but is not readable. */
        FileNotReadable,
        /** Reading the file passed to OpenFile failed. */
        FileReadFailed,
        /** The save target exists but is not writable. */
        FileNotWritable,
        /** Writing the editor contents to the save target failed. */
        FileWriteFailed,
        /** Reading the current file to check for external changes failed. */
        ExternalChangeCheckFailed,
    };

    enum class SavePromptReason {
        /** The document has editor changes that have not been saved. */
        UnsavedChanges,
        /** The current document file was removed outside the editor. */
        FileRemoved,
    };

    enum class SavePromptDecision {
        /** Save or recreate the document before continuing. */
        Save,
        /** Continue without saving or recreating the document. */
        Discard,
        /** Cancel the operation that would discard the current document. */
        Cancel,
    };

    struct SavePrompt {
        SavePromptReason reason = SavePromptReason::UnsavedChanges;
        /** Absolute current file path; invalid when the document is untitled. */
        wxFileName filePath;
    };

    enum class OverwritePromptReason {
        /** The current file was removed and saving would recreate it. */
        FileRemoved,
        /** The current file contents changed since the editor loaded them. */
        FileChanged,
    };

    enum class OverwritePromptDecision {
        /** Continue by recreating the removed file or overwriting its changes. */
        Proceed,
        /** Cancel the save without changing the file. */
        Cancel,
    };

    struct OverwritePrompt {
        OverwritePromptReason reason = OverwritePromptReason::FileChanged;
        /** Absolute path of the file that would be recreated or overwritten. */
        wxFileName filePath;
    };

    using OnDocumentChangedCallback = std::function<void(const DocumentChange&)>;
    using OnStatusChangedCallback = std::function<void(Status, const wxFileName&)>;
    using OnErrorCallback = std::function<void(ErrorCode, const wxFileName&)>;
    using ConfirmSaveBeforeDiscardCallback = std::function<SavePromptDecision(const SavePrompt&)>;
    using ConfirmOverwriteExternalChangesCallback = std::function<OverwritePromptDecision(const OverwritePrompt&)>;
    using OnDocumentWatchRequestedCallback = std::function<void(const wxFileName&)>;

    struct Callbacks {
        /**
         * Called after a document is opened, reloaded from disk, or saved.
         * DocumentChange identifies the operation and carries the current text
         * and absolute file path. diskEntryChanged is true when saving created,
         * recreated, or switched to a different file.
         */
        OnDocumentChangedCallback documentChanged;

        /**
         * Called when the editor status changes. The callback receives the
         * semantic status and absolute document path; the consumer decides
         * how to present it, including its user-facing label.
         */
        OnStatusChangedCallback statusChanged;

        /**
         * Called when continuing may discard unsaved editor contents or leave
         * a removed file unrecreated. The callback presents that choice and
         * returns the user's decision; absence of a callback means Cancel.
         */
        ConfirmSaveBeforeDiscardCallback confirmSaveBeforeDiscard;

        /**
         * Called before saving would recreate a removed current file or
         * overwrite changes made outside the editor. The callback presents
         * that choice and returns the user's decision; absence means Cancel.
         */
        ConfirmOverwriteExternalChangesCallback confirmOverwriteExternalChanges;

        /**
         * Called when a file operation fails. The callback receives a stable
         * error code and the absolute path involved; the consumer decides how
         * to report or otherwise handle the error.
         */
        OnErrorCallback onError;

        /**
         * Called after opening a file, changing the save path, or recreating a
         * missing file. The host should replace its external watcher so it
         * watches the supplied absolute document path.
         */
        OnDocumentWatchRequestedCallback documentWatchRequested;
    };

   private:
    wxStyledTextCtrl* m_textEditor = nullptr;
    Options m_options;
    Callbacks m_callbacks;
    wxFileName m_currentFile;
    wxString m_loadedText;

    [[nodiscard]] wxWindow* GetDialogParent() const;
    [[nodiscard]] bool SaveFile(const wxFileName& filePath, bool missingFileRecreationConfirmed = false);
    [[nodiscard]] bool ConfirmOverwriteExternalChanges(const wxFileName& filePath,
                                                       bool missingFileRecreationConfirmed = false);
    void RequestDocumentWatch() const;
    void NotifyDocumentChanged(ChangeReason reason, const wxString& text, bool diskEntryChanged = false) const;
    void NotifyStatusChanged(Status status) const;
    void NotifyError(ErrorCode errorCode, const wxFileName& filePath) const;
    void ApplyEditorStyles(const wxFont& font);
    void ApplyMarkdownStyles();
    void ApplySelectionColours();
    void UpdateEolModeForText(const wxString& text);
    void UpdateLineNumberMarginWidth();
    void HandleEditorMetricsChanged(wxStyledTextEvent& event);
    void HandleSystemColourChanged(wxSysColourChangedEvent& event);

   public:
    explicit ScintillaTextEditorPanel(wxWindow* parent);
    explicit ScintillaTextEditorPanel(wxWindow* parent, Options options, Callbacks callbacks = {});

    [[nodiscard]] bool ShowOpenDialog();
    [[nodiscard]] bool OpenFile(const wxFileName& filePath);
    [[nodiscard]] bool Save();
    [[nodiscard]] bool SaveAs();
    [[nodiscard]] bool ConfirmSaveBeforeDiscard();

    /**
     * Replace the current document and clear its undo history. KeepPosition
     * also retains the primary selection and visible viewport.
     */
    void LoadText(const wxString& text, LoadBehavior loadBehavior = LoadBehavior::ResetToTop);

    [[nodiscard]] wxString GetText() const;
    [[nodiscard]] bool HasUnsavedChanges() const;
    void MarkSaved();
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
    void SetEditorFont(const wxFont& font);
    [[nodiscard]] wxFont GetEditorFont() const;
    [[nodiscard]] bool ContainsFocus() const;
    void FocusEditor();
    void ShowFontDialog();

   protected:
    /** Process a change reported by the optional external document watcher. */
    void HandleDocumentWatcherChange();
};
