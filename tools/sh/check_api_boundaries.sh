#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

fail() {
  echo "api-boundary-check: $1" >&2
  exit 1
}

PUBLIC_HEADER="$ROOT/include/yai_sdk/public.h"
if rg -n "yai_sdk/registry/" "$PUBLIC_HEADER" >/dev/null 2>&1; then
  fail "public.h must not include registry internal headers"
fi

# Compatibility wrappers must stay wrappers.
if ! rg -n "#include <yai_sdk/public\.h>" "$ROOT/include/yai_sdk/yai_sdk.h" >/dev/null 2>&1; then
  fail "yai_sdk.h must wrap public.h"
fi
if ! rg -n "#include <yai_sdk/public\.h>" "$ROOT/include/yai_sdk/yai.h" >/dev/null 2>&1; then
  fail "yai.h must wrap public.h"
fi

# If sibling CLI repo exists, ensure it does not include SDK registry internals.
CLI_ROOT="$(cd "$ROOT/.." && pwd)/yai-cli"
if [[ -d "$CLI_ROOT" ]]; then
  if rg -n "#include\s+[<\"]yai_sdk/registry/" "$CLI_ROOT/include" "$CLI_ROOT/src" "$CLI_ROOT/tests" -S >/dev/null 2>&1; then
    rg -n "#include\s+[<\"]yai_sdk/registry/" "$CLI_ROOT/include" "$CLI_ROOT/src" "$CLI_ROOT/tests" -S >&2 || true
    fail "yai-cli includes SDK internal registry headers"
  fi
fi

echo "api-boundary-check: ok"
