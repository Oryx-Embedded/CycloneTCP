/**
 * @file rtl8201_driver.h
 * @brief RTL8201 Gigabit Ethernet PHY driver
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

#ifndef _RTL8201_DRIVER_H
#define _RTL8201_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef RTL8201_PHY_ADDR
   #define RTL8201_PHY_ADDR 1
#elif (RTL8201_PHY_ADDR < 0 || RTL8201_PHY_ADDR > 31)
   #error RTL8201_PHY_ADDR parameter is not valid
#endif

//RTL8201 PHY registers
#define RTL8201_BMCR                          0x00
#define RTL8201_BMSR                          0x01
#define RTL8201_PHYID1                        0x02
#define RTL8201_PHYID2                        0x03
#define RTL8201_ANAR                          0x04
#define RTL8201_ANLPAR                        0x05
#define RTL8201_ANER                          0x06
#define RTL8201_MACR                          0x0D
#define RTL8201_MAADR                         0x0E
#define RTL8201_PSMR                          0x18
#define RTL8201_FMLR                          0x1C
#define RTL8201_IISDR                         0x1E
#define RTL8201_PDR                           0x1F

//RTL8201 Extended registers (page 4)
#define RTL8201_EEECER                        0x10
#define RTL8201_EEECR                         0x15

//RTL8201 Extended registers (page 7)
#define RTL8201_RMSR                          0x10
#define RTL8201_CLSR                          0x11
#define RTL8201_EEELER                        0x12
#define RTL8201_IWELFR                        0x13
#define RTL8201_MTIR                          0x14
#define RTL8201_SSCR                          0x18

//RTL8201 MMD registers
#define RTL8201_EEEPC1R                       0x03, 0x00
#define RTL8201_EEEPS1R                       0x03, 0x01
#define RTL8201_EEECR2                        0x03, 0x14
#define RTL8201_EEEWER                        0x03, 0x16
#define RTL8201_EEEAR                         0x07, 0x3C
#define RTL8201_EEELPAR                       0x07, 0x3D

//Basic Mode Control register
#define RTL8201_BMCR_RESET                    0x8000
#define RTL8201_BMCR_LOOPBACK                 0x4000
#define RTL8201_BMCR_SPEED_SEL_LSB            0x2000
#define RTL8201_BMCR_AN_EN                    0x1000
#define RTL8201_BMCR_POWER_DOWN               0x0800
#define RTL8201_BMCR_ISOLATE                  0x0400
#define RTL8201_BMCR_RESTART_AN               0x0200
#define RTL8201_BMCR_DUPLEX_MODE              0x0100
#define RTL8201_BMCR_COL_TEST                 0x0080
#define RTL8201_BMCR_SPEED_SEL_MSB            0x0040

//Basic Mode Status register
#define RTL8201_BMSR_100BT4                   0x8000
#define RTL8201_BMSR_100BTX_FD                0x4000
#define RTL8201_BMSR_100BTX_HD                0x2000
#define RTL8201_BMSR_10BT_FD                  0x1000
#define RTL8201_BMSR_10BT_HD                  0x0800
#define RTL8201_BMSR_MF_PREAMBLE_SUPPR        0x0040
#define RTL8201_BMSR_AN_COMPLETE              0x0020
#define RTL8201_BMSR_REMOTE_FAULT             0x0010
#define RTL8201_BMSR_AN_CAPABLE               0x0008
#define RTL8201_BMSR_LINK_STATUS              0x0004
#define RTL8201_BMSR_JABBER_DETECT            0x0002
#define RTL8201_BMSR_EXTENDED_CAPABLE         0x0001

//PHY Identifier 1 register
#define RTL8201_PHYID1_OUI_MSB                0xFFFF
#define RTL8201_PHYID1_OUI_MSB_DEFAULT        0x001C

//PHY Identifier 2 register
#define RTL8201_PHYID2_OUI_LSB                0xFC00
#define RTL8201_PHYID2_OUI_LSB_DEFAULT        0xC800
#define RTL8201_PHYID2_MODEL_NUM              0x03F0
#define RTL8201_PHYID2_MODEL_NUM_DEFAULT      0x0010
#define RTL8201_PHYID2_REVISION_NUM           0x000F
#define RTL8201_PHYID2_REVISION_NUM_DEFAULT   0x0006

//Auto-Negotiation Advertisement register
#define RTL8201_ANAR_NEXT_PAGE                0x8000
#define RTL8201_ANAR_ACK                      0x4000
#define RTL8201_ANAR_REMOTE_FAULT             0x2000
#define RTL8201_ANAR_ASYM_PAUSE               0x0800
#define RTL8201_ANAR_PAUSE                    0x0400
#define RTL8201_ANAR_100BT4                   0x0200
#define RTL8201_ANAR_100BTX_FD                0x0100
#define RTL8201_ANAR_100BTX_HD                0x0080
#define RTL8201_ANAR_10BT_FD                  0x0040
#define RTL8201_ANAR_10BT_HD                  0x0020
#define RTL8201_ANAR_SELECTOR                 0x001F
#define RTL8201_ANAR_SELECTOR_DEFAULT         0x0001

//Auto-Negotiation Link Partner Ability register
#define RTL8201_ANLPAR_NEXT_PAGE              0x8000
#define RTL8201_ANLPAR_ACK                    0x4000
#define RTL8201_ANLPAR_REMOTE_FAULT           0x2000
#define RTL8201_ANLPAR_ASYM_PAUSE             0x0800
#define RTL8201_ANLPAR_PAUSE                  0x0400
#define RTL8201_ANLPAR_100BT4                 0x0200
#define RTL8201_ANLPAR_100BTX_FD              0x0100
#define RTL8201_ANLPAR_100BTX_HD              0x0080
#define RTL8201_ANLPAR_10BT_FD                0x0040
#define RTL8201_ANLPAR_10BT_HD                0x0020
#define RTL8201_ANLPAR_SELECTOR               0x001F
#define RTL8201_ANLPAR_SELECTOR_DEFAULT       0x0001

//Auto-Negotiation Expansion register
#define RTL8201_ANER_PAR_DETECT_FAULT         0x0010
#define RTL8201_ANER_LP_NEXT_PAGE_ABLE        0x0008
#define RTL8201_ANER_NEXT_PAGE_ABLE           0x0004
#define RTL8201_ANER_PAGE_RECEIVED            0x0002
#define RTL8201_ANER_LP_AN_ABLE               0x0001

//MMD Access Control register
#define RTL8201_MACR_FUNC                     0xC000
#define RTL8201_MACR_FUNC_ADDR                0x0000
#define RTL8201_MACR_FUNC_DATA_NO_POST_INC    0x4000
#define RTL8201_MACR_FUNC_DATA_POST_INC_RW    0x8000
#define RTL8201_MACR_FUNC_DATA_POST_INC_W     0xC000
#define RTL8201_MACR_DEVAD                    0x001F

//Power Saving Mode register
#define RTL8201_PSMR_EN_PWR_SAVE              0x8000

//Fiber Mode and Loopback register
#define RTL8201_FMLR_FX_MODE                  0x0020
#define RTL8201_FMLR_EN_AUTO_MDIX             0x0004
#define RTL8201_FMLR_FORCE_MDI                0x0002

//Interrupt Indicators and SNR Display register
#define RTL8201_IISDR_AN_ERR                  0x8000
#define RTL8201_IISDR_SPD_CHG                 0x4000
#define RTL8201_IISDR_DUPLEX_CHG              0x2000
#define RTL8201_IISDR_LINK_STATUS_CHG         0x0800
#define RTL8201_IISDR_SNR_O                   0x000F

//Page Select register
#define RTL8201_PDR_PAGE_SEL                  0x00FF

//EEE Capability Enable register
#define RTL8201_EEECER_EEE_10_CAP             0x2000
#define RTL8201_EEECER_EEE_NWAY_EN            0x1000
#define RTL8201_EEECER_TX_QUIET_EN            0x0200
#define RTL8201_EEECER_RX_QUIET_EN            0x0100

//EEE Capability register
#define RTL8201_EEECR_RG_DIS_LDVT             0x1000
#define RTL8201_EEECR_EEE_100_CAP             0x0001

//RMII Mode Setting register
#define RTL8201_RMSR_RG_RMII_CLKDIR           0x1000
#define RTL8201_RMSR_RG_RMII_TX_OFFSET        0x0F00
#define RTL8201_RMSR_RG_RMII_RX_OFFSET        0x00F0
#define RTL8201_RMSR_RG_RMII_RXDV_SEL         0x0004
#define RTL8201_RMSR_RG_RMII_RXDSEL           0x0002

//Customized LEDs Setting register
#define RTL8201_CLSR_LED_SEL1                 0x00F0
#define RTL8201_CLSR_LED_SEL1_ACT             0x0080
#define RTL8201_CLSR_LED_SEL1_LINK_100M       0x0020
#define RTL8201_CLSR_LED_SEL1_LINK_10M        0x0010
#define RTL8201_CLSR_LED_SEL0                 0x000F
#define RTL8201_CLSR_LED_SEL0_ACT             0x0008
#define RTL8201_CLSR_LED_SEL0_LINK_100M       0x0002
#define RTL8201_CLSR_LED_SEL0_LINK_10M        0x0001

//EEE LEDs Enable register
#define RTL8201_EEELER_EEE_LED_EN1            0x0002
#define RTL8201_EEELER_EEE_LED_EN0            0x0001

//Interrupt, WOL Enable, and LEDs Function register
#define RTL8201_IWELFR_INT_LINK_CHG           0x2000
#define RTL8201_IWELFR_INT_DUP_CHG            0x1000
#define RTL8201_IWELFR_INT_AN_ERR             0x0800
#define RTL8201_IWELFR_RG_LED0_WOL_SEL        0x0400
#define RTL8201_IWELFR_LED_SEL                0x0030
#define RTL8201_IWELFR_CUSTOMIZED_LED         0x0008
#define RTL8201_IWELFR_EN_10M_LPI             0x0001

//MII TX Isolate register
#define RTL8201_MTIR_RG_TX_ISOLATE_EN         0x8000

//Spread Spectrum Clock register
#define RTL8201_SSCR_RG_DIS_SSC               0x0001

//PCS Control 1  register
#define RTL8201_EEEPC1R_CLOCK_STOP_EN         0x0400

//PCS Status 1 register
#define RTL8201_EEEPS1R_TX_LPI_RCVD           0x0800
#define RTL8201_EEEPS1R_RX_LPI_RCVD           0x0400
#define RTL8201_EEEPS1R_TX_LPI_IND            0x0200
#define RTL8201_EEEPS1R_RX_LPI_IND            0x0100
#define RTL8201_EEEPS1R_CLK_STOP_CAPABLE      0x0040

//EEE Capability 2 register
#define RTL8201_EEECR2_100BTX_EEE             0x0002

//EEE Wake Error register
#define RTL8201_EEEWER_EEE_WAKE_ERROR_COUNTER 0xFFFF

//EEE Advertisement register
#define RTL8201_EEEAR_100BTX_EEE              0x0002

//EEE Link Partner Ability register
#define RTL8201_EEELPAR_LP_100BTX_EEE         0x0002

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//RTL8201 Ethernet PHY driver
extern const PhyDriver rtl8201PhyDriver;

//RTL8201 related functions
error_t rtl8201Init(NetInterface *interface);
void rtl8201InitHook(NetInterface *interface);

void rtl8201Tick(NetInterface *interface);

void rtl8201EnableIrq(NetInterface *interface);
void rtl8201DisableIrq(NetInterface *interface);

void rtl8201EventHandler(NetInterface *interface);

void rtl8201WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t rtl8201ReadPhyReg(NetInterface *interface, uint8_t address);

void rtl8201DumpPhyReg(NetInterface *interface);

void rtl8201WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t rtl8201ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
