# Versioning

`yai-cli` follows Semantic Versioning.

Public release baseline: `v0.1.0` (2026-02-17).

## SemVer

- `MAJOR`: incompatible public CLI changes
- `MINOR`: backward-compatible additions
- `PATCH`: non-breaking fixes, packaging, or documentation updates

## Specs Coordination

Each CLI release must be evaluated against the pinned `deps/yai-law` revision.
Contract-breaking changes require a MAJOR bump or an explicit compatibility range update.

Current contract line: `SPECS_API_VERSION=v1`.
