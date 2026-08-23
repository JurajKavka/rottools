#include "MainFrame.h"

#include <wx/accel.h>
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/progdlg.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stockitem.h>
#include <wx/textdlg.h>

#include <algorithm>
#include <array>

#include "FileManagerPane.h"

wxDEFINE_EVENT(wxEVT_ROTFM_OPERATION_PROGRESS, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_ROTFM_OPERATION_COMPLETE, wxThreadEvent);

namespace fs = std::filesystem;

namespace {

struct OperationProgressPayload {
    std::uint64_t operationId = 0;
    FileOperationProgress progress;
};

struct OperationCompletePayload {
    std::uint64_t operationId = 0;
    FileOperationResult result;
};

enum CommandId {
    kCopyToOtherPane = wxID_HIGHEST + 1,
    kMoveToOtherPane,
    kDelete,
    kRefresh,
    kOperationPulse,
};

fs::path ToPath(const wxFileName& fileName) {
    return fs::path(fileName.GetFullPath().ToStdWstring());
}

wxString ToWxString(const fs::path& path) {
    return wxString(path.wstring());
}

wxString ErrorMessage(const std::error_code& error) {
    return wxString::FromUTF8(error.message());
}

wxString ActionName(FileOperationKind kind) {
    switch (kind) {
        case FileOperationKind::CreateFile:
            return "Create file";
        case FileOperationKind::Copy:
            return "Copy";
        case FileOperationKind::Move:
            return "Move";
        case FileOperationKind::Trash:
            return "Move to Trash";
    }
    return "File operation";
}

}  // namespace

MainFrame::MainFrame(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, "ROT File Manager", wxDefaultPosition, wxSize(1100, 700)),
      m_operationPulseTimer(this, kOperationPulse) {
    const long stockLabelFlags = wxSTOCK_WITH_MNEMONIC | wxSTOCK_WITH_ACCELERATOR;

    auto* fileMenu = new wxMenu;
    fileMenu->Append(wxID_NEW, wxGetStockLabel(wxID_NEW, stockLabelFlags));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, wxGetStockLabel(wxID_EXIT, stockLabelFlags));

    auto* editMenu = new wxMenu;
    editMenu->Append(wxID_CUT, wxGetStockLabel(wxID_CUT, stockLabelFlags));
    editMenu->Append(wxID_COPY, wxGetStockLabel(wxID_COPY, stockLabelFlags));
    editMenu->Append(wxID_PASTE, wxGetStockLabel(wxID_PASTE, stockLabelFlags));
    editMenu->AppendSeparator();
    editMenu->Append(kDelete, "&Delete\tF8");

    auto* commandsMenu = new wxMenu;
    commandsMenu->Append(kCopyToOtherPane, "&Copy to Other Pane\tF5");
    commandsMenu->Append(kMoveToOtherPane, "&Move to Other Pane\tF6");
    commandsMenu->AppendSeparator();
    commandsMenu->Append(kRefresh, "&Refresh\tCtrl+R");

    auto* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(editMenu, "&Edit");
    menuBar->Append(commandsMenu, "&Commands");
    SetMenuBar(menuBar);

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3D);
    m_leftPane = new FileManagerPane(splitter, [this](FileManagerPane* pane) { HandlePaneActivated(pane); });
    m_rightPane = new FileManagerPane(splitter, [this](FileManagerPane* pane) { HandlePaneActivated(pane); });
    splitter->SetMinimumPaneSize(200);
    splitter->SetSashGravity(0.5);
    splitter->SplitVertically(m_leftPane, m_rightPane, 550);
    mainSizer->Add(splitter, 1, wxEXPAND);

    auto* commandSizer = new wxBoxSizer(wxHORIZONTAL);
    commandSizer->Add(new wxButton(this, kCopyToOtherPane, "F5 Copy"), 1, wxEXPAND | wxRIGHT, 4);
    commandSizer->Add(new wxButton(this, kMoveToOtherPane, "F6 Move"), 1, wxEXPAND | wxRIGHT, 4);
    commandSizer->Add(new wxButton(this, wxID_NEW, "Shift+F4 New file"), 1, wxEXPAND | wxRIGHT, 4);
    commandSizer->Add(new wxButton(this, kDelete, "F8 Delete"), 1, wxEXPAND);
    mainSizer->Add(commandSizer, 0, wxEXPAND | wxALL, 6);
    SetSizer(mainSizer);

    CreateStatusBar();

    Bind(wxEVT_MENU, &MainFrame::HandleNewFile, this, wxID_NEW);
    Bind(wxEVT_MENU, &MainFrame::HandleCopy, this, wxID_COPY);
    Bind(wxEVT_MENU, &MainFrame::HandleCut, this, wxID_CUT);
    Bind(wxEVT_MENU, &MainFrame::HandlePaste, this, wxID_PASTE);
    Bind(wxEVT_MENU, &MainFrame::HandleDelete, this, kDelete);
    Bind(wxEVT_MENU, &MainFrame::HandleCopyToOtherPane, this, kCopyToOtherPane);
    Bind(wxEVT_MENU, &MainFrame::HandleMoveToOtherPane, this, kMoveToOtherPane);
    Bind(wxEVT_MENU, &MainFrame::HandleRefresh, this, kRefresh);
    Bind(wxEVT_MENU, &MainFrame::HandleExit, this, wxID_EXIT);
    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::HandleClose, this);
    Bind(wxEVT_ROTFM_OPERATION_PROGRESS, &MainFrame::HandleOperationProgress, this);
    Bind(wxEVT_ROTFM_OPERATION_COMPLETE, &MainFrame::HandleOperationComplete, this);
    Bind(wxEVT_TIMER, &MainFrame::HandleOperationPulse, this, kOperationPulse);

    Bind(wxEVT_BUTTON, &MainFrame::HandleNewFile, this, wxID_NEW);
    Bind(wxEVT_BUTTON, &MainFrame::HandleDelete, this, kDelete);
    Bind(wxEVT_BUTTON, &MainFrame::HandleCopyToOtherPane, this, kCopyToOtherPane);
    Bind(wxEVT_BUTTON, &MainFrame::HandleMoveToOtherPane, this, kMoveToOtherPane);

    const std::array<int, 8> commandIds = {
        wxID_NEW, wxID_COPY, wxID_CUT, wxID_PASTE, kDelete, kCopyToOtherPane, kMoveToOtherPane, kRefresh,
    };
    for (const int id : commandIds) {
        Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateCommand, this, id);
    }

    std::array<wxAcceleratorEntry, 4> accelerators;
    accelerators[0].Set(wxACCEL_NORMAL, WXK_F5, kCopyToOtherPane);
    accelerators[1].Set(wxACCEL_NORMAL, WXK_F6, kMoveToOtherPane);
    accelerators[2].Set(wxACCEL_SHIFT, WXK_F4, wxID_NEW);
    accelerators[3].Set(wxACCEL_NORMAL, WXK_F8, kDelete);
    SetAcceleratorTable(wxAcceleratorTable(static_cast<int>(accelerators.size()), accelerators.data()));

    wxFileName home;
    home.AssignHomeDir();
    m_leftPane->SetDirectory(home);
    m_rightPane->SetDirectory(home);
    HandlePaneActivated(m_leftPane);
    m_leftPane->FocusFileList();
}

