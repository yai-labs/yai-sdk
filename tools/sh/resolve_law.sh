#!/usr/bin/env bash
set -euo pipefail

# Deprecated helper: compatibility/tooling only.
# This script must not be used as structural runtime dependency.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

if [[ -n "${YAI_SDK_COMPAT_REGISTRY_DIR:-}" && -f "${YAI_SDK_COMPAT_REGISTRY_DIR}/commands.v1.json" ]]; then
  echo "${YAI_SDK_COMPAT_REGISTRY_DIR}"
  exit 0
fi

CANDIDATES=(
  "${REPO_ROOT}/compat/law-export/registry"
  "${REPO_ROOT}/deps/yai-law/registry"
  "${REPO_ROOT}/../yai-law/registry"
)

for p in "${CANDIDATES[@]}"; do
  if [[ -f "${p}/primitives.v1.json" ]]; then
    echo "${p}"
    exit 0
  fi
done

echo "WARN: compatibility registry export not found."
echo "This helper is optional and deprecated as a structural dependency mechanism." >&2
exit 2
