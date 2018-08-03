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

// System timekeeping and clock
#include "board_defaults.h"
#include "cc1110_regs.h"
#include "compiler_utils.h"
#include "telemetry.h"
#include "timers.h"

STATIC_ASSERT(timer_period_non_zero, T1_PERIOD > 0);
STATIC_ASSERT(rf_precise_timing_non_zero, RF_PRECISE_TIMING_DELAY > 0);

volatile __bit rtc_set = 0;
volatile __data uint32_t uptime;
volatile __data uint32_t rtc_seconds;
volatile __data uint16_t rtc_milliseconds;
volatile __data uint16_t timer_count_ms;

uint8_t transmit_delay;

void timers_init(void) {
	uptime = 0;
	timer_count_ms = 0; // run this loop immediately on boot

	rtc_set = 0;
	rtc_seconds = 0;
	rtc_milliseconds = 0;

	// Set Timer 1 to overflow every 1ms
	T1CC0H = (T1_PERIOD - 1) >> 8;
	T1CC0L = (T1_PERIOD - 1) & 0xff;

	T1CCTL0 = T1CCTL0_IM_ENABLED |   // Enable the T1 compare interrupt
	          T1CCTL0_CMP_SET_ON_COMPARE |  // Set the interrupt flag on compare match
	          T1CCTL0_MODE_COMPARE;  // Compare mode (rather than external capture)
	T1CTL = T1CTL_DIV_1 |  // Main clock divided by 1
	        T1CTL_MODE_MODULO;  // Reset to 0 when the CC value is reached
	TIMIF |= TIMIF_OVFIM;  // Clear the Timer 1 "overflow" ISR flag
	IEN1 |= IEN1_T1IE;  // Enable the Timer 1 ISR
}

void timers_get_time(__xdata timespec_t *t) {
	uint16_t t_fine;
	uint16_t milliseconds;
	TIMER_INTERRUPTS_DISABLE;
	t->seconds = rtc_seconds;
	milliseconds = (uint32_t) rtc_milliseconds;

	t_fine = T1CNTL;
	t_fine |= T1CNTH << 8;
	TIMER_INTERRUPTS_ENABLE;

	t->nanoseconds = (milliseconds * 1000000) + t_fine * T1_TICK;
}

uint32_t timers_get_seconds(void) {
	uint32_t seconds;
	TIMER_INTERRUPTS_DISABLE;
	seconds = rtc_seconds;
	TIMER_INTERRUPTS_ENABLE;
	return seconds;
}

void timers_set_time(const __xdata timespec_t *t) {
	uint16_t milliseconds;
	milliseconds = t->nanoseconds / 1000000;
	TIMER_INTERRUPTS_DISABLE;
	rtc_milliseconds = milliseconds;
	rtc_seconds = t->seconds;
	rtc_set = 1;
	TIMER_INTERRUPTS_ENABLE;
}

void timers_add_time(__xdata timespec_t *t1, __xdata timespec_t *t2) {
	t1->nanoseconds += t2->nanoseconds;
	if (t1->nanoseconds > 1000000000) {
		t1->nanoseconds -= 1000000000;
		t1->seconds += 1;
	}
	t1->seconds += t2->seconds;
}

void timers_subtract_time(__xdata timespec_t *t1, __xdata timespec_t *t2) {
	if (t1->nanoseconds < t2->nanoseconds) {
		t1->nanoseconds += 1000000000;
		t1->seconds -= 1;
	}
	t1->nanoseconds -= t2->nanoseconds;
	t1->seconds -= t2->seconds;
}

void timers_watch_for_RF(void) {
	// Set Timer 1 channel 1 to capture on radio events
	// Events are set in the RFIM register

	// Note that the rising/falling edge settings don't
	// apply here because we're triggering off an RF event
	// rather than an IO pin
	T1CTL &= ~(T1CTL_CH1IF);  // Clear the interrupt flag
	T1CCTL1 = T1CCTL1_CPSEL_RF_EVENT |
	          T1CCTL1_IM_ENABLED |
	          T1CCTL1_MODE_CAPTURE;
	// When the trigger hits, the timer 1 counter
	// will be stored in T1CC1H/L
}

void timers_trigger_for_RF(void) {
	// Enable the Timer 1 channel 1 interrupt so we can
	// start counting ticks before initializing a STX
	// on the radio. This is used for precise timing
	// for commands like ranging.

	// The T1CC1H/L registers are assumed to contain the
	// last trigger capture event.

	// Make sure T1CNT has incremented before enabling
	// the interrupt
	transmit_delay = RF_PRECISE_TIMING_DELAY;

	// Make sure the counter has advanced at least one
	// so we don't trip the interrupt immediately on receipt.
	// This is very unlikely given how short the timer
	// tick defaults are and how many clock cycles it takes
	// to get here.
	// This might wait an extra tick if T1CC1L == T1CNTL
	// but T1CC1H != T1CNTH but it's easier than dealing
	// with the T1CC1L/H buffer-on-read.
	while (T1CC1L == T1CNTL);

	T1CTL &= ~(T1CTL_CH1IF);  // Clear the interrupt flag
	T1CCTL1 = T1CCTL1_IM_ENABLED |  // Enable the compare interrupt
	          T1CCTL1_CMP_SET_ON_COMPARE |  // Compare match mode
	          T1CCTL1_MODE_COMPARE;  // Compare mode
	// T1CC1H/L should have the previous capture stored in it,
	// so this will trigger the timer interrupt exactly 1 interval
	// after the event (default every 1ms)
}

void t1_isr(void)  __interrupt (T1_VECTOR) __using (1) {
	if (T1CTL & T1CTL_CH0IF) {
		T1CTL &= ~(T1CTL_CH0IF);
		rtc_milliseconds += 1;
		if (timer_count_ms != 0) {
			timer_count_ms--;
		}
		if (rtc_milliseconds >= 1000) {
			rtc_milliseconds = 0;
			rtc_seconds++;
			uptime++;
		}
	} else if (T1CTL & T1CTL_CH1IF) {
		// Timer 1 Channel 1 is used for RF event capture and precision TX
		if (T1CCTL1 & T1CCTL1_CPSEL_RF_EVENT) {
			// If we're set to capture, disable the trigger after the first
			// capture
			T1CCTL1 = 0;
		} else {
			// RF event start for precise timing
			T1CTL &= ~(T1CTL_CH1IF);  // clear the interrupt flag TODO necessary?
			if (--transmit_delay == 0) {
				RFST = RFST_STX;
				T1CCTL1 &= ~(T1CCTL1_IM_ENABLED); // Disable this interrupt
			}
		}
	}
}
