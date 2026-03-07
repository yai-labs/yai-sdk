# SDK API Inventory v1

Generated for API Discipline v1.

## Canonical Public Headers (`public_stable`)

- `include/yai_sdk/public.h`
- `include/yai_sdk/errors.h`
- `include/yai_sdk/paths.h`
- `include/yai_sdk/context.h`
- `include/yai_sdk/client.h`
- `include/yai_sdk/catalog.h`
- `include/yai_sdk/protocol.h`
- `include/yai_sdk/rpc.h`
- `include/yai_sdk/log.h`
- `include/yai_sdk/reply/reply.h`
- `include/yai_sdk/reply/reply_builder.h`
- `include/yai_sdk/reply/reply_json.h`

## Internal Headers (`internal`)

- `include/yai_sdk/registry/registry.h`
- `include/yai_sdk/registry/registry_help.h`
- `include/yai_sdk/registry/registry_paths.h`
- `include/yai_sdk/registry/registry_cache.h`
- `include/yai_sdk/registry/registry_registry.h`
- `include/yai_sdk/registry/registry_types.h`
- `include/yai_sdk/registry/registry_validate.h`

## Module Classification

- `client` -> `public_stable`
- `catalog/help_index` -> `public_stable`
- `context/workspace` -> `public_stable`
- `reply` -> `public_stable`
- `protocol/rpc` -> `public_stable`
- `logging` -> `public_stable`
- `registry raw loaders/validators` -> `internal`

## Examples and Wrappers

- Examples: `examples/01_basic_connection.c`, `examples/02_workspace_context.c`, `examples/03_custom_control_call.c`
- Wrapper skeleton: `wrappers/python/yai_sdk.py`

## CLI Coupling Status

`yai-cli` should consume only `public_stable` headers (prefer `yai_sdk/public.h`).
No `yai_sdk/registry/*` include is allowed in CLI production code.
