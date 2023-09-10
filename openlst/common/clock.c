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

// Clock and timer setup
#ifdef BOOTLOADER
#pragma codeseg HOME
#endif
#include <stdint.h>
#include "cc1110_regs.h"

void clock_init(void) {

	//TODO: Remove this once we've fixed the bootlaoder to wait
	// long enough to flush its UART tx queue
	// For now this provides a ~20ms delay that allows the
	// last byte to exit the UART
	// TODO maybe we can wait on some UART flag equivalent
	// to tx_complete instead of tx_ready?
	uint16_t tick = 33600;
	while (--tick);
	// Switch to the crystal oscillator at 26Mhz

	// Enable the crystal oscillator at f/2 (13.5Mhz)
	// If the crystal is not ready yet (if we just powered on
	// and the bootloader hasn't set it up yet) this
	// will actually use the high-speed RC oscillator at f/2
	// at first and the chip will switch to the cyrstal once
	// it's ready. We use f/2 because that is the maximum
	// for the HSRC oscillator.
	// If the bootloader already enabled the crystal or
	// this is a soft reset, this will actually drop the
	// clock rate to f/2 temporarily.
	CLKCON = CLKCON_OSC32K_RC | // Select the low-power RC oscillator
	         CLKCON_OSC_HSXTAL | // select the high-speed crystal oscillator
	         CLKCON_TICKSPD_F_2 | // TICKSPD (timer clock) of f/2 (13.5Mhz)
	         CLKCON_CLKSPD_F_2;  // CLKSPD of f/2 (13Mhz)

	// Wait for the oscillator to be stable
	while (!(SLEEP & SLEEP_XOSC_STB));

	// Enable the crystal oscillator at f (26Mhz)
	CLKCON = CLKCON_OSC32K_RC | // low-power RC oscillator (no change)
	         CLKCON_OSC_HSXTAL | // high-speed crystal oscillator
	         CLKCON_TICKSPD_F | // TICKSPD of f (27Mhz)
	         CLKCON_CLKSPD_F;  // CLKSPD of f (27Mhz)

	// Wait for the oscillator to be stable
	while (!(SLEEP & SLEEP_XOSC_STB));

	// Power down the unused high-speed RC oscillator
	SLEEP |= SLEEP_OSC_PD;
}
