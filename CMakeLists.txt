# ----------------------------------------------------------------------------
# rottools — a suite/monorepo of small desktop utilities sharing common libs.
#
# This umbrella configures the whole tree in one build. Shared code lives under
# libs/ (namespaced rottools::* targets); each tool lives under apps/<tool>/ and
# is selected independently via ROTTOOLS_BUILD_<TOOL> options so a build can
# contain just one tool. Dependencies resolve either from the system (Homebrew /
# apt) or from vcpkg when a toolchain file is supplied (see CMakePresets.json).
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24)

# Minimum macOS this build targets. Must be set before project() to take effect.
# Without it CMake stamps the binary with the build machine's OS (minos), so it
# refuses to launch on anything older. Keep <= the CI runner OS and <= the minos
# of every dependency. Override with -DCMAKE_OSX_DEPLOYMENT_TARGET=...
if(APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum macOS version")
endif()

project(rottools LANGUAGES CXX)

# The Windows app icon ships in a .rc resource script (see RotToolsPackaging).
# project(... LANGUAGES CXX) alone leaves RC disabled, and a .rc added to a
# target would then be treated as an unknown source type.
if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# --- Per-tool build toggles (each tool builds/releases independently) --------
option(ROTTOOLS_BUILD_ROTREADER "Build the rotreader tool" ON)
# Optional per-library standalone smoke-test executables (was the old
# `if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)` app-per-folder trick).
option(ROTTOOLS_BUILD_LIB_APPS  "Build per-library standalone smoke-test apps" OFF)

# --- wxWidgets: found once for the whole tree --------------------------------
# The classic FindwxWidgets path works with Homebrew, apt, and the vcpkg port.
find_package(wxWidgets COMPONENTS core base stc webview REQUIRED)
include(${wxWidgets_USE_FILE})

# --- Windows: WebView2 loader search path ------------------------------------
# wxWidgets' Edge WebView backend pulls in the loader with
#   #pragma comment(lib, "WebView2LoaderStatic.lib")
# so the linker has to resolve that name off its search path. On Windows
# FindwxWidgets leaves wxWidgets_LIBRARY_DIRS empty and returns absolute library
# paths instead, so include(${wxWidgets_USE_FILE}) adds no link directory and the
# final link fails with LNK1104. Put the directory holding the loader back on the
# search path. link_directories() only affects targets created afterwards, hence
# the placement above add_subdirectory().
if(WIN32)
    find_library(ROTTOOLS_WEBVIEW2_LOADER NAMES WebView2LoaderStatic WebView2Loader.dll)
    if(ROTTOOLS_WEBVIEW2_LOADER)
        get_filename_component(ROTTOOLS_WEBVIEW2_LOADER_DIR "${ROTTOOLS_WEBVIEW2_LOADER}" DIRECTORY)
        link_directories("${ROTTOOLS_WEBVIEW2_LOADER_DIR}")
    else()
        message(WARNING "WebView2 loader library not found; the wxWebView Edge backend will fail to link.")
    endif()
endif()

# --- Windows: wxWidgets' lexilla ---------------------------------------------
# wxSTC needs two builtin libraries, wxscintilla and wxlexilla. vcpkg's
# wxwidgets CMake wrapper pre-seeds the cache for wxscintilla but not wxlexilla,
# so FindwxWidgets reports "missing components: lexilla", drops it from
# wxWidgets_LIBRARIES, and the link ends in LNK2019 on CreateLexer /
# LexerNameFromID / Lexilla::Load. Search for it the way FindwxWidgets would and
# append it, so every target that already links ${wxWidgets_LIBRARIES} picks it
# up after wxscintilla.
if(WIN32 AND NOT "lexilla" IN_LIST wxWidgets_FIND_COMPONENTS)
    find_library(ROTTOOLS_WX_LEXILLA_RELEASE NAMES wxlexilla
        PATHS "${wxWidgets_LIB_DIR}" NO_DEFAULT_PATH)
    find_library(ROTTOOLS_WX_LEXILLA_DEBUG NAMES wxlexillad
        PATHS "${wxWidgets_LIB_DIR}/../../debug/lib" "${wxWidgets_LIB_DIR}" NO_DEFAULT_PATH)
    if(ROTTOOLS_WX_LEXILLA_RELEASE AND ROTTOOLS_WX_LEXILLA_DEBUG)
        list(APPEND wxWidgets_LIBRARIES
            optimized "${ROTTOOLS_WX_LEXILLA_RELEASE}"
            debug "${ROTTOOLS_WX_LEXILLA_DEBUG}")
    elseif(ROTTOOLS_WX_LEXILLA_RELEASE)
        list(APPEND wxWidgets_LIBRARIES "${ROTTOOLS_WX_LEXILLA_RELEASE}")
    else()
        message(WARNING "wxlexilla not found in ${wxWidgets_LIB_DIR}; wxStyledTextCtrl will fail to link.")
    endif()
endif()

add_subdirectory(libs)
add_subdirectory(apps)

# Packaging. Each tool populates the CPACK_* cache vars via rottools_package_app();
# this single include wires them up. Build/package one tool per invocation.
include(CPack)
