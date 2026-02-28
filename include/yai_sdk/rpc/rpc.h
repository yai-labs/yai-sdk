/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/*
 * RPC client (strict, low-level)
 *
 * Single source of truth for the CLI RPC transport client.
 * Implemented by src/runtime/rpc_client.c (connect/handshake/call_raw).
 *
 * Notes:
 * - ws_id is bound at connect time (copied into client)
 * - role/arming are stamped into the envelope for every call
 * - output buffers are caller-provided (no YAI_RPC_LINE_MAX here)
 */

typedef struct yai_rpc_client {
    int fd;

    /* workspace id (sanitized by connect) */
    char ws_id[128];

    /* authority */
    uint8_t role;     /* YAI_ROLE_* from roles.h */
    uint8_t arming;   /* 0/1 */
} yai_rpc_client_t;

/* Connect to root control socket, binding this client to ws_id (string copied). */
int  yai_rpc_connect(yai_rpc_client_t *c, const char *ws_id);

/* Close socket if open; safe to call multiple times. */
void yai_rpc_close(yai_rpc_client_t *c);

/* Set authority that will be stamped on subsequent calls. */
void yai_rpc_set_authority(yai_rpc_client_t *c, int arming, const char *role_str);

/*
 * Raw call: send envelope + payload, receive envelope + payload.
 * Returns 0 on success; negative values are transport/protocol errors.
 */
int yai_rpc_call_raw(
    yai_rpc_client_t *c,
    uint32_t command_id,
    const void *payload,
    uint32_t payload_len,
    void *out_buf,
    size_t out_cap,
    uint32_t *out_len);

/*
 * Handshake: validates server readiness and protocol version.
 * Returns 0 on success.
 */
int yai_rpc_handshake(yai_rpc_client_t *c);

#ifdef __cplusplus
}
#endif