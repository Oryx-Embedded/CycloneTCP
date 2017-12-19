/**
 * @file ksz8895_driver.h
 * @brief KSZ8895 Ethernet switch
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.8.0
 **/

#ifndef _KSZ8895_DRIVER_H
#define _KSZ8895_DRIVER_H

//Dependencies
#include "core/nic.h"

//KSZ8895 ports
#define KSZ8895_PORT1               1
#define KSZ8895_PORT2               2
#define KSZ8895_PORT3               3
#define KSZ8895_PORT4               4

//KSZ8895 registers
#define KSZ8895_PHY_REG_BMCR        0x00
#define KSZ8895_PHY_REG_BMSR        0x01
#define KSZ8895_PHY_REG_PHYIDR1     0x02
#define KSZ8895_PHY_REG_PHYIDR2     0x03
#define KSZ8895_PHY_REG_ANAR        0x04
#define KSZ8895_PHY_REG_ANLPAR      0x05
#define KSZ8895_PHY_REG_LINKMDCS    0x1D
#define KSZ8895_PHY_REG_PHYSCS      0x1F

//BMCR register
#define BMCR_SOFT_RESET             (1 << 15)
#define BMCR_LOOPBACK               (1 << 14)
#define BMCR_FORCE_100              (1 << 13)
#define BMCR_AN_EN                  (1 << 12)
#define BMCR_POWER_DOWN             (1 << 11)
#define BMCR_ISOLATE                (1 << 10)
#define BMCR_RESTART_AN             (1 << 9)
#define BMCR_FORCE_FULL_DUPLEX      (1 << 8)
#define BMCR_COL_TEST               (1 << 7)
#define BMCR_HP_MDIX                (1 << 5)
#define BMCR_FORCE_MDI              (1 << 4)
#define BMCR_DIS_AUTO_MDIX          (1 << 3)
#define BMCR_DIS_FAR_END_FAULT      (1 << 2)
#define BMCR_DIS_TRANSMIT           (1 << 1)
#define BMCR_DIS_LED                (1 << 0)

//BMSR register
#define BMSR_100BT4                 (1 << 15)
#define BMSR_100BTX_FD              (1 << 14)
#define BMSR_100BTX                 (1 << 13)
#define BMSR_10BT_FD                (1 << 12)
#define BMSR_10BT                   (1 << 11)
#define BMSR_NO_PREAMBLE            (1 << 6)
#define BMSR_AN_COMPLETE            (1 << 5)
#define BMSR_FAR_END_FAULT          (1 << 4)
#define BMSR_AN_ABLE                (1 << 3)
#define BMSR_LINK_STATUS            (1 << 2)
#define BMSR_JABBER_TEST            (1 << 1)
#define BMSR_EXTENDED_CAP           (1 << 0)

//ANAR register
#define ANAR_NEXT_PAGE              (1 << 15)
#define ANAR_REMOTE_FAULT           (1 << 13)
#define ANAR_PAUSE                  (1 << 10)
#define ANAR_100BTX_FD              (1 << 8)
#define ANAR_100BTX                 (1 << 7)
#define ANAR_10BT_FD                (1 << 6)
#define ANAR_10BT                   (1 << 5)
#define ANAR_SELECTOR4              (1 << 4)
#define ANAR_SELECTOR3              (1 << 3)
#define ANAR_SELECTOR2              (1 << 2)
#define ANAR_SELECTOR1              (1 << 1)
#define ANAR_SELECTOR0              (1 << 0)

//ANLPAR register
#define ANLPAR_NEXT_PAGE            (1 << 15)
#define ANLPAR_LP_ACK               (1 << 14)
#define ANLPAR_REMOTE_FAULT         (1 << 13)
#define ANLPAR_PAUSE                (1 << 10)
#define ANLPAR_100BTX_FD            (1 << 8)
#define ANLPAR_100BTX               (1 << 7)
#define ANLPAR_10BT_FD              (1 << 6)
#define ANLPAR_10BT                 (1 << 5)

//LINKMDCS register
#define LINKMDCS_VCT_EN             (1 << 15)
#define LINKMDCS_VCT_RESULT1        (1 << 14)
#define LINKMDCS_VCT_RESULT0        (1 << 13)
#define LINKMDCS_VCT_10M_SHORT      (1 << 12)
#define LINKMDCS_VCT_FAULT_COUNT8   (1 << 8)
#define LINKMDCS_VCT_FAULT_COUNT7   (1 << 7)
#define LINKMDCS_VCT_FAULT_COUNT6   (1 << 6)
#define LINKMDCS_VCT_FAULT_COUNT5   (1 << 5)
#define LINKMDCS_VCT_FAULT_COUNT4   (1 << 4)
#define LINKMDCS_VCT_FAULT_COUNT3   (1 << 3)
#define LINKMDCS_VCT_FAULT_COUNT2   (1 << 2)
#define LINKMDCS_VCT_FAULT_COUNT1   (1 << 1)
#define LINKMDCS_VCT_FAULT_COUNT0   (1 << 0)

//PHYSCS register
#define PHYSCS_OP_MODE2             (1 << 10)
#define PHYSCS_OP_MODE1             (1 << 9)
#define PHYSCS_OP_MODE0             (1 << 8)
#define PHYSCS_POLRVS               (1 << 5)
#define PHYSCS_MDIX_STATUS          (1 << 4)
#define PHYSCS_FORCE_LINK           (1 << 3)
#define PHYSCS_PWRSAVE              (1 << 2)
#define PHYSCS_REMOTE_LOOPBACK      (1 << 1)

//Operation mode indication
#define PHYCON1_OP_MODE_MASK        (7 << 8)
#define PHYCON1_OP_MODE_AN          (0 << 8)
#define PHYCON1_OP_MODE_10BT        (1 << 8)
#define PHYCON1_OP_MODE_100BTX      (2 << 8)
#define PHYCON1_OP_MODE_10BT_FD     (5 << 8)
#define PHYCON1_OP_MODE_100BTX_FD   (6 << 8)

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

void ksz8895WritePhyReg(NetInterface *interface,
   uint8_t port, uint8_t address, uint16_t data);

uint16_t ksz8895ReadPhyReg(NetInterface *interface,
   uint8_t port, uint8_t address);

void ksz8895DumpPhyReg(NetInterface *interface, uint8_t port);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
