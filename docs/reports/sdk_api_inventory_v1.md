# SDK API Inventory v1

Generated for Consegna 13.

## Canonical Public Headers (`public_stable`)

- `include/yai_sdk/public.h`
- `include/yai_sdk/errors.h`
- `include/yai_sdk/paths.h`
- `include/yai_sdk/context.h`
- `include/yai_sdk/client.h`
- `include/yai_sdk/catalog.h`
- `include/yai_sdk/protocol.h`
- `include/yai_sdk/rpc.h`
- `include/yai_sdk/reply/reply.h`
- `include/yai_sdk/reply/reply_builder.h`
- `include/yai_sdk/reply/reply_json.h`

## Compatibility Headers (`compat`)

- `include/yai_sdk/yai_sdk.h`
- `include/yai_sdk/yai.h`
- `include/yai_sdk/registry/command_catalog.h`

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
- `catalog` -> `public_stable`
- `context/workspace` -> `public_stable`
- `reply` -> `public_stable`
- `protocol/rpc` -> `public_stable`
- `registry raw loaders/validators` -> `internal`
- `compat wrappers` -> `compat`

## CLI Coupling Cleanup

Resolved:
- `yai-cli/src/parse/parse.c` no longer includes `yai_sdk/registry/command_catalog.h`
- `yai-cli/src/help/help.c` no longer includes `yai_sdk/registry/command_catalog.h`

Now both consume `yai_sdk/catalog.h`.

## Residual Notes

- `tests/sdk_smoke.c` uses explicit internal registry include for raw-registry smoke assertions. This is intentional test-only usage.
- No runtime behavior was changed by this inventory wave.
