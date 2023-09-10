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

#ifndef _TELEMETRY_H
#define _TELEMETRY_H
#include <stdint.h>
#include "adc.h"

typedef struct {
	uint8_t reserved;
	uint32_t uptime;
	uint32_t uart0_rx_count;
	uint32_t uart1_rx_count;
	uint8_t rx_mode;
	uint8_t tx_mode;
	int16_t adc[ADC_NUM_CHANNELS];
	int8_t last_rssi;
	uint8_t last_lqi;
	int8_t last_freqest;
	uint32_t packets_sent;
	uint32_t cs_count;
	uint32_t packets_good;
	uint32_t packets_rejected_checksum;
	uint32_t packets_rejected_reserved;
	uint32_t packets_rejected_other;
	uint32_t reserved0;
	uint32_t reserved1;
	uint32_t custom0;
	uint32_t custom1;

} telemetry_t;

extern __xdata telemetry_t telemetry;

void telemetry_init(void);
void update_telemetry(void);

#endif
