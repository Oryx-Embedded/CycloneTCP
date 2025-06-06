/**
 * @file ncn26000_driver.h
 * @brief NCN26000 10Base-T1S Ethernet PHY driver
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

#ifndef _NCN26000_DRIVER_H
#define _NCN26000_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef NCN26000_PHY_ADDR
   #define NCN26000_PHY_ADDR 0
#elif (NCN26000_PHY_ADDR < 0 || NCN26000_PHY_ADDR > 31)
   #error NCN26000_PHY_ADDR parameter is not valid
#endif

//PLCA support
#ifndef NCN26000_PLCA_SUPPORT
   #define NCN26000_PLCA_SUPPORT ENABLED
#elif (NCN26000_PLCA_SUPPORT != ENABLED && NCN26000_PLCA_SUPPORT != DISABLED)
   #error NCN26000_PLCA_SUPPORT parameter is not valid
#endif

//Node count
#ifndef NCN26000_NODE_COUNT
   #define NCN26000_NODE_COUNT 8
#elif (NCN26000_NODE_COUNT < 0 || NCN26000_NODE_COUNT > 255)
   #error NCN26000_NODE_COUNT parameter is not valid
#endif

//Local ID
#ifndef NCN26000_LOCAL_ID
   #define NCN26000_LOCAL_ID 1
#elif (NCN26000_LOCAL_ID < 0 || NCN26000_LOCAL_ID > 255)
   #error NCN26000_LOCAL_ID parameter is not valid
#endif

//NCN26000 PHY registers
#define NCN26000_CTRL                                          0x00
#define NCN26000_STATUS                                        0x01
#define NCN26000_PHYID1                                        0x02
#define NCN26000_PHYID2                                        0x03
#define NCN26000_MACR                                          0x0D
#define NCN26000_MAADR                                         0x0E
#define NCN26000_MIIM_IRQ_CTRL                                 0x10
#define NCN26000_MIIM_IRQ_STATUS                               0x11
#define NCN26000_DIO_CONFIG                                    0x12

//NCN26000 MMD registers
#define NCN26000_MMD1_DEV_IN_PACKAGE1                          0x01, 0x0005
#define NCN26000_MMD1_DEV_IN_PACKAGE2                          0x01, 0x0006
#define NCN26000_BASE_T1_EXTENDED_ABILITY                      0x01, 0x0012
#define NCN26000_10BASE_T1S_PMA_CTRL                           0x01, 0x08F9
#define NCN26000_10BASE_T1S_PMA_STATUS                         0x01, 0x08FA
#define NCN26000_10BASE_T1S_TEST_MODE                          0x01, 0x08FB
#define NCN26000_MMD3_DEV_IN_PACKAGE1                          0x03, 0x0005
#define NCN26000_MMD3_DEV_IN_PACKAGE2                          0x03, 0x0006
#define NCN26000_10BASE_T1S_PCS_CTRL                           0x03, 0x08F3
#define NCN26000_10BASE_T1S_PCS_STATUS                         0x03, 0x08F4
#define NCN26000_10BASE_T1S_PCS_DIAG1                          0x03, 0x08F5
#define NCN26000_10BASE_T1S_PCS_DIAG2                          0x03, 0x08F6
#define NCN26000_CHIP_REVISION                                 0x1E, 0x1000
#define NCN26000_PHY_TWEAKS                                    0x1E, 0x1001
#define NCN26000_CHIP_INFO                                     0x1E, 0x1004
#define NCN26000_NVM_HEALTH                                    0x1E, 0x1005
#define NCN26000_PHY_REVISION                                  0x1F, 0x8000
#define NCN26000_PHY_CONFIG1                                   0x1F, 0x8001
#define NCN26000_PLCA_EXTENSIONS                               0x1F, 0x8002
#define NCN26000_PMA_TUNE0                                     0x1F, 0x8003
#define NCN26000_PMA_TUNE1                                     0x1F, 0x8004
#define NCN26000_PLCIDVER                                      0x1F, 0xCA00
#define NCN26000_PLCA_CTRL0                                    0x1F, 0xCA01
#define NCN26000_PLCA_CTRL1                                    0x1F, 0xCA02
#define NCN26000_PLCA_STATUS                                   0x1F, 0xCA03
#define NCN26000_PLCA_TX_OPPORTUNITY_TMR                       0x1F, 0xCA04
#define NCN26000_PLCA_BURST_MODE                               0x1F, 0xCA05

//Control register
#define NCN26000_CTRL_RESET                                    0x8000
#define NCN26000_CTRL_LOOPBACK                                 0x4000
#define NCN26000_CTRL_SPEED_LSB                                0x2000
#define NCN26000_CTRL_LINK_CTRL                                0x1000
#define NCN26000_CTRL_LOW_POWER                                0x0800
#define NCN26000_CTRL_ISOLATE                                  0x0400
#define NCN26000_CTRL_LINK_RESET                               0x0200
#define NCN26000_CTRL_DUPLEX_MODE                              0x0100
#define NCN26000_CTRL_COL_TEST                                 0x0080
#define NCN26000_CTRL_SPEED_MSB                                0x0040
#define NCN26000_CTRL_UNIDIRECTIONAL_EN                        0x0020

//Status register
#define NCN26000_STATUS_10MBPS_HD                              0x0800
#define NCN26000_STATUS_UNIDIRECTIONAL_ABLE                    0x0080
#define NCN26000_STATUS_MF_PREAMBLE_SUPPR                      0x0040
#define NCN26000_STATUS_LINK_NEGOTIATION_COMPLETE              0x0020
#define NCN26000_STATUS_REMOTE_FAULT                           0x0010
#define NCN26000_STATUS_AN_ABLE                                0x0008
#define NCN26000_STATUS_LINK_STATUS                            0x0004
#define NCN26000_STATUS_JABBER_DETECT                          0x0002
#define NCN26000_STATUS_EXTENDED_CAPABLE                       0x0001

//PHY Identifier 1 register
#define NCN26000_PHYID1_OUI_MSB                                0xFFFF
#define NCN26000_PHYID1_OUI_MSB_DEFAULT                        0x180F

//PHY Identifier 2 register
#define NCN26000_PHYID2_OUI_LSB                                0xFC00
#define NCN26000_PHYID2_OUI_LSB_DEFAULT                        0xF400
#define NCN26000_PHYID2_IC_MODEL_NUM                           0x03F0
#define NCN26000_PHYID2_IC_MODEL_NUM_DEFAULT                   0x01A0
#define NCN26000_PHYID2_CHIP_REV_NUM                           0x000F
#define NCN26000_PHYID2_CHIP_REV_NUM_DEFAULT                   0x0001

//MMD Access Control register
#define NCN26000_MACR_FUNC                                     0xC000
#define NCN26000_MACR_FUNC_ADDR                                0x0000
#define NCN26000_MACR_FUNC_DATA_NO_POST_INC                    0x4000
#define NCN26000_MACR_FUNC_DATA_POST_INC_RW                    0x8000
#define NCN26000_MACR_FUNC_DATA_POST_INC_W                     0xC000
#define NCN26000_MACR_DEVADD                                   0x001F

//MIIM IRQ Control register
#define NCN26000_MIIM_IRQ_CTRL_PHY_COL                         0x0020
#define NCN26000_MIIM_IRQ_CTRL_PLCA_RECOVERY                   0x0010
#define NCN26000_MIIM_IRQ_CTRL_REMOTE_JABBER                   0x0008
#define NCN26000_MIIM_IRQ_CTRL_LOCAL_JABBER                    0x0004
#define NCN26000_MIIM_IRQ_CTRL_PLCA_STATUS_CHANGE              0x0002
#define NCN26000_MIIM_IRQ_CTRL_LINK_STATUS                     0x0001

//MIIM IRQ Status register
#define NCN26000_MIIM_IRQ_STATUS_RESET                         0x8000
#define NCN26000_MIIM_IRQ_STATUS_PHY_COL                       0x0020
#define NCN26000_MIIM_IRQ_STATUS_PLCA_RECOVERY                 0x0010
#define NCN26000_MIIM_IRQ_STATUS_REMOTE_JABBER                 0x0008
#define NCN26000_MIIM_IRQ_STATUS_LOCAL_JABBER                  0x0004
#define NCN26000_MIIM_IRQ_STATUS_PLCA_STATUS_CHANGE            0x0002
#define NCN26000_MIIM_IRQ_STATUS_LINK_STATUS                   0x0001

//DIO Configuration register
#define NCN26000_DIO_CONFIG_SLEW_RATE1                         0x8000
#define NCN26000_DIO_CONFIG_SLEW_RATE1_FAST                    0x0000
#define NCN26000_DIO_CONFIG_SLEW_RATE1_SLOW                    0x8000
#define NCN26000_DIO_CONFIG_PULL_EN1                           0x4000
#define NCN26000_DIO_CONFIG_PULL_RES_TYPE1                     0x2000
#define NCN26000_DIO_CONFIG_PULL_RES_TYPE1_PU                  0x0000
#define NCN26000_DIO_CONFIG_PULL_RES_TYPE1_PD                  0x2000
#define NCN26000_DIO_CONFIG_FN1                                0x1E00
#define NCN26000_DIO_CONFIG_VAL1                               0x0100
#define NCN26000_DIO_CONFIG_VAL1_DISABLE                       0x0000
#define NCN26000_DIO_CONFIG_VAL1_GPIO                          0x0100
#define NCN26000_DIO_CONFIG_VAL1_SFD_TX                        0x0200
#define NCN26000_DIO_CONFIG_VAL1_SFD_RX                        0x0300
#define NCN26000_DIO_CONFIG_VAL1_LED_LINK_CTRL                 0x0400
#define NCN26000_DIO_CONFIG_VAL1_LED_PLCA_STATUS               0x0500
#define NCN26000_DIO_CONFIG_VAL1_LED_TX                        0x0600
#define NCN26000_DIO_CONFIG_VAL1_LED_RX                        0x0700
#define NCN26000_DIO_CONFIG_VAL1_CLK25M                        0x0800
#define NCN26000_DIO_CONFIG_VAL1_SFD_RX_TX                     0x0B00
#define NCN26000_DIO_CONFIG_VAL1_LED_TX_RX                     0x0F00
#define NCN26000_DIO_CONFIG_SLEW_RATE0                         0x0080
#define NCN26000_DIO_CONFIG_PULL_EN0                           0x0040
#define NCN26000_DIO_CONFIG_PULL_RES_TYPE0                     0x0020
#define NCN26000_DIO_CONFIG_FN0                                0x001E
#define NCN26000_DIO_CONFIG_VAL0                               0x0001
#define NCN26000_DIO_CONFIG_VAL0_DISABLE                       0x0000
#define NCN26000_DIO_CONFIG_VAL0_GPIO                          0x0001
#define NCN26000_DIO_CONFIG_VAL0_SFD_TX                        0x0002
#define NCN26000_DIO_CONFIG_VAL0_SFD_RX                        0x0003
#define NCN26000_DIO_CONFIG_VAL0_LED_LINK_CTRL                 0x0004
#define NCN26000_DIO_CONFIG_VAL0_LED_PLCA_STATUS               0x0005
#define NCN26000_DIO_CONFIG_VAL0_LED_TX                        0x0006
#define NCN26000_DIO_CONFIG_VAL0_LED_RX                        0x0007
#define NCN26000_DIO_CONFIG_VAL0_CLK25M                        0x0008
#define NCN26000_DIO_CONFIG_VAL0_SFD_RX_TX                     0x000B
#define NCN26000_DIO_CONFIG_VAL0_LED_TX_RX                     0x000F

//Devices in Package 1 register
#define NCN26000_MMD1_DEV_IN_PACKAGE1_PCS_PRESENT              0x0008
#define NCN26000_MMD1_DEV_IN_PACKAGE1_PMA_PRESENT              0x0002
#define NCN26000_MMD1_DEV_IN_PACKAGE1_C22_REGS_PRESENT         0x0001

//BASE−T1 Extended Ability register
#define NCN26000_BASE_T1_EXTENDED_ABILITY_10BASE_T1S           0x0008

//10BASE−T1S PMA Control register
#define NCN26000_10BASE_T1S_PMA_CTRL_PMA_RESET                 0x8000
#define NCN26000_10BASE_T1S_PMA_CTRL_TRANSMIT_DIS              0x4000
#define NCN26000_10BASE_T1S_PMA_CTRL_LOW_POWER_MODE            0x0800
#define NCN26000_10BASE_T1S_PMA_CTRL_MULTI_DROP_EN             0x0400
#define NCN26000_10BASE_T1S_PMA_CTRL_LOOPBACK_MODE             0x0001

//10BASE−T1S PMA Status register
#define NCN26000_10BASE_T1S_PMA_STATUS_LOOPBACK_ABILITY        0x2000
#define NCN26000_10BASE_T1S_PMA_STATUS_LOW_POWER_ABILITY       0x0800
#define NCN26000_10BASE_T1S_PMA_STATUS_MULTI_DROP_ABILITY      0x0400
#define NCN26000_10BASE_T1S_PMA_STATUS_RECEIVE_FAULT_ABILITY   0x0200
#define NCN26000_10BASE_T1S_PMA_STATUS_REMOTE_JABBER           0x0002

//10BASE−T1S Test Mode register
#define NCN26000_10BASE_T1S_TEST_MODE_TEST_MODE                0xE000
#define NCN26000_10BASE_T1S_TEST_MODE_TEST_MODE_NORMAL         0x0000
#define NCN26000_10BASE_T1S_TEST_MODE_TEST_MODE_TX_OUT_VOLTAGE 0x2000
#define NCN26000_10BASE_T1S_TEST_MODE_TEST_MODE_TX_OUT_DROOP   0x4000
#define NCN26000_10BASE_T1S_TEST_MODE_TEST_MODE_TX_PSD_MASK    0x6000
#define NCN26000_10BASE_T1S_TEST_MODE_TEST_MODE_TX_HI_Z        0x8000

//Devices in Package 1 register
#define NCN26000_MMD3_DEV_IN_PACKAGE1_PCS_PRESENT              0x0008
#define NCN26000_MMD3_DEV_IN_PACKAGE1_PMA_PRESENT              0x0002
#define NCN26000_MMD3_DEV_IN_PACKAGE1_C22_REGS_PRESENT         0x0001

//10BASE−T1S PCS Control register
#define NCN26000_10BASE_T1S_PCS_CTRL_PCS_RESET                 0x8000
#define NCN26000_10BASE_T1S_PCS_CTRL_LOOPBACK                  0x4000

//10BASE−T1S PCS Status register
#define NCN26000_10BASE_T1S_PCS_STATUS_FAULT                   0x0080

//10BASE−T1S PCS Diagnostic 1 register
#define NCN26000_10BASE_T1S_PCS_DIAG1_PCS_REMOTE_JABBER_COUNT  0xFFFF

//10BASE−T1S PCS Diagnostic 2 register
#define NCN26000_10BASE_T1S_PCS_DIAG2_PCS_PHY_COL_COUNT        0xFFFF

//Chip Revision register
#define NCN26000_CHIP_REVISION_MAJOR_REV                       0xF000
#define NCN26000_CHIP_REVISION_MINOR_REV                       0x0F00
#define NCN26000_CHIP_REVISION_STAGE                           0x00C0
#define NCN26000_CHIP_REVISION_PATCH                           0x003F

//PHY Tweaks register
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD                       0x3C00
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_150MVPP               0x0000
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_200MVPP               0x0400
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_250MVPP               0x0800
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_300MVPP               0x0C00
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_350MVPP               0x1000
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_400MVPP               0x1400
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_450MVPP               0x1800
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_500MVPP               0x1C00
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_550MVPP               0x2000
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_600MVPP               0x2400
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_650MVPP               0x2800
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_700MVPP               0x2C00
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_750MVPP               0x3000
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_800MVPP               0x3400
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_850MVPP               0x3800
#define NCN26000_PHY_TWEAKS_CD_THRESHOLD_900MVPP               0x3C00
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD                    0x03C0
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_150MVPP            0x0000
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_200MVPP            0x0040
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_250MVPP            0x0080
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_300MVPP            0x00C0
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_350MVPP            0x0100
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_400MVPP            0x0140
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_450MVPP            0x0180
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_500MVPP            0x01C0
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_550MVPP            0x0200
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_600MVPP            0x0240
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_650MVPP            0x0280
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_700MVPP            0x02C0
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_750MVPP            0x0300
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_800MVPP            0x0340
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_850MVPP            0x0380
#define NCN26000_PHY_TWEAKS_RX_ED_THRESHOLD_900MVPP            0x03C0
#define NCN26000_PHY_TWEAKS_DIGITAL_SLEW_RATE                  0x0020
#define NCN26000_PHY_TWEAKS_DIGITAL_SLEW_RATE_SLOW             0x0000
#define NCN26000_PHY_TWEAKS_DIGITAL_SLEW_RATE_FAST             0x0020
#define NCN26000_PHY_TWEAKS_CMC_COMP                           0x0018
#define NCN26000_PHY_TWEAKS_CMC_COMP_0R_0_5R                   0x0000
#define NCN26000_PHY_TWEAKS_CMC_COMP_0_5R_2_25R                0x0008
#define NCN26000_PHY_TWEAKS_CMC_COMP_2_25R_3_75R               0x0010
#define NCN26000_PHY_TWEAKS_CMC_COMP_3_75R_5R                  0x0018
#define NCN26000_PHY_TWEAKS_TX_SLEW                            0x0004
#define NCN26000_PHY_TWEAKS_TX_SLEW_SLOW                       0x0000
#define NCN26000_PHY_TWEAKS_TX_SLEW_FAST                       0x0004

//Chip Info register
#define NCN26000_CHIP_INFO_WAFER_Y                             0x7F00
#define NCN26000_CHIP_INFO_WAFER_X                             0x007F

//NVM Health register
#define NCN26000_NVM_HEALTH_RED_ZONE_NVM_WARNING               0x8000
#define NCN26000_NVM_HEALTH_RED_ZONE_NVM_ERROR                 0x4000
#define NCN26000_NVM_HEALTH_YELLOW_ZONE_NVM_WARNING            0x2000
#define NCN26000_NVM_HEALTH_YELLOW_ZONE_NVM_ERROR              0x1000
#define NCN26000_NVM_HEALTH_GREEN_ZONE_NVM_WARNING             0x0800
#define NCN26000_NVM_HEALTH_GREEN_ZONE_NVM_ERROR               0x0400

//PHY Revision register
#define NCN26000_PHY_REVISION_MAJOR_REV                        0xF000
#define NCN26000_PHY_REVISION_MINOR_REV                        0x0F00
#define NCN26000_PHY_REVISION_STAGE                            0x00C0
#define NCN26000_PHY_REVISION_PATCH                            0x003F

//PHY Configuration 1 register
#define NCN26000_PHY_CONFIG1_PKT_LOOP                          0x8000
#define NCN26000_PHY_CONFIG1_ENHANCED_NOISE_IMMUNITY           0x0080
#define NCN26000_PHY_CONFIG1_UNJAB_TMR_EN                      0x0040
#define NCN26000_PHY_CONFIG1_SCRAMBLER_DIS                     0x0004
#define NCN26000_PHY_CONFIG1_NO_COL_MASKING                    0x0002
#define NCN26000_PHY_CONFIG1_RX_DELAYED                        0x0001

//PLCA Extensions register
#define NCN26000_PLCA_EXTENSIONS_PLCA_PRECEDENCE               0x8000
#define NCN26000_PLCA_EXTENSIONS_MII_EXTENSIONS_DIS            0x0800
#define NCN26000_PLCA_EXTENSIONS_COORDINATOR_MODE              0x0002
#define NCN26000_PLCA_EXTENSIONS_COORDINATOR_ROLE              0x0001

//PMA Tune 0 register
#define NCN26000_PMA_TUNE0_PLCA_BEACON_DETECT_THRESH           0x3F00
#define NCN26000_PMA_TUNE0_PLCA_BEACON_DETECT_THRESH_DEFAULT   0x2000
#define NCN26000_PMA_TUNE0_DRIFT_COMP_WINDOW_SELECT            0x0007
#define NCN26000_PMA_TUNE0_DRIFT_COMP_WINDOW_SELECT_31BITS     0x0002
#define NCN26000_PMA_TUNE0_DRIFT_COMP_WINDOW_SELECT_63BITS     0x0003
#define NCN26000_PMA_TUNE0_DRIFT_COMP_WINDOW_SELECT_127BITS    0x0004
#define NCN26000_PMA_TUNE0_DRIFT_COMP_WINDOW_SELECT_OPTIMIZED  0x0005

//PMA Tune 1 register
#define NCN26000_PMA_TUNE1_PKT_PREAMBLE_DETECT_THRESH          0x3F00
#define NCN26000_PMA_TUNE1_PKT_PREAMBLE_DETECT_THRESH_DEFAULT  0x3500
#define NCN26000_PMA_TUNE1_COMMIT_DETECT_THRESH                0x003F
#define NCN26000_PMA_TUNE1_COMMIT_DETECT_THRESH_DEFAULT        0x0020

//PLCA Register Map and Identification (PLCIDVER) register
#define NCN26000_PLCIDVER_MAPID                                0xFF00
#define NCN26000_PLCIDVER_MAPVER                               0x00FF

//PLCA Control 0 register
#define NCN26000_PLCA_CTRL0_EN                                 0x8000
#define NCN26000_PLCA_CTRL0_RESET                              0x4000

//PLCA Control 1 register
#define NCN26000_PLCA_CTRL1_NCNT                               0xFF00
#define NCN26000_PLCA_CTRL1_ID                                 0x00FF

//PLCA Status register
#define NCN26000_PLCA_STATUS_PST                               0x8000

//PLCA Transmit Opportunity Timer register
#define NCN26000_PLCA_TX_OPPORTUNITY_TMR_TOTMR                 0x00FF

//PLCA Burst Mode register
#define NCN26000_PLCA_BURST_MODE_MAX_BURST_COUNT               0xFF00
#define NCN26000_PLCA_BURST_MODE_MAX_BURST_COUNT_DEFAULT       0x0000
#define NCN26000_PLCA_BURST_MODE_IFG_COMP_TMR                  0x00FF
#define NCN26000_PLCA_BURST_MODE_IFG_COMP_TMR_DEFAULT          0x0080

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//NCN26000 Ethernet PHY driver
extern const PhyDriver ncn26000PhyDriver;

//NCN26000 related functions
error_t ncn26000Init(NetInterface *interface);
void ncn26000InitHook(NetInterface *interface);

void ncn26000Tick(NetInterface *interface);

void ncn26000EnableIrq(NetInterface *interface);
void ncn26000DisableIrq(NetInterface *interface);

void ncn26000EventHandler(NetInterface *interface);

void ncn26000WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t ncn26000ReadPhyReg(NetInterface *interface, uint8_t address);

void ncn26000DumpPhyReg(NetInterface *interface);

void ncn26000WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t ncn26000ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

void ncn26000ModifyMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t mask, uint16_t data);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
