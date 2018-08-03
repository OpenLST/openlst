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

# This file provides a template for the radio
# makefile
RADIO_SECTION_START = 0x0400
# 0x7000-0x7FFF is reserved for the bootloader
# 0x0000-0x0400 is also reserved for the bootloader
# 0x6C00-0x6FFF is reserved for non-volatile storage
# 0x6BE0-0x6BFF are reserved for the signature
RADIO_SECTION_SIZE = 26592

RADIO_DIR := $(dir $(lastword $(MAKEFILE_LIST)))radio

RADIO_ASFLAGS = -plogsgff
RADIO_CFLAGS = --model-$(MODEL) -I$(COMMON_DIR) -I$(RADIO_DIR) \
	-DGIT_REV=$(GIT_REV) -DGIT_REV_HEX=$(GIT_REV_HEX)
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
	$(RADIO_DIR)/timers.c

#flash_trigger must come first (code alignment)
RADIO_ASMS = $(RADIO_DIR)/flash_trigger.asm

RADIO_HEADERS = $(RADIO_DIR)/*.h

# Template for building each radio
define RADIO_template

 # Build radio hex file
 $(1)_radio: $(1)_radio.hex

 # Object Files used to build the radio
 # flash_trigger (part of COMMON_ASMS) must come first to preserve
 # even code alignment.
 $(1)_OBJS = $(RADIO_ASMS:%.asm=%.$(1).asm.rel) \
	$($(1)_ASMS:%.asm=%.$(1).asm.rel) \
	$(COMMON_SRCS:%.c=%.$(1).rel) \
	$(RADIO_SRCS:%.c=%.$(1).rel) \
	$($(1)_SRCS:%.c=%.$(1).rel) \
	$($(1)_RADIO_SRCS:%.c=%.$(1).rel)

 # Rule to make the radio firmware hex file
 $(1)_radio.hex: $$($(1)_OBJS)
	$(Q)$(CC) $(RADIO_LDFLAGS) $($(1)_LDFLAGS) $($(1)_CFLAGS) $(RADIO_CFLAGS) $$^ -o $$@
	@echo
	@echo Memory usage summary for $$@:
	$(Q)tail -n5 $(1)_radio.mem
	$(Q)grep ^Stack $(1)_radio.mem
	@echo

 # Rule to assemble objects for the radio
 $(RADIO_DIR)%.$(1).asm.rel: $(RADIO_DIR)%.asm
	$(Q)$(AS) $(RADIO_ASFLAGS) $($(1)_ASFLAGS) $$@ $$^

 # Rule to compile objects for the radio
 $(RADIO_DIR)%.$(1).rel: $(RADIO_DIR)%.c $(RADIO_HEADERS)
	$(Q)$(CC) $(RADIO_CFLAGS) $($(1)_CFLAGS) -c $$< -o $$@

 # Rule to assemble objects for the radio
 $(COMMON_DIR)%.$(1).asm.rel: $(COMMON_DIR)%.asm
	$(Q)$(AS) $(RADIO_ASFLAGS) $($(1)_ASFLAGS) $$@ $$^

 # Rule to compile objects for the radio
 $(COMMON_DIR)%.$(1).rel: $(COMMON_DIR)%.c $(COMMON_HEADERS)
	$(Q)$(CC) $(RADIO_CFLAGS) $($(1)_CFLAGS) -c $$< -o $$@


 # Rule to assemble objects for the radio
 $($(1)_DIR)%.$(1).asm.rel: $($(1)_DIR)%.asm
	$(Q)$(AS) $(RADIO_ASFLAGS) $($(1)_ASFLAGS) $$@ $$^

 # Rule to compile objects for the radio
 $($(1)_DIR)%.$(1).rel: $($(1)_DIR)%.c $($(1)_DIR)/*.h
	$(Q)$(CC) $(RADIO_CFLAGS) $($(1)_CFLAGS) -c $$< -o $$@


 # Append to the list of all objects (used to clean up intermediate files)
 ALL_OBJS += $$($(1)_OBJS) $(1)_radio.hex $(1)_radio.lk $(1)_radio.mem $(1)_radio.map \
	$$($(1)_OBJS:%.rel=%.asm) $$($(1)_OBJS:%.rel=%.lst) \
	$$($(1)_OBJS:%.rel=%.rst) $$($(1)_OBJS:%.rel=%.sym)

endef
