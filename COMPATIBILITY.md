# Compatibility

This document defines the compatibility contract for **yai-sdk**.

Status: Foundation  
Scope: Public C API surface used by `yai-cli` and external consumers.

## Rule 0 — What is public
Canonical public include:

- `#include <yai_sdk/public.h>`

Compatibility wrapper (supported, but secondary):

- `#include <yai_sdk/yai.h>`

`yai_sdk/yai.h` must remain a backward-compatible wrapper over `yai_sdk/public.h`.
Anything not reachable through `yai_sdk/public.h` is outside the default compatibility contract unless explicitly documented.

## Semantic Versioning
`yai-sdk` follows SemVer:

- **MAJOR**: breaking changes to public contract/API/ABI
- **MINOR**: backward-compatible additions
- **PATCH**: backward-compatible fixes, docs, packaging

## What is breaking
A change is breaking when it affects the public surface and does any of the following:

- removes or renames exported functions/types/macros
- changes function signatures
- changes documented return-code semantics
- changes ABI layout of public structs (opaque handles are preferred)
- tightens validation so previously valid inputs are rejected without a MAJOR bump

## What is compatible
The following are compatible (no MAJOR bump):

- adding new functions
- adding new enum values without altering behavior of existing values
- bug fixes preserving documented behavior and rc policy
- adding new headers if consumed through `yai_sdk/public.h`

## Return-code policy (enterprise)
SDK operations that surface to CLI must respect these rc classes:

- `0` success
- `2` usage/contract violation (bad args, unmapped command id, error payload in success flow)
- `3` dependency missing (registry unavailable/missing data)
- `4` runtime not ready (handshake failed/incompatible protocol/runtime not ready)
- `107 (ENOTCONN)` server unavailable (cannot connect root socket)

If an operation emits JSON with `"status":"error"`, it must not return `0`.

## Dependency chain and pinning
Authoritative chain:

`yai-law` -> `yai-sdk` -> `yai-cli` -> `yai` -> `yai-ops`

- `yai-sdk` pins `deps/yai-law` and consumes contracts from it.
- `yai-cli` must not re-implement contract logic owned by `yai-sdk`.
- Pin mismatches across repos must be detected by CI/parity gates.

## Evidence expectations
Every public-surface change must ship with evidence:

- `make test`
- `./build/tests/sdk_smoke` (or equivalent)
- CI logs including SDK version and law pin context
