#ifndef YAI_SDK_ERRORS_H
#define YAI_SDK_ERRORS_H

typedef enum {
    YAI_SDK_OK = 0,
    YAI_SDK_BAD_ARGS = 2,

    YAI_SDK_IO = 10,
    YAI_SDK_RPC = 20,
    YAI_SDK_PROTOCOL = 30,

    YAI_SDK_NOT_IMPLEMENTED = 99
} yai_sdk_err_t;

/* compat alias (temporary) */
#ifndef YAI_SDK_ERR_USAGE
#define YAI_SDK_ERR_USAGE YAI_SDK_BAD_ARGS
#endif

static inline const char* yai_sdk_err_str(yai_sdk_err_t code) {
    switch (code) {
        case YAI_SDK_OK: return "ok";
        case YAI_SDK_BAD_ARGS: return "bad_args";
        case YAI_SDK_IO: return "io";
        case YAI_SDK_RPC: return "rpc";
        case YAI_SDK_PROTOCOL: return "protocol";
        case YAI_SDK_NOT_IMPLEMENTED: return "not_implemented";
        default: return "unknown";
    }
}

#endif