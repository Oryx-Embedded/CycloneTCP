/**
 * @file dp83865_driver.h
 * @brief DP83865 Gigabit Ethernet PHY driver
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2026 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.6.0
 **/

#ifndef _DP83865_DRIVER_H
#define _DP83865_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef DP83865_PHY_ADDR
   #define DP83865_PHY_ADDR 0
#elif (DP83865_PHY_ADDR < 0 || DP83865_PHY_ADDR > 31)
   #error DP83865_PHY_ADDR parameter is not valid
#endif

//DP83865 PHY registers
#define DP83865_BMCR                              0x00
#define DP83865_BMSR                              0x01
#define DP83865_PHYIDR1                           0x02
#define DP83865_PHYIDR2                           0x03
#define DP83865_ANAR                              0x04
#define DP83865_ANLPAR                            0x05
#define DP83865_ANER                              0x06
#define DP83865_ANNPTR                            0x07
#define DP83865_ANNPRR                            0x08
#define DP83865_1KTCR                             0x09
#define DP83865_1KSTSR                            0x0A
#define DP83865_1KSCR                             0x0F
#define DP83865_STRAP_REG                         0x10
#define DP83865_LINK_AN                           0x11
#define DP83865_AUX_CTRL                          0x12
#define DP83865_LED_CTRL                          0x13
#define DP83865_INT_STATUS                        0x14
#define DP83865_INT_MASK                          0x15
#define DP83865_EXP_MEM_CTL                       0x16
#define DP83865_INT_CLEAR                         0x17
#define DP83865_BIST_CNT                          0x18
#define DP83865_BIST_CFG1                         0x19
#define DP83865_BIST_CFG2                         0x1A
#define DP83865_EXP_MEM_DATA                      0x1D
#define DP83865_EXP_MEM_ADDR                      0x1E
#define DP83865_PHY_SUP                           0x1F

//Basic Mode Control register
#define DP83865_BMCR_RESET                        0x8000
#define DP83865_BMCR_LOOPBACK                     0x4000
#define DP83865_BMCR_SPEED_LSB                    0x2000
#define DP83865_BMCR_AN_EN                        0x1000
#define DP83865_BMCR_POWER_DOWN                   0x0800
#define DP83865_BMCR_ISOLATE                      0x0400
#define DP83865_BMCR_RESTART_AN                   0x0200
#define DP83865_BMCR_DUPLEX                       0x0100
#define DP83865_BMCR_COL_TEST                     0x0080
#define DP83865_BMCR_SPEED_MSB                    0x0040

//Basic Mode Status register
#define DP83865_BMSR_100BT4                       0x8000
#define DP83865_BMSR_100BTX_FD                    0x4000
#define DP83865_BMSR_100BTX_HD                    0x2000
#define DP83865_BMSR_10BT_FD                      0x1000
#define DP83865_BMSR_10BT_HD                      0x0800
#define DP83865_BMSR_100BT2_FD                    0x0400
#define DP83865_BMSR_100BT2_HD                    0x0200
#define DP83865_BMSR_EXTENDED_STATUS              0x0100
#define DP83865_BMSR_PREAMBLE_SUPPR               0x0040
#define DP83865_BMSR_AN_COMPLETE                  0x0020
#define DP83865_BMSR_REMOTE_FAULT                 0x0010
#define DP83865_BMSR_AN_CAPABLE                   0x0008
#define DP83865_BMSR_LINK_STATUS                  0x0004
#define DP83865_BMSR_JABBER_DETECT                0x0002
#define DP83865_BMSR_EXTENDED_CAPABLE             0x0001

//PHY Identifier 1 register
#define DP83865_PHYIDR1_OUI_MSB                   0xFFFF
#define DP83865_PHYIDR1_OUI_MSB_DEFAULT           0x2000

