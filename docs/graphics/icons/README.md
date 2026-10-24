# Embedded UI icons

This directory owns the small SVG icons embedded in the reusable wxWidgets UI
components. These are separate from the application logos under
`docs/graphics/rotreader/` and `docs/graphics/rotpad/`.

## Directory layout

- `templates/icon-template.svg` is the reusable Inkscape drawing board for a new
  icon.
- `sources/` contains the editable Inkscape source for each icon.
- `generated/` contains the compact, standalone SVGs consumed by CMake. Do not
  edit these files by hand; commit them after regeneration.

The template and every source use a 24 x 24 design grid. The visible artwork
belongs in the `iconGlyph` group inside `iconExport`. Keep the transparent
`exportBounds` rectangle at 24 x 24: the extractor uses it to define the output
canvas and then removes it. Artwork outside `iconExport` is documentation or a
drawing aid and is not included in the application.

## Workflow

1. Copy `templates/icon-template.svg` to `sources/<name>.svg`.
2. Draw the icon in Inkscape inside `iconGlyph`. Gradients and other referenced
   SVG definitions are supported and copied with the artwork.
3. Register the source and generated paths in `scripts/extract-ui-icons.py`.
4. Run `make ui-icons` from the repository root.
5. Add the generated SVG to `libs/ui/Icons/CMakeLists.txt`, expose the icon from
   `Icons.h`, and implement its cached accessor in `Icons.cpp`.

Extraction uses only Python's standard library. It removes the Inkscape and
Sodipodi metadata, the drawing board, and `exportBounds`; no Inkscape command is
needed. The result retains a transparent background and can be embedded directly
in the C++ library.
