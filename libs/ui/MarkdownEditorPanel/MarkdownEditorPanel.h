#pragma once

#include <wx/string.h>

#include <functional>
#include <memory>

#include "ScintillaTextEditorPanel.h"

class FsWatcher;

/**
 * Markdown-configured Scintilla editor with status labels and file watching.
 *
 * ScintillaTextEditorPanel owns the document lifecycle and reports semantic
 * events. This subclass supplies status labels and the FsWatcher requested
 * through the base panel's callbacks.
 */
class MarkdownEditorPanel final : public ScintillaTextEditorPanel {
   public:
    struct StatusMessage {
        wxString label;
        bool replaceOnPreviewReady = false;
    };

    using OnStatusMessageCallback = std::function<void(const StatusMessage&)>;

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
     */
    MarkdownEditorPanel(wxWindow* parent, OnDocumentChangedCallback onDocumentChanged,
                        OnStatusMessageCallback onStatusMessage,
                        ConfirmSaveBeforeDiscardCallback confirmSaveBeforeDiscard, OnErrorCallback onError);
    ~MarkdownEditorPanel() override;

   private:
    void HandleDocumentWatchRequested(const wxFileName& filePath);

    std::unique_ptr<FsWatcher> m_documentWatcher;
};
