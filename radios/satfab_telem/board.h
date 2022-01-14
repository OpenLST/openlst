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

#ifndef _BOARD_H
#define _BOARD_H

// We use a 27MHz clock
#define F_CLK 27000000

#define CUSTOM_BOARD_INIT 1
#define BOARD_HAS_TX_HOOK 1
#define BOARD_HAS_RX_HOOK 1
#define CONFIG_CAPABLE_RF_RX 1
#define CONFIG_CAPABLE_RF_TX 1

// Enable the power supply sense lines AN0 and AN1
#define ADCCFG_CONFIG 0b00000011

#define RADIO_RANGING_RESPONDER 1

void board_init(void);

#define BOARD_HAS_LED 1
void board_led_set(__bit led_on);
void board_led0_set (__bit led_on);

// These are macros to save space in the bootloader
// Enable bias to on-board 1W RF power amp (RF6504)
#define board_pre_tx() P2_0 = 1;
// Disable on-board power amp bias, to save power
#define board_pre_rx() P2_0 = 0;

// Set output power to -30 dBm, from Table 72 page 207
#define RF_PA_CONFIG 0x12

#endif