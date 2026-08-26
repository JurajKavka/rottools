#include "BookmarkStore.h"

#include <algorithm>
#include <utility>

namespace {
wxFileName NormalizePath(BookmarkStore::Kind kind, const wxFileName& path) {
    wxFileName normalized = kind == BookmarkStore::Kind::Directory ? wxFileName::DirName(path.GetFullPath()) : path;
    normalized.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
    return normalized;
}

bool SamePath(const wxFileName& left, const wxFileName& right) {
    return left.IsOk() && right.IsOk() && left.SameAs(right);
}
}  // namespace

BookmarkStore::BookmarkStore(BookmarksChangedCallback onBookmarksChanged)
    : m_onBookmarksChanged(std::move(onBookmarksChanged)) {}

void BookmarkStore::Initialize() {
    Refresh();
}

void BookmarkStore::Refresh() {
    Load();
    NotifyBookmarksChanged();
}

void BookmarkStore::Load() {
    m_bookmarks.clear();

    for (Bookmark bookmark : rottools::LoadBookmarks()) {
        if (IsFull()) {
            break;
        }
        bookmark.path = NormalizePath(bookmark.kind, bookmark.path);
        if (bookmark.path.IsOk() && !Contains(bookmark.kind, bookmark.path)) {
            m_bookmarks.push_back(std::move(bookmark));
        }
    }
}

void BookmarkStore::Save() const {
    rottools::SaveBookmarks(m_bookmarks);
}

void BookmarkStore::NotifyBookmarksChanged() const {
    if (m_onBookmarksChanged) {
        m_onBookmarksChanged(m_bookmarks);
    }
}

void BookmarkStore::AddOrRemoveBookmark(Kind kind, const wxFileName& path) {
    if (!path.IsOk()) {
        return;
    }

    Load();
    const wxFileName normalized = NormalizePath(kind, path);
    const auto existing = std::ranges::find_if(m_bookmarks, [&](const Bookmark& bookmark) {
        return bookmark.kind == kind && SamePath(bookmark.path, normalized);
    });

    bool changed = false;
    if (existing != m_bookmarks.end()) {
        m_bookmarks.erase(existing);
        changed = true;
    } else if (!IsFull() && (kind == Kind::Directory ? normalized.DirExists() : normalized.FileExists())) {
        m_bookmarks.push_back({.kind = kind, .path = normalized});
        changed = true;
    }

    if (changed) {
        Save();
    }
    NotifyBookmarksChanged();
}

void BookmarkStore::RemoveBookmark(const Bookmark& bookmark) {
    Load();
    const wxFileName normalized = NormalizePath(bookmark.kind, bookmark.path);
    const auto existing = std::ranges::find_if(m_bookmarks, [&](const Bookmark& candidate) {
        return candidate.kind == bookmark.kind && SamePath(candidate.path, normalized);
    });
    if (existing != m_bookmarks.end()) {
        m_bookmarks.erase(existing);
        Save();
    }
    NotifyBookmarksChanged();
}

bool BookmarkStore::Contains(Kind kind, const wxFileName& path) const {
    if (!path.IsOk()) {
        return false;
    }

    const wxFileName normalized = NormalizePath(kind, path);
    return std::ranges::any_of(m_bookmarks, [&](const Bookmark& bookmark) {
        return bookmark.kind == kind && SamePath(bookmark.path, normalized);
    });
}

bool BookmarkStore::IsFull() const noexcept {
    return m_bookmarks.size() >= MaximumBookmarks;
}
