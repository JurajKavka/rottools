#pragma once

#include <wx/filename.h>
#include <wx/fswatcher.h>
#include <wx/timer.h>

#include <functional>

// Watches a directory or a file and invokes a callback, debounced, on a
// relevant change:
//   - directory: a direct child is created/deleted/renamed
//   - file:      that file is modified/created/renamed
// Recreate the object to change what is watched.
class FsWatcher : public wxEvtHandler {
   public:
    using Callback = std::function<void()>;
    static constexpr int kDefaultDebounceMs = 250;

    // target may be a directory or a file; they are told apart by shape
    // (a wxFileName::DirName carries no filename).
    FsWatcher(const wxFileName& target, Callback onChanged, int debounceMs = kDefaultDebounceMs);
    ~FsWatcher() override;

    FsWatcher(const FsWatcher&) = delete;
    FsWatcher& operator=(const FsWatcher&) = delete;

   private:
    void HandleFsEvent(wxFileSystemWatcherEvent& event);
    void HandleDebounce(wxTimerEvent& event);
    bool IsRelevant(wxFileSystemWatcherEvent& event) const;

    bool m_watchingFile;
    wxFileName m_target;
    wxFileName m_watchedDir;
    Callback m_onChanged;
    int m_debounceMs;
    wxFileSystemWatcher m_watcher;
    wxTimer m_timer;
};
