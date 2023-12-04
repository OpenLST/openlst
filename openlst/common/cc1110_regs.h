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

// Get rid of annoying warning (overflow in implicit constant conversion)
#pragma disable_warning 158

// Convenience definitions for CC1110 register bits and settings
#ifndef _CC1110_REGS_H
#define _CC1110_REGS_H

#include <cc1110.h>
#include <stdint.h>

// TODO: the "BITS" macros should go away or be a mask

// CLKCON - Clock Control
#define CLKCON_OSC32K        (1<<7)
#define CLKCON_OSC32K_RC     (1<<7)
#define CLKCON_OSC32K_XTAL   (0<<7)

#define CLKCON_OSC           (1<<6)
#define CLKCON_OSC_HSRC      (1<<6)
#define CLKCON_OSC_HSXTAL    (0<<6)

#define CLKCON_TICKSPD_F     (0<<3)
#define CLKCON_TICKSPD_F_2   (1<<3)
#define CLKCON_TICKSPD_F_4   (2<<3)
#define CLKCON_TICKSPD_F_8   (3<<3)
#define CLKCON_TICKSPD_F_16  (4<<3)
#define CLKCON_TICKSPD_F_32  (5<<3)
#define CLKCON_TICKSPD_F_64  (6<<3)
#define CLKCON_TICKSPD_F_128 (7<<3)

#define CLKCON_CLKSPD_F      (0<<0)
#define CLKCON_CLKSPD_F_2    (1<<0)
#define CLKCON_CLKSPD_F_4    (2<<0)
#define CLKCON_CLKSPD_F_8    (3<<0)
#define CLKCON_CLKSPD_F_16   (4<<0)
#define CLKCON_CLKSPD_F_32   (5<<0)
#define CLKCON_CLKSPD_F_64   (6<<0)
#define CLKCON_CLKSPD_F_128  (7<<0)


// SLEEP - Power Management
#define SLEEP_USB_EN         (1<<7)
#define SLEEP_USB_EN_ENABLE  (1<<7)
#define SLEEP_USB_EN_DISABLE (0<<7)
#define SLEEP_XOSC_STB       (1<<6)
#define SLEEP_HFRC_STB       (1<<5)
#define SLEEP_RST_BITS       (3<<3)
#define SLEEP_RST_POR_BOR    (0<<3)
#define SLEEP_RST_EXTERNAL   (1<<3)
#define SLEEP_RST_WATCHDOG   (2<<3)
#define SLEEP_OSC_PD         (1<<2)
#define SLEEP_OSC_MODE_BITS  (3<<0)
#define SLEEP_OSC_MODE_PM0   (0<<0)
#define SLEEP_OSC_MODE_PM1   (1<<0)
#define SLEEP_OSC_MODE_PM2   (2<<0)
#define SLEEP_OSC_MODE_PM3   (3<<0)


// IP0 - Interrupt Priorities
#define IP0_IPG5             (1<<5)
#define IP0_IPG4             (1<<4)
#define IP0_IPG3             (1<<3)
#define IP0_IPG2             (1<<2)
#define IP0_IPG1             (1<<1)
#define IP0_IPG0             (1<<0)


// IP1 - Interrupt Priorities
#define IP1_IPG5             (1<<5)
#define IP1_IPG4             (1<<4)
#define IP1_IPG3             (1<<3)
#define IP1_IPG2             (1<<2)
#define IP1_IPG1             (1<<1)
#define IP1_IPG0             (1<<0)

// IEN0 - Interrupt Enable 0 Register
#define IEN0_EA              (1<<7)
#define IEN0_STIE            (1<<5)
#define IEN0_ENCIE           (1<<4)
#define IEN0_URX1IE          (1<<3)
#define IEN0_I2SRXIE         (1<<3)
#define IEN0_URX0IE          (1<<2)
#define IEN0_ADCIE           (1<<1)
#define IEN0_RFTXRXIE        (1<<0)

// IEN1 - Interrupt Enable 1 Register
#define IEN1_P0IE            (1<<5)
#define IEN1_T4IE            (1<<4)
#define IEN1_T3IE            (1<<3)
#define IEN1_T2IE            (1<<2)
#define IEN1_T1IE            (1<<1)
#define IEN1_DMAIE           (1<<0)

// IEN2 - Interrupt Enable 2 Register
#define IEN2_WDTIE           (1<<5)
#define IEN2_P1IE            (1<<4)
#define IEN2_UTX1IE          (1<<3)
#define IEN2_I2STXIE         (1<<3)
#define IEN2_UTX0IE          (1<<2)
#define IEN2_P2IE            (1<<1)
#define IEN2_USBIE           (1<<1)
#define IEN2_RFIE            (1<<0)

// Radio Test Outputs (used for IOCFG0..2)
#define IOCFG_GDO_CFG_NONE                        0b000000
#define IOCFG_GDO_CFG_PREAMBLE_QUALITY            0b001000
#define IOCFG_GDO_CFG_CLEAR_CHANNEL               0b001001
#define IOCFG_GDO_CFG_CARRIER_SENSE               0b001110
#define IOCFG_GDO_CFG_CRC_OK                      0b001111
#define IOCFG_GDO_CFG_RX_HARD_DATA1               0b010110
#define IOCFG_GDO_CFG_RX_HARD_DATA2               0b010111
#define IOCFG_GDO_CFG_PA_PD                       0b011011
#define IOCFG_GDO_CFG_LNA_PD                      0b011100
#define IOCFG_GDO_CFG_RX_SYMBOL_TICK              0b011101
#define IOCFG_GDO_CFG_HW                          0b101111

