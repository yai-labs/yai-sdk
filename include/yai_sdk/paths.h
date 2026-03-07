#ifndef YAI_PATHS_H
#define YAI_PATHS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum yai_runtime_deploy_mode {
  YAI_RUNTIME_DEPLOY_UNKNOWN = 0,
  YAI_RUNTIME_DEPLOY_REPO_DEV = 1,
  YAI_RUNTIME_DEPLOY_LOCAL_INSTALL = 2,
  YAI_RUNTIME_DEPLOY_PACKAGED = 3,
  YAI_RUNTIME_DEPLOY_QUALIFICATION = 4
} yai_runtime_deploy_mode_t;

/* Runtime/install context roots. */
int yai_path_runtime_home(char *out, size_t cap);
int yai_path_install_root(char *out, size_t cap);
int yai_path_detect_deploy_mode(yai_runtime_deploy_mode_t *out_mode);

/* Runtime binary resolution by canonical precedence:
 * explicit env override -> install root candidates -> PATH lookup. */
int yai_path_boot_bin(char *out, size_t cap);
int yai_path_root_bin(char *out, size_t cap);
int yai_path_kernel_bin(char *out, size_t cap);
int yai_path_engine_bin(char *out, size_t cap);

/* Root endpoint (machine plane). */
int yai_path_root_sock(char *out, size_t cap);

/* Workspace paths (tenant plane). */
int yai_path_ws_sock(const char *ws_id, char *out, size_t cap);
int yai_path_ws_run_dir(const char *ws_id, char *out, size_t cap);
int yai_path_ws_db(const char *ws_id, char *out, size_t cap);

#ifdef __cplusplus
}
#endif

#endif
