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

#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <stdint.h>
#include "hwid.h"
#include "uart.h"

typedef enum {
  common_msg_ack = 0x10,
  common_msg_nack = 0xff,
  common_msg_ascii = 0x11
} common_msg_no;

// Correctly-sized container of enum value, for when packing/unpacking data
typedef uint8_t radio_msg_no_t;

typedef struct command_header {
	hwid_t hwid;
	uint16_t seqnum;
	uint8_t system;
	radio_msg_no_t command;
} command_header_t;

typedef struct command {
	command_header_t header;
	uint8_t data[ESP_MAX_PAYLOAD - sizeof(command_header_t)];
} command_t;

typedef union command_buffer {
	uint8_t msg[ESP_MAX_PAYLOAD];
 	command_t cmd;
} command_buffer_t;

uint8_t commands_handle_command(const __xdata command_t *cmd, uint8_t len, __xdata command_t *reply);

#endif