/**
 * @file tja1103_driver.h
 * @brief TJA1103 100Base-T1 Ethernet PHY driver
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2026 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.6.0
 **/

#ifndef _TJA1103_DRIVER_H
#define _TJA1103_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef TJA1103_PHY_ADDR
   #define TJA1103_PHY_ADDR 0
#elif (TJA1103_PHY_ADDR < 0 || TJA1103_PHY_ADDR > 31)
   #error TJA1103_PHY_ADDR parameter is not valid
#endif

//TJA1103 PHY registers
#define TJA1103_PHY_ID_1                                       0x02
#define TJA1103_PHY_ID_2                                       0x03
#define TJA1103_CL45_ACCESS_CONTROL                            0x0D
#define TJA1103_CL45_ADDRESS_DATA                              0x0E
#define TJA1103_CL45_ADDRESS                                   0x1E
#define TJA1103_ALWAYS_ACCESSIBLE                              0x1F

//TJA1103 MMD registers
#define TJA1103_BASE_T1_PMA_CONTROL                            0x01, 0x0834
#define TJA1103_PCS_CONTROL1                                   0x03, 0x0000
#define TJA1103_DEVICE_IDENTIFIER3                             0x1E, 0x0004
#define TJA1103_DEVICE_CONTROL                                 0x1E, 0x0040
#define TJA1103_DEVICE_CONFIG                                  0x1E, 0x0048
#define TJA1103_PTP_CONFIG                                     0x1E, 0x1102
#define TJA1103_RX_TS_INSRT_CTRL                               0x1E, 0x114D
#define TJA1103_EGR_RING_DATA_0                                0x1E, 0x114E
#define TJA1103_EGR_RING_CTRL                                  0x1E, 0x1154
#define TJA1103_GPIO0_FUNC_CONFIG                              0x1E, 0x2C40
#define TJA1103_PORT_PTP_CONTROL                               0x1E, 0x9000
#define TJA1103_PORT_INFRA_CONTROL                             0x1E, 0xAC00
#define TJA1103_XMII_ABILITIES                                 0x1E, 0xAFC4
#define TJA1103_MII_BASIC_CONFIG                               0x1E, 0xAFC6
#define TJA1103_RGMII_TXC_DELAY_CONFIG                         0x1E, 0xAFCC
#define TJA1103_RGMII_RXC_DELAY_CONFIG                         0x1E, 0xAFCD
#define TJA1103_RX_PREAMBLE_COUNT                              0x1E, 0xAFCE
#define TJA1103_TX_PREAMBLE_COUNT                              0x1E, 0xAFCF
#define TJA1103_RX_IPG_LENGTH                                  0x1E, 0xAFD0
#define TJA1103_TX_IPG_LENGTH                                  0x1E, 0xAFD1
#define TJA1103_SGMII_BASIC_CONTROL                            0x1E, 0xB000

//TJA1103 Shared MMD registers
#define TJA1103_PORT_CONTROL                                   0x1E, 0x8040
#define TJA1103_PORT_ABILITIES                                 0x1E, 0x8046
#define TJA1103_PORT_FUNC_IRQ_ENABLE                           0x1E, 0x807A
#define TJA1103_EPHY_FUNC_IRQ_SOURCE                           0x1E, 0x80A0
#define TJA1103_EPHY_FUNC_IRQ_ENABLE                           0x1E, 0x80A1
#define TJA1103_EPHY_FUNC_IRQ_MSTATUS                          0x1E, 0x80A2
#define TJA1103_PHY_CONTROL                                    0x1E, 0x8100
#define TJA1103_PHY_STATUS                                     0x1E, 0x8102
#define TJA1103_PHY_CONFIG                                     0x1E, 0x8108
#define TJA1103_SIGNAL_QUALITY                                 0x1E, 0x8320
#define TJA1103_CABLE_TEST                                     0x1E, 0x8330
#define TJA1103_SYMBOL_ERROR_COUNTER                           0x1E, 0x8350
#define TJA1103_ERROR_COUNTER_MISC                             0x1E, 0x8352
#define TJA1103_LINK_LOSSES_AND_FAILURES                       0x1E, 0x8353

