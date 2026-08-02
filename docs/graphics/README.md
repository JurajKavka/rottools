# Icons

Every tool in the monorepo has **one** hand-drawn master SVG. One script turns
it into every platform icon; the build only ever reads finished files.

```
docs/graphics/
  README.md                             this file
  icon-template.svg                     the shared Inkscape template
  <name>/<name>-logo.svg                one master (copy of the template)
    rotreader/rotreader-logo.svg          the rotreader app  — blue  #4d4dff
    rottools/rottools-logo.svg            the suite logo     — grey  #424242

scripts/
  generate-icons.sh <name>              the generator
  icon-tools.py                         its helpers (square variant, .ico, bg colour)

apps/<name>/assets/icons/               generated, committed to git
  macos/AppIcon.icns
  linux/<name>-{16,22,24,32,48,64,128,256,512}.png
  linux/<name>.svg
  windows/<name>.ico
  <name>-1024.png                       square master, for READMEs and og:image
  web/…                                 only with --web
```

Needs **Inkscape 1.x**, **python3**, and **iconutil** (Xcode Command Line Tools)
for the `.icns`. ImageMagick is *not* needed — `icon-tools.py` writes the `.ico`
itself. The script puts `/Applications/Inkscape.app/Contents/MacOS` on `PATH`
for you.

## Two kinds of target

| | Master | Produces | Command |
|---|---|---|---|
| **An app** (`rotreader`) | `docs/graphics/rotreader/` | `.icns`, `.ico`, hicolor set → `apps/rotreader/assets/icons/` | `make icons` |
| **The suite** (`rottools`) | `docs/graphics/rottools/` | favicon set + square PNGs + every tool's mark → `www/public/` | `make web-icons` |

The suite has no binary, so it gets no `.icns`, `.ico` or hicolor set — there is
nothing to package. The script detects that from the absence of `apps/<name>/`,
says so, and skips those steps.

---

# Changing an icon

## 1. Edit the master

Open `docs/graphics/<app>/<app>-logo.svg` in Inkscape — for rotreader that is
[rotreader/rotreader-logo.svg](rotreader/rotreader-logo.svg). It is the only
file you edit by hand.

- Draw inside the layer **"02 - Icon (design here)"**, in the group
  `id="iconClipped"`. Layer 01 (guides) and layer 03 (size previews) are
  reference only and are never exported.
- Stay inside the dashed safe zone. The squircle is 824×824 in a 1024 canvas,
  and Apple requires that 100px margin.
- Layer 03 shows the icon live at 512 / 256 / 128 / 64 / 32 / 16 px. Check the
  16px preview before you regenerate — that is where detail disappears.

## 2. Regenerate

```bash
make icons     # = ./scripts/generate-icons.sh rotreader --name "ROT Reader"
```

Everything under `apps/<app>/assets/icons/` is overwritten. Never edit files in
there by hand.

## 3. Rebuild

```bash
make rebuild && make run
```

An incremental build is enough. The `.icns` is a source file of the target, and
the PNGs compiled into the binary (§5) are registered as configure dependencies,
so changing them makes CMake re-run by itself.

### If the icon still looks old

That is macOS caching it, not the build. Finder and the Dock re-read a bundle's
icon only when the **bundle's own modification date** changes — and an
incremental build replaces `Contents/Resources/AppIcon.icns` deep inside the
bundle without touching the `.app` directory itself. `CFBundleVersion` is
unchanged too, so nothing tells LaunchServices to look again. That is why a full
`make all` (which deletes `build/`) used to be the only thing that worked.

The packaging now runs `touch` on the `.app` after every build to close that
gap. If a stale icon ever survives it anyway:

```sh
killall Finder && killall Dock
```

Bumping `apps/<app>/VERSION` also clears it, and so does `make all`. To confirm
the build itself is fine, compare the two files — they must match:

