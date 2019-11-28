/**
 * @file ksz8463_driver.h
 * @brief KSZ8463 3-port Ethernet switch
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.6
 **/

#ifndef _KSZ8463_DRIVER_H
#define _KSZ8463_DRIVER_H

//Dependencies
#include "core/nic.h"

//KSZ8463 ports
#define KSZ8463_PORT1 1
#define KSZ8463_PORT2 2

//SPI command byte
#define KSZ8463_SPI_CMD_READ  0x0000
#define KSZ8463_SPI_CMD_WRITE 0x8000
#define KSZ8463_SPI_CMD_ADDR  0x7FC0
#define KSZ8463_SPI_CMD_B3    0x0020
#define KSZ8463_SPI_CMD_B2    0x0010
#define KSZ8463_SPI_CMD_B1    0x0008
#define KSZ8463_SPI_CMD_B0    0x0004

//KSZ8463 PHY registers
#define KSZ8463_BMCR                          0x00
#define KSZ8463_BMSR                          0x01
#define KSZ8463_PHYID1                        0x02
#define KSZ8463_PHYID2                        0x03
#define KSZ8463_ANAR                          0x04
#define KSZ8463_ANLPAR                        0x05
#define KSZ8463_LINKMD                        0x1D
#define KSZ8463_PHYSCS                        0x1F

//KSZ8463 Switch registers
#define KSZ8463_CIDER                         0x0000
#define KSZ8463_SGCR1                         0x0002
#define KSZ8463_SGCR2                         0x0004
#define KSZ8463_SGCR3                         0x0006
#define KSZ8463_SGCR6                         0x000C
#define KSZ8463_SGCR7                         0x000E
#define KSZ8463_MACAR1                        0x0010
#define KSZ8463_MACAR2                        0x0012
#define KSZ8463_MACAR3                        0x0014
#define KSZ8463_TOSR1                         0x0016
#define KSZ8463_TOSR2                         0x0018
#define KSZ8463_TOSR3                         0x001A
#define KSZ8463_TOSR4                         0x001C
#define KSZ8463_TOSR5                         0x001E
#define KSZ8463_TOSR6                         0x0020
#define KSZ8463_TOSR7                         0x0022
#define KSZ8463_TOSR8                         0x0024
#define KSZ8463_IADR1                         0x0026
#define KSZ8463_IADR2                         0x0028
#define KSZ8463_IADR3                         0x002A
#define KSZ8463_IADR4                         0x002C
#define KSZ8463_IADR5                         0x002E
#define KSZ8463_IACR                          0x0030
#define KSZ8463_PMCTRL                        0x0032
#define KSZ8463_GST                           0x0036
#define KSZ8463_CTPDC                         0x0038
#define KSZ8463_P1MBCR                        0x004C
#define KSZ8463_P1MBSR                        0x004E
#define KSZ8463_PHY1ILR                       0x0050
#define KSZ8463_PHY1IHR                       0x0052
#define KSZ8463_P1ANAR                        0x0054
#define KSZ8463_P1ANLPR                       0x0056
#define KSZ8463_P2MBCR                        0x0058
#define KSZ8463_P2MBSR                        0x005A
#define KSZ8463_PHY2ILR                       0x005C
#define KSZ8463_PHY2IHR                       0x005E
#define KSZ8463_P2ANAR                        0x0060
#define KSZ8463_P2ANLPR                       0x0062
#define KSZ8463_P1PHYCTRL                     0x0066
#define KSZ8463_P2PHYCTRL                     0x006A
#define KSZ8463_P1CR1                         0x006C
#define KSZ8463_P1CR2                         0x006E
#define KSZ8463_P1VIDCR                       0x0070
#define KSZ8463_P1CR3                         0x0072
#define KSZ8463_P1IRCR0                       0x0074
#define KSZ8463_P1IRCR1                       0x0076
#define KSZ8463_P1ERCR0                       0x0078
#define KSZ8463_P1ERCR1                       0x007A
#define KSZ8463_P1SCSLMD                      0x007C
#define KSZ8463_P1CR4                         0x007E
#define KSZ8463_P1SR                          0x0080
#define KSZ8463_P2CR1                         0x0084
#define KSZ8463_P2CR2                         0x0086
#define KSZ8463_P2VIDCR                       0x0088
#define KSZ8463_P2CR3                         0x008A
#define KSZ8463_P2IRCR0                       0x008C
#define KSZ8463_P2IRCR1                       0x008E
#define KSZ8463_P2ERCR0                       0x0090
#define KSZ8463_P2ERCR1                       0x0092
#define KSZ8463_P2SCSLMD                      0x0094
#define KSZ8463_P2CR4                         0x0096
#define KSZ8463_P2SR                          0x0098
#define KSZ8463_P3CR1                         0x009C
#define KSZ8463_P3CR2                         0x009E
#define KSZ8463_P3VIDCR                       0x00A0
#define KSZ8463_P3CR3                         0x00A2
#define KSZ8463_P3IRCR0                       0x00A4
#define KSZ8463_P3IRCR1                       0x00A6
#define KSZ8463_P3ERCR0                       0x00A8
#define KSZ8463_P3ERCR1                       0x00AA
#define KSZ8463_SGCR8                         0x00AC
#define KSZ8463_SGCR9                         0x00AE
#define KSZ8463_SAFMACA1L                     0x00B0
#define KSZ8463_SAFMACA1M                     0x00B2
#define KSZ8463_SAFMACA1H                     0x00B4
#define KSZ8463_SAFMACA2L                     0x00B6
#define KSZ8463_SAFMACA2M                     0x00B8
#define KSZ8463_SAFMACA2H                     0x00BA
#define KSZ8463_P1TXQRCR1                     0x00C8
#define KSZ8463_P1TXQRCR2                     0x00CA
#define KSZ8463_P2TXQRCR1                     0x00CC
#define KSZ8463_P2TXQRCR2                     0x00CE
#define KSZ8463_P3TXQRCR1                     0x00D0
#define KSZ8463_P3TXQRCR2                     0x00D2
#define KSZ8463_IOMXSEL                       0x00D6
#define KSZ8463_CFGR                          0x00D8
#define KSZ8463_P1ANPT                        0x00DC
#define KSZ8463_P1ALPRNP                      0x00DE
#define KSZ8463_P1EEEA                        0x00E0
#define KSZ8463_P1EEEWEC                      0x00E2
#define KSZ8463_P1EEECS                       0x00E4
#define KSZ8463_P1LPIRTC                      0x00E6
#define KSZ8463_BL2LPIC1                      0x00E7
#define KSZ8463_P2ANPT                        0x00E8
#define KSZ8463_P2ALPRNP                      0x00EA
#define KSZ8463_P2EEEA                        0x00EC
#define KSZ8463_P2EEEWEC                      0x00EE
#define KSZ8463_P2EEECS                       0x00F0
#define KSZ8463_P2LPIRTC                      0x00F2
#define KSZ8463_PCSEEEC                       0x00F3
#define KSZ8463_ETLWTC                        0x00F4
#define KSZ8463_BL2LPIC2                      0x00F6
#define KSZ8463_MBIR                          0x0124
#define KSZ8463_GRR                           0x0126
#define KSZ8463_IER                           0x0190
#define KSZ8463_ISR                           0x0192

