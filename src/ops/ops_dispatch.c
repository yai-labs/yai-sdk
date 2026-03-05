// SPDX-License-Identifier: Apache-2.0
// src/ops/ops_dispatch.c

#include "yai_sdk/ops/ops_dispatch.h"
#include "yai_sdk/ops/ops_dispatch_gen.h"
#include "yai_sdk/errors.h"
#include "yai_sdk/rpc/rpc.h"
#include "yai_sdk/paths.h"

#include <cJSON.h>
#include <yai_protocol_ids.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifndef YAI_CMD_CONTROL_CALL
#define YAI_CMD_CONTROL_CALL 0x0105u
#endif

typedef int (*yai_ops_fn_t)(int argc, char **argv);

typedef struct
{
    const char *id;
    yai_ops_fn_t fn;
} yai_ops_entry_t;

static char g_last_status[16] = "error";
static char g_last_code[64] = "INTERNAL_ERROR";
static char g_last_reason[256] = "uninitialized";
static char g_last_command_id[128] = "yai.unknown.unknown";

/* --------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------- */

static void set_last_reply(const char *status,
                           const char *code,
                           const char *reason,
                           const char *command_id)
{
    snprintf(g_last_status, sizeof(g_last_status), "%s", (status && status[0]) ? status : "error");
    snprintf(g_last_code, sizeof(g_last_code), "%s", (code && code[0]) ? code : "INTERNAL_ERROR");
    snprintf(g_last_reason, sizeof(g_last_reason), "%s", (reason && reason[0]) ? reason : "unknown");
    snprintf(g_last_command_id,
             sizeof(g_last_command_id),
             "%s",
             (command_id && command_id[0]) ? command_id : "yai.unknown.unknown");
}

static void set_last_reply_from_rc(int rc, const char *command_id)
{
    if (rc == YAI_SDK_OK)
        set_last_reply("ok", "OK", "ok", command_id);
    else if (rc == YAI_SDK_NYI)
        set_last_reply("nyi", "NOT_IMPLEMENTED", "nyi_deterministic", command_id);
    else if (rc == YAI_SDK_BAD_ARGS)
        set_last_reply("error", "BAD_ARGS", "bad_args", command_id);
    else if (rc == YAI_SDK_UNAUTHORIZED)
        set_last_reply("error", "UNAUTHORIZED", "unauthorized", command_id);
    else if (rc == YAI_SDK_RUNTIME_NOT_READY)
        set_last_reply("error", "RUNTIME_NOT_READY", "runtime_not_ready", command_id);
    else if (rc == YAI_SDK_SERVER_OFF || rc == ENOTCONN)
        set_last_reply("error", "SERVER_UNAVAILABLE", "server_unavailable", command_id);
    else
        set_last_reply("error", "INTERNAL_ERROR", "failure", command_id);
}

void yai_ops_last_reply(const char **status,
                        const char **code,
                        const char **reason,
                        const char **command_id)
{
    if (status)
        *status = g_last_status;
    if (code)
        *code = g_last_code;
    if (reason)
        *reason = g_last_reason;
    if (command_id)
        *command_id = g_last_command_id;
}

static int is_error_payload(const char *s)
{
    if (!s)
        return 0;
    /* deterministic + minimal: no JSON parser yet */
    return (strstr(s, "\"status\":\"error\"") != NULL) ||
           (strstr(s, "\"status\": \"error\"") != NULL);
}

/* Enterprise rule: use only public RPC client API (no fd touching). */
static int rpc_connect_and_handshake(
    yai_rpc_client_t *c,
    const char *ws_id,
    int arming,
    const char *role_str)
{
    if (!c || !ws_id || !role_str)
        return YAI_SDK_BAD_ARGS;

    memset(c, 0, sizeof(*c));

    if (yai_rpc_connect(c, ws_id) != 0)
    {
        /* keep errno-class semantics */
        return ENOTCONN; /* 107 */
    }

    /* IMPORTANT: stamp authority BEFORE any post-handshake command. */
    yai_rpc_set_authority(c, arming, role_str);

    /* Root requires handshake before processing/forwarding anything. */
    if (yai_rpc_handshake(c) != 0)
    {
        yai_rpc_close(c);
        return YAI_SDK_RUNTIME_NOT_READY;
    }

    return 0;
}

