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

// Radio setup and interrupt service routines
#include "board_defaults.h"
#include "cc1110_regs.h"
#include "crc16.h"
#include "dma.h"
#include "radio.h"
#include "stringx.h"

#ifndef BOOTLOADER
#include "timers.h"
#pragma codeseg APP_UPDATER
#endif

// Radio register settings for transmit and receive
uint8_t radio_mode_tx;
uint8_t radio_mode_rx;

// Telemetry on the last packet
volatile int8_t radio_last_rssi = -128;
volatile uint8_t radio_last_lqi = 0;
volatile int8_t radio_last_freqest = 0;
volatile __xdata uint32_t radio_cs_count = 0;

volatile __bit rf_rx_underway, rf_rx_complete;

__xdata rf_buffer_t rf_rx_buffer;
__xdata rf_buffer_t rf_tx_buffer;

__xdata uint16_t last_rx_ticks;

__xdata uint32_t radio_packets_sent;
__xdata uint32_t radio_packets_good;
__xdata uint32_t radio_packets_rejected_checksum;
__xdata uint32_t radio_packets_rejected_reserved;
__xdata uint32_t radio_packets_rejected_other;

volatile __bit rf_mode_tx = 0;  // controls whether the rftxrx ISR is transmitting or receiving

void radio_set_modes(uint8_t rx_mode, uint8_t tx_mode) {
  // Get register sets from board-specific functionality
	radio_mode_rx = rx_mode;
	radio_mode_tx = tx_mode;
}


static void radio_rx_setup_dma(void) {
	// Set up DMA
	// DMA is used instead of the RFRXTX interrupt, the RF interrupt is still
	// active and monitors for the sync words and end of packets
	// Packet length settings are configured in radio_set_modes()

	// Make sure the trigger interrupts are disabled (probably unnecessary)
	RFTXRXIE = 0;

	// The rf_rx channel transfers data from the radio data register
	// to the rx buffer in memory once a packet starts
	dma_configure_transfer(
		// We are configuring the Radio RX DMA channel
		dma_channel_rf,
		// Data is transfered from the RFD register (radio in)
		&X_RFD,
		// Data is transfered to the RX Buffer
		&rf_rx_buffer.data[(PKTCTRL0 & PKTCTRL0_LENGTH_CONFIG_VARIABLE) ? 0 : 1],
		// RFD produces one byte at a time
		DMA_WORDSIZE_8_BIT |
		// Single transfer (don't automatically rearm, we rearm manually
		// after each call to radio_listen())
		DMA_TMODE_SINGLE |
		// Trigger on the radio RFRXTX interrupt (read each byte
		// when the radio has data)
		DMA_TRIG_RADIO,
		// Do not increment the pointer to RFD
		DMA_SRCINC_ZERO |
		// After each byte, increment our offset in the radio RX buffer
		// by one
		DMA_DESTINC_ONE |
		// No interrupt is generated when the transfer is done
		DMA_IRQMASK_DISABLE |
		// In variable length modes, use all 8 bits of the first
		// byte as the length (rather than just 7)
		DMA_M8_ALL8 |
		// Normal memory priority means this channel round-robins
		// with the CPU and other channels when they compete for
		// memory access
		DMA_PRIORITY_NORMAL);
}

static void radio_tx_setup_dma(void) {
	// Set up DMA
	// DMA is used instead of the RFRXTX interrupt, the RF interrupt is still
	// active and monitors for the sync words and end of packets
	// Packet length settings are configured in radio_set_modes()

	// Make sure the trigger interrupts are disabled (probably unnecessary)
	RFTXRXIE = 0;

	// The rf_tx channel transfers data from the tx buffer in memory
	// to the radio data register when a packet is transmitted
	dma_configure_transfer(
		// We are configuring the Radio TX DMA channel
		dma_channel_rf,
		// Data is transfered from the TX Buffer
		&rf_tx_buffer.data[(PKTCTRL0 & PKTCTRL0_LENGTH_CONFIG_VARIABLE) ? 0 : 1],
		// Data is transfered to the RFD register (radio out)
		&X_RFD,
		// RFD accepts one byte at a time
		DMA_WORDSIZE_8_BIT |
		// Single transfer (don't automatically rearm, we rearm manually
		// when a packet is ready)
		DMA_TMODE_SINGLE |
		// Trigger on the radio RFRXTX interrupt (send each byte
		// when the radio is ready for data)
		DMA_TRIG_RADIO,
		// After each byte, increment our offset in the radio TX buffer
		// by one
		DMA_SRCINC_ONE |
		// Do not increment the pointer to RFD
		DMA_DESTINC_ZERO |
		// No interrupt is generated when the transfer is done
		DMA_IRQMASK_DISABLE |
		// In variable length modes, use all 8 bits of the first
		// byte as the length (rather than just 7)
		DMA_M8_ALL8 |
		// Normal memory priority means this channel round-robins
		// with the CPU and other channels when they compete for
		// memory access
		DMA_PRIORITY_NORMAL);
}