//PHY Identification 1 register
#define TJA1103_PHY_ID_1_OUI_BITS_3_TO_18                      0xFFFF
#define TJA1103_PHY_ID_1_OUI_BITS_3_TO_18_DEFAULT              0x001B

//PHY Identification 2 register
#define TJA1103_PHY_ID_2_OUI_BITS_19_TO_24                     0xFC00
#define TJA1103_PHY_ID_2_OUI_BITS_19_TO_24_DEFAULT             0xB000
#define TJA1103_PHY_ID_2_MODEL                                 0x03F0
#define TJA1103_PHY_ID_2_MODEL_DEFAULT                         0x0010
#define TJA1103_PHY_ID_2_REVISION                              0x000F

//CL45 Access Control register
#define TJA1103_CL45_ACCESS_CONTROL_OP                         0xC000
#define TJA1103_CL45_ACCESS_CONTROL_OP_ADDR                    0x0000
#define TJA1103_CL45_ACCESS_CONTROL_OP_DATA_NO_POST_INC        0x4000
#define TJA1103_CL45_ACCESS_CONTROL_OP_DATA_POST_INC_RW        0x8000
#define TJA1103_CL45_ACCESS_CONTROL_OP_DATA_POST_INC_W         0xC000
#define TJA1103_CL45_ACCESS_CONTROL_MMD                        0x001F
#define TJA1103_CL45_ACCESS_CONTROL_MMD_PMA_PMD                0x0001
#define TJA1103_CL45_ACCESS_CONTROL_MMD_PCS                    0x0003
#define TJA1103_CL45_ACCESS_CONTROL_MMD_VENDOR_SPECIFIC        0x001E

//CL45 Address Data register
#define TJA1103_CL45_ADDRESS_DATA_ADDRESS_DATA                 0xFFFF

//CL45 Address register
#define TJA1103_CL45_ADDRESS_ADDRESS                           0xFFFF

//Always-Accessible register
#define TJA1103_ALWAYS_ACCESSIBLE_FUSA_PASS_IRQ                0x0010

//BASE-T1 PMA Control register
#define TJA1103_BASE_T1_PMA_CONTROL_MANUAL_MASTER_SLAVE_CONFIG 0x8000
#define TJA1103_BASE_T1_PMA_CONTROL_MASTER_SLAVE               0x4000

//PCS Control 1 register
#define TJA1103_PCS_CONTROL1_RESET                             0x8000
#define TJA1103_PCS_CONTROL1_LOOPBACK                          0x4000
#define TJA1103_PCS_CONTROL1_SPEED_SELECT_LSB                  0x2000
#define TJA1103_PCS_CONTROL1_LOW_POWER                         0x0800
#define TJA1103_PCS_CONTROL1_SPEED_SELECT_MSB                  0x0040

//Device Identifier 3 register
#define TJA1103_DEVICE_IDENTIFIER3_TJA1103A                    0x1091
#define TJA1103_DEVICE_IDENTIFIER3_TJA1103B                    0x2401

//Device Control register
#define TJA1103_DEVICE_CONTROL_DEVICE_RESET                    0x8000
#define TJA1103_DEVICE_CONTROL_GLOBAL_CONFIG_ENABLE            0x4000
#define TJA1103_DEVICE_CONTROL_SUPER_CONFIG_ENABLE             0x2000

//PTP Configuration register
#define TJA1103_PTP_CONFIG_PPS_OUT_EN                          0x0008
#define TJA1103_PTP_CONFIG_PPS_OUT_POL                         0x0004
#define TJA1103_PTP_CONFIG_EXT_TRG_EDGE_SEL                    0x0002

