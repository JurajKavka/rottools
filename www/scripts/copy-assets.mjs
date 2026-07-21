// Copies the repo-root assets/ folder into www/public/ before `astro dev` and
// `astro build` (wired as predev/prebuild in package.json).
//
// assets/ is the single source of truth for shared branding — the site and the
// app packaging read the same files, so a logo is never committed twice and
// cannot drift between the two.
//
// www/public/ is FULLY GENERATED and gitignored. It is emptied on every run, so
// never put a hand-made file there: it will be deleted. Add it to assets/.

import { cp, mkdir, rm, readdir } from 'node:fs/promises';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const wwwDir = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const srcDir = resolve(wwwDir, '..', 'assets');
const destDir = resolve(wwwDir, 'public');

// Guard the destructive step: only ever wipe www/public, never anything else.
if (destDir !== resolve(wwwDir, 'public')) {
  throw new Error(`refusing to clear unexpected path: ${destDir}`);
}

let entries;
try {
  entries = await readdir(srcDir);
} catch (err) {
  if (err.code === 'ENOENT') {
    throw new Error(`assets folder not found: ${srcDir}`);
  }
  throw err;
}

await rm(destDir, { recursive: true, force: true });
await mkdir(destDir, { recursive: true });
await cp(srcDir, destDir, { recursive: true });

console.log(`copy-assets: ${entries.length} item(s) from assets/ -> www/public/`);
