/**
 * @file ksz9563_driver.h
 * @brief KSZ9563 3-port Gigabit Ethernet switch driver
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

#ifndef _KSZ9563_DRIVER_H
#define _KSZ9563_DRIVER_H

//Dependencies
#include "core/nic.h"

//Port identifiers
#define KSZ9563_PORT1 1
#define KSZ9563_PORT2 2
#define KSZ9563_PORT3 3

//Port masks
#define KSZ9563_PORT_MASK  0x07
#define KSZ9563_PORT1_MASK 0x01
#define KSZ9563_PORT2_MASK 0x02
#define KSZ9563_PORT3_MASK 0x04

//SPI command byte
#define KSZ9563_SPI_CMD_WRITE 0x40000000
#define KSZ9563_SPI_CMD_READ  0x60000000
#define KSZ9563_SPI_CMD_ADDR  0x001FFFE0

//Size of static and dynamic MAC tables
#define KSZ9563_STATIC_MAC_TABLE_SIZE  16
#define KSZ9563_DYNAMIC_MAC_TABLE_SIZE 4096

//Tail tag rules (host to KSZ9563)
#define KSZ9563_TAIL_TAG_NORMAL_ADDR_LOOKUP     0x40
#define KSZ9563_TAIL_TAG_PORT_BLOCKING_OVERRIDE 0x20
#define KSZ9563_TAIL_TAG_PRIORITY               0x18
#define KSZ9563_TAIL_TAG_DEST_PORT3             0x04
#define KSZ9563_TAIL_TAG_DEST_PORT2             0x02
#define KSZ9563_TAIL_TAG_DEST_PORT1             0x01

//Tail tag rules (KSZ9563 to host)
#define KSZ9563_TAIL_TAG_PTP_MSG                0x80
#define KSZ9563_TAIL_TAG_SRC_PORT               0x03

//KSZ9563 PHY registers
#define KSZ9563_BMCR                                                 0x00
#define KSZ9563_BMSR                                                 0x01
#define KSZ9563_PHYID1                                               0x02
#define KSZ9563_PHYID2                                               0x03
#define KSZ9563_ANAR                                                 0x04
#define KSZ9563_ANLPAR                                               0x05
#define KSZ9563_ANER                                                 0x06
#define KSZ9563_ANNPR                                                0x07
#define KSZ9563_ANLPNPR                                              0x08
#define KSZ9563_GBCR                                                 0x09
#define KSZ9563_GBSR                                                 0x0A
#define KSZ9563_MMDACR                                               0x0D
#define KSZ9563_MMDAADR                                              0x0E
#define KSZ9563_GBESR                                                0x0F
#define KSZ9563_RLB                                                  0x11
#define KSZ9563_LINKMD                                               0x12
#define KSZ9563_DPMAPCSS                                             0x13
#define KSZ9563_RXERCTR                                              0x15
#define KSZ9563_ICSR                                                 0x1B
#define KSZ9563_AUTOMDI                                              0x1C
#define KSZ9563_PHYCON                                               0x1F

//KSZ9563 MMD registers
#define KSZ9563_MMD_LED_MODE                                         0x02, 0x00
#define KSZ9563_MMD_1000BT_EEE_WAKE_TX_TIMER                         0x03, 0x0E
#define KSZ9563_MMD_EEE_ADV                                          0x07, 0x3C

//KSZ9563 Switch registers
#define KSZ9563_CHIP_ID0                                             0x0000
#define KSZ9563_CHIP_ID1                                             0x0001
#define KSZ9563_CHIP_ID2                                             0x0002
#define KSZ9563_CHIP_ID3                                             0x0003
#define KSZ9563_PME_PIN_CTRL                                         0x0006
#define KSZ9563_CHIP_ID4                                             0x000F
#define KSZ9563_GLOBAL_INT_STAT                                      0x0010
#define KSZ9563_GLOBAL_INT_MASK                                      0x0014
#define KSZ9563_GLOBAL_PORT_INT_STAT                                 0x0018
#define KSZ9563_GLOBAL_PORT_INT_MASK                                 0x001C
#define KSZ9563_SERIAL_IO_CTRL                                       0x0100
#define KSZ9563_IBA_CTRL                                             0x0104
#define KSZ9563_IO_DRIVE_STRENGTH                                    0x010D
#define KSZ9563_IBA_OP_STAT1                                         0x0110
#define KSZ9563_LED_OVERRIDE                                         0x0120
#define KSZ9563_LED_OUTPUT                                           0x0124
#define KSZ9563_LED2_0_LED2_1_SRC                                    0x0128
#define KSZ9563_PWR_DOWN_CTRL0                                       0x0201
#define KSZ9563_LED_STRAP_IN                                         0x0210
#define KSZ9563_SWITCH_OP                                            0x0300
#define KSZ9563_SWITCH_MAC_ADDR0                                     0x0302
#define KSZ9563_SWITCH_MAC_ADDR1                                     0x0303
#define KSZ9563_SWITCH_MAC_ADDR2                                     0x0304
#define KSZ9563_SWITCH_MAC_ADDR3                                     0x0305
#define KSZ9563_SWITCH_MAC_ADDR4                                     0x0306
#define KSZ9563_SWITCH_MAC_ADDR5                                     0x0307
#define KSZ9563_SWITCH_MTU                                           0x0308
#define KSZ9563_SWITCH_ISP_TPID                                      0x030A
#define KSZ9563_AVB_CBS_STRATEGY                                     0x030E
#define KSZ9563_SWITCH_LUE_CTRL0                                     0x0310
#define KSZ9563_SWITCH_LUE_CTRL1                                     0x0311
#define KSZ9563_SWITCH_LUE_CTRL2                                     0x0312
#define KSZ9563_SWITCH_LUE_CTRL3                                     0x0313
#define KSZ9563_ALU_TABLE_INT                                        0x0314
#define KSZ9563_ALU_TABLE_MASK                                       0x0315
#define KSZ9563_ALU_TABLE_ENTRY_INDEX0                               0x0316
#define KSZ9563_ALU_TABLE_ENTRY_INDEX1                               0x0318
#define KSZ9563_ALU_TABLE_ENTRY_INDEX2                               0x031A
#define KSZ9563_UNKNOWN_UNICAST_CTRL                                 0x0320
#define KSZ9563_UNKONWN_MULTICAST_CTRL                               0x0324
#define KSZ9563_UNKNOWN_VLAN_ID_CTRL                                 0x0328
#define KSZ9563_SWITCH_MAC_CTRL0                                     0x0330
#define KSZ9563_SWITCH_MAC_CTRL1                                     0x0331
#define KSZ9563_SWITCH_MAC_CTRL2                                     0x0332
#define KSZ9563_SWITCH_MAC_CTRL3                                     0x0333
#define KSZ9563_SWITCH_MAC_CTRL4                                     0x0334
#define KSZ9563_SWITCH_MAC_CTRL5                                     0x0335
#define KSZ9563_SWITCH_MIB_CTRL                                      0x0336
#define KSZ9563_802_1P_PRIO_MAPPING0                                 0x0338
#define KSZ9563_802_1P_PRIO_MAPPING1                                 0x0339
#define KSZ9563_802_1P_PRIO_MAPPING2                                 0x033A
#define KSZ9563_802_1P_PRIO_MAPPING3                                 0x033B
#define KSZ9563_IP_DIFFSERV_PRIO_EN                                  0x033E
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING0                            0x0340
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING1                            0x0341
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING2                            0x0342
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING3                            0x0343
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING4                            0x0344
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING5                            0x0345
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING6                            0x0346
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING7                            0x0347
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING8                            0x0348
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING9                            0x0349
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING10                           0x034A
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING11                           0x034B
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING12                           0x034C
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING13                           0x034D
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING14                           0x034E
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING15                           0x034F
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING16                           0x0350
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING17                           0x0351
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING18                           0x0352
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING19                           0x0353
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING20                           0x0354
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING21                           0x0355
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING22                           0x0356
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING23                           0x0357
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING24                           0x0358
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING25                           0x0359
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING26                           0x035A
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING27                           0x035B
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING28                           0x035C
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING29                           0x035D
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING30                           0x035E
#define KSZ9563_IP_DIFFSERV_PRIO_MAPPING31                           0x035F
#define KSZ9563_GLOBAL_PORT_MIRROR_SNOOP_CTRL                        0x0370
#define KSZ9563_WRED_DIFFSERV_COLOR_MAPPING                          0x0378
#define KSZ9563_PTP_EVENT_MSG_PRIO                                   0x037C
#define KSZ9563_PTP_NON_EVENT_MSG_PRIO                               0x037D
#define KSZ9563_QUEUE_MGMT_CTRL0                                     0x0390
#define KSZ9563_VLAN_TABLE_ENTRY0                                    0x0400
#define KSZ9563_VLAN_TABLE_ENTRY1                                    0x0404
#define KSZ9563_VLAN_TABLE_ENTRY2                                    0x0408
#define KSZ9563_VLAN_TABLE_INDEX                                     0x040C
#define KSZ9563_VLAN_TABLE_ACCESS_CTRL                               0x040E
#define KSZ9563_ALU_TABLE_INDEX0                                     0x0410
#define KSZ9563_ALU_TABLE_INDEX1                                     0x0414
#define KSZ9563_ALU_TABLE_CTRL                                       0x0418
#define KSZ9563_STATIC_MCAST_TABLE_CTRL                              0x041C
#define KSZ9563_ALU_TABLE_ENTRY1                                     0x0420
#define KSZ9563_STATIC_TABLE_ENTRY1                                  0x0420
#define KSZ9563_ALU_TABLE_ENTRY2                                     0x0424
#define KSZ9563_STATIC_TABLE_ENTRY2                                  0x0424
#define KSZ9563_RES_MCAST_TABLE_ENTRY2                               0x0424
#define KSZ9563_ALU_TABLE_ENTRY3                                     0x0428
#define KSZ9563_STATIC_TABLE_ENTRY3                                  0x0428
#define KSZ9563_ALU_TABLE_ENTRY4                                     0x042C
#define KSZ9563_STATIC_TABLE_ENTRY4                                  0x042C
#define KSZ9563_GLOBAL_PTP_CLK_CTRL                                  0x0500
#define KSZ9563_GLOBAL_PTP_RTC_CLK_PHASE                             0x0502
#define KSZ9563_GLOBAL_PTP_RTC_CLK_NS_H                              0x0504
#define KSZ9563_GLOBAL_PTP_RTC_CLK_NS_L                              0x0506
#define KSZ9563_GLOBAL_PTP_RTC_CLK_S_H                               0x0508
#define KSZ9563_GLOBAL_PTP_RTC_CLK_S_L                               0x050A
#define KSZ9563_GLOBAL_PTP_CLK_SUB_NS_RATE_H                         0x050C
#define KSZ9563_GLOBAL_PTP_CLK_SUB_NS_RATE_L                         0x050E
#define KSZ9563_GLOBAL_PTP_CLK_TEMP_ADJ_DURATION_H                   0x0510
#define KSZ9563_GLOBAL_PTP_CLK_TEMP_ADJ_DURATION_L                   0x0512
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1                               0x0514
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2                               0x0516
#define KSZ9563_GLOBAL_PTP_DOMAIN_VERSION                            0x0518
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX                                0x0520
#define KSZ9563_GPIO_STATUS_MONITOR0                                 0x0524
#define KSZ9563_GPIO_STATUS_MONITOR1                                 0x0528
#define KSZ9563_TS_CTRL_STAT                                         0x052C
#define KSZ9563_TOU_TARGET_TIME_NS                                   0x0530
#define KSZ9563_TOU_TARGET_TIME_S                                    0x0534
#define KSZ9563_TOU_CTRL1                                            0x0538
#define KSZ9563_TOU_CTRL2                                            0x053C
#define KSZ9563_TOU_CTRL3                                            0x0540
#define KSZ9563_TOU_CTRL4                                            0x0544
#define KSZ9563_TOU_CTRL5                                            0x0548
#define KSZ9563_TS_STAT_CTRL                                         0x0550
#define KSZ9563_TS_SAMPLE1_TIME_NS                                   0x0554
#define KSZ9563_TS_SAMPLE1_TIME_S                                    0x0558
#define KSZ9563_TS_SAMPLE1_TIME_PHASE                                0x055C
#define KSZ9563_TS_SAMPLE2_TIME_NS                                   0x0560
#define KSZ9563_TS_SAMPLE2_TIME_S                                    0x0564
#define KSZ9563_TS_SAMPLE2_TIME_PHASE                                0x0568
#define KSZ9563_TS_SAMPLE3_TIME_NS                                   0x056C
#define KSZ9563_TS_SAMPLE3_TIME_S                                    0x0570
#define KSZ9563_TS_SAMPLE3_TIME_PHASE                                0x0574
#define KSZ9563_TS_SAMPLE4_TIME_NS                                   0x0578
#define KSZ9563_TS_SAMPLE4_TIME_S                                    0x057C
#define KSZ9563_TS_SAMPLE4_TIME_PHASE                                0x0580
#define KSZ9563_TS_SAMPLE5_TIME_NS                                   0x0584
#define KSZ9563_TS_SAMPLE5_TIME_S                                    0x0588
#define KSZ9563_TS_SAMPLE5_TIME_PHASE                                0x058C
#define KSZ9563_TS_SAMPLE6_TIME_NS                                   0x0590
#define KSZ9563_TS_SAMPLE6_TIME_S                                    0x0594
#define KSZ9563_TS_SAMPLE6_TIME_PHASE                                0x0598
#define KSZ9563_TS_SAMPLE7_TIME_NS                                   0x059C
#define KSZ9563_TS_SAMPLE7_TIME_S                                    0x05A0
#define KSZ9563_TS_SAMPLE7_TIME_PHASE                                0x05A4
#define KSZ9563_TS_SAMPLE8_TIME_NS                                   0x05A8
#define KSZ9563_TS_SAMPLE8_TIME_S                                    0x05AC
#define KSZ9563_TS_SAMPLE8_TIME_PHASE                                0x05B0
#define KSZ9563_PORT1_DEFAULT_TAG0                                   0x1000
#define KSZ9563_PORT1_DEFAULT_TAG1                                   0x1001
#define KSZ9563_PORT1_AVB_SR_CLASS1_TAG0                             0x1004
#define KSZ9563_PORT1_AVB_SR_CLASS1_TAG1                             0x1005
#define KSZ9563_PORT1_AVB_SR_CLASS2_TAG0                             0x1006
#define KSZ9563_PORT1_AVB_SR_CLASS2_TAG1                             0x1007
#define KSZ9563_PORT1_AVB_SR_CLASS1_TYPE                             0x1008
#define KSZ9563_PORT1_AVB_SR_CLASS2_TYPE                             0x100A
#define KSZ9563_PORT1_PME_WOL_EVENT                                  0x1013
#define KSZ9563_PORT1_PME_WOL_EN                                     0x1017
#define KSZ9563_PORT1_INT_STATUS                                     0x101B
#define KSZ9563_PORT1_INT_MASK                                       0x101F
#define KSZ9563_PORT1_OP_CTRL0                                       0x1020
#define KSZ9563_PORT1_OP_CTRL1                                       0x1021
#define KSZ9563_PORT1_STATUS                                         0x1030
#define KSZ9563_PORT1_MAC_CTRL0                                      0x1400
#define KSZ9563_PORT1_MAC_CTRL1                                      0x1401
#define KSZ9563_PORT1_IG_RATE_LIMIT_CTRL                             0x1403
#define KSZ9563_PORT1_PRIO0_IG_LIMIT_CTRL                            0x1410
#define KSZ9563_PORT1_PRIO1_IG_LIMIT_CTRL                            0x1411
#define KSZ9563_PORT1_PRIO2_IG_LIMIT_CTRL                            0x1412
#define KSZ9563_PORT1_PRIO3_IG_LIMIT_CTRL                            0x1413
#define KSZ9563_PORT1_PRIO4_IG_LIMIT_CTRL                            0x1414
#define KSZ9563_PORT1_PRIO5_IG_LIMIT_CTRL                            0x1415
#define KSZ9563_PORT1_PRIO6_IG_LIMIT_CTRL                            0x1416
#define KSZ9563_PORT1_PRIO7_IG_LIMIT_CTRL                            0x1417
#define KSZ9563_PORT1_QUEUE0_EG_LIMIT_CTRL                           0x1420
#define KSZ9563_PORT1_QUEUE1_EG_LIMIT_CTRL                           0x1421
#define KSZ9563_PORT1_QUEUE2_EG_LIMIT_CTRL                           0x1422
#define KSZ9563_PORT1_QUEUE3_EG_LIMIT_CTRL                           0x1423
#define KSZ9563_PORT1_MIB_CTRL_STAT                                  0x1500
#define KSZ9563_PORT1_MIB_DATA                                       0x1504
#define KSZ9563_PORT1_ACL_ACCESS0                                    0x1600
#define KSZ9563_PORT1_ACL_ACCESS1                                    0x1601
#define KSZ9563_PORT1_ACL_ACCESS2                                    0x1602
#define KSZ9563_PORT1_ACL_ACCESS3                                    0x1603
#define KSZ9563_PORT1_ACL_ACCESS4                                    0x1604
#define KSZ9563_PORT1_ACL_ACCESS5                                    0x1605
#define KSZ9563_PORT1_ACL_ACCESS6                                    0x1606
#define KSZ9563_PORT1_ACL_ACCESS7                                    0x1607
#define KSZ9563_PORT1_ACL_ACCESS8                                    0x1608
#define KSZ9563_PORT1_ACL_ACCESS9                                    0x1609
#define KSZ9563_PORT1_ACL_ACCESS10                                   0x160A
#define KSZ9563_PORT1_ACL_ACCESS11                                   0x160B
#define KSZ9563_PORT1_ACL_ACCESS12                                   0x160C
#define KSZ9563_PORT1_ACL_ACCESS13                                   0x160D
#define KSZ9563_PORT1_ACL_ACCESS14                                   0x160E
#define KSZ9563_PORT1_ACL_ACCESS15                                   0x160F
#define KSZ9563_PORT1_ACL_BYTE_EN_MSB                                0x1610
#define KSZ9563_PORT1_ACL_BYTE_EN_LSB                                0x1611
#define KSZ9563_PORT1_ACL_ACCESS_CTRL0                               0x1612
#define KSZ9563_PORT1_MIRRORING_CTRL                                 0x1800
#define KSZ9563_PORT1_PRIO_CTRL                                      0x1801
#define KSZ9563_PORT1_IG_MAC_CTRL                                    0x1802
#define KSZ9563_PORT1_AUTH_CTRL                                      0x1803
#define KSZ9563_PORT1_PTR                                            0x1804
#define KSZ9563_PORT1_PRIO_TO_QUEUE_MAPPING                          0x1808
#define KSZ9563_PORT1_POLICE_CTRL                                    0x180C
#define KSZ9563_PORT1_TX_QUEUE_INDEX                                 0x1900
#define KSZ9563_PORT1_TX_QUEUE_PVID                                  0x1904
#define KSZ9563_PORT1_TX_QUEUE_CTRL0                                 0x1914
#define KSZ9563_PORT1_TX_QUEUE_CTRL1                                 0x1915
#define KSZ9563_PORT1_TX_CREDIT_SHAPER_CTRL0                         0x1916
#define KSZ9563_PORT1_TX_CREDIT_SHAPER_CTRL1                         0x1918
#define KSZ9563_PORT1_TX_CREDIT_SHAPER_CTRL2                         0x191A
#define KSZ9563_PORT1_CTRL0                                          0x1A00
#define KSZ9563_PORT1_CTRL1                                          0x1A04
#define KSZ9563_PORT1_CTRL2                                          0x1B00
#define KSZ9563_PORT1_MSTP_PTR                                       0x1B01
#define KSZ9563_PORT1_MSTP_STATE                                     0x1B04
#define KSZ9563_PORT1_PTP_RX_LATENCY                                 0x1C00
#define KSZ9563_PORT1_PTP_TX_LATENCY                                 0x1C02
#define KSZ9563_PORT1_PTP_ASYM_CORRECTION                            0x1C04
#define KSZ9563_PORT1_PTP_XDLY_REQ_TSH                               0x1C08
#define KSZ9563_PORT1_PTP_XDLY_REQ_TSL                               0x1C0A
#define KSZ9563_PORT1_PTP_SYNC_TSH                                   0x1C0C
#define KSZ9563_PORT1_PTP_SYNC_TSL                                   0x1C0E
#define KSZ9563_PORT1_PTP_PDLY_RESP_TSH                              0x1C10
#define KSZ9563_PORT1_PTP_PDLY_RESP_TSL                              0x1C12
#define KSZ9563_PORT1_PTP_TS_INT_STAT                                0x1C14
#define KSZ9563_PORT1_PTP_TS_INT_EN                                  0x1C16
#define KSZ9563_PORT1_PTP_LINK_DELAY                                 0x1C18
#define KSZ9563_PORT2_DEFAULT_TAG0                                   0x2000
#define KSZ9563_PORT2_DEFAULT_TAG1                                   0x2001
#define KSZ9563_PORT2_AVB_SR_CLASS1_TAG0                             0x2004
#define KSZ9563_PORT2_AVB_SR_CLASS1_TAG1                             0x2005
#define KSZ9563_PORT2_AVB_SR_CLASS2_TAG0                             0x2006
#define KSZ9563_PORT2_AVB_SR_CLASS2_TAG1                             0x2007
#define KSZ9563_PORT2_AVB_SR_CLASS1_TYPE                             0x2008
#define KSZ9563_PORT2_AVB_SR_CLASS2_TYPE                             0x200A
#define KSZ9563_PORT2_PME_WOL_EVENT                                  0x2013
#define KSZ9563_PORT2_PME_WOL_EN                                     0x2017
#define KSZ9563_PORT2_INT_STATUS                                     0x201B
#define KSZ9563_PORT2_INT_MASK                                       0x201F
#define KSZ9563_PORT2_OP_CTRL0                                       0x2020
#define KSZ9563_PORT2_OP_CTRL1                                       0x2021
#define KSZ9563_PORT2_STATUS                                         0x2030
#define KSZ9563_PORT2_MAC_CTRL0                                      0x2400
#define KSZ9563_PORT2_MAC_CTRL1                                      0x2401
#define KSZ9563_PORT2_IG_RATE_LIMIT_CTRL                             0x2403
#define KSZ9563_PORT2_PRIO0_IG_LIMIT_CTRL                            0x2410
#define KSZ9563_PORT2_PRIO1_IG_LIMIT_CTRL                            0x2411
#define KSZ9563_PORT2_PRIO2_IG_LIMIT_CTRL                            0x2412
#define KSZ9563_PORT2_PRIO3_IG_LIMIT_CTRL                            0x2413
#define KSZ9563_PORT2_PRIO4_IG_LIMIT_CTRL                            0x2414
#define KSZ9563_PORT2_PRIO5_IG_LIMIT_CTRL                            0x2415
#define KSZ9563_PORT2_PRIO6_IG_LIMIT_CTRL                            0x2416
#define KSZ9563_PORT2_PRIO7_IG_LIMIT_CTRL                            0x2417
#define KSZ9563_PORT2_QUEUE0_EG_LIMIT_CTRL                           0x2420
#define KSZ9563_PORT2_QUEUE1_EG_LIMIT_CTRL                           0x2421
#define KSZ9563_PORT2_QUEUE2_EG_LIMIT_CTRL                           0x2422
#define KSZ9563_PORT2_QUEUE3_EG_LIMIT_CTRL                           0x2423
#define KSZ9563_PORT2_MIB_CTRL_STAT                                  0x2500
#define KSZ9563_PORT2_MIB_DATA                                       0x2504
#define KSZ9563_PORT2_ACL_ACCESS0                                    0x2600
#define KSZ9563_PORT2_ACL_ACCESS1                                    0x2601
#define KSZ9563_PORT2_ACL_ACCESS2                                    0x2602
#define KSZ9563_PORT2_ACL_ACCESS3                                    0x2603
#define KSZ9563_PORT2_ACL_ACCESS4                                    0x2604
#define KSZ9563_PORT2_ACL_ACCESS5                                    0x2605
#define KSZ9563_PORT2_ACL_ACCESS6                                    0x2606
#define KSZ9563_PORT2_ACL_ACCESS7                                    0x2607
#define KSZ9563_PORT2_ACL_ACCESS8                                    0x2608
#define KSZ9563_PORT2_ACL_ACCESS9                                    0x2609
#define KSZ9563_PORT2_ACL_ACCESS10                                   0x260A
#define KSZ9563_PORT2_ACL_ACCESS11                                   0x260B
#define KSZ9563_PORT2_ACL_ACCESS12                                   0x260C
#define KSZ9563_PORT2_ACL_ACCESS13                                   0x260D
#define KSZ9563_PORT2_ACL_ACCESS14                                   0x260E
#define KSZ9563_PORT2_ACL_ACCESS15                                   0x260F
#define KSZ9563_PORT2_ACL_BYTE_EN_MSB                                0x2610
#define KSZ9563_PORT2_ACL_BYTE_EN_LSB                                0x2611
#define KSZ9563_PORT2_ACL_ACCESS_CTRL0                               0x2612
#define KSZ9563_PORT2_MIRRORING_CTRL                                 0x2800
#define KSZ9563_PORT2_PRIO_CTRL                                      0x2801
#define KSZ9563_PORT2_IG_MAC_CTRL                                    0x2802
#define KSZ9563_PORT2_AUTH_CTRL                                      0x2803
#define KSZ9563_PORT2_PTR                                            0x2804
#define KSZ9563_PORT2_PRIO_TO_QUEUE_MAPPING                          0x2808
#define KSZ9563_PORT2_POLICE_CTRL                                    0x280C
#define KSZ9563_PORT2_TX_QUEUE_INDEX                                 0x2900
#define KSZ9563_PORT2_TX_QUEUE_PVID                                  0x2904
#define KSZ9563_PORT2_TX_QUEUE_CTRL0                                 0x2914
#define KSZ9563_PORT2_TX_QUEUE_CTRL1                                 0x2915
#define KSZ9563_PORT2_TX_CREDIT_SHAPER_CTRL0                         0x2916
#define KSZ9563_PORT2_TX_CREDIT_SHAPER_CTRL1                         0x2918
#define KSZ9563_PORT2_TX_CREDIT_SHAPER_CTRL2                         0x291A
#define KSZ9563_PORT2_CTRL0                                          0x2A00
#define KSZ9563_PORT2_CTRL1                                          0x2A04
#define KSZ9563_PORT2_CTRL2                                          0x2B00
#define KSZ9563_PORT2_MSTP_PTR                                       0x2B01
#define KSZ9563_PORT2_MSTP_STATE                                     0x2B04
#define KSZ9563_PORT2_PTP_RX_LATENCY                                 0x2C00
#define KSZ9563_PORT2_PTP_TX_LATENCY                                 0x2C02
#define KSZ9563_PORT2_PTP_ASYM_CORRECTION                            0x2C04
#define KSZ9563_PORT2_PTP_XDLY_REQ_TSH                               0x2C08
#define KSZ9563_PORT2_PTP_XDLY_REQ_TSL                               0x2C0A
#define KSZ9563_PORT2_PTP_SYNC_TSH                                   0x2C0C
#define KSZ9563_PORT2_PTP_SYNC_TSL                                   0x2C0E
#define KSZ9563_PORT2_PTP_PDLY_RESP_TSH                              0x2C10
#define KSZ9563_PORT2_PTP_PDLY_RESP_TSL                              0x2C12
#define KSZ9563_PORT2_PTP_TS_INT_STAT                                0x2C14
#define KSZ9563_PORT2_PTP_TS_INT_EN                                  0x2C16
#define KSZ9563_PORT2_PTP_LINK_DELAY                                 0x2C18
#define KSZ9563_PORT3_DEFAULT_TAG0                                   0x3000
#define KSZ9563_PORT3_DEFAULT_TAG1                                   0x3001
#define KSZ9563_PORT3_AVB_SR_CLASS1_TAG0                             0x3004
#define KSZ9563_PORT3_AVB_SR_CLASS1_TAG1                             0x3005
#define KSZ9563_PORT3_AVB_SR_CLASS2_TAG0                             0x3006
#define KSZ9563_PORT3_AVB_SR_CLASS2_TAG1                             0x3007
#define KSZ9563_PORT3_AVB_SR_CLASS1_TYPE                             0x3008
#define KSZ9563_PORT3_AVB_SR_CLASS2_TYPE                             0x300A
#define KSZ9563_PORT3_PME_WOL_EVENT                                  0x3013
#define KSZ9563_PORT3_PME_WOL_EN                                     0x3017
#define KSZ9563_PORT3_INT_STATUS                                     0x301B
#define KSZ9563_PORT3_INT_MASK                                       0x301F
#define KSZ9563_PORT3_OP_CTRL0                                       0x3020
#define KSZ9563_PORT3_OP_CTRL1                                       0x3021
#define KSZ9563_PORT3_STATUS                                         0x3030
#define KSZ9563_PORT3_XMII_CTRL0                                     0x3300
#define KSZ9563_PORT3_XMII_CTRL1                                     0x3301
#define KSZ9563_PORT3_XMII_CTRL3                                     0x3303
#define KSZ9563_PORT3_MAC_CTRL0                                      0x3400
#define KSZ9563_PORT3_MAC_CTRL1                                      0x3401
#define KSZ9563_PORT3_IG_RATE_LIMIT_CTRL                             0x3403
#define KSZ9563_PORT3_PRIO0_IG_LIMIT_CTRL                            0x3410
#define KSZ9563_PORT3_PRIO1_IG_LIMIT_CTRL                            0x3411
#define KSZ9563_PORT3_PRIO2_IG_LIMIT_CTRL                            0x3412
#define KSZ9563_PORT3_PRIO3_IG_LIMIT_CTRL                            0x3413
#define KSZ9563_PORT3_PRIO4_IG_LIMIT_CTRL                            0x3414
#define KSZ9563_PORT3_PRIO5_IG_LIMIT_CTRL                            0x3415
#define KSZ9563_PORT3_PRIO6_IG_LIMIT_CTRL                            0x3416
#define KSZ9563_PORT3_PRIO7_IG_LIMIT_CTRL                            0x3417
#define KSZ9563_PORT3_QUEUE0_EG_LIMIT_CTRL                           0x3420
#define KSZ9563_PORT3_QUEUE1_EG_LIMIT_CTRL                           0x3421
#define KSZ9563_PORT3_QUEUE2_EG_LIMIT_CTRL                           0x3422
#define KSZ9563_PORT3_QUEUE3_EG_LIMIT_CTRL                           0x3423
#define KSZ9563_PORT3_MIB_CTRL_STAT                                  0x3500
#define KSZ9563_PORT3_MIB_DATA                                       0x3504
#define KSZ9563_PORT3_ACL_ACCESS0                                    0x3600
#define KSZ9563_PORT3_ACL_ACCESS1                                    0x3601
#define KSZ9563_PORT3_ACL_ACCESS2                                    0x3602
#define KSZ9563_PORT3_ACL_ACCESS3                                    0x3603
#define KSZ9563_PORT3_ACL_ACCESS4                                    0x3604
#define KSZ9563_PORT3_ACL_ACCESS5                                    0x3605
#define KSZ9563_PORT3_ACL_ACCESS6                                    0x3606
#define KSZ9563_PORT3_ACL_ACCESS7                                    0x3607
#define KSZ9563_PORT3_ACL_ACCESS8                                    0x3608
#define KSZ9563_PORT3_ACL_ACCESS9                                    0x3609
#define KSZ9563_PORT3_ACL_ACCESS10                                   0x360A
#define KSZ9563_PORT3_ACL_ACCESS11                                   0x360B
#define KSZ9563_PORT3_ACL_ACCESS12                                   0x360C
#define KSZ9563_PORT3_ACL_ACCESS13                                   0x360D
#define KSZ9563_PORT3_ACL_ACCESS14                                   0x360E
#define KSZ9563_PORT3_ACL_ACCESS15                                   0x360F
#define KSZ9563_PORT3_ACL_BYTE_EN_MSB                                0x3610
#define KSZ9563_PORT3_ACL_BYTE_EN_LSB                                0x3611
#define KSZ9563_PORT3_ACL_ACCESS_CTRL0                               0x3612
#define KSZ9563_PORT3_MIRRORING_CTRL                                 0x3800
#define KSZ9563_PORT3_PRIO_CTRL                                      0x3801
#define KSZ9563_PORT3_IG_MAC_CTRL                                    0x3802
#define KSZ9563_PORT3_AUTH_CTRL                                      0x3803
#define KSZ9563_PORT3_PTR                                            0x3804
#define KSZ9563_PORT3_PRIO_TO_QUEUE_MAPPING                          0x3808
#define KSZ9563_PORT3_POLICE_CTRL                                    0x380C
#define KSZ9563_PORT3_TX_QUEUE_INDEX                                 0x3900
#define KSZ9563_PORT3_TX_QUEUE_PVID                                  0x3904
#define KSZ9563_PORT3_TX_QUEUE_CTRL0                                 0x3914
#define KSZ9563_PORT3_TX_QUEUE_CTRL1                                 0x3915
#define KSZ9563_PORT3_TX_CREDIT_SHAPER_CTRL0                         0x3916
#define KSZ9563_PORT3_TX_CREDIT_SHAPER_CTRL1                         0x3918
#define KSZ9563_PORT3_TX_CREDIT_SHAPER_CTRL2                         0x391A
#define KSZ9563_PORT3_CTRL0                                          0x3A00
#define KSZ9563_PORT3_CTRL1                                          0x3A04
#define KSZ9563_PORT3_CTRL2                                          0x3B00
#define KSZ9563_PORT3_MSTP_PTR                                       0x3B01
#define KSZ9563_PORT3_MSTP_STATE                                     0x3B04
#define KSZ9563_PORT3_PTP_RX_LATENCY                                 0x3C00
#define KSZ9563_PORT3_PTP_TX_LATENCY                                 0x3C02
#define KSZ9563_PORT3_PTP_ASYM_CORRECTION                            0x3C04
#define KSZ9563_PORT3_PTP_XDLY_REQ_TSH                               0x3C08
#define KSZ9563_PORT3_PTP_XDLY_REQ_TSL                               0x3C0A
#define KSZ9563_PORT3_PTP_SYNC_TSH                                   0x3C0C
#define KSZ9563_PORT3_PTP_SYNC_TSL                                   0x3C0E
#define KSZ9563_PORT3_PTP_PDLY_RESP_TSH                              0x3C10
#define KSZ9563_PORT3_PTP_PDLY_RESP_TSL                              0x3C12
#define KSZ9563_PORT3_PTP_TS_INT_STAT                                0x3C14
#define KSZ9563_PORT3_PTP_TS_INT_EN                                  0x3C16
#define KSZ9563_PORT3_PTP_LINK_DELAY                                 0x3C18

//KSZ9563 Switch register access macros
#define KSZ9563_PORTn_DEFAULT_TAG0(port)                             (0x0000 + ((port) * 0x1000))
#define KSZ9563_PORTn_DEFAULT_TAG1(port)                             (0x0001 + ((port) * 0x1000))
#define KSZ9563_PORTn_AVB_SR_CLASS1_TAG0(port)                       (0x0004 + ((port) * 0x1000))
#define KSZ9563_PORTn_AVB_SR_CLASS1_TAG1(port)                       (0x0005 + ((port) * 0x1000))
#define KSZ9563_PORTn_AVB_SR_CLASS2_TAG0(port)                       (0x0006 + ((port) * 0x1000))
#define KSZ9563_PORTn_AVB_SR_CLASS2_TAG1(port)                       (0x0007 + ((port) * 0x1000))
#define KSZ9563_PORTn_AVB_SR_CLASS1_TYPE(port)                       (0x0008 + ((port) * 0x1000))
#define KSZ9563_PORTn_AVB_SR_CLASS2_TYPE(port)                       (0x000A + ((port) * 0x1000))
#define KSZ9563_PORTn_PME_WOL_EVENT(port)                            (0x0013 + ((port) * 0x1000))
#define KSZ9563_PORTn_PME_WOL_EN(port)                               (0x0017 + ((port) * 0x1000))
#define KSZ9563_PORTn_INT_STATUS(port)                               (0x001B + ((port) * 0x1000))
#define KSZ9563_PORTn_INT_MASK(port)                                 (0x001F + ((port) * 0x1000))
#define KSZ9563_PORTn_OP_CTRL0(port)                                 (0x0020 + ((port) * 0x1000))
#define KSZ9563_PORTn_OP_CTRL1(port)                                 (0x0021 + ((port) * 0x1000))
#define KSZ9563_PORTn_STATUS(port)                                   (0x0030 + ((port) * 0x1000))
#define KSZ9563_PORTn_XMII_CTRL0(port)                               (0x0300 + ((port) * 0x1000))
#define KSZ9563_PORTn_XMII_CTRL1(port)                               (0x0301 + ((port) * 0x1000))
#define KSZ9563_PORTn_XMII_CTRL3(port)                               (0x0303 + ((port) * 0x1000))
#define KSZ9563_PORTn_MAC_CTRL0(port)                                (0x0400 + ((port) * 0x1000))
#define KSZ9563_PORTn_MAC_CTRL1(port)                                (0x0401 + ((port) * 0x1000))
#define KSZ9563_PORTn_IG_RATE_LIMIT_CTRL(port)                       (0x0403 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO0_IG_LIMIT_CTRL(port)                      (0x0410 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO1_IG_LIMIT_CTRL(port)                      (0x0411 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO2_IG_LIMIT_CTRL(port)                      (0x0412 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO3_IG_LIMIT_CTRL(port)                      (0x0413 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO4_IG_LIMIT_CTRL(port)                      (0x0414 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO5_IG_LIMIT_CTRL(port)                      (0x0415 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO6_IG_LIMIT_CTRL(port)                      (0x0416 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO7_IG_LIMIT_CTRL(port)                      (0x0417 + ((port) * 0x1000))
#define KSZ9563_PORTn_QUEUE0_EG_LIMIT_CTRL(port)                     (0x0420 + ((port) * 0x1000))
#define KSZ9563_PORTn_QUEUE1_EG_LIMIT_CTRL(port)                     (0x0421 + ((port) * 0x1000))
#define KSZ9563_PORTn_QUEUE2_EG_LIMIT_CTRL(port)                     (0x0422 + ((port) * 0x1000))
#define KSZ9563_PORTn_QUEUE3_EG_LIMIT_CTRL(port)                     (0x0423 + ((port) * 0x1000))
#define KSZ9563_PORTn_MIB_CTRL_STAT(port)                            (0x0500 + ((port) * 0x1000))
#define KSZ9563_PORTn_MIB_DATA(port)                                 (0x0504 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS0(port)                              (0x0600 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS1(port)                              (0x0601 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS2(port)                              (0x0602 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS3(port)                              (0x0603 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS4(port)                              (0x0604 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS5(port)                              (0x0605 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS6(port)                              (0x0606 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS7(port)                              (0x0607 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS8(port)                              (0x0608 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS9(port)                              (0x0609 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS10(port)                             (0x060A + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS11(port)                             (0x060B + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS12(port)                             (0x060C + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS13(port)                             (0x060D + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS14(port)                             (0x060E + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS15(port)                             (0x060F + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_BYTE_EN_MSB(port)                          (0x0610 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_BYTE_EN_LSB(port)                          (0x0611 + ((port) * 0x1000))
#define KSZ9563_PORTn_ACL_ACCESS_CTRL0(port)                         (0x0612 + ((port) * 0x1000))
#define KSZ9563_PORTn_MIRRORING_CTRL(port)                           (0x0800 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO_CTRL(port)                                (0x0801 + ((port) * 0x1000))
#define KSZ9563_PORTn_IG_MAC_CTRL(port)                              (0x0802 + ((port) * 0x1000))
#define KSZ9563_PORTn_AUTH_CTRL(port)                                (0x0803 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTR(port)                                      (0x0804 + ((port) * 0x1000))
#define KSZ9563_PORTn_PRIO_TO_QUEUE_MAPPING(port)                    (0x0808 + ((port) * 0x1000))
#define KSZ9563_PORTn_POLICE_CTRL(port)                              (0x080C + ((port) * 0x1000))
#define KSZ9563_PORTn_TX_QUEUE_INDEX(port)                           (0x0900 + ((port) * 0x1000))
#define KSZ9563_PORTn_TX_QUEUE_PVID(port)                            (0x0904 + ((port) * 0x1000))
#define KSZ9563_PORTn_TX_QUEUE_CTRL0(port)                           (0x0914 + ((port) * 0x1000))
#define KSZ9563_PORTn_TX_QUEUE_CTRL1(port)                           (0x0915 + ((port) * 0x1000))
#define KSZ9563_PORTn_TX_CREDIT_SHAPER_CTRL0(port)                   (0x0916 + ((port) * 0x1000))
#define KSZ9563_PORTn_TX_CREDIT_SHAPER_CTRL1(port)                   (0x0918 + ((port) * 0x1000))
#define KSZ9563_PORTn_TX_CREDIT_SHAPER_CTRL2(port)                   (0x091A + ((port) * 0x1000))
#define KSZ9563_PORTn_CTRL0(port)                                    (0x0A00 + ((port) * 0x1000))
#define KSZ9563_PORTn_CTRL1(port)                                    (0x0A04 + ((port) * 0x1000))
#define KSZ9563_PORTn_CTRL2(port)                                    (0x0B00 + ((port) * 0x1000))
#define KSZ9563_PORTn_MSTP_PTR(port)                                 (0x0B01 + ((port) * 0x1000))
#define KSZ9563_PORTn_MSTP_STATE(port)                               (0x0B04 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_RX_LATENCY(port)                           (0x0C00 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_TX_LATENCY(port)                           (0x0C02 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_ASYM_CORRECTION(port)                      (0x0C04 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_XDLY_REQ_TSH(port)                         (0x0C08 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_XDLY_REQ_TSL(port)                         (0x0C0A + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_SYNC_TSH(port)                             (0x0C0C + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_SYNC_TSL(port)                             (0x0C0E + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_PDLY_RESP_TSH(port)                        (0x0C10 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_PDLY_RESP_TSL(port)                        (0x0C12 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_TS_INT_STAT(port)                          (0x0C14 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_TS_INT_EN(port)                            (0x0C16 + ((port) * 0x1000))
#define KSZ9563_PORTn_PTP_LINK_DELAY(port)                           (0x0C18 + ((port) * 0x1000))
#define KSZ9563_PORTn_ETH_PHY_REG(port, addr)                        (0x0100 + ((port) * 0x1000) + ((addr) * 2))

//PHY Basic Control register
#define KSZ9563_BMCR_RESET                                           0x8000
#define KSZ9563_BMCR_LOOPBACK                                        0x4000
#define KSZ9563_BMCR_SPEED_SEL_LSB                                   0x2000
#define KSZ9563_BMCR_AN_EN                                           0x1000
#define KSZ9563_BMCR_POWER_DOWN                                      0x0800
#define KSZ9563_BMCR_ISOLATE                                         0x0400
#define KSZ9563_BMCR_RESTART_AN                                      0x0200
#define KSZ9563_BMCR_DUPLEX_MODE                                     0x0100
#define KSZ9563_BMCR_COL_TEST                                        0x0080
#define KSZ9563_BMCR_SPEED_SEL_MSB                                   0x0040

//PHY Basic Status register
#define KSZ9563_BMSR_100BT4                                          0x8000
#define KSZ9563_BMSR_100BTX_FD                                       0x4000
#define KSZ9563_BMSR_100BTX_HD                                       0x2000
#define KSZ9563_BMSR_10BT_FD                                         0x1000
#define KSZ9563_BMSR_10BT_HD                                         0x0800
#define KSZ9563_BMSR_EXTENDED_STATUS                                 0x0100
#define KSZ9563_BMSR_MF_PREAMBLE_SUPPR                               0x0040
#define KSZ9563_BMSR_AN_COMPLETE                                     0x0020
#define KSZ9563_BMSR_REMOTE_FAULT                                    0x0010
#define KSZ9563_BMSR_AN_CAPABLE                                      0x0008
#define KSZ9563_BMSR_LINK_STATUS                                     0x0004
#define KSZ9563_BMSR_JABBER_DETECT                                   0x0002
#define KSZ9563_BMSR_EXTENDED_CAPABLE                                0x0001

//PHY ID High register
#define KSZ9563_PHYID1_DEFAULT                                       0x0022

//PHY ID Low register
#define KSZ9563_PHYID2_DEFAULT                                       0x1631

//PHY Auto-Negotiation Advertisement register
#define KSZ9563_ANAR_NEXT_PAGE                                       0x8000
#define KSZ9563_ANAR_REMOTE_FAULT                                    0x2000
#define KSZ9563_ANAR_PAUSE                                           0x0C00
#define KSZ9563_ANAR_100BT4                                          0x0200
#define KSZ9563_ANAR_100BTX_FD                                       0x0100
#define KSZ9563_ANAR_100BTX_HD                                       0x0080
#define KSZ9563_ANAR_10BT_FD                                         0x0040
#define KSZ9563_ANAR_10BT_HD                                         0x0020
#define KSZ9563_ANAR_SELECTOR                                        0x001F
#define KSZ9563_ANAR_SELECTOR_DEFAULT                                0x0001

//PHY Auto-Negotiation Link Partner Ability register
#define KSZ9563_ANLPAR_NEXT_PAGE                                     0x8000
#define KSZ9563_ANLPAR_ACK                                           0x4000
#define KSZ9563_ANLPAR_REMOTE_FAULT                                  0x2000
#define KSZ9563_ANLPAR_PAUSE                                         0x0C00
#define KSZ9563_ANLPAR_100BT4                                        0x0200
#define KSZ9563_ANLPAR_100BTX_FD                                     0x0100
#define KSZ9563_ANLPAR_100BTX_HD                                     0x0080
#define KSZ9563_ANLPAR_10BT_FD                                       0x0040
#define KSZ9563_ANLPAR_10BT_HD                                       0x0020
#define KSZ9563_ANLPAR_SELECTOR                                      0x001F
#define KSZ9563_ANLPAR_SELECTOR_DEFAULT                              0x0001

//PHY Auto-Negotiation Expansion Status register
#define KSZ9563_ANER_PAR_DETECT_FAULT                                0x0010
#define KSZ9563_ANER_LP_NEXT_PAGE_ABLE                               0x0008
#define KSZ9563_ANER_NEXT_PAGE_ABLE                                  0x0004
#define KSZ9563_ANER_PAGE_RECEIVED                                   0x0002
#define KSZ9563_ANER_LP_AN_ABLE                                      0x0001

//PHY Auto-Negotiation Next Page register
#define KSZ9563_ANNPR_NEXT_PAGE                                      0x8000
#define KSZ9563_ANNPR_MSG_PAGE                                       0x2000
#define KSZ9563_ANNPR_ACK2                                           0x1000
#define KSZ9563_ANNPR_TOGGLE                                         0x0800
#define KSZ9563_ANNPR_MESSAGE                                        0x07FF

//PHY Auto-Negotiation Link Partner Next Page Ability register
#define KSZ9563_ANLPNPR_NEXT_PAGE                                    0x8000
#define KSZ9563_ANLPNPR_ACK                                          0x4000
#define KSZ9563_ANLPNPR_MSG_PAGE                                     0x2000
#define KSZ9563_ANLPNPR_ACK2                                         0x1000
#define KSZ9563_ANLPNPR_TOGGLE                                       0x0800
#define KSZ9563_ANLPNPR_MESSAGE                                      0x07FF

//PHY 1000BASE-T Control register
#define KSZ9563_GBCR_TEST_MODE                                       0xE000
#define KSZ9563_GBCR_MS_MAN_CONF_EN                                  0x1000
#define KSZ9563_GBCR_MS_MAN_CONF_VAL                                 0x0800
#define KSZ9563_GBCR_PORT_TYPE                                       0x0400
#define KSZ9563_GBCR_1000BT_FD                                       0x0200
#define KSZ9563_GBCR_1000BT_HD                                       0x0100

//PHY 1000BASE-T Status register
#define KSZ9563_GBSR_MS_CONF_FAULT                                   0x8000
#define KSZ9563_GBSR_MS_CONF_RES                                     0x4000
#define KSZ9563_GBSR_LOCAL_RECEIVER_STATUS                           0x2000
#define KSZ9563_GBSR_REMOTE_RECEIVER_STATUS                          0x1000
#define KSZ9563_GBSR_LP_1000BT_FD                                    0x0800
#define KSZ9563_GBSR_LP_1000BT_HD                                    0x0400
#define KSZ9563_GBSR_IDLE_ERR_COUNT                                  0x00FF

//PHY MMD Setup register
#define KSZ9563_MMDACR_FUNC                                          0xC000
#define KSZ9563_MMDACR_FUNC_ADDR                                     0x0000
#define KSZ9563_MMDACR_FUNC_DATA_NO_POST_INC                         0x4000
#define KSZ9563_MMDACR_FUNC_DATA_POST_INC_RW                         0x8000
#define KSZ9563_MMDACR_FUNC_DATA_POST_INC_W                          0xC000
#define KSZ9563_MMDACR_DEVAD                                         0x001F

//PHY Extended Status register
#define KSZ9563_GBESR_1000BX_FD                                      0x8000
#define KSZ9563_GBESR_1000BX_HD                                      0x4000
#define KSZ9563_GBESR_1000BT_FD                                      0x2000
#define KSZ9563_GBESR_1000BT_HD                                      0x1000

//PHY Remote Loopback register
#define KSZ9563_RLB_REMOTE_LOOPBACK                                  0x0100

//PHY LinkMD register
#define KSZ9563_LINKMD_TEST_EN                                       0x8000
#define KSZ9563_LINKMD_PAIR                                          0x3000
#define KSZ9563_LINKMD_PAIR_A                                        0x0000
#define KSZ9563_LINKMD_PAIR_B                                        0x1000
#define KSZ9563_LINKMD_PAIR_C                                        0x2000
#define KSZ9563_LINKMD_PAIR_D                                        0x3000
#define KSZ9563_LINKMD_STATUS                                        0x0300
#define KSZ9563_LINKMD_STATUS_NORMAL                                 0x0000
#define KSZ9563_LINKMD_STATUS_OPEN                                   0x0100
#define KSZ9563_LINKMD_STATUS_SHORT                                  0x0200

//PHY Digital PMA/PCS Status register
#define KSZ9563_DPMAPCSS_1000BT_LINK_STATUS                          0x0002
#define KSZ9563_DPMAPCSS_100BTX_LINK_STATUS                          0x0001

//Port Interrupt Control/Status register
#define KSZ9563_ICSR_JABBER_IE                                       0x8000
#define KSZ9563_ICSR_RECEIVE_ERROR_IE                                0x4000
#define KSZ9563_ICSR_PAGE_RECEIVED_IE                                0x2000
#define KSZ9563_ICSR_PAR_DETECT_FAULT_IE                             0x1000
#define KSZ9563_ICSR_LP_ACK_IE                                       0x0800
#define KSZ9563_ICSR_LINK_DOWN_IE                                    0x0400
#define KSZ9563_ICSR_REMOTE_FAULT_IE                                 0x0200
#define KSZ9563_ICSR_LINK_UP_IE                                      0x0100
#define KSZ9563_ICSR_JABBER_IF                                       0x0080
#define KSZ9563_ICSR_RECEIVE_ERROR_IF                                0x0040
#define KSZ9563_ICSR_PAGE_RECEIVED_IF                                0x0020
#define KSZ9563_ICSR_PAR_DETECT_FAULT_IF                             0x0010
#define KSZ9563_ICSR_LP_ACK_IF                                       0x0008
#define KSZ9563_ICSR_LINK_DOWN_IF                                    0x0004
#define KSZ9563_ICSR_REMOTE_FAULT_IF                                 0x0002
#define KSZ9563_ICSR_LINK_UP_IF                                      0x0001

//PHY Auto MDI/MDI-X register
#define KSZ9563_AUTOMDI_MDI_SET                                      0x0080
#define KSZ9563_AUTOMDI_SWAP_OFF                                     0x0040

//PHY Control register
#define KSZ9563_PHYCON_JABBER_EN                                     0x0200
#define KSZ9563_PHYCON_SPEED_1000BT                                  0x0040
#define KSZ9563_PHYCON_SPEED_100BTX                                  0x0020
#define KSZ9563_PHYCON_SPEED_10BT                                    0x0010
#define KSZ9563_PHYCON_DUPLEX_STATUS                                 0x0008
#define KSZ9563_PHYCON_1000BT_MS_STATUS                              0x0004

//MMD LED Mode register
#define KSZ9563_MMD_LED_MODE_LED_MODE                                0x0010
#define KSZ9563_MMD_LED_MODE_LED_MODE_TRI_COLOR_DUAL                 0x0000
#define KSZ9563_MMD_LED_MODE_LED_MODE_SINGLE                         0x0010
#define KSZ9563_MMD_LED_MODE_RESERVED                                0x000F
#define KSZ9563_MMD_LED_MODE_RESERVED_DEFAULT                        0x0001

//MMD EEE Advertisement register
#define KSZ9563_MMD_EEE_ADV_1000BT_EEE_EN                            0x0004
#define KSZ9563_MMD_EEE_ADV_100BT_EEE_EN                             0x0002

//Global Chip ID 0 register
#define KSZ9563_CHIP_ID0_DEFAULT                                     0x00

//Global Chip ID 1 register
#define KSZ9563_CHIP_ID1_DEFAULT                                     0x98

//Global Chip ID 2 register
#define KSZ9563_CHIP_ID2_DEFAULT                                     0x93

//Global Chip ID 3 register
#define KSZ9563_CHIP_ID3_REVISION_ID                                 0xF0
#define KSZ9563_CHIP_ID3_GLOBAL_SOFT_RESET                           0x01

//PME Pin Control register
#define KSZ9563_PME_PIN_CTRL_PME_PIN_OUT_EN                          0x02
#define KSZ9563_PME_PIN_CTRL_PME_PIN_OUT_POL                         0x01

//Global Chip ID 4 register
#define KSZ9563_CHIP_ID4_SKU_ID                                      0xFF

//Global Interrupt Status register
#define KSZ9563_GLOBAL_INT_STAT_LUE                                  0x80000000
#define KSZ9563_GLOBAL_INT_STAT_GPIO_TRIG_TS_UNIT                    0x40000000

//Global Interrupt Mask register
#define KSZ9563_GLOBAL_INT_MASK_LUE                                  0x80000000
#define KSZ9563_GLOBAL_INT_MASK_GPIO_TRIG_TS_UNIT                    0x40000000

//Global Port Interrupt Status register
#define KSZ9563_GLOBAL_PORT_INT_STAT_PORT3                           0x00000004
#define KSZ9563_GLOBAL_PORT_INT_STAT_PORT2                           0x00000002
#define KSZ9563_GLOBAL_PORT_INT_STAT_PORT1                           0x00000001

//Global Port Interrupt Mask register
#define KSZ9563_GLOBAL_PORT_INT_MASK_PORT3                           0x00000004
#define KSZ9563_GLOBAL_PORT_INT_MASK_PORT2                           0x00000002
#define KSZ9563_GLOBAL_PORT_INT_MASK_PORT1                           0x00000001

//Serial I/O Control register
#define KSZ9563_SERIAL_IO_CTRL_MIIM_PREAMBLE_SUPPR                   0x04
#define KSZ9563_SERIAL_IO_CTRL_AUTO_SPI_DATA_OUT_EDGE_SEL            0x02
#define KSZ9563_SERIAL_IO_CTRL_SPI_DATA_OUT_EDGE_SEL                 0x01
#define KSZ9563_SERIAL_IO_CTRL_SPI_DATA_OUT_EDGE_SEL_FALLING         0x00
#define KSZ9563_SERIAL_IO_CTRL_SPI_DATA_OUT_EDGE_SEL_RISING          0x01

//In-Band Management Control register
#define KSZ9563_IBA_CTRL_IBA_EN                                      0x80000000
#define KSZ9563_IBA_CTRL_DEST_MAC_ADDR_MATCH_EN                      0x40000000
#define KSZ9563_IBA_CTRL_IBA_RESET                                   0x20000000
#define KSZ9563_IBA_CTRL_RESP_PRIO_QUEUE                             0x00C00000
#define KSZ9563_IBA_CTRL_RESP_PRIO_QUEUE_DEFAULT                     0x00400000
#define KSZ9563_IBA_CTRL_IBA_COMM                                    0x00070000
#define KSZ9563_IBA_CTRL_IBA_COMM_PORT1                              0x00000000
#define KSZ9563_IBA_CTRL_IBA_COMM_PORT2                              0x00010000
#define KSZ9563_IBA_CTRL_IBA_COMM_PORT3                              0x00020000
#define KSZ9563_IBA_CTRL_TPID                                        0x0000FFFF
#define KSZ9563_IBA_CTRL_TPID_DEFAULT                                0x000040FE

//I/O Drive Strength register
#define KSZ9563_IO_DRIVE_STRENGTH_HIGH_SPEED_DRIVE_STRENGTH          0x70
#define KSZ9563_IO_DRIVE_STRENGTH_LOW_SPEED_DRIVE_STRENGTH           0x07

//In-Band Management Operation Status 1 register
#define KSZ9563_IBA_OP_STAT1_GOOD_PKT_DETECT                         0x80000000
#define KSZ9563_IBA_OP_STAT1_RESP_PKT_TX_DONE                        0x40000000
#define KSZ9563_IBA_OP_STAT1_EXEC_DONE                               0x20000000
#define KSZ9563_IBA_OP_STAT1_MAC_ADDR_MISMATCH_ERR                   0x00004000
#define KSZ9563_IBA_OP_STAT1_ACCESS_FORMAT_ERR                       0x00002000
#define KSZ9563_IBA_OP_STAT1_ACCESS_CODE_ERR                         0x00001000
#define KSZ9563_IBA_OP_STAT1_ACCESS_CMD_ERR                          0x00000800
#define KSZ9563_IBA_OP_STAT1_OVERSIZE_PKT_ERR                        0x00000400
#define KSZ9563_IBA_OP_STAT1_ACCESS_CODE_ERR_LOC                     0x0000007F

//LED Override register
#define KSZ9563_LED_OVERRIDE_OVERRIDE                                0x0000000F
#define KSZ9563_LED_OVERRIDE_OVERRIDE_LED1_0                         0x00000001
#define KSZ9563_LED_OVERRIDE_OVERRIDE_LED1_1                         0x00000002
#define KSZ9563_LED_OVERRIDE_OVERRIDE_LED2_0                         0x00000004
#define KSZ9563_LED_OVERRIDE_OVERRIDE_LED2_1                         0x00000008

//LED Output register
#define KSZ9563_LED_OUTPUT_GPIO_OUT_CTRL                             0x0000000F
#define KSZ9563_LED_OUTPUT_GPIO_OUT_CTRL_LED1_0                      0x00000001
#define KSZ9563_LED_OUTPUT_GPIO_OUT_CTRL_LED1_1                      0x00000002
#define KSZ9563_LED_OUTPUT_GPIO_OUT_CTRL_LED2_0                      0x00000004
#define KSZ9563_LED_OUTPUT_GPIO_OUT_CTRL_LED2_1                      0x00000008

//LED2_0/LED2_1 Source register
#define KSZ9563_LED2_0_LED2_1_SRC_LED2_1_SRC                         0x00000008
#define KSZ9563_LED2_0_LED2_1_SRC_LED2_0_SRC                         0x00000004

//Power Down Control 0 register
#define KSZ9563_PWR_DOWN_CTRL0_PLL_PWR_DOWN                          0x20
#define KSZ9563_PWR_DOWN_CTRL0_PWR_MGMT_MODE                         0x18
#define KSZ9563_PWR_DOWN_CTRL0_PWR_MGMT_MODE_NORMAL                  0x00
#define KSZ9563_PWR_DOWN_CTRL0_PWR_MGMT_MODE_EDPD                    0x08
#define KSZ9563_PWR_DOWN_CTRL0_PWR_MGMT_MODE_SOFT_PWR_DOWN           0x10

//LED Strap-In register
#define KSZ9563_LED_STRAP_IN_STRAP_IN                                0x0000000F
#define KSZ9563_LED_STRAP_IN_STRAP_IN_LED1_0                         0x00000001
#define KSZ9563_LED_STRAP_IN_STRAP_IN_LED1_1                         0x00000002
#define KSZ9563_LED_STRAP_IN_STRAP_IN_RXD2_0                         0x00000004
#define KSZ9563_LED_STRAP_IN_STRAP_IN_RXD2_1                         0x00000008

//Switch Operation register
#define KSZ9563_SWITCH_OP_DOUBLE_TAG_EN                              0x80
#define KSZ9563_SWITCH_OP_SOFT_HARD_RESET                            0x02
#define KSZ9563_SWITCH_OP_START_SWITCH                               0x01

//Switch Maximum Transmit Unit register
#define KSZ9563_SWITCH_MTU_MTU                                       0x3FFF
#define KSZ9563_SWITCH_MTU_MTU_DEFAULT                               0x07D0

//Switch ISP TPID register
#define KSZ9563_SWITCH_ISP_TPID_ISP_TAG_TPID                         0xFFFF

//AVB Credit Based Shaper Strategy register
#define KSZ9563_AVB_CBS_STRATEGY_SHAPING_CREDIT_ACCOUNTING           0x0002
#define KSZ9563_AVB_CBS_STRATEGY_POLICING_CREDIT_ACCOUNTING          0x0001

//Switch Lookup Engine Control 0 register
#define KSZ9563_SWITCH_LUE_CTRL0_VLAN_EN                             0x80
#define KSZ9563_SWITCH_LUE_CTRL0_DROP_INVALID_VID                    0x40
#define KSZ9563_SWITCH_LUE_CTRL0_AGE_COUNT                           0x38
#define KSZ9563_SWITCH_LUE_CTRL0_AGE_COUNT_DEFAULT                   0x20
#define KSZ9563_SWITCH_LUE_CTRL0_RESERVED_MCAST_LOOKUP_EN            0x04
#define KSZ9563_SWITCH_LUE_CTRL0_HASH_OPTION                         0x03
#define KSZ9563_SWITCH_LUE_CTRL0_HASH_OPTION_NONE                    0x00
#define KSZ9563_SWITCH_LUE_CTRL0_HASH_OPTION_CRC                     0x01
#define KSZ9563_SWITCH_LUE_CTRL0_HASH_OPTION_XOR                     0x02

//Switch Lookup Engine Control 1 register
#define KSZ9563_SWITCH_LUE_CTRL1_UNICAST_LEARNING_DIS                0x80
#define KSZ9563_SWITCH_LUE_CTRL1_SELF_ADDR_FILT                      0x40
#define KSZ9563_SWITCH_LUE_CTRL1_FLUSH_ALU_TABLE                     0x20
#define KSZ9563_SWITCH_LUE_CTRL1_FLUSH_MSTP_ENTRIES                  0x10
#define KSZ9563_SWITCH_LUE_CTRL1_MCAST_SRC_ADDR_FILT                 0x08
#define KSZ9563_SWITCH_LUE_CTRL1_AGING_EN                            0x04
#define KSZ9563_SWITCH_LUE_CTRL1_FAST_AGING                          0x02
#define KSZ9563_SWITCH_LUE_CTRL1_LINK_DOWN_FLUSH                     0x01

//Switch Lookup Engine Control 2 register
#define KSZ9563_SWITCH_LUE_CTRL2_DOUBLE_TAG_MCAST_TRAP               0x40
#define KSZ9563_SWITCH_LUE_CTRL2_DYNAMIC_ENTRY_EG_VLAN_FILT          0x20
#define KSZ9563_SWITCH_LUE_CTRL2_STATIC_ENTRY_EG_VLAN_FILT           0x10
#define KSZ9563_SWITCH_LUE_CTRL2_FLUSH_OPTION                        0x0C
#define KSZ9563_SWITCH_LUE_CTRL2_FLUSH_OPTION_NONE                   0x00
#define KSZ9563_SWITCH_LUE_CTRL2_FLUSH_OPTION_DYNAMIC                0x04
#define KSZ9563_SWITCH_LUE_CTRL2_FLUSH_OPTION_STATIC                 0x08
#define KSZ9563_SWITCH_LUE_CTRL2_FLUSH_OPTION_BOTH                   0x0C
#define KSZ9563_SWITCH_LUE_CTRL2_MAC_ADDR_PRIORITY                   0x03

//Switch Lookup Engine Control 3 register
#define KSZ9563_SWITCH_LUE_CTRL3_AGE_PERIOD                          0xFF
#define KSZ9563_SWITCH_LUE_CTRL3_AGE_PERIOD_DEFAULT                  0x4B

//Address Lookup Table Interrupt register
#define KSZ9563_ALU_TABLE_INT_LEARN_FAIL                             0x04
#define KSZ9563_ALU_TABLE_INT_ALMOST_FULL                            0x02
#define KSZ9563_ALU_TABLE_INT_WRITE_FAIL                             0x01

//Address Lookup Table Mask register
#define KSZ9563_ALU_TABLE_MASK_LEARN_FAIL                            0x04
#define KSZ9563_ALU_TABLE_MASK_ALMOST_FULL                           0x02
#define KSZ9563_ALU_TABLE_MASK_WRITE_FAIL                            0x01

//Address Lookup Table Entry Index 0 register
#define KSZ9563_ALU_TABLE_ENTRY_INDEX0_ALMOST_FULL_ENTRY_INDEX       0x0FFF
#define KSZ9563_ALU_TABLE_ENTRY_INDEX0_FAIL_WRITE_INDEX              0x03FF

//Address Lookup Table Entry Index 1 register
#define KSZ9563_ALU_TABLE_ENTRY_INDEX1_FAIL_LEARN_INDEX              0x03FF

//Address Lookup Table Entry Index 2 register
#define KSZ9563_ALU_TABLE_ENTRY_INDEX2_CPU_ACCESS_INDEX              0x03FF

//Unknown Unicast Control register
#define KSZ9563_UNKNOWN_UNICAST_CTRL_FWD                             0x80000000
#define KSZ9563_UNKNOWN_UNICAST_CTRL_FWD_MAP                         0x00000007
#define KSZ9563_UNKNOWN_UNICAST_CTRL_FWD_MAP_PORT1                   0x00000001
#define KSZ9563_UNKNOWN_UNICAST_CTRL_FWD_MAP_PORT2                   0x00000002
#define KSZ9563_UNKNOWN_UNICAST_CTRL_FWD_MAP_PORT3                   0x00000004
#define KSZ9563_UNKNOWN_UNICAST_CTRL_FWD_MAP_ALL                     0x00000007

//Unknown Multicast Control register
#define KSZ9563_UNKONWN_MULTICAST_CTRL_FWD                           0x80000000
#define KSZ9563_UNKONWN_MULTICAST_CTRL_FWD_MAP                       0x00000007
#define KSZ9563_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT1                 0x00000001
#define KSZ9563_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT2                 0x00000002
#define KSZ9563_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT3                 0x00000004
#define KSZ9563_UNKONWN_MULTICAST_CTRL_FWD_MAP_ALL                   0x00000007

//Unknown VLAN ID Control register
#define KSZ9563_UNKNOWN_VLAN_ID_CTRL_FWD                             0x80000000
#define KSZ9563_UNKNOWN_VLAN_ID_CTRL_FWD_MAP                         0x00000007
#define KSZ9563_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_PORT1                   0x00000001
#define KSZ9563_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_PORT2                   0x00000002
#define KSZ9563_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_PORT3                   0x00000004
#define KSZ9563_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_ALL                     0x00000007

//Switch MAC Control 0 register
#define KSZ9563_SWITCH_MAC_CTRL0_ALT_BACK_OFF_MODE                   0x80
#define KSZ9563_SWITCH_MAC_CTRL0_FRAME_LEN_CHECK_EN                  0x08
#define KSZ9563_SWITCH_MAC_CTRL0_FLOW_CTRL_PKT_DROP_MODE             0x02
#define KSZ9563_SWITCH_MAC_CTRL0_AGGRESSIVE_BACK_OFF_EN              0x01

//Switch MAC Control 1 register
#define KSZ9563_SWITCH_MAC_CTRL1_MCAST_STORM_PROTECT_DIS             0x40
#define KSZ9563_SWITCH_MAC_CTRL1_BACK_PRESSURE_MODE                  0x20
#define KSZ9563_SWITCH_MAC_CTRL1_FLOW_CTRL_FAIR_MODE                 0x10
#define KSZ9563_SWITCH_MAC_CTRL1_NO_EXCESSIVE_COL_DROP               0x08
#define KSZ9563_SWITCH_MAC_CTRL1_JUMBO_PKT_SUPPORT                   0x04
#define KSZ9563_SWITCH_MAC_CTRL1_MAX_PKT_SIZE_CHECK_DIS              0x02
#define KSZ9563_SWITCH_MAC_CTRL1_PASS_SHORT_PKT                      0x01

//Switch MAC Control 2 register
#define KSZ9563_SWITCH_MAC_CTRL2_NULL_VID_REPLACEMENT                0x08
#define KSZ9563_SWITCH_MAC_CTRL2_BCAST_STORM_PROTECT_RATE_MSB        0x07

//Switch MAC Control 3 register
#define KSZ9563_SWITCH_MAC_CTRL3_BCAST_STORM_PROTECT_RATE_LSB        0xFF

//Switch MAC Control 4 register
#define KSZ9563_SWITCH_MAC_CTRL4_PASS_FLOW_CTRL_PKT                  0x01

//Switch MAC Control 5 register
#define KSZ9563_SWITCH_MAC_CTRL5_IG_RATE_LIMIT_PERIOD                0x30
#define KSZ9563_SWITCH_MAC_CTRL5_IG_RATE_LIMIT_PERIOD_16MS           0x00
#define KSZ9563_SWITCH_MAC_CTRL5_IG_RATE_LIMIT_PERIOD_64MS           0x10
#define KSZ9563_SWITCH_MAC_CTRL5_IG_RATE_LIMIT_PERIOD_256MS          0x20
#define KSZ9563_SWITCH_MAC_CTRL5_QUEUE_BASED_EG_RATE_LIMITE_EN       0x08

//Switch MIB Control register
#define KSZ9563_SWITCH_MIB_CTRL_FLUSH                                0x80
#define KSZ9563_SWITCH_MIB_CTRL_FREEZE                               0x40

//Global Port Mirroring and Snooping Control register
#define KSZ9563_GLOBAL_PORT_MIRROR_SNOOP_CTRL_IGMP_SNOOP_EN          0x40
#define KSZ9563_GLOBAL_PORT_MIRROR_SNOOP_CTRL_MLD_SNOOP_OPT          0x08
#define KSZ9563_GLOBAL_PORT_MIRROR_SNOOP_CTRL_MLD_SNOOP_EN           0x04
#define KSZ9563_GLOBAL_PORT_MIRROR_SNOOP_CTRL_SNIFF_MODE_SEL         0x01

//WRED DiffServ Color Mapping register
#define KSZ9563_WRED_DIFFSERV_COLOR_MAPPING_RED                      0x30
#define KSZ9563_WRED_DIFFSERV_COLOR_MAPPING_YELLOW                   0x0C
#define KSZ9563_WRED_DIFFSERV_COLOR_MAPPING_GREEN                    0x03

//PTP Event Message Priority register
#define KSZ9563_PTP_EVENT_MSG_PRIO_OVERRIDE                          0x80
#define KSZ9563_PTP_EVENT_MSG_PRIO_PRIORITY                          0x0F

//PTP Non-Event Message Priority register
#define KSZ9563_PTP_NON_EVENT_MSG_PRIO_OVERRIDE                      0x80
#define KSZ9563_PTP_NON_EVENT_MSG_PRIO_PRIORITY                      0x0F

//Queue Management Control 0 register
#define KSZ9563_QUEUE_MGMT_CTRL0_PRIORITY_2Q                         0x000000C0
#define KSZ9563_QUEUE_MGMT_CTRL0_UNICAST_PORT_VLAN_DISCARD           0x00000002

//VLAN Table Entry 0 register
#define KSZ9563_VLAN_TABLE_ENTRY0_VALID                              0x80000000
#define KSZ9563_VLAN_TABLE_ENTRY0_FORWARD_OPTION                     0x08000000
#define KSZ9563_VLAN_TABLE_ENTRY0_PRIORITY                           0x07000000
#define KSZ9563_VLAN_TABLE_ENTRY0_MSTP_INDEX                         0x00007000
#define KSZ9563_VLAN_TABLE_ENTRY0_FID                                0x0000007F

//VLAN Table Entry 1 register
#define KSZ9563_VLAN_TABLE_ENTRY1_PORT_UNTAG                         0x00000007
#define KSZ9563_VLAN_TABLE_ENTRY1_PORT3_UNTAG                        0x00000004
#define KSZ9563_VLAN_TABLE_ENTRY1_PORT2_UNTAG                        0x00000002
#define KSZ9563_VLAN_TABLE_ENTRY1_PORT1_UNTAG                        0x00000001

//VLAN Table Entry 2 register
#define KSZ9563_VLAN_TABLE_ENTRY2_PORT_FORWARD                       0x00000007
#define KSZ9563_VLAN_TABLE_ENTRY2_PORT3_FORWARD                      0x00000004
#define KSZ9563_VLAN_TABLE_ENTRY2_PORT2_FORWARD                      0x00000002
#define KSZ9563_VLAN_TABLE_ENTRY2_PORT1_FORWARD                      0x00000001

//VLAN Table Index register
#define KSZ9563_VLAN_TABLE_INDEX_VLAN_INDEX                          0x0FFF

//VLAN Table Access Control register
#define KSZ9563_VLAN_TABLE_ACCESS_CTRL_START_FINISH                  0x80
#define KSZ9563_VLAN_TABLE_ACCESS_CTRL_ACTION                        0x03
#define KSZ9563_VLAN_TABLE_ACCESS_CTRL_ACTION_NOP                    0x00
#define KSZ9563_VLAN_TABLE_ACCESS_CTRL_ACTION_WRITE                  0x01
#define KSZ9563_VLAN_TABLE_ACCESS_CTRL_ACTION_READ                   0x02
#define KSZ9563_VLAN_TABLE_ACCESS_CTRL_ACTION_CLEAR                  0x03

//ALU Table Index 0 register
#define KSZ9563_ALU_TABLE_INDEX0_FID_INDEX                           0x007F0000
#define KSZ9563_ALU_TABLE_INDEX0_MAC_INDEX_MSB                       0x0000FFFF

//ALU Table Index 1 register
#define KSZ9563_ALU_TABLE_INDEX1_MAC_INDEX_LSB                       0xFFFFFFFF

//ALU Table Access Control register
#define KSZ9563_ALU_TABLE_CTRL_VALID_COUNT                           0x3FFF0000
#define KSZ9563_ALU_TABLE_CTRL_START_FINISH                          0x00000080
#define KSZ9563_ALU_TABLE_CTRL_VALID                                 0x00000040
#define KSZ9563_ALU_TABLE_CTRL_VALID_ENTRY_OR_SEARCH_END             0x00000020
#define KSZ9563_ALU_TABLE_CTRL_DIRECT                                0x00000004
#define KSZ9563_ALU_TABLE_CTRL_ACTION                                0x00000003
#define KSZ9563_ALU_TABLE_CTRL_ACTION_NOP                            0x00000000
#define KSZ9563_ALU_TABLE_CTRL_ACTION_WRITE                          0x00000001
#define KSZ9563_ALU_TABLE_CTRL_ACTION_READ                           0x00000002
#define KSZ9563_ALU_TABLE_CTRL_ACTION_SEARCH                         0x00000003

//Static Address and Reserved Multicast Table Control register
#define KSZ9563_STATIC_MCAST_TABLE_CTRL_TABLE_INDEX                  0x003F0000
#define KSZ9563_STATIC_MCAST_TABLE_CTRL_START_FINISH                 0x00000080
#define KSZ9563_STATIC_MCAST_TABLE_CTRL_TABLE_SELECT                 0x00000002
#define KSZ9563_STATIC_MCAST_TABLE_CTRL_ACTION                       0x00000001
#define KSZ9563_STATIC_MCAST_TABLE_CTRL_ACTION_READ                  0x00000000
#define KSZ9563_STATIC_MCAST_TABLE_CTRL_ACTION_WRITE                 0x00000001

//ALU Table Entry 1 register
#define KSZ9563_ALU_TABLE_ENTRY1_STATIC                              0x80000000
#define KSZ9563_ALU_TABLE_ENTRY1_SRC_FILTER                          0x40000000
#define KSZ9563_ALU_TABLE_ENTRY1_DES_FILTER                          0x20000000
#define KSZ9563_ALU_TABLE_ENTRY1_PRIORITY                            0x1C000000
#define KSZ9563_ALU_TABLE_ENTRY1_AGE_COUNT                           0x1C000000
#define KSZ9563_ALU_TABLE_ENTRY1_MSTP                                0x00000007

//ALU Table Entry 2 register
#define KSZ9563_ALU_TABLE_ENTRY2_OVERRIDE                            0x80000000
#define KSZ9563_ALU_TABLE_ENTRY2_PORT_FORWARD                        0x00000007
#define KSZ9563_ALU_TABLE_ENTRY2_PORT3_FORWARD                       0x00000004
#define KSZ9563_ALU_TABLE_ENTRY2_PORT2_FORWARD                       0x00000002
#define KSZ9563_ALU_TABLE_ENTRY2_PORT1_FORWARD                       0x00000001

//ALU Table Entry 3 register
#define KSZ9563_ALU_TABLE_ENTRY3_FID                                 0x007F0000
#define KSZ9563_ALU_TABLE_ENTRY3_MAC_ADDR_MSB                        0x0000FFFF

//ALU Table Entry 4 register
#define KSZ9563_ALU_TABLE_ENTRY4_MAC_ADDR_LSB                        0xFFFFFFFF

//Static Address Table Entry 1 register
#define KSZ9563_STATIC_TABLE_ENTRY1_VALID                            0x80000000
#define KSZ9563_STATIC_TABLE_ENTRY1_SRC_FILTER                       0x40000000
#define KSZ9563_STATIC_TABLE_ENTRY1_DES_FILTER                       0x20000000
#define KSZ9563_STATIC_TABLE_ENTRY1_PRIORITY                         0x1C000000
#define KSZ9563_STATIC_TABLE_ENTRY1_MSTP                             0x00000007

//Static Address Table Entry 2 register
#define KSZ9563_STATIC_TABLE_ENTRY2_OVERRIDE                         0x80000000
#define KSZ9563_STATIC_TABLE_ENTRY2_USE_FID                          0x40000000
#define KSZ9563_STATIC_TABLE_ENTRY2_PORT_FORWARD                     0x00000007
#define KSZ9563_STATIC_TABLE_ENTRY2_PORT3_FORWARD                    0x00000004
#define KSZ9563_STATIC_TABLE_ENTRY2_PORT2_FORWARD                    0x00000002
#define KSZ9563_STATIC_TABLE_ENTRY2_PORT1_FORWARD                    0x00000001

//Static Address Table Entry 3 register
#define KSZ9563_STATIC_TABLE_ENTRY3_FID                              0x007F0000
#define KSZ9563_STATIC_TABLE_ENTRY3_MAC_ADDR_MSB                     0x0000FFFF

//Static Address Table Entry 4 register
#define KSZ9563_STATIC_TABLE_ENTRY4_MAC_ADDR_LSB                     0xFFFFFFFF

//Reserved Multicast Table Entry 2 register
#define KSZ9563_RES_MCAST_TABLE_ENTRY2_PORT_FORWARD                  0x00000007
#define KSZ9563_RES_MCAST_TABLE_ENTRY2_PORT3_FORWARD                 0x00000004
#define KSZ9563_RES_MCAST_TABLE_ENTRY2_PORT2_FORWARD                 0x00000002
#define KSZ9563_RES_MCAST_TABLE_ENTRY2_PORT1_FORWARD                 0x00000001

//Global PTP Clock Control register
#define KSZ9563_GLOBAL_PTP_CLK_CTRL_SW_FREQ_ADJ_DIS                  0x8000
#define KSZ9563_GLOBAL_PTP_CLK_CTRL_PTP_CLK_STEP_ADJ                 0x0040
#define KSZ9563_GLOBAL_PTP_CLK_CTRL_PTP_STEP_DIR                     0x0020
#define KSZ9563_GLOBAL_PTP_CLK_CTRL_PTP_CLK_READ                     0x0010
#define KSZ9563_GLOBAL_PTP_CLK_CTRL_PTP_CLK_LOAD                     0x0008
#define KSZ9563_GLOBAL_PTP_CLK_CTRL_PTP_CLK_CONTINUOUS_ADJ           0x0004
#define KSZ9563_GLOBAL_PTP_CLK_CTRL_PTP_CLK_EN                       0x0002
#define KSZ9563_GLOBAL_PTP_CLK_CTRL_PTP_CLK_RESET                    0x0001

//Global PTP RTC Clock Phase register
#define KSZ9563_GLOBAL_PTP_RTC_CLK_PHASE_PTP_RTC_8NS_PHASE           0x0007

//Global PTP Clock Sub-Nanosecond Rate High Word register
#define KSZ9563_GLOBAL_PTP_CLK_SUB_NS_RATE_H_PTP_RATE_DIR            0x8000
#define KSZ9563_GLOBAL_PTP_CLK_SUB_NS_RATE_H_PTP_TEMP_ADJ_MODE       0x4000
#define KSZ9563_GLOBAL_PTP_CLK_SUB_NS_RATE_H_PTP_RTC_SUB_NS_29_16    0x3FFF

//Global PTP Clock Sub-Nanosecond Rate Low Word register
#define KSZ9563_GLOBAL_PTP_CLK_SUB_NS_RATE_L_PTP_RTC_SUB_NS_15_0     0xFFFF

//Global PTP Message Config 1 register
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_IEEE_1588_PTP_MODE            0x0040
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_ETH_PTP_DETECT                0x0020
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_IPV4_UDP_PTP_DETECT           0x0010
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_IPV6_UDP_PTP_DETECT           0x0008
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_E2E_CLK_MODE                  0x0000
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_P2P_CLK_MODE                  0x0004
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_SLAVE_OC_CLK_MODE             0x0000
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_MASTER_OC_CLK_MODE            0x0002
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_TWO_STEP_CLK_MODE             0x0000
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG1_ONE_STEP_CLK_MODE             0x0001

//Global PTP Message Config 2 register
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2_UNICAST_PTP_EN                0x1000
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2_ALT_MASTER_EN                 0x0800
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2_PTP_MSG_PRIO_TX_QUEUE         0x0400
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2_CHECK_SYNC_FOLLOW_UP          0x0200
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2_CHECK_DELAY_REQ_RESP          0x0100
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2_CHECK_PDELAY_REQ_RESP         0x0080
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2_DROP_SYNC_FOLLOW_UP_DELAY_REQ 0x0020
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2_CHECK_DOMAIN                  0x0010
#define KSZ9563_GLOBAL_PTP_MSG_CONFIG2_IPV4_UDP_CHECKSUM_EN          0x0004

//Global PTP Domain and Version register
#define KSZ9563_GLOBAL_PTP_DOMAIN_VERSION_PTP_VERSION                0x0F00
#define KSZ9563_GLOBAL_PTP_DOMAIN_VERSION_PTP_DOMAIN                 0x00FF

//Global PTP Unit Index register
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_GPIO_PTR_INDEX                 0x00010000
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_GPIO_PTR_INDEX_GPIO1           0x00000000
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_GPIO_PTR_INDEX_GPIO2           0x00010000
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_TS_PTR_INDEX                   0x00000100
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_TS_PTR_INDEX_UNIT0             0x00000000
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_TS_PTR_INDEX_UNIT1             0x00000100
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_TRIGGER_PTR_INDEX              0x00000003
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_TRIGGER_PTR_INDEX_UNIT0        0x00000000
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_TRIGGER_PTR_INDEX_UNIT1        0x00000001
#define KSZ9563_GLOBAL_PTP_UNIT_INDEX_TRIGGER_PTR_INDEX_UNIT2        0x00000002

//GPIO Status Monitor 0 register
#define KSZ9563_GPIO_STATUS_MONITOR0_TRIGGER_ERROR                   0x00070000
#define KSZ9563_GPIO_STATUS_MONITOR0_TRIGGER_DONE                    0x00000007

//GPIO Status Monitor 1 register
#define KSZ9563_GPIO_STATUS_MONITOR1_TRIGGER_INT_STATUS              0x00070000
#define KSZ9563_GPIO_STATUS_MONITOR1_TS_INT_STATUS                   0x00000003

//Timestamp Control and Status register
#define KSZ9563_TS_CTRL_STAT_GPIO_OUT_SEL                            0x00000100
#define KSZ9563_TS_CTRL_STAT_GPIO_IN                                 0x00000080
#define KSZ9563_TS_CTRL_STAT_GPIO_OEN                                0x00000040
#define KSZ9563_TS_CTRL_STAT_TS_INT_ENB                              0x00000020
#define KSZ9563_TS_CTRL_STAT_TRIGGER_ACTIVE                          0x00000010
#define KSZ9563_TS_CTRL_STAT_TRIGGER_EN                              0x00000008
#define KSZ9563_TS_CTRL_STAT_TRIGGER_SW_RESET                        0x00000004
#define KSZ9563_TS_CTRL_STAT_TS_ENB                                  0x00000002
#define KSZ9563_TS_CTRL_STAT_TS_SW_RESET                             0x00000001

//Trigger Output Unit Target Time Nanosecond register
#define KSZ9563_TOU_TARGET_TIME_NS_TRIGGER_TARGET_TIME_NS            0x3FFFFFFF

//Trigger Output Unit Target Time Second register
#define KSZ9563_TOU_TARGET_TIME_S_TRIGGER_TARGET_TIME_S              0xFFFFFFFF

//Trigger Output Unit Control 1 register
#define KSZ9563_TOU_CTRL1_CASCADE_MODE_ENB                           0x80000000
#define KSZ9563_TOU_CTRL1_CASCADE_MODE_TAIL                          0x40000000
#define KSZ9563_TOU_CTRL1_CASCADE_MODE_DONE                          0x0C000000
#define KSZ9563_TOU_CTRL1_TRIGGER_NOW                                0x02000000
#define KSZ9563_TOU_CTRL1_TRIGGER_NOTIFY                             0x01000000
#define KSZ9563_TOU_CTRL1_TRIGGER_EDGE                               0x00800000
#define KSZ9563_TOU_CTRL1_TRIGGER_PATTERN                            0x00700000
#define KSZ9563_TOU_CTRL1_TRIGGER_PATTERN_NEG_EDGE                   0x00000000
#define KSZ9563_TOU_CTRL1_TRIGGER_PATTERN_POS_EDGE                   0x00100000
#define KSZ9563_TOU_CTRL1_TRIGGER_PATTERN_NEG_PULSE                  0x00200000
#define KSZ9563_TOU_CTRL1_TRIGGER_PATTERN_POS_PULSE                  0x00300000
#define KSZ9563_TOU_CTRL1_TRIGGER_PATTERN_NEG_CYCLE                  0x00400000
#define KSZ9563_TOU_CTRL1_TRIGGER_PATTERN_POS_CYCLE                  0x00500000
#define KSZ9563_TOU_CTRL1_TRIGGER_PATTERN_REG_OUTPUT                 0x00600000
#define KSZ9563_TOU_CTRL1_TRIGGER_GPIO                               0x00010000
#define KSZ9563_TOU_CTRL1_TRIGGER_GPIO_1                             0x00000000
#define KSZ9563_TOU_CTRL1_TRIGGER_GPIO_2                             0x00010000
#define KSZ9563_TOU_CTRL1_TRIGGER_PATTERN_ITERATION                  0x0000FFFF

//Trigger Output Unit Control 2 register
#define KSZ9563_TOU_CTRL2_TRIGGER_CYCLE_WIDTH                        0xFFFFFFFF

//Trigger Output Unit Control 3 register
#define KSZ9563_TOU_CTRL3_TRIGGER_CYCLE                              0xFFFF0000
#define KSZ9563_TOU_CTRL3_TRIGGER_BIT_PATTERN                        0x0000FFFF

//Trigger Output Unit Control 4 register
#define KSZ9563_TOU_CTRL4_CASCADE_INTERATION_CYCLE_TIME              0xFFFFFFFF

//Trigger Output Unit Control 5 register
#define KSZ9563_TOU_CTRL5_PPS_PULSE_WIDTH                            0x00FF0000
#define KSZ9563_TOU_CTRL5_TRIGGER_PULSE_WIDTH                        0x0000FFFF

//Timestamp Status and Control register
#define KSZ9563_TS_STAT_CTRL_TS_EVENT_DET_CNT                        0x001E0000
#define KSZ9563_TS_STAT_CTRL_TS_DET_EVENT_CNT_OVERFLOW               0x00010000
#define KSZ9563_TS_STAT_CTRL_TS_RISING_EDGE_ENB                      0x00000080
#define KSZ9563_TS_STAT_CTRL_TS_FALLING_EDGE_ENB                     0x00000040
#define KSZ9563_TS_STAT_CTRL_TS_CASCADE_MODE_TAIL                    0x00000020
#define KSZ9563_TS_STAT_CTRL_TS_UPSTREAM_CASCADE_MODE_SEL            0x00000002
#define KSZ9563_TS_STAT_CTRL_TS_CASCADE_MODE_ENB                     0x00000001

//Timestamp 1st Sample Time Nanoseconds register
#define KSZ9563_TS_SAMPLE1_TIME_NS_TS_SAMPLE_EDGE_1ST                0x40000000
#define KSZ9563_TS_SAMPLE1_TIME_NS_TS_SAMPLE_TIME_NS_1ST             0x3FFFFFFF

//Timestamp 1st Sample Time Seconds register
#define KSZ9563_TS_SAMPLE1_TIME_S_TS_SAMPLE_TIME_S_1ST               0xFFFFFFFF

//Timestamp 1st Sample Time Phase register
#define KSZ9563_TS_SAMPLE1_TIME_PHASE_TS_SAMPLE_TIME_SUB_8NS_1ST     0x00000007

//Timestamp 2nd Sample Time Nanoseconds register
#define KSZ9563_TS_SAMPLE2_TIME_NS_TS_SAMPLE_EDGE_2ND                0x40000000
#define KSZ9563_TS_SAMPLE2_TIME_NS_TS_SAMPLE_TIME_NS_2ND             0x3FFFFFFF

//Timestamp 2nd Sample Time Seconds register
#define KSZ9563_TS_SAMPLE2_TIME_S_TS_SAMPLE_TIME_S_2ND               0xFFFFFFFF

//Timestamp 2nd Sample Time Phase register
#define KSZ9563_TS_SAMPLE2_TIME_PHASE_TS_SAMPLE_TIME_SUB_8NS_2ND     0x00000007

//Timestamp 3rd Sample Time Nanoseconds register
#define KSZ9563_TS_SAMPLE3_TIME_NS_TS_SAMPLE_EDGE_3RD                0x40000000
#define KSZ9563_TS_SAMPLE3_TIME_NS_TS_SAMPLE_TIME_NS_3RD             0x3FFFFFFF

//Timestamp 3rd Sample Time Seconds register
#define KSZ9563_TS_SAMPLE3_TIME_S_TS_SAMPLE_TIME_S_3RD               0xFFFFFFFF

//Timestamp 3rd Sample Time Phase register
#define KSZ9563_TS_SAMPLE3_TIME_PHASE_TS_SAMPLE_TIME_SUB_8NS_3RD     0x00000007

//Timestamp 4th Sample Time Nanoseconds register
#define KSZ9563_TS_SAMPLE4_TIME_NS_TS_SAMPLE_EDGE_4TH                0x40000000
#define KSZ9563_TS_SAMPLE4_TIME_NS_TS_SAMPLE_TIME_NS_4TH             0x3FFFFFFF

//Timestamp 4th Sample Time Seconds register
#define KSZ9563_TS_SAMPLE4_TIME_S_TS_SAMPLE_TIME_S_4TH               0xFFFFFFFF

//Timestamp 4th Sample Time Phase register
#define KSZ9563_TS_SAMPLE4_TIME_PHASE_TS_SAMPLE_TIME_SUB_8NS_4TH     0x00000007

//Timestamp 5th Sample Time Nanoseconds register
#define KSZ9563_TS_SAMPLE5_TIME_NS_TS_SAMPLE_EDGE_5TH                0x40000000
#define KSZ9563_TS_SAMPLE5_TIME_NS_TS_SAMPLE_TIME_NS_5TH             0x3FFFFFFF

//Timestamp 5th Sample Time Seconds register
#define KSZ9563_TS_SAMPLE5_TIME_S_TS_SAMPLE_TIME_S_5TH               0xFFFFFFFF

//Timestamp 5th Sample Time Phase register
#define KSZ9563_TS_SAMPLE5_TIME_PHASE_TS_SAMPLE_TIME_SUB_8NS_5TH     0x00000007

//Timestamp 6th Sample Time Nanoseconds register
#define KSZ9563_TS_SAMPLE6_TIME_NS_TS_SAMPLE_EDGE_6TH                0x40000000
#define KSZ9563_TS_SAMPLE6_TIME_NS_TS_SAMPLE_TIME_NS_6TH             0x3FFFFFFF

//Timestamp 6th Sample Time Seconds register
#define KSZ9563_TS_SAMPLE6_TIME_S_TS_SAMPLE_TIME_S_6TH               0xFFFFFFFF

//Timestamp 6th Sample Time Phase register
#define KSZ9563_TS_SAMPLE6_TIME_PHASE_TS_SAMPLE_TIME_SUB_8NS_6TH     0x00000007

//Timestamp 7th Sample Time Nanoseconds register
#define KSZ9563_TS_SAMPLE7_TIME_NS_TS_SAMPLE_EDGE_7TH                0x40000000
#define KSZ9563_TS_SAMPLE7_TIME_NS_TS_SAMPLE_TIME_NS_7TH             0x3FFFFFFF

//Timestamp 7th Sample Time Seconds register
#define KSZ9563_TS_SAMPLE7_TIME_S_TS_SAMPLE_TIME_S_7TH               0xFFFFFFFF

//Timestamp 7th Sample Time Phase register
#define KSZ9563_TS_SAMPLE7_TIME_PHASE_TS_SAMPLE_TIME_SUB_8NS_7TH     0x00000007

//Timestamp 8th Sample Time Nanoseconds register
#define KSZ9563_TS_SAMPLE8_TIME_NS_TS_SAMPLE_EDGE_8TH                0x40000000
#define KSZ9563_TS_SAMPLE8_TIME_NS_TS_SAMPLE_TIME_NS_8TH             0x3FFFFFFF

//Timestamp 8th Sample Time Seconds register
#define KSZ9563_TS_SAMPLE8_TIME_S_TS_SAMPLE_TIME_S_8TH               0xFFFFFFFF

//Timestamp 8th Sample Time Phase register
#define KSZ9563_TS_SAMPLE8_TIME_PHASE_TS_SAMPLE_TIME_SUB_8NS_8TH     0x00000007

//Port N Default Tag 0 register
#define KSZ9563_PORTn_DEFAULT_TAG0_PCP                               0xE0
#define KSZ9563_PORTn_DEFAULT_TAG0_DEI                               0x10
#define KSZ9563_PORTn_DEFAULT_TAG0_VID_MSB                           0x0F

//Port N Default Tag 1 register
#define KSZ9563_PORTn_DEFAULT_TAG1_VID_LSB                           0xFF

//Port N Interrupt Status register
#define KSZ9563_PORTn_INT_STATUS_PTP                                 0x04
#define KSZ9563_PORTn_INT_STATUS_PHY                                 0x02
#define KSZ9563_PORTn_INT_STATUS_ACL                                 0x01

//Port N Interrupt Mask register
#define KSZ9563_PORTn_INT_MASK_PTP                                   0x04
#define KSZ9563_PORTn_INT_MASK_PHY                                   0x02
#define KSZ9563_PORTn_INT_MASK_ACL                                   0x01

//Port N Operation Control 0 register
#define KSZ9563_PORTn_OP_CTRL0_LOCAL_LOOPBACK                        0x80
#define KSZ9563_PORTn_OP_CTRL0_REMOTE_LOOPBACK                       0x40
#define KSZ9563_PORTn_OP_CTRL0_TAIL_TAG_EN                           0x04
#define KSZ9563_PORTn_OP_CTRL0_TX_QUEUE_SPLIT_EN                     0x03

//Port N Status register
#define KSZ9563_PORTn_STATUS_SPEED                                   0x18
#define KSZ9563_PORTn_STATUS_SPEED_10MBPS                            0x00
#define KSZ9563_PORTn_STATUS_SPEED_100MBPS                           0x08
#define KSZ9563_PORTn_STATUS_SPEED_1000MBPS                          0x10
#define KSZ9563_PORTn_STATUS_DUPLEX                                  0x04
#define KSZ9563_PORTn_STATUS_TX_FLOW_CTRL_EN                         0x02
#define KSZ9563_PORTn_STATUS_RX_FLOW_CTRL_EN                         0x01

//XMII Port N Control 0 register
#define KSZ9563_PORTn_XMII_CTRL0_DUPLEX                              0x40
#define KSZ9563_PORTn_XMII_CTRL0_TX_FLOW_CTRL_EN                     0x20
#define KSZ9563_PORTn_XMII_CTRL0_SPEED_10_100                        0x10
#define KSZ9563_PORTn_XMII_CTRL0_RX_FLOW_CTRL_EN                     0x08

//XMII Port N Control 1 register
#define KSZ9563_PORTn_XMII_CTRL1_SPEED_1000                          0x40
#define KSZ9563_PORTn_XMII_CTRL1_RGMII_ID_IG                         0x10
#define KSZ9563_PORTn_XMII_CTRL1_RGMII_ID_EG                         0x08
#define KSZ9563_PORTn_XMII_CTRL1_MII_RMII_MODE                       0x04
#define KSZ9563_PORTn_XMII_CTRL1_IF_TYPE                             0x03
#define KSZ9563_PORTn_XMII_CTRL1_IF_TYPE_MII                         0x00
#define KSZ9563_PORTn_XMII_CTRL1_IF_TYPE_RMII                        0x01
#define KSZ9563_PORTn_XMII_CTRL1_IF_TYPE_RGMII                       0x03

//XMII Port N Control 3 register
#define KSZ9563_PORTn_XMII_CTRL3_RGMII_IBS_DUPLEX_STATUS             0x08
#define KSZ9563_PORTn_XMII_CTRL3_RGMII_IBS_RX_CLK_SPEED              0x06
#define KSZ9563_PORTn_XMII_CTRL3_RGMII_IBS_LINK_STATUS               0x01

//Port N MAC Control 0 register
#define KSZ9563_PORTn_MAC_CTRL0_BCAST_STORM_PROTECT_EN               0x02

//Port N MAC Control 1 register
#define KSZ9563_PORTn_MAC_CTRL1_BACK_PRESSURE_EN                     0x08
#define KSZ9563_PORTn_MAC_CTRL1_PASS_ALL_FRAMES                      0x01

//Port N MIB Control and Status register
#define KSZ9563_PORTn_MIB_CTRL_STAT_MIB_COUNTER_OVERFLOW             0x80000000
#define KSZ9563_PORTn_MIB_CTRL_STAT_MIB_READ                         0x02000000
#define KSZ9563_PORTn_MIB_CTRL_STAT_MIB_FLUSH_FREEZE                 0x01000000
#define KSZ9563_PORTn_MIB_CTRL_STAT_MIB_INDEX                        0x00FF0000
#define KSZ9563_PORTn_MIB_CTRL_STAT_MIB_COUNTER_VALUE_35_32          0x0000000F

//Port N MIB Data register
#define KSZ9563_PORTn_MIB_DATA_MIB_COUNTER_VALUE_31_0                0xFFFFFFFF

//Port N ACL Access Control 0 register
#define KSZ9563_PORTn_ACL_ACCESS_CTRL0_WRITE_STATUS                  0x40
#define KSZ9563_PORTn_ACL_ACCESS_CTRL0_READ_STATUS                   0x20
#define KSZ9563_PORTn_ACL_ACCESS_CTRL0_READ                          0x00
#define KSZ9563_PORTn_ACL_ACCESS_CTRL0_WRITE                         0x10
#define KSZ9563_PORTn_ACL_ACCESS_CTRL0_ACL_INDEX                     0x0F

//Port N Port Mirroring Control register
#define KSZ9563_PORTn_MIRRORING_CTRL_RECEIVE_SNIFF                   0x40
#define KSZ9563_PORTn_MIRRORING_CTRL_TRANSMIT_SNIFF                  0x20
#define KSZ9563_PORTn_MIRRORING_CTRL_SNIFFER_PORT                    0x02

//Port N Authentication Control register
#define KSZ9563_PORTn_AUTH_CTRL_ACL_EN                               0x04
#define KSZ9563_PORTn_AUTH_CTRL_AUTH_MODE                            0x03
#define KSZ9563_PORTn_AUTH_CTRL_AUTH_MODE_PASS                       0x00
#define KSZ9563_PORTn_AUTH_CTRL_AUTH_MODE_BLOCK                      0x01
#define KSZ9563_PORTn_AUTH_CTRL_AUTH_MODE_TRAP                       0x02

//Port N Pointer register
#define KSZ9563_PORTn_PTR_PORT_INDEX                                 0x00070000
#define KSZ9563_PORTn_PTR_QUEUE_PTR                                  0x00000003

//Port N Control 1 register
#define KSZ9563_PORTn_CTRL1_PORT_VLAN_MEMBERSHIP                     0x00000007
#define KSZ9563_PORTn_CTRL1_PORT3_VLAN_MEMBERSHIP                    0x00000004
#define KSZ9563_PORTn_CTRL1_PORT2_VLAN_MEMBERSHIP                    0x00000002
#define KSZ9563_PORTn_CTRL1_PORT1_VLAN_MEMBERSHIP                    0x00000001

//Port N Control 2 register
#define KSZ9563_PORTn_CTRL2_NULL_VID_LOOKUP_EN                       0x80
#define KSZ9563_PORTn_CTRL2_INGRESS_VLAN_FILT                        0x40
#define KSZ9563_PORTn_CTRL2_DISCARD_NON_PVID_PKT                     0x20
#define KSZ9563_PORTn_CTRL2_802_1X_EN                                0x10
#define KSZ9563_PORTn_CTRL2_SELF_ADDR_FILT                           0x08

//Port N MSTP Pointer register
#define KSZ9563_PORTn_MSTP_PTR_MSTP_PTR                              0x07

//Port N MSTP State register
#define KSZ9563_PORTn_MSTP_STATE_TRANSMIT_EN                         0x04
#define KSZ9563_PORTn_MSTP_STATE_RECEIVE_EN                          0x02
#define KSZ9563_PORTn_MSTP_STATE_LEARNING_DIS                        0x01

//Port N PTP Asymmetry Correction register
#define KSZ9563_PORTn_PTP_ASYM_CORRECTION_PTP_ASYM_COR_SIGN          0x8000
#define KSZ9563_PORTn_PTP_ASYM_CORRECTION_PTP_ASYM_COR               0x7FFF

//Port N PTP Timestamp Interrupt Status register
#define KSZ9563_PORTn_PTP_TS_INT_STAT_TS_SYNC_INT_STATUS             0x8000
#define KSZ9563_PORTn_PTP_TS_INT_STAT_TS_PDLY_REQ_INT_STATUS         0x4000
#define KSZ9563_PORTn_PTP_TS_INT_STAT_TS_PDLY_RESP_INT_STATUS        0x2000

//Port N PTP Timestamp Interrupt Enable register
#define KSZ9563_PORTn_PTP_TS_INT_EN_TS_SYNC_INT_ENB                  0x8000
#define KSZ9563_PORTn_PTP_TS_INT_EN_TS_PDLY_REQ_INT_ENB              0x4000
#define KSZ9563_PORTn_PTP_TS_INT_EN_TS_PDLY_RESP_INT_ENB             0x2000

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//KSZ9563 Ethernet switch driver
extern const SwitchDriver ksz9563SwitchDriver;

//KSZ9563 related functions
error_t ksz9563Init(NetInterface *interface);
void ksz9563InitHook(NetInterface *interface);

void ksz9563Tick(NetInterface *interface);

void ksz9563EnableIrq(NetInterface *interface);
void ksz9563DisableIrq(NetInterface *interface);

void ksz9563EventHandler(NetInterface *interface);

error_t ksz9563TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, NetTxAncillary *ancillary);

error_t ksz9563UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, NetRxAncillary *ancillary);

bool_t ksz9563GetLinkState(NetInterface *interface, uint8_t port);
uint32_t ksz9563GetLinkSpeed(NetInterface *interface, uint8_t port);
NicDuplexMode ksz9563GetDuplexMode(NetInterface *interface, uint8_t port);

void ksz9563SetPortState(NetInterface *interface, uint8_t port,
   SwitchPortState state);

SwitchPortState ksz9563GetPortState(NetInterface *interface, uint8_t port);

void ksz9563SetAgingTime(NetInterface *interface, uint32_t agingTime);

void ksz9563EnableIgmpSnooping(NetInterface *interface, bool_t enable);
void ksz9563EnableMldSnooping(NetInterface *interface, bool_t enable);
void ksz9563EnableRsvdMcastTable(NetInterface *interface, bool_t enable);

error_t ksz9563AddStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry);

error_t ksz9563DeleteStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry);

error_t ksz9563GetStaticFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry);

void ksz9563FlushStaticFdbTable(NetInterface *interface);

error_t ksz9563GetDynamicFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry);

void ksz9563FlushDynamicFdbTable(NetInterface *interface, uint8_t port);

void ksz9563SetUnknownMcastFwdPorts(NetInterface *interface,
   bool_t enable, uint32_t forwardPorts);

void ksz9563WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data);

uint16_t ksz9563ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address);

void ksz9563DumpPhyReg(NetInterface *interface, uint8_t port);

void ksz9563WriteMmdReg(NetInterface *interface, uint8_t port,
   uint8_t devAddr, uint16_t regAddr, uint16_t data);

uint16_t ksz9563ReadMmdReg(NetInterface *interface, uint8_t port,
   uint8_t devAddr, uint16_t regAddr);

void ksz9563WriteSwitchReg8(NetInterface *interface, uint16_t address,
   uint8_t data);

uint8_t ksz9563ReadSwitchReg8(NetInterface *interface, uint16_t address);

void ksz9563WriteSwitchReg16(NetInterface *interface, uint16_t address,
   uint16_t data);

uint16_t ksz9563ReadSwitchReg16(NetInterface *interface, uint16_t address);

void ksz9563WriteSwitchReg32(NetInterface *interface, uint16_t address,
   uint32_t data);

uint32_t ksz9563ReadSwitchReg32(NetInterface *interface, uint16_t address);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
