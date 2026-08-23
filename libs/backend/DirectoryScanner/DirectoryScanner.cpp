#include "DirectoryScanner.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>
#include <unordered_set>

#include "HelperFunctions.h"

namespace fs = std::filesystem;

wxDEFINE_EVENT(wxEVT_DIRECTORY_SCAN_COMPLETE, DirectoryScannerEvent);

// Extension matching is case-insensitive, so ".md" also catches "README.MD"
static std::string ToLowerCopy(const std::string& text) {
    std::string lowered = text;
    std::ranges::transform(lowered, lowered.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lowered;
}

DirectoryScanner::DirectoryScanner() : m_workerThread([this] { WorkerLoop(); }) {}

DirectoryScanner::~DirectoryScanner() {
    {
        const std::lock_guard lock(m_mutex);
        m_stopping = true;
        if (m_activeCancellation) {
            *m_activeCancellation = true;
        }
        if (m_pendingRequest) {
            *m_pendingRequest->cancellation = true;
            m_pendingRequest.reset();
        }
    }
    m_requestAvailable.notify_one();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void DirectoryScanner::CancelScan() {
    const std::lock_guard lock(m_mutex);
    if (m_activeCancellation) {
        *m_activeCancellation = true;
    }
    if (m_pendingRequest) {
        *m_pendingRequest->cancellation = true;
        m_pendingRequest.reset();
    }
}

bool DirectoryScanner::IsScanning() const {
    const std::lock_guard lock(m_mutex);
    return m_activeCancellation != nullptr || m_pendingRequest.has_value();
}

std::uint64_t DirectoryScanner::StartScan(const wxFileName& fileName, const ScanOptions& options,
                                          wxEvtHandler* eventTarget) {
    std::uint64_t scanId = 0;
    {
        const std::lock_guard lock(m_mutex);
        if (m_activeCancellation) {
            *m_activeCancellation = true;
        }
        if (m_pendingRequest) {
            *m_pendingRequest->cancellation = true;
        }

        scanId = m_nextScanId++;
        m_pendingRequest = ScanRequest{
            .directory = fileName,
            .options = options,
            .eventTarget = eventTarget,
            .scanId = scanId,
            .cancellation = std::make_shared<std::atomic<bool>>(false),
        };
    }
    m_requestAvailable.notify_one();
    return scanId;
}

void DirectoryScanner::WorkerLoop() noexcept {
    try {
        for (;;) {
            std::optional<ScanRequest> request;
            {
                std::unique_lock lock(m_mutex);
                m_requestAvailable.wait(lock, [this] { return m_stopping || m_pendingRequest.has_value(); });
                if (m_stopping) {
                    return;
                }
                request = std::move(m_pendingRequest);
                m_pendingRequest.reset();
                m_activeCancellation = request->cancellation;
            }

            ScanThreadLogic(*request);

            {
                const std::lock_guard lock(m_mutex);
                if (m_activeCancellation == request->cancellation) {
                    m_activeCancellation.reset();
                }
            }
        }
    } catch (...) {
        // No exception may escape a std::thread entry point.
    }
}

void DirectoryScanner::ScanThreadLogic(const ScanRequest& request) noexcept {
    try {
        std::vector<FileEntry> results;
        std::unordered_set<std::string> extSet;
        for (const std::string& extension : request.options.extensions) {
            extSet.insert(ToLowerCopy(extension));
        }

        const fs::path rootPath = request.directory.GetFullPath().ToStdWstring();
        if (fs::exists(rootPath) && fs::is_directory(rootPath)) {
            auto dirOptions = fs::directory_options::skip_permission_denied;
            for (const auto& entry : fs::directory_iterator(rootPath, dirOptions)) {
                if (*request.cancellation) {
                    break;
                }

                // A failure on a single entry only skips that entry
                try {
                    // Skip symlinks to prevent circular reference errors
                    if (entry.is_symlink()) {
                        continue;
                    }

                    std::string filename = entry.path().filename().string();
                    // 🛠️ Check 1: Handle Hidden Files Option
                    if (!request.options.showHiddenFiles && filename.starts_with('.')) {
                        continue;
                    }

                    bool isDir = entry.is_directory();
                    std::string ext = entry.path().extension().string();

                    if (isDir || extSet.empty() || extSet.contains(ToLowerCopy(ext))) {
                        results.push_back(FileEntry{
                            .path = entry.path(),
                            .name = std::move(filename),
                            .isDirectory = isDir,
                            .size = isDir ? 0 : fs::file_size(entry.path()),
                        });
                    }
                } catch (const fs::filesystem_error& e) {
                    printError("[Error] Skipping entry: {} - {}", e.code().message(), e.what());
                }
            }
        }

        if (!*request.cancellation && request.eventTarget != nullptr) {
            auto* event = new DirectoryScannerEvent(wxEVT_DIRECTORY_SCAN_COMPLETE, wxID_ANY);
            event->files = std::move(results);
            event->currentDirectory = request.directory;
            event->scanId = request.scanId;
            wxQueueEvent(request.eventTarget, event);
        }
    } catch (const std::exception& error) {
        try {
            printError("[Error] Directory scan aborted: {}", error.what());
        } catch (...) {
        }
    } catch (...) {
        try {
            printError("[Error] Directory scan aborted by an unknown exception");
        } catch (...) {
        }
    }
}

std::vector<FileEntry> DirectoryScanner::SortEntries(const std::vector<FileEntry>& entries) {
    // Sort an explicit local copy; the caller's data stays untouched
    std::vector<FileEntry> sorted = entries;

    std::ranges::sort(sorted, [](const FileEntry& a, const FileEntry& b) {
        // Tier 1: Group Directories at the top
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory > b.isDirectory;
        }
        // Tier 2: Case-insensitive alphabetical sort
        auto lower = [](char c) { return std::tolower(static_cast<unsigned char>(c)); };
        if (std::ranges::lexicographical_compare(a.name, b.name, {}, lower, lower)) {
            return true;
        }
        if (std::ranges::lexicographical_compare(b.name, a.name, {}, lower, lower)) {
            return false;
        }
        // Tie-breaker: If names are identical case-insensitively (e.g., "Apple" vs "apple"),
        // fall back to a case-sensitive check to preserve strict weak ordering requirements.
        return a.name < b.name;
    });

    return sorted;
}
