# rotpad — a small, native plain-text editor.
include(RotToolsVersion)
include(RotToolsPackaging)

add_executable(rotpad
    main.cpp
    MainFrame.cpp
    MainFrameWx.cpp
)

target_compile_features(rotpad PRIVATE cxx_std_20)
target_link_libraries(rotpad PRIVATE
    rottools::ui_texteditor
    rottools::ui_filetree
    rottools::helpers
    ${wxWidgets_LIBRARIES}
)

# ./VERSION is the single source of truth -> generated "version.h" + ${ROTPAD_VERSION}
rottools_apply_version(rotpad ROTPAD "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")

# Install rules + per-OS CPack packaging (.dmg / .deb+.tar.gz / NSIS+.zip).
rottools_package_app(
    TARGET       rotpad
    DISPLAY_NAME "ROT Pad"
    EXE_NAME     rotpad
    BUNDLE_ID    com.jurajkavka.rotpad
    VERSION      "${ROTPAD_VERSION}"
    VENDOR       "Juraj Kavka"
    DESCRIPTION  "A simple native plain-text editor"
    ICON_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons"
    DESKTOP_CATEGORIES "Utility;TextEditor;"
)