//PHY Identifier 2 register
#define DP83865_PHYIDR2_OUI_LSB                   0xFC00
#define DP83865_PHYIDR2_OUI_LSB_DEFAULT           0x5C00
#define DP83865_PHYIDR2_VNDR_MDL                  0x03F0
#define DP83865_PHYIDR2_VNDR_MDL_DEFAULT          0x0070
#define DP83865_PHYIDR2_MDL_REV                   0x000F
#define DP83865_PHYIDR2_MDL_REV_DEFAULT           0x000A

//Auto-Negotiation Advertisement register
#define DP83865_ANAR_NP                           0x8000
#define DP83865_ANAR_RF                           0x2000
#define DP83865_ANAR_ASY_PAUSE                    0x0800
#define DP83865_ANAR_PAUSE                        0x0400
#define DP83865_ANAR_100BT4                       0x0200
#define DP83865_ANAR_100BTX_FD                    0x0100
#define DP83865_ANAR_100BTX_HD                    0x0080
#define DP83865_ANAR_10BT_FD                      0x0040
#define DP83865_ANAR_10BT_HD                      0x0020
#define DP83865_ANAR_PSB                          0x001F

//Auto-Negotiation Link Partner Ability register
#define DP83865_ANLPAR_NP                         0x8000
#define DP83865_ANLPAR_ACK                        0x4000
#define DP83865_ANLPAR_RF                         0x2000
#define DP83865_ANLPAR_ASY_PAUSE                  0x0800
#define DP83865_ANLPAR_PAUSE                      0x0400
#define DP83865_ANLPAR_100BT4                     0x0200
#define DP83865_ANLPAR_100BTX_FD                  0x0100
#define DP83865_ANLPAR_100BTX_HD                  0x0080
#define DP83865_ANLPAR_10BT_FD                    0x0040
#define DP83865_ANLPAR_10BT_HD                    0x0020
#define DP83865_ANLPAR_PSB                        0x001F

//Auto-Negotiation Expansion register
#define DP83865_ANER_PDF                          0x0010
#define DP83865_ANER_LP_NP_ABLE                   0x0008
#define DP83865_ANER_NP_ABLE                      0x0004
#define DP83865_ANER_PAGE_RX                      0x0002
#define DP83865_ANER_LP_AN_ABLE                   0x0001

//Auto-Negotiation Next Page TX register
#define DP83865_ANNPTR_NP                         0x8000
#define DP83865_ANNPTR_ACK                        0x4000
#define DP83865_ANNPTR_MP                         0x2000
#define DP83865_ANNPTR_ACK2                       0x1000
#define DP83865_ANNPTR_TOG_TX                     0x0800
#define DP83865_ANNPTR_CODE                       0x07FF

//Auto-Negotiation Next Page RX register
#define DP83865_ANNPRR_NP                         0x8000
#define DP83865_ANNPRR_ACK                        0x4000
#define DP83865_ANNPRR_MP                         0x2000
#define DP83865_ANNPRR_ACK2                       0x1000
#define DP83865_ANNPRR_TOGGLE                     0x0800
#define DP83865_ANNPRR_CODE                       0x07FF

//1000BASE-T Control register
#define DP83865_1KTCR_TEST_MODE                   0xE000
#define DP83865_1KTCR_TEST_MODE_NORMAL            0x0000
#define DP83865_1KTCR_TEST_MODE_1                 0x2000
#define DP83865_1KTCR_TEST_MODE_2                 0x4000
#define DP83865_1KTCR_TEST_MODE_3                 0x6000
#define DP83865_1KTCR_TEST_MODE_4                 0x8000
#define DP83865_1KTCR_MS_MAN_CONF_EN              0x1000
#define DP83865_1KTCR_MS_MAN_CONF_VAL             0x0800
#define DP83865_1KTCR_REPEATER_DT                 0x0400
#define DP83865_1KTCR_1000BT_FD                   0x0200
#define DP83865_1KTCR_1000BT_HD                   0x0100

