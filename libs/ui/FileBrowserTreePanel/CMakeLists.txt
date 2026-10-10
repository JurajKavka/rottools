# rottools::ui_filetree — file-browser tree; consumes rottools::dirscan.
add_library(rottools_ui_filetree STATIC FileBrowserTreePanel.cpp FileBrowserTreePanelWx.cpp)
add_library(rottools::ui_filetree ALIAS rottools_ui_filetree)

target_include_directories(rottools_ui_filetree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_filetree PUBLIC cxx_std_20)
# Both PUBLIC: FileBrowserTreePanel.h includes "DirectoryScanner.h" and "HelperFunctions.h".
target_link_libraries(rottools_ui_filetree
    PUBLIC rottools::dirscan rottools::helpers ${wxWidgets_LIBRARIES}
    PRIVATE rottools::filemanager
)

# wxGTK before 3.3.3 predates wxMoveToTrash(); use GIO only for that build.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND wxWidgets_VERSION_STRING VERSION_LESS 3.3.3)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)
    target_link_libraries(rottools_ui_filetree PRIVATE PkgConfig::GIO)
endif()

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_filetree_app main.cpp)
    target_link_libraries(rottools_ui_filetree_app PRIVATE rottools::ui_filetree ${wxWidgets_LIBRARIES})
endif()
