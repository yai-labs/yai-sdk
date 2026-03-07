# SDK Dependency Matrix

| area/file | current role | target role | action | public surface impact | tooling impact | follow-up needed |
|---|---|---|---|---|---|---|
| `src/registry/registry.c` | CLI-like `yai law` command entry | tooling-only helper | move-to-tooling | none | medium | isolate from runtime lib build if needed |
| `src/registry/registry_cache.c` | live law loading/cache | internal compatibility loader | keep-internal | none | high | support exported snapshot source first |
| `src/registry/registry_help.c` | runtime help from law data | tooling/helper only | move-to-tooling | none | medium | reduce runtime coupling |
| `src/registry/registry_load.c` | global law registry init | internal compatibility loader | keep-internal | none | medium | decouple from cwd crawling |
| `src/registry/registry_paths.c` | law path resolver | keep-internal (legacy compatibility) | deprecate | none | medium | replace with export-model resolver |
| `src/registry/registry_query.c` | command lookup index | keep-internal | keep-internal | none | low | none |
| `src/registry/registry_validate.c` | law registry validation | move-to-tooling | move-to-tooling | none | high | keep thin internal sanity only |
| `include/yai_sdk/registry/registry_cache.h` | exposed registry cache API | internal header | keep-internal | none | low | mark internal in headers |
| `include/yai_sdk/registry/registry.h` | `yai law` command API | tooling API | deprecate | none | medium | remove from install/public exports later |
| `include/yai_sdk/registry/registry_help.h` | help API | tooling/internal | move-to-tooling | none | medium | same as above |
| `include/yai_sdk/registry/registry_paths.h` | path resolver API | internal compatibility | keep-internal | none | medium | deprecate direct law path use |
| `include/yai_sdk/registry/registry_registry.h` | registry query/init | internal | keep-internal | none | medium | split runtime-safe subset |
| `include/yai_sdk/registry/registry_types.h` | law-shaped structs | internal compatibility types | keep-internal | none | medium | avoid widening dependency |
| `include/yai_sdk/registry/registry_validate.h` | registry validation API | tooling-only | move-to-tooling | none | high | no public guarantee |
| `include/yai_sdk/catalog.h` + `src/catalog/catalog.c` | normalized command catalog | public surface | keep-public | high | low | ensure source can come from exported artifacts |
| `tools/sh/resolve_law.sh` | hard law resolver | optional tooling helper | deprecate | none | medium | replace with compatibility artifact resolver |
