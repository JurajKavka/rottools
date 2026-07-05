cmake_minimum_required(VERSION 4.0)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(HtmlSourcePanel)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    find_package(wxWidgets COMPONENTS core base stc REQUIRED)
    include(${wxWidgets_USE_FILE})
endif()

add_library(HtmlSourcePanel STATIC HtmlSourcePanel.cpp HtmlSourcePanelWx.cpp)

target_include_directories(HtmlSourcePanel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(HtmlSourcePanel PUBLIC cxx_std_20)
target_link_libraries(HtmlSourcePanel PUBLIC ${wxWidgets_LIBRARIES})

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(HtmlSourcePanelApp main.cpp)
    target_link_libraries(HtmlSourcePanelApp PRIVATE ${wxWidgets_LIBRARIES} HtmlSourcePanel)
endif()
