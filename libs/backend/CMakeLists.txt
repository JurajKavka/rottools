if(ROTTOOLS_BUILD_ROTREADER OR ROTTOOLS_BUILD_ROTPAD OR ROTTOOLS_BUILD_LIB_APPS)
    add_subdirectory(AppConfigFunctions)
    add_subdirectory(DirectoryScanner)
endif()

if(ROTTOOLS_BUILD_ROTREADER OR ROTTOOLS_BUILD_LIB_APPS)
    add_subdirectory(BookmarkStore)
    add_subdirectory(MarkdownToHtmlAsync)
    add_subdirectory(FsWatcher)
endif()

if(ROTTOOLS_BUILD_ROTCHESS OR ROTTOOLS_BUILD_LIB_APPS)
    add_subdirectory(ChessGame)
endif()
