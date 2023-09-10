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

#ifndef _UART0_H
#define _UART0_H

#include <cc1110.h>
#include <stdint.h>

#ifndef UART1_RX_BUFFERS
// By default, UART0 is the "low speed" interface
// for simple command and response workflows and
// only has a single buffer.
// If second packet arrives before we process what's
// in the buffer it will be dropped.
// These buffers use a sizeable amount of RAM so only
// about 4 fit between UART0 and UART1
#define UART0_RX_BUFFERS 1
#endif

void uart0_rx_isr() __interrupt (URX0_VECTOR) __using (3);
void uart0_init(void);
uint8_t uart0_get_message(__xdata uint8_t *buf);
void uart0_send_message(const __xdata uint8_t *msg, uint8_t len);

// TODO: better
void dprintf0(const char *msg);
// void dprintf2(char * msg, uint8_t val);

extern volatile __data uint32_t uart0_rx_count;

#endif
