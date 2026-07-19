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

/** Where the release pipeline rsyncs packages to. No trailing slash. */
export const DOWNLOAD_BASE = 'https://downloads.rottools.example';

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
  downloads: Download[];
}

function buildFileName(slug: string, version: string, os: string, arch: string, ext: string): string {
  return `${slug}-${version}-${os}-${arch}.${ext}`;
}

function downloadUrl(slug: string, file: string): string {
  return `${DOWNLOAD_BASE}/${slug}/${file}`;
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
    downloads: [
      {
        platform: 'macos',
        label: 'macOS',
        note: 'Apple Silicon · .dmg',
        href: downloadUrl(
          'rotreader',
          buildFileName('rotreader', ROTREADER_VERSION, 'macos', 'arm64', 'dmg'),
        ),
      },
      {
        platform: 'windows',
        label: 'Windows',
        note: '64-bit · installer',
        href: downloadUrl(
          'rotreader',
          buildFileName('rotreader', ROTREADER_VERSION, 'windows', 'amd64', 'exe'),
        ),
      },
      {
        platform: 'linux',
        label: 'Linux',
        note: 'Debian / Ubuntu · .deb',
        href: downloadUrl(
          'rotreader',
          buildFileName('rotreader', ROTREADER_VERSION, 'linux', 'x86_64', 'deb'),
        ),
      },
    ],
  },
];
