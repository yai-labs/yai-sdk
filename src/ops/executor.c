// SPDX-License-Identifier: Apache-2.0
// src/ops/executor.c

#include "yai_sdk/ops/executor.h"
#include "yai_sdk/client.h"
#include "yai_sdk/errors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cJSON.h>

static void set_out(yai_exec_result_t *out, int code, const char *msg)
{
  if (!out)
    return;
  out->code = code;
  out->message = msg;
  out->status = NULL;
  out->code_name = NULL;
  out->reason = NULL;
  out->command_id = NULL;
  out->trace_id = NULL;
  out->target_plane = NULL;
}

static const char *infer_target_plane(const char *command_id)
{
  if (!command_id)
    return "kernel";
  if (strncmp(command_id, "yai.root.", 9) == 0)
    return "root";
  if (strncmp(command_id, "yai.engine.", 11) == 0)
    return "engine";
  return "kernel";
}

static char *build_control_call_json(const yai_exec_request_t *req)
{
  cJSON *root = cJSON_CreateObject();
  if (!root)
    return NULL;
  cJSON_AddStringToObject(root, "type", "yai.control.call.v1");
  cJSON_AddStringToObject(root, "target_plane", infer_target_plane(req->command_id));
  cJSON_AddStringToObject(root, "command_id", req->command_id);
  cJSON *argv = cJSON_AddArrayToObject(root, "argv");
  for (int i = 0; i < req->argc; i++)
  {
    const char *arg = (req->argv && req->argv[i]) ? req->argv[i] : "";
    cJSON_AddItemToArray(argv, cJSON_CreateString(arg));
  }
  char *json = cJSON_PrintUnformatted(root);
  cJSON_Delete(root);
  return json;
}

int yai_sdk_execute(const yai_exec_request_t *req, yai_exec_result_t *out)
{
  static char msg_buf[384];
  if (!req || !req->command_id || !req->command_id[0])
  {
    set_out(out, YAI_SDK_BAD_ARGS, "yai-sdk: invalid request (missing command_id)");
    return YAI_SDK_BAD_ARGS;
  }

  yai_sdk_client_t *client = NULL;
  yai_sdk_client_opts_t opts = {
      .ws_id = "default",
      .uds_path = NULL,
      .arming = 1,
      .role = "operator",
      .auto_handshake = 1,
  };
  int rc = yai_sdk_client_open(&client, &opts);
  if (rc != YAI_SDK_OK)
  {
    set_out(out, rc, "yai-sdk: error:SERVER_UNAVAILABLE:server_unavailable");
    return rc;
  }

  char *control_call_json = build_control_call_json(req);
  if (!control_call_json)
  {
    yai_sdk_client_close(client);
    set_out(out, YAI_SDK_IO, "yai-sdk: error:INTERNAL_ERROR:request_encode_failed");
    return YAI_SDK_IO;
  }

  yai_sdk_reply_t reply = {0};
  rc = yai_sdk_client_call_json(client, control_call_json, &reply);
  yai_sdk_client_close(client);
  free(control_call_json);

  const char *status = reply.status[0] ? reply.status : "error";
  const char *code = reply.code[0] ? reply.code : "INTERNAL_ERROR";
  const char *reason = reply.reason[0] ? reply.reason : "unknown";
  const char *reply_command_id = reply.command_id[0] ? reply.command_id : req->command_id;
  const char *trace_id = reply.trace_id;
  const char *target_plane = reply.target_plane[0] ? reply.target_plane : "kernel";

  int n = snprintf(msg_buf,
                   sizeof(msg_buf),
                   "yai-sdk: %s:%s:%s (command_id=%s)",
                   status,
                   code,
                   reason,
                   reply_command_id);
  if (n <= 0 || (size_t)n >= sizeof(msg_buf))
    msg_buf[0] = '\0';

  set_out(out, rc, msg_buf[0] ? msg_buf : "yai-sdk: error");
  if (out)
  {
    out->status = status;
    out->code_name = code;
    out->reason = reason;
    out->command_id = reply_command_id;
    out->trace_id = trace_id;
    out->target_plane = target_plane;
  }
  yai_sdk_reply_free(&reply);

  return rc;
}
