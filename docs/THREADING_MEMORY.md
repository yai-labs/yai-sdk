# Threading and Memory Semantics

## Thread-safety

- `yai_sdk_client_t` is an opaque handle and is **not safe for concurrent mutation**.
- Use one client instance per thread, or guard shared instances with external locks.
- Distinct client instances are independent and can be used concurrently.
- The global logging callback (`yai_sdk_set_log_handler`) should be configured during process init and then treated as immutable.

## Reentrancy

- Catalog/query operations are reentrant on independent objects.
- Functions relying on process-global environment (`YAI_REGISTRY_DIR`, context file paths) are deterministic but not transactional across competing writers.

## Memory ownership

- `yai_sdk_reply_t.exec_reply_json` is heap-allocated by SDK and must be released with `yai_sdk_reply_free`.
- Catalog resources returned by `yai_sdk_command_catalog_load` must be released with `yai_sdk_command_catalog_free`.
- Client handles opened by `yai_sdk_client_open` must be closed with `yai_sdk_client_close`.

## Leak checking

Run sanitizer validation with:

```bash
make check
```

Run optional coverage with:

```bash
make coverage
```
