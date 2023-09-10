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

// Radio payload signature checks
#pragma codeseg HOME
#include "signature.h"
#include "compiler_utils.h"
#include "cc1110_regs.h"
#include "dma.h"
#include "flash.h"
#include "stringx.h"

STATIC_ASSERT(flash_size_no_aes_padding, (FLASH_APP_SIGNATURE - FLASH_APP_START) % AES_BLOCK_SIZE == 0);

// The key region contains SIGNATURE_KEY_COUNT keys
// that all default to all ones (FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
__code __at (FLASH_SIGNATURE_KEYS) signature_key_t signature_keys[SIGNATURE_KEY_COUNT];
__code __at (FLASH_APP_SIGNATURE) signature_key_t flash_signature;

static void aes_load_block(__xdata void *addr, uint8_t enccs) {
	dma_configure_source_addr(dma_channel_aes_in, addr);
	while (!(ENCCS & ENCCS_RDY));
	dma_arm(dma_channel_aes_in);
	
	S0CON &= ~(S0CON_ENCIF);
	ENCCS = enccs | ENCCS_ST;
	dma_wait(dma_channel_aes_in);
	while (!(S0CON & S0CON_ENCIF));
}

__bit signature_app_valid(void) {
	__bit signature_match;
	__xdata signature_key_t signature;
	uint8_t i, z;
	uint8_t enccs;
	__xdata uint8_t *app_chunk;
	// TODO: I tried declaring this globally and it looked
	// like there is an issue where the address
	// loaded into the DMA peripheral was zero. I really
	// think it might be something wrong in the linker
	// but more debugging is warranted.
	__xdata uint8_t zeros[AES_BLOCK_SIZE];

	for (z = 0; z < AES_BLOCK_SIZE; z++) {
		zeros[z] = 0;
	}

	// Configure the DMA output channel for the AES peripheral to output
	// signatures to the signature variable
	dma_configure_transfer(
		// This is the "AES out" DMA channel
		dma_channel_aes_out,
		// Data is read from ENCDO (AES output register)
		&X_ENCDO,
		// and written to signature
		(__xdata void *) signature,
		// 8 bit transfer size per the peripheral spec
		DMA_WORDSIZE_8_BIT |
		// Do a single 16 byte transfer each time we ARM
		DMA_TMODE_SINGLE |
		// Trigger on the AES upload DMA event
		DMA_TRIG_ENC_UP,
		// Don't increment the source address (it's ENCDO)
		DMA_SRCINC_ZERO |
		// Single increment after each byte written to signature
		DMA_DESTINC_ONE |
		// High priority since we don't want the timing
		// to depend on RF/UART events and this is a
		// blocking routine
		DMA_PRIORITY_HIGH);
	// The DMA transfer length is one AES-128 block (16 bytes)
	dma_configure_length(
		dma_channel_aes_out,
		DMA_VLEN_FIXED_USE_LEN,
		sizeof(signature));

	// We're going to try all keys rather than short-circuiting
	// after finding a match to provide insulation against
	// potential timing attacks
	signature_match = 0;

	// The DMA AES input channel reads in one AES-128 block at
	// a time, so we can configure the length here for all
	// transfers
	dma_configure_transfer(
		dma_channel_aes_in,
		0,  // Leave the address pointer blank for now
		&X_ENCDI,
		DMA_WORDSIZE_8_BIT |
		DMA_TMODE_SINGLE |
		DMA_TRIG_ENC_DW,
		DMA_SRCINC_ONE |
		DMA_DESTINC_ZERO |
		DMA_PRIORITY_HIGH);
	dma_configure_length(
		dma_channel_aes_in,
		DMA_VLEN_FIXED_USE_LEN,
		AES_BLOCK_SIZE);

	for (i = 0; i < SIGNATURE_KEY_COUNT; i++) {
		// Load the key into the AES peripheral
		aes_load_block(
			(__xdata void *) signature_keys[i].key,
			ENCCS_MODE_CBC_MAC | ENCCS_CMD_LOAD_KEY);

		// Load the nonce (all zeros)
		aes_load_block(
			(__xdata void *) zeros,
			ENCCS_MODE_CBC_MAC | ENCCS_CMD_LOAD_IV_NONCE);

		// Compute the CBC-MAC of the application
		// First configure DMA transfers of one AES block
		// at a time into from the flash to the AES peripheral
		for (app_chunk = (__xdata uint8_t *) FLASH_APP_START;
		     app_chunk < (__xdata uint8_t *) FLASH_APP_SIGNATURE;
		     app_chunk += AES_BLOCK_SIZE) {

			dma_configure_source_addr(dma_channel_aes_in, app_chunk);
			// The first N-1 blocks (of N) need to be in CBC_MAC mode.
			// This mode does not produce an output. For the last
			// block, we switch to CBC mode which will output the ciphertext
			// of block N, which is the CBC-MAC signature
			if (app_chunk < (__xdata uint8_t *) (FLASH_APP_SIGNATURE - AES_BLOCK_SIZE)) {
				// Not the last page - kick off a transfer in CBC MAC mode
				enccs = ENCCS_MODE_CBC_MAC | ENCCS_CMD_ENCRYPT_BLOCK;
			} else {
				// Last page - set up the upload of the MAC
				// and run in mode CBC to read out the signature
				dma_arm(dma_channel_aes_out);
				enccs = ENCCS_MODE_CBC | ENCCS_CMD_ENCRYPT_BLOCK;
			}
			aes_load_block(app_chunk, enccs);
		}
		// Wait for the signature to be read out of the AES peripheral
		dma_wait(dma_channel_aes_out);
		// dprintbin((__xdata void *) &signature, 16);
		// Compare to the in-flash signature
		if (memcmpx_ct((__xdata void *) signature, (__xdata void *) flash_signature, AES_BLOCK_SIZE) == 0) {
			signature_match = 1;
		}
	}
	return signature_match;
}
