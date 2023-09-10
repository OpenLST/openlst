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

#ifndef _SIGNATURE_H
#define _SIGNATURE_H

#include <stdint.h>

#define AES_KEY_SIZE   16
#define AES_BLOCK_SIZE 16
#define SIGNATURE_KEY_COUNT 3

typedef struct {
	uint8_t key[AES_KEY_SIZE];
} signature_key_t;

__bit signature_app_valid(void);

#endif