//KSZ8463 Switch register access macros
#define KSZ8463_PnMBCR(port)                  (0x0040 + ((port) * 0x000C))
#define KSZ8463_PnMBSR(port)                  (0x0042 + ((port) * 0x000C))
#define KSZ8463_PHYnILR(port)                 (0x0044 + ((port) * 0x000C))
#define KSZ8463_PHYnIHR(port)                 (0x0046 + ((port) * 0x000C))
#define KSZ8463_PnANAR(port)                  (0x0048 + ((port) * 0x000C))
#define KSZ8463_PnANLPR(port)                 (0x004A + ((port) * 0x000C))
#define KSZ8463_PnPHYCTRL(port)               (0x0062 + ((port) * 0x0004))
#define KSZ8463_PnCR1(port)                   (0x0054 + ((port) * 0x0018))
#define KSZ8463_PnCR2(port)                   (0x0056 + ((port) * 0x0018))
#define KSZ8463_PnVIDCR(port)                 (0x0058 + ((port) * 0x0018))
#define KSZ8463_PnCR3(port)                   (0x005A + ((port) * 0x0018))
#define KSZ8463_PnIRCR0(port)                 (0x005C + ((port) * 0x0018))
#define KSZ8463_PnIRCR1(port)                 (0x005E + ((port) * 0x0018))
#define KSZ8463_PnERCR0(port)                 (0x0060 + ((port) * 0x0018))
#define KSZ8463_PnERCR1(port)                 (0x0062 + ((port) * 0x0018))
#define KSZ8463_PnSCSLMD(port)                (0x0064 + ((port) * 0x0018))
#define KSZ8463_PnCR4(port)                   (0x0066 + ((port) * 0x0018))
#define KSZ8463_PnSR(port)                    (0x0068 + ((port) * 0x0018))
#define KSZ8463_PnTXQRCR1(port)               (0x00C4 + ((port) * 0x0004))
#define KSZ8463_PnTXQRCR2(port)               (0x00C6 + ((port) * 0x0004))
#define KSZ8463_PnANPT(port)                  (0x00D0 + ((port) * 0x000C))
#define KSZ8463_PnALPRNP(port)                (0x00D2 + ((port) * 0x000C))
#define KSZ8463_PnEEEA(port)                  (0x00D4 + ((port) * 0x000C))
#define KSZ8463_PnEEEWEC(port)                (0x00D6 + ((port) * 0x000C))
#define KSZ8463_PnEEECS(port)                 (0x00D8 + ((port) * 0x000C))
#define KSZ8463_PnLPIRTC(port)                (0x00DA + ((port) * 0x000C))

