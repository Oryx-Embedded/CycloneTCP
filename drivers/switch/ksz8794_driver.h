/**
 * @file ksz8794_driver.h
 * @brief KSZ8794 Ethernet switch
 *
 * @section License
 *
 * Copyright (C) 2010-2018 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.0
 **/

#ifndef _KSZ8794_DRIVER_H
#define _KSZ8794_DRIVER_H

//Dependencies
#include "core/nic.h"

//KSZ8794 ports
#define KSZ8794_PORT1                       1
#define KSZ8794_PORT2                       2
#define KSZ8794_PORT3                       3

//SPI commands
#define KSZ8794_SPI_CMD_WRITE               0x4000
#define KSZ8794_SPI_CMD_READ                0x6000
#define KSZ8794_SPI_ADDR_MASK               0x1FFE

//KSZ8794 PHY registers
#define KSZ8794_PHY_REG_BMCR                0x00
#define KSZ8794_PHY_REG_BMSR                0x01
#define KSZ8794_PHY_REG_PHYIDR1             0x02
#define KSZ8794_PHY_REG_PHYIDR2             0x03
#define KSZ8794_PHY_REG_ANAR                0x04
#define KSZ8794_PHY_REG_ANLPAR              0x05
#define KSZ8794_PHY_REG_LINKMDCS            0x1D
#define KSZ8794_PHY_REG_PHYSCS              0x1F

//BMCR register
#define BMCR_SOFT_RESET                     (1 << 15)
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
#define LINKMDCS_VCT_EN                     (1 << 15)
#define LINKMDCS_VCT_RESULT1                (1 << 14)
#define LINKMDCS_VCT_RESULT0                (1 << 13)
#define LINKMDCS_VCT_10M_SHORT              (1 << 12)
#define LINKMDCS_VCT_FAULT_COUNT8           (1 << 8)
#define LINKMDCS_VCT_FAULT_COUNT7           (1 << 7)
#define LINKMDCS_VCT_FAULT_COUNT6           (1 << 6)
#define LINKMDCS_VCT_FAULT_COUNT5           (1 << 5)
#define LINKMDCS_VCT_FAULT_COUNT4           (1 << 4)
#define LINKMDCS_VCT_FAULT_COUNT3           (1 << 3)
#define LINKMDCS_VCT_FAULT_COUNT2           (1 << 2)
#define LINKMDCS_VCT_FAULT_COUNT1           (1 << 1)
#define LINKMDCS_VCT_FAULT_COUNT0           (1 << 0)

//PHYSCS register
#define PHYSCS_OP_MODE2                     (1 << 10)
#define PHYSCS_OP_MODE1                     (1 << 9)
#define PHYSCS_OP_MODE0                     (1 << 8)
#define PHYSCS_POLRVS                       (1 << 5)
#define PHYSCS_MDIX_STATUS                  (1 << 4)
#define PHYSCS_FORCE_LINK                   (1 << 3)
#define PHYSCS_PWRSAVE                      (1 << 2)
#define PHYSCS_REMOTE_LOOPBACK              (1 << 1)

//Operation mode indication
#define PHYSCS_OP_MODE_MASK                 (7 << 8)
#define PHYSCS_OP_MODE_AN                   (0 << 8)
#define PHYSCS_OP_MODE_10BT                 (1 << 8)
#define PHYSCS_OP_MODE_100BTX               (2 << 8)
#define PHYSCS_OP_MODE_10BT_FD              (5 << 8)
#define PHYSCS_OP_MODE_100BTX_FD            (6 << 8)

//KSZ8794 switch registers
#define KSZ8794_SW_REG_GLOBAL_CTRL10        12
#define KSZ8794_SW_REG_PORT_CTRL0(n)        (16 + (((n) - 1) * 16))
#define KSZ8794_SW_REG_PORT_CTRL1(n)        (17 + (((n) - 1) * 16))
#define KSZ8794_SW_REG_PORT_CTRL2(n)        (18 + (((n) - 1) * 16))
#define KSZ8794_SW_REG_PORT_STAT2(n)        (30 + (((n) - 1) * 16))
#define KSZ8794_SW_REG_PORT_STAT3(n)        (31 + (((n) - 1) * 16))

