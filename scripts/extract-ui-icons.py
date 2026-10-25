#!/usr/bin/env python3
"""Extract clean, embeddable SVGs from the UI icon authoring files."""

from __future__ import annotations

import argparse
import copy
from dataclasses import dataclass
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[1]
SVG_NAMESPACE = "http://www.w3.org/2000/svg"
XLINK_NAMESPACE = "http://www.w3.org/1999/xlink"
EDITOR_NAMESPACE_MARKERS = ("inkscape", "sodipodi")
REFERENCE_PATTERN = re.compile(r"url\(\s*#([^\s)]+)\s*\)")

ET.register_namespace("", SVG_NAMESPACE)
ET.register_namespace("xlink", XLINK_NAMESPACE)


@dataclass(frozen=True)
class Icon:
    name: str
    source: Path
    output: Path


ICONS = (
    Icon(
        name="default-file",
        source=ROOT / "docs/graphics/icons/sources/default-file.svg",
        output=ROOT / "docs/graphics/icons/generated/default-file.svg",
    ),
    Icon(
        name="md-file",
        source=ROOT / "docs/graphics/icons/sources/md-file.svg",
        output=ROOT / "docs/graphics/icons/generated/md-file.svg",
    ),
    Icon(
        name="folder",
        source=ROOT / "docs/graphics/icons/sources/folder.svg",
        output=ROOT / "docs/graphics/icons/generated/folder.svg",
    ),
)


def local_name(qualified_name: str) -> str:
    return qualified_name.rsplit("}", maxsplit=1)[-1]


def namespace(qualified_name: str) -> str:
    if qualified_name.startswith("{"):
        return qualified_name[1:].split("}", maxsplit=1)[0]
    return ""


def is_editor_attribute(attribute_name: str) -> bool:
    attribute_namespace = namespace(attribute_name).lower()
    return any(marker in attribute_namespace for marker in EDITOR_NAMESPACE_MARKERS)


def element_by_id(root: ET.Element, element_id: str) -> ET.Element:
    for element in root.iter():
        if element.get("id") == element_id:
            return element
    raise ValueError(f"missing element with id '{element_id}'")


def referenced_ids(element: ET.Element) -> set[str]:
    references: set[str] = set()
    for node in element.iter():
        for attribute_name, value in node.attrib.items():
            references.update(REFERENCE_PATTERN.findall(value))
            if local_name(attribute_name) == "href" and value.startswith("#"):
                references.add(value[1:])
    return references


def clean_copy(element: ET.Element) -> ET.Element:
    result = copy.deepcopy(element)
    for node in result.iter():
        for attribute_name in tuple(node.attrib):
            if is_editor_attribute(attribute_name):
                del node.attrib[attribute_name]
    return result


def canvas_dimensions(export_group: ET.Element) -> tuple[str, str]:
    for element in export_group.iter():
        if element.get("id") == "exportBounds":
            width = element.get("width")
            height = element.get("height")
            if width and height:
                return width, height
            raise ValueError("exportBounds must define width and height")
    raise ValueError("iconExport must contain an exportBounds rectangle")


def collect_dependencies(root: ET.Element, export_group: ET.Element) -> list[ET.Element]:
    exported_ids = {
        element_id
        for element in export_group.iter()
        if (element_id := element.get("id")) is not None
    }
    required_ids = referenced_ids(export_group) - exported_ids
    pending_ids = list(required_ids)

    while pending_ids:
        dependency_id = pending_ids.pop()
        dependency = element_by_id(root, dependency_id)
        for referenced_id in referenced_ids(dependency) - exported_ids:
            if referenced_id not in required_ids:
                required_ids.add(referenced_id)
                pending_ids.append(referenced_id)

    return [
        clean_copy(element)
        for element in root.iter()
        if element.get("id") in required_ids
    ]


def render_icon(icon: Icon) -> bytes:
    source_root = ET.parse(icon.source).getroot()
    export_group = element_by_id(source_root, "iconExport")
    width, height = canvas_dimensions(export_group)

    output_root = ET.Element(
        f"{{{SVG_NAMESPACE}}}svg",
        {
            "width": width,
            "height": height,
            "viewBox": f"0 0 {width} {height}",
        },
    )

    dependencies = collect_dependencies(source_root, export_group)
    if dependencies:
        definitions = ET.SubElement(output_root, f"{{{SVG_NAMESPACE}}}defs")
        definitions.extend(dependencies)

    for child in export_group:
        if child.get("id") != "exportBounds":
            output_root.append(clean_copy(child))

    ET.indent(output_root, space="  ")
    return (
        ET.tostring(
            output_root,
            encoding="utf-8",
            xml_declaration=True,
            short_empty_elements=True,
        )
        + b"\n"
    )


def process_icon(icon: Icon, check: bool) -> bool:
    expected = render_icon(icon)
    current = icon.output.read_bytes() if icon.output.exists() else None

    if current == expected:
        print(f"unchanged {icon.output.relative_to(ROOT)}")
        return True

    if check:
        print(f"stale {icon.output.relative_to(ROOT)}", file=sys.stderr)
        return False

    icon.output.parent.mkdir(parents=True, exist_ok=True)
    icon.output.write_bytes(expected)
    print(f"generated {icon.output.relative_to(ROOT)}")
    return True


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "icons",
        nargs="*",
        metavar="ICON",
        help="icon names to extract; all registered icons are extracted by default",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="fail if a generated SVG is absent or out of date",
    )
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    icons_by_name = {icon.name: icon for icon in ICONS}
    requested_names = arguments.icons or list(icons_by_name)
    unknown_names = sorted(set(requested_names) - icons_by_name.keys())
    if unknown_names:
        print(f"unknown icon(s): {', '.join(unknown_names)}", file=sys.stderr)
        print(f"available icons: {', '.join(icons_by_name)}", file=sys.stderr)
        return 2

    success = True
    for name in requested_names:
        success = process_icon(icons_by_name[name], arguments.check) and success
    return 0 if success else 1


if __name__ == "__main__":
    raise SystemExit(main())
