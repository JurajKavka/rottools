#!/bin/bash
# Builds a distributable macOS disk image (.dmg) for rotreader
# (the app is shown to users as "ℜ⛤✝ reader"; rotreader is the technical name).
#
# A raw Unix executable is not distributable on macOS: it depends on dylibs
# installed by Homebrew on THIS machine. Distribution means building an .app
# bundle that carries everything with it, then wrapping it in a .dmg. The
# phases below:
#
#   1. Assemble the .app bundle skeleton (Contents/MacOS, Info.plist).
#   2. Copy every non-system dylib the binary (transitively) links against
#      into Contents/Frameworks, and rewrite the Mach-O "install names" so
#      the loader looks for them inside the bundle instead of /opt/homebrew.
#   3. Code-sign everything (ad-hoc). On Apple Silicon unsigned binaries do
#      not run at all, and editing a binary with install_name_tool
#      invalidates its existing signature.
#   4. Pack the bundle into a compressed .dmg with the customary
#      "drag to Applications" symlink.
#
# Only OS tools are used: otool, install_name_tool, codesign, hdiutil.

set -euo pipefail
cd "$(dirname "$0")/.."

EXE_NAME="rotreader"         # CMake target / binary name; also CFBundleExecutable.
                             # MUST match the file CMake produces in build/ — do not change
                             # here without renaming the CMake target.
APP_NAME="ℜ⛤✝ reader"        # user-facing name: the .app bundle, Finder, Dock, menu bar.
VERSION="0.1.0"
BUNDLE_ID="com.jurajkavka.rotreader"
BUILD_DIR="build"
DIST_DIR="dist"
APP_DIR="$DIST_DIR/$APP_NAME.app"
FRAMEWORKS_DIR="$APP_DIR/Contents/Frameworks"
RESOURCES_DIR="$APP_DIR/Contents/Resources"
DMG_PATH="$DIST_DIR/$EXE_NAME-$VERSION.dmg"   # ASCII slug keeps the download file name sane
ICON_SRC="assets/logo.png"   # 1024x1024 master; scaled down into the .icns
ICON_NAME="AppIcon"          # basename of the generated .icns / CFBundleIconFile

echo "==> Building $APP_NAME"
cmake -B "$BUILD_DIR" >/dev/null && cmake --build "$BUILD_DIR" >/dev/null

# ---------------------------------------------------------------------------
# Phase 1: .app bundle skeleton
#
# An .app is just a directory with a fixed layout. Finder shows it as a
# single launchable icon because of the .app suffix and the Info.plist.
# ---------------------------------------------------------------------------
echo "==> Assembling $APP_DIR"
rm -rf "$DIST_DIR"
mkdir -p "$APP_DIR/Contents/MacOS" "$FRAMEWORKS_DIR" "$RESOURCES_DIR"
cp "$BUILD_DIR/$EXE_NAME" "$APP_DIR/Contents/MacOS/$EXE_NAME"

# ---------------------------------------------------------------------------
# App icon
#
# macOS wants an .icns: a container holding the icon rendered at every size
# the system uses (Finder, Dock, Cmd-Tab, Get Info). We build one from the
# 1024x1024 master by scaling it into the Apple-named .iconset layout with
# `sips`, then compiling with `iconutil`. Both are built-in OS tools, so no
# extra dependency. The bundle references it via CFBundleIconFile below.
# ---------------------------------------------------------------------------
ICON_PLIST_ENTRY=""
if [ -f "$ICON_SRC" ]; then
    echo "==> Generating $ICON_NAME.icns from $ICON_SRC"
    ICONSET="$DIST_DIR/$ICON_NAME.iconset"
    mkdir -p "$ICONSET"
    for size in 16 32 128 256 512; do
        sips -z "$size"   "$size"   "$ICON_SRC" --out "$ICONSET/icon_${size}x${size}.png"    >/dev/null
        sips -z $((size*2)) $((size*2)) "$ICON_SRC" --out "$ICONSET/icon_${size}x${size}@2x.png" >/dev/null
    done
    iconutil -c icns "$ICONSET" -o "$RESOURCES_DIR/$ICON_NAME.icns"
    rm -rf "$ICONSET"
    ICON_PLIST_ENTRY="    <key>CFBundleIconFile</key>        <string>$ICON_NAME</string>"
else
    echo "==> WARNING: $ICON_SRC not found; building without an app icon"
fi

# The minimal set of keys Finder and the loader care about
cat > "$APP_DIR/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>      <string>$EXE_NAME</string>
    <key>CFBundleIdentifier</key>      <string>$BUNDLE_ID</string>
    <key>CFBundleName</key>            <string>$APP_NAME</string>
    <key>CFBundleDisplayName</key>     <string>$APP_NAME</string>
