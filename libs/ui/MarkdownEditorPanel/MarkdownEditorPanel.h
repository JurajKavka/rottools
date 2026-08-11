#pragma once

#include <wx/string.h>

#include <functional>
#include <memory>

#include "ScintillaTextEditorPanel.h"

class FsWatcher;

/**
 * Markdown-configured Scintilla editor with UI messages and file watching.
 *
 * ScintillaTextEditorPanel owns the document lifecycle and reports semantic
 * events. This subclass translates statuses, errors, and external-change
 * prompts into user-facing text, and supplies the FsWatcher requested through
 * the base panel's callbacks.
 */
class MarkdownEditorPanel final : public ScintillaTextEditorPanel {
   public:
    struct StatusMessage {
        wxString label;
        bool replaceOnPreviewReady = false;
    };

    /** Localized description of a Scintilla document error. */
    struct ErrorMessage {
        wxString text;
    };

    /** Localized presentation for an external-change confirmation dialog. */
    struct OverwritePromptMessage {
        wxString title;
        wxString text;
        wxString actionLabel;
    };

    using OnStatusMessageCallback = std::function<void(const StatusMessage&)>;
    using OnErrorMessageCallback = std::function<void(const ErrorMessage&)>;
    using ConfirmOverwritePromptCallback = std::function<OverwritePromptDecision(const OverwritePromptMessage&)>;

    struct Callbacks {
        /** Called after the current Markdown document is opened, reloaded, or saved. */
        OnDocumentChangedCallback documentChanged;

        /** Called with translated text whenever the editor status changes. */
        OnStatusMessageCallback statusMessageChanged;

        /**
         * Called before the current document may be replaced or the window may
         * close. See ScintillaTextEditorPanel::ConfirmSaveBeforeDiscardCallback
         * for the decision semantics; absence is treated as Cancel.
         */
        ConfirmSaveBeforeDiscardCallback confirmSaveBeforeDiscard;

        /**
         * Called before saving would recreate a removed file or overwrite
         * external changes. The prompt contains translated presentation text;
         * absence is treated as Cancel.
         */
        ConfirmOverwritePromptCallback confirmOverwriteExternalChanges;

        /** Called with translated text when a document operation fails. */
        OnErrorMessageCallback error;

        /** Presents the host's native Open dialog; an empty result means Cancel. */
        SelectOpenFileCallback selectOpenFile;

        /** Presents the host's native Save As dialog; an empty result means Cancel. */
        SelectSaveFileCallback selectSaveFile;
    };

    explicit MarkdownEditorPanel(wxWindow* parent, Callbacks callbacks = {});
    ~MarkdownEditorPanel() override;

   private:
    void HandleDocumentWatchRequested(const wxFileName& filePath);

    std::unique_ptr<FsWatcher> m_documentWatcher;
};
