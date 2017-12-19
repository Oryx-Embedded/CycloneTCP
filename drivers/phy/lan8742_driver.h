/**
 * @file lan8742_driver.h
 * @brief LAN8742 Ethernet PHY transceiver
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

#ifndef _LAN8742_DRIVER_H
#define _LAN8742_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef LAN8742_PHY_ADDR
   #define LAN8742_PHY_ADDR 0
#elif (LAN8742_PHY_ADDR < 0 || LAN8742_PHY_ADDR > 31)
   #error LAN8742_PHY_ADDR parameter is not valid
#endif

//LAN8742 registers
#define LAN8742_PHY_REG_BMCR        0x00
#define LAN8742_PHY_REG_BMSR        0x01
#define LAN8742_PHY_REG_PHYIDR1     0x02
#define LAN8742_PHY_REG_PHYIDR2     0x03
#define LAN8742_PHY_REG_ANAR        0x04
#define LAN8742_PHY_REG_ANLPAR      0x05
#define LAN8742_PHY_REG_ANER        0x06
#define LAN8742_PHY_REG_ANNPTR      0x07
#define LAN8742_PHY_REG_ANNPRR      0x08
#define LAN8742_PHY_REG_MDDACR      0x0D
#define LAN8742_PHY_REG_MDDAADR     0x0E
#define LAN8742_PHY_REG_ENCTR       0x10
#define LAN8742_PHY_REG_MCSR        0x11
#define LAN8742_PHY_REG_SMR         0x12
#define LAN8742_PHY_REG_TDRPDCR     0x18
#define LAN8742_PHY_REG_TDRCSR      0x19
#define LAN8742_PHY_REG_SECR        0x1A
#define LAN8742_PHY_REG_SCSIR       0x1B
#define LAN8742_PHY_REG_CLR         0x1C
#define LAN8742_PHY_REG_ISR         0x1D
#define LAN8742_PHY_REG_IMR         0x1E
#define LAN8742_PHY_REG_PSCSR       0x1F

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
#define BMSR_100BT2_FD              (1 << 10)
#define BMSR_100BT2                 (1 << 9)
#define BMSR_EXTENTED_STATUS        (1 << 8)
#define BMSR_AN_COMPLETE            (1 << 5)
#define BMSR_REMOTE_FAULT           (1 << 4)
#define BMSR_AN_ABLE                (1 << 3)
#define BMSR_LINK_STATUS            (1 << 2)
#define BMSR_JABBER_DETECT          (1 << 1)
#define BMSR_EXTENDED_CAP           (1 << 0)

//ANAR register
#define ANAR_NP                     (1 << 15)
#define ANAR_RF                     (1 << 13)
#define ANAR_PAUSE1                 (1 << 11)
#define ANAR_PAUSE0                 (1 << 10)
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
#define ANLPAR_NP                   (1 << 15)
#define ANLPAR_ACK                  (1 << 14)
#define ANLPAR_RF                   (1 << 13)
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
#define ANER_RX_NP_LOC_ABLE         (1 << 6)
#define ANER_RX_NP_STOR_LOC         (1 << 5)
#define ANER_PDF                    (1 << 4)
#define ANER_LP_NP_ABLE             (1 << 3)
#define ANER_NP_ABLE                (1 << 2)
#define ANER_PAGE_RX                (1 << 1)
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

//MDDACR register
#define MDDACR_MMD_FUNCTION1        (1 << 15)
#define MDDACR_MMD_FUNCTION0        (1 << 14)
#define MDDACR_MMD_DEVAD4           (1 << 4)
#define MDDACR_MMD_DEVAD3           (1 << 3)
#define MDDACR_MMD_DEVAD2           (1 << 2)
#define MDDACR_MMD_DEVAD1           (1 << 1)
#define MDDACR_MMD_DEVAD0           (1 << 0)

//ENCTR register
#define ENCTR_EDPD_TX_NLP_EN        (1 << 15)
#define ENCTR_EDPD_TX_NLP_ITS1      (1 << 14)
#define ENCTR_EDPD_TX_NLP_ITS0      (1 << 13)
#define ENCTR_EDPD_RX_NLP_WAKE_EN   (1 << 12)
#define ENCTR_EDPD_RX_NLP_MIDS1     (1 << 11)
#define ENCTR_EDPD_RX_NLP_MIDS0     (1 << 10)
#define ENCTR_EDPD_EXT_CROSSOVER    (1 << 1)
#define ENCTR_EXT_CROSSOVER_TIME    (1 << 0)

//MCSR register
#define MCSR_EDPWRDOWN              (1 << 13)
#define MCSR_FARLOOPBACK            (1 << 9)
#define MCSR_ALTINT                 (1 << 6)
#define MCSR_ENERGYON               (1 << 1)

//SMR register
#define SMR_MODE2                   (1 << 7)
#define SMR_MODE1                   (1 << 6)
#define SMR_MODE0                   (1 << 5)
#define SMR_PHYAD4                  (1 << 4)
#define SMR_PHYAD3                  (1 << 3)
#define SMR_PHYAD2                  (1 << 2)
#define SMR_PHYAD1                  (1 << 1)
#define SMR_PHYAD0                  (1 << 0)

//TDRPDCR register
#define TDRPDCR_DELAY_IN            (1 << 15)
#define TDRPDCR_LINE_BREAK_COUNTER2 (1 << 14)
#define TDRPDCR_LINE_BREAK_COUNTER1 (1 << 13)
#define TDRPDCR_LINE_BREAK_COUNTER0 (1 << 12)
#define TDRPDCR_PATTERN_HIGH5       (1 << 11)
#define TDRPDCR_PATTERN_HIGH4       (1 << 10)
#define TDRPDCR_PATTERN_HIGH3       (1 << 9)
#define TDRPDCR_PATTERN_HIGH2       (1 << 8)
#define TDRPDCR_PATTERN_HIGH1       (1 << 7)
#define TDRPDCR_PATTERN_HIGH0       (1 << 6)
#define TDRPDCR_PATTERN_LOW5        (1 << 5)
#define TDRPDCR_PATTERN_LOW4        (1 << 4)
#define TDRPDCR_PATTERN_LOW3        (1 << 3)
#define TDRPDCR_PATTERN_LOW2        (1 << 2)
#define TDRPDCR_PATTERN_LOW1        (1 << 1)
#define TDRPDCR_PATTERN_LOW0        (1 << 0)

//TDRCSR register
#define TDRCSR_EN                   (1 << 15)
#define TDRCSR_AD_FILTER_EN         (1 << 14)
#define TDRCSR_CH_CABLE_TYPE1       (1 << 10)
#define TDRCSR_CH_CABLE_TYPE0       (1 << 9)
#define TDRCSR_CH_STATUS            (1 << 8)
#define TDRCSR_CH_LENGTH7           (1 << 7)
#define TDRCSR_CH_LENGTH6           (1 << 6)
#define TDRCSR_CH_LENGTH5           (1 << 5)
#define TDRCSR_CH_LENGTH4           (1 << 4)
#define TDRCSR_CH_LENGTH3           (1 << 3)
#define TDRCSR_CH_LENGTH2           (1 << 2)
#define TDRCSR_CH_LENGTH1           (1 << 1)
#define TDRCSR_CH_LENGTH0           (1 << 0)

//SCSIR register
#define SCSIR_AMDIXCTRL             (1 << 15)
#define SCSIR_CH_SELECT             (1 << 13)
#define SCSIR_SQEOFF                (1 << 11)
#define SCSIR_XPOL                  (1 << 4)

//CLR register
#define CLR_CBLN3                   (1 << 15)
#define CLR_CBLN2                   (1 << 14)
#define CLR_CBLN1                   (1 << 13)
#define CLR_CBLN0                   (1 << 12)

//ISR register
#define ISR_WOL                     (1 << 8)
#define ISR_ENERGYON                (1 << 7)
#define ISR_AN_COMPLETE             (1 << 6)
#define ISR_REMOTE_FAULT            (1 << 5)
#define ISR_LINK_DOWN               (1 << 4)
#define ISR_AN_LP_ACK               (1 << 3)
#define ISR_PD_FAULT                (1 << 2)
#define ISR_AN_PAGE_RECEIVED        (1 << 1)

//IMR register
#define IMR_WOL                     (1 << 8)
#define IMR_ENERGYON                (1 << 7)
#define IMR_AN_COMPLETE             (1 << 6)
#define IMR_REMOTE_FAULT            (1 << 5)
#define IMR_LINK_DOWN               (1 << 4)
#define IMR_AN_LP_ACK               (1 << 3)
#define IMR_PD_FAULT                (1 << 2)
#define IMR_AN_PAGE_RECEIVED        (1 << 1)

//PSCSR register
#define PSCSR_AUTODONE              (1 << 12)
#define PSCSR_HCDSPEED2             (1 << 4)
#define PSCSR_HCDSPEED1             (1 << 3)
#define PSCSR_HCDSPEED0             (1 << 2)

//Speed indication
#define PSCSR_HCDSPEED_MASK         (7 << 2)
#define PSCSR_HCDSPEED_10BT         (1 << 2)
#define PSCSR_HCDSPEED_100BTX       (2 << 2)
#define PSCSR_HCDSPEED_10BT_FD      (5 << 2)
#define PSCSR_HCDSPEED_100BTX_FD    (6 << 2)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//LAN8742 Ethernet PHY driver
extern const PhyDriver lan8742PhyDriver;

//LAN8742 related functions
error_t lan8742Init(NetInterface *interface);

void lan8742Tick(NetInterface *interface);

void lan8742EnableIrq(NetInterface *interface);
void lan8742DisableIrq(NetInterface *interface);

void lan8742EventHandler(NetInterface *interface);

void lan8742WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t lan8742ReadPhyReg(NetInterface *interface, uint8_t address);

void lan8742DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
