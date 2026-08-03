/**
 * URL helpers for a site served from a subpath.
 *
 * The site is published at https://jurajkavka.github.io/rottools/, so a
 * hand-written "/favicon.ico" would resolve to the domain root and 404. Astro
 * prefixes the URLs it generates itself (its bundled CSS and JS), but it never
 * rewrites strings inside markup. Every hand-written path to something in
 * public/ has to go through asset().
 *
 * Nothing here hardcodes the base: it comes from `base` in astro.config.mjs via
 * import.meta.env.BASE_URL, so moving the site to a domain root is a one-line
 * config change.
 */

/** The base with no trailing slash: "/rottools", or "" at a domain root. */
function baseUrl(): string {
  return import.meta.env.BASE_URL.replace(/\/+$/, '');
}

/**
 * Resolve a file in public/ against the site's base.
 *
 * asset('favicon.ico') and asset('/favicon.ico') both give '/rottools/favicon.ico'.
 */
export function asset(path: string): string {
  return `${baseUrl()}/${path.replace(/^\/+/, '')}`;
}

/** The site root — what the header logo links to. */
export function home(): string {
  return `${baseUrl()}/`;
}

/**
 * Absolute URL for a public/ file, for metadata that crawlers read.
 *
 * og:image must be absolute: a relative one is ignored by most social-card
 * scrapers. Resolves against `site` in astro.config.mjs.
 */
export function absoluteAsset(path: string, site: URL | undefined): string {
  const relative = asset(path);
  return site ? new URL(relative, site).href : relative;
}
