// SPDX-License-Identifier: Apache-2.0
// src/ops/executor.c

#include "yai_sdk/ops/executor.h"
#include "yai_sdk/ops/ops_dispatch.h"
#include "yai_sdk/errors.h"

#include <errno.h>
#include <stdio.h>

static void set_out(yai_exec_result_t *out, int code, const char *msg)
{
  if (!out)
    return;
  out->code = code;
  out->message = msg;
}

int yai_sdk_execute(const yai_exec_request_t *req, yai_exec_result_t *out)
{
  static char msg_buf[384];
  if (!req || !req->command_id || !req->command_id[0])
  {
    set_out(out, YAI_SDK_BAD_ARGS, "yai-sdk: invalid request (missing command_id)");
    return YAI_SDK_BAD_ARGS;
  }

  int argc = req->argc;
  char **argv = (char **)req->argv; // ABI: ops expects char** (read-only usage)

  int rc = yai_ops_dispatch_by_id(req->command_id, argc, argv);
  const char *status = NULL;
  const char *code = NULL;
  const char *reason = NULL;
  const char *reply_command_id = NULL;
  yai_ops_last_reply(&status, &code, &reason, &reply_command_id);

  if (!status) status = "error";
  if (!code) code = "INTERNAL_ERROR";
  if (!reason) reason = "unknown";
  if (!reply_command_id) reply_command_id = req->command_id;

  int n = snprintf(msg_buf,
                   sizeof(msg_buf),
                   "yai-sdk: %s:%s:%s (command_id=%s)",
                   status,
                   code,
                   reason,
                   reply_command_id);
  if (n <= 0 || (size_t)n >= sizeof(msg_buf))
    msg_buf[0] = '\0';

  // Deterministic mapping + normalized message from status/code/reason.
  if (rc == ENOTCONN)
    rc = YAI_SDK_SERVER_OFF;
  set_out(out, rc, msg_buf[0] ? msg_buf : "yai-sdk: error");

  return rc;
}
