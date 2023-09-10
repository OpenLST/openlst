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

#include "commands.h"
#include "cc1110_regs.h"
#include "board_defaults.h"
#include "hwid.h"
#include "radio_commands.h"
#include "radio.h"
#include "schedule.h"
#include "stringx.h"
#include "watchdog.h"

#ifdef CUSTOM_COMMANDS
uint8_t custom_commands(const __xdata command_t *cmd, uint8_t len, __xdata command_t *reply);
#endif

uint8_t commands_handle_command(const __xdata command_t *cmd, uint8_t len, __xdata command_t *reply) {
	uint8_t reply_length;
	#if RADIO_RANGING_RESPONDER == 1
	uint8_t old_tx_mode;
	#endif
	__xdata msg_data_t *cmd_data;
	__xdata msg_data_t *reply_data;

	__xdata radio_callsign_t *olst_callsign;
	__xdata radio_callsign_t olst_callsign_rx;

	// Initialize the reply header
	reply->header.hwid = hwid_flash;
	reply->header.seqnum = cmd->header.seqnum;
	reply->header.system = MSG_TYPE_RADIO_OUT;

	cmd_data = (__xdata msg_data_t *) cmd->data;
	reply_data = (__xdata msg_data_t *) reply->data;

	// Fallthrough case - use "nack" as the default response
	reply->header.command = common_msg_nack;
	reply_length = sizeof(reply->header);

	switch (cmd->header.command) {
		case common_msg_ack:
			reply->header.command = common_msg_ack;
			break;

		case common_msg_nack:
			reply->header.command = common_msg_nack;
			break;

		case radio_msg_reboot:
			// Postpone reboot by specified number of seconds
			if (len < sizeof(cmd->header) + sizeof(cmd_data->reboot_postpone)) {
				// If we get a reboot message without the param, reboot immediately
				// (assume the value is 0)
				reply->header.command = common_msg_ack;
				watchdog_reboot_now();
			} else {
				if (schedule_postpone_reboot(cmd_data->reboot_postpone.postpone_sec) ==
					SCHEDULE_REBOOT_POSTPONED) {
					reply->header.command = common_msg_ack;
				} else {
					reply->header.command = common_msg_nack;
				}
			}
			break;

		case radio_msg_get_time:
			if (rtc_set) {
				reply->header.command = radio_msg_set_time;
				timers_get_time(&reply_data->time);
				reply_length += sizeof(reply_data->time);
			} else {
				// Nack if the time has not been set yet
				reply->header.command = common_msg_nack;
			}
		break;

		case radio_msg_set_time:
			// TODO: any limits on this?
			reply->header.command = common_msg_ack;
			timers_set_time(&cmd_data->time);
		break;

		case radio_msg_get_telem:
			reply->header.command = radio_msg_telem;
			memcpyx(
				(__xdata void *) &reply_data->telemetry,
				(__xdata void *) &telemetry,
				sizeof(reply_data->telemetry));
			reply_length += sizeof(reply_data->telemetry);
		break;

		case radio_msg_set_callsign:
			reply->header.command = common_msg_ack;
			olst_callsign = (__xdata radio_callsign_t *) cmd->data;

			memsetx((__xdata char *) olst_callsign_rx, 0, sizeof(*olst_callsign));
			memcpyx(	(__xdata void *) &olst_callsign_rx,
				(__xdata void *) olst_callsign,
				sizeof(*olst_callsign));
		break;

		case radio_msg_get_callsign:
			reply->header.command = radio_msg_callsign;
			olst_callsign = (__xdata radio_callsign_t *) reply->data;

			memcpyx(	(__xdata void *) olst_callsign,
				(__xdata void *) olst_callsign_rx,
				sizeof(*olst_callsign));

			reply_length += sizeof(*olst_callsign);
		break;

		#if RADIO_RANGING_RESPONDER == 1
		case radio_msg_ranging:
			reply->header.command = radio_msg_ranging_ack;
			// TODO handle encryption
			reply_data->ranging_ack.ack_type = RANGING_ACK_TYPE;
			reply_data->ranging_ack.ack_version= RANGING_ACK_VERSION;
			reply_length += sizeof(reply_data->ranging_ack);
			// TODO wait until timer edge

			// Send this packet using the ranging radio mode
			old_tx_mode = radio_mode_tx;
			radio_mode_tx = RADIO_MODE_RANGING_TX;
			radio_send_packet(reply, reply_length, RF_TIMING_PRECISE, RF_RANGING_UART);
			// Restore the radio settings and mute the normal response
			radio_mode_tx = old_tx_mode;
			reply_length = 0;
		break;
		#endif

		#ifdef CUSTOM_COMMANDS
		default:
			reply_length = custom_commands(cmd, len, reply);
		#endif
	}
	return reply_length;
}
