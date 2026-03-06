// SPDX-License-Identifier: Apache-2.0
#include <stdio.h>
#include <string.h>

#include "yai_sdk/yai_sdk.h"

int main(void)
{
  char ws[64] = {0};
  yai_sdk_workspace_info_t info = {0};
  int rc;

  rc = yai_sdk_context_clear_current_workspace();
  if (rc != 0)
  {
    fprintf(stderr, "workspace_smoke: clear failed rc=%d\n", rc);
    return 1;
  }

  rc = yai_sdk_context_resolve_workspace(NULL, ws, sizeof(ws));
  if (rc != 1)
  {
    fprintf(stderr, "workspace_smoke: expected unresolved context rc=1 got %d\n", rc);
    return 2;
  }

  rc = yai_sdk_context_set_current_workspace("ws_smoke_v2");
  if (rc != 0)
  {
    fprintf(stderr, "workspace_smoke: set failed rc=%d\n", rc);
    return 3;
  }

  rc = yai_sdk_context_resolve_workspace(NULL, ws, sizeof(ws));
  if (rc != 0 || strcmp(ws, "ws_smoke_v2") != 0)
  {
    fprintf(stderr, "workspace_smoke: resolve mismatch rc=%d ws=%s\n", rc, ws);
    return 4;
  }

  rc = yai_sdk_context_resolve_workspace("ws_override", ws, sizeof(ws));
  if (rc != 0 || strcmp(ws, "ws_override") != 0)
  {
    fprintf(stderr, "workspace_smoke: explicit override mismatch rc=%d ws=%s\n", rc, ws);
    return 5;
  }

  rc = yai_sdk_context_validate_current_workspace(&info);
  if (rc == 0)
  {
    if (!info.ws_id[0])
    {
      fprintf(stderr, "workspace_smoke: expected ws_id when context is valid\n");
      return 6;
    }
  }
  else if (rc != YAI_SDK_SERVER_OFF && rc != YAI_SDK_RUNTIME_NOT_READY && rc != YAI_SDK_PROTOCOL)
  {
    fprintf(stderr, "workspace_smoke: unexpected validation rc=%d\n", rc);
    return 7;
  }

  rc = yai_sdk_context_clear_current_workspace();
  if (rc != 0)
  {
    fprintf(stderr, "workspace_smoke: clear(post) failed rc=%d\n", rc);
    return 8;
  }

  puts("workspace_smoke: ok");
  return 0;
}
