#pragma once

#include <wx/filename.h>
#include <wx/font.h>

#include <cstddef>
#include <vector>

namespace rottools {

enum class BookmarkKind {
    Directory,
    Document,
};

struct Bookmark {
    BookmarkKind kind = BookmarkKind::Document;
    wxFileName path;
};

inline constexpr std::size_t MaximumBookmarkCount = 10;

/** Load the persisted editor font, or return fallback if none is valid. */
[[nodiscard]] wxFont LoadEditorFont(const wxFont& fallback);

/** Persist the editor font in the current application's settings store. */
void SaveEditorFont(const wxFont& font);

/** Load path bookmarks from the current application's settings store. */
[[nodiscard]] std::vector<Bookmark> LoadBookmarks();

/** Replace the path bookmarks in the current application's settings store. */
void SaveBookmarks(const std::vector<Bookmark>& bookmarks);

}  // namespace rottools
