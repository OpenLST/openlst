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
#include "flash.h"
#include "board_defaults.h"
#include "cc1110_regs.h"
#include "compiler_utils.h"
#include "dma.h"
#include "hwid.h"
#include "stringx.h"
#include "flash_trigger.h"

// Sanity checks in case we override flash.h settings
// Flash app is page-aligned
STATIC_ASSERT(flash_app_aligned, FLASH_APP_START % FLASH_PAGE_SIZE == 0);
// Storage is page-aligned
STATIC_ASSERT(flash_storage_aligned, FLASH_STORAGE_START % FLASH_PAGE_SIZE == 0);
// Flash app doesn't overlap storage
STATIC_ASSERT(flash_app_no_overlap, FLASH_APP_END < FLASH_STORAGE_START);
// Flash app > 0 pages
STATIC_ASSERT(flash_app_not_empty, FLASH_APP_END > FLASH_APP_START);
// Flash app is after the bootloader
STATIC_ASSERT(flash_app_after_bootloader, FLASH_APP_START > FLASH_BOOTLOADER_END);
// App fits in flash
STATIC_ASSERT(flash_app_fits, FLASH_APP_END < FLASH_SIZE); 


void flash_erase_app(void) __critical {
	uint16_t addr;

	for (addr = FLASH_APP_START; addr <= FLASH_APP_END; addr += FLASH_PAGE_SIZE) {
		// Make sure the flash controller is done with previous erases/writes
		while (FCTL & FCTL_BUSY);

		// Set the flash frequency (the CC1110 needs to know what divisor to use
		// based on the oscillator we are using)
		FWT = FWT_FREQUENCY;
		// The flash address registers are word addressed so it needs to be
		// divided by 2
		FADDRH = (addr >> 9) & FADDRH_PAGE_BITS;
		FADDRL = 0;

		//There seems to be some poorly-documented silicon issue related
		//to alignment of the instruction that triggers the flash erase
		//and of the following instructions.  The symptom of doing this
		//incorrectly is that the for loop doesn't flow as you would
		//expect, not all flash pages get erased, and therefore the time
		//to execute the "flash erase app section" command is
		//substantially shorter than the usual ~600ms.

		//To achieve the proper alignment, the flash_trigger_erase() and
		//flash_trigger_write() functions are written in assembly and
		//included in start.asm, where they are placed in the VECTOR
		//section that the linker puts in a known location.
		//All they actually do is set a bit in FCTL.

		flash_trigger_erase();
		// Wait for completion
		while (FCTL & FCTL_BUSY);
	}

}

uint8_t flash_write(uint16_t addr, __xdata uint8_t *buf, uint16_t len) {

	FWT = FWT_FREQUENCY;

	// FADDR is word addressed (2 byte words)
	FADDRH = addr >> 9;
	FADDRL = (addr >> 1) & 0xff;

	// Set up DMA
	dma_configure_transfer(
		// We are configuring the Flash DMA channel
		dma_channel_flash_write,
		// Data is transfered from the provided buffer
		buf,
		// Data is transfered to the flash write data register
		&X_FWDATA,
		// Per the datasheet, transfer 1 byte at a time
		DMA_WORDSIZE_8_BIT |
		// Also per datasheet, use single transfer mode
		DMA_TMODE_SINGLE |
		// Trigger on the flash controller
		DMA_TRIG_FLASH,
		// After each byte, increment our buffer offset by 1
		DMA_SRCINC_ONE |
		// Do not increment the pointer to FWDATA
		DMA_DESTINC_ZERO |
		// We use the interrupt flag to check for completion
		DMA_IRQMASK_ENABLE |
		// We don't need to set the M8 bit because this is a
		// fixed length transfer.
		// This one is done at high priority since we're just
		// going to spin waiting for it to complete
		DMA_PRIORITY_HIGH
	);
	// Fixed length transfer of size FLASH_WRITE_PAGE_SIZE
	dma_configure_length(
		dma_channel_flash_write,
		DMA_VLEN_FIXED_USE_LEN,
		len);

	// Wait for the flash controller to be ready
	while (FCTL & FCTL_BUSY);

	// Start the transfer
	dma_arm(dma_channel_flash_write);
	// Enable the write
	flash_trigger_write();

	// Wait for the DMA transfer to complete
	dma_wait(dma_channel_flash_write);

	// Wait for the flash to be ready
	while (FCTL & (FCTL_BUSY | FCTL_SWBSY));
	return FLASH_WRITE_OK;
}


uint8_t flash_write_app_page(uint8_t page, __xdata uint8_t *buf) {
	uint16_t addr;
	addr = page * FLASH_WRITE_PAGE_SIZE;

	if (addr < FLASH_APP_START) {
		// Don't attempt to write over ourself
		return FLASH_WRITE_ERR_PROTECTED;
	} else if (addr + FLASH_WRITE_PAGE_SIZE <= FLASH_APP_END + 1) {
		return flash_write(addr, buf, FLASH_WRITE_PAGE_SIZE);
	} else if (addr + FLASH_WRITE_PAGE_SIZE <= FLASH_SIZE) {
		// Don't attempt to write over ourself or the nonvolatile region
		return FLASH_WRITE_ERR_PROTECTED;
	} else {
		// Don't attempt to write past the end of flash
		return FLASH_WRITE_ERR_BAD_ADDR;
	}
}
