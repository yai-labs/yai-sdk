# yai-sdk Documentation

This directory contains the authoritative documentation entrypoints for SDK consumers and maintainers.
The SDK is the programmatic bridge between `yai-law` taxonomy/contracts and YAI command surfaces.

## Start here

- `SDK_API_DISCIPLINE.md` - API boundary policy (`public_stable`, `compat`, `internal`)
- `SDK_SURFACE_CONTRACT.md` - public C surface contract, compatibility semantics, ABI expectations
- `reports/sdk_api_inventory_v1.md` - concrete header/module inventory for v1 discipline

## Documentation intent

SDK documentation is implementation-guiding and consumer-facing, but never normative over `yai-law`.
When a conflict exists between SDK docs and pinned law artifacts, pinned law prevails.

## Cross-repo alignment

This documentation is maintained to stay consistent with:
- `yai-law` (normative contracts)
- `yai-cli` (operator consumption)
- `yai` (runtime surfaces)
- `yai-ops` (evidence and operational validation)
