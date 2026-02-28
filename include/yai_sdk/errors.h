#ifndef YAI_SDK_ERRORS_H
#define YAI_SDK_ERRORS_H

/*
 * Exit code classes (aligned with yai-cli porcelain):
 * 0: success
 * 2: invalid arguments / contract violation
 * 3: dependency missing (registry unavailable etc.)
 * 4: runtime not ready / handshake failed
 * 10+: internal IO/RPC/Protocol classes (still non-zero)
 *
 * NOTE: ENOTCONN (107) is used for "server unavailable".
 */

typedef enum
{
    YAI_SDK_OK = 0,

    /* CLI usage / contract */
    YAI_SDK_BAD_ARGS = 2,

    /* deps missing (registry files not present, etc.) */
    YAI_SDK_DEPS_MISSING = 3,

    /* runtime not ready (handshake fail / not ready state) */
    YAI_SDK_RUNTIME_NOT_READY = 4,

    /* internal classes (still non-zero) */
    YAI_SDK_IO = 10,
    YAI_SDK_RPC = 20,
    YAI_SDK_PROTOCOL = 30,

    YAI_SDK_NOT_IMPLEMENTED = 99
} yai_sdk_err_t;

#ifndef YAI_SDK_ERR_USAGE
#define YAI_SDK_ERR_USAGE YAI_SDK_BAD_ARGS
#endif

static inline const char *yai_sdk_err_str(yai_sdk_err_t code)
{
    switch (code)
    {
    case YAI_SDK_OK:
        return "ok";
    case YAI_SDK_BAD_ARGS:
        return "bad_args";
    case YAI_SDK_DEPS_MISSING:
        return "deps_missing";
    case YAI_SDK_RUNTIME_NOT_READY:
        return "runtime_not_ready";
    case YAI_SDK_IO:
        return "io";
    case YAI_SDK_RPC:
        return "rpc";
    case YAI_SDK_PROTOCOL:
        return "protocol";
    case YAI_SDK_NOT_IMPLEMENTED:
        return "not_implemented";
    default:
        return "unknown";
    }
}

#endif