# rottools::bookmark_store — persistent directory and document bookmarks.
add_library(rottools_bookmark_store STATIC BookmarkStore.cpp)
add_library(rottools::bookmark_store ALIAS rottools_bookmark_store)

target_include_directories(rottools_bookmark_store PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_bookmark_store PUBLIC cxx_std_20)
target_link_libraries(rottools_bookmark_store PUBLIC rottools::appconfig)
