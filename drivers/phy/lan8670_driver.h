/**
 * @file lan8670_driver.h
 * @brief LAN8670 10Base-T1S Ethernet PHY driver
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

#ifndef _LAN8670_DRIVER_H
#define _LAN8670_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef LAN8670_PHY_ADDR
   #define LAN8670_PHY_ADDR 0
#elif (LAN8670_PHY_ADDR < 0 || LAN8670_PHY_ADDR > 31)
   #error LAN8670_PHY_ADDR parameter is not valid
#endif

//PLCA support
#ifndef LAN8670_PLCA_SUPPORT
   #define LAN8670_PLCA_SUPPORT ENABLED
#elif (LAN8670_PLCA_SUPPORT != ENABLED && LAN8670_PLCA_SUPPORT != DISABLED)
   #error LAN8670_PLCA_SUPPORT parameter is not valid
#endif

//Node count
#ifndef LAN8670_NODE_COUNT
   #define LAN8670_NODE_COUNT 8
#elif (LAN8670_NODE_COUNT < 0 || LAN8670_NODE_COUNT > 255)
   #error LAN8670_NODE_COUNT parameter is not valid
#endif

//Local ID
#ifndef LAN8670_LOCAL_ID
   #define LAN8670_LOCAL_ID 1
#elif (LAN8670_LOCAL_ID < 0 || LAN8670_LOCAL_ID > 255)
   #error LAN8670_LOCAL_ID parameter is not valid
#endif

//LAN8670 PHY registers
#define LAN8670_BASIC_CONTROL                   0x00
#define LAN8670_BASIC_STATUS                    0x01
#define LAN8670_PHY_ID0                         0x02
#define LAN8670_PHY_ID1                         0x03
#define LAN8670_MMDCTRL                         0x0D
#define LAN8670_MMDAD                           0x0E
#define LAN8670_STRAP_CTRL0                     0x12

//LAN8670 MMD registers
#define LAN8670_PMA_PMD_EXT_ABILITY             0x01, 0x0012
#define LAN8670_T1PMAPMDCTL                     0x01, 0x0834
#define LAN8670_T1SPMACTL                       0x01, 0x08F9
#define LAN8670_T1SPMASTS                       0x01, 0x08FA
#define LAN8670_T1STSTCTL                       0x01, 0x08FB
#define LAN8670_T1SPCSCTL                       0x02, 0x08F3
#define LAN8670_T1SPCSSTS                       0x02, 0x08F4
#define LAN8670_T1SPCSDIAG1                     0x02, 0x08F5
#define LAN8670_T1SPCSDIAG2                     0x02, 0x08F6
#define LAN8670_CTRL1                           0x1F, 0x0010
#define LAN8670_STS1                            0x1F, 0x0018
#define LAN8670_STS2                            0x1F, 0x0019
#define LAN8670_STS3                            0x1F, 0x001A
#define LAN8670_IMSK1                           0x1F, 0x001C
#define LAN8670_IMSK2                           0x1F, 0x001D
#define LAN8670_CTRCTRL                         0x1F, 0x0020
#define LAN8670_TOCNTH                          0x1F, 0x0024
#define LAN8670_TOCNTL                          0x1F, 0x0025
#define LAN8670_BCNCNTH                         0x1F, 0x0026
#define LAN8670_BCNCNTL                         0x1F, 0x0027
#define LAN8670_PADCTRL3                        0x1F, 0x00CB
#define LAN8670_MIDVER                          0x1F, 0xCA00
#define LAN8670_PLCA_CTRL0                      0x1F, 0xCA01
#define LAN8670_PLCA_CTRL1                      0x1F, 0xCA02
#define LAN8670_PLCA_STS                        0x1F, 0xCA03
#define LAN8670_PLCA_TOTMR                      0x1F, 0xCA04
#define LAN8670_PLCA_BURST                      0x1F, 0xCA05

//BASIC_CONTROL register
#define LAN8670_BASIC_CONTROL_SW_RESET          0x8000
#define LAN8670_BASIC_CONTROL_LOOPBACK          0x4000
#define LAN8670_BASIC_CONTROL_SPD_SEL_LSB       0x2000
#define LAN8670_BASIC_CONTROL_AUTO_NEG_EN       0x1000
#define LAN8670_BASIC_CONTROL_PD                0x0800
#define LAN8670_BASIC_CONTROL_ISOLATE           0x0400
#define LAN8670_BASIC_CONTROL_RE_AUTO_NEG       0x0200
#define LAN8670_BASIC_CONTROL_DUPLEX_MODE       0x0100
#define LAN8670_BASIC_CONTROL_COL_TEST          0x0080
#define LAN8670_BASIC_CONTROL_SPD_SEL_MSB       0x0040

//BASIC_STATUS register
#define LAN8670_BASIC_STATUS_100BT4             0x8000
#define LAN8670_BASIC_STATUS_100BTX_FD          0x4000
#define LAN8670_BASIC_STATUS_100BTX_HD          0x2000
#define LAN8670_BASIC_STATUS_10BT_FD            0x1000
#define LAN8670_BASIC_STATUS_10BT_HD            0x0800
#define LAN8670_BASIC_STATUS_100BT2_FD          0x0400
#define LAN8670_BASIC_STATUS_100BT2_HD          0x0200
#define LAN8670_BASIC_STATUS_EXT_STAT           0x0100
#define LAN8670_BASIC_STATUS_MF_PRE_SUP         0x0040
#define LAN8670_BASIC_STATUS_AUTO_NEG_COMP      0x0020
#define LAN8670_BASIC_STATUS_RMT_FAULT          0x0010
#define LAN8670_BASIC_STATUS_AUTO_NEG           0x0008
#define LAN8670_BASIC_STATUS_LINK_STAT          0x0004
#define LAN8670_BASIC_STATUS_JAB_DET            0x0002
#define LAN8670_BASIC_STATUS_EXT_CAP            0x0001

//PHY_ID0 register
#define LAN8670_PHY_ID0_OUI_2_9                 0xFF00
#define LAN8670_PHY_ID0_OUI_2_9_DEFAULT         0x0000
#define LAN8670_PHY_ID0_OUI_10_17               0x00FF
#define LAN8670_PHY_ID0_OUI_10_17_DEFAULT       0x0007

//PHY_ID1 register
#define LAN8670_PHY_ID1_OUI_18_23               0xFC00
#define LAN8670_PHY_ID1_OUI_18_23_DEFAULT       0xC000
#define LAN8670_PHY_ID1_MODEL                   0x03F0
#define LAN8670_PHY_ID1_MODEL_DEFAULT           0x0160
#define LAN8670_PHY_ID1_REV                     0x000F
#define LAN8670_PHY_ID1_REV_0                   0x0000
#define LAN8670_PHY_ID1_REV_2                   0x0002

//MMDCTRL register
#define LAN8670_MMDCTRL_FNCTN                   0xC000
#define LAN8670_MMDCTRL_FNCTN_ADDR              0x0000
#define LAN8670_MMDCTRL_FNCTN_DATA_NO_POST_INC  0x4000
#define LAN8670_MMDCTRL_FNCTN_DATA_POST_INC_RW  0x8000
#define LAN8670_MMDCTRL_FNCTN_DATA_POST_INC_W   0xC000
#define LAN8670_MMDCTRL_DEVAD                   0x001F
#define LAN8670_MMDCTRL_DEVAD_PMA_PMD           0x0001
#define LAN8670_MMDCTRL_DEVAD_PCS               0x0002
#define LAN8670_MMDCTRL_DEVAD_VENDOR_SPECIFIC_2 0x001F

//MMDAD register
#define LAN8670_MMDAD_ADR_DATA                  0xFFFF

//STRAP_CTRL0 register
#define LAN8670_STRAP_CTRL0_MITYP               0x0180
#define LAN8670_STRAP_CTRL0_MITYP_RMII          0x0080
#define LAN8670_STRAP_CTRL0_MITYP_MII           0x0100
#define LAN8670_STRAP_CTRL0_PKGTYP              0x0060
#define LAN8670_STRAP_CTRL0_PKGTYP_32_PIN       0x0020
#define LAN8670_STRAP_CTRL0_PKGTYP_24_PIN       0x0040
#define LAN8670_STRAP_CTRL0_PKGTYP_36_PIN       0x0060
#define LAN8670_STRAP_CTRL0_SMIADR              0x001F

//PMA_PMD_EXT_ABILITY register
#define LAN8670_PMA_PMD_EXT_ABILITY_T1SABL      0x0008
#define LAN8670_PMA_PMD_EXT_ABILITY_T1LABL      0x0004

//T1PMAPMDCTL register
#define LAN8670_T1PMAPMDCTL_TYPSEL              0x000F
#define LAN8670_T1PMAPMDCTL_TYPSEL_100BASE_T1   0x0000
#define LAN8670_T1PMAPMDCTL_TYPSEL_1000BASE_T1  0x0001
#define LAN8670_T1PMAPMDCTL_TYPSEL_10BASE_T1L   0x0002
#define LAN8670_T1PMAPMDCTL_TYPSEL_10BASE_T1S   0x0003

//T1SPMACTL register
#define LAN8670_T1SPMACTL_RST                   0x8000
#define LAN8670_T1SPMACTL_TXD                   0x4000
#define LAN8670_T1SPMACTL_LPE                   0x0800
#define LAN8670_T1SPMACTL_MDE                   0x0400
#define LAN8670_T1SPMACTL_LBE                   0x0001

//T1SPMASTS register
#define LAN8670_T1SPMASTS_LBA                   0x2000
#define LAN8670_T1SPMASTS_LPA                   0x0800
#define LAN8670_T1SPMASTS_MDA                   0x0400
#define LAN8670_T1SPMASTS_RXFA                  0x0200
#define LAN8670_T1SPMASTS_RXFD                  0x0002

//T1STSTCTL register
#define LAN8670_T1STSTCTL_TSTCTL                0xE000
#define LAN8670_T1STSTCTL_TSTCTL_NORMAL         0x0000
#define LAN8670_T1STSTCTL_TSTCTL_TEST_MODE_1    0x2000
#define LAN8670_T1STSTCTL_TSTCTL_TEST_MODE_2    0x4000
#define LAN8670_T1STSTCTL_TSTCTL_TEST_MODE_3    0x6000
#define LAN8670_T1STSTCTL_TSTCTL_TEST_MODE_4    0x8000

//T1SPCSCTL register
#define LAN8670_T1SPCSCTL_RST                   0x8000
#define LAN8670_T1SPCSCTL_LBE                   0x4000
#define LAN8670_T1SPCSCTL_DUPLEX                0x0100

//T1SPCSSTS register
#define LAN8670_T1SPCSSTS_FAULT                 0x0080

//T1SPCSDIAG1 register
#define LAN8670_T1SPCSDIAG1_RMTJABCNT           0xFFFF

//T1SPCSDIAG2 register
#define LAN8670_T1SPCSDIAG2_CORTXCNT            0xFFFF

//CTRL1 register
#define LAN8670_CTRL1_DIGLBE                    0x0002

//STS1 register
#define LAN8670_STS1_TXCOL                      0x0400
#define LAN8670_STS1_TXJAB                      0x0200
#define LAN8670_STS1_EMPCYC                     0x0080
#define LAN8670_STS1_RXINTO                     0x0040
#define LAN8670_STS1_UNEXPB                     0x0020
#define LAN8670_STS1_BCNBFTO                    0x0010
#define LAN8670_STS1_PLCASYM                    0x0004
#define LAN8670_STS1_ESDERR                     0x0002
#define LAN8670_STS1_DEC5B                      0x0001

//STS2 register
#define LAN8670_STS2_RESETC                     0x0800

//STS3 register
#define LAN8670_STS3_ERRTOID                    0x0080

//IMSK1 register
#define LAN8670_IMSK1_TXCOLM                    0x0400
#define LAN8670_IMSK1_TXJABM                    0x0200
#define LAN8670_IMSK1_EMPCYCM                   0x0080
#define LAN8670_IMSK1_RXINTOM                   0x0040
#define LAN8670_IMSK1_UNEXPBM                   0x0020
#define LAN8670_IMSK1_BCNBFTOM                  0x0010
#define LAN8670_IMSK1_PLCASYMM                  0x0004
#define LAN8670_IMSK1_ESDERRM                   0x0002
#define LAN8670_IMSK1_DEC5BM                    0x0001

//IMSK2 register
#define LAN8670_IMSK2_RESETCM                   0x0800

//CTRCTRL register
#define LAN8670_CTRCTRL_TOCTRE                  0x0002
#define LAN8670_CTRCTRL_BCNCTRE                 0x0001

//TOCNTH register
#define LAN8670_TOCNTH_TOCNT_31_16              0xFFFF

//TOCNTL register
#define LAN8670_TOCNTL_TOCNT_15_0               0xFFFF

//BCNCNTH register
#define LAN8670_BCNCNTH_BCNCNT_31_16            0xFFFF

//BCNCNTL register
#define LAN8670_BCNCNTL_BCNCNT_15_0             0xFFFF

//PADCTRL3 register
#define LAN8670_PADCTRL3_PDRV4                  0xC000
#define LAN8670_PADCTRL3_PDRV4_LOW              0x0000
#define LAN8670_PADCTRL3_PDRV4_MEDIUM_LOW       0x4000
#define LAN8670_PADCTRL3_PDRV4_MEDIUM_HIGH      0x8000
#define LAN8670_PADCTRL3_PDRV4_HIGH             0xC000
#define LAN8670_PADCTRL3_PDRV3                  0x3000
#define LAN8670_PADCTRL3_PDRV3_LOW              0x0000
#define LAN8670_PADCTRL3_PDRV3_MEDIUM_LOW       0x1000
#define LAN8670_PADCTRL3_PDRV3_MEDIUM_HIGH      0x2000
#define LAN8670_PADCTRL3_PDRV3_HIGH             0x3000
#define LAN8670_PADCTRL3_PDRV2                  0x0C00
#define LAN8670_PADCTRL3_PDRV2_LOW              0x0000
#define LAN8670_PADCTRL3_PDRV2_MEDIUM_LOW       0x0400
#define LAN8670_PADCTRL3_PDRV2_MEDIUM_HIGH      0x0800
#define LAN8670_PADCTRL3_PDRV2_HIGH             0x0C00
#define LAN8670_PADCTRL3_PDRV1                  0x0300
#define LAN8670_PADCTRL3_PDRV1_LOW              0x0000
#define LAN8670_PADCTRL3_PDRV1_MEDIUM_LOW       0x0100
#define LAN8670_PADCTRL3_PDRV1_MEDIUM_HIGH      0x0200
#define LAN8670_PADCTRL3_PDRV1_HIGH             0x0300

//MIDVER register
#define LAN8670_MIDVER_IDM                      0xFF00
#define LAN8670_MIDVER_IDM_DEFAULT              0x0A00
#define LAN8670_MIDVER_VER                      0x00FF
#define LAN8670_MIDVER_VER_DEFAULT              0x0010

//PLCA_CTRL0 register
#define LAN8670_PLCA_CTRL0_EN                   0x8000
#define LAN8670_PLCA_CTRL0_RST                  0x4000

//PLCA_CTRL1 register
#define LAN8670_PLCA_CTRL1_NCNT                 0xFF00
#define LAN8670_PLCA_CTRL1_NCNT_DEFAULT         0x0800
#define LAN8670_PLCA_CTRL1_ID                   0x00FF
#define LAN8670_PLCA_CTRL1_ID_PLCA_COORDINATOR  0x0000
#define LAN8670_PLCA_CTRL1_ID_PLCA_DISABLED     0x00FF

//PLCA_STS register
#define LAN8670_PLCA_STS_PST                    0x8000

//PLCA_TOTMR register
#define LAN8670_PLCA_TOTMR_TOTMR                0x00FF
#define LAN8670_PLCA_TOTMR_TOTMR_DEFAULT        0x0020

//PLCA_BURST register
#define LAN8670_PLCA_BURST_MAXBC                0xFF00
#define LAN8670_PLCA_BURST_MAXBC_DISABLED       0x0000
#define LAN8670_PLCA_BURST_BTMR                 0x00FF
#define LAN8670_PLCA_BURST_BTMR_DEFAULT         0x0080

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//LAN8670 Ethernet PHY driver
extern const PhyDriver lan8670PhyDriver;

//LAN8670 related functions
error_t lan8670Init(NetInterface *interface);
void lan8670InitHook(NetInterface *interface);

void lan8670Tick(NetInterface *interface);

void lan8670EnableIrq(NetInterface *interface);
void lan8670DisableIrq(NetInterface *interface);

void lan8670EventHandler(NetInterface *interface);

void lan8670WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t lan8670ReadPhyReg(NetInterface *interface, uint8_t address);

void lan8670DumpPhyReg(NetInterface *interface);

void lan8670WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t lan8670ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

void lan8670ModifyMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t mask, uint16_t data);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
