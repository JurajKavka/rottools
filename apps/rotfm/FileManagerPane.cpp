#include "FileManagerPane.h"

#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/utils.h>

#include <utility>

#include "FileBrowserTreePanel.h"

FileManagerPane::FileManagerPane(wxWindow* parent, ActivatedCallback onActivated)
    : wxPanel(parent), m_onActivated(std::move(onActivated)) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    m_pathText = new wxTextCtrl(this, wxID_ANY, {}, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_browser = new FileBrowserTreePanel(
        this, {
                  .onFileOpened = [this](const wxFileName& file) { HandleFileOpened(file); },
                  .onDirectoryChanged = [this](const wxFileName& directory) { HandleDirectoryChanged(directory); },
                  .onHomeRequested = [this] { HandleHomeRequested(); },
                  .onFocus = [this] { HandleActivated(); },
              });
    m_browser->SetCloseButtonVisible(false);

    sizer->Add(m_pathText, 0, wxEXPAND | wxBOTTOM, 4);
    sizer->Add(m_browser, 1, wxEXPAND);
    SetSizer(sizer);

    m_pathText->Bind(wxEVT_TEXT_ENTER, &FileManagerPane::HandlePathEnter, this);
    m_pathText->Bind(wxEVT_SET_FOCUS, &FileManagerPane::HandlePathFocus, this);
}

void FileManagerPane::SetDirectory(const wxFileName& directory) {
    m_browser->ListDir(wxFileName::DirName(directory.GetFullPath()));
}

wxFileName FileManagerPane::GetCurrentDirectory() const {
    return m_browser->GetCurrentDirectory();
}

std::optional<wxFileName> FileManagerPane::GetSelectedPath() const {
    return m_browser->GetSelectedPath();
}

void FileManagerPane::Reload() {
    m_browser->ReloadCurrentDir();
}

void FileManagerPane::FocusFileList() {
    m_browser->FocusTree();
}

void FileManagerPane::HandleDirectoryChanged(const wxFileName& directory) {
    m_pathText->ChangeValue(directory.GetFullPath());
}

void FileManagerPane::HandleFileOpened(const wxFileName& file) {
    if (!wxLaunchDefaultApplication(file.GetFullPath())) {
        wxMessageBox("Could not open the selected file.", "Open File", wxOK | wxICON_ERROR, this);
    }
}

void FileManagerPane::HandleHomeRequested() {
    wxFileName home;
    home.AssignHomeDir();
    SetDirectory(home);
}

void FileManagerPane::HandleActivated() {
    if (m_onActivated) {
        m_onActivated(this);
    }
}

void FileManagerPane::HandlePathEnter(wxCommandEvent& event) {
    const wxFileName requestedDirectory = wxFileName::DirName(m_pathText->GetValue());
    if (!requestedDirectory.DirExists()) {
        wxMessageBox("Directory does not exist.", "Open Directory", wxOK | wxICON_ERROR, this);
        m_pathText->ChangeValue(GetCurrentDirectory().GetFullPath());
        return;
    }

    SetDirectory(requestedDirectory);
    FocusFileList();
}

void FileManagerPane::HandlePathFocus(wxFocusEvent& event) {
    HandleActivated();
    event.Skip();
}
