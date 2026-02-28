// SPDX-License-Identifier: Apache-2.0
// src/registry/registry_cache.c
//
// File-backed registry cache (block-zero mode).
// Reads deps/yai-law/registry/{commands,artifacts}.v1.json via law_paths.
// Later, when you have a real generator, you can swap this to embedded tables.

#include "yai_sdk/registry/registry_cache.h"
#include "yai_sdk/registry/registry_paths.h"
#include "yai_sdk/registry/registry_types.h"

#include "cJSON.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* read_file_all(const char* path) {
  FILE* f = fopen(path, "rb");
  if (!f) return NULL;
  if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
  long n = ftell(f);
  if (n < 0) { fclose(f); return NULL; }
  rewind(f);

  char* buf = (char*)malloc((size_t)n + 1);
  if (!buf) { fclose(f); return NULL; }

  size_t rd = fread(buf, 1, (size_t)n, f);
  fclose(f);
  if (rd != (size_t)n) { free(buf); return NULL; }

  buf[n] = 0;
  return buf;
}

static char* dup_json_str(cJSON* obj, const char* key) {
  cJSON* v = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (!cJSON_IsString(v) || !v->valuestring) return NULL;
  return strdup(v->valuestring);
}

static int load_string_array(cJSON* arr, const char*** out_items, size_t* out_len) {
  *out_items = NULL; *out_len = 0;
  if (!arr) return 0;
  if (!cJSON_IsArray(arr)) return 1;

  int n = cJSON_GetArraySize(arr);
  if (n <= 0) return 0;

  const char** items = (const char**)calloc((size_t)n, sizeof(char*));
  if (!items) return ENOMEM;

  for (int i = 0; i < n; i++) {
    cJSON* it = cJSON_GetArrayItem(arr, i);
    if (!cJSON_IsString(it) || !it->valuestring) { free(items); return 2; }
    items[i] = strdup(it->valuestring);
  }

  *out_items = items;
  *out_len = (size_t)n;
  return 0;
}

static int load_args_array(cJSON* arr, yai_law_arg_t** out_args, size_t* out_len) {
  *out_args = NULL; *out_len = 0;
  if (!arr) return 0;
  if (!cJSON_IsArray(arr)) return 1;

  int n = cJSON_GetArraySize(arr);
  if (n <= 0) return 0;

  yai_law_arg_t* args = (yai_law_arg_t*)calloc((size_t)n, sizeof(yai_law_arg_t));
  if (!args) return ENOMEM;

  for (int i = 0; i < n; i++) {
    cJSON* a = cJSON_GetArrayItem(arr, i);
    if (!cJSON_IsObject(a)) { free(args); return 2; }

    args[i].name = dup_json_str(a, "name");
    args[i].type = dup_json_str(a, "type");
    args[i].flag = dup_json_str(a, "flag");

    cJSON* pos = cJSON_GetObjectItemCaseSensitive(a, "pos");
    args[i].pos = cJSON_IsNumber(pos) ? (int32_t)pos->valuedouble : 0;

    cJSON* req = cJSON_GetObjectItemCaseSensitive(a, "required");
    args[i].required = cJSON_IsTrue(req) ? 1 : 0;

    cJSON* vals = cJSON_GetObjectItemCaseSensitive(a, "values");
    if (vals) {
      const char** tmp = NULL; size_t tmp_len = 0;
      int rc = load_string_array(vals, &tmp, &tmp_len);
      if (rc != 0) { free(args); return rc; }
      args[i].values = tmp;
      args[i].values_len = tmp_len;
    }

    cJSON* def = cJSON_GetObjectItemCaseSensitive(a, "default");
    if (cJSON_IsBool(def)) {
      args[i].default_b_set = 1;
      args[i].default_b = cJSON_IsTrue(def) ? 1 : 0;
    } else if (cJSON_IsNumber(def)) {
      args[i].default_i_set = 1;
      args[i].default_i = (int64_t)def->valuedouble;
    } else if (cJSON_IsString(def) && def->valuestring) {
      args[i].default_s = strdup(def->valuestring);
    }
  }

  *out_args = args;
  *out_len = (size_t)n;
  return 0;
}

static int load_artifacts_io(cJSON* arr, yai_law_artifact_io_t** out_io, size_t* out_len) {
  *out_io = NULL; *out_len = 0;
  if (!arr) return 0;
  if (!cJSON_IsArray(arr)) return 1;

  int n = cJSON_GetArraySize(arr);
  if (n <= 0) return 0;

  yai_law_artifact_io_t* ios = (yai_law_artifact_io_t*)calloc((size_t)n, sizeof(yai_law_artifact_io_t));
  if (!ios) return ENOMEM;

  for (int i = 0; i < n; i++) {
    cJSON* o = cJSON_GetArrayItem(arr, i);
    if (!cJSON_IsObject(o)) { free(ios); return 2; }
    ios[i].role = dup_json_str(o, "role");
    ios[i].schema_ref = dup_json_str(o, "schema_ref");
    ios[i].path_hint = dup_json_str(o, "path_hint");
  }

  *out_io = ios;
  *out_len = (size_t)n;
  return 0;
}

