/* SPDX-License-Identifier: Apache-2.0 */
#include <stdio.h>

#include <yai_sdk/public.h>

int main(void)
{
    yai_sdk_client_t *client = NULL;
    yai_sdk_client_opts_t opts = {0};
    yai_sdk_reply_t reply = {0};
    const char *request_json =
        "{"
        "\"type\":\"yai.control.call.v1\","
        "\"target_plane\":\"kernel\","
        "\"command_id\":\"yai.kernel.ws_list\","
        "\"argv\":[]"
        "}";
    int rc = 0;

    opts.ws_id = "default";
    opts.role = "operator";
    opts.arming = 1;
    opts.auto_handshake = 1;
    opts.correlation_id = "example-custom";

    rc = yai_sdk_client_open(&client, &opts);
    if (rc != YAI_SDK_OK) {
        fprintf(stderr, "open failed: %s\n", yai_sdk_errstr((yai_sdk_err_t)rc));
        return rc;
    }

    rc = yai_sdk_client_call_json(client, request_json, &reply);
    if (rc != YAI_SDK_OK) {
        fprintf(stderr, "call failed: %s\n", yai_sdk_errstr((yai_sdk_err_t)rc));
    } else {
        printf("code=%s summary=%s\n", reply.code, reply.summary);
        if (reply.exec_reply_json) {
            printf("exec_reply=%s\n", reply.exec_reply_json);
        }
    }

    yai_sdk_reply_free(&reply);
    yai_sdk_client_close(client);
    return rc;
}
