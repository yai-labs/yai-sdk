/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

/*
 * YAI SDK — Official Public Surface (v0)
 *
 * Consumers MUST include only this header:
 *   #include <yai_sdk/yai.h>
 *
 * Direct inclusion of sub-headers is considered advanced use and is not
 * covered by the compatibility contract unless explicitly documented.
 */

#ifdef __cplusplus
extern "C"
{
#endif

/* ---- Return codes / policy ---- */
#include <yai_sdk/errors.h>

/* ---- Platform paths ---- */
#include <yai_sdk/paths.h>

/* ---- Registry (published surface) ---- */
#include <yai_sdk/registry/registry.h>
#include <yai_sdk/registry/registry_help.h>
#include <yai_sdk/registry/registry_validate.h>
#include <yai_sdk/registry/registry_registry.h> /* query API lives here */

/* ---- Execution ---- */
#include <yai_sdk/ops/executor.h>
#include <yai_sdk/ops/ops_dispatch.h>

/* ---- Transport ---- */
#include <yai_sdk/rpc/rpc.h>

#ifdef __cplusplus
}
#endif