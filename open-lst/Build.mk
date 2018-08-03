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

.PHONY: clean

Q ?= @
MODEL = medium
AS = sdas8051
CC = sdcc

GIT_REV = $(shell git describe --always --dirty)
GIT_REV_HEX = 0x$(shell git rev-parse --short=8 HEAD)

ALL_HEXS += 
ROOT_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
COMMON_DIR = $(ROOT_DIR)common
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

include $(ROOT_DIR)Bootloader.mk
include $(ROOT_DIR)Radio.mk

all: bootloaders radios
bootloaders: $(BOOTLOADERS:%=%_bootloader)
radios: $(RADIOS:%=%_radio)

# Apply template to each radio
$(foreach bootloader,$(BOOTLOADERS),$(eval $(call BOOTLOADER_template,$(bootloader))))
$(foreach radio,$(RADIOS),$(eval $(call RADIO_template,$(radio))))

clean:
	$(Q)rm -f $(ALL_OBJS)
