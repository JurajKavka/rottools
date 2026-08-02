# rottools_package_app(TARGET <t> DISPLAY_NAME <n> EXE_NAME <e> BUNDLE_ID <id>
#                       VERSION <v> VENDOR <vendor> DESCRIPTION <d>
#                       ICON_DIR <path> [DESKTOP_CATEGORIES <c>])
#
# One entry point that wires a tool's icons + install rules + CPack packaging per OS:
#   macOS   -> .app bundle (Info.plist + AppIcon.icns) packed into a .dmg (DragNDrop)
#   Windows -> .rc resource holding the .ico, NSIS installer + portable .zip
#   Linux   -> hicolor icon theme + .desktop, .deb (Depends: libwebkit2gtk) + .tar.gz
#
# ICON_DIR points at a directory produced by scripts/generate-icons.sh:
#
#   <ICON_DIR>/macos/AppIcon.icns
#   <ICON_DIR>/linux/<EXE_NAME>-{16,22,24,32,48,64,128,256,512}.png
#   <ICON_DIR>/linux/<EXE_NAME>.svg
#   <ICON_DIR>/windows/<EXE_NAME>.ico
#
# Those files are committed to git on purpose: the Linux and Windows CI runners
# have no Inkscape, so the build only ever reads finished icons. Regenerate them
# with scripts/generate-icons.sh <app> after editing the master SVG. Missing
# icons warn rather than fail, so a new tool can build before it has artwork.
#
# CPACK_* are set as global cache vars so the single top-level include(CPack)
# (in the umbrella CMakeLists) sees them. This fully supports one tool per build
# (the CI model: build+package one tool at a time). Building several tools in one
# tree and packaging each needs per-app cpack config files — a documented follow-up.
include_guard(GLOBAL)

include(RotToolsIcons)

