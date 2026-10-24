# rottools::ui_webview — generic lazy wxWebView wrapper panel.
add_library(rottools_ui_webview STATIC WebViewPanel.cpp WebViewPanelWx.cpp)
add_library(rottools::ui_webview ALIAS rottools_ui_webview)

target_include_directories(rottools_ui_webview PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_webview PUBLIC cxx_std_20)
# helpers is PUBLIC because WebViewPanel.h includes "HelperFunctions.h".
target_link_libraries(rottools_ui_webview PUBLIC rottools::helpers ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../www/src/games/neon-asteroids.html
        neon-asteroids.html
        COPYONLY
    )
    add_executable(rottools_ui_webview_app main.cpp)
    target_link_libraries(rottools_ui_webview_app PRIVATE rottools::ui_webview ${wxWidgets_LIBRARIES})
endif()
