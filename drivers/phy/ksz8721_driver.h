/**
 * @file ksz8721_driver.h
 * @brief KSZ8721 Ethernet PHY transceiver
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

#ifndef _KSZ8721_DRIVER_H
#define _KSZ8721_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef KSZ8721_PHY_ADDR
   #define KSZ8721_PHY_ADDR 1
#elif (KSZ8721_PHY_ADDR < 0 || KSZ8721_PHY_ADDR > 31)
   #error KSZ8721_PHY_ADDR parameter is not valid
#endif

//KSZ8721 registers
#define KSZ8721_PHY_REG_BMCR     0x00
#define KSZ8721_PHY_REG_BMSR     0x01
#define KSZ8721_PHY_REG_PHYIDR1  0x02
#define KSZ8721_PHY_REG_PHYIDR2  0x03
#define KSZ8721_PHY_REG_ANAR     0x04
#define KSZ8721_PHY_REG_ANLPAR   0x05
#define KSZ8721_PHY_REG_ANER     0x06
#define KSZ8721_PHY_REG_ANNPTR   0x07
#define KSZ8721_PHY_REG_LPNPAR   0x08
#define KSZ8721_PHY_REG_RECR     0x15
#define KSZ8721_PHY_REG_ICSR     0x1B
#define KSZ8721_PHY_REG_PHYCON   0x1F

//BMCR register
#define BMCR_RESET               (1 << 15)
#define BMCR_LOOPBACK            (1 << 14)
#define BMCR_SPEED_SEL           (1 << 13)
#define BMCR_AN_EN               (1 << 12)
#define BMCR_POWER_DOWN          (1 << 11)
#define BMCR_ISOLATE             (1 << 10)
#define BMCR_RESTART_AN          (1 << 9)
#define BMCR_DUPLEX_MODE         (1 << 8)
#define BMCR_COL_TEST            (1 << 7)
#define BMCR_TX_DIS              (1 << 0)

//BMSR register
#define BMSR_100BT4              (1 << 15)
#define BMSR_100BTX_FD           (1 << 14)
#define BMSR_100BTX              (1 << 13)
#define BMSR_10BT_FD             (1 << 12)
#define BMSR_10BT                (1 << 11)
#define BMSR_NO_PREAMBLE         (1 << 6)
#define BMSR_AN_COMPLETE         (1 << 5)
#define BMSR_REMOTE_FAULT        (1 << 4)
#define BMSR_AN_ABLE             (1 << 3)
#define BMSR_LINK_STATUS         (1 << 2)
#define BMSR_JABBER_DETECT       (1 << 1)
#define BMSR_EXTENDED_CAP        (1 << 0)

//ANAR register
#define ANAR_NEXT_PAGE           (1 << 15)
#define ANAR_REMOTE_FAULT        (1 << 13)
#define ANAR_PAUSE               (1 << 10)
#define ANAR_100BT4              (1 << 9)
#define ANAR_100BTX_FD           (1 << 8)
#define ANAR_100BTX              (1 << 7)
#define ANAR_10BT_FD             (1 << 6)
#define ANAR_10BT                (1 << 5)
#define ANAR_SELECTOR4           (1 << 4)
#define ANAR_SELECTOR3           (1 << 3)
#define ANAR_SELECTOR2           (1 << 2)
#define ANAR_SELECTOR1           (1 << 1)
#define ANAR_SELECTOR0           (1 << 0)

//ANLPAR register
#define ANLPAR_NEXT_PAGE         (1 << 15)
#define ANLPAR_LP_ACK            (1 << 14)
#define ANLPAR_REMOTE_FAULT      (1 << 13)
#define ANLPAR_PAUSE1            (1 << 11)
#define ANLPAR_PAUSE0            (1 << 10)
#define ANLPAR_100BT4            (1 << 9)
#define ANLPAR_100BTX_FD         (1 << 8)
#define ANLPAR_100BTX            (1 << 7)
#define ANLPAR_10BT_FD           (1 << 6)
#define ANLPAR_10BT              (1 << 5)
#define ANLPAR_SELECTOR4         (1 << 4)
#define ANLPAR_SELECTOR3         (1 << 3)
#define ANLPAR_SELECTOR2         (1 << 2)
#define ANLPAR_SELECTOR1         (1 << 1)
#define ANLPAR_SELECTOR0         (1 << 0)

//ANER register
#define ANER_PAR_DET_FAULT       (1 << 4)
#define ANER_LP_NEXT_PAGE_ABLE   (1 << 3)
#define ANER_NEXT_PAGE_ABLE      (1 << 2)
#define ANER_PAGE_RECEIVED       (1 << 1)
#define ANER_LP_AN_ABLE          (1 << 0)

//ANNPTR register
#define ANNPTR_NEXT_PAGE         (1 << 15)
#define ANNPTR_MSG_PAGE          (1 << 13)
#define ANNPTR_ACK2              (1 << 12)
#define ANNPTR_TOGGLE            (1 << 11)
#define ANNPTR_MESSAGE10         (1 << 10)
#define ANNPTR_MESSAGE9          (1 << 9)
#define ANNPTR_MESSAGE8          (1 << 8)
#define ANNPTR_MESSAGE7          (1 << 7)
#define ANNPTR_MESSAGE6          (1 << 6)
#define ANNPTR_MESSAGE5          (1 << 5)
#define ANNPTR_MESSAGE4          (1 << 4)
#define ANNPTR_MESSAGE3          (1 << 3)
#define ANNPTR_MESSAGE2          (1 << 2)
#define ANNPTR_MESSAGE1          (1 << 1)
#define ANNPTR_MESSAGE0          (1 << 0)

//ICSR register
#define ICSR_JABBER_IE           (1 << 15)
#define ICSR_RECEIVE_ERROR_IE    (1 << 14)
#define ICSR_PAGE_RECEIVED_IE    (1 << 13)
#define ICSR_PAR_DET_FAULT_IE    (1 << 12)
#define ICSR_LP_ACK_IE           (1 << 11)
#define ICSR_LINK_DOWN_IE        (1 << 10)
#define ICSR_REMOTE_FAULT_IE     (1 << 9)
#define ICSR_LINK_UP_IE          (1 << 8)
#define ICSR_JABBER_IF           (1 << 7)
#define ICSR_RECEIVE_ERROR_IF    (1 << 6)
#define ICSR_PAGE_RECEIVED_IF    (1 << 5)
#define ICSR_PAR_DET_FAULT_IF    (1 << 4)
#define ICSR_LP_ACK_IF           (1 << 3)
#define ICSR_LINK_DOWN_IF        (1 << 2)
#define ICSR_REMOTE_FAULT_IF     (1 << 1)
#define ICSR_LINK_UP_IF          (1 << 0)

//PHYCON register
#define PHYCON_PAIR_SWAP_DIS     (1 << 13)
#define PHYCON_ENERGY_DETECT     (1 << 12)
#define PHYCON_FORCE_LINK        (1 << 11)
#define PHYCON_POWER_SAVING      (1 << 10)
#define PHYCON_INT_LEVEL         (1 << 9)
#define PHYCON_JABBER_EN         (1 << 8)
#define PHYCON_AN_COMPLETE       (1 << 7)
#define PHYCON_PAUSE_EN          (1 << 6)
#define PHYCON_ISOLATE           (1 << 5)
#define PHYCON_OP_MODE2          (1 << 4)
#define PHYCON_OP_MODE1          (1 << 3)
#define PHYCON_OP_MODE0          (1 << 2)
#define PHYCON_SQE_TEST_EN       (1 << 1)
#define PHYCON_SCRAMBLER_DIS     (1 << 0)

//Operation mode indication
#define PHYCON_OP_MODE_MASK      (7 << 2)
#define PHYCON_OP_MODE_AN        (0 << 2)
#define PHYCON_OP_MODE_10BT      (1 << 2)
#define PHYCON_OP_MODE_100BTX    (2 << 2)
#define PHYCON_OP_MODE_10BT_FD   (5 << 2)
#define PHYCON_OP_MODE_100BTX_FD (6 << 2)
#define PHYCON_OP_MODE_ISOLATE   (7 << 2)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//KSZ8721 Ethernet PHY driver
extern const PhyDriver ksz8721PhyDriver;

//KSZ8721 related functions
error_t ksz8721Init(NetInterface *interface);

void ksz8721Tick(NetInterface *interface);

void ksz8721EnableIrq(NetInterface *interface);
void ksz8721DisableIrq(NetInterface *interface);

void ksz8721EventHandler(NetInterface *interface);

void ksz8721WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t ksz8721ReadPhyReg(NetInterface *interface, uint8_t address);

void ksz8721DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
