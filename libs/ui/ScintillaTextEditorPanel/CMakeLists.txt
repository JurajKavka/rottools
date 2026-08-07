# rottools::ui_scintillatexteditor — reusable Scintilla-backed plain-text editor panel.
add_library(rottools_ui_scintillatexteditor STATIC ScintillaTextEditorPanel.cpp ScintillaTextEditorPanelWx.cpp)
add_library(rottools::ui_scintillatexteditor ALIAS rottools_ui_scintillatexteditor)

target_include_directories(rottools_ui_scintillatexteditor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_scintillatexteditor PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_scintillatexteditor PUBLIC ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_scintillatexteditor_app main.cpp)
    target_link_libraries(
        rottools_ui_scintillatexteditor_app
        PRIVATE rottools::ui_scintillatexteditor ${wxWidgets_LIBRARIES}
    )
endif()
