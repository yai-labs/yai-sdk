// SPDX-License-Identifier: Apache-2.0
// src/ops/executor.c

#include "yai_sdk/ops/executor.h"
#include "yai_sdk/ops/ops_dispatch.h"
#include "yai_sdk/errors.h"

#include <errno.h>

static void set_out(yai_exec_result_t *out, int code, const char *msg)
{
  if (!out)
    return;
  out->code = code;
  out->message = msg;
}

int yai_sdk_execute(const yai_exec_request_t *req, yai_exec_result_t *out)
{
  if (!req || !req->command_id || !req->command_id[0])
  {
    set_out(out, YAI_SDK_BAD_ARGS, "yai-sdk: invalid request (missing command_id)");
    return YAI_SDK_BAD_ARGS;
  }

  int argc = req->argc;
  char **argv = (char **)req->argv; // ABI: ops expects char** (read-only usage)

  int rc = yai_ops_dispatch_by_id(req->command_id, argc, argv);

  // Best-effort message classes (stable)
  if (rc == 0)
  {
    set_out(out, 0, NULL);
  }
  else if (rc == ENOTCONN)
  {
    set_out(out, rc, "yai-sdk: server unavailable");
  }
  else if (rc == YAI_SDK_RUNTIME_NOT_READY)
  {
    set_out(out, rc, "yai-sdk: runtime not ready");
  }
  else if (rc == YAI_SDK_BAD_ARGS)
  {
    set_out(out, rc, "yai-sdk: invalid arguments");
  }
  else
  {
    set_out(out, rc, "yai-sdk: failure");
  }

  return rc;
}