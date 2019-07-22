/**
 * @file ksz8895_driver.h
 * @brief KSZ8895 5-port Ethernet switch
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

#ifndef _KSZ8895_DRIVER_H
#define _KSZ8895_DRIVER_H

//Dependencies
#include "core/nic.h"

//KSZ8895 ports
#define KSZ8895_PORT1 1
#define KSZ8895_PORT2 2
#define KSZ8895_PORT3 3
#define KSZ8895_PORT4 4

//SPI command byte
#define KSZ8895_SPI_CMD_WRITE 0x02
#define KSZ8895_SPI_CMD_READ  0x03

//KSZ8895 PHY registers
#define KSZ8895_BMCR                                 0x00
#define KSZ8895_BMSR                                 0x01
#define KSZ8895_PHYID1                               0x02
#define KSZ8895_PHYID2                               0x03
#define KSZ8895_ANAR                                 0x04
#define KSZ8895_ANLPAR                               0x05
#define KSZ8895_LINKMD                               0x1D
#define KSZ8895_PHYSCS                               0x1F

//KSZ8895 Switch registers
#define KSZ8895_CHIP_ID0                             0x00
#define KSZ8895_CHIP_ID1                             0x01
#define KSZ8895_GLOBAL_CTRL10                        0x0C
#define KSZ8895_PORT1_CTRL0                          0x10
#define KSZ8895_PORT1_CTRL1                          0x11
#define KSZ8895_PORT1_CTRL2                          0x12
#define KSZ8895_PORT1_STAT0                          0x19
#define KSZ8895_PORT1_STAT1                          0x1E
#define KSZ8895_PORT1_CTRL7_STAT2                    0x1F
#define KSZ8895_PORT2_CTRL0                          0x20
#define KSZ8895_PORT2_CTRL1                          0x21
#define KSZ8895_PORT2_CTRL2                          0x22
#define KSZ8895_PORT2_STAT0                          0x29
#define KSZ8895_PORT2_STAT1                          0x2E
#define KSZ8895_PORT2_CTRL7_STAT2                    0x2F
#define KSZ8895_PORT3_CTRL0                          0x30
#define KSZ8895_PORT3_CTRL1                          0x31
#define KSZ8895_PORT3_CTRL2                          0x32
#define KSZ8895_PORT3_STAT0                          0x39
#define KSZ8895_PORT3_STAT1                          0x3E
#define KSZ8895_PORT3_CTRL7_STAT2                    0x3F
#define KSZ8895_PORT4_CTRL0                          0x40
#define KSZ8895_PORT4_CTRL1                          0x41
#define KSZ8895_PORT4_CTRL2                          0x42
#define KSZ8895_PORT4_STAT0                          0x49
#define KSZ8895_PORT4_STAT1                          0x4E
#define KSZ8895_PORT4_CTRL7_STAT2                    0x4F
#define KSZ8895_PORT5_CTRL0                          0x50
#define KSZ8895_PORT5_CTRL1                          0x51
#define KSZ8895_PORT5_CTRL2                          0x52
#define KSZ8895_PORT5_STAT0                          0x59
#define KSZ8895_PORT5_STAT1                          0x5E
#define KSZ8895_PORT5_CTRL7_STAT2                    0x5F

//KSZ8895 Switch register access macros
#define KSZ8895_PORTn_CTRL0(port)                    (0x00 + ((port) * 0x10))
#define KSZ8895_PORTn_CTRL1(port)                    (0x01 + ((port) * 0x10))
#define KSZ8895_PORTn_CTRL2(port)                    (0x02 + ((port) * 0x10))
#define KSZ8895_PORTn_STAT0(port)                    (0x09 + ((port) * 0x10))
#define KSZ8895_PORTn_STAT1(port)                    (0x0E + ((port) * 0x10))
#define KSZ8895_PORTn_CTRL7_STAT2(port)              (0x0F + ((port) * 0x10))

//MII Control register
#define KSZ8895_BMCR_RESET                           0x8000
#define KSZ8895_BMCR_LOOPBACK                        0x4000
#define KSZ8895_BMCR_FORCE_100                       0x2000
#define KSZ8895_BMCR_AN_EN                           0x1000
#define KSZ8895_BMCR_POWER_DOWN                      0x0800
#define KSZ8895_BMCR_ISOLATE                         0x0400
#define KSZ8895_BMCR_RESTART_AN                      0x0200
#define KSZ8895_BMCR_FORCE_FULL_DUPLEX               0x0100
#define KSZ8895_BMCR_COL_TEST                        0x0080
#define KSZ8895_BMCR_HP_MDIX                         0x0020
#define KSZ8895_BMCR_FORCE_MDI                       0x0010
#define KSZ8895_BMCR_AUTO_MDIX_DIS                   0x0008
#define KSZ8895_BMCR_FAR_END_FAULT_DIS               0x0004
#define KSZ8895_BMCR_TRANSMIT_DIS                    0x0002
#define KSZ8895_BMCR_LED_DIS                         0x0001

//MII Status register
#define KSZ8895_BMSR_100BT4                          0x8000
#define KSZ8895_BMSR_100BTX_FD                       0x4000
#define KSZ8895_BMSR_100BTX_HD                       0x2000
#define KSZ8895_BMSR_10BT_FD                         0x1000
#define KSZ8895_BMSR_10BT_HD                         0x0800
#define KSZ8895_BMSR_PREAMBLE_SUPPR                  0x0040
#define KSZ8895_BMSR_AN_COMPLETE                     0x0020
#define KSZ8895_BMSR_FAR_END_FAULT                   0x0010
#define KSZ8895_BMSR_AN_CAPABLE                      0x0008
#define KSZ8895_BMSR_LINK_STATUS                     0x0004
#define KSZ8895_BMSR_JABBER_TEST                     0x0002
#define KSZ8895_BMSR_EXTENDED_CAPABLE                0x0001

//PHYID High register
#define KSZ8895_PHYID1_DEFAULT                       0x0022

//PHYID Low register
#define KSZ8895_PHYID2_DEFAULT                       0x1450

//Advertisement Ability register
#define KSZ8895_ANAR_NEXT_PAGE                       0x8000
#define KSZ8895_ANAR_REMOTE_FAULT                    0x2000
#define KSZ8895_ANAR_PAUSE                           0x0400
#define KSZ8895_ANAR_100BTX_FD                       0x0100
#define KSZ8895_ANAR_100BTX_HD                       0x0080
#define KSZ8895_ANAR_10BT_FD                         0x0040
#define KSZ8895_ANAR_10BT_HD                         0x0020
#define KSZ8895_ANAR_SELECTOR                        0x001F
#define KSZ8895_ANAR_SELECTOR_DEFAULT                0x0001

//Link Partner Ability register
#define KSZ8895_ANLPAR_NEXT_PAGE                     0x8000
#define KSZ8895_ANLPAR_LP_ACK                        0x4000
#define KSZ8895_ANLPAR_REMOTE_FAULT                  0x2000
#define KSZ8895_ANLPAR_PAUSE                         0x0400
#define KSZ8895_ANLPAR_100BTX_FD                     0x0100
#define KSZ8895_ANLPAR_100BTX_HD                     0x0080
#define KSZ8895_ANLPAR_10BT_FD                       0x0040
#define KSZ8895_ANLPAR_10BT_HD                       0x0020

//LinkMD Control/Status register
#define KSZ8895_LINKMD_TEST_EN                       0x8000
#define KSZ8895_LINKMD_RESULT                        0x6000
#define KSZ8895_LINKMD_SHORT                         0x1000
#define KSZ8895_LINKMD_FAULT_COUNT                   0x01FF

//PHY Special Control/Status register
#define KSZ8895_PHYSCS_OP_MODE                       0x0700
#define KSZ8895_PHYSCS_OP_MODE_AN                    0x0100
#define KSZ8895_PHYSCS_OP_MODE_10BT_HD               0x0200
#define KSZ8895_PHYSCS_OP_MODE_100BTX_HD             0x0300
#define KSZ8895_PHYSCS_OP_MODE_10BT_FD               0x0500
#define KSZ8895_PHYSCS_OP_MODE_100BTX_FD             0x0600
#define KSZ8895_PHYSCS_OP_MODE_ISOLATE               0x0700
#define KSZ8895_PHYSCS_POLRVS                        0x0020
#define KSZ8895_PHYSCS_MDIX_STATUS                   0x0010
#define KSZ8895_PHYSCS_FORCE_LINK                    0x0008
#define KSZ8895_PHYSCS_PWRSAVE                       0x0004
#define KSZ8895_PHYSCS_REMOTE_LOOPBACK               0x0002

//Chip ID0 register
#define KSZ8895_CHIP_ID0_FAMILY_ID                   0xFF
#define KSZ8895_CHIP_ID0_FAMILY_ID_DEFAULT           0x95

//Chip ID1 / Start Switch register
#define KSZ8895_CHIP_ID1_CHIP_ID                     0xF0
#define KSZ8895_CHIP_ID1_CHIP_ID_MQX_FQX_MLX         0x40
#define KSZ8895_CHIP_ID1_CHIP_ID_RQX                 0x60
#define KSZ8895_CHIP_ID1_REVISION_ID                 0x0E
#define KSZ8895_CHIP_ID1_START_SWITCH                0x01

//Global Control 10 register
#define KSZ8895_GLOBAL_CTRL10_CLK_SRC                0x40
#define KSZ8895_GLOBAL_CTRL10_CPU_CLK_SEL            0x30
#define KSZ8895_GLOBAL_CTRL10_RESTORE_PREAMBLE_EN    0x04
#define KSZ8895_GLOBAL_CTRL10_TAIL_TAG_EN            0x02
#define KSZ8895_GLOBAL_CTRL10_PASS_FLOW_CTRL_PACKET  0x01

//Port N Control 2 register
#define KSZ8895_PORTn_CTRL2_USER_PRIO_CEILING        0x80
#define KSZ8895_PORTn_CTRL2_INGRESS_VLAN_FILT        0x40
#define KSZ8895_PORTn_CTRL2_DISCARD_NON_PVID_PACKETS 0x20
#define KSZ8895_PORTn_CTRL2_FORCE_FLOW_CTRL          0x10
#define KSZ8895_PORTn_CTRL2_BACK_PRESSURE_EN         0x08
#define KSZ8895_PORTn_CTRL2_TRANSMIT_EN              0x04
#define KSZ8895_PORTn_CTRL2_RECEIVE_EN               0x02
#define KSZ8895_PORTn_CTRL2_LEARNING_DIS             0x01

//Port N Status 0 register
#define KSZ8895_PORTn_STAT0_HP_MDIX                  0x80
#define KSZ8895_PORTn_STAT0_POLRVS                   0x20
#define KSZ8895_PORTn_STAT0_TX_FLOW_CTRL_EN          0x10
#define KSZ8895_PORTn_STAT0_RX_FLOW_CTRL_EN          0x08
#define KSZ8895_PORTn_STAT0_OP_SPEED                 0x04
#define KSZ8895_PORTn_STAT0_OP_DUPLEX                0x02

//Port N Status 1 register
#define KSZ8895_PORTn_STAT1_MDIX_STATUS              0x80
#define KSZ8895_PORTn_STAT1_AN_DONE                  0x40
#define KSZ8895_PORTn_STAT1_LINK_GOOD                0x20
#define KSZ8895_PORTn_STAT1_LP_FLOW_CTRL_CAPABLE     0x10
#define KSZ8895_PORTn_STAT1_LP_100BTX_FD_CAPABLE     0x08
#define KSZ8895_PORTn_STAT1_LP_100BTX_HF_CAPABLE     0x04
#define KSZ8895_PORTn_STAT1_LP_10BT_FD_CAPABLE       0x02
#define KSZ8895_PORTn_STAT1_LP_10BT_HD_CAPABLE       0x01

//Port N Control 7 / Status 2 register
#define KSZ8895_PORTn_CTRL7_STAT2_PHY_LOOPBACK       0x80
#define KSZ8895_PORTn_CTRL7_STAT2_PHY_ISOLATE        0x20
#define KSZ8895_PORTn_CTRL7_STAT2_SOFT_RESET         0x10
#define KSZ8895_PORTn_CTRL7_STAT2_FORCE_LINK         0x08
#define KSZ8895_PORTn_CTRL7_STAT2_OP_MODE            0x07
#define KSZ8895_PORTn_CTRL7_STAT2_OP_MODE_AN         0x01
#define KSZ8895_PORTn_CTRL7_STAT2_OP_MODE_10BT_HD    0x02
#define KSZ8895_PORTn_CTRL7_STAT2_OP_MODE_100BTX_HD  0x03
#define KSZ8895_PORTn_CTRL7_STAT2_OP_MODE_10BT_FD    0x05
#define KSZ8895_PORTn_CTRL7_STAT2_OP_MODE_100BTX_FD  0x06

//Tail tag encoding
#define KSZ8895_TAIL_TAG_ENCODE(port) (0x40 | (1 << (((port) - 1) & 0x03)))
//Tail tag decoding
#define KSZ8895_TAIL_TAG_DECODE(tag) (((tag) & 0x03) + 1)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//KSZ8895 Ethernet switch driver
extern const PhyDriver ksz8895PhyDriver;

//KSZ8895 related functions
error_t ksz8895Init(NetInterface *interface);

bool_t ksz8895GetLinkState(NetInterface *interface, uint8_t port);

void ksz8895Tick(NetInterface *interface);

void ksz8895EnableIrq(NetInterface *interface);
void ksz8895DisableIrq(NetInterface *interface);

void ksz8895EventHandler(NetInterface *interface);

error_t ksz8895TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type);

error_t ksz8895UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, uint8_t *port);

void ksz8895WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data);

uint16_t ksz8895ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address);

void ksz8895DumpPhyReg(NetInterface *interface, uint8_t port);

void ksz8895WriteSwitchReg(NetInterface *interface, uint8_t address,
   uint8_t data);

uint8_t ksz8895ReadSwitchReg(NetInterface *interface, uint8_t address);

void ksz8895DumpSwitchReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
