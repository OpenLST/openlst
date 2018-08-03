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

#pragma codeseg HOME
#include <cc1110.h>
#include "board_defaults.h"
#include "clock.h"
#include "commands.h"
#include "dma.h"
#include "flash.h"
#include "interrupts.h"
#include "signature.h"
#include "updater.h"
#include "watchdog.h"
// User specified board setup
#ifdef CUSTOM_BOARD_INIT
#include "board.h"
#else
#ifndef NO_CUSTOM_BOARD_INIT
#warning "No board specific board.h/board.c was specified"
#endif
#endif


void bootloader_main(void) {
	INTERRUPT_GLOBAL_DISABLE;
	// Set up the watchdog reset timer for about 1 second
	WATCHDOG_ENABLE;
	WATCHDOG_CLEAR;
	
	// Disable interrupt enables for when we reenable
	// interrupts globally
	IEN0 = IEN1 = IEN2 = 0;

	clock_init();
	dma_init();

	#ifdef CUSTOM_BOARD_INIT
	board_init();  // User specified init functions
	#endif

	// Listen over the radio for updates
	IN_BOOTLOADER_FLAG = 1;
	updater();
	// Try to boot the application
	if (signature_app_valid()) {
		// Disable interrupt enables for when we reenable
		INTERRUPT_GLOBAL_DISABLE;
		// interrupts globally
		IEN0 = IEN1 = IEN2 = 0;
		// Mark the bootloader flag so that the ISR trampoline
		// in start.asm jumps to the application ISRs
		IN_BOOTLOADER_FLAG = 0;
		// Jump to user code
		__asm
		ljmp #0x0400
		__endasm;
	}
}
