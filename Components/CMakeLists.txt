add_subdirectory(WebViewPanel)

add_library(Components MainFrameWx.cpp MainFrame.cpp FileDropTarget/FileDropTarget.cpp)

target_include_directories(Components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Backend)
target_include_directories(Components PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/FileDropTarget)

target_link_libraries(Components PRIVATE HelperFunctions Backend WebViewPanel ${wxWidgets_LIBRARIES})
