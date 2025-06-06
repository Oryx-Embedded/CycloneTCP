/**
 * @file tja1103_driver.h
 * @brief TJA1103 100Base-T1 Ethernet PHY driver
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
#define TJA1103_PHY_ID1                           0x02
#define TJA1103_PHY_ID2                           0x03
#define TJA1103_MMDCTRL                           0x0D
#define TJA1103_MMDAD                             0x0E

//TJA1103 MMD registers
#define TJA1103_B100T1_PMAPMD_CTL                 0x01, 0x0834
#define TJA1103_DEVICE_CTRL                       0x1E, 0x0040
#define TJA1103_DEVICE_CONFIG                     0x1E, 0x0048
#define TJA1103_ALWAYS_ACCESSIBLE                 0x1E, 0x801F
#define TJA1103_PORT_CTRL                         0x1E, 0x8040
#define TJA1103_PORT_ABILITIES                    0x1E, 0x8046
#define TJA1103_PORT_FUNC_IRQ_EN                  0x1E, 0x807A
#define TJA1103_PHY_IRQ_ACK                       0x1E, 0x80A0
#define TJA1103_PHY_IRQ_EN                        0x1E, 0x80A1
#define TJA1103_PHY_IRQ_STATUS                    0x1E, 0x80A2
#define TJA1103_PHY_CTRL                          0x1E, 0x8100
#define TJA1103_PHY_STAT                          0x1E, 0x8102
#define TJA1103_PHY_CONFIG                        0x1E, 0x8108
#define TJA1103_SIGNAL_QUALITY                    0x1E, 0x8320
#define TJA1103_CABLE_TEST                        0x1E, 0x8330
#define TJA1103_SYM_ERR_COUNTER                   0x1E, 0x8350
#define TJA1103_SYM_ERR_COUNTER_EXTENDED          0x1E, 0x8351
#define TJA1103_LINK_DROP_COUNTER                 0x1E, 0x8352
#define TJA1103_LINK_LOSSES_AND_FAILURES          0x1E, 0x8353
#define TJA1103_PTP_IRQ_ACK                       0x1E, 0x9008
#define TJA1103_RX_GOOD_FRAME_COUNTER             0x1E, 0xA950
#define TJA1103_RX_BAD_FRAME_COUNTER              0x1E, 0xA952
#define TJA1103_RX_ERR_FRAME_COUNTER              0x1E, 0xA954
#define TJA1103_INFRA_CTRL                        0x1E, 0xAC00
#define TJA1103_MONITOR_STATUS                    0x1E, 0xAC80
#define TJA1103_MONITOR_CONFIG                    0x1E, 0xAC86
#define TJA1103_ABILITIES                         0x1E, 0xAFC4
#define TJA1103_MII_BASIC_CONFIG                  0x1E, 0xAFC6
#define TJA1103_RXID                              0x1E, 0xAFCC
#define TJA1103_TXID                              0x1E, 0xAFCD
#define TJA1103_RX_PREAMBLE_COUNTER               0x1E, 0xAFCE
#define TJA1103_TX_PREAMBLE_COUNTER               0x1E, 0xAFCF
#define TJA1103_RX_IPG_LENGTH                     0x1E, 0xAFD0
#define TJA1103_TX_IPG_LENGTH                     0x1E, 0xAFD1

//PHY identification 1 register
#define TJA1103_PHY_ID1_OUI_MSB                   0xFFFF
#define TJA1103_PHY_ID1_OUI_MSB_DEFAULT           0x001B

//PHY identification 2 register
#define TJA1103_PHY_ID2_OUI_LSB                   0xFC00
#define TJA1103_PHY_ID2_OUI_LSB_DEFAULT           0xB000
#define TJA1103_PHY_ID2_TYPE_NO                   0x03F0
#define TJA1103_PHY_ID2_TYPE_NO_DEFAULT           0x0010
#define TJA1103_PHY_ID2_REVISION_NO               0x000F

//MMDCTRL register
#define TJA1103_MMDCTRL_FNCTN                     0xC000
#define TJA1103_MMDCTRL_FNCTN_ADDR                0x0000
#define TJA1103_MMDCTRL_FNCTN_DATA_NO_POST_INC    0x4000
#define TJA1103_MMDCTRL_FNCTN_DATA_POST_INC_RW    0x8000
#define TJA1103_MMDCTRL_FNCTN_DATA_POST_INC_W     0xC000
#define TJA1103_MMDCTRL_DEVAD                     0x001F

//B100T1_PMAPMD_CTL register
#define TJA1103_B100T1_PMAPMD_CTL_CONFIG_EN       0x8000
#define TJA1103_B100T1_PMAPMD_CTL_MASTER          0x4000

//DEVICE_CTRL register
#define TJA1103_DEVICE_CTRL_RESET                 0x8000
#define TJA1103_DEVICE_CTRL_CONFIG_GLOBAL_EN      0x4000
#define TJA1103_DEVICE_CTRL_CONFIG_ALL_EN         0x2000

//ALWAYS_ACCESSIBLE register
#define TJA1103_ALWAYS_ACCESSIBLE_FUSA_PASS       0x0010

//PORT_CTRL register
#define TJA1103_PORT_CTRL_EN                      0x4000

//PORT_ABILITIES register
#define TJA1103_PORT_ABILITIES_PTP                0x0010

//PORT_FUNC_IRQ_EN register
#define TJA1103_PORT_FUNC_IRQ_EN_PTP_IRQS         0x0008

//PHY_IRQ_ACK register
#define TJA1103_PHY_IRQ_ACK_LINK_EVENT            0x0002

//PHY_IRQ_EN register
#define TJA1103_PHY_IRQ_EN_LINK_AVAIL             0x0004
#define TJA1103_PHY_IRQ_EN_LINK_EVENT             0x0002

//PHY_IRQ_STATUS register
#define TJA1103_PHY_IRQ_STATUS_LINK_AVAIL         0x0004
#define TJA1103_PHY_IRQ_STATUS_LINK_EVENT         0x0002

//PHY_CTRL register
#define TJA1103_PHY_CTRL_CONFIG_EN                0x4000
#define TJA1103_PHY_CTRL_START_OP                 0x0001

//PHY_CTRL register
#define TJA1103_PHY_STAT_LINK_STATUS              0x0004

//PHY_CONFIG register
#define TJA1103_PHY_CONFIG_AUTO                   0x0001

//SIGNAL_QUALITY register
#define TJA1103_SIGNAL_QUALITY_SQI_VALID          0x4000
#define TJA1103_SIGNAL_QUALITY_SQI_MASK           0x0007

//CABLE_TEST register
#define TJA1103_CABLE_TEST_EN                     0x8000
#define TJA1103_CABLE_TEST_START                  0x4000
#define TJA1103_CABLE_TEST_VALID                  0x2000
#define TJA1103_CABLE_TEST_RESULT                 0x0007
#define TJA1103_CABLE_TEST_RESULT_OK              0x0000
#define TJA1103_CABLE_TEST_RESULT_SHORTED         0x0001
#define TJA1103_CABLE_TEST_RESULT_OPEN            0x0002
#define TJA1103_CABLE_TEST_RESULT_UNKNOWN         0x0007

//INFRA_CTRL register
#define TJA1103_INFRA_CTRL_EN                     0x4000

//ABILITIES register
#define TJA1103_ABILITIES_RGMII_ID                0x8000
#define TJA1103_ABILITIES_RGMII                   0x4000
#define TJA1103_ABILITIES_RMII                    0x0400
#define TJA1103_ABILITIES_REVMII                  0x0200
#define TJA1103_ABILITIES_MII                     0x0100
#define TJA1103_ABILITIES_SGMII                   0x0001

//MII_BASIC_CONFIG register
#define TJA1103_MII_BASIC_CONFIG_REV              0x0100
#define TJA1103_MII_BASIC_CONFIG_MODE             0x000F
#define TJA1103_MII_BASIC_CONFIG_MODE_MII         0x0004
#define TJA1103_MII_BASIC_CONFIG_MODE_RMII        0x0005
#define TJA1103_MII_BASIC_CONFIG_MODE_RGMII       0x0007
#define TJA1103_MII_BASIC_CONFIG_MODE_SGMII       0x0009

//RXID register
#define TJA1103_RXID_EN                           0x8000

//TXID register
#define TJA1103_TXID_EN                           0x8000

//SYM_ERR_COUNTER register
#define TJA1103_SYM_ERR_COUNTER_EN                0x8000

//SYM_ERR_COUNTER_EXTENDED register
#define TJA1103_SYM_ERR_COUNTER_EXTENDED_EN       0x8000

//LINK_DROP_COUNTER register
#define TJA1103_LINK_DROP_COUNTER_EN              0x8000

//LINK_LOSSES_AND_FAILURES register
#define TJA1103_LINK_LOSSES_AND_FAILURES_EN       0x8000

//RX_GOOD_FRAME_COUNTER register
#define TJA1103_RX_GOOD_FRAME_COUNTER_EN          0x8000

//RX_BAD_FRAME_COUNTER register
#define TJA1103_RX_BAD_FRAME_COUNTER_EN           0x8000

//RX_ERR_FRAME_COUNTER register
#define TJA1103_RX_ERR_FRAME_COUNTER_EN           0x8000

//RX_PREAMBLE_COUNTER register
#define TJA1103_RX_PREAMBLE_COUNTER_EN            0x8000

//TX_PREAMBLE_COUNTER register
#define TJA1103_TX_PREAMBLE_COUNTER_EN            0x8000

//MONITOR_STATUS register
#define TJA1103_MONITOR_STATUS_RESET              0x8000

//MONITOR_CONFIG register
#define TJA1103_MONITOR_CONFIG_LOST_FRAMES_CNT_EN 0x0200
#define TJA1103_MONITOR_CONFIG_ALL_FRAMES_CNT_EN  0x0100

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
