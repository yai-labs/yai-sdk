# Compatibility

This document defines the compatibility contract for **yai-sdk**.

Status: Foundation  
Scope: Public C API surface used by `yai-cli` and external consumers.

## Rule 0 — What is “public”
The **only** supported public surface is the entry header:

- `#include <yai_sdk/yai.h>`

Anything not reachable through `yai_sdk/yai.h` is **not** covered by the compatibility contract unless explicitly stated.

## Semantic Versioning
yai-sdk follows Semantic Versioning (SemVer):

- **MAJOR**: breaking changes to the public surface contract
- **MINOR**: backward-compatible additions
- **PATCH**: backward-compatible fixes, documentation, build/packaging

## What is breaking
A change is **breaking** if it applies to the public surface (reachable from `yai_sdk/yai.h`) and it does any of the following:

- Removes or renames exported functions/types/macros.
- Changes a function signature.
- Changes documented return-code semantics.
- Changes ABI layout of a public struct (prefer opaque handles to avoid this).
- Tightens validation in a way that rejects previously valid inputs **without** a major bump.

## What is compatible
The following are **compatible** (no major bump):

- Adding new functions.
- Adding new enum values (without changing behavior of existing ones).
- Bug fixes that preserve documented behavior and rc policy.
- Adding new headers only if consumed through `yai_sdk/yai.h`.

## Return-code policy (enterprise)
SDK operations that surface to CLI MUST respect these rc classes:

- `0` success
- `2` usage / contract violation (bad args, unmapped command id, server returned error payload)
- `3` dependency missing (registry unavailable / missing data)
- `4` runtime not ready (handshake failed / incompatible protocol / runtime not ready)
- `107 (ENOTCONN)` server unavailable (cannot connect root socket)

If an operation prints a JSON payload with `"status":"error"`, it MUST NOT return `0`.

## Dependency chain + pinning
Authoritative chain:

`yai-law` → `yai-sdk` → `yai-cli`

- `yai-sdk` pins `deps/yai-law` and consumes contracts from it.
- `yai-cli` must not re-implement contract logic already owned by `yai-sdk`.
- Pin mismatches across repos must be detected by CI (see parity gates).

## Evidence expectations
Every change that affects the public surface must ship with evidence:

- `make test`
- `./build/tests/sdk_smoke` (or equivalent)
- Version + pin printed in CI logs (added by parity gates)