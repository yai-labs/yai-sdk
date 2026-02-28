#!/usr/bin/env bash
set -euo pipefail

# repo root = directory dove sta questo script: tools/sh/resolve_law.sh -> risali di 3
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

CANDIDATES=(
  "${REPO_ROOT}/deps/yai-law"
  "${REPO_ROOT}/../yai-law"
)

for p in "${CANDIDATES[@]}"; do
  if [[ -f "${p}/registry/primitives.v1.json" ]]; then
    echo "${p}"
    exit 0
  fi
done

echo "FATAL: yai-law not found."
echo "Looked for:"
printf ' - %s\n' "${CANDIDATES[@]}"
echo "Expected file: registry/primitives.v1.json"
exit 1