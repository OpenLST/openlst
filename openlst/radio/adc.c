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

// Analog to Digital conversion setup and control

#include "adc.h"
#include "board_defaults.h"

volatile __bit adc_sample_ready;
volatile int16_t adc_buffer[ADC_NUM_CHANNELS];
static __code uint8_t adc_channels[ADC_NUM_CHANNELS] = {
	ADCCON3_ECH_AIN0,
	ADCCON3_ECH_AIN1,
	ADCCON3_ECH_AIN2,
	ADCCON3_ECH_AIN3,
	ADCCON3_ECH_AIN4,
	ADCCON3_ECH_AIN5,
	ADCCON3_ECH_AIN6,
	ADCCON3_ECH_AIN7,
	ADCCON3_ECH_TEMPERATURE,
	ADCCON3_ECH_VDD_3
};

static volatile uint8_t adc_channel_index;

void adc_init(void) {
	// Enable ADC inputs
	// Keep in mind this overrides any Px_DIR settings!
	ADCCFG = ADCCFG_CONFIG;
	// Disable the interrupt and sample ready bit
	ADCIE = 0;
	adc_sample_ready = 0;
}

void adc_start_sample(void) {
	// Disable any ongoing samples by turning off the
	// interrupt
	ADCIE = 0;
	adc_sample_ready = 0;
	// Start sampling at index 0 using the internal reference
	// in 512 decimation rate mode (12 bit samples)
	adc_channel_index = 0;
	ADCCON3 = ADCCON3_EREF_INTERNAL_1_25V |
	          ADCCON3_EDIV_512_DEC_12_BITS |
	          adc_channels[adc_channel_index];
	// Clear the interrupt flag
	ADCIF = 0;
	// Enable the interrupt which will advance
	// to the next sample
	ADCIE = 1;
}

void adc_wait(void) {
	// Wait for the sample ready bit to be set
	// by the interrupt
	while (!adc_sample_ready);
}

void adc_complete_isr() __interrupt (ADC_VECTOR) __using (1) {
	// Read out the 12 bit sample
	adc_buffer[adc_channel_index++] = (ADCH << 4) | (ADCL >> 4);

	if (adc_channel_index >= ADC_NUM_CHANNELS) {
		// If we're done sampling all channels, set the sample
		// to "ready" and stop
		adc_sample_ready = 1;
		// Disable this interrupt just to be safe
		// (it shouldn't trigger without writing to
		// ADCCON3)
		ADCIE = 0;
	} else {
		// Start the next sample using the internal reference
		// in 512 decimation rate mode (12 bit samples)
		ADCCON3 = ADCCON3_EREF_INTERNAL_1_25V |
		          ADCCON3_EDIV_512_DEC_12_BITS |
		          adc_channels[adc_channel_index];
	}
}
