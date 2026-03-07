# YAI SDK Surface Contract (v1)

Status: Foundation / Public API  
Scope: `yai-sdk` public C API used by `yai-cli` and external consumers.

## Rule 0 — Single include
Consumers MUST include only:
- `#include <yai_sdk/public.h>`

Direct inclusion of sub-headers is advanced use and is covered only when the header is explicitly classified as `public_stable` in `SDK_API_DISCIPLINE.md`.

## Compatibility promise
This document defines what “compatible” means for the SDK public surface.

Compatible changes (no major bump):
- Adding new functions.
- Adding new enum values (when it does not change behavior of existing values).
- Adding new fields ONLY inside structs not exposed in public headers (opaque handles preferred).
- Bug fixes that preserve documented return-code semantics.

Breaking changes (major bump):
- Removing/renaming any function/type/macro included by `yai_sdk/public.h`.
- Changing semantics of return codes for documented operations.
- Changing struct layout/types exposed by public-stable headers.

## Return-code policy (enterprise)
- `0` means success.
- `2` means usage/contract violation (invalid args, unmapped command id, server returned error payload for a “success path”).
- `3` means dependency missing (registry unavailable, missing data).
- `4` means runtime not ready (handshake failure / incompatible protocol).
- `107 (ENOTCONN)` means server unavailable (cannot connect to root socket).

Any CLI-facing operation MUST honor this mapping.

## Reply architecture (v1)

- Runtime replies use envelope `yai.exec.reply.v1`.
- SDK exposes two reply views:
  - `yai_sdk_reply_t` from `client.h` (transport-facing)
  - `yai_reply_t` from `reply/reply.h` (builder/serialization-facing)
- Human channels are `summary` and `hints` (or legacy `hint` compatibility).
- `reason` remains the canonical internal cause token.
- `details` and `data` carry diagnostics and machine payloads.

## Public modules (v1)
The following modules are part of the default public surface exposed by `yai_sdk/public.h`:

- Errors: `yai_sdk/errors.h`
- Paths: `yai_sdk/paths.h`
- Protocol client: `yai_sdk/client.h`
- Catalog: `yai_sdk/catalog.h` (command index view)
- RPC client: `yai_sdk/rpc.h`
- Context/workspace: `yai_sdk/context.h`

Raw registry headers under `yai_sdk/registry/*` are advanced internal views. They are not part of the default public-stable surface.

ABI anchors:
- `YAI_SDK_ABI_VERSION`
- `yai_sdk_abi_version()`
- `yai_sdk_version()`
- `yai_sdk_errstr()`

## Opaque handles rule
When an API needs state that may change, the public surface MUST use opaque handles (`typedef struct yai_xxx yai_xxx_t;`) and provide create/destroy functions.

## Registry Views

SDK exposes three distinct registry-derived layers:

1. Raw registry (`registry/*`):
- File/cache loading and structural validation.
- Close to law JSON shape.

2. Normalized catalog (`catalog.h`):
- Stable command view for consumers.
- Includes taxonomy metadata (`surface`, `stability`, `entrypoint`, `topic`, `op`, `domain`, `layer`, `canonical_path`).
- Supports deterministic query/filter APIs and alias/path lookup.

3. Help index (`catalog.h`):
- Hierarchical index `entrypoint -> topic -> op` for CLI/UI help navigation.
- Supports filtered views (surface vs ancillary/plumbing).

Core APIs:
- `yai_sdk_command_catalog_load()/free()`
- `yai_sdk_command_catalog_query()`
- `yai_sdk_command_catalog_find_by_id()`
- `yai_sdk_command_catalog_find_by_canonical_path()`
- `yai_sdk_command_catalog_resolve_path()`
- `yai_sdk_command_catalog_resolve_alias()`
- `yai_sdk_help_index_build()/free()`
- `yai_sdk_help_find_entrypoint()/find_topic()/find_command()`

Operator capability support APIs:

- `yai_sdk_context_get_current_workspace()`
- `yai_sdk_context_set_current_workspace()`
- `yai_sdk_context_clear_current_workspace()`
- `yai_sdk_context_resolve_workspace()`
- `yai_sdk_context_validate_current_workspace()`
- `yai_sdk_workspace_describe()`

## Evidence
- `make test`
- `./build/tests/sdk_smoke`
