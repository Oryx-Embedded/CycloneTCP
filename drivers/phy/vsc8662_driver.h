/**
 * @file vsc8662_driver.h
 * @brief VSC8662 Gigabit Ethernet PHY driver
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

#ifndef _VSC8662_DRIVER_H
#define _VSC8662_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef VSC8662_PHY_ADDR
   #define VSC8662_PHY_ADDR 0
#elif (VSC8662_PHY_ADDR < 0 || VSC8662_PHY_ADDR > 31)
   #error VSC8662_PHY_ADDR parameter is not valid
#endif

//VSC8662 PHY registers
#define VSC8662_BMCR                                                   0x00
#define VSC8662_BMSR                                                   0x01
#define VSC8662_PHYID1                                                 0x02
#define VSC8662_PHYID2                                                 0x03
#define VSC8662_ANAR                                                   0x04
#define VSC8662_ANLPAR                                                 0x05
#define VSC8662_ANER                                                   0x06
#define VSC8662_ANNPTR                                                 0x07
#define VSC8662_ANLPNPR                                                0x08
#define VSC8662_1000BT_CTRL                                            0x09
#define VSC8662_1000BT_STAT                                            0x0A
#define VSC8662_1000BT_EXT_STAT1                                       0x0F

//VSC8662 PHY registers (page 0)
#define VSC8662_100BTX_EXT_STAT                                        0x10
#define VSC8662_1000BT_EXT_STAT2                                       0x11
#define VSC8662_BYPASS_CTRL                                            0x12
#define VSC8662_ERR_CNT1                                               0x13
#define VSC8662_ERR_CNT2                                               0x14
#define VSC8662_ERR_CNT3                                               0x15
#define VSC8662_EXT_CTRL_STAT                                          0x16
#define VSC8662_EXT_PHY_CTRL1                                          0x17
#define VSC8662_EXT_PHY_CTRL2                                          0x18
#define VSC8662_INT_MASK                                               0x19
#define VSC8662_INT_STATUS                                             0x1A
#define VSC8662_MAC_AN_CTRL_STAT                                       0x1B
#define VSC8662_AUX_CTRL_STAT                                          0x1C
#define VSC8662_LED_MODE_SEL                                           0x1D
#define VSC8662_LED_BEHAVIOR                                           0x1E
#define VSC8662_EXT_PAGE_ACCESS                                        0x1F

//VSC8662 PHY registers (page 1)
#define VSC8662_SERDES_MEDIA_CTRL                                      0x10
#define VSC8662_SERDES_MAC_MEDIA_CTRL                                  0x11
#define VSC8662_CRC_GOOD_CNT                                           0x12
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL                            0x13
#define VSC8662_EXT_PHY_CTRL3                                          0x14
#define VSC8662_EXT_PHY_CTRL4                                          0x17
#define VSC8662_SERDES_MAC_MEDIA_STAT                                  0x1C
#define VSC8662_EPG_CTRL1                                              0x1D
#define VSC8662_EPG_CTRL2                                              0x1E

//VSC8662 PHY registers (page 16)
#define VSC8662_SIGDET_GPIO_CTRL                                       0x0D
#define VSC8662_GPIO_INPUT                                             0x0F
#define VSC8662_GPIO_OUTPUT                                            0x10
#define VSC8662_GPIO_OUTPUT_EN                                         0x11
#define VSC8662_FAST_LINK_FAIL_CTRL                                    0x13
#define VSC8662_I2C_MUX_CTRL1                                          0x14
#define VSC8662_I2C_MUX_CTRL2                                          0x15
#define VSC8662_I2C_MUX_DATA_RW                                        0x16
#define VSC8662_RECOVERED_CLOCK1                                       0x17
#define VSC8662_RECOVERED_CLOCK2                                       0x18
#define VSC8662_LED_PORT_SWAPPING                                      0x19

//Mode control register
#define VSC8662_BMCR_RESET                                             0x8000
#define VSC8662_BMCR_LOOPBACK                                          0x4000
#define VSC8662_BMCR_SPEED_SEL_LSB                                     0x2000
#define VSC8662_BMCR_AN_EN                                             0x1000
#define VSC8662_BMCR_POWER_DOWN                                        0x0800
#define VSC8662_BMCR_ISOLATE                                           0x0400
#define VSC8662_BMCR_RESTART_AN                                        0x0200
#define VSC8662_BMCR_DUPLEX_MODE                                       0x0100
#define VSC8662_BMCR_COL_TEST                                          0x0080
#define VSC8662_BMCR_SPEED_SEL_MSB                                     0x0040
#define VSC8662_BMCR_UNIDIRECTIONAL_EN                                 0x0020

//Mode status register
#define VSC8662_BMSR_100BT4                                            0x8000
#define VSC8662_BMSR_100BTX_FD                                         0x4000
#define VSC8662_BMSR_100BTX_HD                                         0x2000
#define VSC8662_BMSR_10BT_FD                                           0x1000
#define VSC8662_BMSR_10BT_HD                                           0x0800
#define VSC8662_BMSR_100BT2_FD                                         0x0400
#define VSC8662_BMSR_100BT2_HD                                         0x0200
#define VSC8662_BMSR_EXTENDED_STATUS                                   0x0100
#define VSC8662_BMSR_UNIDIRECTIONAL_ABLE                               0x0080
#define VSC8662_BMSR_PREAMBLE_SUPPR                                    0x0040
#define VSC8662_BMSR_AN_COMPLETE                                       0x0020
#define VSC8662_BMSR_REMOTE_FAULT                                      0x0010
#define VSC8662_BMSR_AN_CAPABLE                                        0x0008
#define VSC8662_BMSR_LINK_STATUS                                       0x0004
#define VSC8662_BMSR_JABBER_DETECT                                     0x0002
#define VSC8662_BMSR_EXTENDED_CAPABLE                                  0x0001

//PHY identifier 1 register
#define VSC8662_PHYID1_OUI_MSB                                         0xFFFF
#define VSC8662_PHYID1_OUI_MSB_DEFAULT                                 0x0007

//PHY identifier 2 register
#define VSC8662_PHYID2_OUI_LSB                                         0xFC00
#define VSC8662_PHYID2_OUI_LSB_DEFAULT                                 0x0400
#define VSC8662_PHYID2_MODEL_NUM                                       0x03F0
#define VSC8662_PHYID2_MODEL_NUM_DEFAULT                               0x0260
#define VSC8662_PHYID2_REVISION_NUM                                    0x000F

//Auto-negotiation advertisement register
#define VSC8662_ANAR_NEXT_PAGE                                         0x8000
#define VSC8662_ANAR_REMOTE_FAULT                                      0x2000
#define VSC8662_ANAR_ASYM_PAUSE                                        0x0800
#define VSC8662_ANAR_SYM_PAUSE                                         0x0400
#define VSC8662_ANAR_100BT4                                            0x0200
#define VSC8662_ANAR_100BTX_FD                                         0x0100
#define VSC8662_ANAR_100BTX_HD                                         0x0080
#define VSC8662_ANAR_10BT_FD                                           0x0040
#define VSC8662_ANAR_10BT_HD                                           0x0020
#define VSC8662_ANAR_SELECTOR                                          0x001F
#define VSC8662_ANAR_SELECTOR_DEFAULT                                  0x0001

//Auto-negotiation link partner ability register
#define VSC8662_ANLPAR_NEXT_PAGE                                       0x8000
#define VSC8662_ANLPAR_ACK                                             0x4000
#define VSC8662_ANLPAR_REMOTE_FAULT                                    0x2000
#define VSC8662_ANLPAR_ASYM_PAUSE                                      0x0800
#define VSC8662_ANLPAR_SYM_PAUSE                                       0x0400
#define VSC8662_ANLPAR_100BT4                                          0x0200
#define VSC8662_ANLPAR_100BTX_FD                                       0x0100
#define VSC8662_ANLPAR_100BTX_HD                                       0x0080
#define VSC8662_ANLPAR_10BT_FD                                         0x0040
#define VSC8662_ANLPAR_10BT_HD                                         0x0020
#define VSC8662_ANLPAR_SELECTOR                                        0x001F
#define VSC8662_ANLPAR_SELECTOR_DEFAULT                                0x0001

//Auto-negotiation expansion register
#define VSC8662_ANER_PAR_DETECT_FAULT                                  0x0010
#define VSC8662_ANER_LP_NEXT_PAGE_ABLE                                 0x0008
#define VSC8662_ANER_NEXT_PAGE_ABLE                                    0x0004
#define VSC8662_ANER_PAGE_RECEIVED                                     0x0002
#define VSC8662_ANER_LP_AN_ABLE                                        0x0001

//Auto-negotiation next-page transmit register
#define VSC8662_ANNPTR_NEXT_PAGE                                       0x8000
#define VSC8662_ANNPTR_MSG_PAGE                                        0x2000
#define VSC8662_ANNPTR_ACK2                                            0x1000
#define VSC8662_ANNPTR_TOGGLE                                          0x0800
#define VSC8662_ANNPTR_MESSAGE                                         0x07FF

//Auto-negotiation link partner next-page receive register
#define VSC8662_ANLPNPR_NEXT_PAGE                                      0x8000
#define VSC8662_ANLPNPR_ACK                                            0x4000
#define VSC8662_ANLPNPR_MSG_PAGE                                       0x2000
#define VSC8662_ANLPNPR_ACK2                                           0x1000
#define VSC8662_ANLPNPR_TOGGLE                                         0x0800
#define VSC8662_ANLPNPR_MESSAGE                                        0x07FF

//1000BASE-T control register
#define VSC8662_1000BT_CTRL_TEST_MODE                                  0xE000
#define VSC8662_1000BT_CTRL_MS_MAN_CONF_EN                             0x1000
#define VSC8662_1000BT_CTRL_MS_MAN_CONF_VAL                            0x0800
#define VSC8662_1000BT_CTRL_PORT_TYPE                                  0x0400
#define VSC8662_1000BT_CTRL_1000BT_FD                                  0x0200
#define VSC8662_1000BT_CTRL_1000BT_HD                                  0x0100

//1000BASE-T status register
#define VSC8662_1000BT_STAT_MS_CONF_FAULT                              0x8000
#define VSC8662_1000BT_STAT_MS_CONF_RES                                0x4000
#define VSC8662_1000BT_STAT_LOCAL_RECEIVER_STATUS                      0x2000
#define VSC8662_1000BT_STAT_REMOTE_RECEIVER_STATUS                     0x1000
#define VSC8662_1000BT_STAT_LP_1000BT_FD                               0x0800
#define VSC8662_1000BT_STAT_LP_1000BT_HD                               0x0400
#define VSC8662_1000BT_STAT_IDLE_ERR_COUNT                             0x00FF

//1000BASE-T status extension 1 register
#define VSC8662_1000BT_EXT_STAT1_1000BX_FD                             0x8000
#define VSC8662_1000BT_EXT_STAT1_1000BX_HD                             0x4000
#define VSC8662_1000BT_EXT_STAT1_1000BT_FD                             0x2000
#define VSC8662_1000BT_EXT_STAT1_1000BT_HD                             0x1000

//100BASE-TX status extension register
#define VSC8662_100BTX_EXT_STAT_DESCRAMBLER                            0x8000
#define VSC8662_100BTX_EXT_STAT_LOCK_ERROR                             0x4000
#define VSC8662_100BTX_EXT_STAT_DISCONNECT_STATE                       0x2000
#define VSC8662_100BTX_EXT_STAT_LINK_STATUS                            0x1000
#define VSC8662_100BTX_EXT_STAT_RECEIVE_ERROR                          0x0800
#define VSC8662_100BTX_EXT_STAT_TRANSMIT_ERROR                         0x0400
#define VSC8662_100BTX_EXT_STAT_SSD_ERROR                              0x0200
#define VSC8662_100BTX_EXT_STAT_ESD_ERROR                              0x0100

//1000BASE-T status extension 2 register
#define VSC8662_1000BT_EXT_STAT2_DESCRAMBLER                           0x8000
#define VSC8662_1000BT_EXT_STAT2_LOCK_ERROR                            0x4000
#define VSC8662_1000BT_EXT_STAT2_DISCONNECT_STATE                      0x2000
#define VSC8662_1000BT_EXT_STAT2_LINK_STATUS                           0x1000
#define VSC8662_1000BT_EXT_STAT2_RECEIVE_ERROR                         0x0800
#define VSC8662_1000BT_EXT_STAT2_TRANSMIT_ERROR                        0x0400
#define VSC8662_1000BT_EXT_STAT2_SSD_ERROR                             0x0200
#define VSC8662_1000BT_EXT_STAT2_ESD_ERROR                             0x0100
#define VSC8662_1000BT_EXT_STAT2_CARRIER_EXT_ERROR                     0x0080
#define VSC8662_1000BT_EXT_STAT2_NON_COMP_BCM5400_DETECT               0x0040
#define VSC8662_1000BT_EXT_STAT2_MDI_CROSSOVER_ERROR                   0x0020

//Bypass control register
#define VSC8662_BYPASS_CTRL_TRANSMIT_DIS                               0x8000
#define VSC8662_BYPASS_CTRL_4B5B_ENC_DEC                               0x4000
#define VSC8662_BYPASS_CTRL_SCRAMBLER                                  0x2000
#define VSC8662_BYPASS_CTRL_DESCRAMBLER                                0x1000
#define VSC8662_BYPASS_CTRL_PCS_RECEIVE                                0x0800
#define VSC8662_BYPASS_CTRL_PCS_TRANSMIT                               0x0400
#define VSC8662_BYPASS_CTRL_LFI_TIMER                                  0x0200
#define VSC8662_BYPASS_CTRL_AUTO_MDX_10_100                            0x0080
#define VSC8662_BYPASS_CTRL_NON_COMP_BCM5400_DETECT_DIS                0x0040
#define VSC8662_BYPASS_CTRL_PAIR_SWAP_CORR_DIS                         0x0020
#define VSC8662_BYPASS_CTRL_POL_CORR_DIS                               0x0010
#define VSC8662_BYPASS_CTRL_PAR_DETECT_CONTROL                         0x0008
#define VSC8662_BYPASS_CTRL_PULSE_SHAPING_FILTER                       0x0004
#define VSC8662_BYPASS_CTRL_AUTO_1000BT_NP_DIS                         0x0002
#define VSC8662_BYPASS_CTRL_CLKOUT_OUTPUT_EN                           0x0001

//Error Counter 1 register
#define VSC8662_ERR_CNT1_VALUE                                         0x00FF

//Error Counter 2 register
#define VSC8662_ERR_CNT2_VALUE                                         0x00FF

//Error Counter 3 register
#define VSC8662_ERR_CNT3_VALUE                                         0x00FF

//Extended control and status register
#define VSC8662_EXT_CTRL_STAT_FORCE_10BT_LINK                          0x8000
#define VSC8662_EXT_CTRL_STAT_JABBER_DETECT_DIS                        0x4000
#define VSC8662_EXT_CTRL_STAT_10BT_ECHO_DIS                            0x2000
#define VSC8662_EXT_CTRL_STAT_SQE_MODE_DIS                             0x1000
#define VSC8662_EXT_CTRL_STAT_10BT_SQUELCH_CONTROL                     0x0C00
#define VSC8662_EXT_CTRL_STAT_STICKY_RESET_EN                          0x0200
#define VSC8662_EXT_CTRL_STAT_EOF_ERROR                                0x0100
#define VSC8662_EXT_CTRL_STAT_10BT_DISCONNECT_STATE                    0x0080
#define VSC8662_EXT_CTRL_STAT_10BT_LINK_STATUS                         0x0040
#define VSC8662_EXT_CTRL_STAT_CRS_CONTROL                              0x0006
#define VSC8662_EXT_CTRL_STAT_SMI_BROADCAST_WRITE                      0x0001

//Extended PHY control 1 register
#define VSC8662_EXT_PHY_CTRL1_MAC_AN                                   0x2000
#define VSC8662_EXT_PHY_CTRL1_MAC_MODE                                 0x1000
#define VSC8662_EXT_PHY_CTRL1_AMS_PREFERENCE                           0x0800
#define VSC8662_EXT_PHY_CTRL1_MEDIA_OP_MODE                            0x0700
#define VSC8662_EXT_PHY_CTRL1_FORCE_AMS_OVERRIDE                       0x00C0
#define VSC8662_EXT_PHY_CTRL1_FAR_END_LOOPBACK                         0x0008
#define VSC8662_EXT_PHY_CTRL1_SGMII_ALIGN_ERROR_STATUS                 0x0002

//Extended PHY control 2 register
#define VSC8662_EXT_PHY_CTRL2_100BTX_EDGE_RATE_CONTROL                 0xE000
#define VSC8662_EXT_PHY_CTRL2_PICMG_2_16_REDUCED_PWR_MODE              0x1000
#define VSC8662_EXT_PHY_CTRL2_SGMII_INPUT_PREAMBLE                     0x0180
#define VSC8662_EXT_PHY_CTRL2_SGMII_OUTPUT_PREAMBLE                    0x0040
#define VSC8662_EXT_PHY_CTRL2_JUMBO_PACKET_MODE                        0x0030
#define VSC8662_EXT_PHY_CTRL2_100BTX_TX_AMPLITUDE_CONTROL              0x000E
#define VSC8662_EXT_PHY_CTRL2_1000BT_CONNECTOR_LOOPBACK                0x0001

//Interrupt mask register
#define VSC8662_INT_MASK_MDINT                                         0x8000
#define VSC8662_INT_MASK_SPEED_CHANGE                                  0x4000
#define VSC8662_INT_MASK_LINK_CHANGE                                   0x2000
#define VSC8662_INT_MASK_FDX_CHANGE                                    0x1000
#define VSC8662_INT_MASK_AN_ERROR                                      0x0800
#define VSC8662_INT_MASK_AN_COMPLETE                                   0x0400
#define VSC8662_INT_MASK_POE_DETECT                                    0x0200
#define VSC8662_INT_MASK_SYMBOL_ERROR                                  0x0100
#define VSC8662_INT_MASK_FAST_LINK_FAILURE                             0x0080
#define VSC8662_INT_MASK_TX_FIFO_OVER_UNDERFLOW                        0x0040
#define VSC8662_INT_MASK_RX_FIFO_OVER_UNDERFLOW                        0x0020
#define VSC8662_INT_MASK_AMS_MEDIA_CHANGE                              0x0010
#define VSC8662_INT_MASK_FALSE_CARRIER                                 0x0008
#define VSC8662_INT_MASK_LINK_SPEED_DOWNSHIFT                          0x0004
#define VSC8662_INT_MASK_MS_RESOLUTION_ERROR                           0x0002
#define VSC8662_INT_MASK_RX_ER                                         0x0001

//Interrupt status register
#define VSC8662_INT_STATUS_MDINT                                       0x8000
#define VSC8662_INT_STATUS_SPEED_CHANGE                                0x4000
#define VSC8662_INT_STATUS_LINK_CHANGE                                 0x2000
#define VSC8662_INT_STATUS_FDX_CHANGE                                  0x1000
#define VSC8662_INT_STATUS_AN_ERROR                                    0x0800
#define VSC8662_INT_STATUS_AN_COMPLETE                                 0x0400
#define VSC8662_INT_STATUS_POE_DETECT                                  0x0200
#define VSC8662_INT_STATUS_SYMBOL_ERROR                                0x0100
#define VSC8662_INT_STATUS_FAST_LINK_FAILURE                           0x0080
#define VSC8662_INT_STATUS_TX_FIFO_OVER_UNDERFLOW                      0x0040
#define VSC8662_INT_STATUS_RX_FIFO_OVER_UNDERFLOW                      0x0020
#define VSC8662_INT_STATUS_AMS_MEDIA_CHANGE                            0x0010
#define VSC8662_INT_STATUS_FALSE_CARRIER                               0x0008
#define VSC8662_INT_STATUS_LINK_SPEED_DOWNSHIFT                        0x0004
#define VSC8662_INT_STATUS_MS_RESOLUTION_ERROR                         0x0002
#define VSC8662_INT_STATUS_RX_ER                                       0x0001

//MAC interface auto-negotiation control and status register
#define VSC8662_MAC_AN_CTRL_STAT_MAC_MEDIA_INTERLOCK                   0x8000
#define VSC8662_MAC_AN_CTRL_STAT_RESTART_AN_INTERLOCK                  0x4000
#define VSC8662_MAC_AN_CTRL_STAT_AN_AUTO_SENSE                         0x2000
#define VSC8662_MAC_AN_CTRL_STAT_AN_RESTART                            0x1000
#define VSC8662_MAC_AN_CTRL_STAT_LP_RESTART_REQ                        0x0800
#define VSC8662_MAC_AN_CTRL_STAT_REMOTE_FAULT                          0x0300
#define VSC8662_MAC_AN_CTRL_STAT_ASYM_PAUSE_ADV                        0x0080
#define VSC8662_MAC_AN_CTRL_STAT_SYM_PAUSE_ADV                         0x0040
#define VSC8662_MAC_AN_CTRL_STAT_FD_ADV                                0x0020
#define VSC8662_MAC_AN_CTRL_STAT_HD_ADV                                0x0010
#define VSC8662_MAC_AN_CTRL_STAT_AN_ABLE                               0x0008
#define VSC8662_MAC_AN_CTRL_STAT_LINK_STATUS                           0x0004
#define VSC8662_MAC_AN_CTRL_STAT_AN_COMPLETE                           0x0002
#define VSC8662_MAC_AN_CTRL_STAT_SIGNAL_DETECT                         0x0001

//Auxiliary control and status register
#define VSC8662_AUX_CTRL_STAT_AN_COMPLETE                              0x8000
#define VSC8662_AUX_CTRL_STAT_AN_DIS                                   0x4000
#define VSC8662_AUX_CTRL_STAT_MDI_MDIX_IND                             0x2000
#define VSC8662_AUX_CTRL_STAT_CD_PAIR_SWAP                             0x1000
#define VSC8662_AUX_CTRL_STAT_A_POLARITY_INVERSION                     0x0800
#define VSC8662_AUX_CTRL_STAT_B_POLARITY_INVERSION                     0x0400
#define VSC8662_AUX_CTRL_STAT_C_POLARITY_INVERSION                     0x0200
#define VSC8662_AUX_CTRL_STAT_D_POLARITY_INVERSION                     0x0100
#define VSC8662_AUX_CTRL_STAT_ACTIPHY_LINK_STAT_TMO_MSB                0x0080
#define VSC8662_AUX_CTRL_STAT_ACTIPHY_MODE_EN                          0x0040
#define VSC8662_AUX_CTRL_STAT_FDX_STATUS                               0x0020
#define VSC8662_AUX_CTRL_STAT_SPEED_STATUS                             0x0018
#define VSC8662_AUX_CTRL_STAT_SPEED_STATUS_10                          0x0000
#define VSC8662_AUX_CTRL_STAT_SPEED_STATUS_100                         0x0008
#define VSC8662_AUX_CTRL_STAT_SPEED_STATUS_1000                        0x0010
#define VSC8662_AUX_CTRL_STAT_ACTIPHY_LINK_STAT_TMO_LSB                0x0004
#define VSC8662_AUX_CTRL_STAT_MEDIA_MODE_STATUS                        0x0003

//LED mode select register
#define VSC8662_LED_MODE_SEL_LED3                                      0xF000
#define VSC8662_LED_MODE_SEL_LED2                                      0x0F00
#define VSC8662_LED_MODE_SEL_LED1                                      0x00F0
#define VSC8662_LED_MODE_SEL_LED0                                      0x000F

//LED behavior register
#define VSC8662_LED_BEHAVIOR_COPPER_FIBER_LED_COMBINE_DIS              0x8000
#define VSC8662_LED_BEHAVIOR_ACTIVITY_OUTPUT_SEL                       0x4000
#define VSC8662_LED_BEHAVIOR_LED_PULSING_EN                            0x1000
#define VSC8662_LED_BEHAVIOR_LED_BLINK_PULSE_STRETCH_RATE              0x0C00
#define VSC8662_LED_BEHAVIOR_LED3_PULSE_STRETCH_BLINK_SEL              0x0100
#define VSC8662_LED_BEHAVIOR_LED2_PULSE_STRETCH_BLINK_SEL              0x0080
#define VSC8662_LED_BEHAVIOR_LED1_PULSE_STRETCH_BLINK_SEL              0x0040
#define VSC8662_LED_BEHAVIOR_LED0_PULSE_STRETCH_BLINK_SEL              0x0020
#define VSC8662_LED_BEHAVIOR_LED3_COMBINE_FEATURE_DIS                  0x0008
#define VSC8662_LED_BEHAVIOR_LED2_COMBINE_FEATURE_DIS                  0x0004
#define VSC8662_LED_BEHAVIOR_LED1_COMBINE_FEATURE_DIS                  0x0002
#define VSC8662_LED_BEHAVIOR_LED0_COMBINE_FEATURE_DIS                  0x0001

//Extended register page access register
#define VSC8662_EXT_PAGE_ACCESS_MAIN                                   0x0000
#define VSC8662_EXT_PAGE_ACCESS_EXTENDED                               0x0001
#define VSC8662_EXT_PAGE_ACCESS_GPIO                                   0x0010

//SerDes Media control register
#define VSC8662_SERDES_MEDIA_CTRL_TRANSMIT_REMOTE_FAULT                0xC000
#define VSC8662_SERDES_MEDIA_CTRL_LP_REMOTE_FAULT                      0x3000
#define VSC8662_SERDES_MEDIA_CTRL_PARALLEL_DETECT                      0x0800
#define VSC8662_SERDES_MEDIA_CTRL_SERDES_MEDIA_SIGNAL_DETECT           0x0400
#define VSC8662_SERDES_MEDIA_CTRL_ALLOW_1000BX_LINK_UP                 0x0200
#define VSC8662_SERDES_MEDIA_CTRL_ALLOW_100BX_LINK_UP                  0x0100
#define VSC8662_SERDES_MEDIA_CTRL_SERDES_MEDIA_LP_RESTART_REQ          0x0080
#define VSC8662_SERDES_MEDIA_CTRL_FAR_END_FAULT_DETECT_100BFX          0x0040

//SerDes MAC/Media control register
#define VSC8662_SERDES_MAC_MEDIA_CTRL_SERDES_MEDIA_RX_EQUAL            0x0200
#define VSC8662_SERDES_MAC_MEDIA_CTRL_SERDES_MEDIA_OUT_SWING           0x00E0
#define VSC8662_SERDES_MAC_MEDIA_CTRL_SERDES_MAC_OUT_SWING             0x001C
#define VSC8662_SERDES_MAC_MEDIA_CTRL_SERDES_MAC_HYSTERESIS            0x0002
#define VSC8662_SERDES_MAC_MEDIA_CTRL_SERDES_MEDIA_HYSTERESIS          0x0001

//CRC good counter register
#define VSC8662_CRC_GOOD_CNT_PKT_SINCE_LAST_READ                       0x8000
#define VSC8662_CRC_GOOD_CNT_CONTENTS                                  0x3FFF

//SerDes loopback and SIGDET control register
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_LED3_EXT_MODE              0x8000
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_LED2_EXT_MODE              0x4000
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_LED1_EXT_MODE              0x2000
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_LED0_EXT_MODE              0x1000
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_LED_RESET_BLINK_SUPPR      0x0800
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_SERDES_MEDIA_LOOPBACK_EN   0x0400
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_SERDES_MEDIA_LOOPBACK_MODE 0x0300
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_SERDES_MAC_LOOPBACK_EN     0x0080
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_SERDES_MAC_LOOPBACK_MODE   0x0060
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_FAST_LINK_FAILURE_IND      0x0010
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_FORCE_MDI_CROSSOVER        0x000C
#define VSC8662_SERDES_LOOPBACK_SIGDET_CTRL_SIGDET_POLARITY            0x0001

//Extended PHY control 3 (ActiPHY) register
#define VSC8662_EXT_PHY_CTRL3_CARRIER_EXT_DIS                          0x8000
#define VSC8662_EXT_PHY_CTRL3_ACTIPHY_SLEEP_TIMER                      0x6000
#define VSC8662_EXT_PHY_CTRL3_ACTIPHY_WAKE_UP_TIMER                    0x1800
#define VSC8662_EXT_PHY_CTRL3_PHY_ADDR_REVERSAL                        0x0200
#define VSC8662_EXT_PHY_CTRL3_CLKOUT_FREQ                              0x0100
#define VSC8662_EXT_PHY_CTRL3_MEDIA_MODE_STATUS                        0x00C0
#define VSC8662_EXT_PHY_CTRL3_10BT_NO_PREAMBLE_MODE_EN                 0x0020
#define VSC8662_EXT_PHY_CTRL3_LINK_SPEED_AUTODOWNSHIFT_EN              0x0010
#define VSC8662_EXT_PHY_CTRL3_LINK_SPEED_AUTODOWNSHIFT_CTRL            0x000C
#define VSC8662_EXT_PHY_CTRL3_LINK_SPEED_AUTODOWNSHIFT_STAT            0x0002

//Extended PHY control 4 (PoE and CRC error counter) register
#define VSC8662_EXT_PHY_CTRL4_PHY_ADDR                                 0xF800
#define VSC8662_EXT_PHY_CTRL4_INLINE_PWR_DEVICE_DETECT                 0x0400
#define VSC8662_EXT_PHY_CTRL4_INLINE_PWR_DEVICE_DETECT_STAT            0x0300
#define VSC8662_EXT_PHY_CTRL4_CRC_ERROR_CNT                            0x00FF

//SerDes MAC/media status register
#define VSC8662_SERDES_MAC_MEDIA_STAT_MAC_SYNC_STATUS_FAIL             0x0800
#define VSC8662_SERDES_MAC_MEDIA_STAT_MAC_CGBAD                        0x0400
#define VSC8662_SERDES_MAC_MEDIA_STAT_MAC_PHASE_LOCK_LOSS              0x0200
#define VSC8662_SERDES_MAC_MEDIA_STAT_MAC_RX_PLL_LOCK_LOSS             0x0100
#define VSC8662_SERDES_MAC_MEDIA_STAT_SERDES_MEDIA_SYNC_STATUS_FAIL    0x0008
#define VSC8662_SERDES_MAC_MEDIA_STAT_SERDES_MEDIA_CGBAD               0x0004
#define VSC8662_SERDES_MAC_MEDIA_STAT_SERDES_MEDIA_PHASE_LOCK_LOSS     0x0002
#define VSC8662_SERDES_MAC_MEDIA_STAT_SERDES_MEDIA_RX_PLL_LOCK_LOSS    0x0001

//Ethernet packet generator control 1 register
#define VSC8662_EPG_CTRL1_EPG_EN                                       0x8000
#define VSC8662_EPG_CTRL1_EPG_RUN_STOP                                 0x4000
#define VSC8662_EPG_CTRL1_TRANSMISSION_DURATION                        0x2000
#define VSC8662_EPG_CTRL1_PACKET_LENGTH                                0x1800
#define VSC8662_EPG_CTRL1_INTER_PACKET_GAP                             0x0400
#define VSC8662_EPG_CTRL1_DEST_ADDR                                    0x03C0
#define VSC8662_EPG_CTRL1_SOURCE_ADDR                                  0x003C
#define VSC8662_EPG_CTRL1_PAYLOAD_TYPE                                 0x0002
#define VSC8662_EPG_CTRL1_BAD_FCS_GENERATION                           0x0001

//Ethernet packet generator control 2 register
#define VSC8662_EPG_CTRL2_EPG_PACKET_PAYLOAD                           0xFFFF

//SIGDET vs. GPIO control register
#define VSC8662_SIGDET_GPIO_CTRL_SIGDET1                               0x000C
#define VSC8662_SIGDET_GPIO_CTRL_SIGDET0                               0x0003

//Fast link fail control register
#define VSC8662_FAST_LINK_FAIL_CTRL_PORT                               0x0001

//I2C mux control 1 register
#define VSC8662_I2C_MUX_CTRL1_DEV_ADDR                                 0xFE00
#define VSC8662_I2C_MUX_CTRL1_SCL_CLOCK_FREQ                           0x0030
#define VSC8662_I2C_MUX_CTRL1_MUX_PORT_1_EN                            0x0002
#define VSC8662_I2C_MUX_CTRL1_MUX_PORT_0_EN                            0x0001

//I2C mux control 2 register
#define VSC8662_I2C_MUX_CTRL2_MUX_READY                                0x8000
#define VSC8662_I2C_MUX_CTRL2_PHY_PORT_ADDR                            0x0400
#define VSC8662_I2C_MUX_CTRL2_MUX_ACCESS_EN                            0x0200
#define VSC8662_I2C_MUX_CTRL2_MUX_READ_WRITE                           0x0100
#define VSC8662_I2C_MUX_CTRL2_MUX_ADDR                                 0x00FF

//I2C mux data read/write register
#define VSC8662_I2C_MUX_DATA_RW_MUX_READ_DATA                          0xFF00
#define VSC8662_I2C_MUX_DATA_RW_MUX_WRITE_DATA                         0x00FF

//Recovered clock 1 register
#define VSC8662_RECOVERED_CLOCK1_RCVRD_CLK1_EN                         0x8000
#define VSC8662_RECOVERED_CLOCK1_PHY_CLOCKOUT_SEL                      0x1000
#define VSC8662_RECOVERED_CLOCK1_CLOCK_FREQ                            0x0100
#define VSC8662_RECOVERED_CLOCK1_CLOCK_SQUELCH                         0x0030
#define VSC8662_RECOVERED_CLOCK1_CLOCK_SEL                             0x0003

//Recovered clock 2 register
#define VSC8662_RECOVERED_CLOCK2_RCVRD_CLK2_EN                         0x8000
#define VSC8662_RECOVERED_CLOCK2_PHY_CLOCKOUT_SEL                      0x1000
#define VSC8662_RECOVERED_CLOCK2_CLOCK_FREQ                            0x0100
#define VSC8662_RECOVERED_CLOCK2_CLOCK_SQUELCH                         0x0030
#define VSC8662_RECOVERED_CLOCK2_CLOCK_SEL                             0x0003

//LED port swapping register
#define VSC8662_LED_PORT_SWAPPING_LED_PORT_SWAPPING                    0x0001

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//VSC8662 Ethernet PHY driver
extern const PhyDriver vsc8662PhyDriver;

//VSC8662 related functions
error_t vsc8662Init(NetInterface *interface);
void vsc8662InitHook(NetInterface *interface);

void vsc8662Tick(NetInterface *interface);

void vsc8662EnableIrq(NetInterface *interface);
void vsc8662DisableIrq(NetInterface *interface);

void vsc8662EventHandler(NetInterface *interface);

void vsc8662WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t vsc8662ReadPhyReg(NetInterface *interface, uint8_t address);

void vsc8662DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
