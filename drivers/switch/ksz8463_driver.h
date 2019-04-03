/**
 * @file ksz8463_driver.h
 * @brief KSZ8463 Ethernet switch
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
 * @version 1.9.2
 **/

#ifndef _KSZ8463_DRIVER_H
#define _KSZ8463_DRIVER_H

//Dependencies
#include "core/nic.h"

//KSZ8463 ports
#define KSZ8463_PORT1                       1
#define KSZ8463_PORT2                       2

//SPI command byte
#define KSZ8463_SPI_CMD_READ                0x0000
#define KSZ8463_SPI_CMD_WRITE               0x8000
#define KSZ8463_SPI_CMD_ADDR                0x7FC0
#define KSZ8463_SPI_CMD_B3                  0x0020
#define KSZ8463_SPI_CMD_B2                  0x0010
#define KSZ8463_SPI_CMD_B1                  0x0008
#define KSZ8463_SPI_CMD_B0                  0x0004

//KSZ8463 PHY registers
#define KSZ8463_PHY_REG_BMCR                0x00
#define KSZ8463_PHY_REG_BMSR                0x01
#define KSZ8463_PHY_REG_PHYIDR1             0x02
#define KSZ8463_PHY_REG_PHYIDR2             0x03
#define KSZ8463_PHY_REG_ANAR                0x04
#define KSZ8463_PHY_REG_ANLPAR              0x05
#define KSZ8463_PHY_REG_LINKMDCS            0x1D
#define KSZ8463_PHY_REG_PHYSCS              0x1F

//BMCR register
#define BMCR_LOOPBACK                       (1 << 14)
#define BMCR_FORCE_100                      (1 << 13)
#define BMCR_AN_EN                          (1 << 12)
#define BMCR_POWER_DOWN                     (1 << 11)
#define BMCR_ISOLATE                        (1 << 10)
#define BMCR_RESTART_AN                     (1 << 9)
#define BMCR_FORCE_FULL_DUPLEX              (1 << 8)
#define BMCR_COL_TEST                       (1 << 7)
#define BMCR_HP_MDIX                        (1 << 5)
#define BMCR_FORCE_MDI                      (1 << 4)
#define BMCR_DIS_AUTO_MDIX                  (1 << 3)
#define BMCR_DIS_FAR_END_FAULT              (1 << 2)
#define BMCR_DIS_TRANSMIT                   (1 << 1)
#define BMCR_DIS_LED                        (1 << 0)

//BMSR register
#define BMSR_100BT4                         (1 << 15)
#define BMSR_100BTX_FD                      (1 << 14)
#define BMSR_100BTX                         (1 << 13)
#define BMSR_10BT_FD                        (1 << 12)
#define BMSR_10BT                           (1 << 11)
#define BMSR_NO_PREAMBLE                    (1 << 6)
#define BMSR_AN_COMPLETE                    (1 << 5)
#define BMSR_FAR_END_FAULT                  (1 << 4)
#define BMSR_AN_ABLE                        (1 << 3)
#define BMSR_LINK_STATUS                    (1 << 2)
#define BMSR_JABBER_TEST                    (1 << 1)
#define BMSR_EXTENDED_CAP                   (1 << 0)

//ANAR register
#define ANAR_NEXT_PAGE                      (1 << 15)
#define ANAR_REMOTE_FAULT                   (1 << 13)
#define ANAR_PAUSE                          (1 << 10)
#define ANAR_100BTX_FD                      (1 << 8)
#define ANAR_100BTX                         (1 << 7)
#define ANAR_10BT_FD                        (1 << 6)
#define ANAR_10BT                           (1 << 5)
#define ANAR_SELECTOR4                      (1 << 4)
#define ANAR_SELECTOR3                      (1 << 3)
#define ANAR_SELECTOR2                      (1 << 2)
#define ANAR_SELECTOR1                      (1 << 1)
#define ANAR_SELECTOR0                      (1 << 0)

//ANLPAR register
#define ANLPAR_NEXT_PAGE                    (1 << 15)
#define ANLPAR_LP_ACK                       (1 << 14)
#define ANLPAR_REMOTE_FAULT                 (1 << 13)
#define ANLPAR_PAUSE                        (1 << 10)
#define ANLPAR_100BTX_FD                    (1 << 8)
#define ANLPAR_100BTX                       (1 << 7)
#define ANLPAR_10BT_FD                      (1 << 6)
#define ANLPAR_10BT                         (1 << 5)