//Global control 10 register
#define GLOBAL_CTRL10_TAIL_TAG_EN           (1 << 1)
#define GLOBAL_CTRL10_PASS_FLOW_CTRL_PACKET (1 << 0)

//Port control 2 register
#define PORT_CTRL2_USER_PRIO_CEILING        (1 << 7)
#define PORT_CTRL2_INGRESS_VLAN_FILT        (1 << 6)
#define PORT_CTRL2_DISCARD_NON_PVID_PACKET  (1 << 5)
#define PORT_CTRL2_FORCE_FLOW_CTRL          (1 << 4)
#define PORT_CTRL2_BACK_PRESSURE_EN         (1 << 3)
#define PORT_CTRL2_TRANSMIT_EN              (1 << 2)
#define PORT_CTRL2_RECEIVE_EN               (1 << 1)
#define PORT_CTRL2_LEARNING_DIS             (1 << 0)

//Port status 2 register
#define PORT_STAT2_LINK_MDIX_STATUS         (1 << 7)
#define PORT_STAT2_AUTO_NEGOTIATION_DONE    (1 << 6)
#define PORT_STAT2_LINK_GOOD                (1 << 5)

//Port status 3 register
#define PORT_STAT3_PHY_LOOPBACK             (1 << 7)
#define PORT_STAT3_PHY_PHY_ISOLATE          (1 << 5)
#define PORT_STAT3_PHY_SOFT_RESET           (1 << 4)
#define PORT_STAT3_PHY_FORCE_LINK           (1 << 3)
#define PORT_STAT3_PHY_OP_MODE2             (1 << 2)
#define PORT_STAT3_PHY_OP_MODE1             (1 << 1)
#define PORT_STAT3_PHY_OP_MODE0             (1 << 0)

//Operation mode indication
#define PORT_STAT3_PHY_OP_MODE_MASK         (7 << 0)
#define PORT_STAT3_PHY_OP_MODE_AN           (1 << 0)
#define PORT_STAT3_PHY_OP_MODE_10BT         (2 << 0)
#define PORT_STAT3_PHY_OP_MODE_100BTX       (3 << 0)
#define PORT_STAT3_PHY_OP_MODE_10BT_FD      (5 << 0)
#define PORT_STAT3_PHY_OP_MODE_100BTX_FD    (6 << 0)

//Tail tag encoding
#define KSZ8794_TAIL_TAG_ENCODE(port) (0x40 | (1 << (((port) - 1) & 0x03)))
//Tail tag decoding
#define KSZ8794_TAIL_TAG_DECODE(tag) (((tag) & 0x03) + 1)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//KSZ8794 Ethernet switch driver
extern const PhyDriver ksz8794PhyDriver;

//KSZ8794 related functions
error_t ksz8794Init(NetInterface *interface);

bool_t ksz8794GetLinkState(NetInterface *interface, uint8_t port);

void ksz8794Tick(NetInterface *interface);

void ksz8794EnableIrq(NetInterface *interface);
void ksz8794DisableIrq(NetInterface *interface);

void ksz8794EventHandler(NetInterface *interface);

error_t ksz8794TagFrame(NetInterface **interface, NetBuffer *buffer,
   size_t *offset, uint16_t *type);

error_t ksz8794UntagFrame(NetInterface **interface, uint8_t **frame,
   size_t *length);

void ksz8794WritePhyReg(NetInterface *interface,
   uint8_t port, uint8_t address, uint16_t data);

uint16_t ksz8794ReadPhyReg(NetInterface *interface,
   uint8_t port, uint8_t address);

void ksz8794DumpPhyReg(NetInterface *interface, uint8_t port);

void ksz8794WriteSwitchReg(NetInterface *interface,
   uint16_t address, uint8_t data);

uint8_t ksz8794ReadSwitchReg(NetInterface *interface,
   uint16_t address);

void ksz8794DumpSwitchReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
