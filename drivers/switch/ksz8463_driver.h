/**
 * @file ksz8463_driver.h
 * @brief KSZ8463 3-port Ethernet switch
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.4
 **/

#ifndef _KSZ8463_DRIVER_H
#define _KSZ8463_DRIVER_H

//Dependencies
#include "core/nic.h"

//KSZ8463 ports
#define KSZ8463_PORT1 1
#define KSZ8463_PORT2 2

//SPI command byte
#define KSZ8463_SPI_CMD_READ  0x0000
#define KSZ8463_SPI_CMD_WRITE 0x8000
#define KSZ8463_SPI_CMD_ADDR  0x7FC0
#define KSZ8463_SPI_CMD_B3    0x0020
#define KSZ8463_SPI_CMD_B2    0x0010
#define KSZ8463_SPI_CMD_B1    0x0008
#define KSZ8463_SPI_CMD_B0    0x0004

//KSZ8463 PHY registers
#define KSZ8463_BMCR                           0x00
#define KSZ8463_BMSR                           0x01
#define KSZ8463_PHYID1                         0x02
#define KSZ8463_PHYID2                         0x03
#define KSZ8463_ANAR                           0x04
#define KSZ8463_ANLPAR                         0x05
#define KSZ8463_LINKMD                         0x1D
#define KSZ8463_PHYSCS                         0x1F

//KSZ8463 Switch registers
#define KSZ8463_CIDER                          0x00
#define KSZ8463_SGCR1                          0x02
#define KSZ8463_SGCR2                          0x04
#define KSZ8463_SGCR3                          0x06
#define KSZ8463_SGCR6                          0x0C
#define KSZ8463_SGCR7                          0x0E
#define KSZ8463_P1CR1                          0x6C
#define KSZ8463_P1CR2                          0x6E
#define KSZ8463_P1SR                           0x80
#define KSZ8463_P2CR1                          0x84
#define KSZ8463_P2CR2                          0x86
#define KSZ8463_P2SR                           0x98
#define KSZ8463_P3CR1                          0x9C
#define KSZ8463_P3CR2                          0x9E
#define KSZ8463_SGCR8                          0xAC

//KSZ8463 Switch register access macros
#define KSZ8463_PnCR1(port)                    (0x54 + ((port) * 0x18))
#define KSZ8463_PnCR2(port)                    (0x56 + ((port) * 0x18))
#define KSZ8463_PnSR(port)                     (0x68 + ((port) * 0x18))

//Basic Control register
#define KSZ8463_BMCR_LOOPBACK                  0x4000
#define KSZ8463_BMCR_FORCE_100                 0x2000
#define KSZ8463_BMCR_AN_EN                     0x1000
#define KSZ8463_BMCR_POWER_DOWN                0x0800
#define KSZ8463_BMCR_ISOLATE                   0x0400
#define KSZ8463_BMCR_RESTART_AN                0x0200
#define KSZ8463_BMCR_FORCE_FULL_DUPLEX         0x0100
#define KSZ8463_BMCR_COL_TEST                  0x0080
#define KSZ8463_BMCR_HP_MDIX                   0x0020
#define KSZ8463_BMCR_FORCE_MDI                 0x0010
#define KSZ8463_BMCR_AUTO_MDIX_DIS             0x0008
#define KSZ8463_BMCR_FAR_END_FAULT_DIS         0x0004
#define KSZ8463_BMCR_TRANSMIT_DIS              0x0002
#define KSZ8463_BMCR_LED_DIS                   0x0001

//Basic Status register
#define KSZ8463_BMSR_100BT4                    0x8000
#define KSZ8463_BMSR_100BTX_FD                 0x4000
#define KSZ8463_BMSR_100BTX_HD                 0x2000
#define KSZ8463_BMSR_10BT_FD                   0x1000
#define KSZ8463_BMSR_10BT_HD                   0x0800
#define KSZ8463_BMSR_PREAMBLE_SUPPR            0x0040
#define KSZ8463_BMSR_AN_COMPLETE               0x0020
#define KSZ8463_BMSR_FAR_END_FAULT             0x0010
#define KSZ8463_BMSR_AN_CAPABLE                0x0008
#define KSZ8463_BMSR_LINK_STATUS               0x0004
#define KSZ8463_BMSR_JABBER_TEST               0x0002
#define KSZ8463_BMSR_EXTENDED_CAPABLE          0x0001

//PHYID High register
#define KSZ8463_PHYID1_DEFAULT                 0x0022

//PHYID Low register
#define KSZ8463_PHYID2_DEFAULT                 0x1430

//Auto-Negotiation Advertisement Ability register
#define KSZ8463_ANAR_NEXT_PAGE                 0x8000
#define KSZ8463_ANAR_REMOTE_FAULT              0x2000
#define KSZ8463_ANAR_PAUSE                     0x0400
#define KSZ8463_ANAR_100BTX_FD                 0x0100
#define KSZ8463_ANAR_100BTX_HD                 0x0080
#define KSZ8463_ANAR_10BT_FD                   0x0040
#define KSZ8463_ANAR_10BT_HD                   0x0020
#define KSZ8463_ANAR_SELECTOR                  0x001F
#define KSZ8463_ANAR_SELECTOR_DEFAULT          0x0001

//Auto-Negotiation Link Partner Ability register
#define KSZ8463_ANLPAR_NEXT_PAGE               0x8000
#define KSZ8463_ANLPAR_LP_ACK                  0x4000
#define KSZ8463_ANLPAR_REMOTE_FAULT            0x2000
#define KSZ8463_ANLPAR_PAUSE                   0x0400
#define KSZ8463_ANLPAR_100BTX_FD               0x0100
#define KSZ8463_ANLPAR_100BTX_HD               0x0080
#define KSZ8463_ANLPAR_10BT_FD                 0x0040
#define KSZ8463_ANLPAR_10BT_HD                 0x0020

