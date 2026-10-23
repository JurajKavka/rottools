#!/usr/bin/env zsh
# Pull the merged version bump and push the corresponding per-tool release tag.
#
# Usage:
#   scripts/release-tag.zsh <tool>
#
# Example:
#   scripts/release-tag.zsh rotreader
set -e
set -u
set -o pipefail

usage() {
    echo "usage: scripts/release-tag.zsh <tool>" >&2
}

if [ "$#" -ne 1 ]; then
    usage
    exit 2
fi

TOOL="$1"
if [[ ! "$TOOL" =~ ^[a-z][a-z0-9-]*$ ]]; then
    echo "invalid tool name: $TOOL" >&2
    exit 2
fi

SCRIPT_DIR="${0:A:h}"
ROOT="${SCRIPT_DIR:h}"

cd "$ROOT"

if [ -n "$(git status --porcelain)" ]; then
    echo "working tree is not clean; commit or stash changes before creating a release tag" >&2
    exit 1
fi

git remote get-url origin >/dev/null
git switch main
git pull --ff-only origin main

LOCAL_HEAD="$(git rev-parse HEAD)"
REMOTE_HEAD="$(git rev-parse refs/remotes/origin/main)"
if [ "$LOCAL_HEAD" != "$REMOTE_HEAD" ]; then
    echo "local main is not synchronized with origin/main" >&2
    exit 1
fi

VERSION_FILE="apps/$TOOL/VERSION"
RELEASE_WORKFLOW=".github/workflows/release-$TOOL.yml"
if [ ! -f "$VERSION_FILE" ]; then
    echo "version file not found: $VERSION_FILE" >&2
    exit 1
fi
if [ ! -f "$RELEASE_WORKFLOW" ]; then
    echo "release workflow not found: $RELEASE_WORKFLOW" >&2
    exit 1
fi

VERSION="$(tr -d '\r\n' < "$VERSION_FILE")"
SEMVER_COMPONENT='(0|[1-9][0-9]*)'
if [[ ! "$VERSION" =~ ^${SEMVER_COMPONENT}\.${SEMVER_COMPONENT}\.${SEMVER_COMPONENT}$ ]]; then
    echo "invalid version in $VERSION_FILE: $VERSION" >&2
    exit 1
fi

TAG="$TOOL-v$VERSION"
if git show-ref --verify --quiet "refs/tags/$TAG"; then
    echo "local tag already exists: $TAG" >&2
    exit 1
fi

REMOTE_TAG="$(git ls-remote --tags --refs origin "refs/tags/$TAG")"
if [ -n "$REMOTE_TAG" ]; then
    echo "remote tag already exists: $TAG" >&2
    exit 1
fi

git tag --annotate "$TAG" --message "$TOOL $VERSION"
git push origin "refs/tags/$TAG"

echo "pushed release tag: $TAG"
