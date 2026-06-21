#!/usr/bin/env bash
# Cut a new Bricktron release: tag it, repin the Flatpak manifest, and build the
# distributable single-file bundle (bricktron.flatpak).
#
# Usage:
#   ./release.sh vX.Y        e.g. ./release.sh v1.1
#
# Run this AFTER you have committed and pushed all the code for the release.
# The script refuses to run on a dirty or unpushed tree so the tag and the
# manifest pin always point at exactly what is on the remote.
set -euo pipefail

VERSION="${1:?usage: ./release.sh vX.Y  (e.g. ./release.sh v1.1)}"
case "$VERSION" in
  v[0-9]*) ;;
  *) echo "ERROR: version must look like v1.1" >&2; exit 1 ;;
esac

cd "$(cd "$(dirname "$0")" && pwd)"
MANIFEST="io.github.arturkos.Bricktron.yaml"
APP_ID="io.github.arturkos.Bricktron"
BRANCH="$(git rev-parse --abbrev-ref HEAD)"

# --- 1. refuse dirty / unpushed tree -----------------------------------------
if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
  echo "ERROR: uncommitted changes — commit & push the release code first." >&2
  git status --short
  exit 1
fi
git fetch --quiet origin
if [ "$(git rev-parse @)" != "$(git rev-parse '@{u}' 2>/dev/null || echo none)" ]; then
  echo "ERROR: $BRANCH is not in sync with origin — push first." >&2
  exit 1
fi
if git rev-parse "$VERSION" >/dev/null 2>&1; then
  echo "ERROR: tag $VERSION already exists." >&2
  exit 1
fi

COMMIT="$(git rev-parse HEAD)"

# --- 2. tag the release commit -----------------------------------------------
echo ">> Tagging $VERSION at $COMMIT"
git tag -a "$VERSION" -m "Release $VERSION"
git push origin "$VERSION"

# --- 3. repin the manifest to the freshly tagged commit ----------------------
# Only the bricktron git source carries tag:/commit: keys, so anchored sed on the
# indented lines is safe.
echo ">> Repinning $MANIFEST -> tag $VERSION / $COMMIT"
sed -i -E "s|^( *tag: ).*|\1$VERSION|" "$MANIFEST"
sed -i -E "s|^( *commit: ).*|\1$COMMIT|" "$MANIFEST"
git add "$MANIFEST"
git commit -m "Bump Flatpak manifest to $VERSION"
git push origin "$BRANCH"

# --- 4. build the distributable bundle ---------------------------------------
echo ">> Building bundle from the pinned commit"
flatpak-builder --user --repo=repo --force-clean build-flatpak "$MANIFEST"
flatpak build-bundle repo bricktron.flatpak "$APP_ID"

echo
echo ">> Bundle ready:"
ls -lh bricktron.flatpak

# --- 5. publish to a GitHub Release ------------------------------------------
if command -v gh >/dev/null 2>&1; then
  echo ">> Creating GitHub release $VERSION and uploading the bundle"
  gh release create "$VERSION" bricktron.flatpak \
    --title "Bricktron $VERSION" \
    --notes "Single-file Flatpak bundle. Install with: flatpak install --user bricktron.flatpak"
else
  cat <<EOF

NOTE: 'gh' CLI not found — upload the bundle by hand:
  https://github.com/ArturKos/bricktron/releases/new?tag=$VERSION
  -> attach bricktron.flatpak

Also remember to add a <release version="${VERSION#v}" .../> entry to
io.github.arturkos.Bricktron.metainfo.xml for this version.
EOF
fi
