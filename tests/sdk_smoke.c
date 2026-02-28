#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "yai_sdk/registry/registry_registry.h"
#include "yai_sdk/ops/executor.h"

int main(void)
{
  // 1) Registry must resolve a canonical id (offline)
  const yai_law_command_t *c = yai_law_cmd_by_id("yai.kernel.ws");
  if (!c)
  {
    fprintf(stderr, "sdk_smoke: registry lookup failed\n");
    return 1;
  }

  // 2) Deterministic server-off semantics:
  // force root sock to a path that cannot exist -> connect must fail with ENOTCONN
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

  printf("sdk_smoke: ok (registry=%s, server_off_rc=%d)\n", c->id, rc);
  return 0;
}