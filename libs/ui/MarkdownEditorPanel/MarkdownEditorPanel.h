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
 * statuses. This subclass supplies their user-facing labels and the FsWatcher
 * requested through the base panel's callback.
 */
class MarkdownEditorPanel final : public ScintillaTextEditorPanel {
   public:
    struct StatusMessage {
        wxString label;
        bool replaceOnPreviewReady = false;
    };

    using OnStatusMessageCallback = std::function<void(const StatusMessage&)>;

    MarkdownEditorPanel(wxWindow* parent, OnDocumentChangedCallback onDocumentChanged,
                        OnStatusMessageCallback onStatusMessage, OnErrorCallback onError);
    ~MarkdownEditorPanel() override;

   private:
    void HandleDocumentWatchRequested(const wxFileName& filePath);

    std::unique_ptr<FsWatcher> m_documentWatcher;
};
