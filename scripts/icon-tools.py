#!/usr/bin/env python3
"""Helpers for generate-icons.sh — the two steps plain shell tools cannot do.

  square-variant <in.svg> <out.svg>
      Write a copy of an icon master with the macOS squircle mask taken off, so
      Linux / Windows / web get a full-bleed square icon. Two edits:
        * the group id="iconClipped" loses its clip-path attribute
        * the white base id="squircleBase" (a squircle-shaped <use>) becomes a
          rect covering the whole canvas
      Without the second edit the semi-transparent background composites over
      white inside the squircle and over nothing outside it, which shows up as a
      squircle-shaped ghost in the corners of the square icon.

  bg-color <in.svg>
      Print the icon's flat background colour as #rrggbb. Starts from the white
      squircleBase and composites every full-canvas rect above it, so a
      half-transparent fill reports the colour you actually see. Used for
      theme_color in site.webmanifest.

  pack-ico <out.ico> <in.png>...
      Build a multi-resolution Windows .ico. The entries are stored as PNG,
      which Windows has read since Vista. This exists so the pipeline needs no
      ImageMagick.

Both subcommands fail loudly rather than silently producing something wrong.
"""

import struct
import sys
import xml.etree.ElementTree as ET

SVG_NS = "http://www.w3.org/2000/svg"

# Keep Inkscape's own prefixes on the round-trip; ElementTree would otherwise
# rename them to ns0/ns1 and the layer metadata becomes unreadable in Inkscape.
NAMESPACES = {
    "": SVG_NS,
    "sodipodi":"http://sodipodi.sourceforge.net/DTD/sodipodi-0.0.dtd",
    "inkscape": "http://www.inkscape.org/namespaces/inkscape",
    "xlink": "http://www.w3.org/1999/xlink",
    "dc": "http://purl.org/dc/elements/1.1/",
    "cc": "http://creativecommons.org/ns#",
    "rdf": "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
}


def die(message):
    sys.stderr.write("icon-tools: %s\n" % message)
    sys.exit(1)


# --------------------------------------------------------------------------
# square-variant
# --------------------------------------------------------------------------


def find_by_id(root, wanted):
    """Return (element, parent) for the element carrying id="wanted"."""
    for parent in root.iter():
        for child in parent:
            if child.get("id") == wanted:
                return child, parent
    if root.get("id") == wanted:
        return root, None
    return None, None


def canvas_size(root):
    """Canvas edge length, read off the transparent export spacer."""
    spacer, _ = find_by_id(root, "exportSpacer")
    if spacer is None:
        return 1024.0
    try:
        return max(float(spacer.get("width", 1024)), float(spacer.get("height", 1024)))
    except ValueError:
        return 1024.0


def square_variant(src, dst):
    for prefix, uri in NAMESPACES.items():
        ET.register_namespace(prefix, uri)

    tree = ET.parse(src)
    root = tree.getroot()
    size = canvas_size(root)

    clipped, _ = find_by_id(root, "iconClipped")
    if clipped is None:
        die('no element with id="iconClipped" in %s' % src)
    if "clip-path" not in clipped.attrib:
        die('id="iconClipped" in %s has no clip-path attribute — did the master '
            "change? Restore it in Inkscape's XML editor." % src)
    del clipped.attrib["clip-path"]

    base, parent = find_by_id(root, "squircleBase")
    if base is None:
        die('no element with id="squircleBase" in %s' % src)

    square = ET.Element("{%s}rect" % SVG_NS)
    square.set("id", "squircleBase")
    square.set("x", "0")
    square.set("y", "0")
    square.set("width", "%g" % size)
    square.set("height", "%g" % size)
    # Carry the base's own paint over so a recoloured master stays correct.
    for attr in ("fill", "style", "fill-opacity"):
        if base.get(attr):
            square.set(attr, base.get(attr))
    if not square.get("fill") and not square.get("style"):
        square.set("fill", "#ffffff")

    parent[list(parent).index(base)] = square
    tree.write(dst, encoding="UTF-8", xml_declaration=True)


