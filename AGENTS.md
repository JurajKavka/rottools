# AGENTS.md

## Language

This project is written in C++20.

## Repository map

- `apps/rotreader/` and `apps/rotpad/` contain the desktop applications. Each app owns its
  sources, wxFormBuilder project, `VERSION`, packaging inputs, generated icons, and thin Makefile.
- `libs/` contains reusable C++ libraries exposed as `rottools::*` CMake targets.
- `make/App.mk` implements the shared per-app build, run, clean, and package workflow used by both
  app Makefiles.
- `cmake/` contains shared version, icon, vcpkg-triplet, and CPack packaging support.
- `scripts/` contains release/branch automation and icon generation tools. Prefer their root
  `Makefile` wrappers where one exists.
- `docs/graphics/` contains the hand-edited icon masters and graphics documentation.
- `www/` is the static Astro download/marketing website, separate from the C++ build.
- `.github/workflows/` contains path-filtered CI, per-tool release workflows, and Pages deployment.

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

Release helpers are not exceptions to these rules. `make bump-version` creates a commit and pushes
a branch; `make release-tag` creates and pushes a tag. Run either only when the user explicitly asks
for that exact version/release operation. Approval to bump a version does not authorize publishing
the later release tag.

## Native build and local development

Do not run configure, build, run, package, or development-server commands without the user's
explicit approval. This includes `cmake`, the native `make` targets below, and the `npm` commands in
the website section. When one would help, first state the exact command and why it is needed, then
wait for approval. Approval is scoped to the command or verification described in that request and
does not carry over to later commands. Without approval, leave execution to the user.

CMake monorepo (`rottools` suite). The umbrella [CMakeLists.txt](CMakeLists.txt) builds the
whole tree: shared code lives under `libs/` as `rottools::*` targets, and applications live under
`apps/<tool>/` (currently `rotreader` and `rotpad`). Use the app-specific targets in the root
`Makefile` for normal development; do not substitute suite-wide or direct CMake commands when the
user asks to build or run one application. Read `make help`, the root `Makefile`, and `make/App.mk`
instead of inferring target names or behavior.

Local native builds require CMake 3.24+, Ninja, and system wxWidgets with `core`, `base`, `stc`, and
`webview` components. The local Make targets use system dependencies (Homebrew/apt); md4c can be
fetched by CMake. CI/release presets use the dependency strategy recorded in `CMakePresets.json` and
`vcpkg.json`.

For `rotreader`:

- `make rotreader-all` — **standard from-scratch workflow**: clean only `build/rotreader`,
  configure and build `rotreader` with its required dependencies, then run it in the foreground.
  When the user asks to build `rotreader` "from scratch", use this exact command.
- `make rotreader-dev` (or `make rotreader`) — configure, incrementally build, and run
  `rotreader` in the foreground. Use this for the normal edit/build/run cycle.
- `make rotreader-build` — configure and build only `rotreader` without launching it.
- `make rotreader-rebuild` — rebuild the existing isolated configuration without launching it.
- `make rotreader-run` / `make rotreader-run-fg` — run the already-built application.
- `make rotreader-package` — build and package only `rotreader` via CPack.
- `make rotreader-clean` — remove only the isolated `build/rotreader` tree.

For `rotpad`, use the corresponding targets with the same meanings:

- `make rotpad-all` — **standard from-scratch workflow**: clean, configure, build, and run
  `rotpad` in the foreground.
- `make rotpad-dev` (or `make rotpad`) — configure, incrementally build, and run `rotpad`.
- `make rotpad-build`, `make rotpad-rebuild`, `make rotpad-run`, `make rotpad-run-fg`, and
  `make rotpad-package` — build, rebuild, run, or package only `rotpad`, respectively.
- `make rotpad-clean` — remove only the isolated `build/rotpad` tree.

Suite-wide and quality commands:

- `make help` — print the authoritative list of root targets and their current descriptions.
- `make build` — configure and build the complete suite.
- `make rebuild` — incrementally build the configured complete suite.
- `make format` — run clang-format over hand-written C++ only; generated `*Wx.*` files are skipped.
- `make check` — cppcheck static analysis over `libs` + `apps` (excludes wxFormBuilder-generated
  `*Wx.h`/`*Wx.cpp`; exits non-zero on any finding). Run it before committing.

Isolated app builds land in `build/<tool>/`; the suite build lands in `build/`. The app `*-all` and
`*-dev` targets run the executable in the foreground after building, so they remain attached until
the application exits. Use direct CMake commands only for CI parity, low-level diagnosis, or a
shared-library target not covered by a Make target. Relevant examples are:

