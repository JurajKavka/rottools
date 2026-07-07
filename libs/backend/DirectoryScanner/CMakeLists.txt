# rottools::dirscan — async recursive directory scanner.
add_library(rottools_dirscan STATIC DirectoryScanner.cpp)
add_library(rottools::dirscan ALIAS rottools_dirscan)

target_include_directories(rottools_dirscan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_dirscan PUBLIC cxx_std_20)
target_link_libraries(rottools_dirscan PUBLIC rottools::helpers ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_dirscan_app main.cpp)
    target_link_libraries(rottools_dirscan_app PRIVATE rottools::dirscan rottools::helpers ${wxWidgets_LIBRARIES})
endif()
