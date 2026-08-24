#include "AppConfigFunctions.h"

#include <wx/config.h>
#include <wx/string.h>

#include <algorithm>

namespace {
constexpr auto kEditorFontSetting = "/editor/font";
constexpr auto kBookmarksGroup = "/bookmarks";

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
    long storedCount = 0;
    config->Read(wxString(kBookmarksGroup) + "/count", &storedCount, 0L);

    std::vector<Bookmark> bookmarks;
    if (storedCount <= 0) {
        return bookmarks;
    }
    storedCount = std::min(storedCount, static_cast<long>(MaximumBookmarkCount));
    bookmarks.reserve(static_cast<std::size_t>(storedCount));

    for (long index = 0; index < storedCount; ++index) {
        wxString storedKind;
        wxString storedPath;
        if (!config->Read(BookmarkConfigKey(static_cast<std::size_t>(index), "kind"), &storedKind) ||
            !config->Read(BookmarkConfigKey(static_cast<std::size_t>(index), "path"), &storedPath) ||
            storedPath.IsEmpty()) {
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
    config->Write(wxString(kBookmarksGroup) + "/count", static_cast<long>(count));

    for (std::size_t index = 0; index < count; ++index) {
        const Bookmark& bookmark = bookmarks[index];
        config->Write(BookmarkConfigKey(index, "kind"),
                      bookmark.kind == BookmarkKind::Directory ? "directory" : "document");
        config->Write(BookmarkConfigKey(index, "path"), bookmark.path.GetFullPath());
    }
    config->Flush();
}

}  // namespace rottools
