#include "BookmarkStore.h"

#include <wx/artprov.h>
#include <wx/intl.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/stockitem.h>
#include <wx/window.h>

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

wxString DirectoryLeaf(const wxFileName& directory) {
    const wxArrayString& directories = directory.GetDirs();
    return directories.IsEmpty() ? directory.GetFullPath() : directories.Last();
}

wxString BaseLabel(const BookmarkStore::Bookmark& bookmark) {
    return bookmark.kind == BookmarkStore::Kind::Directory ? DirectoryLeaf(bookmark.path) : bookmark.path.GetFullName();
}

wxString ParentLabel(const BookmarkStore::Bookmark& bookmark) {
    wxFileName parent = bookmark.kind == BookmarkStore::Kind::Directory
                            ? wxFileName::DirName(bookmark.path.GetFullPath())
                            : wxFileName::DirName(bookmark.path.GetPath());
    if (bookmark.kind == BookmarkStore::Kind::Directory) {
        parent.RemoveLastDir();
    }
    return DirectoryLeaf(parent);
}
}  // namespace

BookmarkStore::BookmarkStore(wxWindow* owner, wxMenu* menu, wxMenuItem* directoryCommand, wxMenuItem* documentCommand,
                             Callbacks callbacks)
    : m_owner(owner),
      m_menu(menu),
      m_directoryCommand(directoryCommand),
      m_documentCommand(documentCommand),
      m_callbacks(std::move(callbacks)),
      m_bookmarkMenuBaseId(wxWindow::NewControlId(static_cast<int>(MaximumBookmarks))) {
    wxASSERT(m_owner != nullptr);
    wxASSERT(m_menu != nullptr);
    wxASSERT(m_directoryCommand != nullptr);
    wxASSERT(m_documentCommand != nullptr);

    m_owner->Bind(wxEVT_MENU_OPEN, &BookmarkStore::HandleMenuOpen, this);
    m_owner->Bind(wxEVT_MENU, &BookmarkStore::HandleToggleDirectory, this, m_directoryCommand->GetId());
    m_owner->Bind(wxEVT_MENU, &BookmarkStore::HandleToggleDocument, this, m_documentCommand->GetId());
    m_owner->Bind(wxEVT_MENU, &BookmarkStore::HandleOpenBookmark, this, m_bookmarkMenuBaseId,
                  m_bookmarkMenuBaseId + static_cast<int>(MaximumBookmarks) - 1);

    Refresh();
}

BookmarkStore::~BookmarkStore() {
    m_owner->Unbind(wxEVT_MENU_OPEN, &BookmarkStore::HandleMenuOpen, this);
    m_owner->Unbind(wxEVT_MENU, &BookmarkStore::HandleToggleDirectory, this, m_directoryCommand->GetId());
    m_owner->Unbind(wxEVT_MENU, &BookmarkStore::HandleToggleDocument, this, m_documentCommand->GetId());
    m_owner->Unbind(wxEVT_MENU, &BookmarkStore::HandleOpenBookmark, this, m_bookmarkMenuBaseId,
                    m_bookmarkMenuBaseId + static_cast<int>(MaximumBookmarks) - 1);
}

void BookmarkStore::Refresh() {
    Load();
    RebuildMenu();
}

void BookmarkStore::RefreshCurrentPaths() {
    const wxFileName currentDirectory = m_callbacks.currentDirectory ? m_callbacks.currentDirectory() : wxFileName{};
    const wxFileName currentDocument = m_callbacks.currentDocument ? m_callbacks.currentDocument() : wxFileName{};
    UpdateCommand(m_directoryCommand, Kind::Directory, currentDirectory);
    UpdateCommand(m_documentCommand, Kind::Document, currentDocument);
}