//LINKMDCS register
#define LINKMDCS_CDT_EN                     (1 << 15)
#define LINKMDCS_CDT_RESULT1                (1 << 14)
#define LINKMDCS_CDT_RESULT0                (1 << 13)
#define LINKMDCS_CDT_10M_SHORT              (1 << 12)
#define LINKMDCS_CDT_FAULT_COUNT8           (1 << 8)
#define LINKMDCS_CDT_FAULT_COUNT7           (1 << 7)
#define LINKMDCS_CDT_FAULT_COUNT6           (1 << 6)
#define LINKMDCS_CDT_FAULT_COUNT5           (1 << 5)
#define LINKMDCS_CDT_FAULT_COUNT4           (1 << 4)
#define LINKMDCS_CDT_FAULT_COUNT3           (1 << 3)
#define LINKMDCS_CDT_FAULT_COUNT2           (1 << 2)
#define LINKMDCS_CDT_FAULT_COUNT1           (1 << 1)
#define LINKMDCS_CDT_FAULT_COUNT0           (1 << 0)

//PHYSCS register
#define PHYSCS_POL_REVERSE                  (1 << 5)
#define PHYSCS_MDIX_STATUS                  (1 << 4)
#define PHYSCS_FORCE_LINK                   (1 << 3)
#define PHYSCS_EEE_EN                       (1 << 2)
#define PHYSCS_REMOTE_LOOPBACK              (1 << 1)

//KSZ8463 switch registers
#define KSZ8463_SW_REG_PORT_CTRL2(n)        (0x006E + (((n) - 1) * 0x18))
#define KSZ8463_SW_REG_PORT_STAT(n)         (0x0080 + (((n) - 1) * 0x18))
#define KSZ8463_SW_REG_GLOBAL_CTRL8         0x00AC

//Port control 2 register
#define PORT_CTRL2_INGRESS_VLAN_FILT        (1 << 14)
#define PORT_CTRL2_DISCARD_NON_PVID_PACKETS (1 << 13)
#define PORT_CTRL2_FORCE_FLOW_CTRL          (1 << 12)
#define PORT_CTRL2_BACK_PRESSURE_EN         (1 << 11)
#define PORT_CTRL2_TRANSMIT_EN              (1 << 10)
#define PORT_CTRL2_RECEIVE_EN               (1 << 9)
#define PORT_CTRL2_LEARNING_DIS             (1 << 8)
#define PORT_CTRL2_SNIFFER_PORT             (1 << 7)
#define PORT_CTRL2_RECEIVE_SNIFF            (1 << 6)
#define PORT_CTRL2_TRANSMIT_SNIFF           (1 << 5)
#define PORT_CTRL2_USER_PRIO_CEILING        (1 << 3)
#define PORT_CTRL2_PORT_VLAN_MEMBERSHIP2    (1 << 2)
#define PORT_CTRL2_PORT_VLAN_MEMBERSHIP1    (1 << 1)
#define PORT_CTRL2_PORT_VLAN_MEMBERSHIP0    (1 << 0)

//Port status register
#define PORT_STAT_HP_MDIX                   (1 << 15)
#define PORT_STAT_POL_REVERSE               (1 << 13)
#define PORT_STAT_TX_FLOW_CTRL_EN           (1 << 12)
#define PORT_STAT_RX_FLOW_CTRL_EN           (1 << 11)
#define PORT_STAT_OP_SPEED                  (1 << 10)
#define PORT_STAT_OP_MODE                   (1 << 9)
#define PORT_STAT_FAR_END_FAULT             (1 << 8)
#define PORT_STAT_MDIX_STATUS               (1 << 7)
#define PORT_STAT_AN_DONE                   (1 << 6)
#define PORT_STAT_LINK_STATUS               (1 << 5)
#define PORT_STAT_LP_FLOW_CTRL_CAPABLE      (1 << 4)
#define PORT_STAT_100BTX_FD_CAPABLE         (1 << 3)
#define PORT_STAT_100BTX_HF_CAPABLE         (1 << 2)
#define PORT_STAT_10BT_FD_CAPABLE           (1 << 1)
#define PORT_STAT_10BT_HD_CAPABLE           (1 << 0)

//Global control 8 register
#define GLOBAL_CTRL8_QUEUE_PRIO_MAPPING1      (1 << 15)
#define GLOBAL_CTRL8_QUEUE_PRIO_MAPPING0      (1 << 14)
#define GLOBAL_CTRL8_FLUSH_DYNAMIC_MAC_TABLE  (1 << 10)
#define GLOBAL_CTRL8_FLUSH_STATIC_MAC_TABLE   (1 << 9)
#define GLOBAL_CTRL8_TAIL_TAG_EN              (1 << 8)
#define GLOBAL_CTRL8_PAUSE_OFF_TIME7          (1 << 7)
#define GLOBAL_CTRL8_PAUSE_OFF_TIME6          (1 << 6)
#define GLOBAL_CTRL8_PAUSE_OFF_TIME5          (1 << 5)
#define GLOBAL_CTRL8_PAUSE_OFF_TIME4          (1 << 4)
#define GLOBAL_CTRL8_PAUSE_OFF_TIME3          (1 << 3)
#define GLOBAL_CTRL8_PAUSE_OFF_TIME2          (1 << 2)
#define GLOBAL_CTRL8_PAUSE_OFF_TIME1          (1 << 1)
#define GLOBAL_CTRL8_PAUSE_OFF_TIME0          (1 << 0)

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
