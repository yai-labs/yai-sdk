// SPDX-License-Identifier: Apache-2.0
// src/ops/ops_dispatch.c

#include "yai_sdk/ops/ops_dispatch.h"
#include "yai_sdk/ops/ops_dispatch_gen.h"
#include "yai_sdk/errors.h"
#include "yai_sdk/rpc/rpc.h"

#include <yai_protocol_ids.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // close()

typedef int (*yai_ops_fn_t)(int argc, char **argv);

typedef struct
{
  const char *id;
  yai_ops_fn_t fn;
} yai_ops_entry_t;

// ------------------- helpers -------------------

static void rpc_client_close(yai_rpc_client_t *c)
{
  if (!c)
    return;

  // Prefer a dedicated close if it exists (uncomment if your rpc.h provides it)
  // yai_rpc_close(c);

  // Fallback: close fd if present.
  // This assumes yai_rpc_client_t has an 'fd' field (it usually does).
  if (c->fd >= 0)
  {
    close(c->fd);
    c->fd = -1;
  }
}

// Minimal ping runner: connect -> handshake -> ping -> print payload
static int rpc_ping(const char *ws_id)
{
  yai_rpc_client_t c;
  memset(&c, 0, sizeof(c));
  c.fd = -1;

  if (yai_rpc_connect(&c, ws_id) != 0)
  {
    fprintf(stderr, "yai-sdk: failed to connect to root socket\n");
    rpc_client_close(&c);
    return ENOTCONN; // 107
  }

  // Root enforces handshake before forwarding anything.
  if (yai_rpc_handshake(&c) != 0)
  {
    fprintf(stderr, "yai-sdk: handshake failed\n");
    rpc_client_close(&c);
    return 4; // runtime not ready-ish
  }

  char payload[256];
  memset(payload, 0, sizeof(payload));

  uint32_t out_len = 0;
  int rc = yai_rpc_call_raw(&c, YAI_CMD_PING, NULL, 0, payload, sizeof(payload), &out_len);
  if (out_len >= sizeof(payload)) out_len = (uint32_t)sizeof(payload) - 1;
  payload[out_len] = 0;
  if (rc != 0)
  {
    fprintf(stderr, "yai-sdk: ping call failed (rc=%d)\n", rc);
    rpc_client_close(&c);
    return rc;
  }

  puts(payload[0] ? payload : "{\"status\":\"ok\"}");
  rpc_client_close(&c);
  return 0;
}

// ------------------- bootstrap handlers (RPC minimal) -------------------

// Root ping: goes to root.sock; ws_id must be valid because root validates it.
static int ops_root_ping(int argc, char **argv)
{
  (void)argc;
  (void)argv;
  return rpc_ping("default");
}

// Kernel ping: forwarded by root to kernel; kernel requires ws_id non-empty too.
static int ops_kernel_ping(int argc, char **argv)
{
  (void)argc;
  (void)argv;
  return rpc_ping("default");
}

// Kernel ws: runtime doesn’t implement workspace mgmt yet (kernel only handles PING).
static int ops_kernel_ws(int argc, char **argv)
{
  (void)argv;
  if (argc < 1)
  {
    fprintf(stderr, "yai-sdk: missing required arg 'action' (kernel ws)\n");
    return YAI_SDK_BAD_ARGS;
  }
  fprintf(stderr, "yai-sdk: kernel ws not implemented in runtime yet\n");
  return 2;
}

// Map command_id -> handler (bootstrap-only for now)
static const yai_ops_entry_t kBootstrapMap[] = {
    {"yai.root.ping", ops_root_ping},
    {"yai.kernel.ping", ops_kernel_ping},
    {"yai.kernel.ws", ops_kernel_ws},
};

static yai_ops_fn_t find_bootstrap(const char *command_id)
{
  for (size_t i = 0; i < (sizeof(kBootstrapMap) / sizeof(kBootstrapMap[0])); i++)
  {
    if (kBootstrapMap[i].id && strcmp(kBootstrapMap[i].id, command_id) == 0)
    {
      return kBootstrapMap[i].fn;
    }
  }
  return NULL;
}

// ------------------- public dispatch -------------------

int yai_ops_dispatch_by_id(const char *command_id, int argc, char **argv)
{
  if (!command_id || command_id[0] == '\0')
  {
    fprintf(stderr, "yai-sdk: missing command id\n");
    return YAI_SDK_BAD_ARGS;
  }

  // 0) bootstrap map first
  yai_ops_fn_t bs = find_bootstrap(command_id);
  if (bs)
    return bs(argc, argv);

  // 1) generated map
  if (kOpsMapLen != 0)
  {
    for (size_t i = 0; i < kOpsMapLen; i++)
    {
      const char *id = kOpsMap[i].id;
      if (id && strcmp(id, command_id) == 0)
      {
        return kOpsMap[i].fn(argc, argv);
      }
    }
  }

  // 2) unmapped
  fprintf(stderr, "yai-sdk: command id not mapped to an ops handler yet\n");
  return 2;
}