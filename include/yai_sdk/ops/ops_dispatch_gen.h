// include/yai_sdk/ops/ops_dispatch_gen.h
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char* id;
  int (*fn)(int, char**);
} yai_ops_map_t;

// Generated map (may be empty). We expose pointer+len to allow empty map cleanly.
extern const yai_ops_map_t* kOpsMap;
extern const size_t kOpsMapLen;

#ifdef __cplusplus
}
#endif