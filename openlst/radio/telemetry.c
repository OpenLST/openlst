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

// Radio telemetry acquisition and reporting

#include "telemetry.h"
#include "adc.h"
#include "radio.h"
#include "stringx.h"
#include "timers.h"
#include "uart0.h"
#include "uart1.h"

__xdata telemetry_t telemetry;

void telemetry_init(void) {
	memsetx((__xdata void*) &telemetry, 0, sizeof(telemetry));
	// This one is signed and we want the default to be
	// -128dBm rather than 0dBm
	telemetry.last_rssi = -128;
}

void update_telemetry(void) {
	telemetry.reserved = 0;
	__critical {
		telemetry.uptime = uptime;
		telemetry.uart0_rx_count = uart0_rx_count;
		telemetry.uart1_rx_count = uart1_rx_count;
	}
	if (adc_sample_ready) {
		// The ADC sample should always be ready except for
		// the first iteration of this loop. We give it
		// one full tick of the 10Hz loop (100ms) to complete
		// the chain of samples, but this check is here
		// to prevent garbage data in case something the user
		// added is delaying the ADC sample in the loop.
		memcpyx(
			(__xdata void *) telemetry.adc,
			(__xdata void *) adc_buffer,
			sizeof(telemetry.adc));
	}
	// RSSI is stored as a two's complement value in the RSSI register
	__critical {
		telemetry.last_rssi = radio_last_rssi;
		telemetry.last_lqi = radio_last_lqi;
		telemetry.last_freqest = radio_last_freqest;
		telemetry.cs_count = radio_cs_count;
	}

	telemetry.rx_mode = radio_mode_rx;
	telemetry.tx_mode = radio_mode_tx;
	//TODO cs_count

	telemetry.packets_sent = radio_packets_sent;
	telemetry.packets_good = radio_packets_good;
	telemetry.packets_rejected_checksum = radio_packets_rejected_checksum;
	telemetry.packets_rejected_reserved = radio_packets_rejected_reserved;
	telemetry.packets_rejected_other = radio_packets_rejected_other;

}
