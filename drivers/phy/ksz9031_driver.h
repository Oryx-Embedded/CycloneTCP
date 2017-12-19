/**
 * @file ksz9031_driver.h
 * @brief KSZ9031 Gigabit Ethernet PHY transceiver
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

#ifndef _KSZ9031_DRIVER_H
#define _KSZ9031_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef KSZ9031_PHY_ADDR
   #define KSZ9031_PHY_ADDR 7
#elif (KSZ9031_PHY_ADDR < 0 || KSZ9031_PHY_ADDR > 31)
   #error KSZ9031_PHY_ADDR parameter is not valid
#endif

//KSZ9031 registers
#define KSZ9031_PHY_REG_BMCR          0x00
#define KSZ9031_PHY_REG_BMSR          0x01
#define KSZ9031_PHY_REG_PHYIDR1       0x02
#define KSZ9031_PHY_REG_PHYIDR2       0x03
#define KSZ9031_PHY_REG_ANAR          0x04
#define KSZ9031_PHY_REG_ANLPAR        0x05
#define KSZ9031_PHY_REG_ANER          0x06
#define KSZ9031_PHY_REG_ANNPTR        0x07
#define KSZ9031_PHY_REG_LPNPAR        0x08
#define KSZ9031_PHY_REG_1000BT_CTRL   0x09
#define KSZ9031_PHY_REG_1000BT_STATUS 0x0A
#define KSZ9031_PHY_REG_MMD_CTRL      0x0D
#define KSZ9031_PHY_REG_MMD_DATA      0x0E
#define KSZ9031_PHY_REG_EXT_STATUS    0x0F
#define KSZ9031_PHY_REG_RLB           0x11
#define KSZ9031_PHY_REG_LINKMDCD      0x12
#define KSZ9031_PHY_REG_DPMAPCSS      0x13
#define KSZ9031_PHY_REG_RXERCTR       0x15
#define KSZ9031_PHY_REG_ICSR          0x1B
#define KSZ9031_PHY_REG_AUTOMDI       0x1C
#define KSZ9031_PHY_REG_PHYCON        0x1F

//BMCR register
#define BMCR_RESET                    (1 << 15)
#define BMCR_LOOPBACK                 (1 << 14)
#define BMCR_SPEED_SEL                (1 << 13)
#define BMCR_AN_EN                    (1 << 12)
#define BMCR_POWER_DOWN               (1 << 11)
#define BMCR_ISOLATE                  (1 << 10)
#define BMCR_RESTART_AN               (1 << 9)
#define BMCR_DUPLEX_MODE              (1 << 8)

//BMSR register
#define BMSR_100BT4                   (1 << 15)
#define BMSR_100BTX_FD                (1 << 14)
#define BMSR_100BTX_HD                (1 << 13)
#define BMSR_10BT_FD                  (1 << 12)
#define BMSR_10BT_HD                  (1 << 11)
#define BMSR_EXTENDED_STATUS          (1 << 8)
#define BMSR_NO_PREAMBLE              (1 << 6)
#define BMSR_AN_COMPLETE              (1 << 5)
#define BMSR_REMOTE_FAULT             (1 << 4)
#define BMSR_AN_ABLE                  (1 << 3)
#define BMSR_LINK_STATUS              (1 << 2)
#define BMSR_JABBER_DETECT            (1 << 1)
#define BMSR_EXTENDED_CAP             (1 << 0)

//ANAR register
#define ANAR_NEXT_PAGE                (1 << 15)
#define ANAR_REMOTE_FAULT             (1 << 13)
#define ANAR_PAUSE1                   (1 << 11)
#define ANAR_PAUSE0                   (1 << 10)
#define ANAR_100BT4                   (1 << 9)
#define ANAR_100BTX_FD                (1 << 8)
#define ANAR_100BTX_HD                (1 << 7)
#define ANAR_10BT_FD                  (1 << 6)
#define ANAR_10BT_HD                  (1 << 5)
#define ANAR_SELECTOR4                (1 << 4)
#define ANAR_SELECTOR3                (1 << 3)
#define ANAR_SELECTOR2                (1 << 2)
#define ANAR_SELECTOR1                (1 << 1)
#define ANAR_SELECTOR0                (1 << 0)

//ANLPAR register
#define ANLPAR_NEXT_PAGE              (1 << 15)
#define ANLPAR_LP_ACK                 (1 << 14)
#define ANLPAR_REMOTE_FAULT           (1 << 13)
#define ANLPAR_PAUSE1                 (1 << 11)
#define ANLPAR_PAUSE0                 (1 << 10)
#define ANLPAR_100BT4                 (1 << 9)
#define ANLPAR_100BTX_FD              (1 << 8)
#define ANLPAR_100BTX_HD              (1 << 7)
#define ANLPAR_10BT_FD                (1 << 6)
#define ANLPAR_10BT_HD                (1 << 5)
#define ANLPAR_SELECTOR4              (1 << 4)
#define ANLPAR_SELECTOR3              (1 << 3)
#define ANLPAR_SELECTOR2              (1 << 2)
#define ANLPAR_SELECTOR1              (1 << 1)
#define ANLPAR_SELECTOR0              (1 << 0)

//ANER register
#define ANER_PAR_DET_FAULT            (1 << 4)
#define ANER_LP_NEXT_PAGE_ABLE        (1 << 3)
#define ANER_NEXT_PAGE_ABLE           (1 << 2)
#define ANER_PAGE_RECEIVED            (1 << 1)
#define ANER_LP_AN_ABLE               (1 << 0)

//ANNPTR register
#define ANNPTR_NEXT_PAGE              (1 << 15)
#define ANNPTR_MSG_PAGE               (1 << 13)
#define ANNPTR_ACK2                   (1 << 12)
#define ANNPTR_TOGGLE                 (1 << 11)
#define ANNPTR_MESSAGE10              (1 << 10)
#define ANNPTR_MESSAGE9               (1 << 9)
#define ANNPTR_MESSAGE8               (1 << 8)
#define ANNPTR_MESSAGE7               (1 << 7)
#define ANNPTR_MESSAGE6               (1 << 6)
#define ANNPTR_MESSAGE5               (1 << 5)
#define ANNPTR_MESSAGE4               (1 << 4)
#define ANNPTR_MESSAGE3               (1 << 3)
#define ANNPTR_MESSAGE2               (1 << 2)
#define ANNPTR_MESSAGE1               (1 << 1)
#define ANNPTR_MESSAGE0               (1 << 0)

//LPNPAR register
#define LPNPAR_NEXT_PAGE              (1 << 15)
#define LPNPAR_ACK                    (1 << 14)
#define LPNPAR_MSG_PAGE               (1 << 13)
#define LPNPAR_ACK2                   (1 << 12)
#define LPNPAR_TOGGLE                 (1 << 11)
#define LPNPAR_MESSAGE10              (1 << 10)
#define LPNPAR_MESSAGE9               (1 << 9)
#define LPNPAR_MESSAGE8               (1 << 8)
#define LPNPAR_MESSAGE7               (1 << 7)
#define LPNPAR_MESSAGE6               (1 << 6)
#define LPNPAR_MESSAGE5               (1 << 5)
#define LPNPAR_MESSAGE4               (1 << 4)
#define LPNPAR_MESSAGE3               (1 << 3)
#define LPNPAR_MESSAGE2               (1 << 2)
#define LPNPAR_MESSAGE1               (1 << 1)
#define LPNPAR_MESSAGE0               (1 << 0)

//1000BT_CTRL register
#define _1000BT_CTRL_TEST_MODE2       (1 << 15)
#define _1000BT_CTRL_TEST_MODE1       (1 << 14)
#define _1000BT_CTRL_TEST_MODE0       (1 << 13)
#define _1000BT_CTRL_MS_MAN_CONF_EN   (1 << 12)
#define _1000BT_CTRL_MS_MAN_CONF_VAL  (1 << 11)
#define _1000BT_CTRL_PORT_TYPE        (1 << 10)
#define _1000BT_CTRL_1000BT_FD        (1 << 9)
#define _1000BT_CTRL_1000BT_HD        (1 << 8)

//1000BT_STATUS register
#define _1000BT_STATUS_MS_CONF_FAULT  (1 << 15)
#define _1000BT_STATUS_MS_CONF_RES    (1 << 14)
#define _1000BT_STATUS_LOC_REC_STATUS (1 << 13)
#define _1000BT_STATUS_REM_REC_STATUS (1 << 12)
#define _1000BT_STATUS_LP_1000BT_FD   (1 << 11)
#define _1000BT_STATUS_LP_1000BT_HD   (1 << 10)
#define _1000BT_STATUS_IDLE_ERR_CTR7  (1 << 7)
#define _1000BT_STATUS_IDLE_ERR_CTR6  (1 << 6)
#define _1000BT_STATUS_IDLE_ERR_CTR5  (1 << 5)
#define _1000BT_STATUS_IDLE_ERR_CTR4  (1 << 4)
#define _1000BT_STATUS_IDLE_ERR_CTR3  (1 << 3)
#define _1000BT_STATUS_IDLE_ERR_CTR2  (1 << 2)
#define _1000BT_STATUS_IDLE_ERR_CTR1  (1 << 1)
#define _1000BT_STATUS_IDLE_ERR_CTR0  (1 << 0)

//MMD_CTRL register
#define MMD_CTRL_DEVICE_OP_MODE1      (1 << 15)
#define MMD_CTRL_DEVICE_OP_MODE0      (1 << 14)
#define MMD_CTRL_DEVICE_ADDR4         (1 << 4)
#define MMD_CTRL_DEVICE_ADDR3         (1 << 3)
#define MMD_CTRL_DEVICE_ADDR2         (1 << 2)
#define MMD_CTRL_DEVICE_ADDR1         (1 << 1)
#define MMD_CTRL_DEVICE_ADDR0         (1 << 0)

//EXT_STATUS register
#define EXT_STATUS_1000BX_FD          (1 << 15)
#define EXT_STATUS_1000BX_HD          (1 << 14)
#define EXT_STATUS_1000BT_FD          (1 << 13)
#define EXT_STATUS_1000BT_HD          (1 << 12)

//RLB register
#define RLB_REMOTE_LOOPBACK           (1 << 8)

//LINKMDCD register
#define LINKMDCD_DIAG_EN              (1 << 15)
#define LINKMDCD_DIAG_TEST_PAIR1      (1 << 13)
#define LINKMDCD_DIAG_TEST_PAIR0      (1 << 12)
#define LINKMDCD_FAULT_STATUS1        (1 << 9)
#define LINKMDCD_FAULT_STATUS0        (1 << 8)
#define LINKMDCD_FAULT_DATA7          (1 << 7)
#define LINKMDCD_FAULT_DATA6          (1 << 6)
#define LINKMDCD_FAULT_DATA5          (1 << 5)
#define LINKMDCD_FAULT_DATA4          (1 << 4)
#define LINKMDCD_FAULT_DATA3          (1 << 3)
#define LINKMDCD_FAULT_DATA2          (1 << 2)
#define LINKMDCD_FAULT_DATA1          (1 << 1)
#define LINKMDCD_FAULT_DATA0          (1 << 0)

//DPMAPCSS register
#define DPMAPCSS_1000BT_LINK_STATUS   (1 << 2)
#define DPMAPCSS_100BTX_LINK_STATUS   (1 << 1)

//ICSR register
#define ICSR_JABBER_IE                (1 << 15)
#define ICSR_RECEIVE_ERROR_IE         (1 << 14)
#define ICSR_PAGE_RECEIVED_IE         (1 << 13)
#define ICSR_PAR_DET_FAULT_IE         (1 << 12)
#define ICSR_LP_ACK_IE                (1 << 11)
#define ICSR_LINK_DOWN_IE             (1 << 10)
#define ICSR_REMOTE_FAULT_IE          (1 << 9)
#define ICSR_LINK_UP_IE               (1 << 8)
#define ICSR_JABBER_IF                (1 << 7)
#define ICSR_RECEIVE_ERROR_IF         (1 << 6)
#define ICSR_PAGE_RECEIVED_IF         (1 << 5)
#define ICSR_PAR_DET_FAULT_IF         (1 << 4)
#define ICSR_LP_ACK_IF                (1 << 3)
#define ICSR_LINK_DOWN_IF             (1 << 2)
#define ICSR_REMOTE_FAULT_IF          (1 << 1)
#define ICSR_LINK_UP_IF               (1 << 0)

//AUTOMDI register
#define AUTOMDI_MDI_SEL               (1 << 7)
#define AUTOMDI_SWAP_OFF              (1 << 6)

//PHYCON register
#define PHYCON_INT_LEVEL              (1 << 14)
#define PHYCON_JABBER_EN              (1 << 9)
#define PHYCON_SPEED_1000BT           (1 << 6)
#define PHYCON_SPEED_100BTX           (1 << 5)
#define PHYCON_SPEED_10BT             (1 << 4)
#define PHYCON_DUPLEX_STATUS          (1 << 3)
#define PHYCON_1000BT_MS_STATUS       (1 << 2)
#define PHYCON_LINK_STATUS_CHECK_FAIL (1 << 0)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//KSZ9031 Ethernet PHY driver
extern const PhyDriver ksz9031PhyDriver;

//KSZ9031 related functions
error_t ksz9031Init(NetInterface *interface);

void ksz9031Tick(NetInterface *interface);

void ksz9031EnableIrq(NetInterface *interface);
void ksz9031DisableIrq(NetInterface *interface);

void ksz9031EventHandler(NetInterface *interface);

void ksz9031WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t ksz9031ReadPhyReg(NetInterface *interface, uint8_t address);

void ksz9031DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