void BookmarkStore::Load() {
    m_bookmarks.clear();

    for (Bookmark bookmark : rottools::LoadBookmarks()) {
        if (m_bookmarks.size() == MaximumBookmarks) {
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

void BookmarkStore::RebuildMenu() {
    for (wxMenuItem* item : m_dynamicMenuItems) {
        m_menu->Destroy(item);
    }
    m_dynamicMenuItems.clear();
    m_visibleBookmarks.clear();

    RefreshCurrentPaths();

    if (m_bookmarks.empty()) {
        return;
    }

    m_dynamicMenuItems.push_back(m_menu->AppendSeparator());
    auto appendKind = [this](Kind kind) {
        for (const Bookmark& bookmark : m_bookmarks) {
            if (bookmark.kind != kind) {
                continue;
            }

            const int id = m_bookmarkMenuBaseId + static_cast<int>(m_visibleBookmarks.size());
            wxMenuItem* item = m_menu->Append(id, MakeLabel(bookmark), bookmark.path.GetFullPath());
            const wxArtID artId = kind == Kind::Directory ? wxART_FOLDER : wxART_NORMAL_FILE;
            item->SetBitmap(wxArtProvider::GetBitmap(artId, wxART_MENU));
            m_dynamicMenuItems.push_back(item);
            m_visibleBookmarks.push_back(bookmark);
        }
    };

    const bool hasDirectories =
        std::ranges::any_of(m_bookmarks, [](const Bookmark& bookmark) { return bookmark.kind == Kind::Directory; });
    const bool hasDocuments =
        std::ranges::any_of(m_bookmarks, [](const Bookmark& bookmark) { return bookmark.kind == Kind::Document; });

    appendKind(Kind::Directory);
    if (hasDirectories && hasDocuments) {
        m_dynamicMenuItems.push_back(m_menu->AppendSeparator());
    }
    appendKind(Kind::Document);
}

void BookmarkStore::UpdateCommand(wxMenuItem* command, Kind kind, const wxFileName& currentPath) {
    const bool validPath = currentPath.IsOk();
    const bool bookmarked = validPath && Contains(kind, currentPath);
    const bool targetExists =
        validPath && (kind == Kind::Directory ? currentPath.DirExists() : currentPath.FileExists());
    command->Enable(bookmarked || (targetExists && m_bookmarks.size() < MaximumBookmarks));

    if (kind == Kind::Directory) {
        command->SetItemLabel(bookmarked ? _("Remove Current Directory Bookmark\tCtrl+Shift+D")
                                         : _("Bookmark Current Directory\tCtrl+Shift+D"));
    } else {
        command->SetItemLabel(bookmarked ? _("Remove Current Document Bookmark\tCtrl+D")
                                         : _("Bookmark Current Document\tCtrl+D"));
    }
}

void BookmarkStore::Toggle(Kind kind, const wxFileName& path) {
    if (!path.IsOk()) {
        return;
    }

    Load();
    const wxFileName normalized = NormalizePath(kind, path);
    const auto existing = std::ranges::find_if(m_bookmarks, [&](const Bookmark& bookmark) {
        return bookmark.kind == kind && SamePath(bookmark.path, normalized);
    });
    if (existing != m_bookmarks.end()) {
        m_bookmarks.erase(existing);
    } else if (m_bookmarks.size() < MaximumBookmarks &&
               (kind == Kind::Directory ? normalized.DirExists() : normalized.FileExists())) {
        m_bookmarks.push_back({.kind = kind, .path = normalized});
    }

    Save();
    RebuildMenu();
}

void BookmarkStore::Remove(const Bookmark& bookmark) {
    Load();
    const auto existing = std::ranges::find_if(m_bookmarks, [&](const Bookmark& candidate) {
        return candidate.kind == bookmark.kind && SamePath(candidate.path, bookmark.path);
    });
    if (existing == m_bookmarks.end()) {
        return;
    }

    m_bookmarks.erase(existing);
    Save();
    RebuildMenu();
}

bool BookmarkStore::Contains(Kind kind, const wxFileName& path) const {
    return std::ranges::any_of(
        m_bookmarks, [&](const Bookmark& bookmark) { return bookmark.kind == kind && SamePath(bookmark.path, path); });
}

wxString BookmarkStore::MakeLabel(const Bookmark& bookmark) const {
    const wxString base = BaseLabel(bookmark);
    const auto hasSameBase = [&](const Bookmark& candidate) {
        return candidate.kind == bookmark.kind && !SamePath(candidate.path, bookmark.path) &&
               BaseLabel(candidate) == base;
    };
    if (!std::ranges::any_of(m_bookmarks, hasSameBase)) {
        return base;
    }

    const wxString withParent = base + " — " + ParentLabel(bookmark);
    const auto hasSameParentLabel = [&](const Bookmark& candidate) {
        return hasSameBase(candidate) && BaseLabel(candidate) + " — " + ParentLabel(candidate) == withParent;
    };
    return std::ranges::any_of(m_bookmarks, hasSameParentLabel) ? bookmark.path.GetFullPath() : withParent;
}

void BookmarkStore::HandleMenuOpen(wxMenuEvent& event) {
    if (event.GetMenu() == m_menu) {
        Refresh();
    }
    event.Skip();
}

void BookmarkStore::HandleToggleDirectory(wxCommandEvent& event) {
    if (m_callbacks.currentDirectory) {
        Toggle(Kind::Directory, m_callbacks.currentDirectory());
    }
}

void BookmarkStore::HandleToggleDocument(wxCommandEvent& event) {
    if (m_callbacks.currentDocument) {
        Toggle(Kind::Document, m_callbacks.currentDocument());
    }
}

void BookmarkStore::HandleOpenBookmark(wxCommandEvent& event) {
    const std::size_t index = static_cast<std::size_t>(event.GetId() - m_bookmarkMenuBaseId);
    if (index >= m_visibleBookmarks.size()) {
        return;
    }

    const Bookmark bookmark = m_visibleBookmarks[index];
    const bool exists = bookmark.kind == Kind::Directory ? bookmark.path.DirExists() : bookmark.path.FileExists();
    if (!exists) {
        const wxString target = bookmark.kind == Kind::Directory ? _("directory") : _("document");
        wxMessageDialog dialog(
            m_owner,
            wxString::Format(_("This bookmarked %s no longer exists.\n\nRemove the bookmark?"), target.c_str()),
            _("Missing Bookmark"), wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_WARNING);
        dialog.SetOKCancelLabels(_("Remove Bookmark"), wxGetStockLabel(wxID_CANCEL));
        if (dialog.ShowModal() == wxID_OK) {
            Remove(bookmark);
        }
        return;
    }

    if (bookmark.kind == Kind::Directory) {
        if (m_callbacks.openDirectory) {
            m_callbacks.openDirectory(bookmark.path);
        }
    } else if (m_callbacks.openDocument) {
        m_callbacks.openDocument(bookmark.path);
    }
}
