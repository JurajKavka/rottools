add_subdirectory(WebViewPanel)
add_subdirectory(FileBrowserTreePanel)
add_subdirectory(HtmlSourcePanel)

add_library(Components STATIC MainFrameWx.cpp MainFrame.cpp FileDropTarget/FileDropTarget.cpp FileBrowserTreePanel/FileBrowserTreePanel.cpp)

target_include_directories(Components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Backend)
target_include_directories(Components PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/FileDropTarget)

target_link_libraries(Components PRIVATE HelperFunctions Backend WebViewPanel FileBrowserTreePanel HtmlSourcePanel ${wxWidgets_LIBRARIES})
