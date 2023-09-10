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

// UART0 setup and interrupt handler routines
#ifdef BOOTLOADER
#pragma codeseg APP_UPDATER
#endif
#include <cc1110.h>
#include <string.h>
#include <stdint.h>
#include "cc1110_regs.h"
#include "board_defaults.h"
#include "hwid.h"
#include "uart.h"
#include "uart0.h"
#include "stringx.h"

volatile __data uint32_t uart0_rx_count;

static esp_state_t __data rx_esp_state;
static uint8_t __data rx_buffer_ready[UART0_RX_BUFFERS];
static uint8_t __data rx_buffer_len[UART0_RX_BUFFERS];
static uint8_t __data rx_active_buffer;
static uint8_t __data rx_buffer_offset;
static uint8_t __xdata rx_buffer[UART0_RX_BUFFERS][ESP_MAX_PAYLOAD];
static uint8_t __xdata tx_buffer[ESP_MAX_PAYLOAD];

#if UART0_ENABLED == 1
void uart0_init(void) {
	uint8_t b;

	// Initialize the receive counter
	uart0_rx_count = 0;
	// Select the "alternate 2" pin configuration for UART0
	PERCFG |= 1<<0;
	// Set the TX pin of "alternate 2" to be an output
	P1DIR |= 1<<5;
	// Select the peripheral function (rather than GPIO) for the TX and RX pins
	P1SEL |= (1<<5) | (1<<4);
	// The baud rate is given by:
	// baud_rate = (256 + BAUD_M) * 2 ^ (BAUD_E) / (2 ^ 28) * F
	// where F is the clock frequency
	U0BAUD = CONFIG_UART0_BAUD;  // U0BAUD[7:0] is BAUD_M
	// Bit 5 sets the bit order to MSB first
	// Bits 6 and 7 are not used (they are SPI settings)
	// For CONFIG_UART0_CGR, 12 = 115200 baud, 14 = 460800 baud
	U0GCR = CONFIG_UART0_GCR; // U0GCR[4:0] is BAUD_M

	// TODO: flow control
	// TODO: High priority ISR

	U0CSR = (1<<7) | // UART mode (not SPI)
	        (1<<6);  // receiver enable
	U0UCR = CONFIG_UART0_UCR;


	// Clear any rx buffers
	for (b = 0; b < UART0_RX_BUFFERS; b++) {
		rx_buffer_ready[b] = 0;
	}

	// TODO: flow control

	// TODO: these look redundant
	IEN2 &= ~IEN2_UTX0IE; // disable the tx interrupt (which one is it?)

	URX0IE = 1; // enable RX interrupt
	UTX0IF = 1; // set the TX interrupt (ready for data)
	IEN0 |= IEN0_URX0IE;

}


uint8_t uart0_get_message(__xdata uint8_t *buf) {
	// If there is a completed message in a buffer,
	// this function will copy that message to buf
	// and return the length in bytes.
	// If no messages are ready, buf is left unchanged
	// and 0 is returned.
	uint8_t i;
	uint8_t len;
	for (i = 0; i < UART0_RX_BUFFERS; i++) {
		if (rx_buffer_ready[i]) {
			// Copy the message to the output buffer
			len = rx_buffer_len[i];
			memcpyx(buf, rx_buffer[i], len);
			// Release the buffer
			rx_buffer_ready[i] = 0;
			return len;
		}
	}
	// No finished buffers found
	return 0;
}

void uart0_put(char c) {
  while (!UTX0IF);
  U0DBUF = c;
  UTX0IF = 0;
}

// TODO: use interrupts
void uart0_send_message(const __xdata uint8_t *msg, uint8_t len) {
	// ESP header
	uart0_put(ESP_START_BYTE_0);
	uart0_put(ESP_START_BYTE_1);
	uart0_put(len);
	while (len--) {
		uart0_put(*(msg++));
	}
}

static __xdata command_t print_buf;

// Send a string out the UART as an "ASCII" command
void dprintf0(const char * msg) {
	uint8_t len;
	print_buf.header.hwid = hwid_flash;
	// TODO no hardcode
	print_buf.header.seqnum = 0;
	print_buf.header.system = MSG_TYPE_RADIO_OUT;
	print_buf.header.command = common_msg_ascii;
	len = strcpylenx((__xdata void *) print_buf.data, (__xdata void *) msg);
	uart0_send_message((__xdata void *) &print_buf, len + sizeof(print_buf.header));
}


// UART ISR
//
// For high baud rates (460800), this ISR must complete as fast as
// possible. Mostly because HW flow control is not effective (CC1110
// has 2 byte FIFO, FTDI will send 0-3 bytes after RTS is de-asserted).
// The problem is that this UART could stall the other UART long enough
// to lose a byte.
//
// At 460800, a character takes ~20us, that means we need to have all UART
// ISRs, other ISRs at the same or higher priority and all critical sections
// complete in roughly that time.
//
// To keep this fast, the index variables are in fast access RAM. We do
// not check the UART error flags.
void uart0_rx_isr() __interrupt (URX0_VECTOR) __using (3) {
	uint8_t c;

	c = U0DBUF;
	switch (rx_esp_state) {
		case wait_for_start0:
			// Waiting for a packet to start
			if (c == ESP_START_BYTE_0) {
				rx_esp_state = wait_for_start1;
			}
			break;

		case wait_for_start1:
			if (c == ESP_START_BYTE_1) {
				rx_esp_state = wait_for_length;
			} else if (c == ESP_START_BYTE_0) {
				rx_esp_state = wait_for_start1;
			}
			break;

		case wait_for_length:
			if (c > ESP_MAX_PAYLOAD || c < 1) {
				// Skip this packet if it is too long to handle
				rx_esp_state = wait_for_start1;
			} else {
				// Find a free buffer
				rx_active_buffer = 0;
				while (rx_buffer_ready[rx_active_buffer]) {
					rx_active_buffer++;
				}
				if (rx_active_buffer >= UART0_RX_BUFFERS) {
					// No free buffers, just skip this packet
					rx_active_buffer = 0;
					rx_esp_state = wait_for_start0;
				} else {
					rx_buffer_len[rx_active_buffer] = c;
					rx_buffer_offset = 0;
					rx_esp_state = receive_data;
				}
			}
			break;

		case receive_data:
			rx_buffer[rx_active_buffer][rx_buffer_offset++] = c;
			if (rx_buffer_offset == rx_buffer_len[rx_active_buffer]) {
				// This packet is done
				rx_buffer_ready[rx_active_buffer] = 1;
				rx_esp_state = wait_for_start0;
				uart0_rx_count++;
			}
			break;
	}
}

#endif
