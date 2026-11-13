add_subdirectory(AppConfigFunctions)
add_subdirectory(BookmarkStore)
add_subdirectory(FileManagerBackend)

# Backend workers (async, non-UI logic).
# ROT Pad only edits text. Do not resolve or download md4c for its isolated
# build; Markdown rendering is used by ROT Reader and the standalone demos.
if(ROTTOOLS_BUILD_ROTREADER OR ROTTOOLS_BUILD_LIB_APPS)
    add_subdirectory(MarkdownToHtmlAsync)
endif()
add_subdirectory(DirectoryScanner)
add_subdirectory(FsWatcher)