//1000BASE-T Status register
#define DP83865_1KSTSR_MS_CONF_FAULT              0x8000
#define DP83865_1KSTSR_MS_CONF_RES                0x4000
#define DP83865_1KSTSR_LOCAL_RECEIVER_STATUS      0x2000
#define DP83865_1KSTSR_REMOTE_RECEIVER_STATUS     0x1000
#define DP83865_1KSTSR_LP_1000BT_FD               0x0800
#define DP83865_1KSTSR_LP_1000BT_HD               0x0400
#define DP83865_1KSTSR_IDLE_ERR_COUNT             0x00FF

//1000BASE-T Extended Status register
#define DP83865_1KSCR_1000BX_FD                   0x8000
#define DP83865_1KSCR_1000BX_HD                   0x4000
#define DP83865_1KSCR_1000BT_FD                   0x2000
#define DP83865_1KSCR_1000BT_HD                   0x1000

//Strap Options register
#define DP83865_STRAP_REG_AN_EN                   0x8000
#define DP83865_STRAP_REG_DUPLEX_MODE             0x4000
#define DP83865_STRAP_REG_SPEED                   0x3000
#define DP83865_STRAP_REG_NC_MODE_EN              0x0400
#define DP83865_STRAP_REG_MAC_CLOCK_EN            0x0080
#define DP83865_STRAP_REG_MDIX_EN                 0x0040
#define DP83865_STRAP_REG_MULTI_EN                0x0020
#define DP83865_STRAP_REG_PHYADDR                 0x001F

//Link and Auto-Negotiation Status register
#define DP83865_LINK_AN_TP_POLARITY               0xF000
#define DP83865_LINK_AN_POWER_DOWN_STATUS         0x0800
#define DP83865_LINK_AN_MDIX_STATUS               0x0400
#define DP83865_LINK_AN_FIFO_ERROR                0x0200
#define DP83865_LINK_AN_SHALLOW_LOOPBACK_STATUS   0x0080
#define DP83865_LINK_AN_DEEP_LOOPBACK_STATUS      0x0040
#define DP83865_LINK_AN_NON_COMPLIANT_MODE_STATUS 0x0020
#define DP83865_LINK_AN_SPEED_STATUS              0x0018
#define DP83865_LINK_AN_SPEED_STATUS_10MBPS       0x0000
#define DP83865_LINK_AN_SPEED_STATUS_100MBPS      0x0008
#define DP83865_LINK_AN_SPEED_STATUS_1000MBPS     0x0010
#define DP83865_LINK_AN_LINK_STATUS               0x0004
#define DP83865_LINK_AN_DUPLEX_STATUS             0x0002
#define DP83865_LINK_AN_MS_CONF_STATUS            0x0001

//Auxiliary Control register
#define DP83865_AUX_CTRL_AUTO_MDIX_EN             0x8000
#define DP83865_AUX_CTRL_MANUAL_MDIX_VALUE        0x4000
#define DP83865_AUX_CTRL_RGMII_EN                 0x3000
#define DP83865_AUX_CTRL_RGMII_EN_GMII_MODE       0x0000
#define DP83865_AUX_CTRL_RGMII_EN_RGMII_HP_MODE   0x2000
#define DP83865_AUX_CTRL_RGMII_EN_RGMII_3COM_MODE 0x3000
#define DP83865_AUX_CTRL_NON_COMPLIANT_MODE       0x0200
#define DP83865_AUX_CTRL_RGMII_INBAND_STATUS_EN   0x0100
#define DP83865_AUX_CTRL_TX_TCLK_EN               0x0080
#define DP83865_AUX_CTRL_TX_TRIGGER_SYN_EN        0x0040
#define DP83865_AUX_CTRL_SHALLOW_DEEP_LOOPBACK_EN 0x0020
#define DP83865_AUX_CTRL_X_MAC                    0x0010
#define DP83865_AUX_CTRL_JABBER_DIS               0x0001

