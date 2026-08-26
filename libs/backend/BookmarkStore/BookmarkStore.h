#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include "AppConfigFunctions.h"

/** Persistent bookmark state for directories and documents. */
class BookmarkStore final {
   public:
    using Kind = rottools::BookmarkKind;
    using Bookmark = rottools::Bookmark;
    using BookmarksChangedCallback = std::function<void(const std::vector<Bookmark>&)>;

    static constexpr std::size_t MaximumBookmarks = rottools::MaximumBookmarkCount;

    explicit BookmarkStore(BookmarksChangedCallback onBookmarksChanged);

    BookmarkStore(const BookmarkStore&) = delete;
    BookmarkStore& operator=(const BookmarkStore&) = delete;

    /** Load persisted bookmarks and publish the initialized state. */
    void Initialize();

    /** Reload persisted bookmarks and publish the refreshed state. */
    void Refresh();

    void AddOrRemoveBookmark(Kind kind, const wxFileName& path);
    void RemoveBookmark(const Bookmark& bookmark);

    [[nodiscard]] bool Contains(Kind kind, const wxFileName& path) const;
    [[nodiscard]] bool IsFull() const noexcept;

   private:
    std::vector<Bookmark> m_bookmarks;
    BookmarksChangedCallback m_onBookmarksChanged;

    void Load();
    void Save() const;
    void NotifyBookmarksChanged() const;
};
