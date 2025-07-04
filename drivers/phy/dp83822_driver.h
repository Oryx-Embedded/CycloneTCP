/**
 * @file dp83822_driver.h
 * @brief DP83822 Ethernet PHY driver
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

#ifndef _DP83822_DRIVER_H
#define _DP83822_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef DP83822_PHY_ADDR
   #define DP83822_PHY_ADDR 1
#elif (DP83822_PHY_ADDR < 0 || DP83822_PHY_ADDR > 31)
   #error DP83822_PHY_ADDR parameter is not valid
#endif

//DP83822 PHY registers
#define DP83822_BMCR                               0x00
#define DP83822_BMSR                               0x01
#define DP83822_PHYIDR1                            0x02
#define DP83822_PHYIDR2                            0x03
#define DP83822_ANAR                               0x04
#define DP83822_ANLPAR                             0x05
#define DP83822_ANER                               0x06
#define DP83822_ANNPTR                             0x07
#define DP83822_ANLNPTR                            0x08
#define DP83822_CR1                                0x09
#define DP83822_CR2                                0x0A
#define DP83822_CR3                                0x0B
#define DP83822_REGCR                              0x0D
#define DP83822_ADDAR                              0x0E
#define DP83822_FLDS                               0x0F
#define DP83822_PHYSTS                             0x10
#define DP83822_PHYSCR                             0x11
#define DP83822_MISR1                              0x12
#define DP83822_MISR2                              0x13
#define DP83822_FCSCR                              0x14
#define DP83822_RECR                               0x15
#define DP83822_BISCR                              0x16
#define DP83822_RCSR                               0x17
#define DP83822_LEDCR                              0x18
#define DP83822_PHYCR                              0x19
#define DP83822_10BTSCR                            0x1A
#define DP83822_BICSR1                             0x1B
#define DP83822_BICSR2                             0x1C
#define DP83822_CDCR                               0x1E
#define DP83822_PHYRCR                             0x1F

//DP83822 MMD registers
#define DP83822_MMD3_PCS_CTRL_1                    0x03, 0x3000
#define DP83822_MMD3_PCS_STATUS_1                  0x03, 0x3001
#define DP83822_MMD3_EEE_CAPABILITY                0x03, 0x3014
#define DP83822_MMD3_WAKE_ERR_CNT                  0x03, 0x3016
#define DP83822_MMD7_EEE_ADVERTISEMENT             0x07, 0x703C
#define DP83822_MMD7_EEE_LP_ABILITY                0x07, 0x703D
#define DP83822_MLEDCR                             0x1F, 0x0025
#define DP83822_COMPT                              0x1F, 0x0027
#define DP83822_PTPPSEL                            0x1F, 0x003E
#define DP83822_PTPCFG                             0x1F, 0x003F
#define DP83822_TXCPSR                             0x1F, 0x0042
#define DP83822_DFCR1                              0x1F, 0x0106
#define DP83822_DFCR2                              0x1F, 0x0107
#define DP83822_DSPCR                              0x1F, 0x010F
#define DP83822_DFECR                              0x1F, 0x0114
#define DP83822_AGCBCR                             0x1F, 0x0116
#define DP83822_DETR                               0x1F, 0x0126
#define DP83822_ALCDRR1                            0x1F, 0x0155
#define DP83822_CDSCR                              0x1F, 0x0170
#define DP83822_CDSCR2                             0x1F, 0x0171
#define DP83822_CDSCR3                             0x1F, 0x0173
#define DP83822_CDSCR4                             0x1F, 0x0177
#define DP83822_CDLRR1                             0x1F, 0x0180
#define DP83822_CDLRR2                             0x1F, 0x0181
#define DP83822_CDLRR3                             0x1F, 0x0182
#define DP83822_CDLRR4                             0x1F, 0x0183
#define DP83822_CDLRR5                             0x1F, 0x0184
#define DP83822_CDLAR1                             0x1F, 0x0185
#define DP83822_CDLAR2                             0x1F, 0x0186
#define DP83822_CDLAR3                             0x1F, 0x0187
#define DP83822_CDLAR4                             0x1F, 0x0188
#define DP83822_CDLAR5                             0x1F, 0x0189
#define DP83822_CDLGR                              0x1F, 0x018A
#define DP83822_ALCDRR2                            0x1F, 0x0215
#define DP83822_ALCDRR3                            0x1F, 0x021D
#define DP83822_LDCTRL                             0x1F, 0x0403
#define DP83822_LDCSEL                             0x1F, 0x0404
#define DP83822_DPDWN                              0x1F, 0x0428
#define DP83822_GENCFG                             0x1F, 0x0456
#define DP83822_LEDCFG1                            0x1F, 0x0460
#define DP83822_IOCTRL                             0x1F, 0x0461
#define DP83822_IOCTRL1                            0x1F, 0x0462
#define DP83822_IOCTRL2                            0x1F, 0x0463
#define DP83822_FIBER_GENCFG                       0x1F, 0x0465
#define DP83822_SOR1                               0x1F, 0x0467
#define DP83822_SOR2                               0x1F, 0x0468
#define DP83822_LEDCFG2                            0x1F, 0x0469
#define DP83822_RXFCFG                             0x1F, 0x04A0
#define DP83822_RXFS                               0x1F, 0x04A1
#define DP83822_RXFPMD1                            0x1F, 0x04A2
#define DP83822_RXFPMD2                            0x1F, 0x04A3
#define DP83822_RXFPMD3                            0x1F, 0x04A4
#define DP83822_RXFSOP1                            0x1F, 0x04A5
#define DP83822_RXFSOP2                            0x1F, 0x04A6
#define DP83822_RXFSOP3                            0x1F, 0x04A7
#define DP83822_RXFPAT1                            0x1F, 0x04A8
#define DP83822_RXFPAT2                            0x1F, 0x04A9
#define DP83822_RXFPAT3                            0x1F, 0x04AA
#define DP83822_RXFPAT4                            0x1F, 0x04AB
#define DP83822_RXFPAT5                            0x1F, 0x04AC
#define DP83822_RXFPAT6                            0x1F, 0x04AD
#define DP83822_RXFPAT7                            0x1F, 0x04AE
#define DP83822_RXFPAT8                            0x1F, 0x04AF
#define DP83822_RXFPAT9                            0x1F, 0x04B0
#define DP83822_RXFPAT10                           0x1F, 0x04B1
#define DP83822_RXFPAT11                           0x1F, 0x04B2
#define DP83822_RXFPAT12                           0x1F, 0x04B3
#define DP83822_RXFPAT13                           0x1F, 0x04B4
#define DP83822_RXFPAT14                           0x1F, 0x04B5
#define DP83822_RXFPAT15                           0x1F, 0x04B6
#define DP83822_RXFPAT16                           0x1F, 0x04B7
#define DP83822_RXFPAT17                           0x1F, 0x04B8
#define DP83822_RXFPAT18                           0x1F, 0x04B9
#define DP83822_RXFPAT19                           0x1F, 0x04BA
#define DP83822_RXFPAT20                           0x1F, 0x04BB
#define DP83822_RXFPAT21                           0x1F, 0x04BC
#define DP83822_RXFPAT22                           0x1F, 0x04BD
#define DP83822_RXFPAT23                           0x1F, 0x04BE
#define DP83822_RXFPAT24                           0x1F, 0x04BF
#define DP83822_RXFPAT25                           0x1F, 0x04C0
#define DP83822_RXFPAT26                           0x1F, 0x04C1
#define DP83822_RXFPAT27                           0x1F, 0x04C2
#define DP83822_RXFPAT28                           0x1F, 0x04C3
#define DP83822_RXFPAT29                           0x1F, 0x04C4
#define DP83822_RXFPAT30                           0x1F, 0x04C5
#define DP83822_RXFPAT31                           0x1F, 0x04C6
#define DP83822_RXFPAT32                           0x1F, 0x04C7
#define DP83822_RXFPBM1                            0x1F, 0x04C8
#define DP83822_RXFPBM2                            0x1F, 0x04C9
#define DP83822_RXFPBM3                            0x1F, 0x04CA
#define DP83822_RXFPBM4                            0x1F, 0x04CB
#define DP83822_RXFPATC                            0x1F, 0x04CC
#define DP83822_EEECFG2                            0x1F, 0x04D0
#define DP83822_EEECFG3                            0x1F, 0x04D1
#define DP83822_TLBCR1                             0x1F, 0x04D4
#define DP83822_TLBCR2                             0x1F, 0x04D5
#define DP83822_TLBCR3                             0x1F, 0x04D6

//Basic Mode Control register
#define DP83822_BMCR_RESET                         0x8000
#define DP83822_BMCR_LOOPBACK                      0x4000
#define DP83822_BMCR_SPEED_SEL                     0x2000
#define DP83822_BMCR_AN_EN                         0x1000
#define DP83822_BMCR_POWER_DOWN                    0x0800
#define DP83822_BMCR_ISOLATE                       0x0400
#define DP83822_BMCR_RESTART_AN                    0x0200
#define DP83822_BMCR_DUPLEX_MODE                   0x0100
#define DP83822_BMCR_COL_TEST                      0x0080

//Basic Mode Status register
#define DP83822_BMSR_100BT4                        0x8000
#define DP83822_BMSR_100BTX_FD                     0x4000
#define DP83822_BMSR_100BTX_HD                     0x2000
#define DP83822_BMSR_10BT_FD                       0x1000
#define DP83822_BMSR_10BT_HD                       0x0800
#define DP83822_BMSR_SMI_PREAMBLE_SUPPR            0x0040
#define DP83822_BMSR_AN_COMPLETE                   0x0020
#define DP83822_BMSR_REMOTE_FAULT                  0x0010
#define DP83822_BMSR_AN_CAPABLE                    0x0008
#define DP83822_BMSR_LINK_STATUS                   0x0004
#define DP83822_BMSR_JABBER_DETECT                 0x0002
#define DP83822_BMSR_EXTENDED_CAPABLE              0x0001

//PHY Identifier 1 register
#define DP83822_PHYIDR1_OUI_MSB                    0xFFFF
#define DP83822_PHYIDR1_OUI_MSB_DEFAULT            0x2000

//PHY Identifier 2 register
#define DP83822_PHYIDR2_OUI_LSB                    0xFC00
#define DP83822_PHYIDR2_OUI_LSB_DEFAULT            0xA000
#define DP83822_PHYIDR2_MODEL_NUMBER               0x03F0
#define DP83822_PHYIDR2_MODEL_NUMBER_DEFAULT       0x0240
#define DP83822_PHYIDR2_REV_NUMBER                 0x000F

//Auto-Negotiation Advertisement register
#define DP83822_ANAR_NEXT_PAGE                     0x8000
#define DP83822_ANAR_REMOTE_FAULT                  0x2000
#define DP83822_ANAR_ASYM_DIR                      0x0800
#define DP83822_ANAR_PAUSE                         0x0400
#define DP83822_ANAR_100BT4                        0x0200
#define DP83822_ANAR_100BTX_FD                     0x0100
#define DP83822_ANAR_100BTX_HD                     0x0080
#define DP83822_ANAR_10BT_FD                       0x0040
#define DP83822_ANAR_10BT_HD                       0x0020
#define DP83822_ANAR_SELECTOR                      0x001F
#define DP83822_ANAR_SELECTOR_DEFAULT              0x0001

//Auto-Negotiation Link Partner Ability register
#define DP83822_ANLPAR_NEXT_PAGE                   0x8000
#define DP83822_ANLPAR_ACK                         0x4000
#define DP83822_ANLPAR_REMOTE_FAULT                0x2000
#define DP83822_ANLPAR_ASYM_DIR                    0x0800
#define DP83822_ANLPAR_PAUSE                       0x0400
#define DP83822_ANLPAR_100BT4                      0x0200
#define DP83822_ANLPAR_100BTX_FD                   0x0100
#define DP83822_ANLPAR_100BTX_HD                   0x0080
#define DP83822_ANLPAR_10BT_FD                     0x0040
#define DP83822_ANLPAR_10BT_HD                     0x0020
#define DP83822_ANLPAR_SELECTOR                    0x001F
#define DP83822_ANLPAR_SELECTOR_DEFAULT            0x0001

//Auto-Negotiation Expansion register
#define DP83822_ANER_PAR_DETECT_FAULT              0x0010
#define DP83822_ANER_LP_NEXT_PAGE_ABLE             0x0008
#define DP83822_ANER_NEXT_PAGE_ABLE                0x0004
#define DP83822_ANER_PAGE_RECEIVED                 0x0002
#define DP83822_ANER_LP_AN_ABLE                    0x0001

//Auto-Negotiation Next Page TX register
#define DP83822_ANNPTR_NEXT_PAGE                   0x8000
#define DP83822_ANNPTR_MSG_PAGE                    0x2000
#define DP83822_ANNPTR_ACK2                        0x1000
#define DP83822_ANNPTR_TOGGLE                      0x0800
#define DP83822_ANNPTR_CODE                        0x07FF

//Auto-Negotiation Link Partner Ability Next Page register
#define DP83822_ANLNPTR_NEXT_PAGE                  0x8000
#define DP83822_ANLNPTR_ACK                        0x4000
#define DP83822_ANLNPTR_MSG_PAGE                   0x2000
#define DP83822_ANLNPTR_ACK2                       0x1000
#define DP83822_ANLNPTR_TOGGLE                     0x0800
#define DP83822_ANLNPTR_MESSAGE                    0x07FF

//Control 1 register
#define DP83822_CR1_RMII_ENHANCED_MODE             0x0200
#define DP83822_CR1_TDR_AUTO_RUN                   0x0100
#define DP83822_CR1_LINK_LOSS_RECOVERY             0x0080
#define DP83822_CR1_FAST_AUTO_MDIX                 0x0040
#define DP83822_CR1_ROBUST_AUTO_MDIX               0x0020
#define DP83822_CR1_FAST_AN_EN                     0x0010
#define DP83822_CR1_FAST_AN_SEL                    0x000C
#define DP83822_CR1_FAST_RX_DV_DETECT              0x0002

//Control 2 register
#define DP83822_CR2_FORCE_FAR_END_LINK_DROP        0x8000
#define DP83822_CR2_100BFX_EN                      0x4000
#define DP83822_CR2_FAST_LINK_UP_IN_PD             0x0040
#define DP83822_CR2_EXTENDED_FD_ABLE               0x0020
#define DP83822_CR2_ENHANCED_LED_LINK              0x0010
#define DP83822_CR2_ISOLATE_MII                    0x0008
#define DP83822_CR2_RX_ER_DURING_IDLE              0x0004
#define DP83822_CR2_ODD_NIBBLE_DETECT_DIS          0x0002
#define DP83822_CR2_RMII_RECEIVE_CLK               0x0001

//Control 3 register
#define DP83822_CR3_DESCRAMBLER_FAST_LINK_DOWN     0x0400
#define DP83822_CR3_POLARITY_SWAP                  0x0040
#define DP83822_CR3_MDIX_SWAP                      0x0020
#define DP83822_CR3_FAST_LINK_DOWN_MODE            0x000F

//Register Control register
#define DP83822_REGCR_CMD                          0xC000
#define DP83822_REGCR_CMD_ADDR                     0x0000
#define DP83822_REGCR_CMD_DATA_NO_POST_INC         0x4000
#define DP83822_REGCR_CMD_DATA_POST_INC_RW         0x8000
#define DP83822_REGCR_CMD_DATA_POST_INC_W          0xC000
#define DP83822_REGCR_DEVAD                        0x001F

//Fast Link Down Status register
#define DP83822_FLDS_FAST_LINK_DOWN_STATUS         0x01F0

//PHY Status register
#define DP83822_PHYSTS_MDIX_MODE                   0x4000
#define DP83822_PHYSTS_RECEIVE_ERROR_LATCH         0x2000
#define DP83822_PHYSTS_POLARITY_STATUS             0x1000
#define DP83822_PHYSTS_FALSE_CARRIER_SENSE_LATCH   0x0800
#define DP83822_PHYSTS_SIGNAL_DETECT               0x0400
#define DP83822_PHYSTS_DESCRAMBLER_LOCK            0x0200
#define DP83822_PHYSTS_PAGE_RECEIVED               0x0100
#define DP83822_PHYSTS_MII_INTERRUPT               0x0080
#define DP83822_PHYSTS_REMOTE_FAULT                0x0040
#define DP83822_PHYSTS_JABBER_DETECT               0x0020
#define DP83822_PHYSTS_AN_STATUS                   0x0010
#define DP83822_PHYSTS_LOOPBACK_STATUS             0x0008
#define DP83822_PHYSTS_DUPLEX_STATUS               0x0004
#define DP83822_PHYSTS_SPEED_STATUS                0x0002
#define DP83822_PHYSTS_LINK_STATUS                 0x0001

//PHY Specific Control register
#define DP83822_PHYSCR_PLL_DIS                     0x8000
#define DP83822_PHYSCR_POWER_SAVE_MODE_EN          0x4000
#define DP83822_PHYSCR_POWER_SAVE_MODE             0x3000
#define DP83822_PHYSCR_SCRAMBLER_BYPASS            0x0800
#define DP83822_PHYSCR_LOOPBACK_FIFO_DEPTH         0x0300
#define DP83822_PHYSCR_COL_FD_EN                   0x0010
#define DP83822_PHYSCR_INT_POLARITY                0x0008
#define DP83822_PHYSCR_TEST_INT                    0x0004
#define DP83822_PHYSCR_INT_EN                      0x0002
#define DP83822_PHYSCR_INT_OE                      0x0001

//MII Interrupt Status 1 register
#define DP83822_MISR1_LQ_INT                       0x8000
#define DP83822_MISR1_ED_INT                       0x4000
#define DP83822_MISR1_LINK_INT                     0x2000
#define DP83822_MISR1_SPD_INT                      0x1000
#define DP83822_MISR1_DUP_INT                      0x0800
#define DP83822_MISR1_ANC_INT                      0x0400
#define DP83822_MISR1_FHF_INT                      0x0200
#define DP83822_MISR1_RHF_INT                      0x0100
#define DP83822_MISR1_LQ_INT_EN                    0x0080
#define DP83822_MISR1_ED_INT_EN                    0x0040
#define DP83822_MISR1_LINK_INT_EN                  0x0020
#define DP83822_MISR1_SPD_INT_EN                   0x0010
#define DP83822_MISR1_DUP_INT_EN                   0x0008
#define DP83822_MISR1_ANC_INT_EN                   0x0004
#define DP83822_MISR1_FHF_INT_EN                   0x0002
#define DP83822_MISR1_RHF_INT_EN                   0x0001

//MII Interrupt Status 2 register
#define DP83822_MISR2_EEE_ERROR_INT                0x8000
#define DP83822_MISR2_AN_ERROR_INT                 0x4000
#define DP83822_MISR2_PR_INT                       0x2000
#define DP83822_MISR2_FIFO_OF_UF_INT               0x1000
#define DP83822_MISR2_MDI_CHANGE_INT               0x0800
#define DP83822_MISR2_SLEEP_MODE_INT               0x0400
#define DP83822_MISR2_POL_CHANGE_INT               0x0200
#define DP83822_MISR2_JABBER_DETECT_INT            0x0100
#define DP83822_MISR2_EEE_ERROR_INT_EN             0x0080
#define DP83822_MISR2_AN_ERROR_INT_EN              0x0040
#define DP83822_MISR2_PR_INT_EN                    0x0020
#define DP83822_MISR2_FIFO_OF_UF_INT_EN            0x0010
#define DP83822_MISR2_MDI_CHANGE_INT_EN            0x0008
#define DP83822_MISR2_SLEEP_MODE_INT_EN            0x0004
#define DP83822_MISR2_POL_CHANGE_INT_EN            0x0002
#define DP83822_MISR2_JABBER_DETECT_INT_EN         0x0001

//False Carrier Sense Counter register
#define DP83822_FCSCR_FCSCNT                       0x00FF

//Receive Error Counter register
#define DP83822_RECR_RXERCNT                       0xFFFF

//BIST Control register
#define DP83822_BISCR_ERROR_COUNTER_MODE           0x4000
#define DP83822_BISCR_PRBS_CHECKER                 0x2000
#define DP83822_BISCR_PACKET_GEN_EN                0x1000
#define DP83822_BISCR_PRBS_CHECKER_LOCK_SYNC       0x0800
#define DP83822_BISCR_PRBS_CHECKER_SYNC_LOSS       0x0400
#define DP83822_BISCR_PACKET_GEN_STATUS            0x0200
#define DP83822_BISCR_POWER_MODE                   0x0100
#define DP83822_BISCR_TX_MII_LOOPBACK              0x0040
#define DP83822_BISCR_LOOPBACK_MODE                0x001F
#define DP83822_BISCR_LOOPBACK_MODE_PCS_INPUT      0x0001
#define DP83822_BISCR_LOOPBACK_MODE_PCS_OUTPUT     0x0002
#define DP83822_BISCR_LOOPBACK_MODE_DIGITAL        0x0004
#define DP83822_BISCR_LOOPBACK_MODE_ANALOG         0x0008
#define DP83822_BISCR_LOOPBACK_MODE_REVERSE        0x0010

//RMII and Status register
#define DP83822_RCSR_RGMII_RX_CLK_SHIFT            0x1000
#define DP83822_RCSR_RGMII_TX_CLK_SHIFT            0x0800
#define DP83822_RCSR_RGMII_TX_SYNCED               0x0400
#define DP83822_RCSR_RGMII_MODE                    0x0200
#define DP83822_RCSR_RMII_TX_CLOCK_SHIFT           0x0100
#define DP83822_RCSR_RMII_CLK_SEL                  0x0080
#define DP83822_RCSR_RMII_ASYNC_FIFO_BYPASS        0x0040
#define DP83822_RCSR_RMII_MODE                     0x0020
#define DP83822_RCSR_RMII_REV_SEL                  0x0010
#define DP83822_RCSR_RMII_OVF_STATUS               0x0008
#define DP83822_RCSR_RMII_UNF_STATUS               0x0004
#define DP83822_RCSR_RX_ELAST_BUFFER_SIZE          0x0003
#define DP83822_RCSR_RX_ELAST_BUFFER_SIZE_14_BITS  0x0000
#define DP83822_RCSR_RX_ELAST_BUFFER_SIZE_2_BITS   0x0001
#define DP83822_RCSR_RX_ELAST_BUFFER_SIZE_6_BITS   0x0002
#define DP83822_RCSR_RX_ELAST_BUFFER_SIZE_10_BITS  0x0003

//LED Direct Control register
#define DP83822_LEDCR_BLINK_RATE                   0x0600
#define DP83822_LEDCR_BLINK_RATE_20MHZ             0x0000
#define DP83822_LEDCR_BLINK_RATE_10MHZ             0x0200
#define DP83822_LEDCR_BLINK_RATE_5MHZ              0x0400
#define DP83822_LEDCR_BLINK_RATE_2MHZ              0x0600
#define DP83822_LEDCR_LED_0_POLARITY               0x0080
#define DP83822_LEDCR_DRIVE_LED_0                  0x0010
#define DP83822_LEDCR_LED_0_ON_OFF                 0x0002

//PHY Control register
#define DP83822_PHYCR_MDIX_EN                      0x8000
#define DP83822_PHYCR_FORCE_MDIX                   0x4000
#define DP83822_PHYCR_PAUSE_RX_STATUS              0x2000
#define DP83822_PHYCR_PAUSE_TX_STATUS              0x1000
#define DP83822_PHYCR_MII_LINK_STATUS              0x0800
#define DP83822_PHYCR_BYPASS_LED_STRETCH           0x0080
#define DP83822_PHYCR_LED_CONFIG                   0x0020
#define DP83822_PHYCR_PHY_ADDR                     0x001F

//10Base-T Status/Control register
#define DP83822_10BTSCR_RX_THRESHOLD_EN            0x2000
#define DP83822_10BTSCR_SQUELCH                    0x1E00
#define DP83822_10BTSCR_SQUELCH_200MV              0x0000
#define DP83822_10BTSCR_SQUELCH_250MV              0x0200
#define DP83822_10BTSCR_SQUELCH_300MV              0x0400
#define DP83822_10BTSCR_SQUELCH_350MV              0x0600
#define DP83822_10BTSCR_SQUELCH_400MV              0x0800
#define DP83822_10BTSCR_SQUELCH_450MV              0x0A00
#define DP83822_10BTSCR_SQUELCH_500MV              0x0C00
#define DP83822_10BTSCR_SQUELCH_550MV              0x0E00
#define DP83822_10BTSCR_SQUELCH_600MV              0x1000
#define DP83822_10BTSCR_NLP_DIS                    0x0080
#define DP83822_10BTSCR_POLARITY_STATUS            0x0010
#define DP83822_10BTSCR_JABBER_DIS                 0x0001

//BIST Control and Status 1 register
#define DP83822_BICSR1_BIST_ERROR_COUNT            0xFF00
#define DP83822_BICSR1_BIST_IPG_LENGTH             0x00FF

//BIST Control and Status 2 register
#define DP83822_BICSR2_BIST_PACKET_LENGTH          0x07FF

//Cable Diagnostic Control register
#define DP83822_CDCR_CABLE_DIAG_START              0x8000
#define DP83822_CDCR_CDCR_CABLE_DIAG_STATUS        0x0002
#define DP83822_CDCR_CDCR_CABLE_DIAG_TEST_FAIL     0x0001

//PHY Reset Control register
#define DP83822_PHYRCR_SOFT_RESET                  0x8000
#define DP83822_PHYRCR_DIGITAL_RESTART             0x4000

//Multi-LED Control register
#define DP83822_MLEDCR_MLED_POLARITY_SWAP          0x0200
#define DP83822_MLEDCR_MLED_CONFIG                 0x0078
#define DP83822_MLEDCR_MLED_CONFIG_LINK            0x0000
#define DP83822_MLEDCR_MLED_CONFIG_ACT             0x0008
#define DP83822_MLEDCR_MLED_CONFIG_TX_ACT          0x0010
#define DP83822_MLEDCR_MLED_CONFIG_RX_ACT          0x0018
#define DP83822_MLEDCR_MLED_CONFIG_COL             0x0020
#define DP83822_MLEDCR_MLED_CONFIG_SPEED_100       0x0028
#define DP83822_MLEDCR_MLED_CONFIG_SPEED_10        0x0030
#define DP83822_MLEDCR_MLED_CONFIG_FD              0x0038
#define DP83822_MLEDCR_MLED_CONFIG_LINK_ACT        0x0040
#define DP83822_MLEDCR_MLED_CONFIG_ACT_STRETCH_SIG 0x0048
#define DP83822_MLEDCR_MLED_CONFIG_MII_LINK        0x0050
#define DP83822_MLEDCR_MLED_CONFIG_LPI_MODE        0x0058
#define DP83822_MLEDCR_MLED_CONFIG_MII_ERR         0x0060
#define DP83822_MLEDCR_MLED_CONFIG_LINK_LOST       0x0068
#define DP83822_MLEDCR_MLED_CONFIG_PRBS_ERR        0x0070
#define DP83822_MLEDCR_MLED_ROUTE                  0x0003
#define DP83822_MLEDCR_MLED_ROUTE_COL              0x0000
#define DP83822_MLEDCR_MLED_ROUTE_LED0             0x0003

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//DP83822 Ethernet PHY driver
extern const PhyDriver dp83822PhyDriver;

//DP83822 related functions
error_t dp83822Init(NetInterface *interface);
void dp83822InitHook(NetInterface *interface);

void dp83822Tick(NetInterface *interface);

void dp83822EnableIrq(NetInterface *interface);
void dp83822DisableIrq(NetInterface *interface);

void dp83822EventHandler(NetInterface *interface);

void dp83822WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t dp83822ReadPhyReg(NetInterface *interface, uint8_t address);

void dp83822DumpPhyReg(NetInterface *interface);

void dp83822WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t dp83822ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
