PROJECT     = rplatform
DEVICE      = stm32f103c8t6
OPENCM3_DIR = lib/libopencm3

CFLAGS  = -std=c99 -Os -g -flto
CFLAGS += -funsigned-char -fomit-frame-pointer
CFLAGS += -Wall -Wextra -Werror

LDFLAGS = -static -nostartfiles -Os -g -flto
LIBNAME = opencm3_stm32f1

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

# Default to silent mode, run 'make V=1' for a verbose build.
ifneq ($(V),1)
Q := @
endif

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/gcc-config.mk

.PHONY: all clean debug flash

all: $(PROJECT).bin

$(LIBDEPS):
	$(MAKE) -C $(OPENCM3_DIR) TARGETS=stm32/f1 CFLAGS=-flto

flash: $(PROJECT).bin
	openocd \
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
	$(MAKE) -C $(OPENCM3_DIR) clean
	$(RM) $(OBJS) $(LDSCRIPT)
	$(RM) $(PROJECT).bin $(PROJECT).elf

include $(OPENCM3_DIR)/mk/genlink-rules.mk
include $(OPENCM3_DIR)/mk/gcc-rules.mk
