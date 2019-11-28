/**
 * @file lan9303_driver.h
 * @brief LAN9303 3-port Ethernet switch
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

#ifndef _LAN9303_DRIVER_H
#define _LAN9303_DRIVER_H

//Dependencies
#include "core/nic.h"

//LAN9303 ports
#define LAN9303_PORT1 1
#define LAN9303_PORT2 2

//LAN9303 PHY registers
#define LAN9303_BMCR                                 0x00
#define LAN9303_BMSR                                 0x01
#define LAN9303_PHYID1                               0x02
#define LAN9303_PHYID2                               0x03
#define LAN9303_ANAR                                 0x04
#define LAN9303_ANLPAR                               0x05
#define LAN9303_ANER                                 0x06
#define LAN9303_PMCSR                                0x11
#define LAN9303_PSMR                                 0x12
#define LAN9303_PSCSIR                               0x1B
#define LAN9303_PISR                                 0x1D
#define LAN9303_PIMR                                 0x1E
#define LAN9303_PSCSR                                0x1F

//LAN9303 System registers
#define LAN9303_BYTE_TEST                            0x0064
#define LAN9303_HW_CFG                               0x0074
#define LAN9303_SWITCH_CSR_DATA                      0x01AC
#define LAN9303_SWITCH_CSR_CMD                       0x01B0

//LAN9303 Switch Fabric registers
#define LAN9303_SW_DEV_ID                            0x0000
#define LAN9303_SW_RESET                             0x0001
#define LAN9303_SW_IMR                               0x0004
#define LAN9303_SW_IPR                               0x0005
#define LAN9303_MAC_VER_ID0                          0x0400
#define LAN9303_MAC_RX_CFG0                          0x0401
#define LAN9303_MAC_TX_CFG0                          0x0440
#define LAN9303_MAC_VER_ID1                          0x0800
#define LAN9303_MAC_RX_CFG1                          0x0801
#define LAN9303_MAC_TX_CFG1                          0x0840
#define LAN9303_MAC_VER_ID2                          0x0C00
#define LAN9303_MAC_RX_CFG2                          0x0C01
#define LAN9303_MAC_TX_CFG2                          0x0C40
#define LAN9303_SWE_PORT_STATE                       0x1843
#define LAN9303_SWE_PORT_MIRROR                      0x1846
#define LAN9303_SWE_INGRSS_PORT_TYP                  0x1847
#define LAN9303_BM_EGRSS_PORT_TYPE                   0x1C0C

//LAN9303 Switch Fabric register access macros
#define LAN9303_MAC_VER_ID(port)                     (0x0400 + ((port) * 0x0400))
#define LAN9303_MAC_RX_CFG(port)                     (0x0401 + ((port) * 0x0400))
#define LAN9303_MAC_TX_CFG(port)                     (0x0440 + ((port) * 0x0400))

//PHY Basic Control register
#define LAN9303_BMCR_RESET                           0x8000
#define LAN9303_BMCR_LOOPBACK                        0x4000
#define LAN9303_BMCR_SPEED_SEL                       0x2000
#define LAN9303_BMCR_AN_EN                           0x1000
#define LAN9303_BMCR_POWER_DOWN                      0x0800
#define LAN9303_BMCR_RESTART_AN                      0x0200
#define LAN9303_BMCR_DUPLEX_MODE                     0x0100
#define LAN9303_BMCR_COL_TEST                        0x0080

//PHY Basic Status register
#define LAN9303_BMSR_100BT4                          0x8000
#define LAN9303_BMSR_100BTX_FD                       0x4000
#define LAN9303_BMSR_100BTX_HD                       0x2000
#define LAN9303_BMSR_10BT_FD                         0x1000
#define LAN9303_BMSR_10BT_HD                         0x0800
#define LAN9303_BMSR_100BT2_FD                       0x0400
#define LAN9303_BMSR_100BT2_HD                       0x0200
#define LAN9303_BMSR_AN_COMPLETE                     0x0020
#define LAN9303_BMSR_REMOTE_FAULT                    0x0010
#define LAN9303_BMSR_AN_CAPABLE                      0x0008
#define LAN9303_BMSR_LINK_STATUS                     0x0004
#define LAN9303_BMSR_JABBER_DETECT                   0x0002
#define LAN9303_BMSR_EXTENDED_CAPABLE                0x0001

//PHY Identification MSB register
#define LAN9303_PHYID1_PHY_ID_MSB                    0xFFFF
#define LAN9303_PHYID1_PHY_ID_MSB_DEFAULT            0x0007

//PHY Identification LSB register
#define LAN9303_PHYID2_PHY_ID_LSB                    0xFFFF
#define LAN9303_PHYID2_PHY_ID_LSB_DEFAULT            0x0030
#define LAN9303_PHYID2_MODEL_NUM                     0x03F0
#define LAN9303_PHYID2_MODEL_NUM_DEFAULT             0x00D0
#define LAN9303_PHYID2_REVISION_NUM                  0x000F

//PHY Auto-Negotiation Advertisement register
#define LAN9303_ANAR_REMOTE_FAULT                    0x2000
#define LAN9303_ANAR_ASYM_PAUSE                      0x0800
#define LAN9303_ANAR_SYM_PAUSE                       0x0400
#define LAN9303_ANAR_100BTX_FD                       0x0100
#define LAN9303_ANAR_100BTX_HD                       0x0080
#define LAN9303_ANAR_10BT_FD                         0x0040
#define LAN9303_ANAR_10BT_HD                         0x0020
#define LAN9303_ANAR_SELECTOR                        0x001F
#define LAN9303_ANAR_SELECTOR_DEFAULT                0x0001

//PHY Auto-Negotiation Link Partner Base Page Ability register
#define LAN9303_ANLPAR_NEXT_PAGE                     0x8000
#define LAN9303_ANLPAR_ACK                           0x4000
#define LAN9303_ANLPAR_REMOTE_FAULT                  0x2000
#define LAN9303_ANLPAR_ASYM_PAUSE                    0x0800
#define LAN9303_ANLPAR_SYM_PAUSE                     0x0400
#define LAN9303_ANLPAR_100BT4                        0x0200
#define LAN9303_ANLPAR_100BTX_FD                     0x0100
#define LAN9303_ANLPAR_100BTX_HD                     0x0080
#define LAN9303_ANLPAR_10BT_FD                       0x0040
#define LAN9303_ANLPAR_10BT_HD                       0x0020
#define LAN9303_ANLPAR_SELECTOR                      0x001F
#define LAN9303_ANLPAR_SELECTOR_DEFAULT              0x0001

//PHY Auto-Negotiation Expansion register
#define LAN9303_ANER_PAR_DETECT_FAULT                0x0010
#define LAN9303_ANER_LP_NEXT_PAGE_ABLE               0x0008
#define LAN9303_ANER_NEXT_PAGE_ABLE                  0x0004
#define LAN9303_ANER_PAGE_RECEIVED                   0x0002
#define LAN9303_ANER_LP_AN_ABLE                      0x0001

//PHY Mode Control/Status register
#define LAN9303_PMCSR_EDPWRDOWN                      0x2000
#define LAN9303_PMCSR_ENERGYON                       0x0002

//PHY Special Modes register
#define LAN9303_PSMR_MODE                            0x00E0
#define LAN9303_PSMR_MODE_10BT_HD                    0x0000
#define LAN9303_PSMR_MODE_10BT_FD                    0x0020
#define LAN9303_PSMR_MODE_100BTX_HD                  0x0040
#define LAN9303_PSMR_MODE_100BTX_FD                  0x0060
#define LAN9303_PSMR_MODE_POWER_DOWN                 0x00C0
#define LAN9303_PSMR_MODE_AN                         0x00E0
#define LAN9303_PSMR_PHYAD                           0x001F

//PHY Special Control/Status Indication register
#define LAN9303_PSCSIR_AMDIXCTRL                     0x8000
#define LAN9303_PSCSIR_AMDIXEN                       0x4000
#define LAN9303_PSCSIR_AMDIXSTATE                    0x2000
#define LAN9303_PSCSIR_SQEOFF                        0x0800
#define LAN9303_PSCSIR_VCOOFF_LP                     0x0400
#define LAN9303_PSCSIR_XPOL                          0x0010

//PHY Interrupt Source Flags register
#define LAN9303_PISR_ENERGYON                        0x0080
#define LAN9303_PISR_AN_COMPLETE                     0x0040
#define LAN9303_PISR_REMOTE_FAULT                    0x0020
#define LAN9303_PISR_LINK_DOWN                       0x0010
#define LAN9303_PISR_AN_LP_ACK                       0x0008
#define LAN9303_PISR_PAR_DETECT_FAULT                0x0004
#define LAN9303_PISR_AN_PAGE_RECEIVED                0x0002

//PHY Interrupt Mask register
#define LAN9303_PIMR_ENERGYON                        0x0080
#define LAN9303_PIMR_AN_COMPLETE                     0x0040
#define LAN9303_PIMR_REMOTE_FAULT                    0x0020
#define LAN9303_PIMR_LINK_DOWN                       0x0010
#define LAN9303_PIMR_AN_LP_ACK                       0x0008
#define LAN9303_PIMR_PAR_DETECT_FAULT                0x0004
#define LAN9303_PIMR_AN_PAGE_RECEIVED                0x0002

//PHY Special Control/Status register
#define LAN9303_PSCSR_AUTODONE                       0x1000
#define LAN9303_PSCSR_SPEED                          0x001C
#define LAN9303_PSCSR_SPEED_10BT_HD                  0x0004
#define LAN9303_PSCSR_SPEED_100BTX_HD                0x0008
#define LAN9303_PSCSR_SPEED_10BT_FD                  0x0014
#define LAN9303_PSCSR_SPEED_100BTX_FD                0x0018

//Byte Order Test register
#define LAN9303_BYTE_TEST_DEFAULT                    0x87654321

//Hardware Configuration register
#define LAN9303_HW_CFG_DEVICE_READY                  0x08000000
#define LAN9303_HW_CFG_AMDIX_EN_STRAP_STATE_PORT2    0x04000000
#define LAN9303_HW_CFG_AMDIX_EN_STRAP_STATE_PORT1    0x02000000

//Switch Fabric CSR Interface Command register
#define LAN9303_SWITCH_CSR_CMD_BUSY                  0x80000000
#define LAN9303_SWITCH_CSR_CMD_READ                  0x40000000
#define LAN9303_SWITCH_CSR_CMD_AUTO_INC              0x20000000
#define LAN9303_SWITCH_CSR_CMD_AUTO_DEC              0x10000000
#define LAN9303_SWITCH_CSR_CMD_BE                    0x000F0000
#define LAN9303_SWITCH_CSR_CMD_BE_0                  0x00010000
#define LAN9303_SWITCH_CSR_CMD_BE_1                  0x00020000
#define LAN9303_SWITCH_CSR_CMD_BE_2                  0x00040000
#define LAN9303_SWITCH_CSR_CMD_BE_3                  0x00080000
#define LAN9303_SWITCH_CSR_CMD_ADDR                  0x0000FFFF

//Switch Device ID register
#define LAN9303_SW_DEV_ID_DEVICE_TYPE                0x00FF0000
#define LAN9303_SW_DEV_ID_DEVICE_TYPE_DEFAULT        0x00030000
#define LAN9303_SW_DEV_ID_CHIP_VERSION               0x0000FF00
#define LAN9303_SW_DEV_ID_CHIP_VERSION_DEFAULT       0x00000400
#define LAN9303_SW_DEV_ID_REVISION                   0x000000FF
#define LAN9303_SW_DEV_ID_REVISION_DEFAULT           0x00000007

//Switch Reset register
#define LAN9303_SW_RESET_SW_RESET                    0x00000001

//Switch Global Interrupt Mask register
#define LAN9303_SW_IMR_BM                            0x00000040
#define LAN9303_SW_IMR_SWE                           0x00000020
#define LAN9303_SW_IMR_MAC2                          0x00000004
#define LAN9303_SW_IMR_MAC1                          0x00000002
#define LAN9303_SW_IMR_MAC0                          0x00000001

//Switch Global Interrupt Pending register
#define LAN9303_SW_IPR_BM                            0x00000040
#define LAN9303_SW_IPR_SWE                           0x00000020
#define LAN9303_SW_IPR_MAC2                          0x00000004
#define LAN9303_SW_IPR_MAC1                          0x00000002
#define LAN9303_SW_IPR_MAC0                          0x00000001

//Port x MAC Version ID register
#define LAN9303_MAC_VER_ID_DEVICE_TYPE               0x00000F00
#define LAN9303_MAC_VER_ID_DEVICE_TYPE_DEFAULT       0x00000500
#define LAN9303_MAC_VER_ID_CHIP_VERSION              0x000000F0
#define LAN9303_MAC_VER_ID_CHIP_VERSION_DEFAULT      0x00000080
#define LAN9303_MAC_VER_ID_REVISION                  0x0000000F
#define LAN9303_MAC_VER_ID_REVISION_DEFAULT          0x00000003

//Port x MAC Receive Configuration register
#define LAN9303_MAC_RX_CFG_RECEIVE_OWN_TRANSMIT_EN   0x00000020
#define LAN9303_MAC_RX_CFG_JUMBO_2K                  0x00000008
#define LAN9303_MAC_RX_CFG_REJECT_MAC_TYPES          0x00000002
#define LAN9303_MAC_RX_CFG_RX_EN                     0x00000001

//Port x MAC Transmit Configuration register
#define LAN9303_MAC_TX_CFG_MAC_COUNTER_TEST          0x00000080
#define LAN9303_MAC_TX_CFG_IFG_CONFIG                0x0000007C
#define LAN9303_MAC_TX_CFG_IFG_CONFIG_DEFAULT        0x00000054
#define LAN9303_MAC_TX_CFG_TX_PAD_EN                 0x00000002
#define LAN9303_MAC_TX_CFG_TX_EN                     0x00000001

//Switch Engine Port State register
#define LAN9303_SWE_PORT_STATE_PORT2                 0x00000030
#define LAN9303_SWE_PORT_STATE_PORT2_FORWARDING      0x00000000
#define LAN9303_SWE_PORT_STATE_PORT2_LISTENING       0x00000010
#define LAN9303_SWE_PORT_STATE_PORT2_LEARNING        0x00000020
#define LAN9303_SWE_PORT_STATE_PORT2_DISABLED        0x00000030
#define LAN9303_SWE_PORT_STATE_PORT1                 0x0000000C
#define LAN9303_SWE_PORT_STATE_PORT1_FORWARDING      0x00000000
#define LAN9303_SWE_PORT_STATE_PORT1_LISTENING       0x00000004
#define LAN9303_SWE_PORT_STATE_PORT1_LEARNING        0x00000008
#define LAN9303_SWE_PORT_STATE_PORT1_DISABLED        0x0000000C
#define LAN9303_SWE_PORT_STATE_PORT0                 0x00000003
#define LAN9303_SWE_PORT_STATE_PORT0_FORWARDING      0x00000000
#define LAN9303_SWE_PORT_STATE_PORT0_LISTENING       0x00000001
#define LAN9303_SWE_PORT_STATE_PORT0_LEARNING        0x00000002
#define LAN9303_SWE_PORT_STATE_PORT0_DISABLED        0x00000003

//Switch Engine Port Mirroring register
#define LAN9303_SWE_PORT_MIRROR_RX_MIRRORING_FILT_EN 0x00000100
#define LAN9303_SWE_PORT_MIRROR_SNIFFER              0x000000E0
#define LAN9303_SWE_PORT_MIRROR_SNIFFER_PORT0        0x00000020
#define LAN9303_SWE_PORT_MIRROR_SNIFFER_PORT1        0x00000040
#define LAN9303_SWE_PORT_MIRROR_SNIFFER_PORT2        0x00000080
#define LAN9303_SWE_PORT_MIRROR_MIRRORED             0x0000001C
#define LAN9303_SWE_PORT_MIRROR_MIRRORED_PORT0       0x00000004
#define LAN9303_SWE_PORT_MIRROR_MIRRORED_PORT1       0x00000008
#define LAN9303_SWE_PORT_MIRROR_MIRRORED_PORT2       0x00000010
#define LAN9303_SWE_PORT_MIRROR_RX_MIRRORING_EN      0x00000002
#define LAN9303_SWE_PORT_MIRROR_TX_MIRRORING_EN      0x00000001

//Switch Engine Ingress Port Type register
#define LAN9303_SWE_INGRSS_PORT_TYP_PORT2            0x00000030
#define LAN9303_SWE_INGRSS_PORT_TYP_PORT2_DIS        0x00000000
#define LAN9303_SWE_INGRSS_PORT_TYP_PORT2_EN         0x00000030
#define LAN9303_SWE_INGRSS_PORT_TYP_PORT1            0x0000000C
#define LAN9303_SWE_INGRSS_PORT_TYP_PORT1_DIS        0x00000000
#define LAN9303_SWE_INGRSS_PORT_TYP_PORT1_EN         0x0000000C
#define LAN9303_SWE_INGRSS_PORT_TYP_PORT0            0x00000003
#define LAN9303_SWE_INGRSS_PORT_TYP_PORT0_DIS        0x00000000
#define LAN9303_SWE_INGRSS_PORT_TYP_PORT0_EN         0x00000003

//Buffer Manager Egress Port Type register
#define LAN9303_BM_EGRSS_PORT_TYPE_VID_SEL_PORT2     0x00400000
#define LAN9303_BM_EGRSS_PORT_TYPE_INSERT_TAG_PORT2  0x00200000
#define LAN9303_BM_EGRSS_PORT_TYPE_CHANGE_VID_PORT2  0x00100000
#define LAN9303_BM_EGRSS_PORT_TYPE_CHANGE_PRIO_PORT2 0x00080000
#define LAN9303_BM_EGRSS_PORT_TYPE_CHANGE_TAG_PORT2  0x00040000
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT2             0x00030000
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT2_DUMB        0x00000000
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT2_ACCESS      0x00010000
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT2_HYBRID      0x00020000
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT2_CPU         0x00030000
#define LAN9303_BM_EGRSS_PORT_TYPE_VID_SEL_PORT1     0x00004000
#define LAN9303_BM_EGRSS_PORT_TYPE_INSERT_TAG_PORT1  0x00002000
#define LAN9303_BM_EGRSS_PORT_TYPE_CHANGE_VID_PORT1  0x00001000
#define LAN9303_BM_EGRSS_PORT_TYPE_CHANGE_PRIO_PORT1 0x00000800
#define LAN9303_BM_EGRSS_PORT_TYPE_CHANGE_TAG_PORT1  0x00000400
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT1             0x00000300
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT1_DUMB        0x00000000
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT1_ACCESS      0x00000100
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT1_HYBRID      0x00000200
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT1_CPU         0x00000300
#define LAN9303_BM_EGRSS_PORT_TYPE_VID_SEL_PORT0     0x00000040
#define LAN9303_BM_EGRSS_PORT_TYPE_INSERT_TAG_PORT0  0x00000020
#define LAN9303_BM_EGRSS_PORT_TYPE_CHANGE_VID_PORT0  0x00000010
#define LAN9303_BM_EGRSS_PORT_TYPE_CHANGE_PRIO_PORT0 0x00000008
#define LAN9303_BM_EGRSS_PORT_TYPE_CHANGE_TAG_PORT0  0x00000004
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT0             0x00000003
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT0_DUMB        0x00000000
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT0_ACCESS      0x00000001
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT0_HYBRID      0x00000002
#define LAN9303_BM_EGRSS_PORT_TYPE_PORT0_CPU         0x00000003

//VLAN tag encoding
#define LAN9303_VLAN_ID_ENCODE(port) htons(0x10 | ((port) & 0x03))
//VLAN tag decoding
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
