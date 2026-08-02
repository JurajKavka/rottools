// @ts-check
import { defineConfig } from 'astro/config';

// Static output: `astro build` writes a plain directory of HTML/CSS/JS to
// dist/, which .github/workflows/pages.yml uploads to GitHub Pages. No Node
// process runs in production.

// GitHub Pages serves a *project* site from <user>.github.io/<repo>/, so the
// published site lives under this subpath. It must match the repository name.
const PROD_BASE = '/rottools';

// Only the built site needs the subpath. `astro dev` stays at the root, so
// local work is plain http://localhost:4321/.
//
// `preview` is included because it serves the *built* dist/, which already has
// the subpath baked into its URLs — serving that at the root would 404.
//
// Read off argv rather than NODE_ENV: Vite sets NODE_ENV while resolving the
// config, which is after this file has already been evaluated, so it is not
// reliable here. argv also avoids an env-var dependency that would need
// cross-env to work on Windows.
// `process` is a Node global, but @types/node is not a dependency here and this
// file is @ts-check'd — hence the cast, rather than pulling in a package for a
// single lookup.
const command = /** @type {any} */ (globalThis).process?.argv?.[2];
const base = command === 'build' || command === 'preview' ? PROD_BASE : '/';

export default defineConfig({
  // `site` is the origin only; `base` is the subpath. Astro prefixes the URLs
  // it generates itself, but NOT paths written by hand in markup — those go
  // through asset() in src/lib/url.ts, which reads whichever base is active.
  site: 'https://jurajkavka.github.io',
  base,
  output: 'static',
  build: {
    // Emit `about/index.html` rather than `about.html`, so URLs work the same
    // on any static host without rewrite rules.
    format: 'directory',
  },
});
