# Standalone GoldHEN system payload; depends on PS4 payload SDK v0.9.
# Upstream virtual-device library is included at build time (GPL-3.0), not
# installed as Control4Free on the console.
ifdef PS4_PAYLOAD_SDK
include $(PS4_PAYLOAD_SDK)/toolchain/orbis.mk
else
$(error PS4_PAYLOAD_SDK is undefined; see .github/workflows/build.yml)
endif
OUT := build/slisk-x360-directusb.elf
CC_CFLAGS := -std=gnu11 -O2 -Wall -Wextra -Wno-unused-parameter -I./src -Iupstream/include -DC4F_VERSION='"slisk-0.9.1"'
SRCS := src/main.c src/slisk_usb.c src/xw_core.c upstream/src/log.c upstream/src/vda.c upstream/src/klog_line.c upstream/src/feedback.c
OBJS := $(patsubst %.c,build/%.o,$(SRCS))
CFLAGS += $(CC_CFLAGS)
LDFLAGS += -lScePad -lSceUserService -ldl -lpthread
.PHONY: all clean
all: $(OUT)
$(OUT): $(OBJS)
	@mkdir -p $(@D)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf build
