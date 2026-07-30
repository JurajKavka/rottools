# rottools::ui_mdsource — editable Markdown source view (wxStyledTextCtrl).
add_library(rottools_ui_mdsource STATIC MarkdownSourcePanel.cpp MarkdownSourcePanelWx.cpp)
add_library(rottools::ui_mdsource ALIAS rottools_ui_mdsource)

target_include_directories(rottools_ui_mdsource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_mdsource PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_mdsource PUBLIC rottools::helpers ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_mdsource_app main.cpp)
    target_link_libraries(rottools_ui_mdsource_app PRIVATE rottools::ui_mdsource ${wxWidgets_LIBRARIES})
endif()
