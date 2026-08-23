#pragma once

#include <wx/event.h>
#include <wx/filename.h>
#include <wx/frame.h>
#include <wx/timer.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

#include "FileClipboard.h"
#include "FileOperationTask.h"

class FileManagerPane;
class wxProgressDialog;

wxDECLARE_EVENT(wxEVT_ROTFM_OPERATION_PROGRESS, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_ROTFM_OPERATION_COMPLETE, wxThreadEvent);

/** The dual-pane ROT File Manager window. */
class MainFrame final : public wxFrame {
   public:
    explicit MainFrame(wxWindow* parent);

    void OpenDirectory(const wxFileName& directory);

   private:
    FileManagerPane* m_leftPane = nullptr;
    FileManagerPane* m_rightPane = nullptr;
    FileManagerPane* m_activePane = nullptr;
    FileClipboard m_fileClipboard;
    std::unique_ptr<FileOperationTask> m_operationTask;
    wxTimer m_operationPulseTimer;
    wxProgressDialog* m_progressDialog = nullptr;
    bool m_clipboardPasteInProgress = false;
    bool m_closeAfterOperation = false;
    bool m_hasDeterminateProgress = false;
    std::uint64_t m_nextOperationId = 1;
    std::uint64_t m_activeOperationId = 0;

    void HandlePaneActivated(FileManagerPane* pane);
    void HandleNewFile(wxCommandEvent& event);
    void HandleCopy(wxCommandEvent& event);
    void HandleCut(wxCommandEvent& event);
    void HandlePaste(wxCommandEvent& event);
    void HandleDelete(wxCommandEvent& event);
    void HandleCopyToOtherPane(wxCommandEvent& event);
    void HandleMoveToOtherPane(wxCommandEvent& event);
    void HandleRefresh(wxCommandEvent& event);
    void HandleExit(wxCommandEvent& event);
    void HandleClose(wxCloseEvent& event);
    void HandleUpdateCommand(wxUpdateUIEvent& event);
    void HandleOperationProgress(wxThreadEvent& event);
    void HandleOperationComplete(wxThreadEvent& event);
    void HandleOperationPulse(wxTimerEvent& event);
    void HandleOperationCleanup();

    [[nodiscard]] FileManagerPane* GetOtherPane() const;
    [[nodiscard]] std::optional<std::filesystem::path> GetSelectedPath() const;
    void TransferToOtherPane(FileOperationKind kind);
    void StartFileOperation(FileOperationRequest request, bool clipboardPaste);
    void ReloadPanes();
    void ReportResult(const wxString& action, const FileOperationResult& result);
};
