#!/usr/bin/env bash
# generate-icons.sh — build every platform icon for one target from its master SVG.
#
#   Input   docs/graphics/<name>/<name>-logo.svg  (hand-edited in Inkscape)
#   Output  apps/<name>/assets/icons/             (generated, committed to git)
#
# The output is committed because the Linux and Windows CI runners have no
# Inkscape: the build reads finished .icns/.ico/.png files, it never rasterises.
# Re-run this script whenever the master SVG changes, then commit the result.
#
# A target without an apps/<name> directory — the rottools suite logo, which has
# no binary — gets no .icns/.ico/hicolor set. There is nothing to package. It
# needs --web-out instead, which writes the favicon set straight into the site.
#
# Usage:
#   scripts/generate-icons.sh <name> [--name "<display name>"] [--web] [--web-out <dir>]
#
#   --web           also write the favicon set into <output>/web/
#   --web-out DIR   write the favicon set flat into DIR (e.g. www/public) instead,
#                   plus every tool's own mark as DIR/<tool>-rottools-logo.svg.
#                   DIR is never wiped; only the files it writes are replaced.
#
# Examples:
#   scripts/generate-icons.sh rotreader --name "ROT Reader"
#   scripts/generate-icons.sh rottools  --name "ROT Tools" --web-out www/public
#
# Requirements: Inkscape 1.x, python3, and on macOS iconutil (Xcode Command Line
# Tools) for the .icns. Everything else is plain shell. ImageMagick is NOT
# needed — scripts/icon-tools.py writes the .ico itself.
#
# See docs/graphics/README.md for the full reference.
set -euo pipefail

export PATH="/Applications/Inkscape.app/Contents/MacOS:$PATH"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TOOLS="$SCRIPT_DIR/icon-tools.py"

APP=""
DISPLAY_NAME=""
WANT_WEB=0
WEB_OUT=""

usage() {
    echo "usage: scripts/generate-icons.sh <name> [--name \"<display name>\"] [--web] [--web-out <dir>]" >&2
}

while [ $# -gt 0 ]; do
    case "$1" in
        --name)    DISPLAY_NAME="${2:-}"; shift 2 ;;
        --web)     WANT_WEB=1; shift ;;
        --web-out) WEB_OUT="${2:-}"; WANT_WEB=1; shift 2 ;;
        -h|--help) sed -n '2,30p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        -*) echo "unknown option: $1" >&2; usage; exit 1 ;;
        *)  APP="$1"; shift ;;
    esac
done

if [ -z "$APP" ]; then
    usage
    exit 1
fi
[ -n "$DISPLAY_NAME" ] || DISPLAY_NAME="$APP"

MASTER="$ROOT/docs/graphics/$APP/$APP-logo.svg"
OUT="$ROOT/apps/$APP/assets/icons"

if [ ! -f "$MASTER" ]; then
    echo "master SVG not found: $MASTER" >&2
    echo "Copy docs/graphics/icon-template.svg there and design the icon in Inkscape." >&2
    exit 1
fi

# A target with no apps/<name> is not packaged, so it gets no platform icons.
HAS_APP=0
[ -d "$ROOT/apps/$APP" ] && HAS_APP=1
if [ "$HAS_APP" -eq 0 ] && [ -z "$WEB_OUT" ]; then
    echo "apps/$APP does not exist, so there are no app icons to build." >&2
    echo "For a target with no binary (the rottools suite logo), say where the" >&2
    echo "web icons should go:" >&2
    echo "  scripts/generate-icons.sh $APP --name \"...\" --web-out www/public" >&2
    exit 1
fi

if [ -n "$WEB_OUT" ]; then
    case "$WEB_OUT" in
        /*) : ;;
        *)  WEB_OUT="$ROOT/$WEB_OUT" ;;
    esac
    if [ ! -d "$WEB_OUT" ]; then
        echo "--web-out directory does not exist: $WEB_OUT" >&2
        exit 1
    fi
fi

for tool in inkscape python3; do
    command -v "$tool" >/dev/null 2>&1 || { echo "$tool not found on PATH" >&2; exit 1; }
done

EXPORT_ID="iconExport"
grep -q "id=\"$EXPORT_ID\"" "$MASTER" || {
    echo "ERROR: no object with id=\"$EXPORT_ID\" in $MASTER" >&2
    echo "Inkscape strips ids on ungroup/regroup — restore it via Edit > XML Editor." >&2
    exit 1
}

# --- output tree -----------------------------------------------------------
# The app icon directory holds nothing but generated files, so it is wiped to
# clear stale sizes. Guard the rm so it can only ever hit that path. A
# --web-out directory belongs to someone else (the website) and is never wiped;
# there we replace individual files and print each one.
if [ "$HAS_APP" -eq 1 ]; then
    case "$OUT" in
        */assets/icons) rm -rf "$OUT" ;;
        *) echo "refusing to delete unexpected path: $OUT" >&2; exit 1 ;;
    esac
    mkdir -p "$OUT/macos" "$OUT/linux" "$OUT/windows"
