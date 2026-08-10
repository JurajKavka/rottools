# rottools::ui_markdowneditor — Markdown-configured editor with file watching.
add_library(rottools_ui_markdowneditor STATIC MarkdownEditorPanel.cpp)
add_library(rottools::ui_markdowneditor ALIAS rottools_ui_markdowneditor)

target_include_directories(rottools_ui_markdowneditor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_markdowneditor PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_markdowneditor
    PUBLIC rottools::ui_scintillatexteditor
    PRIVATE rottools::fswatcher
)
