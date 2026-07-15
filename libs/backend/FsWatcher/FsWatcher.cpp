#include "FsWatcher.h"

#include "HelperFunctions.h"

FsWatcher::FsWatcher(const wxFileName& target, Callback onChanged, int debounceMs)
    : m_watchingFile(!target.GetFullName().IsEmpty()),
      m_target(target),
      m_watchedDir(m_watchingFile ? wxFileName::DirName(target.GetPath()) : target),
      m_onChanged(std::move(onChanged)),
      m_debounceMs(debounceMs) {
    m_watcher.SetOwner(this);
    Bind(wxEVT_FSWATCHER, &FsWatcher::HandleFsEvent, this);
    m_timer.SetOwner(this);
    Bind(wxEVT_TIMER, &FsWatcher::HandleDebounce, this);

    // AddTree is required for reliable macOS FSEvents integration.
    if (m_watchedDir.DirExists()) {
        if (!m_watcher.AddTree(m_watchedDir)) {
            printError("[FsWatcher] failed to add path: {}", m_watchedDir.GetFullPath());
        }
    }
}

FsWatcher::~FsWatcher() {
    // Works around https://github.com/wxWidgets/wxWidgets/issues/26658: on macOS
    // ~wxFsEventsFileSystemWatcher drops its stream map without
    // FSEventStreamStop/Invalidate, leaving AddTree() streams scheduled on the
    // run loop with a dangling context -> SIGSEGV on the next fs event.
    // RemoveAll() on the still-alive watcher invalidates them. Drop once fixed.
    m_watcher.RemoveAll();
}

bool FsWatcher::IsRelevant(wxFileSystemWatcherEvent& event) const {
    int changeType = event.GetChangeType();

    if (m_watchingFile) {
        if (!(changeType & (wxFSW_EVENT_MODIFY | wxFSW_EVENT_CREATE | wxFSW_EVENT_RENAME))) {
            return false;
        }
        if (event.GetPath().SameAs(m_target)) {
            return true;
        }
        return (changeType & wxFSW_EVENT_RENAME) && event.GetNewPath().SameAs(m_target);
    }

    if (!(changeType & (wxFSW_EVENT_CREATE | wxFSW_EVENT_DELETE | wxFSW_EVENT_RENAME))) {
        return false;
    }
    if (wxFileName::DirName(event.GetPath().GetPath()).SameAs(m_watchedDir)) {
        return true;
    }
    return (changeType & wxFSW_EVENT_RENAME) && event.GetNewPath().IsOk() &&
           wxFileName::DirName(event.GetNewPath().GetPath()).SameAs(m_watchedDir);
}

void FsWatcher::HandleFsEvent(wxFileSystemWatcherEvent& event) {
    if (IsRelevant(event)) {
        m_timer.StartOnce(m_debounceMs);
    }
}

void FsWatcher::HandleDebounce(wxTimerEvent& event) {
    if (m_onChanged) {
        m_onChanged();
    }
}
