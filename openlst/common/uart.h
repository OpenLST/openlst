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

#ifndef _UART_H
#define _UART_H


#define ESP_START_BYTE_0 0x22           /** First start byte  */
#define ESP_START_BYTE_1 0x69           /** Second start byte  */
#define ESP_MAX_PAYLOAD 251
#define RTS_OK   0
#define RTS_WAIT 1

typedef enum {
  wait_for_start0,
  wait_for_start1,
  wait_for_length,
  receive_data
} esp_state_t;

#endif
