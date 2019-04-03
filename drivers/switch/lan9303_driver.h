/**
 * @file lan9303_driver.h
 * @brief LAN9303 Ethernet switch
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
 * @version 1.9.2
 **/

#ifndef _LAN9303_DRIVER_H
#define _LAN9303_DRIVER_H

//Dependencies
#include "core/nic.h"

//LAN9303 ports
#define LAN9303_PORT1                        1
#define LAN9303_PORT2                        2

//LAN9303 PHY registers
#define LAN9303_PHY_REG_BMCR                 0x00
#define LAN9303_PHY_REG_BMSR                 0x01
#define LAN9303_PHY_REG_PHYIDR1              0x02
#define LAN9303_PHY_REG_PHYIDR2              0x03
#define LAN9303_PHY_REG_ANAR                 0x04
#define LAN9303_PHY_REG_ANLPAR               0x05
#define LAN9303_PHY_REG_ANER                 0x06
#define LAN9303_PHY_REG_MCSR                 0x11
#define LAN9303_PHY_REG_SMR                  0x12
#define LAN9303_PHY_REG_SCSIR                0x1B
#define LAN9303_PHY_REG_PISR                 0x1D
#define LAN9303_PHY_REG_PIMR                 0x1E
#define LAN9303_PHY_REG_PSCSR                0x1F

//BMCR register
#define BMCR_RESET                           (1 << 15)
#define BMCR_LOOPBACK                        (1 << 14)
#define BMCR_SPEED_SEL                       (1 << 13)
#define BMCR_AN_EN                           (1 << 12)
#define BMCR_POWER_DOWN                      (1 << 11)
#define BMCR_ISOLATE                         (1 << 10)
#define BMCR_RESTART_AN                      (1 << 9)
#define BMCR_DUPLEX_MODE                     (1 << 8)
#define BMCR_COL_TEST                        (1 << 7)

//BMSR register
#define BMSR_100BT4                          (1 << 15)
#define BMSR_100BTX_FD                       (1 << 14)
#define BMSR_100BTX                          (1 << 13)
#define BMSR_10BT_FD                         (1 << 12)
#define BMSR_10BT                            (1 << 11)
#define BMSR_100BT2_FD                       (1 << 10)
#define BMSR_100BT2                          (1 << 9)
#define BMSR_EXTENTED_STATUS                 (1 << 8)
#define BMSR_AN_COMPLETE                     (1 << 5)
#define BMSR_REMOTE_FAULT                    (1 << 4)
#define BMSR_AN_ABLE                         (1 << 3)
#define BMSR_LINK_STATUS                     (1 << 2)
#define BMSR_JABBER_DETECT                   (1 << 1)
#define BMSR_EXTENDED_CAP                    (1 << 0)

//ANAR register
#define ANAR_RF                              (1 << 13)
#define ANAR_PAUSE1                          (1 << 11)
#define ANAR_PAUSE0                          (1 << 10)
#define ANAR_100BTX_FD                       (1 << 8)
#define ANAR_100BTX                          (1 << 7)
#define ANAR_10BT_FD                         (1 << 6)
#define ANAR_10BT                            (1 << 5)
#define ANAR_SELECTOR4                       (1 << 4)
#define ANAR_SELECTOR3                       (1 << 3)
#define ANAR_SELECTOR2                       (1 << 2)
#define ANAR_SELECTOR1                       (1 << 1)
#define ANAR_SELECTOR0                       (1 << 0)

//ANLPAR register
#define ANLPAR_NP                            (1 << 15)
#define ANLPAR_ACK                           (1 << 14)
#define ANLPAR_RF                            (1 << 13)
#define ANLPAR_PAUSE1                        (1 << 11)
#define ANLPAR_PAUSE0                        (1 << 10)
#define ANLPAR_100BT4                        (1 << 9)
#define ANLPAR_100BTX_FD                     (1 << 8)
#define ANLPAR_100BTX                        (1 << 7)
#define ANLPAR_10BT_FD                       (1 << 6)
#define ANLPAR_10BT                          (1 << 5)
#define ANLPAR_SELECTOR4                     (1 << 4)
#define ANLPAR_SELECTOR3                     (1 << 3)
#define ANLPAR_SELECTOR2                     (1 << 2)
#define ANLPAR_SELECTOR1                     (1 << 1)
#define ANLPAR_SELECTOR0                     (1 << 0)

