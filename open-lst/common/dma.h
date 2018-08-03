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

#ifndef _DMA_H
#define _DMA_H
#include <stdint.h>
#include "cc1110_regs.h"

typedef enum {
	dma_channel_flash_write = 0,
	dma_channel_rf = 1,
	dma_channel_aes_in = 2,
	dma_channel_aes_out = 3
} dma_channel_t;


#define DMA_ADDR_HIGH(p) (((uint16_t) (__xdata uint8_t *) (p)) >> 8)
#define DMA_ADDR_LOW(p) (((uint16_t) (__xdata uint8_t *) (p)) & 0xFF)

void dma_init(void);
void dma_configure_transfer(dma_channel_t channel,
                            __xdata uint8_t *src, __xdata uint8_t *dest,
                            uint8_t trig_cfg, uint8_t inc_cfg);
void dma_configure_length(dma_channel_t channel, uint8_t vlen, uint16_t len);

void dma_configure_source_addr(dma_channel_t channel, __xdata uint8_t *src);

#define dma_arm(channel) \
	DMAIRQ &= ~(1<<(channel)); \
	DMAARM |= (1<<(channel));

#define dma_abort(channel) \
	DMAARM = DMAARM_ABORT | (1<<(channel));

#define dma_wait(channel) \
	while (!(DMAIRQ & (1<<(channel))));

extern __xdata dma_config_t dma_configs[DMA_NUM_CHANNELS];

#endif
