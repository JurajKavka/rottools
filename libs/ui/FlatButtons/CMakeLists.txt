# rottools::ui_flatbuttons — small flat wxWidgets buttons.
add_library(rottools_ui_flatbuttons STATIC FlatButtonsBase.cpp FlatCloseButton.cpp FlatHomeButton.cpp)
add_library(rottools::ui_flatbuttons ALIAS rottools_ui_flatbuttons)

target_include_directories(rottools_ui_flatbuttons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_flatbuttons PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_flatbuttons PUBLIC ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_flatbuttons_app main.cpp)
    target_link_libraries(rottools_ui_flatbuttons_app PRIVATE rottools::ui_flatbuttons ${wxWidgets_LIBRARIES})
endif()
