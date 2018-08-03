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

// DMA setup and interrupt handler routines
#ifdef BOOTLOADER
#pragma codeseg HOME
#endif
#include "dma.h"
#include "stringx.h"

// DMA configuration memory
__xdata dma_config_t dma_configs[DMA_NUM_CHANNELS];

// Set up the DMA peripheral
void dma_init(void) {
	// Set to 0 just to make sure all the triggers default to NONE
	memsetx((__xdata char *) dma_configs, 0, sizeof(dma_configs));

	// The DMA channel 0 configuration struct
	DMA0CFGH = DMA_ADDR_HIGH(&dma_configs[0]);
	DMA0CFGL = DMA_ADDR_LOW(&dma_configs[0]);
	// The DMA channels 1-4 configuration structs
	DMA1CFGH = DMA_ADDR_HIGH(&dma_configs[1]);
	DMA1CFGL = DMA_ADDR_LOW(&dma_configs[1]);
}

// Configure a DMA channel
void dma_configure_transfer(dma_channel_t channel,
                   __xdata uint8_t *src, __xdata uint8_t *dest,
                   uint8_t trig_cfg, uint8_t inc_cfg) {
	__xdata dma_config_t *cfg;

	cfg = &dma_configs[channel];
	cfg->src_h = DMA_ADDR_HIGH(src);
	cfg->src_l = DMA_ADDR_LOW(src);
	cfg->dest_h = DMA_ADDR_HIGH(dest);
	cfg->dest_l = DMA_ADDR_LOW(dest);
	cfg->trig_cfg = trig_cfg;
	cfg->inc_cfg = inc_cfg;
}

// Configure the length settings on a DMA channel
void dma_configure_length(dma_channel_t channel, uint8_t vlen, uint16_t len) {
	__xdata dma_config_t *cfg;
	cfg = &dma_configs[channel];

	cfg->len_h = ((len >> 8) & ~DMA_VLEN_MASK) | vlen;
	cfg->len_l = len & 0xff;
}

// Reconfigure just the source address
// This is especially useful for the AES input channel
// Which needs to pull data from several sources
// (key, IV, data)
void dma_configure_source_addr(dma_channel_t channel, __xdata uint8_t *src) {
	__xdata dma_config_t *cfg;

	cfg = &dma_configs[channel];
	cfg->src_h = DMA_ADDR_HIGH(src);
	cfg->src_l = DMA_ADDR_LOW(src);
}
