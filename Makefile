# ==========================================
# yai-sdk (C) Build System - Modular Version
# ==========================================

CC ?= cc
AR ?= ar
ARFLAGS ?= rcs
ROOT_DIR := $(abspath .)

OUT_BUILD_DIR ?= $(ROOT_DIR)/build
OUT_LIB_DIR   ?= $(ROOT_DIR)/dist/lib
BUILD_DIR := $(OUT_BUILD_DIR)
LIB_DIR   := $(OUT_LIB_DIR)

# --- Risoluzione Law ---
YAI_LAW_ROOT ?= $(shell ./tools/sh/resolve_law.sh)
ifeq ($(YAI_LAW_ROOT),)
  $(error ERRORE: Impossibile trovare yai-law. Verifica resolve_law.sh o imposta YAI_LAW_ROOT)
endif

# Path Contratti
LAW_INC_PROTOCOL := $(YAI_LAW_ROOT)/contracts/protocol/include
LAW_INC_VAULT    := $(YAI_LAW_ROOT)/contracts/vault/include
LAW_INC_RUNTIME  := $(YAI_LAW_ROOT)/contracts/protocol/runtime/include

# Compiler Flags
CFLAGS ?= -Wall -Wextra -O2 -std=c11 -MMD -MP
CFLAGS += -I$(ROOT_DIR)/include -I$(ROOT_DIR)/third_party/cjson
CFLAGS += -I$(LAW_INC_PROTOCOL) -I$(LAW_INC_VAULT) -I$(LAW_INC_RUNTIME)
CFLAGS += -DYAI_LAW_ROOT=\"$(YAI_LAW_ROOT)\" -D_POSIX_C_SOURCE=200809L

# --- DEFINIZIONE ARTEFATTI (FIX #2) ---
SDK_LIB      := $(LIB_DIR)/libyai_sdk.a
PROTOCOL_LIB := $(LIB_DIR)/libruntime_protocol.a

# Sorgenti del solo Protocollo (Internal Core)
SRCS_PROTOCOL := \
  src/rpc/rpc_client.c \
  src/ops/ops_dispatch_gen.c \
  third_party/cjson/cJSON.c

# Sorgenti dell'SDK (High Level API)
SRCS_SDK := \
  src/platform/paths.c \
  src/registry/registry.c \
  src/registry/registry_help.c \
  src/registry/registry_paths.c \
  src/registry/registry_cache.c \
  src/registry/registry_load.c \
  src/registry/registry_query.c \
  src/registry/registry_validate.c \
  src/ops/ops_dispatch.c \
  src/ops/executor.c

OBJS_PROTOCOL := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS_PROTOCOL))
OBJS_SDK      := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS_SDK))
ALL_OBJS      := $(OBJS_PROTOCOL) $(OBJS_SDK)
DEPS          := $(ALL_OBJS:.o=.d)

# --- TARGETS ---
.PHONY: all clean dirs test info libs

all: libs

libs: $(SDK_LIB) $(PROTOCOL_LIB)

info:
	@echo "YAI_SDK_ROOT: $(ROOT_DIR)"
	@echo "YAI_LAW_ROOT: $(YAI_LAW_ROOT)"
	@echo "Protocol Objects: $(OBJS_PROTOCOL)"

dirs:
	@mkdir -p $(BUILD_DIR) $(LIB_DIR) $(BUILD_DIR)/tests

# Libreria interna del Protocollo (Leggera)
$(PROTOCOL_LIB): $(OBJS_PROTOCOL)
	@echo "[AR] $@"
	@$(AR) $(ARFLAGS) $@ $^

# Libreria SDK completa (Include tutto)
$(SDK_LIB): $(ALL_OBJS)
	@echo "[AR] $@"
	@$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)/%.o: %.c | dirs
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR) $(LIB_DIR)

-include $(DEPS)