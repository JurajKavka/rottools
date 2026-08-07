include_guard(GLOBAL)

include(FetchContent)

# Release builds compile wxWidgets into the application so the macOS and
# Windows packages don't depend on Homebrew or separately installed DLLs.
set(wxBUILD_SHARED OFF CACHE BOOL "Build wxWidgets as static libraries" FORCE)
set(wxBUILD_INSTALL OFF CACHE BOOL "Skip wxWidgets install rules" FORCE)
set(wxBUILD_SAMPLES OFF CACHE STRING "Skip wxWidgets samples" FORCE)
set(wxBUILD_TESTS OFF CACHE STRING "Skip wxWidgets tests" FORCE)
set(wxBUILD_DEMOS OFF CACHE BOOL "Skip wxWidgets demos" FORCE)
set(wxBUILD_BENCHMARKS OFF CACHE BOOL "Skip wxWidgets benchmarks" FORCE)
set(wxBUILD_LOCALES OFF CACHE BOOL "Skip wxWidgets locale catalogs" FORCE)

# rottools only links core, base, STC and WebView. Disabling the other optional
# libraries keeps release builds focused while leaving all core controls native.
set(wxUSE_AUI OFF CACHE BOOL "Disable unused wxAUI library" FORCE)
set(wxUSE_DEBUGREPORT OFF CACHE BOOL "Disable unused wxDebugReport library" FORCE)
set(wxUSE_HTML OFF CACHE BOOL "Disable unused wxHTML library" FORCE)
set(wxUSE_MEDIACTRL OFF CACHE BOOL "Disable unused wxMediaCtrl library" FORCE)
set(wxUSE_OPENGL OFF CACHE BOOL "Disable unused wxGLCanvas library" FORCE)
set(wxUSE_PROPGRID OFF CACHE BOOL "Disable unused wxPropertyGrid library" FORCE)
set(wxUSE_RIBBON OFF CACHE BOOL "Disable unused wxRibbon library" FORCE)
set(wxUSE_RICHTEXT OFF CACHE BOOL "Disable unused wxRichText library" FORCE)
set(wxUSE_XRC OFF CACHE BOOL "Disable unused wxXRC library" FORCE)
set(wxUSE_XML OFF CACHE BOOL "Disable unused wxXML library" FORCE)
set(wxUSE_STC ON CACHE BOOL "Build wxStyledTextCtrl" FORCE)
set(wxUSE_WEBVIEW ON CACHE BOOL "Build wxWebView" FORCE)

if(APPLE)
    set(wxUSE_WEBVIEW_WEBKIT ON CACHE BOOL "Use WebKit for wxWebView" FORCE)
elseif(WIN32)
    set(wxUSE_WEBVIEW_EDGE ON CACHE BOOL "Use Edge WebView2 for wxWebView" FORCE)
    set(wxUSE_WEBVIEW_EDGE_STATIC ON CACHE BOOL "Link the WebView2 loader statically" FORCE)
    set(wxUSE_WEBVIEW_IE OFF CACHE BOOL "Disable the legacy IE wxWebView backend" FORCE)
endif()

# Use the full release archive rather than GitHub's automatically generated
# source archive: the release archive contains the Scintilla and Lexilla trees
# required by wxStyledTextCtrl.
FetchContent_Declare(
    wxwidgets
    URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.3.3/wxWidgets-3.3.3.tar.bz2
    URL_HASH SHA256=81b09d6dd9f1ed9301f8c55a968a488d0491f264dc2bab19a7e407ac67009482
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(wxwidgets)

# Keep the existing consumers independent of whether wxWidgets came from a
# system package or the bundled source build. Target usage requirements replace
# the flags formerly supplied by FindwxWidgets' wxWidgets_USE_FILE.
set(wxWidgets_LIBRARIES wx::core wx::base wx::stc wx::webview)
