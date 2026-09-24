add_subdirectory(AppConfigFunctions)
add_subdirectory(BookmarkStore)
add_subdirectory(FileManagerBackend)

# Backend workers (async, non-UI logic).
add_subdirectory(MarkdownToHtmlAsync)
add_subdirectory(DirectoryScanner)
add_subdirectory(FsWatcher)
