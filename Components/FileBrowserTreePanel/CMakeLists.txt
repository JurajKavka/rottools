cmake_minimum_required(VERSION 4.0)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(FileBrowserTreePanel)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../HelperFunctions ${CMAKE_CURRENT_BINARY_DIR}/HelperFunctions)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../Backend ${CMAKE_CURRENT_BINARY_DIR}/Backend)
endif()

add_library(FileBrowserTreePanel STATIC FileBrowserTreePanel.cpp FileBrowserTreePanelWx.cpp)

target_include_directories(FileBrowserTreePanel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(FileBrowserTreePanel PUBLIC cxx_std_20)
target_link_libraries(FileBrowserTreePanel PUBLIC HelperFunctions Backend)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    find_package(wxWidgets REQUIRED)
    include(${wxWidgets_USE_FILE})
    add_executable(FileBrowserTreePanelApp main.cpp)
    target_link_libraries(FileBrowserTreePanelApp PRIVATE ${wxWidgets_LIBRARIES} Backend FileBrowserTreePanel HelperFunctions)
endif()