fi

# Where the favicon set goes: flat into --web-out, else under the app's icons.
if [ -n "$WEB_OUT" ]; then
    WEB_DIR="$WEB_OUT"
else
    WEB_DIR="$OUT/web"
fi
[ "$WANT_WEB" -eq 1 ] && mkdir -p "$WEB_DIR"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# The square (unmasked) master: Linux, Windows and the web want a full-bleed
# square, because those platforms apply their own rounding. Only macOS bakes in
# the squircle.
SQUARE="$TMP/$APP-square.svg"
python3 "$TOOLS" square-variant "$MASTER" "$SQUARE"

# export_png <source.svg> <size> <output.png>
export_png() {
    local svg="$1" size="$2" out="$3"
    inkscape "$svg" \
        --export-id="$EXPORT_ID" --export-id-only \
        --export-type=png -w "$size" -h "$size" \
        -o "$out" >/dev/null 2>&1
    if [ ! -s "$out" ]; then
        echo "ERROR: Inkscape produced no output for $out" >&2
        echo "Re-run without redirection to see its error:" >&2
        echo "  inkscape \"$svg\" --export-id=$EXPORT_ID --export-id-only --export-type=png -w $size -h $size -o /tmp/test.png" >&2
        exit 1
    fi
    # Intermediates land in $TMP; only report the files that survive.
    case "$out" in
        "$ROOT"/*) printf '  %s (%sx%s)\n' "${out#$ROOT/}" "$size" "$size" ;;
    esac
}

# scalable_svg <source.svg> <output.svg> — the flat, guides-free square icon.
scalable_svg() {
    local svg="$1" out="$2"
    inkscape "$svg" \
        --export-id="$EXPORT_ID" --export-id-only \
        --export-plain-svg --export-type=svg \
        -o "$out" >/dev/null 2>&1
    [ -s "$out" ] || { echo "ERROR: scalable SVG export failed for $out" >&2; exit 1; }
    echo "  ${out#$ROOT/} (scalable)"
}

echo "master: ${MASTER#$ROOT/}"
if [ "$HAS_APP" -eq 1 ]; then
    echo "output: ${OUT#$ROOT/}"
else
    echo "output: ${WEB_DIR#$ROOT/}  (no apps/$APP — skipping the .icns, .ico and"
    echo "        hicolor set: there is no binary to package)"
fi

if [ "$HAS_APP" -eq 1 ]; then

# --- macOS: squircle, 824x824 inside a 1024 canvas -------------------------
echo "== macOS =="
ICONSET="$TMP/AppIcon.iconset"
mkdir -p "$ICONSET"
export_png "$MASTER" 16   "$ICONSET/icon_16x16.png"
export_png "$MASTER" 32   "$ICONSET/icon_16x16@2x.png"
export_png "$MASTER" 32   "$ICONSET/icon_32x32.png"
export_png "$MASTER" 64   "$ICONSET/icon_32x32@2x.png"
export_png "$MASTER" 128  "$ICONSET/icon_128x128.png"
export_png "$MASTER" 256  "$ICONSET/icon_128x128@2x.png"
export_png "$MASTER" 256  "$ICONSET/icon_256x256.png"
export_png "$MASTER" 512  "$ICONSET/icon_256x256@2x.png"
export_png "$MASTER" 512  "$ICONSET/icon_512x512.png"
export_png "$MASTER" 1024 "$ICONSET/icon_512x512@2x.png"

if command -v iconutil >/dev/null 2>&1; then
    iconutil -c icns "$ICONSET" -o "$OUT/macos/AppIcon.icns"
    echo "  ${OUT#$ROOT/}/macos/AppIcon.icns"
else
    echo "  iconutil not found (not on macOS) — keeping the .iconset instead" >&2
    cp -R "$ICONSET" "$OUT/macos/AppIcon.iconset"
fi
# --- Linux: hicolor theme, square, plus the scalable SVG -------------------
echo "== Linux =="
for size in 16 22 24 32 48 64 128 256 512; do
    export_png "$SQUARE" "$size" "$OUT/linux/$APP-$size.png"
done
scalable_svg "$SQUARE" "$OUT/linux/$APP.svg"

# --- Windows: one multi-resolution .ico ------------------------------------
echo "== Windows =="
WIN_PNGS=()
for size in 16 24 32 48 64 128 256; do
    export_png "$SQUARE" "$size" "$TMP/win-$size.png"
    WIN_PNGS+=("$TMP/win-$size.png")
done
python3 "$TOOLS" pack-ico "$OUT/windows/$APP.ico" "${WIN_PNGS[@]}"
echo "  ${OUT#$ROOT/}/windows/$APP.ico (16,24,32,48,64,128,256)"

# --- square master, used for READMEs and og:image --------------------------
export_png "$SQUARE" 1024 "$OUT/$APP-1024.png"

fi  # HAS_APP

# --- Web: favicon set + square content images ------------------------------
if [ "$WANT_WEB" -eq 1 ]; then
    echo "== Web =="
    export_png "$SQUARE" 16   "$WEB_DIR/favicon-16x16.png"
    export_png "$SQUARE" 32   "$WEB_DIR/favicon-32x32.png"
    export_png "$SQUARE" 48   "$WEB_DIR/favicon-48x48.png"
    export_png "$SQUARE" 180  "$WEB_DIR/apple-touch-icon.png"
    export_png "$SQUARE" 192  "$WEB_DIR/android-chrome-192x192.png"
    export_png "$SQUARE" 512  "$WEB_DIR/android-chrome-512x512.png"

    # Square images for page content: 256 inline, 512 cards, 1024 masters.
    for size in 256 512 1024; do
        export_png "$SQUARE" "$size" "$WEB_DIR/$APP-logo-$size.png"
    done
    scalable_svg "$SQUARE" "$WEB_DIR/$APP-logo.svg"

    python3 "$TOOLS" pack-ico "$WEB_DIR/favicon.ico" \
        "$WEB_DIR/favicon-16x16.png" "$WEB_DIR/favicon-32x32.png" "$WEB_DIR/favicon-48x48.png"
    echo "  ${WEB_DIR#$ROOT/}/favicon.ico (16,32,48)"

    # theme_color is read off the master rather than hardcoded, so each target's
    # manifest matches its own icon.
    THEME="$(python3 "$TOOLS" bg-color "$MASTER")"
    # Paths are relative, not root-absolute: a browser resolves them against the
    # manifest's own URL, so this stays correct whether the site is served from
    # a domain root or from a subpath (GitHub Pages puts it under /rottools/).
    cat > "$WEB_DIR/site.webmanifest" <<EOF
{
  "name": "$DISPLAY_NAME",
  "short_name": "$DISPLAY_NAME",
  "icons": [
    { "src": "android-chrome-192x192.png", "sizes": "192x192", "type": "image/png" },
    { "src": "android-chrome-512x512.png", "sizes": "512x512", "type": "image/png" }
  ],
  "start_url": "./",
  "scope": "./",
  "theme_color": "$THEME",
  "background_color": "#ffffff",
  "display": "standalone"
}
EOF
    echo "  ${WEB_DIR#$ROOT/}/site.webmanifest (theme_color $THEME)"
fi

# --- each tool's own mark, for the site's download cards -------------------
# The suite site shows a per-tool icon next to each download, so every app's
# scalable icon is copied in beside the suite logo. The square (unmasked)
# variant is used: a page can round it with CSS, but it cannot un-round a
# baked-in squircle.
if [ -n "$WEB_OUT" ]; then
    echo "== Tool marks =="
    copied=0
    for dir in "$ROOT"/apps/*; do
        [ -d "$dir" ] || continue
        tool="$(basename "$dir")"
        svg="$dir/assets/icons/linux/$tool.svg"
        if [ -f "$svg" ]; then
            cp "$svg" "$WEB_DIR/$tool-rottools-logo.svg"
            echo "  ${WEB_DIR#$ROOT/}/$tool-rottools-logo.svg  <- ${svg#$ROOT/}"
            copied=$((copied + 1))
        else
            echo "  skipped $tool: no ${svg#$ROOT/} — run scripts/generate-icons.sh $tool" >&2
        fi
    done
    [ "$copied" -gt 0 ] || echo "  no tool icons found under apps/" >&2
fi

echo
if [ "$HAS_APP" -eq 1 ]; then
    echo "Done. Commit ${OUT#$ROOT/} — the Linux and Windows builds read these files directly."
else
    echo "Done. Commit ${WEB_DIR#$ROOT/}."
fi
if [ -n "$WEB_OUT" ]; then
    echo "Reference the favicons from the site's <head>; see docs/graphics/README.md."
fi
