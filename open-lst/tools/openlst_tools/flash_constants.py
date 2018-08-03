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

FLASH_BOOTLOADER_START = 0x0000
FLASH_BOOTLOADER_STORAGE = 0x03CC
FLASH_SIGNATURE_KEYS = 0x03CC
FLASH_RESERVED = 0x03FC
FLASH_HWID = 0x03FE
FLASH_SIGNATURE_START = 0x6BF0
FLASH_SIGNATURE_LEN = 16
FLASH_APP_START = 0x0400
FLASH_APP_END = 0x6BFF
FLASH_PAGE_SIZE = 128
FLASH_APP_SIGNATURE = 0x6BF0
FLASH_UPDATER_START = 0x7000
FLASH_STORAGE_START = 0x6C00
FLASH_STORAGE_END = 0x6FFF