// IOCFG2 - Radio Test Signal Configuration for P1_6
#define IOCFG2_GDO2_INV              (1<<6)
#define IOCFG2_GDO2_INV_ACTIVE_HIGH  (1<<6)
#define IOCFG2_GDO2_INV_ACTIVE_LOW   (0<<6)

// IOCFG1 - Radio Test Signal Configuration for P1_6
#define IOCFG1_GDO_DS                (1<<7)
#define IOCFG1_GDO_DS_MAXIMUM        (1<<7)
#define IOCFG1_GDO_DS_MINIMUM        (0<<7)
#define IOCFG1_GDO1_INV              (1<<6)
#define IOCFG1_GDO1_INV_ACTIVE_HIGH  (1<<6)
#define IOCFG1_GDO1_INV_ACTIVE_LOW   (0<<6)

// IOCFG0 - Radio Test Signal Configuration for P1_6
#define IOCFG0_GDO2_INV              (1<<6)
#define IOCFG0_GDO2_INV_ACTIVE_HIGH  (1<<6)
#define IOCFG0_GDO2_INV_ACTIVE_LOW   (0<<6)

// RFST - RF Strobe Commands
#define RFST_SFSTXON           0x00
#define RFST_SCAL              0x01
#define RFST_SRX               0x02
#define RFST_STX               0x03
#define RFST_SIDLE             0x04
#define RFST_SNOP              0xFF


// PKTCTRL1 - Packet Automation Control
#define PKTCTRL1_PQT_BITS                    (1<<5)
#define PKTCTRL1_PQT_0                       (0b000<<5)
#define PKTCTRL1_PQT_4                       (0b001<<5)
#define PKTCTRL1_PQT_8                       (0b010<<5)
#define PKTCTRL1_PQT_12                      (0b011<<5)
#define PKTCTRL1_PQT_16                      (0b100<<5)
#define PKTCTRL1_PQT_20                      (0b101<<5)
#define PKTCTRL1_PQT_24                      (0b110<<5)
#define PKTCTRL1_PQT_28                      (0b111<<5)
#define PKTCTRL1_APPEND_STATUS               (1<<2)
#define PKTCTRL1_NO_APPEND_STATUS            (0<<2)
#define PKTCTRL1_ADDR_CHECK_NONE             (0b00<<0)
#define PKTCTRL1_ADDR_CHECK_ADDR_NO_BCAST    (0b01<<0)
#define PKTCTRL1_ADDR_CHECK_ADDR_00_BCAST    (0b10<<0)
#define PKTCTRL1_ADDR_CHECK_ADDR_00_FF_BCAST (0b11<<0)

// PKTCTRL0 - Packet Automation Control
#define PKTCTRL0_WHITE_DATA                    (1<<6)
#define PKTCTRL0_WHITE_DATA_WHITENING_ENABLED  (1<<6)
#define PKTCTRL0_WHITE_DATA_WHITENING_DISABLED (0<<6)
#define PKTCTRL0_PKT_FORMAT_NORMAL             (0b00<<4)
#define PKTCTRL0_PKT_FORMAT_RANDOM             (0b10<<4)
#define PKTCTRL0_CRC_EN                        (1<<2)
#define PKTCTRL0_CRC_ENABLED                   (1<<2)
#define PKTCTRL0_CRC_DISABLED                  (0<<2)
#define PKTCTRL0_LENGTH_CONFIG_FIXED           (0b00<<0)
#define PKTCTRL0_LENGTH_CONFIG_VARIABLE        (0b01<<0)

// MDMCFG4 - Modem Configuration
#define MDMCFG4_CHANBW_E_SHIFT          (6)
#define MDMCFG4_CHANBW_E_BITS           (1<<6)
#define MDMCFG4_CHANBW_M_SHIFT          (4)
#define MDMCFG4_CHANBW_M_BITS           (1<<4)
#define MDMCFG4_DRATE_E_SHIFT           (0)
#define MDMCFG4_DRATE_E_BITS            (1<<0)
#define MDMCFG_CHAN_BW(f_ref, chanbw_m, chanbw_e) \
	(f_ref / (8 * (4 + chanbw_m) * (1 << chanbw_e)))

// MDMCFG3 - Modem Configuration
#define MDMCFG3_DRATE_M_SHIFT           (0)
#define MDMCFG3_DRATE_M_BITS            (1<<0)

// SDCC doesn't support these (it hits an integer overflow)
// #define MDMCFG_DRATE(f_ref, drate_m, drate_e)
// 	(((256 + drate_m) * (1 << drate_e) * (f_ref)) >> 28)
// #define MDMCFG_CHANSPC(f_ref, chanspc_m, chanspc_e)
// 	(((f_ref * (256 + chanspc_m) << chanspc_e)) >> 18)

// MDMCFG2 - Modem Configuration
#define MDMCFG2_DEM_DCFILT_OFF          (1<<7)
#define MDMCFG2_DEM_DCFILT_OFF_DISABLE  (1<<7)
#define MDMCFG2_DEM_DCFILT_OFF_ENABLE   (0<<7)
#define MDMCFG2_MOD_FORMAT_2_FSK        (0b000<<4)
#define MDMCFG2_MOD_FORMAT_GFSK         (0b001<<4)
#define MDMCFG2_MOD_FORMAT_ASK          (0b011<<4)
#define MDMCFG2_MOD_FORMAT_MSK          (0b111<<4)
#define MDMCFG2_MANCHESTER_EN           (1<<3)
#define MDMCFG2_MANCHESTER_ENABLED      (1<<3)
#define MDMCFG2_MANCHESTER_DISABLED     (0<<3)
#define MDMCFG2_SYNC_MODE_NONE          (0b000<<0)
#define MDMCFG2_SYNC_MODE_15_16         (0b001<<0)
#define MDMCFG2_SYNC_MODE_16_16         (0b010<<0)
#define MDMCFG2_SYNC_MODE_30_32         (0b011<<0)
#define MDMCFG2_SYNC_MODE_CS_ONLY       (0b100<<0)
#define MDMCFG2_SYNC_MODE_15_16_CS      (0b101<<0)
#define MDMCFG2_SYNC_MODE_16_16_CS      (0b110<<0)
#define MDMCFG2_SYNC_MODE_30_32_CS      (0b111<<0)

