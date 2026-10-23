#include "AppConfigFunctions.h"

#include <wx/config.h>
#include <wx/string.h>

#include <algorithm>

namespace {
constexpr auto kEditorFontSetting = "/editor/font";
constexpr auto kBookmarksGroup = "/bookmarks";

class ScopedConfigPath final {
   public:
    ScopedConfigPath(wxConfigBase* config, const wxString& path) : m_config(config), m_originalPath(config->GetPath()) {
        m_config->SetPath(path);
    }

    ~ScopedConfigPath() {
        m_config->SetPath(m_originalPath);
    }

    ScopedConfigPath(const ScopedConfigPath&) = delete;
    ScopedConfigPath& operator=(const ScopedConfigPath&) = delete;

   private:
    wxConfigBase* m_config;
    wxString m_originalPath;
};

wxString BookmarkConfigKey(std::size_t index, const wxString& field) {
    return wxString(kBookmarksGroup) + "/" + wxString::Format("%llu", static_cast<unsigned long long>(index)) + "/" +
           field;
}
}  // namespace

namespace rottools {

wxFont LoadEditorFont(const wxFont& fallback) {
    wxString nativeFontInfo;
    if (!wxConfigBase::Get()->Read(kEditorFontSetting, &nativeFontInfo)) {
        return fallback;
    }

    wxFont font;
    if (!font.SetNativeFontInfo(nativeFontInfo) || !font.IsOk()) {
        return fallback;
    }

    return font;
}

void SaveEditorFont(const wxFont& font) {
    wxConfigBase* config = wxConfigBase::Get();
    if (config->Write(kEditorFontSetting, font.GetNativeFontInfoDesc())) {
        config->Flush();
    }
}

std::vector<Bookmark> LoadBookmarks() {
    wxConfigBase* config = wxConfigBase::Get();
    std::vector<Bookmark> bookmarks;
    if (!config->HasGroup(kBookmarksGroup)) {
        return bookmarks;
    }

    std::vector<std::size_t> indices;
    {
        ScopedConfigPath bookmarksPath(config, kBookmarksGroup);
        wxString groupName;
        long cookie = 0;
        for (bool found = config->GetFirstGroup(groupName, cookie); found;
             found = config->GetNextGroup(groupName, cookie)) {
            wxULongLong_t index = 0;
            if (groupName.ToULongLong(&index) && index < static_cast<wxULongLong_t>(MaximumBookmarkCount)) {
                const auto bookmarkIndex = static_cast<std::size_t>(index);
                const wxString canonicalName = wxString::Format("%llu", static_cast<unsigned long long>(bookmarkIndex));
                if (groupName == canonicalName) {
                    indices.push_back(bookmarkIndex);
                }
            }
        }
    }

    std::ranges::sort(indices);
    bookmarks.reserve(indices.size());

    for (const std::size_t index : indices) {
        wxString storedKind;
        wxString storedPath;
        if (!config->Read(BookmarkConfigKey(index, "kind"), &storedKind) ||
            !config->Read(BookmarkConfigKey(index, "path"), &storedPath) || storedPath.IsEmpty()) {
            continue;
        }

        BookmarkKind kind;
        if (storedKind == "directory") {
            kind = BookmarkKind::Directory;
        } else if (storedKind == "document") {
            kind = BookmarkKind::Document;
        } else {
            continue;
        }
        bookmarks.push_back({.kind = kind, .path = wxFileName(storedPath)});
    }

    return bookmarks;
}

void SaveBookmarks(const std::vector<Bookmark>& bookmarks) {
    wxConfigBase* config = wxConfigBase::Get();
    const std::size_t count = std::min(bookmarks.size(), MaximumBookmarkCount);
    config->DeleteGroup(kBookmarksGroup);

    for (std::size_t index = 0; index < count; ++index) {
        const Bookmark& bookmark = bookmarks[index];
        config->Write(BookmarkConfigKey(index, "kind"),
                      bookmark.kind == BookmarkKind::Directory ? "directory" : "document");
        config->Write(BookmarkConfigKey(index, "path"), bookmark.path.GetFullPath());
    }
    config->Flush();
}

}  // namespace rottools
