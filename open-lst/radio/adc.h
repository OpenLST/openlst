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

#ifndef _ADC_H
#define _ADC_H

#include "cc1110_regs.h"

#define ADC_NUM_CHANNELS 10

void adc_init(void);
void adc_start_sample(void);
void adc_wait(void);
void adc_complete_isr() __interrupt (ADC_VECTOR) __using (1);

extern volatile __bit adc_sample_ready;
extern volatile int16_t adc_buffer[ADC_NUM_CHANNELS];

#endif