// MDMCFG1 - Modem Configuration
#define MDMCFG1_FEC_EN                  (1<<7)
#define MDMCFG1_FEC_ENABLED             (1<<7)
#define MDMCFG1_FEC_DISABLED            (0<<7)
#define MDMCFG1_NUM_PREAMBLE_2          (0b000<<4)
#define MDMCFG1_NUM_PREAMBLE_3          (0b001<<4)
#define MDMCFG1_NUM_PREAMBLE_4          (0b010<<4)
#define MDMCFG1_NUM_PREAMBLE_6          (0b011<<4)
#define MDMCFG1_NUM_PREAMBLE_8          (0b100<<4)
#define MDMCFG1_NUM_PREAMBLE_12         (0b101<<4)
#define MDMCFG1_NUM_PREAMBLE_16         (0b110<<4)
#define MDMCFG1_NUM_PREAMBLE_24         (0b111<<4)
#define MDMCFG1_CHANSPC_E_SHIFT         (0)
#define MDMCFG1_CHANSPC_E_BITS          (1<<0)

// MDMCFG0 - Modem Configuration
#define MDMCFG0_CHANSPC_M_SHIFT         (0)
#define MDMCFG0_CHANSPC_M_BITS          (1<<0)

// DEVIATN - Modem Deviation Setting
#define DEVIATN_E_SHIFT                 (4)
#define DEVIATN_E_BITS                  (0b111<<4)
#define DEVIATN_M_SHIFT                 (0)
#define DEVIATN_M_BITS                  (0b111<<0)

// MCSM2 - Main Radio Control State Machine Configuration
#define MCSM2_RX_TIME_RSSI               (1<<4)
#define MCSM2_RX_TIME_RSSI_DISABLED      (0<<4)
#define MCSM2_RX_TIME_RSSI_ENABLED       (1<<4)
#define MCSM2_RX_TIME_QUAL               (1<<3)
#define MCSM2_RX_TIME_QUAL_SYNC_WORD     (0<<3)
#define MCSM2_RX_TIME_QUAL_SYNC_OR_PQT   (1<<3)
#define MCSM2_RX_TIME_SHIFT              (0)
#define MCSM2_RX_TIME_END_OF_PACKET      (0b111<<0)

// MCSM1 - Main Radio Control State Machine Configuration
#define MCSM1_CCA_MODE_ALWAYS            (0b00<<4)
#define MCSM1_CCA_MODE_RSSI_BELOW_THRESH (0b01<<4)
#define MCSM1_CCA_MODE_UNLESS_RX_PACKET  (0b10<<4)
#define MCSM1_CCA_MODE_BOTH              (0b11<<4)
#define MCSM1_RXOFF_MODE_IDLE            (0b00<<2)
#define MCSM1_RXOFF_MODE_FSTXON          (0b01<<2)
#define MCSM1_RXOFF_MODE_TX              (0b10<<2)
#define MCSM1_RXOFF_MODE_STAY_RX         (0b11<<2)
#define MCSM1_TXOFF_MODE_IDLE            (0b00<<0)
#define MCSM1_TXOFF_MODE_FSTXON          (0b01<<0)
#define MCSM1_TXOFF_MODE_STAY_TX         (0b10<<0)
#define MCSM1_TXOFF_MODE_RX              (0b11<<0)

// MCSM0 - Main Radio Control State Machine Configuration
#define MCSM0_FS_AUTOCAL_NEVER           (0b00<<4)
#define MCSM0_FS_AUTOCAL_FROM_IDLE       (0b01<<4)
#define MCSM0_FS_AUTOCAL_TO_IDLE         (0b10<<4)
#define MCSM0_FS_AUTOCAL_EVERY_4TH_IDLE  (0b11<<4)
#define MCSM0_SMARTRF_1                  (1<<3)
#define MCSM0_SMARTRF_1_ON               (1<<3)
#define MCSM0_SMARTRF_1_OFF              (0<<3)
#define MCSM0_SMARTRF_0                  (1<<2)
#define MCSM0_SMARTRF_0_ON               (1<<2)
#define MCSM0_SMARTRF_0_OFF              (0<<2)
#define MCSM0_CLOSE_IN_RX_0DB            (0b00<<0)
#define MCSM0_CLOSE_IN_RX_6DB            (0b01<<0)
#define MCSM0_CLOSE_IN_RX_12DB           (0b10<<0)
#define MCSM0_CLOSE_IN_RX_18DB           (0b11<<0)

