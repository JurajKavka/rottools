# rottools::ui_textfilepreviewdialog — reusable native plain-text preview dialog.
add_library(rottools_ui_textfilepreviewdialog STATIC
    TextFilePreviewDialog.cpp
    TextFilePreviewDialogWx.cpp
)
add_library(rottools::ui_textfilepreviewdialog ALIAS rottools_ui_textfilepreviewdialog)

target_include_directories(rottools_ui_textfilepreviewdialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rottools_ui_textfilepreviewdialog PUBLIC cxx_std_20)
target_link_libraries(rottools_ui_textfilepreviewdialog PUBLIC ${wxWidgets_LIBRARIES})

if(ROTTOOLS_BUILD_LIB_APPS)
    add_executable(rottools_ui_textfilepreviewdialog_app main.cpp)
    target_link_libraries(rottools_ui_textfilepreviewdialog_app
        PRIVATE rottools::ui_textfilepreviewdialog ${wxWidgets_LIBRARIES}
    )
endif()
