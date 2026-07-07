# rottools

A suite (monorepo) of small native desktop utilities built with C++20 and
**wxWidgets**, sharing a set of common libraries. Each tool has its own version
and its own macOS / Linux / Windows builds, and is released independently.

## Tools

| Tool         | Description                                                        |
|--------------|-------------------------------------------------------------------|
| **rotreader** | A native Markdown viewer (drag & drop, live reload, source panels). Renders locally via native WebView. UI name: **ℜ⛤✝ reader**. |

## Repository layout

```
rottools/
  CMakeLists.txt         # umbrella build (selects tools via ROTTOOLS_BUILD_<TOOL>)
  CMakePresets.json      # `dev` (system deps) + `ci-macos/linux/windows` (vcpkg)
  vcpkg.json             # cross-platform dependency manifest (wxwidgets, md4c)
  cmake/                 # shared modules: version + CPack packaging
  libs/                  # SHARED, reusable libraries (rottools::* targets)
    HelperFunctions/         rottools::helpers
    backend/
      MarkdownToHtmlAsync/   rottools::md2html
      DirectoryScanner/      rottools::dirscan
    ui/
      WebViewPanel/          rottools::ui_webview
      HtmlSourcePanel/       rottools::ui_htmlsource
      MarkdownSourcePanel/   rottools::ui_mdsource
      FileBrowserTreePanel/  rottools::ui_filetree
      FileDropTarget/        rottools::ui_filedrop
  apps/
    rotreader/           # first tool: sources, VERSION, packaging/ inputs
```

## Build and run (local dev)

Uses system-provided wxWidgets (e.g. `brew install wxwidgets`) and fetches md4c.

```bash
cmake --preset dev              # configure (Ninja, ./build)
cmake --build build             # build everything
make run                        # launch rotreader (opens the .app on macOS)
```

Build a single shared component in isolation:

```bash
cmake --build build --target rottools_ui_webview
```

Build the optional per-library standalone smoke-test apps:

```bash
cmake -B build -DROTTOOLS_BUILD_LIB_APPS=ON && cmake --build build
```

## Package a distributable

```bash
make dmg                        # macOS .dmg via CPack (DragNDrop)
# or, on any OS, after configuring:
cd build && cpack               # -> rotreader-<version>-<os>-<arch>.<ext>
```

For a fully self-contained artifact (deps linked statically instead of against
Homebrew/apt), configure with the vcpkg preset for your OS, e.g.
`cmake --preset ci-macos` (requires `VCPKG_ROOT` set).

## Versioning & releases

Each tool's version lives in `apps/<tool>/VERSION` (single source of truth) and
flows into a generated `version.h`, the macOS `Info.plist`, and the CPack package
name. Releases are per-tool and tag-driven: push a tag like `rotreader-v0.1.0`
and the `Release rotreader` GitHub Actions workflow builds all three OSes and
publishes a GitHub Release. Other tools are untouched.

## Adding a new tool

1. Create `apps/<tool>/` with `main.cpp`, a `VERSION` file, and `packaging/`
   inputs (copy rotreader's as a starting point).
2. In `apps/<tool>/CMakeLists.txt`, link only the `rottools::*` libraries you
   need, then call `rottools_apply_version(...)` and `rottools_package_app(...)`.
3. Add a `ROTTOOLS_BUILD_<TOOL>` option in the umbrella `CMakeLists.txt` and an
   `add_subdirectory` guard in `apps/CMakeLists.txt`.
4. Add a `release-<tool>.yml` workflow and a `<tool>` path filter in `ci.yml`.
