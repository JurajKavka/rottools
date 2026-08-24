# rottools::ui_bookmarks — persistent flat bookmarks menu.
add_library(rottools_ui_bookmarks STATIC BookmarkStore.cpp)
add_library(rottools::ui_bookmarks ALIAS rottools_ui_bookmarks)

target_include_directories(rottools_ui_bookmarks PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_bookmarks PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_bookmarks PUBLIC rottools::appconfig ${wxWidgets_LIBRARIES})
