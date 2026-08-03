/**
 * Single source of truth for the site's tool list and download links.
 *
 * Downloads point at GitHub Releases. Two naming rules have to hold, both owned
 * by the repo rather than by this file:
 *
 *   tag    <slug>-v<version>            e.g. rotreader-v0.1.0
 *          set by the `on: push: tags` filter in
 *          .github/workflows/release-<tool>.yml
 *
 *   asset  <slug>-<version>-<os>-<arch>.<ext>
 *          CPack's CPACK_PACKAGE_FILE_NAME from cmake/RotToolsPackaging.cmake,
 *          where <os> is macos|linux|windows and <arch> is a lowercased
 *          CMAKE_SYSTEM_PROCESSOR.
 *
 * Keep releaseUrl() and buildFileName() in step if either ever changes.
 */

/** GitHub's release asset root for this repo. No trailing slash. */
export const DOWNLOAD_BASE = 'https://github.com/JurajKavka/rottools/releases/download';

/** Every release, for the "all downloads" case. */
export const RELEASES_URL = 'https://github.com/JurajKavka/rottools/releases';

export type Platform = 'macos' | 'windows' | 'linux';

export interface Download {
  platform: Platform;
  /** Shown on the button, e.g. "macOS (Apple Silicon)". */
  label: string;
  /** Shown under it, e.g. ".dmg - 3.4 MB". */
  note: string;
  href: string;
}

export interface Tool {
  /** CMake target / binary name, and the URL slug. */
  slug: string;
  /** User-facing name (matches DISPLAY_NAME in the CMake packaging call). */
  displayName: string;
  tagline: string;
  description: string;
  version: string;
  /**
   * File name of the tool's mark in public/, shown next to its name. Copied
   * there from the app's generated icons by `make web-icons` — see
   * docs/graphics/README.md. Regenerate rather than editing the file.
   *
   * A bare file name, not a path: the site is served from a subpath, so
   * ToolCard resolves it through asset() in src/lib/url.ts.
   */
  icon: string;
  downloads: Download[];
}

function buildFileName(slug: string, version: string, os: string, arch: string, ext: string): string {
  return `${slug}-${version}-${os}-${arch}.${ext}`;
}

/** The release tag a tool's assets hang off: rotreader-v0.1.0. */
function releaseTag(slug: string, version: string): string {
  return `${slug}-v${version}`;
}

function downloadUrl(slug: string, version: string, file: string): string {
  return `${DOWNLOAD_BASE}/${releaseTag(slug, version)}/${file}`;
}

const ROTREADER_VERSION = '0.1.0';

export const tools: Tool[] = [
  {
    slug: 'rotreader',
    displayName: 'ℜ⛤𝔗 reader',
    tagline: 'A native Markdown viewer.',
    description:
      'Source panels show the raw Markdown and the generated HTML side by side. Rendering happens locally in a native WebView — nothing is uploaded anywhere.',
    version: ROTREADER_VERSION,
    icon: 'rotreader-rottools-logo.svg',
    downloads: [
      {
        platform: 'macos',
        label: 'macOS',
        note: 'Apple Silicon · .dmg',
        href: downloadUrl(
          'rotreader',
          ROTREADER_VERSION,
          buildFileName('rotreader', ROTREADER_VERSION, 'macos', 'arm64', 'dmg'),
        ),
      },
      {
        platform: 'windows',
        label: 'Windows',
        note: '64-bit · installer',
        href: downloadUrl(
          'rotreader',
          ROTREADER_VERSION,
          buildFileName('rotreader', ROTREADER_VERSION, 'windows', 'amd64', 'exe'),
        ),
      },
      {
        platform: 'linux',
        label: 'Linux',
        note: 'Debian / Ubuntu · .deb',
        href: downloadUrl(
          'rotreader',
          ROTREADER_VERSION,
          buildFileName('rotreader', ROTREADER_VERSION, 'linux', 'x86_64', 'deb'),
        ),
      },
    ],
  },
];
