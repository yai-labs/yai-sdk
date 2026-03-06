// SPDX-License-Identifier: Apache-2.0
#include <stdio.h>
#include <string.h>

#include "yai_sdk/public.h"

int main(void)
{
  char resolved[64] = {0};
  yai_sdk_workspace_info_t info = {0};
  int rc;

  rc = yai_sdk_context_clear_current_workspace();
  if (rc != 0) {
    fprintf(stderr, "workspace_smoke: clear failed rc=%d\n", rc);
    return 1;
  }

  rc = yai_sdk_context_resolve_workspace(NULL, resolved, sizeof(resolved));
  if (rc != 1) {
    fprintf(stderr, "workspace_smoke: expected unresolved workspace rc=1, got %d\n", rc);
    return 2;
  }

  rc = yai_sdk_context_set_current_workspace("ws_smoke_v2");
  if (rc != 0) {
    fprintf(stderr, "workspace_smoke: set failed rc=%d\n", rc);
    return 3;
  }

  rc = yai_sdk_context_resolve_workspace(NULL, resolved, sizeof(resolved));
  if (rc != 0 || strcmp(resolved, "ws_smoke_v2") != 0) {
    fprintf(stderr, "workspace_smoke: resolve mismatch rc=%d ws=%s\n", rc, resolved);
    return 4;
  }

  rc = yai_sdk_context_resolve_workspace("ws_explicit", resolved, sizeof(resolved));
  if (rc != 0 || strcmp(resolved, "ws_explicit") != 0) {
    fprintf(stderr, "workspace_smoke: explicit override mismatch rc=%d ws=%s\n", rc, resolved);
    return 5;
  }

  rc = yai_sdk_context_validate_current_workspace(&info);
  if (rc == 0 && info.ws_id[0] != '\0') {
    /* runtime might be up in local dev; this branch is allowed */
  } else if (rc != YAI_SDK_SERVER_OFF && rc != YAI_SDK_RUNTIME_NOT_READY && rc != YAI_SDK_PROTOCOL) {
    fprintf(stderr, "workspace_smoke: unexpected validation rc=%d\n", rc);
    return 6;
  }

  rc = yai_sdk_context_clear_current_workspace();
  if (rc != 0) {
    fprintf(stderr, "workspace_smoke: clear(post) failed rc=%d\n", rc);
    return 7;
  }

  puts("workspace_smoke: ok");
  return 0;
}
