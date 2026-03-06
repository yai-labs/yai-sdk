/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum yai_sdk_catalog_surface_mask {
  YAI_SDK_CATALOG_SURFACE_USER = 1 << 0,
  YAI_SDK_CATALOG_SURFACE_TOOL = 1 << 1,
  YAI_SDK_CATALOG_SURFACE_INTERNAL = 1 << 2,
  YAI_SDK_CATALOG_SURFACE_ALL =
      YAI_SDK_CATALOG_SURFACE_USER |
      YAI_SDK_CATALOG_SURFACE_TOOL |
      YAI_SDK_CATALOG_SURFACE_INTERNAL,
} yai_sdk_catalog_surface_mask_t;

typedef struct yai_sdk_command_ref {
  char group[32];
  char name[64];
  char id[128];
  char summary[160];

  char surface[16];
  char entrypoint[32];
  char topic[64];
  char op[64];
  char layer[16];
  char stability[16];
} yai_sdk_command_ref_t;

typedef struct yai_sdk_command_group {
  char group[32];
  yai_sdk_command_ref_t *commands;
  size_t command_count;
} yai_sdk_command_group_t;

typedef struct yai_sdk_command_catalog {
  yai_sdk_command_group_t *groups;
  size_t group_count;
} yai_sdk_command_catalog_t;

typedef yai_sdk_command_catalog_t yai_catalog_t;
typedef yai_sdk_command_group_t yai_catalog_group_t;
typedef yai_sdk_command_ref_t yai_catalog_command_t;

int yai_sdk_command_catalog_load(yai_sdk_command_catalog_t *out);
void yai_sdk_command_catalog_free(yai_sdk_command_catalog_t *cat);

const yai_sdk_command_group_t *yai_sdk_command_catalog_find_group(
    const yai_sdk_command_catalog_t *cat,
    const char *group);

const yai_sdk_command_ref_t *yai_sdk_command_catalog_find_command(
    const yai_sdk_command_catalog_t *cat,
    const char *group,
    const char *name);

const yai_sdk_command_ref_t *yai_sdk_command_catalog_find_by_id(
    const yai_sdk_command_catalog_t *cat,
    const char *canonical_id);

const yai_sdk_command_ref_t *yai_sdk_command_catalog_find_by_path(
    const yai_sdk_command_catalog_t *cat,
    const char *entrypoint,
    const char *topic,
    const char *op,
    int surface_mask);

size_t yai_sdk_command_catalog_collect_entrypoints(
    const yai_sdk_command_catalog_t *cat,
    int surface_mask,
    const char **out_entrypoints,
    size_t out_cap);

/* Canonical short aliases (stable surface for CLI/UI layers). */
size_t yai_catalog_list_groups(
    const yai_catalog_t *cat,
    const yai_catalog_group_t **out_groups);

size_t yai_catalog_list_commands(
    const yai_catalog_t *cat,
    const char *group,
    const yai_catalog_command_t **out_commands);

const yai_catalog_command_t *yai_catalog_find_by_id(
    const yai_catalog_t *cat,
    const char *canonical_id);

#ifdef __cplusplus
}
#endif
