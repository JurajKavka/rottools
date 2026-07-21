// @ts-check
import { defineConfig } from 'astro/config';

// Static output: `astro build` writes a plain directory of HTML/CSS/JS to
// dist/, which is what the Pages workflow uploads. No Node process runs in
// production.
//
// The site is served from a project page, so every URL is prefixed with
// /rottools/. Never hard-code that prefix in markup — use
// `import.meta.env.BASE_URL` so `astro dev` (which serves from the same base)
// and a future move to a custom domain both keep working.
export default defineConfig({
  site: 'https://jurajkavka.github.io',
  base: '/rottools',
  output: 'static',
  build: {
    // Emit `about/index.html` rather than `about.html`, so URLs work the same
    // on any static host without rewrite rules.
    format: 'directory',
  },
});
