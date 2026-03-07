# Runtime Resolution Policy v1

## Purpose
`yai-sdk` is the canonical resolver for runtime/install/context paths used by CLI and other surfaces.

## Core Concepts
- `runtime_home`: base directory for runtime sockets, pids, and logs.
- `install_root`: repository or installation root used to locate runtime binaries.
- `deploy_mode`: detected execution context (`repo_dev`, `local_install`, `packaged`, `qualification`, `unknown`).

## Override Precedence
1. Explicit function-level override (when present in caller API).
2. Environment override (`YAI_*` variables).
3. Canonical resolver defaults.
4. Deterministic failure.

## Supported Environment Overrides
- `YAI_RUNTIME_HOME`
- `YAI_INSTALL_ROOT`
- `YAI_ROOT_SOCK`
- `YAI_BOOT_BIN`
- `YAI_ROOT_BIN`
- `YAI_KERNEL_BIN`
- `YAI_ENGINE_BIN`

## Binary Discovery Policy
Resolver order for runtime binaries:
1. explicit env binary path
2. `<install_root>/dist/bin/<binary>`
3. `<install_root>/build/bin/<binary>`
4. `<install_root>/bin/<binary>`
5. `$PATH` lookup

No machine-specific absolute path is permitted in callers.

## Consumer Rule
`yai-cli`, `yai`, and tooling must consume SDK resolver APIs instead of implementing local fallback lists.
