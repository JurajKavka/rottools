#include "FileOperationTask.h"

#include <system_error>
#include <utility>

FileOperationTask::FileOperationTask(FileOperationRequest request, ProgressCallback handleProgress,
                                     CompletionCallback handleComplete)
    : m_state(std::make_shared<State>()),
      m_worker([state = m_state, request = std::move(request), handleProgress = std::move(handleProgress),
                handleComplete = std::move(handleComplete)]() mutable {
          Run(state, std::move(request), std::move(handleProgress), std::move(handleComplete));
      }) {}

FileOperationTask::~FileOperationTask() {
    Cancel();
    if (!m_worker.joinable()) {
        return;
    }
    if (m_worker.get_id() == std::this_thread::get_id()) {
        m_worker.detach();
    } else {
        m_worker.join();
    }
}

void FileOperationTask::Cancel() noexcept {
    m_state->cancellation.Cancel();
}

bool FileOperationTask::IsRunning() const noexcept {
    return m_state->isRunning;
}

void FileOperationTask::Run(const std::shared_ptr<State>& state, FileOperationRequest request,
                            ProgressCallback handleProgress, CompletionCallback handleComplete) noexcept {
    FileOperationResult result;
    try {
        result = FileOperations::Execute(request, state->cancellation, handleProgress);
    } catch (...) {
        result = {
            .request = std::move(request),
            .outcome = FileOperationOutcome::Failed,
            .error = std::make_error_code(std::errc::io_error),
        };
    }

    if (handleComplete) {
        try {
            handleComplete(result);
        } catch (...) {
            // Observers cannot be allowed to escape the std::thread boundary.
        }
    }
    state->isRunning = false;
}
