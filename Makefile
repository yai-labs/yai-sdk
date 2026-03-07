# ==========================================
# yai-sdk (C) Build System
# ==========================================

CC ?= cc
AR ?= ar
ARFLAGS ?= rcs
ROOT_DIR := $(abspath .)

OUT_BUILD_DIR ?= $(ROOT_DIR)/build
OUT_LIB_DIR   ?= $(ROOT_DIR)/dist/lib
OUT_BIN_DIR   ?= $(ROOT_DIR)/dist/bin
BUILD_DIR := $(OUT_BUILD_DIR)
LIB_DIR   := $(OUT_LIB_DIR)
BIN_DIR   := $(OUT_BIN_DIR)
DOCS_DIR  ?= $(ROOT_DIR)/dist/docs

PREFIX ?= /usr/local
INSTALL_LIB_DIR ?= $(PREFIX)/lib
INSTALL_INC_DIR ?= $(PREFIX)/include

# --- Resolve yai-law path ---
YAI_LAW_ROOT ?= $(shell ./tools/sh/resolve_law.sh)
ifeq ($(YAI_LAW_ROOT),)
  $(error ERROR: Unable to find yai-law. Configure resolve_law.sh or set YAI_LAW_ROOT)
endif

LAW_INC_PROTOCOL := $(YAI_LAW_ROOT)/contracts/protocol/include
LAW_INC_VAULT    := $(YAI_LAW_ROOT)/contracts/vault/include
LAW_INC_RUNTIME  := $(YAI_LAW_ROOT)/contracts/protocol/runtime/include

# Compiler and linker flags
CFLAGS ?= -Wall -Wextra -O2 -std=c11 -MMD -MP
CFLAGS += -I$(ROOT_DIR)/include -I$(ROOT_DIR)/third_party/cjson
CFLAGS += -I$(LAW_INC_PROTOCOL) -I$(LAW_INC_VAULT) -I$(LAW_INC_RUNTIME)
CFLAGS += -DYAI_LAW_ROOT='"$(YAI_LAW_ROOT)"' -D_POSIX_C_SOURCE=200809L
CFLAGS += -DYAI_SDK_VERSION_STR='"$(shell cat VERSION)"'
CFLAGS += -fPIC
LDFLAGS ?=
EXTRA_CFLAGS ?=

SDK_LIB      := $(LIB_DIR)/libyai_sdk.a
PROTOCOL_LIB := $(LIB_DIR)/libruntime_protocol.a
SDK_SO       := $(LIB_DIR)/libyai_sdk.so
PROTOCOL_SO  := $(LIB_DIR)/libruntime_protocol.so

SRCS_PROTOCOL := \
  src/rpc/rpc_client.c \
  third_party/cjson/cJSON.c

SRCS_SDK := \
  src/sdk_public.c \
  src/platform/paths.c \
  src/platform/context.c \
  src/platform/log.c \
  src/client/client.c \
  src/reply/reply_builder.c \
  src/reply/reply_json.c \
  src/protocol/reply_map.c \
  src/registry/registry.c \
  src/catalog/catalog.c \
  src/registry/registry_help.c \
  src/registry/registry_paths.c \
  src/registry/registry_cache.c \
  src/registry/registry_load.c \
  src/registry/registry_query.c \
  src/registry/registry_validate.c

OBJS_PROTOCOL := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS_PROTOCOL))
OBJS_SDK      := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS_SDK))
ALL_OBJS      := $(OBJS_PROTOCOL) $(OBJS_SDK)
DEPS          := $(ALL_OBJS:.o=.d)

TEST_BIN := $(BUILD_DIR)/tests/sdk_smoke
CATALOG_TEST_BIN := $(BUILD_DIR)/tests/catalog_smoke
HELP_INDEX_TEST_BIN := $(BUILD_DIR)/tests/help_index_smoke
WORKSPACE_TEST_BIN := $(BUILD_DIR)/tests/workspace_smoke
PUBLIC_SURFACE_TEST_BIN := $(BUILD_DIR)/tests/public_surface_smoke
EXAMPLE_BASIC_BIN := $(BIN_DIR)/example_01_basic_connection
EXAMPLE_CONTEXT_BIN := $(BIN_DIR)/example_02_workspace_context
EXAMPLE_CUSTOM_BIN := $(BIN_DIR)/example_03_custom_control_call

.PHONY: all clean dirs test info libs api-boundary-check check coverage docs docs-clean install examples

all: libs

libs: $(SDK_LIB) $(PROTOCOL_LIB) $(SDK_SO) $(PROTOCOL_SO)

info:
	@echo "YAI_SDK_ROOT: $(ROOT_DIR)"
	@echo "YAI_LAW_ROOT: $(YAI_LAW_ROOT)"
	@echo "Protocol Objects: $(OBJS_PROTOCOL)"

dirs:
	@mkdir -p $(BUILD_DIR) $(LIB_DIR) $(BUILD_DIR)/tests $(BIN_DIR)

$(PROTOCOL_LIB): $(OBJS_PROTOCOL)
	@echo "[AR] $@"
	@$(AR) $(ARFLAGS) $@ $^

$(SDK_LIB): $(ALL_OBJS)
	@echo "[AR] $@"
	@$(AR) $(ARFLAGS) $@ $^

$(SDK_SO): $(ALL_OBJS)
	@echo "[LD.SO] $@"
	@$(CC) -shared -Wl,-soname,libyai_sdk.so -o $@ $^ $(LDFLAGS)

