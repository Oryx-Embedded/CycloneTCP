/**
 * @file ksz8873_driver.h
 * @brief KSZ8873 3-port Ethernet switch
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

#ifndef _KSZ8873_DRIVER_H
#define _KSZ8873_DRIVER_H

//Dependencies
#include "core/nic.h"

//KSZ8873 ports
#define KSZ8873_PORT1 1
#define KSZ8873_PORT2 2

//SPI command byte
#define KSZ8873_SPI_CMD_WRITE 0x02
#define KSZ8873_SPI_CMD_READ  0x03

//KSZ8873 PHY registers
#define KSZ8873_BMCR                                 0x00
#define KSZ8873_BMSR                                 0x01
#define KSZ8873_PHYID1                               0x02
#define KSZ8873_PHYID2                               0x03
#define KSZ8873_ANAR                                 0x04
#define KSZ8873_ANLPAR                               0x05
#define KSZ8873_LINKMD                               0x1D
#define KSZ8873_PHYSCS                               0x1F

//KSZ8873 Switch registers
#define KSZ8873_CHIP_ID0                             0x00
#define KSZ8873_CHIP_ID1                             0x01
#define KSZ8873_GLOBAL_CTRL0                         0x02
#define KSZ8873_GLOBAL_CTRL1                         0x03
#define KSZ8873_PORT1_CTRL0                          0x10
#define KSZ8873_PORT1_CTRL1                          0x11
#define KSZ8873_PORT1_CTRL2                          0x12
#define KSZ8873_PORT1_STAT0                          0x1E
#define KSZ8873_PORT1_STAT1                          0x1F
#define KSZ8873_PORT2_CTRL0                          0x20
#define KSZ8873_PORT2_CTRL1                          0x21
#define KSZ8873_PORT2_CTRL2                          0x22
#define KSZ8873_PORT2_STAT0                          0x2E
#define KSZ8873_PORT2_STAT1                          0x2F
#define KSZ8873_PORT3_CTRL0                          0x30
#define KSZ8873_PORT3_CTRL1                          0x31
#define KSZ8873_PORT3_CTRL2                          0x32
#define KSZ8873_PORT3_STAT0                          0x3E
#define KSZ8873_PORT3_STAT1                          0x3F

//KSZ8873 Switch register access macros
#define KSZ8873_PORTn_CTRL0(port)                    (0x00 + ((port) * 0x10))
#define KSZ8873_PORTn_CTRL1(port)                    (0x01 + ((port) * 0x10))
#define KSZ8873_PORTn_CTRL2(port)                    (0x02 + ((port) * 0x10))
#define KSZ8873_PORTn_STAT0(port)                    (0x0E + ((port) * 0x10))
#define KSZ8873_PORTn_STAT1(port)                    (0x0F + ((port) * 0x10))

//MII Basic Control register
#define KSZ8873_BMCR_RESET                           0x8000
#define KSZ8873_BMCR_LOOPBACK                        0x4000
#define KSZ8873_BMCR_FORCE_100                       0x2000
#define KSZ8873_BMCR_AN_EN                           0x1000
#define KSZ8873_BMCR_POWER_DOWN                      0x0800
#define KSZ8873_BMCR_ISOLATE                         0x0400
#define KSZ8873_BMCR_RESTART_AN                      0x0200
#define KSZ8873_BMCR_FORCE_FULL_DUPLEX               0x0100
#define KSZ8873_BMCR_COL_TEST                        0x0080
#define KSZ8873_BMCR_HP_MDIX                         0x0020
#define KSZ8873_BMCR_FORCE_MDI                       0x0010
#define KSZ8873_BMCR_AUTO_MDIX_DIS                   0x0008
#define KSZ8873_BMCR_FAR_END_FAULT_DIS               0x0004
#define KSZ8873_BMCR_TRANSMIT_DIS                    0x0002
#define KSZ8873_BMCR_LED_DIS                         0x0001

//MII Basic Status register
#define KSZ8873_BMSR_100BT4                          0x8000
#define KSZ8873_BMSR_100BTX_FD                       0x4000
#define KSZ8873_BMSR_100BTX_HD                       0x2000
#define KSZ8873_BMSR_10BT_FD                         0x1000
#define KSZ8873_BMSR_10BT_HD                         0x0800
#define KSZ8873_BMSR_PREAMBLE_SUPPR                  0x0040
#define KSZ8873_BMSR_AN_COMPLETE                     0x0020
#define KSZ8873_BMSR_FAR_END_FAULT                   0x0010
#define KSZ8873_BMSR_AN_CAPABLE                      0x0008
#define KSZ8873_BMSR_LINK_STATUS                     0x0004
#define KSZ8873_BMSR_JABBER_TEST                     0x0002
#define KSZ8873_BMSR_EXTENDED_CAPABLE                0x0001

//PHYID High register
#define KSZ8873_PHYID1_DEFAULT                       0x0022

//PHYID Low register
#define KSZ8873_PHYID2_DEFAULT                       0x1430

//Auto-Negotiation Advertisement Ability register
#define KSZ8873_ANAR_NEXT_PAGE                       0x8000
#define KSZ8873_ANAR_REMOTE_FAULT                    0x2000
#define KSZ8873_ANAR_PAUSE                           0x0400
#define KSZ8873_ANAR_100BTX_FD                       0x0100
#define KSZ8873_ANAR_100BTX_HD                       0x0080
#define KSZ8873_ANAR_10BT_FD                         0x0040
#define KSZ8873_ANAR_10BT_HD                         0x0020
#define KSZ8873_ANAR_SELECTOR                        0x001F
#define KSZ8873_ANAR_SELECTOR_DEFAULT                0x0001

//Auto-Negotiation Link Partner Ability register
#define KSZ8873_ANLPAR_NEXT_PAGE                     0x8000
#define KSZ8873_ANLPAR_LP_ACK                        0x4000
#define KSZ8873_ANLPAR_REMOTE_FAULT                  0x2000
#define KSZ8873_ANLPAR_PAUSE                         0x0400
#define KSZ8873_ANLPAR_100BTX_FD                     0x0100
#define KSZ8873_ANLPAR_100BTX_HD                     0x0080
#define KSZ8873_ANLPAR_10BT_FD                       0x0040
#define KSZ8873_ANLPAR_10BT_HD                       0x0020

//LinkMD Control/Status register
#define KSZ8873_LINKMD_TEST_EN                       0x8000
#define KSZ8873_LINKMD_RESULT                        0x6000
#define KSZ8873_LINKMD_SHORT                         0x1000
#define KSZ8873_LINKMD_FAULT_COUNT                   0x01FF

//PHY Special Control/Status register
#define KSZ8873_PHYSCS_OP_MODE                       0x0700
#define KSZ8873_PHYSCS_OP_MODE_AN                    0x0100
#define KSZ8873_PHYSCS_OP_MODE_10BT_HD               0x0200
#define KSZ8873_PHYSCS_OP_MODE_100BTX_HD             0x0300
#define KSZ8873_PHYSCS_OP_MODE_10BT_FD               0x0500
#define KSZ8873_PHYSCS_OP_MODE_100BTX_FD             0x0600
#define KSZ8873_PHYSCS_OP_MODE_ISOLATE               0x0700
#define KSZ8873_PHYSCS_POLRVS                        0x0020
#define KSZ8873_PHYSCS_MDIX_STATUS                   0x0010
#define KSZ8873_PHYSCS_FORCE_LINK                    0x0008
#define KSZ8873_PHYSCS_PWRSAVE                       0x0004
#define KSZ8873_PHYSCS_REMOTE_LOOPBACK               0x0002

//Chip ID0 register
#define KSZ8873_CHIP_ID0_FAMILY_ID                   0xFF
#define KSZ8873_CHIP_ID0_FAMILY_ID_DEFAULT           0x88

//Chip ID1 / Start Switch register
#define KSZ8873_CHIP_ID1_CHIP_ID                     0xF0
#define KSZ8873_CHIP_ID1_CHIP_ID_DEFAULT             0x30
#define KSZ8873_CHIP_ID1_REVISION_ID                 0x0E
#define KSZ8873_CHIP_ID1_START_SWITCH                0x01

//Global Control 1 register
#define KSZ8873_GLOBAL_CTRL1_PASS_ALL_FRAMES         0x80
#define KSZ8873_GLOBAL_CTRL1_TAIL_TAG_EN             0x40
#define KSZ8873_GLOBAL_CTRL1_TX_FLOW_CTRL_EN         0x20
#define KSZ8873_GLOBAL_CTRL1_RX_FLOW_CTRL_EN         0x10
#define KSZ8873_GLOBAL_CTRL1_FRAME_LEN_CHECK_EN      0x08
#define KSZ8873_GLOBAL_CTRL1_AGING_EN                0x04
#define KSZ8873_GLOBAL_CTRL1_FAST_AGE_EN             0x02
#define KSZ8873_GLOBAL_CTRL1_AGGRESSIVE_BACK_OFF_EN  0x01

//Port N Control 2 register
#define KSZ8873_PORTn_CTRL2_TX_QUEUE_SPLIT_EN        0x80
#define KSZ8873_PORTn_CTRL2_INGRESS_VLAN_FILT        0x40
#define KSZ8873_PORTn_CTRL2_DISCARD_NON_PVID_PACKETS 0x20
#define KSZ8873_PORTn_CTRL2_FORCE_FLOW_CTRL          0x10
#define KSZ8873_PORTn_CTRL2_BACK_PRESSURE_EN         0x08
#define KSZ8873_PORTn_CTRL2_TRANSMIT_EN              0x04
#define KSZ8873_PORTn_CTRL2_RECEIVE_EN               0x02
#define KSZ8873_PORTn_CTRL2_LEARNING_DIS             0x01

//Port N Status 0 register
#define KSZ8873_PORTn_STAT0_MDIX_STATUS              0x80
#define KSZ8873_PORTn_STAT0_AN_DONE                  0x40
#define KSZ8873_PORTn_STAT0_LINK_GOOD                0x20
#define KSZ8873_PORTn_STAT0_LP_FLOW_CTRL_CAPABLE     0x10
#define KSZ8873_PORTn_STAT0_LP_100BTX_FD_CAPABLE     0x08
#define KSZ8873_PORTn_STAT0_LP_100BTX_HF_CAPABLE     0x04
#define KSZ8873_PORTn_STAT0_LP_10BT_FD_CAPABLE       0x02
#define KSZ8873_PORTn_STAT0_LP_10BT_HD_CAPABLE       0x01

//Port N Status 1 register
#define KSZ8873_PORTn_STAT1_HP_MDIX                  0x80
#define KSZ8873_PORTn_STAT1_POLRVS                   0x20
#define KSZ8873_PORTn_STAT1_TX_FLOW_CTRL_EN          0x10
#define KSZ8873_PORTn_STAT1_RX_FLOW_CTRL_EN          0x08
#define KSZ8873_PORTn_STAT1_OP_SPEED                 0x04
#define KSZ8873_PORTn_STAT1_OP_DUPLEX                0x02
#define KSZ8873_PORTn_STAT1_FAR_END_FAULT            0x01

//Tail tag encoding
#define KSZ8873_TAIL_TAG_ENCODE(port) ((port) & 0x03)
//Tail tag decoding
#define KSZ8873_TAIL_TAG_DECODE(tag) (((tag) & 0x01) + 1)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//KSZ8873 Ethernet switch driver
extern const PhyDriver ksz8873PhyDriver;

//KSZ8873 related functions
error_t ksz8873Init(NetInterface *interface);

bool_t ksz8873GetLinkState(NetInterface *interface, uint8_t port);

void ksz8873Tick(NetInterface *interface);

void ksz8873EnableIrq(NetInterface *interface);
void ksz8873DisableIrq(NetInterface *interface);

void ksz8873EventHandler(NetInterface *interface);

error_t ksz8873TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type);

error_t ksz8873UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, uint8_t *port);

void ksz8873WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data);

uint16_t ksz8873ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address);

void ksz8873DumpPhyReg(NetInterface *interface, uint8_t port);

void ksz8873WriteSwitchReg(NetInterface *interface, uint8_t address,
   uint8_t data);

uint8_t ksz8873ReadSwitchReg(NetInterface *interface, uint8_t address);

void ksz8873DumpSwitchReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
