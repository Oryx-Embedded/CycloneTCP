/**
 * @file lan9370_driver.h
 * @brief LAN9370 5-port 100Base-T1 Ethernet switch driver
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

#ifndef _LAN9370_DRIVER_H
#define _LAN9370_DRIVER_H

//Dependencies
#include "core/nic.h"

//Port identifiers
#define LAN9370_PORT1 1
#define LAN9370_PORT2 2
#define LAN9370_PORT3 3
#define LAN9370_PORT4 4
#define LAN9370_PORT5 5

//Port masks
#define LAN9370_PORT_MASK  0x1F
#define LAN9370_PORT1_MASK 0x01
#define LAN9370_PORT2_MASK 0x02
#define LAN9370_PORT3_MASK 0x04
#define LAN9370_PORT4_MASK 0x08
#define LAN9370_PORT5_MASK 0x10

//SPI command byte
#define LAN9370_SPI_CMD_WRITE 0x40000000
#define LAN9370_SPI_CMD_READ  0x60000000
#define LAN9370_SPI_CMD_ADDR  0x001FFFE0

//Size of static and dynamic MAC tables
#define LAN9370_STATIC_MAC_TABLE_SIZE  256
#define LAN9370_DYNAMIC_MAC_TABLE_SIZE 1024

//Tail tag rules (host to LAN9370)
#define LAN9370_TAIL_TAG_VALID                  0x2000
#define LAN9370_TAIL_TAG_NORMAL_ADDR_LOOKUP     0x1000
#define LAN9370_TAIL_TAG_PORT_BLOCKING_OVERRIDE 0x0800
#define LAN9370_TAIL_TAG_PRIORITY               0x0700
#define LAN9370_TAIL_TAG_DEST_PORT5             0x0010
#define LAN9370_TAIL_TAG_DEST_PORT4             0x0008
#define LAN9370_TAIL_TAG_DEST_PORT3             0x0004
#define LAN9370_TAIL_TAG_DEST_PORT2             0x0002
#define LAN9370_TAIL_TAG_DEST_PORT1             0x0001

//Tail tag rules (LAN9370 to host)
#define LAN9370_TAIL_TAG_PTP_MSG                0x80
#define LAN9370_TAIL_TAG_SRC_PORT               0x07

//LAN9370 PHY registers
#define LAN9370_BASIC_CONTROL                                     0x00
#define LAN9370_BASIC_STATUS                                      0x01
#define LAN9370_PHY_ID1                                           0x02
#define LAN9370_PHY_ID2                                           0x03
#define LAN9370_MASTER_SLAVE_CONTROL                              0x09
#define LAN9370_MASTER_SLAVE_STATUS                               0x0A
#define LAN9370_EXT_REG_CTL                                       0x14
#define LAN9370_EXT_REG_RD_DATA                                   0x15
#define LAN9370_EXT_REG_WR_DATA                                   0x16

//LAN9370 Switch registers
#define LAN9370_CHIP_ID0                                          0x0000
#define LAN9370_CHIP_ID1                                          0x0001
#define LAN9370_CHIP_ID2                                          0x0002
#define LAN9370_CHIP_ID3                                          0x0003
#define LAN9370_GLOBAL_CTRL0                                      0x0007
#define LAN9370_SWITCH_OP                                         0x0300
#define LAN9370_SWITCH_LUE_CTRL0                                  0x0310
#define LAN9370_SWITCH_LUE_CTRL1                                  0x0311
#define LAN9370_SWITCH_LUE_CTRL2                                  0x0312
#define LAN9370_SWITCH_LUE_CTRL3                                  0x0313
#define LAN9370_SWITCH_LUE_AGE                                    0x0320
#define LAN9370_UNKONWN_MULTICAST_CTRL                            0x0324
#define LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL                     0x0370
#define LAN9370_ALU_TABLE_CTRL                                    0x0418
#define LAN9370_STATIC_MCAST_TABLE_CTRL                           0x041C
#define LAN9370_ALU_TABLE_ENTRY1                                  0x0420
#define LAN9370_STATIC_TABLE_ENTRY1                               0x0420
#define LAN9370_ALU_TABLE_ENTRY2                                  0x0424
#define LAN9370_STATIC_TABLE_ENTRY2                               0x0424
#define LAN9370_ALU_TABLE_ENTRY3                                  0x0428
#define LAN9370_STATIC_TABLE_ENTRY3                               0x0428
#define LAN9370_ALU_TABLE_ENTRY4                                  0x042C
#define LAN9370_STATIC_TABLE_ENTRY4                               0x042C
#define LAN9370_IND_ADDR                                          0x075C
#define LAN9370_IND_DATA                                          0x0760
#define LAN9370_IND_CTRL                                          0x0768
#define LAN9370_VPHY_SPECIAL_CTRL                                 0x077C
#define LAN9370_PORT1_OP_CTRL0                                    0x1020
#define LAN9370_PORT1_STATUS                                      0x1030
#define LAN9370_PORT1_MAC_CTRL0                                   0x1400
#define LAN9370_PORT1_MSTP_STATE                                  0x1B04
#define LAN9370_PORT2_OP_CTRL0                                    0x2020
#define LAN9370_PORT2_STATUS                                      0x2030
#define LAN9370_PORT2_MAC_CTRL0                                   0x2400
#define LAN9370_PORT2_MSTP_STATE                                  0x2B04
#define LAN9370_PORT3_OP_CTRL0                                    0x3020
#define LAN9370_PORT3_STATUS                                      0x3030
#define LAN9370_PORT3_MAC_CTRL0                                   0x3400
#define LAN9370_PORT3_MSTP_STATE                                  0x3B04
#define LAN9370_PORT4_OP_CTRL0                                    0x4020
#define LAN9370_PORT4_STATUS                                      0x4030
#define LAN9370_PORT4_MAC_CTRL0                                   0x4400
#define LAN9370_PORT4_MSTP_STATE                                  0x4B04
#define LAN9370_PORT5_OP_CTRL0                                    0x5020
#define LAN9370_PORT5_STATUS                                      0x5030
#define LAN9370_PORT5_XMII_CTRL0                                  0x5300
#define LAN9370_PORT5_XMII_CTRL1                                  0x5301
#define LAN9370_PORT5_MAC_CTRL0                                   0x5400
#define LAN9370_PORT5_MSTP_STATE                                  0x5B04

//LAN9370 Switch register access macros
#define LAN9370_PORTn_OP_CTRL0(port)                              (0x0020 + ((port) * 0x1000))
#define LAN9370_PORTn_STATUS(port)                                (0x0030 + ((port) * 0x1000))
#define LAN9370_PORTn_XMII_CTRL0(port)                            (0x0300 + ((port) * 0x1000))
#define LAN9370_PORTn_XMII_CTRL1(port)                            (0x0301 + ((port) * 0x1000))
#define LAN9370_PORTn_MAC_CTRL0(port)                             (0x0400 + ((port) * 0x1000))
#define LAN9370_PORTn_MSTP_STATE(port)                            (0x0B04 + ((port) * 0x1000))
#define LAN9370_PORTn_T1_PHY_REG(port, addr)                      (0x0100 + ((port) * 0x1000) + ((addr) * 4))

//Basic Control register
#define LAN9370_BASIC_CONTROL_SW_RESET                            0x8000
#define LAN9370_BASIC_CONTROL_LOOPBACK                            0x4000
#define LAN9370_BASIC_CONTROL_SPEED_SEL_LSB                       0x2000
#define LAN9370_BASIC_CONTROL_AN_EN                               0x1000
#define LAN9370_BASIC_CONTROL_POWER_DOWN                          0x0800
#define LAN9370_BASIC_CONTROL_ISOLATE                             0x0400
#define LAN9370_BASIC_CONTROL_RESTART_AN                          0x0200
#define LAN9370_BASIC_CONTROL_DUPLEX_MODE                         0x0100
#define LAN9370_BASIC_CONTROL_COL_TEST                            0x0080
#define LAN9370_BASIC_CONTROL_SPEED_SEL_MSB                       0x0040
#define LAN9370_BASIC_CONTROL_UNIDIRECTIONAL_EN                   0x0020

//Basic Status register
#define LAN9370_BASIC_STATUS_100BT4                               0x8000
#define LAN9370_BASIC_STATUS_100BTX_FD                            0x4000
#define LAN9370_BASIC_STATUS_100BTX_HD                            0x2000
#define LAN9370_BASIC_STATUS_10BT_FD                              0x1000
#define LAN9370_BASIC_STATUS_10BT_HD                              0x0800
#define LAN9370_BASIC_STATUS_100BT2_FD                            0x0400
#define LAN9370_BASIC_STATUS_100BT2_HD                            0x0200
#define LAN9370_BASIC_STATUS_EXTENDED_STATUS                      0x0100
#define LAN9370_BASIC_STATUS_UNIDIRECTIONAL_EN                    0x0080
#define LAN9370_BASIC_STATUS_MF_PREAMBLE_SUPPR                    0x0040
#define LAN9370_BASIC_STATUS_AN_COMPLETE                          0x0020
#define LAN9370_BASIC_STATUS_REMOTE_FAULT                         0x0010
#define LAN9370_BASIC_STATUS_AN_CAPABLE                           0x0008
#define LAN9370_BASIC_STATUS_LINK_STATUS                          0x0004
#define LAN9370_BASIC_STATUS_JABBER_DETECT                        0x0002
#define LAN9370_BASIC_STATUS_EXTENDED_CAPABLE                     0x0001

//PHY Identifier 1 register
#define LAN9370_PHY_ID1_PHY_ID_MSB                                0xFFFF
#define LAN9370_PHY_ID1_PHY_ID_MSB_DEFAULT                        0x0007

//PHY Identifier 2 register
#define LAN9370_PHY_ID2_PHY_ID_LSB                                0xFC00
#define LAN9370_PHY_ID2_PHY_ID_LSB_DEFAULT                        0xC000
#define LAN9370_PHY_ID2_MODEL_NUM                                 0x03F0
#define LAN9370_PHY_ID2_MODEL_NUM_DEFAULT                         0x0180
#define LAN9370_PHY_ID2_REVISION_NUM                              0x000F

//Master/Slave Control register
#define LAN9370_MASTER_SLAVE_CONTROL_TEST_MODE                    0xE000
#define LAN9370_MASTER_SLAVE_CONTROL_MS_MAN_CONF_EN               0x1000
#define LAN9370_MASTER_SLAVE_CONTROL_MS_MAN_CONF_VAL              0x0800
#define LAN9370_MASTER_SLAVE_CONTROL_PORT_TYPE                    0x0400
#define LAN9370_MASTER_SLAVE_CONTROL_1000BT_FD                    0x0200
#define LAN9370_MASTER_SLAVE_CONTROL_1000BT_HD                    0x0100

//Master/Slave Status register
#define LAN9370_MASTER_SLAVE_STATUS_MS_CONF_FAULT                 0x8000
#define LAN9370_MASTER_SLAVE_STATUS_MS_CONF_RES                   0x4000
#define LAN9370_MASTER_SLAVE_STATUS_LOC_RCVR_STATUS               0x2000
#define LAN9370_MASTER_SLAVE_STATUS_REM_RCVR_STATUS               0x1000
#define LAN9370_MASTER_SLAVE_STATUS_LP_1000BT_FD                  0x0800
#define LAN9370_MASTER_SLAVE_STATUS_LP_1000BT_HD                  0x0400
#define LAN9370_MASTER_SLAVE_STATUS_IDLE_ERR_COUNT                0x00FF

//Extended Register Control register
#define LAN9370_EXT_REG_CTL_READ_CONTROL                          0x1000
#define LAN9370_EXT_REG_CTL_WRITE_CONTROL                         0x0800
#define LAN9370_EXT_REG_CTL_REGISTER_BANK                         0x0700
#define LAN9370_EXT_REG_CTL_REGISTER_BANK_MISC                    0x0100
#define LAN9370_EXT_REG_CTL_REGISTER_BANK_PCS                     0x0200
#define LAN9370_EXT_REG_CTL_REGISTER_BANK_AFE                     0x0300
#define LAN9370_EXT_REG_CTL_REGISTER_BANK_DSP                     0x0400
#define LAN9370_EXT_REG_CTL_REGISTER_ADDR                         0x00FF

//Global Chip ID 0 register
#define LAN9370_CHIP_ID0_DEFAULT                                  0x00

//Global Chip ID 1 register
#define LAN9370_CHIP_ID1_DEFAULT                                  0x93

//Global Chip ID 2 register
#define LAN9370_CHIP_ID2_DEFAULT                                  0x70

//Global Chip ID 3 register
#define LAN9370_CHIP_ID3_REVISION_ID                              0xF0

//Global Control 0 register
#define LAN9370_GLOBAL_CTRL0_APB_PHY_REG_BLK                      0x80
#define LAN9370_GLOBAL_CTRL0_SW_INI_DONE                          0x10

//Switch Operation register
#define LAN9370_SWITCH_OP_DOUBLE_TAG_EN                           0x80
#define LAN9370_SWITCH_OP_OVER_TEMP_ADJ                           0x70
#define LAN9370_SWITCH_OP_OVER_TEMP_FORCE                         0x08
#define LAN9370_SWITCH_OP_OVER_TEMP_EN                            0x04
#define LAN9370_SWITCH_OP_SOFT_HARD_RESET                         0x02
#define LAN9370_SWITCH_OP_START_SWITCH                            0x01

//Switch Lookup Engine Control 0 register
#define LAN9370_SWITCH_LUE_CTRL0_VLAN_EN                          0x80
#define LAN9370_SWITCH_LUE_CTRL0_DROP_INVALID_VID                 0x40
#define LAN9370_SWITCH_LUE_CTRL0_AGE_COUNT                        0x38
#define LAN9370_SWITCH_LUE_CTRL0_AGE_COUNT_DEFAULT                0x20
#define LAN9370_SWITCH_LUE_CTRL0_RESERVED_LUE_EN                  0x04
#define LAN9370_SWITCH_LUE_CTRL0_HASH_OPTION                      0x03
#define LAN9370_SWITCH_LUE_CTRL0_HASH_OPTION_NONE                 0x00
#define LAN9370_SWITCH_LUE_CTRL0_HASH_OPTION_CRC                  0x01
#define LAN9370_SWITCH_LUE_CTRL0_HASH_OPTION_XOR                  0x02

//Switch Lookup Engine Control 1 register
#define LAN9370_SWITCH_LUE_CTRL1_UNICAST_LEARNING_DIS             0x80
#define LAN9370_SWITCH_LUE_CTRL1_SELF_ADDR_FILT                   0x40
#define LAN9370_SWITCH_LUE_CTRL1_FLUSH_ALU_TABLE                  0x20
#define LAN9370_SWITCH_LUE_CTRL1_FLUSH_MSTP_ENTRIES               0x10
#define LAN9370_SWITCH_LUE_CTRL1_SRC_ADDR_FILT                    0x08
#define LAN9370_SWITCH_LUE_CTRL1_AGING_EN                         0x04
#define LAN9370_SWITCH_LUE_CTRL1_FAST_AGE_EN                      0x02
#define LAN9370_SWITCH_LUE_CTRL1_LINK_DOWN_FLUSH                  0x01

//Switch Lookup Engine Control 2 register
#define LAN9370_SWITCH_LUE_CTRL2_MID_RANGE_AGE                    0x80
#define LAN9370_SWITCH_LUE_CTRL2_LINK_DOWN_FLUSH_ACTIVATED        0x40
#define LAN9370_SWITCH_LUE_CTRL2_DYNAMIC_EGRESS_VLAN_FILTERING    0x20
#define LAN9370_SWITCH_LUE_CTRL2_STATIC_EGRESS_VLAN_FILTERING     0x10
#define LAN9370_SWITCH_LUE_CTRL2_FLUSH_OPTION                     0x0C
#define LAN9370_SWITCH_LUE_CTRL2_FLUSH_OPTION_NONE                0x00
#define LAN9370_SWITCH_LUE_CTRL2_FLUSH_OPTION_DYNAMIC             0x04
#define LAN9370_SWITCH_LUE_CTRL2_FLUSH_OPTION_STATIC              0x08
#define LAN9370_SWITCH_LUE_CTRL2_FLUSH_OPTION_BOTH                0x0C
#define LAN9370_SWITCH_LUE_CTRL2_ALU_PRIORITY_GEN                 0x03

//Global Switch Lookup Engine Control 3 register
#define LAN9370_SWITCH_LUE_CTRL3_AGE_PERIOD_7_0                   0xFF
#define LAN9370_SWITCH_LUE_CTRL3_AGE_PERIOD_7_0_DEFAULT           0x4B

//Global Switch LUE Age Period register
#define LAN9370_SWITCH_LUE_AGE_AGE_PERIOD_19_8                    0x0FFF
#define LAN9370_SWITCH_LUE_AGE_AGE_PERIOD_19_8_DEFAULT            0x0000

//Unknown Multicast Control register
#define LAN9370_UNKONWN_MULTICAST_CTRL_FWD                        0x80000000
#define LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP                    0x0000001F
#define LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT1              0x00000001
#define LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT2              0x00000002
#define LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT3              0x00000004
#define LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT4              0x00000008
#define LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT5              0x00000010
#define LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP_ALL                0x0000001F

//Global Port Mirroring and Snooping Control register
#define LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL_IGMP_SNOOP_EN       0x40
#define LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL_MLD_SNOOP_OPT       0x08
#define LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL_MLD_SNOOP_EN        0x04
#define LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL_SNIFF_MODE_SEL      0x01

//ALU Table Access Control register
#define LAN9370_ALU_TABLE_CTRL_VALID_COUNT                        0x3FFF0000
#define LAN9370_ALU_TABLE_CTRL_START_FINISH                       0x00000080
#define LAN9370_ALU_TABLE_CTRL_VALID                              0x00000040
#define LAN9370_ALU_TABLE_CTRL_VALID_ENTRY_OR_SEARCH_END          0x00000020
#define LAN9370_ALU_TABLE_CTRL_DIRECT                             0x00000004
#define LAN9370_ALU_TABLE_CTRL_ACTION                             0x00000003
#define LAN9370_ALU_TABLE_CTRL_ACTION_NOP                         0x00000000
#define LAN9370_ALU_TABLE_CTRL_ACTION_WRITE                       0x00000001
#define LAN9370_ALU_TABLE_CTRL_ACTION_READ                        0x00000002
#define LAN9370_ALU_TABLE_CTRL_ACTION_SEARCH                      0x00000003

//Static Address and Reserved Multicast Table Control register
#define LAN9370_STATIC_MCAST_TABLE_CTRL_VALID_COUNT               0x1FF00000
#define LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_INDEX               0x0000FF00
#define LAN9370_STATIC_MCAST_TABLE_CTRL_START_FINISH              0x00000080
#define LAN9370_STATIC_MCAST_TABLE_CTRL_VALID                     0x00000040
#define LAN9370_STATIC_MCAST_TABLE_CTRL_VALID_ENTRY_OR_SEARCH_END 0x00000020
#define LAN9370_STATIC_MCAST_TABLE_CTRL_DIRECT_ACCESS             0x00000008
#define LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_SELECT              0x00000004
#define LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_SELECT_STATIC       0x00000000
#define LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_SELECT_RESERVED     0x00000004
#define LAN9370_STATIC_MCAST_TABLE_CTRL_ACTION                    0x00000003
#define LAN9370_STATIC_MCAST_TABLE_CTRL_ACTION_NOP                0x00000000
#define LAN9370_STATIC_MCAST_TABLE_CTRL_ACTION_WRITE              0x00000001
#define LAN9370_STATIC_MCAST_TABLE_CTRL_ACTION_READ               0x00000002
#define LAN9370_STATIC_MCAST_TABLE_CTRL_ACTION_SEARCH             0x00000003

//ALU Table Entry 1 register
#define LAN9370_ALU_TABLE_ENTRY1_STATIC                           0x80000000
#define LAN9370_ALU_TABLE_ENTRY1_SRC_FILTER                       0x40000000
#define LAN9370_ALU_TABLE_ENTRY1_DES_FILTER                       0x20000000
#define LAN9370_ALU_TABLE_ENTRY1_PRIORITY                         0x1C000000
#define LAN9370_ALU_TABLE_ENTRY1_AGE_COUNT                        0x1C000000
#define LAN9370_ALU_TABLE_ENTRY1_MSTP                             0x00000007

//ALU Table Entry 2 register
#define LAN9370_ALU_TABLE_ENTRY2_OVERRIDE                         0x80000000
#define LAN9370_ALU_TABLE_ENTRY2_PORT_FORWARD                     0x0000001F
#define LAN9370_ALU_TABLE_ENTRY2_PORT5_FORWARD                    0x00000010
#define LAN9370_ALU_TABLE_ENTRY2_PORT4_FORWARD                    0x00000008
#define LAN9370_ALU_TABLE_ENTRY2_PORT3_FORWARD                    0x00000004
#define LAN9370_ALU_TABLE_ENTRY2_PORT2_FORWARD                    0x00000002
#define LAN9370_ALU_TABLE_ENTRY2_PORT1_FORWARD                    0x00000001

//ALU Table Entry 3 register
#define LAN9370_ALU_TABLE_ENTRY3_FID                              0x007F0000
#define LAN9370_ALU_TABLE_ENTRY3_MAC_ADDR_MSB                     0x0000FFFF

//ALU Table Entry 4 register
#define LAN9370_ALU_TABLE_ENTRY4_MAC_ADDR_LSB                     0xFFFFFFFF

//Static Address Table Entry 1 register
#define LAN9370_STATIC_TABLE_ENTRY1_VALID                         0x80000000
#define LAN9370_STATIC_TABLE_ENTRY1_SRC_FILTER                    0x40000000
#define LAN9370_STATIC_TABLE_ENTRY1_DES_FILTER                    0x20000000
#define LAN9370_STATIC_TABLE_ENTRY1_PRIORITY                      0x1C000000
#define LAN9370_STATIC_TABLE_ENTRY1_MSTP                          0x00000007

//Static Address Table Entry 2 register
#define LAN9370_STATIC_TABLE_ENTRY2_OVERRIDE                      0x80000000
#define LAN9370_STATIC_TABLE_ENTRY2_USE_FID                       0x40000000
#define LAN9370_STATIC_TABLE_ENTRY2_PORT_FORWARD                  0x0000001F
#define LAN9370_STATIC_TABLE_ENTRY2_PORT5_FORWARD                 0x00000010
#define LAN9370_STATIC_TABLE_ENTRY2_PORT4_FORWARD                 0x00000008
#define LAN9370_STATIC_TABLE_ENTRY2_PORT3_FORWARD                 0x00000004
#define LAN9370_STATIC_TABLE_ENTRY2_PORT2_FORWARD                 0x00000002
#define LAN9370_STATIC_TABLE_ENTRY2_PORT1_FORWARD                 0x00000001

//Static Address Table Entry 3 register
#define LAN9370_STATIC_TABLE_ENTRY3_FID                           0x007F0000
#define LAN9370_STATIC_TABLE_ENTRY3_MAC_ADDR_MSB                  0x0000FFFF

//Static Address Table Entry 4 register
#define LAN9370_STATIC_TABLE_ENTRY4_MAC_ADDR_LSB                  0xFFFFFFFF

//Indirect Control register
#define LAN9370_IND_CTRL_OP                                       0x0002
#define LAN9370_IND_CTRL_OP_READ                                  0x0000
#define LAN9370_IND_CTRL_OP_WRITE                                 0x0002
#define LAN9370_IND_CTRL_BUSY                                     0x0001

//Virtual PHY Special Control/Status register
#define LAN9370_VPHY_SPECIAL_CTRL_SMI_INDIRECT_EN                 0x8000
#define LAN9370_VPHY_SPECIAL_CTRL_SW_LOOPBACK                     0x4000
#define LAN9370_VPHY_SPECIAL_CTRL_MDIO_INT_EN                     0x2000
#define LAN9370_VPHY_SPECIAL_CTRL_SPI_INDIRECT_EN                 0x1000
#define LAN9370_VPHY_SPECIAL_CTRL_MODE                            0x0300
#define LAN9370_VPHY_SPECIAL_CTRL_MODE_RGMII                      0x0000
#define LAN9370_VPHY_SPECIAL_CTRL_MODE_MII_PHY                    0x0100
#define LAN9370_VPHY_SPECIAL_CTRL_MODE_SGMII                      0x0200
#define LAN9370_VPHY_SPECIAL_CTRL_MODE_RMII_PHY                   0x0300
#define LAN9370_VPHY_SPECIAL_CTRL_SW_COL_TEST                     0x0080
#define LAN9370_VPHY_SPECIAL_CTRL_SPEED_DUPLEX_IND                0x001C
#define LAN9370_VPHY_SPECIAL_CTRL_SPEED_DUPLEX_IND_10_HD          0x0004
#define LAN9370_VPHY_SPECIAL_CTRL_SPEED_DUPLEX_IND_100_HD         0x0008
#define LAN9370_VPHY_SPECIAL_CTRL_SPEED_DUPLEX_IND_10_FD          0x0014
#define LAN9370_VPHY_SPECIAL_CTRL_SPEED_DUPLEX_IND_100_FD         0x0018

//Port N Operation Control 0 register
#define LAN9370_PORTn_OP_CTRL0_LOCAL_LOOPBACK                     0x80
#define LAN9370_PORTn_OP_CTRL0_REMOTE_LOOPBACK                    0x40
#define LAN9370_PORTn_OP_CTRL0_K2L_INSERT_EN                      0x20
#define LAN9370_PORTn_OP_CTRL0_K2L_DEBUG_EN                       0x10
#define LAN9370_PORTn_OP_CTRL0_TAIL_TAG_EN                        0x04
#define LAN9370_PORTn_OP_CTRL0_TX_QUEUE_SPLIT_EN                  0x03

//Port N Status register
#define LAN9370_PORTn_STATUS_SPEED                                0x18
#define LAN9370_PORTn_STATUS_SPEED_10MBPS                         0x00
#define LAN9370_PORTn_STATUS_SPEED_100MBPS                        0x08
#define LAN9370_PORTn_STATUS_SPEED_1000MBPS                       0x10
#define LAN9370_PORTn_STATUS_DUPLEX                               0x04
#define LAN9370_PORTn_STATUS_TX_FLOW_CTRL_EN                      0x02
#define LAN9370_PORTn_STATUS_RX_FLOW_CTRL_EN                      0x01

//XMII Port N Control 0 register
#define LAN9370_PORTn_XMII_CTRL0_DUPLEX                           0x40
#define LAN9370_PORTn_XMII_CTRL0_TX_FLOW_CTRL_EN                  0x20
#define LAN9370_PORTn_XMII_CTRL0_SPEED_10_100                     0x10
#define LAN9370_PORTn_XMII_CTRL0_RX_FLOW_CTRL_EN                  0x08
#define LAN9370_PORTn_XMII_CTRL0_CRS_COL_EN                       0x02
#define LAN9370_PORTn_XMII_CTRL0_AUTO_COL_DET                     0x01

//Port N XMII Control 1 register
#define LAN9370_PORTn_XMII_CTRL1_RGMII_SPEED_1000                 0x40
#define LAN9370_PORTn_XMII_CTRL1_SEL_EDGE                         0x20
#define LAN9370_PORTn_XMII_CTRL1_RGMII_ID_IG                      0x10
#define LAN9370_PORTn_XMII_CTRL1_RGMII_ID_EG                      0x08
#define LAN9370_PORTn_XMII_CTRL1_MII_RMII_MODE                    0x04
#define LAN9370_PORTn_XMII_CTRL1_MII_INTF_SEL                     0x03
#define LAN9370_PORTn_XMII_CTRL1_MII_INTF_SEL_RGMII               0x00
#define LAN9370_PORTn_XMII_CTRL1_MII_INTF_SEL_RMII                0x01
#define LAN9370_PORTn_XMII_CTRL1_MII_INTF_SEL_MII                 0x03

//Port N MAC Control 0 register
#define LAN9370_PORTn_MAC_CTRL0_FR_LEN_CHK                        0x04
#define LAN9370_PORTn_MAC_CTRL0_BCAST_STORM_EN                    0x02
#define LAN9370_PORTn_MAC_CTRL0_JUMBO_PKT_EN                      0x01

//Port N MSTP State register
#define LAN9370_PORTn_MSTP_STATE_TRANSMIT_EN                      0x04
#define LAN9370_PORTn_MSTP_STATE_RECEIVE_EN                       0x02
#define LAN9370_PORTn_MSTP_STATE_LEARNING_DIS                     0x01

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//LAN9370 Ethernet switch driver
extern const SwitchDriver lan9370SwitchDriver;

//LAN9370 related functions
error_t lan9370Init(NetInterface *interface);
void lan9370InitHook(NetInterface *interface);

void lan9370Tick(NetInterface *interface);

void lan9370EnableIrq(NetInterface *interface);
void lan9370DisableIrq(NetInterface *interface);

void lan9370EventHandler(NetInterface *interface);

error_t lan9370TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, NetTxAncillary *ancillary);

error_t lan9370UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, NetRxAncillary *ancillary);

bool_t lan9370GetLinkState(NetInterface *interface, uint8_t port);
uint32_t lan9370GetLinkSpeed(NetInterface *interface, uint8_t port);
NicDuplexMode lan9370GetDuplexMode(NetInterface *interface, uint8_t port);

void lan9370SetPortState(NetInterface *interface, uint8_t port,
   SwitchPortState state);

SwitchPortState lan9370GetPortState(NetInterface *interface, uint8_t port);

void lan9370SetAgingTime(NetInterface *interface, uint32_t agingTime);

void lan9370EnableIgmpSnooping(NetInterface *interface, bool_t enable);
void lan9370EnableMldSnooping(NetInterface *interface, bool_t enable);
void lan9370EnableRsvdMcastTable(NetInterface *interface, bool_t enable);

error_t lan9370AddStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry);

error_t lan9370DeleteStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry);

error_t lan9370GetStaticFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry);

void lan9370FlushStaticFdbTable(NetInterface *interface);

error_t lan9370GetDynamicFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry);

void lan9370FlushDynamicFdbTable(NetInterface *interface, uint8_t port);

void lan9370SetUnknownMcastFwdPorts(NetInterface *interface,
   bool_t enable, uint32_t forwardPorts);

void lan9370WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data);

uint16_t lan9370ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address);

void lan9370DumpPhyReg(NetInterface *interface, uint8_t port);

void lan9370WriteExtReg(NetInterface *interface, uint8_t port, uint8_t bank,
   uint8_t addr, uint16_t data);

uint16_t lan9370ReadExtReg(NetInterface *interface, uint8_t port, uint8_t bank,
   uint8_t addr);

void lan9370WriteSwitchReg8(NetInterface *interface, uint16_t address,
   uint8_t data);

uint8_t lan9370ReadSwitchReg8(NetInterface *interface, uint16_t address);

void lan9370WriteSwitchReg16(NetInterface *interface, uint16_t address,
   uint16_t data);

uint16_t lan9370ReadSwitchReg16(NetInterface *interface, uint16_t address);

void lan9370WriteSwitchReg32(NetInterface *interface, uint16_t address,
   uint32_t data);

uint32_t lan9370ReadSwitchReg32(NetInterface *interface, uint16_t address);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
