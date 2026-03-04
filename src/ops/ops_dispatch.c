// SPDX-License-Identifier: Apache-2.0
// src/ops/ops_dispatch.c

#include "yai_sdk/ops/ops_dispatch.h"
#include "yai_sdk/ops/ops_dispatch_gen.h"
#include "yai_sdk/errors.h"
#include "yai_sdk/rpc/rpc.h"

#include <cJSON.h>
#include <yai_protocol_ids.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef YAI_CMD_CONTROL_CALL
#define YAI_CMD_CONTROL_CALL 0x0105u
#endif

typedef int (*yai_ops_fn_t)(int argc, char **argv);

typedef struct
{
    const char *id;
    yai_ops_fn_t fn;
} yai_ops_entry_t;

/* --------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------- */

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
        return YAI_SDK_BAD_ARGS;
    return YAI_SDK_PROTOCOL;
}

static int rpc_call_control_call(const char *command_id, int argc, char **argv)
{
    yai_rpc_client_t c;
    int rc = rpc_connect_and_handshake(&c, "default", /*arming=*/1, /*role_str=*/"operator");
    if (rc != 0)
    {
        return (rc == ENOTCONN) ? YAI_SDK_SERVER_OFF : YAI_SDK_RUNTIME_NOT_READY;
    }

    cJSON *req = cJSON_CreateObject();
    if (!req)
    {
        yai_rpc_close(&c);
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
        return (rc == ENOTCONN) ? YAI_SDK_SERVER_OFF : YAI_SDK_RPC;
    }

    out[out_len < sizeof(out) ? out_len : sizeof(out) - 1] = '\0';
    cJSON *resp = cJSON_Parse(out);
    if (!resp)
    {
        return YAI_SDK_PROTOCOL;
    }

    const cJSON *status = cJSON_GetObjectItemCaseSensitive(resp, "status");
    const cJSON *code = cJSON_GetObjectItemCaseSensitive(resp, "code");
    int mapped = YAI_SDK_PROTOCOL;

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

    cJSON_Delete(resp);
    return mapped;
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
    /* Not implemented in runtime yet: keep explicit. */
    if (argc < 1 || !argv || !argv[0])
    {
        fprintf(stderr, "yai-sdk: missing required arg 'action' (kernel ws)\n");
        return YAI_SDK_BAD_ARGS;
    }
    fprintf(stderr, "yai-sdk: kernel ws not implemented in runtime yet (action=%s)\n", argv[0]);
    return 2;
}

static const yai_ops_entry_t kBootstrapMap[] = {
    {"yai.root.ping", ops_root_ping},
    {"yai.kernel.ping", ops_kernel_ping},
    {"yai.kernel.ws", ops_kernel_ws},
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

    /* 0) bootstrap first */
    yai_ops_fn_t bs = find_bootstrap(command_id);
    if (bs)
        return bs(argc, argv);

    /* 1) generated map */
    if (kOpsMapLen != 0 && kOpsMap)
    {
        for (size_t i = 0; i < kOpsMapLen; i++)
        {
            const char *id = kOpsMap[i].id;
            if (id && strcmp(id, command_id) == 0)
            {
                return kOpsMap[i].fn(argc, argv);
            }
        }
    }

    /* 2) unmapped -> generic deterministic control.call path */
    return rpc_call_control_call(command_id, argc, argv);
}
