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

## Installing on macOS

The builds are ad-hoc signed — there is no paid Apple Developer ID and no
notarization — so the first launch is blocked. macOS says it cannot verify the
developer. Let it through once:

1. Open the `.dmg` and drag **rotreader** to Applications.
2. Double-click it. macOS offers *Move to Trash* or *Done* — press **Done**.
   This step is required: the Open Anyway button in the next step does not appear
   until the app has been blocked once and the dialog dismissed.
3. Open **System Settings > Privacy & Security** and scroll to *Security*. A line
   about rotreader being blocked now has an **Open Anyway** button.
4. Click it, authenticate, then confirm **Open**.

macOS remembers the decision; later launches are normal. Right-click > Open no
longer works — macOS 15 replaced it with the Privacy & Security route above.

From a terminal the same thing is one command, run after dragging the app across:

```sh
xattr -dr com.apple.quarantine /Applications/rotreader.app
```

### If it says "damaged" instead (it should not happen)

> "rotreader" is damaged and can't be opened. You should move it to the Trash.

That wording means the code signature is *invalid*, not merely unrecognized, and
there is no Open Anyway button for it. It should not happen: the packaging
re-signs the bundle after install for exactly this reason (see
[cmake/RotToolsPackaging.cmake](cmake/RotToolsPackaging.cmake)) because
`CPACK_STRIP_FILES` and install-time rpath rewriting both invalidate the ad-hoc
signature the linker applies to arm64 binaries. If a build ever reports it
again, the signature broke somewhere after that step:

```sh
codesign -vvv --deep --strict /Applications/rotreader.app
```

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
