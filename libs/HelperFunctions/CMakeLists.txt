# rottools::helpers — logging, wxString std::formatter, shared file/config
# helpers, and the window-icon bundle built from PNGs compiled into the app.
add_library(rottools_helpers STATIC HelperFunctions.cpp AppIcon.cpp)
add_library(rottools::helpers ALIAS rottools_helpers)

target_include_directories(rottools_helpers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_helpers PUBLIC cxx_std_20)
target_link_libraries(rottools_helpers PUBLIC ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_helpers_app main.cpp)
    target_link_libraries(rottools_helpers_app PRIVATE rottools::helpers ${wxWidgets_LIBRARIES})
endif()
