/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include <yai_sdk/client.h>
#include <yai_sdk/rpc.h>

struct yai_sdk_client {
    yai_rpc_client_t rpc;
    char ws_id[128];
    char role[32];
    int arming;
    int auto_handshake;
    int is_open;
    int handshaken;
};