# --------------------------------------------------------------------------
# bg-color
# --------------------------------------------------------------------------


def style_value(element, name):
    """Read name from the element's style="" or from a plain attribute."""
    style = element.get("style", "")
    for entry in style.split(";"):
        key, _, value = entry.partition(":")
        if key.strip() == name:
            return value.strip()
    return element.get(name)


def parse_rgb(value):
    if not value or not value.startswith("#"):
        return None
    digits = value[1:]
    if len(digits) == 3:
        digits = "".join(c * 2 for c in digits)
    if len(digits) != 6:
        return None
    try:
        return tuple(int(digits[i:i + 2], 16) for i in (0, 2, 4))
    except ValueError:
        return None


def bg_color(src):
    tree = ET.parse(src)
    root = tree.getroot()
    size = canvas_size(root)

    base = find_by_id(root, "squircleBase")[0]
    colour = (255, 255, 255)
    if base is not None:
        parsed = parse_rgb(style_value(base, "fill"))
        if parsed:
            colour = parsed

    clipped, _ = find_by_id(root, "iconClipped")
    if clipped is None:
        die('no element with id="iconClipped" in %s' % src)

    for child in clipped:
        if child.tag != "{%s}rect" % SVG_NS:
            continue
        try:
            if float(child.get("width", 0)) < size or float(child.get("height", 0)) < size:
                continue
        except ValueError:
            continue
        parsed = parse_rgb(style_value(child, "fill"))
        if not parsed:
            continue
        try:
            alpha = float(style_value(child, "fill-opacity") or 1.0)
            alpha *= float(style_value(child, "opacity") or 1.0)
        except ValueError:
            alpha = 1.0
        colour = tuple(round(parsed[i] * alpha + colour[i] * (1 - alpha)) for i in range(3))

    sys.stdout.write("#%02x%02x%02x\n" % colour)


# --------------------------------------------------------------------------
# pack-ico
# --------------------------------------------------------------------------


def png_size(path):
    with open(path, "rb") as handle:
        head = handle.read(24)
    if len(head) < 24 or head[:8] != b"\x89PNG\r\n\x1a\n" or head[12:16] != b"IHDR":
        die("%s is not a PNG" % path)
    return struct.unpack(">II", head[16:24])


def pack_ico(dst, sources):
    if not sources:
        die("pack-ico needs at least one PNG")

    images = []
    for path in sources:
        width, height = png_size(path)
        if width > 256 or height > 256:
            die("%s is %dx%d; .ico entries max out at 256x256" % (path, width, height))
        with open(path, "rb") as handle:
            images.append((width, height, handle.read()))
    images.sort(key=lambda entry: entry[0])

    header = struct.pack("<HHH", 0, 1, len(images))
    offset = len(header) + 16 * len(images)

    directory = b""
    for width, height, blob in images:
        directory += struct.pack(
            "<BBBBHHII",
            0 if width == 256 else width,   # 0 means 256 in the .ico directory
            0 if height == 256 else height,
            0,                              # palette size, 0 for truecolour
            0,                              # reserved
            1,                              # colour planes
            32,                             # bits per pixel
            len(blob),
            offset,
        )
        offset += len(blob)

    with open(dst, "wb") as handle:
        handle.write(header)
        handle.write(directory)
        for _, _, blob in images:
            handle.write(blob)


# --------------------------------------------------------------------------


def main(argv):
    if len(argv) < 2:
        die(__doc__)
    command = argv[1]
    if command == "square-variant":
        if len(argv) != 4:
            die("usage: icon-tools.py square-variant <in.svg> <out.svg>")
        square_variant(argv[2], argv[3])
    elif command == "bg-color":
        if len(argv) != 3:
            die("usage: icon-tools.py bg-color <in.svg>")
        bg_color(argv[2])
    elif command == "pack-ico":
        if len(argv) < 4:
            die("usage: icon-tools.py pack-ico <out.ico> <in.png>...")
        pack_ico(argv[2], argv[3:])
    else:
        die("unknown command: %s" % command)


if __name__ == "__main__":
    main(sys.argv)
