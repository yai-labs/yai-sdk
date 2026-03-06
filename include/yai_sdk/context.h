/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char ws_id[64];
    int exists;
    int valid;
    char state[32];
    char root_path[512];
} yai_sdk_workspace_info_t;

/*
 * Current workspace binding (CLI/session ergonomics layer).
 * Return 0 on success, non-zero on error.
 */
int yai_sdk_context_set_current_workspace(const char *ws_id);
int yai_sdk_context_get_current_workspace(char *out, size_t out_cap); /* 0 found, 1 empty, <0 error */
int yai_sdk_context_clear_current_workspace(void);

/*
 * Effective workspace resolution precedence:
 * 1) explicit_ws if non-empty
 * 2) current workspace binding
 * Return 0 with out populated, 1 when unresolved, <0 on error.
 */
int yai_sdk_context_resolve_workspace(const char *explicit_ws, char *out, size_t out_cap);

/*
 * Runtime-backed workspace validation/description.
 * Uses yai.kernel.ws_status and maps payload into info.
 */
int yai_sdk_workspace_describe(const char *ws_id, yai_sdk_workspace_info_t *info); /* 0 success */
int yai_sdk_context_validate_current_workspace(yai_sdk_workspace_info_t *info);      /* 0 valid */

#ifdef __cplusplus
}
#endif
