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

#pragma codeseg APP_UPDATER
#include <stdint.h>
#include <cc1110.h>
#include "board_defaults.h"
#include "input_handlers.h"
#include "interrupts.h"
#include "uart0.h"
#include "uart1.h"
#include "radio.h"
#include "watchdog.h"


#ifndef GIT_REV
#define GIT_REV
#endif

#define TO_STR(x) #x
#define BOOT_STRING(git_rev) TO_STR(OpenLST BL git_rev)

// TODO: use an interrupt!
uint16_t timeout;

static void initialize(void) {
	IN_BOOTLOADER_FLAG = 1;
	WATCHDOG_CLEAR;

	// The clock and DMA descriptors were already
	// set in bootloader_main
	// board_init has already been called as well

	// Now we enable the UARTs and radio to receive
	// updates
	#if UART0_ENABLED == 1
	uart0_init();
	#endif
	#if UART1_ENABLED == 1
	uart1_init();
	#endif

	INTERRUPT_GLOBAL_ENABLE;
	radio_init();

	#if CONFIG_CAPABLE_RF_RX == 1
	radio_listen();
	#endif
}


#if TX_LOG_MESSAGES_RF == 1
// TODO: rf send
#define send_message(s) dprintf1(s)
#else
#define send_message(s) dprintf1(s)
#endif

void updater(void) {
	initialize();

	#if BOARD_HAS_LED == 1
	board_led_set(1);
	#endif
	
	send_message(BOOT_STRING(GIT_REV));
	timeout = COMMAND_WATCHDOG_DELAY;
	while (--timeout) {
		#if UART0_ENABLED == 1
		input_handle_uart0_rx();
		#endif
		#if UART1_ENABLED == 1
		input_handle_uart1_rx();
		#endif
		input_handle_rf_rx();
	}
}
