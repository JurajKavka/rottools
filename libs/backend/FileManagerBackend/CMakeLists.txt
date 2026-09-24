# Synchronous filesystem operations, independent of the desktop applications.
add_library(rottools_filemanager STATIC FileManagerBackend.cpp)
add_library(rottools::filemanager ALIAS rottools_filemanager)

target_include_directories(rottools_filemanager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_filemanager PUBLIC cxx_std_20)

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_filemanager_app main.cpp)
    target_link_libraries(rottools_filemanager_app PRIVATE rottools::filemanager rottools::helpers)
endif()