// FOCCFG - Frequency Offset Compensation Configuration
#define FOCCFG_FOC_BS_CS_GATE            (1<<5)
#define FOCCFG_FOC_BS_CS_GATE_CS_FREEZE  (1<<5)
#define FOCCFG_FOC_BS_CS_GATE_NO_FREEZE  (0<<5)
#define FOCCFG_FOC_PRE_K_K               (0b00<<3)
#define FOCCFG_FOC_PRE_K_2K              (0b01<<3)
#define FOCCFG_FOC_PRE_K_3K              (0b10<<3)
#define FOCCFG_FOC_PRE_K_4K              (0b11<<3)
#define FOCCFG_FOC_POST_K                (1<<2)
#define FOCCFG_FOC_POST_K_SAME_AS_PRE    (0<<2)
#define FOCCFG_FOC_POST_K_K_2            (1<<2)
#define FOCCFG_FOC_LIMIT_NO_COMPENSATE   (0b00<<0)
#define FOCCFG_FOC_LIMIT_BW_CHAN_8       (0b01<<0)
#define FOCCFG_FOC_LIMIT_BW_CHAN_4       (0b10<<0)
#define FOCCFG_FOC_LIMIT_BW_CHAN_2       (0b11<<0)

// BSCFG - Bit Synchronization Configuration
#define BSCFG_BS_PRE_KI_KI               (0b00<<6)
#define BSCFG_BS_PRE_KI_2KI              (0b01<<6)
#define BSCFG_BS_PRE_KI_3KI              (0b10<<6)
#define BSCFG_BS_PRE_KI_4KI              (0b11<<6)
#define BSCFG_BS_PRE_KP_KP               (0b00<<4)
#define BSCFG_BS_PRE_KP_2KP              (0b01<<4)
#define BSCFG_BS_PRE_KP_3KP              (0b10<<4)
#define BSCFG_BS_PRE_KP_4KP              (0b11<<4)
#define BSCFG_BS_POST_KI_SAME_AS_PRE     (0<<3)
#define BSCFG_BS_POST_KI_KI_2            (1<<3)
#define BSCFG_BS_POST_KP_SAME_AS_PRE     (0<<2)
#define BSCFG_BS_POST_KP_KP_2            (1<<2)
#define BSCFG_BS_LIMIT_NO_COMPENSATE     (0b00<<0)
#define BSCFG_BS_LIMIT_3_125_PCT         (0b01<<0)
#define BSCFG_BS_LIMIT_6_25_PCT          (0b10<<0)
#define BSCFG_BS_LIMIT_12_5_PCT          (0b11<<0)

// AGCCTRL2 - AGC Control
#define AGCCTRL2_MAX_DVGA_GAIN_SHIFT            (6)
#define AGCCTRL2_MAX_DVGA_GAIN_ALL_SETTINGS     (0b00<<6)
#define AGCCTRL2_MAX_DVGA_GAIN_NOT_HIGHEST      (0b01<<6)
#define AGCCTRL2_MAX_DVGA_GAIN_NOT_2_HIGHEST    (0b10<<6)
#define AGCCTRL2_MAX_DVGA_GAIN_NOT_3_HIGHEST    (0b11<<6)
#define AGCCTRL2_MAX_LNA_GAIN_SHIFT             (3)
#define AGCCTRL2_MAX_LNA_GAIN_MAX               (0b000<<3)
#define AGCCTRL2_MAX_LNA_GAIN_2_6DB_BELOW_MAX   (0b001<<3)
#define AGCCTRL2_MAX_LNA_GAIN_6_1DB_BELOW_MAX   (0b010<<3)
#define AGCCTRL2_MAX_LNA_GAIN_7_4DB_BELOW_MAX   (0b011<<3)
#define AGCCTRL2_MAX_LNA_GAIN_9_2DB_BELOW_MAX   (0b100<<3)
#define AGCCTRL2_MAX_LNA_GAIN_11_5DB_BELOW_MAX  (0b101<<3)
#define AGCCTRL2_MAX_LNA_GAIN_14_6DB_BELOW_MAX  (0b110<<3)
#define AGCCTRL2_MAX_LNA_GAIN_17_1DB_BELOW_MAX  (0b111<<3)
#define AGCCTRL2_MAGN_TARGET_SHIFT              (0)
#define AGCCTRL2_MAGN_TARGET_24DB               (0b000<<0)
#define AGCCTRL2_MAGN_TARGET_27DB               (0b001<<0)
#define AGCCTRL2_MAGN_TARGET_30DB               (0b010<<0)
#define AGCCTRL2_MAGN_TARGET_33DB               (0b011<<0)
#define AGCCTRL2_MAGN_TARGET_36DB               (0b100<<0)
#define AGCCTRL2_MAGN_TARGET_38DB               (0b101<<0)
#define AGCCTRL2_MAGN_TARGET_40DB               (0b110<<0)
#define AGCCTRL2_MAGN_TARGET_42DB               (0b111<<0)

// AGCCTRL1 - AGC Control
#define AGCCRTL1_AGC_LNA_PRIORITY               (1<<6)
#define AGCCRTL1_AGC_LNA_PRIORITY_LNA2_FIRST    (0<<6)
#define AGCCRTL1_AGC_LNA_PRIORITY_LNA_FIRST     (1<<6)
#define AGCCTRL1_CARRIER_SENSE_REL_THR_SHIFT    (4)
#define AGCCTRL1_CARRIER_SENSE_REL_THR_DISABLED (0b00<<4)
#define AGCCTRL1_CARRIER_SENSE_REL_THR_6DB      (0b01<<4)
#define AGCCTRL1_CARRIER_SENSE_REL_THR_10DB     (0b10<<4)
#define AGCCTRL1_CARRIER_SENSE_REL_THR_14DB     (0b11<<4)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_SHIFT    (0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_DISABLED (0b1000<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_NEG_7DB  (0b1001<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_NEG_6DB  (0b1010<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_NEG_5DB  (0b1011<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_NEG_4DB  (0b1100<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_NEG_3DB  (0b1101<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_NEG_2DB  (0b1110<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_NEG_1DB  (0b1111<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_0DB      (0b0000<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_POS_1DB  (0b0001<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_POS_2DB  (0b0010<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_POS_3DB  (0b0011<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_POS_4DB  (0b0100<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_POS_5DB  (0b0101<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_POS_6DB  (0b0110<<0)
#define AGCCTRL1_CARRIER_SENSE_ABS_THR_POS_7DB  (0b0111<<0)