static int rpc_call_ping(const char *ws_id)
{
    yai_rpc_client_t c;

    int rc = rpc_connect_and_handshake(&c, ws_id, /*arming=*/1, /*role_str=*/"operator");
    if (rc != 0)
    {
        if (rc == ENOTCONN)
        {
            fprintf(stderr, "yai-sdk: server unavailable (cannot connect root socket)\n");
        }
        else
        {
            fprintf(stderr, "yai-sdk: runtime not ready (handshake failed)\n");
        }
        return rc;
    }

    char out[256];
    memset(out, 0, sizeof(out));
    uint32_t out_len = 0;

    rc = yai_rpc_call_raw(
        &c,
        YAI_CMD_PING,
        /*payload=*/NULL,
        /*payload_len=*/0,
        /*out_buf=*/out,
        /*out_cap=*/sizeof(out),
        /*out_len=*/&out_len);

    /* Always close (single-request client for now) */
    yai_rpc_close(&c);

    if (rc != 0)
    {
        fprintf(stderr, "yai-sdk: ping call failed (rc=%d)\n", rc);
        return rc;
    }

    /* NUL terminate safely */
    if (out_len >= (uint32_t)sizeof(out))
        out_len = (uint32_t)sizeof(out) - 1;
    out[out_len] = '\0';

    /* If server returned an error JSON, make it a failing command. */
    if (is_error_payload(out))
    {
        /* print payload as-is (machine-friendly) */
        puts(out);
        return 2; /* invalid_arguments_or_contract_violation class */
    }

    puts(out[0] ? out : "{\"status\":\"ok\"}");
    return 0;
}

static int map_control_error_code(const char *code)
{
    if (!code || !code[0])
        return YAI_SDK_PROTOCOL;

    if (strcmp(code, "OK") == 0)
        return YAI_SDK_OK;
    if (strcmp(code, "NOT_IMPLEMENTED") == 0)
        return YAI_SDK_NYI;
    if (strcmp(code, "BAD_ARGS") == 0)
        return YAI_SDK_BAD_ARGS;
    if (strcmp(code, "UNAUTHORIZED") == 0)
        return YAI_SDK_UNAUTHORIZED;
    if (strcmp(code, "INVALID_TARGET") == 0)
        return YAI_SDK_PROTOCOL;
    if (strcmp(code, "RUNTIME_NOT_READY") == 0)
        return YAI_SDK_RUNTIME_NOT_READY;
    if (strcmp(code, "SERVER_UNAVAILABLE") == 0)
        return YAI_SDK_SERVER_OFF;
    if (strcmp(code, "PROTOCOL_ERROR") == 0)
        return YAI_SDK_PROTOCOL;
    if (strcmp(code, "INTERNAL_ERROR") == 0)
        return YAI_SDK_PROTOCOL;
    return YAI_SDK_PROTOCOL;
}

