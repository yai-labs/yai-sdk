// src/registry/law.c
#include "yai_sdk/registry/registry.h"
#include "yai_sdk/registry/registry_help.h"
#include "yai_sdk/registry/registry_registry.h"

#include <stdio.h>
#include <string.h>

int yai_cmd_law(int argc, char** argv) {
  // usage:
  //   yai law help [token]
  //   yai law validate
  //   yai law list <group>
  if (argc < 2) {
    return yai_law_help_print_global();
  }

  const char* sub = argv[1];

  if (strcmp(sub, "help") == 0) {
    const char* tok = (argc >= 3) ? argv[2] : NULL;
    return yai_law_help_print_any(tok);
  }

  if (strcmp(sub, "validate") == 0) {
    if (yai_law_registry_init() != 0) return 2;
    const yai_law_registry_t* r = yai_law_registry();
    if (!r) return 3;
    // validation happens inside init pipeline later; for now just ensure registry loads
    printf("OK: registry loaded (commands=%zu artifacts=%zu)\n", r->commands_len, r->artifacts_len);
    return 0;
  }

  if (strcmp(sub, "list") == 0 && argc >= 3) {
    return yai_law_help_print_group(argv[2]);
  }

  // fallback: treat as token
  return yai_law_help_print_any(sub);
}