void MainFrame::OpenDirectory(const wxFileName& directory) {
    if (!directory.DirExists()) {
        return;
    }
    m_leftPane->SetDirectory(directory);
    HandlePaneActivated(m_leftPane);
    m_leftPane->FocusFileList();
}

void MainFrame::HandlePaneActivated(FileManagerPane* pane) {
    m_activePane = pane;
    SetStatusText(pane->GetCurrentDirectory().GetFullPath());
}

void MainFrame::HandleNewFile(wxCommandEvent& event) {
    wxTextEntryDialog dialog(this, "Enter the name for the new empty file:", "New File");
    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    const fs::path name = fs::path(dialog.GetValue().ToStdWstring());
    if (name.empty() || name != name.filename() || name == "." || name == "..") {
        wxMessageBox("Enter a file name without directory components.", "New File", wxOK | wxICON_ERROR, this);
        return;
    }

    const fs::path filePath = ToPath(m_activePane->GetCurrentDirectory()) / name;
    const FileOperationCancellation cancellation;
    const FileOperationResult result =
        FileOperations::Execute({.kind = FileOperationKind::CreateFile, .source = filePath}, cancellation, {});
    ReportResult("Create file", result);
    ReloadPanes();
    m_activePane->FocusFileList();
}

void MainFrame::HandleCopy(wxCommandEvent& event) {
    const std::optional<fs::path> selected = GetSelectedPath();
    if (!selected) {
        return;
    }
    const FileClipboardSelectionResult result = m_fileClipboard.SelectForCopy(*selected);
    if (result.Succeeded()) {
        SetStatusText("Selected for copy: " + ToWxString(result.selectedPath));
    } else {
        wxMessageBox(ErrorMessage(result.error), "Select for copy", wxOK | wxICON_ERROR, this);
    }
}

