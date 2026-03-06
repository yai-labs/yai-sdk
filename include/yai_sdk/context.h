/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns 0 and writes current ws_id when present.
 * Returns 1 when no current workspace is set.
 * Returns -1 on I/O/validation errors. */
int yai_sdk_context_get_current_workspace(char *out_ws_id, size_t out_cap);

/* Sets current workspace binding.
 * Returns 0 on success, -1 on validation/I/O errors. */
int yai_sdk_context_set_current_workspace(const char *ws_id);

/* Clears current workspace binding.
 * Returns 0 on success (also when already absent), -1 on I/O errors. */
int yai_sdk_context_clear_current_workspace(void);

/* Resolve effective workspace by precedence:
 * 1) explicit_ws_id when set
 * 2) current workspace context
 * Returns:
 * 0 -> resolved
 * 1 -> no workspace available
 * -1 -> errors */
int yai_sdk_context_resolve_workspace(
    const char *explicit_ws_id,
    char *out_ws_id,
    size_t out_cap);

#ifdef __cplusplus
}
#endif
