#include "FileManagerBackend.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <fstream>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/stdio.h>
#elif defined(__linux__)
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace FileManagerBackend {
namespace {

std::error_code Error(std::errc code) {
    return std::make_error_code(code);
}

fs::path WithoutTrailingSeparator(fs::path path) {
    while (!path.has_filename() && path != path.parent_path())
        path = path.parent_path();
    return path;
}

bool IsLeafName(const fs::path& name) {
    return !name.empty() && name != "." && name != ".." && !name.has_root_path() && name == name.filename();
}

std::error_code CheckDirectory(const fs::path& directory) {
    std::error_code ec;
    const fs::file_status status = fs::status(directory.empty() ? fs::path{"."} : directory, ec);
    if (ec) {
        return ec;
    }
    if (!fs::is_directory(status)) {
        return Error(std::errc::not_a_directory);
    }
    return {};
}

std::error_code CheckSource(const fs::path& source) {
    if (source.empty() || source.filename() == "." || source.filename() == ".." || !IsLeafName(source.filename())) {
        return Error(std::errc::invalid_argument);
    }

    std::error_code ec;
    const fs::path absolute = fs::absolute(source, ec);
    if (ec) {
        return ec;
    }
    if (absolute.lexically_normal().filename().empty()) {
        return Error(std::errc::invalid_argument);
    }

    const fs::file_status status = fs::symlink_status(source, ec);
    if (ec) {
        return ec;
    }
    if (!fs::is_regular_file(status) && !fs::is_directory(status) && !fs::is_symlink(status)) {
        return Error(std::errc::operation_not_supported);
    }
    return {};
}

bool IsWithin(const fs::path& candidate, const fs::path& directory) {
    auto part = candidate.begin();
    for (const auto& dirPart : directory) {
        if (part == candidate.end() || *part != dirPart) {
            return false;
        }
        ++part;
    }
    return true;
}

std::error_code CheckDestination(const fs::path& source, const fs::path& destination) {
    if (destination.empty() || !IsLeafName(destination.filename())) {
        return Error(std::errc::invalid_argument);
    }
    if (auto ec = CheckDirectory(destination.parent_path()); ec) {
        return ec;
    }

    std::error_code ec;
    const fs::file_status targetStatus = fs::symlink_status(destination, ec);
    if (!ec && fs::exists(targetStatus)) {
        return Error(std::errc::file_exists);
    }
    if (ec && ec != std::errc::no_such_file_or_directory) {
        return ec;
    }

    // A directory cannot be copied or moved into itself, including through a
    // symlink in the destination's parent path.
    const fs::file_status sourceStatus = fs::symlink_status(source, ec);
    if (ec) {
        return ec;
    }
    if (fs::is_directory(sourceStatus)) {
        const fs::path resolvedSource = fs::canonical(source, ec);
        if (ec) {
            return ec;
        }
        const fs::path resolvedParent =
            fs::canonical(destination.parent_path().empty() ? fs::path{"."} : destination.parent_path(), ec);
        if (ec) {
            return ec;
        }
        if (IsWithin(resolvedParent, resolvedSource)) {
            return Error(std::errc::invalid_argument);
        }
    }
    return {};
}

// The native operations fail when the target appears concurrently. Plain
// std::filesystem::rename can replace an existing file on POSIX systems.
std::error_code RenameNoReplace(const fs::path& source, const fs::path& destination) {
#if defined(_WIN32)
    if (MoveFileExW(source.c_str(), destination.c_str(), MOVEFILE_WRITE_THROUGH)) {
        return {};
    }
    return {static_cast<int>(GetLastError()), std::system_category()};
#elif defined(__APPLE__)
    if (renamex_np(source.c_str(), destination.c_str(), RENAME_EXCL) == 0) {
        return {};
    }
    return {errno, std::generic_category()};
#elif defined(__linux__)
    if (syscall(SYS_renameat2, AT_FDCWD, source.c_str(), AT_FDCWD, destination.c_str(), RENAME_NOREPLACE) == 0) {
        return {};
    }
    return {errno, std::generic_category()};
#else
    return Error(std::errc::operation_not_supported);
#endif
}

std::error_code MakeStage(const fs::path& parent, fs::path& stage) {
    static std::atomic<unsigned long long> sequence{0};
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    for (int attempt = 0; attempt < 100; ++attempt) {
        stage = parent / (".rottools-filemanager-" + std::to_string(now) + "-" + std::to_string(sequence.fetch_add(1)));
        std::error_code ec;
        if (fs::create_directory(stage, ec)) {
            return {};
        }
        if (ec && ec != std::errc::file_exists) {
            return ec;
        }
    }
    return Error(std::errc::file_exists);
}

void CleanStage(Result& result, const fs::path& stage) {
    std::error_code ec;
    fs::remove_all(stage, ec);
    if (ec) {
        result.leftover = stage;
        result.cleanupError = ec;
    }
}

Result CopyOrMove(const fs::path& source, const fs::path& destination, bool move) {
    Result result{source, destination};
    if (result.error = CheckSource(source); result.error) {
        return result;
    }
    if (result.error = CheckDestination(source, destination); result.error) {
        return result;
    }

    if (move) {
        // A same-volume, atomic rename keeps the original intact on failure.
        // A cross-volume move reports an error; callers can choose Copy then
        // Delete explicitly after reviewing the completed copy.
        result.error = RenameNoReplace(source, destination);
        return result;
    }

    fs::path stage;
    if (result.error = MakeStage(destination.parent_path(), stage); result.error) {
        return result;
    }
    const fs::path stagedItem = stage / "item";
    fs::copy(source, stagedItem, fs::copy_options::recursive | fs::copy_options::copy_symlinks, result.error);
    if (!result.error) {
        result.error = RenameNoReplace(stagedItem, destination);
    }
    CleanStage(result, stage);
    return result;
}

fs::path SourceIdentity(const fs::path& source) {
    const fs::path normalized = WithoutTrailingSeparator(source);
    std::error_code ec;
    const fs::path parent =
        fs::weakly_canonical(normalized.parent_path().empty() ? fs::path{"."} : normalized.parent_path(), ec);
    if (!ec) {
        return (parent / normalized.filename()).lexically_normal();
    }
    return fs::absolute(normalized, ec).lexically_normal();
}

std::vector<bool> Conflicts(const std::vector<fs::path>& sources, bool checkNames, bool checkOverlap) {
    std::vector<bool> conflicts(sources.size(), false);
    std::vector<fs::path> identities(sources.size());
    std::transform(sources.begin(), sources.end(), identities.begin(), SourceIdentity);

    for (std::size_t i = 0; i < sources.size(); ++i) {
        for (std::size_t j = i + 1; j < sources.size(); ++j) {
            if ((checkOverlap && (IsWithin(identities[i], identities[j]) || IsWithin(identities[j], identities[i]))) ||
                (checkNames &&
                 WithoutTrailingSeparator(sources[i]).filename() == WithoutTrailingSeparator(sources[j]).filename())) {
                conflicts[i] = conflicts[j] = true;
            }
        }
    }
    return conflicts;
}

std::vector<Result> TransferMany(const std::vector<fs::path>& sources, const fs::path& destinationDirectory,
                                 bool move) {
    std::vector<Result> results;
    results.reserve(sources.size());
    const std::vector<bool> conflicts = Conflicts(sources, true, move);
    for (std::size_t i = 0; i < sources.size(); ++i) {
        const fs::path source = WithoutTrailingSeparator(sources[i]);
        const fs::path destination = destinationDirectory / source.filename();
        if (conflicts[i]) {
            results.push_back({source, destination, Error(std::errc::invalid_argument)});
        } else {
            results.push_back(CopyOrMove(source, destination, move));
        }
    }
    return results;
}
}  // namespace

Result CreateNewDirectory(const fs::path& parent, const fs::path& name) {
    Result result{parent / name};
    if (!IsLeafName(name)) {
        result.error = Error(std::errc::invalid_argument);
    } else if (result.error = CheckDirectory(parent); !result.error) {
        std::error_code ec;
        if (!fs::create_directory(result.source, ec)) {
            result.error = ec ? ec : Error(std::errc::file_exists);
        }
    }
    return result;
}

Result CreateTextFile(const fs::path& parent, const fs::path& name) {
    Result result{parent / name};
    if (!IsLeafName(name)) {
        result.error = Error(std::errc::invalid_argument);
        return result;
    }
    if (result.error = CheckDirectory(parent); result.error) {
        return result;
    }
    std::error_code ec;
    if (fs::exists(fs::symlink_status(result.source, ec))) {
        result.error = Error(std::errc::file_exists);
        return result;
    }
    if (ec && ec != std::errc::no_such_file_or_directory) {
        result.error = ec;
        return result;
    }

    fs::path stage;
    if (result.error = MakeStage(parent, stage); result.error) {
        return result;
    }
    {
        std::ofstream file(stage / "item", std::ios::binary);
        if (!file) {
            result.error = Error(std::errc::io_error);
        }
        file.close();
        if (!result.error && !file) {
            result.error = Error(std::errc::io_error);
        }
    }
    if (!result.error) {
        result.error = RenameNoReplace(stage / "item", result.source);
    }
    CleanStage(result, stage);
    return result;
}

Result Rename(const fs::path& source, const fs::path& newName) {
    if (!IsLeafName(newName)) {
        return {source, {}, Error(std::errc::invalid_argument)};
    }
    const fs::path normalized = WithoutTrailingSeparator(source);
    return Move(normalized, normalized.parent_path() / newName);
}

Result Copy(const fs::path& source, const fs::path& destination) {
    return CopyOrMove(WithoutTrailingSeparator(source), destination, false);
}

Result Move(const fs::path& source, const fs::path& destination) {
    return CopyOrMove(WithoutTrailingSeparator(source), destination, true);
}

Result Delete(const fs::path& source) {
    Result result{WithoutTrailingSeparator(source)};
    if (result.error = CheckSource(result.source); result.error) {
        return result;
    }
    fs::remove_all(result.source, result.error);
    return result;
}

std::vector<Result> CopyMany(const std::vector<fs::path>& sources, const fs::path& destinationDirectory) {
    return TransferMany(sources, destinationDirectory, false);
}

std::vector<Result> MoveMany(const std::vector<fs::path>& sources, const fs::path& destinationDirectory) {
    return TransferMany(sources, destinationDirectory, true);
}

std::vector<Result> DeleteMany(const std::vector<fs::path>& sources) {
    std::vector<Result> results;
    results.reserve(sources.size());
    const std::vector<bool> conflicts = Conflicts(sources, false, true);
    for (std::size_t i = 0; i < sources.size(); ++i) {
        if (conflicts[i]) {
            results.push_back({sources[i], {}, Error(std::errc::invalid_argument)});
        } else {
            results.push_back(Delete(sources[i]));
        }
    }
    return results;
}
}  // namespace FileManagerBackend
