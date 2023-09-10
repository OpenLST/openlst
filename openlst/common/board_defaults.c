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

// Hardware initialization and setup
#ifdef BOOTLOADER
#pragma codeseg APP_UPDATER
#endif
#include <stdint.h>
#include <cc1110.h>
#include "cc1110_regs.h"
#include "board_defaults.h"

// TODO: are there GPIOs that need to be initialized on boot?
// TODO: frequency overrides so we don't have to copy paste this whole thing

#ifndef BOARD_RF_SETTINGS


uint8_t board_apply_radio_settings(uint8_t mode) {
	// The default settings are for 437Mhz 7kbps with FEC
	// These were derived from RF Studio using the following inputs:
	// Base Frequency: 437.000305 Mhz
	// Xtal Frequency: 27.000000 Mhz
	// Modulation Format: 2-FSK
	// Whitening: Yes
	// Channel Number: 0
	// Data Rate: 7.41577 kBaud
	// Deviation: 3.707866 kHz
	// Channel Spacing: 363.372803
	// RX Filter BW: 60.267857 kHz
	// TX Power: 10dBm
	// Carrier Frequency: 437.000305
	// Manchester Enable: No
	// PA Ramping: No
	// Sync Word: 30/32 sync word bits detected
	// Address Config: No address check
	// Addr: 0
	// Length Config: Variable packet length mode


	// We use variable length packets so this sets the maximum packet length
	// in RX mode
	PKTLEN = 255;
	// The preamble quality threshold is set to 4, meaning the number of
	// good bits must be at least 4 more than 8x the number of mismatched bits.
	// Address checks are not used (the CC1110 only supports 1 byte addresses so
	// we do this ourselves)
	// Status bytes are not used
	PKTCTRL1 =  PKTCTRL1_PQT_4 |
	            PKTCTRL1_NO_APPEND_STATUS |
	            PKTCTRL1_ADDR_CHECK_NONE;
	// Data is passed through the whitening filter to reduce DC bias.
	// The "normal" format is used (the other option is random test mode)
	// The CRC is disabled because we do our own two byte CRC
	// This mode uses variable length packets
	PKTCTRL0 =  PKTCTRL0_WHITE_DATA_WHITENING_ENABLED |
                PKTCTRL0_PKT_FORMAT_NORMAL |
                PKTCTRL0_CRC_DISABLED |
                PKTCTRL0_LENGTH_CONFIG_VARIABLE;

	// The offset channel is set to zero. This can be set elsewhere to support
	// multiple channels
	CHANNR =    0;
	// These default to the CC1110 defaults but can be overridden by the project
	SYNC0 =     RF_SYNC_WORD0;
	SYNC1 =     RF_SYNC_WORD1;
	// See above for the constants - this sets the channel bandwidth to 60.267kHz
	// And the channel data rate to 7415 baud
	MDMCFG4 =   RF_CHAN_BW_E << MDMCFG4_CHANBW_E_SHIFT |
	            RF_CHAN_BW_M << MDMCFG4_CHANBW_M_SHIFT |
	            RF_DRATE_E << MDMCFG4_DRATE_E_SHIFT;
	MDMCFG3 =   RF_DRATE_M << MDMCFG3_DRATE_M_SHIFT;
	// DC blocking before the demodulator is enabled
	// This mode is 2-FSK without Manchester encoding
	// The sync word is 32 bits with a minimum of 30 matching
	MDMCFG2 =   MDMCFG2_DEM_DCFILT_OFF_ENABLE |
	            MDMCFG2_MOD_FORMAT_2_FSK |
	            MDMCFG2_MANCHESTER_DISABLED |
	            MDMCFG2_SYNC_MODE_30_32;
	MDMCFG1 =   MDMCFG1_FEC_ENABLED |
	            MDMCFG1_NUM_PREAMBLE_4 |
	            RF_CHANSPC_E << MDMCFG1_CHANSPC_E_SHIFT;
	MDMCFG0 =   RF_CHANSPC_M << MDMCFG0_CHANSPC_M_SHIFT;
	DEVIATN =   RF_DEVIATN_M << DEVIATN_M_SHIFT |
	            RF_DEVIATN_E << DEVIATN_E_SHIFT;
	// Disable RX timeout based on carrier-sense
    // or other options and just wait until the
	// end of the packet
	MCSM2 =     MCSM2_RX_TIME_END_OF_PACKET;
	// The channel is considered clear if the
	// RSSI is below the threshold unless we
	// are currently receiving a packet
	MCSM1 =     MCSM1_CCA_MODE_RSSI_BELOW_THRESH |
	            MCSM1_CCA_MODE_UNLESS_RX_PACKET;
	// Run autocal before leaving the IDLE state
	// for RX or TX. Don't use the CLOSE_IN_RX option
	// (no attenuation). The source of the SmartRF magic
	// bits is unknown but matches LST2.
	MCSM0 =     MCSM0_FS_AUTOCAL_FROM_IDLE |
	            MCSM0_SMARTRF_1_ON |
	            MCSM0_SMARTRF_1_OFF |
	            MCSM0_CLOSE_IN_RX_0DB;
	// Let the frequency offset compensation and clock
	// recovery feedback loops run even when there is
	// no carrier sense. The frequency compensation loop
	// uses a gain of 3K before the sync word and
	// K/2 after the sync word. A max of BW_chan/2 is the
	// largest available compensation setting.
	// I assume these parameters were chosen with RFStudio
	// but I don't really know the source. The wide limit
	// on compensation makes sense given the range of
	// Doppler shift we expect to see.
	FOCCFG =    FOCCFG_FOC_BS_CS_GATE_NO_FREEZE |
	            FOCCFG_FOC_PRE_K_3K |
	            FOCCFG_FOC_POST_K_K_2 |
	            FOCCFG_FOC_LIMIT_BW_CHAN_2;
	// Bit synchronization for data rate is not enabled,
	// so the gain settings probably don't matter. These
	// were recommended by RF Studio
	BSCFG =     BSCFG_BS_PRE_KI_2KI |
	            BSCFG_BS_PRE_KP_3KP |
	            BSCFG_BS_POST_KI_KI_2 |
	            BSCFG_BS_POST_KP_KP_2 |
	            BSCFG_BS_LIMIT_NO_COMPENSATE;
	// Allow all AGC settings for the DVGA and LNA
	// Set a target amplitude of 33dB from the digital
	// channel filter (this is the default setting)
	AGCCTRL2 =  AGCCTRL2_MAX_DVGA_GAIN_ALL_SETTINGS |
	            AGCCTRL2_MAX_LNA_GAIN_MAX |
	            AGCCTRL2_MAGN_TARGET_33DB;
	// Use the default LNA first. The relative
	// threshold for carrier sense is not used (this is the
	// default). The absolute threshold is also the default
	// (equal to the MAGN_TARGET setting)
	AGCCTRL1 =  AGCCRTL1_AGC_LNA_PRIORITY_LNA_FIRST |
	            AGCCTRL1_CARRIER_SENSE_REL_THR_DISABLED |
	            AGCCTRL1_CARRIER_SENSE_ABS_THR_0DB;
	// These are all the default settings
	AGCCTRL0 =  AGCCTRL0_HYST_LEVEL_MEDIUM |
	            AGCCTRL0_WAIT_TIME_16 |
	            AGCCTRL0_AGC_FREEZE_NORMAL |
	            AGCCTRL0_FILTER_LENGTH_16;
	// Front end settings are just the defaults
	FREND1 =    FREND1_LNA_CURRENT_DEFAULT |
	            FREND1_LNA2MIX_CURRENT_DEFAULT |
	            FREND1_LODIV_BUF_CURRENT_RX_DEFAULT |
	            FREND1_MIX_CURRENT_DEFAULT ;
	FREND0 =    FREND1_LODIV_BUF_CURRENT_TX_DEFAULT |
	            FREND1_PA_POWER_DEFAULT;
	// I assume these settings were chosen in SmartRF studio
	// The "High" VCO is used (not sure why)
	// VCO calibration is enabled (this is the default)
	FSCAL3 =    (RF_FSCAL3_CONFIG & (uint8_t) ~FSCAL3_CHP_CURR_CAL_EN_MASK) |
	            (FSCAL3_DEFAULT & FSCAL3_CHP_CURR_CAL_EN_MASK);
	FSCAL2 =    FSCAL2_VCO_CORE_H_HIGH |
	            RF_FSCAL2_CONFIG << FSCAL2_FSCAL2_SHIFT;
	FSCAL1 =    RF_FSCAL1_CONFIG << FSCAL1_FSCAL1_SHIFT;
	FSCAL0 =    RF_FSCAL0_CONFIG << FSCAL0_FSCAL0_SHIFT;
	TEST2 =     TEST2_TEST2_DEFAULT;  // TODO: can use IMPROVE_RX?
	TEST1 =     TEST1_TEST1_TX | TEST1_TEST1_TX;  // TODO: can use IMPROVE_RX?
	TEST0 =     (RF_TEST0_CONFIG & (uint8_t) ~TEST0_VCO_SEL_CAL_EN) |
	            TEST0_VCO_SEL_CAL_EN;  // TODO: RF studio says leave this off
	PA_TABLE0 = RF_PA_CONFIG;
	// Other PA table settings are not used (no power ramping)
	switch (mode) {
		case amateur_rf_mode_437_7k_FEC:
			FSCTRL1 =   RF_FSCTRL1;
			FSCTRL0 =   RF_FSCTRL0;
			FREQ2 =     RF_FREQ2;
			FREQ1 =     RF_FREQ1;
			FREQ0 =     RF_FREQ0;
		break;
		#ifndef BOOTLOADER
		case amateur_rf_mode_437_10k_ranging:
			// Ranging packets use the same settings except that they
			// Include the 1 byte hardware-generated CRC since we
			// don't include our CRC in the data
			PKTCTRL0 =  PKTCTRL0_WHITE_DATA_WHITENING_ENABLED |
			            PKTCTRL0_PKT_FORMAT_NORMAL |
			            PKTCTRL0_CRC_ENABLED |
			            PKTCTRL0_LENGTH_CONFIG_VARIABLE;
			SYNC0 =     RF_SYNC_WORD0;  // TODO repeat of above
			SYNC1 =     RF_SYNC_WORD1;
			MDMCFG4 =   RF_CHAN_BW_RANGING_E << MDMCFG4_CHANBW_E_SHIFT |
			            RF_CHAN_BW_RANGING_M << MDMCFG4_CHANBW_M_SHIFT |
			            RF_DRATE_RANGING_E << MDMCFG4_DRATE_E_SHIFT;
			MDMCFG3 =   RF_DRATE_RANGING_M << MDMCFG3_DRATE_M_SHIFT;
			// DC blocking before the demodulator is enabled
			// This mode is GFSK without Manchester encoding
			// The sync word is 32 bits with a minimum of 30 matching
			MDMCFG2 =   MDMCFG2_DEM_DCFILT_OFF_ENABLE |
			            MDMCFG2_MOD_FORMAT_GFSK |
			            MDMCFG2_MANCHESTER_DISABLED |
			            MDMCFG2_SYNC_MODE_30_32;
			MDMCFG1 =   MDMCFG1_FEC_DISABLED |
			            MDMCFG1_NUM_PREAMBLE_4 |
			            RF_CHANSPC_RANGING_E << MDMCFG1_CHANSPC_E_SHIFT;
			MDMCFG0 =   RF_CHANSPC_RANGING_M << MDMCFG0_CHANSPC_M_SHIFT;
			DEVIATN =   RF_DEVIATN_RANGING_M << DEVIATN_M_SHIFT |
			            RF_DEVIATN_RANGING_E << DEVIATN_E_SHIFT;
			TEST2 =     TEST2_TEST2_DEFAULT;

			FSCTRL1 =   RF_FSCTRL1;
			FSCTRL0 =   RF_FSCTRL0;
			FREQ2 =     RF_FREQ2;
			FREQ1 =     RF_FREQ1;
			FREQ0 =     RF_FREQ0;
		break;
		#endif
		default:
			return RADIO_MODE_INVALID;
	}
	return RADIO_MODE_OK;
}

#endif