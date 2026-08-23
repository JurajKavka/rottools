#include <gio/gio.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "NativeFileTrash.h"

namespace {

std::error_code ErrorFromGio(const GError* error) {
    if (error == nullptr) {
        return std::make_error_code(std::errc::io_error);
    }
    if (error->domain != G_IO_ERROR) {
        return std::make_error_code(std::errc::io_error);
    }

    switch (static_cast<GIOErrorEnum>(error->code)) {
        case G_IO_ERROR_CANCELLED:
            return std::make_error_code(std::errc::operation_canceled);
        case G_IO_ERROR_NOT_FOUND:
            return std::make_error_code(std::errc::no_such_file_or_directory);
        case G_IO_ERROR_EXISTS:
            return std::make_error_code(std::errc::file_exists);
        case G_IO_ERROR_NOT_DIRECTORY:
            return std::make_error_code(std::errc::not_a_directory);
        case G_IO_ERROR_PERMISSION_DENIED:
            return std::make_error_code(std::errc::permission_denied);
        case G_IO_ERROR_NOT_SUPPORTED:
            return std::make_error_code(std::errc::operation_not_supported);
        default:
            return std::make_error_code(std::errc::io_error);
    }
}

}  // namespace

NativeFileTrashResult NativeFileTrash::Trash(const std::filesystem::path& path,
                                             const FileOperationCancellation& cancellation) {
    if (cancellation.IsCancelled()) {
        return {
            .outcome = NativeFileTrashOutcome::Cancelled,
            .error = std::make_error_code(std::errc::operation_canceled),
        };
    }

    GFile* file = g_file_new_for_path(path.c_str());
    GCancellable* gioCancellation = g_cancellable_new();
    if (file == nullptr || gioCancellation == nullptr) {
        if (file != nullptr) {
            g_object_unref(file);
        }
        if (gioCancellation != nullptr) {
            g_object_unref(gioCancellation);
        }
        return {
            .outcome = NativeFileTrashOutcome::Failed,
            .error = std::make_error_code(std::errc::not_enough_memory),
        };
    }

    std::atomic<bool> finished{false};
    std::jthread cancellationWatcher([&](const std::stop_token& stopToken) {
        while (!stopToken.stop_requested() && !finished) {
            if (cancellation.IsCancelled()) {
                g_cancellable_cancel(gioCancellation);
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    GError* error = nullptr;
    const gboolean trashed = g_file_trash(file, gioCancellation, &error);
    finished = true;
    cancellationWatcher.request_stop();
    cancellationWatcher.join();

    g_object_unref(gioCancellation);
    g_object_unref(file);

    if (trashed != FALSE) {
        if (error != nullptr) {
            g_error_free(error);
        }
        return {.outcome = NativeFileTrashOutcome::Completed};
    }

    const std::error_code operationError = ErrorFromGio(error);
    const bool wasCancelled = error != nullptr && error->domain == G_IO_ERROR && error->code == G_IO_ERROR_CANCELLED;
    if (error != nullptr) {
        g_error_free(error);
    }
    return {
        .outcome = wasCancelled ? NativeFileTrashOutcome::Cancelled : NativeFileTrashOutcome::Failed,
        .error = operationError,
    };
}
