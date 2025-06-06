/**
 * @file mv88e1112_driver.h
 * @brief 88E1112 Gigabit Ethernet PHY driver
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

#ifndef _MV88E1112_DRIVER_H
#define _MV88E1112_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef MV88E1112_PHY_ADDR
   #define MV88E1112_PHY_ADDR 0
#elif (MV88E1112_PHY_ADDR < 0 || MV88E1112_PHY_ADDR > 31)
   #error MV88E1112_PHY_ADDR parameter is not valid
#endif

//MV88E1112 PHY registers (page 0)
#define MV88E1112_COPPER_CTRL                                         0x00
#define MV88E1112_COPPER_STAT                                         0x01
#define MV88E1112_COPPER_PHYID1                                       0x02
#define MV88E1112_COPPER_PHYID2                                       0x03
#define MV88E1112_COPPER_ANAR                                         0x04
#define MV88E1112_COPPER_ANLPAR                                       0x05
#define MV88E1112_COPPER_ANER                                         0x06
#define MV88E1112_COPPER_ANNPTR                                       0x07
#define MV88E1112_COPPER_ANLPNPR                                      0x08
#define MV88E1112_GBCR                                                0x09
#define MV88E1112_GBSR                                                0x0A
#define MV88E1112_GBESR                                               0x0F
#define MV88E1112_COPPER_CTRL1                                        0x10
#define MV88E1112_COPPER_STAT1                                        0x11
#define MV88E1112_COPPER_INT_EN                                       0x12
#define MV88E1112_COPPER_STAT2                                        0x13
#define MV88E1112_COPPER_RX_ERR_COUNTER                               0x15
#define MV88E1112_PAGE_ADDR                                           0x16
#define MV88E1112_COPPER_CTRL2                                        0x1A

//MV88E1112 PHY registers (page 1)
#define MV88E1112_FIBER_CTRL                                          0x00
#define MV88E1112_FIBER_STAT                                          0x01
#define MV88E1112_FIBER_ANAR                                          0x04
#define MV88E1112_FIBER_ANLPAR                                        0x05
#define MV88E1112_FIBER_ANER                                          0x06
#define MV88E1112_FIBER_ANNPTR                                        0x07
#define MV88E1112_FIBER_ANLPNPR                                       0x08
#define MV88E1112_FIBER_CTRL1                                         0x10
#define MV88E1112_FIBER_STAT1                                         0x11
#define MV88E1112_FIBER_INT_EN                                        0x12
#define MV88E1112_FIBER_STAT2                                         0x13
#define MV88E1112_FIBER_CTRL2                                         0x1A

//MV88E1112 PHY registers (page 2)
#define MV88E1112_MAC_CTRL                                            0x00
#define MV88E1112_MAC_CTRL1                                           0x10
#define MV88E1112_MAC_STAT1                                           0x11
#define MV88E1112_MAC_INT_EN                                          0x12
#define MV88E1112_MAC_STAT2                                           0x13
#define MV88E1112_MAC_CTRL2                                           0x1A

//MV88E1112 PHY registers (page 3)
#define MV88E1112_LED_FUNC_CTRL                                       0x10
#define MV88E1112_LED_POL_CTRL                                        0x11
#define MV88E1112_LED_TIMER_CTRL                                      0x12

//MV88E1112 PHY registers (page 4)
#define MV88E1112_NVM_ADDR                                            0x10
#define MV88E1112_NVM_RD_DATA_STAT                                    0x11
#define MV88E1112_NVM_WR_DATA_CTRL                                    0x12
#define MV88E1112_RAM_DATA                                            0x13
#define MV88E1112_RAM_ADDR                                            0x14

//MV88E1112 PHY registers (page 5)
#define MV88E1112_MDI0_VCT_STAT                                       0x10
#define MV88E1112_MDI1_VCT_STAT                                       0x11
#define MV88E1112_MDI2_VCT_STAT                                       0x12
#define MV88E1112_MDI3_VCT_STAT                                       0x13
#define MV88E1112_1000BT_PAIR_SKEW                                    0x14
#define MV88E1112_1000BT_PAIR_SWAP_POL                                0x15
#define MV88E1112_VCT_DSP_DIST                                        0x1A

//MV88E1112 PHY registers (page 6)
#define MV88E1112_PKT_GEN                                             0x10
#define MV88E1112_CRC_COUNTERS                                        0x11

//Copper Control register
#define MV88E1112_COPPER_CTRL_RESET                                   0x8000
#define MV88E1112_COPPER_CTRL_LOOPBACK                                0x4000
#define MV88E1112_COPPER_CTRL_SPEED_SEL_LSB                           0x2000
#define MV88E1112_COPPER_CTRL_AN_EN                                   0x1000
#define MV88E1112_COPPER_CTRL_POWER_DOWN                              0x0800
#define MV88E1112_COPPER_CTRL_ISOLATE                                 0x0400
#define MV88E1112_COPPER_CTRL_RESTART_AN                              0x0200
#define MV88E1112_COPPER_CTRL_DUPLEX_MODE                             0x0100
#define MV88E1112_COPPER_CTRL_COL_TEST                                0x0080
#define MV88E1112_COPPER_CTRL_SPEED_SEL_MSB                           0x0040

//Copper Status register
#define MV88E1112_COPPER_STAT_100BT4                                  0x8000
#define MV88E1112_COPPER_STAT_100BTX_FD                               0x4000
#define MV88E1112_COPPER_STAT_100BTX_HD                               0x2000
#define MV88E1112_COPPER_STAT_10BT_FD                                 0x1000
#define MV88E1112_COPPER_STAT_10BT_HD                                 0x0800
#define MV88E1112_COPPER_STAT_100BT2_FD                               0x0400
#define MV88E1112_COPPER_STAT_100BT2_HD                               0x0200
#define MV88E1112_COPPER_STAT_EXTENDED_STATUS                         0x0100
#define MV88E1112_COPPER_STAT_MF_PREAMBLE_SUPPR                       0x0040
#define MV88E1112_COPPER_STAT_AN_COMPLETE                             0x0020
#define MV88E1112_COPPER_STAT_REMOTE_FAULT                            0x0010
#define MV88E1112_COPPER_STAT_AN_CAPABLE                              0x0008
#define MV88E1112_COPPER_STAT_LINK_STATUS                             0x0004
#define MV88E1112_COPPER_STAT_JABBER_DETECT                           0x0002
#define MV88E1112_COPPER_STAT_EXTENDED_CAPABLE                        0x0001

//PHY Identifier 1 register
#define MV88E1112_COPPER_PHYID1_OUI_MSB                               0xFFFF
#define MV88E1112_COPPER_PHYID1_OUI_MSB_DEFAULT                       0x0141

//PHY Identifier 2 register
#define MV88E1112_COPPER_PHYID2_OUI_LSB                               0xFC00
#define MV88E1112_COPPER_PHYID2_OUI_LSB_DEFAULT                       0x0C00
#define MV88E1112_COPPER_PHYID2_MODEL_NUM                             0x03F0
#define MV88E1112_COPPER_PHYID2_MODEL_NUM_DEFAULT                     0x0090
#define MV88E1112_COPPER_PHYID2_REVISION_NUM                          0x000F

//Copper Auto-Negotiation Advertisement register
#define MV88E1112_COPPER_ANAR_NEXT_PAGE                               0x8000
#define MV88E1112_COPPER_ANAR_ACK                                     0x4000
#define MV88E1112_COPPER_ANAR_REMOTE_FAULT                            0x2000
#define MV88E1112_COPPER_ANAR_ASYM_PAUSE                              0x0800
#define MV88E1112_COPPER_ANAR_PAUSE                                   0x0400
#define MV88E1112_COPPER_ANAR_100BT4                                  0x0200
#define MV88E1112_COPPER_ANAR_100BTX_FD                               0x0100
#define MV88E1112_COPPER_ANAR_100BTX_HD                               0x0080
#define MV88E1112_COPPER_ANAR_10BT_FD                                 0x0040
#define MV88E1112_COPPER_ANAR_10BT_HD                                 0x0020
#define MV88E1112_COPPER_ANAR_SELECTOR                                0x001F
#define MV88E1112_COPPER_ANAR_SELECTOR_DEFAULT                        0x0001

//Copper Link Partner Ability register
#define MV88E1112_COPPER_ANLPAR_NEXT_PAGE                             0x8000
#define MV88E1112_COPPER_ANLPAR_ACK                                   0x4000
#define MV88E1112_COPPER_ANLPAR_REMOTE_FAULT                          0x2000
#define MV88E1112_COPPER_ANLPAR_TECH_ABLE                             0x1000
#define MV88E1112_COPPER_ANLPAR_ASYM_PAUSE                            0x0800
#define MV88E1112_COPPER_ANLPAR_PAUSE                                 0x0400
#define MV88E1112_COPPER_ANLPAR_100BT4                                0x0200
#define MV88E1112_COPPER_ANLPAR_100BTX_FD                             0x0100
#define MV88E1112_COPPER_ANLPAR_100BTX_HD                             0x0080
#define MV88E1112_COPPER_ANLPAR_10BT_FD                               0x0040
#define MV88E1112_COPPER_ANLPAR_10BT_HD                               0x0020
#define MV88E1112_COPPER_ANLPAR_SELECTOR                              0x001F
#define MV88E1112_COPPER_ANLPAR_SELECTOR_DEFAULT                      0x0001

//Copper Auto-Negotiation Expansion register
#define MV88E1112_COPPER_ANER_PAR_DETECT_FAULT                        0x0010
#define MV88E1112_COPPER_ANER_LP_NEXT_PAGE_ABLE                       0x0008
#define MV88E1112_COPPER_ANER_NEXT_PAGE_ABLE                          0x0004
#define MV88E1112_COPPER_ANER_PAGE_RECEIVED                           0x0002
#define MV88E1112_COPPER_ANER_LP_AN_ABLE                              0x0001

//Copper Next Page Transmit register
#define MV88E1112_COPPER_ANNPTR_NEXT_PAGE                             0x8000
#define MV88E1112_COPPER_ANNPTR_MSG_PAGE                              0x2000
#define MV88E1112_COPPER_ANNPTR_ACK2                                  0x1000
#define MV88E1112_COPPER_ANNPTR_TOGGLE                                0x0800
#define MV88E1112_COPPER_ANNPTR_MESSAGE                               0x07FF

//Copper Link Partner Next Page register
#define MV88E1112_COPPER_ANLPNPR_NEXT_PAGE                            0x8000
#define MV88E1112_COPPER_ANLPNPR_ACK                                  0x4000
#define MV88E1112_COPPER_ANLPNPR_MSG_PAGE                             0x2000
#define MV88E1112_COPPER_ANLPNPR_ACK2                                 0x1000
#define MV88E1112_COPPER_ANLPNPR_TOGGLE                               0x0800
#define MV88E1112_COPPER_ANLPNPR_MESSAGE                              0x07FF

//1000BASE-T Control register
#define MV88E1112_GBCR_TEST_MODE                                      0xE000
#define MV88E1112_GBCR_MS_MAN_CONF_EN                                 0x1000
#define MV88E1112_GBCR_MS_MAN_CONF_VAL                                0x0800
#define MV88E1112_GBCR_PORT_TYPE                                      0x0400
#define MV88E1112_GBCR_1000BT_FD                                      0x0200
#define MV88E1112_GBCR_1000BT_HD                                      0x0100

//1000BASE-T Status register
#define MV88E1112_GBSR_MS_CONF_FAULT                                  0x8000
#define MV88E1112_GBSR_MS_CONF_RES                                    0x4000
#define MV88E1112_GBSR_LOCAL_RECEIVER_STATUS                          0x2000
#define MV88E1112_GBSR_REMOTE_RECEIVER_STATUS                         0x1000
#define MV88E1112_GBSR_LP_1000BT_FD                                   0x0800
#define MV88E1112_GBSR_LP_1000BT_HD                                   0x0400
#define MV88E1112_GBSR_IDLE_ERR_COUNT                                 0x00FF

//Extended Status register
#define MV88E1112_GBESR_1000BX_FD                                     0x8000
#define MV88E1112_GBESR_1000BX_HD                                     0x4000
#define MV88E1112_GBESR_1000BT_FD                                     0x2000
#define MV88E1112_GBESR_1000BT_HD                                     0x1000

//Copper Specific Control 1 register
#define MV88E1112_COPPER_CTRL1_LINK_PULSE_DIS                         0x8000
#define MV88E1112_COPPER_CTRL1_DOWNSHIFT_COUNTER                      0x7000
#define MV88E1112_COPPER_CTRL1_DOWNSHIFT_EN                           0x0800
#define MV88E1112_COPPER_CTRL1_FORCE_LINK_GOOD                        0x0400
#define MV88E1112_COPPER_CTRL1_ENERGY_DETECT                          0x0300
#define MV88E1112_COPPER_CTRL1_EXTENDED_DIST_EN                       0x0080
#define MV88E1112_COPPER_CTRL1_MDI_CROSSOVER_MODE                     0x0060
#define MV88E1112_COPPER_CTRL1_MDI_CROSSOVER_MODE_MANUAL_MDI          0x0000
#define MV88E1112_COPPER_CTRL1_MDI_CROSSOVER_MODE_MANUAL_MDIX         0x0020
#define MV88E1112_COPPER_CTRL1_MDI_CROSSOVER_MODE_AUTO                0x0060
#define MV88E1112_COPPER_CTRL1_TX_DIS                                 0x0008
#define MV88E1112_COPPER_CTRL1_POLARITY_REVERSAL_DIS                  0x0002
#define MV88E1112_COPPER_CTRL1_JABBER_DIS                             0x0001

//Copper Specific Status 1 register
#define MV88E1112_COPPER_STAT1_SPEED                                  0xC000
#define MV88E1112_COPPER_STAT1_SPEED_10MBPS                           0x0000
#define MV88E1112_COPPER_STAT1_SPEED_100MBPS                          0x4000
#define MV88E1112_COPPER_STAT1_SPEED_1000MBPS                         0x8000
#define MV88E1112_COPPER_STAT1_DUPLEX                                 0x2000
#define MV88E1112_COPPER_STAT1_PAGE_RECEIVED                          0x1000
#define MV88E1112_COPPER_STAT1_SPEED_DUPLEX_RESOLVED                  0x0800
#define MV88E1112_COPPER_STAT1_LINK                                   0x0400
#define MV88E1112_COPPER_STAT1_TX_PAUSE_EN                            0x0200
#define MV88E1112_COPPER_STAT1_RX_PAUSE_EN                            0x0100
#define MV88E1112_COPPER_STAT1_FIBER_COPPER_RESOLUTION                0x0080
#define MV88E1112_COPPER_STAT1_MDI_CROSSOVER_STATUS                   0x0040
#define MV88E1112_COPPER_STAT1_DOWNSHIFT_STATUS                       0x0020
#define MV88E1112_COPPER_STAT1_ENERGY_DETECT_STATUS                   0x0010
#define MV88E1112_COPPER_STAT1_GLOBAL_LINK_STATUS                     0x0008
#define MV88E1112_COPPER_STAT1_DTE_POWER_STATUS                       0x0004
#define MV88E1112_COPPER_STAT1_POLARITY                               0x0002
#define MV88E1112_COPPER_STAT1_JABBER                                 0x0001

//Copper Interrupt Enable register
#define MV88E1112_COPPER_INT_EN_AN_ERROR                              0x8000
#define MV88E1112_COPPER_INT_EN_SPEED_CHANGED                         0x4000
#define MV88E1112_COPPER_INT_EN_DUPLEX_CHANGED                        0x2000
#define MV88E1112_COPPER_INT_EN_PAGE_RECEIVED                         0x1000
#define MV88E1112_COPPER_INT_EN_AN_COMPLETE                           0x0800
#define MV88E1112_COPPER_INT_EN_LINK_STATUS_CHANGED                   0x0400
#define MV88E1112_COPPER_INT_EN_SYMBOL_ERROR                          0x0200
#define MV88E1112_COPPER_INT_EN_FALSE_CARRIER                         0x0100
#define MV88E1112_COPPER_INT_EN_MDI_CROSSOVER_CHANGED                 0x0040
#define MV88E1112_COPPER_INT_EN_DOWNSHIFT                             0x0020
#define MV88E1112_COPPER_INT_EN_ENERGY_DETECT                         0x0010
#define MV88E1112_COPPER_INT_EN_DTE_POWER_DET_STATUS_CHANGED          0x0004
#define MV88E1112_COPPER_INT_EN_POLARITY_CHANGED                      0x0002
#define MV88E1112_COPPER_INT_EN_JABBER                                0x0001

//Copper Specific Status 2 register
#define MV88E1112_COPPER_STAT2_AN_ERROR                               0x8000
#define MV88E1112_COPPER_STAT2_SPEED_CHANGED                          0x4000
#define MV88E1112_COPPER_STAT2_DUPLEX_CHANGED                         0x2000
#define MV88E1112_COPPER_STAT2_PAGE_RECEIVED                          0x1000
#define MV88E1112_COPPER_STAT2_AN_COMPLETE                            0x0800
#define MV88E1112_COPPER_STAT2_LINK_STATUS_CHANGED                    0x0400
#define MV88E1112_COPPER_STAT2_SYMBOL_ERROR                           0x0200
#define MV88E1112_COPPER_STAT2_FALSE_CARRIER                          0x0100
#define MV88E1112_COPPER_STAT2_MDI_CROSSOVER_CHANGED                  0x0040
#define MV88E1112_COPPER_STAT2_DOWNSHIFT                              0x0020
#define MV88E1112_COPPER_STAT2_ENERGY_DETECT                          0x0010
#define MV88E1112_COPPER_STAT2_DTE_POWER_DET_STATUS_CHANGED           0x0004
#define MV88E1112_COPPER_STAT2_POLARITY_CHANGED                       0x0002
#define MV88E1112_COPPER_STAT2_JABBER                                 0x0001

//Page Address register
#define MV88E1112_PAGE_ADDR_AUTO_MEDIUM_REG_SEL                       0x8000
#define MV88E1112_PAGE_ADDR_PAGE_SEL                                  0x00FF

//Copper Specific Control 2 register
#define MV88E1112_COPPER_CTRL2_TRANSMITTER_TYPE                       0x8000
#define MV88E1112_COPPER_CTRL2_TRANSMITTER_TYPE_CLASS_B               0x0000
#define MV88E1112_COPPER_CTRL2_TRANSMITTER_TYPE_CLASS_A               0x8000
#define MV88E1112_COPPER_CTRL2_DTE_DETECT_EN                          0x0100
#define MV88E1112_COPPER_CTRL2_DTE_DETECT_STATUS_DROP_HYST            0x00F0
#define MV88E1112_COPPER_CTRL2_DTE_DETECT_STATUS_DROP_HYST_DEFAULT    0x0040
#define MV88E1112_COPPER_CTRL2_100MB_TEST_SEL                         0x000C
#define MV88E1112_COPPER_CTRL2_100MB_TEST_SEL_NORMAL                  0x0000
#define MV88E1112_COPPER_CTRL2_100MB_TEST_SEL_112NS_SEQ               0x0008
#define MV88E1112_COPPER_CTRL2_100MB_TEST_SEL_16NS_SEQ                0x000C
#define MV88E1112_COPPER_CTRL2_10BT_POL_FORCE                         0x0002

//Fiber Control register
#define MV88E1112_FIBER_CTRL_RESET                                    0x8000
#define MV88E1112_FIBER_CTRL_LOOPBACK                                 0x4000
#define MV88E1112_FIBER_CTRL_SPEED_SEL_LSB                            0x2000
#define MV88E1112_FIBER_CTRL_AN_EN                                    0x1000
#define MV88E1112_FIBER_CTRL_POWER_DOWN                               0x0800
#define MV88E1112_FIBER_CTRL_ISOLATE                                  0x0400
#define MV88E1112_FIBER_CTRL_RESTART_AN                               0x0200
#define MV88E1112_FIBER_CTRL_DUPLEX_MODE                              0x0100
#define MV88E1112_FIBER_CTRL_COL_TEST                                 0x0080
#define MV88E1112_FIBER_CTRL_SPEED_SEL_MSB                            0x0040

//Fiber Status register
#define MV88E1112_FIBER_STAT_100BT4                                   0x8000
#define MV88E1112_FIBER_STAT_100BTX_FD                                0x4000
#define MV88E1112_FIBER_STAT_100BTX_HD                                0x2000
#define MV88E1112_FIBER_STAT_10BT_FD                                  0x1000
#define MV88E1112_FIBER_STAT_10BT_HD                                  0x0800
#define MV88E1112_FIBER_STAT_100BT2_FD                                0x0400
#define MV88E1112_FIBER_STAT_100BT2_HD                                0x0200
#define MV88E1112_FIBER_STAT_EXTENDED_STATUS                          0x0100
#define MV88E1112_FIBER_STAT_MF_PREAMBLE_SUPPR                        0x0040
#define MV88E1112_FIBER_STAT_AN_COMPLETE                              0x0020
#define MV88E1112_FIBER_STAT_REMOTE_FAULT                             0x0010
#define MV88E1112_FIBER_STAT_AN_CAPABLE                               0x0008
#define MV88E1112_FIBER_STAT_LINK_STATUS                              0x0004
#define MV88E1112_FIBER_STAT_EXTENDED_CAPABLE                         0x0001

//Fiber Auto-Negotiation Advertisement register
#define MV88E1112_FIBER_ANAR_NEXT_PAGE                                0x8000
#define MV88E1112_FIBER_ANAR_REMOTE_FAULT                             0x3000
#define MV88E1112_FIBER_ANAR_REMOTE_FAULT_NO_ERROR                    0x0000
#define MV88E1112_FIBER_ANAR_REMOTE_FAULT_LINK_FAILURE                0x1000
#define MV88E1112_FIBER_ANAR_REMOTE_FAULT_OFFLINE                     0x2000
#define MV88E1112_FIBER_ANAR_REMOTE_FAULT_AN_ERROR                    0x3000
#define MV88E1112_FIBER_ANAR_PAUSE                                    0x0180
#define MV88E1112_FIBER_ANAR_1000BX_HD                                0x0040
#define MV88E1112_FIBER_ANAR_1000BX_FD                                0x0020

//Fiber Link Partner Ability register
#define MV88E1112_FIBER_ANLPAR_NEXT_PAGE                              0x8000
#define MV88E1112_FIBER_ANLPAR_ACK                                    0x4000
#define MV88E1112_FIBER_ANLPAR_REMOTE_FAULT                           0x3000
#define MV88E1112_FIBER_ANLPAR_REMOTE_FAULT_NO_ERROR                  0x0000
#define MV88E1112_FIBER_ANLPAR_REMOTE_FAULT_LINK_FAILURE              0x1000
#define MV88E1112_FIBER_ANLPAR_REMOTE_FAULT_OFFLINE                   0x2000
#define MV88E1112_FIBER_ANLPAR_REMOTE_FAULT_AN_ERROR                  0x3000
#define MV88E1112_FIBER_ANLPAR_ASYM_PAUSE                             0x0180
#define MV88E1112_FIBER_ANLPAR_1000BX_HD                              0x0040
#define MV88E1112_FIBER_ANLPAR_1000BX_FD                              0x0020

//Fiber Auto-Negotiation Expansion register
#define MV88E1112_FIBER_ANER_LP_NEXT_PAGE_ABLE                        0x0008
#define MV88E1112_FIBER_ANER_NEXT_PAGE_ABLE                           0x0004
#define MV88E1112_FIBER_ANER_PAGE_RECEIVED                            0x0002
#define MV88E1112_FIBER_ANER_LP_AN_ABLE                               0x0001

//Fiber Next Page Transmit register
#define MV88E1112_FIBER_ANNPTR_NEXT_PAGE                              0x8000
#define MV88E1112_FIBER_ANNPTR_MSG_PAGE                               0x2000
#define MV88E1112_FIBER_ANNPTR_ACK2                                   0x1000
#define MV88E1112_FIBER_ANNPTR_TOGGLE                                 0x0800
#define MV88E1112_FIBER_ANNPTR_MESSAGE                                0x07FF

//Fiber Link Partner Next Page register
#define MV88E1112_FIBER_ANLPNPR_NEXT_PAGE                             0x8000
#define MV88E1112_FIBER_ANLPNPR_ACK                                   0x4000
#define MV88E1112_FIBER_ANLPNPR_MSG_PAGE                              0x2000
#define MV88E1112_FIBER_ANLPNPR_ACK2                                  0x1000
#define MV88E1112_FIBER_ANLPNPR_TOGGLE                                0x0800
#define MV88E1112_FIBER_ANLPNPR_MESSAGE                               0x07FF

//Fiber Specific Control 1 register
#define MV88E1112_FIBER_CTRL1_FEFI_EN                                 0x0800
#define MV88E1112_FIBER_CTRL1_FORCE_LINK_GOOD                         0x0400
#define MV88E1112_FIBER_CTRL1_SIGDET_POL                              0x0200
#define MV88E1112_FIBER_CTRL1_TX_DIS                                  0x0008

//Fiber Specific Status 1 register
#define MV88E1112_FIBER_STAT1_SPEED                                   0xC000
#define MV88E1112_FIBER_STAT1_SPEED_100MBPS                           0x4000
#define MV88E1112_FIBER_STAT1_DUPLEX                                  0x2000
#define MV88E1112_FIBER_STAT1_PAGE_RECEIVED                           0x1000
#define MV88E1112_FIBER_STAT1_SPEED_DUPLEX_RESOLVED                   0x0800
#define MV88E1112_FIBER_STAT1_LINK                                    0x0400
#define MV88E1112_FIBER_STAT1_TX_PAUSE_EN                             0x0200
#define MV88E1112_FIBER_STAT1_RX_PAUSE_EN                             0x0100
#define MV88E1112_FIBER_STAT1_FIBER_COPPER_RESOLUTION                 0x0080
#define MV88E1112_FIBER_STAT1_AN_BYPASS_STATUS                        0x0020
#define MV88E1112_FIBER_STAT1_AUTOSEL_FIBER_ED_STATUS                 0x0010
#define MV88E1112_FIBER_STAT1_GLOBAL_LINK_STATUS                      0x0008

//Fiber Interrupt Enable register
#define MV88E1112_FIBER_INT_EN_DUPLEX_CHANGED                         0x2000
#define MV88E1112_FIBER_INT_EN_PAGE_RECEIVED                          0x1000
#define MV88E1112_FIBER_INT_EN_AN_COMPLETE                            0x0800
#define MV88E1112_FIBER_INT_EN_LINK_STATUS_CHANGED                    0x0400
#define MV88E1112_FIBER_INT_EN_SYMBOL_ERROR                           0x0200
#define MV88E1112_FIBER_INT_EN_FALSE_CARRIER                          0x0100
#define MV88E1112_FIBER_INT_EN_ENERGY_DETECT                          0x0010

//Fiber Specific Status 2 register
#define MV88E1112_FIBER_STAT2_DUPLEX_CHANGED                          0x2000
#define MV88E1112_FIBER_STAT2_PAGE_RECEIVED                           0x1000
#define MV88E1112_FIBER_STAT2_AN_COMPLETE                             0x0800
#define MV88E1112_FIBER_STAT2_LINK_STATUS_CHANGED                     0x0400
#define MV88E1112_FIBER_STAT2_SYMBOL_ERROR                            0x0200
#define MV88E1112_FIBER_STAT2_FALSE_CARRIER                           0x0100
#define MV88E1112_FIBER_STAT2_ENERGY_DETECT                           0x0010

//Fiber Specific Control 2 register
#define MV88E1112_FIBER_CTRL2_AN_BYPASS_EN                            0x8000
#define MV88E1112_FIBER_CTRL2_SGMII_AN_TIMER                          0x6000
#define MV88E1112_FIBER_CTRL2_SGMII_AN_TIMER_1_6MS_TO_2_0MS           0x0000
#define MV88E1112_FIBER_CTRL2_SGMII_AN_TIMER_0_5US_TO_0_6US           0x2000
#define MV88E1112_FIBER_CTRL2_SGMII_AN_TIMER_0_98US_TO_1_2US          0x4000
#define MV88E1112_FIBER_CTRL2_SGMII_AN_TIMER_2_1US_TO_2_3US           0x6000
#define MV88E1112_FIBER_CTRL2_EXT_FIBER_SIG_DETECT_IN_EN              0x0080
#define MV88E1112_FIBER_CTRL2_OUT_AMPLITUDE                           0x0007
#define MV88E1112_FIBER_CTRL2_OUT_AMPLITUDE_0_50V                     0x0000
#define MV88E1112_FIBER_CTRL2_OUT_AMPLITUDE_0_60V                     0x0001
#define MV88E1112_FIBER_CTRL2_OUT_AMPLITUDE_0_70V                     0x0002
#define MV88E1112_FIBER_CTRL2_OUT_AMPLITUDE_0_80V                     0x0003
#define MV88E1112_FIBER_CTRL2_OUT_AMPLITUDE_0_90V                     0x0004
#define MV88E1112_FIBER_CTRL2_OUT_AMPLITUDE_1_00V                     0x0005
#define MV88E1112_FIBER_CTRL2_OUT_AMPLITUDE_1_10V                     0x0006
#define MV88E1112_FIBER_CTRL2_OUT_AMPLITUDE_1_20V                     0x0007

//MAC Control register
#define MV88E1112_MAC_CTRL_RESET                                      0x8000
#define MV88E1112_MAC_CTRL_LOOPBACK                                   0x4000
#define MV88E1112_MAC_CTRL_SPEED_SEL_LSB                              0x2000
#define MV88E1112_MAC_CTRL_AN_EN                                      0x1000
#define MV88E1112_MAC_CTRL_POWER_DOWN                                 0x0800
#define MV88E1112_MAC_CTRL_SPEED_SEL_MSB                              0x0040

//MAC Specific Control 1 register
#define MV88E1112_MAC_CTRL1_TX_FIFO_DEPTH                             0xC000
#define MV88E1112_MAC_CTRL1_RX_FIFO_DEPTH                             0x3000
#define MV88E1112_MAC_CTRL1_AUTOSEL_PREF_MEDIA                        0x0C00
#define MV88E1112_MAC_CTRL1_MODE_SELECT                               0x0380
#define MV88E1112_MAC_CTRL1_MODE_SELECT_100FX                         0x0000
#define MV88E1112_MAC_CTRL1_MODE_SELECT_COPPER_GBIC                   0x0080
#define MV88E1112_MAC_CTRL1_MODE_SELECT_AUTO_COPPER_SGMII             0x0100
#define MV88E1112_MAC_CTRL1_MODE_SELECT_AUTO_COPPER_1000BX            0x0180
#define MV88E1112_MAC_CTRL1_MODE_SELECT_COPPER_ONLY                   0x0280
#define MV88E1112_MAC_CTRL1_MODE_SELECT_SGMII_ONLY                    0x0300
#define MV88E1112_MAC_CTRL1_MODE_SELECT_1000BX_ONLY                   0x0380
#define MV88E1112_MAC_CTRL1_SGMII_MAC_POWER_DOWN                      0x0008
#define MV88E1112_MAC_CTRL1_ENHANCED_SGMII                            0x0004

//MAC Specific Status 1 register
#define MV88E1112_MAC_STAT1_AN_BYPASS_STATUS                          0x0020

//MAC Interrupt Enable register
#define MV88E1112_MAC_INT_EN_FIFO_OVER_UNDERFLOW                      0x0080

//MAC Specific Status 2 register
#define MV88E1112_MAC_STAT2_FIFO_OVER_UNDERFLOW                       0x0080

//MAC Specific Control 2 register
#define MV88E1112_MAC_CTRL2_AN_BYPASS_EN                              0x8000
#define MV88E1112_MAC_CTRL2_SGMII_MAC_AN_TIMER                        0x6000
#define MV88E1112_MAC_CTRL2_SGMII_MAC_AN_TIMER_1_6MS_TO_2_0MS         0x0000
#define MV88E1112_MAC_CTRL2_SGMII_MAC_AN_TIMER_0_5US_TO_0_6US         0x2000
#define MV88E1112_MAC_CTRL2_SGMII_MAC_AN_TIMER_0_98US_TO_1_2US        0x4000
#define MV88E1112_MAC_CTRL2_SGMII_MAC_AN_TIMER_2_1US_TO_2_3US         0x6000
#define MV88E1112_MAC_CTRL2_FORCE_FIBER_LINK_DOWN                     0x0080
#define MV88E1112_MAC_CTRL2_SGMII_MAC_CLK_EN                          0x0010
#define MV88E1112_MAC_CTRL2_OUT_AMPLITUDE                             0x0007
#define MV88E1112_MAC_CTRL2_OUT_AMPLITUDE_0_50V                       0x0000
#define MV88E1112_MAC_CTRL2_OUT_AMPLITUDE_0_60V                       0x0001
#define MV88E1112_MAC_CTRL2_OUT_AMPLITUDE_0_70V                       0x0002
#define MV88E1112_MAC_CTRL2_OUT_AMPLITUDE_0_80V                       0x0003
#define MV88E1112_MAC_CTRL2_OUT_AMPLITUDE_0_90V                       0x0004
#define MV88E1112_MAC_CTRL2_OUT_AMPLITUDE_1_00V                       0x0005
#define MV88E1112_MAC_CTRL2_OUT_AMPLITUDE_1_10V                       0x0006
#define MV88E1112_MAC_CTRL2_OUT_AMPLITUDE_1_20V                       0x0007

//LOS, INIT, STATUS[1:0] Function Control register
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL                              0xF000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_NORMAL                       0x0000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_LINK_ACT                     0x1000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_LINK_RX                      0x2000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_ACT                          0x3000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_ACT_BLINK                    0x4000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_TX                           0x5000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_SPEED_10_1000                0x6000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_LINK_FIBER                   0x7000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_FORCE_OFF                    0x8000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_FORCE_ON                     0x9000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_FORCE_HIZ                    0xA000
#define MV88E1112_LED_FUNC_CTRL_LOS_CTRL_FORCE_BLINK                  0xB000
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL                             0x0F00
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_LINK                        0x0000
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_LINK_ACT                    0x0100
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_NORMAL                      0x0200
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_ACT                         0x0300
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_ACT_BLINK                   0x0400
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_TX                          0x0500
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_SPEED_10_1000               0x0600
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_SPEED_10                    0x0700
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_FORCE_OFF                   0x0800
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_FORCE_ON                    0x0900
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_FORCE_HIZ                   0x0A00
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_FORCE_BLINK                 0x0B00
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_MODE1                       0x0C00
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_MODE2                       0x0D00
#define MV88E1112_LED_FUNC_CTRL_INIT_CTRL_INT                         0x0E00
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL                          0x00F0
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_COPPER_LINK              0x0000
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_LINK_ACT                 0x0010
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_LINK_RX                  0x00A0
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_LINK                     0x00B0
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_LINK_BLINK               0x0640
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_SPEED_100_FIBER          0x0650
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_SPEED_100_1000           0x06E0
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_SPEED_100                0x06F0
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_FORCE_OFF                0x3E80
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_FORCE_ON                 0x3E90
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_FORCE_HIZ                0x3F20
#define MV88E1112_LED_FUNC_CTRL_STATUS1_CTRL_FORCE_BLINK              0x3F30
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL                          0x000F
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_LINK                     0x0000
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_LINK_ACT                 0x0001
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_SPEED_BLINK              0x0002
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_ACT                      0x0003
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_ACT_BLINK                0x0004
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_TX                       0x0005
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_COPPER_LINK              0x0006
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_SPEED_1000               0x0007
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_FORCE_OFF                0x0008
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_FORCE_ON                 0x0009
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_FORCE_HIZ                0x000A
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_FORCE_BLINK              0x000B
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_MODE1                    0x000C
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_MODE2                    0x000D
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_MODE3                    0x000E
#define MV88E1112_LED_FUNC_CTRL_STATUS0_CTRL_MODE4                    0x000F

//LOS, INIT, STATUS[1:0] Polarity Control register
#define MV88E1112_LED_POL_CTRL_LOS_STATUS1_MIX                        0xF000
#define MV88E1112_LED_POL_CTRL_LOS_STATUS1_MIX_0                      0x0000
#define MV88E1112_LED_POL_CTRL_LOS_STATUS1_MIX_12_5                   0x0000
#define MV88E1112_LED_POL_CTRL_LOS_STATUS1_MIX_87_5                   0x7000
#define MV88E1112_LED_POL_CTRL_LOS_STATUS1_MIX_100                    0x8000
#define MV88E1112_LED_POL_CTRL_INIT_STATUS0_MIX                       0x0F00
#define MV88E1112_LED_POL_CTRL_INIT_STATUS0_MIX_0                     0x0000
#define MV88E1112_LED_POL_CTRL_INIT_STATUS0_MIX_12_5                  0x0000
#define MV88E1112_LED_POL_CTRL_INIT_STATUS0_MIX_87_5                  0x0700
#define MV88E1112_LED_POL_CTRL_INIT_STATUS0_MIX_100                   0x0800
#define MV88E1112_LED_POL_CTRL_LOS_POL                                0x00C0
#define MV88E1112_LED_POL_CTRL_LOS_POL_ON_LOW_OFF_HIGH                0x0000
#define MV88E1112_LED_POL_CTRL_LOS_POL_ON_LOW_OFF_TRIS                0x0080
#define MV88E1112_LED_POL_CTRL_LOS_POL_ON_HIGH_OFF_TRIS               0x00C0
#define MV88E1112_LED_POL_CTRL_INIT_POL                               0x0030
#define MV88E1112_LED_POL_CTRL_INIT_POL_ON_LOW_OFF_HIGH               0x0000
#define MV88E1112_LED_POL_CTRL_INIT_POL_ON_HIGH_OFF_LOW               0x0010
#define MV88E1112_LED_POL_CTRL_INIT_POL_ON_LOW_OFF_TRIS               0x0020
#define MV88E1112_LED_POL_CTRL_INIT_POL_ON_HIGH_OFF_TRIS              0x0030
#define MV88E1112_LED_POL_CTRL_STATUS1_POL                            0x000C
#define MV88E1112_LED_POL_CTRL_STATUS1_POL_ON_LOW_OFF_HIGH            0x0000
#define MV88E1112_LED_POL_CTRL_STATUS1_POL_ON_HIGH_OFF_LOW            0x0004
#define MV88E1112_LED_POL_CTRL_STATUS1_POL_ON_LOW_OFF_TRIS            0x0008
#define MV88E1112_LED_POL_CTRL_STATUS1_POL_ON_HIGH_OFF_TRIS           0x000C
#define MV88E1112_LED_POL_CTRL_STATUS0_POL                            0x0003
#define MV88E1112_LED_POL_CTRL_STATUS0_POL_ON_LOW_OFF_HIGH            0x0000
#define MV88E1112_LED_POL_CTRL_STATUS0_POL_ON_HIGH_OFF_LOW            0x0001
#define MV88E1112_LED_POL_CTRL_STATUS0_POL_ON_LOW_OFF_TRIS            0x0002
#define MV88E1112_LED_POL_CTRL_STATUS0_POL_ON_HIGH_OFF_TRIS           0x0003

//LOS, INIT, STATUS[1:0] Timer Control register
#define MV88E1112_LED_TIMER_CTRL_PULSE_STRETCH                        0x7000
#define MV88E1112_LED_TIMER_CTRL_PULSE_STRETCH_NO                     0x0000
#define MV88E1112_LED_TIMER_CTRL_PULSE_STRETCH_21MS_TO_42MS           0x1000
#define MV88E1112_LED_TIMER_CTRL_PULSE_STRETCH_42MS_TO_84MS           0x2000
#define MV88E1112_LED_TIMER_CTRL_PULSE_STRETCH_84MS_TO_170MS          0x3000
#define MV88E1112_LED_TIMER_CTRL_PULSE_STRETCH_170MS_TO_340MS         0x4000
#define MV88E1112_LED_TIMER_CTRL_PULSE_STRETCH_340MS_TO_670MS         0x5000
#define MV88E1112_LED_TIMER_CTRL_PULSE_STRETCH_670MS_TO_1_3S          0x6000
#define MV88E1112_LED_TIMER_CTRL_PULSE_STRETCH_1_3S_TO_2_7S           0x7000
#define MV88E1112_LED_TIMER_CTRL_BLINK_RATE                           0x0700
#define MV88E1112_LED_TIMER_CTRL_BLINK_RATE_42MS                      0x0000
#define MV88E1112_LED_TIMER_CTRL_BLINK_RATE_84MS                      0x0100
#define MV88E1112_LED_TIMER_CTRL_BLINK_RATE_170MS                     0x0200
#define MV88E1112_LED_TIMER_CTRL_BLINK_RATE_340MS                     0x0300
#define MV88E1112_LED_TIMER_CTRL_BLINK_RATE_670MS                     0x0400
#define MV88E1112_LED_TIMER_CTRL_SPEED_OFF_PULSE                      0x000C
#define MV88E1112_LED_TIMER_CTRL_SPEED_OFF_PULSE_84MS                 0x0000
#define MV88E1112_LED_TIMER_CTRL_SPEED_OFF_PULSE_170MS                0x0004
#define MV88E1112_LED_TIMER_CTRL_SPEED_OFF_PULSE_340MS                0x0008
#define MV88E1112_LED_TIMER_CTRL_SPEED_OFF_PULSE_670MS                0x000C
#define MV88E1112_LED_TIMER_CTRL_SPEED_ON_PULSE                       0x0003
#define MV88E1112_LED_TIMER_CTRL_SPEED_ON_PULSE_84MS                  0x0000
#define MV88E1112_LED_TIMER_CTRL_SPEED_ON_PULSE_170MS                 0x0001
#define MV88E1112_LED_TIMER_CTRL_SPEED_ON_PULSE_340MS                 0x0002
#define MV88E1112_LED_TIMER_CTRL_SPEED_ON_PULSE_670MS                 0x0003

//Non-Volatile Memory Address register
#define MV88E1112_NVM_ADDR_SLAVE_ADDR                                 0xFE00
#define MV88E1112_NVM_ADDR_RD_WR                                      0x0100
#define MV88E1112_NVM_ADDR_BYTE_ADDR                                  0x00FF

//Non-Volatile Memory Read Data and Status register
#define MV88E1112_NVM_RD_DATA_STAT_RAM_STATUS                         0xE000
#define MV88E1112_NVM_RD_DATA_STAT_TWI_STATUS                         0x0700
#define MV88E1112_NVM_RD_DATA_STAT_READ_DATA                          0x00FF

//Non-Volatile Memory Write Data and Control register
#define MV88E1112_NVM_WR_DATA_CTRL_EEPROM_WRITE_TIME                  0xF000
#define MV88E1112_NVM_WR_DATA_CTRL_EEPROM_WRITE_TIME_0MS              0x0000
#define MV88E1112_NVM_WR_DATA_CTRL_EEPROM_WRITE_TIME_1_05MS           0x1000
#define MV88E1112_NVM_WR_DATA_CTRL_EEPROM_WRITE_TIME_14_68MS          0xE000
#define MV88E1112_NVM_WR_DATA_CTRL_EEPROM_WRITE_TIME_15_73MS          0xF000
#define MV88E1112_NVM_WR_DATA_CTRL_RELOAD_EEPROM_VALUES               0x0C00
#define MV88E1112_NVM_WR_DATA_CTRL_RELOAD_EEPROM_VALUES_NORMAL        0x0000
#define MV88E1112_NVM_WR_DATA_CTRL_RELOAD_EEPROM_VALUES_RELOAD        0x0800
#define MV88E1112_NVM_WR_DATA_CTRL_RELOAD_EEPROM_VALUES_RELOAD_REINIT 0x0C00
#define MV88E1112_NVM_WR_DATA_CTRL_AUTO_RB_AFTER_WR                   0x0200
#define MV88E1112_NVM_WR_DATA_CTRL_WRITE_DATA                         0x00FF

//RAM Data register
#define MV88E1112_RAM_DATA_RAM_DATA                                   0x00FF

//RAM Address register
#define MV88E1112_RAM_ADDR_RAM_BYTE_ADDR                              0x00FF

//MDI[0] Virtual Cable Tester Status register
#define MV88E1112_MDI0_VCT_STAT_RUN_VCT_TEST                          0x8000
#define MV88E1112_MDI0_VCT_STAT_STATUS                                0x6000
#define MV88E1112_MDI0_VCT_STAT_STATUS_VALID_NORMAL                   0x0000
#define MV88E1112_MDI0_VCT_STAT_STATUS_VALID_SHORT                    0x2000
#define MV88E1112_MDI0_VCT_STAT_STATUS_VALID_OPEN                     0x4000
#define MV88E1112_MDI0_VCT_STAT_STATUS_FAILET                         0x6000
#define MV88E1112_MDI0_VCT_STAT_AMPLITUDE                             0x1F00
#define MV88E1112_MDI0_VCT_STAT_AMPLITUDE_MAX_NEG                     0x0000
#define MV88E1112_MDI0_VCT_STAT_AMPLITUDE_ZERO                        0x1000
#define MV88E1112_MDI0_VCT_STAT_AMPLITUDE_MAX_POS                     0x1F00
#define MV88E1112_MDI0_VCT_STAT_DISTANCE                              0x00FF

//MDI[1] Virtual Cable Tester Status register
#define MV88E1112_MDI1_VCT_STAT_WAITING_PERIOD_DIS                    0x8000
#define MV88E1112_MDI1_VCT_STAT_STATUS                                0x6000
#define MV88E1112_MDI1_VCT_STAT_STATUS_VALID_NORMAL                   0x0000
#define MV88E1112_MDI1_VCT_STAT_STATUS_VALID_SHORT                    0x2000
#define MV88E1112_MDI1_VCT_STAT_STATUS_VALID_OPEN                     0x4000
#define MV88E1112_MDI1_VCT_STAT_STATUS_FAILET                         0x6000
#define MV88E1112_MDI1_VCT_STAT_AMPLITUDE                             0x1F00
#define MV88E1112_MDI1_VCT_STAT_AMPLITUDE_MAX_NEG                     0x0000
#define MV88E1112_MDI1_VCT_STAT_AMPLITUDE_ZERO                        0x1000
#define MV88E1112_MDI1_VCT_STAT_AMPLITUDE_MAX_POS                     0x1F00
#define MV88E1112_MDI1_VCT_STAT_DISTANCE                              0x00FF

//MDI[2] Virtual Cable Tester Status register
#define MV88E1112_MDI2_VCT_STAT_STATUS                                0x6000
#define MV88E1112_MDI2_VCT_STAT_STATUS_VALID_NORMAL                   0x0000
#define MV88E1112_MDI2_VCT_STAT_STATUS_VALID_SHORT                    0x2000
#define MV88E1112_MDI2_VCT_STAT_STATUS_VALID_OPEN                     0x4000
#define MV88E1112_MDI2_VCT_STAT_STATUS_FAILET                         0x6000
#define MV88E1112_MDI2_VCT_STAT_AMPLITUDE                             0x1F00
#define MV88E1112_MDI2_VCT_STAT_AMPLITUDE_MAX_NEG                     0x0000
#define MV88E1112_MDI2_VCT_STAT_AMPLITUDE_ZERO                        0x1000
#define MV88E1112_MDI2_VCT_STAT_AMPLITUDE_MAX_POS                     0x1F00
#define MV88E1112_MDI2_VCT_STAT_DISTANCE                              0x00FF

//MDI[3] Virtual Cable Tester Status register
#define MV88E1112_MDI3_VCT_STAT_STATUS                                0x6000
#define MV88E1112_MDI3_VCT_STAT_STATUS_VALID_NORMAL                   0x0000
#define MV88E1112_MDI3_VCT_STAT_STATUS_VALID_SHORT                    0x2000
#define MV88E1112_MDI3_VCT_STAT_STATUS_VALID_OPEN                     0x4000
#define MV88E1112_MDI3_VCT_STAT_STATUS_FAILET                         0x6000
#define MV88E1112_MDI3_VCT_STAT_AMPLITUDE                             0x1F00
#define MV88E1112_MDI3_VCT_STAT_AMPLITUDE_MAX_NEG                     0x0000
#define MV88E1112_MDI3_VCT_STAT_AMPLITUDE_ZERO                        0x1000
#define MV88E1112_MDI3_VCT_STAT_AMPLITUDE_MAX_POS                     0x1F00
#define MV88E1112_MDI3_VCT_STAT_DISTANCE                              0x00FF

//1000 BASE-T Pair Skew register
#define MV88E1112_1000BT_PAIR_SKEW_PAIR_7_8_MDI3                      0xF000
#define MV88E1112_1000BT_PAIR_SKEW_PAIR_4_5_MDI2                      0x0F00
#define MV88E1112_1000BT_PAIR_SKEW_PAIR_3_6_MDI1                      0x00F0
#define MV88E1112_1000BT_PAIR_SKEW_PAIR_3_6_MDI0                      0x000F

//1000 BASE-T Pair Swap and Polarity register
#define MV88E1112_1000BT_PAIR_SWAP_POL_REG_21_4_21_5_VALID            0x0040
#define MV88E1112_1000BT_PAIR_SWAP_POL_C_D_CROSSOVER                  0x0020
#define MV88E1112_1000BT_PAIR_SWAP_POL_A_B_CROSSOVER                  0x0010
#define MV88E1112_1000BT_PAIR_SWAP_POL_PAIR_7_8_MDI3_POL              0x0008
#define MV88E1112_1000BT_PAIR_SWAP_POL_PAIR_4_5_MDI2_POL              0x0004
#define MV88E1112_1000BT_PAIR_SWAP_POL_PAIR_3_6_MDI1_POL              0x0002
#define MV88E1112_1000BT_PAIR_SWAP_POL_PAIR_1_2_MDI0_POL              0x0001

//VCT DSP Distance register
#define MV88E1112_VCT_DSP_DIST_CABLE_LENGTH                           0x0007

//Packet Generation register
#define MV88E1112_PKT_GEN_STUB_TEST_EN                                0x0020
#define MV88E1112_PKT_GEN_CRC_CHECKER_EN                              0x0010
#define MV88E1112_PKT_GEN_PKT_GEN_EN                                  0x0008
#define MV88E1112_PKT_GEN_PAYLOAD                                     0x0004
#define MV88E1112_PKT_GEN_PAYLOAD_PSEUDO_RANDOM                       0x0000
#define MV88E1112_PKT_GEN_PAYLOAD_5A_A5                               0x0004
#define MV88E1112_PKT_GEN_LENGTH                                      0x0002
#define MV88E1112_PKT_GEN_LENGTH_64B                                  0x0000
#define MV88E1112_PKT_GEN_LENGTH_1518B                                0x0002
#define MV88E1112_PKT_GEN_TX_ERR_PKT                                  0x0001

//CRC Counters register
#define MV88E1112_CRC_COUNTERS_CRC_PKT_COUNT                          0xFF00
#define MV88E1112_CRC_COUNTERS_CRC_ERR_COUNT                          0x00FF

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//88E1112 Ethernet PHY driver
extern const PhyDriver mv88e1112PhyDriver;

//88E1112 related functions
error_t mv88e1112Init(NetInterface *interface);
void mv88e1112InitHook(NetInterface *interface);

void mv88e1112Tick(NetInterface *interface);

void mv88e1112EnableIrq(NetInterface *interface);
void mv88e1112DisableIrq(NetInterface *interface);

void mv88e1112EventHandler(NetInterface *interface);

void mv88e1112WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t mv88e1112ReadPhyReg(NetInterface *interface, uint8_t address);

void mv88e1112DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
