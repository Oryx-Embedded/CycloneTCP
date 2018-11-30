/**
 * @file ip101_driver.h
 * @brief IC+ IP101 Ethernet PHY transceiver
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

#ifndef _IP101_DRIVER_H
#define _IP101_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef IP101_PHY_ADDR
   #define IP101_PHY_ADDR 1
#elif (IP101_PHY_ADDR < 0 || IP101_PHY_ADDR > 31)
   #error IP101_PHY_ADDR parameter is not valid
#endif

//IP101 registers
#define IP101_PHY_REG_BMCR        0x00
#define IP101_PHY_REG_BMSR        0x01
#define IP101_PHY_REG_PHYIDR1     0x02
#define IP101_PHY_REG_PHYIDR2     0x03
#define IP101_PHY_REG_ANAR        0x04
#define IP101_PHY_REG_ANLPAR      0x05
#define IP101_PHY_REG_ANER        0x06
#define IP101_PHY_REG_ANNPTR      0x07
#define IP101_PHY_REG_LPNPAR      0x08
#define IP101_PHY_REG_MMDACR      0x0D
#define IP101_PHY_REG_MMDAADR     0x0E
#define IP101_PHY_REG_PHYSCR      0x10
#define IP101_PHY_REG_ICSR        0x11
#define IP101_PHY_REG_PHYSMR      0x12
#define IP101_PHY_REG_IOSCR       0x1D
#define IP101_PHY_REG_PHYMCSSR    0x1E

//BMCR register
#define BMCR_RESET                (1 << 15)
#define BMCR_LOOPBACK             (1 << 14)
#define BMCR_SPEED_SEL            (1 << 13)
#define BMCR_AN_EN                (1 << 12)
#define BMCR_POWER_DOWN           (1 << 11)
#define BMCR_ISOLATE              (1 << 10)
#define BMCR_RESTART_AN           (1 << 9)
#define BMCR_DUPLEX_MODE          (1 << 8)
#define BMCR_COL_TEST             (1 << 7)

//BMSR register
#define BMSR_100BT4               (1 << 15)
#define BMSR_100BTX_FD            (1 << 14)
#define BMSR_100BTX               (1 << 13)
#define BMSR_10BT_FD              (1 << 12)
#define BMSR_10BT                 (1 << 11)
#define BMSR_NO_PREAMBLE          (1 << 6)
#define BMSR_AN_COMPLETE          (1 << 5)
#define BMSR_REMOTE_FAULT         (1 << 4)
#define BMSR_AN_ABLE              (1 << 3)
#define BMSR_LINK_STATUS          (1 << 2)
#define BMSR_JABBER_DETECT        (1 << 1)
#define BMSR_EXTENDED_CAP         (1 << 0)

//ANAR register
#define ANAR_NP                   (1 << 15)
#define ANAR_RF                   (1 << 13)
#define ANAR_ASYMMETRIC_PAUSE     (1 << 11)
#define ANAR_PAUSE                (1 << 10)
#define ANAR_100BT4               (1 << 9)
#define ANAR_100BTX_FD            (1 << 8)
#define ANAR_100BTX               (1 << 7)
#define ANAR_10BT_FD              (1 << 6)
#define ANAR_10BT                 (1 << 5)
#define ANAR_SELECTOR4            (1 << 4)
#define ANAR_SELECTOR3            (1 << 3)
#define ANAR_SELECTOR2            (1 << 2)
#define ANAR_SELECTOR1            (1 << 1)
#define ANAR_SELECTOR0            (1 << 0)

//ANLPAR register
#define ANLPAR_NP                 (1 << 15)
#define ANLPAR_ACK                (1 << 14)
#define ANLPAR_RF                 (1 << 13)
#define ANLPAR_ASYMMETRIC_PAUSE   (1 << 11)
#define ANLPAR_PAUSE              (1 << 10)
#define ANLPAR_100BT4             (1 << 9)
#define ANLPAR_100BTX_FD          (1 << 8)
#define ANLPAR_100BTX             (1 << 7)
#define ANLPAR_10BT_FD            (1 << 6)
#define ANLPAR_10BT               (1 << 5)
#define ANLPAR_SELECTOR4          (1 << 4)
#define ANLPAR_SELECTOR3          (1 << 3)
#define ANLPAR_SELECTOR2          (1 << 2)
#define ANLPAR_SELECTOR1          (1 << 1)
#define ANLPAR_SELECTOR0          (1 << 0)

//ANER register
#define ANER_MLF                  (1 << 4)
#define ANER_LP_NP_ABLE           (1 << 3)
#define ANER_NP_ABLE              (1 << 2)
#define ANER_PAGE_RX              (1 << 1)
#define ANER_LP_AN_ABLE           (1 << 0)

//ANNPTR register
#define ANNPTR_NP                 (1 << 15)
#define ANNPTR_MP                 (1 << 13)
#define ANNPTR_ACK2               (1 << 12)
#define ANNPTR_TOGGLE             (1 << 11)
#define ANNPTR_CODE10             (1 << 10)
#define ANNPTR_CODE9              (1 << 9)
#define ANNPTR_CODE8              (1 << 8)
#define ANNPTR_CODE7              (1 << 7)
#define ANNPTR_CODE6              (1 << 6)
#define ANNPTR_CODE5              (1 << 5)
#define ANNPTR_CODE4              (1 << 4)
#define ANNPTR_CODE3              (1 << 3)
#define ANNPTR_CODE2              (1 << 2)
#define ANNPTR_CODE1              (1 << 1)
#define ANNPTR_CODE0              (1 << 0)

//LPNPAR register
#define LPNPAR_NEXT_PAGE          (1 << 15)
#define LPNPAR_MSG_PAGE           (1 << 13)
#define LPNPAR_ACK2               (1 << 12)
#define LPNPAR_TOGGLE             (1 << 11)
#define LPNPAR_MESSAGE10          (1 << 10)
#define LPNPAR_MESSAGE9           (1 << 9)
#define LPNPAR_MESSAGE8           (1 << 8)
#define LPNPAR_MESSAGE7           (1 << 7)
#define LPNPAR_MESSAGE6           (1 << 6)
#define LPNPAR_MESSAGE5           (1 << 5)
#define LPNPAR_MESSAGE4           (1 << 4)
#define LPNPAR_MESSAGE3           (1 << 3)
#define LPNPAR_MESSAGE2           (1 << 2)
#define LPNPAR_MESSAGE1           (1 << 1)
#define LPNPAR_MESSAGE0           (1 << 0)

//MMDACR register
#define MMDACR_FUNCTION1          (1 << 15)
#define MMDACR_FUNCTION0          (1 << 14)
#define MMDACR_DEVAD4             (1 << 4)
#define MMDACR_DEVAD3             (1 << 3)
#define MMDACR_DEVAD2             (1 << 2)
#define MMDACR_DEVAD1             (1 << 1)
#define MMDACR_DEVAD0             (1 << 0)

//PHYSCR register
#define PHYSCR_RMII_V10           (1 << 13)
#define PHYSCR_RMII_V12           (1 << 12)
#define PHYSCR_AUTO_MDIX_DIS      (1 << 11)
#define PHYSCR_JABBER_ENABLE      (1 << 9)
#define PHYSCR_FEF_DISABLE        (1 << 8)
#define PHYSCR_NWAY_PSAVE_DIS     (1 << 7)
#define PHYSCR_BYPASS_DSP_RESET   (1 << 5)
#define PHYSCR_REPEATER_MODE      (1 << 2)
#define PHYSCR_LDPS_ENABLE        (1 << 1)
#define PHYSCR_ANALOG_OFF         (1 << 0)

//ICSR register
#define ICSR_INTR_EN              (1 << 15)
#define ICSR_RESERVED2            (1 << 14)
#define ICSR_RESERVED1            (1 << 13)
#define ICSR_RESERVED0            (1 << 12)
#define ICSR_ALL_MASK             (1 << 11)
#define ICSR_SPEED_MASK           (1 << 10)
#define ICSR_DUPLEX_MASK          (1 << 9)
#define ICSR_LINK_MASK            (1 << 8)
#define ICSR_INTR_STATUS          (1 << 3)
#define ICSR_SPEED_CHANGE         (1 << 2)
#define ICSR_DUPLEX_CHANGE        (1 << 1)
#define ICSR_LINK_CHANGE          (1 << 0)

//PHYSMR register
#define PHYSMR_SPEED              (1 << 14)
#define PHYSMR_DUPLEX             (1 << 13)
#define PHYSMR_AN_COMPLETE        (1 << 11)
#define PHYSMR_LINK_UP            (1 << 10)
#define PHYSMR_MDIX               (1 << 9)
#define PHYSMR_POLARITY           (1 << 8)
#define PHYSMR_JABBER             (1 << 7)
#define PHYSMR_AN_ARBIT_STATE3    (1 << 3)
#define PHYSMR_AN_ARBIT_STATE2    (1 << 2)
#define PHYSMR_AN_ARBIT_STATE1    (1 << 1)
#define PHYSMR_AN_ARBIT_STATE0    (1 << 0)

//IOSCR register
#define IOSCR_RMII_WITH_ER        (1 << 7)
#define IOSCR_SEL_INTR32          (1 << 2)

//PHYMCSSR register
#define PHYMCSSR_LINK_UP          (1 << 8)
#define PHYMCSSR_FORCE_MDIX       (1 << 3)
#define PHYMCSSR_OP_MODE2         (1 << 2)
#define PHYMCSSR_OP_MODE1         (1 << 1)
#define PHYMCSSR_OP_MODE0         (1 << 0)

//Operation mode indication
#define PHYMCSSR_OP_MODE_MASK     (7 << 0)
#define PHYMCSSR_OP_MODE_LINK_OFF (0 << 0)
#define PHYMCSSR_OP_MODE_10M_HD   (1 << 0)
#define PHYMCSSR_OP_MODE_100M_HD  (2 << 0)
#define PHYMCSSR_OP_MODE_10M_FD   (5 << 0)
#define PHYMCSSR_OP_MODE_100_FD   (6 << 0)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//IP101 Ethernet PHY driver
extern const PhyDriver ip101PhyDriver;

//IP101 related functions
error_t ip101Init(NetInterface *interface);

void ip101Tick(NetInterface *interface);

void ip101EnableIrq(NetInterface *interface);
void ip101DisableIrq(NetInterface *interface);

void ip101EventHandler(NetInterface *interface);

void ip101WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t ip101ReadPhyReg(NetInterface *interface, uint8_t address);

void ip101DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
