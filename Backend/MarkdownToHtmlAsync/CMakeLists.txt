cmake_minimum_required(VERSION 4.0)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(MarkdownToHtmlAsync)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../HelperFunctions ${CMAKE_CURRENT_BINARY_DIR}/HelperFunctions)
endif()

include(FetchContent)
FetchContent_Declare(
    md4c
    GIT_REPOSITORY https://github.com/mity/md4c.git
    GIT_TAG        release-0.5.3
)
FetchContent_MakeAvailable(md4c)

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})

add_library(MarkdownToHtmlAsync STATIC MarkdownToHtmlAsync.cpp)

target_include_directories(MarkdownToHtmlAsync PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(MarkdownToHtmlAsync PRIVATE cxx_std_20)
target_link_libraries(MarkdownToHtmlAsync PRIVATE md4c-html ${wxWidgets_LIBRARIES} HelperFunctions)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(MarkdownToHtmlAsyncApp main.cpp)
    target_link_libraries(MarkdownToHtmlAsyncApp PRIVATE MarkdownToHtmlAsync HelperFunctions)
endif()




