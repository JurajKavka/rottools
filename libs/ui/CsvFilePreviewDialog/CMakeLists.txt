add_library(rottools_ui_csvfilepreviewdialog STATIC
    CsvFilePreviewDialog.cpp
    CsvFilePreviewDialogWx.cpp
)
add_library(rottools::ui_csvfilepreviewdialog ALIAS rottools_ui_csvfilepreviewdialog)
target_include_directories(rottools_ui_csvfilepreviewdialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_csvfilepreviewdialog PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_csvfilepreviewdialog PUBLIC rottools::csvparser ${wxWidgets_LIBRARIES})
if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_csvfilepreviewdialog_app main.cpp)
    target_link_libraries(rottools_ui_csvfilepreviewdialog_app PRIVATE rottools::ui_csvfilepreviewdialog)
endif()
