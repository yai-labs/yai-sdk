#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Lifecycle
int yai_ops_lifecycle_up(int argc, char** argv);
int yai_ops_lifecycle_restart(int argc, char** argv);
int yai_ops_lifecycle_down(int argc, char** argv);

// Inspect
int yai_ops_inspect_status(int argc, char** argv);
int yai_ops_inspect_logs(int argc, char** argv);
int yai_ops_inspect_monitor(int argc, char** argv);
int yai_ops_inspect_events(int argc, char** argv);

// Verify
int yai_ops_verify_verify(int argc, char** argv);
int yai_ops_verify_test(int argc, char** argv);

// Control
int yai_ops_control_root(int argc, char** argv);
int yai_ops_control_kernel(int argc, char** argv);
int yai_ops_control_sessions(int argc, char** argv);
int yai_ops_control_providers(int argc, char** argv);
int yai_ops_control_chat(int argc, char** argv);
int yai_ops_control_shell(int argc, char** argv);
int yai_ops_control_dsar(int argc, char** argv);
int yai_ops_control_workspace(int argc, char** argv);

#ifdef __cplusplus
}
#endif

// Exit codes (must match registry defaults)
enum {
  YAI_EXIT_OK = 0,
  YAI_EXIT_GENERIC_FAILURE = 1,
  YAI_EXIT_INVALID_ARGS = 2,
  YAI_EXIT_DEPENDENCY_MISSING = 3,
  YAI_EXIT_RUNTIME_NOT_READY = 4,
};

static inline int yai_ops_exit_normalize(int rc) {
  // never return negatives to the shell
  if (rc < 0) return YAI_EXIT_GENERIC_FAILURE;
  return rc;
}