void MainFrame::HandleCut(wxCommandEvent& event) {
    const std::optional<fs::path> selected = GetSelectedPath();
    if (!selected) {
        return;
    }
    const FileClipboardSelectionResult result = m_fileClipboard.SelectForMove(*selected);
    if (result.Succeeded()) {
        SetStatusText("Selected for move: " + ToWxString(result.selectedPath));
    } else {
        wxMessageBox(ErrorMessage(result.error), "Select for move", wxOK | wxICON_ERROR, this);
    }
}

void MainFrame::HandlePaste(wxCommandEvent& event) {
    const FileClipboardPasteRequestResult paste =
        m_fileClipboard.CreatePasteRequest(ToPath(m_activePane->GetCurrentDirectory()));
    if (!paste.Succeeded()) {
        wxMessageBox(ErrorMessage(paste.error), "Paste", wxOK | wxICON_ERROR, this);
        return;
    }
    StartFileOperation(*paste.request, true);
}

void MainFrame::HandleDelete(wxCommandEvent& event) {
    const std::optional<fs::path> selected = GetSelectedPath();
    if (!selected) {
        return;
    }

    const wxString prompt = "Move this item to the system Trash?\n\n" + ToWxString(*selected);
    if (wxMessageBox(prompt, "Move to Trash", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this) != wxYES) {
        return;
    }
    StartFileOperation({.kind = FileOperationKind::Trash, .source = *selected}, false);
}

void MainFrame::HandleCopyToOtherPane(wxCommandEvent& event) {
    TransferToOtherPane(FileOperationKind::Copy);
}

void MainFrame::HandleMoveToOtherPane(wxCommandEvent& event) {
    TransferToOtherPane(FileOperationKind::Move);
}

void MainFrame::HandleRefresh(wxCommandEvent& event) {
    ReloadPanes();
    m_activePane->FocusFileList();
}

void MainFrame::HandleExit(wxCommandEvent& event) {
    Close();
}

void MainFrame::HandleClose(wxCloseEvent& event) {
    if (!m_operationTask) {
        event.Skip();
        return;
    }

    m_operationTask->Cancel();
    if (event.CanVeto()) {
        m_closeAfterOperation = true;
        event.Veto();
        return;
    }

    // System shutdown cannot be vetoed. Join while the wxEvtHandler base is
    // still alive, so the worker cannot queue into a destroyed event target.
    m_operationPulseTimer.Stop();
    m_operationTask.reset();
    event.Skip();
}

void MainFrame::HandleUpdateCommand(wxUpdateUIEvent& event) {
    if (m_operationTask) {
        event.Enable(false);
        return;
    }
    if (event.GetId() == wxID_PASTE) {
        event.Enable(m_activePane != nullptr && m_fileClipboard.HasSelection());
        return;
    }
    if (event.GetId() == wxID_NEW || event.GetId() == kRefresh) {
        event.Enable(m_activePane != nullptr);
        return;
    }
    event.Enable(GetSelectedPath().has_value());
}

FileManagerPane* MainFrame::GetOtherPane() const {
    return m_activePane == m_leftPane ? m_rightPane : m_leftPane;
}

std::optional<fs::path> MainFrame::GetSelectedPath() const {
    if (m_activePane == nullptr) {
        return std::nullopt;
    }
    const std::optional<wxFileName> selected = m_activePane->GetSelectedPath();
    if (!selected) {
        return std::nullopt;
    }
    return ToPath(*selected);
}

void MainFrame::TransferToOtherPane(FileOperationKind kind) {
    const std::optional<fs::path> selected = GetSelectedPath();
    if (!selected) {
        return;
    }

    const fs::path destination = ToPath(GetOtherPane()->GetCurrentDirectory()) / selected->filename();
    StartFileOperation({.kind = kind, .source = *selected, .destination = destination}, false);
}

