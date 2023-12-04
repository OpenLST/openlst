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

#include "board.h"
#include "timers.h"
#include "telemetry.h"

typedef enum {
	radio_msg_reboot       = 0x12,
	radio_msg_get_time     = 0x13,
	radio_msg_set_time     = 0x14,
	radio_msg_ranging      = 0x15,
	radio_msg_ranging_ack  = 0x16,
	radio_msg_get_telem    = 0x17,
	radio_msg_telem        = 0x18,
	radio_msg_get_callsign = 0x19,
	radio_msg_set_callsign = 0x1a,
	radio_msg_callsign     = 0x1b
} radio_msg_no;

#define RANGING_ACK_TYPE 1
#define RANGING_ACK_VERSION 1

typedef struct {
	uint8_t ack_type;
	uint8_t ack_version;
} radio_ranging_ack_t;

typedef struct {
	uint32_t postpone_sec;
} reboot_postpone_t;

typedef union {
	timespec_t time;
	radio_ranging_ack_t ranging_ack;
	reboot_postpone_t reboot_postpone;
	telemetry_t telemetry;
	uint8_t data[1];
#ifdef CUSTOM_COMMANDS
	rf_params_t rf_params;
#endif
} msg_data_t;

typedef struct radio_callsign{
	char callsign[8];
} radio_callsign_t;

#endif
