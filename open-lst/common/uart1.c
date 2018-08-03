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

// UART1 setup and interrupt handler routines
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
#include "uart1.h"
#include "stringx.h"

volatile __data uint32_t uart1_rx_count;

static esp_state_t __data rx_esp_state;
static uint8_t __data rx_buffer_ready[UART1_RX_BUFFERS];
static uint8_t __data rx_buffer_len[UART1_RX_BUFFERS];
static uint8_t __data rx_active_buffer;
static uint8_t __data rx_buffer_offset;
static uint8_t __xdata rx_buffer[UART1_RX_BUFFERS][ESP_MAX_PAYLOAD];
static uint8_t __xdata tx_buffer[ESP_MAX_PAYLOAD];

#if UART1_ENABLED == 1
void uart1_init(void) {
	uint8_t b;

	// Initialize the receive counter
	uart1_rx_count = 0;

	// Give USART1 priority on port 2
	// USART0 defaults to this port so this is necessary
	// because we don't set up the UART0 outputs
	// when UART0_ENABLED is defined as 0
	P2DIR = (P2DIR & ~P2DIR_PRIP0_MASK) | P2DIR_PRIP0_USART1_USART0;

	// Select the "alternate 1" pin configuration for UART1
	PERCFG &= ~(1<<1);
	// Set the TX pin of "alternate 1" to be an output
	P0DIR |= 1<<4;
	// Select the peripheral function (rather than GPIO) for the TX and RX pins
	P0SEL |= (1<<5) | (1<<4);

	// The baud rate is given by:
	// baud_rate = (256 + BAUD_M) * 2 ^ (BAUD_E) / (2 ^ 28) * F
	// where F is the clock frequency
	U1BAUD = CONFIG_UART1_BAUD;  // U0BAUD[7:0] is BAUD_M
	// Bit 5 sets the bit order to MSB first
	// Bits 6 and 7 are not used (they are SPI settings)
	// For CONFIG_UART0_CGR, 12 = 115200 baud, 14 = 460800 baud
	U1GCR = CONFIG_UART1_GCR; // U0GCR[4:0] is BAUD_M

	// Clear any rx buffers
	for (b = 0; b < UART1_RX_BUFFERS; b++) {
		rx_buffer_ready[b] = 0;
	}

	U1CSR = (1<<7) | // UART mode (not SPI)
	        (1<<6);  // receiver enable
	U1UCR = CONFIG_UART1_UCR;

	// Set UART1 interrupts to the highest priority
	// This prevents other interrupts (timers, RF)
	// from taking too long and causing us to miss a byte
	// from the high-speed UART
	// Sets priority to 1/3 vs. the default of 0/3
	// (see http://www.ti.com/lit/ds/symlink/cc1110-cc1111.pdf pg 69)
	// TODO make this an option?
	IP0 |= IP0_IPG3;
	IP1 &= ~(IP1_IPG3);

#if defined(CONFIG_UART1_USE_FLOW_CTRL)
	P0DIR |= 1<<3;
	CONFIG_UART1_FLOW_PIN = RTS_OK;
#endif

	// TODO: these look redundant

	IEN2 &= ~IEN2_UTX1IE; // disable the tx interrupt

	URX1IE = 1; // enable RX interrupt
	UTX1IF = 1; // set the TX interrupt (ready for data)
	IEN0 |= IEN0_URX1IE;

}


uint8_t uart1_get_message(__xdata uint8_t *buf) {
	// If there is a completed message in a buffer,
	// this function will copy that message to buf
	// and return the length in bytes.
	// If no messages are ready, buf is left unchanged
	// and 0 is returned.
	uint8_t i;
	uint8_t len;
	for (i = 0; i < UART1_RX_BUFFERS; i++) {
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

void uart1_put(char c) {
  while (!UTX1IF);
  U1DBUF = c;
  UTX1IF = 0;
}

// TODO: use interrupts
void uart1_send_message(const __xdata uint8_t *msg, uint8_t len) {
	// ESP header
	uart1_put(ESP_START_BYTE_0);
	uart1_put(ESP_START_BYTE_1);
	uart1_put(len);
	while (len--) {
		uart1_put(*(msg++));
	}
}

static __xdata command_t print_buf;

// Send a string out the UART as an "ASCII" command
void dprintf1(const char * msg) {
	uint8_t len;
	print_buf.header.hwid = hwid_flash;
	print_buf.header.seqnum = 0;
	print_buf.header.system = MSG_TYPE_RADIO_OUT;
	print_buf.header.command = common_msg_ascii;
	len = strcpylenx((__xdata void *) print_buf.data, (__xdata void *) msg);
	uart1_send_message((__xdata void *) &print_buf, len + sizeof(print_buf.header));
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
//
// We use register bank 2 because this ISR is set to a higher priority
// group than other ISRs using bank 0
void uart1_rx_isr() __interrupt (URX1_VECTOR) __using (2) {
	uint8_t c;

	c = U1DBUF;
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
				if (rx_active_buffer >= UART1_RX_BUFFERS) {
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
				uart1_rx_count++;
			}
			break;
	}
}

#endif
