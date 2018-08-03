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

// Handle scheduled events like reboots and security leases

#include "schedule.h"
#include "board_defaults.h"
#include "adc.h"
#include "timers.h"
#include "radio.h"
#include "telemetry.h"
#include "watchdog.h"

__xdata uint32_t auto_reboot;

void schedule_init(void) {
	#if AUTO_REBOOT_SECONDS == 0
	auto_reboot = 0;
	#else
	schedule_postpone_reboot(AUTO_REBOOT_SECONDS);
	#endif
}

uint8_t schedule_postpone_reboot(uint32_t postpone) {
	#if AUTO_REBOOT_MAX != 0
	if (postpone > AUTO_REBOOT_MAX) {
		return SCHEDULE_REBOOT_TOO_LONG;
	}
	#endif
	auto_reboot = uptime + postpone;
	return SCHEDULE_REBOOT_POSTPONED;
}

void schedule_handle_events(void) {
	// Reboot if our uptime is too high
	if (auto_reboot != 0 && uptime >= auto_reboot) {
		watchdog_reboot_now();
	}

	// Handle the 10Hz loop
	if (timer_count_ms == 0) {
		timer_count_ms = TIMER_COUNT_PERIOD;
		update_telemetry();
		// Assume that this will take < 100ms
		// Othewise we may have some garbage samples
		adc_start_sample();
		// If we haven't recieved a packet in a while
		// reset the receiver
		if (++last_rx_ticks >= MAX_RX_TICKS) {
			last_rx_ticks = 0;
			radio_listen();
		}
	}

}