//ANER register
#define ANER_PDF                             (1 << 4)
#define ANER_LP_NP_ABLE                      (1 << 3)
#define ANER_NP_ABLE                         (1 << 2)
#define ANER_PAGE_RX                         (1 << 1)
#define ANER_LP_AN_ABLE                      (1 << 0)

//MCSR register
#define MCSR_EDPWRDOWN                       (1 << 13)
#define MCSR_ENERGYON                        (1 << 1)

//SMR register
#define SMR_MODE2                            (1 << 7)
#define SMR_MODE1                            (1 << 6)
#define SMR_MODE0                            (1 << 5)
#define SMR_PHYAD4                           (1 << 4)
#define SMR_PHYAD3                           (1 << 3)
#define SMR_PHYAD2                           (1 << 2)
#define SMR_PHYAD1                           (1 << 1)
#define SMR_PHYAD0                           (1 << 0)

//SCSIR register
#define SCSIR_AMDIXCTRL                      (1 << 15)
#define SCSIR_AMDIXEN                        (1 << 14)
#define SCSIR_AMDIXSTATE                     (1 << 13)
#define SCSIR_SQEOFF                         (1 << 11)
#define SCSIR_VCOOFF_LP                      (1 << 10)
#define SCSIR_XPOL                           (1 << 4)

//ISR register
#define ISR_ENERGYON                         (1 << 7)
#define ISR_AN_COMPLETE                      (1 << 6)
#define ISR_REMOTE_FAULT                     (1 << 5)
#define ISR_LINK_DOWN                        (1 << 4)
#define ISR_AN_LP_ACK                        (1 << 3)
#define ISR_PD_FAULT                         (1 << 2)
#define ISR_AN_PAGE_RECEIVED                 (1 << 1)

//IMR register
#define IMR_ENERGYON                         (1 << 7)
#define IMR_AN_COMPLETE                      (1 << 6)
#define IMR_REMOTE_FAULT                     (1 << 5)
#define IMR_LINK_DOWN                        (1 << 4)
#define IMR_AN_LP_ACK                        (1 << 3)
#define IMR_PD_FAULT                         (1 << 2)
#define IMR_AN_PAGE_RECEIVED                 (1 << 1)

//PSCSR register
#define PSCSR_AUTODONE                       (1 << 12)
#define PSCSR_HCDSPEED2                      (1 << 4)
#define PSCSR_HCDSPEED1                      (1 << 3)
#define PSCSR_HCDSPEED0                      (1 << 2)

//Speed indication
#define PSCSR_HCDSPEED_MASK                  (7 << 2)
#define PSCSR_HCDSPEED_10BT                  (1 << 2)
#define PSCSR_HCDSPEED_100BTX                (2 << 2)
#define PSCSR_HCDSPEED_10BT_FD               (5 << 2)
#define PSCSR_HCDSPEED_100BTX_FD             (6 << 2)

//LAN9303 system registers
#define LAN9303_SYS_REG_SWITCH_CSR_DATA      0x01AC
#define LAN9303_SYS_REG_SWITCH_CSR_CMD       0x01B0

//SWITCH_CSR_CMD register
#define SWITCH_CSR_CMD_BUSY                  (1U << 31)
#define SWITCH_CSR_CMD_READ                  (1 << 30)
#define SWITCH_CSR_CMD_AUTO_INC              (1 << 29)
#define SWITCH_CSR_CMD_AUTO_DEC              (1 << 28)
#define SWITCH_CSR_CMD_BE                    (15 << 16)
#define SWITCH_CSR_CMD_BE3                   (1 << 19)
#define SWITCH_CSR_CMD_BE2                   (1 << 18)
#define SWITCH_CSR_CMD_BE1                   (1 << 17)
#define SWITCH_CSR_CMD_BE0                   (1 << 16)
#define SWITCH_CSR_CMD_ADDR                  0xFFFF

//LAN9303 switch fabric registers
#define LAN9303_SW_REG_MAC_RX_CFG(n)         (0x0401 + ((n) * 0x400))
#define LAN9303_SW_REG_MAC_TX_CFG(n)         (0x0440 + ((n) * 0x400))
#define LAN9303_SW_REG_SWE_PORT_STATE        0x1843
#define LAN9303_SW_REG_SWE_PORT_MIRROR       0x1846
#define LAN9303_SW_REG_SWE_INGRSS_PORT_TYP   0x1847
#define LAN9303_SW_REG_BM_EGRSS_PORT_TYPE    0x1C0C

