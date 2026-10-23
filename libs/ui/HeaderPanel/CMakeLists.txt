# rottools::ui_headerpanel — a header row with tool buttons and a close button.
add_library(rottools_ui_headerpanel STATIC HeaderPanel.cpp)
add_library(rottools::ui_headerpanel ALIAS rottools_ui_headerpanel)

target_include_directories(rottools_ui_headerpanel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_headerpanel PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_headerpanel PUBLIC rottools::ui_flatbuttons ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_headerpanel_app main.cpp)
    target_link_libraries(rottools_ui_headerpanel_app PRIVATE rottools::ui_headerpanel ${wxWidgets_LIBRARIES})
endif()