//LED Control register
#define DP83865_LED_CTRL_ACT_LED                  0xC000
#define DP83865_LED_CTRL_ACT_LED_NORMAL           0x0000
#define DP83865_LED_CTRL_ACT_LED_FORCED_OFF       0x4000
#define DP83865_LED_CTRL_ACT_LED_BLINK_MODE       0x8000
#define DP83865_LED_CTRL_ACT_LED_FORCED_ON        0xC000
#define DP83865_LED_CTRL_LINK10_LED               0x3000
#define DP83865_LED_CTRL_LINK10_LED_NORMAL        0x0000
#define DP83865_LED_CTRL_LINK10_LED_FORCED_OFF    0x1000
#define DP83865_LED_CTRL_LINK10_LED_BLINK_MODE    0x2000
#define DP83865_LED_CTRL_LINK10_LED_FORCED_ON     0x3000
#define DP83865_LED_CTRL_LINK100_LED              0x0C00
#define DP83865_LED_CTRL_LINK100_LED_NORMAL       0x0000
#define DP83865_LED_CTRL_LINK100_LED_FORCED_OFF   0x0400
#define DP83865_LED_CTRL_LINK100_LED_BLINK_MODE   0x0800
#define DP83865_LED_CTRL_LINK100_LED_FORCED_ON    0x0C00
#define DP83865_LED_CTRL_LINK1000_LED             0x0300
#define DP83865_LED_CTRL_LINK1000_LED_NORMAL      0x0000
#define DP83865_LED_CTRL_LINK1000_LED_FORCED_OFF  0x0100
#define DP83865_LED_CTRL_LINK1000_LED_BLINK_MODE  0x0200
#define DP83865_LED_CTRL_LINK1000_LED_FORCED_ON   0x0300
#define DP83865_LED_CTRL_DUPLEX_LED               0x00C0
#define DP83865_LED_CTRL_DUPLEX_LED_NORMAL        0x0000
#define DP83865_LED_CTRL_DUPLEX_LED_FORCED_OFF    0x0040
#define DP83865_LED_CTRL_DUPLEX_LED_BLINK_MODE    0x0080
#define DP83865_LED_CTRL_DUPLEX_LED_FORCED_ON     0x00C0
#define DP83865_LED_CTRL_REDUCED_LED_EN           0x0020
#define DP83865_LED_CTRL_LED_ON_CRC               0x0010
#define DP83865_LED_CTRL_LED_ON_IE                0x0008
#define DP83865_LED_CTRL_AN_FALLBACK_AN           0x0004
#define DP83865_LED_CTRL_AN_FALLBACK_CRC          0x0002
#define DP83865_LED_CTRL_AN_FALLBACK_IE           0x0001

//Interrupt Status register
#define DP83865_INT_STATUS_SPD_CNG_INT            0x8000
#define DP83865_INT_STATUS_LNK_CNG_INT            0x4000
#define DP83865_INT_STATUS_DPLX_CNG_INT           0x2000
#define DP83865_INT_STATUS_MDIX_CNG_INT           0x1000
#define DP83865_INT_STATUS_POL_CNG_INT            0x0800
#define DP83865_INT_STATUS_PRL_DET_FLT_INT        0x0400
#define DP83865_INT_STATUS_MAS_SLA_ERR_INT        0x0200
#define DP83865_INT_STATUS_NO_HCD_INT             0x0100
#define DP83865_INT_STATUS_NO_LNK_INT             0x0080
#define DP83865_INT_STATUS_JABBER_CNG_INT         0x0040
#define DP83865_INT_STATUS_NXT_PG_RCVD_INT        0x0020
#define DP83865_INT_STATUS_AN_CMPL_INT            0x0010
#define DP83865_INT_STATUS_REM_FLT_CNG_INT        0x0008

