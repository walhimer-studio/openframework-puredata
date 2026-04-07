#!/usr/bin/env bash
# Run this script in Terminal.app (not inside Cursor), from this folder:
#   cd ~/Desktop/technical-emergent-of && chmod +x push-to-github.sh && ./push-to-github.sh
#
# Pushes to: https://github.com/walhimer-studio/openframework-puredata

set -euo pipefail
REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$REPO_ROOT"

if [[ ! -d .git ]]; then
  git init
  git add -A
  git commit -m "Initial commit: technical-emergent-of (openFrameworks + Pd/OSC)"
  git branch -M main
fi

if git remote get-url origin >/dev/null 2>&1; then
  git remote set-url origin "https://github.com/walhimer-studio/openframework-puredata.git"
else
  git remote add origin "https://github.com/walhimer-studio/openframework-puredata.git"
fi

echo "Pushing to origin main..."
git push -u origin main

echo "Done. Repo: https://github.com/walhimer-studio/openframework-puredata"
