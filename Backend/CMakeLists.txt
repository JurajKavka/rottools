cmake_minimum_required(VERSION 4.0)

include(FetchContent)
FetchContent_Declare(
    md4c
    GIT_REPOSITORY https://github.com/mity/md4c.git
    GIT_TAG        release-0.5.3
)
FetchContent_MakeAvailable(md4c)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(BackendApp)

    # Pull in HelperFunctions from the parent directory for standalone
    add_subdirectory(../HelperFunctions helper_build)
endif()

add_library(Backend INTERFACE)

target_include_directories(Backend INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Backend INTERFACE md4c-html HelperFunctions)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    find_package(wxWidgets REQUIRED)
    include(${wxWidgets_USE_FILE})

    add_executable(BackendApp main.cpp)

    target_link_libraries(BackendApp PRIVATE Backend ${wxWidgets_LIBRARIES})
endif()
