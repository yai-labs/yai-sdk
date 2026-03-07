# Integration Examples

The SDK ships with self-contained examples under `examples/`:

- `01_basic_connection.c`: open client, configure logging, execute ping
- `02_workspace_context.c`: set/get/resolve current workspace binding
- `03_custom_control_call.c`: send explicit control call JSON

Build all examples:

```bash
make examples
```

Binaries are generated under `dist/bin/`.
