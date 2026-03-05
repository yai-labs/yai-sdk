/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

/*
 * YAI SDK — Official Public Surface (v1)
 *
 * Consumers MUST include only this header:
 *   #include <yai_sdk/public.h>
 */

#ifdef __cplusplus
extern "C"
{
#endif

#define YAI_SDK_ABI_VERSION 1

int yai_sdk_abi_version(void);
const char *yai_sdk_version(void);

/* ---- Return codes / policy ---- */
#include <yai_sdk/errors.h>

/* ---- Platform paths ---- */
#include <yai_sdk/paths.h>

/* ---- Protocol-first client API ---- */
#include <yai_sdk/client.h>

/* ---- Registry (published surface) ---- */
#include <yai_sdk/registry/registry.h>
#include <yai_sdk/registry/registry_help.h>
#include <yai_sdk/registry/registry_validate.h>
#include <yai_sdk/registry/registry_registry.h>

/* ---- Transport ---- */
#include <yai_sdk/rpc/rpc.h>

#ifdef __cplusplus
}
#endif