//Basic Control register
#define KSZ8463_BMCR_LOOPBACK                 0x4000
#define KSZ8463_BMCR_FORCE_100                0x2000
#define KSZ8463_BMCR_AN_EN                    0x1000
#define KSZ8463_BMCR_POWER_DOWN               0x0800
#define KSZ8463_BMCR_ISOLATE                  0x0400
#define KSZ8463_BMCR_RESTART_AN               0x0200
#define KSZ8463_BMCR_FORCE_FULL_DUPLEX        0x0100
#define KSZ8463_BMCR_COL_TEST                 0x0080
#define KSZ8463_BMCR_HP_MDIX                  0x0020
#define KSZ8463_BMCR_FORCE_MDI                0x0010
#define KSZ8463_BMCR_AUTO_MDIX_DIS            0x0008
#define KSZ8463_BMCR_FAR_END_FAULT_DIS        0x0004
#define KSZ8463_BMCR_TRANSMIT_DIS             0x0002
#define KSZ8463_BMCR_LED_DIS                  0x0001

//Basic Status register
#define KSZ8463_BMSR_100BT4                   0x8000
#define KSZ8463_BMSR_100BTX_FD                0x4000
#define KSZ8463_BMSR_100BTX_HD                0x2000
#define KSZ8463_BMSR_10BT_FD                  0x1000
#define KSZ8463_BMSR_10BT_HD                  0x0800
#define KSZ8463_BMSR_PREAMBLE_SUPPR           0x0040
#define KSZ8463_BMSR_AN_COMPLETE              0x0020
#define KSZ8463_BMSR_FAR_END_FAULT            0x0010
#define KSZ8463_BMSR_AN_CAPABLE               0x0008
#define KSZ8463_BMSR_LINK_STATUS              0x0004
#define KSZ8463_BMSR_JABBER_TEST              0x0002
#define KSZ8463_BMSR_EXTENDED_CAPABLE         0x0001

//PHYID High register
#define KSZ8463_PHYID1_DEFAULT                0x0022

//PHYID Low register
#define KSZ8463_PHYID2_DEFAULT                0x1430

//Auto-Negotiation Advertisement Ability register
#define KSZ8463_ANAR_NEXT_PAGE                0x8000
#define KSZ8463_ANAR_REMOTE_FAULT             0x2000
#define KSZ8463_ANAR_PAUSE                    0x0400
#define KSZ8463_ANAR_100BTX_FD                0x0100
#define KSZ8463_ANAR_100BTX_HD                0x0080
#define KSZ8463_ANAR_10BT_FD                  0x0040
#define KSZ8463_ANAR_10BT_HD                  0x0020
#define KSZ8463_ANAR_SELECTOR                 0x001F
#define KSZ8463_ANAR_SELECTOR_DEFAULT         0x0001

//Auto-Negotiation Link Partner Ability register
#define KSZ8463_ANLPAR_NEXT_PAGE              0x8000
#define KSZ8463_ANLPAR_LP_ACK                 0x4000
#define KSZ8463_ANLPAR_REMOTE_FAULT           0x2000
#define KSZ8463_ANLPAR_PAUSE                  0x0400
#define KSZ8463_ANLPAR_100BTX_FD              0x0100
#define KSZ8463_ANLPAR_100BTX_HD              0x0080
#define KSZ8463_ANLPAR_10BT_FD                0x0040
#define KSZ8463_ANLPAR_10BT_HD                0x0020

//LinkMD Control/Status register
#define KSZ8463_LINKMD_TEST_EN                0x8000
#define KSZ8463_LINKMD_RESULT                 0x6000
#define KSZ8463_LINKMD_SHORT                  0x1000
#define KSZ8463_LINKMD_FAULT_COUNT            0x01FF

//PHY Special Control/Status register
#define KSZ8463_PHYSCS_POL_REVERSE            0x0020
#define KSZ8463_PHYSCS_MDIX_STATUS            0x0010
#define KSZ8463_PHYSCS_FORCE_LINK             0x0008
#define KSZ8463_PHYSCS_EEE_EN                 0x0004
#define KSZ8463_PHYSCS_REMOTE_LOOPBACK        0x0002

