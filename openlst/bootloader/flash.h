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

#ifndef _FLASH_H
#define _FLASH_H

#include <stdint.h>
#include "flash_constants.h"
#include "hwid.h"
#include "signature.h"

#define FLASH_WRITE_OK             0
#define FLASH_WRITE_ERR_PROTECTED  1
#define FLASH_WRITE_ERR_BAD_ADDR   2
#define FLASH_WRITE_ERR_NOT_EMPTY  3

void flash_erase_app(void) __critical;
uint8_t flash_write_app_page(uint8_t page, __xdata uint8_t *buf);

#endif
