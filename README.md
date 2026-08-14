# ROT tools

A suite (monorepo) of small native desktop utilities built with C++20 and
**wxWidgets**, sharing a set of common libraries. Each tool has its own version
and its own macOS / Linux / Windows builds, and is released independently.

## Tools

| Tool         | Description                                                        |
|--------------|-------------------------------------------------------------------|
| **rotreader** | A native Markdown reader/editor (drag & drop, live reload, source panels). Renders locally via native WebView. UI name: **ROT reader**. |
| **rotpad**    | A small native plain-text editor inspired by classic Notepad. UI name: **ROT pad**. |

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
      TextEditorPanel/       rottools::ui_texteditor
      ScintillaTextEditorPanel/ rottools::ui_scintillatexteditor
      MarkdownEditorPanel/   rottools::ui_markdowneditor
      FileBrowserTreePanel/  rottools::ui_filetree
      FileDropTarget/        rottools::ui_filedrop
  apps/
    rotreader/           # Markdown reader: sources, VERSION, packaging inputs
    rotpad/              # plain-text editor: sources, VERSION, packaging inputs
      Makefile           # app-specific build/run/package workflow
      assets/            # generated icons eventually live under assets/icons/
  docs/graphics/         # icon masters (Inkscape) + README.md: how to change an icon
  scripts/               # generate-icons.sh and its helper
```

## Build and run (local dev)

Uses system-provided wxWidgets (e.g. `brew install wxwidgets`) and fetches md4c.
`make help` prints the full menu. Root targets operate on the complete suite;
app-prefixed targets use an isolated build directory and operate on one app:

| `make`                  | Does                                      |
|-------------------------|-------------------------------------------|
| `make build`            | Configure and build the complete suite    |
| `make rotreader-dev`    | Configure, build, and run only rotreader  |
| `make rotpad-dev`       | Configure, build, and run only rotpad     |
| `make rotpad-build`     | Configure and build only rotpad           |
| `make rotpad-package`   | Build and package only rotpad             |

Each app also has the same shorter workflow from its own directory, for example
`make -C apps/rotpad dev`, `make -C apps/rotpad build`, or
`make -C apps/rotpad package`. Isolated builds land in `build/<app>/`; the suite
build remains in `build/`.

Build/run a single shared component in isolation (`make help` lists them all):

| `make`               | Component            | Equivalent CMake |
|----------------------|----------------------|------------------|
| `make run-filetree`  | FileBrowserTreePanel | `cmake -B build -DROTTOOLS_BUILD_LIB_APPS=ON`<br>`cmake --build build --target rottools_ui_filetree_app`<br>`./build/libs/ui/FileBrowserTreePanel/rottools_ui_filetree_app` |
| `make build-webview` | WebViewPanel (lib only) | `cmake --build build --target rottools_ui_webview` |

The other demos follow the same pattern: `run-htmlsource`, `run-texteditor`,
`run-scintillatexteditor`, `run-dirscan`, `run-md2html`,
`run-helpers` (and `build-filedrop`). Build-only editor smoke targets are also
available as `build-texteditor` and `build-scintillatexteditor`.

## Icons

Each tool has one hand-drawn master SVG at `docs/graphics/<tool>/<tool>-logo.svg`.
Every platform icon is generated from it — macOS `.icns`, the Linux hicolor PNGs
and scalable SVG, the Windows `.ico`:

```sh
make rotreader-icons  # edit the master in Inkscape first, then rebuild as usual
```

The generated files live in `apps/<tool>/assets/icons/` and are committed, so the
Linux and Windows CI runners never have to rasterise anything. Never edit them by
hand. Full process and reference: [docs/graphics/README.md](docs/graphics/README.md).

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
4. Draw its icon: copy `docs/graphics/icon-template.svg` to
   `docs/graphics/<tool>/<tool>-logo.svg` and run
   `scripts/generate-icons.sh <tool>`. Missing icons only warn, so this can wait.
5. Add a `release-<tool>.yml` workflow and a `<tool>` path filter in `ci.yml`.
