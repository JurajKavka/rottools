# rotfm — a native dual-pane file manager.
include(RotToolsVersion)
include(RotToolsPackaging)

add_executable(rotfm
    main.cpp
    MainFrame.cpp
    FileManagerPane.cpp
)

target_compile_features(rotfm PRIVATE cxx_std_20)
target_link_libraries(rotfm PRIVATE
    rottools::fileops
    rottools::ui_filetree
    ${wxWidgets_LIBRARIES}
)

rottools_apply_version(rotfm ROTFM "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")

rottools_package_app(
    TARGET       rotfm
    DISPLAY_NAME "ROT File Manager"
    EXE_NAME     rotfm
    BUNDLE_ID    com.jurajkavka.rotfm
    VERSION      "${ROTFM_VERSION}"
    VENDOR       "Juraj Kavka"
    DESCRIPTION  "A native dual-pane file manager"
    ICON_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons"
    DESKTOP_CATEGORIES "Utility;FileManager;"
)
