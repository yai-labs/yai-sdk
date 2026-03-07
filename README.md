# yai-sdk — Contract-Governed C SDK for YAI

`yai-sdk` is the canonical C SDK for interacting with YAI runtime surfaces under pinned `yai-law` contracts.

Dependency chain:

`yai-law` -> `yai-sdk` -> `yai-cli` -> `yai` -> `yai-ops`

## What this repo is

- A stable programmatic client surface (`public.h` + module headers)
- Catalog/help/reply/workspace access layer for consumers
- RPC transport and protocol integration constrained by pinned law

## What this repo is not

- Not a normative source for command/reply/workspace semantics (owned by `yai-law`)
- Not a CLI surface implementation (owned by `yai-cli`)

## Public API entrypoint

Use:

```c
#include <yai_sdk/public.h>
```

API governance:

- `docs/SDK_API_DISCIPLINE.md`
- `docs/SDK_SURFACE_CONTRACT.md`
- `COMPATIBILITY.md`
- `VERSIONING.md`

## Developer Experience and Reliability

- Integration examples: `examples/`
- Python wrapper skeleton: `wrappers/python/`
- Sanitizer checks: `make check`
- Coverage: `make coverage`
- Doxygen reference: `make docs`
- Install artifacts: `make install`

## Build

```bash
make -j4
make test
```

## License

Apache-2.0 (see `LICENSE`).
