# AGENTS.md

## Language

This project is written in C++20.

## Build system

Any directory containing a `Makefile` (e.g. the repo root, `Backend/DirectoryScanner`,
`Backend/MarkdownToHtmlAsync`, `Components/FileBrowserTreePanel`, `HelperFunctions`) is an
independently buildable and runnable component. `cd` into that directory and use:

- `make all` — clean, build, and run
- `make rebuild` — incremental build (no clean)
- `make run` — run the already-built binary
- `make dev` — rebuild and run (fast inner loop, no clean)

At the repo root, `make check` runs cppcheck static analysis (excludes wxFormBuilder-generated
`*Wx.h`/`*Wx.cpp` files; exits non-zero on any finding). Run it before committing.

Use these `make` targets only; do not invoke `cmake` directly.

## macOS packaging (`.app` / `.dmg`)

`make dmg` at the repo root builds the distributable `ℜ⛤✝ reader.app` and a compressed
`rotreader-0.1.0.dmg` into `dist/`. It runs [scripts/make-dmg.sh](scripts/make-dmg.sh),
which uses only built-in OS tools (`otool`, `install_name_tool`, `codesign`, `hdiutil`,
`sips`, `iconutil`) — no CMake bundle target, no extra dependencies. Phases:

1. Assembles the `.app` skeleton and writes `Contents/Info.plist` by hand (there is no
   checked-in plist — it is generated from variables at the top of the script). Two distinct
   name variables: `EXE_NAME` (`rotreader`) is the CMake target / binary / `CFBundleExecutable`
   and must match the file CMake produces in `build/` (i.e. the `project()`/`add_executable()`
   name in [CMakeLists.txt](CMakeLists.txt) and the `make run` path in the Makefile);
   `APP_NAME` (`ℜ⛤✝ reader`) is the user-facing name used for the `.app` bundle dir,
   `CFBundleName`/`CFBundleDisplayName`, and the mounted `.dmg` volume. The downloadable `.dmg`
   file itself stays ASCII: `rotreader-0.1.0.dmg`. Other variables: `VERSION`, `BUNDLE_ID`
   (`com.jurajkavka.rotreader`).
2. Copies every non-system dylib the binary links against into `Contents/Frameworks` and
   rewrites the Mach-O install names to `@executable_path/../Frameworks/...`.
3. Ad-hoc code-signs (`codesign --sign -`) — enough to run on Apple Silicon, but NOT
   Gatekeeper-clean on other machines (needs a Developer ID cert + notarization).
4. Packs the `.dmg` with the "drag to Applications" symlink.

**App icon:** generated on every `make dmg` from `assets/logo.png` (1024×1024 master) into
`Contents/Resources/AppIcon.icns` via `sips` + `iconutil`, and referenced by the
`CFBundleIconFile` key in the generated Info.plist. No `.icns` is committed. To change the
icon, replace `assets/logo.png` and rebuild. To bump the version or bundle id, edit the
variables at the top of `scripts/make-dmg.sh`.

## Naming conventions

Event/callback handler functions are named `Handle*` (e.g. `HandleMarkdownError`), never
`On*`. The `On` prefix is reserved for the assignment points: wxWidgets framework virtual
overrides (`OnInit`, `OnExit`, `OnDropFiles` — keep their framework names) and members
holding callbacks (`m_onFileOpened`). Any function bound with `Bind()`, passed as a C
callback, or stored in a `std::function` must start with `Handle`.

## wxFormBuilder files — hands off

Never modify `*.fbp` files or the generated `*Wx.h`/`*Wx.cpp` files. The `.fbp` projects are
edited exclusively by the user in wxFormBuilder, and the `*Wx.*` sources are generated from
them. If a change is needed there (renaming a class, adding a widget or a menu item), ask the
user to make it in wxFormBuilder; put all hand-written code in the non-`Wx` subclasses
(e.g. `WebViewPanel` subclasses `WebViewPanelWx`).
