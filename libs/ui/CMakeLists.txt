# Reusable wxWidgets UI components. Isolated app builds configure only the
# components they consume; the demo option exposes the complete component set.
if(ROTTOOLS_BUILD_ROTREADER OR ROTTOOLS_BUILD_LIB_APPS)
    add_subdirectory(FileDropTarget)
    add_subdirectory(WebViewPanel)
    add_subdirectory(HtmlSourcePanel)
    add_subdirectory(MarkdownPreviewPanel)
    add_subdirectory(ScintillaTextEditorPanel)
    add_subdirectory(MarkdownEditorPanel)
endif()

if(ROTTOOLS_BUILD_ROTREADER OR ROTTOOLS_BUILD_ROTPAD OR ROTTOOLS_BUILD_LIB_APPS)
    add_subdirectory(FileBrowserTreePanel)
endif()

if(ROTTOOLS_BUILD_ROTPAD OR ROTTOOLS_BUILD_LIB_APPS)
    add_subdirectory(TextEditorPanel)
endif()