static int load_artifacts_table(const char* path, yai_law_artifact_role_t** out, size_t* out_len, char** out_version, char** out_binary) {
  *out = NULL; *out_len = 0;
  if (out_version) *out_version = NULL;
  if (out_binary) *out_binary = NULL;

  char* txt = read_file_all(path);
  if (!txt) return ENOENT;

  cJSON* root = cJSON_Parse(txt);
  free(txt);
  if (!root) return 2;

  if (out_version) *out_version = dup_json_str(root, "version");
  if (out_binary)  *out_binary  = dup_json_str(root, "binary");

  cJSON* arr = cJSON_GetObjectItemCaseSensitive(root, "artifacts");
  if (!cJSON_IsArray(arr)) { cJSON_Delete(root); return 3; }

  int n = cJSON_GetArraySize(arr);
  yai_law_artifact_role_t* roles = (yai_law_artifact_role_t*)calloc((size_t)n, sizeof(yai_law_artifact_role_t));
  if (!roles) { cJSON_Delete(root); return ENOMEM; }

  for (int i = 0; i < n; i++) {
    cJSON* a = cJSON_GetArrayItem(arr, i);
    if (!cJSON_IsObject(a)) { free(roles); cJSON_Delete(root); return 4; }
    roles[i].role = dup_json_str(a, "role");
    roles[i].schema_ref = dup_json_str(a, "schema_ref");
    roles[i].description = dup_json_str(a, "description");
  }

  cJSON_Delete(root);
  *out = roles;
  *out_len = (size_t)n;
  return 0;
}

static int load_commands_table(const char* path, yai_law_command_t** out, size_t* out_len, char** out_version, char** out_binary) {
  *out = NULL; *out_len = 0;
  if (out_version) *out_version = NULL;
  if (out_binary) *out_binary = NULL;

  char* txt = read_file_all(path);
  if (!txt) return ENOENT;

  cJSON* root = cJSON_Parse(txt);
  free(txt);
  if (!root) return 2;

  if (out_version) *out_version = dup_json_str(root, "version");
  if (out_binary)  *out_binary  = dup_json_str(root, "binary");

  cJSON* arr = cJSON_GetObjectItemCaseSensitive(root, "commands");
  if (!cJSON_IsArray(arr)) { cJSON_Delete(root); return 3; }

  int n = cJSON_GetArraySize(arr);
  yai_law_command_t* cmds = (yai_law_command_t*)calloc((size_t)n, sizeof(yai_law_command_t));
  if (!cmds) { cJSON_Delete(root); return ENOMEM; }

  for (int i = 0; i < n; i++) {
    cJSON* c = cJSON_GetArrayItem(arr, i);
    if (!cJSON_IsObject(c)) { free(cmds); cJSON_Delete(root); return 4; }

    cmds[i].id = dup_json_str(c, "id");
    cmds[i].name = dup_json_str(c, "name");
    cmds[i].group = dup_json_str(c, "group");
    cmds[i].summary = dup_json_str(c, "summary");

    (void)load_args_array(cJSON_GetObjectItemCaseSensitive(c, "args"), (yai_law_arg_t**)&cmds[i].args, &cmds[i].args_len);

    (void)load_string_array(cJSON_GetObjectItemCaseSensitive(c, "outputs"), &cmds[i].outputs, &cmds[i].outputs_len);
    (void)load_string_array(cJSON_GetObjectItemCaseSensitive(c, "side_effects"), &cmds[i].side_effects, &cmds[i].side_effects_len);

    (void)load_string_array(cJSON_GetObjectItemCaseSensitive(c, "law_hooks"), &cmds[i].law_hooks, &cmds[i].law_hooks_len);
    (void)load_string_array(cJSON_GetObjectItemCaseSensitive(c, "law_invariants"), &cmds[i].law_invariants, &cmds[i].law_invariants_len);
    (void)load_string_array(cJSON_GetObjectItemCaseSensitive(c, "law_boundaries"), &cmds[i].law_boundaries, &cmds[i].law_boundaries_len);

    (void)load_string_array(cJSON_GetObjectItemCaseSensitive(c, "uses_primitives"), &cmds[i].uses_primitives, &cmds[i].uses_primitives_len);

    (void)load_artifacts_io(cJSON_GetObjectItemCaseSensitive(c, "emits_artifacts"), (yai_law_artifact_io_t**)&cmds[i].emits_artifacts, &cmds[i].emits_artifacts_len);
    (void)load_artifacts_io(cJSON_GetObjectItemCaseSensitive(c, "consumes_artifacts"), (yai_law_artifact_io_t**)&cmds[i].consumes_artifacts, &cmds[i].consumes_artifacts_len);
  }

  cJSON_Delete(root);
  *out = cmds;
  *out_len = (size_t)n;
  return 0;
}

