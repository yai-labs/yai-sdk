#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Dispatch a canonical command id (yai.<group>.<name>) to an ops handler.
int yai_ops_dispatch_by_id(const char* command_id, int argc, char** argv);

// Returns last normalized execution reply metadata captured by dispatch.
// Pointers are process-local static storage and must be treated as read-only.
void yai_ops_last_reply(const char** status,
                        const char** code,
                        const char** reason,
                        const char** command_id);

void yai_ops_last_reply_ext(const char** status,
                            const char** code,
                            const char** reason,
                            const char** command_id,
                            const char** trace_id,
                            const char** target_plane);

// Returns last reply "data" JSON object (serialized) when available.
// The pointer references process-local static storage.
const char* yai_ops_last_reply_data_json(void);

#ifdef __cplusplus
}
#endif
