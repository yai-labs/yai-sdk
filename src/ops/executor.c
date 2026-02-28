// SPDX-License-Identifier: Apache-2.0
// yai-sdk executor (minimal).
//
// Purpose:
// - Provide a stable "executor installed" entrypoint for yai-cli.
// - Map a small set of canonical ids to SDK handlers.
// - Everything else returns "not mapped".
//
// You will later replace/extend this with a generated registry->handler map.

#include "yai_sdk/ops/executor.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

// ---------- helpers ----------

static void set_out(yai_exec_result_t* out, int code, const char* msg) {
  if (!out) return;
  out->code = code;
  out->message = msg;
}

typedef int (*yai_exec_fn_t)(const yai_exec_request_t* req, yai_exec_result_t* out);

typedef struct {
  const char* command_id;
  yai_exec_fn_t fn;
} yai_exec_map_entry_t;

// ---------- stub handlers (replace with real RPC calls) ----------
//
// These are deliberately minimal: they let you distinguish
// "UNMAPPED" vs "SERVEROFF" vs "NEEDARGS" at the CLI layer.

static int exec_root_ping(const yai_exec_request_t* req, yai_exec_result_t* out) {
  (void)req;
  // TODO: call real transport handshake / rpc client.
  // For now: pretend server is off.
  set_out(out, ENOTCONN, "yai-sdk: server unavailable (root ping)");
  return ENOTCONN;
}

static int exec_kernel_ping(const yai_exec_request_t* req, yai_exec_result_t* out) {
  (void)req;
  // TODO: call real kernel ping RPC.
  set_out(out, ENOTCONN, "yai-sdk: server unavailable (kernel ping)");
  return ENOTCONN;
}

static int exec_kernel_ws(const yai_exec_request_t* req, yai_exec_result_t* out) {
  // kernel-ws requires at least an action; if missing, return EINVAL-ish.
  if (!req || req->argc < 1) {
    set_out(out, EINVAL, "yai-sdk: missing required arg 'action' (kernel ws)");
    return EINVAL;
  }
  // TODO: call real kernel ws RPC.
  set_out(out, ENOTCONN, "yai-sdk: server unavailable (kernel ws)");
  return ENOTCONN;
}

// ---------- mapping table (minimal bootstrap) ----------

static const yai_exec_map_entry_t k_map[] = {
  { "yai.root.ping",   exec_root_ping   },
  { "yai.kernel.ping", exec_kernel_ping },
  { "yai.kernel.ws",   exec_kernel_ws   },
};

static yai_exec_fn_t find_handler(const char* command_id) {
  if (!command_id || !command_id[0]) return NULL;
  for (size_t i = 0; i < (sizeof(k_map) / sizeof(k_map[0])); i++) {
    if (strcmp(k_map[i].command_id, command_id) == 0) return k_map[i].fn;
  }
  return NULL;
}

// ---------- public entrypoint ----------

int yai_sdk_execute(const yai_exec_request_t* req, yai_exec_result_t* out) {
  if (!req || !req->command_id || !req->command_id[0]) {
    set_out(out, EINVAL, "yai-sdk: invalid request (missing command_id)");
    return EINVAL;
  }

  yai_exec_fn_t fn = find_handler(req->command_id);
  if (!fn) {
    // This is your current state: "not mapped".
    // Keep rc=2 because you already use that convention in CLI probes.
    set_out(out, 2, "yai-sdk: command id not mapped to an ops handler yet");
    return 2;
  }

  // Delegate to handler
  int rc = fn(req, out);
  if (out && out->code == 0) out->code = rc;
  return rc;
}