//GPIO0 Function Configuration register
#define TJA1103_GPIO0_FUNC_CONFIG_ENABLE                       0x8000
#define TJA1103_GPIO0_FUNC_CONFIG_SIGNAL_SELECT                0x001F
#define TJA1103_GPIO0_FUNC_CONFIG_SIGNAL_SELECT_PTP_TRIGGER    0x0001
#define TJA1103_GPIO0_FUNC_CONFIG_SIGNAL_SELECT_PPS_OUT        0x0012

//Port PTP Control register
#define TJA1103_PORT_PTP_CONTROL_BYPASS                        0x0800

//Port Infrastructure Control register
#define TJA1103_PORT_INFRA_CONTROL_RESET                       0x8000
#define TJA1103_PORT_INFRA_CONTROL_CONFIG_ENABLE               0x4000

//XMII Abilities register
#define TJA1103_XMII_ABILITIES_RGMII_ID_ABILITY                0x8000
#define TJA1103_XMII_ABILITIES_RGMII_ABILITY                   0x4000
#define TJA1103_XMII_ABILITIES_RMII_MASTER_ABILITY             0x0800
#define TJA1103_XMII_ABILITIES_RMII_SLAVE_ABILITY              0x0400
#define TJA1103_XMII_ABILITIES_MII_SLAVE_ABILITY               0x0200
#define TJA1103_XMII_ABILITIES_MII_MASTER_ABILITY              0x0100
#define TJA1103_XMII_ABILITIES_SGMII_ABILITY                   0x0001

//MII Basic Configuration register
#define TJA1103_MII_BASIC_CONFIG_ROLE                          0x0010
#define TJA1103_MII_BASIC_CONFIG_XMII_MODE                     0x000F
#define TJA1103_MII_BASIC_CONFIG_XMII_MODE_MII                 0x0004
#define TJA1103_MII_BASIC_CONFIG_XMII_MODE_RMII                0x0005
#define TJA1103_MII_BASIC_CONFIG_XMII_MODE_RGMII               0x0007
#define TJA1103_MII_BASIC_CONFIG_XMII_MODE_SGMII               0x0008

//RGMII TXC Delay Configuration register
#define TJA1103_RGMII_TXC_DELAY_CONFIG_ENABLE                  0x8000
#define TJA1103_RGMII_TXC_DELAY_CONFIG_PHASE_SHIFT             0x001F
#define TJA1103_RGMII_TXC_DELAY_CONFIG_PHASE_SHIFT_DEFAULT     0x0012

//RGMII RXC Delay Configuration register
#define TJA1103_RGMII_RXC_DELAY_CONFIG_ENABLE                  0x8000
#define TJA1103_RGMII_RXC_DELAY_CONFIG_PHASE_SHIFT             0x001F
#define TJA1103_RGMII_RXC_DELAY_CONFIG_PHASE_SHIFT_DEFAULT     0x0012

//RX Preamble Counter register
#define TJA1103_RX_PREAMBLE_COUNT_ENABLE                       0x8000
#define TJA1103_RX_PREAMBLE_COUNT_COUNT                        0x003F

//TX Preamble Counter register
#define TJA1103_TX_PREAMBLE_COUNT_ENABLE                       0x8000
#define TJA1103_TX_PREAMBLE_COUNT_COUNT                        0x003F

//RX IPG Length Capture register
#define TJA1103_RX_IPG_LENGTH_ENABLE                           0x8000
#define TJA1103_RX_IPG_LENGTH_LENGTH                           0x01FF

//TX IPG Length Capture register
#define TJA1103_TX_IPG_LENGTH_ENABLE                           0x8000
#define TJA1103_TX_IPG_LENGTH_LENGTH                           0x01FF

//SGMII Basic Control register
#define TJA1103_SGMII_BASIC_CONTROL_LPM                        0x0800

//Port Control register
#define TJA1103_PORT_CONTROL_CONFIG_ENABLE                     0x4000

