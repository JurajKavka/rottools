# rottools::appconfig — access to settings stored for the current application.
add_library(rottools_appconfig STATIC AppConfigFunctions.cpp)
add_library(rottools::appconfig ALIAS rottools_appconfig)

target_include_directories(rottools_appconfig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_appconfig PUBLIC cxx_std_20)
target_link_libraries(rottools_appconfig PUBLIC ${wxWidgets_LIBRARIES})