```sh
shasum -a256 apps/rotreader/assets/icons/macos/AppIcon.icns \
             build/apps/rotreader/rotreader.app/Contents/Resources/AppIcon.icns
```

## 4. Commit both

The master SVG **and** `apps/<app>/assets/icons/`.

## Adding a tool's first icon

Same flow, one extra step at the front:

```bash
mkdir -p docs/graphics/mytool
cp docs/graphics/icon-template.svg docs/graphics/mytool/mytool-logo.svg
# design it, then
scripts/generate-icons.sh mytool --name "My Tool"
```

`rottools_package_app()` picks the icons up through its `ICON_DIR` argument.

## Why the output is committed

The Linux and Windows CI runners have no Inkscape. If the build rasterised the
SVG, those platforms could not package at all. So generation is a manual step
run on a workstation, and `apps/<app>/assets/icons/` is checked in. CMake warns
(it does not fail) when a file is missing, so a new tool can build before it has
artwork.

---

# Reference

## 1. The master SVG

- Design inside Inkscape **Layer 02 — Icon (design here)**.
- The export target is the group `id="iconExport"`. It must stay **outside
  `<defs>`** — Inkscape does not render `<defs>` content, so exporting from
  there yields blank PNGs.
- `id="exportSpacer"` is a transparent 1024×1024 rect. It forces the export
  bounding box to the full canvas so the icon keeps the 100px margin around the
  squircle. **Do not delete.**
- `id="iconClipped"` is clipped to the squircle path. Artwork goes inside it.
- `id="squircleBase"` is the opaque white backing under the artwork, so
  partially transparent colours composite predictably.

Those four ids must survive every edit. Inkscape strips ids on ungroup/regroup;
restore them via Edit → XML Editor (`Ctrl+Shift+X`). The generator checks all
four and stops with a message naming the missing one, so bad output cannot slip
through.

### Platform shape rule

| Platform | Shape | Notes |
|---|---|---|
| macOS | Squircle, 824×824 inside 1024 canvas | Apple HIG shape; the margin is required |
| Linux (GTK/freedesktop) | **Square, no mask** | The desktop does not clip; a baked-in squircle looks wrong |
| Windows | **Square, no mask** | Transparency is fine; rounding is convention, not required |
| Web | **Square, no mask** | Browser/OS applies its own rounding |

Only macOS uses the squircle. For the other three the script builds a **square
variant** of the master in a temp file, by two edits:

1. `iconClipped` loses its `clip-path`, and
2. `squircleBase` — a squircle-shaped `<use>` — becomes a rect covering the
   whole canvas.

Step 2 matters. Without it the semi-transparent background composites over white
inside the squircle and over nothing outside it, and the square icon ends up
with a visible squircle-shaped ghost in the corners.

---

## 2. macOS — `.icns` + app bundle + DMG

`generate-icons.sh` renders each `.iconset` member from the SVG at its own
resolution (sharper than downscaling one 1024px PNG) and runs
`iconutil -c icns`.

Members: `icon_16x16`, `icon_16x16@2x`, `icon_32x32`, `icon_32x32@2x`,
`icon_128x128`, `icon_128x128@2x`, `icon_256x256`, `icon_256x256@2x`,
`icon_512x512`, `icon_512x512@2x` (= 1024px).

The build side is wired in
[cmake/RotToolsPackaging.cmake](../../cmake/RotToolsPackaging.cmake): the
committed `AppIcon.icns` is added to the target with

```cmake
set_source_files_properties("${_icns}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
```

and `packaging/macos/Info.plist.in` names it:

```xml
<key>CFBundleIconFile</key>
<string>AppIcon</string>
```

The app icon inside the DMG comes from the bundle — nothing extra is needed. A
custom *volume* icon (the mounted drive itself) is not set up; it would need
either `create-dmg --volicon` or, on a mounted read-write DMG before
`hdiutil convert`:

