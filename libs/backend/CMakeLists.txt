add_subdirectory(AppConfigFunctions)
add_subdirectory(BookmarkStore)

# Backend workers (async, non-UI logic).
add_subdirectory(MarkdownToHtmlAsync)
add_subdirectory(DirectoryScanner)
add_subdirectory(FsWatcher)
