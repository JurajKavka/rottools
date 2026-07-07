# rotreader — the first tool in the suite: a native Markdown viewer.
include(RotToolsVersion)
include(RotToolsPackaging)

add_executable(rotreader
    main.cpp
    MainFrame.cpp
    MainFrameWx.cpp
)

target_compile_features(rotreader PRIVATE cxx_std_20)
target_link_libraries(rotreader PRIVATE
    rottools::ui_webview
    rottools::ui_htmlsource
    rottools::ui_mdsource
    rottools::ui_filetree
    rottools::ui_filedrop
    rottools::md2html
    rottools::helpers
    ${wxWidgets_LIBRARIES}
)

# ./VERSION is the single source of truth -> generated "version.h" + ${ROTREADER_VERSION}
rottools_apply_version(rotreader ROTREADER "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")

# Install rules + per-OS CPack packaging (.dmg / .deb+.tar.gz / NSIS+.zip)
rottools_package_app(
    TARGET       rotreader
    DISPLAY_NAME "ℜ⛤✝ reader"
    EXE_NAME     rotreader
    BUNDLE_ID    com.jurajkavka.rotreader
    VERSION      "${ROTREADER_VERSION}"
    VENDOR       "Juraj Kavka"
    DESCRIPTION  "A native Markdown viewer"
    ICON_PNG     "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.png"
    DESKTOP_CATEGORIES "Utility;TextTools;"
)