//Chip ID And Enable register
#define KSZ8463_CIDER_FAMILY_ID               0xFF00
#define KSZ8463_CIDER_FAMILY_ID_DEFAULT       0x8400
#define KSZ8463_CIDER_CHIP_ID                 0x00F0
#define KSZ8463_CIDER_CHIP_ID_ML_FML          0x0040
#define KSZ8463_CIDER_CHIP_ID_RL_FRL          0x0050
#define KSZ8463_CIDER_REVISION_ID             0x000E
#define KSZ8463_CIDER_START_SWITCH            0x0001

//Port N Control 2 register
#define KSZ8463_PnCR2_INGRESS_VLAN_FILT       0x4000
#define KSZ8463_PnCR2_DISCARD_NON_PVID_PKT    0x2000
#define KSZ8463_PnCR2_FORCE_FLOW_CTRL         0x1000
#define KSZ8463_PnCR2_BACK_PRESSURE_EN        0x0800
#define KSZ8463_PnCR2_TRANSMIT_EN             0x0400
#define KSZ8463_PnCR2_RECEIVE_EN              0x0200
#define KSZ8463_PnCR2_LEARNING_DIS            0x0100
#define KSZ8463_PnCR2_SNIFFER_PORT            0x0080
#define KSZ8463_PnCR2_RECEIVE_SNIFF           0x0040
#define KSZ8463_PnCR2_TRANSMIT_SNIFF          0x0020
#define KSZ8463_PnCR2_USER_PRIO_CEILING       0x0008
#define KSZ8463_PnCR2_PORT_VLAN_MEMBERSHIP    0x0007

//Port N VID Control register
#define KSZ8463_PnVIDCR_PRIORITY              0xE000
#define KSZ8463_PnVIDCR_CFI                   0x1000
#define KSZ8463_PnVIDCR_VID                   0x0FFF

//Port N Status register
#define KSZ8463_PnSR_HP_MDIX                  0x8000
#define KSZ8463_PnSR_POL_REVERSE              0x2000
#define KSZ8463_PnSR_TX_FLOW_CTRL_EN          0x1000
#define KSZ8463_PnSR_RX_FLOW_CTRL_EN          0x0800
#define KSZ8463_PnSR_OP_SPEED                 0x0400
#define KSZ8463_PnSR_OP_DUPLEX                0x0200
#define KSZ8463_PnSR_FAR_END_FAULT            0x0100
#define KSZ8463_PnSR_MDIX_STATUS              0x0080
#define KSZ8463_PnSR_AN_DONE                  0x0040
#define KSZ8463_PnSR_LINK_STATUS              0x0020
#define KSZ8463_PnSR_LP_FLOW_CTRL_CAPABLE     0x0010
#define KSZ8463_PnSR_LP_100BTX_FD_CAPABLE     0x0008
#define KSZ8463_PnSR_LP_100BTX_HF_CAPABLE     0x0004
#define KSZ8463_PnSR_LP_10BT_FD_CAPABLE       0x0002
#define KSZ8463_PnSR_LP_10BT_HD_CAPABLE       0x0001

//Switch Global Control 8 register
#define KSZ8463_SGCR8_QUEUE_PRIO_MAPPING      0xC000
#define KSZ8463_SGCR8_FLUSH_DYNAMIC_MAC_TABLE 0x0400
#define KSZ8463_SGCR8_FLUSH_STATIC_MAC_TABLE  0x0200
#define KSZ8463_SGCR8_TAIL_TAG_EN             0x0100
#define KSZ8463_SGCR8_PAUSE_OFF_LIMIT_TIME    0x00FF

//Tail tag encoding
#define KSZ8463_TAIL_TAG_ENCODE(port) ((port) & 0x03)
//Tail tag decoding
#define KSZ8463_TAIL_TAG_DECODE(tag) (((tag) & 0x01) + 1)

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//KSZ8463 Ethernet switch driver
extern const PhyDriver ksz8463PhyDriver;

//KSZ8463 related functions
error_t ksz8463Init(NetInterface *interface);

bool_t ksz8463GetLinkState(NetInterface *interface, uint8_t port);

void ksz8463Tick(NetInterface *interface);

void ksz8463EnableIrq(NetInterface *interface);
void ksz8463DisableIrq(NetInterface *interface);

void ksz8463EventHandler(NetInterface *interface);

error_t ksz8463TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type);

error_t ksz8463UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, uint8_t *port);

void ksz8463WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data);

uint16_t ksz8463ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address);

void ksz8463DumpPhyReg(NetInterface *interface, uint8_t port);

void ksz8463WriteSwitchReg(NetInterface *interface, uint16_t address,
   uint16_t data);

uint16_t ksz8463ReadSwitchReg(NetInterface *interface, uint16_t address);

void ksz8463DumpSwitchReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