//MAC_RX_CFG register
#define MAC_RX_CFG_EN_RECEIVE_OWN_TRANSMIT   (1 << 5)
#define MAC_RX_CFG_JUMBO_2K                  (1 << 3)
#define MAC_RX_CFG_REJECT_MAC_TYPES          (1 << 1)
#define MAC_RX_CFG_RX_EN                     (1 << 0)

//MAC_TX_CFG register
#define MAC_TX_CFG_MAC_COUNTER_TEST          (1 << 7)
#define MAC_TX_CFG_IFG_CONFIG                (31 << 2)
#define MAC_TX_CFG_IFG_CONFIG_DEFAULT        (21 << 2)
#define MAC_TX_CFG_TX_PAD_EN                 (1 << 1)
#define MAC_TX_CFG_TX_EN                     (1 << 0)

//SWE_PORT_STATE register
#define SWE_PORT_STATE_PORT2_FORWARDING      (0 << 4)
#define SWE_PORT_STATE_PORT2_LISTENING       (1 << 4)
#define SWE_PORT_STATE_PORT2_LEARNING        (2 << 4)
#define SWE_PORT_STATE_PORT2_DISABLED        (4 << 4)
#define SWE_PORT_STATE_PORT1_FORWARDING      (0 << 2)
#define SWE_PORT_STATE_PORT1_LISTENING       (1 << 2)
#define SWE_PORT_STATE_PORT1_LEARNING        (2 << 2)
#define SWE_PORT_STATE_PORT1_DISABLED        (4 << 2)
#define SWE_PORT_STATE_PORT0_FORWARDING      (0 << 0)
#define SWE_PORT_STATE_PORT0_LISTENING       (1 << 0)
#define SWE_PORT_STATE_PORT0_LEARNING        (2 << 0)
#define SWE_PORT_STATE_PORT0_DISABLED        (4 << 0)

//SWE_PORT_MIRROR register
#define SWE_PORT_MIRROR_EN_RX_MIRRORING_FILT (1 << 8)
#define SWE_PORT_MIRROR_SNIFFER_PORT2        (1 << 7)
#define SWE_PORT_MIRROR_SNIFFER_PORT1        (1 << 6)
#define SWE_PORT_MIRROR_SNIFFER_PORT0        (1 << 5)
#define SWE_PORT_MIRROR_MIRRORED_PORT2       (1 << 4)
#define SWE_PORT_MIRROR_MIRRORED_PORT1       (1 << 3)
#define SWE_PORT_MIRROR_MIRRORED_PORT0       (1 << 2)
#define SWE_PORT_MIRROR_EN_RX_MIRRORING      (1 << 1)
#define SWE_PORT_MIRROR_EN_TX_MIRRORING      (1 << 0)

//SWE_INGRSS_PORT_TYP register
#define SWE_INGRSS_PORT_TYP_PORT2            (3 << 4)
#define SWE_INGRSS_PORT_TYP_PORT1            (3 << 2)
#define SWE_INGRSS_PORT_TYP_PORT0            (3 << 0)

//BM_EGRSS_PORT_TYPE register
#define BM_EGRSS_PORT_TYPE_PORT0_CPU         (3 << 0)

//Tail tag encoding
#define LAN9303_VLAN_ID_ENCODE(port) htons(0x10 | ((port) & 0x03))
//Tail tag decoding
#define LAN9303_VLAN_ID_DECODE(tag) (ntohs(tag) & 0x03)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//LAN9303 Ethernet switch driver
extern const PhyDriver lan9303PhyDriver;

//LAN9303 related functions
error_t lan9303Init(NetInterface *interface);

bool_t lan9303GetLinkState(NetInterface *interface, uint8_t port);

void lan9303Tick(NetInterface *interface);

void lan9303EnableIrq(NetInterface *interface);
void lan9303DisableIrq(NetInterface *interface);

void lan9303EventHandler(NetInterface *interface);

error_t lan9303TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type);

error_t lan9303UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, uint8_t *port);

void lan9303WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data);

uint16_t lan9303ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address);

void lan9303DumpPhyReg(NetInterface *interface, uint8_t port);

void lan9303WriteSysReg(NetInterface *interface, uint16_t address,
   uint32_t data);

uint32_t lan9303ReadSysReg(NetInterface *interface, uint16_t address);

void lan9303DumpSysReg(NetInterface *interface);

void lan9303WriteSwitchReg(NetInterface *interface, uint16_t address,
   uint32_t data);

uint32_t lan9303ReadSwitchReg(NetInterface *interface, uint16_t address);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
