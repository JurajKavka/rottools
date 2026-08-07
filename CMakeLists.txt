# ----------------------------------------------------------------------------
# rottools — a suite/monorepo of small desktop utilities sharing common libs.
#
# This umbrella configures the whole tree in one build. Shared code lives under
# libs/ (namespaced rottools::* targets); each tool lives under apps/<tool>/ and
# is selected independently via ROTTOOLS_BUILD_<TOOL> options so a build can
# contain just one tool. Dependencies resolve either from the system (Homebrew /
# apt) or, for macOS and Windows releases, build wxWidgets from a pinned source
# archive (see CMakePresets.json).
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
option(ROTTOOLS_BUILD_ROTPAD    "Build the rotpad tool" ON)
option(ROTTOOLS_USE_BUNDLED_WXWIDGETS
       "Build the pinned wxWidgets release from source" OFF)
# Optional per-library standalone smoke-test executables (was the old
# `if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)` app-per-folder trick).
option(ROTTOOLS_BUILD_LIB_APPS  "Build per-library standalone smoke-test apps" OFF)

# --- wxWidgets: resolved once for the whole tree -----------------------------
if(ROTTOOLS_USE_BUNDLED_WXWIDGETS)
    include(RotToolsWxWidgets)
else()
    find_package(wxWidgets COMPONENTS core base stc webview REQUIRED)
    if(wxWidgets_USE_FILE)
        include(${wxWidgets_USE_FILE})
    endif()
endif()

add_subdirectory(libs)
add_subdirectory(apps)

# Packaging. Each tool populates the CPACK_* cache vars via rottools_package_app();
# this single include wires them up. Build/package one tool per invocation.
include(CPack)
