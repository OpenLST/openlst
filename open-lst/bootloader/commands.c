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

#pragma codeseg APP_UPDATER
#include "board_defaults.h"
#include "bootloader_commands.h"
#include "cc1110_regs.h"
#include "commands.h"
#include "flash.h"
#include "hwid.h"
#include "radio.h"
#include "watchdog.h"
#include "signature.h"
#include "stringx.h"

extern uint16_t timeout;

uint8_t commands_handle_command(const __xdata command_t *cmd, uint8_t len, __xdata command_t *reply) {
	uint8_t reply_length;
	uint8_t flash_err;
	__xdata msg_data_t *cmd_data;
	__xdata msg_data_t *reply_data;
	__bit reset_timeout = 0;

	len;  // To avoid compiler warnings about this being unused

	cmd_data = (__xdata msg_data_t *) cmd->data;
	reply_data = (__xdata msg_data_t *) reply->data;

	// Initialize the reply header
	reply->header.hwid = hwid_flash;
	reply->header.seqnum = cmd->header.seqnum;
	reply->header.system = MSG_TYPE_RADIO_OUT;

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

		case bootloader_msg_ping:
			WATCHDOG_CLEAR;
			reset_timeout = 1;
			reply->header.command = bootloader_msg_ack;
			reply_data->ack.ack_message = BOOTLOADER_ACK_MSG_PONG;
			reply_length += sizeof(reply_data->ack.ack_message);
			break;

		case bootloader_msg_erase:
			WATCHDOG_CLEAR;
			reset_timeout = 1;
			flash_erase_app();
			reply->header.command = bootloader_msg_ack;
			reply_data->ack.ack_message = BOOTLOADER_ACK_MSG_ERASED;
			reply_length += sizeof(reply_data->ack.ack_message);
			break;

		case bootloader_msg_write_page:
			WATCHDOG_CLEAR;
			reset_timeout = 1;
			flash_err = flash_write_app_page(cmd_data->write_page.flash_page, cmd_data->write_page.page_data);
			if (flash_err != FLASH_WRITE_OK) {
				reply->header.command = bootloader_msg_nack;
			} else {
				// TODO check length
				if (cmd_data->write_page.flash_page == 255 && signature_app_valid()) { // TODO constant
					timeout = 1000;  // TODO constant
				}
				reply->header.command = bootloader_msg_ack;
				reply_data->ack.ack_message = cmd_data->write_page.flash_page;
				reply_length += sizeof(reply_data->ack.ack_message);
			}
			break;
	}
	if (reset_timeout) {
		timeout = COMMAND_WATCHDOG_DELAY;
	}
	return reply_length;
}
