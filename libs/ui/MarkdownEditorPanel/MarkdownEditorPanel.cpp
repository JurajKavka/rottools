#include "MarkdownEditorPanel.h"

#include <functional>
#include <utility>

#include "FsWatcher.h"

namespace {
constexpr auto kMarkdownFileWildcard = "Markdown files (*.md;*.markdown)|*.md;*.markdown";

MarkdownEditorPanel::StatusMessage MakeStatusMessage(ScintillaTextEditorPanel::Status status,
                                                     const wxFileName& filePath) {
    switch (status) {
        case ScintillaTextEditorPanel::Status::Loading:
            return {.label = "Loading ...", .replaceOnPreviewReady = true};
        case ScintillaTextEditorPanel::Status::Reloading:
            return {.label = "Reloading ...", .replaceOnPreviewReady = true};
        case ScintillaTextEditorPanel::Status::Saved:
            return {.label = wxString("Saved ") + filePath.GetFullPath()};
        case ScintillaTextEditorPanel::Status::FileRemoved:
            return {.label =
                        wxString("File was removed on disk - the editor copy was kept: ") + filePath.GetFullPath()};
        case ScintillaTextEditorPanel::Status::FileChangedWithUnsavedEdits:
            return {.label =
                        wxString("File changed on disk - your unsaved edits were kept: ") + filePath.GetFullPath()};
    }

    return {};
}

void HandleStatusChanged(const MarkdownEditorPanel::OnStatusMessageCallback& onStatusMessage,
                         ScintillaTextEditorPanel::Status status, const wxFileName& filePath) {
    if (onStatusMessage) {
        onStatusMessage(MakeStatusMessage(status, filePath));
    }
}
}  // namespace

MarkdownEditorPanel::MarkdownEditorPanel(wxWindow* parent, OnDocumentChangedCallback onDocumentChanged,
                                         OnStatusMessageCallback onStatusMessage,
                                         ConfirmSaveBeforeDiscardCallback confirmSaveBeforeDiscard,
                                         OnErrorCallback onError)
    : ScintillaTextEditorPanel(
          parent,
          {.syntax = Syntax::Markdown,
           .lineNumbers = true,
           .wordWrap = true,
           .fileWildcard = kMarkdownFileWildcard,
           .openDialogTitle = "Open Markdown File",
           .saveDialogTitle = "Save Markdown File"},
          {.documentChanged = std::move(onDocumentChanged),
           .statusChanged = std::bind_front(&HandleStatusChanged, std::move(onStatusMessage)),
           .confirmSaveBeforeDiscard = std::move(confirmSaveBeforeDiscard),
           .onError = std::move(onError),
           .documentWatchRequested = std::bind_front(&MarkdownEditorPanel::HandleDocumentWatchRequested, this)}) {}

MarkdownEditorPanel::~MarkdownEditorPanel() = default;

void MarkdownEditorPanel::HandleDocumentWatchRequested(const wxFileName& filePath) {
    m_documentWatcher.reset();
    if (filePath.IsOk()) {
        m_documentWatcher = std::make_unique<FsWatcher>(
            filePath, std::bind_front(&MarkdownEditorPanel::HandleDocumentWatcherChange, this));
    }
}
