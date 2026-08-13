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
    rottools::appconfig
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

# Advertise rotpad in Windows' Open with list for its supported text formats
# without changing the user's current defaults. Linux uses the desktop file's
# MIME declarations; macOS uses CFBundleDocumentTypes in Info.plist.
if(WIN32)
    # Preserve NSIS quotes, backslashes, and $INSTDIR while CPack writes and
    # reloads CPackConfig.cmake.
    set(CPACK_VERBATIM_VARIABLES TRUE CACHE INTERNAL "")
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS [[
        WriteRegStr HKCR "com.jurajkavka.rotpad.text" "" "ROT Pad Text Document"
        WriteRegStr HKCR "com.jurajkavka.rotpad.text\DefaultIcon" "" "$INSTDIR\rotpad.exe,0"
        WriteRegStr HKCR "com.jurajkavka.rotpad.text\shell\open\command" "" '"$INSTDIR\rotpad.exe" "%1"'
        WriteRegStr HKCR ".txt\OpenWithProgids" "com.jurajkavka.rotpad.text" ""
        WriteRegStr HKCR ".json\OpenWithProgids" "com.jurajkavka.rotpad.text" ""
        WriteRegStr HKCR ".csv\OpenWithProgids" "com.jurajkavka.rotpad.text" ""
        WriteRegStr HKCR ".md\OpenWithProgids" "com.jurajkavka.rotpad.text" ""
        WriteRegStr HKCR ".sql\OpenWithProgids" "com.jurajkavka.rotpad.text" ""
        System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
    ]] CACHE INTERNAL "")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS [[
        DeleteRegValue HKCR ".txt\OpenWithProgids" "com.jurajkavka.rotpad.text"
        DeleteRegValue HKCR ".json\OpenWithProgids" "com.jurajkavka.rotpad.text"
        DeleteRegValue HKCR ".csv\OpenWithProgids" "com.jurajkavka.rotpad.text"
        DeleteRegValue HKCR ".md\OpenWithProgids" "com.jurajkavka.rotpad.text"
        DeleteRegValue HKCR ".sql\OpenWithProgids" "com.jurajkavka.rotpad.text"
        DeleteRegKey HKCR "com.jurajkavka.rotpad.text"
        System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
    ]] CACHE INTERNAL "")
endif()
