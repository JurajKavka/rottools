# rottools::ui_markdownpreview — renders a markdown file to HTML in a web view.
add_library(rottools_ui_markdownpreview STATIC MarkdownPreviewPanel.cpp)
add_library(rottools::ui_markdownpreview ALIAS rottools_ui_markdownpreview)

target_include_directories(rottools_ui_markdownpreview PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_markdownpreview PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_markdownpreview PUBLIC ${wxWidgets_LIBRARIES} rottools::ui_webview rottools::md2html)

if(ROTTOOLS_BUILD_LIB_APPS)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    add_executable(rottools_ui_markdownpreview_app main.cpp)
    target_link_libraries(rottools_ui_markdownpreview_app PRIVATE rottools::ui_markdownpreview ${wxWidgets_LIBRARIES})
endif()
