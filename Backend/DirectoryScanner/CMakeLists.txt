cmake_minimum_required(VERSION 4.0)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(DirectoryScanner)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../HelperFunctions ${CMAKE_CURRENT_BINARY_DIR}/HelperFunctions)
endif()

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})

add_library(DirectoryScanner STATIC DirectoryScanner.cpp)

target_include_directories(DirectoryScanner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(DirectoryScanner PRIVATE cxx_std_20)
target_link_libraries(DirectoryScanner PRIVATE ${wxWidgets_LIBRARIES} HelperFunctions)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(DirectoryScannerApp main.cpp)
    target_link_libraries(DirectoryScannerApp PRIVATE DirectoryScanner HelperFunctions)
endif()




