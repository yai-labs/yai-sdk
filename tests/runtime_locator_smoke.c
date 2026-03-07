/* SPDX-License-Identifier: Apache-2.0 */

#include <yai_sdk/paths.h>

#include <stdio.h>

int main(void)
{
  char buf[1024];
  yai_runtime_deploy_mode_t mode = YAI_RUNTIME_DEPLOY_UNKNOWN;

  if (yai_path_runtime_home(buf, sizeof(buf)) != 0 || !buf[0]) {
    fprintf(stderr, "runtime_home resolution failed\n");
    return 1;
  }

  if (yai_path_detect_deploy_mode(&mode) != 0) {
    fprintf(stderr, "deploy_mode resolution failed\n");
    return 1;
  }

  if (yai_path_root_sock(buf, sizeof(buf)) != 0 || !buf[0]) {
    fprintf(stderr, "root socket resolution failed\n");
    return 1;
  }

  /* Boot binary may be unresolved on minimal test hosts; ensure deterministic API behavior. */
  (void)yai_path_boot_bin(buf, sizeof(buf));
  (void)yai_path_root_bin(buf, sizeof(buf));
  (void)yai_path_kernel_bin(buf, sizeof(buf));
  (void)yai_path_engine_bin(buf, sizeof(buf));

  return 0;
}
