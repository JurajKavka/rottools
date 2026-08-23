#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#include "FileOperations.h"

/**
 * Runs exactly one FileOperations request on a worker thread.
 *
 * This layer contains no filesystem policy. Progress and completion callbacks
 * execute on the worker thread; UI callers must marshal immutable snapshots to
 * their event thread.
 */
class FileOperationTask final {
   public:
    using ProgressCallback = std::function<void(const FileOperationProgress&)>;
    using CompletionCallback = std::function<void(const FileOperationResult&)>;

    FileOperationTask(FileOperationRequest request, ProgressCallback handleProgress, CompletionCallback handleComplete);
    ~FileOperationTask();

    FileOperationTask(const FileOperationTask&) = delete;
    FileOperationTask& operator=(const FileOperationTask&) = delete;

    void Cancel() noexcept;
    [[nodiscard]] bool IsRunning() const noexcept;

   private:
    struct State {
        FileOperationCancellation cancellation;
        std::atomic<bool> isRunning{true};
    };

    std::shared_ptr<State> m_state;
    std::thread m_worker;

    static void Run(const std::shared_ptr<State>& state, FileOperationRequest request, ProgressCallback handleProgress,
                    CompletionCallback handleComplete) noexcept;
};
