# Runtime Resolution Policy

## Purpose

Defines canonical runtime/install/context path resolution for SDK consumers.

## Dependency distinction

Path resolution does not imply structural dependency on `yai-law` repository layout.
If law-derived artifacts are needed, they must be consumed via explicit compatibility/export paths in tooling contexts.

## Preferred model

1. explicit caller override
2. environment override
3. resolver defaults
4. deterministic failure

## Legacy compatibility knobs

Legacy env knobs for direct law path resolution are compatibility-only and should be phased down.
