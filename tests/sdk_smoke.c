// SPDX-License-Identifier: Apache-2.0
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yai_sdk/public.h"
#include "yai_sdk/registry/registry_registry.h"

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

  yai_sdk_client_t *client = NULL;
  yai_sdk_client_opts_t opts = {
      .ws_id = "default",
      .uds_path = NULL,
      .arming = 1,
      .role = "operator",
      .auto_handshake = 1,
  };
  int rc = yai_sdk_client_open(&client, &opts);

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
    rc = yai_sdk_client_open(&client, &opts);
    if (rc != 0)
    {
      fprintf(stderr, "sdk_smoke: online ping failed rc=%d\n", rc);
      return 4;
    }

    const char *json = "{\"type\":\"yai.control.call.v1\",\"target_plane\":\"root\",\"command_id\":\"yai.root.ping\",\"argv\":[]}";
    yai_sdk_reply_t out = {0};
    rc = yai_sdk_client_call_json(client, json, &out);
    yai_sdk_client_close(client);
    if (rc != 0)
    {
      fprintf(stderr, "sdk_smoke: online call failed rc=%d code=%s reason=%s\n", rc, out.code, out.reason);
      yai_sdk_reply_free(&out);
      return 5;
    }
    if (!out.exec_reply_json || !out.exec_reply_json[0])
    {
      fprintf(stderr, "sdk_smoke: expected exec reply json\n");
      yai_sdk_reply_free(&out);
      return 6;
    }
    yai_sdk_reply_free(&out);
  }

  printf("sdk_smoke: ok (abi=%d version=%s registry=%s, server_off_rc=%d)\n",
         yai_sdk_abi_version(), yai_sdk_version(), c->id, YAI_SDK_SERVER_OFF);
  return 0;
}
