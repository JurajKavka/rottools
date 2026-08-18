# rottools::ui_htmlsource — read-only HTML source view (wxStyledTextCtrl).
add_library(rottools_ui_htmlsource STATIC HtmlSourcePanel.cpp HtmlSourcePanelWx.cpp)
add_library(rottools::ui_htmlsource ALIAS rottools_ui_htmlsource)

target_include_directories(rottools_ui_htmlsource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_htmlsource PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_htmlsource PRIVATE rottools::helpers PUBLIC ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_htmlsource_app main.cpp)
    target_link_libraries(rottools_ui_htmlsource_app PRIVATE rottools::ui_htmlsource ${wxWidgets_LIBRARIES})
endif()
