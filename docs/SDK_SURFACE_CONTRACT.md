# YAI SDK Surface Contract (v0)

Status: Foundation / Public API  
Scope: `yai-sdk` public C API used by `yai-cli` and external consumers.

## Rule 0 — Single include
Consumers MUST include only:
- `#include <yai_sdk/yai.h>`

Direct inclusion of sub-headers is considered “advanced use” and is NOT covered by the compatibility contract unless explicitly documented.

## Compatibility promise
This document defines what “compatible” means for the SDK public surface.

Compatible changes (no major bump):
- Adding new functions.
- Adding new enum values (when it does not change behavior of existing values).
- Adding new fields ONLY inside structs not exposed in public headers (opaque handles preferred).
- Bug fixes that preserve documented return-code semantics.

Breaking changes (major bump):
- Removing/renaming any function/type/macro included by `yai_sdk/yai.h`.
- Changing semantics of return codes for documented operations.
- Changing struct layout/types exposed by `yai_sdk/yai.h`.

## Return-code policy (enterprise)
- `0` means success.
- `2` means usage/contract violation (invalid args, unmapped command id, server returned error payload for a “success path”).
- `3` means dependency missing (registry unavailable, missing data).
- `4` means runtime not ready (handshake failure / incompatible protocol).
- `107 (ENOTCONN)` means server unavailable (cannot connect to root socket).

Any CLI-facing operation MUST honor this mapping.

## Public modules (v0)
The following modules are included in `yai_sdk/yai.h` and are public:

- Errors: `yai_sdk/errors.h`
- Paths: `yai_sdk/paths.h`
- Registry: `yai_sdk/registry/*` (lookup/help/validate)
- Executor: `yai_sdk/ops/executor.h`
- Dispatch: `yai_sdk/ops/ops_dispatch.h`
- RPC client: `yai_sdk/rpc/rpc.h`

## Opaque handles rule
When an API needs state that may change, the public surface MUST use opaque handles (`typedef struct yai_xxx yai_xxx_t;`) and provide create/destroy functions.

## Evidence
- `make test`
- `./build/tests/sdk_smoke`