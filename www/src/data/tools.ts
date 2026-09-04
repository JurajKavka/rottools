import rotpadVersionFile from '../../../apps/rotpad/VERSION?raw';
import rotreaderVersionFile from '../../../apps/rotreader/VERSION?raw';

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
 *   asset  <slug>-<version>-<package>-<arch>.<ext>
 *          CPack's CPACK_PACKAGE_FILE_NAME from cmake/RotToolsPackaging.cmake,
 *          where <package> is normally macos|linux|windows (or a deliberate
 *          variant such as macos-26) and <arch> is a lowercased
 *          CMAKE_SYSTEM_PROCESSOR.
 *
 * Keep releaseTag() and buildFileName() in step if either ever changes.
 */

/** GitHub's release asset root for this repo. No trailing slash. */
export const DOWNLOAD_BASE = 'https://github.com/JurajKavka/rottools/releases/download';

/** Every release, for the "all downloads" case. */
export const RELEASES_URL = 'https://github.com/JurajKavka/rottools/releases';

export type Platform = 'macos' | 'windows' | 'linux';

export interface Download {
  platform: Platform;
  /** Package choice shown inside its platform box, e.g. "MacOS 26" or "Portable". */
  label: string;
  /** Supporting package details, e.g. "Apple Silicon · .dmg". */
  note: string;
  href: string;
}

export interface Tool {
  /** CMake target / binary name, and the URL slug. */
  slug: string;
  /** User-facing marketing name; the packaged app name may use plain ASCII. */
  displayName: string;
  tagline: string;
  description?: string;
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

function buildFileName(slug: string, version: string, packageSlug: string, arch: string, ext: string): string {
  return `${slug}-${version}-${packageSlug}-${arch}.${ext}`;
}

/** The release tag a tool's assets hang off: rotreader-v0.1.0. */
function releaseTag(slug: string, version: string): string {
  return `${slug}-v${version}`;
}

function downloadUrl(slug: string, version: string, file: string): string {
  return `${DOWNLOAD_BASE}/${releaseTag(slug, version)}/${file}`;
}

const ROTREADER_VERSION = rotreaderVersionFile.trim();
const ROTPAD_VERSION = rotpadVersionFile.trim();

export const tools: Tool[] = [
  {
    slug: 'rotreader',
    displayName: 'ROT reader',
    tagline: 'A Markdown reader that can edit too.',
    version: ROTREADER_VERSION,
    icon: 'rotreader-rottools-logo.svg',
    downloads: [
      {
        platform: 'macos',
        label: 'MacOS 26',
        note: 'Apple Silicon · .dmg',
        href: downloadUrl(
          'rotreader',
          ROTREADER_VERSION,
          buildFileName('rotreader', ROTREADER_VERSION, 'macos-26', 'arm64', 'dmg'),
        ),
      },
      {
        platform: 'macos',
        label: 'MacOS 15+',
        note: 'Apple Silicon · .dmg',
        href: downloadUrl(
          'rotreader',
          ROTREADER_VERSION,
          buildFileName('rotreader', ROTREADER_VERSION, 'macos', 'arm64', 'dmg'),
        ),
      },
      {
        platform: 'windows',
        label: 'Installer',
        note: '64-bit · .exe',
        href: downloadUrl(
          'rotreader',
          ROTREADER_VERSION,
          buildFileName('rotreader', ROTREADER_VERSION, 'windows', 'amd64', 'exe'),
        ),
      },
      {
        platform: 'windows',
        label: 'Portable',
        note: '64-bit · .zip',
        href: downloadUrl(
          'rotreader',
          ROTREADER_VERSION,
          buildFileName('rotreader', ROTREADER_VERSION, 'windows', 'amd64', 'zip'),
        ),
      },
      {
        platform: 'linux',
        label: 'Debian / Ubuntu',
        note: 'x86-64 · .deb',
        href: downloadUrl(
          'rotreader',
          ROTREADER_VERSION,
          buildFileName('rotreader', ROTREADER_VERSION, 'linux', 'x86_64', 'deb'),
        ),
      },
      {
        platform: 'linux',
        label: 'Archive',
        note: 'x86-64 · .tar.gz',
        href: downloadUrl(
          'rotreader',
          ROTREADER_VERSION,
          buildFileName('rotreader', ROTREADER_VERSION, 'linux', 'x86_64', 'tar.gz'),
        ),
      },
    ],
  },
  {
    slug: 'rotpad',
    displayName: 'ROT pad',
    tagline: 'The very simple, enjoyable text editor.',
    version: ROTPAD_VERSION,
    icon: 'rotpad-rottools-logo.svg',
    downloads: [
      {
        platform: 'macos',
        label: 'Apple Silicon',
        note: '.dmg',
        href: downloadUrl(
          'rotpad',
          ROTPAD_VERSION,
          buildFileName('rotpad', ROTPAD_VERSION, 'macos', 'arm64', 'dmg'),
        ),
      },
      {
        platform: 'windows',
        label: 'Installer',
        note: '64-bit · .exe',
        href: downloadUrl(
          'rotpad',
          ROTPAD_VERSION,
          buildFileName('rotpad', ROTPAD_VERSION, 'windows', 'amd64', 'exe'),
        ),
      },
      {
        platform: 'windows',
        label: 'Portable',
        note: '64-bit · .zip',
        href: downloadUrl(
          'rotpad',
          ROTPAD_VERSION,
          buildFileName('rotpad', ROTPAD_VERSION, 'windows', 'amd64', 'zip'),
        ),
      },
      {
        platform: 'linux',
        label: 'Debian / Ubuntu',
        note: 'x86-64 · .deb',
        href: downloadUrl(
          'rotpad',
          ROTPAD_VERSION,
          buildFileName('rotpad', ROTPAD_VERSION, 'linux', 'x86_64', 'deb'),
        ),
      },
      {
        platform: 'linux',
        label: 'Archive',
        note: 'x86-64 · .tar.gz',
        href: downloadUrl(
          'rotpad',
          ROTPAD_VERSION,
          buildFileName('rotpad', ROTPAD_VERSION, 'linux', 'x86_64', 'tar.gz'),
        ),
      },
    ],
  },
];