//LinkMD Control/Status register
#define KSZ8463_LINKMD_TEST_EN                 0x8000
#define KSZ8463_LINKMD_RESULT                  0x6000
#define KSZ8463_LINKMD_SHORT                   0x1000
#define KSZ8463_LINKMD_FAULT_COUNT             0x01FF

//PHY Special Control/Status register
#define KSZ8463_PHYSCS_POL_REVERSE             0x0020
#define KSZ8463_PHYSCS_MDIX_STATUS             0x0010
#define KSZ8463_PHYSCS_FORCE_LINK              0x0008
#define KSZ8463_PHYSCS_EEE_EN                  0x0004
#define KSZ8463_PHYSCS_REMOTE_LOOPBACK         0x0002

//Chip ID And Enable register
#define KSZ8463_CIDER_FAMILY_ID                0xFF00
#define KSZ8463_CIDER_FAMILY_ID_DEFAULT        0x8400
#define KSZ8463_CIDER_CHIP_ID                  0x00F0
#define KSZ8463_CIDER_CHIP_ID_ML_FML           0x0040
#define KSZ8463_CIDER_CHIP_ID_RL_FRL           0x0050
#define KSZ8463_CIDER_REVISION_ID              0x000E
#define KSZ8463_CIDER_START_SWITCH             0x0001

//Port N Control 2 register
#define KSZ8463_PnCR2_INGRESS_VLAN_FILT        0x4000
#define KSZ8463_PnCR2_DISCARD_NON_PVID_PACKETS 0x2000
#define KSZ8463_PnCR2_FORCE_FLOW_CTRL          0x1000
#define KSZ8463_PnCR2_BACK_PRESSURE_EN         0x0800
#define KSZ8463_PnCR2_TRANSMIT_EN              0x0400
#define KSZ8463_PnCR2_RECEIVE_EN               0x0200
#define KSZ8463_PnCR2_LEARNING_DIS             0x0100
#define KSZ8463_PnCR2_SNIFFER_PORT             0x0080
#define KSZ8463_PnCR2_RECEIVE_SNIFF            0x0040
#define KSZ8463_PnCR2_TRANSMIT_SNIFF           0x0020
#define KSZ8463_PnCR2_USER_PRIO_CEILING        0x0008
#define KSZ8463_PnCR2_PORT_VLAN_MEMBERSHIP     0x0007

//Port N Status register
#define KSZ8463_PnSR_HP_MDIX                   0x8000
#define KSZ8463_PnSR_POL_REVERSE               0x2000
#define KSZ8463_PnSR_TX_FLOW_CTRL_EN           0x1000
#define KSZ8463_PnSR_RX_FLOW_CTRL_EN           0x0800
#define KSZ8463_PnSR_OP_SPEED                  0x0400
#define KSZ8463_PnSR_OP_DUPLEX                 0x0200
#define KSZ8463_PnSR_FAR_END_FAULT             0x0100
#define KSZ8463_PnSR_MDIX_STATUS               0x0080
#define KSZ8463_PnSR_AN_DONE                   0x0040
#define KSZ8463_PnSR_LINK_STATUS               0x0020
#define KSZ8463_PnSR_LP_FLOW_CTRL_CAPABLE      0x0010
#define KSZ8463_PnSR_LP_100BTX_FD_CAPABLE      0x0008
#define KSZ8463_PnSR_LP_100BTX_HF_CAPABLE      0x0004
#define KSZ8463_PnSR_LP_10BT_FD_CAPABLE        0x0002
#define KSZ8463_PnSR_LP_10BT_HD_CAPABLE        0x0001

//Switch Global Control 8 register
#define KSZ8463_SGCR8_QUEUE_PRIO_MAPPING       0xC000
#define KSZ8463_SGCR8_FLUSH_DYNAMIC_MAC_TABLE  0x0400
#define KSZ8463_SGCR8_FLUSH_STATIC_MAC_TABLE   0x0200
#define KSZ8463_SGCR8_TAIL_TAG_EN              0x0100
#define KSZ8463_SGCR8_PAUSE_OFF_LIMIT_TIME     0x00FF

//Tail tag encoding
#define KSZ8463_TAIL_TAG_ENCODE(port) ((port) & 0x03)
//Tail tag decoding
#define KSZ8463_TAIL_TAG_DECODE(tag) (((tag) & 0x01) + 1)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//KSZ8463 Ethernet switch driver
extern const PhyDriver ksz8463PhyDriver;

//KSZ8463 related functions
error_t ksz8463Init(NetInterface *interface);

bool_t ksz8463GetLinkState(NetInterface *interface, uint8_t port);

void ksz8463Tick(NetInterface *interface);

void ksz8463EnableIrq(NetInterface *interface);
void ksz8463DisableIrq(NetInterface *interface);

void ksz8463EventHandler(NetInterface *interface);

error_t ksz8463TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type);

error_t ksz8463UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, uint8_t *port);

void ksz8463WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data);

uint16_t ksz8463ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address);

void ksz8463DumpPhyReg(NetInterface *interface, uint8_t port);

void ksz8463WriteSwitchReg(NetInterface *interface, uint16_t address,
   uint16_t data);

uint16_t ksz8463ReadSwitchReg(NetInterface *interface, uint16_t address);

void ksz8463DumpSwitchReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
