# SDK API Discipline v1

Status: Active
Owner: yai-sdk

## Scope
This document defines API boundary rules for `yai-sdk`.
It classifies headers and modules as `public_stable`, `public_experimental`, or `internal`.

## Canonical Entry
Canonical public entrypoint:
- `#include <yai_sdk/public.h>`

## API Classes

### public_stable
Supported for external consumers and semver-protected.

Headers:
- `yai_sdk/public.h`
- `yai_sdk/errors.h`
- `yai_sdk/paths.h`
- `yai_sdk/context.h`
- `yai_sdk/client.h`
- `yai_sdk/catalog.h`
- `yai_sdk/protocol.h`
- `yai_sdk/rpc.h`
- `yai_sdk/log.h`
- `yai_sdk/reply/reply.h`
- `yai_sdk/reply/reply_builder.h`
- `yai_sdk/reply/reply_json.h`

### public_experimental
Publicly visible but not yet stable for strong compatibility guarantees.

Headers:
- none in v1 (reserved class)

### internal
Not part of the public default surface; used for SDK implementation internals.
Advanced consumers may include them explicitly at their own risk.

Headers:
- `yai_sdk/registry/registry.h`
- `yai_sdk/registry/registry_help.h`
- `yai_sdk/registry/registry_paths.h`
- `yai_sdk/registry/registry_cache.h`
- `yai_sdk/registry/registry_registry.h`
- `yai_sdk/registry/registry_types.h`
- `yai_sdk/registry/registry_validate.h`

## Boundary Rules

1. CLI consumers must prefer `public.h` and stable module headers.
2. CLI consumers must not include `yai_sdk/registry/*` headers directly.
3. `public.h` must not include internal headers.
4. Internal headers can change without semver guarantees unless promoted.

## Promotion and Deprecation

- Internal/experimental -> public_stable requires:
  - stable API naming
  - compatibility note in `COMPATIBILITY.md`
  - semver impact review in `VERSIONING.md`
  - tests proving external usage

- public_stable deprecation requires:
  - docs-first warning
  - compatibility wrapper or migration path
  - delayed removal aligned with semver major policy

## Integration Aids

- Examples: `examples/`
- Wrapper skeletons: `wrappers/`

## Consumer Guidance

- Use `public.h` for normal integrations.
- Include module headers directly only when needed for advanced scenarios.
- Do not rely on registry internals for CLI behavior; use catalog/help APIs.