//Interrupt Mask register
#define DP83865_INT_MASK_SPD_CNG_INT_MSK          0x8000
#define DP83865_INT_MASK_LNK_CNG_INT_MSK          0x4000
#define DP83865_INT_MASK_DPLX_CNG_INT_MSK         0x2000
#define DP83865_INT_MASK_MDIX_CNG_INT_MSK         0x1000
#define DP83865_INT_MASK_POL_CNG_INT_MSK          0x0800
#define DP83865_INT_MASK_PRL_DET_FLT_INT_MSK      0x0400
#define DP83865_INT_MASK_MAS_SLA_ERR_INT_MSK      0x0200
#define DP83865_INT_MASK_NO_HCD_INT_MSK           0x0100
#define DP83865_INT_MASK_NO_LNK_INT_MSK           0x0080
#define DP83865_INT_MASK_JABBER_CNG_INT_MSK       0x0040
#define DP83865_INT_MASK_NXT_PG_RCVD_INT_MSK      0x0020
#define DP83865_INT_MASK_AN_CMPL_INT_MSK          0x0010
#define DP83865_INT_MASK_REM_FLT_CNG_INT_MSK      0x0008

//Expanded Memory Access Control register
#define DP83865_EXP_MEM_CTL_GLOBAL_RESET          0x8000
#define DP83865_EXP_MEM_CTL_BROADCAST_EN          0x0080
#define DP83865_EXP_MEM_CTL_ADDRESS_CONTROL       0x0003

//Interrupt Clear register
#define DP83865_INT_CLEAR_SPD_CNG_INT_CLR         0x8000
#define DP83865_INT_CLEAR_LNK_CNG_INT_CLR         0x4000
#define DP83865_INT_CLEAR_DPLX_CNG_INT_CLR        0x2000
#define DP83865_INT_CLEAR_MDIX_CNG_INT_CLR        0x1000
#define DP83865_INT_CLEAR_POL_CNG_INT_CLR         0x0800
#define DP83865_INT_CLEAR_PRL_DET_FLT_INT_CLR     0x0400
#define DP83865_INT_CLEAR_MAS_SLA_ERR_INT_CLR     0x0200
#define DP83865_INT_CLEAR_NO_HCD_INT_CLR          0x0100
#define DP83865_INT_CLEAR_NO_LNK_INT_CLR          0x0080
#define DP83865_INT_CLEAR_JABBER_CNG_INT_CLR      0x0040
#define DP83865_INT_CLEAR_NXT_PG_RCVD_INT_CLR     0x0020
#define DP83865_INT_CLEAR_AN_CMPL_INT_CLR         0x0010
#define DP83865_INT_CLEAR_REM_FLT_CNG_INT_CLR     0x0008

//BIST Configuration 1 register
#define DP83865_BIST_CFG1_BIST_CNT_TYPE           0x8000
#define DP83865_BIST_CFG1_BIST_CNT_CLR            0x4000
#define DP83865_BIST_CFG1_TX_BIST_PAK_LEN         0x2000
#define DP83865_BIST_CFG1_TX_BIST_IFG             0x1000
#define DP83865_BIST_CFG1_TX_BIST_EN              0x0800
#define DP83865_BIST_CFG1_TX_BIST_PAK_TYPE        0x0400
#define DP83865_BIST_CFG1_TX_BIST_PAK             0x00FF

//BIST Configuration 2 register
#define DP83865_BIST_CFG2_RX_BIST_EN              0x8000
#define DP83865_BIST_CFG2_BIST_CNT_SEL            0x4000
#define DP83865_BIST_CFG2_TX_BIST_PAK_CNT         0x3800
#define DP83865_BIST_CFG2_LINK_LINK_ACT_SEL       0x0001

//PHY Support register
#define DP83865_PHY_SUP_PHY_ADDR                  0x001F

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//DP83865 Ethernet PHY driver
extern const PhyDriver dp83865PhyDriver;

//DP83865 related functions
error_t dp83865Init(NetInterface *interface);
void dp83865InitHook(NetInterface *interface);

void dp83865Tick(NetInterface *interface);

void dp83865EnableIrq(NetInterface *interface);
void dp83865DisableIrq(NetInterface *interface);

void dp83865EventHandler(NetInterface *interface);

void dp83865WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t dp83865ReadPhyReg(NetInterface *interface, uint8_t address);

void dp83865DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
