# OpenLST
# Copyright (C) 2018 Planet Labs Inc.
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# This file provides a template for the bootloader
# makefile
HWID ?= 0x0000

BOOTLOADER_SECTION_START = 0x0000
BOOTLOADER_SECTION_SIZE = 0x8000

BOOTLOADER_DIR := $(dir $(lastword $(MAKEFILE_LIST)))bootloader

BOOTLOADER_ASFLAGS = -plogsgff
BOOTLOADER_CFLAGS = --model-$(MODEL) -I$(COMMON_DIR) -I$(BOOTLOADER_DIR) \
	-DBOOTLOADER -DKEEP_CODE_SMALL=1 -DHWID=$(HWID) -DGIT_REV=$(GIT_REV)
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

# Template for building each radio bootloader
define BOOTLOADER_template

 # Build bootloader hex file
 $(1)_bootloader: $(1)_bootloader.hex

 # Object Files used to build the radio
 # flash_trigger (part of COMMON_ASMS) must come first to preserve
 # even code alignment.
 $(1)_BL_OBJS = $(BOOTLOADER_ASMS:%.asm=%.$(1).bl.asm.rel) \
	$($(1)_ASMS:%.asm=%.$(1).bl.asm.rel) \
	$(COMMON_SRCS:%.c=%.$(1).bl.rel) \
	$(BOOTLOADER_SRCS:%.c=%.$(1).bl.rel) \
	$($(1)_SRCS:%.c=%.$(1).bl.rel) \
	$($(1)_BOOTLOADER_SRCS:%.c=%.$(1).rel)

 # Rule to make the radio firmware hex file
 $(1)_bootloader.hex: $$($(1)_BL_OBJS)
	$(Q)$(CC) $(BOOTLOADER_LDFLAGS) $($(1)_BOOTLOADER_LDFLAGS) $($(1)_CFLAGS) $($(1)_BOOTLOADER_CFLAGS) $(BOOTLOADER_CFLAGS) $$^ -o $$@
	@echo
	@echo Memory usage summary for $$@:
	$(Q)tail -n5 $(1)_bootloader.mem
	$(Q)grep ^Stack $(1)_bootloader.mem
	@echo

 # Rule to assemble objects for the radio
 $(BOOTLOADER_DIR)%.$(1).bl.asm.rel: $(BOOTLOADER_DIR)%.asm
	$(Q)$(AS) $(BOOTLOADER_ASFLAGS) $($(1)_ASFLAGS) $$@ $$^

 # Rule to compile objects for the radio
 $(BOOTLOADER_DIR)%.$(1).bl.rel: $(BOOTLOADER_DIR)%.c $(BOOTLOADER_HEADERS)
	$(Q)$(CC) $(BOOTLOADER_CFLAGS) $($(1)_CFLAGS) $($(1)_BOOTLOADER_CFLAGS) -c $$< -o $$@

 # Rule to assemble objects for the radio
 $(COMMON_DIR)%.$(1).asm.bl.rel: $(COMMON_DIR)%.asm
	$(Q)$(AS) $(BOOTLOADER_ASFLAGS) $($(1)_ASFLAGS) $$@ $$^

 # Rule to compile objects for the radio
 $(COMMON_DIR)%.$(1).bl.rel: $(COMMON_DIR)%.c $(COMMON_HEADERS)
	$(Q)$(CC) $(BOOTLOADER_CFLAGS) $($(1)_CFLAGS) $($(1)_BOOTLOADER_CFLAGS) -c $$< -o $$@


 # Rule to assemble objects for the radio
 $($(1)_DIR)%.$(1).asm.bl.rel: $($(1)_DIR)%.asm
	$(Q)$(AS) $(BOOTLOADER_ASFLAGS) $($(1)_ASFLAGS) $$@ $$^

 # Rule to compile objects for the radio
 $($(1)_DIR)%.$(1).bl.rel: $($(1)_DIR)%.c $($(1)_DIR)/*.h
	$(Q)$(CC) $(BOOTLOADER_CFLAGS) $($(1)_CFLAGS) $($(1)_BOOTLOADER_CFLAGS) -c $$< -o $$@


 # Append to the list of all objects (used to clean up intermediate files)
 ALL_OBJS += $$($(1)_BL_OBJS) $(1)_bootloader.hex $(1)_bootloader.lk $(1)_bootloader.mem $(1)_bootloader.map \
	$$($(1)_BL_OBJS:%.rel=%.asm) $$($(1)_BL_OBJS:%.rel=%.lst) \
	$$($(1)_BL_OBJS:%.rel=%.rst) $$($(1)_BL_OBJS:%.rel=%.sym)

endef
