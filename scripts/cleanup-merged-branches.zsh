#!/usr/bin/env zsh
# Delete local and origin branches whose tips are already contained in main.
#
# Usage:
#   scripts/cleanup-merged-branches.zsh [--yes]
set -e
set -u
set -o pipefail

usage() {
    echo "usage: scripts/cleanup-merged-branches.zsh [--yes]" >&2
}

ASSUME_YES=0
if [ "$#" -gt 1 ]; then
    usage
    exit 2
fi
if [ "$#" -eq 1 ]; then
    case "$1" in
        --yes) ASSUME_YES=1 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "unknown option: $1" >&2; usage; exit 2 ;;
    esac
fi

SCRIPT_DIR="${0:A:h}"
ROOT="${SCRIPT_DIR:h}"

cd "$ROOT"

if [ -n "$(git status --porcelain)" ]; then
    echo "working tree is not clean; commit or stash changes before cleaning branches" >&2
    exit 1
fi

git remote get-url origin >/dev/null
git switch main
git fetch --prune origin
git merge --ff-only refs/remotes/origin/main

LOCAL_HEAD="$(git rev-parse HEAD)"
REMOTE_HEAD="$(git rev-parse refs/remotes/origin/main)"
if [ "$LOCAL_HEAD" != "$REMOTE_HEAD" ]; then
    echo "local main is not synchronized with origin/main" >&2
    exit 1
fi

local_branches=()
while IFS= read -r branch; do
    if [ -n "$branch" ] && [ "$branch" != "main" ]; then
        local_branches+=("$branch")
    fi
done < <(git for-each-ref --merged=refs/remotes/origin/main \
    --format='%(refname:strip=2)' refs/heads)

remote_branches=()
while IFS= read -r branch; do
    if [ -n "$branch" ] && [ "$branch" != "HEAD" ] && [ "$branch" != "main" ]; then
        remote_branches+=("$branch")
    fi
done < <(git for-each-ref --merged=refs/remotes/origin/main \
    --format='%(refname:strip=3)' refs/remotes/origin)

if [ "${#local_branches[@]}" -eq 0 ] && [ "${#remote_branches[@]}" -eq 0 ]; then
    echo "no merged branches to delete"
    exit 0
fi

echo "Merged local branches:"
if [ "${#local_branches[@]}" -eq 0 ]; then
    echo "  (none)"
else
    printf '  %s\n' "${local_branches[@]}"
fi

echo "Merged origin branches:"
if [ "${#remote_branches[@]}" -eq 0 ]; then
    echo "  (none)"
else
    printf '  %s\n' "${remote_branches[@]}"
fi

if [ "$ASSUME_YES" -eq 0 ]; then
    if [ ! -t 0 ]; then
        echo "confirmation requires a terminal; rerun with --yes for non-interactive use" >&2
        exit 1
    fi

    reply=""
    read "reply?Delete these merged branches? [y/N] "
    case "$reply" in
        y|Y|yes|YES) ;;
        *) echo "branch cleanup cancelled"; exit 0 ;;
    esac
fi

# Refresh before the remote deletion and retain only refs that still exist and
# are still merged. This protects against a branch advancing while the prompt
# is waiting for confirmation.
git fetch --prune origin
remote_refs=()
for branch in "${remote_branches[@]}"; do
    remote_ref="refs/remotes/origin/$branch"
    if git show-ref --verify --quiet "$remote_ref" && \
       git merge-base --is-ancestor "$remote_ref" refs/remotes/origin/main; then
        remote_refs+=("refs/heads/$branch")
    else
        echo "skipping origin/$branch because it no longer exists or is no longer merged" >&2
    fi
done

if [ "${#remote_refs[@]}" -gt 0 ]; then
    git push origin --delete "${remote_refs[@]}"
fi

if [ "${#local_branches[@]}" -gt 0 ]; then
    git branch -d -- "${local_branches[@]}"
fi

echo "merged branch cleanup complete"
