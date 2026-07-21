/**
 * Single source of truth for the site's tool list and download links.
 *
 * File names mirror CPack's CPACK_PACKAGE_FILE_NAME from
 * cmake/RotToolsPackaging.cmake:
 *
 *     <exe>-<version>-<os>-<arch>.<ext>
 *
 * where <os> is macos|linux|windows and <arch> is a lowercased
 * CMAKE_SYSTEM_PROCESSOR. Keep buildFileName() in step with that if the
 * packaging ever changes.
 */

/**
 * Packages are served straight off GitHub Releases — the assets that
 * .github/workflows/release-<tool>.yml uploads. No separate download host.
 */
export const REPO_URL = 'https://github.com/JurajKavka/rottools';

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
  /**
   * Icon file name inside the repo-root assets/ folder. copy-assets.mjs mirrors
   * that folder into public/, so the file is served at <base>/<icon>.
   */
  icon: string;
  tagline: string;
  description: string;
  version: string;
  downloads: Download[];
}

function buildFileName(slug: string, version: string, os: string, arch: string, ext: string): string {
  return `${slug}-${version}-${os}-${arch}.${ext}`;
}

/**
 * Release tags are app-prefixed (`rotreader-v0.1.0`) so each tool versions
 * independently — see the `tags:` filter in release-<tool>.yml.
 */
function downloadUrl(slug: string, version: string, file: string): string {
  return `${REPO_URL}/releases/download/${slug}-v${version}/${file}`;
}

const ROTREADER_VERSION = '0.1.0';

export const tools: Tool[] = [
  {
    slug: 'rotreader',
    displayName: 'ℜ⛤𝔗 reader',
    icon: 'rotreader-rottools-accent.svg',
    tagline: 'A native Markdown viewer.',
    description:
      'Source panels show the raw Markdown and the generated HTML side by side. Rendering happens locally in a native WebView — nothing is uploaded anywhere.',
    version: ROTREADER_VERSION,
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
