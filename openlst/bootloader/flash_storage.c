// OpenLST
// Copyright (C) 2018 Planet Labs Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdint.h>
#include "hwid.h"

// If we use too much space in the "HOME" region, bootloader code will overlap with the
// designated storage areas in flash sector 0.
// Code memory regions are defined here so that the linker will detect this overlap.
// NOTE that these are only linker warnings and compilation will proceed regardless
// TODO figure out a way to error out

// These are reserved regions for the bootloader
__code __at (FLASH_SIGNATURE_KEYS) uint8_t flash_keys[FLASH_RESERVED - FLASH_SIGNATURE_KEYS] = {0};
__code __at (FLASH_RESERVED)       uint16_t reserved_flash = 0;
__code __at (FLASH_HWID)           hwid_t   hwid_flash     = HWID;

// These are reserved regions for the application
__code __at (FLASH_APP_START)      uint8_t  flash_app[FLASH_STORAGE_START - FLASH_APP_START] = {0};
__code __at (FLASH_STORAGE_START)  uint8_t  flash_storage[FLASH_UPDATER_START - FLASH_STORAGE_START] = {0};