```bash
cp apps/rotreader/assets/icons/macos/AppIcon.icns /Volumes/<vol>/.VolumeIcon.icns
SetFile -c icnC /Volumes/<vol>/.VolumeIcon.icns
SetFile -a C /Volumes/<vol>
```

---

## 3. Linux — freedesktop `hicolor` theme

The square PNGs and the scalable SVG install into the theme:

```
share/icons/hicolor/16x16/apps/<app>.png
share/icons/hicolor/22x22/apps/<app>.png
…24, 32, 48, 64, 128, 256, 512…
share/icons/hicolor/scalable/apps/<app>.svg
share/applications/<app>.desktop
```

`packaging/linux/app.desktop.in` carries the bare icon name — no path, no
extension — and the theme resolves it:

```ini
Icon=@ROTTOOLS_APP_EXE_NAME@
```

That `.desktop` entry is also how GNOME on Wayland picks the window icon; it
ignores what the toolkit sets at runtime. After installing by hand:

```bash
gtk-update-icon-cache /usr/share/icons/hicolor/
update-desktop-database
```

---

## 4. Windows — `.ico` + resource script

The `.ico` holds 16, 24, 32, 48, 64, 128 and 256 px. Entries are stored as PNG,
which Windows has read since Vista.

`packaging/windows/app.rc.in` is configured into the build tree so it can name
the `.ico` by absolute path:

```rc
IDI_APPICON ICON "@ROTTOOLS_APP_ICO@"
#include "wx/msw/wx.rc"
```

`IDI_APPICON` must stay first — Explorer and the taskbar show the icon that
sorts first in the resource table. `wx/msw/wx.rc` brings in wxWidgets' own
cursors and `wxSTD_*` icons; its app manifest is opt-in via `wxUSE_RC_MANIFEST`,
which stays off so it cannot clash with the linker-generated one.

The root [CMakeLists.txt](../../CMakeLists.txt) calls `enable_language(RC)` on
Windows, because `project(... LANGUAGES CXX)` alone leaves `.rc` unhandled.

---

## 5. Runtime window icon

macOS reads the window and Dock icon from `AppIcon.icns` in the `.app` bundle.
Windows and X11 need it set explicitly, and there is no single file path that
works for a `.deb`, a portable `.tar.gz` and a plain build tree at once — so the
PNGs are compiled into the executable instead.

`rottools_embed_window_icon()`
([cmake/RotToolsIcons.cmake](../../cmake/RotToolsIcons.cmake)) writes
`AppIconData.{h,cpp}` into the target's build dir holding the 16–256 px PNG
bytes. The frame then does:

```cpp
#include "AppIcon.h"
#include "AppIconData.h"

#ifndef __WXOSX__
    SetIcons(rottools::MakeIconBundle(kAppIconPngs, kAppIconPngCount));
#endif
```

`rottools::MakeIconBundle` lives in `rottools::helpers`
([libs/HelperFunctions/AppIcon.cpp](../../libs/HelperFunctions/AppIcon.cpp)).
The bytes are read at **configure** time, so CMake refreshes the generated
source when a PNG changes.

---

## 6. Web — favicon set

Off by default. Two ways to get it:

- `--web` writes it into `<app>/assets/icons/web/`, alongside the app icons.
- `--web-out <dir>` writes it **flat** into a directory you name — how the
  website gets its icons:

```bash
make web-icons
# = scripts/generate-icons.sh rottools --name "ROT Tools" --web-out www/public
```

| File | Size | Purpose |
|---|---|---|
| `favicon.ico` | 16/32/48 | Legacy + browser tab fallback |
| `favicon-16x16.png` | 16 | Tab icon |
| `favicon-32x32.png` | 32 | Tab icon (HiDPI) |
| `favicon-48x48.png` | 48 | Packed into `favicon.ico` |
| `apple-touch-icon.png` | 180 | iOS home screen |
| `android-chrome-192x192.png` | 192 | Android / PWA |
| `android-chrome-512x512.png` | 512 | PWA splash |
| `<name>-logo.svg` | — | Scalable tab icon |
| `<name>-logo-{256,512,1024}.png` | 256/512/1024 | Square images for page content |
| `site.webmanifest` | — | PWA manifest |
| `<tool>-rottools-logo.svg` | — | One per tool — see below |

