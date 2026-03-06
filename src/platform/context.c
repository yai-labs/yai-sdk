/* SPDX-License-Identifier: Apache-2.0 */

#include "yai_sdk/context.h"
#include "yai_sdk/errors.h"
#include "yai_sdk/ops/executor.h"
#include "yai_sdk/ops/ops_dispatch.h"

#include <cJSON.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef YAI_SDK_CONTEXT_FILE
#define YAI_SDK_CONTEXT_FILE ".yai/context/current_workspace"
#endif

static int build_context_path(char *out, size_t out_cap)
{
    const char *home = getenv("HOME");
    if (!out || out_cap == 0 || !home || !home[0]) {
        return -1;
    }
    if (snprintf(out, out_cap, "%s/%s", home, YAI_SDK_CONTEXT_FILE) <= 0) {
        return -1;
    }
    return 0;
}

static int ensure_parent_dir(const char *path)
{
    char tmp[1024];
    char *p;
    if (!path || !path[0]) return -1;
    snprintf(tmp, sizeof(tmp), "%s", path);
    p = strrchr(tmp, '/');
    if (!p) return 0;
    *p = '\0';
    for (p = tmp + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) return -1;
    return 0;
}

static int is_valid_ws_id(const char *ws_id)
{
    size_t i;
    if (!ws_id || !ws_id[0]) return 0;
    for (i = 0; ws_id[i]; i++) {
        const char ch = ws_id[i];
        const int ok = (ch >= 'a' && ch <= 'z') ||
                       (ch >= 'A' && ch <= 'Z') ||
                       (ch >= '0' && ch <= '9') ||
                       ch == '_' || ch == '-';
        if (!ok) return 0;
    }
    return 1;
}

int yai_sdk_context_set_current_workspace(const char *ws_id)
{
    char path[1024];
    FILE *f = NULL;
    if (!is_valid_ws_id(ws_id)) return YAI_SDK_BAD_ARGS;
    if (build_context_path(path, sizeof(path)) != 0) return YAI_SDK_IO;
    if (ensure_parent_dir(path) != 0) return YAI_SDK_IO;
    f = fopen(path, "w");
    if (!f) return YAI_SDK_IO;
    fprintf(f, "%s\n", ws_id);
    fclose(f);
    return YAI_SDK_OK;
}

int yai_sdk_context_get_current_workspace(char *out, size_t out_cap)
{
    char path[1024];
    FILE *f = NULL;
    if (!out || out_cap == 0) return -1;
    out[0] = '\0';
    if (build_context_path(path, sizeof(path)) != 0) return -1;
    f = fopen(path, "r");
    if (!f) {
        return (errno == ENOENT) ? 1 : -1;
    }
    if (!fgets(out, (int)out_cap, f)) {
        fclose(f);
        return 1;
    }
    fclose(f);
    out[strcspn(out, "\r\n")] = '\0';
    if (!out[0]) return 1;
    return 0;
}

int yai_sdk_context_clear_current_workspace(void)
{
    char path[1024];
    if (build_context_path(path, sizeof(path)) != 0) return YAI_SDK_IO;
    if (remove(path) != 0 && errno != ENOENT) return YAI_SDK_IO;
    return YAI_SDK_OK;
}

int yai_sdk_context_resolve_workspace(const char *explicit_ws, char *out, size_t out_cap)
{
    if (!out || out_cap == 0) return -1;
    out[0] = '\0';
    if (explicit_ws && explicit_ws[0]) {
        if (!is_valid_ws_id(explicit_ws)) return YAI_SDK_BAD_ARGS;
        snprintf(out, out_cap, "%s", explicit_ws);
        return 0;
    }
    return yai_sdk_context_get_current_workspace(out, out_cap);
}

int yai_sdk_workspace_describe(const char *ws_id, yai_sdk_workspace_info_t *info)
{
    yai_exec_request_t req;
    yai_exec_result_t out = {0};
    const char *argv[3];
    int rc;
    const char *data_json = NULL;
    cJSON *data = NULL;
    const cJSON *item = NULL;

    if (!ws_id || !ws_id[0] || !info) return YAI_SDK_BAD_ARGS;
    memset(info, 0, sizeof(*info));
    snprintf(info->ws_id, sizeof(info->ws_id), "%s", ws_id);

    argv[0] = "--ws-id";
    argv[1] = ws_id;
    argv[2] = NULL;
    req.command_id = "yai.kernel.ws_status";
    req.argc = 2;
    req.argv = argv;
    req.json_mode = 1;

    rc = yai_sdk_execute(&req, &out);
    if (rc != 0) return rc;

    data_json = yai_ops_last_reply_data_json();
    if (!data_json || !data_json[0]) {
        return YAI_SDK_PROTOCOL;
    }

    data = cJSON_Parse(data_json);
    if (!data) return YAI_SDK_PROTOCOL;

    item = cJSON_GetObjectItemCaseSensitive(data, "exists");
    info->exists = cJSON_IsTrue(item) ? 1 : 0;
    info->valid = info->exists;

    item = cJSON_GetObjectItemCaseSensitive(data, "state");
    if (cJSON_IsString(item) && item->valuestring) {
        snprintf(info->state, sizeof(info->state), "%s", item->valuestring);
    }
    item = cJSON_GetObjectItemCaseSensitive(data, "root_path");
    if (cJSON_IsString(item) && item->valuestring) {
        snprintf(info->root_path, sizeof(info->root_path), "%s", item->valuestring);
    }

    cJSON_Delete(data);
    return YAI_SDK_OK;
}

int yai_sdk_context_validate_current_workspace(yai_sdk_workspace_info_t *info)
{
    char ws_id[64] = {0};
    int rc = yai_sdk_context_get_current_workspace(ws_id, sizeof(ws_id));
    if (rc != 0) {
        return (rc == 1) ? YAI_SDK_BAD_ARGS : YAI_SDK_IO;
    }
    return yai_sdk_workspace_describe(ws_id, info);
}
