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

    // TODO: Replace the positional callback parameters below with a documented
    // MarkdownEditorPanel::Callbacks aggregate. Keep document state decisions
    // in ScintillaTextEditorPanel, message translation and FsWatcher ownership
    // here, and native dialog presentation in MainFrame.

    /**
     * @param confirmSaveBeforeDiscard Synchronous confirmation callback used
     * before the current document may be replaced or the window may close. It
     * is called in two cases: the editor contains unsaved changes, or the
     * current file was removed outside the application and can be recreated.
     *
     * Returning Save writes the current editor contents first, opening Save As
     * for an untitled document or recreating a removed file as appropriate.
     * Returning Discard allows the pending operation to continue without
     * writing the document. Returning Cancel vetoes the pending operation. An
     * empty callback is treated as Cancel. Clean documents whose files still
     * exist do not invoke the callback.
     *
     * @param confirmOverwriteExternalChanges Synchronous confirmation callback
     * used before saving would recreate a removed file or overwrite changes
     * made outside the editor. It receives localized presentation text and
     * returns whether the save may proceed.
     */
    MarkdownEditorPanel(wxWindow* parent, OnDocumentChangedCallback onDocumentChanged,
                        OnStatusMessageCallback onStatusMessage,
                        ConfirmSaveBeforeDiscardCallback confirmSaveBeforeDiscard,
                        ConfirmOverwritePromptCallback confirmOverwriteExternalChanges,
                        OnErrorMessageCallback onErrorMessage);
    ~MarkdownEditorPanel() override;

   private:
    void HandleDocumentWatchRequested(const wxFileName& filePath);

    std::unique_ptr<FsWatcher> m_documentWatcher;
};
