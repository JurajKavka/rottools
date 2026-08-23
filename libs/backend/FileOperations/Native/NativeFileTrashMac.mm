#import <AppKit/AppKit.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

#include "NativeFileTrash.h"

namespace {

class CocoaErrorCategory final : public std::error_category {
   public:
    [[nodiscard]] const char* name() const noexcept override {
        return "macos.cocoa";
    }

    [[nodiscard]] std::string message(int error) const override {
        return "macOS trash error " + std::to_string(error);
    }
};

const std::error_category& GetCocoaErrorCategory() {
    static const CocoaErrorCategory category;
    return category;
}

std::error_code ErrorFromNSError(NSError* error) {
    if (error == nil) {
        return {};
    }

    NSError* underlying = error.userInfo[NSUnderlyingErrorKey];
    if ([error.domain isEqualToString:NSPOSIXErrorDomain]) {
        return std::error_code(static_cast<int>(error.code), std::generic_category());
    }
    if (underlying != nil && [underlying.domain isEqualToString:NSPOSIXErrorDomain]) {
        return std::error_code(static_cast<int>(underlying.code), std::generic_category());
    }
    return std::error_code(static_cast<int>(error.code), GetCocoaErrorCategory());
}

struct TrashCompletion {
    std::mutex mutex;
    std::condition_variable condition;
    bool done = false;
    bool movedToTrash = false;
    std::error_code error;
};

}  // namespace

NativeFileTrashResult NativeFileTrash::Trash(const std::filesystem::path& path,
                                             const FileOperationCancellation& cancellation) {
    if (cancellation.IsCancelled()) {
        return {
            .outcome = NativeFileTrashOutcome::Cancelled,
            .error = std::make_error_code(std::errc::operation_canceled),
        };
    }

    @autoreleasepool {
        NSString* pathString = [[NSFileManager defaultManager]
            stringWithFileSystemRepresentation:path.c_str()
                                        length:std::char_traits<char>::length(path.c_str())];
        if (pathString == nil) {
            return {
                .outcome = NativeFileTrashOutcome::Failed,
                .error = std::make_error_code(std::errc::invalid_argument),
            };
        }

        NSURL* url = [NSURL fileURLWithPath:pathString];
        auto completion = std::make_shared<TrashCompletion>();
        [[NSWorkspace sharedWorkspace] recycleURLs:@[ url ]
                                 completionHandler:^(NSDictionary<NSURL*, NSURL*>* newURLs, NSError* error) {
                                   const std::lock_guard lock(completion->mutex);
                                   completion->movedToTrash = [newURLs objectForKey:url] != nil;
                                   completion->error = ErrorFromNSError(error);
                                   completion->done = true;
                                   completion->condition.notify_one();
                                 }];

        std::unique_lock lock(completion->mutex);
        completion->condition.wait(lock, [&completion] { return completion->done; });
        if (completion->movedToTrash) {
            return {.outcome = NativeFileTrashOutcome::Completed};
        }
        return {
            .outcome = NativeFileTrashOutcome::Failed,
            .error = completion->error ? completion->error : std::make_error_code(std::errc::io_error),
        };
    }
}
