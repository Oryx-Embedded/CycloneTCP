/**
 * @file dp83tc812_driver.h
 * @brief DP83TC812 100Base-T1 Ethernet PHY driver
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

#ifndef _DP83TC812_DRIVER_H
#define _DP83TC812_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef DP83TC812_PHY_ADDR
   #define DP83TC812_PHY_ADDR 0
#elif (DP83TC812_PHY_ADDR < 0 || DP83TC812_PHY_ADDR > 31)
   #error DP83TC812_PHY_ADDR parameter is not valid
#endif

//DP83TC812 PHY registers
#define DP83TC812_BMCR                                            0x00
#define DP83TC812_BMSR                                            0x01
#define DP83TC812_PHYIDR1                                         0x02
#define DP83TC812_PHYIDR2                                         0x03
#define DP83TC812_REGCR                                           0x0D
#define DP83TC812_ADDAR                                           0x0E
#define DP83TC812_PHYSTS                                          0x10
#define DP83TC812_PHYSCR                                          0x11
#define DP83TC812_MISR1                                           0x12
#define DP83TC812_MISR2                                           0x13
#define DP83TC812_RECR                                            0x15
#define DP83TC812_BISCR                                           0x16
#define DP83TC812_MISR3                                           0x18
#define DP83TC812_REG_19                                          0x19
#define DP83TC812_TC10_ABORT_REG                                  0x1B
#define DP83TC812_CDCR                                            0x1E
#define DP83TC812_PHYRCR                                          0x1F

//DP83TC812 MMD registers
#define DP83TC812_MMD1_PMA_CTRL_1                                 0x01, 0x1000
#define DP83TC812_MMD1_PMA_STATUS_1                               0x01, 0x1001
#define DP83TC812_MMD1_PMA_STAUS_2                                0x01, 0x1007
#define DP83TC812_MMD1_PMA_EXT_ABILITY_1                          0x01, 0x100B
#define DP83TC812_MMD1_PMA_EXT_ABILITY_2                          0x01, 0x1012
#define DP83TC812_MMD1_PMA_CTRL_2                                 0x01, 0x1834
#define DP83TC812_MMD1_PMA_TEST_MODE_CTRL                         0x01, 0x1836
#define DP83TC812_MMD3_PCS_CTRL_1                                 0x03, 0x3000
#define DP83TC812_MMD3_PCS_STATUS_1                               0x03, 0x3001
#define DP83TC812_REG_41                                          0x1F, 0x0041
#define DP83TC812_REG_133                                         0x1F, 0x0133
#define DP83TC812_REG_17F                                         0x1F, 0x017F
#define DP83TC812_REG_180                                         0x1F, 0x0180
#define DP83TC812_REG_181                                         0x1F, 0x0181
#define DP83TC812_REG_182                                         0x1F, 0x0182
#define DP83TC812_LPS_CFG4                                        0x1F, 0x0183
#define DP83TC812_LPS_CFG                                         0x1F, 0x0184
#define DP83TC812_LPS_CFG5                                        0x1F, 0x0185
#define DP83TC812_LPS_CFG7                                        0x1F, 0x0187
#define DP83TC812_LPS_CFG8                                        0x1F, 0x0188
#define DP83TC812_LPS_CFG9                                        0x1F, 0x0189
#define DP83TC812_LPS_CFG10                                       0x1F, 0x018A
#define DP83TC812_LPS_CFG2                                        0x1F, 0x018B
#define DP83TC812_LPS_CFG3                                        0x1F, 0x018C
#define DP83TC812_LPS_STATUS                                      0x1F, 0x018E
#define DP83TC812_TDR_TX_CFG                                      0x1F, 0x0300
#define DP83TC812_TAP_PROCESS_CFG                                 0x1F, 0x0301
#define DP83TC812_TDR_CFG1                                        0x1F, 0x0302
#define DP83TC812_TDR_CFG2                                        0x1F, 0x0303
#define DP83TC812_TDR_CFG3                                        0x1F, 0x0304
#define DP83TC812_TDR_CFG4                                        0x1F, 0x0305
#define DP83TC812_TDR_CFG5                                        0x1F, 0x0306
#define DP83TC812_TDR_TC1                                         0x1F, 0x0310
#define DP83TC812_A2D_REG_48                                      0x1F, 0x0430
#define DP83TC812_A2D_REG_68                                      0x1F, 0x0444
#define DP83TC812_LEDS_CFG_1                                      0x1F, 0x0450
#define DP83TC812_LEDS_CFG_2                                      0x1F, 0x0451
#define DP83TC812_IO_MUX_CFG_1                                    0x1F, 0x0452
#define DP83TC812_IO_MUX_CFG_2                                    0x1F, 0x0453
#define DP83TC812_IO_MUX_CFG                                      0x1F, 0x0456
#define DP83TC812_IO_STATUS_1                                     0x1F, 0x0457
#define DP83TC812_IO_STATUS_2                                     0x1F, 0x0458
#define DP83TC812_CHIP_SOR_1                                      0x1F, 0x045D
#define DP83TC812_LED1_CLKOUT_ANA_CTRL                            0x1F, 0x045F
#define DP83TC812_PCS_CTRL_1                                      0x1F, 0x0485
#define DP83TC812_PCS_CTRL_2                                      0x1F, 0x0486
#define DP83TC812_TX_INTER_CFG                                    0x1F, 0x0489
#define DP83TC812_JABBER_CFG                                      0x1F, 0x0496
#define DP83TC812_TEST_MODE_CTRL                                  0x1F, 0x0497
#define DP83TC812_RXF_CFG                                         0x1F, 0x04A0
#define DP83TC812_PG_REG_4                                        0x1F, 0x0553
#define DP83TC812_TC1_CFG_RW                                      0x1F, 0x0560
#define DP83TC812_TC1_LINK_FAIL_LOSS                              0x1F, 0x0561
#define DP83TC812_TC1_LINK_TRAINING_TIME                          0x1F, 0x0562
#define DP83TC812_RGMII_CTRL                                      0x1F, 0x0600
#define DP83TC812_RGMII_FIFO_STATUS                               0x1F, 0x0601
#define DP83TC812_RGMII_CLK_SHIFT_CTRL                            0x1F, 0x0602
#define DP83TC812_RGMII_EEE_CTRL                                  0x1F, 0x0603
#define DP83TC812_SGMII_CTRL_1                                    0x1F, 0x0608
#define DP83TC812_SGMII_EEE_CTRL_1                                0x1F, 0x0609
#define DP83TC812_SGMII_STATUS                                    0x1F, 0x060A
#define DP83TC812_SGMII_EEE_CTRL_2                                0x1F, 0x060B
#define DP83TC812_SGMII_CTRL_2                                    0x1F, 0x060C
#define DP83TC812_SGMII_FIFO_STATUS                               0x1F, 0x060D
#define DP83TC812_PRBS_STATUS_1                                   0x1F, 0x0618
#define DP83TC812_PRBS_CTRL_1                                     0x1F, 0x0619
#define DP83TC812_PRBS_CTRL_2                                     0x1F, 0x061A
#define DP83TC812_PRBS_CTRL_3                                     0x1F, 0x061B
#define DP83TC812_PRBS_STATUS_2                                   0x1F, 0x061C
#define DP83TC812_PRBS_STATUS_3                                   0x1F, 0x061D
#define DP83TC812_PRBS_STATUS_4                                   0x1F, 0x061E
#define DP83TC812_PRBS_STATUS_5                                   0x1F, 0x0620
#define DP83TC812_PRBS_STATUS_6                                   0x1F, 0x0622
#define DP83TC812_PRBS_STATUS_7                                   0x1F, 0x0623
#define DP83TC812_PRBS_CTRL_4                                     0x1F, 0x0624
#define DP83TC812_PATTERN_CTRL_1                                  0x1F, 0x0625
#define DP83TC812_PATTERN_CTRL_2                                  0x1F, 0x0626
#define DP83TC812_PATTERN_CTRL_3                                  0x1F, 0x0627
#define DP83TC812_PMATCH_CTRL_1                                   0x1F, 0x0628
#define DP83TC812_PMATCH_CTRL_2                                   0x1F, 0x0629
#define DP83TC812_PMATCH_CTRL_3                                   0x1F, 0x062A
#define DP83TC812_TX_PKT_CNT_1                                    0x1F, 0x0639
#define DP83TC812_TX_PKT_CNT_2                                    0x1F, 0x063A
#define DP83TC812_TX_PKT_CNT_3                                    0x1F, 0x063B
#define DP83TC812_RX_PKT_CNT_1                                    0x1F, 0x063C
#define DP83TC812_RX_PKT_CNT_2                                    0x1F, 0x063D
#define DP83TC812_RX_PKT_CNT_3                                    0x1F, 0x063E
#define DP83TC812_RMII_CTRL_1                                     0x1F, 0x0648
#define DP83TC812_RMII_STATUS_1                                   0x1F, 0x0649
#define DP83TC812_RMII_OVERRIDE_CTRL                              0x1F, 0x064A
#define DP83TC812_DSP_REG_71                                      0x1F, 0x0871

//BMCR register
#define DP83TC812_BMCR_RESET                                      0x8000
#define DP83TC812_BMCR_LOOPBACK                                   0x4000
#define DP83TC812_BMCR_SPEED_SEL                                  0x2000
#define DP83TC812_BMCR_AN_EN                                      0x1000
#define DP83TC812_BMCR_POWER_DOWN                                 0x0800
#define DP83TC812_BMCR_ISOLATE                                    0x0400
#define DP83TC812_BMCR_DUPLEX_MODE                                0x0100

//BMSR register
#define DP83TC812_BMSR_100BT4                                     0x8000
#define DP83TC812_BMSR_100BX_FD                                   0x4000
#define DP83TC812_BMSR_100BX_HD                                   0x2000
#define DP83TC812_BMSR_10BT_FD                                    0x1000
#define DP83TC812_BMSR_10BT_HD                                    0x0800
#define DP83TC812_BMSR_MF_PREAMBLE_SUPPR                          0x0040
#define DP83TC812_BMSR_AN_COMPLETE                                0x0020
#define DP83TC812_BMSR_REMOTE_FAULT                               0x0010
#define DP83TC812_BMSR_AN_CAPABLE                                 0x0008
#define DP83TC812_BMSR_LINK_STATUS                                0x0004
#define DP83TC812_BMSR_JABBER_DETECT                              0x0002
#define DP83TC812_BMSR_EXTENDED_CAPABLE                           0x0001

//PHYIDR1 register
#define DP83TC812_PHYIDR1_OUI_MSB                                 0xFFFF
#define DP83TC812_PHYIDR1_OUI_MSB_DEFAULT                         0x2000

//PHYIDR2 register
#define DP83TC812_PHYIDR2_OUI_LSB                                 0xFC00
#define DP83TC812_PHYIDR2_OUI_LSB_DEFAULT                         0xA000
#define DP83TC812_PHYIDR2_MODEL_NUM                               0x03F0
#define DP83TC812_PHYIDR2_MODEL_NUM_DEFAULT                       0x0270
#define DP83TC812_PHYIDR2_REVISION_NUM                            0x000F
#define DP83TC812_PHYIDR2_REVISION_NUM_1_0                        0x0000
#define DP83TC812_PHYIDR2_REVISION_NUM_2_0                        0x0001

//REGCR register
#define DP83TC812_REGCR_CMD                                       0xC000
#define DP83TC812_REGCR_CMD_ADDR                                  0x0000
#define DP83TC812_REGCR_CMD_DATA_NO_POST_INC                      0x4000
#define DP83TC812_REGCR_CMD_DATA_POST_INC_RW                      0x8000
#define DP83TC812_REGCR_CMD_DATA_POST_INC_W                       0xC000
#define DP83TC812_REGCR_DEVAD                                     0x001F

//PHYSTS register
#define DP83TC812_PHYSTS_RECEIVE_ERROR_LATCH                      0x2000
#define DP83TC812_PHYSTS_SIGNAL_DETECT                            0x0400
#define DP83TC812_PHYSTS_DESCRAMBLER_LOCK                         0x0200
#define DP83TC812_PHYSTS_MII_INTERRUPT                            0x0080
#define DP83TC812_PHYSTS_JABBER_DTCT                              0x0020
#define DP83TC812_PHYSTS_LOOPBACK_STATUS                          0x0008
#define DP83TC812_PHYSTS_DUPLEX_STATUS                            0x0004
#define DP83TC812_PHYSTS_LINK_STATUS                              0x0001

//PHYSCR register
#define DP83TC812_PHYSCR_DIS_CLK_125                              0x8000
#define DP83TC812_PHYSCR_PWR_SAVE_MODE_EN                         0x4000
#define DP83TC812_PHYSCR_PWR_SAVE_MODE                            0x3000
#define DP83TC812_PHYSCR_SGMII_SOFT_RESET                         0x0800
#define DP83TC812_PHYSCR_USE_PHYAD0_AS_ISOLATE                    0x0400
#define DP83TC812_PHYSCR_TX_FIFO_DEPTH                            0x0300
#define DP83TC812_PHYSCR_TX_FIFO_DEPTH_4_NIBBLES                  0x0000
#define DP83TC812_PHYSCR_TX_FIFO_DEPTH_5_NIBBLES                  0x0100
#define DP83TC812_PHYSCR_TX_FIFO_DEPTH_6_NIBBLES                  0x0A00
#define DP83TC812_PHYSCR_TX_FIFO_DEPTH_8_NIBBLES                  0x0B00
#define DP83TC812_PHYSCR_INT_POL                                  0x0008
#define DP83TC812_PHYSCR_FORCE_INT                                0x0004
#define DP83TC812_PHYSCR_INT_EN                                   0x0002
#define DP83TC812_PHYSCR_INT_OE                                   0x0001

//MISR1 register
#define DP83TC812_MISR1_ENERGY_DET_INT                            0x4000
#define DP83TC812_MISR1_LINK_INT                                  0x2000
#define DP83TC812_MISR1_WOL_INT                                   0x1000
#define DP83TC812_MISR1_ESD_INT                                   0x0800
#define DP83TC812_MISR1_MS_TRAIN_DONE_INT                         0x0400
#define DP83TC812_MISR1_FHF_INT                                   0x0200
#define DP83TC812_MISR1_RHF_INT                                   0x0100
#define DP83TC812_MISR1_LINK_QUAL_INT_EN                          0x0080
#define DP83TC812_MISR1_ENERGY_DET_INT_EN                         0x0040
#define DP83TC812_MISR1_LINK_INT_EN                               0x0020
#define DP83TC812_MISR1_WOL_INT_EN                                0x0010
#define DP83TC812_MISR1_ESD_INT_EN                                0x0008
#define DP83TC812_MISR1_MS_TRAIN_DONE_INT_EN                      0x0004
#define DP83TC812_MISR1_FHF_INT_EN                                0x0002
#define DP83TC812_MISR1_RHF_INT_EN                                0x0001

//MISR2 register
#define DP83TC812_MISR2_UNDER_VOLT_INT                            0x8000
#define DP83TC812_MISR2_OVER_VOLT_INT                             0x4000
#define DP83TC812_MISR2_SLEEP_INT                                 0x0400
#define DP83TC812_MISR2_POL_INT                                   0x0200
#define DP83TC812_MISR2_JABBER_INT                                0x0100
#define DP83TC812_MISR2_UNDER_VOLT_INT_EN                         0x0080
#define DP83TC812_MISR2_OVER_VOLT_INT_EN                          0x0040
#define DP83TC812_MISR2_PAGE_RCVD_INT_EN                          0x0020
#define DP83TC812_MISR2_FIFO_INT_EN                               0x0010
#define DP83TC812_MISR2_SLEEP_INT_EN                              0x0004
#define DP83TC812_MISR2_POL_INT_EN                                0x0002
#define DP83TC812_MISR2_JABBER_INT_EN                             0x0001

//RECR register
#define DP83TC812_RECR_RX_ERR_CNT                                 0xFFFF

//BISCR register
#define DP83TC812_BISCR_PRBS_SYNC_LOSS                            0x0400
#define DP83TC812_BISCR_CORE_PWR_MODE                             0x0100
#define DP83TC812_BISCR_TX_MII_LPBK                               0x0040
#define DP83TC812_BISCR_LOOPBACK_MODE                             0x003C
#define DP83TC812_BISCR_LOOPBACK_MODE_DIGITAL                     0x0004
#define DP83TC812_BISCR_LOOPBACK_MODE_ANALOG                      0x0008
#define DP83TC812_BISCR_LOOPBACK_MODE_REVERSE                     0x0010
#define DP83TC812_BISCR_LOOPBACK_MODE_EXTERNAL                    0x0020
#define DP83TC812_BISCR_PCS_LPBCK                                 0x0002

//MISR3 register
#define DP83TC812_MISR3_WUP_PSV_INT                               0x8000
#define DP83TC812_MISR3_NO_LINK_INT                               0x4000
#define DP83TC812_MISR3_SLEEP_FAIL_INT                            0x2000
#define DP83TC812_MISR3_POR_DONE_INT                              0x1000
#define DP83TC812_MISR3_NO_FRAME_INT                              0x0800
#define DP83TC812_MISR3_WAKE_REQ_INT                              0x0400
#define DP83TC812_MISR3_WUP_SLEEP_INT                             0x0200
#define DP83TC812_MISR3_LPS_INT                                   0x0100
#define DP83TC812_MISR3_WUP_PSV_INT_EN                            0x0080
#define DP83TC812_MISR3_NO_LINK_INT_EN                            0x0040
#define DP83TC812_MISR3_SLEEP_FAIL_INT_EN                         0x0020
#define DP83TC812_MISR3_POR_DONE_INT_EN                           0x0010
#define DP83TC812_MISR3_NO_FRAME_INT_EN                           0x0008
#define DP83TC812_MISR3_WAKE_REQ_INT_EN                           0x0004
#define DP83TC812_MISR3_WUP_SLEEP_INT_EN                          0x0002
#define DP83TC812_MISR3_LPS_INT_EN                                0x0001

//REG_19 register
#define DP83TC812_REG_19_DSP_ENERGY_DETECT                        0x0400
#define DP83TC812_REG_19_PHY_ADDR                                 0x001F

//TC10_ABORT_REG register
#define DP83TC812_TC10_ABORT_REG_CFG_TC10_ABORT_GPIO_EN           0x0002
#define DP83TC812_TC10_ABORT_REG_CFG_SLEEP_ABORT                  0x0001

//CDCR register
#define DP83TC812_CDCR_TDR_START                                  0x8000
#define DP83TC812_CDCR_CFG_TDR_AUTO_RUN                           0x4000
#define DP83TC812_CDCR_TDR_DONE                                   0x0002
#define DP83TC812_CDCR_TDR_FAIL                                   0x0001

//PHYRCR register
#define DP83TC812_PHYRCR_SOFT_GLOBAL_RESET                        0x8000
#define DP83TC812_PHYRCR_DIGITAL_RESET                            0x4000
#define DP83TC812_PHYRCR_STANDBY_MODE                             0x0080

//MMD1_PMA_CTRL_1 register
#define DP83TC812_MMD1_PMA_CTRL_1_PMA_RESET                       0x8000
#define DP83TC812_MMD1_PMA_CTRL_1_PMA_LOOPBACK                    0x0001

//MMD1_PMA_STATUS_1 register
#define DP83TC812_MMD1_PMA_STATUS_1_LINK_STATUS                   0x0004

//MMD1_PMA_STAUS_2 register
#define DP83TC812_MMD1_PMA_STAUS_2_PMA_PMD_TYPE_SEL               0x003F

//MMD1_PMA_EXT_ABILITY_1 register
#define DP83TC812_MMD1_PMA_EXT_ABILITY_1_BT1_EXT_ABLE             0x0800

//MMD1_PMA_EXT_ABILITY_2 register
#define DP83TC812_MMD1_PMA_EXT_ABILITY_2_100BT1_ABLE              0x0001

//MMD1_PMA_CTRL_2 register
#define DP83TC812_MMD1_PMA_CTRL_2_MASTER_SLAVE_MAN_CFG_EN         0x8000
#define DP83TC812_MMD1_PMA_CTRL_2_BRK_MS_CFG                      0x4000
#define DP83TC812_MMD1_PMA_CTRL_2_TYPE_SEL                        0x000F

//MMD1_PMA_TEST_MODE_CTRL register
#define DP83TC812_MMD1_PMA_TEST_MODE_CTRL_BRK_TEST_MODE           0xE000

//MMD3_PCS_CTRL_1 register
#define DP83TC812_MMD3_PCS_CTRL_1_PCS_RESET                       0x8000
#define DP83TC812_MMD3_PCS_CTRL_1_PCS_LOOPBACK                    0x4000
#define DP83TC812_MMD3_PCS_CTRL_1_RX_CLOCK_STOPPABLE              0x0400

//MMD3_PCS_STATUS_1 register
#define DP83TC812_MMD3_PCS_STATUS_1_TX_LPI_RECEIVED               0x0800
#define DP83TC812_MMD3_PCS_STATUS_1_RX_LPI_RECEIVED               0x0400
#define DP83TC812_MMD3_PCS_STATUS_1_TX_LPI_INDICATION             0x0200
#define DP83TC812_MMD3_PCS_STATUS_1_RX_LPI_INDICATION             0x0100
#define DP83TC812_MMD3_PCS_STATUS_1_TX_CLOCK_STOPPABLE            0x0040

//REG_41 register
#define DP83TC812_REG_41_CFG_ETHER_TYPE_PATTERN                   0xFFFF

//REG_133 register
#define DP83TC812_REG_133_LINK_UP_C_AND_S                         0x4000
#define DP83TC812_REG_133_LINK_STATUS_PC                          0x2000
#define DP83TC812_REG_133_LINK_STATUS                             0x1000
#define DP83TC812_REG_133_DESCR_SYNC                              0x0004
#define DP83TC812_REG_133_LOC_RCVR_STATUS                         0x0002
#define DP83TC812_REG_133_REM_RCVR_STATUS                         0x0001

//REG_17F register
#define DP83TC812_REG_17F_CFG_EN_WUR_VIA_WAKE                     0x8000
#define DP83TC812_REG_17F_CFG_EN_WUP_VIA_WAKE                     0x4000
#define DP83TC812_REG_17F_CFG_WAKE_PIN_LEN_FR_WUR_TH              0x00FF

//REG_180 register
#define DP83TC812_REG_180_CFG_SLEEP_REQ_TIMER_SEL                 0x0018
#define DP83TC812_REG_180_CFG_SLEEP_ACK_TIMER_SEL                 0x0003

//REG_181 register
#define DP83TC812_REG_181_RX_LPS_CNT                              0x03FF

//REG_182 register
#define DP83TC812_REG_182_TX_LPS_CNT                              0x03FF

//LPS_CFG4 register
#define DP83TC812_LPS_CFG4_CFG_SEND_WUP_DIS_TX                    0x8000
#define DP83TC812_LPS_CFG4_CFG_FORCE_LPS_SLEEP_EN                 0x4000
#define DP83TC812_LPS_CFG4_CFG_FORCE_LPS_SLEEP                    0x2000
#define DP83TC812_LPS_CFG4_CFG_FORCE_TX_LPS_EN                    0x1000
#define DP83TC812_LPS_CFG4_CFG_FORCE_TX_LPS                       0x0800
#define DP83TC812_LPS_CFG4_CFG_FORCE_LPS_LINK_CONTROL_EN          0x0400
#define DP83TC812_LPS_CFG4_CFG_FORCE_LPS_LINK_CONTROL             0x0200
#define DP83TC812_LPS_CFG4_CFG_FORCE_LPS_ST_EN                    0x0100
#define DP83TC812_LPS_CFG4_CFG_FORCE_LPS_ST                       0x007F

//LPS_CFG register
#define DP83TC812_LPS_CFG_CFG_RESET_WUR_CNT_RX_DATA               0x8000
#define DP83TC812_LPS_CFG_CFG_RESET_LPS_CNT_RX_DATA               0x1000
#define DP83TC812_LPS_CFG_CFG_RESET_WUR_CNT_TX_DATA               0x0200
#define DP83TC812_LPS_CFG_CFG_RESET_LPS_CNT_TX_DATA               0x0040
#define DP83TC812_LPS_CFG_CFG_WAKE_FWD_EN_WUP_PSV_LINK            0x0020
#define DP83TC812_LPS_CFG_CFG_WAKE_FWD_MAN_TRIG                   0x0010
#define DP83TC812_LPS_CFG_CFG_WAKE_FWD_DIG_TIMER                  0x000C
#define DP83TC812_LPS_CFG_CFG_WAKE_FWD_EN_WUR                     0x0002
#define DP83TC812_LPS_CFG_CFG_WAKE_FWD_EN_WUP                     0x0001

//LPS_CFG5 register
#define DP83TC812_LPS_CFG5_CFG_WUP_TIMER                          0xE000
#define DP83TC812_LPS_CFG5_CFG_RX_WUR_SYM_GAP                     0x000C
#define DP83TC812_LPS_CFG5_CFG_RX_LPS_SYM_GAP                     0x0003

//LPS_CFG7 register
#define DP83TC812_LPS_CFG7_CFG_TX_LPS_STOP_ON_DONE                0x8000
#define DP83TC812_LPS_CFG7_CFG_TX_LPS_SEL                         0x03FF

//LPS_CFG8 register
#define DP83TC812_LPS_CFG8_CFG_TX_WUR_SEL                         0x03FF

//LPS_CFG9 register
#define DP83TC812_LPS_CFG9_CFG_RX_LPS_SEL                         0x03FF

//LPS_CFG10 register
#define DP83TC812_LPS_CFG10_CFG_RX_WUR_SEL                        0x03FF

//LPS_CFG2 register
#define DP83TC812_LPS_CFG2_CFG_STOP_SLEEP_NEG_ON_NO_SEND_N        0x1000
#define DP83TC812_LPS_CFG2_CFG_STOP_SLEEP_NEG_ON_ACTIVITY         0x0800
#define DP83TC812_LPS_CFG2_CFG_AUTO_MODE_EN                       0x0040
#define DP83TC812_LPS_CFG2_CFG_LPS_MON_EN                         0x0020
#define DP83TC812_LPS_CFG2_CFG_LPS_SLEEP_EN                       0x0002

//LPS_CFG3 register
#define DP83TC812_LPS_CFG3_CFG_LPS_PWR_MODE                       0x01FF

//LPS_STATUS register
#define DP83TC812_LPS_STATUS_STATUS_LPS_ST                        0x007F
#define DP83TC812_LPS_STATUS_STATUS_LPS_ST_SLEEP                  0x0001
#define DP83TC812_LPS_STATUS_STATUS_LPS_ST_STANDBY                0x0002
#define DP83TC812_LPS_STATUS_STATUS_LPS_ST_NORMAL                 0x0004
#define DP83TC812_LPS_STATUS_STATUS_LPS_ST_SLEEP_ACK              0x0008
#define DP83TC812_LPS_STATUS_STATUS_LPS_ST_SLEEP_REQ              0x0010
#define DP83TC812_LPS_STATUS_STATUS_LPS_ST_SLEEP_FAIL             0x0020
#define DP83TC812_LPS_STATUS_STATUS_LPS_ST_SLEEP_SILENT           0x0040
#define DP83TC812_LPS_STATUS_STATUS_LPS_ST_PASSIVE_LINK           0x0041

//TDR_TX_CFG register
#define DP83TC812_TDR_TX_CFG_CFG_TDR_TX_DURATION                  0xFFFF

//TAP_PROCESS_CFG register
#define DP83TC812_TAP_PROCESS_CFG_CFG_END_TAP_INDEX               0x1F00
#define DP83TC812_TAP_PROCESS_CFG_CFG_START_TAP_INDEX             0x001F

//TDR_CFG1 register
#define DP83TC812_TDR_CFG1_CFG_FORWARD_SHADOW                     0x00F0
#define DP83TC812_TDR_CFG1_CFG_POST_SILENCE_TIME                  0x000C
#define DP83TC812_TDR_CFG1_CFG_PRE_SILENCE_TIME                   0x0003

//TDR_CFG2 register
#define DP83TC812_TDR_CFG2_CFG_TDR_FILT_LOC_OFFSET                0x1F00
#define DP83TC812_TDR_CFG2_CFG_TDR_FILT_INIT                      0x00FF

//TDR_CFG3 register
#define DP83TC812_TDR_CFG3_CFG_TDR_FILT_SLOPE                     0x00FF

//TDR_CFG4 register
#define DP83TC812_TDR_CFG4_HPF_GAIN_TDR                           0x0030
#define DP83TC812_TDR_CFG4_PGA_GAIN_TDR                           0x000F

//TDR_CFG5 register
#define DP83TC812_TDR_CFG5_CFG_HALF_OPEN_DET_EN                   0x0010
#define DP83TC812_TDR_CFG5_CFG_CABLE_DELAY_NUM                    0x000F

//TDR_TC1 register
#define DP83TC812_TDR_TC1_HALF_OPEN_DETECT                        0x0100
#define DP83TC812_TDR_TC1_PEAK_DETECT                             0x0080
#define DP83TC812_TDR_TC1_PEAK_SIGN                               0x0040
#define DP83TC812_TDR_TC1_PEAK_LOC_IN_METERS                      0x003F

//A2D_REG_48 register
#define DP83TC812_A2D_REG_48_DLL_TX_DELAY_CTRL_RGMII_SL           0x0F00
#define DP83TC812_A2D_REG_48_DLL_RX_DELAY_CTRL_RGMII_SL           0x00F0

//A2D_REG_68 register
#define DP83TC812_A2D_REG_68_GOTO_SLEEP_FORCE_VAL                 0x0008
#define DP83TC812_A2D_REG_68_GOTO_SLEEP_FORCE_CONTROL             0x0004
#define DP83TC812_A2D_REG_68_WAKE_FWD_FORCE_VAL                   0x0002
#define DP83TC812_A2D_REG_68_WAKE_FWD_FORCE_CONTROL               0x0001

//LEDS_CFG_1 register
#define DP83TC812_LEDS_CFG_1_LEDS_BYPASS_STRETCHING               0x4000
#define DP83TC812_LEDS_CFG_1_LEDS_BLINK_RATE                      0x3000
#define DP83TC812_LEDS_CFG_1_LEDS_BLINK_RATE_20HZ                 0x0000
#define DP83TC812_LEDS_CFG_1_LEDS_BLINK_RATE_10HZ                 0x1000
#define DP83TC812_LEDS_CFG_1_LEDS_BLINK_RATE_5HZ                  0x2000
#define DP83TC812_LEDS_CFG_1_LEDS_BLINK_RATE_2HZ                  0x3000
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION                         0x0F00
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_LINK_OK                 0x0000
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_LINK_OK_TX_RX_ACT       0x0100
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_LINK_OK_TX_ACT          0x0200
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_LINK_OK_RX_ACT          0x0300
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_LINK_OK_MASTER          0x0400
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_LINK_OK_SLAVE           0x0500
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_TX_RX_ACT               0x0600
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_LINK_LOST               0x0900
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_PRBS_ERR                0x0A00
#define DP83TC812_LEDS_CFG_1_LED_2_OPTION_XMII_TX_RX_ERR          0x0B00
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION                         0x00F0
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_LINK_OK                 0x0000
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_LINK_OK_TX_RX_ACT       0x0010
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_LINK_OK_TX_ACT          0x0020
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_LINK_OK_RX_ACT          0x0030
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_LINK_OK_MASTER          0x0040
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_LINK_OK_SLAVE           0x0050
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_TX_RX_ACT               0x0060
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_LINK_LOST               0x0090
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_PRBS_ERR                0x00A0
#define DP83TC812_LEDS_CFG_1_LED_1_OPTION_XMII_TX_RX_ERR          0x00B0
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION                         0x000F
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_LINK_OK                 0x0000
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_LINK_OK_TX_RX_ACT       0x0001
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_LINK_OK_TX_ACT          0x0002
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_LINK_OK_RX_ACT          0x0003
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_LINK_OK_MASTER          0x0004
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_LINK_OK_SLAVE           0x0005
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_TX_RX_ACT               0x0006
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_LINK_LOST               0x0009
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_PRBS_ERR                0x000A
#define DP83TC812_LEDS_CFG_1_LED_0_OPTION_XMII_TX_RX_ERR          0x000B

//LEDS_CFG_2 register
#define DP83TC812_LEDS_CFG_2_CLK_O_GPIO_CTRL_3                    0x8000
#define DP83TC812_LEDS_CFG_2_LED_1_GPIO_CTRL_3                    0x4000
#define DP83TC812_LEDS_CFG_2_LED_0_GPIO_CTRL_3                    0x2000
#define DP83TC812_LEDS_CFG_2_LED_2_DRV_EN                         0x0100
#define DP83TC812_LEDS_CFG_2_LED_2_DRV_VAL                        0x0080
#define DP83TC812_LEDS_CFG_2_LED_2_POLARITY                       0x0040
#define DP83TC812_LEDS_CFG_2_LED_1_DRV_EN                         0x0020
#define DP83TC812_LEDS_CFG_2_LED_1_DRV_VAL                        0x0010
#define DP83TC812_LEDS_CFG_2_LED_1_POLARITY                       0x0008
#define DP83TC812_LEDS_CFG_2_LED_0_DRV_EN                         0x0004
#define DP83TC812_LEDS_CFG_2_LED_0_DRV_VAL                        0x0002
#define DP83TC812_LEDS_CFG_2_LED_0_POLARITY                       0x0001

//IO_MUX_CFG_1 register
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_DIV_2_EN                 0x8000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_SOURCE                   0x7000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_SOURCE_XI_CLK            0x0000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_SOURCE_200M_PLL_CLK      0x1000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_SOURCE_67M_ADC_CLK       0x2000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_SOURCE_FREE_200M_CLK     0x3000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_SOURCE_25M_MII_CLK       0x4000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_SOURCE_25M_CLK_TO_PLL    0x5000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_SOURCE_CORE_100M_CLK     0x6000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_SOURCE_67M_DSP_CLK       0x7000
#define DP83TC812_IO_MUX_CFG_1_LED_1_CLK_INV_EN                   0x0800
#define DP83TC812_IO_MUX_CFG_1_LED_1_GPIO_CTRL                    0x0700
#define DP83TC812_IO_MUX_CFG_1_LED_1_GPIO_CTRL_LED_1              0x0000
#define DP83TC812_IO_MUX_CFG_1_LED_1_GPIO_CTRL_LED_1_CLK_MUX_OUT  0x0100
#define DP83TC812_IO_MUX_CFG_1_LED_1_GPIO_CTRL_WOL                0x0200
#define DP83TC812_IO_MUX_CFG_1_LED_1_GPIO_CTRL_UV_INDICATION      0x0300
#define DP83TC812_IO_MUX_CFG_1_LED_1_GPIO_CTRL_1588_TX            0x0400
#define DP83TC812_IO_MUX_CFG_1_LED_1_GPIO_CTRL_1588_RX            0x0500
#define DP83TC812_IO_MUX_CFG_1_LED_1_GPIO_CTRL_ESD                0x0600
#define DP83TC812_IO_MUX_CFG_1_LED_1_GPIO_CTRL_INT                0x0700
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_DIV_2_EN                 0x0080
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_SOURCE                   0x0070
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_SOURCE_XI_CLK            0x0000
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_SOURCE_200M_PLL_CLK      0x0010
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_SOURCE_67M_ADC_CLK       0x0020
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_SOURCE_FREE_200M_CLK     0x0030
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_SOURCE_25M_MII_CLK       0x0040
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_SOURCE_25M_CLK_TO_PLL    0x0050
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_SOURCE_CORE_100M_CLK     0x0060
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_SOURCE_67M_DSP_CLK       0x0070
#define DP83TC812_IO_MUX_CFG_1_LED_0_CLK_INV_EN                   0x0008
#define DP83TC812_IO_MUX_CFG_1_LED_0_GPIO_CTRL                    0x0007
#define DP83TC812_IO_MUX_CFG_1_LED_0_GPIO_CTRL_LED_0              0x0000
#define DP83TC812_IO_MUX_CFG_1_LED_0_GPIO_CTRL_LED_0_CLK_MUX_OUT  0x0001
#define DP83TC812_IO_MUX_CFG_1_LED_0_GPIO_CTRL_WOL                0x0002
#define DP83TC812_IO_MUX_CFG_1_LED_0_GPIO_CTRL_UV_INDICATION      0x0003
#define DP83TC812_IO_MUX_CFG_1_LED_0_GPIO_CTRL_1588_TX            0x0004
#define DP83TC812_IO_MUX_CFG_1_LED_0_GPIO_CTRL_1588_RX            0x0005
#define DP83TC812_IO_MUX_CFG_1_LED_0_GPIO_CTRL_ESD                0x0006
#define DP83TC812_IO_MUX_CFG_1_LED_0_GPIO_CTRL_INT                0x0007

//IO_MUX_CFG_2 register
#define DP83TC812_IO_MUX_CFG_2_CFG_TX_ER_ON_LED1                  0x8000
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_DIV_2_EN                 0x0100
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE                   0x00F0
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_XI_CLK            0x0000
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_200M_PLL_CLK      0x0010
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_67M_ADC_CLK       0x0020
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_FREE_200M_CLK     0x00B0
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_25M_MII_CLK       0x0000
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_25M_CLK_TO_PLL    0x0650
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_CORE_100M_CLK     0x0020
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_67M_DSP_CLK       0x06F0
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_25M_50M_CLK       0x0000
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_50M_RMII_RX_CLK   0x3E90
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_SGMII_SER_CLK     0x0020
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_SGMII_DES_CLK     0x3F30
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_30NS_TICK         0x0000
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_40NS_TICK         0x44D0
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_DLL_TX_CLK        0x0020
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_DLL_RX_CLK        0x4570
#define DP83TC812_IO_MUX_CFG_2_CLK_O_CLK_INV_EN                   0x0008
#define DP83TC812_IO_MUX_CFG_2_CLK_O_GPIO_CTRL                    0x0007
#define DP83TC812_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_LED_1              0x0000
#define DP83TC812_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_CLKOUT_CLK_MUX_OUT 0x0001
#define DP83TC812_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_WOL                0x0002
#define DP83TC812_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_UV_INDICATION      0x0003
#define DP83TC812_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_1588_TX            0x0004
#define DP83TC812_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_1588_RX            0x0005
#define DP83TC812_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_ESD                0x0006
#define DP83TC812_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_INT                0x0007

//IO_MUX_CFG register
#define DP83TC812_IO_MUX_CFG_RX_PINS_PUPD_VALUE                   0xC000
#define DP83TC812_IO_MUX_CFG_RX_PINS_PUPD_VALUE_NO_PULL           0x0000
#define DP83TC812_IO_MUX_CFG_RX_PINS_PUPD_VALUE_PULL_UP           0x4000
#define DP83TC812_IO_MUX_CFG_RX_PINS_PUPD_VALUE_PULL_DOWN         0x8000
#define DP83TC812_IO_MUX_CFG_RX_PINS_PUPD_FORCE_CONTROL           0x2000
#define DP83TC812_IO_MUX_CFG_TX_PINS_PUPD_VALUE                   0x1800
#define DP83TC812_IO_MUX_CFG_TX_PINS_PUPD_VALUE_NO_PULL           0x0000
#define DP83TC812_IO_MUX_CFG_TX_PINS_PUPD_VALUE_PULL_UP           0x0800
#define DP83TC812_IO_MUX_CFG_TX_PINS_PUPD_VALUE_PULL_DOWN         0x1000
#define DP83TC812_IO_MUX_CFG_TX_PINS_PUPD_FORCE_CONTROL           0x0400
#define DP83TC812_IO_MUX_CFG_MAC_RX_IMPEDANCE_CTRL                0x03E0
#define DP83TC812_IO_MUX_CFG_MAC_TX_IMPEDANCE_CTRL                0x001F

//IO_STATUS_1 register
#define DP83TC812_IO_STATUS_1_RX_D0                               0x8000
#define DP83TC812_IO_STATUS_1_LED_1                               0x4000
#define DP83TC812_IO_STATUS_1_RX_ERR                              0x2000
#define DP83TC812_IO_STATUS_1_RX_DV                               0x0800
#define DP83TC812_IO_STATUS_1_RX_CLK                              0x0400
#define DP83TC812_IO_STATUS_1_LED_0                               0x0200
#define DP83TC812_IO_STATUS_1_CLKOUT                              0x0100
#define DP83TC812_IO_STATUS_1_INT_N                               0x0080
#define DP83TC812_IO_STATUS_1_TX_D3                               0x0040
#define DP83TC812_IO_STATUS_1_TX_D2                               0x0020
#define DP83TC812_IO_STATUS_1_TX_D1                               0x0010
#define DP83TC812_IO_STATUS_1_TX_D0                               0x0008
#define DP83TC812_IO_STATUS_1_TX_EN                               0x0004
#define DP83TC812_IO_STATUS_1_TX_CLK                              0x0002
#define DP83TC812_IO_STATUS_1_RX_D3                               0x0001

//IO_STATUS_2 register
#define DP83TC812_IO_STATUS_2_IO_STATUS_2                         0x0003

//CHIP_SOR_1 register
#define DP83TC812_CHIP_SOR_1_LED1_POR                             0x2000
#define DP83TC812_CHIP_SOR_1_RX_D3_POR                            0x1000
#define DP83TC812_CHIP_SOR_1_LED0_STRAP                           0x0200
#define DP83TC812_CHIP_SOR_1_RXD3_STRAP                           0x0100
#define DP83TC812_CHIP_SOR_1_RXD2_STRAP                           0x0080
#define DP83TC812_CHIP_SOR_1_RXD1_STRAP                           0x0040
#define DP83TC812_CHIP_SOR_1_RXD0_STRAP                           0x0020
#define DP83TC812_CHIP_SOR_1_RXCLK_STRAP                          0x0010
#define DP83TC812_CHIP_SOR_1_RXER_STRAP                           0x000C
#define DP83TC812_CHIP_SOR_1_RXDV_STRAP                           0x0003

//LED1_CLKOUT_ANA_CTRL register
#define DP83TC812_LED1_CLKOUT_ANA_CTRL_CLKOUT_ANA_SEL_1P0V_SL     0x0010
#define DP83TC812_LED1_CLKOUT_ANA_CTRL_LED_1_ANA_MUX_CTRL         0x000C
#define DP83TC812_LED1_CLKOUT_ANA_CTRL_CLKOUT_ANA_MUX_CTRL        0x0003

//PCS_CTRL_1 register
#define DP83TC812_PCS_CTRL_1_CFG_FORCE_SLAVE_PHASE1_DONE          0x4000
#define DP83TC812_PCS_CTRL_1_CFG_DIS_IPG_SCR_LOCK_CHECK           0x2000
#define DP83TC812_PCS_CTRL_1_CFG_LINK_CONTROL                     0x1000
#define DP83TC812_PCS_CTRL_1_CFG_DESC_FIRST_LOCK_COUNT            0x01FF

//PCS_CTRL_2 register
#define DP83TC812_PCS_CTRL_2_CFG_DESC_ERROR_COUNT                 0xFF00
#define DP83TC812_PCS_CTRL_2_CFG_REM_RCVR_STS_ERROR_CNT           0x001F

//TX_INTER_CFG register
#define DP83TC812_TX_INTER_CFG_CFG_FORCE_TX_INTERLEAVE            0x0004
#define DP83TC812_TX_INTER_CFG_CFG_TX_INTERLEAVE_EN               0x0002
#define DP83TC812_TX_INTER_CFG_CFG_INTERLEAVE_DET_EN              0x0001

//JABBER_CFG register
#define DP83TC812_JABBER_CFG_CFG_RCV_JAB_TIMER_VAL                0x07FF

//TEST_MODE_CTRL register
#define DP83TC812_TEST_MODE_CTRL_CFG_TEST_MODE1_SYMBOL_CNT        0x03F0

//RXF_CFG register
#define DP83TC812_RXF_CFG_BITS_NIBBLES_SWAP                       0xC000
#define DP83TC812_RXF_CFG_BITS_NIBBLES_SWAP_REGULAR               0x0000
#define DP83TC812_RXF_CFG_BITS_NIBBLES_SWAP_BITS                  0x4000
#define DP83TC812_RXF_CFG_BITS_NIBBLES_SWAP_NIBBLES               0x8000
#define DP83TC812_RXF_CFG_BITS_NIBBLES_SWAP_BITS_IN_NIBBLE        0xC000
#define DP83TC812_RXF_CFG_SFD_BYTE                                0x2000
#define DP83TC812_RXF_CFG_ENHANCED_MAC_SUPPORT                    0x0080

//PG_REG_4 register
#define DP83TC812_PG_REG_4_FORCE_POL_EN                           0x2000
#define DP83TC812_PG_REG_4_FORCE_POL_VAL                          0x1000

//TC1_CFG_RW register
#define DP83TC812_TC1_CFG_RW_CFG_LINK_STATUS_METRIC               0x1800
#define DP83TC812_TC1_CFG_RW_CFG_LINK_FAILURE_MULTIHOT            0x07E0
#define DP83TC812_TC1_CFG_RW_CFG_COMM_TIMER_THRS                  0x0018
#define DP83TC812_TC1_CFG_RW_CFG_BAD_SQI_THRS                     0x0007

//TC1_LINK_FAIL_LOSS register
#define DP83TC812_TC1_LINK_FAIL_LOSS_CFG_BAD_SQI_THRS             0xFC00
#define DP83TC812_TC1_LINK_FAIL_LOSS_LINK_FAILURES                0x03FF

//TC1_LINK_TRAINING_TIME register
#define DP83TC812_TC1_LINK_TRAINING_TIME_COMM_READY               0x8000
#define DP83TC812_TC1_LINK_TRAINING_TIME_LQ_LTT                   0x00FF

//RGMII_CTRL register
#define DP83TC812_RGMII_CTRL_RGMII_TX_HALF_FULL_TH                0x0070
#define DP83TC812_RGMII_CTRL_CFG_RGMII_EN                         0x0008
#define DP83TC812_RGMII_CTRL_INV_RGMII_TXD                        0x0004
#define DP83TC812_RGMII_CTRL_INV_RGMII_RXD                        0x0002
#define DP83TC812_RGMII_CTRL_SUP_TX_ERR_FD_RGMII                  0x0001

//RGMII_FIFO_STATUS register
#define DP83TC812_RGMII_FIFO_STATUS_RGMII_TX_AF_FULL_ERR          0x0002
#define DP83TC812_RGMII_FIFO_STATUS_RGMII_TX_AF_EMPTY_ERR         0x0001

//RGMII_CLK_SHIFT_CTRL register
#define DP83TC812_RGMII_CLK_SHIFT_CTRL_CFG_RGMII_RX_CLK_SHIFT_SEL 0x0002
#define DP83TC812_RGMII_CLK_SHIFT_CTRL_CFG_RGMII_TX_CLK_SHIFT_SEL 0x0001

//RGMII_EEE_CTRL register
#define DP83TC812_RGMII_EEE_CTRL_CFG_RGMII_WAKE_SIGNALING_EN      0x0003

//SGMII_CTRL_1 register
#define DP83TC812_SGMII_CTRL_1_SGMII_TX_ERR_DIS                   0x8000
#define DP83TC812_SGMII_CTRL_1_CFG_ALIGN_IDX_FORCE_EN             0x4000
#define DP83TC812_SGMII_CTRL_1_CFG_ALIGN_IDX_VALUE                0x3C00
#define DP83TC812_SGMII_CTRL_1_CFG_SGMII_EN                       0x0200
#define DP83TC812_SGMII_CTRL_1_CFG_SGMII_RX_POL_INVERT            0x0100
#define DP83TC812_SGMII_CTRL_1_CFG_SGMII_TX_POL_INVERT            0x0080
#define DP83TC812_SGMII_CTRL_1_SERDES_TX_BITS_ORDER               0x0060
#define DP83TC812_SGMII_CTRL_1_SERDES_RX_BITS_ORDER               0x0010
#define DP83TC812_SGMII_CTRL_1_CFG_SGMII_ALIGN_PKT_EN             0x0008
#define DP83TC812_SGMII_CTRL_1_SGMII_AUTONEG_TIMER                0x0006
#define DP83TC812_SGMII_CTRL_1_SGMII_AUTONEG_EN                   0x0001

//SGMII_EEE_CTRL_1 register
#define DP83TC812_SGMII_EEE_CTRL_1_CFG_SGMII_TX_TR_TIMER_VAL      0xF800
#define DP83TC812_SGMII_EEE_CTRL_1_CFG_SGMII_TX_TQ_TIMER_VAL      0x07C0
#define DP83TC812_SGMII_EEE_CTRL_1_CFG_SGMII_TX_TS_TIMER_VAL      0x003E
#define DP83TC812_SGMII_EEE_CTRL_1_CFG_NON_EEE_MAC_SGMII_EN       0x0001

//SGMII_STATUS register
#define DP83TC812_SGMII_STATUS_SGMII_PAGE_RECEIVED                0x1000
#define DP83TC812_SGMII_STATUS_LINK_STATUS_1000BX                 0x0800
#define DP83TC812_SGMII_STATUS_SGMII_AUTONEG_COMPLETE             0x0400
#define DP83TC812_SGMII_STATUS_CFG_ALIGN_EN                       0x0200
#define DP83TC812_SGMII_STATUS_CFG_SYNC_STATUS                    0x0100
#define DP83TC812_SGMII_STATUS_CFG_ALIGN_IDX                      0x00F0

//SGMII_EEE_CTRL_2 register
#define DP83TC812_SGMII_EEE_CTRL_2_CFG_SGMII_RX_QUIET_TIMER_VAL   0x000F

//SGMII_CTRL_2 register
#define DP83TC812_SGMII_CTRL_2_SGMII_CDR_LOCK_FORCE_VAL           0x0100
#define DP83TC812_SGMII_CTRL_2_SGMII_CDR_LOCK_FORCE_CTRL          0x0080
#define DP83TC812_SGMII_CTRL_2_SGMII_MR_RESTART_AN                0x0040
#define DP83TC812_SGMII_CTRL_2_TX_HALF_FULL_TH                    0x0038
#define DP83TC812_SGMII_CTRL_2_RX_HALF_FULL_TH                    0x0007

//SGMII_FIFO_STATUS register
#define DP83TC812_SGMII_FIFO_STATUS_SGMII_RX_AF_FULL_ERR          0x0008
#define DP83TC812_SGMII_FIFO_STATUS_SGMII_RX_AF_EMPTY_ERR         0x0004
#define DP83TC812_SGMII_FIFO_STATUS_SGMII_TX_AF_FULL_ERR          0x0002
#define DP83TC812_SGMII_FIFO_STATUS_SGMII_TX_AF_EMPTY_ERR         0x0001

//PRBS_STATUS_1 register
#define DP83TC812_PRBS_STATUS_1_PRBS_ERR_OV_CNT                   0x00FF

//PRBS_CTRL_1 register
#define DP83TC812_PRBS_CTRL_1_CFG_PKT_GEN_64                      0x2000
#define DP83TC812_PRBS_CTRL_1_SEND_PKT                            0x1000
#define DP83TC812_PRBS_CTRL_1_CFG_PRBS_CHK_SEL                    0x0700
#define DP83TC812_PRBS_CTRL_1_CFG_PRBS_GEN_SEL                    0x0070
#define DP83TC812_PRBS_CTRL_1_CFG_PRBS_CNT_MODE                   0x0008
#define DP83TC812_PRBS_CTRL_1_CFG_PRBS_CHK_ENABLE                 0x0004
#define DP83TC812_PRBS_CTRL_1_CFG_PKT_GEN_PRBS                    0x0002
#define DP83TC812_PRBS_CTRL_1_PKT_GEN_EN                          0x0001

//PRBS_CTRL_2 register
#define DP83TC812_PRBS_CTRL_2_CFG_PKT_LEN_PRBS                    0xFFFF

//PRBS_CTRL_3 register
#define DP83TC812_PRBS_CTRL_3_CFG_IPG_LEN                         0x00FF

//PRBS_STATUS_2 register
#define DP83TC812_PRBS_STATUS_2_PRBS_BYTE_CNT                     0xFFFF

//PRBS_STATUS_3 register
#define DP83TC812_PRBS_STATUS_3_PRBS_PKT_CNT_15_0                 0xFFFF

//PRBS_STATUS_4 register
#define DP83TC812_PRBS_STATUS_4_PRBS_PKT_CNT_31_16                0xFFFF

//PRBS_STATUS_5 register
#define DP83TC812_PRBS_STATUS_5_PRBS_PKT_CNT_31_16                0x1000
#define DP83TC812_PRBS_STATUS_5_PKT_GEN_BUSY                      0x0800
#define DP83TC812_PRBS_STATUS_5_PRBS_PKT_OV                       0x0400
#define DP83TC812_PRBS_STATUS_5_PRBS_BYTE_OV                      0x0200
#define DP83TC812_PRBS_STATUS_5_PRBS_LOCK                         0x0100
#define DP83TC812_PRBS_STATUS_5_PRBS_ERR_CNT                      0x00FF

//PRBS_STATUS_6 register
#define DP83TC812_PRBS_STATUS_6_PKT_ERR_CNT_15_0                  0xFFFF

//PRBS_STATUS_7 register
#define DP83TC812_PRBS_STATUS_7_PKT_ERR_CNT_31_16                 0xFFFF

//PRBS_CTRL_4 register
#define DP83TC812_PRBS_CTRL_4_CFG_PKT_DATA                        0xFF00
#define DP83TC812_PRBS_CTRL_4_CFG_PKT_MODE                        0x00C0
#define DP83TC812_PRBS_CTRL_4_CFG_PATTERN_VLD_BYTES               0x0038
#define DP83TC812_PRBS_CTRL_4_CFG_PKT_CNT                         0x0007

//PATTERN_CTRL_1 register
#define DP83TC812_PATTERN_CTRL_1_PATTERN_15_0                     0xFFFF

//PATTERN_CTRL_2 register
#define DP83TC812_PATTERN_CTRL_2_PATTERN_31_16                    0xFFFF

//PATTERN_CTRL_3 register
#define DP83TC812_PATTERN_CTRL_3_PATTERN_47_32                    0xFFFF

//PMATCH_CTRL_1 register
#define DP83TC812_PMATCH_CTRL_1_PMATCH_DATA_15_0                  0xFFFF

//PMATCH_CTRL_2 register
#define DP83TC812_PMATCH_CTRL_2_PMATCH_DATA_31_16                 0xFFFF

//PMATCH_CTRL_3 register
#define DP83TC812_PMATCH_CTRL_3_PMATCH_DATA_47_32                 0xFFFF

//TX_PKT_CNT_1 register
#define DP83TC812_TX_PKT_CNT_1_TX_PKT_CNT_15_0                    0xFFFF

//TX_PKT_CNT_2 register
#define DP83TC812_TX_PKT_CNT_2_TX_PKT_CNT_31_16                   0xFFFF

//TX_PKT_CNT_3 register
#define DP83TC812_TX_PKT_CNT_3_TX_ERR_PKT_CNT                     0xFFFF

//RX_PKT_CNT_1 register
#define DP83TC812_RX_PKT_CNT_1_RX_PKT_CNT_15_0                    0xFFFF

//RX_PKT_CNT_2 register
#define DP83TC812_RX_PKT_CNT_2_RX_PKT_CNT_31_16                   0xFFFF

//RX_PKT_CNT_3 register
#define DP83TC812_RX_PKT_CNT_3_RX_ERR_PKT_CNT                     0xFFFF

//RMII_CTRL_1 register
#define DP83TC812_RMII_CTRL_1_CFG_RMII_DIS_DELAYED_TXD_EN         0x0400
#define DP83TC812_RMII_CTRL_1_CFG_RMII_HALF_FULL_TH               0x0380
#define DP83TC812_RMII_CTRL_1_CFG_RMII_MODE                       0x0040
#define DP83TC812_RMII_CTRL_1_CFG_RMII_BYPASS_AFIFO_EN            0x0020
#define DP83TC812_RMII_CTRL_1_CFG_XI_50                           0x0010
#define DP83TC812_RMII_CTRL_1_CFG_RMII_REV1_0                     0x0002
#define DP83TC812_RMII_CTRL_1_CFG_RMII_ENH                        0x0001

//RMII_STATUS_1 register
#define DP83TC812_RMII_STATUS_1_RMII_AF_UNF_ERR                   0x0002
#define DP83TC812_RMII_STATUS_1_RMII_AF_OVF_ERR                   0x0001

//RMII_OVERRIDE_CTRL register
#define DP83TC812_RMII_OVERRIDE_CTRL_CFG_CLK50_TX_DLL             0x0400
#define DP83TC812_RMII_OVERRIDE_CTRL_CFG_CLK50_DLL                0x0200

//DSP_REG_71 register
#define DP83TC812_DSP_REG_71_WORST_SQI_OUT                        0x0080
#define DP83TC812_DSP_REG_71_SQI_OUT                              0x000E

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//DP83TC812 Ethernet PHY driver
extern const PhyDriver dp83tc812PhyDriver;

//DP83TC812 related functions
error_t dp83tc812Init(NetInterface *interface);
void dp83tc812InitHook(NetInterface *interface);

void dp83tc812Tick(NetInterface *interface);

void dp83tc812EnableIrq(NetInterface *interface);
void dp83tc812DisableIrq(NetInterface *interface);

void dp83tc812EventHandler(NetInterface *interface);

void dp83tc812WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t dp83tc812ReadPhyReg(NetInterface *interface, uint8_t address);

void dp83tc812DumpPhyReg(NetInterface *interface);

void dp83tc812WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t dp83tc812ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
