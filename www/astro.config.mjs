// @ts-check
import { defineConfig } from 'astro/config';

// Static output: `astro build` writes a plain directory of HTML/CSS/JS to
// dist/, which is what the release pipeline rsyncs to the download server.
// No Node process runs in production.
export default defineConfig({
  site: 'https://rottools.example',
  output: 'static',
  build: {
    // Emit `about/index.html` rather than `about.html`, so URLs work the same
    // on any static host without rewrite rules.
    format: 'directory',
  },
});
