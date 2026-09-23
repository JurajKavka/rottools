#include "FileBrowserTreePanel.h"

#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/control.h>
#include <wx/dataobj.h>
#include <wx/dcbuffer.h>
#include <wx/imaglist.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/toolbar.h>

#include <algorithm>
#include <cstddef>
#include <utility>

namespace {
wxColour BlendColours(const wxColour& background, const wxColour& foreground, int foregroundAlpha) {
    const int backgroundAlpha = 255 - foregroundAlpha;
    return wxColour((background.Red() * backgroundAlpha + foreground.Red() * foregroundAlpha) / 255,
                    (background.Green() * backgroundAlpha + foreground.Green() * foregroundAlpha) / 255,
                    (background.Blue() * backgroundAlpha + foreground.Blue() * foregroundAlpha) / 255);
}

class FlatCloseButton final : public wxControl {
   public:
    explicit FlatCloseButton(wxWindow* parent)
        : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxWANTS_CHARS) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetLabel(_("Close File Browser"));
        Bind(wxEVT_PAINT, &FlatCloseButton::HandlePaint, this);
        Bind(wxEVT_ENTER_WINDOW, &FlatCloseButton::HandleEnter, this);
        Bind(wxEVT_LEAVE_WINDOW, &FlatCloseButton::HandleLeave, this);
        Bind(wxEVT_LEFT_DOWN, &FlatCloseButton::HandleLeftDown, this);
        Bind(wxEVT_LEFT_UP, &FlatCloseButton::HandleLeftUp, this);
        Bind(wxEVT_MOUSE_CAPTURE_LOST, &FlatCloseButton::HandleCaptureLost, this);
        Bind(wxEVT_KEY_DOWN, &FlatCloseButton::HandleKeyDown, this);
        Bind(wxEVT_SET_FOCUS, &FlatCloseButton::HandleFocusChanged, this);
        Bind(wxEVT_KILL_FOCUS, &FlatCloseButton::HandleFocusChanged, this);
        Bind(wxEVT_SYS_COLOUR_CHANGED, &FlatCloseButton::HandleSystemColourChanged, this);
    }

   protected:
    wxSize DoGetBestSize() const override {
        return FromDIP(wxSize(24, 24));
    }

   private:
    bool m_hovered = false;
    bool m_pressed = false;

    void Activate() {
        wxCommandEvent click(wxEVT_BUTTON, GetId());
        click.SetEventObject(this);
        ProcessWindowEvent(click);
    }

    void HandlePaint(wxPaintEvent& event) {
        wxAutoBufferedPaintDC dc(this);
        const wxColour background = GetParent()->GetBackgroundColour();
        const wxColour textColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
        const wxColour foreground = m_hovered ? textColour : BlendColours(background, textColour, 180);
        dc.SetBackground(wxBrush(background));
        dc.Clear();

        wxRect buttonRect = GetClientRect();
        buttonRect.Deflate(FromDIP(2));
        dc.SetPen(wxPen(foreground, FromDIP(1)));
        const wxSize size = GetClientSize();
        const wxPoint center(size.x / 2, size.y / 2);
        const int arm = FromDIP(4);
        dc.DrawLine(center.x - arm, center.y - arm, center.x + arm, center.y + arm);
        dc.DrawLine(center.x - arm, center.y + arm, center.x + arm, center.y - arm);

        if (HasFocus()) {
            dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT), FromDIP(1)));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRoundedRectangle(buttonRect, FromDIP(4));
        }
    }

    void HandleEnter(wxMouseEvent& event) {
        m_hovered = true;
        Refresh();
    }

    void HandleLeave(wxMouseEvent& event) {
        m_hovered = false;
        Refresh();
    }

    void HandleLeftDown(wxMouseEvent& event) {
        SetFocus();
        CaptureMouse();
        m_pressed = true;
        m_hovered = true;
        Refresh();
    }

    void HandleLeftUp(wxMouseEvent& event) {
        const bool activate = m_pressed && GetClientRect().Contains(event.GetPosition());
        m_pressed = false;
        if (HasCapture()) {
            ReleaseMouse();
        }
        Refresh();
        if (activate) {
            Activate();
        }
    }

    void HandleCaptureLost(wxMouseCaptureLostEvent& event) {
        m_pressed = false;
        Refresh();
    }

    void HandleKeyDown(wxKeyEvent& event) {
        const int key = event.GetKeyCode();
        if (key == WXK_SPACE || key == WXK_RETURN || key == WXK_NUMPAD_ENTER) {
            Activate();
        } else {
            event.Skip();
        }
    }

    void HandleFocusChanged(wxFocusEvent& event) {
        Refresh();
        event.Skip();
    }

    void HandleSystemColourChanged(wxSysColourChangedEvent& event) {
        Refresh();
        event.Skip();
    }
};
}  // namespace

