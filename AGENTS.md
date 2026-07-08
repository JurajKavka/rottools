# AGENTS.md

## Language

This project is written in C++20.

## Build system

CMake monorepo (`rottools` suite). The umbrella [CMakeLists.txt](CMakeLists.txt) builds the
whole tree: shared code lives under `libs/` as `rottools::*` targets, each tool under
`apps/<tool>/` (currently `apps/rotreader`). The root `Makefile` wraps the common commands:

- `make build` — configure (`dev` preset, Ninja) and build everything
- `make run` — launch rotreader
- `make rebuild` — incremental build (no clean)
- `make dmg` — package rotreader via CPack
- `make check` — cppcheck static analysis over `libs` + `apps` (excludes wxFormBuilder-generated
  `*Wx.h`/`*Wx.cpp`; exits non-zero on any finding). Run it before committing.

Configure/build directly with presets when needed:

- `cmake --preset dev && cmake --build build`
- Build one shared component in isolation: `cmake --build build --target rottools_ui_webview`
- Build the per-library standalone smoke-test apps: `cmake -B build -DROTTOOLS_BUILD_LIB_APPS=ON`

Dependencies resolve from the system (Homebrew/apt) for the `dev` preset, or from vcpkg
([vcpkg.json](vcpkg.json)) for the `ci-macos`/`ci-linux`/`ci-windows` presets.

## Packaging (CPack)

Packaging is unified across macOS/Linux/Windows via CPack, driven by
[cmake/RotToolsPackaging.cmake](cmake/RotToolsPackaging.cmake) (`rottools_package_app()`),
called from each app's `CMakeLists.txt`. `cd build && cpack` (or `make dmg`) produces
`rotreader-<version>-<os>-<arch>.<ext>`:

- **macOS:** `.dmg` (DragNDrop) wrapping the `.app` bundle.
- **Linux:** `.deb` (Depends on `libwebkit2gtk-4.1-0`, `libgtk-3-0`) + portable `.tar.gz`.
- **Windows:** NSIS installer + portable `.zip`.

Version is single-sourced from `apps/<tool>/VERSION` and flows into a generated `version.h`,
the macOS `Info.plist` ([packaging/macos/Info.plist.in](apps/rotreader/packaging/macos/Info.plist.in)),
and the package name. The macOS `.app` icon is generated from
`apps/rotreader/assets/logo.png` (1024×1024 master) into `AppIcon.icns` at build time.

Two distinct names: `EXE_NAME` (`rotreader`) is the CMake target / binary / `CFBundleExecutable`;
`DISPLAY_NAME` (`ℜ⛤𝔗 reader`) is the user-facing `.app`/`CFBundleName`/`.dmg` volume name.
Bundle id `com.jurajkavka.rotreader`. To change any of these, edit the `rottools_package_app(...)`
call in [apps/rotreader/CMakeLists.txt](apps/rotreader/CMakeLists.txt).

macOS signing is still ad-hoc (CPack default); Developer-ID cert + notarization is a follow-up.
For a fully self-contained `.dmg` (deps statically linked, not against Homebrew), configure with
`--preset ci-macos` (vcpkg).

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
