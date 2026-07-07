# rottools_package_app(TARGET <t> DISPLAY_NAME <n> EXE_NAME <e> BUNDLE_ID <id>
#                       VERSION <v> VENDOR <vendor> DESCRIPTION <d>
#                       ICON_PNG <path> [DESKTOP_CATEGORIES <c>])
#
# One entry point that wires a tool's install rules + CPack packaging per OS:
#   macOS   -> .app bundle (Info.plist + generated .icns) packed into a .dmg (DragNDrop)
#   Windows -> NSIS installer + portable .zip
#   Linux   -> .deb (Depends: libwebkit2gtk) + portable .tar.gz
#
# CPACK_* are set as global cache vars so the single top-level include(CPack)
# (in the umbrella CMakeLists) sees them. This fully supports one tool per build
# (the CI model: build+package one tool at a time). Building several tools in one
# tree and packaging each needs per-app cpack config files — a documented follow-up.
include_guard(GLOBAL)

function(rottools_package_app)
    set(oneValueArgs TARGET DISPLAY_NAME EXE_NAME BUNDLE_ID VERSION VENDOR DESCRIPTION ICON_PNG DESKTOP_CATEGORIES)
    cmake_parse_arguments(APP "" "${oneValueArgs}" "" ${ARGN})

    if(NOT APP_DESKTOP_CATEGORIES)
        set(APP_DESKTOP_CATEGORIES "Utility;")
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
        # Generate AppIcon.icns from the 1024px master (same sizes make-dmg.sh used).
        if(APP_ICON_PNG AND EXISTS "${APP_ICON_PNG}")
            set(_iconset "${CMAKE_CURRENT_BINARY_DIR}/AppIcon.iconset")
            set(_icns "${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns")
            add_custom_command(
                OUTPUT "${_icns}"
                COMMAND ${CMAKE_COMMAND} -E rm -rf "${_iconset}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${_iconset}"
                COMMAND sips -z 16 16     "${APP_ICON_PNG}" --out "${_iconset}/icon_16x16.png"
                COMMAND sips -z 32 32     "${APP_ICON_PNG}" --out "${_iconset}/icon_16x16@2x.png"
                COMMAND sips -z 32 32     "${APP_ICON_PNG}" --out "${_iconset}/icon_32x32.png"
                COMMAND sips -z 64 64     "${APP_ICON_PNG}" --out "${_iconset}/icon_32x32@2x.png"
                COMMAND sips -z 128 128   "${APP_ICON_PNG}" --out "${_iconset}/icon_128x128.png"
                COMMAND sips -z 256 256   "${APP_ICON_PNG}" --out "${_iconset}/icon_128x128@2x.png"
                COMMAND sips -z 256 256   "${APP_ICON_PNG}" --out "${_iconset}/icon_256x256.png"
                COMMAND sips -z 512 512   "${APP_ICON_PNG}" --out "${_iconset}/icon_256x256@2x.png"
                COMMAND sips -z 512 512   "${APP_ICON_PNG}" --out "${_iconset}/icon_512x512.png"
                COMMAND sips -z 1024 1024 "${APP_ICON_PNG}" --out "${_iconset}/icon_512x512@2x.png"
                COMMAND iconutil -c icns "${_iconset}" -o "${_icns}"
                DEPENDS "${APP_ICON_PNG}"
                COMMENT "Generating AppIcon.icns for ${APP_DISPLAY_NAME}"
                VERBATIM)
            target_sources(${APP_TARGET} PRIVATE "${_icns}")
            set_source_files_properties("${_icns}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        endif()

        # Configure Info.plist from the app's template.
        set(ROTREADER_DISPLAY_NAME "${APP_DISPLAY_NAME}")
        set(ROTREADER_EXE_NAME     "${APP_EXE_NAME}")
        set(ROTREADER_BUNDLE_ID    "${APP_BUNDLE_ID}")
        set(ROTREADER_VERSION      "${APP_VERSION}")
        set(_plist "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/Info.plist.in" "${_plist}" @ONLY)

        set_target_properties(${APP_TARGET} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${_plist}"
            MACOSX_BUNDLE_BUNDLE_NAME "${APP_DISPLAY_NAME}"
            MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_BUNDLE_ID}")

        install(TARGETS ${APP_TARGET} BUNDLE DESTINATION ".")

    elseif(WIN32)
        set_target_properties(${APP_TARGET} PROPERTIES WIN32_EXECUTABLE TRUE)
        install(TARGETS ${APP_TARGET} RUNTIME DESTINATION ".")

    else() # Linux / other Unix
        install(TARGETS ${APP_TARGET} RUNTIME DESTINATION "bin")

        # .desktop entry
        set(ROTREADER_DISPLAY_NAME "${APP_DISPLAY_NAME}")
        set(ROTREADER_EXE_NAME     "${APP_EXE_NAME}")
        set(ROTREADER_DESCRIPTION  "${APP_DESCRIPTION}")
        set(ROTREADER_CATEGORIES   "${APP_DESKTOP_CATEGORIES}")
        set(_desktop "${CMAKE_CURRENT_BINARY_DIR}/${APP_EXE_NAME}.desktop")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/app.desktop.in" "${_desktop}" @ONLY)
        install(FILES "${_desktop}" DESTINATION "share/applications")

        if(APP_ICON_PNG AND EXISTS "${APP_ICON_PNG}")
            install(FILES "${APP_ICON_PNG}"
                    DESTINATION "share/icons/hicolor/512x512/apps"
                    RENAME "${APP_EXE_NAME}.png")
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
        # WebKitGTK (wxWebView engine) + GTK are depended on, not bundled.
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libwebkit2gtk-4.1-0, libgtk-3-0" CACHE INTERNAL "")
        set(CPACK_DEBIAN_PACKAGE_SECTION "utils"         CACHE INTERNAL "")
    endif()
endfunction()
