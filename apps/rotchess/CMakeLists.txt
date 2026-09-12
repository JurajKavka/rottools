# rotchess — a compact, native two-player chess board.
include(RotToolsVersion)
include(RotToolsPackaging)

add_executable(rotchess
    main.cpp
    MainFrame.cpp
    ui/ChessBoardPanel.cpp
)

target_compile_features(rotchess PRIVATE cxx_std_20)
target_link_libraries(rotchess PRIVATE
    rottools::chessgame
    rottools::helpers
    ${ROTTOOLS_WX_CORE_LIBRARIES}
)

# ./VERSION is the single source of truth -> generated "version.h" + ${ROTCHESS_VERSION}
rottools_apply_version(rotchess ROTCHESS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")

# Install rules + per-OS CPack packaging (.dmg / .deb+.tar.gz / NSIS+.zip).
rottools_package_app(
    TARGET       rotchess
    DISPLAY_NAME "ROT Chess"
    EXE_NAME     rotchess
    BUNDLE_ID    com.jurajkavka.rotchess
    VERSION      "${ROTCHESS_VERSION}"
    VENDOR       "Juraj Kavka"
    DESCRIPTION  "A compact native chess board for two local players"
    ICON_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons"
    DESKTOP_CATEGORIES "Game;BoardGame;"
)
