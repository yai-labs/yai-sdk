# ==========================================
# yai-sdk (C) Build System
# ==========================================

CC ?= cc
ROOT_DIR := $(abspath .)

OUT_BUILD_DIR ?= $(ROOT_DIR)/build
OUT_LIB_DIR   ?= $(ROOT_DIR)/dist/lib

BUILD_DIR := $(OUT_BUILD_DIR)
LIB_DIR   := $(OUT_LIB_DIR)

LAW_DIR := $(ROOT_DIR)/deps/yai-law
LAW_INC_PROTOCOL := $(LAW_DIR)/contracts/protocol/include
LAW_INC_VAULT    := $(LAW_DIR)/contracts/vault/include
LAW_INC_RUNTIME  := $(LAW_DIR)/contracts/protocol/runtime/include

CFLAGS ?= -Wall -Wextra -O2 -std=c11 -MMD -MP
CFLAGS += -I$(ROOT_DIR)/include
CFLAGS += -I$(LAW_INC_PROTOCOL) -I$(LAW_INC_VAULT) -I$(LAW_INC_RUNTIME)
CFLAGS += -I$(ROOT_DIR)/third_party/cjson

# Expose the SDK repo root to C code (used to resolve deps/yai-law reliably)
CFLAGS += -DYAI_SDK_ROOT=\"$(ROOT_DIR)\"
CFLAGS += -D_POSIX_C_SOURCE=200809L

AR ?= ar
ARFLAGS ?= rcs

# ------------------------------------------
# Packaging / Install (enterprise)
# ------------------------------------------

PREFIX      ?= /usr/local
DESTDIR     ?=

includedir  ?= $(PREFIX)/include
libdir      ?= $(PREFIX)/lib
pkgconfigdir?= $(libdir)/pkgconfig
datadir     ?= $(PREFIX)/share
sdkdatadir  ?= $(datadir)/yai-sdk

INSTALL     ?= install

VERSION_FILE := $(ROOT_DIR)/VERSION

LIB := $(LIB_DIR)/libyai_sdk.a

PC_IN  := packaging/yai-sdk.pc.in
PC_OUT := $(BUILD_DIR)/yai-sdk.pc

SRCS := \
  src/rpc/rpc_client.c \
  src/platform/paths.c \
  src/registry/registry.c \
  src/registry/registry_help.c \
  src/registry/registry_paths.c \
  src/registry/registry_cache.c \
  src/registry/registry_load.c \
  src/registry/registry_query.c \
  src/registry/registry_validate.c \
  src/ops/ops_dispatch.c \
  src/ops/ops_dispatch_gen.c \
  third_party/cjson/cJSON.c \
  src/ops/executor.c \

OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

TEST_BIN := $(BUILD_DIR)/tests/sdk_smoke

.PHONY: all clean dirs test pc install uninstall

all: dirs $(LIB)

dirs:
	@mkdir -p $(BUILD_DIR) $(LIB_DIR) $(BUILD_DIR)/tests

$(LIB): $(OBJS)
	@echo "[AR] $@"
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(BUILD_DIR)/%.o: %.c | dirs
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BIN)
	@echo "[TEST] $(TEST_BIN)"
	@$(TEST_BIN)

$(TEST_BIN): tests/sdk_smoke.c $(LIB) | dirs
	@$(CC) $(CFLAGS) $< -o $@ $(LIB)

# ------------------------------------------
# pkg-config
# ------------------------------------------

pc: $(PC_OUT)

$(PC_OUT): $(PC_IN) $(VERSION_FILE) | dirs
	@mkdir -p $(dir $@)
	@echo "[PC] $@"
	@ver=$$(cat $(VERSION_FILE)); \
	sed \
	  -e "s|@prefix@|$(PREFIX)|g" \
	  -e "s|@includedir@|$(includedir)|g" \
	  -e "s|@libdir@|$(libdir)|g" \
	  -e "s|@version@|$$ver|g" \
	  $(PC_IN) > $@

# ------------------------------------------
# install / uninstall (DESTDIR-safe)
# ------------------------------------------

install: $(LIB) pc
	@echo "[INSTALL] prefix=$(PREFIX) destdir=$(DESTDIR)"
	@$(INSTALL) -d $(DESTDIR)$(includedir)/yai_sdk
	@$(INSTALL) -d $(DESTDIR)$(libdir)
	@$(INSTALL) -d $(DESTDIR)$(pkgconfigdir)
	@$(INSTALL) -d $(DESTDIR)$(sdkdatadir)

	@# headers (preserve subdirs)
	@cp -R $(ROOT_DIR)/include/yai_sdk/* $(DESTDIR)$(includedir)/yai_sdk/

	@# library
	@$(INSTALL) -m 0644 $(LIB) $(DESTDIR)$(libdir)/

	@# pkg-config
	@$(INSTALL) -m 0644 $(PC_OUT) $(DESTDIR)$(pkgconfigdir)/yai-sdk.pc

	@# version stamp
	@$(INSTALL) -m 0644 $(VERSION_FILE) $(DESTDIR)$(sdkdatadir)/VERSION

uninstall:
	@echo "[UNINSTALL] prefix=$(PREFIX) destdir=$(DESTDIR)"
	@rm -f $(DESTDIR)$(libdir)/libyai_sdk.a
	@rm -f $(DESTDIR)$(pkgconfigdir)/yai-sdk.pc
	@rm -f $(DESTDIR)$(sdkdatadir)/VERSION
	@rm -rf $(DESTDIR)$(includedir)/yai_sdk

clean:
	@rm -rf $(BUILD_DIR) $(LIB_DIR)

-include $(DEPS)