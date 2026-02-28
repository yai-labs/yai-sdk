// SPDX-License-Identifier: Apache-2.0
// src/ops/ops_dispatch.c

#include "yai_sdk/ops/ops_dispatch.h"
#include "yai_sdk/ops/ops_dispatch_gen.h"
#include "yai_sdk/errors.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

typedef int (*yai_ops_fn_t)(int argc, char** argv);

typedef struct {
  const char* id;
  yai_ops_fn_t fn;
} yai_ops_entry_t;

// ------------------- bootstrap handlers (minimal) -------------------
// Replace these with real RPC calls later. For now they give you correct rc classes.

static int ops_root_ping(int argc, char** argv) {
  (void)argc; (void)argv;
  fprintf(stderr, "yai-sdk: server unavailable (root ping)\n");
  return ENOTCONN;
}

static int ops_kernel_ping(int argc, char** argv) {
  (void)argc; (void)argv;
  fprintf(stderr, "yai-sdk: server unavailable (kernel ping)\n");
  return ENOTCONN;
}

static int ops_kernel_ws(int argc, char** argv) {
  // Expect at least one positional action (create/destroy/reset)
  if (argc < 1) {
    fprintf(stderr, "yai-sdk: missing required arg 'action' (kernel ws)\n");
    return YAI_SDK_BAD_ARGS;
  }
  (void)argv;
  fprintf(stderr, "yai-sdk: server unavailable (kernel ws)\n");
  return ENOTCONN;
}

static const yai_ops_entry_t kBootstrapMap[] = {
  { "yai.root.ping",   ops_root_ping   },
  { "yai.kernel.ping", ops_kernel_ping },
  { "yai.kernel.ws",   ops_kernel_ws   },
};

static yai_ops_fn_t find_bootstrap(const char* command_id) {
  for (size_t i = 0; i < (sizeof(kBootstrapMap) / sizeof(kBootstrapMap[0])); i++) {
    if (kBootstrapMap[i].id && strcmp(kBootstrapMap[i].id, command_id) == 0) {
      return kBootstrapMap[i].fn;
    }
  }
  return NULL;
}

// ------------------- public dispatch -------------------

int yai_ops_dispatch_by_id(const char* command_id, int argc, char** argv)
{
  if (!command_id || command_id[0] == '\0') {
    fprintf(stderr, "yai-sdk: missing command id\n");
    return YAI_SDK_BAD_ARGS;
  }

  // 0) bootstrap map first (gives you real rc semantics immediately)
  yai_ops_fn_t bs = find_bootstrap(command_id);
  if (bs) return bs(argc, argv);

  // 1) generated map (future: real registry->handler install)
  if (kOpsMapLen != 0) {
    for (size_t i = 0; i < kOpsMapLen; i++) {
      const char* id = kOpsMap[i].id;
      if (id && strcmp(id, command_id) == 0) {
        return kOpsMap[i].fn(argc, argv);
      }
    }
  }

  // 2) unmapped (NOT bad args)
  fprintf(stderr, "yai-sdk: command id not mapped to an ops handler yet\n");
  return 2;
}