static void radio_setup_dma_len(void) {
	// Configure the length settings for DMA
	if (PKTCTRL0 & PKTCTRL0_LENGTH_CONFIG_VARIABLE) {
		dma_configure_length(
			dma_channel_rf,
			// Transfer the length byte + N bytes
			DMA_VLEN_VARIABLE_PLUS_1,
			// Maximum transfer size is the size of the buffer
			RF_BUFFER_SIZE);
	} else {
		dma_configure_length(
			dma_channel_rf,
			// Transfer the length byte + N bytes
			DMA_VLEN_FIXED_USE_LEN,
			// Fixed transfer size
			PKTLEN);
	}
}

void radio_init(void) {
	// Reset the radio to the idle state
	RFST = RFST_SIDLE;

	last_rx_ticks = 0;
	// TODO: default channel
	radio_packets_sent = 0;
	radio_packets_good = 0;
	radio_packets_rejected_checksum = 0;
	radio_packets_rejected_reserved = 0;
	radio_packets_rejected_other = 0;
	// TODO: default channel?
	radio_set_modes(RADIO_MODE_DEFAULT_RX, RADIO_MODE_DEFAULT_TX);
}


uint8_t radio_get_message(__xdata command_t *cmd, uint8_t *uart_sel) {
	uint8_t rf_pkt_length;
	uint8_t msg_length;
	__xdata rf_message_footer_t *footer;
	// If there is no packet ready just return 0
	if (!rf_rx_complete) {
		return 0;
	}
	// Make sure the packet is long enough to be parseable
	rf_pkt_length = rf_rx_buffer.header.length;
	if (rf_pkt_length < sizeof(rf_rx_buffer.header.flags) + // A flags field must be included
	                    sizeof(cmd->header) +  // The packet must have enough data to fill a command struct
	                    sizeof(*footer) - // The packet must include the CRC footer
	                    sizeof(footer->hwid)) {  // The HWID is already accounted for in the command header
		// If not just drop it
		radio_packets_rejected_other++;
		rf_rx_complete = 0;
		return 0;
	}
	// The footer is at the end of the message
	footer = (__xdata rf_message_footer_t *) &rf_rx_buffer.data[rf_pkt_length +
	                                                            sizeof(rf_rx_buffer.header.length) -
	                                                            sizeof(*footer)];

	// Next we compute the size of the message inside the RF packet
	// Several RF fields are not included in the command structure:
	msg_length = rf_pkt_length -
	             sizeof(rf_rx_buffer.header.flags) -  // The flags byte is not passed through
	             sizeof(*footer) +  // The CRC in the footer is dropped
	             sizeof(footer->hwid);  // However the HWID is included (moved to the beginning)
	// Now copy the message to the cmd struct. This will include
	// the length and flags bytes at the beginning. These get overwritten
	// later.
    
    // Check CRCs
	// TODO Print this over the UART?
	if (PKTCTRL0 & PKTCTRL0_LENGTH_CONFIG_VARIABLE) {
		if (crc16(&rf_rx_buffer.header.length,
			      rf_pkt_length - sizeof(footer->crc) + sizeof(rf_rx_buffer.header.length)) != footer->crc) {
			rf_rx_complete = 0;
			radio_packets_rejected_checksum++;
			radio_listen();
			return 0;
		}
	} else {
		if (crc16(&rf_rx_buffer.header.flags,
			      rf_pkt_length - sizeof(footer->crc)) != footer->crc) {
			rf_rx_complete = 0;
			radio_packets_rejected_checksum++;
			radio_listen();
			return 0;
		}
	}

	// This is a good packet - update the last received time
	last_rx_ticks = 0;

	memcpyx((__xdata void *) cmd,
	        (__xdata void *) rf_rx_buffer.data,
	        msg_length);
	// Now overwrite the length/flags bytes with the HWID
	cmd->header.hwid = footer->hwid;
	*uart_sel = (rf_rx_buffer.header.flags & FLAGS_UART_SEL) ? 1 : 0;
	rf_rx_complete = 0;
        radio_packets_good++;
	return msg_length;
}

// RF ISR: Packet SFD or DONE (or other RF events, see datasheet p. 188)
void rf_isr(void)  __interrupt (RF_VECTOR) __using (1) {
	S1CON = 0;  // Clear RFIF_1 and RFIF_2
	if (RFIF & RFIF_IM_TXUNF) {
		rf_mode_tx = 0;
		// TODO: error flag
	}
	if (RFIF & RFIF_IM_DONE && rf_mode_tx) {
		rf_mode_tx = 0;
	}
	if (RFIF & RFIF_IM_DONE && !rf_mode_tx) {
		rf_rx_complete = 1;
		RFIF = (uint8_t)~RFIF_IM_DONE;
		radio_last_rssi = *((int8_t *) &RSSI);
		radio_last_lqi = LQI;
		radio_last_freqest = *((int8_t *) &FREQEST);
	}
	if (RFIF & RFIF_IM_SFD && !rf_mode_tx) {
		// RX SFD - Packet reception begun (sync word detected)
		rf_rx_underway = 1;
	}
	if (RFIF & RFIF_IM_CS) {
		radio_cs_count++;
	}
	RFIF = 0;
}