static int rpc_call_control_call_ws(const char *ws_id, const char *command_id, int argc, char **argv)
{
    yai_rpc_client_t c;
    const char *resolved_ws = (ws_id && ws_id[0]) ? ws_id : "default";
    int rc = rpc_connect_and_handshake(&c, resolved_ws, /*arming=*/1, /*role_str=*/"operator");
    if (rc != 0)
    {
        if (rc == ENOTCONN)
        {
            set_last_reply("error", "SERVER_UNAVAILABLE", "server_unavailable", command_id);
        }
        else
        {
            set_last_reply("error", "RUNTIME_NOT_READY", "runtime_not_ready", command_id);
        }
        return (rc == ENOTCONN) ? YAI_SDK_SERVER_OFF : YAI_SDK_RUNTIME_NOT_READY;
    }

    cJSON *req = cJSON_CreateObject();
    if (!req)
    {
        yai_rpc_close(&c);
        set_last_reply("error", "INTERNAL_ERROR", "request_encode_failed", command_id);
        return YAI_SDK_IO;
    }
    cJSON_AddStringToObject(req, "type", "yai.control.call.v1");
    cJSON_AddStringToObject(req, "target_plane", "kernel");
    cJSON_AddStringToObject(req, "command_id", command_id);
    cJSON *argv_json = cJSON_AddArrayToObject(req, "argv");
    for (int i = 0; i < argc; i++)
    {
        cJSON_AddItemToArray(argv_json, cJSON_CreateString(argv[i] ? argv[i] : ""));
    }

    char *payload = cJSON_PrintUnformatted(req);
    cJSON_Delete(req);
    if (!payload)
    {
        yai_rpc_close(&c);
        set_last_reply("error", "INTERNAL_ERROR", "request_encode_failed", command_id);
        return YAI_SDK_IO;
    }

    char out[2048];
    memset(out, 0, sizeof(out));
    uint32_t out_len = 0;

    rc = yai_rpc_call_raw(
        &c,
        YAI_CMD_CONTROL_CALL,
        payload,
        (uint32_t)strlen(payload),
        out,
        sizeof(out) - 1,
        &out_len);

    yai_rpc_close(&c);
    free(payload);

    if (rc != 0)
    {
        if (rc == ENOTCONN)
            set_last_reply("error", "SERVER_UNAVAILABLE", "server_unavailable", command_id);
        else
            set_last_reply("error", "PROTOCOL_ERROR", "rpc_call_failed", command_id);
        return (rc == ENOTCONN) ? YAI_SDK_SERVER_OFF : YAI_SDK_RPC;
    }

    out[out_len < sizeof(out) ? out_len : sizeof(out) - 1] = '\0';
    cJSON *resp = cJSON_Parse(out);
    if (!resp)
    {
        set_last_reply("error", "PROTOCOL_ERROR", "response_parse_failed", command_id);
        return YAI_SDK_PROTOCOL;
    }

    const cJSON *type = cJSON_GetObjectItemCaseSensitive(resp, "type");
    const cJSON *status = cJSON_GetObjectItemCaseSensitive(resp, "status");
    const cJSON *code = cJSON_GetObjectItemCaseSensitive(resp, "code");
    const cJSON *reason = cJSON_GetObjectItemCaseSensitive(resp, "reason");
    const cJSON *reply_command_id = cJSON_GetObjectItemCaseSensitive(resp, "command_id");
    int mapped = YAI_SDK_PROTOCOL;

    if (!cJSON_IsString(type) || !type->valuestring || strcmp(type->valuestring, "yai.exec.reply.v1") != 0)
    {
        cJSON_Delete(resp);
        set_last_reply("error", "PROTOCOL_ERROR", "bad_response_type", command_id);
        return YAI_SDK_PROTOCOL;
    }

    if (cJSON_IsString(status) && status->valuestring)
    {
        if (strcmp(status->valuestring, "ok") == 0)
        {
            mapped = YAI_SDK_OK;
        }
        else if (strcmp(status->valuestring, "nyi") == 0)
        {
            mapped = YAI_SDK_NYI;
        }
        else if (strcmp(status->valuestring, "error") == 0)
        {
            mapped = map_control_error_code(cJSON_IsString(code) ? code->valuestring : NULL);
        }
    }

    set_last_reply(
        cJSON_IsString(status) ? status->valuestring : "error",
        cJSON_IsString(code) ? code->valuestring : "PROTOCOL_ERROR",
        cJSON_IsString(reason) ? reason->valuestring : "missing_reason",
        cJSON_IsString(reply_command_id) ? reply_command_id->valuestring : command_id);

    cJSON_Delete(resp);
    return mapped;
}

static int rpc_call_control_call(const char *command_id, int argc, char **argv)
{
    return rpc_call_control_call_ws("default", command_id, argc, argv);
}

static int runtime_is_up(const char *ws_id)
{
    yai_rpc_client_t c;
    int rc = rpc_connect_and_handshake(&c, ws_id ? ws_id : "default", /*arming=*/1, /*role_str=*/"operator");
    if (rc != 0)
        return 0;
    yai_rpc_close(&c);
    return 1;
}

static int spawn_boot_detached(void)
{
    pid_t pid = fork();
    if (pid < 0)
        return YAI_SDK_IO;

    if (pid == 0)
    {
        (void)setsid();
        int devnull = open("/dev/null", O_RDWR);
        if (devnull >= 0)
        {
            (void)dup2(devnull, STDIN_FILENO);
            (void)dup2(devnull, STDOUT_FILENO);
            (void)dup2(devnull, STDERR_FILENO);
            if (devnull > STDERR_FILENO)
                (void)close(devnull);
        }

        const char *env_boot = getenv("YAI_BOOT_BIN");
        if (env_boot && env_boot[0])
            execl(env_boot, env_boot, "--master", (char *)NULL);

        execlp("yai-boot", "yai-boot", "--master", (char *)NULL);
        execl("../yai/build/bin/yai-boot", "../yai/build/bin/yai-boot", "--master", (char *)NULL);
        execl("../yai/dist/bin/yai-boot", "../yai/dist/bin/yai-boot", "--master", (char *)NULL);
        _exit(127);
    }

    return 0;
}

static void terminate_stack_processes(void)
{
    (void)system("pkill -TERM -f yai-root-server >/dev/null 2>&1 || true");
    (void)system("pkill -TERM -f yai-kernel >/dev/null 2>&1 || true");
    (void)system("pkill -TERM -f yai-engine >/dev/null 2>&1 || true");
    (void)system("pkill -TERM -f yai-boot >/dev/null 2>&1 || true");
}

static void sleep_ms(long ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    while (nanosleep(&ts, &ts) != 0 && errno == EINTR)
    {
    }
}

/* --------------------------------------------------------------------------
 * Bootstrap handlers (minimal, but REAL RPC)
 * -------------------------------------------------------------------------- */

