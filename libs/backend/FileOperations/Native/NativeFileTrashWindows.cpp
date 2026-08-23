#include <shobjidl.h>
#include <windows.h>

#include "NativeFileTrash.h"

namespace {

template <typename T>
class ComPointer final {
   public:
    ~ComPointer() {
        if (m_value != nullptr) {
            m_value->Release();
        }
    }

    ComPointer(const ComPointer&) = delete;
    ComPointer& operator=(const ComPointer&) = delete;

    ComPointer() = default;

    [[nodiscard]] T* Get() const noexcept {
        return m_value;
    }

    [[nodiscard]] T** Put() noexcept {
        return &m_value;
    }

   private:
    T* m_value = nullptr;
};

class ComApartment final {
   public:
    ComApartment() : m_result(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) {}

    ~ComApartment() {
        if (SUCCEEDED(m_result)) {
            CoUninitialize();
        }
    }

    [[nodiscard]] HRESULT Result() const noexcept {
        return m_result;
    }

   private:
    HRESULT m_result;
};

NativeFileTrashResult FailedResult(HRESULT result) {
    return {
        .outcome = NativeFileTrashOutcome::Failed,
        .error = std::error_code(static_cast<int>(result), std::system_category()),
    };
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

    const ComApartment apartment;
    if (FAILED(apartment.Result())) {
        return FailedResult(apartment.Result());
    }

    ComPointer<IFileOperation> operation;
    HRESULT result =
        CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(operation.Put()));
    if (FAILED(result)) {
        return FailedResult(result);
    }

    constexpr DWORD flags =
        FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOFX_EARLYFAILURE | FOFX_RECYCLEONDELETE;
    result = operation.Get()->SetOperationFlags(flags);
    if (FAILED(result)) {
        return FailedResult(result);
    }

    ComPointer<IShellItem> item;
    result = SHCreateItemFromParsingName(path.c_str(), nullptr, IID_PPV_ARGS(item.Put()));
    if (FAILED(result)) {
        return FailedResult(result);
    }

    result = operation.Get()->DeleteItem(item.Get(), nullptr);
    if (FAILED(result)) {
        return FailedResult(result);
    }
    if (cancellation.IsCancelled()) {
        return {
            .outcome = NativeFileTrashOutcome::Cancelled,
            .error = std::make_error_code(std::errc::operation_canceled),
        };
    }

    result = operation.Get()->PerformOperations();
    if (FAILED(result)) {
        return FailedResult(result);
    }

    BOOL aborted = FALSE;
    result = operation.Get()->GetAnyOperationsAborted(&aborted);
    if (FAILED(result)) {
        return FailedResult(result);
    }
    if (aborted != FALSE) {
        return {
            .outcome = NativeFileTrashOutcome::Cancelled,
            .error = std::make_error_code(std::errc::operation_canceled),
        };
    }
    return {.outcome = NativeFileTrashOutcome::Completed};
}
