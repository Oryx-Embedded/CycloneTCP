/**
 * @file lan8741_driver.h
 * @brief LAN8741 Ethernet PHY driver
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2025 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.5.2
 **/

#ifndef _LAN8741_DRIVER_H
#define _LAN8741_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef LAN8741_PHY_ADDR
   #define LAN8741_PHY_ADDR 0
#elif (LAN8741_PHY_ADDR < 0 || LAN8741_PHY_ADDR > 31)
   #error LAN8741_PHY_ADDR parameter is not valid
#endif

//LAN8741 PHY registers
#define LAN8741_BMCR                           0x00
#define LAN8741_BMSR                           0x01
#define LAN8741_PHYID1                         0x02
#define LAN8741_PHYID2                         0x03
#define LAN8741_ANAR                           0x04
#define LAN8741_ANLPAR                         0x05
#define LAN8741_ANER                           0x06
#define LAN8741_ANNPTR                         0x07
#define LAN8741_ANNPRR                         0x08
#define LAN8741_MMDACR                         0x0D
#define LAN8741_MMDAADR                        0x0E
#define LAN8741_ENCTECR                        0x10
#define LAN8741_MCSR                           0x11
#define LAN8741_SMR                            0x12
#define LAN8741_SECR                           0x1A
#define LAN8741_SCSIR                          0x1B
#define LAN8741_ISR                            0x1D
#define LAN8741_IMR                            0x1E
#define LAN8741_PSCSR                          0x1F

//LAN8741 MMD registers
#define LAN8741_PCS_CTRL1                      0x03, 0x00
#define LAN8741_PCS_STAT1                      0x03, 0x01
#define LAN8741_PCS_MMD_DEV_PRES1              0x03, 0x05
#define LAN8741_PCS_MMD_DEV_PRES2              0x03, 0x06
#define LAN8741_EEE_CAPABILITY                 0x03, 0x14
#define LAN8741_EEE_WAKE_ERROR                 0x03, 0x16
#define LAN8741_AN_MMD_DEV_PRES1               0x07, 0x05
#define LAN8741_AN_MMD_DEV_PRES2               0x07, 0x06
#define LAN8741_EEE_ADV                        0x07, 0x3C
#define LAN8741_EEE_LP_ADV                     0x07, 0x3D
#define LAN8741_VENDOR_SPECIFIC_MMD1_DEV_ID1   0x1E, 0x02
#define LAN8741_VENDOR_SPECIFIC_MMD1_DEV_ID2   0x1E, 0x03
#define LAN8741_VENDOR_SPECIFIC_MMD1_DEV_PRES1 0x1E, 0x05
#define LAN8741_VENDOR_SPECIFIC_MMD1_DEV_PRES2 0x1E, 0x06
#define LAN8741_VENDOR_SPECIFIC_MMD1_STAT      0x1E, 0x08
#define LAN8741_TDR_MATCH_THRESHOLD            0x1E, 0x0B
#define LAN8741_TDR_SHORT_OPEN_THRESHOLD       0x1E, 0x0C
#define LAN8741_VENDOR_SPECIFIC_MMD1_PKG_ID1   0x1E, 0x0E
#define LAN8741_VENDOR_SPECIFIC_MMD1_PKG_ID2   0x1E, 0x0F

//Basic Control register
#define LAN8741_BMCR_RESET                     0x8000
#define LAN8741_BMCR_LOOPBACK                  0x4000
#define LAN8741_BMCR_SPEED_SEL                 0x2000
#define LAN8741_BMCR_AN_EN                     0x1000
#define LAN8741_BMCR_POWER_DOWN                0x0800
#define LAN8741_BMCR_ISOLATE                   0x0400
#define LAN8741_BMCR_RESTART_AN                0x0200
#define LAN8741_BMCR_DUPLEX_MODE               0x0100
#define LAN8741_BMCR_COL_TEST                  0x0080