### Per-tool marks

`--web-out` also copies every app's scalable icon into the same directory as
`<tool>-rottools-logo.svg`, so the site can show each tool's own mark on its
download card. Today that is `rotreader-rottools-logo.svg`, copied byte-for-byte
from `apps/rotreader/assets/icons/linux/rotreader.svg`. A new tool appears here
automatically once its icons have been generated; one that has not been
generated yet is reported as skipped rather than silently left out.

The **square** (unmasked) variant is used on purpose: a page can round it with
CSS `border-radius`, but it cannot un-round a baked-in squircle.

`theme_color` in the manifest is not hardcoded: `icon-tools.py bg-color` reads
the master's background and composites it over the white base, so each target's
manifest matches its own icon (`#424242` for rottools, `#4d4dff` for rotreader).

A `--web-out` directory belongs to the website, so unlike an app's icon
directory it is **never wiped** — only the files in that table are replaced, and
each one is printed as it is written.

The layout head is already wired in
[www/src/layouts/Layout.astro](../../www/src/layouts/Layout.astro).

### og:image

`og:image` points at the square `rottools-logo-1024.png`. Social cards want
**1200×630**; a square image gets cropped or letterboxed. Drop a purpose-made
`og-image.png` into `www/public/` and update that one `<meta>` — the generator
does not produce it, because a good social card is a layout (mark + wordmark +
padding), not a scaled icon.

Everything in `www/public/` is generated by `make web-icons`. Nothing there is
hand-made, so it is safe to delete the directory's contents and regenerate.

---

## 7. Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| Exported PNG is blank/empty | Export target is inside `<defs>` (never rendered) | Move the group onto a real layer |
| Export includes guides + preview strip | No `--export-id`, so the whole page rasterized | Add `--export-id=iconExport --export-id-only` |
| Icon fills edge-to-edge, no margin | `exportSpacer` deleted, bbox shrank to the squircle | Restore the transparent 1024×1024 rect |
| `inkscape not found on PATH` | The CLI binary lives inside the `.app` | `export PATH="/Applications/Inkscape.app/Contents/MacOS:$PATH"` |
| Artwork missing after an Inkscape edit | `id` stripped by ungroup/regroup | Re-add the `id` in the XML editor |
| Square icon has a squircle-shaped ghost | `squircleBase` still squircle-shaped | The script handles it; if it errors, restore `id="squircleBase"` |
| Linux icon looks like an inset blob | The macOS squircle mask got baked in | Regenerate; the square variant is automatic |
| macOS icon not updating after a rebuild | Finder/Dock icon cache — the bundle's own mtime did not change | `killall Finder && killall Dock`, or bump `VERSION`. See "If the icon still looks old" |
| Windows `.exe` shows the generic icon | `.ico` missing at configure time | Regenerate, then re-run CMake (the path is checked with `EXISTS`) |

## 8. Quick command reference

```bash
# Full pipeline for one tool
scripts/generate-icons.sh rotreader --name "ROT Reader"

# One-off single size
export PATH="/Applications/Inkscape.app/Contents/MacOS:$PATH"
inkscape docs/graphics/rotreader/rotreader-logo.svg \
  --export-id=iconExport --export-id-only --export-type=png -w 512 -h 512 -o out.png

# Inspect what ids exist in a master
grep -o 'id="[^"]*"' docs/graphics/rotreader/rotreader-logo.svg | sort -u

# Verify the .icns
iconutil -c iconset apps/rotreader/assets/icons/macos/AppIcon.icns -o /tmp/check.iconset && ls /tmp/check.iconset

# Verify the .ico
file apps/rotreader/assets/icons/windows/rotreader.ico
```
