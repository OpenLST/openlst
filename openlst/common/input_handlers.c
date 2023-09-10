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

// Input handlers for command processing and forwarding
#ifdef BOOTLOADER
#pragma codeseg APP_UPDATER
#endif
#include "board_defaults.h"
#include "commands.h"
#include "hwid.h"
#include "input_handlers.h"
#include "uart.h"
#include "uart0.h"
#include "uart1.h"
#include "radio.h"

static __xdata command_buffer_t buffer;
static __xdata command_buffer_t reply;

// TODO: good idea?
// Overridable in case zero-length messages,
// messages with no command opcode, no seqnum
// are desired. Also in case we want to include
// an extra required field
#ifndef MIN_UART_MSG_SIZE
#define MIN_UART_MSG_SIZE  sizeof(command_header_t)
#endif
#ifndef MIN_RADIO_MSG_SIZE
#define MIN_RADIO_MSG_SIZE sizeof(command_header_t)
#endif

#if UART0_ENABLED == 1
void input_handle_uart0_rx(void) {
	uint8_t len;
	uint8_t reply_len;
	len = uart0_get_message(buffer.msg);
	if (len == 0) { // no messages
		return;
	}

	// See if this message is addressed to us,
	// is a full message, and is targeted at the radio
	if (len < MIN_UART_MSG_SIZE) {
		// Just ignore the message if it is too small
	} else if (buffer.cmd.header.system == MSG_TYPE_RADIO_IN &&
	           (buffer.cmd.header.hwid == hwid_flash ||
	            buffer.cmd.header.hwid == HWID_LOCAL)) {
		// If it is for us, pass the message off to the command handler
		reply_len = commands_handle_command(&buffer.cmd, len, &reply.cmd);
		if (reply_len) {
			uart0_send_message(reply.msg, reply_len);
		}
		return;
	} else {
		#if FORWARD_MESSAGES_UART0 == 1
		// If it's addressed elsewhere, attempt to forward it out
		// over the RF link
		radio_send_packet(&buffer.cmd, len, RF_TIMING_NOW, 0);
		#endif
	}
	return;
}
#endif

#if UART1_ENABLED == 1
void input_handle_uart1_rx(void) {
	uint8_t len;
	uint8_t reply_len;
	len = uart1_get_message(buffer.msg);
	if (len == 0) { // no messages
		return;
	}

	// See if this message is addressed to us,
	// is a full message, and is targeted at the radio
	if (len < MIN_UART_MSG_SIZE) {
		// Just ignore the message if it is too small
	} else if (buffer.cmd.header.system == MSG_TYPE_RADIO_IN &&
	           (buffer.cmd.header.hwid == hwid_flash ||
	            buffer.cmd.header.hwid == HWID_LOCAL)) {
		// If it is for us, pass the message off to the command handler
		reply_len = commands_handle_command(&buffer.cmd, len, &reply.cmd);
		if (reply_len) {
			uart1_send_message(reply.msg, reply_len);
		}
		return;
	} else {
		#if FORWARD_MESSAGES_UART1 == 1
		// If it's addressed elsewhere, attempt to forward it out
		// over the RF link
		radio_send_packet(&buffer.cmd, len, RF_TIMING_NOW, 1);
		#endif
	}
	return;
}
#endif

void input_handle_rf_rx(void) {
	uint8_t len;
	uint8_t reply_len;
	uint8_t uart_sel;
	len = radio_get_message(&buffer.cmd, &uart_sel);
	if (len == 0) { // no messages
		return;
	}
	// Start listening again now that we've copied the message over
	radio_listen();
	// See if this message is addressed to us,
	// is a full message, and is targeted at the radio
	if (len >= MIN_RADIO_MSG_SIZE &&
		buffer.cmd.header.system == MSG_TYPE_RADIO_IN &&
		(buffer.cmd.header.hwid == hwid_flash ||
		 buffer.cmd.header.hwid == HWID_LOCAL)) {
		// If it is, pass the message off to the command handler
		reply_len = commands_handle_command(&buffer.cmd, len, &reply.cmd);
		if (reply_len) {
			radio_send_packet(&reply.cmd, reply_len, RF_TIMING_NOW, uart_sel);
		}
		return;
	} else {
		// If it's addressed elsewhere, attempt to forward it out
		// over the serial link
		// TODO: respect the UART selection in flags
		#if FORWARD_MESSAGES_RF == 1
		uart1_send_message(buffer.msg, len);
		#endif
	}
	return;
}