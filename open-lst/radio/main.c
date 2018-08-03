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

#include <stdint.h>
#include <cc1110.h>
#include "board_defaults.h"
#include "adc.h"
#include "clock.h"
#include "commands.h"
#include "dma.h"
#include "input_handlers.h"
#include "interrupts.h"
#include "schedule.h"
#include "uart0.h"
#include "uart1.h"
#include "radio.h"
#include "telemetry.h"
#include "timers.h"
#include "watchdog.h"
// User specified board setup
#ifdef CUSTOM_BOARD_INIT
#include "board.h"
#else
#ifndef NO_CUSTOM_BOARD_INIT
#warning "No board specific board.h/board.c was specified"
#endif
#endif


#ifndef GIT_REV
#define GIT_REV
#endif

#define TO_STR(x) #x
#define BOOT_STRING(git_rev) TO_STR(OpenLST git_rev)

// don't make this too long or it could get stuck in a watchdog reset loop
#define TIMEOUT 100000

uint32_t timeout = TIMEOUT;

static void initialize(void) {
	// Set up the watchdog reset timer for about 1 second
	WATCHDOG_ENABLE;
	WATCHDOG_CLEAR;

	clock_init();
	timers_init();
	dma_init();
	telemetry_init();
	#ifdef CUSTOM_BOARD_INIT
	board_init();  // User specified init functions
	#endif
	// TODO: wait for ccboot to finish whatever it's sending in case we just
	// did a bootload
	uart0_init();
	uart1_init();
	INTERRUPT_GLOBAL_ENABLE;

	adc_init();
	schedule_init();
	radio_init();
	#if CONFIG_CAPABLE_RF_RX == 1
	radio_listen();
	#endif
}

void main(void) {
	initialize();

	#if BOARD_HAS_LED == 1
	board_led_set(0);
	#endif

	dprintf1(BOOT_STRING(GIT_REV));
	while (1) {
		WATCHDOG_CLEAR;
		schedule_handle_events();
		input_handle_uart0_rx();
		input_handle_uart1_rx();
		input_handle_rf_rx();

	}
}
