/* SPDX-License-Identifier: Apache-2.0 */
#include <stdio.h>
#include <stdlib.h>

#include <yai_sdk/public.h>

static void sdk_logger(yai_sdk_log_level_t level,
                       const char *component,
                       const char *message,
                       void *user_data)
{
    (void)user_data;
    fprintf(stderr, "[sdk:%d:%s] %s\n", (int)level,
            component ? component : "sdk",
            message ? message : "");
}

int main(void)
{
    yai_sdk_client_t *client = NULL;
    yai_sdk_client_opts_t opts = {0};
    yai_sdk_reply_t reply = {0};
    int rc = 0;

    opts.ws_id = "default";
    opts.role = "operator";
    opts.arming = 1;
    opts.auto_handshake = 1;
    opts.correlation_id = "example-basic";

    yai_sdk_set_log_handler(sdk_logger, NULL);
    yai_sdk_set_log_level(YAI_SDK_LOG_INFO);

    rc = yai_sdk_client_open(&client, &opts);
    if (rc != YAI_SDK_OK) {
        fprintf(stderr, "open failed: %s\n", yai_sdk_errstr((yai_sdk_err_t)rc));
        return rc;
    }

    rc = yai_sdk_client_ping(client, "yai.root.ping", &reply);
    if (rc != YAI_SDK_OK) {
        fprintf(stderr, "ping failed: %s\n", yai_sdk_errstr((yai_sdk_err_t)rc));
    } else {
        printf("status=%s code=%s summary=%s trace=%s\n",
               reply.status,
               reply.code,
               reply.summary,
               reply.trace_id);
    }

    yai_sdk_reply_free(&reply);
    yai_sdk_client_close(client);
    return rc;
}
