.PHONY: clean

all: bootloader radio

clean:
	@rm -rf $(BUILD_DIR)

Q ?= @
MODEL = medium
AS = sdas8051
CC = sdcc

GIT_REV = $(shell git describe --always --dirty)
GIT_REV_HEX = 0x$(shell git rev-parse --short=8 HEAD)

HWID ?= 0x0000

BUILD_DIR := build
BOARD_DIR := board

CFLAGS := -DCUSTOM_BOARD_INIT -Iboard
SRCS := board/board.c

### Common 

ALL_HEXS += 
# ROOT_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
# COMMON_DIR = $(ROOT_DIR)common
COMMON_DIR = common
COMMON_SRCS = $(COMMON_DIR)/board_defaults.c \
	$(COMMON_DIR)/clock.c \
	$(COMMON_DIR)/crc16.c \
	$(COMMON_DIR)/dma.c \
	$(COMMON_DIR)/input_handlers.c \
	$(COMMON_DIR)/interrupts.c \
	$(COMMON_DIR)/stringx.c \
	$(COMMON_DIR)/radio.c \
	$(COMMON_DIR)/uart0.c \
	$(COMMON_DIR)/uart1.c \
	$(COMMON_DIR)/watchdog.c
COMMON_HEADERS = $(COMMON_DIR)/*.h

### Bootloader

BOOTLOADER_SECTION_START = 0x0000
BOOTLOADER_SECTION_SIZE = 0x8000

BOOTLOADER_DIR := bootloader

BOOTLOADER_ASFLAGS = -plogsgff
BOOTLOADER_CFLAGS = --model-$(MODEL) -I$(COMMON_DIR) -I$(BOOTLOADER_DIR) \
	-DBOOTLOADER -DKEEP_CODE_SMALL=1 -DHWID=$(HWID) -DGIT_REV=$(GIT_REV) \
	-DUART0_ENABLED=0
BOOTLOADER_LDFLAGS = --model-$(MODEL) --out-fmt-ihx \
	--xram-loc 0xf000 --xram-size 0x0d00 --iram-size 0x100 \
	--code-loc $(BOOTLOADER_SECTION_START) \
	--code-size $(BOOTLOADER_SECTION_SIZE) \
	-Wl-bAPP_UPDATER=0x7000

BOOTLOADER_SRCS = $(BOOTLOADER_DIR)/main.c \
	$(BOOTLOADER_DIR)/commands.c \
	$(BOOTLOADER_DIR)/flash_storage.c \
	$(BOOTLOADER_DIR)/updater.c \
	$(BOOTLOADER_DIR)/flash.c \
	$(BOOTLOADER_DIR)/signature.c

BOOTLOADER_ASMS = $(BOOTLOADER_DIR)/start.asm

BOOTLOADER_HEADERS = $(BOOTLOADER_DIR)/*.h

# Build bootloader hex file
bootloader: $(BUILD_DIR)/bootloader.hex

# Object Files used to build the radio
# flash_trigger (part of COMMON_ASMS) must come first to preserve
# even code alignment.
BL_OBJS = $(BOOTLOADER_ASMS:%.asm=$(BUILD_DIR)/%.bl.asm.rel) \
	$(ASMS:%.asm=$(BUILD_DIR)/%.bl.asm.rel) \
	$(COMMON_SRCS:%.c=$(BUILD_DIR)/%.bl.rel) \
	$(BOOTLOADER_SRCS:%.c=$(BUILD_DIR)/%.bl.rel) \
	$(SRCS:%.c=$(BUILD_DIR)/%.bl.rel)

# Rule to make the radio firmware hex file
$(BUILD_DIR)/bootloader.hex: $(BL_OBJS)
	$(Q)$(CC) $(BOOTLOADER_LDFLAGS) $(CFLAGS) $(BOOTLOADER_CFLAGS) $^ -o $@
	@echo
	@echo Memory usage summary for $@:
	$(Q)tail -n5 $(BUILD_DIR)/bootloader.mem
	$(Q)grep ^Stack $(BUILD_DIR)/bootloader.mem
	@echo

# Assemble bootloader objects
$(BUILD_DIR)/$(BOOTLOADER_DIR)%.bl.asm.rel: $(BOOTLOADER_DIR)%.asm
	@mkdir -p $(@D)
	$(Q)$(AS) $(BOOTLOADER_ASFLAGS) $(ASFLAGS) $@ $^

# Compile bootloader objects
$(BUILD_DIR)/$(BOOTLOADER_DIR)%.bl.rel: $(BOOTLOADER_DIR)%.c $(BOOTLOADER_HEADERS)
	@mkdir -p $(@D)
	$(Q)$(CC) $(BOOTLOADER_CFLAGS) $(CFLAGS) -c $< -o $@

# Assemble common objects
$(BUILD_DIR)/$(COMMON_DIR)%.asm.bl.rel: $(COMMON_DIR)%.asm
	$(Q)$(AS) $(BOOTLOADER_ASFLAGS) $(ASFLAGS) $@ $^

# Compile common objects
$(BUILD_DIR)/$(COMMON_DIR)%.bl.rel: $(COMMON_DIR)%.c $(COMMON_HEADERS)
	@mkdir -p $(@D)
	$(Q)$(CC) $(BOOTLOADER_CFLAGS) $(CFLAGS) -c $< -o $@

# Assemble board objects
$(BUILD_DIR)/$(BOARD_DIR)%.asm.bl.rel: $(BOARD_DIR)%.asm
	$(Q)$(AS) $(BOOTLOADER_ASFLAGS) $(ASFLAGS) $@ $^

# Compile board objects
$(BUILD_DIR)/$(BOARD_DIR)%.bl.rel: $(BOARD_DIR)%.c $(BOARD_DIR)/*.h
	@mkdir -p $(@D)
	$(Q)$(CC) $(BOOTLOADER_CFLAGS) $(CFLAGS) -c $< -o $@

### Radio

RADIO_SECTION_START = 0x0400
# 0x7000-0x7FFF is reserved for the bootloader
# 0x0000-0x0400 is also reserved for the bootloader
# 0x6C00-0x6FFF is reserved for non-volatile storage
# 0x6BE0-0x6BFF are reserved for the signature
RADIO_SECTION_SIZE = 26592

RADIO_DIR := radio

RADIO_ASFLAGS = -plogsgff
RADIO_CFLAGS = --model-$(MODEL) -I$(COMMON_DIR) -I$(RADIO_DIR) \
	-DGIT_REV=$(GIT_REV) -DGIT_REV_HEX=$(GIT_REV_HEX) $(CFLAGS)
RADIO_LDFLAGS = --model-$(MODEL) --out-fmt-ihx \
	--xram-loc 0xf000 --xram-size 0x0d00 --iram-size 0x100 \
	--code-loc $(RADIO_SECTION_START) \
	--code-size $(RADIO_SECTION_SIZE) \
	-Wl-bFLASHTRIGSEG=0x0520

RADIO_SRCS = $(RADIO_DIR)/main.c \
	$(RADIO_DIR)/adc.c \
	$(RADIO_DIR)/commands.c \
	$(RADIO_DIR)/schedule.c \
	$(RADIO_DIR)/telemetry.c \
	$(RADIO_DIR)/timers.c \
	$(BOARD_DIR)/board.c

#flash_trigger must come first (code alignment)
RADIO_ASMS = $(RADIO_DIR)/flash_trigger.asm

RADIO_HEADERS = $(RADIO_DIR)/*.h

# Build radio hex file
radio: $(BUILD_DIR)/radio.hex

# Object Files used to build the radio
# flash_trigger (part of COMMON_ASMS) must come first to preserve
# even code alignment.
RADIO_OBJS = $(RADIO_ASMS:%.asm=$(BUILD_DIR)/%.asm.rel) \
	$(COMMON_SRCS:%.c=$(BUILD_DIR)/%.rel) \
	$(RADIO_SRCS:%.c=$(BUILD_DIR)/%.rel) \
	# $($(1)_SRCS:%.c=%.$(1).rel) \
	$($(1)_RADIO_SRCS:%.c=%.$(1).rel)

# Rule to make the radio firmware hex file
$(BUILD_DIR)/radio.hex: $(RADIO_OBJS)
	$(Q)$(CC) $(RADIO_LDFLAGS) $(RADIO_CFLAGS) $^ -o $@
	@echo
	@echo Memory usage summary for $@:
	$(Q)tail -n5 $(BUILD_DIR)/radio.mem
	$(Q)grep ^Stack $(BUILD_DIR)/radio.mem
	@echo

# Assemble radio objects
$(BUILD_DIR)/$(RADIO_DIR)%.asm.rel: $(RADIO_DIR)%.asm
	@mkdir -p $(@D)
	$(Q)$(AS) $(RADIO_ASFLAGS) $@ $^

# Compile radio objects
$(BUILD_DIR)/$(RADIO_DIR)%.rel: $(RADIO_DIR)%.c $(RADIO_HEADERS)
	@mkdir -p $(@D)
	$(Q)$(CC) $(RADIO_CFLAGS) -c $< -o $@

# Assemble common objects
$(BUILD_DIR)/$(COMMON_DIR)%.asm.rel: $(COMMON_DIR)%.asm
	@mkdir -p $(@D)
	$(Q)$(AS) $(RADIO_ASFLAGS) $@ $^

# Compile common objects
$(BUILD_DIR)/$(COMMON_DIR)%.rel: $(COMMON_DIR)%.c $(COMMON_HEADERS)
	@mkdir -p $(@D)
	$(Q)$(CC) $(RADIO_CFLAGS) -c $< -o $@

# Assemble board objects
$(BUILD_DIR)/$(BOARD_DIR)%.asm.rel: $(BOARD_DIR)%.asm
	@mkdir -p $(@D)
	$(Q)$(AS) $(RADIO_ASFLAGS) $@ $^

 # Compile board objects
$(BUILD_DIR)/$(BOARD_DIR)%.rel: $(BOARD_DIR)%.c $(BOARD_DIR)/*.h
	@mkdir -p $(@D)
	$(Q)$(CC) $(RADIO_CFLAGS) -c $< -o $@