// AGCCTRL0 - AGC Control
#define AGCCTRL0_HYST_LEVEL_SHIFT               (6)
#define AGCCTRL0_HYST_LEVEL_NONE                (0b00<<6)
#define AGCCTRL0_HYST_LEVEL_LOW                 (0b01<<6)
#define AGCCTRL0_HYST_LEVEL_MEDIUM              (0b10<<6)
#define AGCCTRL0_HYST_LEVEL_LARGE               (0b11<<6)
#define AGCCTRL0_WAIT_TIME_SHIFT                (4)
#define AGCCTRL0_WAIT_TIME_8                    (0b00<<4)
#define AGCCTRL0_WAIT_TIME_16                   (0b01<<4)
#define AGCCTRL0_WAIT_TIME_24                   (0b10<<4)
#define AGCCTRL0_WAIT_TIME_32                   (0b11<<4)
#define AGCCTRL0_AGC_FREEZE_SHIFT               (2)
#define AGCCTRL0_AGC_FREEZE_NORMAL              (0b00<<2)
#define AGCCTRL0_AGC_FREEZE_FREEZE_ON_SYNC      (0b01<<2)
#define AGCCTRL0_AGC_FREEZE_MANUAL_ANALOG       (0b10<<2)
#define AGCCTRL0_AGC_FREEZE_MANUAL_A_AND_D      (0b11<<2)
#define AGCCTRL0_FILTER_LENGTH_SHIFT            (0)
#define AGCCTRL0_FILTER_LENGTH_8                (0b00<<0)
#define AGCCTRL0_FILTER_LENGTH_16               (0b01<<0)
#define AGCCTRL0_FILTER_LENGTH_32               (0b10<<0)
#define AGCCTRL0_FILTER_LENGTH_64               (0b11<<0)

// FREND1 - Front End RX Configuration
#define FREND1_LNA_CURRENT_SHIFT                (6)
#define FREND1_LNA_CURRENT_DEFAULT              (0b01<<6)
#define FREND1_LNA2MIX_CURRENT_SHIFT            (4)
#define FREND1_LNA2MIX_CURRENT_DEFAULT          (0b01<<4)
#define FREND1_LODIV_BUF_CURRENT_RX_SHIFT       (2)
#define FREND1_LODIV_BUF_CURRENT_RX_DEFAULT     (0b01<<2)
#define FREND1_MIX_CURRENT_SHIFT                (0)
#define FREND1_MIX_CURRENT_DEFAULT              (0b10<<0)

// FREND0 - Front End TX Configuration
#define FREND1_LODIV_BUF_CURRENT_TX_SHIFT       (4)
#define FREND1_LODIV_BUF_CURRENT_TX_DEFAULT     (0b01<<4)
#define FREND1_PA_POWER_SHIFT                   (0)
#define FREND1_PA_POWER_DEFAULT                 (0b000<<0)

// FSCAL3 - Frequency Synthesizer Calibration
#define FSCAL3_DEFAULT                          (0b10101001)
#define FSCAL3_CHP_CURR_CAL_EN_MASK             (0b11<<4)

// FSCAL2 - Frequency Synthesizer Calibration
#define FSCAL2_VCO_CORE_H_EN                    (1<<5)
#define FSCAL2_VCO_CORE_H_LOW                   (0<<5)
#define FSCAL2_VCO_CORE_H_HIGH                  (1<<5)
#define FSCAL2_FSCAL2_SHIFT                     (0)
#define FSCAL2_FSCAL2_DEFAULT                   (0b01010<<0)

// FSCAL1 - Frequency Synthesizer Calibration

#define FSCAL1_FSCAL1_SHIFT                     (0)
#define FSCAL1_FSCAL1_DEFAULT                   (0b100000<<0)

// FSCAL0 - Frequency Synthesizer Calibration
#define FSCAL0_FSCAL0_SHIFT                     (0)
#define FSCAL0_FSCAL0_DEFAULT                   (0b0001101<<0)


// TEST2 - "Various Test Settings"
#define TEST2_TEST2_DEFAULT          0x88
#define TEST2_TEST2_IMPROVE_RX       0x81

// TEST1 - "Various Test Settings"
#define TEST1_TEST1_TX               0x31
#define TEST1_TEST1_IMPROVE_RX       0x35

// TEST0 - "Various Test Settings"
#define TEST0_DEFAULT          0b00001011
#define TEST0_VCO_SEL_CAL_EN   (1<<1)

// RFIM - RF Interrupt Mask
#define RFIM_IM_TXUNF     (1<<7)
#define RFIM_IM_RXOVF     (1<<6)
#define RFIM_IM_TIMEOUT   (1<<5)
#define RFIM_IM_DONE      (1<<4)
#define RFIM_IM_CS        (1<<3)
#define RFIM_IM_PQT       (1<<2)
#define RFIM_IM_CCA       (1<<1)
#define RFIM_IM_SFD       (1<<0)

// RFIF - RF Interrupt Flag
#define RFIF_IM_TXUNF     (1<<7)
#define RFIF_IM_RXOVF     (1<<6)
#define RFIF_IM_TIMEOUT   (1<<5)
#define RFIF_IM_DONE      (1<<4)
#define RFIF_IM_CS        (1<<3)
#define RFIF_IM_PQT       (1<<2)
#define RFIF_IM_CCA       (1<<1)
#define RFIF_IM_SFD       (1<<0)

