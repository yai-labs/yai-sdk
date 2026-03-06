# Versioning

`yai-sdk` follows Semantic Versioning.

Public release baseline: `v0.1.0` (2026-02-17).

## SemVer

- `MAJOR`: incompatible public SDK changes (contract/API/ABI)
- `MINOR`: backward-compatible additions
- `PATCH`: non-breaking fixes, packaging, docs

## Public Surface Contract (SDK)
The official SDK public surface is defined by:

- `include/yai_sdk/public.h` (canonical)
- `docs/SDK_SURFACE_CONTRACT.md`
- `docs/SDK_API_DISCIPLINE.md`

A **MAJOR** bump is required if a breaking change occurs in anything reachable from `yai_sdk/public.h`, including:
- removal/rename of functions/types/macros
- signature changes
- rc-policy semantic changes
- ABI changes to public structs (opaque handles preferred)

## Contracts coordination (law pin)
Each SDK release must be evaluated against the pinned `deps/yai-law` revision.

Contract-breaking changes require:
- a MAJOR bump, or
- an explicit compatibility-range policy update (if introduced later)

Current contract line: `SPECS_API_VERSION=v1`.

## Notes
- Docs-only changes are typically PATCH.
- Internal headers not reachable from `yai_sdk/public.h` are outside the public contract.
