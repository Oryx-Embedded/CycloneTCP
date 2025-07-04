/**
 * @file ksz9131_driver.h
 * @brief KSZ9131 Gigabit Ethernet PHY driver
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2025 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

#ifndef _KSZ9131_DRIVER_H
#define _KSZ9131_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef KSZ9131_PHY_ADDR
   #define KSZ9131_PHY_ADDR 3
#elif (KSZ9131_PHY_ADDR < 0 || KSZ9131_PHY_ADDR > 31)
   #error KSZ9131_PHY_ADDR parameter is not valid
#endif

//KSZ9131 PHY registers
#define KSZ9131_BMCR                                  0x00
#define KSZ9131_BMSR                                  0x01
#define KSZ9131_PHYID1                                0x02
#define KSZ9131_PHYID2                                0x03
#define KSZ9131_ANAR                                  0x04
#define KSZ9131_ANLPAR                                0x05
#define KSZ9131_ANER                                  0x06
#define KSZ9131_ANNPTR                                0x07
#define KSZ9131_ANNPRR                                0x08
#define KSZ9131_GBCR                                  0x09
#define KSZ9131_GBSR                                  0x0A
#define KSZ9131_MMDACR                                0x0D
#define KSZ9131_MMDAADR                               0x0E
#define KSZ9131_GBESR                                 0x0F
#define KSZ9131_RLB                                   0x11
#define KSZ9131_LINKMD                                0x12
#define KSZ9131_DPMAPCSS                              0x13
#define KSZ9131_RXERCTR                               0x15
#define KSZ9131_LED_MODE_SEL                          0x16
#define KSZ9131_LED_BEHAVIOR                          0x17
#define KSZ9131_MDIO_DRIVE                            0x19
#define KSZ9131_LEGACY_LED_MODE                       0x1A
#define KSZ9131_ICSR                                  0x1B
#define KSZ9131_AUTOMDI                               0x1C
#define KSZ9131_SPDC                                  0x1D
#define KSZ9131_EXT_LOOPBACK                          0x1E
#define KSZ9131_PHYCON                                0x1F

//KSZ9131 MMD registers
#define KSZ9131_MEAN_SLICER_ERROR                     0x01, 0xE1
#define KSZ9131_DCQ_MEAN_SQUARE_ERROR                 0x01, 0xE2
#define KSZ9131_DCQ_MEAN_SQUARE_ERROR_WORST_CASE      0x01, 0xE3
#define KSZ9131_DCQ_SQI                               0x01, 0xE4
#define KSZ9131_DCQ_PEAK_MSE                          0x01, 0xE5
#define KSZ9131_DCQ_CTRL                              0x01, 0xE6
#define KSZ9131_DCQ_CONFIG                            0x01, 0xE7
#define KSZ9131_DCQ_SQI_TABLE                         0x01, 0xE8
#define KSZ9131_COMMON_CTRL                           0x02, 0x00
#define KSZ9131_STRAP_STAT                            0x02, 0x01
#define KSZ9131_OP_MODE_STRAP_OVERRIDE                0x02, 0x02
#define KSZ9131_OP_MODE_STRAP                         0x02, 0x03
#define KSZ9131_CLOCK_INV_CTRL_SIGNAL_PAD_SKEW        0x02, 0x04
#define KSZ9131_RGMII_RX_DATA_PAD_SKEW                0x02, 0x05
#define KSZ9131_RGMII_TX_DATA_PAD_SKEW                0x02, 0x06
#define KSZ9131_CLK_PAD_SKEW                          0x02, 0x08
#define KSZ9131_SELF_TEST_PKT_COUNT_LO                0x02, 0x09
#define KSZ9131_SELF_TEST_PKT_COUNT_HI                0x02, 0x0A
#define KSZ9131_SELF_TEST_STAT                        0x02, 0x0B
#define KSZ9131_SELF_TEST_FRAME_COUNT_EN              0x02, 0x0C
#define KSZ9131_SELF_TEST_PGEN_EN                     0x02, 0x0D
#define KSZ9131_SELF_TEST_EN                          0x02, 0x0E
#define KSZ9131_WOL_CTRL                              0x02, 0x10
#define KSZ9131_WOL_MAC_LO                            0x02, 0x11
#define KSZ9131_WOL_MAC_MI                            0x02, 0x12
#define KSZ9131_WOL_MAC_HI                            0x02, 0x13
#define KSZ9131_CUSTOM_PKT0_CRC_LO                    0x02, 0x14
#define KSZ9131_CUSTOM_PKT0_CRC_HI                    0x02, 0x15
#define KSZ9131_CUSTOM_PKT1_CRC_LO                    0x02, 0x16
#define KSZ9131_CUSTOM_PKT1_CRC_HI                    0x02, 0x17
#define KSZ9131_CUSTOM_PKT2_CRC_LO                    0x02, 0x18
#define KSZ9131_CUSTOM_PKT2_CRC_HI                    0x02, 0x19
#define KSZ9131_CUSTOM_PKT3_CRC_LO                    0x02, 0x1A
#define KSZ9131_CUSTOM_PKT3_CRC_HI                    0x02, 0x1B
#define KSZ9131_CUSTOM_PKT0_MASK_LL                   0x02, 0x1C
#define KSZ9131_CUSTOM_PKT0_MASK_LH                   0x02, 0x1D
#define KSZ9131_CUSTOM_PKT0_MASK_HL                   0x02, 0x1E
#define KSZ9131_CUSTOM_PKT0_MASK_HH                   0x02, 0x1F
#define KSZ9131_CUSTOM_PKT1_MASK_LL                   0x02, 0x20
#define KSZ9131_CUSTOM_PKT1_MASK_LH                   0x02, 0x21
#define KSZ9131_CUSTOM_PKT1_MASK_HL                   0x02, 0x22
#define KSZ9131_CUSTOM_PKT1_MASK_HH                   0x02, 0x23
#define KSZ9131_CUSTOM_PKT2_MASK_LL                   0x02, 0x24
#define KSZ9131_CUSTOM_PKT2_MASK_LH                   0x02, 0x25
#define KSZ9131_CUSTOM_PKT2_MASK_HL                   0x02, 0x26
#define KSZ9131_CUSTOM_PKT2_MASK_HH                   0x02, 0x27
#define KSZ9131_CUSTOM_PKT3_MASK_LL                   0x02, 0x28
#define KSZ9131_CUSTOM_PKT3_MASK_LH                   0x02, 0x29
#define KSZ9131_CUSTOM_PKT3_MASK_HL                   0x02, 0x2A
#define KSZ9131_CUSTOM_PKT3_MASK_HH                   0x02, 0x2B
#define KSZ9131_WOL_CTRL_STAT                         0x02, 0x2C
#define KSZ9131_WOL_CUSTOM_PKT_RECEIVE_STAT           0x02, 0x2D
#define KSZ9131_WOL_MAGIC_PKT_RECEIVE_STAT            0x02, 0x2E
#define KSZ9131_WOL_DATA_MODULE_STAT                  0x02, 0x2F
#define KSZ9131_CUSTOM_PKT0_RCVD_CRC_L                0x02, 0x30
#define KSZ9131_CUSTOM_PKT0_RCVD_CRC_H                0x02, 0x31
#define KSZ9131_CUSTOM_PKT1_RCVD_CRC_L                0x02, 0x32
#define KSZ9131_CUSTOM_PKT1_RCVD_CRC_H                0x02, 0x33
#define KSZ9131_CUSTOM_PKT2_RCVD_CRC_L                0x02, 0x34
#define KSZ9131_CUSTOM_PKT2_RCVD_CRC_H                0x02, 0x35
#define KSZ9131_CUSTOM_PKT3_RCVD_CRC_L                0x02, 0x36
#define KSZ9131_CUSTOM_PKT3_RCVD_CRC_H                0x02, 0x37
#define KSZ9131_SELF_TEST_CORRECT_COUNT_LO            0x02, 0x3C
#define KSZ9131_SELF_TEST_CORRECT_COUNT_HI            0x02, 0x3D
#define KSZ9131_SELF_TEST_ERROR_COUNT_LO              0x02, 0x3E
#define KSZ9131_SELF_TEST_ERROR_COUNT_HI              0x02, 0x3F
#define KSZ9131_SELF_TEST_BAD_SFD_COUNT_LO            0x02, 0x40
#define KSZ9131_SELF_TEST_BAD_SFD_COUNT_HI            0x02, 0x41
#define KSZ9131_RX_DLL_CTRL                           0x02, 0x4C
#define KSZ9131_TX_DLL_CTRL                           0x02, 0x4D
#define KSZ9131_PCS_CTRL1                             0x03, 0x00
#define KSZ9131_PCS_STAT1                             0x03, 0x01
#define KSZ9131_EEE_QUIET_TIMER                       0x03, 0x08
#define KSZ9131_EEE_UPDATE_TIMER                      0x03, 0x09
#define KSZ9131_EEE_LINK_FAIL_TIMER                   0x03, 0x0A
#define KSZ9131_EEE_POST_UPDATE_TIMER                 0x03, 0x0B
#define KSZ9131_EEE_WAIT_WQ_TIMER                     0x03, 0x0C
#define KSZ9131_EEE_WAKE_TIMER                        0x03, 0x0D
#define KSZ9131_EEE_WAKE_TX_TIMER                     0x03, 0x0E
#define KSZ9131_EEE_WAKE_MZ_TIMER                     0x03, 0x0F
#define KSZ9131_EEE_CTRL_CAPABILITY                   0x03, 0x14
#define KSZ9131_EEE_WAKE_ERROR_COUNTER                0x03, 0x16
#define KSZ9131_EEE_100_TIMER0                        0x03, 0x18
#define KSZ9131_EEE_100_TIMER1                        0x03, 0x19
#define KSZ9131_EEE_100_TIMER2                        0x03, 0x1A
#define KSZ9131_EEE_100_TIMER3                        0x03, 0x1B
#define KSZ9131_EEE_ADV                               0x07, 0x3C
#define KSZ9131_EEE_LP_ABILITY                        0x07, 0x3D
#define KSZ9131_EEE_LP_ABILITY_OVERRIDE               0x07, 0x3E
#define KSZ9131_EEE_MSG_CODE                          0x07, 0x3F
#define KSZ9131_XTAL_CTRL                             0x1C, 0x01
#define KSZ9131_AFED_CTRL                             0x1C, 0x09
#define KSZ9131_LDO_CTRL                              0x1C, 0x0E
#define KSZ9131_EDPD_CTRL                             0x1C, 0x24
#define KSZ9131_EMITX_CTRL                            0x1C, 0x25
#define KSZ9131_EMITX_COEFF                           0x1C, 0x26
#define KSZ9131_MMD31_REG19                           0x1F, 0x13

//Basic Control register
#define KSZ9131_BMCR_RESET                            0x8000
#define KSZ9131_BMCR_LOOPBACK                         0x4000
#define KSZ9131_BMCR_SPEED_SEL_LSB                    0x2000
#define KSZ9131_BMCR_AN_EN                            0x1000
#define KSZ9131_BMCR_POWER_DOWN                       0x0800
#define KSZ9131_BMCR_ISOLATE                          0x0400
#define KSZ9131_BMCR_RESTART_AN                       0x0200
#define KSZ9131_BMCR_DUPLEX_MODE                      0x0100
#define KSZ9131_BMCR_COL_TEST                         0x0080
#define KSZ9131_BMCR_SPEED_SEL_MSB                    0x0040

//Basic Status register
#define KSZ9131_BMSR_100BT4                           0x8000
#define KSZ9131_BMSR_100BTX_FD                        0x4000
#define KSZ9131_BMSR_100BTX_HD                        0x2000
#define KSZ9131_BMSR_10BT_FD                          0x1000
#define KSZ9131_BMSR_10BT_HD                          0x0800
#define KSZ9131_BMSR_100BT2_FD                        0x0400
#define KSZ9131_BMSR_100BT2_HD                        0x0200
#define KSZ9131_BMSR_EXTENDED_STATUS                  0x0100
#define KSZ9131_BMSR_UNIDIRECTIONAL_ABLE              0x0080
#define KSZ9131_BMSR_MF_PREAMBLE_SUPPR                0x0040
#define KSZ9131_BMSR_AN_COMPLETE                      0x0020
#define KSZ9131_BMSR_REMOTE_FAULT                     0x0010
#define KSZ9131_BMSR_AN_CAPABLE                       0x0008
#define KSZ9131_BMSR_LINK_STATUS                      0x0004
#define KSZ9131_BMSR_JABBER_DETECT                    0x0002
#define KSZ9131_BMSR_EXTENDED_CAPABLE                 0x0001

//PHY Identifier 1 register
#define KSZ9131_PHYID1_PHY_ID_MSB                     0xFFFF
#define KSZ9131_PHYID1_PHY_ID_MSB_DEFAULT             0x0022

//PHY Identifier 2 register
#define KSZ9131_PHYID2_PHY_ID_LSB                     0xFC00
#define KSZ9131_PHYID2_PHY_ID_LSB_DEFAULT             0x1400
#define KSZ9131_PHYID2_MODEL_NUM                      0x03F0
#define KSZ9131_PHYID2_MODEL_NUM_DEFAULT              0x0240
#define KSZ9131_PHYID2_REVISION_NUM                   0x000F

//Auto-Negotiation Advertisement register
#define KSZ9131_ANAR_NEXT_PAGE                        0x8000
#define KSZ9131_ANAR_REMOTE_FAULT                     0x2000
#define KSZ9131_ANAR_EXTENDED_NEXT_PAGE               0x1000
#define KSZ9131_ANAR_ASYM_PAUSE                       0x0800
#define KSZ9131_ANAR_SYM_PAUSE                        0x0400
#define KSZ9131_ANAR_100BT4                           0x0200
#define KSZ9131_ANAR_100BTX_FD                        0x0100
#define KSZ9131_ANAR_100BTX_HD                        0x0080
#define KSZ9131_ANAR_10BT_FD                          0x0040
#define KSZ9131_ANAR_10BT_HD                          0x0020
#define KSZ9131_ANAR_SELECTOR                         0x001F
#define KSZ9131_ANAR_SELECTOR_DEFAULT                 0x0001

//Auto-Negotiation Link Partner Ability register
#define KSZ9131_ANLPAR_NEXT_PAGE                      0x8000
#define KSZ9131_ANLPAR_ACK                            0x4000
#define KSZ9131_ANLPAR_REMOTE_FAULT                   0x2000
#define KSZ9131_ANLPAR_EXTENDED_NEXT_PAGE             0x1000
#define KSZ9131_ANLPAR_ASYM_PAUSE                     0x0800
#define KSZ9131_ANLPAR_PAUSE                          0x0400
#define KSZ9131_ANLPAR_100BT4                         0x0200
#define KSZ9131_ANLPAR_100BTX_FD                      0x0100
#define KSZ9131_ANLPAR_100BTX_HD                      0x0080
#define KSZ9131_ANLPAR_10BT_FD                        0x0040
#define KSZ9131_ANLPAR_10BT_HD                        0x0020
#define KSZ9131_ANLPAR_SELECTOR                       0x001F
#define KSZ9131_ANLPAR_SELECTOR_DEFAULT               0x0001

//Auto-Negotiation Expansion register
#define KSZ9131_ANER_RECEIVE_NP_LOC_ABLE              0x0040
#define KSZ9131_ANER_RECEIVE_NP_STOR_LOC              0x0020
#define KSZ9131_ANER_PAR_DETECT_FAULT                 0x0010
#define KSZ9131_ANER_LP_NEXT_PAGE_ABLE                0x0008
#define KSZ9131_ANER_NEXT_PAGE_ABLE                   0x0004
#define KSZ9131_ANER_PAGE_RECEIVED                    0x0002
#define KSZ9131_ANER_LP_AN_ABLE                       0x0001

//Auto-Negotiation Next Page TX register
#define KSZ9131_ANNPTR_NEXT_PAGE                      0x8000
#define KSZ9131_ANNPTR_MSG_PAGE                       0x2000
#define KSZ9131_ANNPTR_ACK2                           0x1000
#define KSZ9131_ANNPTR_TOGGLE                         0x0800
#define KSZ9131_ANNPTR_MESSAGE                        0x07FF

//Auto-Negotiation Next Page RX register
#define KSZ9131_ANNPRR_NEXT_PAGE                      0x8000
#define KSZ9131_ANNPRR_ACK                            0x4000
#define KSZ9131_ANNPRR_MSG_PAGE                       0x2000
#define KSZ9131_ANNPRR_ACK2                           0x1000
#define KSZ9131_ANNPRR_TOGGLE                         0x0800
#define KSZ9131_ANNPRR_MESSAGE                        0x07FF

//Auto-Negotiation Master Slave Control register
#define KSZ9131_GBCR_TEST_MODE                        0xE000
#define KSZ9131_GBCR_MS_MAN_CONF_EN                   0x1000
#define KSZ9131_GBCR_MS_MAN_CONF_VAL                  0x0800
#define KSZ9131_GBCR_PORT_TYPE                        0x0400
#define KSZ9131_GBCR_1000BT_FD                        0x0200
#define KSZ9131_GBCR_1000BT_HD                        0x0100

//Auto-Negotiation Master Slave Status register
#define KSZ9131_GBSR_MS_CONF_FAULT                    0x8000
#define KSZ9131_GBSR_MS_CONF_RES                      0x4000
#define KSZ9131_GBSR_LOCAL_RECEIVER_STATUS            0x2000
#define KSZ9131_GBSR_REMOTE_RECEIVER_STATUS           0x1000
#define KSZ9131_GBSR_LP_1000BT_FD                     0x0800
#define KSZ9131_GBSR_LP_1000BT_HD                     0x0400
#define KSZ9131_GBSR_IDLE_ERR_COUNT                   0x00FF

//MMD Access Control register
#define KSZ9131_MMDACR_FUNC                           0xC000
#define KSZ9131_MMDACR_FUNC_ADDR                      0x0000
#define KSZ9131_MMDACR_FUNC_DATA_NO_POST_INC          0x4000
#define KSZ9131_MMDACR_FUNC_DATA_POST_INC_RW          0x8000
#define KSZ9131_MMDACR_FUNC_DATA_POST_INC_W           0xC000
#define KSZ9131_MMDACR_DEVAD                          0x001F

//Extended Status register
#define KSZ9131_GBESR_1000BX_FD                       0x8000
#define KSZ9131_GBESR_1000BX_HD                       0x4000
#define KSZ9131_GBESR_1000BT_FD                       0x2000
#define KSZ9131_GBESR_1000BT_HD                       0x1000

//Remote Loopback register
#define KSZ9131_RLB_REMOTE_LOOPBACK                   0x0100

//LinkMD Cable Diagnostic register
#define KSZ9131_LINKMD_TEST_EN                        0x8000
#define KSZ9131_LINKMD_TX_DIS                         0x4000
#define KSZ9131_LINKMD_PAIR                           0x3000
#define KSZ9131_LINKMD_PAIR_A                         0x0000
#define KSZ9131_LINKMD_PAIR_B                         0x1000
#define KSZ9131_LINKMD_PAIR_C                         0x2000
#define KSZ9131_LINKMD_PAIR_D                         0x3000
#define KSZ9131_LINKMD_SEL                            0x0C00
#define KSZ9131_LINKMD_STATUS                         0x0300
#define KSZ9131_LINKMD_STATUS_NORMAL                  0x0000
#define KSZ9131_LINKMD_STATUS_OPEN                    0x0100
#define KSZ9131_LINKMD_STATUS_SHORT                   0x0200
#define KSZ9131_LINKMD_STATUS_TEST_FAILED             0x0300
#define KSZ9131_LINKMD_FAULT_DATA                     0x00FF

//Digital PMA/PCS Status register
#define KSZ9131_DPMAPCSS_1000BT_LINK_STATUS           0x0002
#define KSZ9131_DPMAPCSS_100BTX_LINK_STATUS           0x0001

//LED Mode Select register
#define KSZ9131_LED_MODE_SEL_LED2_CONFIG              0x00F0
#define KSZ9131_LED_MODE_SEL_LED1_CONFIG              0x000F

//LED Behavior register
#define KSZ9131_LED_BEHAVIOR_LED_ACT_OUT_SEL          0x4000
#define KSZ9131_LED_BEHAVIOR_LED_PULSING_EN           0x1000
#define KSZ9131_LED_BEHAVIOR_LED_BLINK_RATE           0x0C00
#define KSZ9131_LED_BEHAVIOR_LED_BLINK_RATE_2_5HZ     0x0000
#define KSZ9131_LED_BEHAVIOR_LED_BLINK_RATE_5HZ       0x0400
#define KSZ9131_LED_BEHAVIOR_LED_BLINK_RATE_10HZ      0x0800
#define KSZ9131_LED_BEHAVIOR_LED_BLINK_RATE_20HZ      0x0C00
#define KSZ9131_LED_BEHAVIOR_LED_PULSE_STRECH_EN      0x0060
#define KSZ9131_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED2 0x0040
#define KSZ9131_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED1 0x0020
#define KSZ9131_LED_BEHAVIOR_LED_COMBINATION_DIS      0x0003
#define KSZ9131_LED_BEHAVIOR_LED_COMBINATION_DIS_LED2 0x0002
#define KSZ9131_LED_BEHAVIOR_LED_COMBINATION_DIS_LED1 0x0001

//MDIO Drive register
#define KSZ9131_MDIO_DRIVE_MDIO_DRIVE                 0x0002

//KSZ9031 LED Mode register
#define KSZ9131_LEGACY_LED_MODE_KSZ9031_LED_MODE      0x4000

//Interrupt Control/Status register
#define KSZ9131_ICSR_JABBER_IE                        0x8000
#define KSZ9131_ICSR_RECEIVE_ERROR_IE                 0x4000
#define KSZ9131_ICSR_PAGE_RECEIVED_IE                 0x2000
#define KSZ9131_ICSR_PAR_DETECT_FAULT_IE              0x1000
#define KSZ9131_ICSR_LP_ACK_IE                        0x0800
#define KSZ9131_ICSR_LINK_DOWN_IE                     0x0400
#define KSZ9131_ICSR_REMOTE_FAULT_IE                  0x0200
#define KSZ9131_ICSR_LINK_UP_IE                       0x0100
#define KSZ9131_ICSR_JABBER_IF                        0x0080
#define KSZ9131_ICSR_RECEIVE_ERROR_IF                 0x0040
#define KSZ9131_ICSR_PAGE_RECEIVED_IF                 0x0020
#define KSZ9131_ICSR_PAR_DETECT_FAULT_IF              0x0010
#define KSZ9131_ICSR_LP_ACK_IF                        0x0008
#define KSZ9131_ICSR_LINK_DOWN_IF                     0x0004
#define KSZ9131_ICSR_REMOTE_FAULT_IF                  0x0002
#define KSZ9131_ICSR_LINK_UP_IF                       0x0001

//Auto MDI/MDI-X register
#define KSZ9131_AUTOMDI_MDI_SET                       0x0080
#define KSZ9131_AUTOMDI_SWAP_OFF                      0x0040

//Software Power Down Control register
#define KSZ9131_SPDC_CLK_GATE_OVERRIDE                0x0800
#define KSZ9131_SPDC_PLL_DIS                          0x0400
#define KSZ9131_SPDC_IO_DC_TEST_EN                    0x0080
#define KSZ9131_SPDC_VOH                              0x0040

//External Loopback register
#define KSZ9131_EXT_LOOPBACK_EXT_LPBK                 0x0008

//Control register
#define KSZ9131_PHYCON_INT_POL_INVERT                 0x4000
#define KSZ9131_PHYCON_JABBER_EN                      0x0200
#define KSZ9131_PHYCON_SQE_TEST_EN                    0x0100
#define KSZ9131_PHYCON_SPEED_1000BT                   0x0040
#define KSZ9131_PHYCON_SPEED_100BTX                   0x0020
#define KSZ9131_PHYCON_SPEED_10BT                     0x0010
#define KSZ9131_PHYCON_DUPLEX_STATUS                  0x0008
#define KSZ9131_PHYCON_1000BT_MS_STATUS               0x0004
#define KSZ9131_PHYCON_SOFT_RESET                     0x0002
#define KSZ9131_PHYCON_LINK_STATUS_CHECK_FAIL         0x0001

//RX DLL Control register
#define KSZ9131_RX_DLL_CTRL_RXDLL_TUNE_DIS            0x4000
#define KSZ9131_RX_DLL_CTRL_RXDLL_RESET               0x2000
#define KSZ9131_RX_DLL_CTRL_BYPASS_RXDLL              0x1000
#define KSZ9131_RX_DLL_CTRL_RXDLL_TAP_SEL             0x0FC0
#define KSZ9131_RX_DLL_CTRL_RXDLL_TAP_SEL_DEFAULT     0x06C0
#define KSZ9131_RX_DLL_CTRL_RXDLL_TAP_ADJ             0x003F
#define KSZ9131_RX_DLL_CTRL_RXDLL_TAP_ADJ_DEFAULT     0x0011

//TX DLL Control register
#define KSZ9131_TX_DLL_CTRL_TXDLL_TUNE_DIS            0x4000
#define KSZ9131_TX_DLL_CTRL_TXDLL_RESET               0x2000
#define KSZ9131_TX_DLL_CTRL_BYPASS_TXDLL              0x1000
#define KSZ9131_TX_DLL_CTRL_TXDLL_TAP_SEL             0x0FC0
#define KSZ9131_TX_DLL_CTRL_TXDLL_TAP_SEL_DEFAULT     0x0440
#define KSZ9131_TX_DLL_CTRL_TXDLL_TAP_ADJ             0x003F
#define KSZ9131_TX_DLL_CTRL_TXDLL_TAP_ADJ_DEFAULT     0x0011

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//KSZ9131 Ethernet PHY driver
extern const PhyDriver ksz9131PhyDriver;

//KSZ9131 related functions
error_t ksz9131Init(NetInterface *interface);
void ksz9131InitHook(NetInterface *interface);

void ksz9131Tick(NetInterface *interface);

void ksz9131EnableIrq(NetInterface *interface);
void ksz9131DisableIrq(NetInterface *interface);

void ksz9131EventHandler(NetInterface *interface);

void ksz9131WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t ksz9131ReadPhyReg(NetInterface *interface, uint8_t address);

void ksz9131DumpPhyReg(NetInterface *interface);

void ksz9131WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t ksz9131ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
