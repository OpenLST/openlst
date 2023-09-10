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

#ifndef _FLASH_CONSTANTS_H
#define _FLASH_CONSTANTS_H

// The CC1110 uses 1024 byte (512 word) pages
// when performing erases
#define FLASH_PAGE_SIZE 1024
// We use 128 byte chunks when doing writes of
// new app firmware
#define FLASH_WRITE_PAGE_SIZE 128

// Memory map:
// 0x0000 - 0x0FFF - Bootloader
// 0x1000 - 0x7BFF - Application
// 0x7C00 - 0x7FFF - Persistent storage
#define FLASH_BOOTLOADER_END 0x03FF
// Signature key storage (3 16 byte keys)
#define FLASH_SIGNATURE_KEYS 0x03CC
// Two reserved bytes
#define FLASH_RESERVED       0x03FC
// The radio HWID
#define FLASH_HWID           0x03FE
#define FLASH_APP_START      0x0400
#define FLASH_APP_END        0x6BFF
#define FLASH_APP_SIGNATURE  0x6BF0
#define FLASH_APP_CRC        0x6BFE
#define FLASH_STORAGE_START  0x6C00
#define FLASH_STORAGE_END    0x6FFF
#define FLASH_UPDATER_START  0x7000
#define FLASH_SIZE           0x8000

#endif