FileBrowserTreePanel::FileBrowserTreePanel(wxWindow* parent, Callbacks callbacks,
                                           std::vector<FileTypeFilter> fileTypeFilter)
    : FileBrowserTreePanelWx(parent),
      m_fileTypeFilters(std::move(fileTypeFilter)),
      m_onFileOpened(std::move(callbacks.onFileOpened)),
      m_onDirectoryChanged(std::move(callbacks.onDirectoryChanged)),
      m_onHomeRequested(std::move(callbacks.onHomeRequested)),
      m_onCloseRequested(std::move(callbacks.onCloseRequested)) {
    // The checked-in generated base still contains the old toolbar. Hide it
    // until wxFormBuilder regenerates the base from the updated project.
    wxSizerItem* firstItem = GetSizer()->GetItem(static_cast<std::size_t>(0));
    if (auto* oldToolbar = dynamic_cast<wxToolBar*>(firstItem ? firstItem->GetWindow() : nullptr)) {
        oldToolbar->Hide();
    }

    wxPanel* header = new wxPanel(this);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* title = new wxStaticText(header, wxID_ANY, _("File Browser"));
    FlatCloseButton* closeButton = new FlatCloseButton(header);
    headerSizer->Add(title, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(6));
    headerSizer->Add(closeButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(4));
    header->SetSizer(headerSizer);
    GetSizer()->Insert(0, header, 0, wxEXPAND);
    header->Bind(wxEVT_CONTEXT_MENU, &FileBrowserTreePanel::HandleHeaderContextMenu, this);
    title->Bind(wxEVT_CONTEXT_MENU, &FileBrowserTreePanel::HandleHeaderContextMenu, this);
    closeButton->Bind(wxEVT_BUTTON, &FileBrowserTreePanel::HandleCloseButtonClick, this);
    Layout();

    Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::HandleDirectoryScanComplete, this);
    m_hiddenFilesCheckbox->Bind(wxEVT_CHECKBOX, &FileBrowserTreePanel::HandleHiddenFilesCheckbox, this);
    m_fileTypeChoice->Bind(wxEVT_CHOICE, &FileBrowserTreePanel::HandleFileTypeChoice, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &FileBrowserTreePanel::HandleItemActivated, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &FileBrowserTreePanel::HandleItemContextMenu, this);

    if (m_fileTypeFilters.empty()) {
        // No file type choices means no extension filter and no dropdown.
        m_fileTypeChoice->Hide();
        Layout();
    } else {
        // Keep the supplied order and use its first choice for the initial scan.
        for (const FileTypeFilter& filter : m_fileTypeFilters) {
            m_fileTypeChoice->Append(filter.label);
        }
        m_fileTypeChoice->SetSelection(0);
        ApplySelectedFileType();
    }
    m_scanOptions.showHiddenFiles = m_hiddenFilesCheckbox->IsChecked();

    // 2. Create an Image List (16x16 is the standard size for tree nodes)
    // The 'true' parameter means it supports transparency (alpha channels)
    wxImageList* imageList = new wxImageList(16, 16, true);

    // 3. Fetch native OS icons and add them to the list
    // Index 0: The Folder Icon
    imageList->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16)));

    // Index 1: The File Icon
    imageList->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)));

    // 4. Assign the image list to your tree control
    // 'AssignImageList' tells the tree to take ownership, so you don't need to 'delete' it later
    m_dataViewTreeCtrl1->AssignImageList(imageList);
}

FileBrowserTreePanel::~FileBrowserTreePanel() {
    Unbind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::HandleDirectoryScanComplete, this);
}

wxDataViewItem FileBrowserTreePanel::FindChildByText(const wxString& text) const {
    wxDataViewItem root;
    int count = m_dataViewTreeCtrl1->GetChildCount(root);
    for (int i = 0; i < count; ++i) {
        wxDataViewItem child = m_dataViewTreeCtrl1->GetNthChild(root, i);
        if (child.IsOk() && m_dataViewTreeCtrl1->GetItemText(child) == text) {
            return child;
        }
    }
    return wxDataViewItem();
}

