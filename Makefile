PROJECT     = rplatform
DEVICE      = stm32f103c8t6
OPENCM3_DIR = lib/libopencm3

CFLAGS  = -std=c99 -Os -g -flto
CFLAGS += -funsigned-char -fomit-frame-pointer
CFLAGS += -Wall -Wextra -Werror

LDFLAGS = -static -nostartfiles -Os -g -flto
LIBNAME = opencm3_stm32f1

# Dynamically generate a file containing the current git commit hash.
VERFILE = src/version.c
VERSION = $(shell git rev-parse --short=6 HEAD)

SRCS  = $(filter-out $(VERFILE),$(wildcard src/*.c))
SRCS += $(VERFILE)
OBJS  = $(SRCS:.c=.o)

# Default to silent mode, run 'make V=1' for a verbose build.
ifneq ($(V),1)
Q := @
MAKEFLAGS += --no-print-directory
endif

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/gcc-config.mk

.PHONY: all clean debug flash

all: $(PROJECT).bin

$(LIBDEPS):
	$(Q)$(MAKE) -C $(OPENCM3_DIR) TARGETS=stm32/f1 CFLAGS=-flto

$(VERFILE):
	@printf "  GENVER  $@\n";
	$(Q)echo "const char version[] = \"$(VERSION)\";" > $@

flash: $(PROJECT).bin
	@printf "  FLASH   $^\n";
	$(Q)openocd \
	  -f /usr/share/openocd/scripts/interface/stlink.cfg \
	  -c "transport select hla_swd" \
	  -f /usr/share/openocd/scripts/target/stm32f1x.cfg \
	  -c "init" \
	  -c "reset halt" \
	  -c "stm32f1x unlock 0" \
	  -c "flash write_image erase unlock $(PROJECT).bin 0x08000000" \
	  -c "reset run" \
	  -c "shutdown"

debug: $(PROJECT).elf
	xterm -e 'openocd \
	  -f /usr/share/openocd/scripts/interface/stlink.cfg \
	  -c "transport select hla_swd" \
	  -f /usr/share/openocd/scripts/target/stm32f1x.cfg \
	  -c "init" \
	  -c "halt"' &
	$(GDB) --eval-command="target remote localhost:3333" $(PROJECT).elf

clean:
	$(Q)$(MAKE) -C $(OPENCM3_DIR) clean
	$(Q)$(RM) $(OBJS) $(VERFILE) $(LDSCRIPT)
	$(Q)$(RM) $(PROJECT).bin $(PROJECT).elf

include $(OPENCM3_DIR)/mk/genlink-rules.mk
include $(OPENCM3_DIR)/mk/gcc-rules.mk
