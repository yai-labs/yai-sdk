#include <stdio.h>
#include "yai_sdk/registry/registry_registry.h"

int main(void) {
  const yai_law_command_t* c = yai_law_cmd_by_id("yai.kernel.ws");
  if (!c) {
    fprintf(stderr, "sdk_smoke: registry lookup failed\n");
    return 1;
  }
  printf("sdk_smoke: ok (%s)\n", c->id);
  return 0;
}
