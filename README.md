# ℜ⛤𝔗 tools

A suite (monorepo) of small native desktop utilities built with C++20 and
**wxWidgets**, sharing a set of common libraries. Each tool has its own version
and its own macOS / Linux / Windows builds, and is released independently.

## Tools

| Tool         | Description                                                        |
|--------------|-------------------------------------------------------------------|
| **rotreader** | A native Markdown viewer (drag & drop, live reload, source panels). Renders locally via native WebView. UI name: **ℜ⛤𝔗 reader**. |

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
`make help` prints the full menu; the common targets and their raw CMake
equivalents are:

| `make`         | Does                             | Equivalent CMake |
|----------------|----------------------------------|------------------|
| `make build`   | Configure + build the whole tree | `cmake --preset dev && cmake --build build` |
| `make run`     | Launch rotreader (`.app`)        | `open ./build/apps/rotreader/rotreader.app` |
| `make rebuild` | Incremental build                | `cmake --build build` |
| `make all`     | Clean + build + run              | `make clean build run` |
| `make dev`     | Rebuild + run (fast inner loop)  | `make rebuild run` |

Build/run a single shared component in isolation (`make help` lists them all):

| `make`               | Component            | Equivalent CMake |
|----------------------|----------------------|------------------|
| `make run-filetree`  | FileBrowserTreePanel | `cmake -B build -DROTTOOLS_BUILD_LIB_APPS=ON`<br>`cmake --build build --target rottools_ui_filetree_app`<br>`./build/libs/ui/FileBrowserTreePanel/rottools_ui_filetree_app` |
| `make build-webview` | WebViewPanel (lib only) | `cmake --build build --target rottools_ui_webview` |

The other demos follow the same pattern: `run-htmlsource`, `run-mdsource`,
`run-dirscan`, `run-md2html`, `run-helpers` (and `build-filedrop`).

## Package a distributable

| `make`     | Does                     | Equivalent CMake |
|------------|--------------------------|------------------|
| `make dmg` | macOS `.dmg` via CPack   | `cmake --build build && cd build && cpack -G DragNDrop` |

On any OS, once configured, `cd build && cpack` produces
`rotreader-<version>-<os>-<arch>.<ext>` (`.dmg` / `.deb`+`.tar.gz` / NSIS+`.zip`).

For a fully self-contained artifact (deps linked statically instead of against
Homebrew/apt), configure with the vcpkg preset for your OS, e.g.
`cmake --preset ci-macos` (requires `VCPKG_ROOT` set).

## Installing on macOS ("rotreader is damaged")

macOS refuses to launch a downloaded `.app` with:

> "rotreader" is damaged and can't be opened. You should move it to the Trash.

Nothing is damaged. The builds are ad-hoc signed (no Apple Developer ID, no
notarization), and anything a browser downloads gets tagged
`com.apple.quarantine`. A quarantined app that Gatekeeper can't verify produces
that misleading wording. Clear the tag once, after dragging the app out of the
`.dmg`:

```sh
xattr -dr com.apple.quarantine /Applications/rotreader.app
```

If it still refuses, re-apply an ad-hoc signature and clear the tag again:

```sh
codesign --force --deep --sign - /Applications/rotreader.app
xattr -dr com.apple.quarantine /Applications/rotreader.app
```

Two things that do *not* help: right-click → Open (that only covers the milder
"unidentified developer" case, and macOS 15 dropped it in favour of System
Settings > Privacy & Security > "Open Anyway"), and downloading from a GitHub
Release rather than a CI artifact — the `.dmg` is identical either way and
quarantine is applied on download.

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
