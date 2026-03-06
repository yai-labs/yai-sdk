/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <stdlib.h>

#include "yai_sdk/yai_sdk.h"

int main(void)
{
  yai_sdk_command_catalog_t cat = {0};
  yai_sdk_help_index_t idx = {0};
  yai_sdk_catalog_filter_t filter = {0};
  const yai_sdk_help_entrypoint_t *run = NULL;
  const yai_sdk_help_topic_t *root = NULL;
  const yai_sdk_command_ref_t *ping = NULL;
  int rc;

  (void)setenv("YAI_REGISTRY_DIR", "../yai-law", 1);

  rc = yai_sdk_command_catalog_load(&cat);
  if (rc != 0) {
    fprintf(stderr, "help_index_smoke: catalog load failed rc=%d\n", rc);
    return 1;
  }

  filter.surface_mask = YAI_SDK_CATALOG_SURFACE_SURFACE;
  filter.stability_mask = YAI_SDK_CATALOG_STABILITY_STABLE | YAI_SDK_CATALOG_STABILITY_EXPERIMENTAL;
  filter.include_hidden = 0;
  filter.include_deprecated = 0;

  rc = yai_sdk_help_index_build(&cat, &filter, &idx);
  if (rc != 0) {
    fprintf(stderr, "help_index_smoke: build failed rc=%d\n", rc);
    yai_sdk_command_catalog_free(&cat);
    return 2;
  }
  if (idx.entrypoint_count == 0) {
    fprintf(stderr, "help_index_smoke: empty index\n");
    yai_sdk_help_index_free(&idx);
    yai_sdk_command_catalog_free(&cat);
    return 3;
  }

  run = yai_sdk_help_find_entrypoint(&idx, "run");
  if (!run) {
    fprintf(stderr, "help_index_smoke: expected run entrypoint\n");
    yai_sdk_help_index_free(&idx);
    yai_sdk_command_catalog_free(&cat);
    return 4;
  }

  root = yai_sdk_help_find_topic(&idx, "run", "root");
  if (!root) {
    fprintf(stderr, "help_index_smoke: expected run/root topic\n");
    yai_sdk_help_index_free(&idx);
    yai_sdk_command_catalog_free(&cat);
    return 5;
  }

  ping = yai_sdk_help_find_command(&idx, "run", "root", "ping");
  if (!ping) {
    fprintf(stderr, "help_index_smoke: expected run/root/ping\n");
    yai_sdk_help_index_free(&idx);
    yai_sdk_command_catalog_free(&cat);
    return 6;
  }

  yai_sdk_help_index_free(&idx);
  yai_sdk_command_catalog_free(&cat);
  puts("help_index_smoke: ok");
  return 0;
}
