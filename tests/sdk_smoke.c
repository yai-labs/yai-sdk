// SPDX-License-Identifier: Apache-2.0
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yai_sdk/public.h"
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
  (void)setenv("YAI_REGISTRY_DIR", "../yai-law", 1);

  if (yai_sdk_abi_version() != YAI_SDK_ABI_VERSION)
  {
    fprintf(stderr, "sdk_smoke: ABI mismatch runtime=%d compile=%d\n",
            yai_sdk_abi_version(), YAI_SDK_ABI_VERSION);
    return 1;
  }
  if (strcmp(yai_sdk_errstr(YAI_SDK_SERVER_OFF), "server_off") != 0)
  {
    fprintf(stderr, "sdk_smoke: errstr mapping failed for YAI_SDK_SERVER_OFF\n");
    return 1;
  }

  /* 1) Offline registry lookup must work */
  const yai_law_command_t *c = yai_law_cmd_by_id("yai.kernel.ws");
  if (!c)
  {
    fprintf(stderr, "sdk_smoke: registry lookup failed\n");
    return 2;
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

  if (rc != YAI_SDK_SERVER_OFF)
  {
    fprintf(stderr, "sdk_smoke: expected YAI_SDK_SERVER_OFF=%d, got rc=%d\n", YAI_SDK_SERVER_OFF, rc);
    return 3;
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
      return 4;
    }
    if (out.message && is_error_payload(out.message))
    {
      fprintf(stderr, "sdk_smoke: online ping returned error payload: %s\n", out.message);
      return 5;
    }
  }

  printf("sdk_smoke: ok (abi=%d version=%s registry=%s, server_off_rc=%d)\n",
         yai_sdk_abi_version(), yai_sdk_version(), c->id, YAI_SDK_SERVER_OFF);
  return 0;
}