static void free_string_array_owned(const char** items, size_t len) {
  if (!items) return;
  for (size_t i = 0; i < len; i++) free((void*)items[i]);
  free((void*)items);
}

static void free_args_owned(const yai_law_arg_t* args, size_t len) {
  if (!args) return;
  for (size_t i = 0; i < len; i++) {
    free((void*)args[i].name);
    free((void*)args[i].flag);
    free((void*)args[i].type);
    free((void*)args[i].default_s);
    free_string_array_owned(args[i].values, args[i].values_len);
  }
  free((void*)args);
}

static void free_artifacts_io_owned(const yai_law_artifact_io_t* io, size_t len) {
  if (!io) return;
  for (size_t i = 0; i < len; i++) {
    free((void*)io[i].role);
    free((void*)io[i].schema_ref);
    free((void*)io[i].path_hint);
  }
  free((void*)io);
}

void yai_law_registry_cache_init(yai_law_registry_cache_t* cache) {
  if (!cache) return;
  memset(cache, 0, sizeof(*cache));
}

void yai_law_registry_cache_clear(yai_law_registry_cache_t* cache) {
  if (!cache) return;
  memset(&cache->registry, 0, sizeof(cache->registry));
  cache->loaded = 0;
}

void yai_law_registry_cache_free(yai_law_registry_cache_t* cache) {
  if (!cache) return;

  if (cache->registry.commands) {
    for (size_t i = 0; i < cache->registry.commands_len; i++) {
      yai_law_command_t* c = (yai_law_command_t*)&cache->registry.commands[i];
      free((void*)c->id);
      free((void*)c->name);
      free((void*)c->group);
      free((void*)c->summary);

      free_args_owned(c->args, c->args_len);
      free_string_array_owned(c->outputs, c->outputs_len);
      free_string_array_owned(c->side_effects, c->side_effects_len);

      free_string_array_owned(c->law_hooks, c->law_hooks_len);
      free_string_array_owned(c->law_invariants, c->law_invariants_len);
      free_string_array_owned(c->law_boundaries, c->law_boundaries_len);
      free_string_array_owned(c->uses_primitives, c->uses_primitives_len);

      free_artifacts_io_owned(c->emits_artifacts, c->emits_artifacts_len);
      free_artifacts_io_owned(c->consumes_artifacts, c->consumes_artifacts_len);
    }
    free((void*)cache->registry.commands);
  }

  if (cache->registry.artifacts) {
    for (size_t i = 0; i < cache->registry.artifacts_len; i++) {
      yai_law_artifact_role_t* a = (yai_law_artifact_role_t*)&cache->registry.artifacts[i];
      free((void*)a->role);
      free((void*)a->schema_ref);
      free((void*)a->description);
    }
    free((void*)cache->registry.artifacts);
  }

  free((void*)cache->registry.version);
  free((void*)cache->registry.binary);

  memset(&cache->registry, 0, sizeof(cache->registry));
  cache->loaded = 0;
}

int yai_law_registry_cache_load_from_files(
    yai_law_registry_cache_t* cache,
    const char* commands_json_path,
    const char* artifacts_json_path)
{
  if (!cache || !commands_json_path || !artifacts_json_path) return EINVAL;

  yai_law_registry_cache_free(cache);
  memset(&cache->registry, 0, sizeof(cache->registry));

  yai_law_artifact_role_t* roles = NULL;
  size_t roles_len = 0;
  char* av = NULL;
  char* ab = NULL;

  int rc = load_artifacts_table(artifacts_json_path, &roles, &roles_len, &av, &ab);
  if (rc != 0) return rc;

  yai_law_command_t* cmds = NULL;
  size_t cmds_len = 0;
  char* cv = NULL;
  char* cb = NULL;

  rc = load_commands_table(commands_json_path, &cmds, &cmds_len, &cv, &cb);
  if (rc != 0) {
    free(av); free(ab);
    free(roles);
    return rc;
  }

  cache->registry.version = cv ? cv : av;
  cache->registry.binary  = cb ? cb : ab;

  cache->registry.commands = cmds;
  cache->registry.commands_len = cmds_len;

  cache->registry.artifacts = roles;
  cache->registry.artifacts_len = roles_len;

  cache->loaded = 1;
  return 0;
}

int yai_law_registry_cache_load(yai_law_registry_cache_t* cache) {
  if (!cache) return 1;
  if (cache->loaded) return 0;

  yai_law_paths_t p;
  int rc = yai_law_paths_init(&p, NULL);
  if (rc != 0) return rc;

  const char* commands = yai_law_registry_commands(&p);
  const char* artifacts = yai_law_registry_artifacts(&p);

  rc = yai_law_registry_cache_load_from_files(cache, commands, artifacts);
  yai_law_paths_free(&p);
  return rc;
}

const yai_law_registry_t* yai_law_registry_cache_get(const yai_law_registry_cache_t* cache) {
  if (!cache || !cache->loaded) return NULL;
  return &cache->registry;
}
