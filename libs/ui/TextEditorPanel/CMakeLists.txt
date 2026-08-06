# rottools::ui_texteditor — reusable plain-text editor panel.
add_library(rottools_ui_texteditor STATIC TextEditorPanel.cpp TextEditorPanelWx.cpp)
add_library(rottools::ui_texteditor ALIAS rottools_ui_texteditor)

target_include_directories(rottools_ui_texteditor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_texteditor PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_texteditor PUBLIC ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_texteditor_app main.cpp)
    target_link_libraries(rottools_ui_texteditor_app PRIVATE rottools::ui_texteditor ${wxWidgets_LIBRARIES})
endif()