static int ops_root_ping(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    /* Root validates ws_id. Keep deterministic default. */
    return rpc_call_ping("default");
}

static int ops_kernel_ping(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    /* Root forwards to kernel; kernel requires ws_id non-empty too. */
    return rpc_call_ping("default");
}

static int ops_kernel_ws(int argc, char **argv)
{
    if (argc < 1 || !argv || !argv[0])
    {
        fprintf(stderr, "yai-sdk: missing required arg 'action' (kernel ws)\n");
        return YAI_SDK_BAD_ARGS;
    }
    if (strcmp(argv[0], "create") != 0 &&
        strcmp(argv[0], "reset") != 0 &&
        strcmp(argv[0], "destroy") != 0)
    {
        fprintf(stderr, "yai-sdk: unsupported kernel ws action '%s'\n", argv[0]);
        return YAI_SDK_BAD_ARGS;
    }

    const char *ws_id = "default";
    for (int i = 1; i + 1 < argc; i++)
    {
        if ((strcmp(argv[i], "--ws-id") == 0 || strcmp(argv[i], "--ws") == 0) &&
            argv[i + 1] && argv[i + 1][0])
        {
            ws_id = argv[i + 1];
            break;
        }
    }

    return rpc_call_control_call_ws(ws_id, "yai.kernel.ws", argc, argv);
}

static int ops_lifecycle_up(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (runtime_is_up("default"))
        return YAI_SDK_OK;

    if (spawn_boot_detached() != 0)
        return YAI_SDK_IO;

    for (int i = 0; i < 50; i++)
    {
        if (runtime_is_up("default"))
            return YAI_SDK_OK;
        sleep_ms(100);
    }

    return YAI_SDK_RUNTIME_NOT_READY;
}

static int ops_lifecycle_down(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    terminate_stack_processes();
    sleep_ms(200);

    char root_sock[512];
    if (yai_path_root_sock(root_sock, sizeof(root_sock)) == 0)
    {
        (void)unlink(root_sock);
    }

    return YAI_SDK_OK;
}

static int ops_lifecycle_restart(int argc, char **argv)
{
    int rc = ops_lifecycle_down(argc, argv);
    if (rc != 0)
        return rc;
    return ops_lifecycle_up(argc, argv);
}

static const yai_ops_entry_t kBootstrapMap[] = {
    {"yai.root.ping", ops_root_ping},
    {"yai.kernel.ping", ops_kernel_ping},
    {"yai.kernel.ws", ops_kernel_ws},
    {"yai.lifecycle.up", ops_lifecycle_up},
    {"yai.lifecycle.down", ops_lifecycle_down},
    {"yai.lifecycle.restart", ops_lifecycle_restart},
};

static yai_ops_fn_t find_bootstrap(const char *command_id)
{
    for (size_t i = 0; i < (sizeof(kBootstrapMap) / sizeof(kBootstrapMap[0])); i++)
    {
        if (kBootstrapMap[i].id && strcmp(kBootstrapMap[i].id, command_id) == 0)
        {
            return kBootstrapMap[i].fn;
        }
    }
    return NULL;
}

/* --------------------------------------------------------------------------
 * Public dispatch
 * -------------------------------------------------------------------------- */

int yai_ops_dispatch_by_id(const char *command_id, int argc, char **argv)
{
    if (!command_id || command_id[0] == '\0')
    {
        fprintf(stderr, "yai-sdk: missing command id\n");
        return YAI_SDK_BAD_ARGS;
    }

    set_last_reply("error", "INTERNAL_ERROR", "dispatch_pending", command_id);

    /* 0) bootstrap first */
    yai_ops_fn_t bs = find_bootstrap(command_id);
    if (bs)
    {
        int rc = bs(argc, argv);
        if (strcmp(g_last_reason, "dispatch_pending") == 0)
            set_last_reply_from_rc(rc, command_id);
        return rc;
    }

    /* 1) generated map */
    if (kOpsMapLen != 0 && kOpsMap)
    {
        for (size_t i = 0; i < kOpsMapLen; i++)
        {
            const char *id = kOpsMap[i].id;
            if (id && strcmp(id, command_id) == 0)
            {
                int rc = kOpsMap[i].fn(argc, argv);
                if (strcmp(g_last_reason, "dispatch_pending") == 0)
                    set_last_reply_from_rc(rc, command_id);
                return rc;
            }
        }
    }

    /* 2) unmapped -> generic deterministic control.call path */
    {
        int rc = rpc_call_control_call(command_id, argc, argv);
        if (strcmp(g_last_reason, "dispatch_pending") == 0)
            set_last_reply_from_rc(rc, command_id);
        return rc;
    }
}
