# rottools::fswatcher — directory/file watcher with a debounced callback.
add_library(rottools_fswatcher STATIC FsWatcher.cpp)
add_library(rottools::fswatcher ALIAS rottools_fswatcher)

target_include_directories(rottools_fswatcher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_fswatcher PUBLIC cxx_std_20)
target_link_libraries(rottools_fswatcher PUBLIC rottools::helpers ${wxWidgets_LIBRARIES})
