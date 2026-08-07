# AGENTS.md

## Language

This project is written in C++20.

## Platform-native UI — discuss exceptions

Prefer platform-native controls, dialogs, appearance, and behavior. It is acceptable—and often
desirable—for the application to look or behave slightly differently on macOS, Linux, and Windows.
Do not normalize system selection colours, fonts, shortcuts, dialog conventions, or similar UI
details merely to make the platforms look identical. When a custom control is already justified,
prefer its platform defaults and system-derived settings over hard-coded styling.

This is a design preference, not an absolute prohibition. A custom or cross-platform solution may
still be the right choice for performance, functionality, or maintainability (for example,
`wxStyledTextCtrl` for editor capabilities). Before introducing custom behavior, replacing a native
solution, or deliberately normalizing behavior across platforms, discuss the trade-offs with the
user and get direction. Do not silently reverse an established decision solely because of this
preference.

## Git — never commit or push on your own

Never run `git commit` or `git push` unless the user asks for it in that message. Make the
edits, then stop and report what changed and why. The user reviews the diff first and decides
when it gets committed. "Fix X" is a request to change files, not a request to commit them.
This also applies after a green pipeline or a successful build — those are not approval.

**Never push to `main`.** Not directly, not via `git push origin HEAD:main`, not with a force
push, and not even when the user says "commit and push" — that means the branch that is
currently checked out, never `main`. `main` is protected and takes changes only through a pull
request. If a plan you wrote mentions pushing to `main` and the user approves the plan, that is
still not approval to push to `main`; push the current branch and open a PR instead. If a push
prints a "Bypassed rule violations" warning, stop and report it rather than continuing.

## Build system

Do not run `cmake`, `make build`, `make rebuild`, or any other configure/build command without
the user's explicit approval. When a build would help, first state the exact command and why it is
needed, then wait for approval before running it. Approval is scoped to the command or verification
described in that request and does not carry over to later builds. Without approval, leave building
and verification to the user. The commands below document the project for reference.

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

Dependencies resolve from the system (Homebrew/apt) for the `dev` and `ci-linux`
presets. The `ci-macos` and `ci-windows` presets build the pinned wxWidgets release
declared in [cmake/RotToolsWxWidgets.cmake](cmake/RotToolsWxWidgets.cmake).

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
and the package name.

Two distinct names: `EXE_NAME` (`rotreader`) is the CMake target / binary / `CFBundleExecutable`;
`DISPLAY_NAME` (`ℜ⛤𝔗 reader`) is the user-facing `.app`/`CFBundleName`/`.dmg` volume name.
Bundle id `com.jurajkavka.rotreader`. To change any of these, edit the `rottools_package_app(...)`
call in [apps/rotreader/CMakeLists.txt](apps/rotreader/CMakeLists.txt).

macOS signing is still ad-hoc (CPack default); Developer-ID cert + notarization is a follow-up.
For a fully self-contained `.dmg` (wxWidgets statically linked, not against Homebrew), configure
with `--preset ci-macos`.

## Icons

Each tool has one hand-drawn master SVG at `docs/graphics/<tool>/<tool>-logo.svg`, copied from
the shared [docs/graphics/icon-template.svg](docs/graphics/icon-template.svg). Every platform
icon comes from it:

```
scripts/generate-icons.sh rotreader --name "ROT Reader"   # or: make icons
```

Output lands in `apps/<tool>/assets/icons/` (`.icns`, hicolor PNGs + scalable SVG, `.ico`) and
**is committed** — the Linux and Windows CI runners have no Inkscape, so the build only ever
reads finished files. Never hand-edit anything under `assets/icons/`; edit the master SVG in
Inkscape and re-run the script. Full reference: [docs/graphics/README.md](docs/graphics/README.md).

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
