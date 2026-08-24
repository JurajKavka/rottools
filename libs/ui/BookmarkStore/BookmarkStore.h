#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include "AppConfigFunctions.h"

class wxCommandEvent;
class wxMenu;
class wxMenuEvent;
class wxMenuItem;
class wxWindow;

/**
 * Persistent, native-menu bookmarks for directories and documents.
 *
 * The owner supplies the application-specific meaning of "current" and
 * "open". BookmarkStore owns the flat menu presentation, stale-path prompts,
 * and persistence in the current application's wxConfig store.
 */
class BookmarkStore final {
   public:
    using Kind = rottools::BookmarkKind;
    using Bookmark = rottools::Bookmark;

    using CurrentPathCallback = std::function<wxFileName()>;
    using OpenPathCallback = std::function<void(const wxFileName&)>;

    struct Callbacks {
        CurrentPathCallback currentDirectory;
        CurrentPathCallback currentDocument;
        OpenPathCallback openDirectory;
        OpenPathCallback openDocument;
    };

    static constexpr std::size_t MaximumBookmarks = rottools::MaximumBookmarkCount;

    BookmarkStore(wxWindow* owner, wxMenu* menu, wxMenuItem* directoryCommand, wxMenuItem* documentCommand,
                  Callbacks callbacks);
    ~BookmarkStore();

    BookmarkStore(const BookmarkStore&) = delete;
    BookmarkStore& operator=(const BookmarkStore&) = delete;

    /** Reload persisted bookmarks and rebuild the commands and dynamic entries. */
    void Refresh();

    /** Update the two current-path commands without rebuilding bookmark entries. */
    void RefreshCurrentPaths();

   private:
    wxWindow* m_owner = nullptr;
    wxMenu* m_menu = nullptr;
    wxMenuItem* m_directoryCommand = nullptr;
    wxMenuItem* m_documentCommand = nullptr;
    Callbacks m_callbacks;
    std::vector<Bookmark> m_bookmarks;
    std::vector<Bookmark> m_visibleBookmarks;
    std::vector<wxMenuItem*> m_dynamicMenuItems;
    int m_bookmarkMenuBaseId = 0;

    void Load();
    void Save() const;
    void RebuildMenu();
    void UpdateCommand(wxMenuItem* command, Kind kind, const wxFileName& currentPath);
    void Toggle(Kind kind, const wxFileName& path);
    void Remove(const Bookmark& bookmark);
    [[nodiscard]] bool Contains(Kind kind, const wxFileName& path) const;
    [[nodiscard]] wxString MakeLabel(const Bookmark& bookmark) const;

    void HandleMenuOpen(wxMenuEvent& event);
    void HandleToggleDirectory(wxCommandEvent& event);
    void HandleToggleDocument(wxCommandEvent& event);
    void HandleOpenBookmark(wxCommandEvent& event);
};
