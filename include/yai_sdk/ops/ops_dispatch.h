#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Dispatch a canonical command id (yai.<group>.<name>) to an ops handler.
int yai_ops_dispatch_by_id(const char* command_id, int argc, char** argv);

#ifdef __cplusplus
}
#endif