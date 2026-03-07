# SDK API Discipline

## Canonical entry

- `#include <yai_sdk/public.h>`

## API classes

### public_stable
- `yai_sdk/public.h`
- `yai_sdk/errors.h`
- `yai_sdk/paths.h`
- `yai_sdk/context.h`
- `yai_sdk/client.h`
- `yai_sdk/catalog.h`
- `yai_sdk/protocol.h`
- `yai_sdk/rpc.h`
- `yai_sdk/log.h`
- `yai_sdk/reply/*`

### internal (or advanced internal)
- `yai_sdk/registry/*`

## Rules

1. Do not make registry internals the public identity of SDK.
2. Public API evolution is semver-governed.
3. Internal headers may evolve without public-stable guarantees.
4. Compatibility with law surfaces is declared, not structurally pinned.
