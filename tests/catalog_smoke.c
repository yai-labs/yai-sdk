/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <stdlib.h>

#include "yai_sdk/public.h"

int main(void)
{
  yai_sdk_command_catalog_t cat = {0};
  const yai_sdk_command_group_t *g = NULL;
  const yai_sdk_command_ref_t *c = NULL;
  int rc;

  (void)setenv("YAI_REGISTRY_DIR", "../yai-law", 1);

  rc = yai_sdk_command_catalog_load(&cat);
  if (rc != 0) {
    fprintf(stderr, "catalog_smoke: load failed rc=%d\n", rc);
    return 1;
  }
  if (cat.group_count == 0) {
    fprintf(stderr, "catalog_smoke: empty catalog\n");
    yai_sdk_command_catalog_free(&cat);
    return 2;
  }

  g = yai_sdk_command_catalog_find_group(&cat, "root");
  if (!g) {
    fprintf(stderr, "catalog_smoke: expected root group\n");
    yai_sdk_command_catalog_free(&cat);
    return 3;
  }

  c = yai_sdk_command_catalog_find_by_id(&cat, "yai.root.ping");
  if (!c) {
    fprintf(stderr, "catalog_smoke: expected command yai.root.ping\n");
    yai_sdk_command_catalog_free(&cat);
    return 4;
  }

  c = yai_sdk_command_catalog_find_command(&cat, "root", "ping");
  if (!c) {
    fprintf(stderr, "catalog_smoke: expected command root/ping\n");
    yai_sdk_command_catalog_free(&cat);
    return 5;
  }

  yai_sdk_command_catalog_free(&cat);
  puts("catalog_smoke: ok");
  return 0;
}
