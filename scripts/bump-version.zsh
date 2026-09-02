#!/usr/bin/env zsh
# Prepare and push a per-tool version-bump branch.
#
# Usage:
#   scripts/bump-version.zsh <tool> <X.Y.Z>
#
# Example:
#   scripts/bump-version.zsh rotreader 0.3.0
set -e
set -u
set -o pipefail

usage() {
    echo "usage: scripts/bump-version.zsh <tool> <X.Y.Z>" >&2
}

if [ "$#" -ne 2 ]; then
    usage
    exit 2
fi

TOOL="$1"
VERSION="$2"

if [[ ! "$TOOL" =~ ^[a-z][a-z0-9-]*$ ]]; then
    echo "invalid tool name: $TOOL" >&2
    exit 2
fi

SEMVER_COMPONENT='(0|[1-9][0-9]*)'
if [[ ! "$VERSION" =~ ^${SEMVER_COMPONENT}\.${SEMVER_COMPONENT}\.${SEMVER_COMPONENT}$ ]]; then
    echo "invalid version: $VERSION (expected X.Y.Z)" >&2
    exit 2
fi

SCRIPT_DIR="${0:A:h}"
ROOT="${SCRIPT_DIR:h}"
VERSION_FILE="apps/$TOOL/VERSION"
BRANCH="$TOOL-$VERSION"

cd "$ROOT"

if [ ! -f "$VERSION_FILE" ]; then
    echo "version file not found: $VERSION_FILE" >&2
    exit 1
fi

CURRENT_BRANCH="$(git branch --show-current)"
if [ "$CURRENT_BRANCH" != "main" ]; then
    echo "release branches must be created from main (currently on $CURRENT_BRANCH)" >&2
    exit 1
fi

if [ -n "$(git status --porcelain)" ]; then
    echo "working tree is not clean; commit or stash changes before bumping a version" >&2
    exit 1
fi

CURRENT_VERSION="$(tr -d '\r\n' < "$VERSION_FILE")"
if [[ ! "$CURRENT_VERSION" =~ ^${SEMVER_COMPONENT}\.${SEMVER_COMPONENT}\.${SEMVER_COMPONENT}$ ]]; then
    echo "invalid current version in $VERSION_FILE: $CURRENT_VERSION" >&2
    exit 1
fi

IFS=. read -r current_major current_minor current_patch <<< "$CURRENT_VERSION"
IFS=. read -r new_major new_minor new_patch <<< "$VERSION"
if ((new_major < current_major ||
     (new_major == current_major && new_minor < current_minor) ||
     (new_major == current_major && new_minor == current_minor && new_patch <= current_patch))); then
    echo "new version must be greater than current version $CURRENT_VERSION" >&2
    exit 1
fi

if git show-ref --verify --quiet "refs/heads/$BRANCH"; then
    echo "local branch already exists: $BRANCH" >&2
    exit 1
fi

git remote get-url origin >/dev/null
git fetch --quiet origin refs/heads/main:refs/remotes/origin/main

LOCAL_HEAD="$(git rev-parse HEAD)"
REMOTE_HEAD="$(git rev-parse refs/remotes/origin/main)"
if [ "$LOCAL_HEAD" != "$REMOTE_HEAD" ]; then
    echo "local main is not synchronized with origin/main" >&2
    exit 1
fi

REMOTE_BRANCH="$(git ls-remote --heads origin "refs/heads/$BRANCH")"
if [ -n "$REMOTE_BRANCH" ]; then
    echo "remote branch already exists: $BRANCH" >&2
    exit 1
fi

git switch -c "$BRANCH"
printf '%s\n' "$VERSION" > "$VERSION_FILE"
git add -- "$VERSION_FILE"
git commit -m "bump $VERSION"
git push --set-upstream origin "$BRANCH"

echo "pushed version bump branch: $BRANCH"