//Basic Status register
#define LAN8741_BMSR_100BT4                    0x8000
#define LAN8741_BMSR_100BTX_FD                 0x4000
#define LAN8741_BMSR_100BTX_HD                 0x2000
#define LAN8741_BMSR_10BT_FD                   0x1000
#define LAN8741_BMSR_10BT_HD                   0x0800
#define LAN8741_BMSR_100BT2_FD                 0x0400
#define LAN8741_BMSR_100BT2_HD                 0x0200
#define LAN8741_BMSR_EXTENDED_STATUS           0x0100
#define LAN8741_BMSR_AN_COMPLETE               0x0020
#define LAN8741_BMSR_REMOTE_FAULT              0x0010
#define LAN8741_BMSR_AN_CAPABLE                0x0008
#define LAN8741_BMSR_LINK_STATUS               0x0004
#define LAN8741_BMSR_JABBER_DETECT             0x0002
#define LAN8741_BMSR_EXTENDED_CAPABLE          0x0001

//PHY Identifier 1 register
#define LAN8741_PHYID1_PHY_ID_MSB              0xFFFF
#define LAN8741_PHYID1_PHY_ID_MSB_DEFAULT      0x0007

//PHY Identifier 2 register
#define LAN8741_PHYID2_PHY_ID_LSB              0xFC00
#define LAN8741_PHYID2_PHY_ID_LSB_DEFAULT      0xC000
#define LAN8741_PHYID2_MODEL_NUM               0x03F0
#define LAN8741_PHYID2_MODEL_NUM_DEFAULT       0x0120
#define LAN8741_PHYID2_REVISION_NUM            0x000F

//Auto-Negotiation Advertisement register
#define LAN8741_ANAR_NEXT_PAGE                 0x8000
#define LAN8741_ANAR_REMOTE_FAULT              0x2000
#define LAN8741_ANAR_PAUSE                     0x0C00
#define LAN8741_ANAR_100BTX_FD                 0x0100
#define LAN8741_ANAR_100BTX_HD                 0x0080
#define LAN8741_ANAR_10BT_FD                   0x0040
#define LAN8741_ANAR_10BT_HD                   0x0020
#define LAN8741_ANAR_SELECTOR                  0x001F
#define LAN8741_ANAR_SELECTOR_DEFAULT          0x0001

//Auto-Negotiation Link Partner Ability register
#define LAN8741_ANLPAR_NEXT_PAGE               0x8000
#define LAN8741_ANLPAR_ACK                     0x4000
#define LAN8741_ANLPAR_REMOTE_FAULT            0x2000
#define LAN8741_ANLPAR_PAUSE                   0x0400
#define LAN8741_ANLPAR_100BT4                  0x0200
#define LAN8741_ANLPAR_100BTX_FD               0x0100
#define LAN8741_ANLPAR_100BTX_HD               0x0080
#define LAN8741_ANLPAR_10BT_FD                 0x0040
#define LAN8741_ANLPAR_10BT_HD                 0x0020
#define LAN8741_ANLPAR_SELECTOR                0x001F
#define LAN8741_ANLPAR_SELECTOR_DEFAULT        0x0001

//Auto-Negotiation Expansion register
#define LAN8741_ANER_RECEIVE_NP_LOC_ABLE       0x0040
#define LAN8741_ANER_RECEIVE_NP_STOR_LOC       0x0020
#define LAN8741_ANER_PAR_DETECT_FAULT          0x0010
#define LAN8741_ANER_LP_NEXT_PAGE_ABLE         0x0008
#define LAN8741_ANER_NEXT_PAGE_ABLE            0x0004
#define LAN8741_ANER_PAGE_RECEIVED             0x0002
#define LAN8741_ANER_LP_AN_ABLE                0x0001

//Auto Negotiation Next Page TX register
#define LAN8741_ANNPTR_NEXT_PAGE               0x8000
#define LAN8741_ANNPTR_MSG_PAGE                0x2000
#define LAN8741_ANNPTR_ACK2                    0x1000
#define LAN8741_ANNPTR_TOGGLE                  0x0800
#define LAN8741_ANNPTR_MESSAGE                 0x07FF

//Auto Negotiation Next Page RX register
#define LAN8741_ANNPRR_NEXT_PAGE               0x8000
#define LAN8741_ANNPRR_ACK                     0x4000
#define LAN8741_ANNPRR_MSG_PAGE                0x2000
#define LAN8741_ANNPRR_ACK2                    0x1000
#define LAN8741_ANNPRR_TOGGLE                  0x0800
#define LAN8741_ANNPRR_MESSAGE                 0x07FF

