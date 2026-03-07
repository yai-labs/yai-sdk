# YAI SDK Surface Contract

## Scope

Defines stable public API commitments for `yai-sdk`.

## Public surface center

Primary public surface modules:
- `yai_sdk/public.h`
- `client`, `context`, `paths`, `protocol`, `reply`, `rpc`, `catalog`

## Registry posture

`include/yai_sdk/registry/*` is not default public-stable surface.
It is internal/advanced compatibility support and may change more aggressively.

## Dependency posture

This SDK surface must remain usable without structural pinning to `yai-law`.
Compatibility is declared via version/compat docs and optional exported artifact workflows.
