/**
 * @file ksz8051_driver.h
 * @brief KSZ8051 Ethernet PHY driver
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

#ifndef _KSZ8051_DRIVER_H
#define _KSZ8051_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef KSZ8051_PHY_ADDR
   #define KSZ8051_PHY_ADDR 1
#elif (KSZ8051_PHY_ADDR < 0 || KSZ8051_PHY_ADDR > 31)
   #error KSZ8051_PHY_ADDR parameter is not valid
#endif

//KSZ8051 PHY registers
#define KSZ8051_BMCR                        0x00
#define KSZ8051_BMSR                        0x01
#define KSZ8051_PHYID1                      0x02
#define KSZ8051_PHYID2                      0x03
#define KSZ8051_ANAR                        0x04
#define KSZ8051_ANLPAR                      0x05
#define KSZ8051_ANER                        0x06
#define KSZ8051_ANNPR                       0x07
#define KSZ8051_ANLPNPR                     0x08
#define KSZ8051_DRCON                       0x10
#define KSZ8051_AFECON1                     0x11
#define KSZ8051_RXERCTR                     0x15
#define KSZ8051_OMSO                        0x16
#define KSZ8051_OMSS                        0x17
#define KSZ8051_EXCON                       0x18
#define KSZ8051_ICSR                        0x1B
#define KSZ8051_LINKMD                      0x1D
#define KSZ8051_PHYCON1                     0x1E
#define KSZ8051_PHYCON2                     0x1F

//Basic Control register
#define KSZ8051_BMCR_RESET                  0x8000
#define KSZ8051_BMCR_LOOPBACK               0x4000
#define KSZ8051_BMCR_SPEED_SEL              0x2000
#define KSZ8051_BMCR_AN_EN                  0x1000
#define KSZ8051_BMCR_POWER_DOWN             0x0800
#define KSZ8051_BMCR_ISOLATE                0x0400
#define KSZ8051_BMCR_RESTART_AN             0x0200
#define KSZ8051_BMCR_DUPLEX_MODE            0x0100
#define KSZ8051_BMCR_COL_TEST               0x0080

//Basic Status register
#define KSZ8051_BMSR_100BT4                 0x8000
#define KSZ8051_BMSR_100BTX_FD              0x4000
#define KSZ8051_BMSR_100BTX_HD              0x2000
#define KSZ8051_BMSR_10BT_FD                0x1000
#define KSZ8051_BMSR_10BT_HD                0x0800
#define KSZ8051_BMSR_NO_PREAMBLE            0x0040
#define KSZ8051_BMSR_AN_COMPLETE            0x0020
#define KSZ8051_BMSR_REMOTE_FAULT           0x0010
#define KSZ8051_BMSR_AN_CAPABLE             0x0008
#define KSZ8051_BMSR_LINK_STATUS            0x0004
#define KSZ8051_BMSR_JABBER_DETECT          0x0002
#define KSZ8051_BMSR_EXTENDED_CAPABLE       0x0001

//PHY Identifier 1 register
#define KSZ8051_PHYID1_PHY_ID_MSB           0xFFFF
#define KSZ8051_PHYID1_PHY_ID_MSB_DEFAULT   0x0022

//PHY Identifier 2 register
#define KSZ8051_PHYID2_PHY_ID_LSB           0xFC00
#define KSZ8051_PHYID2_PHY_ID_LSB_DEFAULT   0x1400
#define KSZ8051_PHYID2_MODEL_NUM            0x03F0
#define KSZ8051_PHYID2_MODEL_NUM_DEFAULT    0x0160
#define KSZ8051_PHYID2_REVISION_NUM         0x000F

//Auto-Negotiation Advertisement register
#define KSZ8051_ANAR_NEXT_PAGE              0x8000
#define KSZ8051_ANAR_REMOTE_FAULT           0x2000
#define KSZ8051_ANAR_PAUSE                  0x0C00
#define KSZ8051_ANAR_100BT4                 0x0200
#define KSZ8051_ANAR_100BTX_FD              0x0100
#define KSZ8051_ANAR_100BTX_HD              0x0080
#define KSZ8051_ANAR_10BT_FD                0x0040
#define KSZ8051_ANAR_10BT_HD                0x0020
#define KSZ8051_ANAR_SELECTOR               0x001F
#define KSZ8051_ANAR_SELECTOR_DEFAULT       0x0001

//Auto-Negotiation Link Partner Ability register
#define KSZ8051_ANLPAR_NEXT_PAGE            0x8000
#define KSZ8051_ANLPAR_ACK                  0x4000
#define KSZ8051_ANLPAR_REMOTE_FAULT         0x2000
#define KSZ8051_ANLPAR_PAUSE                0x0C00
#define KSZ8051_ANLPAR_100BT4               0x0200
#define KSZ8051_ANLPAR_100BTX_FD            0x0100
#define KSZ8051_ANLPAR_100BTX_HD            0x0080
#define KSZ8051_ANLPAR_10BT_FD              0x0040
#define KSZ8051_ANLPAR_10BT_HD              0x0020
#define KSZ8051_ANLPAR_SELECTOR             0x001F
#define KSZ8051_ANLPAR_SELECTOR_DEFAULT     0x0001

//Auto-Negotiation Expansion register
#define KSZ8051_ANER_PAR_DETECT_FAULT       0x0010
#define KSZ8051_ANER_LP_NEXT_PAGE_ABLE      0x0008
#define KSZ8051_ANER_NEXT_PAGE_ABLE         0x0004
#define KSZ8051_ANER_PAGE_RECEIVED          0x0002
#define KSZ8051_ANER_LP_AN_ABLE             0x0001

//Auto-Negotiation Next Page register
#define KSZ8051_ANNPR_NEXT_PAGE             0x8000
#define KSZ8051_ANNPR_MSG_PAGE              0x2000
#define KSZ8051_ANNPR_ACK2                  0x1000
#define KSZ8051_ANNPR_TOGGLE                0x0800
#define KSZ8051_ANNPR_MESSAGE               0x07FF

//Link Partner Next Page Ability register
#define KSZ8051_ANLPNPR_NEXT_PAGE           0x8000
#define KSZ8051_ANLPNPR_ACK                 0x4000
#define KSZ8051_ANLPNPR_MSG_PAGE            0x2000
#define KSZ8051_ANLPNPR_ACK2                0x1000
#define KSZ8051_ANLPNPR_TOGGLE              0x0800
#define KSZ8051_ANLPNPR_MESSAGE             0x07FF

//Digital Reserved Control register
#define KSZ8051_DRCON_PLL_OFF               0x0010

//AFE Control 1 register
#define KSZ8051_AFECON1_SLOW_OSC_MODE_EN    0x0020

//Operation Mode Strap Override register
#define KSZ8051_OMSO_BCAST_OFF_OVERRIDE     0x0200
#define KSZ8051_OMSO_MII_BTB_OVERRIDE       0x0080
#define KSZ8051_OMSO_RMII_BTB_OVERRIDE      0x0040
#define KSZ8051_OMSO_NAND_TREE_OVERRIDE     0x0020
#define KSZ8051_OMSO_RMII_OVERRIDE          0x0002
#define KSZ8051_OMSO_MII_OVERRIDE           0x0001

//Operation Mode Strap Status register
#define KSZ8051_OMSS_PHYAD                  0xE000
#define KSZ8051_OMSS_BCAST_OFF_STRAP_STATUS 0x0200
#define KSZ8051_OMSS_MII_BTB_STRAP_STATUS   0x0080
#define KSZ8051_OMSS_RMII_BTB_STRAP_STATUS  0x0040
#define KSZ8051_OMSS_NAND_TREE_STRAP_STATUS 0x0020
#define KSZ8051_OMSS_RMII_STRAP_STATUS      0x0002
#define KSZ8051_OMSS_MII_STRAP_STATUS       0x0001

//Expanded Control register
#define KSZ8051_EXCON_EDPD_DIS              0x0800
#define KSZ8051_EXCON_100BTX_LATENCY        0x0400
#define KSZ8051_EXCON_10BT_PREAMBLE_RESTORE 0x0040

//Interrupt Control/Status register
#define KSZ8051_ICSR_JABBER_IE              0x8000
#define KSZ8051_ICSR_RECEIVE_ERROR_IE       0x4000
#define KSZ8051_ICSR_PAGE_RECEIVED_IE       0x2000
#define KSZ8051_ICSR_PAR_DETECT_FAULT_IE    0x1000
#define KSZ8051_ICSR_LP_ACK_IE              0x0800
#define KSZ8051_ICSR_LINK_DOWN_IE           0x0400
#define KSZ8051_ICSR_REMOTE_FAULT_IE        0x0200
#define KSZ8051_ICSR_LINK_UP_IE             0x0100
#define KSZ8051_ICSR_JABBER_IF              0x0080
#define KSZ8051_ICSR_RECEIVE_ERROR_IF       0x0040
#define KSZ8051_ICSR_PAGE_RECEIVED_IF       0x0020
#define KSZ8051_ICSR_PAR_DETECT_FAULT_IF    0x0010
#define KSZ8051_ICSR_LP_ACK_IF              0x0008
#define KSZ8051_ICSR_LINK_DOWN_IF           0x0004
#define KSZ8051_ICSR_REMOTE_FAULT_IF        0x0002
#define KSZ8051_ICSR_LINK_UP_IF             0x0001

//LinkMD Control/Status register
#define KSZ8051_LINKMD_TEST_EN              0x8000
#define KSZ8051_LINKMD_RESULT               0x6000
#define KSZ8051_LINKMD_SHORT                0x1000
#define KSZ8051_LINKMD_FAULT_COUNT          0x01FF

//PHY Control 1 register
#define KSZ8051_PHYCON1_PAUSE_EN            0x0200
#define KSZ8051_PHYCON1_LINK_STATUS         0x0100
#define KSZ8051_PHYCON1_POL_STATUS          0x0080
#define KSZ8051_PHYCON1_MDIX_STATE          0x0020
#define KSZ8051_PHYCON1_ENERGY_DETECT       0x0010
#define KSZ8051_PHYCON1_PHY_ISOLATE         0x0008
#define KSZ8051_PHYCON1_OP_MODE             0x0007
#define KSZ8051_PHYCON1_OP_MODE_AN          0x0000
#define KSZ8051_PHYCON1_OP_MODE_10BT_HD     0x0001
#define KSZ8051_PHYCON1_OP_MODE_100BTX_HD   0x0002
#define KSZ8051_PHYCON1_OP_MODE_10BT_FD     0x0005
#define KSZ8051_PHYCON1_OP_MODE_100BTX_FD   0x0006

//PHY Control 2 register
#define KSZ8051_PHYCON2_HP_MDIX             0x8000
#define KSZ8051_PHYCON2_MDIX_SEL            0x4000
#define KSZ8051_PHYCON2_PAIR_SWAP_DIS       0x2000
#define KSZ8051_PHYCON2_FORCE_LINK          0x0800
#define KSZ8051_PHYCON2_POWER_SAVING        0x0400
#define KSZ8051_PHYCON2_INT_LEVEL           0x0200
#define KSZ8051_PHYCON2_JABBER_EN           0x0100
#define KSZ8051_PHYCON2_RMII_REF_CLK_SEL    0x0080
#define KSZ8051_PHYCON2_LED_MODE            0x0030
#define KSZ8051_PHYCON2_TX_DIS              0x0008
#define KSZ8051_PHYCON2_REMOTE_LOOPBACK     0x0004
#define KSZ8051_PHYCON2_SQE_TEST_EN         0x0002
#define KSZ8051_PHYCON2_DATA_SCRAMBLING_DIS 0x0001

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//KSZ8051 Ethernet PHY driver
extern const PhyDriver ksz8051PhyDriver;

//KSZ8051 related functions
error_t ksz8051Init(NetInterface *interface);
void ksz8051InitHook(NetInterface *interface);

void ksz8051Tick(NetInterface *interface);

void ksz8051EnableIrq(NetInterface *interface);
void ksz8051DisableIrq(NetInterface *interface);

void ksz8051EventHandler(NetInterface *interface);

void ksz8051WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t ksz8051ReadPhyReg(NetInterface *interface, uint8_t address);

void ksz8051DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
