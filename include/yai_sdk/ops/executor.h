// SPDX-License-Identifier: Apache-2.0
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char* command_id;   // e.g. "yai.kernel.ping"
  int argc;
  const char** argv;        // raw argv (excluding program name is fine)
  int json_mode;            // 0 human, 1 json
} yai_exec_request_t;

typedef struct {
  int code;                 // 0 ok, non-zero = error
  const char* message;      // optional static message (or NULL)
} yai_exec_result_t;

// Main entrypoint: registry-driven execution.
// Returns 0 on success, non-zero on failure. Also sets out->code.
int yai_sdk_execute(const yai_exec_request_t* req, yai_exec_result_t* out);

#ifdef __cplusplus
}
#endif