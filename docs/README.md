# yai-sdk Documentation

This directory contains the authoritative programmatic documentation for `yai-sdk`.
The SDK is the bridge between pinned `yai-law` artifacts and command surfaces (`yai-cli`, future TUI/GUI bindings).

## Start Here

- `SDK_API_DISCIPLINE.md` - API classes and boundary rules
- `SDK_SURFACE_CONTRACT.md` - public C surface guarantees
- `THREADING_MEMORY.md` - thread-safety and ownership semantics
- `INTEGRATION_EXAMPLES.md` - example map and usage
- `reports/sdk_api_inventory_v1.md` - current public/internal inventory
- `reports/dx_reliability_checklist.md` - DX/reliability implementation status

## Build and Verification

- `make test` - smoke and boundary tests
- `make check` - ASAN/UBSAN validation
- `make coverage` - gcovr report (if available)
- `make docs` - Doxygen HTML reference
- `make examples` - compile integration examples

## Scope and Authority

- This docs tree is **programmatic**, not normative.
- Normative command/reply/workspace contracts remain owned by pinned `yai-law`.
- If SDK docs diverge from pinned law artifacts, pinned law prevails.
