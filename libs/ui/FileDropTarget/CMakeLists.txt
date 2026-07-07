# rottools::ui_filedrop — generic std::function<void(const wxFileName&)> drop target.
# (Previously compiled straight into the app-specific Components lib; now its own lib.)
add_library(rottools_ui_filedrop STATIC FileDropTarget.cpp)
add_library(rottools::ui_filedrop ALIAS rottools_ui_filedrop)

target_include_directories(rottools_ui_filedrop PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_filedrop PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_filedrop PUBLIC ${wxWidgets_LIBRARIES})
