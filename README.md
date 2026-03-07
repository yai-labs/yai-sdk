# YAI SDK (C)

YAI SDK is the canonical C interface for building on the YAI platform.

It carries platform contracts into code: stable interfaces, governed integration surfaces, and aligned access to YAI system behavior under pinned law.

This repository exists to make YAI programmable without weakening its rules.

## Platform position

YAI SDK operates inside a disciplined platform chain:

`yai-law` → `yai-sdk` → `yai-cli` → `yai` → `yai-ops`

Law defines the rules.  
The SDK carries the interface.  
Everything built on top of it inherits that discipline.

## What this repository is

This repository contains the canonical SDK surface for YAI, including:

- stable public headers for system integration
- governed client interfaces for platform access
- protocol and transport integration aligned to pinned law
- examples, wrappers, and developer-facing assets for adoption and extension

## What this repository is not

This repository does not own:

- normative command, reply, workspace, or protocol semantics (`yai-law`)
- the canonical command implementation (`yai-cli`)
- the governed systems implementation itself (`yai`)

## Public API

Primary entrypoint:

```c
#include <yai_sdk/public.h>
```

## API discipline

- `docs/SDK_API_DISCIPLINE.md`
- `docs/SDK_SURFACE_CONTRACT.md`
- `COMPATIBILITY.md`
- `VERSIONING.md`

## Developer surfaces

- integration examples: `examples/`
- Python wrapper skeleton: `wrappers/python/`
- sanitizer checks: `make check`
- coverage: `make coverage`
- API reference: `make docs`
- install artifacts: `make install`

## Build

```bash
make -j4
make test
```

## License

Apache-2.0. See `LICENSE`.