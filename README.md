# YAI SDK (C)

YAI SDK is the canonical C programmatic surface for YAI consumers.

It exposes stable client/context/protocol/reply/rpc APIs for applications and tooling.
It is a compatibility surface consumer, not a normative source.

## Platform role

- `yai-law`: normative authority source
- `yai`: integration/runtime authority
- `yai-sdk`: programmatic compatibility surface
- `yai-cli`: operator compatibility surface

## Dependency posture

- `yai-sdk` does not structurally pin `yai-law`.
- `yai-sdk` may consume exported/generated compatibility artifacts.
- `yai-sdk` must declare compatibility, not enforce live cross-repo dependency.

## Public API entrypoint

```c
#include <yai_sdk/public.h>
```

## Primary public modules

- `client`
- `context`
- `paths`
- `protocol`
- `reply`
- `rpc`
- `catalog` (public query surface)

`registry/*` is not the repo identity axis; it is internal/tooling-oriented support.

## Reference docs

- `docs/SDK_SURFACE_CONTRACT.md`
- `docs/SDK_API_DISCIPLINE.md`
- `docs/SDK_COMPATIBILITY_MODEL.md`
- `docs/SDK_DEPENDENCY_MATRIX.md`
- `COMPATIBILITY.md`
- `VERSIONING.md`
