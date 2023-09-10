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

#ifndef HWID_H
#define HWID_H

#include <stdint.h>
#include "flash_constants.h"

#define HWID_BCAST (0x0000)  // Broadcast to all satellites
#define HWID_LOCAL (0xFFFF)  // Local (groundstation) radio
#define HWID_UNSET (0xFFFF)  // Uninitialized flash

typedef uint16_t hwid_t;

__code __at (FLASH_HWID) hwid_t hwid_flash;

#endif  // HWID_H