$(PROTOCOL_SO): $(OBJS_PROTOCOL)
	@echo "[LD.SO] $@"
	@$(CC) -shared -Wl,-soname,libruntime_protocol.so -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c | dirs
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(DOCS_DIR) $(ROOT_DIR)/coverage

api-boundary-check:
	@tools/sh/check_api_boundaries.sh

test: api-boundary-check $(TEST_BIN) $(CATALOG_TEST_BIN) $(HELP_INDEX_TEST_BIN) $(WORKSPACE_TEST_BIN) $(PUBLIC_SURFACE_TEST_BIN)
	@$(MAKE) api-boundary-check
	@echo "[RUN] $(TEST_BIN)"
	@$(TEST_BIN)
	@echo "[RUN] $(CATALOG_TEST_BIN)"
	@$(CATALOG_TEST_BIN)
	@echo "[RUN] $(HELP_INDEX_TEST_BIN)"
	@$(HELP_INDEX_TEST_BIN)
	@echo "[RUN] $(WORKSPACE_TEST_BIN)"
	@$(WORKSPACE_TEST_BIN)
	@echo "[RUN] $(PUBLIC_SURFACE_TEST_BIN)"
	@$(PUBLIC_SURFACE_TEST_BIN)

check:
	@$(MAKE) clean
	@$(MAKE) EXTRA_CFLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" LDFLAGS="$(LDFLAGS) -fsanitize=address,undefined" test

coverage:
	@$(MAKE) clean
	@$(MAKE) EXTRA_CFLAGS="-O0 -g --coverage" LDFLAGS="$(LDFLAGS) --coverage" test
	@mkdir -p coverage
	@if command -v gcovr >/dev/null 2>&1; then \
	  gcovr -r . --html --html-details -o coverage/index.html; \
	  echo "[COVERAGE] coverage/index.html"; \
	else \
	  echo "[COVERAGE] gcovr not found; raw .gcda/.gcno files generated under build/"; \
	fi

docs:
	@mkdir -p $(DOCS_DIR)
	@doxygen Doxyfile
	@echo "[DOCS] $(DOCS_DIR)/html/index.html"

docs-clean:
	@rm -rf $(DOCS_DIR)

examples: $(EXAMPLE_BASIC_BIN) $(EXAMPLE_CONTEXT_BIN) $(EXAMPLE_CUSTOM_BIN)
	@echo "[EXAMPLES] built under $(BIN_DIR)"

install: libs
	@install -d "$(DESTDIR)$(INSTALL_LIB_DIR)" "$(DESTDIR)$(INSTALL_INC_DIR)/yai_sdk"
	@install -m 0644 $(SDK_LIB) "$(DESTDIR)$(INSTALL_LIB_DIR)/"
	@install -m 0644 $(PROTOCOL_LIB) "$(DESTDIR)$(INSTALL_LIB_DIR)/"
	@install -m 0755 $(SDK_SO) "$(DESTDIR)$(INSTALL_LIB_DIR)/"
	@install -m 0755 $(PROTOCOL_SO) "$(DESTDIR)$(INSTALL_LIB_DIR)/"
	@cp -R include/yai_sdk "$(DESTDIR)$(INSTALL_INC_DIR)/"
	@echo "[INSTALL] libs -> $(DESTDIR)$(INSTALL_LIB_DIR), headers -> $(DESTDIR)$(INSTALL_INC_DIR)/yai_sdk"

$(TEST_BIN): tests/sdk_smoke.c $(SDK_LIB) | dirs
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -L$(LIB_DIR) -lyai_sdk $(LDFLAGS) -Wl,-rpath,$(LIB_DIR) -o $@

$(CATALOG_TEST_BIN): tests/catalog_smoke.c $(SDK_LIB) | dirs
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -L$(LIB_DIR) -lyai_sdk $(LDFLAGS) -Wl,-rpath,$(LIB_DIR) -o $@

$(HELP_INDEX_TEST_BIN): tests/help_index_smoke.c $(SDK_LIB) | dirs
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -L$(LIB_DIR) -lyai_sdk $(LDFLAGS) -Wl,-rpath,$(LIB_DIR) -o $@

$(WORKSPACE_TEST_BIN): tests/workspace_smoke.c $(SDK_LIB) | dirs
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -L$(LIB_DIR) -lyai_sdk $(LDFLAGS) -Wl,-rpath,$(LIB_DIR) -o $@

$(PUBLIC_SURFACE_TEST_BIN): tests/public_surface_smoke.c $(SDK_LIB) | dirs
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -L$(LIB_DIR) -lyai_sdk $(LDFLAGS) -Wl,-rpath,$(LIB_DIR) -o $@

$(EXAMPLE_BASIC_BIN): examples/01_basic_connection.c $(SDK_LIB) | dirs
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -L$(LIB_DIR) -lyai_sdk $(LDFLAGS) -Wl,-rpath,$(LIB_DIR) -o $@

$(EXAMPLE_CONTEXT_BIN): examples/02_workspace_context.c $(SDK_LIB) | dirs
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -L$(LIB_DIR) -lyai_sdk $(LDFLAGS) -Wl,-rpath,$(LIB_DIR) -o $@

$(EXAMPLE_CUSTOM_BIN): examples/03_custom_control_call.c $(SDK_LIB) | dirs
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -L$(LIB_DIR) -lyai_sdk $(LDFLAGS) -Wl,-rpath,$(LIB_DIR) -o $@

-include $(DEPS)
