/* SPDX-License-Identifier: Apache-2.0 */

#define _POSIX_C_SOURCE 200809L

#include <yai_sdk/context.h>
#include <yai_sdk/paths.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int is_valid_ws_id(const char *ws_id)
{
    const char *p;
    if (!ws_id || !ws_id[0])
        return 0;
    if (strchr(ws_id, '/') || strstr(ws_id, "..") || ws_id[0] == '~')
        return 0;
    for (p = ws_id; *p; p++)
    {
        const char c = *p;
        const int ok =
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '_') || (c == '-') || (c == '.');
        if (!ok)
            return 0;
    }
    return 1;
}

static const char *home_dir(void)
{
    const char *home = getenv("HOME");
    return (home && home[0]) ? home : NULL;
}

static int ensure_dir(const char *path, mode_t mode)
{
    struct stat st;
    if (stat(path, &st) == 0)
        return S_ISDIR(st.st_mode) ? 0 : -1;
    return mkdir(path, mode);
}

static int ensure_context_parent_dirs(void)
{
    char yai_dir[512];
    char ctx_dir[512];
    const char *home = home_dir();
    if (!home)
        return -1;
    if (snprintf(yai_dir, sizeof(yai_dir), "%s/.yai", home) <= 0)
        return -1;
    if (snprintf(ctx_dir, sizeof(ctx_dir), "%s/.yai/context", home) <= 0)
        return -1;
    if (ensure_dir(yai_dir, 0755) != 0)
        return -1;
    if (ensure_dir(ctx_dir, 0755) != 0)
        return -1;
    return 0;
}

static int context_file_path(char *out, size_t cap)
{
    const char *home = home_dir();
    if (!home || !out || cap == 0)
        return -1;
    if (snprintf(out, cap, "%s/.yai/context/current_workspace", home) <= 0)
        return -1;
    return 0;
}

int yai_sdk_context_get_current_workspace(char *out_ws_id, size_t out_cap)
{
    char path[512];
    FILE *f;
    char buf[128];
    size_t n;

    if (!out_ws_id || out_cap == 0)
        return -1;
    out_ws_id[0] = '\0';
    if (context_file_path(path, sizeof(path)) != 0)
        return -1;

    f = fopen(path, "r");
    if (!f)
        return 1;

    if (!fgets(buf, sizeof(buf), f))
    {
        fclose(f);
        return 1;
    }
    fclose(f);

    n = strlen(buf);
    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r' || buf[n - 1] == ' ' || buf[n - 1] == '\t'))
    {
        buf[n - 1] = '\0';
        n--;
    }

    if (!is_valid_ws_id(buf))
        return -1;

    if (snprintf(out_ws_id, out_cap, "%s", buf) <= 0)
        return -1;
    return 0;
}

int yai_sdk_context_set_current_workspace(const char *ws_id)
{
    char path[512];
    char tmp_path[544];
    FILE *f;

    if (!is_valid_ws_id(ws_id))
        return -1;
    if (ensure_context_parent_dirs() != 0)
        return -1;
    if (context_file_path(path, sizeof(path)) != 0)
        return -1;
    if (snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path) <= 0)
        return -1;

    f = fopen(tmp_path, "w");
    if (!f)
        return -1;
    if (fprintf(f, "%s\n", ws_id) <= 0)
    {
        fclose(f);
        unlink(tmp_path);
        return -1;
    }
    fclose(f);
    if (rename(tmp_path, path) != 0)
    {
        unlink(tmp_path);
        return -1;
    }
    return 0;
}

int yai_sdk_context_clear_current_workspace(void)
{
    char path[512];
    if (context_file_path(path, sizeof(path)) != 0)
        return -1;
    if (unlink(path) != 0)
    {
        return (access(path, F_OK) == 0) ? -1 : 0;
    }
    return 0;
}

int yai_sdk_context_resolve_workspace(
    const char *explicit_ws_id,
    char *out_ws_id,
    size_t out_cap)
{
    if (!out_ws_id || out_cap == 0)
        return -1;
    out_ws_id[0] = '\0';

    if (explicit_ws_id && explicit_ws_id[0])
    {
        if (!is_valid_ws_id(explicit_ws_id))
            return -1;
        if (snprintf(out_ws_id, out_cap, "%s", explicit_ws_id) <= 0)
            return -1;
        return 0;
    }

    return yai_sdk_context_get_current_workspace(out_ws_id, out_cap);
}