- `cmake --preset dev && cmake --build --preset dev`
- Build one shared component in isolation: `cmake --build build --target rottools_ui_webview`
- Build the per-library standalone smoke-test apps: `cmake -B build -DROTTOOLS_BUILD_LIB_APPS=ON`

The root Makefile also wraps shared-component smoke apps (`run-flatbuttons`, `run-headerpanel`,
`run-filetree`, `run-htmlsource`, `run-markdownpreview`, `run-texteditor`, `run-scintillatexteditor`,
`run-textfilepreviewdialog`, `run-dirscan`, `run-md2html`, and `run-helpers`) plus build-only
component targets. Prefer those named targets over reconstructing their configure/build/run
sequences.

## Adding a component under `libs/`

- Put the component's hand-written sources, public headers, `CMakeLists.txt`, and `main.cpp` in its
  own directory under the appropriate `libs/` area. Register that directory with
  `add_subdirectory()` in its parent `CMakeLists.txt`.
- Define a library target with a `rottools::<name>` alias, expose the public include directory,
  require C++20, and link its dependencies through CMake. Keep the component usable without
  linking either desktop application.
- Every new component must have a `main.cpp` with a meaningful, standalone smoke test. It must
  provide its own entry point (or `wxApp` for a GUI component), instantiate or call the component,
  and exercise its main behavior with visible output or interaction. It must compile as a separate
  executable using the component library and its declared dependencies, without an app target.
- In the component's `CMakeLists.txt`, add that executable inside
  `if(ROTTOOLS_BUILD_LIB_APPS)` and link it to the component's `rottools::*` target. Use the
  `rottools_<name>_app` target convention so the demo can be built independently.
- Add a root `Makefile` target that configures demos through `_demos`, builds only this executable,
  and runs it from its path under `build/libs/`. Add the target to `.PHONY` and give its rule an
  inline `##` description: `make help` lists targets from those descriptions. Check that the
  component's command appears in `make help` and points to the correct executable. Follow the
  approval rule above before running configure, build, or demo commands.

## CI and verification

`.github/workflows/ci.yml` runs the reusable three-platform build on pushes to `main` and on manual
dispatch, not on ordinary feature-branch pushes or pull requests. Path filters build both apps for
shared `libs/`, `cmake/`, or top-level build changes, and only the affected app for app-local
changes. CI artifacts are temporary test installers; they are not GitHub Releases.

Before handing off C++ changes, run `make check` and the relevant app/component build only when the
user has approved those exact commands. Before handing off website changes, the relevant checks are
`npm run check`, `npm run build`, and `npm run preview`, again only with approval. Report clearly
which commands were and were not run; a successful check/build never authorizes a commit or push.

## Packaging (CPack)

Packaging is unified across macOS/Linux/Windows via CPack, driven by
[cmake/RotToolsPackaging.cmake](cmake/RotToolsPackaging.cmake) (`rottools_package_app()`),
called from each app's `CMakeLists.txt`. Use `make rotreader-package` or `make rotpad-package`;
packages are named `<tool>-<version>-<os>-<arch>.<ext>`:

- **macOS:** `.dmg` (DragNDrop) wrapping the `.app` bundle.
- **Linux:** `.deb` (Depends on `libwebkit2gtk-4.1-0`, `libgtk-3-0`) + portable `.tar.gz`.
- **Windows:** NSIS installer + portable `.zip`.

Version is single-sourced from `apps/<tool>/VERSION` and flows into a generated `version.h`,
the macOS `Info.plist` ([packaging/macos/Info.plist.in](apps/rotreader/packaging/macos/Info.plist.in)),
the package name, and the website download data. Do not maintain a second website version.

Each app has two names: `EXE_NAME` is its CMake target/binary/`CFBundleExecutable`, while
`DISPLAY_NAME` is the user-facing app/package name. They are `rotreader` / `ROT Reader` and
`rotpad` / `ROT Pad`; their bundle IDs are `com.jurajkavka.rotreader` and
`com.jurajkavka.rotpad`. Change these only in the app's `rottools_package_app(...)` call.

The packaging module explicitly ad-hoc signs macOS development and installed bundles; there is no
Developer ID certificate or notarization yet. For a fully self-contained `.dmg` (dependencies from
vcpkg rather than Homebrew), use the `ci-macos` preset or the official release workflow.

## Version bumps and releases

Versions and releases are independent per tool. Substitute `rotreader` or `rotpad` for `<tool>`.
Use the provided automation rather than manually editing `VERSION`, constructing release branches,
or creating tags.

To prepare a version bump from a clean, fully synchronized `main`:

```sh
make bump-version TOOL=<tool> VERSION=<X.Y.Z>
```