void MainFrame::StartFileOperation(FileOperationRequest request, bool clipboardPaste) {
    if (m_operationTask) {
        return;
    }

    m_clipboardPasteInProgress = clipboardPaste;
    m_hasDeterminateProgress = false;
    const std::uint64_t operationId = m_nextOperationId++;
    m_activeOperationId = operationId;
    const wxString action = ActionName(request.kind);
    m_progressDialog = new wxProgressDialog(
        action, action + " " + ToWxString(request.source.filename()), 100, this,
        wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME | wxPD_SMOOTH);
    m_operationPulseTimer.Start(100);

    m_operationTask = std::make_unique<FileOperationTask>(
        std::move(request),
        [this, operationId](const FileOperationProgress& progress) {
            auto* event = new wxThreadEvent(wxEVT_ROTFM_OPERATION_PROGRESS);
            event->SetPayload(OperationProgressPayload{.operationId = operationId, .progress = progress});
            wxQueueEvent(this, event);
        },
        [this, operationId](const FileOperationResult& result) {
            auto* event = new wxThreadEvent(wxEVT_ROTFM_OPERATION_COMPLETE);
            event->SetPayload(OperationCompletePayload{.operationId = operationId, .result = result});
            wxQueueEvent(this, event);
        });
}

void MainFrame::HandleOperationProgress(wxThreadEvent& event) {
    const OperationProgressPayload payload = event.GetPayload<OperationProgressPayload>();
    if (payload.operationId != m_activeOperationId || m_progressDialog == nullptr || m_operationTask == nullptr) {
        return;
    }

    const FileOperationProgress& progress = payload.progress;
    bool shouldContinue = true;
    const wxString message = ActionName(progress.request.kind) + " " + ToWxString(progress.request.source.filename());
    if (progress.totalBytes == 0) {
        shouldContinue = m_progressDialog->Pulse(message);
    } else {
        m_hasDeterminateProgress = true;
        // Reaching the dialog's maximum switches some native implementations
        // into their terminal modal state. Completion owns closing the dialog,
        // so progress stays below the maximum until that notification arrives.
        const auto percentage = static_cast<int>(
            std::min<long double>(99, (static_cast<long double>(progress.bytesCompleted) * 100) / progress.totalBytes));
        shouldContinue = m_progressDialog->Update(percentage, message);
    }
    if (!shouldContinue) {
        m_operationTask->Cancel();
    }
}

void MainFrame::HandleOperationComplete(wxThreadEvent& event) {
    const OperationCompletePayload payload = event.GetPayload<OperationCompletePayload>();
    if (payload.operationId != m_activeOperationId || m_operationTask == nullptr) {
        return;
    }

    const FileOperationResult& result = payload.result;
    m_operationPulseTimer.Stop();
    if (m_progressDialog != nullptr) {
        m_progressDialog->Destroy();
        m_progressDialog = nullptr;
    }

    if (m_clipboardPasteInProgress) {
        m_fileClipboard.HandleCompleted(result);
    }
    ReportResult(ActionName(result.request.kind), result);

    m_clipboardPasteInProgress = false;
    m_activeOperationId = 0;
    CallAfter([this] { HandleOperationCleanup(); });
}

void MainFrame::HandleOperationPulse(wxTimerEvent& event) {
    if (m_hasDeterminateProgress || m_progressDialog == nullptr || m_operationTask == nullptr) {
        return;
    }
    if (!m_progressDialog->Pulse()) {
        m_operationTask->Cancel();
    }
}

void MainFrame::HandleOperationCleanup() {
    // The completion notification originates on the worker. Avoid joining that
    // worker from the UI thread until it has fully returned from its callback.
    if (m_operationTask && m_operationTask->IsRunning()) {
        CallAfter([this] { HandleOperationCleanup(); });
        return;
    }

    m_operationTask.reset();
    if (m_closeAfterOperation) {
        m_closeAfterOperation = false;
        Close();
        return;
    }
    ReloadPanes();
    m_activePane->FocusFileList();
}

void MainFrame::ReloadPanes() {
    m_leftPane->Reload();
    m_rightPane->Reload();
}

void MainFrame::ReportResult(const wxString& action, const FileOperationResult& result) {
    if (result.Succeeded()) {
        SetStatusText(action + " completed");
        return;
    }
    if (result.outcome == FileOperationOutcome::Cancelled && !result.residualPath) {
        SetStatusText(action + " cancelled");
        return;
    }

    SetStatusText(action + (result.outcome == FileOperationOutcome::Cancelled ? " cancelled" : " failed"));
    wxString message = action + (result.outcome == FileOperationOutcome::Cancelled ? " was cancelled." : " failed.");
    if (!result.request.source.empty()) {
        message += "\n\nSource: " + ToWxString(result.request.source);
    }
    if (result.request.destination) {
        message += "\nDestination: " + ToWxString(*result.request.destination);
    }
    message += "\n" + ErrorMessage(result.error);
    if (result.residualPath) {
        message += "\n\nAn entry remains and requires review:\n" + ToWxString(*result.residualPath);
    }
    wxMessageBox(message, action, wxOK | wxICON_ERROR, this);
}
