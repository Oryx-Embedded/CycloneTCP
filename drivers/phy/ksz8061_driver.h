/**
 * @file ksz8061_driver.h
 * @brief KSZ8061 Ethernet PHY driver
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

#ifndef _KSZ8061_DRIVER_H
#define _KSZ8061_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef KSZ8061_PHY_ADDR
   #define KSZ8061_PHY_ADDR 1
#elif (KSZ8061_PHY_ADDR < 0 || KSZ8061_PHY_ADDR > 31)
   #error KSZ8061_PHY_ADDR parameter is not valid
#endif

//KSZ8061 PHY registers
#define KSZ8061_BMCR                              0x00
#define KSZ8061_BMSR                              0x01
#define KSZ8061_PHYID1                            0x02
#define KSZ8061_PHYID2                            0x03
#define KSZ8061_ANAR                              0x04
#define KSZ8061_ANLPAR                            0x05
#define KSZ8061_ANER                              0x06
#define KSZ8061_ANNPR                             0x07
#define KSZ8061_ANLPNPR                           0x08
#define KSZ8061_MMDACR                            0x0D
#define KSZ8061_MMDAADR                           0x0E
#define KSZ8061_DCON                              0x10
#define KSZ8061_AFECON0                           0x11
#define KSZ8061_AFECON1                           0x12
#define KSZ8061_AFECON2                           0x13
#define KSZ8061_AFECON3                           0x14
#define KSZ8061_RXERCTR                           0x15
#define KSZ8061_OM                                0x16
#define KSZ8061_OMSS                              0x17
#define KSZ8061_EXCON                             0x18
#define KSZ8061_ICSR                              0x1B
#define KSZ8061_FUNCON                            0x1C
#define KSZ8061_LINKMD                            0x1D
#define KSZ8061_PHYCON1                           0x1E
#define KSZ8061_PHYCON2                           0x1F

//KSZ8061 MMD registers
#define KSZ8061_AFED_CTRL                         0x1B, 0x00
#define KSZ8061_SIGNAL_QUALITY                    0x1C, 0xAC

//Basic Control register
#define KSZ8061_BMCR_RESET                        0x8000
#define KSZ8061_BMCR_LOOPBACK                     0x4000
#define KSZ8061_BMCR_SPEED_SEL                    0x2000
#define KSZ8061_BMCR_AN_EN                        0x1000
#define KSZ8061_BMCR_POWER_DOWN                   0x0800
#define KSZ8061_BMCR_ISOLATE                      0x0400
#define KSZ8061_BMCR_RESTART_AN                   0x0200
#define KSZ8061_BMCR_DUPLEX_MODE                  0x0100
#define KSZ8061_BMCR_COL_TEST                     0x0080

//Basic Status register
#define KSZ8061_BMSR_100BT4                       0x8000
#define KSZ8061_BMSR_100BTX_FD                    0x4000
#define KSZ8061_BMSR_100BTX_HD                    0x2000
#define KSZ8061_BMSR_10BT_FD                      0x1000
#define KSZ8061_BMSR_10BT_HD                      0x0800
#define KSZ8061_BMSR_NO_PREAMBLE                  0x0040
#define KSZ8061_BMSR_AN_COMPLETE                  0x0020
#define KSZ8061_BMSR_REMOTE_FAULT                 0x0010
#define KSZ8061_BMSR_AN_CAPABLE                   0x0008
#define KSZ8061_BMSR_LINK_STATUS                  0x0004
#define KSZ8061_BMSR_JABBER_DETECT                0x0002
#define KSZ8061_BMSR_EXTENDED_CAPABLE             0x0001

//PHY Identifier 1 register
#define KSZ8061_PHYID1_PHY_ID_MSB                 0xFFFF
#define KSZ8061_PHYID1_PHY_ID_MSB_DEFAULT         0x0022

//PHY Identifier 2 register
#define KSZ8061_PHYID2_PHY_ID_LSB                 0xFC00
#define KSZ8061_PHYID2_PHY_ID_LSB_DEFAULT         0x1400
#define KSZ8061_PHYID2_MODEL_NUM                  0x03F0
#define KSZ8061_PHYID2_MODEL_NUM_DEFAULT          0x0170
#define KSZ8061_PHYID2_REVISION_NUM               0x000F

//Auto-Negotiation Advertisement register
#define KSZ8061_ANAR_NEXT_PAGE                    0x8000
#define KSZ8061_ANAR_REMOTE_FAULT                 0x2000
#define KSZ8061_ANAR_PAUSE                        0x0C00
#define KSZ8061_ANAR_100BT4                       0x0200
#define KSZ8061_ANAR_100BTX_FD                    0x0100
#define KSZ8061_ANAR_100BTX_HD                    0x0080
#define KSZ8061_ANAR_10BT_FD                      0x0040
#define KSZ8061_ANAR_10BT_HD                      0x0020
#define KSZ8061_ANAR_SELECTOR                     0x001F
#define KSZ8061_ANAR_SELECTOR_DEFAULT             0x0001

//Auto-Negotiation Link Partner Ability register
#define KSZ8061_ANLPAR_NEXT_PAGE                  0x8000
#define KSZ8061_ANLPAR_ACK                        0x4000
#define KSZ8061_ANLPAR_REMOTE_FAULT               0x2000
#define KSZ8061_ANLPAR_PAUSE                      0x0C00
#define KSZ8061_ANLPAR_100BT4                     0x0200
#define KSZ8061_ANLPAR_100BTX_FD                  0x0100
#define KSZ8061_ANLPAR_100BTX_HD                  0x0080
#define KSZ8061_ANLPAR_10BT_FD                    0x0040
#define KSZ8061_ANLPAR_10BT_HD                    0x0020
#define KSZ8061_ANLPAR_SELECTOR                   0x001F
#define KSZ8061_ANLPAR_SELECTOR_DEFAULT           0x0001

//Auto-Negotiation Expansion register
#define KSZ8061_ANER_PAR_DETECT_FAULT             0x0010
#define KSZ8061_ANER_LP_NEXT_PAGE_ABLE            0x0008
#define KSZ8061_ANER_NEXT_PAGE_ABLE               0x0004
#define KSZ8061_ANER_PAGE_RECEIVED                0x0002
#define KSZ8061_ANER_LP_AN_ABLE                   0x0001

//Auto-Negotiation Next Page register
#define KSZ8061_ANNPR_NEXT_PAGE                   0x8000
#define KSZ8061_ANNPR_MSG_PAGE                    0x2000
#define KSZ8061_ANNPR_ACK2                        0x1000
#define KSZ8061_ANNPR_TOGGLE                      0x0800
#define KSZ8061_ANNPR_MESSAGE                     0x07FF

//Link Partner Next Page Ability register
#define KSZ8061_ANLPNPR_NEXT_PAGE                 0x8000
#define KSZ8061_ANLPNPR_ACK                       0x4000
#define KSZ8061_ANLPNPR_MSG_PAGE                  0x2000
#define KSZ8061_ANLPNPR_ACK2                      0x1000
#define KSZ8061_ANLPNPR_TOGGLE                    0x0800
#define KSZ8061_ANLPNPR_MESSAGE                   0x07FF

//MMD Access Control register
#define KSZ8061_MMDACR_FUNC                       0xC000
#define KSZ8061_MMDACR_FUNC_ADDR                  0x0000
#define KSZ8061_MMDACR_FUNC_DATA_NO_POST_INC      0x4000
#define KSZ8061_MMDACR_FUNC_DATA_POST_INC_RW      0x8000
#define KSZ8061_MMDACR_FUNC_DATA_POST_INC_W       0xC000
#define KSZ8061_MMDACR_DEVAD                      0x001F

//Digital Control register
#define KSZ8061_DCON_PLL_OFF                      0x0010

//AFE Control 0 register
#define KSZ8061_AFECON0_SLOW_OSC_MODE_PD          0x0040

//AFE Control 1 register
#define KSZ8061_AFECON1_100BT_AMPLITUDE           0xF000

//AFE Control 2 register
#define KSZ8061_AFECON2_LINKMD_DETECT_THRESHOLD   0x8000
#define KSZ8061_AFECON2_SLOW_OSC_MODE_UDS         0x0001

//AFE Control 3 register
#define KSZ8061_AFECON3_UDS_METHOD                0x0040
#define KSZ8061_AFECON3_MANUAL_UDS_MODE           0x0020
#define KSZ8061_AFECON3_NV_REG_ACCESS             0x0010
#define KSZ8061_AFECON3_UDS_MODE_SIGDET_EN        0x0008
#define KSZ8061_AFECON3_RX_INTERNAL_TERM_DIS      0x0004
#define KSZ8061_AFECON3_SIGDET_DEASSERT_DELAY     0x0002
#define KSZ8061_AFECON3_SIGDET_POL                0x0001

//Operation Mode register
#define KSZ8061_OM_QWF_DIS                        0x1000

//Operation Mode Strap Status register
#define KSZ8061_OMSS_PHYAD                        0xE000
#define KSZ8061_OMSS_QWF_STRAP_STATUS             0x0100
#define KSZ8061_OMSS_MII_BTB_STRAP_STATUS         0x0080
#define KSZ8061_OMSS_RMII_BTB_STRAP_STATUS        0x0040
#define KSZ8061_OMSS_NAND_TREE_STRAP_STATUS       0x0020
#define KSZ8061_OMSS_RMII_STRAP_STATUS            0x0002
#define KSZ8061_OMSS_MII_STRAP_STATUS             0x0001

//Expanded Control register
#define KSZ8061_EXCON_EDPD_DIS                    0x0800
#define KSZ8061_EXCON_RX_PHY_LATENCY              0x0400
#define KSZ8061_EXCON_10BT_PREAMBLE_EN            0x0040

//Interrupt Control/Status register
#define KSZ8061_ICSR_JABBER_IE                    0x8000
#define KSZ8061_ICSR_RECEIVE_ERROR_IE             0x4000
#define KSZ8061_ICSR_PAGE_RECEIVED_IE             0x2000
#define KSZ8061_ICSR_PAR_DETECT_FAULT_IE          0x1000
#define KSZ8061_ICSR_LP_ACK_IE                    0x0800
#define KSZ8061_ICSR_LINK_DOWN_IE                 0x0400
#define KSZ8061_ICSR_REMOTE_FAULT_IE              0x0200
#define KSZ8061_ICSR_LINK_UP_IE                   0x0100
#define KSZ8061_ICSR_JABBER_IF                    0x0080
#define KSZ8061_ICSR_RECEIVE_ERROR_IF             0x0040
#define KSZ8061_ICSR_PAGE_RECEIVED_IF             0x0020
#define KSZ8061_ICSR_PAR_DETECT_FAULT_IF          0x0010
#define KSZ8061_ICSR_LP_ACK_IF                    0x0008
#define KSZ8061_ICSR_LINK_DOWN_IF                 0x0004
#define KSZ8061_ICSR_REMOTE_FAULT_IF              0x0002
#define KSZ8061_ICSR_LINK_UP_IF                   0x0001

//Function Control register
#define KSZ8061_FUNCON_LOCAL_LOOPBACK             0x0020

//LinkMD Control/Status register
#define KSZ8061_LINKMD_TEST_EN                    0x8000
#define KSZ8061_LINKMD_RESULT                     0x6000
#define KSZ8061_LINKMD_SHORT                      0x1000
#define KSZ8061_LINKMD_FAULT_COUNT                0x01FF

//PHY Control 1 register
#define KSZ8061_PHYCON1_PAUSE_EN                  0x0200
#define KSZ8061_PHYCON1_LINK_STATUS               0x0100
#define KSZ8061_PHYCON1_POL_STATUS                0x0080
#define KSZ8061_PHYCON1_MDIX_STATE                0x0020
#define KSZ8061_PHYCON1_ENERGY_DETECT             0x0010
#define KSZ8061_PHYCON1_PHY_ISOLATE               0x0008
#define KSZ8061_PHYCON1_OP_MODE                   0x0007
#define KSZ8061_PHYCON1_OP_MODE_AN                0x0000
#define KSZ8061_PHYCON1_OP_MODE_10BT_HD           0x0001
#define KSZ8061_PHYCON1_OP_MODE_100BTX_HD         0x0002
#define KSZ8061_PHYCON1_OP_MODE_10BT_FD           0x0005
#define KSZ8061_PHYCON1_OP_MODE_100BTX_FD         0x0006

//PHY Control 2 register
#define KSZ8061_PHYCON2_HP_MDIX                   0x8000
#define KSZ8061_PHYCON2_MDIX_SEL                  0x4000
#define KSZ8061_PHYCON2_PAIR_SWAP_DIS             0x2000
#define KSZ8061_PHYCON2_FORCE_LINK                0x0800
#define KSZ8061_PHYCON2_POWER_SAVING              0x0400
#define KSZ8061_PHYCON2_INT_LEVEL                 0x0200
#define KSZ8061_PHYCON2_JABBER_EN                 0x0100
#define KSZ8061_PHYCON2_LED_MODE                  0x0030
#define KSZ8061_PHYCON2_TX_DIS                    0x0008
#define KSZ8061_PHYCON2_REMOTE_LOOPBACK           0x0004
#define KSZ8061_PHYCON2_SQE_TEST_EN               0x0002
#define KSZ8061_PHYCON2_DATA_SCRAMBLING_DIS       0x0001

//AFED Control register
#define KSZ8061_AFED_CTRL_LINKMD_PULSE_AMPLITUDE  0x00F0

//Signal Quality register
#define KSZ8061_SIGNAL_QUALITY_SIGNAL_QUALITY_IND 0x7F00

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//KSZ8061 Ethernet PHY driver
extern const PhyDriver ksz8061PhyDriver;

//KSZ8061 related functions
error_t ksz8061Init(NetInterface *interface);
void ksz8061InitHook(NetInterface *interface);

void ksz8061Tick(NetInterface *interface);

void ksz8061EnableIrq(NetInterface *interface);
void ksz8061DisableIrq(NetInterface *interface);

void ksz8061EventHandler(NetInterface *interface);

void ksz8061WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t ksz8061ReadPhyReg(NetInterface *interface, uint8_t address);

void ksz8061DumpPhyReg(NetInterface *interface);

void ksz8061WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t ksz8061ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
