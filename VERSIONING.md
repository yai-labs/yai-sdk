# Versioning

`yai-sdk` follows Semantic Versioning.

Public release baseline: `v0.1.0` (2026-02-17).

## SemVer

- `MAJOR`: incompatible public SDK changes (surface contract)
- `MINOR`: backward-compatible additions
- `PATCH`: non-breaking fixes, packaging, or documentation updates

## Public Surface Contract (SDK)
The official SDK public surface is defined by:

- `include/yai_sdk/yai.h`
- `docs/SDK_SURFACE_CONTRACT.md`

A **MAJOR** bump is required if a breaking change occurs in anything reachable from `yai_sdk/yai.h`, including:
- removal/rename of functions/types/macros
- signature changes
- rc-policy semantic changes
- ABI changes to public structs (prefer opaque handles)

## Contracts Coordination (yai-law pin)
Each SDK release must be evaluated against the pinned `deps/yai-law` revision.

Contract-breaking changes require:
- a MAJOR bump, or
- an explicit compatibility range update (only if you formalize ranges later)

Current contract line: `SPECS_API_VERSION=v1`.

## Notes
- “Docs-only” changes are typically PATCH.
- Internal headers not reachable from `yai_sdk/yai.h` are not covered by the public contract.