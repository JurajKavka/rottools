# rottools::ui_filetree — file-browser tree; consumes rottools::dirscan.
add_library(rottools_ui_filetree STATIC FileBrowserTreePanel.cpp FileBrowserTreePanelWx.cpp)
add_library(rottools::ui_filetree ALIAS rottools_ui_filetree)

target_include_directories(rottools_ui_filetree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_filetree PUBLIC cxx_std_20)
# Both PUBLIC: FileBrowserTreePanel.h includes "DirectoryScanner.h" and "HelperFunctions.h".
target_link_libraries(rottools_ui_filetree PUBLIC rottools::dirscan rottools::helpers ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_filetree_app main.cpp)
    target_link_libraries(rottools_ui_filetree_app PRIVATE rottools::ui_filetree ${wxWidgets_LIBRARIES})
endif()