void FileBrowserTreePanel::UpdateTree(const std::vector<FileEntry>& entries) {
    auto sortedData = DirectoryScanner::SortEntries(entries);

    m_dataViewTreeCtrl1->DeleteAllItems();

    wxDataViewItem root;

    m_dataViewTreeCtrl1->AppendItem(root, "..", 0);

    for (const auto& entry : sortedData) {
        m_dataViewTreeCtrl1->AppendItem(root, entry.name, entry.isDirectory ? 0 : 1);
    }

    if (!m_savedSelectionText.IsEmpty()) {
        wxDataViewItem selection = FindChildByText(m_savedSelectionText);
        if (selection.IsOk()) {
            m_dataViewTreeCtrl1->Select(selection);
            // Pulling the selection into view would defeat KeepPosition: the
            // user may have scrolled away from it deliberately.
            if (m_scrollBehavior == ScrollBehavior::ResetToTop) {
                m_dataViewTreeCtrl1->EnsureVisible(selection);
            }
        }
    }

    if (m_scrollBehavior == ScrollBehavior::KeepPosition && !m_savedTopItemText.IsEmpty()) {
        wxDataViewItem topItem = FindChildByText(m_savedTopItemText);
        if (topItem.IsOk()) {
            // EnsureVisible alone would leave the row at the bottom edge (the
            // rebuilt view starts at the top). Scrolling to the end first makes
            // the second call approach from below, which puts the row back at
            // the top edge. Both run before the next paint, so only the final
            // position is ever drawn.
            int count = m_dataViewTreeCtrl1->GetChildCount(root);
            if (count > 0) {
                m_dataViewTreeCtrl1->EnsureVisible(m_dataViewTreeCtrl1->GetNthChild(root, count - 1));
            }
            m_dataViewTreeCtrl1->EnsureVisible(topItem);
        }
    }
}

void FileBrowserTreePanel::ListDir(const wxFileName& fileName, ScrollBehavior scrollBehavior) {
    m_scrollBehavior = scrollBehavior;

    wxDataViewItem currentSelection = m_dataViewTreeCtrl1->GetSelection();
    if (currentSelection.IsOk()) {
        m_savedSelectionText = m_dataViewTreeCtrl1->GetItemText(currentSelection);
    } else {
        m_savedSelectionText.clear();
    }

    m_savedTopItemText.clear();
    if (scrollBehavior == ScrollBehavior::KeepPosition) {
        wxDataViewItem topItem = m_dataViewTreeCtrl1->GetTopItem();
        if (topItem.IsOk()) {
            m_savedTopItemText = m_dataViewTreeCtrl1->GetItemText(topItem);
        }
    }

    // Force the path to interpret its entire string structure as a directory.
    // It is probably not needed when working with `wxFileName` API ...
    m_currentPath = wxFileName::DirName(fileName.GetFullPath());

    m_directoryScanner.StartScan(fileName, m_scanOptions, this);
}

/**
 * Lists a file's containing directory and selects that file after the scan.
 *
 * Use this when another part of the application opens or saves a document so
 * the browser follows the active file even when it lives in another directory.
 */
void FileBrowserTreePanel::ShowFile(const wxFileName& fileName) {
    wxFileName absoluteFile(fileName);
    absoluteFile.MakeAbsolute();

    m_scrollBehavior = ScrollBehavior::ResetToTop;
    m_savedSelectionText = absoluteFile.GetFullName();
    m_savedTopItemText.clear();
    m_currentPath = wxFileName::DirName(absoluteFile.GetPath());
    m_directoryScanner.StartScan(m_currentPath, m_scanOptions, this);
}

wxFileName FileBrowserTreePanel::GetCurrentDirectory() const {
    return m_currentPath;
}

void FileBrowserTreePanel::HandleDirectoryScanComplete(DirectoryScannerEvent& event) {
    UpdateTree(event.files);
    if (m_onDirectoryChanged) {
        m_onDirectoryChanged(event.currentDirectory);
    }
}

void FileBrowserTreePanel::HandleHiddenFilesCheckbox(wxCommandEvent& event) {
    // 1. Update the configuration state with the checkbox value
    m_scanOptions.showHiddenFiles = event.IsChecked();

    // 2. If a valid directory is currently being shown, re-scan it immediately
    // with the updated configuration layout
    if (m_currentPath.IsOk()) {
        ListDir(m_currentPath);
    }
}

