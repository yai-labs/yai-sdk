/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define YAI_SDK_ABI_VERSION 1

int yai_sdk_abi_version(void);
const char *yai_sdk_version(void);

#include <yai_sdk/errors.h>
#include <yai_sdk/paths.h>
#include <yai_sdk/client.h>
#include <yai_sdk/catalog.h>
#include <yai_sdk/protocol.h>
#include <yai_sdk/rpc.h>
#include <yai_sdk/reply/reply.h>
#include <yai_sdk/reply/reply_builder.h>
#include <yai_sdk/reply/reply_json.h>

/* Legacy law views kept public for one transition wave. */
#include <yai_sdk/registry/registry.h>
#include <yai_sdk/registry/registry_help.h>
#include <yai_sdk/registry/registry_validate.h>
#include <yai_sdk/registry/registry_registry.h>

#ifdef __cplusplus
}
#endif
