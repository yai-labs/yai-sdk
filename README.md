# yai-sdk — Enterprise Contract Client for the YAI Platform

`yai-sdk` is the canonical C SDK for contract-driven interaction with governed YAI runtime surfaces.

It translates pinned law artifacts into a stable client interface consumed by `yai-cli` and external integrators.

## Platform role

Dependency and authority chain:

`yai-law` -> `yai-sdk` -> `yai-cli` -> `yai` -> `yai-ops`

- `yai-law`: canonical normative contracts
- `yai-sdk`: law-constrained API/ABI surface
- `yai-cli`: command surface built on SDK primitives
- `yai`: runtime implementation
- `yai-ops`: operational validation and evidence

`yai-sdk` is not a policy authority.
It is a governed consumer of pinned law.

## Scope

`yai-sdk` provides:
- registry-aware command discovery and validation
- protocol-first client API (`yai_sdk_client_*`) for generic control calls
- RPC transport primitives
- enterprise return-code guarantees for operator/automation integration
- versioned API/ABI anchors for compatibility control

## Public surface contract

Canonical include for public consumers:
- `#include <yai_sdk/public.h>`

Compatibility and surface guarantees:
- `docs/SDK_SURFACE_CONTRACT.md`
- `COMPATIBILITY.md`
- `VERSIONING.md`

## Compatibility and ABI discipline

- Semantic versioning is mandatory.
- Breaking API/ABI changes require major version transitions.
- Return-code semantics are treated as contract, not implementation detail.
- Opaque-handle patterns are preferred for ABI stability.

Key ABI anchors:
- `YAI_SDK_ABI_VERSION`
- `yai_sdk_abi_version()`
- `yai_sdk_version()`
- `yai_sdk_errstr()`

## Governance constraints

- Law pinning is explicit and never floating.
- Behavior that drifts from pinned law is a defect.
- SDK must not redefine normative semantics owned by `yai-law`.

## Evidence expectations

SDK changes affecting public behavior should include:
- `make test`
- `./build/tests/sdk_smoke`
- CI evidence for pin/version visibility

## Documentation entrypoint

See:
- `docs/README.md`
- `docs/SDK_SURFACE_CONTRACT.md`

## License

Apache-2.0. See `LICENSE`.
