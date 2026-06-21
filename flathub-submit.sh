#!/usr/bin/env bash
# Helper: prepare a Flathub submission PR for io.github.arturkos.Bricktron.
#
# ONE-TIME MANUAL STEP FIRST:
#   Fork https://github.com/flathub/flathub on the GitHub website
#   (button "Fork"). The script clones YOUR fork, not the upstream.
#
# After the script finishes, open the PR it prints:
#   from branch  io.github.arturkos.Bricktron
#   onto branch  new-pr   in  flathub/flathub
set -euo pipefail

APP_ID="io.github.arturkos.Bricktron"
MANIFEST="$(cd "$(dirname "$0")" && pwd)/${APP_ID}.yaml"

# Your fork of flathub/flathub. Uses the same SSH host alias as the bricktron
# remote (github-arturkos -> github.com/ArturKos). Change if you forked under
# a different account.
FORK_SSH="github-arturkos:ArturKos/flathub.git"
UPSTREAM="https://github.com/flathub/flathub.git"

WORKDIR="${1:-$HOME/flathub-submit}"

if [ ! -f "$MANIFEST" ]; then
  echo "ERROR: manifest not found at $MANIFEST" >&2
  exit 1
fi

# A GitHub fork copies only the default branch (master), but submissions must
# be based on the upstream 'new-pr' branch, so fetch it from upstream directly.
echo ">> Cloning your flathub fork into $WORKDIR"
if [ -d "$WORKDIR/.git" ]; then
  echo "   $WORKDIR already exists, reusing it."
  cd "$WORKDIR"
else
  git clone "$FORK_SSH" "$WORKDIR"
  cd "$WORKDIR"
fi

echo ">> Fetching new-pr from upstream flathub/flathub"
git remote add upstream "$UPSTREAM" 2>/dev/null || git remote set-url upstream "$UPSTREAM"
git fetch upstream new-pr

echo ">> Creating branch $APP_ID off upstream/new-pr"
git checkout -B "$APP_ID" upstream/new-pr

echo ">> Copying manifest"
cp "$MANIFEST" "./${APP_ID}.yaml"

echo ">> Committing"
git add "${APP_ID}.yaml"
git commit -m "Add ${APP_ID}"

echo ">> Pushing branch to your fork"
git push -u origin "$APP_ID" --force-with-lease

cat <<EOF

Done. Now open the pull request:

  https://github.com/flathub/flathub/compare/new-pr...ArturKos:${APP_ID}?expand=1

  (base repository: flathub/flathub   base: new-pr
   head repository: ArturKos/flathub  compare: ${APP_ID})

The Flathub build bot will comment with a test build; a reviewer takes it
from there. Respond to review feedback with more commits on this branch.
EOF
