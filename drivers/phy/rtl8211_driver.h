/**
 * @file rtl8211_driver.h
 * @brief RTL8211 Ethernet PHY transceiver
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

#ifndef _RTL8211_DRIVER_H
#define _RTL8211_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef RTL8211_PHY_ADDR
   #define RTL8211_PHY_ADDR 1
#elif (RTL8211_PHY_ADDR < 0 || RTL8211_PHY_ADDR > 31)
   #error RTL8211_PHY_ADDR parameter is not valid
#endif

//RTL8211 registers
#define RTL8211_PHY_REG_BMCR        0x00
#define RTL8211_PHY_REG_BMSR        0x01
#define RTL8211_PHY_REG_PHYIDR1     0x02
#define RTL8211_PHY_REG_PHYIDR2     0x03
#define RTL8211_PHY_REG_ANAR        0x04
#define RTL8211_PHY_REG_ANLPAR      0x05
#define RTL8211_PHY_REG_ANER        0x06
#define RTL8211_PHY_REG_ANNPRR      0x07
#define RTL8211_PHY_REG_LPNPAR      0x08
#define RTL8211_PHY_REG_GBCR        0x09
#define RTL8211_PHY_REG_GBSR        0x0A
#define RTL8211_PHY_REG_MACR        0x0D
#define RTL8211_PHY_REG_MAADR       0x0E
#define RTL8211_PHY_REG_GBESR       0x0F
#define RTL8211_PHY_REG_PHYCR       0x10
#define RTL8211_PHY_REG_PHYSR       0x11
#define RTL8211_PHY_REG_INER        0x12
#define RTL8211_PHY_REG_INSR        0x13
#define RTL8211_PHY_REG_RXERC       0x18
#define RTL8211_PHY_REG_LDPSR       0x1B
#define RTL8211_PHY_REG_EPAGSR      0x1E
#define RTL8211_PHY_REG_PAGSEL      0x1F

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

//ANNPRR register
#define ANNPRR_NEXT_PAGE            (1 << 15)
#define ANNPRR_ACK                  (1 << 14)
#define ANNPRR_MSG_PAGE             (1 << 13)
#define ANNPRR_ACK2                 (1 << 12)
#define ANNPRR_TOGGLE               (1 << 11)
#define ANNPRR_MESSAGE10            (1 << 10)
#define ANNPRR_MESSAGE9             (1 << 9)
#define ANNPRR_MESSAGE8             (1 << 8)
#define ANNPRR_MESSAGE7             (1 << 7)
#define ANNPRR_MESSAGE6             (1 << 6)
#define ANNPRR_MESSAGE5             (1 << 5)
#define ANNPRR_MESSAGE4             (1 << 4)
#define ANNPRR_MESSAGE3             (1 << 3)
#define ANNPRR_MESSAGE2             (1 << 2)
#define ANNPRR_MESSAGE1             (1 << 1)
#define ANNPRR_MESSAGE0             (1 << 0)

//GBCR register
#define GBCR_TEST_MODE2             (1 << 15)
#define GBCR_TEST_MODE1             (1 << 14)
#define GBCR_TEST_MODE0             (1 << 13)
#define GBCR_MS_MAN_CONF_EN         (1 << 12)
#define GBCR_MS_MAN_CONF_VAL        (1 << 11)
#define GBCR_PORT_TYPE              (1 << 10)
#define GBCR_1000BT_FD              (1 << 9)
#define GBCR_1000BT_HD              (1 << 8)

//GBSR register
#define GBSR_MS_CONF_FAULT          (1 << 15)
#define GBSR_MS_CONF_RES            (1 << 14)
#define GBSR_LOC_REC_STATUS         (1 << 13)
#define GBSR_REM_REC_STATUS         (1 << 12)
#define GBSR_LP_1000BT_FD           (1 << 11)
#define GBSR_LP_1000BT_HD           (1 << 10)
#define GBSR_IDLE_ERR_CTR7          (1 << 7)
#define GBSR_IDLE_ERR_CTR6          (1 << 6)
#define GBSR_IDLE_ERR_CTR5          (1 << 5)
#define GBSR_IDLE_ERR_CTR4          (1 << 4)
#define GBSR_IDLE_ERR_CTR3          (1 << 3)
#define GBSR_IDLE_ERR_CTR2          (1 << 2)
#define GBSR_IDLE_ERR_CTR1          (1 << 1)
#define GBSR_IDLE_ERR_CTR0          (1 << 0)

//MACR register
#define MACR_FUNCTION1              (1 << 15)
#define MACR_FUNCTION0              (1 << 14)
#define MACR_DEVAD4                 (1 << 4)
#define MACR_DEVAD3                 (1 << 3)
#define MACR_DEVAD2                 (1 << 2)
#define MACR_DEVAD1                 (1 << 1)
#define MACR_DEVAD0                 (1 << 0)

//GBESR register
#define GBESR_1000BX_FD             (1 << 15)
#define GBESR_1000BX_HD             (1 << 14)
#define GBESR_1000BT_FD             (1 << 13)
#define GBESR_1000BT_HD             (1 << 12)

//PHYCR register
#define PHYCR_DISABLE_RXC           (1 << 15)
#define PHYCR_FPR_FAIL_SEL2         (1 << 14)
#define PHYCR_FPR_FAIL_SEL1         (1 << 13)
#define PHYCR_FPR_FAIL_SEL0         (1 << 12)
#define PHYCR_ASSERT_CRS_ON_TX      (1 << 11)
#define PHYCR_FORCE_LINK_GOOD       (1 << 10)
#define PHYCR_ENABLE_CROSSOVER      (1 << 6)
#define PHYCR_MDI_MODE              (1 << 5)
#define PHYCR_DISABLE CLK125        (1 << 4)
#define PHYCR_DISABLE_JABBER        (1 << 0)

//PHYSR register
#define PHYSR_SPEED1                (1 << 15)
#define PHYSR_SPEED0                (1 << 14)
#define PHYSR_DUPLEX                (1 << 13)
#define PHYSR_PAGE_RECEIVED         (1 << 12)
#define PHYSR_SPEED_DUPLEX_RESOLVED (1 << 11)
#define PHYSR_LINK                  (1 << 10)
#define PHYSR_MDI_CROSSOVER_STATUS  (1 << 6)
#define PHYSR_RE_LINK_OK            (1 << 1)
#define PHYSR_JABBER                (1 << 0)

//Speed
#define PHYSR_SPEED_MASK            (3 << 14)
#define PHYSR_SPEED_10              (0 << 14)
#define PHYSR_SPEED_100             (1 << 14)
#define PHYSR_SPEED_1000            (2 << 14)

//INER register
#define INER_AN_ERROR               (1 << 15)
#define INER_PAGE_RECEIVED          (1 << 12)
#define INER_AN_COMPLETE            (1 << 11)
#define INER_LINK_STATUS            (1 << 10)
#define INER_SYMBOL_ERROR           (1 << 9)
#define INER_FALSE_CARRIER          (1 << 8)
#define INER_JABBER                 (1 << 0)

//INSR register
#define INSR_AN_ERROR               (1 << 15)
#define INSR_PAGE_RECEIVED          (1 << 12)
#define INSR_AN_COMPLETE            (1 << 11)
#define INSR_LINK_STATUS            (1 << 10)
#define INSR_SYMBOL_ERROR           (1 << 9)
#define INSR_FALSE_CARRIER          (1 << 8)
#define INSR_JABBER                 (1 << 0)

//LDPSR register
#define LDPSR_POWER_SAVE_MODE       (1 << 0)

//EPAGSR register
#define EPAGSR_EXT_PAGE_SEL7        (1 << 7)
#define EPAGSR_EXT_PAGE_SEL6        (1 << 6)
#define EPAGSR_EXT_PAGE_SEL5        (1 << 5)
#define EPAGSR_EXT_PAGE_SEL4        (1 << 4)
#define EPAGSR_EXT_PAGE_SEL3        (1 << 3)
#define EPAGSR_EXT_PAGE_SEL2        (1 << 2)
#define EPAGSR_EXT_PAGE_SEL1        (1 << 1)
#define EPAGSR_EXT_PAGE_SEL0        (1 << 0)

//PAGSEL register
#define PAGSEL_PAGE_SEL2            (1 << 2)
#define PAGSEL_PAGE_SEL1            (1 << 1)
#define PAGSEL_PAGE_SEL0            (1 << 0)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//RTL8211 Ethernet PHY driver
extern const PhyDriver rtl8211PhyDriver;

//RTL8211 related functions
error_t rtl8211Init(NetInterface *interface);

void rtl8211Tick(NetInterface *interface);

void rtl8211EnableIrq(NetInterface *interface);
void rtl8211DisableIrq(NetInterface *interface);

void rtl8211EventHandler(NetInterface *interface);

void rtl8211WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t rtl8211ReadPhyReg(NetInterface *interface, uint8_t address);

void rtl8211DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
