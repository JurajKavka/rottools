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
| `npm start`       | Same, and opens the browser for you            |
| `npm run build`   | Production build into `dist/`                  |
| `npm run preview` | Serve `dist/` locally to check the real output |
| `npm run check`   | Type-check `.astro` and `.ts` files            |

## Layout

```
www/
  astro.config.mjs        # static output, directory-style URLs
  public/                 # copied verbatim. GENERATED — do not edit by hand:
                          #   make web-icons   (from docs/graphics/rottools/)
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

## Publishing

Deployed to **https://jurajkavka.github.io/rottools/** by
[.github/workflows/pages.yml](../.github/workflows/pages.yml) on every push to
`main` that touches `www/`. It runs `npm ci`, `npm run check`, `npm run build`
and uploads `dist/` to GitHub Pages. There is no server and no rsync.

One-time repo setup: **Settings > Pages > Build and deployment > Source:
"GitHub Actions"**. Until that is set, the workflow builds but the deploy step
fails.

### The subpath is build-only

`astro.config.mjs` applies `base: '/rottools'` for `build` and `preview` only.
`npm run dev` stays at plain `http://localhost:4321/`.

The command is read from `process.argv`, not `NODE_ENV`: Vite sets `NODE_ENV`
while resolving the config, which is *after* this file has been evaluated, so it
is empty when we need it. `preview` gets the base because it serves the built
`dist/`, whose URLs already carry the subpath.

One consequence worth knowing: **a hand-written `/foo.png` works in dev and 404s
in production.** Dev no longer catches that class of mistake. Two habits keep it
from biting:

- put every `public/` path through `asset()` (see below) — it is correct under
  either base;
- run `npm run preview` before pushing anything that touches markup. It serves
  the real built output at the real subpath.

### The site lives under a subpath

A GitHub Pages *project* site is served from `<user>.github.io/<repo>/`, so
`astro.config.mjs` sets `base: '/rottools'`, which must match the repository
name. Astro prefixes the URLs it generates itself, but **not** paths written by
hand in markup — a bare `src="/favicon.ico"` resolves to the domain root and
404s.

So every hand-written path to something in `public/` goes through
[src/lib/url.ts](src/lib/url.ts):

```astro
---
import { asset, home, absoluteAsset } from '../lib/url';
---
<img src={asset('rottools-logo.svg')} />   <!-- /rottools/rottools-logo.svg -->
<a href={home()}>…</a>                     <!-- /rottools/ -->
```

`absoluteAsset()` is for metadata crawlers read: `og:image` must be a full URL,
because scrapers ignore relative ones. `site.webmanifest` sidesteps the problem
differently — its icon paths are relative, so the browser resolves them against
the manifest's own URL.

Moving to a domain root later is a one-line change: drop `base` from
`astro.config.mjs`. Nothing else hardcodes it.

## Verified, not assumed

The download links point at real GitHub Release assets and were checked with
`curl` — all three return 200. The tag format (`rotreader-v0.1.0`) comes from
`.github/workflows/release-rotreader.yml`; the asset names come from CPack's
`CPACK_PACKAGE_FILE_NAME`.

`src/data/tools.ts` imports each tool's version directly from
`apps/<tool>/VERSION`. A version-bump merge does not deploy the site by itself:
the matching release workflow calls the reusable Pages workflow only after the
GitHub Release and its assets have been published. This keeps the app, package,
and website versions in sync without exposing download links early.
