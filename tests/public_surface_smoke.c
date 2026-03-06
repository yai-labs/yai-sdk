// SPDX-License-Identifier: Apache-2.0
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "yai_sdk/public.h"

int main(void)
{
  const char *law_root = getenv("YAI_LAW_ROOT");
  yai_sdk_command_catalog_t catalog = {0};
  yai_sdk_catalog_filter_t filter = {
      .surface_mask = YAI_SDK_CATALOG_SURFACE_SURFACE,
      .stability_mask = YAI_SDK_CATALOG_STABILITY_STABLE,
  };
  const yai_sdk_command_ref_t *matches[1] = {0};

  if (yai_sdk_abi_version() != YAI_SDK_ABI_VERSION) {
    fprintf(stderr, "public_surface_smoke: abi mismatch\n");
    return 1;
  }
  if (!yai_sdk_version() || !yai_sdk_version()[0]) {
    fprintf(stderr, "public_surface_smoke: empty version\n");
    return 1;
  }

  if (law_root && law_root[0] != '\0') {
    (void)setenv("YAI_REGISTRY_DIR", law_root, 1);
  } else {
    (void)setenv("YAI_REGISTRY_DIR", "../yai-law", 1);
  }

  if (yai_sdk_command_catalog_load(&catalog) != 0) {
    fprintf(stderr, "public_surface_smoke: catalog load failed\n");
    return 2;
  }

  if (yai_sdk_command_catalog_query(&catalog, &filter, matches, 1) == 0) {
    fprintf(stderr, "public_surface_smoke: expected at least one stable surface command\n");
    yai_sdk_command_catalog_free(&catalog);
    return 2;
  }

  yai_sdk_command_catalog_free(&catalog);

  if (strcmp(yai_sdk_errstr(YAI_SDK_SERVER_OFF), "server_off") != 0) {
    fprintf(stderr, "public_surface_smoke: errstr mismatch\n");
    return 3;
  }

  printf("public_surface_smoke: ok\n");
  return 0;
}
