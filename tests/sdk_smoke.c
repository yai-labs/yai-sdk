// SPDX-License-Identifier: Apache-2.0
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yai_sdk/ops/executor.h"
#include "yai_sdk/registry/registry_registry.h"

static int is_error_payload(const char *s)
{
  if (!s)
    return 0;
  return (strstr(s, "\"status\":\"error\"") != NULL) ||
         (strstr(s, "\"status\": \"error\"") != NULL);
}

int main(void)
{
  /* 1) Offline registry lookup must work */
  const yai_law_command_t *c = yai_law_cmd_by_id("yai.kernel.ws");
  if (!c)
  {
    fprintf(stderr, "sdk_smoke: registry lookup failed\n");
    return 1;
  }

  /* 2) Deterministic server-off semantics (always) */
  (void)setenv("YAI_ROOT_SOCK", "/tmp/yai-root-sock-does-not-exist.sock", 1);

  yai_exec_request_t req = {
      .command_id = "yai.root.ping",
      .argc = 0,
      .argv = NULL,
      .json_mode = 1,
  };

  yai_exec_result_t out = {0};
  int rc = yai_sdk_execute(&req, &out);

  if (rc != ENOTCONN)
  {
    fprintf(stderr, "sdk_smoke: expected ENOTCONN=%d, got rc=%d\n", ENOTCONN, rc);
    return 2;
  }

  /* 3) Optional online probe (does NOT fail CI by default) */
  const char *online = getenv("YAI_SDK_SMOKE_ONLINE");
  if (online && strcmp(online, "1") == 0)
  {
    (void)unsetenv("YAI_ROOT_SOCK"); /* use default path */
    memset(&out, 0, sizeof(out));

    rc = yai_sdk_execute(&req, &out);
    if (rc != 0)
    {
      fprintf(stderr, "sdk_smoke: online ping failed rc=%d\n", rc);
      if (out.message)
        fprintf(stderr, "sdk_smoke: msg=%s\n", out.message);
      return 3;
    }
    if (out.message && is_error_payload(out.message))
    {
      fprintf(stderr, "sdk_smoke: online ping returned error payload: %s\n", out.message);
      return 4;
    }
  }

  printf("sdk_smoke: ok (registry=%s, server_off_rc=%d)\n", c->id, ENOTCONN);
  return 0;
}