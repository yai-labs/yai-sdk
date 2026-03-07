# DX and Reliability Checklist (SDK)

## Implemented in this wave

- Integration examples under `examples/`
- Shared library build (`libyai_sdk.so`) and `make install`
- `make check` sanitizer target (ASAN + UBSAN)
- `make coverage` gcovr report generation
- Doxygen config (`Doxyfile`) + `make docs`
- Logging callback API (`yai_sdk/log.h`) with correlation-id support in client opts
- Python ctypes wrapper skeleton under `wrappers/python/`
- CI matrix covering Ubuntu + Debian + Alpine and ARM runner

## Pending / future hardening

- Dedicated Valgrind target in CI for long-running RPC traces
- Fuzzing target for JSON parsing paths (`reply_json`, `reply_map`)
- Additional wrappers (Go/cgo)