void FileBrowserTreePanel::ApplySelectedFileType() {
    m_scanOptions.extensions.clear();

    const int selection = m_fileTypeChoice->GetSelection();
    if (selection == wxNOT_FOUND) {
        return;
    }

    const auto& extensions = m_fileTypeFilters[static_cast<std::size_t>(selection)].extensions;
    if (std::ranges::find(extensions, kFilterAllFiles) != extensions.end()) {
        return;
    }

    for (const std::string& extension : extensions) {
        if (!extension.empty()) {
            // DirectoryScanner compares against path::extension(), which includes the dot.
            m_scanOptions.extensions.push_back(extension.front() == '.' ? extension : "." + extension);
        }
    }
}

void FileBrowserTreePanel::HandleFileTypeChoice(wxCommandEvent& event) {
    ApplySelectedFileType();
    if (m_currentPath.IsOk()) {
        ListDir(m_currentPath);
    }
}

void FileBrowserTreePanel::HandleItemActivated(wxDataViewEvent& event) {
    const wxFileName path = ResolveItemPath(event.GetItem());
    if (!path.IsOk()) {
        return;
    }

    OpenPath(path);
}

wxFileName FileBrowserTreePanel::ResolveItemPath(const wxDataViewItem& item) const {
    if (!item.IsOk() || !m_currentPath.IsOk()) {
        return {};
    }

    const wxString itemText = m_dataViewTreeCtrl1->GetItemText(item);
    if (itemText == "..") {
        wxFileName parentPath = m_currentPath;
        if (parentPath.GetDirCount() > 0) {
            parentPath.RemoveLastDir();
        }
        return parentPath;
    }

    wxFileName directoryPath = m_currentPath;
    directoryPath.AppendDir(itemText);
    if (directoryPath.DirExists()) {
        return directoryPath;
    }

    wxFileName filePath = m_currentPath;
    filePath.SetFullName(itemText);
    if (filePath.FileExists()) {
        return filePath;
    }

    return {};
}

void FileBrowserTreePanel::OpenPath(const wxFileName& path) {
    // DirExists() checks the directory portion of a wxFileName, which also
    // exists for an ordinary file. Test the complete file path first.
    if (path.FileExists()) {
        if (m_onFileOpened) {
            m_onFileOpened(path);
        }
    } else if (path.DirExists()) {
        ListDir(path);
    }
}

void FileBrowserTreePanel::CopyPath(const wxFileName& path) {
    wxClipboardLocker clipboard;
    if (!clipboard) {
        return;
    }

    wxTheClipboard->SetData(new wxTextDataObject(path.GetFullPath()));
}

void FileBrowserTreePanel::HandleItemContextMenu(wxDataViewEvent& event) {
    const wxDataViewItem item = event.GetItem();
    const wxFileName path = ResolveItemPath(item);
    if (path.IsOk()) {
        m_dataViewTreeCtrl1->Select(item);
    }

    ShowBrowserContextMenu(m_dataViewTreeCtrl1, path);
}

void FileBrowserTreePanel::HandleHeaderContextMenu(wxContextMenuEvent& event) {
    ShowBrowserContextMenu(this, {});
}

void FileBrowserTreePanel::ShowBrowserContextMenu(wxWindow* owner, const wxFileName& path) {
    wxMenu menu;
    int openId = wxID_NONE;
    int copyPathId = wxID_NONE;
    // Custom IDs avoid macOS applying responder-chain validation for stock
    // commands such as wxID_COPY and disabling the item.
    if (path.IsOk()) {
        openId = menu.Append(wxID_ANY, _("Open"))->GetId();
        copyPathId = menu.Append(wxID_ANY, _("Copy Path"))->GetId();
        menu.AppendSeparator();
    }
    const int homeId = menu.Append(wxID_ANY, _("Home"))->GetId();

    const int selection = owner->GetPopupMenuSelectionFromUser(menu);
    if (path.IsOk() && selection == openId) {
        OpenPath(path);
    } else if (path.IsOk() && selection == copyPathId) {
        CopyPath(path);
    } else if (selection == homeId && m_onHomeRequested) {
        m_onHomeRequested();
    }
}

void FileBrowserTreePanel::HandleCloseButtonClick(wxCommandEvent& event) {
    if (m_onCloseRequested) {
        m_onCloseRequested();
    }
}

bool FileBrowserTreePanel::IsShowingDir(const wxFileName& dir) const {
    return m_currentPath.IsOk() && m_currentPath.SameAs(wxFileName::DirName(dir.GetFullPath()));
}

void FileBrowserTreePanel::ReloadCurrentDir() {
    if (m_currentPath.IsOk() && m_currentPath.DirExists()) {
        // A live reload after an fs event: the list refreshes under the user,
        // so the view must not jump to the top.
        ListDir(m_currentPath, ScrollBehavior::KeepPosition);
    }
}
