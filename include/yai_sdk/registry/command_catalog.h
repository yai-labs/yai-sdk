/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct yai_sdk_command_ref {
  char group[32];
  char name[64];
  char id[128];
  char summary[160];
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

#ifdef __cplusplus
}
#endif
