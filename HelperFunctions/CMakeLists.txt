cmake_minimum_required(VERSION 4.0)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(HelperFunctions)
endif()

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})

add_library(HelperFunctions STATIC HelperFunctions.cpp)

target_include_directories(HelperFunctions PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(HelperFunctions PUBLIC cxx_std_20)
target_link_libraries(HelperFunctions PUBLIC ${wxWidgets_LIBRARIES})

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(HelperFunctionsApp main.cpp)
    target_link_libraries(HelperFunctionsApp PRIVATE ${wxWidgets_LIBRARIES} HelperFunctions)
endif()
