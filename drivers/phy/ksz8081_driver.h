/**
 * @file ksz8081_driver.h
 * @brief KSZ8081 Ethernet PHY transceiver
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

#ifndef _KSZ8081_DRIVER_H
#define _KSZ8081_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef KSZ8081_PHY_ADDR
   #define KSZ8081_PHY_ADDR 0
#elif (KSZ8081_PHY_ADDR < 0 || KSZ8081_PHY_ADDR > 31)
   #error KSZ8081_PHY_ADDR parameter is not valid
#endif

//50MHz clock mode support
#ifndef KSZ8081_50MHZ_CLOCK_MODE_SUPPORT
   #define KSZ8081_50MHZ_CLOCK_MODE_SUPPORT DISABLED
#elif (KSZ8081_50MHZ_CLOCK_MODE_SUPPORT != ENABLED && KSZ8081_50MHZ_CLOCK_MODE_SUPPORT != DISABLED)
   #error KSZ8081_50MHZ_CLOCK_MODE_SUPPORT parameter is not valid
#endif

//KSZ8081 registers
#define KSZ8081_PHY_REG_BMCR        0x00
#define KSZ8081_PHY_REG_BMSR        0x01
#define KSZ8081_PHY_REG_PHYIDR1     0x02
#define KSZ8081_PHY_REG_PHYIDR2     0x03
#define KSZ8081_PHY_REG_ANAR        0x04
#define KSZ8081_PHY_REG_ANLPAR      0x05
#define KSZ8081_PHY_REG_ANER        0x06
#define KSZ8081_PHY_REG_ANNPTR      0x07
#define KSZ8081_PHY_REG_LPNPAR      0x08
#define KSZ8081_PHY_REG_DRC         0x10
#define KSZ8081_PHY_REG_AFECON1     0x11
#define KSZ8081_PHY_REG_RXERCTR     0x15
#define KSZ8081_PHY_REG_OMSO        0x16
#define KSZ8081_PHY_REG_OMSS        0x17
#define KSZ8081_PHY_REG_EXCON       0x18
#define KSZ8081_PHY_REG_ICSR        0x1B
#define KSZ8081_PHY_REG_LINKMDCS    0x1D
#define KSZ8081_PHY_REG_PHYCON1     0x1E
#define KSZ8081_PHY_REG_PHYCON2     0x1F

//BMCR register
#define BMCR_RESET                  (1 << 15)
#define BMCR_LOOPBACK               (1 << 14)
#define BMCR_SPEED_SEL              (1 << 13)
#define BMCR_AN_EN                  (1 << 12)
#define BMCR_POWER_DOWN             (1 << 11)
#define BMCR_ISOLATE                (1 << 10)
#define BMCR_RESTART_AN             (1 << 9)
#define BMCR_DUPLEX_MODE            (1 << 8)
#define BMCR_COL_TEST               (1 << 7)

//BMSR register
#define BMSR_100BT4                 (1 << 15)
#define BMSR_100BTX_FD              (1 << 14)
#define BMSR_100BTX                 (1 << 13)
#define BMSR_10BT_FD                (1 << 12)
#define BMSR_10BT                   (1 << 11)
#define BMSR_NO_PREAMBLE            (1 << 6)
#define BMSR_AN_COMPLETE            (1 << 5)
#define BMSR_REMOTE_FAULT           (1 << 4)
#define BMSR_AN_ABLE                (1 << 3)
#define BMSR_LINK_STATUS            (1 << 2)
#define BMSR_JABBER_DETECT          (1 << 1)
#define BMSR_EXTENDED_CAP           (1 << 0)

//ANAR register
#define ANAR_NEXT_PAGE              (1 << 15)
#define ANAR_REMOTE_FAULT           (1 << 13)
#define ANAR_PAUSE1                 (1 << 11)
#define ANAR_PAUSE0                 (1 << 10)
#define ANAR_100BT4                 (1 << 9)
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
#define ANLPAR_PAUSE1               (1 << 11)
#define ANLPAR_PAUSE0               (1 << 10)
#define ANLPAR_100BT4               (1 << 9)
#define ANLPAR_100BTX_FD            (1 << 8)
#define ANLPAR_100BTX               (1 << 7)
#define ANLPAR_10BT_FD              (1 << 6)
#define ANLPAR_10BT                 (1 << 5)
#define ANLPAR_SELECTOR4            (1 << 4)
#define ANLPAR_SELECTOR3            (1 << 3)
#define ANLPAR_SELECTOR2            (1 << 2)
#define ANLPAR_SELECTOR1            (1 << 1)
#define ANLPAR_SELECTOR0            (1 << 0)

//ANER register
#define ANER_PAR_DET_FAULT          (1 << 4)
#define ANER_LP_NEXT_PAGE_ABLE      (1 << 3)
#define ANER_NEXT_PAGE_ABLE         (1 << 2)
#define ANER_PAGE_RECEIVED          (1 << 1)
#define ANER_LP_AN_ABLE             (1 << 0)

//ANNPTR register
#define ANNPTR_NEXT_PAGE            (1 << 15)
#define ANNPTR_MSG_PAGE             (1 << 13)
#define ANNPTR_ACK2                 (1 << 12)
#define ANNPTR_TOGGLE               (1 << 11)
#define ANNPTR_MESSAGE10            (1 << 10)
#define ANNPTR_MESSAGE9             (1 << 9)
#define ANNPTR_MESSAGE8             (1 << 8)
#define ANNPTR_MESSAGE7             (1 << 7)
#define ANNPTR_MESSAGE6             (1 << 6)
#define ANNPTR_MESSAGE5             (1 << 5)
#define ANNPTR_MESSAGE4             (1 << 4)
#define ANNPTR_MESSAGE3             (1 << 3)
#define ANNPTR_MESSAGE2             (1 << 2)
#define ANNPTR_MESSAGE1             (1 << 1)
#define ANNPTR_MESSAGE0             (1 << 0)

//LPNPAR register
#define LPNPAR_NEXT_PAGE            (1 << 15)
#define LPNPAR_ACK                  (1 << 14)
#define LPNPAR_MSG_PAGE             (1 << 13)
#define LPNPAR_ACK2                 (1 << 12)
#define LPNPAR_TOGGLE               (1 << 11)
#define LPNPAR_MESSAGE10            (1 << 10)
#define LPNPAR_MESSAGE9             (1 << 9)
#define LPNPAR_MESSAGE8             (1 << 8)
#define LPNPAR_MESSAGE7             (1 << 7)
#define LPNPAR_MESSAGE6             (1 << 6)
#define LPNPAR_MESSAGE5             (1 << 5)
#define LPNPAR_MESSAGE4             (1 << 4)
#define LPNPAR_MESSAGE3             (1 << 3)
#define LPNPAR_MESSAGE2             (1 << 2)
#define LPNPAR_MESSAGE1             (1 << 1)
#define LPNPAR_MESSAGE0             (1 << 0)

//DRC register
#define DRC_PLL_OFF                 (1 << 4)

//AFECON1 register
#define AFECON1_SLOW_OSC_MODE_EN    (1 << 5)

//OMSO register
#define OMSO_BCAST_OFF_OVERRIDE     (1 << 9)
#define OMSO_MII_BTB_OVERRIDE       (1 << 7)
#define OMSO_RMII_BTB_OVERRIDE      (1 << 6)
#define OMSO_NAND_TREE_OVERRIDE     (1 << 5)
#define OMSO_RMII_OVERRIDE          (1 << 1)
#define OMSO_MII_OVERRIDE           (1 << 0)

//OMSS register
#define OMSS_PHYAD2                 (1 << 15)
#define OMSS_PHYAD1                 (1 << 14)
#define OMSS_PHYAD0                 (1 << 13)
#define OMSS_RMII_STATUS            (1 << 1)

//EXCON register
#define EXCON_EDPD_DIS              (1 << 11)

//ICSR register
#define ICSR_JABBER_IE              (1 << 15)
#define ICSR_RECEIVE_ERROR_IE       (1 << 14)
#define ICSR_PAGE_RECEIVED_IE       (1 << 13)
#define ICSR_PAR_DET_FAULT_IE       (1 << 12)
#define ICSR_LP_ACK_IE              (1 << 11)
#define ICSR_LINK_DOWN_IE           (1 << 10)
#define ICSR_REMOTE_FAULT_IE        (1 << 9)
#define ICSR_LINK_UP_IE             (1 << 8)
#define ICSR_JABBER_IF              (1 << 7)
#define ICSR_RECEIVE_ERROR_IF       (1 << 6)
#define ICSR_PAGE_RECEIVED_IF       (1 << 5)
#define ICSR_PAR_DET_FAULT_IF       (1 << 4)
#define ICSR_LP_ACK_IF              (1 << 3)
#define ICSR_LINK_DOWN_IF           (1 << 2)
#define ICSR_REMOTE_FAULT_IF        (1 << 1)
#define ICSR_LINK_UP_IF             (1 << 0)

//LINKMDCS register
#define LINKMDCS_CABLE_DIAG_EN      (1 << 15)
#define LINKMDCS_CABLE_DIAG_RES1    (1 << 14)
#define LINKMDCS_CABLE_DIAG_RES0    (1 << 13)
#define LINKMDCS_SHORT_CABLE        (1 << 12)
#define LINKMDCS_CABLE_FAULT_CNT8   (1 << 8)
#define LINKMDCS_CABLE_FAULT_CNT7   (1 << 7)
#define LINKMDCS_CABLE_FAULT_CNT6   (1 << 6)
#define LINKMDCS_CABLE_FAULT_CNT5   (1 << 5)
#define LINKMDCS_CABLE_FAULT_CNT4   (1 << 4)
#define LINKMDCS_CABLE_FAULT_CNT3   (1 << 3)
#define LINKMDCS_CABLE_FAULT_CNT2   (1 << 2)
#define LINKMDCS_CABLE_FAULT_CNT1   (1 << 1)
#define LINKMDCS_CABLE_FAULT_CNT0   (1 << 0)

//PHYCON1 register
#define PHYCON1_PAUSE_EN            (1 << 9)
#define PHYCON1_LINK_STATUS         (1 << 8)
#define PHYCON1_POL_STATUS          (1 << 7)
#define PHYCON1_MDIX_STATE          (1 << 5)
#define PHYCON1_ENERGY_DETECT       (1 << 4)
#define PHYCON1_ISOLATE             (1 << 3)
#define PHYCON1_OP_MODE2            (1 << 2)
#define PHYCON1_OP_MODE1            (1 << 1)
#define PHYCON1_OP_MODE0            (1 << 0)

//Operation mode indication
#define PHYCON1_OP_MODE_MASK        (7 << 0)
#define PHYCON1_OP_MODE_AN          (0 << 0)
#define PHYCON1_OP_MODE_10BT        (1 << 0)
#define PHYCON1_OP_MODE_100BTX      (2 << 0)
#define PHYCON1_OP_MODE_10BT_FD     (5 << 0)
#define PHYCON1_OP_MODE_100BTX_FD   (6 << 0)

//PHYCON2 register
#define PHYCON2_HP_MDIX             (1 << 15)
#define PHYCON2_MDIX_SEL            (1 << 14)
#define PHYCON2_PAIR_SWAP_DIS       (1 << 13)
#define PHYCON2_FORCE_LINK          (1 << 11)
#define PHYCON2_POWER_SAVING        (1 << 10)
#define PHYCON2_INT_LEVEL           (1 << 9)
#define PHYCON2_JABBER_EN           (1 << 8)
#define PHYCON2_RMII_REF_CLK_SEL    (1 << 7)
#define PHYCON2_LED_MODE1           (1 << 5)
#define PHYCON2_LED_MODE0           (1 << 4)
#define PHYCON2_TX_DIS              (1 << 3)
#define PHYCON2_REMOTE_LOOPBACK     (1 << 2)
#define PHYCON2_SCRAMBLER_DIS       (1 << 0)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//KSZ8081 Ethernet PHY driver
extern const PhyDriver ksz8081PhyDriver;

//KSZ8081 related functions
error_t ksz8081Init(NetInterface *interface);

void ksz8081Tick(NetInterface *interface);

void ksz8081EnableIrq(NetInterface *interface);
void ksz8081DisableIrq(NetInterface *interface);

void ksz8081EventHandler(NetInterface *interface);

void ksz8081WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t ksz8081ReadPhyReg(NetInterface *interface, uint8_t address);

void ksz8081DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
