/* SPDX-License-Identifier: Apache-2.0 */

#include "yai_sdk/registry/command_catalog.h"
#include "yai_sdk/registry/registry_registry.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct group_counter {
  char group[32];
  size_t count;
  size_t write_cursor;
} group_counter_t;

static int cmp_group(const void *a, const void *b)
{
  const yai_sdk_command_group_t *ga = (const yai_sdk_command_group_t *)a;
  const yai_sdk_command_group_t *gb = (const yai_sdk_command_group_t *)b;
  return strcmp(ga->group, gb->group);
}

static int cmp_command(const void *a, const void *b)
{
  const yai_sdk_command_ref_t *ca = (const yai_sdk_command_ref_t *)a;
  const yai_sdk_command_ref_t *cb = (const yai_sdk_command_ref_t *)b;
  return strcmp(ca->name, cb->name);
}

static int find_counter_slot(group_counter_t *counters, size_t len, const char *group)
{
  if (!counters || !group || !group[0]) return -1;
  for (size_t i = 0; i < len; i++) {
    if (strcmp(counters[i].group, group) == 0) return (int)i;
  }
  return -1;
}

static void free_partial(yai_sdk_command_catalog_t *out)
{
  if (!out || !out->groups) return;
  for (size_t i = 0; i < out->group_count; i++) {
    free(out->groups[i].commands);
    out->groups[i].commands = NULL;
    out->groups[i].command_count = 0;
  }
  free(out->groups);
  out->groups = NULL;
  out->group_count = 0;
}

int yai_sdk_command_catalog_load(yai_sdk_command_catalog_t *out)
{
  const yai_law_registry_t *reg;
  group_counter_t *counters = NULL;
  size_t counter_len = 0;
  if (!out) return 1;
  memset(out, 0, sizeof(*out));

  if (yai_law_registry_init() != 0) return 2;
  reg = yai_law_registry();
  if (!reg || !reg->commands || reg->commands_len == 0) return 3;

  counters = (group_counter_t *)calloc(reg->commands_len, sizeof(*counters));
  if (!counters) return 4;

  for (size_t i = 0; i < reg->commands_len; i++) {
    const yai_law_command_t *c = &reg->commands[i];
    int slot;
    if (!c || !c->group || !c->group[0] || !c->name || !c->id) continue;
    slot = find_counter_slot(counters, counter_len, c->group);
    if (slot < 0) {
      slot = (int)counter_len++;
      snprintf(counters[(size_t)slot].group, sizeof(counters[(size_t)slot].group), "%s", c->group);
      counters[(size_t)slot].count = 0;
      counters[(size_t)slot].write_cursor = 0;
    }
    counters[(size_t)slot].count++;
  }

  if (counter_len == 0) {
    free(counters);
    return 5;
  }

  out->groups = (yai_sdk_command_group_t *)calloc(counter_len, sizeof(*out->groups));
  if (!out->groups) {
    free(counters);
    return 6;
  }
  out->group_count = counter_len;

  for (size_t i = 0; i < counter_len; i++) {
    snprintf(out->groups[i].group, sizeof(out->groups[i].group), "%s", counters[i].group);
    out->groups[i].command_count = counters[i].count;
    out->groups[i].commands = (yai_sdk_command_ref_t *)calloc(counters[i].count, sizeof(yai_sdk_command_ref_t));
    if (!out->groups[i].commands) {
      free(counters);
      free_partial(out);
      return 7;
    }
  }

  for (size_t i = 0; i < reg->commands_len; i++) {
    const yai_law_command_t *c = &reg->commands[i];
    int cslot;
    size_t w;
    yai_sdk_command_ref_t *ref;
    if (!c || !c->group || !c->group[0] || !c->name || !c->id) continue;
    cslot = find_counter_slot(counters, counter_len, c->group);
    if (cslot < 0) continue;
    w = counters[(size_t)cslot].write_cursor++;
    if (w >= out->groups[(size_t)cslot].command_count) {
      free(counters);
      free_partial(out);
      return 8;
    }
    ref = &out->groups[(size_t)cslot].commands[w];
    snprintf(ref->group, sizeof(ref->group), "%s", c->group);
    snprintf(ref->name, sizeof(ref->name), "%s", c->name);
    snprintf(ref->id, sizeof(ref->id), "%s", c->id);
    if (c->summary && c->summary[0]) {
      snprintf(ref->summary, sizeof(ref->summary), "%s", c->summary);
    } else {
      snprintf(ref->summary, sizeof(ref->summary), "No description.");
    }
  }

  for (size_t i = 0; i < out->group_count; i++) {
    qsort(out->groups[i].commands, out->groups[i].command_count, sizeof(out->groups[i].commands[0]), cmp_command);
  }
  qsort(out->groups, out->group_count, sizeof(out->groups[0]), cmp_group);

  free(counters);
  return 0;
}

void yai_sdk_command_catalog_free(yai_sdk_command_catalog_t *cat)
{
  if (!cat) return;
  free_partial(cat);
}

const yai_sdk_command_group_t *yai_sdk_command_catalog_find_group(
    const yai_sdk_command_catalog_t *cat,
    const char *group)
{
  if (!cat || !group || !group[0]) return NULL;
  for (size_t i = 0; i < cat->group_count; i++) {
    if (strcmp(cat->groups[i].group, group) == 0) return &cat->groups[i];
  }
  return NULL;
}

const yai_sdk_command_ref_t *yai_sdk_command_catalog_find_command(
    const yai_sdk_command_catalog_t *cat,
    const char *group,
    const char *name)
{
  const yai_sdk_command_group_t *g = yai_sdk_command_catalog_find_group(cat, group);
  if (!g || !name || !name[0]) return NULL;
  for (size_t i = 0; i < g->command_count; i++) {
    if (strcmp(g->commands[i].name, name) == 0) return &g->commands[i];
  }
  return NULL;
}

const yai_sdk_command_ref_t *yai_sdk_command_catalog_find_by_id(
    const yai_sdk_command_catalog_t *cat,
    const char *canonical_id)
{
  if (!cat || !canonical_id || !canonical_id[0]) return NULL;
  for (size_t i = 0; i < cat->group_count; i++) {
    const yai_sdk_command_group_t *g = &cat->groups[i];
    for (size_t j = 0; j < g->command_count; j++) {
      if (strcmp(g->commands[j].id, canonical_id) == 0) return &g->commands[j];
    }
  }
  return NULL;
}
