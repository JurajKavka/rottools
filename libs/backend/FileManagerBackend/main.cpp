#include <chrono>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "FileManagerBackend.h"
#include "HelperFunctions.h"

namespace fs = std::filesystem;
namespace manager = FileManagerBackend;

void Expect(bool condition, const std::string& description) {
    if (!condition) {
        throw std::runtime_error(description);
    }
    printLog("OK: {}", description);
}

std::string ReadContents(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot read " + path.string());
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

int main() {
    std::error_code ec;
    const fs::path temp = fs::temp_directory_path(ec);
    if (ec) {
        printError("Cannot locate temporary directory: {}", ec.message());
        return 1;
    }

    fs::path root;
    const auto start = std::chrono::steady_clock::now().time_since_epoch().count();
    bool created = false;
    for (int attempt = 0; attempt < 100; ++attempt) {
        root = temp / ("rottools-filemanager-demo-" + std::to_string(start) + "-" + std::to_string(attempt));
        if (fs::create_directory(root, ec)) {
            created = true;
            break;
        }
        if (ec && ec != std::errc::file_exists) {
            printError("Cannot create demo directory: {}", ec.message());
            return 1;
        }
    }
    if (!created) {
        printError("Cannot reserve demo directory");
        return 1;
    }

    int exitCode = 0;
    try {
        const fs::path source = root / "source";
        const fs::path copies = root / "copies";
        const fs::path moved = root / "moved";
        Expect(manager::CreateNewDirectory(root, "source").Succeeded(), "create directory");
        Expect(manager::CreateNewDirectory(root, "copies").Succeeded(), "create destination");
        Expect(manager::CreateNewDirectory(root, "moved").Succeeded(), "create move destination");
        Expect(manager::CreateTextFile(source, "note.txt").Succeeded(), "create empty text file");
        {
            std::ofstream file(source / "note.txt", std::ios::binary);
            file << "hello";
            Expect(static_cast<bool>(file), "write demo contents");
        }

        Expect(!manager::CreateTextFile(source, "note.txt").Succeeded(), "existing file is never truncated");
        {
            std::ifstream file(source / "note.txt", std::ios::binary);
            std::string contents;
            file >> contents;
            Expect(contents == "hello", "name clash preserves file contents");
        }
        Expect(manager::Rename(source / "note.txt", "renamed.txt").Succeeded(), "rename file");
        Expect(manager::CreateNewDirectory(source, "nested").Succeeded(), "create nested directory");
        Expect(manager::Rename(source / "nested", "subdir").Succeeded(), "rename directory");

        const auto copiesResult = manager::CopyMany({source / "renamed.txt", source}, copies);
        Expect(copiesResult.size() == 2 && copiesResult[0].Succeeded() && copiesResult[1].Succeeded(),
               "bulk copy file and directory");
        Expect(fs::exists(source / "renamed.txt") && fs::exists(copies / "source" / "renamed.txt") &&
                   fs::is_directory(copies / "source" / "subdir"),
               "bulk copy keeps originals and nested content");
        Expect(!manager::Copy(source / "renamed.txt", copies / "renamed.txt").Succeeded(),
               "copy refuses an existing name");

        {
            std::ofstream file(source / "renamed.txt", std::ios::binary);
            file << "updated";
            Expect(static_cast<bool>(file), "change source before replacement");
        }
        Expect(!manager::Copy(source / "renamed.txt", copies / "renamed.txt").Succeeded() &&
                   ReadContents(copies / "renamed.txt") == "hello",
               "default copy leaves conflicting destination untouched");
        Expect(
            manager::Copy(source / "renamed.txt", copies / "renamed.txt", manager::ExistingFile::Replace).Succeeded() &&
                ReadContents(copies / "renamed.txt") == "updated" && ReadContents(source / "renamed.txt") == "updated",
            "explicit replacement copies new contents and keeps source");
        Expect(!manager::Copy(source / "missing.txt", copies / "renamed.txt", manager::ExistingFile::Replace)
                       .Succeeded() &&
                   ReadContents(copies / "renamed.txt") == "updated",
               "failed replacement preserves destination contents");
        Expect(!manager::Copy(source / "renamed.txt", source / "renamed.txt", manager::ExistingFile::Replace)
                       .Succeeded() &&
                   ReadContents(source / "renamed.txt") == "updated",
               "replacement refuses copying a file onto itself");
        Expect(!manager::Copy(source / "renamed.txt", copies / "source", manager::ExistingFile::Replace).Succeeded() &&
                   fs::exists(copies / "source" / "renamed.txt"),
               "replacement refuses to overwrite a directory");
        Expect(!manager::Copy(source, copies / "renamed.txt", manager::ExistingFile::Replace).Succeeded() &&
                   ReadContents(copies / "renamed.txt") == "updated",
               "replacement refuses to overwrite a file with a directory");

        fs::create_hard_link(source / "renamed.txt", copies / "hardlink.txt", ec);
        if (!ec) {
            Expect(!manager::Copy(source / "renamed.txt", copies / "hardlink.txt", manager::ExistingFile::Replace)
                           .Succeeded() &&
                       fs::equivalent(source / "renamed.txt", copies / "hardlink.txt") &&
                       ReadContents(source / "renamed.txt") == "updated",
                   "replacement refuses a hard link to the source");
        }
        fs::create_symlink(source / "renamed.txt", copies / "symlink.txt", ec);
        if (!ec) {
            Expect(!manager::Copy(source / "renamed.txt", copies / "symlink.txt", manager::ExistingFile::Replace)
                           .Succeeded() &&
                       fs::is_symlink(copies / "symlink.txt"),
                   "replacement preserves a destination symlink");
            Expect(!manager::Copy(copies / "symlink.txt", copies / "renamed.txt", manager::ExistingFile::Replace)
                           .Succeeded() &&
                       ReadContents(copies / "renamed.txt") == "updated",
                   "replacement refuses to overwrite a file with a symlink");
        }

        const auto movedResult = manager::MoveMany({copies / "renamed.txt", copies / "source"}, moved);
        Expect(movedResult.size() == 2 && movedResult[0].Succeeded() && movedResult[1].Succeeded(),
               "bulk move file and directory");
        Expect(!fs::exists(copies / "source") && fs::exists(moved / "source" / "renamed.txt"),
               "move transfers complete directory");

        const auto clash = manager::Move(source / "renamed.txt", moved / "renamed.txt");
        Expect(!clash.Succeeded() && fs::exists(source / "renamed.txt"), "failed move leaves original untouched");
        Expect(!manager::Copy(source, source / "child").Succeeded(), "copy into own directory is rejected");

        const fs::path moveSource = source / "move-replace.txt";
        const fs::path moveTarget = moved / "move-replace.txt";
        {
            std::ofstream incoming(moveSource, std::ios::binary);
            std::ofstream existing(moveTarget, std::ios::binary);
            incoming << "incoming";
            existing << "existing";
            Expect(static_cast<bool>(incoming) && static_cast<bool>(existing), "prepare conflicting move files");
        }
        Expect(!manager::Move(moveSource, moveTarget).Succeeded() && ReadContents(moveSource) == "incoming" &&
                   ReadContents(moveTarget) == "existing",
               "default move preserves both conflicting files");
        Expect(!manager::Move(moveSource, moveSource, manager::ExistingFile::Replace).Succeeded() &&
                   ReadContents(moveSource) == "incoming",
               "replacement refuses moving a file onto itself");
        Expect(!manager::Move(moveSource, moved / "source", manager::ExistingFile::Replace).Succeeded() &&
                   ReadContents(moveSource) == "incoming" && fs::exists(moved / "source" / "renamed.txt"),
               "move replacement preserves a conflicting directory and its source file");
        Expect(!manager::Move(source, moveTarget, manager::ExistingFile::Replace).Succeeded() &&
                   fs::is_directory(source) && ReadContents(moveTarget) == "existing",
               "move replacement refuses to overwrite a file with a directory");
        Expect(!manager::Move(source / "missing.txt", moveTarget, manager::ExistingFile::Replace).Succeeded() &&
                   ReadContents(moveTarget) == "existing",
               "failed move replacement preserves destination contents");
        fs::create_hard_link(moveSource, copies / "move-hardlink.txt", ec);
        if (!ec) {
            Expect(
                !manager::Move(moveSource, copies / "move-hardlink.txt", manager::ExistingFile::Replace).Succeeded() &&
                    fs::equivalent(moveSource, copies / "move-hardlink.txt"),
                "move replacement refuses a hard link to the source");
        }
        fs::create_symlink(moveSource, copies / "move-symlink.txt", ec);
        if (!ec) {
            Expect(
                !manager::Move(moveSource, copies / "move-symlink.txt", manager::ExistingFile::Replace).Succeeded() &&
                    fs::is_symlink(copies / "move-symlink.txt") && ReadContents(moveSource) == "incoming",
                "move replacement preserves a destination symlink and its source file");
            Expect(
                !manager::Move(copies / "move-symlink.txt", moveTarget, manager::ExistingFile::Replace).Succeeded() &&
                    fs::is_symlink(copies / "move-symlink.txt") && ReadContents(moveTarget) == "existing",
                "move replacement refuses to overwrite a file with a symlink");
        }
        Expect(manager::Move(moveSource, moveTarget, manager::ExistingFile::Replace).Succeeded() &&
                   !fs::exists(moveSource) && ReadContents(moveTarget) == "incoming",
               "explicit move replacement transfers the source and replaces destination contents");

        const auto overlap = manager::DeleteMany({moved / "source", moved / "source" / "renamed.txt"});
        Expect(overlap.size() == 2 && !overlap[0].Succeeded() && !overlap[1].Succeeded() &&
                   fs::exists(moved / "source" / "renamed.txt"),
               "overlapping bulk deletion is rejected");
        const auto deleted = manager::DeleteMany({moved / "source", moved / "renamed.txt"});
        Expect(deleted.size() == 2 && deleted[0].Succeeded() && deleted[1].Succeeded() && !fs::exists(moved / "source"),
               "bulk delete removes directory recursively and file");
    } catch (const std::exception& error) {
        printError("FileManagerBackend demo failed: {}", error.what());
        exitCode = 1;
    }

    fs::remove_all(root, ec);
    if (ec) {
        printError("Demo cleanup failed at {}: {}", root.string(), ec.message());
        exitCode = 1;
    }
    return exitCode;
}