// DMA Controller Configuration Structure
typedef struct {
	uint8_t src_h;     // High bits of the source address
	uint8_t src_l;     // Low bits of the source address
	uint8_t dest_h;    // High bits of the source address
	uint8_t dest_l;    // Low bits of the source address
	uint8_t len_h;     // Transfer count high bits (when applicable) and VLEN
	uint8_t len_l;     // Transfer count low bits (when applicable)
	uint8_t trig_cfg;  // TMODE and TRIG
	uint8_t inc_cfg;   // SRCINC and DESTINC
} dma_config_t;

#define DMA_NUM_CHANNELS 5

// DMA Configuration Structure Members
#define DMA_VLEN_MASK              (0b111<<5)
#define DMA_VLEN_FIXED_USE_LEN     (0b000<<5)
#define DMA_VLEN_VARIABLE_PLUS_1   (0b001<<5)
#define DMA_VLEN_VARIABLE          (0b010<<5)
#define DMA_VLEN_VARIABLE_PLUS_2   (0b011<<5)
#define DMA_VLEN_VARIABLE_PLUS_3   (0b100<<5)
#define DMA_VLEN_FIXED_USE_LEN_ALT (0b111<<5)

#define DMA_WORDSIZE               (1<<7)
#define DMA_WORDSIZE_8_BIT         (0<<7)
#define DMA_WORDSIZE_16_BIT        (1<<7)
#define DMA_TMODE_SINGLE           (0b00<<5)
#define DMA_TMODE_BLOCK            (0b01<<5)
#define DMA_TMODE_REPEATED_SINGLE  (0b10<<5)
#define DMA_TMODE_REPEATED_BLOCK   (0b11<<5)
#define DMA_TRIG_NONE              0
#define DMA_TRIG_PREV              1
#define DMA_TRIG_T1_CH0            2
#define DMA_TRIG_T1_CH1            3
#define DMA_TRIG_T1_CH2            4
#define DMA_TRIG_T2_OVFL           6
#define DMA_TRIG_T3_CH0            7
#define DMA_TRIG_T3_CH1            8
#define DMA_TRIG_T4_CH0            9
#define DMA_TRIG_T4_CH1            10
#define DMA_TRIG_IOC_0             12
#define DMA_TRIG_IOC_1             13
#define DMA_TRIG_URX0              14
#define DMA_TRIG_UTX0              15
#define DMA_TRIG_URX1              16
#define DMA_TRIG_UTX1              17
#define DMA_TRIG_FLASH             18
#define DMA_TRIG_RADIO             19
#define DMA_TRIG_ADC_CHALL         20
#define DMA_TRIG_ADC_CH0           21
#define DMA_TRIG_ADC_CH1           22
#define DMA_TRIG_ADC_CH2           23
#define DMA_TRIG_ADC_CH3           24
#define DMA_TRIG_ADC_CH4           25
#define DMA_TRIG_ADC_CH5           26
#define DMA_TRIG_ADC_CH6           27
#define DMA_TRIG_I2SRX             27
#define DMA_TRIG_ADC_CH7           28
#define DMA_TRIG_I2STX             28
#define DMA_TRIG_ENC_DW            29
#define DMA_TRIG_ENC_UP            30

#define DMA_SRCINC_ZERO            (0b00<<6)
#define DMA_SRCINC_ONE             (0b01<<6)
#define DMA_SRCINC_TWO             (0b10<<6)
#define DMA_SRCINC_MINUS_ONE       (0b11<<6)

#define DMA_DESTINC_ZERO           (0b00<<4)
#define DMA_DESTINC_ONE            (0b01<<4)
#define DMA_DESTINC_TWO            (0b10<<4)
#define DMA_DESTINC_MINUS_ONE      (0b11<<4)

#define DMA_IRQMASK                (1<<3)
#define DMA_IRQMASK_ENABLE         (1<<3)
#define DMA_IRQMASK_DISABLE        (0<<3)

#define DMA_M8                     (1<<2)
#define DMA_M8_ALL8                (0<<2)
#define DMA_M8_7LSB                (1<<2)

#define DMA_PRIORITY_LOW           (0b00<<0)
#define DMA_PRIORITY_NORMAL        (0b01<<0)
#define DMA_PRIORITY_HIGH          (0b10<<0)

// DMAARM - DMA Channel Arm
#define DMAARM_ABORT               (1<<7)
#define DMAARM_DMAARM4             (1<<4)
#define DMAARM_DMAARM3             (1<<3)
#define DMAARM_DMAARM2             (1<<2)
#define DMAARM_DMAARM1             (1<<1)
#define DMAARM_DMAARM0             (1<<0)

// DMAREQ - DMA Channel Start Request and Status 
#define DMAREQ_DMAREQ4             (1<<4)
#define DMAREQ_DMAREQ3             (1<<3)
#define DMAREQ_DMAREQ2             (1<<2)
#define DMAREQ_DMAREQ1             (1<<1)
#define DMAREQ_DMAREQ0             (1<<0)

// DMAIRQ - DMA Interrupt Flag
#define DMAIRQ_DMAIF4             (1<<4)
#define DMAIRQ_DMAIF3             (1<<3)
#define DMAIRQ_DMAIF2             (1<<2)
#define DMAIRQ_DMAIF1             (1<<1)
#define DMAIRQ_DMAIF0             (1<<0)

// FCTL - Flash Control

#define FCTL_BUSY                 (1<<7)
#define FCTL_SWBSY                (1<<6)
#define FCTL_CONTRD               (1<<4)
#define FCTL_CONTRD_ENABLE        (1<<4)
#define FCTL_CONTRD_DISABLE       (0<<4)
#define FCTL_WRITE                (1<<1)
#define FCTL_ERASE                (1<<0)

