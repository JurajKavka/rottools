#include "MarkdownEditorPanel.h"

#include <wx/intl.h>

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

MarkdownEditorPanel::MarkdownEditorPanel(wxWindow* parent, OnDocumentChangedCallback onDocumentChanged,
                                         OnStatusMessageCallback onStatusMessage,
                                         ConfirmSaveBeforeDiscardCallback confirmSaveBeforeDiscard,
                                         ConfirmOverwritePromptCallback confirmOverwriteExternalChanges,
                                         OnErrorMessageCallback onErrorMessage)
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
           .confirmOverwriteExternalChanges =
               std::bind_front(&HandleConfirmOverwriteExternalChanges, std::move(confirmOverwriteExternalChanges)),
           .onError = std::bind_front(&HandleError, std::move(onErrorMessage)),
           .documentWatchRequested = std::bind_front(&MarkdownEditorPanel::HandleDocumentWatchRequested, this)}) {}

MarkdownEditorPanel::~MarkdownEditorPanel() = default;

void MarkdownEditorPanel::HandleDocumentWatchRequested(const wxFileName& filePath) {
    m_documentWatcher.reset();
    if (filePath.IsOk()) {
        m_documentWatcher = std::make_unique<FsWatcher>(
            filePath, std::bind_front(&MarkdownEditorPanel::HandleDocumentWatcherChange, this));
    }
}
