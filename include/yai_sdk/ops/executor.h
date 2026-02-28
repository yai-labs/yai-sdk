// SPDX-License-Identifier: Apache-2.0
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h> // size_t
#include <stdint.h> // uint32_t

  typedef struct
  {
    const char *command_id; // e.g. "yai.kernel.ping"
    int argc;
    const char **argv; // argv slice (may be NULL if argc==0)
    int json_mode;     // 0 human, 1 json
  } yai_exec_request_t;

  typedef struct
  {
    int code;            // 0 ok, non-zero error class / errno
    const char *message; // optional (may be NULL)
  } yai_exec_result_t;

  /*
   * Executor entrypoint:
   * - returns rc (0 ok)
   * - also sets out->code (best-effort)
   */
  int yai_sdk_execute(const yai_exec_request_t *req, yai_exec_result_t *out);

#ifdef __cplusplus
}
#endif