#define FADDRH_PAGE_BITS          (0b00111110)

// T1CTL - Timer 1 Control and Status
#define T1CTL_CH2IF               (1<<7)
#define T1CTL_CH1IF               (1<<6)
#define T1CTL_CH0IF               (1<<5)
#define T1CTL_OVFIF               (1<<4)
#define T1CTL_DIV_BITS            (1<<2)
#define T1CTL_DIV_1               (0b00<<2)
#define T1CTL_DIV_8               (0b01<<2)
#define T1CTL_DIV_32              (0b10<<2)
#define T1CTL_DIV_128             (0b11<<2)
#define T1CTL_MODE_BITS           (1<<0)
#define T1CTL_MODE_SUSPENDED      (0b00<<0)
#define T1CTL_MODE_FREE_RUN       (0b01<<0)
#define T1CTL_MODE_MODULO         (0b10<<0)
#define T1CTL_MODE_UP_DOWN        (0b11<<0)

// T1CCTL0 - Timer 1 Channel 0 Capture/Compare Control
#define T1CCTL0_CPSEL                  (1<<7)
#define T1CCTL0_CPSEL_NORMAL           (0<<7)
#define T1CCTL0_CPSEL_RF_EVENT         (1<<7)
#define T1CCTL0_IM                     (1<<6)
#define T1CCTL0_IM_ENABLED             (1<<6)
#define T1CCTL0_IM_DISABLED            (0<<6)
#define T1CCTL0_CMP_BITS               (1<<3)
#define T1CCTL0_CMP_SET_ON_COMPARE     (0b000<<3)
#define T1CCTL0_CMP_CLEAR_ON_COMPARE   (0b001<<3)
#define T1CCTL0_CMP_TOGGLE_ON_COMPARE  (0b010<<3)
#define T1CCTL0_CMP_SET_CMP_UP         (0b011<<3)
#define T1CCTL0_CMP_CLEAR_CMP_UP       (0b100<<3)
#define T1CCTL0_MODE                   (1<<2)
#define T1CCTL0_MODE_CAPTURE           (0<<2)
#define T1CCTL0_MODE_COMPARE           (1<<2)
#define T1CCTL0_CAP_BITS               (1<<0)
#define T1CCTL0_CAP_NO_CAPTURE         (0b00<<0)
#define T1CCTL0_CAP_RISING_EDGE        (0b01<<0)
#define T1CCTL0_CAP_FALLING_EDGE       (0b10<<0)
#define T1CCTL0_CAP_BOTH_EDGES         (0b11<<0)

// T1CCTL1 - Timer 1 Channel 1 Capture/Compare Control
#define T1CCTL1_CPSEL                  (1<<7)
#define T1CCTL1_CPSEL_NORMAL           (0<<7)
#define T1CCTL1_CPSEL_RF_EVENT         (1<<7)
#define T1CCTL1_IM                     (1<<6)
#define T1CCTL1_IM_ENABLED             (1<<6)
#define T1CCTL1_IM_DISABLED            (0<<6)
#define T1CCTL1_CMP_BITS               (1<<3)
#define T1CCTL1_CMP_SET_ON_COMPARE     (0b000<<3)
#define T1CCTL1_CMP_CLEAR_ON_COMPARE   (0b001<<3)
#define T1CCTL1_CMP_TOGGLE_ON_COMPARE  (0b010<<3)
#define T1CCTL1_CMP_SET_CMP_UP         (0b011<<3)
#define T1CCTL1_CMP_CLEAR_CMP_UP       (0b100<<3)
#define T1CCTL1_MODE                   (1<<2)
#define T1CCTL1_MODE_CAPTURE           (0<<2)
#define T1CCTL1_MODE_COMPARE           (1<<2)
#define T1CCTL1_CAP_BITS               (1<<0)
#define T1CCTL1_CAP_NO_CAPTURE         (0b00<<0)
#define T1CCTL1_CAP_RISING_EDGE        (0b01<<0)
#define T1CCTL1_CAP_FALLING_EDGE       (0b10<<0)
#define T1CCTL1_CAP_BOTH_EDGES         (0b11<<0)

// TIMIF - Timers 1/3/4 Interrupt Mask/Flag
#define TIMIF_OVFIM                    (1<<6)
#define TIMIF_T4CH1IF                  (1<<5)
#define TIMIF_T4CH0IF                  (1<<4)
#define TIMIF_T4OVFIF                  (1<<3)
#define TIMIF_T3CH1IF                  (1<<2)
#define TIMIF_T3CH0IF                  (1<<1)
#define TIMIF_T3OVFIF                  (1<<0)

// P2DIR - Port 2 Direction
#define P2DIR_PRIP0_BITS               (1<<6)
#define P2DIR_PRIP0_MASK               (0b11<<6)
#define P2DIR_PRIP0_USART0_USART1      (0b00<<6)
#define P2DIR_PRIP0_USART1_USART0      (0b01<<6)
#define P2DIR_PRIP0_T1CH01_USART1      (0b10<<6)
#define P2DIR_PRIP0_T1CH2_USART0       (0b11<<6)
#define P2DIR_DIRP2_4                  (1<<4)
#define P2DIR_DIRP2_3                  (1<<3)
#define P2DIR_DIRP2_2                  (1<<2)
#define P2DIR_DIRP2_1                  (1<<1)
#define P2DIR_DIRP2_0                  (1<<0)

