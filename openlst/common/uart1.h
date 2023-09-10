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

#ifndef _UART1_H
#define _UART1_H

#include <cc1110.h>
#include <stdint.h>

#ifndef UART1_RX_BUFFERS
// By default, UART1 is the "high speed" interface
// for passing data through to another device
// Packets are buffered so that if the radio
// falls behind by 1-2 packets it can still catch
// up without dropping anything.
// These buffers use a sizeable amount of RAM so only
// about 4 fit between UART0 and UART1
#define UART1_RX_BUFFERS 3
#endif

void uart1_rx_isr() __interrupt (URX1_VECTOR) __using (2);
void uart1_init(void);
uint8_t uart1_get_message(__xdata uint8_t *buf);
void uart1_send_message(const __xdata uint8_t *msg, uint8_t len);

// TODO: better
void dprintf1(const char *msg);
// void dprintf2(char * msg, uint8_t val);

extern volatile __data uint32_t uart1_rx_count;

#endif