The command validates a strictly greater semantic version, fetches `origin/main`, and refuses to
continue unless local `main` exactly matches it. It then creates `<tool>-<X.Y.Z>`, updates only
`apps/<tool>/VERSION`, commits it as `bump <X.Y.Z>`, and pushes the new branch to `origin`. It does
not open or merge a pull request. Never use this command while already working on a feature branch.

After that branch has been reviewed and merged through a pull request, publish the release from a
clean working tree with:

```sh
make release-tag TOOL=<tool>
```

There is deliberately no `VERSION=` argument: the script switches to `main`, fast-forwards it from
`origin/main`, reads the merged `apps/<tool>/VERSION`, verifies that
`.github/workflows/release-<tool>.yml` exists, creates annotated tag `<tool>-v<X.Y.Z>`, and pushes
that tag. The tag push is the release trigger, so state these side effects and obtain explicit user
approval before running it.

The release workflow verifies that the tag matches the app's `VERSION`, builds/packages the app on
macOS, Linux, and Windows, and publishes the installers as a GitHub Release. `rotreader` additionally
publishes its dedicated macOS 26 build. Only after the GitHub Release exists does the workflow
redeploy the website, keeping its version and download links synchronized with available assets.

`make cleanup-branches` switches to and fast-forwards `main`, lists local and remote branches that
are already merged into `origin/main`, asks for confirmation, then deletes them. It is destructive;
run it only when the user explicitly requests branch cleanup. The direct
`scripts/cleanup-merged-branches.zsh --yes` form skips confirmation and requires equally explicit
authorization.

## Scripts

- `scripts/bump-version.zsh` — implementation behind `make bump-version`; creates, commits, and
  pushes a per-tool version branch.
- `scripts/release-tag.zsh` — implementation behind `make release-tag`; updates local `main` and
  creates/pushes the per-tool release tag.
- `scripts/cleanup-merged-branches.zsh` — guarded cleanup of branches already merged to main.
- `scripts/generate-icons.sh` — regenerates platform and/or website assets from an SVG master. Use
  `make rotreader-icons`, `make rotpad-icons`, or `make web-icons` for normal work.
- `scripts/icon-tools.py` — internal helper for SVG variants, background-color extraction, and ICO
  packing; it is normally invoked only by `generate-icons.sh`.

## Website (`www/`)

`www/` is an Astro 5 static site. Production runs no Node server: `npm run build` writes static
HTML/CSS/JS to `www/dist/`, and GitHub Pages publishes that directory. CI uses Node 22. Run website
commands from `www/` (or use `npm --prefix www ...` from the repository root):

- `npm ci` — install the exact dependency versions from `package-lock.json`.
- `npm run dev` — start the live-reload site at `http://localhost:4321/`.
- `npm start` — start the dev server and open it in a browser.
- `npm run check` — type-check `.astro` and TypeScript sources.
- `npm run build` — create the production site in the ignored `www/dist/` directory.
- `npm run preview` — serve the built output with the production `/rottools` base path. Use this
  after changes to markup or asset URLs, because dev mode serves at `/` and cannot expose every
  production-subpath mistake.

Important website ownership rules:

- `www/src/data/tools.ts` owns tool marketing metadata and release asset URL construction, but
  imports versions directly from `apps/<tool>/VERSION`. Never hard-code a duplicate version there.
- `www/public/` contains generated logo/favicon assets. Do not hand-edit them; regenerate them with
  `make web-icons`. If a tool icon changed, regenerate that tool's app icons first so `web-icons`
  copies the current scalable mark.
- The production site is served at `/rottools`. Route every hand-written `public/` asset path
  through `asset()`/`absoluteAsset()` in `www/src/lib/url.ts`; root-absolute paths can work in dev
  and fail after deployment.
- Pushes to `main` that touch `www/**` or the Pages workflow deploy the site. App `VERSION` changes
  intentionally do not deploy it early; the corresponding release workflow deploys only after the
  release assets have been published.

## Icons

Each tool has one hand-drawn master SVG at `docs/graphics/<tool>/<tool>-logo.svg`, copied from
the shared [docs/graphics/icon-template.svg](docs/graphics/icon-template.svg). Every platform
icon comes from it:

```
make rotreader-icons
make rotpad-icons
make web-icons
```

Output lands in `apps/<tool>/assets/icons/` (`.icns`, hicolor PNGs + scalable SVG, `.ico`) and
**is committed** — the Linux and Windows CI runners have no Inkscape, so the build only ever
reads finished files. Never hand-edit anything under `assets/icons/`; edit the master SVG in
Inkscape and re-run the matching Make target. Icon generation requires Inkscape 1.x and Python 3;
macOS `.icns` generation additionally uses `iconutil`. Full reference:
[docs/graphics/README.md](docs/graphics/README.md).

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
