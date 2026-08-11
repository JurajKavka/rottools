#include "MarkdownEditorPanel.h"

#include <wx/intl.h>

#include <functional>
#include <utility>

#include "FsWatcher.h"

namespace {
MarkdownEditorPanel::StatusMessage MakeStatusMessage(ScintillaTextEditorPanel::Status status,
                                                     const wxFileName& filePath) {
    switch (status) {
        case ScintillaTextEditorPanel::Status::Loading:
            return {.label = _("Loading ..."), .replaceOnPreviewReady = true};
        case ScintillaTextEditorPanel::Status::Reloading:
            return {.label = _("Reloading ..."), .replaceOnPreviewReady = true};
        case ScintillaTextEditorPanel::Status::Saved:
            return {.label = wxString::Format(_("Saved %s"), filePath.GetFullPath().c_str())};
        case ScintillaTextEditorPanel::Status::FileRemoved:
            return {.label = wxString::Format(_("File was removed on disk - the editor copy was kept: %s"),
                                              filePath.GetFullPath().c_str())};
        case ScintillaTextEditorPanel::Status::FileChangedWithUnsavedEdits:
            return {.label = wxString::Format(_("File changed on disk - your unsaved edits were kept: %s"),
                                              filePath.GetFullPath().c_str())};
    }

    return {};
}

void HandleStatusChanged(const MarkdownEditorPanel::OnStatusMessageCallback& onStatusMessage,
                         ScintillaTextEditorPanel::Status status, const wxFileName& filePath) {
    if (onStatusMessage) {
        onStatusMessage(MakeStatusMessage(status, filePath));
    }
}

MarkdownEditorPanel::ErrorMessage MakeErrorMessage(ScintillaTextEditorPanel::ErrorCode errorCode,
                                                   const wxFileName& filePath) {
    const wxString path = filePath.GetFullPath();
    switch (errorCode) {
        case ScintillaTextEditorPanel::ErrorCode::FileDoesNotExist:
            return {.text = wxString::Format(_("File does not exist: %s"), path.c_str())};
        case ScintillaTextEditorPanel::ErrorCode::FileNotReadable:
            return {.text = wxString::Format(_("No permission to read file: %s"), path.c_str())};
        case ScintillaTextEditorPanel::ErrorCode::FileReadFailed:
            return {.text = wxString::Format(_("Could not read file: %s"), path.c_str())};
        case ScintillaTextEditorPanel::ErrorCode::FileNotWritable:
            return {.text = wxString::Format(_("No permission to write file: %s"), path.c_str())};
        case ScintillaTextEditorPanel::ErrorCode::FileWriteFailed:
            return {.text = wxString::Format(_("Could not save file: %s"), path.c_str())};
        case ScintillaTextEditorPanel::ErrorCode::ExternalChangeCheckFailed:
            return {.text = wxString::Format(_("Could not check the current file before saving: %s"), path.c_str())};
    }

    return {};
}

void HandleError(const MarkdownEditorPanel::OnErrorMessageCallback& onErrorMessage,
                 ScintillaTextEditorPanel::ErrorCode errorCode, const wxFileName& filePath) {
    if (onErrorMessage) {
        onErrorMessage(MakeErrorMessage(errorCode, filePath));
    }
}

MarkdownEditorPanel::OverwritePromptMessage MakeOverwritePromptMessage(
    const ScintillaTextEditorPanel::OverwritePrompt& prompt) {
    const wxString path = prompt.filePath.GetFullPath();
    switch (prompt.reason) {
        case ScintillaTextEditorPanel::OverwritePromptReason::FileRemoved:
            return {
                .title = _("File Changed on Disk"),
                .text = wxString::Format(
                    _("The file was removed by another application:\n%s\n\nRecreate it with your editor contents?"),
                    path.c_str()),
                .actionLabel = _("Recreate"),
            };
        case ScintillaTextEditorPanel::OverwritePromptReason::FileChanged:
            return {
                .title = _("File Changed on Disk"),
                .text = wxString::Format(
                    _("The file changed in another application:\n%s\n\nOverwrite those external changes with your "
                      "editor contents?"),
                    path.c_str()),
                .actionLabel = _("Overwrite"),
            };
    }

    return {};
}

ScintillaTextEditorPanel::OverwritePromptDecision HandleConfirmOverwriteExternalChanges(
    const MarkdownEditorPanel::ConfirmOverwritePromptCallback& confirmOverwriteExternalChanges,
    const ScintillaTextEditorPanel::OverwritePrompt& prompt) {
    if (!confirmOverwriteExternalChanges) {
        return ScintillaTextEditorPanel::OverwritePromptDecision::Cancel;
    }
    return confirmOverwriteExternalChanges(MakeOverwritePromptMessage(prompt));
}
}  // namespace

MarkdownEditorPanel::MarkdownEditorPanel(wxWindow* parent, Callbacks callbacks)
    : ScintillaTextEditorPanel(
          parent, {.syntax = Syntax::Markdown, .lineNumbers = true, .wordWrap = true},
          {.documentChanged = std::move(callbacks.documentChanged),
           .statusChanged = std::bind_front(&HandleStatusChanged, std::move(callbacks.statusMessageChanged)),
           .confirmSaveBeforeDiscard = std::move(callbacks.confirmSaveBeforeDiscard),
           .confirmOverwriteExternalChanges = std::bind_front(&HandleConfirmOverwriteExternalChanges,
                                                              std::move(callbacks.confirmOverwriteExternalChanges)),
           .onError = std::bind_front(&HandleError, std::move(callbacks.error)),
           .documentWatchRequested = std::bind_front(&MarkdownEditorPanel::HandleDocumentWatchRequested, this),
           .selectOpenFile = std::move(callbacks.selectOpenFile),
           .selectSaveFile = std::move(callbacks.selectSaveFile)}) {}

MarkdownEditorPanel::~MarkdownEditorPanel() = default;

void MarkdownEditorPanel::HandleDocumentWatchRequested(const wxFileName& filePath) {
    m_documentWatcher.reset();
    if (filePath.IsOk()) {
        m_documentWatcher = std::make_unique<FsWatcher>(
            filePath, std::bind_front(&MarkdownEditorPanel::HandleDocumentWatcherChange, this));
    }
}