// ENCCS - Encryption Control and Status
#define ENCCS_MODE_BITS                (1<<4)
#define ENCCS_MODE_CBC                 (0b000<<4)
#define ENCCS_MODE_CFB                 (0b001<<4)
#define ENCCS_MODE_OFB                 (0b010<<4)
#define ENCCS_MODE_CTR                 (0b011<<4)
#define ENCCS_MODE_ECB                 (0b100<<4)
#define ENCCS_MODE_CBC_MAC             (0b101<<4)
#define ENCCS_RDY                      (1<<3)
#define ENCCS_CMD_BITS                 (1<<1)
#define ENCCS_CMD_ENCRYPT_BLOCK        (0b00<<1)
#define ENCCS_CMD_DECRYPT_BLOCK        (0b01<<1)
#define ENCCS_CMD_LOAD_KEY             (0b10<<1)
#define ENCCS_CMD_LOAD_IV_NONCE        (0b11<<1)
#define ENCCS_ST                       (1<<0)

// S0CON - CPU Interrupt Flag 2
#define S0CON_ENCIF_1                  (1<<1)
#define S0CON_ENCIF_0                  (1<<0)
#define S0CON_ENCIF                    (0b11<<0)


// ADCCON1 - ADC Control 1
#define ADCCON1_EOC                    (1<<7)
#define ADCCON1_ST                     (1<<6)
#define ADCCON1_STSEL_EXTERNAL         (0b00<<4)
#define ADCCON1_STSEL_FULL_SPEED       (0b01<<4)
#define ADCCON1_STSEL_TIMER1_CH0       (0b10<<4)
#define ADCCON1_STSEL_ADCCON1_ST       (0b11<<4)
#define ADCCON1_RCTRL_OP_COMPLETED     (0b00<<2)
#define ADCCON1_RCTRL_CLOCK_LFSR_ONCE  (0b10<<2)
#define ADCCON1_RESERVED_DEFAULT       (0b11<<0)

// ADCCON2 - ADC Control 2
#define ADCCON2_SREF_INTERNAL_1_25V    (0b00<<6)
#define ADCCON2_SREF_EXTERNAL          (0b01<<6)
#define ADCCON2_SREF_AVDD              (0b10<<6)
#define ADCCON2_SREF_EXTERNAL_DIFF     (0b11<<6)
#define ADCCON2_SDIV_64_DEC_7_BITS     (0b00<<4)
#define ADCCON2_SDIV_128_DEC_9_BITS    (0b01<<4)
#define ADCCON2_SDIV_256_DEC_10_BITS   (0b10<<4)
#define ADCCON2_SDIV_512_DEC_12_BITS   (0b11<<4)
#define ADCCON2_SCH_AIN0               (0b0000<<0)
#define ADCCON2_SCH_AIN1               (0b0001<<0)
#define ADCCON2_SCH_AIN2               (0b0010<<0)
#define ADCCON2_SCH_AIN3               (0b0011<<0)
#define ADCCON2_SCH_AIN4               (0b0100<<0)
#define ADCCON2_SCH_AIN5               (0b0101<<0)
#define ADCCON2_SCH_AIN6               (0b0110<<0)
#define ADCCON2_SCH_AIN7               (0b0111<<0)
#define ADCCON2_SCH_AIN0_AIN1          (0b1000<<0)
#define ADCCON2_SCH_AIN2_AIN3          (0b1001<<0)
#define ADCCON2_SCH_AIN4_AIN5          (0b1010<<0)
#define ADCCON2_SCH_AIN6_AIN7          (0b1011<<0)
#define ADCCON2_SCH_GND                (0b1100<<0)
#define ADCCON2_SCH_POS_REFERENCE      (0b1101<<0)
#define ADCCON2_SCH_TEMPERATURE        (0b1110<<0)
#define ADCCON2_SCH_VDD_3              (0b1111<<0)

// ADCCON3 - ADC Control 3
#define ADCCON3_EREF_INTERNAL_1_25V    (0b00<<6)
#define ADCCON3_EREF_EXTERNAL          (0b01<<6)
#define ADCCON3_EREF_AVDD              (0b10<<6)
#define ADCCON3_EREF_EXTERNAL_DIFF     (0b11<<6)
#define ADCCON3_EDIV_64_DEC_7_BITS     (0b00<<4)
#define ADCCON3_EDIV_128_DEC_9_BITS    (0b01<<4)
#define ADCCON3_EDIV_256_DEC_10_BITS   (0b10<<4)
#define ADCCON3_EDIV_512_DEC_12_BITS   (0b11<<4)
#define ADCCON3_ECH_AIN0               (0b0000<<0)
#define ADCCON3_ECH_AIN1               (0b0001<<0)
#define ADCCON3_ECH_AIN2               (0b0010<<0)
#define ADCCON3_ECH_AIN3               (0b0011<<0)
#define ADCCON3_ECH_AIN4               (0b0100<<0)
#define ADCCON3_ECH_AIN5               (0b0101<<0)
#define ADCCON3_ECH_AIN6               (0b0110<<0)
#define ADCCON3_ECH_AIN7               (0b0111<<0)
#define ADCCON3_ECH_AIN0_AIN1          (0b1000<<0)
#define ADCCON3_ECH_AIN2_AIN3          (0b1001<<0)
#define ADCCON3_ECH_AIN4_AIN5          (0b1010<<0)
#define ADCCON3_ECH_AIN6_AIN7          (0b1011<<0)
#define ADCCON3_ECH_GND                (0b1100<<0)
#define ADCCON3_ECH_POS_REFERENCE      (0b1101<<0)
#define ADCCON3_ECH_TEMPERATURE        (0b1110<<0)
#define ADCCON3_ECH_VDD_3              (0b1111<<0)

#endif
