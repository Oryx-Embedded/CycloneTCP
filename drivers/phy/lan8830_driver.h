/**
 * @file lan8830_driver.h
 * @brief LAN8830 Gigabit Ethernet PHY driver
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

#ifndef _LAN8830_DRIVER_H
#define _LAN8830_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef LAN8830_PHY_ADDR
   #define LAN8830_PHY_ADDR 3
#elif (LAN8830_PHY_ADDR < 0 || LAN8830_PHY_ADDR > 31)
   #error LAN8830_PHY_ADDR parameter is not valid
#endif

//LAN8830 PHY registers
#define LAN8830_BMCR                                       0x00
#define LAN8830_BMSR                                       0x01
#define LAN8830_PHYID1                                     0x02
#define LAN8830_PHYID2                                     0x03
#define LAN8830_ANAR                                       0x04
#define LAN8830_ANLPAR                                     0x05
#define LAN8830_ANER                                       0x06
#define LAN8830_ANNPTR                                     0x07
#define LAN8830_ANNPRR                                     0x08
#define LAN8830_GBCR                                       0x09
#define LAN8830_GBSR                                       0x0A
#define LAN8830_MMDACR                                     0x0D
#define LAN8830_MMDAADR                                    0x0E
#define LAN8830_GBESR                                      0x0F
#define LAN8830_RLB                                        0x11
#define LAN8830_LINKMD                                     0x12
#define LAN8830_DPMAPCSS                                   0x13
#define LAN8830_RXERCTR                                    0x15
#define LAN8830_LED_MODE_SEL                               0x16
#define LAN8830_LED_BEHAVIOR                               0x17
#define LAN8830_OUT_CTRL                                   0x19
#define LAN8830_LEGACY_LED_MODE                            0x1A
#define LAN8830_ISR                                        0x1B
#define LAN8830_AUTOMDI                                    0x1C
#define LAN8830_SPDC                                       0x1D
#define LAN8830_EXT_LOOPBACK                               0x1E
#define LAN8830_PHYCON                                     0x1F

//LAN8830 MMD registers
#define LAN8830_MEAN_SLICER_ERROR                          0x01, 0xE1
#define LAN8830_DCQ_MEAN_SQUARE_ERROR                      0x01, 0xE2
#define LAN8830_DCQ_MEAN_SQUARE_ERROR_WORST_CASE           0x01, 0xE3
#define LAN8830_DCQ_SQI                                    0x01, 0xE4
#define LAN8830_DCQ_PEAK_MSE                               0x01, 0xE5
#define LAN8830_DCQ_CTRL                                   0x01, 0xE6
#define LAN8830_DCQ_CONFIG                                 0x01, 0xE7
#define LAN8830_DCQ_SQI_TABLE                              0x01, 0xE8
#define LAN8830_COMMON_CTRL                                0x02, 0x00
#define LAN8830_STRAP_STAT                                 0x02, 0x01
#define LAN8830_OP_MODE_STRAP_OVERRIDE                     0x02, 0x02
#define LAN8830_OP_MODE_STRAP                              0x02, 0x03
#define LAN8830_CLOCK_INV_CTRL_SIGNAL_PAD_SKEW             0x02, 0x04
#define LAN8830_RGMII_RX_DATA_PAD_SKEW                     0x02, 0x05
#define LAN8830_RGMII_TX_DATA_PAD_SKEW                     0x02, 0x06
#define LAN8830_CLK_PAD_SKEW                               0x02, 0x08
#define LAN8830_SELF_TEST_PKT_COUNT_LO                     0x02, 0x09
#define LAN8830_SELF_TEST_PKT_COUNT_HI                     0x02, 0x0A
#define LAN8830_SELF_TEST_STAT                             0x02, 0x0B
#define LAN8830_SELF_TEST_FRAME_COUNT_EN                   0x02, 0x0C
#define LAN8830_SELF_TEST_PGEN_EN                          0x02, 0x0D
#define LAN8830_SELF_TEST_EN                               0x02, 0x0E
#define LAN8830_WOL_CTRL                                   0x02, 0x10
#define LAN8830_WOL_MAC_LO                                 0x02, 0x11
#define LAN8830_WOL_MAC_MI                                 0x02, 0x12
#define LAN8830_WOL_MAC_HI                                 0x02, 0x13
#define LAN8830_CUSTOM_PKT0_CRC_LO                         0x02, 0x14
#define LAN8830_CUSTOM_PKT0_CRC_HI                         0x02, 0x15
#define LAN8830_CUSTOM_PKT1_CRC_LO                         0x02, 0x16
#define LAN8830_CUSTOM_PKT1_CRC_HI                         0x02, 0x17
#define LAN8830_CUSTOM_PKT2_CRC_LO                         0x02, 0x18
#define LAN8830_CUSTOM_PKT2_CRC_HI                         0x02, 0x19
#define LAN8830_CUSTOM_PKT3_CRC_LO                         0x02, 0x1A
#define LAN8830_CUSTOM_PKT3_CRC_HI                         0x02, 0x1B
#define LAN8830_CUSTOM_PKT0_MASK_LL                        0x02, 0x1C
#define LAN8830_CUSTOM_PKT0_MASK_LH                        0x02, 0x1D
#define LAN8830_CUSTOM_PKT0_MASK_HL                        0x02, 0x1E
#define LAN8830_CUSTOM_PKT0_MASK_HH                        0x02, 0x1F
#define LAN8830_CUSTOM_PKT1_MASK_LL                        0x02, 0x20
#define LAN8830_CUSTOM_PKT1_MASK_LH                        0x02, 0x21
#define LAN8830_CUSTOM_PKT1_MASK_HL                        0x02, 0x22
#define LAN8830_CUSTOM_PKT1_MASK_HH                        0x02, 0x23
#define LAN8830_CUSTOM_PKT2_MASK_LL                        0x02, 0x24
#define LAN8830_CUSTOM_PKT2_MASK_LH                        0x02, 0x25
#define LAN8830_CUSTOM_PKT2_MASK_HL                        0x02, 0x26
#define LAN8830_CUSTOM_PKT2_MASK_HH                        0x02, 0x27
#define LAN8830_CUSTOM_PKT3_MASK_LL                        0x02, 0x28
#define LAN8830_CUSTOM_PKT3_MASK_LH                        0x02, 0x29
#define LAN8830_CUSTOM_PKT3_MASK_HL                        0x02, 0x2A
#define LAN8830_CUSTOM_PKT3_MASK_HH                        0x02, 0x2B
#define LAN8830_WOL_CTRL_STAT                              0x02, 0x2C
#define LAN8830_WOL_CUSTOM_PKT_RECEIVE_STAT                0x02, 0x2D
#define LAN8830_WOL_MAGIC_PKT_RECEIVE_STAT                 0x02, 0x2E
#define LAN8830_WOL_DATA_MODULE_STAT                       0x02, 0x2F
#define LAN8830_CUSTOM_PKT0_RCVD_CRC_L                     0x02, 0x30
#define LAN8830_CUSTOM_PKT0_RCVD_CRC_H                     0x02, 0x31
#define LAN8830_CUSTOM_PKT1_RCVD_CRC_L                     0x02, 0x32
#define LAN8830_CUSTOM_PKT1_RCVD_CRC_H                     0x02, 0x33
#define LAN8830_CUSTOM_PKT2_RCVD_CRC_L                     0x02, 0x34
#define LAN8830_CUSTOM_PKT2_RCVD_CRC_H                     0x02, 0x35
#define LAN8830_CUSTOM_PKT3_RCVD_CRC_L                     0x02, 0x36
#define LAN8830_CUSTOM_PKT3_RCVD_CRC_H                     0x02, 0x37
#define LAN8830_SELF_TEST_CORRECT_COUNT_LO                 0x02, 0x3C
#define LAN8830_SELF_TEST_CORRECT_COUNT_HI                 0x02, 0x3D
#define LAN8830_SELF_TEST_ERROR_COUNT_LO                   0x02, 0x3E
#define LAN8830_SELF_TEST_ERROR_COUNT_HI                   0x02, 0x3F
#define LAN8830_RX_DLL_CTRL                                0x02, 0x4C
#define LAN8830_TX_DLL_CTRL                                0x02, 0x4D
#define LAN8830_DRIVING_STRENGTH                           0x02, 0x6F
#define LAN8830_GPIO_EN                                    0x02, 0x80
#define LAN8830_GPIO_DIR                                   0x02, 0x81
#define LAN8830_GPIO_BUF                                   0x02, 0x82
#define LAN8830_GPIO_DATA_SEL1                             0x02, 0x83
#define LAN8830_GPIO_DATA_SEL2                             0x02, 0x84
#define LAN8830_GPIO_DATA                                  0x02, 0x85
#define LAN8830_GPIO_INT_STS                               0x02, 0x86
#define LAN8830_GPIO_INT_EN                                0x02, 0x87
#define LAN8830_GPIO_INT_POL                               0x02, 0x88
#define LAN8830_PCS_CTRL1                                  0x03, 0x00
#define LAN8830_PCS_STAT1                                  0x03, 0x01
#define LAN8830_EEE_QUIET_TIMER                            0x03, 0x08
#define LAN8830_EEE_UPDATE_TIMER                           0x03, 0x09
#define LAN8830_EEE_LINK_FAIL_TIMER                        0x03, 0x0A
#define LAN8830_EEE_POST_UPDATE_TIMER                      0x03, 0x0B
#define LAN8830_EEE_WAIT_WQ_TIMER                          0x03, 0x0C
#define LAN8830_EEE_WAKE_TIMER                             0x03, 0x0D
#define LAN8830_EEE_WAKE_TX_TIMER                          0x03, 0x0E
#define LAN8830_EEE_WAKE_MZ_TIMER                          0x03, 0x0F
#define LAN8830_EEE_CTRL_CAPABILITY                        0x03, 0x14
#define LAN8830_EEE_WAKE_ERROR_COUNTER                     0x03, 0x16
#define LAN8830_EEE_100_TIMER0                             0x03, 0x18
#define LAN8830_EEE_100_TIMER1                             0x03, 0x19
#define LAN8830_EEE_100_TIMER2                             0x03, 0x1A
#define LAN8830_EEE_100_TIMER3                             0x03, 0x1B
#define LAN8830_EEE_ADV                                    0x07, 0x3C
#define LAN8830_EEE_LP_ABILITY                             0x07, 0x3D
#define LAN8830_EEE_LP_ABILITY_OVERRIDE                    0x07, 0x3E
#define LAN8830_EEE_MSG_CODE                               0x07, 0x3F
#define LAN8830_XTAL_CTRL                                  0x1C, 0x01
#define LAN8830_AFED_CTRL                                  0x1C, 0x09
#define LAN8830_LDO_CTRL                                   0x1C, 0x0E
#define LAN8830_EDPD_CTRL                                  0x1C, 0x24
#define LAN8830_EMITX_CTRL                                 0x1C, 0x25
#define LAN8830_EMITX_COEFF                                0x1C, 0x26

//Basic Control register
#define LAN8830_BMCR_RESET                                 0x8000
#define LAN8830_BMCR_LOOPBACK                              0x4000
#define LAN8830_BMCR_SPEED_SEL_LSB                         0x2000
#define LAN8830_BMCR_AN_EN                                 0x1000
#define LAN8830_BMCR_POWER_DOWN                            0x0800
#define LAN8830_BMCR_ISOLATE                               0x0400
#define LAN8830_BMCR_RESTART_AN                            0x0200
#define LAN8830_BMCR_DUPLEX_MODE                           0x0100
#define LAN8830_BMCR_COL_TEST                              0x0080
#define LAN8830_BMCR_SPEED_SEL_MSB                         0x0040

//Basic Status register
#define LAN8830_BMSR_100BT4                                0x8000
#define LAN8830_BMSR_100BTX_FD                             0x4000
#define LAN8830_BMSR_100BTX_HD                             0x2000
#define LAN8830_BMSR_10BT_FD                               0x1000
#define LAN8830_BMSR_10BT_HD                               0x0800
#define LAN8830_BMSR_100BT2_FD                             0x0400
#define LAN8830_BMSR_100BT2_HD                             0x0200
#define LAN8830_BMSR_EXTENDED_STATUS                       0x0100
#define LAN8830_BMSR_UNIDIRECTIONAL_ABLE                   0x0080
#define LAN8830_BMSR_MF_PREAMBLE_SUPPR                     0x0040
#define LAN8830_BMSR_AN_COMPLETE                           0x0020
#define LAN8830_BMSR_REMOTE_FAULT                          0x0010
#define LAN8830_BMSR_AN_CAPABLE                            0x0008
#define LAN8830_BMSR_LINK_STATUS                           0x0004
#define LAN8830_BMSR_JABBER_DETECT                         0x0002
#define LAN8830_BMSR_EXTENDED_CAPABLE                      0x0001

//Device Identifier 1 register
#define LAN8830_PHYID1_PHY_ID_MSB                          0xFFFF
#define LAN8830_PHYID1_PHY_ID_MSB_DEFAULT                  0x0022

//Device Identifier 2 register
#define LAN8830_PHYID2_PHY_ID_LSB                          0xFC00
#define LAN8830_PHYID2_PHY_ID_LSB_DEFAULT                  0x1400
#define LAN8830_PHYID2_MODEL_NUM                           0x03F0
#define LAN8830_PHYID2_MODEL_NUM_DEFAULT                   0x0250
#define LAN8830_PHYID2_REVISION_NUM                        0x000F

//Auto-Negotiation Advertisement register
#define LAN8830_ANAR_NEXT_PAGE                             0x8000
#define LAN8830_ANAR_REMOTE_FAULT                          0x2000
#define LAN8830_ANAR_EXTENDED_NEXT_PAGE                    0x1000
#define LAN8830_ANAR_ASYM_PAUSE                            0x0800
#define LAN8830_ANAR_SYM_PAUSE                             0x0400
#define LAN8830_ANAR_100BT4                                0x0200
#define LAN8830_ANAR_100BTX_FD                             0x0100
#define LAN8830_ANAR_100BTX_HD                             0x0080
#define LAN8830_ANAR_10BT_FD                               0x0040
#define LAN8830_ANAR_10BT_HD                               0x0020
#define LAN8830_ANAR_SELECTOR                              0x001F
#define LAN8830_ANAR_SELECTOR_DEFAULT                      0x0001

//Auto-Negotiation Link Partner Base Page Ability register
#define LAN8830_ANLPAR_NEXT_PAGE                           0x8000
#define LAN8830_ANLPAR_ACK                                 0x4000
#define LAN8830_ANLPAR_REMOTE_FAULT                        0x2000
#define LAN8830_ANLPAR_EXTENDED_NEXT_PAGE                  0x1000
#define LAN8830_ANLPAR_ASYM_PAUSE                          0x0800
#define LAN8830_ANLPAR_PAUSE                               0x0400
#define LAN8830_ANLPAR_100BT4                              0x0200
#define LAN8830_ANLPAR_100BTX_FD                           0x0100
#define LAN8830_ANLPAR_100BTX_HD                           0x0080
#define LAN8830_ANLPAR_10BT_FD                             0x0040
#define LAN8830_ANLPAR_10BT_HD                             0x0020
#define LAN8830_ANLPAR_SELECTOR                            0x001F
#define LAN8830_ANLPAR_SELECTOR_DEFAULT                    0x0001

//Auto-Negotiation Expansion register
#define LAN8830_ANER_RECEIVE_NP_LOC_ABLE                   0x0040
#define LAN8830_ANER_RECEIVE_NP_STOR_LOC                   0x0020
#define LAN8830_ANER_PAR_DETECT_FAULT                      0x0010
#define LAN8830_ANER_LP_NEXT_PAGE_ABLE                     0x0008
#define LAN8830_ANER_NEXT_PAGE_ABLE                        0x0004
#define LAN8830_ANER_PAGE_RECEIVED                         0x0002
#define LAN8830_ANER_LP_AN_ABLE                            0x0001

//Auto-Negotiation Next Page TX register
#define LAN8830_ANNPTR_NEXT_PAGE                           0x8000
#define LAN8830_ANNPTR_MSG_PAGE                            0x2000
#define LAN8830_ANNPTR_ACK2                                0x1000
#define LAN8830_ANNPTR_TOGGLE                              0x0800
#define LAN8830_ANNPTR_MESSAGE                             0x07FF

//Auto-Negotiation Next Page RX register
#define LAN8830_ANNPRR_NEXT_PAGE                           0x8000
#define LAN8830_ANNPRR_ACK                                 0x4000
#define LAN8830_ANNPRR_MSG_PAGE                            0x2000
#define LAN8830_ANNPRR_ACK2                                0x1000
#define LAN8830_ANNPRR_TOGGLE                              0x0800
#define LAN8830_ANNPRR_MESSAGE                             0x07FF

//Auto-Negotiation Master Slave Control register
#define LAN8830_GBCR_TEST_MODE                             0xE000
#define LAN8830_GBCR_MS_MAN_CONF_EN                        0x1000
#define LAN8830_GBCR_MS_MAN_CONF_VAL                       0x0800
#define LAN8830_GBCR_PORT_TYPE                             0x0400
#define LAN8830_GBCR_1000BT_FD                             0x0200
#define LAN8830_GBCR_1000BT_HD                             0x0100

//Auto-Negotiation Master Slave Status register
#define LAN8830_GBSR_MS_CONF_FAULT                         0x8000
#define LAN8830_GBSR_MS_CONF_RES                           0x4000
#define LAN8830_GBSR_LOCAL_RECEIVER_STATUS                 0x2000
#define LAN8830_GBSR_REMOTE_RECEIVER_STATUS                0x1000
#define LAN8830_GBSR_LP_1000BT_FD                          0x0800
#define LAN8830_GBSR_LP_1000BT_HD                          0x0400
#define LAN8830_GBSR_IDLE_ERR_COUNT                        0x00FF

//MMD Access Control register
#define LAN8830_MMDACR_FUNC                                0xC000
#define LAN8830_MMDACR_FUNC_ADDR                           0x0000
#define LAN8830_MMDACR_FUNC_DATA_NO_POST_INC               0x4000
#define LAN8830_MMDACR_FUNC_DATA_POST_INC_RW               0x8000
#define LAN8830_MMDACR_FUNC_DATA_POST_INC_W                0xC000
#define LAN8830_MMDACR_DEVAD                               0x001F

//Extended Status register
#define LAN8830_GBESR_1000BX_FD                            0x8000
#define LAN8830_GBESR_1000BX_HD                            0x4000
#define LAN8830_GBESR_1000BT_FD                            0x2000
#define LAN8830_GBESR_1000BT_HD                            0x1000

//Remote Loopback register
#define LAN8830_RLB_REMOTE_LOOPBACK                        0x0100

//LinkMD Cable Diagnostic register
#define LAN8830_LINKMD_TEST_EN                             0x8000
#define LAN8830_LINKMD_TX_DIS                              0x4000
#define LAN8830_LINKMD_PAIR                                0x3000
#define LAN8830_LINKMD_PAIR_A                              0x0000
#define LAN8830_LINKMD_PAIR_B                              0x1000
#define LAN8830_LINKMD_PAIR_C                              0x2000
#define LAN8830_LINKMD_PAIR_D                              0x3000
#define LAN8830_LINKMD_SEL                                 0x0C00
#define LAN8830_LINKMD_STATUS                              0x0300
#define LAN8830_LINKMD_STATUS_NORMAL                       0x0000
#define LAN8830_LINKMD_STATUS_OPEN                         0x0100
#define LAN8830_LINKMD_STATUS_SHORT                        0x0200
#define LAN8830_LINKMD_STATUS_TEST_FAILED                  0x0300
#define LAN8830_LINKMD_FAULT_DATA                          0x00FF

//Digital PMA/PCS Status register
#define LAN8830_DPMAPCSS_1000BT_LINK_STATUS                0x0002
#define LAN8830_DPMAPCSS_100BTX_LINK_STATUS                0x0001

//LED Mode Select register
#define LAN8830_LED_MODE_SEL_LED4_CONFIG                   0xF000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_LINK_ACT          0x0000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_LINK_1000_ACT     0x1000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_LINK_100_ACT      0x2000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_LINK_10_ACT       0x3000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_LINK_100_1000_ACT 0x4000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_LINK_10_1000_ACT  0x5000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_LINK_10_100_ACT   0x6000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_DUPLEX_COL        0x8000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_COL               0x9000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_ACT               0xA000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_AN_FAULT          0xC000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_OFF               0xE000
#define LAN8830_LED_MODE_SEL_LED4_CONFIG_ON                0xF000
#define LAN8830_LED_MODE_SEL_LED3_CONFIG                   0x0F00
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_LINK_ACT          0x0000
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_LINK_1000_ACT     0x0100
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_LINK_100_ACT      0x0200
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_LINK_10_ACT       0x0300
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_LINK_100_1000_ACT 0x0400
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_LINK_10_1000_ACT  0x0500
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_LINK_10_100_ACT   0x0600
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_DUPLEX_COL        0x0800
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_COL               0x0900
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_ACT               0x0A00
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_AN_FAULT          0x0C00
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_OFF               0x0E00
#define LAN8830_LED_MODE_SEL_LED3_CONFIG_ON                0x0F00
#define LAN8830_LED_MODE_SEL_LED2_CONFIG                   0x00F0
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_LINK_ACT          0x0000
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_LINK_1000_ACT     0x0010
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_LINK_100_ACT      0x0020
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_LINK_10_ACT       0x0030
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_LINK_100_1000_ACT 0x0040
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_LINK_10_1000_ACT  0x0050
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_LINK_10_100_ACT   0x0060
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_DUPLEX_COL        0x0080
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_COL               0x0090
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_ACT               0x00A0
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_AN_FAULT          0x00C0
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_OFF               0x00E0
#define LAN8830_LED_MODE_SEL_LED2_CONFIG_ON                0x00F0
#define LAN8830_LED_MODE_SEL_LED1_CONFIG                   0x000F
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_LINK_ACT          0x0000
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_LINK_1000_ACT     0x0001
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_LINK_100_ACT      0x0002
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_LINK_10_ACT       0x0003
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_LINK_100_1000_ACT 0x0004
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_LINK_10_1000_ACT  0x0005
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_LINK_10_100_ACT   0x0006
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_DUPLEX_COL        0x0008
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_COL               0x0009
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_ACT               0x000A
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_AN_FAULT          0x000C
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_OFF               0x000E
#define LAN8830_LED_MODE_SEL_LED1_CONFIG_ON                0x000F

//LED Behavior register
#define LAN8830_LED_BEHAVIOR_LED_ACT_OUT_SEL               0x4000
#define LAN8830_LED_BEHAVIOR_LED_PULSING_EN                0x1000
#define LAN8830_LED_BEHAVIOR_LED_BLINK_RATE                0x0C00
#define LAN8830_LED_BEHAVIOR_LED_BLINK_RATE_2_5HZ          0x0000
#define LAN8830_LED_BEHAVIOR_LED_BLINK_RATE_5HZ            0x0400
#define LAN8830_LED_BEHAVIOR_LED_BLINK_RATE_10HZ           0x0800
#define LAN8830_LED_BEHAVIOR_LED_BLINK_RATE_20HZ           0x0C00
#define LAN8830_LED_BEHAVIOR_LED_PULSE_STRECH_EN           0x01E0
#define LAN8830_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED4      0x0100
#define LAN8830_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED3      0x0080
#define LAN8830_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED2      0x0040
#define LAN8830_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED1      0x0020
#define LAN8830_LED_BEHAVIOR_LED_COMBINATION_DIS           0x000F
#define LAN8830_LED_BEHAVIOR_LED_COMBINATION_DIS_LED4      0x0008
#define LAN8830_LED_BEHAVIOR_LED_COMBINATION_DIS_LED3      0x0004
#define LAN8830_LED_BEHAVIOR_LED_COMBINATION_DIS_LED2      0x0002
#define LAN8830_LED_BEHAVIOR_LED_COMBINATION_DIS_LED1      0x0001

//Output Control register
#define LAN8830_OUT_CTRL_MDIO_BUFFER_TYPE                  0x8000
#define LAN8830_OUT_CTRL_INT_BUFFER_TYPE                   0x4000
#define LAN8830_OUT_CTRL_LED_BUFFER_TYPE                   0x3F00
#define LAN8830_OUT_CTRL_LED_BUFFER_TYPE_LED4              0x0800
#define LAN8830_OUT_CTRL_LED_BUFFER_TYPE_LED3              0x0400
#define LAN8830_OUT_CTRL_LED_BUFFER_TYPE_LED2              0x0200
#define LAN8830_OUT_CTRL_LED_BUFFER_TYPE_LED1              0x0100
#define LAN8830_OUT_CTRL_PME_POLARITY                      0x0080
#define LAN8830_OUT_CTRL_LED_POLARITY                      0x003F
#define LAN8830_OUT_CTRL_LED_POLARITY_LED4                 0x0008
#define LAN8830_OUT_CTRL_LED_POLARITY_LED3                 0x0004
#define LAN8830_OUT_CTRL_LED_POLARITY_LED2                 0x0002
#define LAN8830_OUT_CTRL_LED_POLARITY_LED1                 0x0001

//KSZ9031 LED Mode register
#define LAN8830_LEGACY_LED_MODE_KSZ9031_LED_MODE           0x4000

//Interrupt Status register
#define LAN8830_ISR_ENERGY_NOT_DETECT                      0x0800
#define LAN8830_ISR_ENERGY_DETECT                          0x0400
#define LAN8830_ISR_GPIO                                   0x0100
#define LAN8830_ISR_JABBER                                 0x0080
#define LAN8830_ISR_RECEIVE_ERROR                          0x0040
#define LAN8830_ISR_PAGE_RECEIVED                          0x0020
#define LAN8830_ISR_PAR_DETECT_FAULT                       0x0010
#define LAN8830_ISR_LP_ACK                                 0x0008
#define LAN8830_ISR_LINK_DOWN                              0x0004
#define LAN8830_ISR_ADC_FIFO_ERROR                         0x0002
#define LAN8830_ISR_LINK_UP                                0x0001

//Auto-MDI/MDI-X register
#define LAN8830_AUTOMDI_MDI_SET                            0x0080
#define LAN8830_AUTOMDI_SWAP_OFF                           0x0040

//Software Power Down Control register
#define LAN8830_SPDC_CLK_GATE_OVERRIDE                     0x0800
#define LAN8830_SPDC_PLL_DIS                               0x0400
#define LAN8830_SPDC_IO_DC_TEST_EN                         0x0080
#define LAN8830_SPDC_VOH                                   0x0040

//External Loopback register
#define LAN8830_EXT_LOOPBACK_EXT_LPBK                      0x0008

//Control register
#define LAN8830_PHYCON_INT_POL_INVERT                      0x4000
#define LAN8830_PHYCON_JABBER_EN                           0x0200
#define LAN8830_PHYCON_SQE_TEST_EN                         0x0100
#define LAN8830_PHYCON_SPEED_1000BT                        0x0040
#define LAN8830_PHYCON_SPEED_100BTX                        0x0020
#define LAN8830_PHYCON_SPEED_10BT                          0x0010
#define LAN8830_PHYCON_DUPLEX_STATUS                       0x0008
#define LAN8830_PHYCON_1000BT_MS_STATUS                    0x0004
#define LAN8830_PHYCON_SOFT_RESET                          0x0002
#define LAN8830_PHYCON_LINK_STATUS_CHECK_FAIL              0x0001

//RX DLL Control register
#define LAN8830_RX_DLL_CTRL_RXDLL_TUNE_DIS                 0x8000
#define LAN8830_RX_DLL_CTRL_BYPASS_RXDLL                   0x4000
#define LAN8830_RX_DLL_CTRL_RXDLL_TAP_SEL                  0x3F80
#define LAN8830_RX_DLL_CTRL_RXDLL_TAP_SEL_DEFAULT          0x0D80
#define LAN8830_RX_DLL_CTRL_RXDLL_TAP_ADJ                  0x007F
#define LAN8830_RX_DLL_CTRL_RXDLL_TAP_ADJ_DEFAULT          0x001B

//TX DLL Control register
#define LAN8830_TX_DLL_CTRL_TXDLL_TUNE_DIS                 0x8000
#define LAN8830_TX_DLL_CTRL_BYPASS_TXDLL                   0x4000
#define LAN8830_TX_DLL_CTRL_TXDLL_TAP_SEL                  0x3F80
#define LAN8830_TX_DLL_CTRL_TXDLL_TAP_SEL_DEFAULT          0x0D80
#define LAN8830_TX_DLL_CTRL_TXDLL_TAP_ADJ                  0x007F
#define LAN8830_TX_DLL_CTRL_TXDLL_TAP_ADJ_DEFAULT          0x001B

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//LAN8830 Ethernet PHY driver
extern const PhyDriver lan8830PhyDriver;

//LAN8830 related functions
error_t lan8830Init(NetInterface *interface);
void lan8830InitHook(NetInterface *interface);

void lan8830Tick(NetInterface *interface);

void lan8830EnableIrq(NetInterface *interface);
void lan8830DisableIrq(NetInterface *interface);

void lan8830EventHandler(NetInterface *interface);

void lan8830WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t lan8830ReadPhyReg(NetInterface *interface, uint8_t address);

void lan8830DumpPhyReg(NetInterface *interface);

void lan8830WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t lan8830ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
