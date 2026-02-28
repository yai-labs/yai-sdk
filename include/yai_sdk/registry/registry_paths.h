// include/yai_sdk/registry/registry_paths.h
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct yai_law_paths {
  char* law_dir;                 // .../deps/yai-law

  // registries (new yai-law layout)
  char* registry_primitives;     // .../registry/primitives.v1.json
  char* registry_commands;       // .../registry/commands.v1.json
  char* registry_artifacts;      // .../registry/artifacts.v1.json

  // schemas (new yai-law layout)
  char* schema_primitives;       // .../registry/schema/primitives.v1.schema.json
  char* schema_commands;         // .../registry/schema/commands.v1.schema.json
  char* schema_artifacts;        // .../registry/schema/artifacts.v1.schema.json

  // schema dir for artifacts payloads (optional; keep for future)
  // NOTE: if you don’t have a dedicated dir in yai-law yet, you can point to registry/schema
  char* artifacts_schema_dir;    // .../registry/schema
} yai_law_paths_t;

// Initialize by resolving deps/yai-law.
// Resolution order:
//  1) env YAI_REGISTRY_DIR (preferred); fallback: YAI_REGISTRY_DIR (legacy) (must exist)
//  2) repo_root_hint + "/deps/yai-law" (if provided, must exist)
//  3) search upwards from cwd for "/deps/yai-law"
int yai_law_paths_init(yai_law_paths_t* out, const char* repo_root_hint);

// Frees all allocated strings in out.
void yai_law_paths_free(yai_law_paths_t* p);

// Convenience getters (never NULL after successful init)
const char* yai_law_dir(const yai_law_paths_t* p);
const char* yai_law_registry_primitives(const yai_law_paths_t* p);
const char* yai_law_registry_commands(const yai_law_paths_t* p);
const char* yai_law_registry_artifacts(const yai_law_paths_t* p);

const char* yai_law_schema_primitives(const yai_law_paths_t* p);
const char* yai_law_schema_commands(const yai_law_paths_t* p);
const char* yai_law_schema_artifacts(const yai_law_paths_t* p);

const char* yai_law_artifacts_schema_dir(const yai_law_paths_t* p);

#ifdef __cplusplus
}
#endif