/**
 * @file lan9646_driver.h
 * @brief LAN9646 6-port Gigabit Ethernet switch driver
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

#ifndef _LAN9646_DRIVER_H
#define _LAN9646_DRIVER_H

//Dependencies
#include "core/nic.h"

//Port identifiers
#define LAN9646_PORT1 1
#define LAN9646_PORT2 2
#define LAN9646_PORT3 3
#define LAN9646_PORT4 4
#define LAN9646_PORT6 6
#define LAN9646_PORT7 7

//Port masks
#define LAN9646_PORT_MASK  0x6F
#define LAN9646_PORT1_MASK 0x01
#define LAN9646_PORT2_MASK 0x02
#define LAN9646_PORT3_MASK 0x04
#define LAN9646_PORT4_MASK 0x08
#define LAN9646_PORT6_MASK 0x20
#define LAN9646_PORT7_MASK 0x40

//SPI command byte
#define LAN9646_SPI_CMD_WRITE 0x40000000
#define LAN9646_SPI_CMD_READ  0x60000000
#define LAN9646_SPI_CMD_ADDR  0x001FFFE0

//Size of static and dynamic MAC tables
#define LAN9646_STATIC_MAC_TABLE_SIZE  16
#define LAN9646_DYNAMIC_MAC_TABLE_SIZE 4096

//Tail tag rules (host to LAN9646)
#define LAN9646_TAIL_TAG_NORMAL_ADDR_LOOKUP     0x0400
#define LAN9646_TAIL_TAG_PORT_BLOCKING_OVERRIDE 0x0200
#define LAN9646_TAIL_TAG_PRIORITY               0x0180
#define LAN9646_TAIL_TAG_DEST_PORT7             0x0040
#define LAN9646_TAIL_TAG_DEST_PORT6             0x0020
#define LAN9646_TAIL_TAG_DEST_PORT4             0x0008
#define LAN9646_TAIL_TAG_DEST_PORT3             0x0004
#define LAN9646_TAIL_TAG_DEST_PORT2             0x0002
#define LAN9646_TAIL_TAG_DEST_PORT1             0x0001

//Tail tag rules (LAN9646 to host)
#define LAN9646_TAIL_TAG_PTP_MSG                0x80
#define LAN9646_TAIL_TAG_SRC_PORT               0x07

//LAN9646 PHY registers
#define LAN9646_BMCR                                           0x00
#define LAN9646_BMSR                                           0x01
#define LAN9646_PHYID1                                         0x02
#define LAN9646_PHYID2                                         0x03
#define LAN9646_ANAR                                           0x04
#define LAN9646_ANLPAR                                         0x05
#define LAN9646_ANER                                           0x06
#define LAN9646_ANNPR                                          0x07
#define LAN9646_ANLPNPR                                        0x08
#define LAN9646_GBCR                                           0x09
#define LAN9646_GBSR                                           0x0A
#define LAN9646_MMDACR                                         0x0D
#define LAN9646_MMDAADR                                        0x0E
#define LAN9646_GBESR                                          0x0F
#define LAN9646_RLB                                            0x11
#define LAN9646_LINKMD                                         0x12
#define LAN9646_DPMAPCSS                                       0x13
#define LAN9646_RXERCTR                                        0x15
#define LAN9646_ICSR                                           0x1B
#define LAN9646_AUTOMDI                                        0x1C
#define LAN9646_PHYCON                                         0x1F

//LAN9646 MMD registers
#define LAN9646_MMD_LED_MODE                                   0x02, 0x00
#define LAN9646_MMD_EEE_ADV                                    0x07, 0x3C

//LAN9646 Switch registers
#define LAN9646_CHIP_ID0                                       0x0000
#define LAN9646_CHIP_ID1                                       0x0001
#define LAN9646_CHIP_ID2                                       0x0002
#define LAN9646_CHIP_ID3                                       0x0003
#define LAN9646_PME_PIN_CTRL                                   0x0006
#define LAN9646_GLOBAL_INT_STAT                                0x0010
#define LAN9646_GLOBAL_INT_MASK                                0x0014
#define LAN9646_GLOBAL_PORT_INT_STAT                           0x0018
#define LAN9646_GLOBAL_PORT_INT_MASK                           0x001C
#define LAN9646_SERIAL_IO_CTRL                                 0x0100
#define LAN9646_OUT_CLK_CTRL                                   0x0103
#define LAN9646_IBA_CTRL                                       0x0104
#define LAN9646_IO_DRIVE_STRENGTH                              0x010D
#define LAN9646_IBA_OP_STAT1                                   0x0110
#define LAN9646_LED_OVERRIDE                                   0x0120
#define LAN9646_LED_OUTPUT                                     0x0124
#define LAN9646_PWR_DOWN_CTRL0                                 0x0201
#define LAN9646_LED_STRAP_IN                                   0x0210
#define LAN9646_SWITCH_OP                                      0x0300
#define LAN9646_SWITCH_MAC_ADDR0                               0x0302
#define LAN9646_SWITCH_MAC_ADDR1                               0x0303
#define LAN9646_SWITCH_MAC_ADDR2                               0x0304
#define LAN9646_SWITCH_MAC_ADDR3                               0x0305
#define LAN9646_SWITCH_MAC_ADDR4                               0x0306
#define LAN9646_SWITCH_MAC_ADDR5                               0x0307
#define LAN9646_SWITCH_MTU                                     0x0308
#define LAN9646_SWITCH_ISP_TPID                                0x030A
#define LAN9646_SWITCH_LUE_CTRL0                               0x0310
#define LAN9646_SWITCH_LUE_CTRL1                               0x0311
#define LAN9646_SWITCH_LUE_CTRL2                               0x0312
#define LAN9646_SWITCH_LUE_CTRL3                               0x0313
#define LAN9646_ALU_TABLE_INT                                  0x0314
#define LAN9646_ALU_TABLE_MASK                                 0x0315
#define LAN9646_ALU_TABLE_ENTRY_INDEX0                         0x0316
#define LAN9646_ALU_TABLE_ENTRY_INDEX1                         0x0318
#define LAN9646_ALU_TABLE_ENTRY_INDEX2                         0x031A
#define LAN9646_UNKNOWN_UNICAST_CTRL                           0x0320
#define LAN9646_UNKONWN_MULTICAST_CTRL                         0x0324
#define LAN9646_UNKNOWN_VLAN_ID_CTRL                           0x0328
#define LAN9646_SWITCH_MAC_CTRL0                               0x0330
#define LAN9646_SWITCH_MAC_CTRL1                               0x0331
#define LAN9646_SWITCH_MAC_CTRL2                               0x0332
#define LAN9646_SWITCH_MAC_CTRL3                               0x0333
#define LAN9646_SWITCH_MAC_CTRL4                               0x0334
#define LAN9646_SWITCH_MAC_CTRL5                               0x0335
#define LAN9646_SWITCH_MIB_CTRL                                0x0336
#define LAN9646_802_1P_PRIO_MAPPING0                           0x0338
#define LAN9646_802_1P_PRIO_MAPPING1                           0x0339
#define LAN9646_802_1P_PRIO_MAPPING2                           0x033A
#define LAN9646_802_1P_PRIO_MAPPING3                           0x033B
#define LAN9646_IP_DIFFSERV_PRIO_EN                            0x033E
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING0                      0x0340
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING1                      0x0341
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING2                      0x0342
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING3                      0x0343
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING4                      0x0344
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING5                      0x0345
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING6                      0x0346
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING7                      0x0347
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING8                      0x0348
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING9                      0x0349
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING10                     0x034A
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING11                     0x034B
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING12                     0x034C
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING13                     0x034D
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING14                     0x034E
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING15                     0x034F
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING16                     0x0350
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING17                     0x0351
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING18                     0x0352
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING19                     0x0353
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING20                     0x0354
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING21                     0x0355
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING22                     0x0356
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING23                     0x0357
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING24                     0x0358
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING25                     0x0359
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING26                     0x035A
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING27                     0x035B
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING28                     0x035C
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING29                     0x035D
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING30                     0x035E
#define LAN9646_IP_DIFFSERV_PRIO_MAPPING31                     0x035F
#define LAN9646_GLOBAL_PORT_MIRROR_SNOOP_CTRL                  0x0370
#define LAN9646_WRED_DIFFSERV_COLOR_MAPPING                    0x0378
#define LAN9646_QUEUE_MGMT_CTRL0                               0x0390
#define LAN9646_VLAN_TABLE_ENTRY0                              0x0400
#define LAN9646_VLAN_TABLE_ENTRY1                              0x0404
#define LAN9646_VLAN_TABLE_ENTRY2                              0x0408
#define LAN9646_VLAN_TABLE_INDEX                               0x040C
#define LAN9646_VLAN_TABLE_ACCESS_CTRL                         0x040E
#define LAN9646_ALU_TABLE_INDEX0                               0x0410
#define LAN9646_ALU_TABLE_INDEX1                               0x0414
#define LAN9646_ALU_TABLE_CTRL                                 0x0418
#define LAN9646_STATIC_MCAST_TABLE_CTRL                        0x041C
#define LAN9646_ALU_TABLE_ENTRY1                               0x0420
#define LAN9646_STATIC_TABLE_ENTRY1                            0x0420
#define LAN9646_ALU_TABLE_ENTRY2                               0x0424
#define LAN9646_STATIC_TABLE_ENTRY2                            0x0424
#define LAN9646_RES_MCAST_TABLE_ENTRY2                         0x0424
#define LAN9646_ALU_TABLE_ENTRY3                               0x0428
#define LAN9646_STATIC_TABLE_ENTRY3                            0x0428
#define LAN9646_ALU_TABLE_ENTRY4                               0x042C
#define LAN9646_STATIC_TABLE_ENTRY4                            0x042C
#define LAN9646_PORT1_DEFAULT_TAG0                             0x1000
#define LAN9646_PORT1_DEFAULT_TAG1                             0x1001
#define LAN9646_PORT1_PME_WOL_EVENT                            0x1013
#define LAN9646_PORT1_PME_WOL_EN                               0x1017
#define LAN9646_PORT1_INT_STATUS                               0x101B
#define LAN9646_PORT1_INT_MASK                                 0x101F
#define LAN9646_PORT1_OP_CTRL0                                 0x1020
#define LAN9646_PORT1_STATUS                                   0x1030
#define LAN9646_PORT1_MAC_CTRL0                                0x1400
#define LAN9646_PORT1_MAC_CTRL1                                0x1401
#define LAN9646_PORT1_IG_RATE_LIMIT_CTRL                       0x1403
#define LAN9646_PORT1_PRIO0_IG_LIMIT_CTRL                      0x1410
#define LAN9646_PORT1_PRIO1_IG_LIMIT_CTRL                      0x1411
#define LAN9646_PORT1_PRIO2_IG_LIMIT_CTRL                      0x1412
#define LAN9646_PORT1_PRIO3_IG_LIMIT_CTRL                      0x1413
#define LAN9646_PORT1_PRIO4_IG_LIMIT_CTRL                      0x1414
#define LAN9646_PORT1_PRIO5_IG_LIMIT_CTRL                      0x1415
#define LAN9646_PORT1_PRIO6_IG_LIMIT_CTRL                      0x1416
#define LAN9646_PORT1_PRIO7_IG_LIMIT_CTRL                      0x1417
#define LAN9646_PORT1_QUEUE0_EG_LIMIT_CTRL                     0x1420
#define LAN9646_PORT1_QUEUE1_EG_LIMIT_CTRL                     0x1421
#define LAN9646_PORT1_QUEUE2_EG_LIMIT_CTRL                     0x1422
#define LAN9646_PORT1_QUEUE3_EG_LIMIT_CTRL                     0x1423
#define LAN9646_PORT1_MIB_CTRL_STAT                            0x1500
#define LAN9646_PORT1_MIB_DATA                                 0x1504
#define LAN9646_PORT1_ACL_ACCESS0                              0x1600
#define LAN9646_PORT1_ACL_ACCESS1                              0x1601
#define LAN9646_PORT1_ACL_ACCESS2                              0x1602
#define LAN9646_PORT1_ACL_ACCESS3                              0x1603
#define LAN9646_PORT1_ACL_ACCESS4                              0x1604
#define LAN9646_PORT1_ACL_ACCESS5                              0x1605
#define LAN9646_PORT1_ACL_ACCESS6                              0x1606
#define LAN9646_PORT1_ACL_ACCESS7                              0x1607
#define LAN9646_PORT1_ACL_ACCESS8                              0x1608
#define LAN9646_PORT1_ACL_ACCESS9                              0x1609
#define LAN9646_PORT1_ACL_ACCESS10                             0x160A
#define LAN9646_PORT1_ACL_ACCESS11                             0x160B
#define LAN9646_PORT1_ACL_ACCESS12                             0x160C
#define LAN9646_PORT1_ACL_ACCESS13                             0x160D
#define LAN9646_PORT1_ACL_ACCESS14                             0x160E
#define LAN9646_PORT1_ACL_ACCESS15                             0x160F
#define LAN9646_PORT1_ACL_BYTE_EN_MSB                          0x1610
#define LAN9646_PORT1_ACL_BYTE_EN_LSB                          0x1611
#define LAN9646_PORT1_ACL_ACCESS_CTRL0                         0x1612
#define LAN9646_PORT1_MIRRORING_CTRL                           0x1800
#define LAN9646_PORT1_PRIO_CTRL                                0x1801
#define LAN9646_PORT1_IG_MAC_CTRL                              0x1802
#define LAN9646_PORT1_AUTH_CTRL                                0x1803
#define LAN9646_PORT1_PTR                                      0x1804
#define LAN9646_PORT1_PRIO_TO_QUEUE_MAPPING                    0x1808
#define LAN9646_PORT1_POLICE_CTRL                              0x180C
#define LAN9646_PORT1_POLICE_QUEUE_RATE                        0x1820
#define LAN9646_PORT1_POLICE_QUEUE_BURST_SIZE                  0x1824
#define LAN9646_PORT1_WRED_PKT_MEM_CTRL0                       0x1830
#define LAN9646_PORT1_WRED_PKT_MEM_CTRL1                       0x1834
#define LAN9646_PORT1_WRED_QUEUE_CTRL0                         0x1840
#define LAN9646_PORT1_WRED_QUEUE_CTRL1                         0x1844
#define LAN9646_PORT1_WRED_QUEUE_PERF_MON_CTRL                 0x1848
#define LAN9646_PORT1_TX_QUEUE_INDEX                           0x1900
#define LAN9646_PORT1_TX_QUEUE_PVID                            0x1904
#define LAN9646_PORT1_TX_QUEUE_CTRL0                           0x1914
#define LAN9646_PORT1_TX_QUEUE_CTRL1                           0x1915
#define LAN9646_PORT1_CTRL0                                    0x1A00
#define LAN9646_PORT1_CTRL1                                    0x1A04
#define LAN9646_PORT1_CTRL2                                    0x1B00
#define LAN9646_PORT1_MSTP_PTR                                 0x1B01
#define LAN9646_PORT1_MSTP_STATE                               0x1B04
#define LAN9646_PORT2_DEFAULT_TAG0                             0x2000
#define LAN9646_PORT2_DEFAULT_TAG1                             0x2001
#define LAN9646_PORT2_PME_WOL_EVENT                            0x2013
#define LAN9646_PORT2_PME_WOL_EN                               0x2017
#define LAN9646_PORT2_INT_STATUS                               0x201B
#define LAN9646_PORT2_INT_MASK                                 0x201F
#define LAN9646_PORT2_OP_CTRL0                                 0x2020
#define LAN9646_PORT2_STATUS                                   0x2030
#define LAN9646_PORT2_MAC_CTRL0                                0x2400
#define LAN9646_PORT2_MAC_CTRL1                                0x2401
#define LAN9646_PORT2_IG_RATE_LIMIT_CTRL                       0x2403
#define LAN9646_PORT2_PRIO0_IG_LIMIT_CTRL                      0x2410
#define LAN9646_PORT2_PRIO1_IG_LIMIT_CTRL                      0x2411
#define LAN9646_PORT2_PRIO2_IG_LIMIT_CTRL                      0x2412
#define LAN9646_PORT2_PRIO3_IG_LIMIT_CTRL                      0x2413
#define LAN9646_PORT2_PRIO4_IG_LIMIT_CTRL                      0x2414
#define LAN9646_PORT2_PRIO5_IG_LIMIT_CTRL                      0x2415
#define LAN9646_PORT2_PRIO6_IG_LIMIT_CTRL                      0x2416
#define LAN9646_PORT2_PRIO7_IG_LIMIT_CTRL                      0x2417
#define LAN9646_PORT2_QUEUE0_EG_LIMIT_CTRL                     0x2420
#define LAN9646_PORT2_QUEUE1_EG_LIMIT_CTRL                     0x2421
#define LAN9646_PORT2_QUEUE2_EG_LIMIT_CTRL                     0x2422
#define LAN9646_PORT2_QUEUE3_EG_LIMIT_CTRL                     0x2423
#define LAN9646_PORT2_MIB_CTRL_STAT                            0x2500
#define LAN9646_PORT2_MIB_DATA                                 0x2504
#define LAN9646_PORT2_ACL_ACCESS0                              0x2600
#define LAN9646_PORT2_ACL_ACCESS1                              0x2601
#define LAN9646_PORT2_ACL_ACCESS2                              0x2602
#define LAN9646_PORT2_ACL_ACCESS3                              0x2603
#define LAN9646_PORT2_ACL_ACCESS4                              0x2604
#define LAN9646_PORT2_ACL_ACCESS5                              0x2605
#define LAN9646_PORT2_ACL_ACCESS6                              0x2606
#define LAN9646_PORT2_ACL_ACCESS7                              0x2607
#define LAN9646_PORT2_ACL_ACCESS8                              0x2608
#define LAN9646_PORT2_ACL_ACCESS9                              0x2609
#define LAN9646_PORT2_ACL_ACCESS10                             0x260A
#define LAN9646_PORT2_ACL_ACCESS11                             0x260B
#define LAN9646_PORT2_ACL_ACCESS12                             0x260C
#define LAN9646_PORT2_ACL_ACCESS13                             0x260D
#define LAN9646_PORT2_ACL_ACCESS14                             0x260E
#define LAN9646_PORT2_ACL_ACCESS15                             0x260F
#define LAN9646_PORT2_ACL_BYTE_EN_MSB                          0x2610
#define LAN9646_PORT2_ACL_BYTE_EN_LSB                          0x2611
#define LAN9646_PORT2_ACL_ACCESS_CTRL0                         0x2612
#define LAN9646_PORT2_MIRRORING_CTRL                           0x2800
#define LAN9646_PORT2_PRIO_CTRL                                0x2801
#define LAN9646_PORT2_IG_MAC_CTRL                              0x2802
#define LAN9646_PORT2_AUTH_CTRL                                0x2803
#define LAN9646_PORT2_PTR                                      0x2804
#define LAN9646_PORT2_PRIO_TO_QUEUE_MAPPING                    0x2808
#define LAN9646_PORT2_POLICE_CTRL                              0x280C
#define LAN9646_PORT2_POLICE_QUEUE_RATE                        0x2820
#define LAN9646_PORT2_POLICE_QUEUE_BURST_SIZE                  0x2824
#define LAN9646_PORT2_WRED_PKT_MEM_CTRL0                       0x2830
#define LAN9646_PORT2_WRED_PKT_MEM_CTRL1                       0x2834
#define LAN9646_PORT2_WRED_QUEUE_CTRL0                         0x2840
#define LAN9646_PORT2_WRED_QUEUE_CTRL1                         0x2844
#define LAN9646_PORT2_WRED_QUEUE_PERF_MON_CTRL                 0x2848
#define LAN9646_PORT2_TX_QUEUE_INDEX                           0x2900
#define LAN9646_PORT2_TX_QUEUE_PVID                            0x2904
#define LAN9646_PORT2_TX_QUEUE_CTRL0                           0x2914
#define LAN9646_PORT2_TX_QUEUE_CTRL1                           0x2915
#define LAN9646_PORT2_CTRL0                                    0x2A00
#define LAN9646_PORT2_CTRL1                                    0x2A04
#define LAN9646_PORT2_CTRL2                                    0x2B00
#define LAN9646_PORT2_MSTP_PTR                                 0x2B01
#define LAN9646_PORT2_MSTP_STATE                               0x2B04
#define LAN9646_PORT3_DEFAULT_TAG0                             0x3000
#define LAN9646_PORT3_DEFAULT_TAG1                             0x3001
#define LAN9646_PORT3_PME_WOL_EVENT                            0x3013
#define LAN9646_PORT3_PME_WOL_EN                               0x3017
#define LAN9646_PORT3_INT_STATUS                               0x301B
#define LAN9646_PORT3_INT_MASK                                 0x301F
#define LAN9646_PORT3_OP_CTRL0                                 0x3020
#define LAN9646_PORT3_STATUS                                   0x3030
#define LAN9646_PORT3_MAC_CTRL0                                0x3400
#define LAN9646_PORT3_MAC_CTRL1                                0x3401
#define LAN9646_PORT3_IG_RATE_LIMIT_CTRL                       0x3403
#define LAN9646_PORT3_PRIO0_IG_LIMIT_CTRL                      0x3410
#define LAN9646_PORT3_PRIO1_IG_LIMIT_CTRL                      0x3411
#define LAN9646_PORT3_PRIO2_IG_LIMIT_CTRL                      0x3412
#define LAN9646_PORT3_PRIO3_IG_LIMIT_CTRL                      0x3413
#define LAN9646_PORT3_PRIO4_IG_LIMIT_CTRL                      0x3414
#define LAN9646_PORT3_PRIO5_IG_LIMIT_CTRL                      0x3415
#define LAN9646_PORT3_PRIO6_IG_LIMIT_CTRL                      0x3416
#define LAN9646_PORT3_PRIO7_IG_LIMIT_CTRL                      0x3417
#define LAN9646_PORT3_QUEUE0_EG_LIMIT_CTRL                     0x3420
#define LAN9646_PORT3_QUEUE1_EG_LIMIT_CTRL                     0x3421
#define LAN9646_PORT3_QUEUE2_EG_LIMIT_CTRL                     0x3422
#define LAN9646_PORT3_QUEUE3_EG_LIMIT_CTRL                     0x3423
#define LAN9646_PORT3_MIB_CTRL_STAT                            0x3500
#define LAN9646_PORT3_MIB_DATA                                 0x3504
#define LAN9646_PORT3_ACL_ACCESS0                              0x3600
#define LAN9646_PORT3_ACL_ACCESS1                              0x3601
#define LAN9646_PORT3_ACL_ACCESS2                              0x3602
#define LAN9646_PORT3_ACL_ACCESS3                              0x3603
#define LAN9646_PORT3_ACL_ACCESS4                              0x3604
#define LAN9646_PORT3_ACL_ACCESS5                              0x3605
#define LAN9646_PORT3_ACL_ACCESS6                              0x3606
#define LAN9646_PORT3_ACL_ACCESS7                              0x3607
#define LAN9646_PORT3_ACL_ACCESS8                              0x3608
#define LAN9646_PORT3_ACL_ACCESS9                              0x3609
#define LAN9646_PORT3_ACL_ACCESS10                             0x360A
#define LAN9646_PORT3_ACL_ACCESS11                             0x360B
#define LAN9646_PORT3_ACL_ACCESS12                             0x360C
#define LAN9646_PORT3_ACL_ACCESS13                             0x360D
#define LAN9646_PORT3_ACL_ACCESS14                             0x360E
#define LAN9646_PORT3_ACL_ACCESS15                             0x360F
#define LAN9646_PORT3_ACL_BYTE_EN_MSB                          0x3610
#define LAN9646_PORT3_ACL_BYTE_EN_LSB                          0x3611
#define LAN9646_PORT3_ACL_ACCESS_CTRL0                         0x3612
#define LAN9646_PORT3_MIRRORING_CTRL                           0x3800
#define LAN9646_PORT3_PRIO_CTRL                                0x3801
#define LAN9646_PORT3_IG_MAC_CTRL                              0x3802
#define LAN9646_PORT3_AUTH_CTRL                                0x3803
#define LAN9646_PORT3_PTR                                      0x3804
#define LAN9646_PORT3_PRIO_TO_QUEUE_MAPPING                    0x3808
#define LAN9646_PORT3_POLICE_CTRL                              0x380C
#define LAN9646_PORT3_POLICE_QUEUE_RATE                        0x3820
#define LAN9646_PORT3_POLICE_QUEUE_BURST_SIZE                  0x3824
#define LAN9646_PORT3_WRED_PKT_MEM_CTRL0                       0x3830
#define LAN9646_PORT3_WRED_PKT_MEM_CTRL1                       0x3834
#define LAN9646_PORT3_WRED_QUEUE_CTRL0                         0x3840
#define LAN9646_PORT3_WRED_QUEUE_CTRL1                         0x3844
#define LAN9646_PORT3_WRED_QUEUE_PERF_MON_CTRL                 0x3848
#define LAN9646_PORT3_TX_QUEUE_INDEX                           0x3900
#define LAN9646_PORT3_TX_QUEUE_PVID                            0x3904
#define LAN9646_PORT3_TX_QUEUE_CTRL0                           0x3914
#define LAN9646_PORT3_TX_QUEUE_CTRL1                           0x3915
#define LAN9646_PORT3_CTRL0                                    0x3A00
#define LAN9646_PORT3_CTRL1                                    0x3A04
#define LAN9646_PORT3_CTRL2                                    0x3B00
#define LAN9646_PORT3_MSTP_PTR                                 0x3B01
#define LAN9646_PORT3_MSTP_STATE                               0x3B04
#define LAN9646_PORT4_DEFAULT_TAG0                             0x4000
#define LAN9646_PORT4_DEFAULT_TAG1                             0x4001
#define LAN9646_PORT4_PME_WOL_EVENT                            0x4013
#define LAN9646_PORT4_PME_WOL_EN                               0x4017
#define LAN9646_PORT4_INT_STATUS                               0x401B
#define LAN9646_PORT4_INT_MASK                                 0x401F
#define LAN9646_PORT4_OP_CTRL0                                 0x4020
#define LAN9646_PORT4_STATUS                                   0x4030
#define LAN9646_PORT4_MAC_CTRL0                                0x4400
#define LAN9646_PORT4_MAC_CTRL1                                0x4401
#define LAN9646_PORT4_IG_RATE_LIMIT_CTRL                       0x4403
#define LAN9646_PORT4_PRIO0_IG_LIMIT_CTRL                      0x4410
#define LAN9646_PORT4_PRIO1_IG_LIMIT_CTRL                      0x4411
#define LAN9646_PORT4_PRIO2_IG_LIMIT_CTRL                      0x4412
#define LAN9646_PORT4_PRIO3_IG_LIMIT_CTRL                      0x4413
#define LAN9646_PORT4_PRIO4_IG_LIMIT_CTRL                      0x4414
#define LAN9646_PORT4_PRIO5_IG_LIMIT_CTRL                      0x4415
#define LAN9646_PORT4_PRIO6_IG_LIMIT_CTRL                      0x4416
#define LAN9646_PORT4_PRIO7_IG_LIMIT_CTRL                      0x4417
#define LAN9646_PORT4_QUEUE0_EG_LIMIT_CTRL                     0x4420
#define LAN9646_PORT4_QUEUE1_EG_LIMIT_CTRL                     0x4421
#define LAN9646_PORT4_QUEUE2_EG_LIMIT_CTRL                     0x4422
#define LAN9646_PORT4_QUEUE3_EG_LIMIT_CTRL                     0x4423
#define LAN9646_PORT4_MIB_CTRL_STAT                            0x4500
#define LAN9646_PORT4_MIB_DATA                                 0x4504
#define LAN9646_PORT4_ACL_ACCESS0                              0x4600
#define LAN9646_PORT4_ACL_ACCESS1                              0x4601
#define LAN9646_PORT4_ACL_ACCESS2                              0x4602
#define LAN9646_PORT4_ACL_ACCESS3                              0x4603
#define LAN9646_PORT4_ACL_ACCESS4                              0x4604
#define LAN9646_PORT4_ACL_ACCESS5                              0x4605
#define LAN9646_PORT4_ACL_ACCESS6                              0x4606
#define LAN9646_PORT4_ACL_ACCESS7                              0x4607
#define LAN9646_PORT4_ACL_ACCESS8                              0x4608
#define LAN9646_PORT4_ACL_ACCESS9                              0x4609
#define LAN9646_PORT4_ACL_ACCESS10                             0x460A
#define LAN9646_PORT4_ACL_ACCESS11                             0x460B
#define LAN9646_PORT4_ACL_ACCESS12                             0x460C
#define LAN9646_PORT4_ACL_ACCESS13                             0x460D
#define LAN9646_PORT4_ACL_ACCESS14                             0x460E
#define LAN9646_PORT4_ACL_ACCESS15                             0x460F
#define LAN9646_PORT4_ACL_BYTE_EN_MSB                          0x4610
#define LAN9646_PORT4_ACL_BYTE_EN_LSB                          0x4611
#define LAN9646_PORT4_ACL_ACCESS_CTRL0                         0x4612
#define LAN9646_PORT4_MIRRORING_CTRL                           0x4800
#define LAN9646_PORT4_PRIO_CTRL                                0x4801
#define LAN9646_PORT4_IG_MAC_CTRL                              0x4802
#define LAN9646_PORT4_AUTH_CTRL                                0x4803
#define LAN9646_PORT4_PTR                                      0x4804
#define LAN9646_PORT4_PRIO_TO_QUEUE_MAPPING                    0x4808
#define LAN9646_PORT4_POLICE_CTRL                              0x480C
#define LAN9646_PORT4_POLICE_QUEUE_RATE                        0x4820
#define LAN9646_PORT4_POLICE_QUEUE_BURST_SIZE                  0x4824
#define LAN9646_PORT4_WRED_PKT_MEM_CTRL0                       0x4830
#define LAN9646_PORT4_WRED_PKT_MEM_CTRL1                       0x4834
#define LAN9646_PORT4_WRED_QUEUE_CTRL0                         0x4840
#define LAN9646_PORT4_WRED_QUEUE_CTRL1                         0x4844
#define LAN9646_PORT4_WRED_QUEUE_PERF_MON_CTRL                 0x4848
#define LAN9646_PORT4_TX_QUEUE_INDEX                           0x4900
#define LAN9646_PORT4_TX_QUEUE_PVID                            0x4904
#define LAN9646_PORT4_TX_QUEUE_CTRL0                           0x4914
#define LAN9646_PORT4_TX_QUEUE_CTRL1                           0x4915
#define LAN9646_PORT4_CTRL0                                    0x4A00
#define LAN9646_PORT4_CTRL1                                    0x4A04
#define LAN9646_PORT4_CTRL2                                    0x4B00
#define LAN9646_PORT4_MSTP_PTR                                 0x4B01
#define LAN9646_PORT4_MSTP_STATE                               0x4B04
#define LAN9646_PORT6_DEFAULT_TAG0                             0x6000
#define LAN9646_PORT6_DEFAULT_TAG1                             0x6001
#define LAN9646_PORT6_PME_WOL_EVENT                            0x6013
#define LAN9646_PORT6_PME_WOL_EN                               0x6017
#define LAN9646_PORT6_INT_STATUS                               0x601B
#define LAN9646_PORT6_INT_MASK                                 0x601F
#define LAN9646_PORT6_OP_CTRL0                                 0x6020
#define LAN9646_PORT6_STATUS                                   0x6030
#define LAN9646_PORT6_XMII_CTRL0                               0x6300
#define LAN9646_PORT6_XMII_CTRL1                               0x6301
#define LAN9646_PORT6_MAC_CTRL0                                0x6400
#define LAN9646_PORT6_MAC_CTRL1                                0x6401
#define LAN9646_PORT6_IG_RATE_LIMIT_CTRL                       0x6403
#define LAN9646_PORT6_PRIO0_IG_LIMIT_CTRL                      0x6410
#define LAN9646_PORT6_PRIO1_IG_LIMIT_CTRL                      0x6411
#define LAN9646_PORT6_PRIO2_IG_LIMIT_CTRL                      0x6412
#define LAN9646_PORT6_PRIO3_IG_LIMIT_CTRL                      0x6413
#define LAN9646_PORT6_PRIO4_IG_LIMIT_CTRL                      0x6414
#define LAN9646_PORT6_PRIO5_IG_LIMIT_CTRL                      0x6415
#define LAN9646_PORT6_PRIO6_IG_LIMIT_CTRL                      0x6416
#define LAN9646_PORT6_PRIO7_IG_LIMIT_CTRL                      0x6417
#define LAN9646_PORT6_QUEUE0_EG_LIMIT_CTRL                     0x6420
#define LAN9646_PORT6_QUEUE1_EG_LIMIT_CTRL                     0x6421
#define LAN9646_PORT6_QUEUE2_EG_LIMIT_CTRL                     0x6422
#define LAN9646_PORT6_QUEUE3_EG_LIMIT_CTRL                     0x6423
#define LAN9646_PORT6_MIB_CTRL_STAT                            0x6500
#define LAN9646_PORT6_MIB_DATA                                 0x6504
#define LAN9646_PORT6_ACL_ACCESS0                              0x6600
#define LAN9646_PORT6_ACL_ACCESS1                              0x6601
#define LAN9646_PORT6_ACL_ACCESS2                              0x6602
#define LAN9646_PORT6_ACL_ACCESS3                              0x6603
#define LAN9646_PORT6_ACL_ACCESS4                              0x6604
#define LAN9646_PORT6_ACL_ACCESS5                              0x6605
#define LAN9646_PORT6_ACL_ACCESS6                              0x6606
#define LAN9646_PORT6_ACL_ACCESS7                              0x6607
#define LAN9646_PORT6_ACL_ACCESS8                              0x6608
#define LAN9646_PORT6_ACL_ACCESS9                              0x6609
#define LAN9646_PORT6_ACL_ACCESS10                             0x660A
#define LAN9646_PORT6_ACL_ACCESS11                             0x660B
#define LAN9646_PORT6_ACL_ACCESS12                             0x660C
#define LAN9646_PORT6_ACL_ACCESS13                             0x660D
#define LAN9646_PORT6_ACL_ACCESS14                             0x660E
#define LAN9646_PORT6_ACL_ACCESS15                             0x660F
#define LAN9646_PORT6_ACL_BYTE_EN_MSB                          0x6610
#define LAN9646_PORT6_ACL_BYTE_EN_LSB                          0x6611
#define LAN9646_PORT6_ACL_ACCESS_CTRL0                         0x6612
#define LAN9646_PORT6_MIRRORING_CTRL                           0x6800
#define LAN9646_PORT6_PRIO_CTRL                                0x6801
#define LAN9646_PORT6_IG_MAC_CTRL                              0x6802
#define LAN9646_PORT6_AUTH_CTRL                                0x6803
#define LAN9646_PORT6_PTR                                      0x6804
#define LAN9646_PORT6_PRIO_TO_QUEUE_MAPPING                    0x6808
#define LAN9646_PORT6_POLICE_CTRL                              0x680C
#define LAN9646_PORT6_POLICE_QUEUE_RATE                        0x6820
#define LAN9646_PORT6_POLICE_QUEUE_BURST_SIZE                  0x6824
#define LAN9646_PORT6_WRED_PKT_MEM_CTRL0                       0x6830
#define LAN9646_PORT6_WRED_PKT_MEM_CTRL1                       0x6834
#define LAN9646_PORT6_WRED_QUEUE_CTRL0                         0x6840
#define LAN9646_PORT6_WRED_QUEUE_CTRL1                         0x6844
#define LAN9646_PORT6_WRED_QUEUE_PERF_MON_CTRL                 0x6848
#define LAN9646_PORT6_TX_QUEUE_INDEX                           0x6900
#define LAN9646_PORT6_TX_QUEUE_PVID                            0x6904
#define LAN9646_PORT6_TX_QUEUE_CTRL0                           0x6914
#define LAN9646_PORT6_TX_QUEUE_CTRL1                           0x6915
#define LAN9646_PORT6_CTRL0                                    0x6A00
#define LAN9646_PORT6_CTRL1                                    0x6A04
#define LAN9646_PORT6_CTRL2                                    0x6B00
#define LAN9646_PORT6_MSTP_PTR                                 0x6B01
#define LAN9646_PORT6_MSTP_STATE                               0x6B04
#define LAN9646_PORT7_DEFAULT_TAG0                             0x7000
#define LAN9646_PORT7_DEFAULT_TAG1                             0x7001
#define LAN9646_PORT7_PME_WOL_EVENT                            0x7013
#define LAN9646_PORT7_PME_WOL_EN                               0x7017
#define LAN9646_PORT7_INT_STATUS                               0x701B
#define LAN9646_PORT7_INT_MASK                                 0x701F
#define LAN9646_PORT7_OP_CTRL0                                 0x7020
#define LAN9646_PORT7_STATUS                                   0x7030
#define LAN9646_PORT7_SGMII_ADDR                               0x7200
#define LAN9646_PORT7_SGMII_DATA                               0x7206
#define LAN9646_PORT7_XMII_CTRL0                               0x7300
#define LAN9646_PORT7_XMII_CTRL1                               0x7301
#define LAN9646_PORT7_MAC_CTRL0                                0x7400
#define LAN9646_PORT7_MAC_CTRL1                                0x7401
#define LAN9646_PORT7_IG_RATE_LIMIT_CTRL                       0x7403
#define LAN9646_PORT7_PRIO0_IG_LIMIT_CTRL                      0x7410
#define LAN9646_PORT7_PRIO1_IG_LIMIT_CTRL                      0x7411
#define LAN9646_PORT7_PRIO2_IG_LIMIT_CTRL                      0x7412
#define LAN9646_PORT7_PRIO3_IG_LIMIT_CTRL                      0x7413
#define LAN9646_PORT7_PRIO4_IG_LIMIT_CTRL                      0x7414
#define LAN9646_PORT7_PRIO5_IG_LIMIT_CTRL                      0x7415
#define LAN9646_PORT7_PRIO6_IG_LIMIT_CTRL                      0x7416
#define LAN9646_PORT7_PRIO7_IG_LIMIT_CTRL                      0x7417
#define LAN9646_PORT7_QUEUE0_EG_LIMIT_CTRL                     0x7420
#define LAN9646_PORT7_QUEUE1_EG_LIMIT_CTRL                     0x7421
#define LAN9646_PORT7_QUEUE2_EG_LIMIT_CTRL                     0x7422
#define LAN9646_PORT7_QUEUE3_EG_LIMIT_CTRL                     0x7423
#define LAN9646_PORT7_MIB_CTRL_STAT                            0x7500
#define LAN9646_PORT7_MIB_DATA                                 0x7504
#define LAN9646_PORT7_ACL_ACCESS0                              0x7600
#define LAN9646_PORT7_ACL_ACCESS1                              0x7601
#define LAN9646_PORT7_ACL_ACCESS2                              0x7602
#define LAN9646_PORT7_ACL_ACCESS3                              0x7603
#define LAN9646_PORT7_ACL_ACCESS4                              0x7604
#define LAN9646_PORT7_ACL_ACCESS5                              0x7605
#define LAN9646_PORT7_ACL_ACCESS6                              0x7606
#define LAN9646_PORT7_ACL_ACCESS7                              0x7607
#define LAN9646_PORT7_ACL_ACCESS8                              0x7608
#define LAN9646_PORT7_ACL_ACCESS9                              0x7609
#define LAN9646_PORT7_ACL_ACCESS10                             0x760A
#define LAN9646_PORT7_ACL_ACCESS11                             0x760B
#define LAN9646_PORT7_ACL_ACCESS12                             0x760C
#define LAN9646_PORT7_ACL_ACCESS13                             0x760D
#define LAN9646_PORT7_ACL_ACCESS14                             0x760E
#define LAN9646_PORT7_ACL_ACCESS15                             0x760F
#define LAN9646_PORT7_ACL_BYTE_EN_MSB                          0x7610
#define LAN9646_PORT7_ACL_BYTE_EN_LSB                          0x7611
#define LAN9646_PORT7_ACL_ACCESS_CTRL0                         0x7612
#define LAN9646_PORT7_MIRRORING_CTRL                           0x7800
#define LAN9646_PORT7_PRIO_CTRL                                0x7801
#define LAN9646_PORT7_IG_MAC_CTRL                              0x7802
#define LAN9646_PORT7_AUTH_CTRL                                0x7803
#define LAN9646_PORT7_PTR                                      0x7804
#define LAN9646_PORT7_PRIO_TO_QUEUE_MAPPING                    0x7808
#define LAN9646_PORT7_POLICE_CTRL                              0x780C
#define LAN9646_PORT7_POLICE_QUEUE_RATE                        0x7820
#define LAN9646_PORT7_POLICE_QUEUE_BURST_SIZE                  0x7824
#define LAN9646_PORT7_WRED_PKT_MEM_CTRL0                       0x7830
#define LAN9646_PORT7_WRED_PKT_MEM_CTRL1                       0x7834
#define LAN9646_PORT7_WRED_QUEUE_CTRL0                         0x7840
#define LAN9646_PORT7_WRED_QUEUE_CTRL1                         0x7844
#define LAN9646_PORT7_WRED_QUEUE_PERF_MON_CTRL                 0x7848
#define LAN9646_PORT7_TX_QUEUE_INDEX                           0x7900
#define LAN9646_PORT7_TX_QUEUE_PVID                            0x7904
#define LAN9646_PORT7_TX_QUEUE_CTRL0                           0x7914
#define LAN9646_PORT7_TX_QUEUE_CTRL1                           0x7915
#define LAN9646_PORT7_CTRL0                                    0x7A00
#define LAN9646_PORT7_CTRL1                                    0x7A04
#define LAN9646_PORT7_CTRL2                                    0x7B00
#define LAN9646_PORT7_MSTP_PTR                                 0x7B01
#define LAN9646_PORT7_MSTP_STATE                               0x7B04

//LAN9646 Switch register access macros
#define LAN9646_PORTn_DEFAULT_TAG0(port)                       (0x0000 + ((port) * 0x1000))
#define LAN9646_PORTn_DEFAULT_TAG1(port)                       (0x0001 + ((port) * 0x1000))
#define LAN9646_PORTn_PME_WOL_EVENT(port)                      (0x0013 + ((port) * 0x1000))
#define LAN9646_PORTn_PME_WOL_EN(port)                         (0x0017 + ((port) * 0x1000))
#define LAN9646_PORTn_INT_STATUS(port)                         (0x001B + ((port) * 0x1000))
#define LAN9646_PORTn_INT_MASK(port)                           (0x001F + ((port) * 0x1000))
#define LAN9646_PORTn_OP_CTRL0(port)                           (0x0020 + ((port) * 0x1000))
#define LAN9646_PORTn_STATUS(port)                             (0x0030 + ((port) * 0x1000))
#define LAN9646_PORTn_XMII_CTRL0(port)                         (0x0300 + ((port) * 0x1000))
#define LAN9646_PORTn_XMII_CTRL1(port)                         (0x0301 + ((port) * 0x1000))
#define LAN9646_PORTn_MAC_CTRL0(port)                          (0x0400 + ((port) * 0x1000))
#define LAN9646_PORTn_MAC_CTRL1(port)                          (0x0401 + ((port) * 0x1000))
#define LAN9646_PORTn_IG_RATE_LIMIT_CTRL(port)                 (0x0403 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO0_IG_LIMIT_CTRL(port)                (0x0410 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO1_IG_LIMIT_CTRL(port)                (0x0411 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO2_IG_LIMIT_CTRL(port)                (0x0412 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO3_IG_LIMIT_CTRL(port)                (0x0413 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO4_IG_LIMIT_CTRL(port)                (0x0414 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO5_IG_LIMIT_CTRL(port)                (0x0415 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO6_IG_LIMIT_CTRL(port)                (0x0416 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO7_IG_LIMIT_CTRL(port)                (0x0417 + ((port) * 0x1000))
#define LAN9646_PORTn_QUEUE0_EG_LIMIT_CTRL(port)               (0x0420 + ((port) * 0x1000))
#define LAN9646_PORTn_QUEUE1_EG_LIMIT_CTRL(port)               (0x0421 + ((port) * 0x1000))
#define LAN9646_PORTn_QUEUE2_EG_LIMIT_CTRL(port)               (0x0422 + ((port) * 0x1000))
#define LAN9646_PORTn_QUEUE3_EG_LIMIT_CTRL(port)               (0x0423 + ((port) * 0x1000))
#define LAN9646_PORTn_MIB_CTRL_STAT(port)                      (0x0500 + ((port) * 0x1000))
#define LAN9646_PORTn_MIB_DATA(port)                           (0x0504 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS0(port)                        (0x0600 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS1(port)                        (0x0601 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS2(port)                        (0x0602 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS3(port)                        (0x0603 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS4(port)                        (0x0604 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS5(port)                        (0x0605 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS6(port)                        (0x0606 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS7(port)                        (0x0607 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS8(port)                        (0x0608 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS9(port)                        (0x0609 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS10(port)                       (0x060A + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS11(port)                       (0x060B + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS12(port)                       (0x060C + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS13(port)                       (0x060D + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS14(port)                       (0x060E + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS15(port)                       (0x060F + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_BYTE_EN_MSB(port)                    (0x0610 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_BYTE_EN_LSB(port)                    (0x0611 + ((port) * 0x1000))
#define LAN9646_PORTn_ACL_ACCESS_CTRL0(port)                   (0x0612 + ((port) * 0x1000))
#define LAN9646_PORTn_MIRRORING_CTRL(port)                     (0x0800 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO_CTRL(port)                          (0x0801 + ((port) * 0x1000))
#define LAN9646_PORTn_IG_MAC_CTRL(port)                        (0x0802 + ((port) * 0x1000))
#define LAN9646_PORTn_AUTH_CTRL(port)                          (0x0803 + ((port) * 0x1000))
#define LAN9646_PORTn_PTR(port)                                (0x0804 + ((port) * 0x1000))
#define LAN9646_PORTn_PRIO_TO_QUEUE_MAPPING(port)              (0x0808 + ((port) * 0x1000))
#define LAN9646_PORTn_POLICE_CTRL(port)                        (0x080C + ((port) * 0x1000))
#define LAN9646_PORTn_POLICE_QUEUE_RATE(port)                  (0x0820 + ((port) * 0x1000))
#define LAN9646_PORTn_POLICE_QUEUE_BURST_SIZE(port)            (0x0824 + ((port) * 0x1000))
#define LAN9646_PORTn_WRED_PKT_MEM_CTRL0(port)                 (0x0830 + ((port) * 0x1000))
#define LAN9646_PORTn_WRED_PKT_MEM_CTRL1(port)                 (0x0834 + ((port) * 0x1000))
#define LAN9646_PORTn_WRED_QUEUE_CTRL0(port)                   (0x0840 + ((port) * 0x1000))
#define LAN9646_PORTn_WRED_QUEUE_CTRL1(port)                   (0x0844 + ((port) * 0x1000))
#define LAN9646_PORTn_WRED_QUEUE_PERF_MON_CTRL(port)           (0x0848 + ((port) * 0x1000))
#define LAN9646_PORTn_TX_QUEUE_INDEX(port)                     (0x0900 + ((port) * 0x1000))
#define LAN9646_PORTn_TX_QUEUE_PVID(port)                      (0x0904 + ((port) * 0x1000))
#define LAN9646_PORTn_TX_QUEUE_CTRL0(port)                     (0x0914 + ((port) * 0x1000))
#define LAN9646_PORTn_TX_QUEUE_CTRL1(port)                     (0x0915 + ((port) * 0x1000))
#define LAN9646_PORTn_CTRL0(port)                              (0x0A00 + ((port) * 0x1000))
#define LAN9646_PORTn_CTRL1(port)                              (0x0A04 + ((port) * 0x1000))
#define LAN9646_PORTn_CTRL2(port)                              (0x0B00 + ((port) * 0x1000))
#define LAN9646_PORTn_MSTP_PTR(port)                           (0x0B01 + ((port) * 0x1000))
#define LAN9646_PORTn_MSTP_STATE(port)                         (0x0B04 + ((port) * 0x1000))
#define LAN9646_PORTn_ETH_PHY_REG(port, addr)                  (0x0100 + ((port) * 0x1000) + ((addr) * 2))

//PHY Basic Control register
#define LAN9646_BMCR_RESET                                     0x8000
#define LAN9646_BMCR_LOOPBACK                                  0x4000
#define LAN9646_BMCR_SPEED_SEL_LSB                             0x2000
#define LAN9646_BMCR_AN_EN                                     0x1000
#define LAN9646_BMCR_POWER_DOWN                                0x0800
#define LAN9646_BMCR_ISOLATE                                   0x0400
#define LAN9646_BMCR_RESTART_AN                                0x0200
#define LAN9646_BMCR_DUPLEX_MODE                               0x0100
#define LAN9646_BMCR_COL_TEST                                  0x0080
#define LAN9646_BMCR_SPEED_SEL_MSB                             0x0040

//PHY Basic Status register
#define LAN9646_BMSR_100BT4                                    0x8000
#define LAN9646_BMSR_100BTX_FD                                 0x4000
#define LAN9646_BMSR_100BTX_HD                                 0x2000
#define LAN9646_BMSR_10BT_FD                                   0x1000
#define LAN9646_BMSR_10BT_HD                                   0x0800
#define LAN9646_BMSR_EXTENDED_STATUS                           0x0100
#define LAN9646_BMSR_MF_PREAMBLE_SUPPR                         0x0040
#define LAN9646_BMSR_AN_COMPLETE                               0x0020
#define LAN9646_BMSR_REMOTE_FAULT                              0x0010
#define LAN9646_BMSR_AN_CAPABLE                                0x0008
#define LAN9646_BMSR_LINK_STATUS                               0x0004
#define LAN9646_BMSR_JABBER_DETECT                             0x0002
#define LAN9646_BMSR_EXTENDED_CAPABLE                          0x0001

//PHY ID High register
#define LAN9646_PHYID1_DEFAULT                                 0x0022

//PHY ID Low register
#define LAN9646_PHYID2_DEFAULT                                 0x1631

//PHY Auto-Negotiation Advertisement register
#define LAN9646_ANAR_NEXT_PAGE                                 0x8000
#define LAN9646_ANAR_REMOTE_FAULT                              0x2000
#define LAN9646_ANAR_PAUSE                                     0x0C00
#define LAN9646_ANAR_100BT4                                    0x0200
#define LAN9646_ANAR_100BTX_FD                                 0x0100
#define LAN9646_ANAR_100BTX_HD                                 0x0080
#define LAN9646_ANAR_10BT_FD                                   0x0040
#define LAN9646_ANAR_10BT_HD                                   0x0020
#define LAN9646_ANAR_SELECTOR                                  0x001F
#define LAN9646_ANAR_SELECTOR_DEFAULT                          0x0001

//PHY Auto-Negotiation Link Partner Ability register
#define LAN9646_ANLPAR_NEXT_PAGE                               0x8000
#define LAN9646_ANLPAR_ACK                                     0x4000
#define LAN9646_ANLPAR_REMOTE_FAULT                            0x2000
#define LAN9646_ANLPAR_PAUSE                                   0x0C00
#define LAN9646_ANLPAR_100BT4                                  0x0200
#define LAN9646_ANLPAR_100BTX_FD                               0x0100
#define LAN9646_ANLPAR_100BTX_HD                               0x0080
#define LAN9646_ANLPAR_10BT_FD                                 0x0040
#define LAN9646_ANLPAR_10BT_HD                                 0x0020
#define LAN9646_ANLPAR_SELECTOR                                0x001F
#define LAN9646_ANLPAR_SELECTOR_DEFAULT                        0x0001

//PHY Auto-Negotiation Expansion Status register
#define LAN9646_ANER_PAR_DETECT_FAULT                          0x0010
#define LAN9646_ANER_LP_NEXT_PAGE_ABLE                         0x0008
#define LAN9646_ANER_NEXT_PAGE_ABLE                            0x0004
#define LAN9646_ANER_PAGE_RECEIVED                             0x0002
#define LAN9646_ANER_LP_AN_ABLE                                0x0001

//PHY Auto-Negotiation Next Page register
#define LAN9646_ANNPR_NEXT_PAGE                                0x8000
#define LAN9646_ANNPR_MSG_PAGE                                 0x2000
#define LAN9646_ANNPR_ACK2                                     0x1000
#define LAN9646_ANNPR_TOGGLE                                   0x0800
#define LAN9646_ANNPR_MESSAGE                                  0x07FF

//PHY Auto-Negotiation Link Partner Next Page Ability register
#define LAN9646_ANLPNPR_NEXT_PAGE                              0x8000
#define LAN9646_ANLPNPR_ACK                                    0x4000
#define LAN9646_ANLPNPR_MSG_PAGE                               0x2000
#define LAN9646_ANLPNPR_ACK2                                   0x1000
#define LAN9646_ANLPNPR_TOGGLE                                 0x0800
#define LAN9646_ANLPNPR_MESSAGE                                0x07FF

//PHY 1000BASE-T Control register
#define LAN9646_GBCR_TEST_MODE                                 0xE000
#define LAN9646_GBCR_MS_MAN_CONF_EN                            0x1000
#define LAN9646_GBCR_MS_MAN_CONF_VAL                           0x0800
#define LAN9646_GBCR_PORT_TYPE                                 0x0400
#define LAN9646_GBCR_1000BT_FD                                 0x0200
#define LAN9646_GBCR_1000BT_HD                                 0x0100

//PHY 1000BASE-T Status register
#define LAN9646_GBSR_MS_CONF_FAULT                             0x8000
#define LAN9646_GBSR_MS_CONF_RES                               0x4000
#define LAN9646_GBSR_LOCAL_RECEIVER_STATUS                     0x2000
#define LAN9646_GBSR_REMOTE_RECEIVER_STATUS                    0x1000
#define LAN9646_GBSR_LP_1000BT_FD                              0x0800
#define LAN9646_GBSR_LP_1000BT_HD                              0x0400
#define LAN9646_GBSR_IDLE_ERR_COUNT                            0x00FF

//PHY MMD Setup register
#define LAN9646_MMDACR_FUNC                                    0xC000
#define LAN9646_MMDACR_FUNC_ADDR                               0x0000
#define LAN9646_MMDACR_FUNC_DATA_NO_POST_INC                   0x4000
#define LAN9646_MMDACR_FUNC_DATA_POST_INC_RW                   0x8000
#define LAN9646_MMDACR_FUNC_DATA_POST_INC_W                    0xC000
#define LAN9646_MMDACR_DEVAD                                   0x001F

//PHY Extended Status register
#define LAN9646_GBESR_1000BX_FD                                0x8000
#define LAN9646_GBESR_1000BX_HD                                0x4000
#define LAN9646_GBESR_1000BT_FD                                0x2000
#define LAN9646_GBESR_1000BT_HD                                0x1000

//PHY Remote Loopback register
#define LAN9646_RLB_REMOTE_LOOPBACK                            0x0100

//PHY LinkMD register
#define LAN9646_LINKMD_TEST_EN                                 0x8000
#define LAN9646_LINKMD_PAIR                                    0x3000
#define LAN9646_LINKMD_PAIR_A                                  0x0000
#define LAN9646_LINKMD_PAIR_B                                  0x1000
#define LAN9646_LINKMD_PAIR_C                                  0x2000
#define LAN9646_LINKMD_PAIR_D                                  0x3000
#define LAN9646_LINKMD_STATUS                                  0x0300
#define LAN9646_LINKMD_STATUS_NORMAL                           0x0000
#define LAN9646_LINKMD_STATUS_OPEN                             0x0100
#define LAN9646_LINKMD_STATUS_SHORT                            0x0200
#define LAN9646_LINKMD_RESULT                                  0x00FF

//PHY Digital PMA/PCS Status register
#define LAN9646_DPMAPCSS_1000BT_LINK_STATUS                    0x0002
#define LAN9646_DPMAPCSS_100BTX_LINK_STATUS                    0x0001

//Port Interrupt Control/Status register
#define LAN9646_ICSR_JABBER_IE                                 0x8000
#define LAN9646_ICSR_RECEIVE_ERROR_IE                          0x4000
#define LAN9646_ICSR_PAGE_RECEIVED_IE                          0x2000
#define LAN9646_ICSR_PAR_DETECT_FAULT_IE                       0x1000
#define LAN9646_ICSR_LP_ACK_IE                                 0x0800
#define LAN9646_ICSR_LINK_DOWN_IE                              0x0400
#define LAN9646_ICSR_REMOTE_FAULT_IE                           0x0200
#define LAN9646_ICSR_LINK_UP_IE                                0x0100
#define LAN9646_ICSR_JABBER_IF                                 0x0080
#define LAN9646_ICSR_RECEIVE_ERROR_IF                          0x0040
#define LAN9646_ICSR_PAGE_RECEIVED_IF                          0x0020
#define LAN9646_ICSR_PAR_DETECT_FAULT_IF                       0x0010
#define LAN9646_ICSR_LP_ACK_IF                                 0x0008
#define LAN9646_ICSR_LINK_DOWN_IF                              0x0004
#define LAN9646_ICSR_REMOTE_FAULT_IF                           0x0002
#define LAN9646_ICSR_LINK_UP_IF                                0x0001

//PHY Auto MDI/MDI-X register
#define LAN9646_AUTOMDI_MDI_SET                                0x0080
#define LAN9646_AUTOMDI_SWAP_OFF                               0x0040

//PHY Control register
#define LAN9646_PHYCON_JABBER_EN                               0x0200
#define LAN9646_PHYCON_SPEED_1000BT                            0x0040
#define LAN9646_PHYCON_SPEED_100BTX                            0x0020
#define LAN9646_PHYCON_SPEED_10BT                              0x0010
#define LAN9646_PHYCON_DUPLEX_STATUS                           0x0008
#define LAN9646_PHYCON_1000BT_MS_STATUS                        0x0004

//MMD LED Mode register
#define LAN9646_MMD_LED_MODE_LED_MODE                          0x0010
#define LAN9646_MMD_LED_MODE_LED_MODE_TRI_COLOR_DUAL           0x0000
#define LAN9646_MMD_LED_MODE_LED_MODE_SINGLE                   0x0010
#define LAN9646_MMD_LED_MODE_RESERVED                          0x000F
#define LAN9646_MMD_LED_MODE_RESERVED_DEFAULT                  0x0001

//MMD EEE Advertisement register
#define LAN9646_MMD_EEE_ADV_1000BT_EEE_EN                      0x0004
#define LAN9646_MMD_EEE_ADV_100BT_EEE_EN                       0x0002

//Global Chip ID 0 register
#define LAN9646_CHIP_ID0_DEFAULT                               0x00

//Global Chip ID 1 register
#define LAN9646_CHIP_ID1_DEFAULT                               0x94

//Global Chip ID 2 register
#define LAN9646_CHIP_ID2_DEFAULT                               0x77

//Global Chip ID 3 register
#define LAN9646_CHIP_ID3_REVISION_ID                           0xF0
#define LAN9646_CHIP_ID3_GLOBAL_SOFT_RESET                     0x01

//PME Pin Control register
#define LAN9646_PME_PIN_CTRL_PME_PIN_OUT_EN                    0x02
#define LAN9646_PME_PIN_CTRL_PME_PIN_OUT_POL                   0x01

//Global Interrupt Status register
#define LAN9646_GLOBAL_INT_STAT_LUE                            0x80000000

//Global Interrupt Mask register
#define LAN9646_GLOBAL_INT_MASK_LUE                            0x80000000

//Global Port Interrupt Status register
#define LAN9646_GLOBAL_PORT_INT_STAT_PORT7                     0x00000040
#define LAN9646_GLOBAL_PORT_INT_STAT_PORT6                     0x00000020
#define LAN9646_GLOBAL_PORT_INT_STAT_PORT4                     0x00000008
#define LAN9646_GLOBAL_PORT_INT_STAT_PORT3                     0x00000004
#define LAN9646_GLOBAL_PORT_INT_STAT_PORT2                     0x00000002
#define LAN9646_GLOBAL_PORT_INT_STAT_PORT1                     0x00000001

//Global Port Interrupt Mask register
#define LAN9646_GLOBAL_PORT_INT_MASK_PORT7                     0x00000040
#define LAN9646_GLOBAL_PORT_INT_MASK_PORT6                     0x00000020
#define LAN9646_GLOBAL_PORT_INT_MASK_PORT4                     0x00000008
#define LAN9646_GLOBAL_PORT_INT_MASK_PORT3                     0x00000004
#define LAN9646_GLOBAL_PORT_INT_MASK_PORT2                     0x00000002
#define LAN9646_GLOBAL_PORT_INT_MASK_PORT1                     0x00000001

//Serial I/O Control register
#define LAN9646_SERIAL_IO_CTRL_MIIM_PREAMBLE_SUPPR             0x04
#define LAN9646_SERIAL_IO_CTRL_AUTO_SPI_DATA_OUT_EDGE_SEL      0x02
#define LAN9646_SERIAL_IO_CTRL_SPI_DATA_OUT_EDGE_SEL           0x01
#define LAN9646_SERIAL_IO_CTRL_SPI_DATA_OUT_EDGE_SEL_FALLING   0x00
#define LAN9646_SERIAL_IO_CTRL_SPI_DATA_OUT_EDGE_SEL_RISING    0x01

//Output Clock Control register
#define LAN9646_OUT_CLK_CTRL_CLKO_25_125_EN                    0x02
#define LAN9646_OUT_CLK_CTRL_CLKO_25_125_FREQ                  0x01
#define LAN9646_OUT_CLK_CTRL_CLKO_25_125_FREQ_25MHZ            0x00
#define LAN9646_OUT_CLK_CTRL_CLKO_25_125_FREQ_125MHZ           0x01

//In-Band Management Control register
#define LAN9646_IBA_CTRL_IBA_EN                                0x80000000
#define LAN9646_IBA_CTRL_DEST_MAC_ADDR_MATCH_EN                0x40000000
#define LAN9646_IBA_CTRL_IBA_RESET                             0x20000000
#define LAN9646_IBA_CTRL_RESP_PRIO_QUEUE                       0x00C00000
#define LAN9646_IBA_CTRL_RESP_PRIO_QUEUE_DEFAULT               0x00400000
#define LAN9646_IBA_CTRL_IBA_COMM                              0x00070000
#define LAN9646_IBA_CTRL_IBA_COMM_PORT1                        0x00000000
#define LAN9646_IBA_CTRL_IBA_COMM_PORT2                        0x00010000
#define LAN9646_IBA_CTRL_IBA_COMM_PORT3                        0x00020000
#define LAN9646_IBA_CTRL_IBA_COMM_PORT4                        0x00030000
#define LAN9646_IBA_CTRL_IBA_COMM_PORT6                        0x00050000
#define LAN9646_IBA_CTRL_IBA_COMM_PORT7                        0x00060000
#define LAN9646_IBA_CTRL_TPID                                  0x0000FFFF
#define LAN9646_IBA_CTRL_TPID_DEFAULT                          0x000040FE

//I/O Drive Strength register
#define LAN9646_IO_DRIVE_STRENGTH_HIGH_SPEED_DRIVE_STRENGTH    0x70
#define LAN9646_IO_DRIVE_STRENGTH_LOW_SPEED_DRIVE_STRENGTH     0x07

//In-Band Management Operation Status 1 register
#define LAN9646_IBA_OP_STAT1_GOOD_PKT_DETECT                   0x80000000
#define LAN9646_IBA_OP_STAT1_RESP_PKT_TX_DONE                  0x40000000
#define LAN9646_IBA_OP_STAT1_EXEC_DONE                         0x20000000
#define LAN9646_IBA_OP_STAT1_MAC_ADDR_MISMATCH_ERR             0x00004000
#define LAN9646_IBA_OP_STAT1_ACCESS_FORMAT_ERR                 0x00002000
#define LAN9646_IBA_OP_STAT1_ACCESS_CODE_ERR                   0x00001000
#define LAN9646_IBA_OP_STAT1_ACCESS_CMD_ERR                    0x00000800
#define LAN9646_IBA_OP_STAT1_OVERSIZE_PKT_ERR                  0x00000400
#define LAN9646_IBA_OP_STAT1_ACCESS_CODE_ERR_LOC               0x0000007F

//LED Override register
#define LAN9646_LED_OVERRIDE_OVERRIDE                          0x000003FF
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED1_0                   0x00000001
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED1_1                   0x00000002
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED2_0                   0x00000004
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED2_1                   0x00000008
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED3_0                   0x00000010
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED3_1                   0x00000020
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED4_0                   0x00000040
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED4_1                   0x00000080
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED5_0                   0x00000100
#define LAN9646_LED_OVERRIDE_OVERRIDE_LED5_1                   0x00000200

//LED Output register
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL                       0x000003FF
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED1_0                0x00000001
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED1_1                0x00000002
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED2_0                0x00000004
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED2_1                0x00000008
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED3_0                0x00000010
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED3_1                0x00000020
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED4_0                0x00000040
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED4_1                0x00000080
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED5_0                0x00000100
#define LAN9646_LED_OUTPUT_GPIO_OUT_CTRL_LED5_1                0x00000200

//Power Down Control 0 register
#define LAN9646_PWR_DOWN_CTRL0_PLL_PWR_DOWN                    0x20
#define LAN9646_PWR_DOWN_CTRL0_PWR_MGMT_MODE                   0x18
#define LAN9646_PWR_DOWN_CTRL0_PWR_MGMT_MODE_NORMAL            0x00
#define LAN9646_PWR_DOWN_CTRL0_PWR_MGMT_MODE_EDPD              0x08
#define LAN9646_PWR_DOWN_CTRL0_PWR_MGMT_MODE_SOFT_PWR_DOWN     0x10

//LED Strap-In register
#define LAN9646_LED_STRAP_IN_STRAP_IN                          0x000003FF
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED1_0                   0x00000001
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED1_1                   0x00000002
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED2_0                   0x00000004
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED2_1                   0x00000008
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED3_0                   0x00000010
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED3_1                   0x00000020
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED4_0                   0x00000040
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED4_1                   0x00000080
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED5_0                   0x00000100
#define LAN9646_LED_STRAP_IN_STRAP_IN_LED5_1                   0x00000200

//Switch Operation register
#define LAN9646_SWITCH_OP_DOUBLE_TAG_EN                        0x80
#define LAN9646_SWITCH_OP_SOFT_HARD_RESET                      0x02
#define LAN9646_SWITCH_OP_START_SWITCH                         0x01

//Switch Maximum Transmit Unit register
#define LAN9646_SWITCH_MTU_MTU                                 0x3FFF
#define LAN9646_SWITCH_MTU_MTU_DEFAULT                         0x07D0

//Switch Lookup Engine Control 0 register
#define LAN9646_SWITCH_LUE_CTRL0_VLAN_EN                       0x80
#define LAN9646_SWITCH_LUE_CTRL0_DROP_INVALID_VID              0x40
#define LAN9646_SWITCH_LUE_CTRL0_AGE_COUNT                     0x38
#define LAN9646_SWITCH_LUE_CTRL0_AGE_COUNT_DEFAULT             0x20
#define LAN9646_SWITCH_LUE_CTRL0_RESERVED_MCAST_LOOKUP_EN      0x04
#define LAN9646_SWITCH_LUE_CTRL0_HASH_OPTION                   0x03
#define LAN9646_SWITCH_LUE_CTRL0_HASH_OPTION_NONE              0x00
#define LAN9646_SWITCH_LUE_CTRL0_HASH_OPTION_CRC               0x01
#define LAN9646_SWITCH_LUE_CTRL0_HASH_OPTION_XOR               0x02

//Switch Lookup Engine Control 1 register
#define LAN9646_SWITCH_LUE_CTRL1_UNICAST_LEARNING_DIS          0x80
#define LAN9646_SWITCH_LUE_CTRL1_SELF_ADDR_FILT                0x40
#define LAN9646_SWITCH_LUE_CTRL1_FLUSH_ALU_TABLE               0x20
#define LAN9646_SWITCH_LUE_CTRL1_FLUSH_MSTP_ENTRIES            0x10
#define LAN9646_SWITCH_LUE_CTRL1_MCAST_SRC_ADDR_FILT           0x08
#define LAN9646_SWITCH_LUE_CTRL1_AGING_EN                      0x04
#define LAN9646_SWITCH_LUE_CTRL1_FAST_AGING                    0x02
#define LAN9646_SWITCH_LUE_CTRL1_LINK_DOWN_FLUSH               0x01

//Switch Lookup Engine Control 2 register
#define LAN9646_SWITCH_LUE_CTRL2_DOUBLE_TAG_MCAST_TRAP         0x40
#define LAN9646_SWITCH_LUE_CTRL2_DYNAMIC_ENTRY_EG_VLAN_FILT    0x20
#define LAN9646_SWITCH_LUE_CTRL2_STATIC_ENTRY_EG_VLAN_FILT     0x10
#define LAN9646_SWITCH_LUE_CTRL2_FLUSH_OPTION                  0x0C
#define LAN9646_SWITCH_LUE_CTRL2_FLUSH_OPTION_NONE             0x00
#define LAN9646_SWITCH_LUE_CTRL2_FLUSH_OPTION_DYNAMIC          0x04
#define LAN9646_SWITCH_LUE_CTRL2_FLUSH_OPTION_STATIC           0x08
#define LAN9646_SWITCH_LUE_CTRL2_FLUSH_OPTION_BOTH             0x0C
#define LAN9646_SWITCH_LUE_CTRL2_MAC_ADDR_PRIORITY             0x03

//Switch Lookup Engine Control 3 register
#define LAN9646_SWITCH_LUE_CTRL3_AGE_PERIOD                    0xFF
#define LAN9646_SWITCH_LUE_CTRL3_AGE_PERIOD_DEFAULT            0x4B

//Address Lookup Table Interrupt register
#define LAN9646_ALU_TABLE_INT_LEARN_FAIL                       0x04
#define LAN9646_ALU_TABLE_INT_ALMOST_FULL                      0x02
#define LAN9646_ALU_TABLE_INT_WRITE_FAIL                       0x01

//Address Lookup Table Mask register
#define LAN9646_ALU_TABLE_MASK_LEARN_FAIL                      0x04
#define LAN9646_ALU_TABLE_MASK_ALMOST_FULL                     0x02
#define LAN9646_ALU_TABLE_MASK_WRITE_FAIL                      0x01

//Address Lookup Table Entry Index 0 register
#define LAN9646_ALU_TABLE_ENTRY_INDEX0_ALMOST_FULL_ENTRY_INDEX 0x0FFF
#define LAN9646_ALU_TABLE_ENTRY_INDEX0_FAIL_WRITE_INDEX        0x03FF

//Address Lookup Table Entry Index 1 register
#define LAN9646_ALU_TABLE_ENTRY_INDEX1_FAIL_LEARN_INDEX        0x03FF

//Address Lookup Table Entry Index 2 register
#define LAN9646_ALU_TABLE_ENTRY_INDEX2_CPU_ACCESS_INDEX        0x03FF

//Unknown Unicast Control register
#define LAN9646_UNKNOWN_UNICAST_CTRL_FWD                       0x80000000
#define LAN9646_UNKNOWN_UNICAST_CTRL_FWD_MAP                   0x0000007F
#define LAN9646_UNKNOWN_UNICAST_CTRL_FWD_MAP_PORT1             0x00000001
#define LAN9646_UNKNOWN_UNICAST_CTRL_FWD_MAP_PORT2             0x00000002
#define LAN9646_UNKNOWN_UNICAST_CTRL_FWD_MAP_PORT3             0x00000004
#define LAN9646_UNKNOWN_UNICAST_CTRL_FWD_MAP_PORT4             0x00000008
#define LAN9646_UNKNOWN_UNICAST_CTRL_FWD_MAP_PORT6             0x00000020
#define LAN9646_UNKNOWN_UNICAST_CTRL_FWD_MAP_PORT7             0x00000040
#define LAN9646_UNKNOWN_UNICAST_CTRL_FWD_MAP_ALL               0x0000007F

//Unknown Multicast Control register
#define LAN9646_UNKONWN_MULTICAST_CTRL_FWD                     0x80000000
#define LAN9646_UNKONWN_MULTICAST_CTRL_FWD_MAP                 0x0000007F
#define LAN9646_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT1           0x00000001
#define LAN9646_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT2           0x00000002
#define LAN9646_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT3           0x00000004
#define LAN9646_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT4           0x00000008
#define LAN9646_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT6           0x00000020
#define LAN9646_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT7           0x00000040
#define LAN9646_UNKONWN_MULTICAST_CTRL_FWD_MAP_ALL             0x0000007F

//Unknown VLAN ID Control register
#define LAN9646_UNKNOWN_VLAN_ID_CTRL_FWD                       0x80000000
#define LAN9646_UNKNOWN_VLAN_ID_CTRL_FWD_MAP                   0x0000007F
#define LAN9646_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_PORT1             0x00000001
#define LAN9646_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_PORT2             0x00000002
#define LAN9646_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_PORT3             0x00000004
#define LAN9646_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_PORT4             0x00000008
#define LAN9646_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_PORT6             0x00000020
#define LAN9646_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_PORT7             0x00000040
#define LAN9646_UNKNOWN_VLAN_ID_CTRL_FWD_MAP_ALL               0x0000007F

//Switch MAC Control 0 register
#define LAN9646_SWITCH_MAC_CTRL0_ALT_BACK_OFF_MODE             0x80
#define LAN9646_SWITCH_MAC_CTRL0_FRAME_LEN_CHECK_EN            0x08
#define LAN9646_SWITCH_MAC_CTRL0_FLOW_CTRL_PKT_DROP_MODE       0x02
#define LAN9646_SWITCH_MAC_CTRL0_AGGRESSIVE_BACK_OFF_EN        0x01

//Switch MAC Control 1 register
#define LAN9646_SWITCH_MAC_CTRL1_MCAST_STORM_PROTECT_DIS       0x40
#define LAN9646_SWITCH_MAC_CTRL1_BACK_PRESSURE_MODE            0x20
#define LAN9646_SWITCH_MAC_CTRL1_FLOW_CTRL_FAIR_MODE           0x10
#define LAN9646_SWITCH_MAC_CTRL1_NO_EXCESSIVE_COL_DROP         0x08
#define LAN9646_SWITCH_MAC_CTRL1_JUMBO_PKT_SUPPORT             0x04
#define LAN9646_SWITCH_MAC_CTRL1_MAX_PKT_SIZE_CHECK_DIS        0x02
#define LAN9646_SWITCH_MAC_CTRL1_PASS_SHORT_PKT                0x01

//Switch MAC Control 2 register
#define LAN9646_SWITCH_MAC_CTRL2_NULL_VID_REPLACEMENT          0x08
#define LAN9646_SWITCH_MAC_CTRL2_BCAST_STORM_PROTECT_RATE_MSB  0x07

//Switch MAC Control 3 register
#define LAN9646_SWITCH_MAC_CTRL3_BCAST_STORM_PROTECT_RATE_LSB  0xFF

//Switch MAC Control 4 register
#define LAN9646_SWITCH_MAC_CTRL4_PASS_FLOW_CTRL_PKT            0x01

//Switch MAC Control 5 register
#define LAN9646_SWITCH_MAC_CTRL5_IG_RATE_LIMIT_PERIOD          0x30
#define LAN9646_SWITCH_MAC_CTRL5_IG_RATE_LIMIT_PERIOD_16MS     0x00
#define LAN9646_SWITCH_MAC_CTRL5_IG_RATE_LIMIT_PERIOD_64MS     0x10
#define LAN9646_SWITCH_MAC_CTRL5_IG_RATE_LIMIT_PERIOD_256MS    0x20
#define LAN9646_SWITCH_MAC_CTRL5_QUEUE_BASED_EG_RATE_LIMITE_EN 0x08

//Switch MIB Control register
#define LAN9646_SWITCH_MIB_CTRL_FLUSH                          0x80
#define LAN9646_SWITCH_MIB_CTRL_FREEZE                         0x40

//Global Port Mirroring and Snooping Control register
#define LAN9646_GLOBAL_PORT_MIRROR_SNOOP_CTRL_IGMP_SNOOP_EN    0x40
#define LAN9646_GLOBAL_PORT_MIRROR_SNOOP_CTRL_MLD_SNOOP_OPT    0x08
#define LAN9646_GLOBAL_PORT_MIRROR_SNOOP_CTRL_MLD_SNOOP_EN     0x04
#define LAN9646_GLOBAL_PORT_MIRROR_SNOOP_CTRL_SNIFF_MODE_SEL   0x01

//WRED DiffServ Color Mapping register
#define LAN9646_WRED_DIFFSERV_COLOR_MAPPING_RED                0x30
#define LAN9646_WRED_DIFFSERV_COLOR_MAPPING_YELLOW             0x0C
#define LAN9646_WRED_DIFFSERV_COLOR_MAPPING_GREEN              0x03

//Queue Management Control 0 register
#define LAN9646_QUEUE_MGMT_CTRL0_PRIORITY_2Q                   0x000000C0
#define LAN9646_QUEUE_MGMT_CTRL0_UNICAST_PORT_VLAN_DISCARD     0x00000002

//VLAN Table Entry 0 register
#define LAN9646_VLAN_TABLE_ENTRY0_VALID                        0x80000000
#define LAN9646_VLAN_TABLE_ENTRY0_FORWARD_OPTION               0x08000000
#define LAN9646_VLAN_TABLE_ENTRY0_PRIORITY                     0x07000000
#define LAN9646_VLAN_TABLE_ENTRY0_MSTP_INDEX                   0x00007000
#define LAN9646_VLAN_TABLE_ENTRY0_FID                          0x0000007F

//VLAN Table Entry 1 register
#define LAN9646_VLAN_TABLE_ENTRY1_PORT_UNTAG                   0x0000007F
#define LAN9646_VLAN_TABLE_ENTRY1_PORT7_UNTAG                  0x00000040
#define LAN9646_VLAN_TABLE_ENTRY1_PORT6_UNTAG                  0x00000020
#define LAN9646_VLAN_TABLE_ENTRY1_PORT4_UNTAG                  0x00000008
#define LAN9646_VLAN_TABLE_ENTRY1_PORT3_UNTAG                  0x00000004
#define LAN9646_VLAN_TABLE_ENTRY1_PORT2_UNTAG                  0x00000002
#define LAN9646_VLAN_TABLE_ENTRY1_PORT1_UNTAG                  0x00000001

//VLAN Table Entry 2 register
#define LAN9646_VLAN_TABLE_ENTRY2_PORT_FORWARD                 0x0000007F
#define LAN9646_VLAN_TABLE_ENTRY2_PORT7_FORWARD                0x00000040
#define LAN9646_VLAN_TABLE_ENTRY2_PORT6_FORWARD                0x00000020
#define LAN9646_VLAN_TABLE_ENTRY2_PORT4_FORWARD                0x00000008
#define LAN9646_VLAN_TABLE_ENTRY2_PORT3_FORWARD                0x00000004
#define LAN9646_VLAN_TABLE_ENTRY2_PORT2_FORWARD                0x00000002
#define LAN9646_VLAN_TABLE_ENTRY2_PORT1_FORWARD                0x00000001

//VLAN Table Index register
#define LAN9646_VLAN_TABLE_INDEX_VLAN_INDEX                    0x0FFF

//VLAN Table Access Control register
#define LAN9646_VLAN_TABLE_ACCESS_CTRL_START_FINISH            0x80
#define LAN9646_VLAN_TABLE_ACCESS_CTRL_ACTION                  0x03
#define LAN9646_VLAN_TABLE_ACCESS_CTRL_ACTION_NOP              0x00
#define LAN9646_VLAN_TABLE_ACCESS_CTRL_ACTION_WRITE            0x01
#define LAN9646_VLAN_TABLE_ACCESS_CTRL_ACTION_READ             0x02
#define LAN9646_VLAN_TABLE_ACCESS_CTRL_ACTION_CLEAR            0x03

//ALU Table Index 0 register
#define LAN9646_ALU_TABLE_INDEX0_FID_INDEX                     0x007F0000
#define LAN9646_ALU_TABLE_INDEX0_MAC_INDEX_MSB                 0x0000FFFF

//ALU Table Index 1 register
#define LAN9646_ALU_TABLE_INDEX1_MAC_INDEX_LSB                 0xFFFFFFFF

//ALU Table Access Control register
#define LAN9646_ALU_TABLE_CTRL_VALID_COUNT                     0x3FFF0000
#define LAN9646_ALU_TABLE_CTRL_START_FINISH                    0x00000080
#define LAN9646_ALU_TABLE_CTRL_VALID                           0x00000040
#define LAN9646_ALU_TABLE_CTRL_VALID_ENTRY_OR_SEARCH_END       0x00000020
#define LAN9646_ALU_TABLE_CTRL_DIRECT                          0x00000004
#define LAN9646_ALU_TABLE_CTRL_ACTION                          0x00000003
#define LAN9646_ALU_TABLE_CTRL_ACTION_NOP                      0x00000000
#define LAN9646_ALU_TABLE_CTRL_ACTION_WRITE                    0x00000001
#define LAN9646_ALU_TABLE_CTRL_ACTION_READ                     0x00000002
#define LAN9646_ALU_TABLE_CTRL_ACTION_SEARCH                   0x00000003

//Static Address and Reserved Multicast Table Control register
#define LAN9646_STATIC_MCAST_TABLE_CTRL_TABLE_INDEX            0x003F0000
#define LAN9646_STATIC_MCAST_TABLE_CTRL_START_FINISH           0x00000080
#define LAN9646_STATIC_MCAST_TABLE_CTRL_TABLE_SELECT           0x00000002
#define LAN9646_STATIC_MCAST_TABLE_CTRL_ACTION                 0x00000001
#define LAN9646_STATIC_MCAST_TABLE_CTRL_ACTION_READ            0x00000000
#define LAN9646_STATIC_MCAST_TABLE_CTRL_ACTION_WRITE           0x00000001

//ALU Table Entry 1 register
#define LAN9646_ALU_TABLE_ENTRY1_STATIC                        0x80000000
#define LAN9646_ALU_TABLE_ENTRY1_SRC_FILTER                    0x40000000
#define LAN9646_ALU_TABLE_ENTRY1_DES_FILTER                    0x20000000
#define LAN9646_ALU_TABLE_ENTRY1_PRIORITY                      0x1C000000
#define LAN9646_ALU_TABLE_ENTRY1_AGE_COUNT                     0x1C000000
#define LAN9646_ALU_TABLE_ENTRY1_MSTP                          0x00000007

//ALU Table Entry 2 register
#define LAN9646_ALU_TABLE_ENTRY2_OVERRIDE                      0x80000000
#define LAN9646_ALU_TABLE_ENTRY2_PORT_FORWARD                  0x0000007F
#define LAN9646_ALU_TABLE_ENTRY2_PORT7_FORWARD                 0x00000040
#define LAN9646_ALU_TABLE_ENTRY2_PORT6_FORWARD                 0x00000020
#define LAN9646_ALU_TABLE_ENTRY2_PORT4_FORWARD                 0x00000008
#define LAN9646_ALU_TABLE_ENTRY2_PORT3_FORWARD                 0x00000004
#define LAN9646_ALU_TABLE_ENTRY2_PORT2_FORWARD                 0x00000002
#define LAN9646_ALU_TABLE_ENTRY2_PORT1_FORWARD                 0x00000001

//ALU Table Entry 3 register
#define LAN9646_ALU_TABLE_ENTRY3_FID                           0x007F0000
#define LAN9646_ALU_TABLE_ENTRY3_MAC_ADDR_MSB                  0x0000FFFF

//ALU Table Entry 4 register
#define LAN9646_ALU_TABLE_ENTRY4_MAC_ADDR_LSB                  0xFFFFFFFF

//Static Address Table Entry 1 register
#define LAN9646_STATIC_TABLE_ENTRY1_VALID                      0x80000000
#define LAN9646_STATIC_TABLE_ENTRY1_SRC_FILTER                 0x40000000
#define LAN9646_STATIC_TABLE_ENTRY1_DES_FILTER                 0x20000000
#define LAN9646_STATIC_TABLE_ENTRY1_PRIORITY                   0x1C000000
#define LAN9646_STATIC_TABLE_ENTRY1_MSTP                       0x00000007

//Static Address Table Entry 2 register
#define LAN9646_STATIC_TABLE_ENTRY2_OVERRIDE                   0x80000000
#define LAN9646_STATIC_TABLE_ENTRY2_USE_FID                    0x40000000
#define LAN9646_STATIC_TABLE_ENTRY2_PORT_FORWARD               0x0000007F
#define LAN9646_STATIC_TABLE_ENTRY2_PORT7_FORWARD              0x00000040
#define LAN9646_STATIC_TABLE_ENTRY2_PORT6_FORWARD              0x00000020
#define LAN9646_STATIC_TABLE_ENTRY2_PORT4_FORWARD              0x00000008
#define LAN9646_STATIC_TABLE_ENTRY2_PORT3_FORWARD              0x00000004
#define LAN9646_STATIC_TABLE_ENTRY2_PORT2_FORWARD              0x00000002
#define LAN9646_STATIC_TABLE_ENTRY2_PORT1_FORWARD              0x00000001

//Static Address Table Entry 3 register
#define LAN9646_STATIC_TABLE_ENTRY3_FID                        0x007F0000
#define LAN9646_STATIC_TABLE_ENTRY3_MAC_ADDR_MSB               0x0000FFFF

//Static Address Table Entry 4 register
#define LAN9646_STATIC_TABLE_ENTRY4_MAC_ADDR_LSB               0xFFFFFFFF

//Reserved Multicast Table Entry 2 register
#define LAN9646_RES_MCAST_TABLE_ENTRY2_PORT_FORWARD            0x0000007F
#define LAN9646_RES_MCAST_TABLE_ENTRY2_PORT7_FORWARD           0x00000040
#define LAN9646_RES_MCAST_TABLE_ENTRY2_PORT6_FORWARD           0x00000020
#define LAN9646_RES_MCAST_TABLE_ENTRY2_PORT4_FORWARD           0x00000008
#define LAN9646_RES_MCAST_TABLE_ENTRY2_PORT3_FORWARD           0x00000004
#define LAN9646_RES_MCAST_TABLE_ENTRY2_PORT2_FORWARD           0x00000002
#define LAN9646_RES_MCAST_TABLE_ENTRY2_PORT1_FORWARD           0x00000001

//Port N Default Tag 0 register
#define LAN9646_PORTn_DEFAULT_TAG0_PCP                         0xE0
#define LAN9646_PORTn_DEFAULT_TAG0_DEI                         0x10
#define LAN9646_PORTn_DEFAULT_TAG0_VID_MSB                     0x0F

//Port N Default Tag 1 register
#define LAN9646_PORTn_DEFAULT_TAG1_VID_LSB                     0xFF

//Port N Interrupt Status register
#define LAN9646_PORTn_INT_STATUS_SGMII_AN_DONE                 0x08
#define LAN9646_PORTn_INT_STATUS_PHY                           0x02
#define LAN9646_PORTn_INT_STATUS_ACL                           0x01

//Port N Interrupt Mask register
#define LAN9646_PORTn_INT_MASK_SGMII_AN_DONE                   0x08
#define LAN9646_PORTn_INT_MASK_PHY                             0x02
#define LAN9646_PORTn_INT_MASK_ACL                             0x01

//Port N Operation Control 0 register
#define LAN9646_PORTn_OP_CTRL0_LOCAL_LOOPBACK                  0x80
#define LAN9646_PORTn_OP_CTRL0_REMOTE_LOOPBACK                 0x40
#define LAN9646_PORTn_OP_CTRL0_TAIL_TAG_EN                     0x04
#define LAN9646_PORTn_OP_CTRL0_TX_QUEUE_SPLIT_EN               0x03

//Port N Status register
#define LAN9646_PORTn_STATUS_SPEED                             0x18
#define LAN9646_PORTn_STATUS_SPEED_10MBPS                      0x00
#define LAN9646_PORTn_STATUS_SPEED_100MBPS                     0x08
#define LAN9646_PORTn_STATUS_SPEED_1000MBPS                    0x10
#define LAN9646_PORTn_STATUS_DUPLEX                            0x04
#define LAN9646_PORTn_STATUS_TX_FLOW_CTRL_EN                   0x02
#define LAN9646_PORTn_STATUS_RX_FLOW_CTRL_EN                   0x01

//XMII Port N Control 0 register
#define LAN9646_PORTn_XMII_CTRL0_DUPLEX                        0x40
#define LAN9646_PORTn_XMII_CTRL0_TX_FLOW_CTRL_EN               0x20
#define LAN9646_PORTn_XMII_CTRL0_SPEED_10_100                  0x10
#define LAN9646_PORTn_XMII_CTRL0_RX_FLOW_CTRL_EN               0x08

//XMII Port N Control 1 register
#define LAN9646_PORTn_XMII_CTRL1_SPEED_1000                    0x40
#define LAN9646_PORTn_XMII_CTRL1_RGMII_ID_IG                   0x10
#define LAN9646_PORTn_XMII_CTRL1_RGMII_ID_EG                   0x08
#define LAN9646_PORTn_XMII_CTRL1_MII_RMII_MODE                 0x04
#define LAN9646_PORTn_XMII_CTRL1_IF_TYPE                       0x03
#define LAN9646_PORTn_XMII_CTRL1_IF_TYPE_RGMII                 0x00
#define LAN9646_PORTn_XMII_CTRL1_IF_TYPE_RMII                  0x01
#define LAN9646_PORTn_XMII_CTRL1_IF_TYPE_MII                   0x03

//Port N MAC Control 0 register
#define LAN9646_PORTn_MAC_CTRL0_BCAST_STORM_PROTECT_EN         0x02

//Port N MAC Control 1 register
#define LAN9646_PORTn_MAC_CTRL1_BACK_PRESSURE_EN               0x08
#define LAN9646_PORTn_MAC_CTRL1_PASS_ALL_FRAMES                0x01

//Port N MIB Control and Status register
#define LAN9646_PORTn_MIB_CTRL_STAT_MIB_COUNTER_OVERFLOW       0x80000000
#define LAN9646_PORTn_MIB_CTRL_STAT_MIB_READ                   0x02000000
#define LAN9646_PORTn_MIB_CTRL_STAT_MIB_FLUSH_FREEZE           0x01000000
#define LAN9646_PORTn_MIB_CTRL_STAT_MIB_INDEX                  0x00FF0000
#define LAN9646_PORTn_MIB_CTRL_STAT_MIB_COUNTER_VALUE_35_32    0x0000000F

//Port N MIB Data register
#define LAN9646_PORTn_MIB_DATA_MIB_COUNTER_VALUE_31_0          0xFFFFFFFF

//Port N ACL Access Control 0 register
#define LAN9646_PORTn_ACL_ACCESS_CTRL0_WRITE_STATUS            0x40
#define LAN9646_PORTn_ACL_ACCESS_CTRL0_READ_STATUS             0x20
#define LAN9646_PORTn_ACL_ACCESS_CTRL0_READ                    0x00
#define LAN9646_PORTn_ACL_ACCESS_CTRL0_WRITE                   0x10
#define LAN9646_PORTn_ACL_ACCESS_CTRL0_ACL_INDEX               0x0F

//Port N Port Mirroring Control register
#define LAN9646_PORTn_MIRRORING_CTRL_RECEIVE_SNIFF             0x40
#define LAN9646_PORTn_MIRRORING_CTRL_TRANSMIT_SNIFF            0x20
#define LAN9646_PORTn_MIRRORING_CTRL_SNIFFER_PORT              0x02

//Port N Authentication Control register
#define LAN9646_PORTn_AUTH_CTRL_ACL_EN                         0x04
#define LAN9646_PORTn_AUTH_CTRL_AUTH_MODE                      0x03
#define LAN9646_PORTn_AUTH_CTRL_AUTH_MODE_PASS                 0x00
#define LAN9646_PORTn_AUTH_CTRL_AUTH_MODE_BLOCK                0x01
#define LAN9646_PORTn_AUTH_CTRL_AUTH_MODE_TRAP                 0x02

//Port N Pointer register
#define LAN9646_PORTn_PTR_PORT_INDEX                           0x00070000
#define LAN9646_PORTn_PTR_QUEUE_PTR                            0x00000003

//Port N Control 1 register
#define LAN9646_PORTn_CTRL1_PORT_VLAN_MEMBERSHIP               0x0000007F
#define LAN9646_PORTn_CTRL1_PORT7_VLAN_MEMBERSHIP              0x00000040
#define LAN9646_PORTn_CTRL1_PORT6_VLAN_MEMBERSHIP              0x00000020
#define LAN9646_PORTn_CTRL1_PORT4_VLAN_MEMBERSHIP              0x00000008
#define LAN9646_PORTn_CTRL1_PORT3_VLAN_MEMBERSHIP              0x00000004
#define LAN9646_PORTn_CTRL1_PORT2_VLAN_MEMBERSHIP              0x00000002
#define LAN9646_PORTn_CTRL1_PORT1_VLAN_MEMBERSHIP              0x00000001

//Port N Control 2 register
#define LAN9646_PORTn_CTRL2_NULL_VID_LOOKUP_EN                 0x80
#define LAN9646_PORTn_CTRL2_INGRESS_VLAN_FILT                  0x40
#define LAN9646_PORTn_CTRL2_DISCARD_NON_PVID_PKT               0x20
#define LAN9646_PORTn_CTRL2_802_1X_EN                          0x10
#define LAN9646_PORTn_CTRL2_SELF_ADDR_FILT                     0x08

//Port N MSTP Pointer register
#define LAN9646_PORTn_MSTP_PTR_MSTP_PTR                        0x07

//Port N MSTP State register
#define LAN9646_PORTn_MSTP_STATE_TRANSMIT_EN                   0x04
#define LAN9646_PORTn_MSTP_STATE_RECEIVE_EN                    0x02
#define LAN9646_PORTn_MSTP_STATE_LEARNING_DIS                  0x01

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//LAN9646 Ethernet switch driver
extern const SwitchDriver lan9646SwitchDriver;

//LAN9646 related functions
error_t lan9646Init(NetInterface *interface);
void lan9646InitHook(NetInterface *interface);

void lan9646Tick(NetInterface *interface);

void lan9646EnableIrq(NetInterface *interface);
void lan9646DisableIrq(NetInterface *interface);

void lan9646EventHandler(NetInterface *interface);

error_t lan9646TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, NetTxAncillary *ancillary);

error_t lan9646UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, NetRxAncillary *ancillary);

bool_t lan9646GetLinkState(NetInterface *interface, uint8_t port);
uint32_t lan9646GetLinkSpeed(NetInterface *interface, uint8_t port);
NicDuplexMode lan9646GetDuplexMode(NetInterface *interface, uint8_t port);

void lan9646SetPortState(NetInterface *interface, uint8_t port,
   SwitchPortState state);

SwitchPortState lan9646GetPortState(NetInterface *interface, uint8_t port);

void lan9646SetAgingTime(NetInterface *interface, uint32_t agingTime);

void lan9646EnableIgmpSnooping(NetInterface *interface, bool_t enable);
void lan9646EnableMldSnooping(NetInterface *interface, bool_t enable);
void lan9646EnableRsvdMcastTable(NetInterface *interface, bool_t enable);

error_t lan9646AddStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry);

error_t lan9646DeleteStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry);

error_t lan9646GetStaticFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry);

void lan9646FlushStaticFdbTable(NetInterface *interface);

error_t lan9646GetDynamicFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry);

void lan9646FlushDynamicFdbTable(NetInterface *interface, uint8_t port);

void lan9646SetUnknownMcastFwdPorts(NetInterface *interface,
   bool_t enable, uint32_t forwardPorts);

void lan9646WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data);

uint16_t lan9646ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address);

void lan9646DumpPhyReg(NetInterface *interface, uint8_t port);

void lan9646WriteMmdReg(NetInterface *interface, uint8_t port,
   uint8_t devAddr, uint16_t regAddr, uint16_t data);

uint16_t lan9646ReadMmdReg(NetInterface *interface, uint8_t port,
   uint8_t devAddr, uint16_t regAddr);

void lan9646WriteSwitchReg8(NetInterface *interface, uint16_t address,
   uint8_t data);

uint8_t lan9646ReadSwitchReg8(NetInterface *interface, uint16_t address);

void lan9646WriteSwitchReg16(NetInterface *interface, uint16_t address,
   uint16_t data);

uint16_t lan9646ReadSwitchReg16(NetInterface *interface, uint16_t address);

void lan9646WriteSwitchReg32(NetInterface *interface, uint16_t address,
   uint32_t data);

uint32_t lan9646ReadSwitchReg32(NetInterface *interface, uint16_t address);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
