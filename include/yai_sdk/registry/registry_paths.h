/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct yai_law_paths {
  char* law_dir;                 /* .../deps/yai-law */

  /* Registries */
  char* registry_primitives;     /* .../registry/primitives.v1.json */
  char* registry_commands;       /* .../registry/commands.v1.json */
  char* registry_artifacts;      /* .../registry/artifacts.v1.json */

  /* Schemas */
  char* schema_primitives;       /* .../registry/schema/primitives.v1.schema.json */
  char* schema_commands;         /* .../registry/schema/commands.v1.schema.json */
  char* schema_artifacts;        /* .../registry/schema/artifacts.v1.schema.json */

  /* Schema directory for artifact payload definitions. */
  char* artifacts_schema_dir;    /* .../registry/schema */
} yai_law_paths_t;

/*
 * Resolve the pinned yai-law directory and registry/schema paths.
 * Resolution order:
 *   1) env YAI_REGISTRY_DIR (preferred)
 *   2) repo_root_hint + "/deps/yai-law"
 *   3) upward search from current working directory
 */
int yai_law_paths_init(yai_law_paths_t* out, const char* repo_root_hint);

/* Free all allocated strings in `out`. */
void yai_law_paths_free(yai_law_paths_t* p);

/* Convenience getters (never NULL after successful init). */
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
