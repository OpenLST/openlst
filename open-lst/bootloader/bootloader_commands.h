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

// Command message opcodes
#ifndef _MESSAGES_H
#define _MESSAGES_H

#include "hwid.h"
#include "flash.h"
#include "signature.h"

typedef enum {
	// Older versions of the bootloader used
	// separate command codes for serial connections
	// and radio connections.
	// Both are included here for compatiblity
	bootloader_msg_ping        = 0x00,
	bootloader_msg_erase       = 0x0c,
	bootloader_msg_write_page  = 0x02,
	bootloader_msg_ack         = 0x01,
	bootloader_msg_nack        = 0x0f
} bootloader_msg_no;


typedef struct {
	uint8_t flash_page;
	uint8_t page_data[FLASH_WRITE_PAGE_SIZE];
} msg_bootloader_write_page_t;

#define BOOTLOADER_ACK_MSG_PONG   0
#define BOOTLOADER_ACK_MSG_ERASED 1

typedef struct {
	uint8_t ack_message;
} msg_bootloader_ack_t;

typedef union {
	msg_bootloader_write_page_t write_page;
	msg_bootloader_ack_t ack;
	signature_key_t load_keys[SIGNATURE_KEY_COUNT];
	hwid_t hwid;
	uint8_t data[1];
} msg_data_t;

#endif
