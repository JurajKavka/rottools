# www — the ROT tools website

Static [Astro](https://astro.build) site. `astro build` writes plain HTML, CSS
and JS to `dist/`, which the release pipeline rsyncs to the download server. No
Node process runs in production.

## Commands

Run these from `www/`:

| Command           | Does                                          |
|-------------------|-----------------------------------------------|
| `npm install`     | Install dependencies (first time only)        |
| `npm run dev`     | Dev server with live reload, `localhost:4321`  |
| `npm run build`   | Production build into `dist/`                  |
| `npm run preview` | Serve `dist/` locally to check the real output |
| `npm run check`   | Type-check `.astro` and `.ts` files            |

## Layout

```
www/
  astro.config.mjs        # static output, directory-style URLs
  public/                 # copied verbatim: logo.svg, logo.png
  src/
    data/tools.ts         # THE place to edit tools, versions, download URLs
    layouts/Layout.astro  # <head>, header, footer, global CSS
    components/
      ToolCard.astro      # one tool: name, description, download buttons
      MacNote.astro       # the Gatekeeper "damaged" warning
      ThemeToggle.astro   # Rotdown Light <-> Rotdown Dark switch
    pages/index.astro     # landing page
    styles/
      global.css          # imports the below, plus base element styles
      scale.css           # type/space/lines, no colour (copy of rotdown's)
      palettes/
        rotdown-light.css # colour only — the default
        rotdown-dark.css  # colour only — toggle, and the OS preference
```

## Theming

Colours come from the [rotdown](../../rotdown) design system, the same one
rotreader renders Markdown with, so the site and the app match. That system
splits `scale.css` (everything colourless) from `palettes/` (nothing but
colour), and this site keeps the split.

Rotdown Light is the default. The toggle in the header stamps
`data-theme="light|dark"` on `<html>` and remembers the choice in
`localStorage`; an inline script in `Layout.astro` applies it before first paint
so the page never flashes the wrong theme. With JavaScript off, the OS
preference decides.

The palette files are **copies**, not imports — `rotdown/` is a separate repo.
If its palettes change, re-copy the values.

## Adding a tool, or bumping a version

Everything user-facing lives in [src/data/tools.ts](src/data/tools.ts). Add an
entry to the `tools` array, or change a `version`, and the page follows. Nothing
else needs touching.

Download file names are built to match CPack's `CPACK_PACKAGE_FILE_NAME` from
[../cmake/RotToolsPackaging.cmake](../cmake/RotToolsPackaging.cmake):

```
<exe>-<version>-<os>-<arch>.<ext>
```

If the packaging naming changes, update `buildFileName()` to match.

## Before going live

- `DOWNLOAD_BASE` in `src/data/tools.ts` is a placeholder
  (`https://downloads.rottools.example`). Point it at the real download server.
- `site` in `astro.config.mjs` is a placeholder too. It is used for absolute
  URLs in metadata, so set it before publishing.
- The download links are constructed, not verified. Until the rsync step exists
  and has run, they will 404.
