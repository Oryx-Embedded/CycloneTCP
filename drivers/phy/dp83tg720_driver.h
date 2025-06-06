/**
 * @file dp83tg720_driver.h
 * @brief DP83TG720 1000Base-T1 Ethernet PHY driver
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

#ifndef _DP83TG720_DRIVER_H
#define _DP83TG720_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef DP83TG720_PHY_ADDR
   #define DP83TG720_PHY_ADDR 0
#elif (DP83TG720_PHY_ADDR < 0 || DP83TG720_PHY_ADDR > 31)
   #error DP83TG720_PHY_ADDR parameter is not valid
#endif

//DP83TG720 PHY registers
#define DP83TG720_BMCR                                                 0x00
#define DP83TG720_BMSR                                                 0x01
#define DP83TG720_PHYID1                                               0x02
#define DP83TG720_PHYID2                                               0x03
#define DP83TG720_REGCR                                                0x0D
#define DP83TG720_ADDAR                                                0x0E
#define DP83TG720_MII_REG_10                                           0x10
#define DP83TG720_MII_REG_11                                           0x11
#define DP83TG720_MII_REG_12                                           0x12
#define DP83TG720_MII_REG_13                                           0x13
#define DP83TG720_MII_REG_16                                           0x16
#define DP83TG720_MII_REG_18                                           0x18
#define DP83TG720_MII_REG_19                                           0x19
#define DP83TG720_MII_REG_1E                                           0x1E
#define DP83TG720_MII_REG_1F                                           0x1F

//DP83TG720 MMD registers
#define DP83TG720_PMA_PMD_CONTROL_1                                    0x01, 0x1000
#define DP83TG720_PMA_PMD_CONTROL_2                                    0x01, 0x1007
#define DP83TG720_PMA_PMD_TRANSMIT_DISABLE                             0x01, 0x1009
#define DP83TG720_PMA_PMD_EXTENDED_ABILITY2                            0x01, 0x100B
#define DP83TG720_PMA_PMD_EXTENDED_ABILITY                             0x01, 0x1012
#define DP83TG720_PMA_PMD_CONTROL                                      0x01, 0x1834
#define DP83TG720_PMA_CONTROL                                          0x01, 0x1900
#define DP83TG720_PMA_STATUS                                           0x01, 0x1901
#define DP83TG720_TRAINING                                             0x01, 0x1902
#define DP83TG720_LP_TRAINING                                          0x01, 0x1903
#define DP83TG720_TEST_MODE_CONTROL                                    0x01, 0x1904
#define DP83TG720_PCS_CONTROL_COPY                                     0x03, 0x3000
#define DP83TG720_PCS_CONTROL                                          0x03, 0x3900
#define DP83TG720_PCS_STATUS                                           0x03, 0x3901
#define DP83TG720_PCS_STATUS_2                                         0x03, 0x3902
#define DP83TG720_OAM_TRANSMIT                                         0x03, 0x3904
#define DP83TG720_OAM_TX_MESSAGE_1                                     0x03, 0x3905
#define DP83TG720_OAM_TX_MESSAGE_2                                     0x03, 0x3906
#define DP83TG720_OAM_TX_MESSAGE_3                                     0x03, 0x3907
#define DP83TG720_OAM_TX_MESSAGE_4                                     0x03, 0x3908
#define DP83TG720_OAM_RECEIVE                                          0x03, 0x3909
#define DP83TG720_OAM_RX_MESSAGE_1                                     0x03, 0x390A
#define DP83TG720_OAM_RX_MESSAGE_2                                     0x03, 0x390B
#define DP83TG720_OAM_RX_MESSAGE_3                                     0x03, 0x390C
#define DP83TG720_OAM_RX_MESSAGE_4                                     0x03, 0x390D
#define DP83TG720_AN_CFG                                               0x07, 0x7200
#define DP83TG720_LSR                                                  0x1F, 0x0180
#define DP83TG720_LPS_CFG2                                             0x1F, 0x018B
#define DP83TG720_LPS_CFG3                                             0x1F, 0x018C
#define DP83TG720_TDR_STATUS0                                          0x1F, 0x0309
#define DP83TG720_TDR_STATUS1                                          0x1F, 0x030A
#define DP83TG720_TDR_STATUS2                                          0x1F, 0x030B
#define DP83TG720_TDR_STATUS5                                          0x1F, 0x030E
#define DP83TG720_TDR_TC12                                             0x1F, 0x030F
#define DP83TG720_A2D_REG_05                                           0x1F, 0x0405
#define DP83TG720_A2D_REG_30                                           0x1F, 0x041E
#define DP83TG720_A2D_REG_31                                           0x1F, 0x041F
#define DP83TG720_A2D_REG_40                                           0x1F, 0x0428
#define DP83TG720_A2D_REG_41                                           0x1F, 0x0429
#define DP83TG720_A2D_REG_43                                           0x1F, 0x042B
#define DP83TG720_A2D_REG_44                                           0x1F, 0x042C
#define DP83TG720_A2D_REG_46                                           0x1F, 0x042E
#define DP83TG720_A2D_REG_47                                           0x1F, 0x042F
#define DP83TG720_A2D_REG_48                                           0x1F, 0x0430
#define DP83TG720_A2D_REG_66                                           0x1F, 0x0442
#define DP83TG720_LEDS_CFG_1                                           0x1F, 0x0450
#define DP83TG720_LEDS_CFG_2                                           0x1F, 0x0451
#define DP83TG720_IO_MUX_CFG_1                                         0x1F, 0x0452
#define DP83TG720_IO_MUX_CFG_2                                         0x1F, 0x0453
#define DP83TG720_IO_CONTROL_1                                         0x1F, 0x0454
#define DP83TG720_IO_CONTROL_2                                         0x1F, 0x0455
#define DP83TG720_IO_CONTROL_3                                         0x1F, 0x0456
#define DP83TG720_IO_STATUS_1                                          0x1F, 0x0457
#define DP83TG720_IO_STATUS_2                                          0x1F, 0x0458
#define DP83TG720_IO_CONTROL_4                                         0x1F, 0x0459
#define DP83TG720_IO_CONTROL_5                                         0x1F, 0x045A
#define DP83TG720_SOR_VECTOR_1                                         0x1F, 0x045D
#define DP83TG720_SOR_VECTOR_2                                         0x1F, 0x045E
#define DP83TG720_MONITOR_CTRL1                                        0x1F, 0x0467
#define DP83TG720_MONITOR_CTRL2                                        0x1F, 0x0468
#define DP83TG720_MONITOR_CTRL4                                        0x1F, 0x046A
#define DP83TG720_MONITOR_STAT1                                        0x1F, 0x047B
#define DP83TG720_BREAK_LINK_TIMER                                     0x1F, 0x050A
#define DP83TG720_RS_DECODER                                           0x1F, 0x0510
#define DP83TG720_LPS_CONTROL_1                                        0x1F, 0x0514
#define DP83TG720_LPS_CONTROL_2                                        0x1F, 0x0515
#define DP83TG720_MAXWAIT_TIMER                                        0x1F, 0x0518
#define DP83TG720_PHY_CTRL_1G                                          0x1F, 0x0519
#define DP83TG720_TEST_MODE                                            0x1F, 0x0531
#define DP83TG720_LINK_QUAL_1                                          0x1F, 0x0543
#define DP83TG720_LINK_QUAL_2                                          0x1F, 0x0544
#define DP83TG720_LINK_DOWN_LATCH_STAT                                 0x1F, 0x0545
#define DP83TG720_LINK_QUAL_3                                          0x1F, 0x0547
#define DP83TG720_LINK_QUAL_4                                          0x1F, 0x0548
#define DP83TG720_RS_DECODER_FRAME_STAT_2                              0x1F, 0x0552
#define DP83TG720_PMA_WATCHDOG                                         0x1F, 0x0559
#define DP83TG720_SYMB_POL_CFG                                         0x1F, 0x055B
#define DP83TG720_OAM_CFG                                              0x1F, 0x055C
#define DP83TG720_TEST_MEM_CFG                                         0x1F, 0x0561
#define DP83TG720_FORCE_CTRL1                                          0x1F, 0x0573
#define DP83TG720_RGMII_CTRL                                           0x1F, 0x0600
#define DP83TG720_RGMII_FIFO_STATUS                                    0x1F, 0x0601
#define DP83TG720_RGMII_DELAY_CTRL                                     0x1F, 0x0602
#define DP83TG720_SGMII_CTRL_1                                         0x1F, 0x0608
#define DP83TG720_SGMII_STATUS                                         0x1F, 0x060A
#define DP83TG720_SGMII_CTRL_2                                         0x1F, 0x060C
#define DP83TG720_SGMII_FIFO_STATUS                                    0x1F, 0x060D
#define DP83TG720_PRBS_STATUS_1                                        0x1F, 0x0618
#define DP83TG720_PRBS_CTRL_1                                          0x1F, 0x0619
#define DP83TG720_PRBS_CTRL_2                                          0x1F, 0x061A
#define DP83TG720_PRBS_CTRL_3                                          0x1F, 0x061B
#define DP83TG720_PRBS_STATUS_2                                        0x1F, 0x061C
#define DP83TG720_PRBS_STATUS_3                                        0x1F, 0x061D
#define DP83TG720_PRBS_STATUS_4                                        0x1F, 0x061E
#define DP83TG720_PRBS_STATUS_6                                        0x1F, 0x0620
#define DP83TG720_PRBS_STATUS_8                                        0x1F, 0x0622
#define DP83TG720_PRBS_STATUS_9                                        0x1F, 0x0623
#define DP83TG720_PRBS_CTRL_4                                          0x1F, 0x0624
#define DP83TG720_PRBS_CTRL_5                                          0x1F, 0x0625
#define DP83TG720_PRBS_CTRL_6                                          0x1F, 0x0626
#define DP83TG720_PRBS_CTRL_7                                          0x1F, 0x0627
#define DP83TG720_PRBS_CTRL_8                                          0x1F, 0x0628
#define DP83TG720_PRBS_CTRL_9                                          0x1F, 0x0629
#define DP83TG720_PRBS_CTRL_10                                         0x1F, 0x062A
#define DP83TG720_CRC_STATUS                                           0x1F, 0x0638
#define DP83TG720_PKT_STAT_1                                           0x1F, 0x0639
#define DP83TG720_PKT_STAT_2                                           0x1F, 0x063A
#define DP83TG720_PKT_STAT_3                                           0x1F, 0x063B
#define DP83TG720_PKT_STAT_4                                           0x1F, 0x063C
#define DP83TG720_PKT_STAT_5                                           0x1F, 0x063D
#define DP83TG720_PKT_STAT_6                                           0x1F, 0x063E
#define DP83TG720_SQI_REG_1                                            0x1F, 0x0871
#define DP83TG720_DSP_REG_75                                           0x1F, 0x0875
#define DP83TG720_SQI_1                                                0x1F, 0x08AD

//BMCR register
#define DP83TG720_BMCR_MII_RESET                                       0x8000
#define DP83TG720_BMCR_LOOPBACK                                        0x4000
#define DP83TG720_BMCR_POWER_DOWN                                      0x0800
#define DP83TG720_BMCR_ISOLATE                                         0x0400
#define DP83TG720_BMCR_SPEED_SEL_MSB                                   0x0040

//BMSR register
#define DP83TG720_BMSR_EXTENDED_STATUS                                 0x0100
#define DP83TG720_BMSR_UNIDIRECTIONAL_ABILITY                          0x0080
#define DP83TG720_BMSR_PREAMBLE_SUPRESSION                             0x0040
#define DP83TG720_BMSR_ANEG_COMPLETE                                   0x0020
#define DP83TG720_BMSR_REMOTE_FAULT                                    0x0010
#define DP83TG720_BMSR_ANEG_ABILITY                                    0x0008
#define DP83TG720_BMSR_LINK_STATUS                                     0x0004
#define DP83TG720_BMSR_JABBER_DETECT                                   0x0002
#define DP83TG720_BMSR_EXTENDED_CAPABILITY                             0x0001

//PHYID1 register
#define DP83TG720_PHYID1_OUI_21_16                                     0xFFFF
#define DP83TG720_PHYID1_OUI_21_16_DEFAULT                             0x2000

//PHYID2 register
#define DP83TG720_PHYID2_OUI_5_0                                       0xFC00
#define DP83TG720_PHYID2_OUI_5_0_DEFAULT                               0xA000
#define DP83TG720_PHYID2_MODEL_NUMBER                                  0x03E0
#define DP83TG720_PHYID2_MODEL_NUMBER_DEFAULT                          0x0500
#define DP83TG720_PHYID2_REVISION_NUMBER                               0x001F
#define DP83TG720_PHYID2_REVISION_NUMBER_DEFAULT                       0x0004

//REGCR register
#define DP83TG720_REGCR_CMD                                            0xC000
#define DP83TG720_REGCR_CMD_ADDR                                       0x0000
#define DP83TG720_REGCR_CMD_DATA_NO_POST_INC                           0x4000
#define DP83TG720_REGCR_CMD_DATA_POST_INC_RW                           0x8000
#define DP83TG720_REGCR_CMD_DATA_POST_INC_W                            0xC000
#define DP83TG720_REGCR_DEVAD                                          0x001F

//MII_REG_10 register
#define DP83TG720_MII_REG_10_SIGNAL_DETECT                             0x0400
#define DP83TG720_MII_REG_10_DESCR_LOCK                                0x0200
#define DP83TG720_MII_REG_10_MII_INT                                   0x0080
#define DP83TG720_MII_REG_10_MII_LOOPBACK                              0x0008
#define DP83TG720_MII_REG_10_DUPLEX_MODE_ENV                           0x0004
#define DP83TG720_MII_REG_10_LINK_STATUS                               0x0001

//MII_REG_11 register
#define DP83TG720_MII_REG_11_INT_POLARITY                              0x0008
#define DP83TG720_MII_REG_11_FORCE_INTERRUPT                           0x0004
#define DP83TG720_MII_REG_11_INT_EN                                    0x0002

//MII_REG_12 register
#define DP83TG720_MII_REG_12_LINK_QUAL_INT                             0x8000
#define DP83TG720_MII_REG_12_ENERGY_DET_INT                            0x4000
#define DP83TG720_MII_REG_12_LINK_INT                                  0x2000
#define DP83TG720_MII_REG_12_ESD_INT                                   0x0800
#define DP83TG720_MII_REG_12_MS_TRAIN_DONE_INT                         0x0400
#define DP83TG720_MII_REG_12_LINK_QUAL_INT_EN                          0x0080
#define DP83TG720_MII_REG_12_ENERGY_DET_INT_EN                         0x0040
#define DP83TG720_MII_REG_12_LINK_INT_EN                               0x0020
#define DP83TG720_MII_REG_12_UNUSED_INT_3                              0x0010
#define DP83TG720_MII_REG_12_ESD_INT_EN                                0x0008
#define DP83TG720_MII_REG_12_MS_TRAIN_DONE_INT_EN                      0x0004
#define DP83TG720_MII_REG_12_UNUSED_INT_2                              0x0002
#define DP83TG720_MII_REG_12_UNUSED_INT_1                              0x0001

//MII_REG_13 register
#define DP83TG720_MII_REG_13_UNDER_VOLT_INT                            0x8000
#define DP83TG720_MII_REG_13_OVER_VOLT_INT                             0x4000
#define DP83TG720_MII_REG_13_OVER_TEMP_INT                             0x0800
#define DP83TG720_MII_REG_13_SLEEP_INT                                 0x0400
#define DP83TG720_MII_REG_13_POL_CHANGE_INT                            0x0200
#define DP83TG720_MII_REG_13_NOT_ONE_HOT_INT                           0x0100
#define DP83TG720_MII_REG_13_UNDER_VOLT_INT_EN                         0x0080
#define DP83TG720_MII_REG_13_OVER_VOLT_INT_EN                          0x0040
#define DP83TG720_MII_REG_13_UNUSED_INT_6                              0x0020
#define DP83TG720_MII_REG_13_UNUSED_INT_5                              0x0010
#define DP83TG720_MII_REG_13_OVER_TEMP_INT_EN                          0x0008
#define DP83TG720_MII_REG_13_SLEEP_INT_EN                              0x0004
#define DP83TG720_MII_REG_13_POL_CHANGE_INT_EN                         0x0002
#define DP83TG720_MII_REG_13_NOT_ONE_HOT_INT_EN                        0x0001

//MII_REG_16 register
#define DP83TG720_MII_REG_16_PRBS_SYNC_LOSS                            0x0400
#define DP83TG720_MII_REG_16_CORE_PWR_MODE                             0x0100
#define DP83TG720_MII_REG_16_CFG_DIG_PCS_LOOPBACK                      0x0080
#define DP83TG720_MII_REG_16_LOOPBACK_MODE                             0x007F
#define DP83TG720_MII_REG_16_LOOPBACK_MODE_PCS                         0x0001
#define DP83TG720_MII_REG_16_LOOPBACK_MODE_RS                          0x0002
#define DP83TG720_MII_REG_16_LOOPBACK_MODE_DIGITAL                     0x0004
#define DP83TG720_MII_REG_16_LOOPBACK_MODE_ANALOG                      0x0008
#define DP83TG720_MII_REG_16_LOOPBACK_MODE_REVERSE                     0x0010

//MII_REG_18 register
#define DP83TG720_MII_REG_18_ACK_RECEIVED_INT                          0x8000
#define DP83TG720_MII_REG_18_TX_VALID_CLR_INT                          0x4000
#define DP83TG720_MII_REG_18_POR_DONE_INT                              0x0800
#define DP83TG720_MII_REG_18_NO_FRAME_INT                              0x0400
#define DP83TG720_MII_REG_18_WAKE_REQ_INT                              0x0200
#define DP83TG720_MII_REG_18_LPS_INT                                   0x0100
#define DP83TG720_MII_REG_18_ACK_RECEIVED_INT_EN                       0x0080
#define DP83TG720_MII_REG_18_TX_VALID_CLR_INT_EN                       0x0040
#define DP83TG720_MII_REG_18_POR_DONE_INT_EN                           0x0008
#define DP83TG720_MII_REG_18_NO_FRAME_INT_EN                           0x0004
#define DP83TG720_MII_REG_18_WAKE_REQ_INT_EN                           0x0002
#define DP83TG720_MII_REG_18_LPS_INT_EN                                0x0001

//MII_REG_19 register
#define DP83TG720_MII_REG_19_SOR_PHYADDR                               0x001F

//MII_REG_1E register
#define DP83TG720_MII_REG_1E_TDR_START                                 0x8000
#define DP83TG720_MII_REG_1E_CFG_TDR_AUTO_RUN                          0x4000
#define DP83TG720_MII_REG_1E_TDR_DONE                                  0x0002
#define DP83TG720_MII_REG_1E_TDR_FAIL                                  0x0001

//MII_REG_1F register
#define DP83TG720_MII_REG_1F_SW_GLOBAL_RESET                           0x8000
#define DP83TG720_MII_REG_1F_DIGITAL_RESET                             0x4000

//PMA_PMD_CONTROL_1 register
#define DP83TG720_PMA_PMD_CONTROL_1_PMA_RESET_2                        0x8000
#define DP83TG720_PMA_PMD_CONTROL_1_CFG_LOW_POWER_2                    0x0800

//PMA_PMD_CONTROL_2 register
#define DP83TG720_PMA_PMD_CONTROL_2_CFG_PMA_TYPE_SELECTION             0x003F

//PMA_PMD_TRANSMIT_DISABLE register
#define DP83TG720_PMA_PMD_TRANSMIT_DISABLE_CFG_TRANSMIT_DISABLE_2      0x0001

//PMA_PMD_EXTENDED_ABILIT register
#define DP83TG720_PMA_PMD_EXTENDED_ABILITY2_BASE_T1_EXTENDED_ABILITIES 0x0800

//PMA_PMD_EXTENDED_ABILIT register
#define DP83TG720_PMA_PMD_EXTENDED_ABILITY_MR_1000_BASE_T1_ABILITY     0x0002
#define DP83TG720_PMA_PMD_EXTENDED_ABILITY_MR_100_BASE_T1_ABILITY      0x0001

//PMA_PMD_CONTROL register
#define DP83TG720_PMA_PMD_CONTROL_CFG_MASTER_SLAVE_VAL                 0x4000

//PMA_CONTROL register
#define DP83TG720_PMA_CONTROL_PMA_RESET                                0x8000
#define DP83TG720_PMA_CONTROL_CFG_TRANSMIT_DISABLE                     0x4000
#define DP83TG720_PMA_CONTROL_CFG_LOW_POWER                            0x0800

//PMA_STATUS register
#define DP83TG720_PMA_STATUS_OAM_ABILITY                               0x0800
#define DP83TG720_PMA_STATUS_EEE_ABILITY                               0x0400
#define DP83TG720_PMA_STATUS_RECEIVE_FAULT_ABILITY                     0x0200
#define DP83TG720_PMA_STATUS_LOW_POWER_ABILITY                         0x0100
#define DP83TG720_PMA_STATUS_RECEIVE_POLARITY                          0x0004
#define DP83TG720_PMA_STATUS_RECEIVE_FAULT                             0x0002
#define DP83TG720_PMA_STATUS_PMA_RECEIVE_LINK_STATUS_LL                0x0001

//TRAINING register
#define DP83TG720_TRAINING_CFG_TRAINING_USER_FLD                       0x07F0
#define DP83TG720_TRAINING_CFG_OAM_EN                                  0x0002
#define DP83TG720_TRAINING_CFG_EEE_EN                                  0x0001

//LP_TRAINING register
#define DP83TG720_LP_TRAINING_LP_TRAINING_USER_FLD                     0x07F0
#define DP83TG720_LP_TRAINING_LP_OAM_ADV                               0x0002
#define DP83TG720_LP_TRAINING_LP_EEE_ADV                               0x0001

//TEST_MODE_CONTROL register
#define DP83TG720_TEST_MODE_CONTROL_CFG_TEST_MODE                      0xE000

//PCS_CONTROL_COPY register
#define DP83TG720_PCS_CONTROL_COPY_PCS_RESET_2                         0x8000
#define DP83TG720_PCS_CONTROL_COPY_MMD3_LOOPBACK_2                     0x4000

//PCS_CONTROL register
#define DP83TG720_PCS_CONTROL_PCS_RESET                                0x8000
#define DP83TG720_PCS_CONTROL_MMD3_LOOPBACK                            0x4000

//PCS_STATUS register
#define DP83TG720_PCS_STATUS_TX_LPI_RECEIVED_LH                        0x0800
#define DP83TG720_PCS_STATUS_RX_LPI_RECEIVED_LH                        0x0400
#define DP83TG720_PCS_STATUS_TX_LPI_INDICATION                         0x0200
#define DP83TG720_PCS_STATUS_RX_LPI_INDICATION                         0x0100
#define DP83TG720_PCS_STATUS_PCS_FAULT                                 0x0080
#define DP83TG720_PCS_STATUS_PCS_RECEIVE_LINK_STATUS_LL                0x0004

//PCS_STATUS_2 register
#define DP83TG720_PCS_STATUS_2_PCS_RECEIVE_LINK_STATUS                 0x0400
#define DP83TG720_PCS_STATUS_2_HI_RFER                                 0x0200
#define DP83TG720_PCS_STATUS_2_BLOCK_LOCK                              0x0100
#define DP83TG720_PCS_STATUS_2_HI_RFER_LH                              0x0080
#define DP83TG720_PCS_STATUS_2_BLOCK_LOCK_LL                           0x0040

//OAM_TRANSMIT register
#define DP83TG720_OAM_TRANSMIT_MR_TX_VALID                             0x8000
#define DP83TG720_OAM_TRANSMIT_MR_TX_TOGGLE                            0x4000
#define DP83TG720_OAM_TRANSMIT_MR_TX_RECEIVED                          0x2000
#define DP83TG720_OAM_TRANSMIT_MR_TX_RECEIVED_TOGGLE                   0x1000
#define DP83TG720_OAM_TRANSMIT_MR_TX_MESSAGE_NUM                       0x0F00
#define DP83TG720_OAM_TRANSMIT_MR_RX_PING                              0x0008
#define DP83TG720_OAM_TRANSMIT_MR_TX_PING                              0x0004
#define DP83TG720_OAM_TRANSMIT_MR_TX_SNR                               0x0003

//OAM_TX_MESSAGE_1 register
#define DP83TG720_OAM_TX_MESSAGE_1_MR_TX_MESSAGE_15_0                  0xFFFF

//OAM_TX_MESSAGE_2 register
#define DP83TG720_OAM_TX_MESSAGE_2_MR_TX_MESSAGE_31_16                 0xFFFF

//OAM_TX_MESSAGE_3 register
#define DP83TG720_OAM_TX_MESSAGE_3_MR_TX_MESSAGE_47_32                 0xFFFF

//OAM_TX_MESSAGE_4 register
#define DP83TG720_OAM_TX_MESSAGE_4_MR_TX_MESSAGE_63_48                 0xFFFF

//OAM_RECEIVE register
#define DP83TG720_OAM_RECEIVE_MR_RX_LP_VALID                           0x8000
#define DP83TG720_OAM_RECEIVE_MR_RX_LP_TOGGLE                          0x4000
#define DP83TG720_OAM_RECEIVE_MR_RX_LP_MESSAGE_NUM                     0x0F00
#define DP83TG720_OAM_RECEIVE_MR_RX_LP_SNR                             0x0003

//OAM_RX_MESSAGE_1 register
#define DP83TG720_OAM_RX_MESSAGE_1_MR_RX_LP_MESSAGE_15_0               0xFFFF

//OAM_RX_MESSAGE_2 register
#define DP83TG720_OAM_RX_MESSAGE_2_MR_RX_LP_MESSAGE_31_16              0xFFFF

//OAM_RX_MESSAGE_3 register
#define DP83TG720_OAM_RX_MESSAGE_3_MR_RX_LP_MESSAGE_47_32              0xFFFF

//OAM_RX_MESSAGE_4 register
#define DP83TG720_OAM_RX_MESSAGE_4_MR_RX_LP_MESSAGE_63_48              0xFFFF

//LSR register
#define DP83TG720_LSR_LINK_UP                                          0x8000
#define DP83TG720_LSR_LINK_DOWN                                        0x4000
#define DP83TG720_LSR_PHY_CTRL_SEND_DATA                               0x2000
#define DP83TG720_LSR_LINK_STATUS                                      0x1000
#define DP83TG720_LSR_DESCR_SYNC                                       0x0004
#define DP83TG720_LSR_LOC_RCVR_STATUS                                  0x0002
#define DP83TG720_LSR_REM_RCVR_STATUS                                  0x0001

//LPS_CFG2 register
#define DP83TG720_LPS_CFG2_ED_EN                                       0x0100
#define DP83TG720_LPS_CFG2_SLEEP_EN                                    0x0080
#define DP83TG720_LPS_CFG2_CFG_AUTO_MODE_EN_STRAP                      0x0040
#define DP83TG720_LPS_CFG2_CFG_LPS_MON_EN_STRAP                        0x0020
#define DP83TG720_LPS_CFG2_CFG_LPS_SLEEP_AUTO                          0x0010
#define DP83TG720_LPS_CFG2_CFG_LPS_SLP_CONFIRM                         0x0008
#define DP83TG720_LPS_CFG2_CFG_LPS_AUTO_PWRDN                          0x0004
#define DP83TG720_LPS_CFG2_CFG_LPS_SLEEP_EN                            0x0002
#define DP83TG720_LPS_CFG2_CFG_LPS_SM_EN                               0x0001

//LPS_CFG3 register
#define DP83TG720_LPS_CFG3_CFG_LPS_PWR_MODE_7                          0x0080
#define DP83TG720_LPS_CFG3_CFG_LPS_PWR_MODE_6                          0x0040
#define DP83TG720_LPS_CFG3_CFG_LPS_PWR_MODE_5                          0x0020
#define DP83TG720_LPS_CFG3_CFG_LPS_PWR_MODE_4                          0x0010
#define DP83TG720_LPS_CFG3_CFG_LPS_PWR_MODE_3                          0x0008
#define DP83TG720_LPS_CFG3_CFG_LPS_PWR_MODE_2                          0x0004
#define DP83TG720_LPS_CFG3_CFG_LPS_PWR_MODE_1                          0x0002
#define DP83TG720_LPS_CFG3_CFG_LPS_PWR_MODE_0                          0x0001

//TDR_STATUS0 register
#define DP83TG720_TDR_STATUS0_PEAK1_LOC                                0xFF00
#define DP83TG720_TDR_STATUS0_PEAK0_LOC                                0x00FF

//TDR_STATUS1 register
#define DP83TG720_TDR_STATUS1_PEAK3_LOC                                0xFF00
#define DP83TG720_TDR_STATUS1_PEAK2_LOC                                0x00FF

//TDR_STATUS2 register
#define DP83TG720_TDR_STATUS2_PEAK0_AMP                                0xFF00
#define DP83TG720_TDR_STATUS2_PEAK4_LOC                                0x00FF

//TDR_STATUS5 register
#define DP83TG720_TDR_STATUS5_PEAK4_SIGN                               0x0010
#define DP83TG720_TDR_STATUS5_PEAK3_SIGN                               0x0008
#define DP83TG720_TDR_STATUS5_PEAK2_SIGN                               0x0004
#define DP83TG720_TDR_STATUS5_PEAK1_SIGN                               0x0002
#define DP83TG720_TDR_STATUS5_PEAK0_SIGN                               0x0001

//TDR_TC12 register
#define DP83TG720_TDR_TC12_FAULT_LOC                                   0x3F00
#define DP83TG720_TDR_TC12_TDR_STATE                                   0x00F0
#define DP83TG720_TDR_TC12_TDR_ACTIVATION                              0x0003

//A2D_REG_05 register
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL                           0xFC00
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_400_MV                    0x2800
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_440_MV                    0x2C00
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_480_MV                    0x3000
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_520_MV                    0x3400
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_560_MV                    0x3800
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_600_MV                    0x3C00
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_640_MV                    0x4000
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_680_MV                    0x4400
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_720_MV                    0x4800
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_760_MV                    0x4C00
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_800_MV                    0x5000
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_840_MV                    0x5400
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_880_MV                    0x5800
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_920_MV                    0x5C00
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_960_MV                    0x6000
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_1000_MV                   0x6400
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_1040_MV                   0x6800
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_1080_MV                   0x6C00
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_1120_MV                   0x7000
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_1160_MV                   0x7400
#define DP83TG720_A2D_REG_05_LD_BIAS_1P0V_SL_1200_MV                   0x7800

//A2D_REG_30 register
#define DP83TG720_A2D_REG_30_SPARE_IN_2_FROMDIG_SL_FORCE_EN            0x0100

//A2D_REG_31 register

//A2D_REG_40 register
#define DP83TG720_A2D_REG_40_SGMII_TESTMODE                            0x6000
#define DP83TG720_A2D_REG_40_SGMII_SOP_SON_SLEW_CTRL                   0x0800

//A2D_REG_41 register
#define DP83TG720_A2D_REG_41_SGMII_IO_LOOPBACK_EN                      0x0002

//A2D_REG_43 register
#define DP83TG720_A2D_REG_43_SGMII_CDR_TESTMODE_1                      0xFFFF

//A2D_REG_44 register
#define DP83TG720_A2D_REG_44_SGMII_DIG_LOOPBACK_EN                     0x0010

//A2D_REG_46 register
#define DP83TG720_A2D_REG_46_SGMII_CALIB_WATCHDOG_DIS                  0x0800
#define DP83TG720_A2D_REG_46_SGMII_CALIB_WATCHDOG_VAL                  0x0600
#define DP83TG720_A2D_REG_46_SGMII_CALIB_AVG                           0x0180
#define DP83TG720_A2D_REG_46_SGMII_DO_CALIB                            0x0040
#define DP83TG720_A2D_REG_46_SGMII_CDR_LOCK_SL                         0x0020
#define DP83TG720_A2D_REG_46_SGMII_MODE_FORCE_EN                       0x0010
#define DP83TG720_A2D_REG_46_SGMII_INPUT_TERM_EN_FORCE_EN              0x0008
#define DP83TG720_A2D_REG_46_SGMII_OUTPUT_EN_FORCE_EN                  0x0004
#define DP83TG720_A2D_REG_46_SGMII_COMP_OFFSET_TUNE_FORCE_EN           0x0002
#define DP83TG720_A2D_REG_46_SGMII_DATA_SYNC_SL                        0x0001

//A2D_REG_47 register
#define DP83TG720_A2D_REG_47_SPARE_IN_2_FROMDIG_SL_2                   0x0004
#define DP83TG720_A2D_REG_47_SPARE_IN_2_FROMDIG_SL_1                   0x0002
#define DP83TG720_A2D_REG_47_SPARE_IN_2_FROMDIG_SL_0                   0x0001

//A2D_REG_48 register
#define DP83TG720_A2D_REG_48_DLL_EN                                    0x1000
#define DP83TG720_A2D_REG_48_DLL_TX_DELAY_CTRL_SL                      0x0F00
#define DP83TG720_A2D_REG_48_DLL_RX_DELAY_CTRL_SL                      0x00F0

//A2D_REG_66 register
#define DP83TG720_A2D_REG_66_ESD_EVENT_COUNT                           0x7E00

//LEDS_CFG_1 register
#define DP83TG720_LEDS_CFG_1_LEDS_BYPASS_STRETCHING                    0x4000
#define DP83TG720_LEDS_CFG_1_LEDS_BLINK_RATE                           0x3000
#define DP83TG720_LEDS_CFG_1_LEDS_BLINK_RATE_20HZ                      0x0000
#define DP83TG720_LEDS_CFG_1_LEDS_BLINK_RATE_10HZ                      0x1000
#define DP83TG720_LEDS_CFG_1_LEDS_BLINK_RATE_5HZ                       0x2000
#define DP83TG720_LEDS_CFG_1_LEDS_BLINK_RATE_2HZ                       0x3000
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION                              0x0F00
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_LINK_OK                      0x0000
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_TX_RX_ACT                    0x0100
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_TX_ACT                       0x0200
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_RX_ACT                       0x0300
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_MASTER                       0x0400
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_SLAVE                        0x0500
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_TX_RX_ACT_STRETCH            0x0600
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_LINK_LOST                    0x0900
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_PRBS_ERROR                   0x0A00
#define DP83TG720_LEDS_CFG_1_LED_2_OPTION_XMII_TX_RX_ERROR             0x0B00
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION                              0x00F0
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_LINK_OK                      0x0000
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_TX_RX_ACT                    0x0010
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_TX_ACT                       0x0020
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_RX_ACT                       0x0030
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_MASTER                       0x0040
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_SLAVE                        0x0050
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_TX_RX_ACT_STRETCH            0x0060
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_LINK_LOST                    0x0090
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_PRBS_ERROR                   0x00A0
#define DP83TG720_LEDS_CFG_1_LED_1_OPTION_XMII_TX_RX_ERROR             0x00B0
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION                              0x000F
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_LINK_OK                      0x0000
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_TX_RX_ACT                    0x0001
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_TX_ACT                       0x0002
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_RX_ACT                       0x0003
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_MASTER                       0x0004
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_SLAVE                        0x0005
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_TX_RX_ACT_STRETCH            0x0006
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_LINK_LOST                    0x0009
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_PRBS_ERROR                   0x000A
#define DP83TG720_LEDS_CFG_1_LED_0_OPTION_XMII_TX_RX_ERROR             0x000B

//LEDS_CFG_2 register
#define DP83TG720_LEDS_CFG_2_CFG_IEEE_COMPL_SEL                        0x0E00
#define DP83TG720_LEDS_CFG_2_CFG_IEEE_COMPL_SEL_LOC_RCVR_STATUS        0x0000
#define DP83TG720_LEDS_CFG_2_CFG_IEEE_COMPL_SEL_REM_RCVR_STATUS        0x0200
#define DP83TG720_LEDS_CFG_2_CFG_IEEE_COMPL_SEL_LOC_SNR_MARGIN         0x0400
#define DP83TG720_LEDS_CFG_2_CFG_IEEE_COMPL_SEL_REM_PHY_READY          0x0600
#define DP83TG720_LEDS_CFG_2_CFG_IEEE_COMPL_SEL_PMA_WATCHDOG_STATUS    0x0800
#define DP83TG720_LEDS_CFG_2_CFG_IEEE_COMPL_SEL_LINK_SYNC_LINK_CONTROL 0x0A00
#define DP83TG720_LEDS_CFG_2_LED_2_DRV_EN                              0x0100
#define DP83TG720_LEDS_CFG_2_LED_2_DRV_VAL                             0x0080
#define DP83TG720_LEDS_CFG_2_LED_2_POLARITY                            0x0040
#define DP83TG720_LEDS_CFG_2_LED_1_DRV_EN                              0x0020
#define DP83TG720_LEDS_CFG_2_LED_1_DRV_VAL                             0x0010
#define DP83TG720_LEDS_CFG_2_LED_1_POLARITY                            0x0008
#define DP83TG720_LEDS_CFG_2_LED_0_DRV_EN                              0x0004
#define DP83TG720_LEDS_CFG_2_LED_0_DRV_VAL                             0x0002
#define DP83TG720_LEDS_CFG_2_LED_0_POLARITY                            0x0001

//IO_MUX_CFG_1 register
#define DP83TG720_IO_MUX_CFG_1_LED_1_GPIO_CTRL                         0x0700
#define DP83TG720_IO_MUX_CFG_1_LED_1_GPIO_CTRL_LED_1                   0x0000
#define DP83TG720_IO_MUX_CFG_1_LED_1_GPIO_CTRL_RGMII_DATA_MATCH        0x0200
#define DP83TG720_IO_MUX_CFG_1_LED_1_GPIO_CTRL_UNDER_VOLTAGE           0x0300
#define DP83TG720_IO_MUX_CFG_1_LED_1_GPIO_CTRL_INTERRUPT               0x0400
#define DP83TG720_IO_MUX_CFG_1_LED_1_GPIO_CTRL_IEEE                    0x0500
#define DP83TG720_IO_MUX_CFG_1_LED_1_GPIO_CTRL_LOW                     0x0600
#define DP83TG720_IO_MUX_CFG_1_LED_1_GPIO_CTRL_HIGH                    0x0700
#define DP83TG720_IO_MUX_CFG_1_LED_0_GPIO_CTRL                         0x0007
#define DP83TG720_IO_MUX_CFG_1_LED_0_GPIO_CTRL_LED_0                   0x0000
#define DP83TG720_IO_MUX_CFG_1_LED_0_GPIO_CTRL_RGMII_DATA_MATCH        0x0002
#define DP83TG720_IO_MUX_CFG_1_LED_0_GPIO_CTRL_UNDER_VOLTAGE           0x0003
#define DP83TG720_IO_MUX_CFG_1_LED_0_GPIO_CTRL_INTERRUPT               0x0004
#define DP83TG720_IO_MUX_CFG_1_LED_0_GPIO_CTRL_IEEE                    0x0005
#define DP83TG720_IO_MUX_CFG_1_LED_0_GPIO_CTRL_LOW                     0x0006
#define DP83TG720_IO_MUX_CFG_1_LED_0_GPIO_CTRL_HIGH                    0x0007

//IO_MUX_CFG_2 register
#define DP83TG720_IO_MUX_CFG_2_CLK_O_CLK_SOURCE                        0x0038
#define DP83TG720_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_XI_OSC_25M_1P0V_DL     0x0000
#define DP83TG720_IO_MUX_CFG_2_CLK_O_CLK_SOURCE_125MHZ_CLK             0x0018
#define DP83TG720_IO_MUX_CFG_2_CLK_O_GPIO_CTRL                         0x0007
#define DP83TG720_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_LED_2                   0x0000
#define DP83TG720_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_RGMII_DATA_MATCH        0x0002
#define DP83TG720_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_UNDER_VOLTAGE           0x0003
#define DP83TG720_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_LOW                     0x0004
#define DP83TG720_IO_MUX_CFG_2_CLK_O_GPIO_CTRL_HIGH                    0x0007

//IO_CONTROL_1 register
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1                            0xFFFF
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_LED_0_GPIO_0               0x0000
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_LED_1_GPIO_1               0x0001
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_CLKOUT_GPIO_2              0x0002
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_INT_N                      0x0003
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_INH                        0x0006
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_TX_CLK                     0x0007
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_TX_CTRL                    0x0008
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_TX_D0                      0x0009
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_TX_D1                      0x000A
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_TX_D2                      0x000B
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_TX_D3                      0x000C
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_RX_CLK                     0x000D
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_RX_CTRL                    0x000E
#define DP83TG720_IO_CONTROL_1_IO_CONTROL_1_RX_D0                      0x000F

//IO_CONTROL_2 register
#define DP83TG720_IO_CONTROL_2_CFG_OTHER_IMPEDANCE                     0x3E00
#define DP83TG720_IO_CONTROL_2_CFG_OTHER_IMPEDANCE_DEFAULT             0x0000
#define DP83TG720_IO_CONTROL_2_CFG_OTHER_IMPEDANCE_SLOWER              0x0200
#define DP83TG720_IO_CONTROL_2_CFG_OTHER_IMPEDANCE_FASTER              0x0400
#define DP83TG720_IO_CONTROL_2_PUPD_VALUE                              0x0180
#define DP83TG720_IO_CONTROL_2_PUPD_VALUE_NO_PULL                      0x0000
#define DP83TG720_IO_CONTROL_2_PUPD_VALUE_PULL_UP                      0x0080
#define DP83TG720_IO_CONTROL_2_PUPD_VALUE_PULL_DOWN                    0x0100
#define DP83TG720_IO_CONTROL_2_PUPD_VALUE_BOTH                         0x0180
#define DP83TG720_IO_CONTROL_2_PUPD_FORCE_CNTL                         0x0040
#define DP83TG720_IO_CONTROL_2_IO_OE_N_VALUE                           0x0020
#define DP83TG720_IO_CONTROL_2_IO_OE_N_FORCE_CTRL                      0x0010
#define DP83TG720_IO_CONTROL_2_IO_CONTROL_2                            0x000F
#define DP83TG720_IO_CONTROL_2_IO_CONTROL_2_RX_D1                      0x0000
#define DP83TG720_IO_CONTROL_2_IO_CONTROL_2_RX_D2                      0x0001
#define DP83TG720_IO_CONTROL_2_IO_CONTROL_2_RX_D3                      0x0002
#define DP83TG720_IO_CONTROL_2_IO_CONTROL_2_STRP_1                     0x0003

//IO_CONTROL_3 register
#define DP83TG720_IO_CONTROL_3_CFG_MAC_RX_IMPEDANCE                    0x03E0
#define DP83TG720_IO_CONTROL_3_CFG_MAC_RX_IMPEDANCE_MEDIUM             0x0140
#define DP83TG720_IO_CONTROL_3_CFG_MAC_RX_IMPEDANCE_SLOWEST            0x0160
#define DP83TG720_IO_CONTROL_3_CFG_MAC_RX_IMPEDANCE_DEFAULT            0x0100

//IO_STATUS_1 register
#define DP83TG720_IO_STATUS_1_IO_STATUS_1                              0xFFFF
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_LED_0_GPIO_0                 0x0000
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_LED_1_GPIO_1                 0x0001
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_CLKOUT_GPIO_2                0x0002
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_INT_N                        0x0003
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_INH                          0x0006
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_TX_CLK                       0x0007
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_TX_CTRL                      0x0008
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_TX_D0                        0x0009
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_TX_D1                        0x000A
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_TX_D2                        0x000B
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_TX_D3                        0x000C
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_RX_CLK                       0x000D
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_RX_CTRL                      0x000E
#define DP83TG720_IO_STATUS_1_IO_STATUS_1_RX_D0                        0x000F

//IO_STATUS_2 register
#define DP83TG720_IO_STATUS_2_IO_STATUS_2                              0x000F
#define DP83TG720_IO_STATUS_2_IO_STATUS_2_RX_D1                        0x0000
#define DP83TG720_IO_STATUS_2_IO_STATUS_2_RX_D2                        0x0001
#define DP83TG720_IO_STATUS_2_IO_STATUS_2_RX_D3                        0x0002
#define DP83TG720_IO_STATUS_2_IO_STATUS_2_STRP_1                       0x0003

//IO_CONTROL_4 register
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE                           0xFFFF
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_LED_0_GPIO_0              0x0000
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_LED_1_GPIO_1              0x0001
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_CLKOUT_GPIO_2             0x0002
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_INT_N                     0x0003
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_TX_CLK                    0x0004
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_TX_CTRL                   0x0005
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_TX_D0                     0x0006
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_TX_D1                     0x0007
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_TX_D2                     0x0008
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_TX_D3                     0x0009
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_RX_CLK                    0x000A
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_RX_CTRL                   0x000B
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_RX_D0                     0x000C
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_RX_D1                     0x000D
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_RX_D2                     0x000E
#define DP83TG720_IO_CONTROL_4_IO_INPUT_MODE_RX_D3                     0x000F

//IO_CONTROL_5 register
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE                          0xFFFF
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_LED_0_GPIO_0             0x0000
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_LED_1_GPIO_1             0x0001
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_CLKOUT_GPIO_2            0x0002
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_INT_N                    0x0003
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_TX_CLK                   0x0004
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_TX_CTRL                  0x0005
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_TX_D0                    0x0006
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_TX_D1                    0x0007
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_TX_D2                    0x0008
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_TX_D3                    0x0009
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_RX_CLK                   0x000A
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_RX_CTRL                  0x000B
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_RX_D0                    0x000C
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_RX_D1                    0x000D
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_RX_D2                    0x000E
#define DP83TG720_IO_CONTROL_5_IO_OUTPUT_MODE_RX_D3                    0x000F

//SOR_VECTOR_1 register
#define DP83TG720_SOR_VECTOR_1_RGMII_TX_SHIFT                          0x8000
#define DP83TG720_SOR_VECTOR_1_RGMII_RX_SHIFT                          0x4000
#define DP83TG720_SOR_VECTOR_1_SGMII_EN                                0x2000
#define DP83TG720_SOR_VECTOR_1_RGMII_EN                                0x1000
#define DP83TG720_SOR_VECTOR_1_TEST_MODE                               0x0E00
#define DP83TG720_SOR_VECTOR_1_MAC_MODE                                0x01C0
#define DP83TG720_SOR_VECTOR_1_MAC_MODE_SGMII                          0x0000
#define DP83TG720_SOR_VECTOR_1_MAC_MODE_RGMII_ALIGN                    0x0100
#define DP83TG720_SOR_VECTOR_1_MAC_MODE_RGMII_TX_SHIFT                 0x0140
#define DP83TG720_SOR_VECTOR_1_MAC_MODE_RGMII_TX_RX_SHIFT              0x0180
#define DP83TG720_SOR_VECTOR_1_MAC_MODE_RGMII_RX_SHIFT                 0x01C0
#define DP83TG720_SOR_VECTOR_1_MAS_SLV                                 0x0020
#define DP83TG720_SOR_VECTOR_1_PHY_AD                                  0x001F

//SOR_VECTOR_2 register
#define DP83TG720_SOR_VECTOR_2_AUTO_MANAGED                            0x0001

//MONITOR_CTRL1 register
#define DP83TG720_MONITOR_CTRL1_CFG_DC_OFFSET_2C                       0xFF00
#define DP83TG720_MONITOR_CTRL1_CFG_CIC_GAIN12_ARITH                   0x00C0
#define DP83TG720_MONITOR_CTRL1_CFG_CIC_GAIN2                          0x0038
#define DP83TG720_MONITOR_CTRL1_CFG_CIC_GAIN1                          0x0007

//MONITOR_CTRL2 register
#define DP83TG720_MONITOR_CTRL2_CFG_BYPASS_RESET_SENSOR_VAL            0x8000
#define DP83TG720_MONITOR_CTRL2_CFG_RD_DATA                            0x7000
#define DP83TG720_MONITOR_CTRL2_CFG_DEC_FACTOR_SENSORS                 0x0E00
#define DP83TG720_MONITOR_CTRL2_CFG_DEC_FACTOR_GAIN_CALIB              0x01C0
#define DP83TG720_MONITOR_CTRL2_CFG_DEC_FACTOR_DC_CALIB                0x0038
#define DP83TG720_MONITOR_CTRL2_CFG_BYPASS_SEL_NUM                     0x0007

//MONITOR_CTRL4 register
#define DP83TG720_MONITOR_CTRL4_CFG_HIST_CLR                           0x0100
#define DP83TG720_MONITOR_CTRL4_CFG_DISCARD_SAMPLE_NUM                 0x0080
#define DP83TG720_MONITOR_CTRL4_CFG_AVG_SAMPLE_NUM                     0x0040
#define DP83TG720_MONITOR_CTRL4_CFG_ADC_CLK_DIV                        0x0030
#define DP83TG720_MONITOR_CTRL4_CFG_FORCE_START                        0x0008
#define DP83TG720_MONITOR_CTRL4_CFG_RESET                              0x0004
#define DP83TG720_MONITOR_CTRL4_PERIODIC                               0x0002
#define DP83TG720_MONITOR_CTRL4_START                                  0x0001

//MONITOR_STAT1 register
#define DP83TG720_MONITOR_STAT1_STAT_RD_DATA                           0xFFFF

//BREAK_LINK_TIMER register
#define DP83TG720_BREAK_LINK_TIMER_CFG_FIFO_RESET_IN_BREAK_LINK        0x1000
#define DP83TG720_BREAK_LINK_TIMER_CFG_SLAVE_SEND_S_32_MODE            0x0800

//RS_DECODER register
#define DP83TG720_RS_DECODER_CFG_RS_DECODER_BYPASS                     0x8000

//LPS_CONTROL_1 register
#define DP83TG720_LPS_CONTROL_1_CFG_TX_WAKE_CG                         0x0E00
#define DP83TG720_LPS_CONTROL_1_CFG_TX_SLEEP_CG                        0x01C0
#define DP83TG720_LPS_CONTROL_1_CFG_RX_WAKE_CG                         0x0038
#define DP83TG720_LPS_CONTROL_1_CFG_RX_SLEEP_CG                        0x0007

//LPS_CONTROL_2 register
#define DP83TG720_LPS_CONTROL_2_CFG_WAKE_CG_CNT_TH                     0x7F00
#define DP83TG720_LPS_CONTROL_2_CFG_SLEEP_CG_CNT_TH                    0x007F

//MAXWAIT_TIMER register
#define DP83TG720_MAXWAIT_TIMER_CFG_MAXWAIT_TIMER_INIT                 0xFFFF

//PHY_CTRL_1G register
#define DP83TG720_PHY_CTRL_1G_CFG_FORCE_LINK_STAT_VAL                  0x0800
#define DP83TG720_PHY_CTRL_1G_CFG_FORCE_LINK_STAT                      0x0400
#define DP83TG720_PHY_CTRL_1G_CFG_MINWAIT_TIMER_INIT                   0x00FF

//TEST_MODE register
#define DP83TG720_TEST_MODE_CFG_TEST_MODE4_TX_ORDER                    0x0100
#define DP83TG720_TEST_MODE_CFG_TEST_MODE_7_DATA                       0x00FF

//LINK_QUAL_1 register
#define DP83TG720_LINK_QUAL_1_LINK_TRAINING_TIME                       0x00FF

//LINK_QUAL_2 register
#define DP83TG720_LINK_QUAL_2_REMOTE_RECEIVER_TIME                     0xFF00
#define DP83TG720_LINK_QUAL_2_LOCAL_RECEIVER_TIME                      0x00FF

//LINK_DOWN_LATCH_STAT register
#define DP83TG720_LINK_DOWN_LATCH_STAT_CHANNEL_OK_LL                   0x0020
#define DP83TG720_LINK_DOWN_LATCH_STAT_LINK_FAIL_INHIBIT_LH            0x0010
#define DP83TG720_LINK_DOWN_LATCH_STAT_SEND_S_SIGDET_LH                0x0008
#define DP83TG720_LINK_DOWN_LATCH_STAT_HI_RFER_LH                      0x0004
#define DP83TG720_LINK_DOWN_LATCH_STAT_BLOCK_LOCK_LL                   0x0002
#define DP83TG720_LINK_DOWN_LATCH_STAT_PMA_WATCHDOG_LL                 0x0001

//LINK_QUAL_3 register
#define DP83TG720_LINK_QUAL_3_LINK_LOSS_CNT                            0xFC00
#define DP83TG720_LINK_QUAL_3_LINK_FAIL_CNT                            0x03FF

//LINK_QUAL_4 register
#define DP83TG720_LINK_QUAL_4_COMM_READY                               0x0001

//RS_DECODER_FRAME_STAT_2 register
#define DP83TG720_RS_DECODER_FRAME_STAT_2_RS_DEC_UNCORR_FRAME_CNT      0xFFFF

//PMA_WATCHDOG register
#define DP83TG720_PMA_WATCHDOG_CFG_PMA_WATCHDOG_FORCE_VAL              0x0040
#define DP83TG720_PMA_WATCHDOG_CFG_PMA_WATCHDOG_FORCE_EN               0x0020
#define DP83TG720_PMA_WATCHDOG_CFG_IEEE_WATCHDOG_EN                    0x0010
#define DP83TG720_PMA_WATCHDOG_CFG_WATCHDOG_CNT_CLR_TH                 0x000F

//SYMB_POL_CFG register
#define DP83TG720_SYMB_POL_CFG_CFG_SLAVE_AUTO_POL_CORRECTION_EN        0x0010
#define DP83TG720_SYMB_POL_CFG_CFG_RX_SYMB_ORDER_INV                   0x0008
#define DP83TG720_SYMB_POL_CFG_CFG_RX_SYMB_POL_INV                     0x0004
#define DP83TG720_SYMB_POL_CFG_CFG_TX_SYMB_ORDER_INV                   0x0002
#define DP83TG720_SYMB_POL_CFG_CFG_TX_SYMB_POL_INV                     0x0001

//OAM_CFG register
#define DP83TG720_OAM_CFG_CFG_RX_OAM_CRC_DATA_IN_ORDER                 0x0002
#define DP83TG720_OAM_CFG_CFG_TX_OAM_CRC_DATA_IN_ORDER                 0x0001

//TEST_MEM_CFG register
#define DP83TG720_TEST_MEM_CFG_CFG_WAIT_TIME_XCORR_WEN                 0x1FC0
#define DP83TG720_TEST_MEM_CFG_CFG_XCORR_DBG_SEL                       0x0020
#define DP83TG720_TEST_MEM_CFG_CFG_SEND_S_INFINITE_LOOP                0x0010
#define DP83TG720_TEST_MEM_CFG_CFG_XCORR_DBG_TEST_MEM                  0x0008
#define DP83TG720_TEST_MEM_CFG_CFG_ECC_EN                              0x0004
#define DP83TG720_TEST_MEM_CFG_CFG_TEST_MEM_SIGDET_DEBUG               0x0002
#define DP83TG720_TEST_MEM_CFG_CFG_PCS_TEST_MEM_MODE                   0x0001

//FORCE_CTRL1 register
#define DP83TG720_FORCE_CTRL1_CFG_FORCE_LINK_SYNC_STATE_EN             0x0100
#define DP83TG720_FORCE_CTRL1_CFG_FORCE_LINK_SYNC_STATE_VAL            0x00FF

//RGMII_CTRL register
#define DP83TG720_RGMII_CTRL_RGMII_RX_HALF_FULL_TH                     0x0380
#define DP83TG720_RGMII_CTRL_RGMII_RX_HALF_FULL_TH_DEFAULT             0x0100
#define DP83TG720_RGMII_CTRL_RGMII_TX_HALF_FULL_TH                     0x0070
#define DP83TG720_RGMII_CTRL_RGMII_TX_HALF_FULL_TH_DEFAULT             0x0020
#define DP83TG720_RGMII_CTRL_RGMII_TX_IF_EN                            0x0008
#define DP83TG720_RGMII_CTRL_INVERT_RGMII_TXD                          0x0004
#define DP83TG720_RGMII_CTRL_INVERT_RGMII_RXD                          0x0002
#define DP83TG720_RGMII_CTRL_SUP_TX_ERR_FD                             0x0001

//RGMII_FIFO_STATUS register
#define DP83TG720_RGMII_FIFO_STATUS_RGMII_RX_AF_FULL_ERR               0x0008
#define DP83TG720_RGMII_FIFO_STATUS_RGMII_RX_AF_EMPTY_ERR              0x0004
#define DP83TG720_RGMII_FIFO_STATUS_RGMII_TX_AF_FULL_ERR               0x0002
#define DP83TG720_RGMII_FIFO_STATUS_RGMII_TX_AF_EMPTY_ERR              0x0001

//RGMII_DELAY_CTRL register
#define DP83TG720_RGMII_DELAY_CTRL_RX_CLK_SEL                          0x0002
#define DP83TG720_RGMII_DELAY_CTRL_TX_CLK_SEL                          0x0001

//SGMII_CTRL_1 register
#define DP83TG720_SGMII_CTRL_1_SGMII_TX_ERR_DIS                        0x8000
#define DP83TG720_SGMII_CTRL_1_CFG_ALIGN_IDX_FORCE                     0x4000
#define DP83TG720_SGMII_CTRL_1_CFG_ALIGN_IDX_VALUE                     0x3C00
#define DP83TG720_SGMII_CTRL_1_CFG_SGMII_EN                            0x0200
#define DP83TG720_SGMII_CTRL_1_CFG_SGMII_RX_POL_INVERT                 0x0100
#define DP83TG720_SGMII_CTRL_1_CFG_SGMII_TX_POL_INVERT                 0x0080
#define DP83TG720_SGMII_CTRL_1_SGMII_AUTONEG_TIMER                     0x0006
#define DP83TG720_SGMII_CTRL_1_MR_AN_ENABLE                            0x0001

//SGMII_STATUS register
#define DP83TG720_SGMII_STATUS_SGMII_PAGE_RECEIVED                     0x1000
#define DP83TG720_SGMII_STATUS_LINK_STATUS_1000BX                      0x0800
#define DP83TG720_SGMII_STATUS_MR_AN_COMPLETE                          0x0400
#define DP83TG720_SGMII_STATUS_CFG_ALIGN_EN                            0x0200
#define DP83TG720_SGMII_STATUS_CFG_SYNC_STATUS                         0x0100
#define DP83TG720_SGMII_STATUS_CFG_ALIGN_IDX                           0x00F0
#define DP83TG720_SGMII_STATUS_CFG_STATE                               0x000F

//SGMII_CTRL_2 register
#define DP83TG720_SGMII_CTRL_2_SGMII_SIGNAL_DETECT_FORCE_VAL           0x0100
#define DP83TG720_SGMII_CTRL_2_SGMII_SIGNAL_DETECT_FORCE_EN            0x0080
#define DP83TG720_SGMII_CTRL_2_MR_RESTART_AN                           0x0040
#define DP83TG720_SGMII_CTRL_2_TX_HALF_FULL_TH                         0x0038
#define DP83TG720_SGMII_CTRL_2_RX_HALF_FULL_TH                         0x0007

//SGMII_FIFO_STATUS register
#define DP83TG720_SGMII_FIFO_STATUS_SGMII_RX_AF_FULL_ERR               0x0008
#define DP83TG720_SGMII_FIFO_STATUS_SGMII_RX_AF_EMPTY_ERR              0x0004
#define DP83TG720_SGMII_FIFO_STATUS_SGMII_TX_AF_FULL_ERR               0x0002
#define DP83TG720_SGMII_FIFO_STATUS_SGMII_TX_AF_EMPTY_ERR              0x0001

//PRBS_STATUS_1 register
#define DP83TG720_PRBS_STATUS_1_PRBS_ERR_OV_CNT                        0x00FF

//PRBS_CTRL_1 register
#define DP83TG720_PRBS_CTRL_1_CFG_PKT_GEN_64                           0x2000
#define DP83TG720_PRBS_CTRL_1_SEND_PKT                                 0x1000
#define DP83TG720_PRBS_CTRL_1_CFG_PRBS_CHK_SEL                         0x0700
#define DP83TG720_PRBS_CTRL_1_CFG_PRBS_GEN_SEL                         0x0070
#define DP83TG720_PRBS_CTRL_1_CFG_PRBS_CNT_MODE                        0x0008
#define DP83TG720_PRBS_CTRL_1_CFG_PRBS_CHK_ENABLE                      0x0004
#define DP83TG720_PRBS_CTRL_1_CFG_PKT_GEN_PRBS                         0x0002
#define DP83TG720_PRBS_CTRL_1_PKT_GEN_EN                               0x0001

//PRBS_CTRL_2 register
#define DP83TG720_PRBS_CTRL_2_CFG_PKT_LEN_PRBS                         0xFFFF

//PRBS_CTRL_3 register
#define DP83TG720_PRBS_CTRL_3_CFG_IPG_LEN                              0x00FF

//PRBS_STATUS_2 register
#define DP83TG720_PRBS_STATUS_2_PRBS_BYTE_CNT                          0xFFFF

//PRBS_STATUS_3 register
#define DP83TG720_PRBS_STATUS_3_PRBS_PKT_CNT_15_0                      0xFFFF

//PRBS_STATUS_4 register
#define DP83TG720_PRBS_STATUS_4_PRBS_PKT_CNT_31_16                     0xFFFF

//PRBS_STATUS_6 register
#define DP83TG720_PRBS_STATUS_6_PKT_DONE                               0x1000
#define DP83TG720_PRBS_STATUS_6_PKT_GEN_BUSY                           0x0800
#define DP83TG720_PRBS_STATUS_6_PRBS_PKT_OV                            0x0400
#define DP83TG720_PRBS_STATUS_6_PRBS_BYTE_OV                           0x0200
#define DP83TG720_PRBS_STATUS_6_PRBS_LOCK                              0x0100
#define DP83TG720_PRBS_STATUS_6_PRBS_ERR_CNT                           0x00FF

//PRBS_STATUS_8 register
#define DP83TG720_PRBS_STATUS_8_PKT_ERR_CNT_15_0                       0xFFFF

//PRBS_STATUS_9 register
#define DP83TG720_PRBS_STATUS_9_PKT_ERR_CNT_31_16                      0xFFFF

//PRBS_CTRL_4 register
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_DATA                             0xFF00
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_DATA_DEFAULT                     0x5500
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_MODE                             0x00C0
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_MODE_INCREMENTAL                 0x0000
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_MODE_FIXED                       0x0040
#define DP83TG720_PRBS_CTRL_4_CFG_PATTERN_VLD_BYTES                    0x0038
#define DP83TG720_PRBS_CTRL_4_CFG_PATTERN_VLD_BYTES_0_BYTE             0x0000
#define DP83TG720_PRBS_CTRL_4_CFG_PATTERN_VLD_BYTES_1_BYTE             0x0008
#define DP83TG720_PRBS_CTRL_4_CFG_PATTERN_VLD_BYTES_2_BYTES            0x0010
#define DP83TG720_PRBS_CTRL_4_CFG_PATTERN_VLD_BYTES_3_BYTES            0x0018
#define DP83TG720_PRBS_CTRL_4_CFG_PATTERN_VLD_BYTES_4_BYTES            0x0020
#define DP83TG720_PRBS_CTRL_4_CFG_PATTERN_VLD_BYTES_5_BYTES            0x0028
#define DP83TG720_PRBS_CTRL_4_CFG_PATTERN_VLD_BYTES_6_BYTES            0x0030
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_CNT                              0x0007
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_CNT_1_PACKET                     0x0000
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_CNT_10_PACKETS                   0x0001
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_CNT_100_PACKETS                  0x0002
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_CNT_1000_PACKETS                 0x0003
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_CNT_10000_PACKETS                0x0004
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_CNT_100000_PACKETS               0x0005
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_CNT_1000000_PACKETS              0x0006
#define DP83TG720_PRBS_CTRL_4_CFG_PKT_CNT_CONTINUOUS                   0x0007

//PRBS_CTRL_5 register
#define DP83TG720_PRBS_CTRL_5_PATTERN_15_0                             0xFFFF

//PRBS_CTRL_6 register
#define DP83TG720_PRBS_CTRL_6_PATTERN_31_16                            0xFFFF

//PRBS_CTRL_7 register
#define DP83TG720_PRBS_CTRL_7_PATTERN_47_32                            0xFFFF

//PRBS_CTRL_8 register
#define DP83TG720_PRBS_CTRL_8_PMATCH_DATA_15_0                         0xFFFF

//PRBS_CTRL_9 register
#define DP83TG720_PRBS_CTRL_9_PMATCH_DATA_31_16                        0xFFFF

//PRBS_CTRL_10 register
#define DP83TG720_PRBS_CTRL_10_PMATCH_DATA_47_32                       0xFFFF

//CRC_STATUS register
#define DP83TG720_CRC_STATUS_RX_BAD_CRC                                0x0002
#define DP83TG720_CRC_STATUS_TX_BAD_CRC                                0x0001

//PKT_STAT_1 register
#define DP83TG720_PKT_STAT_1_TX_PKT_CNT_15_0                           0xFFFF

//PKT_STAT_2 register
#define DP83TG720_PKT_STAT_2_TX_PKT_CNT_31_16                          0xFFFF

//PKT_STAT_3 register
#define DP83TG720_PKT_STAT_3_TX_ERR_PKT_CNT                            0xFFFF

//PKT_STAT_4 register
#define DP83TG720_PKT_STAT_4_RX_PKT_CNT_15_0                           0xFFFF

//PKT_STAT_5 register
#define DP83TG720_PKT_STAT_5_RX_PKT_CNT_31_16                          0xFFFF

//PKT_STAT_6 register
#define DP83TG720_PKT_STAT_6_RX_ERR_PKT_CNT                            0xFFFF

//SQI_REG_1 register
#define DP83TG720_SQI_REG_1_WORST_SQI_OUT                              0x00E0
#define DP83TG720_SQI_REG_1_SQI_OUT                                    0x000E

//DSP_REG_75 register
#define DP83TG720_DSP_REG_75_MSE_LOCK                                  0x03FF

//SQI_1 register
#define DP83TG720_SQI_1_CFG_HIST_1_2                                   0xF000
#define DP83TG720_SQI_1_CFG_ACC_WINDOW_SEL                             0x0C00
#define DP83TG720_SQI_1_CFG_SQI_TH_1_2                                 0x03FF

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//DP83TG720 Ethernet PHY driver
extern const PhyDriver dp83tg720PhyDriver;

//DP83TG720 related functions
error_t dp83tg720Init(NetInterface *interface);
void dp83tg720InitHook(NetInterface *interface);

void dp83tg720Tick(NetInterface *interface);

void dp83tg720EnableIrq(NetInterface *interface);
void dp83tg720DisableIrq(NetInterface *interface);

void dp83tg720EventHandler(NetInterface *interface);

void dp83tg720WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t dp83tg720ReadPhyReg(NetInterface *interface, uint8_t address);

void dp83tg720DumpPhyReg(NetInterface *interface);

void dp83tg720WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t dp83tg720ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