//MMD Access Control register
#define LAN8741_MMDACR_FUNC                    0xC000
#define LAN8741_MMDACR_FUNC_ADDR               0x0000
#define LAN8741_MMDACR_FUNC_DATA_NO_POST_INC   0x4000
#define LAN8741_MMDACR_DEVAD                   0x001F

//EDPD NLP/Crossover Time/EEE Configuration register
#define LAN8741_ENCTECR_EDPD_TX_NLP_EN         0x8000
#define LAN8741_ENCTECR_EDPD_TX_NLP_ITS        0x6000
#define LAN8741_ENCTECR_EDPD_RX_NLP_WAKE_EN    0x1000
#define LAN8741_ENCTECR_EDPD_RX_NLP_MIDS       0x0C00
#define LAN8741_ENCTECR_PHY_EEE_EN             0x0004
#define LAN8741_ENCTECR_EDPD_EXT_CROSSOVER     0x0002
#define LAN8741_ENCTECR_EXT_CROSSOVER_TIME     0x0001

//Mode Control/Status register
#define LAN8741_MCSR_EDPWRDOWN                 0x2000
#define LAN8741_MCSR_FARLOOPBACK               0x0200
#define LAN8741_MCSR_ALTINT                    0x0040
#define LAN8741_MCSR_ENERGYON                  0x0002

//Special Modes register
#define LAN8741_SMR_MIIMODE                    0x4000
#define LAN8741_SMR_MODE                       0x00E0
#define LAN8741_SMR_PHYAD                      0x001F

//Symbol Error Counter register
#define LAN8741_SECR_SYM_ERR_CNT               0xFFFF

//Special Control/Status Indication register
#define LAN8741_SCSIR_AMDIXCTRL                0x8000
#define LAN8741_SCSIR_CH_SELECT                0x2000
#define LAN8741_SCSIR_SQEOFF                   0x0800
#define LAN8741_SCSIR_XPOL                     0x0010

//Interrupt Source Flag register
#define LAN8741_ISR_ENERGYON                   0x0080
#define LAN8741_ISR_AN_COMPLETE                0x0040
#define LAN8741_ISR_REMOTE_FAULT               0x0020
#define LAN8741_ISR_LINK_DOWN                  0x0010
#define LAN8741_ISR_AN_LP_ACK                  0x0008
#define LAN8741_ISR_PAR_DETECT_FAULT           0x0004
#define LAN8741_ISR_AN_PAGE_RECEIVED           0x0002

//Interrupt Mask register
#define LAN8741_IMR_ENERGYON                   0x0080
#define LAN8741_IMR_AN_COMPLETE                0x0040
#define LAN8741_IMR_REMOTE_FAULT               0x0020
#define LAN8741_IMR_LINK_DOWN                  0x0010
#define LAN8741_IMR_AN_LP_ACK                  0x0008
#define LAN8741_IMR_PAR_DETECT_FAULT           0x0004
#define LAN8741_IMR_AN_PAGE_RECEIVED           0x0002

//PHY Special Control/Status register
#define LAN8741_PSCSR_AUTODONE                 0x1000
#define LAN8741_PSCSR_4B5B_EN                  0x0040
#define LAN8741_PSCSR_HCDSPEED                 0x001C
#define LAN8741_PSCSR_HCDSPEED_10BT_HD         0x0004
#define LAN8741_PSCSR_HCDSPEED_100BTX_HD       0x0008
#define LAN8741_PSCSR_HCDSPEED_10BT_FD         0x0014
#define LAN8741_PSCSR_HCDSPEED_100BTX_FD       0x0018

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//LAN8741 Ethernet PHY driver
extern const PhyDriver lan8741PhyDriver;

//LAN8741 related functions
error_t lan8741Init(NetInterface *interface);
void lan8741InitHook(NetInterface *interface);

void lan8741Tick(NetInterface *interface);

void lan8741EnableIrq(NetInterface *interface);
void lan8741DisableIrq(NetInterface *interface);

void lan8741EventHandler(NetInterface *interface);

void lan8741WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t lan8741ReadPhyReg(NetInterface *interface, uint8_t address);

void lan8741DumpPhyReg(NetInterface *interface);

void lan8741WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t lan8741ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