//Port Abilities register
#define TJA1103_PORT_ABILITIES_PTP_ABILITY                     0x0008

//Port-Level IRQ Enable register
#define TJA1103_PORT_FUNC_IRQ_ENABLE_PTP_IRQ                   0x0008

//Ethernet PHY Functional IRQ Source register
#define TJA1103_EPHY_FUNC_IRQ_SOURCE_LINK_AVAILABLE_EVENT      0x0004
#define TJA1103_EPHY_FUNC_IRQ_SOURCE_LINK_STATUS_EVENT         0x0002

//Ethernet PHY Functional IRQ Enable register
#define TJA1103_EPHY_FUNC_IRQ_ENABLE_LINK_AVAILABLE_EVENT      0x0004
#define TJA1103_EPHY_FUNC_IRQ_ENABLE_LINK_STATUS_EVENT         0x0002

//Ethernet PHY Functional IRQ Masked Status register
#define TJA1103_EPHY_FUNC_IRQ_MSTATUS_LINK_AVAILABLE_EVENT     0x0004
#define TJA1103_EPHY_FUNC_IRQ_MSTATUS_LINK_STATUS_EVENT        0x0002

//PHY Control register
#define TJA1103_PHY_CONTROL_CONFIG_ENABLE                      0x4000
#define TJA1103_PHY_CONTROL_START_OPERATION                    0x0001

//PHY Status register
#define TJA1103_PHY_STATUS_LINK_STATUS                         0x0004

//PHY Configuration register
#define TJA1103_PHY_CONFIG_AUTO_OPERATION                      0x0001

//Signal Quality register
#define TJA1103_SIGNAL_QUALITY_VALID                           0x4000
#define TJA1103_SIGNAL_QUALITY_SQI                             0x0007

//Cable Test register
#define TJA1103_CABLE_TEST_ENABLE                              0x8000
#define TJA1103_CABLE_TEST_START                               0x4000
#define TJA1103_CABLE_TEST_VALID                               0x2000
#define TJA1103_CABLE_TEST_FAULT_TYPE                          0x0007
#define TJA1103_CABLE_TEST_FAULT_TYPE_OK                       0x0000
#define TJA1103_CABLE_TEST_FAULT_TYPE_SHORTED                  0x0001
#define TJA1103_CABLE_TEST_FAULT_TYPE_OPEN                     0x0002
#define TJA1103_CABLE_TEST_FAULT_TYPE_UNKNOWN                  0x0007

//Symbol Error Counter register
#define TJA1103_SYMBOL_ERROR_COUNTER_SYMBOL_ERRORS             0xFFFF

//Error Counter Miscellaneous register
#define TJA1103_ERROR_COUNTER_MISC_COUNTER_ENABLE              0x8000
#define TJA1103_ERROR_COUNTER_MISC_LINK_STATUS_DROPS           0x3F00
#define TJA1103_ERROR_COUNTER_MISC_LINK_AVAILABLE_DROPS        0x003F

//Link Losses And Failures Counter register
#define TJA1103_LINK_LOSSES_AND_FAILURES_LINK_LOSSES           0xFC00
#define TJA1103_LINK_LOSSES_AND_FAILURES_LINK_FAILURES         0x03FF

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//TJA1103 Ethernet PHY driver
extern const PhyDriver tja1103PhyDriver;

//TJA1103 related functions
error_t tja1103Init(NetInterface *interface);
void tja1103InitHook(NetInterface *interface);

void tja1103Tick(NetInterface *interface);

void tja1103EnableIrq(NetInterface *interface);
void tja1103DisableIrq(NetInterface *interface);

void tja1103EventHandler(NetInterface *interface);

void tja1103WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t tja1103ReadPhyReg(NetInterface *interface, uint8_t address);

void tja1103DumpPhyReg(NetInterface *interface);

void tja1103WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t tja1103ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