function(rottools_package_app)
    set(oneValueArgs TARGET DISPLAY_NAME EXE_NAME BUNDLE_ID VERSION VENDOR DESCRIPTION ICON_DIR DESKTOP_CATEGORIES)
    cmake_parse_arguments(APP "" "${oneValueArgs}" "" ${ARGN})

    if(NOT APP_DESKTOP_CATEGORIES)
        set(APP_DESKTOP_CATEGORIES "Utility;")
    endif()

    # Sizes installed into the freedesktop hicolor theme on Linux. 22 and 24 are
    # the GTK toolbar/menu sizes; the rest are the standard launcher sizes.
    set(_linux_icon_sizes 16 22 24 32 48 64 128 256 512)

    # Sizes compiled into the executable for the runtime window icon (X11 and
    # Windows pick whichever fits; macOS uses the bundle icon instead). Kept
    # short on purpose — these bytes end up in the binary.
    set(_window_icon_sizes 16 32 48 64 128 256)

    # Substitution values shared by the per-OS templates in packaging/.
    set(ROTTOOLS_APP_DISPLAY_NAME "${APP_DISPLAY_NAME}")
    set(ROTTOOLS_APP_EXE_NAME     "${APP_EXE_NAME}")
    set(ROTTOOLS_APP_BUNDLE_ID    "${APP_BUNDLE_ID}")
    set(ROTTOOLS_APP_VERSION      "${APP_VERSION}")
    set(ROTTOOLS_APP_DESCRIPTION  "${APP_DESCRIPTION}")
    set(ROTTOOLS_APP_CATEGORIES   "${APP_DESKTOP_CATEGORIES}")

    # --- window icon compiled into the binary --------------------------------
    set(_window_icon_pngs "")
    foreach(_size IN LISTS _window_icon_sizes)
        set(_png "${APP_ICON_DIR}/linux/${APP_EXE_NAME}-${_size}.png")
        if(EXISTS "${_png}")
            list(APPEND _window_icon_pngs "${_png}")
        endif()
    endforeach()
    if(_window_icon_pngs)
        rottools_embed_window_icon(TARGET ${APP_TARGET} PNGS ${_window_icon_pngs})
    else()
        message(WARNING
            "No square icon PNGs under ${APP_ICON_DIR}/linux — ${APP_TARGET} will have no "
            "window icon. Run scripts/generate-icons.sh ${APP_EXE_NAME}.")
    endif()

    # --- OS / arch slug used in artifact file names --------------------------
    if(APPLE)
        set(_os macos)
    elseif(WIN32)
        set(_os windows)
    else()
        set(_os linux)
    endif()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _arch)
    if(NOT _arch)
        set(_arch "unknown")
    endif()

    set_target_properties(${APP_TARGET} PROPERTIES OUTPUT_NAME "${APP_EXE_NAME}")

    # ------------------------------------------------------------------------
    # Platform bundling + install rules
    # ------------------------------------------------------------------------
    if(APPLE)
        # AppIcon.icns is generated ahead of time by scripts/generate-icons.sh:
        # every size is rendered from the SVG at its own resolution, which is
        # sharper than downscaling one 1024px PNG, and CFBundleIconFile in
        # Info.plist names it.
        set(_icns "${APP_ICON_DIR}/macos/AppIcon.icns")
        if(EXISTS "${_icns}")
            target_sources(${APP_TARGET} PRIVATE "${_icns}")
            set_source_files_properties("${_icns}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        else()
            message(WARNING "${_icns} is missing — the .app will use the generic icon. "
                            "Run scripts/generate-icons.sh ${APP_EXE_NAME}.")
        endif()

        # Finder and the Dock cache a bundle's icon and re-read it only when the
        # bundle's own modification date changes. An incremental build replaces
        # Contents/Resources/AppIcon.icns without touching the .app directory
        # itself, so a new icon would keep showing as the old one until the
        # bundle was deleted and rebuilt. Touching it closes that gap, so
        # `make rebuild` is enough after `make icons`.
        add_custom_command(TARGET ${APP_TARGET} POST_BUILD
            COMMAND touch "$<TARGET_BUNDLE_DIR:${APP_TARGET}>"
            COMMENT "Touching ${APP_EXE_NAME}.app so Finder re-reads its icon"
            VERBATIM)

        # Configure Info.plist from the app's template.
        set(_plist "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/Info.plist.in" "${_plist}" @ONLY)

        set_target_properties(${APP_TARGET} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${_plist}"
            MACOSX_BUNDLE_BUNDLE_NAME "${APP_DISPLAY_NAME}"
            MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_BUNDLE_ID}")

        install(TARGETS ${APP_TARGET} BUNDLE DESTINATION "." COMPONENT ${APP_EXE_NAME})

        # Re-sign after installing. The linker gives every arm64 binary an ad-hoc
        # signature, and both CPACK_STRIP_FILES (strip) and CMake's install-time
        # rpath rewriting invalidate it. macOS rejects an invalidly signed app
        # outright — "<app> is damaged and can't be opened", with no way past it —
        # whereas a validly signed but un-notarized app gets the ordinary
        # unidentified-developer prompt, which the user can clear via
        # Privacy & Security > Open Anyway. Same ad-hoc identity ("-"), just
        # applied last. Declared after install(TARGETS) because install rules run
        # in declaration order.
        install(CODE "
            execute_process(
                COMMAND codesign --force --deep --sign - \"\${CMAKE_INSTALL_PREFIX}/${APP_EXE_NAME}.app\"
                RESULT_VARIABLE _rottools_codesign_result)
            if(NOT _rottools_codesign_result EQUAL 0)
                message(WARNING
                    \"codesign failed (\${_rottools_codesign_result}); macOS will report the app as damaged\")
            endif()
        " COMPONENT ${APP_EXE_NAME})

    elseif(WIN32)
        set_target_properties(${APP_TARGET} PROPERTIES WIN32_EXECUTABLE TRUE)

        # The .ico reaches Explorer and the taskbar through a resource script.
        # app.rc.in is configured into the build tree so it can name the .ico by
        # absolute path, and it pulls in wx/msw/wx.rc for wxWidgets' own cursors
        # and wxSTD_* icons. RC is enabled in the root CMakeLists on Windows.
        set(_ico "${APP_ICON_DIR}/windows/${APP_EXE_NAME}.ico")
        if(EXISTS "${_ico}")
            set(ROTTOOLS_APP_ICO "${_ico}")
            set(_rc "${CMAKE_CURRENT_BINARY_DIR}/${APP_EXE_NAME}.rc")
            configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/app.rc.in" "${_rc}" @ONLY)
            target_sources(${APP_TARGET} PRIVATE "${_rc}")
        else()
            message(WARNING "${_ico} is missing — the .exe will use the generic icon. "
                            "Run scripts/generate-icons.sh ${APP_EXE_NAME}.")
        endif()

        install(TARGETS ${APP_TARGET} RUNTIME DESTINATION "." COMPONENT ${APP_EXE_NAME})

    else() # Linux / other Unix
        install(TARGETS ${APP_TARGET} RUNTIME DESTINATION "bin" COMPONENT ${APP_EXE_NAME})

        # .desktop entry. Its "Icon=" holds the bare name; the icon theme below
        # resolves it, which is also how GNOME/Wayland picks the window icon.
        set(_desktop "${CMAKE_CURRENT_BINARY_DIR}/${APP_EXE_NAME}.desktop")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/app.desktop.in" "${_desktop}" @ONLY)
        install(FILES "${_desktop}" DESTINATION "share/applications" COMPONENT ${APP_EXE_NAME})

        # freedesktop hicolor theme: one PNG per size plus the scalable SVG.
        foreach(_size IN LISTS _linux_icon_sizes)
            set(_png "${APP_ICON_DIR}/linux/${APP_EXE_NAME}-${_size}.png")
            if(EXISTS "${_png}")
                install(FILES "${_png}"
                        DESTINATION "share/icons/hicolor/${_size}x${_size}/apps"
                        RENAME "${APP_EXE_NAME}.png"
                        COMPONENT ${APP_EXE_NAME})
            endif()
        endforeach()

        set(_svg "${APP_ICON_DIR}/linux/${APP_EXE_NAME}.svg")
        if(EXISTS "${_svg}")
            install(FILES "${_svg}"
                    DESTINATION "share/icons/hicolor/scalable/apps"
                    COMPONENT ${APP_EXE_NAME})
        endif()
    endif()

    # ------------------------------------------------------------------------
    # CPack configuration (global cache vars; consumed by include(CPack))
    # ------------------------------------------------------------------------
    set(CPACK_PACKAGE_NAME                "${APP_EXE_NAME}"                    CACHE INTERNAL "")
    set(CPACK_PACKAGE_VERSION             "${APP_VERSION}"                     CACHE INTERNAL "")
    set(CPACK_PACKAGE_VENDOR              "${APP_VENDOR}"                      CACHE INTERNAL "")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APP_DESCRIPTION}"                 CACHE INTERNAL "")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY   "${APP_DISPLAY_NAME}"               CACHE INTERNAL "")
    set(CPACK_PACKAGE_FILE_NAME "${APP_EXE_NAME}-${APP_VERSION}-${_os}-${_arch}" CACHE INTERNAL "")
    set(CPACK_STRIP_FILES                 TRUE                                 CACHE INTERNAL "")

    # Package ONLY this app's component, in one archive. Excludes stray install()
    # rules pulled in by dependencies built in-tree (e.g. md4c's bin/include/lib).
    set(CPACK_COMPONENTS_ALL       "${APP_EXE_NAME}"        CACHE INTERNAL "")
    set(CPACK_COMPONENTS_GROUPING  "ALL_COMPONENTS_IN_ONE"  CACHE INTERNAL "")
    set(CPACK_COMPONENT_${APP_EXE_NAME}_HIDDEN TRUE         CACHE INTERNAL "")

    if(APPLE)
        set(CPACK_GENERATOR       "DragNDrop"          CACHE INTERNAL "")
        set(CPACK_DMG_VOLUME_NAME "${APP_DISPLAY_NAME}" CACHE INTERNAL "")
    elseif(WIN32)
        set(CPACK_GENERATOR          "ZIP;NSIS"           CACHE INTERNAL "")
        set(CPACK_NSIS_DISPLAY_NAME  "${APP_DISPLAY_NAME}" CACHE INTERNAL "")
        set(CPACK_NSIS_PACKAGE_NAME  "${APP_DISPLAY_NAME}" CACHE INTERNAL "")
        set(CPACK_NSIS_INSTALLED_ICON_NAME "${APP_EXE_NAME}.exe" CACHE INTERNAL "")
    else()
        set(CPACK_GENERATOR                "TGZ;DEB"      CACHE INTERNAL "")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${APP_VENDOR}" CACHE INTERNAL "")
        # GTK, WebKitGTK (the wxWebView engine) and wxWidgets itself are depended
        # on, not bundled. Let dpkg-shlibdeps read the linked binary and derive
        # the list: hand-maintaining it means guessing exact package names, which
        # drift across releases (Ubuntu 24.04's 64-bit time_t "t64" renames, for
        # one). The hardcoded list stays as the fallback if shlibdeps is absent.
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON CACHE INTERNAL "")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libwebkit2gtk-4.1-0, libgtk-3-0" CACHE INTERNAL "")
        set(CPACK_DEBIAN_PACKAGE_SECTION "utils"         CACHE INTERNAL "")
    endif()
endfunction()
