# SDK Compatibility / Export Model

## Goal

Define how `yai-sdk` consumes law-aligned information without structural dependency on live `yai-law`.

## Allowed inputs

- exported compatibility snapshots
- generated artifacts produced by controlled tooling
- compatibility manifests with explicit version/range declaration

## Forbidden model

- treating live `yai-law` tree as mandatory runtime dependency
- structural pinning from `yai-sdk` to `yai-law`

## Distinctions

- law source of truth: `yai-law`
- exported compatibility baseline: generated snapshot/manifests used by SDK tooling
- SDK public surface: stable C APIs consumed by downstream clients

## Implementation posture

- runtime-critical SDK code must not require live repo traversal
- optional tooling may resolve external law artifacts for verification/generation
- compatibility results must be explicit and reproducible
