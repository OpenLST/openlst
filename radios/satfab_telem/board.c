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

#include <cc1110.h>
#include "cc1110_regs.h"

void board_init(void) {
	// LED0 setup - just turn it on
	P0SEL &= ~(1<<6);  // GPIO not peripheral
	P0DIR |= 1<<6;  // Output not input
	P0_6 = 1;

	// LED1 setup
	P0SEL &= ~(1<<7);  // GPIO not peripheral
	P0DIR |= 1<<7;  // Output not input

	// Bypass pin setup
	P1SEL &= ~(1<<1); // GPIO not peripheral
	P1DIR |= 1<<1; // Output not input
	P1_1 = 1;

	// Power amplifier bias control on P2.0
	// P2SEL &= ~(1<<0);  // GPIO not peripheral
	// P2DIR |= 1<<0;  // Output not input
	// P2_0 = 0;

	/* Set up radio "test" signal pins.  We're setting these up to be
	   outputs from the radio core indicating when the radio is
	   transmitting and receiving, to control off-chip, on-board and
	   off-board RF path switches.  Once set up, they are automatically
	   toggled appropriately by the radio core whenever the radio state
	   changes. */
	// P1_7 = LNA_PD (asserted high in TX) = LST_TX_MODE
	IOCFG2 = IOCFG2_GDO2_INV_ACTIVE_HIGH | IOCFG_GDO_CFG_PA_PD;
	// P1_6 = PA_PD  (asserted low in RX) = !LST_RX_MODE
	IOCFG1 = IOCFG1_GDO1_INV_ACTIVE_LOW | IOCFG_GDO_CFG_LNA_PD;
	// No special function for P1_5 (IOCFG0)
}

void board_led_set(__bit led_on) {
	P0_7 = led_on;
}

void board_led0_set(__bit led_on) {
	P0_6 = led_on;
}