void radio_listen(void) {
	// Drop to the IDLE state
	// If we hit any error states (like underflow/overflow)
	// this will also clear that error
	IEN2 &= ~IEN2_RFIE;
	RFST = RFST_SIDLE;
	
	#if BOARD_HAS_RX_HOOK == 1
	board_pre_rx();
	#endif

	board_apply_radio_settings(radio_mode_rx);

	// Abort any ongoing DMA transaction (RX or TX) on our channel
	dma_abort(dma_channel_rf);

	radio_rx_setup_dma();
	radio_setup_dma_len();

	// Clear the TX/RX interrupt flag
	RFTXRXIF = 0;

	// Arm the DMA transfer
	dma_arm(dma_channel_rf);

	// Clear all RF IRQ flags
	RFIF = 0;

	// Set the interrupt mask
	RFIM = RFIM_IM_DONE |  // Packet received or transmitted
	       RFIM_IM_SFD |   // Start of frame (sync word) detected
	       RFIM_IM_CS;     // Carrier sense (for telemetry)
	IEN2 |= IEN2_RFIE;

	rf_mode_tx = 0;
	rf_rx_underway = 0;
	rf_rx_complete = 0;

	#ifndef BOOTLOADER
	// Set timer 1 to watch for the frame start and capture
	// precise timing for ranging
	timers_watch_for_RF();
	#endif

	RFST = RFST_SRX;
}


void radio_send_packet(const __xdata command_t* cmd, uint8_t len,
                       __bit precise_timing, uint8_t uart_sel) {
	__xdata rf_message_footer_t *footer;
	uint8_t rf_extras;
	uint8_t rf_msg_len;
	#ifndef BOOTLOADER
	if (precise_timing) {
		// Enable the timer interrupt now. The interrupt will send STX
		// after the specified number of counts.
		// This assumes we can get the reply ready before the specified
		// number of ticks occur. Otherwise we will experience an underflow.
		// The default is two 1ms ticks which is plenty of time.
		timers_trigger_for_RF();
	}
	#else
	precise_timing;  // To avoid compiler warnings about this variable
	#endif

	// Make sure the packet isn't too big
	// The RF packet adds a footer. The length byte does not include itself.
	rf_extras = sizeof(*footer) - sizeof(rf_tx_buffer.header.length);
	if (len > RF_BUFFER_SIZE - rf_extras) {
		// TODO logging?
		return;
	}
	rf_msg_len = len + rf_extras;

	// Drop to the IDLE state
	// If we hit any error states (like underflow/overflow)
	// this will also clear that error
	IEN2 &= ~IEN2_RFIE;
	RFST = RFST_SIDLE;

	board_apply_radio_settings(radio_mode_tx);

	// Abort any ongoing DMA transaction (RX or TX) on our channel
	dma_abort(dma_channel_rf);

	radio_tx_setup_dma();
	radio_setup_dma_len();

	// Clear the buffer just to be safe
	memsetx(rf_tx_buffer.data, 0, RF_BUFFER_SIZE);
	// First, copy in the command to the RF buffer
	memcpyx((void __xdata *) rf_tx_buffer.data, (void __xdata *) cmd, len);
	// Find the footer location
	footer = (__xdata rf_message_footer_t *) &rf_tx_buffer.data[len];
	rf_tx_buffer.header.flags = uart_sel ? FLAGS_UART1_SEL : FLAGS_UART0_SEL;
	// Copy the HWID over to the footer
	footer->hwid = cmd->header.hwid;
	if (PKTCTRL0 & PKTCTRL0_LENGTH_CONFIG_VARIABLE) {
		// Set the length byte
		rf_tx_buffer.header.length = rf_msg_len;
		// Compute the CRC starting at the length byte
		footer->crc = crc16(&rf_tx_buffer.header.length,
		                    rf_msg_len - sizeof(footer->crc) +
		                    sizeof(rf_tx_buffer.header.length));
	} else {
		rf_tx_buffer.header.length = PKTLEN;
		// Compute the CRC starting at the flags byte (skip length)
		footer->crc = crc16(&rf_tx_buffer.header.flags,
		                    rf_msg_len - sizeof(footer->crc));
	}


	#if BOARD_HAS_TX_HOOK == 1
	board_pre_tx();
	#endif

	RFIM = RFIM_IM_TXUNF | // TX Underflow
	       RFIM_IM_DONE;

	RFTXRXIF = 0;
	RFTXRXIE = 0;  // ensure that RFTXRX interrupt is not used
	RFIF = 0;
	IEN2 |= IEN2_RFIE;

	dma_arm(dma_channel_rf);

	// Start transmitting now if we aren't using the timer interrupt
	// to control the transmit time
	rf_mode_tx = 1;
	#ifdef BOOTLOADER
	RFST = RFST_STX;
	#else
	if (!precise_timing) {
		RFST = RFST_STX;
	}
	#endif

	while(rf_mode_tx); // Block until TX complete

	radio_listen();
	radio_packets_sent++;
}