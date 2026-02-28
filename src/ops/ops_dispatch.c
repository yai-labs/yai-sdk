// SPDX-License-Identifier: Apache-2.0
// src/ops/ops_dispatch.c

#include "yai_sdk/ops/ops_dispatch.h"
#include "yai_sdk/ops/ops_dispatch_gen.h"
#include "yai_sdk/errors.h"

#include <stdio.h>
#include <string.h>

int yai_ops_dispatch_by_id(const char* command_id, int argc, char** argv)
{
  (void)argc;
  (void)argv;

  if (!command_id || command_id[0] == '\0') {
    fprintf(stderr, "yai-sdk: missing command id\n");
    return YAI_SDK_BAD_ARGS;
  }

  // Generated map exists, but it may be empty until executor/handlers are installed.
  if (kOpsMapLen == 0) {
    fprintf(stderr,
            "yai-sdk: command id not mapped to an ops handler yet "
            "(registry-driven mode: executor not installed)\n");
    return YAI_SDK_BAD_ARGS;
  }

  for (size_t i = 0; i < kOpsMapLen; i++) {
    const char* id = kOpsMap[i].id;
    if (id && strcmp(id, command_id) == 0) {
      // Forward to the handler. Handler decides rc semantics.
      return kOpsMap[i].fn(argc, argv);
    }
  }

  fprintf(stderr, "yai-sdk: command id not mapped to an ops handler yet\n");
  return YAI_SDK_BAD_ARGS;
}