$ICON_PLIST_ENTRY
    <key>CFBundlePackageType</key>     <string>APPL</string>
    <key>CFBundleShortVersionString</key> <string>$VERSION</string>
    <key>CFBundleVersion</key>         <string>$VERSION</string>
    <key>NSHighResolutionCapable</key> <true/>
</dict>
</plist>
PLIST

# ---------------------------------------------------------------------------
# Phase 2: bundle the dylib dependencies
#
# Every Mach-O binary records, per dependency, the "install name" — the path
# where the loader should find it at runtime (see them with `otool -L`).
# Our binary records /opt/homebrew/... for wxWidgets and @rpath/... for
# md4c, neither of which exists on a user's machine. So we walk the
# dependency graph breadth-first: copy each non-system dylib into
# Contents/Frameworks and rewrite the reference to
# @executable_path/../Frameworks/<name> — a path relative to the running
# executable, i.e. inside the bundle. Copied dylibs have dependencies of
# their own (wx core pulls libpng, pcre2, ...), so they join the queue too.
# ---------------------------------------------------------------------------
echo "==> Bundling dylibs into Contents/Frameworks"

# Directories to search when a dependency is recorded as @rpath/...
# (md4c and any internal shared lib are built somewhere inside build/)
RPATH_SEARCH_DIRS=$(find "$BUILD_DIR" -name "*.dylib" -print0 2>/dev/null | xargs -0 -n1 dirname 2>/dev/null | sort -u)

resolve_dep() { # translate a recorded install name into an existing file path
    local dep="$1"
    if [[ "$dep" == @rpath/* ]]; then
        local name="${dep#@rpath/}"
        local dir
        for dir in $RPATH_SEARCH_DIRS; do
            if [ -f "$dir/$name" ]; then echo "$dir/$name"; return 0; fi
        done
        echo "error: cannot resolve $dep" >&2; return 1
    fi
    echo "$dep"
}

queue=("$APP_DIR/Contents/MacOS/$EXE_NAME")
while [ ${#queue[@]} -gt 0 ]; do
    file="${queue[0]}"; queue=("${queue[@]:1}")

    # otool -L prints the file's own install name first for dylibs; skip it
    # by ignoring entries whose basename matches the file itself
    for dep in $(otool -L "$file" | tail -n +2 | awk '{print $1}'); do
        case "$dep" in /usr/lib/*|/System/*) continue ;; esac
        name=$(basename "$dep")
        [ "$name" = "$(basename "$file")" ] && continue

        target="$FRAMEWORKS_DIR/$name"
        if [ ! -f "$target" ]; then
            cp -L "$(resolve_dep "$dep")" "$target"   # -L: dereference Homebrew symlinks
            chmod u+w "$target"
            # A dylib's first otool -L line is its ID — what OTHER binaries
            # linking against it will record. Point it into the bundle.
            install_name_tool -id "@executable_path/../Frameworks/$name" "$target" 2>/dev/null
            queue+=("$target")
        fi
        install_name_tool -change "$dep" "@executable_path/../Frameworks/$name" "$file" 2>/dev/null
    done
done
echo "    bundled $(ls "$FRAMEWORKS_DIR" | wc -l | tr -d ' ') dylibs"

# ---------------------------------------------------------------------------
# Phase 3: code signing (ad-hoc)
#
# "--sign -" creates an ad-hoc signature: no certificate, no identity. It
# satisfies the Apple Silicon requirement that all code be signed, but is
# NOT enough for Gatekeeper on other machines — downloaded copies get the
# quarantine flag and users must right-click > Open the first time. Real
# distribution needs a Developer ID certificate + notarization; the flow is
# the same, with an identity instead of "-" plus a notarytool submission.
# Signing goes inside-out: the dylibs first, then the bundle.
# ---------------------------------------------------------------------------
echo "==> Code signing (ad-hoc)"
codesign --force --sign - "$FRAMEWORKS_DIR"/*.dylib 2>/dev/null
codesign --force --sign - "$APP_DIR" 2>/dev/null
codesign --verify --strict "$APP_DIR"

# ---------------------------------------------------------------------------
# Phase 4: pack the .dmg
#
# A .dmg is a mountable disk image. The convention: it contains the .app and
# a symlink to /Applications, so installing is one drag. UDZO = compressed,
# read-only — the standard distribution format. hdiutil does everything.
# ---------------------------------------------------------------------------
echo "==> Creating $DMG_PATH"
STAGING="$DIST_DIR/dmg-root"
mkdir "$STAGING"
cp -R "$APP_DIR" "$STAGING/"
ln -s /Applications "$STAGING/Applications"
hdiutil create -volname "$APP_NAME" -srcfolder "$STAGING" -ov -format UDZO "$DMG_PATH" >/dev/null
rm -rf "$STAGING"

echo "==> Done: $DMG_PATH ($(du -h "$DMG_PATH" | cut -f1 | tr -d ' '))"
