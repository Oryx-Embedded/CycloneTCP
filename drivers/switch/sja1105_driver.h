/**
 * @file sja1105_driver.h
 * @brief SJA1105 5-port Ethernet switch driver
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

#ifndef _SJA1105_DRIVER_H
#define _SJA1105_DRIVER_H

//Dependencies
#include "core/nic.h"

//Port identifiers
#define SJA1105_PORT0 1
#define SJA1105_PORT1 2
#define SJA1105_PORT2 3
#define SJA1105_PORT3 4
#define SJA1105_PORT4 5

//SPI control word
#define SJA1105_SPI_CTRL_WRITE  0x80000000
#define SJA1105_SPI_CTRL_READ   0x00000000
#define SJA1105_SPI_CTRL_RC     0x7E000000
#define SJA1105_SPI_CTRL_ADDR   0x01FFFFF0

//SJA1105 memory map
#define SJA1105_ETH_DYN_BASE    0x00000000
#define SJA1105_ETH_STATIC_BASE 0x00020000
#define SJA1105_CGU_BASE        0x00100000
#define SJA1105_RGU_BASE        0x00100400
#define SJA1105_ACU_BASE        0x00100800
#define SJA1105_SGMII_BASE      0x00180000

//SJA1105 SPI registers
#define SJA1105_DEVICE_ID                                        0x000000
#define SJA1105_INIT_DEV_CONFIG_FLAG                             0x000001
#define SJA1105_VL_ROUTE_PARTITION_STATUS                        0x000002
#define SJA1105_GENERAL_STATUS1                                  0x000003
#define SJA1105_GENERAL_STATUS2                                  0x000004
#define SJA1105_GENERAL_STATUS3                                  0x000005
#define SJA1105_GENERAL_STATUS4                                  0x000006
#define SJA1105_GENERAL_STATUS5                                  0x000007
#define SJA1105_GENERAL_STATUS6                                  0x000008
#define SJA1105_GENERAL_STATUS7                                  0x000009
#define SJA1105_GENERAL_STATUS8                                  0x00000A
#define SJA1105_GENERAL_STATUS9                                  0x00000B
#define SJA1105_GENERAL_STATUS10                                 0x00000C
#define SJA1105_GENERAL_STATUS11                                 0x00000D
#define SJA1105_RAM_PARITY_CHECK_CONFIG0                         0x00000E
#define SJA1105_RAM_PARITY_CHECK_CONFIG1                         0x00000F
#define SJA1105_PORT_STATUS_CTRL0                                0x000010
#define SJA1105_VL_STATUS_CTRL                                   0x000011
#define SJA1105_PORT_STATUS_CTRL1                                0x000012
#define SJA1105_PTP_CTRL2                                        0x000013
#define SJA1105_PTP_CTRL3                                        0x000014
#define SJA1105_PTP_CTRL4                                        0x000015
#define SJA1105_PTP_CTRL5                                        0x000016
#define SJA1105_PTP_CTRL6                                        0x000017
#define SJA1105_PTP_CTRL1                                        0x000018
#define SJA1105_PTP_CTRL7                                        0x000019
#define SJA1105_PTP_CTRL8                                        0x00001A
#define SJA1105_PTP_CTRL9                                        0x00001B
#define SJA1105_PTP_CTRL10                                       0x00001C
#define SJA1105_PTP_CTRL11                                       0x00001D
#define SJA1105_PTP_CTRL12                                       0x00001E
#define SJA1105_PTP_CTRL13                                       0x00001F
#define SJA1105_PTP_CTRL14                                       0x000020
#define SJA1105_L2_ADDR_LUT_RECONFIG1                            0x000024
#define SJA1105_L2_ADDR_LUT_RECONFIG2                            0x000025
#define SJA1105_L2_ADDR_LUT_RECONFIG3                            0x000026
#define SJA1105_L2_ADDR_LUT_RECONFIG4                            0x000027
#define SJA1105_L2_ADDR_LUT_RECONFIG5                            0x000028
#define SJA1105_L2_ADDR_LUT_RECONFIG0                            0x000029
#define SJA1105_L2_FWD_TABLE_RECONFIG1                           0x00002A
#define SJA1105_L2_FWD_TABLE_RECONFIG2                           0x00002B
#define SJA1105_L2_FWD_TABLE_RECONFIG0                           0x00002C
#define SJA1105_VLAN_LUT_RECONFIG1                               0x00002D
#define SJA1105_VLAN_LUT_RECONFIG2                               0x00002E
#define SJA1105_VLAN_LUT_RECONFIG0                               0x000030
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG1                   0x000032
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG2                   0x000033
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG3                   0x000034
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG4                   0x000035
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG5                   0x000036
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG0                   0x000037
#define SJA1105_RETAGGING_TABLE_RECONFIG1                        0x000038
#define SJA1105_RETAGGING_TABLE_RECONFIG2                        0x000039
#define SJA1105_RETAGGING_TABLE_RECONFIG0                        0x00003A
#define SJA1105_GENERAL_PARAMS_RECONFIG1                         0x00003B
#define SJA1105_GENERAL_PARAMS_RECONFIG2                         0x00003C
#define SJA1105_GENERAL_PARAMS_RECONFIG3                         0x00003D
#define SJA1105_GENERAL_PARAMS_RECONFIG4                         0x00003E
#define SJA1105_GENERAL_PARAMS_RECONFIG5                         0x00003F
#define SJA1105_GENERAL_PARAMS_RECONFIG6                         0x000040
#define SJA1105_GENERAL_PARAMS_RECONFIG7                         0x000041
#define SJA1105_GENERAL_PARAMS_RECONFIG8                         0x000042
#define SJA1105_GENERAL_PARAMS_RECONFIG9                         0x000043
#define SJA1105_GENERAL_PARAMS_RECONFIG10                        0x000044
#define SJA1105_GENERAL_PARAMS_RECONFIG11                        0x000045
#define SJA1105_GENERAL_PARAMS_RECONFIG0                         0x000046
#define SJA1105_VL_LUT_RECONFIG1                                 0x000047
#define SJA1105_VL_LUT_RECONFIG2                                 0x000048
#define SJA1105_VL_LUT_RECONFIG3                                 0x000049
#define SJA1105_VL_LUT_RECONFIG0                                 0x00004A
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG1                       0x00004B
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2                       0x00004C
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG3                       0x00004D
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4                       0x00004E
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG5                       0x00004F
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG6                       0x000050
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG7                       0x000051
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG8                       0x000052
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG0                       0x000053
#define SJA1105_L2_LOOKUP_PARAMS1                                0x000054
#define SJA1105_L2_LOOKUP_PARAMS2                                0x000055
#define SJA1105_L2_LOOKUP_PARAMS3                                0x000056
#define SJA1105_L2_LOOKUP_PARAMS4                                0x000057
#define SJA1105_L2_LOOKUP_PARAMS0                                0x000058
#define SJA1105_SYNC_ENGINE_STATUS0                              0x00005A
#define SJA1105_SYNC_ENGINE_STATUS1                              0x00005B
#define SJA1105_SYNC_ENGINE_STATUS2                              0x00005C
#define SJA1105_SYNC_ENGINE_STATUS3                              0x00005D
#define SJA1105_SYNC_ENGINE_STATUS4                              0x00005E
#define SJA1105_SYNC_ENGINE_STATUS5                              0x00005F
#define SJA1105_SYNC_ENGINE_STATUS6                              0x000060
#define SJA1105_SYNC_ENGINE_STATUS7                              0x000061
#define SJA1105_SYNC_ENGINE_STATUS8                              0x000062
#define SJA1105_SYNC_ENGINE_STATUS9                              0x000063
#define SJA1105_SYNC_ENGINE_STATUS10                             0x000064
#define SJA1105_SYNC_ENGINE_STATUS11                             0x000065
#define SJA1105_SYNC_ENGINE_STATUS12                             0x000066
#define SJA1105_SYNC_ENGINE_STATUS13                             0x000067
#define SJA1105_SYNC_ENGINE_STATUS14                             0x000068
#define SJA1105_SYNC_ENGINE_STATUS15                             0x000069
#define SJA1105_SYNC_ENGINE_STATUS16                             0x00006A
#define SJA1105_SYNC_ENGINE_STATUS17                             0x00006B
#define SJA1105_SYNC_ENGINE_STATUS18                             0x00006C
#define SJA1105_SYNC_ENGINE_STATUS19                             0x00006D
#define SJA1105_SYNC_ENGINE_STATUS20                             0x00006E
#define SJA1105_SYNC_ENGINE_STATUS21                             0x00006F
#define SJA1105_SYNC_ENGINE_STATUS23                             0x000070
#define SJA1105_SYNC_ENGINE_STATUS24                             0x000071
#define SJA1105_VL_MEM_PARTI_STATUS0                             0x000080
#define SJA1105_VL_MEM_PARTI_STATUS1                             0x000081
#define SJA1105_VL_MEM_PARTI_STATUS2                             0x000082
#define SJA1105_VL_MEM_PARTI_STATUS3                             0x000083
#define SJA1105_VL_MEM_PARTI_STATUS4                             0x000084
#define SJA1105_VL_MEM_PARTI_STATUS5                             0x000085
#define SJA1105_VL_MEM_PARTI_STATUS6                             0x000086
#define SJA1105_VL_MEM_PARTI_STATUS7                             0x000087
#define SJA1105_PTP_TS0                                          0x0000C0
#define SJA1105_PTPEGR_TS0                                       0x0000C1
#define SJA1105_PTP_TS1                                          0x0000C2
#define SJA1105_PTPEGR_TS1                                       0x0000C3
#define SJA1105_PTP_TS2                                          0x0000C4
#define SJA1105_PTPEGR_TS2                                       0x0000C5
#define SJA1105_PTP_TS3                                          0x0000C6
#define SJA1105_PTPEGR_TS3                                       0x0000C7
#define SJA1105_PTP_TS4                                          0x0000C8
#define SJA1105_PTPEGR_TS4                                       0x0000C9
#define SJA1105_PTP_TS5                                          0x0000CA
#define SJA1105_PTPEGR_TS5                                       0x0000CB
#define SJA1105_PTP_TS6                                          0x0000CC
#define SJA1105_PTPEGR_TS6                                       0x0000CD
#define SJA1105_PTP_TS7                                          0x0000CE
#define SJA1105_PTPEGR_TS7                                       0x0000CF
#define SJA1105_PTP_TS8                                          0x0000D0
#define SJA1105_PTPEGR_TS8                                       0x0000D1
#define SJA1105_PTP_TS9                                          0x0000D2
#define SJA1105_PTPEGR_TS9                                       0x0000D3
#define SJA1105_L2_MEM_PART_STATUS0                              0x000100
#define SJA1105_L2_MEM_PART_STATUS1                              0x000101
#define SJA1105_L2_MEM_PART_STATUS2                              0x000102
#define SJA1105_L2_MEM_PART_STATUS3                              0x000103
#define SJA1105_L2_MEM_PART_STATUS4                              0x000104
#define SJA1105_L2_MEM_PART_STATUS5                              0x000105
#define SJA1105_L2_MEM_PART_STATUS6                              0x000106
#define SJA1105_L2_MEM_PART_STATUS7                              0x000107
#define SJA1105_PORT0_MAC_DIAG_CNT                               0x000200
#define SJA1105_PORT0_MAC_DIAG_FLAGS                             0x000201
#define SJA1105_PORT1_MAC_DIAG_CNT                               0x000202
#define SJA1105_PORT1_MAC_DIAG_FLAGS                             0x000203
#define SJA1105_PORT2_MAC_DIAG_CNT                               0x000204
#define SJA1105_PORT2_MAC_DIAG_FLAGS                             0x000205
#define SJA1105_PORT3_MAC_DIAG_CNT                               0x000206
#define SJA1105_PORT3_MAC_DIAG_FLAGS                             0x000207
#define SJA1105_PORT4_MAC_DIAG_CNT                               0x000208
#define SJA1105_PORT4_MAC_DIAG_FLAGS                             0x000209
#define SJA1105_PORT0_N_TXBYTE                                   0x000400
#define SJA1105_PORT0_N_TXBYTESH                                 0x000401
#define SJA1105_PORT0_N_TXFRM                                    0x000402
#define SJA1105_PORT0_N_TXFRMSH                                  0x000403
#define SJA1105_PORT0_N_RXBYTE                                   0x000404
#define SJA1105_PORT0_N_RXBYTESH                                 0x000405
#define SJA1105_PORT0_N_RXFRM                                    0x000406
#define SJA1105_PORT0_N_RXFRMSH                                  0x000407
#define SJA1105_PORT0_N_POLERR                                   0x000408
#define SJA1105_PORT0_N_CTPOLERR                                 0x000409
#define SJA1105_PORT0_N_VLNOTFOUND                               0x00040A
#define SJA1105_PORT0_N_CRCERR                                   0x00040B
#define SJA1105_PORT0_N_SIZEERR                                  0x00040C
#define SJA1105_PORT0_N_UNRELEASED                               0x00040D
#define SJA1105_PORT0_N_VLANERR                                  0x00040E
#define SJA1105_PORT0_N_N664ERR                                  0x00040F
#define SJA1105_PORT1_N_TXBYTE                                   0x000410
#define SJA1105_PORT1_N_TXBYTESH                                 0x000411
#define SJA1105_PORT1_N_TXFRM                                    0x000412
#define SJA1105_PORT1_N_TXFRMSH                                  0x000413
#define SJA1105_PORT1_N_RXBYTE                                   0x000414
#define SJA1105_PORT1_N_RXBYTESH                                 0x000415
#define SJA1105_PORT1_N_RXFRM                                    0x000416
#define SJA1105_PORT1_N_RXFRMSH                                  0x000417
#define SJA1105_PORT1_N_POLERR                                   0x000418
#define SJA1105_PORT1_N_CTPOLERR                                 0x000419
#define SJA1105_PORT1_N_VLNOTFOUND                               0x00041A
#define SJA1105_PORT1_N_CRCERR                                   0x00041B
#define SJA1105_PORT1_N_SIZEERR                                  0x00041C
#define SJA1105_PORT1_N_UNRELEASED                               0x00041D
#define SJA1105_PORT1_N_VLANERR                                  0x00041E
#define SJA1105_PORT1_N_N664ERR                                  0x00041F
#define SJA1105_PORT2_N_TXBYTE                                   0x000420
#define SJA1105_PORT2_N_TXBYTESH                                 0x000421
#define SJA1105_PORT2_N_TXFRM                                    0x000422
#define SJA1105_PORT2_N_TXFRMSH                                  0x000423
#define SJA1105_PORT2_N_RXBYTE                                   0x000424
#define SJA1105_PORT2_N_RXBYTESH                                 0x000425
#define SJA1105_PORT2_N_RXFRM                                    0x000426
#define SJA1105_PORT2_N_RXFRMSH                                  0x000427
#define SJA1105_PORT2_N_POLERR                                   0x000428
#define SJA1105_PORT2_N_CTPOLERR                                 0x000429
#define SJA1105_PORT2_N_VLNOTFOUND                               0x00042A
#define SJA1105_PORT2_N_CRCERR                                   0x00042B
#define SJA1105_PORT2_N_SIZEERR                                  0x00042C
#define SJA1105_PORT2_N_UNRELEASED                               0x00042D
#define SJA1105_PORT2_N_VLANERR                                  0x00042E
#define SJA1105_PORT2_N_N664ERR                                  0x00042F
#define SJA1105_PORT3_N_TXBYTE                                   0x000430
#define SJA1105_PORT3_N_TXBYTESH                                 0x000431
#define SJA1105_PORT3_N_TXFRM                                    0x000432
#define SJA1105_PORT3_N_TXFRMSH                                  0x000433
#define SJA1105_PORT3_N_RXBYTE                                   0x000434
#define SJA1105_PORT3_N_RXBYTESH                                 0x000435
#define SJA1105_PORT3_N_RXFRM                                    0x000436
#define SJA1105_PORT3_N_RXFRMSH                                  0x000437
#define SJA1105_PORT3_N_POLERR                                   0x000438
#define SJA1105_PORT3_N_CTPOLERR                                 0x000439
#define SJA1105_PORT3_N_VLNOTFOUND                               0x00043A
#define SJA1105_PORT3_N_CRCERR                                   0x00043B
#define SJA1105_PORT3_N_SIZEERR                                  0x00043C
#define SJA1105_PORT3_N_UNRELEASED                               0x00043D
#define SJA1105_PORT3_N_VLANERR                                  0x00043E
#define SJA1105_PORT3_N_N664ERR                                  0x00043F
#define SJA1105_PORT4_N_TXBYTE                                   0x000440
#define SJA1105_PORT4_N_TXBYTESH                                 0x000441
#define SJA1105_PORT4_N_TXFRM                                    0x000442
#define SJA1105_PORT4_N_TXFRMSH                                  0x000443
#define SJA1105_PORT4_N_RXBYTE                                   0x000444
#define SJA1105_PORT4_N_RXBYTESH                                 0x000445
#define SJA1105_PORT4_N_RXFRM                                    0x000446
#define SJA1105_PORT4_N_RXFRMSH                                  0x000447
#define SJA1105_PORT4_N_POLERR                                   0x000448
#define SJA1105_PORT4_N_CTPOLERR                                 0x000449
#define SJA1105_PORT4_N_VLNOTFOUND                               0x00044A
#define SJA1105_PORT4_N_CRCERR                                   0x00044B
#define SJA1105_PORT4_N_SIZEERR                                  0x00044C
#define SJA1105_PORT4_N_UNRELEASED                               0x00044D
#define SJA1105_PORT4_N_VLANERR                                  0x00044E
#define SJA1105_PORT4_N_N664ERR                                  0x00044F
#define SJA1105_PORT0_N_NOT_REACH                                0x000600
#define SJA1105_PORT0_N_EGR_DISABLED                             0x000601
#define SJA1105_PORT0_N_PART_DROP                                0x000602
#define SJA1105_PORT0_N_QFULL                                    0x000603
#define SJA1105_PORT0_QLEVEL0                                    0x000604
#define SJA1105_PORT0_QLEVEL1                                    0x000605
#define SJA1105_PORT0_QLEVEL2                                    0x000606
#define SJA1105_PORT0_QLEVEL3                                    0x000607
#define SJA1105_PORT0_QLEVEL4                                    0x000608
#define SJA1105_PORT0_QLEVEL5                                    0x000609
#define SJA1105_PORT0_QLEVEL6                                    0x00060A
#define SJA1105_PORT0_QLEVEL7                                    0x00060B
#define SJA1105_PORT1_N_NOT_REACH                                0x000610
#define SJA1105_PORT1_N_EGR_DISABLED                             0x000611
#define SJA1105_PORT1_N_PART_DROP                                0x000612
#define SJA1105_PORT1_N_QFULL                                    0x000613
#define SJA1105_PORT1_QLEVEL0                                    0x000614
#define SJA1105_PORT1_QLEVEL1                                    0x000615
#define SJA1105_PORT1_QLEVEL2                                    0x000616
#define SJA1105_PORT1_QLEVEL3                                    0x000617
#define SJA1105_PORT1_QLEVEL4                                    0x000618
#define SJA1105_PORT1_QLEVEL5                                    0x000619
#define SJA1105_PORT1_QLEVEL6                                    0x00061A
#define SJA1105_PORT1_QLEVEL7                                    0x00061B
#define SJA1105_PORT2_N_NOT_REACH                                0x000620
#define SJA1105_PORT2_N_EGR_DISABLED                             0x000621
#define SJA1105_PORT2_N_PART_DROP                                0x000622
#define SJA1105_PORT2_N_QFULL                                    0x000623
#define SJA1105_PORT2_QLEVEL0                                    0x000624
#define SJA1105_PORT2_QLEVEL1                                    0x000625
#define SJA1105_PORT2_QLEVEL2                                    0x000626
#define SJA1105_PORT2_QLEVEL3                                    0x000627
#define SJA1105_PORT2_QLEVEL4                                    0x000628
#define SJA1105_PORT2_QLEVEL5                                    0x000629
#define SJA1105_PORT2_QLEVEL6                                    0x00062A
#define SJA1105_PORT2_QLEVEL7                                    0x00062B
#define SJA1105_PORT3_N_NOT_REACH                                0x000630
#define SJA1105_PORT3_N_EGR_DISABLED                             0x000631
#define SJA1105_PORT3_N_PART_DROP                                0x000632
#define SJA1105_PORT3_N_QFULL                                    0x000633
#define SJA1105_PORT3_QLEVEL0                                    0x000634
#define SJA1105_PORT3_QLEVEL1                                    0x000635
#define SJA1105_PORT3_QLEVEL2                                    0x000636
#define SJA1105_PORT3_QLEVEL3                                    0x000637
#define SJA1105_PORT3_QLEVEL4                                    0x000638
#define SJA1105_PORT3_QLEVEL5                                    0x000639
#define SJA1105_PORT3_QLEVEL6                                    0x00063A
#define SJA1105_PORT3_QLEVEL7                                    0x00063B
#define SJA1105_PORT4_N_NOT_REACH                                0x000640
#define SJA1105_PORT4_N_EGR_DISABLED                             0x000641
#define SJA1105_PORT4_N_PART_DROP                                0x000642
#define SJA1105_PORT4_N_QFULL                                    0x000643
#define SJA1105_PORT4_QLEVEL0                                    0x000644
#define SJA1105_PORT4_QLEVEL1                                    0x000645
#define SJA1105_PORT4_QLEVEL2                                    0x000646
#define SJA1105_PORT4_QLEVEL3                                    0x000647
#define SJA1105_PORT4_QLEVEL4                                    0x000648
#define SJA1105_PORT4_QLEVEL5                                    0x000649
#define SJA1105_PORT4_QLEVEL6                                    0x00064A
#define SJA1105_PORT4_QLEVEL7                                    0x00064B
#define SJA1105_VL_MEM_PART_ERR_CNT0                             0x000800
#define SJA1105_VL_MEM_PART_ERR_CNT1                             0x000801
#define SJA1105_VL_MEM_PART_ERR_CNT2                             0x000802
#define SJA1105_VL_MEM_PART_ERR_CNT3                             0x000803
#define SJA1105_VL_MEM_PART_ERR_CNT4                             0x000804
#define SJA1105_VL_MEM_PART_ERR_CNT5                             0x000805
#define SJA1105_VL_MEM_PART_ERR_CNT6                             0x000806
#define SJA1105_VL_MEM_PART_ERR_CNT7                             0x000807
#define SJA1105_L2_MEM_PART_ERR_CNT0                             0x001000
#define SJA1105_L2_MEM_PART_ERR_CNT1                             0x001001
#define SJA1105_L2_MEM_PART_ERR_CNT2                             0x001002
#define SJA1105_L2_MEM_PART_ERR_CNT3                             0x001003
#define SJA1105_L2_MEM_PART_ERR_CNT4                             0x001004
#define SJA1105_L2_MEM_PART_ERR_CNT5                             0x001005
#define SJA1105_L2_MEM_PART_ERR_CNT6                             0x001006
#define SJA1105_L2_MEM_PART_ERR_CNT7                             0x001007
#define SJA1105_PORT0_N_RX_BCAST                                 0x001400
#define SJA1105_PORT0_N_RX_MCAST                                 0x001401
#define SJA1105_PORT0_N_RX_BYTES_64                              0x001402
#define SJA1105_PORT0_N_RX_BYTES_65_127                          0x001403
#define SJA1105_PORT0_N_RX_BYTES_128_255                         0x001404
#define SJA1105_PORT0_N_RX_BYTES_256_511                         0x001405
#define SJA1105_PORT0_N_RX_BYTES_512_1023                        0x001406
#define SJA1105_PORT0_N_RX_BYTES_1024_1518                       0x001407
#define SJA1105_PORT0_N_TX_BCAST                                 0x001408
#define SJA1105_PORT0_N_TX_MCAST                                 0x001409
#define SJA1105_PORT0_N_TX_BYTES_64                              0x00140A
#define SJA1105_PORT0_N_TX_BYTES_65_127                          0x00140B
#define SJA1105_PORT0_N_TX_BYTES_128_255                         0x00140C
#define SJA1105_PORT0_N_TX_BYTES_256_511                         0x00140D
#define SJA1105_PORT0_N_TX_BYTES_512_1023                        0x00140E
#define SJA1105_PORT0_N_TX_BYTES_1024_1518                       0x00140F
#define SJA1105_PORT0_N_DROPS_UTAG                               0x001410
#define SJA1105_PORT0_N_DROPS_SITAG                              0x001411
#define SJA1105_PORT0_N_DROPS_SOTAG                              0x001412
#define SJA1105_PORT0_N_DROPS_DTAG                               0x001413
#define SJA1105_PORT0_N_DROPS_ILL_DTAG                           0x001414
#define SJA1105_PORT0_N_DROPS_EMPTY_ROUTE                        0x001415
#define SJA1105_PORT0_N_DROPS_NOLEARN                            0x001416
#define SJA1105_PORT1_N_RX_BCAST                                 0x001418
#define SJA1105_PORT1_N_RX_MCAST                                 0x001419
#define SJA1105_PORT1_N_RX_BYTES_64                              0x00141A
#define SJA1105_PORT1_N_RX_BYTES_65_127                          0x00141B
#define SJA1105_PORT1_N_RX_BYTES_128_255                         0x00141C
#define SJA1105_PORT1_N_RX_BYTES_256_511                         0x00141D
#define SJA1105_PORT1_N_RX_BYTES_512_1023                        0x00141E
#define SJA1105_PORT1_N_RX_BYTES_1024_1518                       0x00141F
#define SJA1105_PORT1_N_TX_BCAST                                 0x001420
#define SJA1105_PORT1_N_TX_MCAST                                 0x001421
#define SJA1105_PORT1_N_TX_BYTES_64                              0x001422
#define SJA1105_PORT1_N_TX_BYTES_65_127                          0x001423
#define SJA1105_PORT1_N_TX_BYTES_128_255                         0x001424
#define SJA1105_PORT1_N_TX_BYTES_256_511                         0x001425
#define SJA1105_PORT1_N_TX_BYTES_512_1023                        0x001426
#define SJA1105_PORT1_N_TX_BYTES_1024_1518                       0x001427
#define SJA1105_PORT1_N_DROPS_UTAG                               0x001428
#define SJA1105_PORT1_N_DROPS_SITAG                              0x001429
#define SJA1105_PORT1_N_DROPS_SOTAG                              0x00142A
#define SJA1105_PORT1_N_DROPS_DTAG                               0x00142B
#define SJA1105_PORT1_N_DROPS_ILL_DTAG                           0x00142C
#define SJA1105_PORT1_N_DROPS_EMPTY_ROUTE                        0x00142D
#define SJA1105_PORT1_N_DROPS_NOLEARN                            0x00142E
#define SJA1105_PORT2_N_RX_BCAST                                 0x001430
#define SJA1105_PORT2_N_RX_MCAST                                 0x001431
#define SJA1105_PORT2_N_RX_BYTES_64                              0x001432
#define SJA1105_PORT2_N_RX_BYTES_65_127                          0x001433
#define SJA1105_PORT2_N_RX_BYTES_128_255                         0x001434
#define SJA1105_PORT2_N_RX_BYTES_256_511                         0x001435
#define SJA1105_PORT2_N_RX_BYTES_512_1023                        0x001436
#define SJA1105_PORT2_N_RX_BYTES_1024_1518                       0x001437
#define SJA1105_PORT2_N_TX_BCAST                                 0x001438
#define SJA1105_PORT2_N_TX_MCAST                                 0x001439
#define SJA1105_PORT2_N_TX_BYTES_64                              0x00143A
#define SJA1105_PORT2_N_TX_BYTES_65_127                          0x00143B
#define SJA1105_PORT2_N_TX_BYTES_128_255                         0x00143C
#define SJA1105_PORT2_N_TX_BYTES_256_511                         0x00143D
#define SJA1105_PORT2_N_TX_BYTES_512_1023                        0x00143E
#define SJA1105_PORT2_N_TX_BYTES_1024_1518                       0x00143F
#define SJA1105_PORT2_N_DROPS_UTAG                               0x001440
#define SJA1105_PORT2_N_DROPS_SITAG                              0x001441
#define SJA1105_PORT2_N_DROPS_SOTAG                              0x001442
#define SJA1105_PORT2_N_DROPS_DTAG                               0x001443
#define SJA1105_PORT2_N_DROPS_ILL_DTAG                           0x001444
#define SJA1105_PORT2_N_DROPS_EMPTY_ROUTE                        0x001445
#define SJA1105_PORT2_N_DROPS_NOLEARN                            0x001446
#define SJA1105_PORT3_N_RX_BCAST                                 0x001448
#define SJA1105_PORT3_N_RX_MCAST                                 0x001449
#define SJA1105_PORT3_N_RX_BYTES_64                              0x00144A
#define SJA1105_PORT3_N_RX_BYTES_65_127                          0x00144B
#define SJA1105_PORT3_N_RX_BYTES_128_255                         0x00144C
#define SJA1105_PORT3_N_RX_BYTES_256_511                         0x00144D
#define SJA1105_PORT3_N_RX_BYTES_512_1023                        0x00144E
#define SJA1105_PORT3_N_RX_BYTES_1024_1518                       0x00144F
#define SJA1105_PORT3_N_TX_BCAST                                 0x001450
#define SJA1105_PORT3_N_TX_MCAST                                 0x001451
#define SJA1105_PORT3_N_TX_BYTES_64                              0x001452
#define SJA1105_PORT3_N_TX_BYTES_65_127                          0x001453
#define SJA1105_PORT3_N_TX_BYTES_128_255                         0x001454
#define SJA1105_PORT3_N_TX_BYTES_256_511                         0x001455
#define SJA1105_PORT3_N_TX_BYTES_512_1023                        0x001456
#define SJA1105_PORT3_N_TX_BYTES_1024_1518                       0x001457
#define SJA1105_PORT3_N_DROPS_UTAG                               0x001458
#define SJA1105_PORT3_N_DROPS_SITAG                              0x001459
#define SJA1105_PORT3_N_DROPS_SOTAG                              0x00145A
#define SJA1105_PORT3_N_DROPS_DTAG                               0x00145B
#define SJA1105_PORT3_N_DROPS_ILL_DTAG                           0x00145C
#define SJA1105_PORT3_N_DROPS_EMPTY_ROUTE                        0x00145D
#define SJA1105_PORT3_N_DROPS_NOLEARN                            0x00145E
#define SJA1105_PORT4_N_RX_BCAST                                 0x001460
#define SJA1105_PORT4_N_RX_MCAST                                 0x001461
#define SJA1105_PORT4_N_RX_BYTES_64                              0x001462
#define SJA1105_PORT4_N_RX_BYTES_65_127                          0x001463
#define SJA1105_PORT4_N_RX_BYTES_128_255                         0x001464
#define SJA1105_PORT4_N_RX_BYTES_256_511                         0x001465
#define SJA1105_PORT4_N_RX_BYTES_512_1023                        0x001466
#define SJA1105_PORT4_N_RX_BYTES_1024_1518                       0x001467
#define SJA1105_PORT4_N_TX_BCAST                                 0x001468
#define SJA1105_PORT4_N_TX_MCAST                                 0x001469
#define SJA1105_PORT4_N_TX_BYTES_64                              0x00146A
#define SJA1105_PORT4_N_TX_BYTES_65_127                          0x00146B
#define SJA1105_PORT4_N_TX_BYTES_128_255                         0x00146C
#define SJA1105_PORT4_N_TX_BYTES_256_511                         0x00146D
#define SJA1105_PORT4_N_TX_BYTES_512_1023                        0x00146E
#define SJA1105_PORT4_N_TX_BYTES_1024_1518                       0x00146F
#define SJA1105_PORT4_N_DROPS_UTAG                               0x001470
#define SJA1105_PORT4_N_DROPS_SITAG                              0x001471
#define SJA1105_PORT4_N_DROPS_SOTAG                              0x001472
#define SJA1105_PORT4_N_DROPS_DTAG                               0x001473
#define SJA1105_PORT4_N_DROPS_ILL_DTAG                           0x001474
#define SJA1105_PORT4_N_DROPS_EMPTY_ROUTE                        0x001475
#define SJA1105_PORT4_N_DROPS_NOLEARN                            0x001476
#define SJA1105_L2_FWD_PARAMS_RECONFIG0                          0x008000
#define SJA1105_L2_FWD_PARAMS_RECONFIG1                          0x008001
#define SJA1105_L2_FWD_PARAMS_RECONFIG2                          0x008002
#define SJA1105_AVB_PARAMS_RECONFIG1                             0x008003
#define SJA1105_AVB_PARAMS_RECONFIG2                             0x008004
#define SJA1105_AVB_PARAMS_RECONFIG3                             0x008005
#define SJA1105_AVB_PARAMS_RECONFIG4                             0x008006
#define SJA1105_AVB_PARAMS_RECONFIG0                             0x008007
#define SJA1105_VIRTUAL_LINK_STATUS0                             0x010000
#define SJA1105_VIRTUAL_LINK_STATUS1                             0x010001
#define SJA1105_RFRQ                                             0x100005
#define SJA1105_XO66M_0_C                                        0x100006
#define SJA1105_PLL_0_S                                          0x100007
#define SJA1105_PLL_0_C                                          0x100008
#define SJA1105_PLL_1_S                                          0x100009
#define SJA1105_PLL_1_C                                          0x10000A
#define SJA1105_IDIV_0_C                                         0x10000B
#define SJA1105_IDIV_1_C                                         0x10000C
#define SJA1105_IDIV_2_C                                         0x10000D
#define SJA1105_IDIV_3_C                                         0x10000E
#define SJA1105_IDIV_4_C                                         0x10000F
#define SJA1105_MII_TX_CLK_0                                     0x100013
#define SJA1105_MII_RX_CLK_0                                     0x100014
#define SJA1105_RMII_REF_CLK_0                                   0x100015
#define SJA1105_RGMII_TX_CLK_0                                   0x100016
#define SJA1105_EXT_TX_CLK_0                                     0x100017
#define SJA1105_EXT_RX_CLK_0                                     0x100018
#define SJA1105_MII_TX_CLK_1                                     0x100019
#define SJA1105_MII_RX_CLK_1                                     0x10001A
#define SJA1105_RMII_REF_CLK_1                                   0x10001B
#define SJA1105_RGMII_TX_CLK_1                                   0x10001C
#define SJA1105_EXT_TX_CLK_1                                     0x10001D
#define SJA1105_EXT_RX_CLK_1                                     0x10001E
#define SJA1105_MII_TX_CLK_2                                     0x10001F
#define SJA1105_MII_RX_CLK_2                                     0x100020
#define SJA1105_RMII_REF_CLK_2                                   0x100021
#define SJA1105_RGMII_TX_CLK_2                                   0x100022
#define SJA1105_EXT_TX_CLK_2                                     0x100023
#define SJA1105_EXT_RX_CLK_2                                     0x100024
#define SJA1105_MII_TX_CLK_3                                     0x100025
#define SJA1105_MII_RX_CLK_3                                     0x100026
#define SJA1105_RMII_REF_CLK_3                                   0x100027
#define SJA1105_RGMII_TX_CLK_3                                   0x100028
#define SJA1105_EXT_TX_CLK_3                                     0x100029
#define SJA1105_EXT_RX_CLK_3                                     0x10002A
#define SJA1105_MII_TX_CLK_4                                     0x10002B
#define SJA1105_MII_RX_CLK_4                                     0x10002C
#define SJA1105_RMII_REF_CLK_4                                   0x10002D
#define SJA1105_RGMII_TX_CLK_4                                   0x10002E
#define SJA1105_EXT_TX_CLK_4                                     0x10002F
#define SJA1105_EXT_RX_CLK_4                                     0x100030
#define SJA1105_RESET_CTRL                                       0x100440
#define SJA1105_RGU_UNIT_DISABLE                                 0x1007FD
#define SJA1105_CFG_PAD_MII0_TX                                  0x100800
#define SJA1105_CFG_PAD_MII0_RX                                  0x100801
#define SJA1105_CFG_PAD_MII1_TX                                  0x100802
#define SJA1105_CFG_PAD_MII1_RX                                  0x100803
#define SJA1105_CFG_PAD_MII2_TX                                  0x100804
#define SJA1105_CFG_PAD_MII2_RX                                  0x100805
#define SJA1105_CFG_PAD_MII3_TX                                  0x100806
#define SJA1105_CFG_PAD_MII3_RX                                  0x100807
#define SJA1105_CFG_PAD_MII4_TX                                  0x100808
#define SJA1105_CFG_PAD_MII4_RX                                  0x100809
#define SJA1105_CFG_PAD_MII0_ID                                  0x100810
#define SJA1105_CFG_PAD_MII1_ID                                  0x100811
#define SJA1105_CFG_PAD_MII2_ID                                  0x100812
#define SJA1105_CFG_PAD_MII3_ID                                  0x100813
#define SJA1105_CFG_PAD_MII4_ID                                  0x100814
#define SJA1105_CFG_PAD_MISC                                     0x100840
#define SJA1105_CFG_PAD_SPI                                      0x100880
#define SJA1105_CFG_PAD_JTAG                                     0x100881
#define SJA1105_PORT_STATUS_MII0                                 0x100900
#define SJA1105_PORT_STATUS_MII1                                 0x100901
#define SJA1105_PORT_STATUS_MII2                                 0x100902
#define SJA1105_PORT_STATUS_MII3                                 0x100903
#define SJA1105_PORT_STATUS_MII4                                 0x100904
#define SJA1105_TS_CONFIG                                        0x100A00
#define SJA1105_TS_STATUS                                        0x100A01
#define SJA1105_PROD_CFG                                         0x100BC0
#define SJA1105_PROD_ID                                          0x100BC3
#define SJA1105_ACU_UNIT_DISABLE                                 0x100BFD
#define SJA1105_BASIC_CONTROL                                    0x1F0000
#define SJA1105_BASIC_STATUS                                     0x1F0001
#define SJA1105_PHY_IDENTIFIER_1                                 0x1F0002
#define SJA1105_PHY_IDENTIFIER_2                                 0x1F0003
#define SJA1105_AUTONEG_ADV                                      0x1F0004
#define SJA1105_AUTONEG_LP_BABL                                  0x1F0005
#define SJA1105_AUTONEG_EXPN                                     0x1F0006
#define SJA1105_EXTENDED_STATUS                                  0x1F000F
#define SJA1105_DIGITAL_CONTROL_1                                0x1F8000
#define SJA1105_AUTONEG_CONTROL                                  0x1F8001
#define SJA1105_AUTONEG_INTR_STATUS                              0x1F8002
#define SJA1105_TEST_CONTROL                                     0x1F8003
#define SJA1105_DEBUG_CONTROL                                    0x1F8005
#define SJA1105_DIGITAL_STATUS                                   0x1F8010
#define SJA1105_ICG_ERROR_CNT                                    0x1F8011
#define SJA1105_TX_BOOST_CONTROL                                 0x1F8030
#define SJA1105_TX_ATTN_CONTROL                                  0x1F8031
#define SJA1105_TX_EDGE_CONTROL                                  0x1F8033
#define SJA1105_RX_EQ_CONTROL                                    0x1F8051
#define SJA1105_LEVEL_CONTROL                                    0x1F8090
#define SJA1105_DIGITAL_CONTROL_2                                0x1F80E1
#define SJA1105_DIGITAL_ERROR_CNT                                0x1F80E2

//SJA1105 SPI register access macros
#define SJA1105_PORTn_MAC_DIAG_CNT(port)                         (0x000200 + ((port) * 0x000002))
#define SJA1105_PORTn_MAC_DIAG_FLAGS(port)                       (0x000201 + ((port) * 0x000002))
#define SJA1105_PORTn_N_TXBYTE(port)                             (0x000400 + ((port) * 0x000010))
#define SJA1105_PORTn_N_TXBYTESH(port)                           (0x000401 + ((port) * 0x000010))
#define SJA1105_PORTn_N_TXFRM(port)                              (0x000402 + ((port) * 0x000010))
#define SJA1105_PORTn_N_TXFRMSH(port)                            (0x000403 + ((port) * 0x000010))
#define SJA1105_PORTn_N_RXBYTE(port)                             (0x000404 + ((port) * 0x000010))
#define SJA1105_PORTn_N_RXBYTESH(port)                           (0x000405 + ((port) * 0x000010))
#define SJA1105_PORTn_N_RXFRM(port)                              (0x000406 + ((port) * 0x000010))
#define SJA1105_PORTn_N_RXFRMSH(port)                            (0x000407 + ((port) * 0x000010))
#define SJA1105_PORTn_N_POLERR(port)                             (0x000408 + ((port) * 0x000010))
#define SJA1105_PORTn_N_CTPOLERR(port)                           (0x000409 + ((port) * 0x000010))
#define SJA1105_PORTn_N_VLNOTFOUND(port)                         (0x00040A + ((port) * 0x000010))
#define SJA1105_PORTn_N_CRCERR(port)                             (0x00040B + ((port) * 0x000010))
#define SJA1105_PORTn_N_SIZEERR(port)                            (0x00040C + ((port) * 0x000010))
#define SJA1105_PORTn_N_UNRELEASED(port)                         (0x00040D + ((port) * 0x000010))
#define SJA1105_PORTn_N_VLANERR(port)                            (0x00040E + ((port) * 0x000010))
#define SJA1105_PORTn_N_N664ERR(port)                            (0x00040F + ((port) * 0x000010))
#define SJA1105_PORTn_N_NOT_REACH(port)                          (0x000600 + ((port) * 0x000010))
#define SJA1105_PORTn_N_EGR_DISABLED(port)                       (0x000601 + ((port) * 0x000010))
#define SJA1105_PORTn_N_PART_DROP(port)                          (0x000602 + ((port) * 0x000010))
#define SJA1105_PORTn_N_QFULL(port)                              (0x000603 + ((port) * 0x000010))
#define SJA1105_PORTn_QLEVEL0(port)                              (0x000604 + ((port) * 0x000010))
#define SJA1105_PORTn_QLEVEL1(port)                              (0x000605 + ((port) * 0x000010))
#define SJA1105_PORTn_QLEVEL2(port)                              (0x000606 + ((port) * 0x000010))
#define SJA1105_PORTn_QLEVEL3(port)                              (0x000607 + ((port) * 0x000010))
#define SJA1105_PORTn_QLEVEL4(port)                              (0x000608 + ((port) * 0x000010))
#define SJA1105_PORTn_QLEVEL5(port)                              (0x000609 + ((port) * 0x000010))
#define SJA1105_PORTn_QLEVEL6(port)                              (0x00060A + ((port) * 0x000010))
#define SJA1105_PORTn_QLEVEL7(port)                              (0x00060B + ((port) * 0x000010))
#define SJA1105_PORTn_N_RX_BCAST(port)                           (0x001400 + ((port) * 0x000018))
#define SJA1105_PORTn_N_RX_MCAST(port)                           (0x001401 + ((port) * 0x000018))
#define SJA1105_PORTn_N_RX_BYTES_64(port)                        (0x001402 + ((port) * 0x000018))
#define SJA1105_PORTn_N_RX_BYTES_65_127(port)                    (0x001403 + ((port) * 0x000018))
#define SJA1105_PORTn_N_RX_BYTES_128_255(port)                   (0x001404 + ((port) * 0x000018))
#define SJA1105_PORTn_N_RX_BYTES_256_511(port)                   (0x001405 + ((port) * 0x000018))
#define SJA1105_PORTn_N_RX_BYTES_512_1023(port)                  (0x001406 + ((port) * 0x000018))
#define SJA1105_PORTn_N_RX_BYTES_1024_1518(port)                 (0x001407 + ((port) * 0x000018))
#define SJA1105_PORTn_N_TX_BCAST(port)                           (0x001408 + ((port) * 0x000018))
#define SJA1105_PORTn_N_TX_MCAST(port)                           (0x001409 + ((port) * 0x000018))
#define SJA1105_PORTn_N_TX_BYTES_64(port)                        (0x00140A + ((port) * 0x000018))
#define SJA1105_PORTn_N_TX_BYTES_65_127(port)                    (0x00140B + ((port) * 0x000018))
#define SJA1105_PORTn_N_TX_BYTES_128_255(port)                   (0x00140C + ((port) * 0x000018))
#define SJA1105_PORTn_N_TX_BYTES_256_511(port)                   (0x00140D + ((port) * 0x000018))
#define SJA1105_PORTn_N_TX_BYTES_512_1023(port)                  (0x00140E + ((port) * 0x000018))
#define SJA1105_PORTn_N_TX_BYTES_1024_1518(port)                 (0x00140F + ((port) * 0x000018))
#define SJA1105_PORTn_N_DROPS_UTAG(port)                         (0x001410 + ((port) * 0x000018))
#define SJA1105_PORTn_N_DROPS_SITAG(port)                        (0x001411 + ((port) * 0x000018))
#define SJA1105_PORTn_N_DROPS_SOTAG(port)                        (0x001412 + ((port) * 0x000018))
#define SJA1105_PORTn_N_DROPS_DTAG(port)                         (0x001413 + ((port) * 0x000018))
#define SJA1105_PORTn_N_DROPS_ILL_DTAG(port)                     (0x001414 + ((port) * 0x000018))
#define SJA1105_PORTn_N_DROPS_EMPTY_ROUTE(port)                  (0x001415 + ((port) * 0x000018))
#define SJA1105_PORTn_N_DROPS_NOLEARN(port)                      (0x001416 + ((port) * 0x000018))
#define SJA1105_IDIV_x_C(port)                                   (0x10000B + ((port) * 0x000001))
#define SJA1105_MII_TX_CLK_x(port)                               (0x100013 + ((port) * 0x000006))
#define SJA1105_MII_RX_CLK_x(port)                               (0x100014 + ((port) * 0x000006))
#define SJA1105_RMII_REF_CLK_x(port)                             (0x100015 + ((port) * 0x000006))
#define SJA1105_RGMII_TX_CLK_x(port)                             (0x100016 + ((port) * 0x000006))
#define SJA1105_EXT_TX_CLK_x(port)                               (0x100017 + ((port) * 0x000006))
#define SJA1105_EXT_RX_CLK_x(port)                               (0x100018 + ((port) * 0x000006))
#define SJA1105_CFG_PAD_MIIx_TX(port)                            (0x100800 + ((port) * 0x000002))
#define SJA1105_CFG_PAD_MIIx_RX(port)                            (0x100801 + ((port) * 0x000002))
#define SJA1105_CFG_PAD_MIIx_ID(port)                            (0x100810 + ((port) * 0x000001))
#define SJA1105_PORT_STATUS_MIIx(port)                           (0x100900 + ((port) * 0x000001))

//Device ID register
#define SJA1105_DEVICE_ID_SJA1105PEL                             0xAF00030E
#define SJA1105_DEVICE_ID_SJA1105QEL                             0xAE00030E
#define SJA1105_DEVICE_ID_SJA1105REL                             0xAF00030E
#define SJA1105_DEVICE_ID_SJA1105SEL                             0xAE00030E

//Initial Device Configuration Flag register
#define SJA1105_INIT_DEV_CONFIG_FLAG_CONFIGS                     0x80000000
#define SJA1105_INIT_DEV_CONFIG_FLAG_CRCCHKL                     0x40000000
#define SJA1105_INIT_DEV_CONFIG_FLAG_IDS                         0x20000000
#define SJA1105_INIT_DEV_CONFIG_FLAG_CRCCHKG                     0x10000000
#define SJA1105_INIT_DEV_CONFIG_FLAG_NSLOT                       0x0000000F

//VL Route/Partition Status register
#define SJA1105_VL_ROUTE_PARTITION_STATUS_VLIND                  0xFFFF0000
#define SJA1105_VL_ROUTE_PARTITION_STATUS_VLPARIND               0x0000FF00
#define SJA1105_VL_ROUTE_PARTITION_STATUS_VLROUTES               0x00000002
#define SJA1105_VL_ROUTE_PARTITION_STATUS_VLPARTS                0x00000001

//General Status 1 register
#define SJA1105_GENERAL_STATUS1_MACADDRL                         0xFFFF0000
#define SJA1105_GENERAL_STATUS1_PORTENF                          0x0000FF00
#define SJA1105_GENERAL_STATUS1_FWDS                             0x00000010
#define SJA1105_GENERAL_STATUS1_MACFDS                           0x00000008
#define SJA1105_GENERAL_STATUS1_ENFFDS                           0x00000004
#define SJA1105_GENERAL_STATUS1_L2BUSYFDS                        0x00000002
#define SJA1105_GENERAL_STATUS1_L2BUSYS                          0x00000001

//General Status 2 register
#define SJA1105_GENERAL_STATUS2_MACADDRU                         0xFFFFFFFF

//General Status 3 register
#define SJA1105_GENERAL_STATUS3_MACADDHCL                        0xFFFF0000
#define SJA1105_GENERAL_STATUS3_VLANIDHC                         0x0000FFF0
#define SJA1105_GENERAL_STATUS3_CONFLICT                         0x00000001

//General Status 4 register
#define SJA1105_GENERAL_STATUS4_MACADDHCU                        0xFFFFFFFF

//General Status 5 register
#define SJA1105_GENERAL_STATUS5_WPVLANID                         0xFFFF0000
#define SJA1105_GENERAL_STATUS5_PORT                             0x0000FF00
#define SJA1105_GENERAL_STATUS5_VLANBUSYS                        0x00000010
#define SJA1105_GENERAL_STATUS5_WRONGPORTS                       0x00000008
#define SJA1105_GENERAL_STATUS5_VNOTFOUNDS                       0x00000004

//General Status 6 register
#define SJA1105_GENERAL_STATUS6_VLID                             0xFFFF0000
#define SJA1105_GENERAL_STATUS6_PORTVL                           0x0000FF00
#define SJA1105_GENERAL_STATUS6_VLNOTFOUND                       0x00000001

//General Status 7 register
#define SJA1105_GENERAL_STATUS7_EMPTYS                           0x80000000
#define SJA1105_GENERAL_STATUS7_HANDLES                          0x7FFFFFFF

//General Status 8 register
#define SJA1105_GENERAL_STATUS8_HLWMARK                          0x7FFFFFFF

//General Status 9 register
#define SJA1105_GENERAL_STATUS9_PORT                             0x0000FF00
#define SJA1105_GENERAL_STATUS9_FWDS                             0x00000002
#define SJA1105_GENERAL_STATUS9_PARTS                            0x00000001

//General Status 10 register
#define SJA1105_GENERAL_STATUS10_RAMPARERRL                      0x007FFFFF

//General Status 11 register
#define SJA1105_GENERAL_STATUS11_RAMPARERRU                      0x0000001F

//Timestamp x register
#define SJA1105_PTP_TSx_UPDATE                                   0x00000001

//Synchronization Engine Status 0 register
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE                    0xFFFFFFFF
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE_INIT               0x00000000
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE_INTEGRATE          0x00000001
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE_UNSYNC             0x00000002
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE_WAIT_4_CYCLE_START 0x00000003
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE_CA_ENABLED         0x00000004
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE_WAIT_FOR_IN        0x00000005
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE_TENTATIVE_SYNC     0x00000006
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE_SYNC               0x00000007
#define SJA1105_SYNC_ENGINE_STATUS0_SYNCSTATE_STABLE             0x00000008

//Synchronization Engine Status 1 register
#define SJA1105_SYNC_ENGINE_STATUS1_INTEGCY                      0xFFFFFFFF

//Synchronization Engine Status 2 register
#define SJA1105_SYNC_ENGINE_STATUS2_ACTCORR                      0xFFFFFFFF

//Synchronization Engine Status 3 register
#define SJA1105_SYNC_ENGINE_STATUS3_MAXCORR                      0xFFFFFFFF

//Synchronization Engine Status 4 register
#define SJA1105_SYNC_ENGINE_STATUS4_MINCORR                      0xFFFFFFFF

//Synchronization Engine Status 5 register
#define SJA1105_SYNC_ENGINE_STATUS5_SYNCLOSS                     0x0000FFFF

//Synchronization Engine Status 6 register
#define SJA1105_SYNC_ENGINE_STATUS6_LOCMEM                       0x000000FF

//Synchronization Engine Status 7 register
#define SJA1105_SYNC_ENGINE_STATUS7_MINMEMVAR                    0x000000FF

//Synchronization Engine Status 8 register
#define SJA1105_SYNC_ENGINE_STATUS8_MAXMEMVAR                    0x000000FF

//Synchronization Engine Status 9 register
#define SJA1105_SYNC_ENGINE_STATUS9_LOCASYNMEM1                  0x00FF0000
#define SJA1105_SYNC_ENGINE_STATUS9_LOCASYNMEM0                  0x000000FF

//Synchronization Engine Status 10 register
#define SJA1105_SYNC_ENGINE_STATUS10_MINASYNMEM                  0x000000FF

//Synchronization Engine Status 11 register
#define SJA1105_SYNC_ENGINE_STATUS11_MAXASYNMEM                  0x000000FF

//Synchronization Engine Status 12 register
#define SJA1105_SYNC_ENGINE_STATUS12_CAMEM                       0x000000FF

//Synchronization Engine Status 13 register
#define SJA1105_SYNC_ENGINE_STATUS13_SYSNMEM                     0x000000FF

//Synchronization Engine Status 14 register
#define SJA1105_SYNC_ENGINE_STATUS14_MOFFSET0                    0xFFFFFFFF

//Synchronization Engine Status 15 register
#define SJA1105_SYNC_ENGINE_STATUS15_MOFFSET1                    0xFFFFFFFF

//Synchronization Engine Status 16 register
#define SJA1105_SYNC_ENGINE_STATUS16_MOFFSET2                    0xFFFFFFFF

//Synchronization Engine Status 17 register
#define SJA1105_SYNC_ENGINE_STATUS17_MOFFSET3                    0xFFFFFFFF

//Synchronization Engine Status 18 register
#define SJA1105_SYNC_ENGINE_STATUS18_MOFFSET4                    0xFFFFFFFF

//Synchronization Engine Status 19 register
#define SJA1105_SYNC_ENGINE_STATUS19_MOFFSET5                    0xFFFFFFFF

//Synchronization Engine Status 20 register
#define SJA1105_SYNC_ENGINE_STATUS20_MOFFSET6                    0xFFFFFFFF

//Synchronization Engine Status 21 register
#define SJA1105_SYNC_ENGINE_STATUS21_MOFFSET7                    0xFFFFFFFF

//Synchronization Engine Status 23 register
#define SJA1105_SYNC_ENGINE_STATUS23_TIMER                       0x3FFFFFFF

//Synchronization Engine Status 24 register
#define SJA1105_SYNC_ENGINE_STATUS24_CLOCK                       0x07FFFFFF

//VL Memory Partition Status x register
#define SJA1105_VL_MEM_PARTI_STATUSx_PARTDRPVL                   0x80000000
#define SJA1105_VL_MEM_PARTI_STATUSx_PARTSPCVL                   0x7FFFFFFF

//VL memory Partition Error Counter x register
#define SJA1105_VL_MEM_PART_ERR_CNTx_PARTDRPCNTVL                0xFFFFFFFF

//L2 Memory Partition Status x register
#define SJA1105_L2_MEM_PART_STATUSx_L2PARTS                      0x80000000
#define SJA1105_L2_MEM_PART_STATUSx_L2PSPC                       0x7FFFFFFF

//L2 Memory Partition Error Counter x register
#define SJA1105_L2_MEM_PART_ERR_CNTx_PARTDROP                    0xFFFFFFFF

//Port N MAC-level Diagnostic Counters register
#define SJA1105_PORTn_MAC_DIAG_CNT_N_RUNT                        0xFF000000
#define SJA1105_PORTn_MAC_DIAG_CNT_N_SOFERR                      0x00FF0000
#define SJA1105_PORTn_MAC_DIAG_CNT_N_ALIGNERR                    0x0000FF00
#define SJA1105_PORTn_MAC_DIAG_CNT_N_MIIERR                      0x000000FF

//Port N MAC-level Diagnostic Flags register
#define SJA1105_PORTn_MAC_DIAG_FLAGS_TYPEERR                     0x08000000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_SIZEERR                     0x04000000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_TCTIMEOUT                   0x02000000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_PRIORERR                    0x01000000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_NOMASTER                    0x00800000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_MEMOV                       0x00400000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_MEMERR                      0x00200000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_INVTYP                      0x00080000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_INTCYOV                     0x00040000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_DOMERR                      0x00020000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_PCFBAGDROP                  0x00010000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_SPCPRIOR                    0x0000F000
#define SJA1105_PORTn_MAC_DIAG_FLAGS_AGEPRIOR                    0x00000F00
#define SJA1105_PORTn_MAC_DIAG_FLAGS_PORTDROP                    0x00000040
#define SJA1105_PORTn_MAC_DIAG_FLAGS_LENDROP                     0x00000020
#define SJA1105_PORTn_MAC_DIAG_FLAGS_BAGDROP                     0x00000010
#define SJA1105_PORTn_MAC_DIAG_FLAGS_POLIECERR                   0x00000008
#define SJA1105_PORTn_MAC_DIAG_FLAGS_DRPNONA664ERR               0x00000004
#define SJA1105_PORTn_MAC_DIAG_FLAGS_SPCERR                      0x00000002
#define SJA1105_PORTn_MAC_DIAG_FLAGS_AGEDRP                      0x00000001

//Port N QLEVEL0 register
#define SJA1105_PORTn_QLEVEL0_QLEVEL_HWM                         0x01FF0000
#define SJA1105_PORTn_QLEVEL0_QLEVEL                             0x000001FF

//Port N QLEVEL1 register
#define SJA1105_PORTn_QLEVEL1_QLEVEL_HWM                         0x01FF0000
#define SJA1105_PORTn_QLEVEL1_QLEVEL                             0x000001FF

//Port N QLEVEL2 register
#define SJA1105_PORTn_QLEVEL2_QLEVEL_HWM                         0x01FF0000
#define SJA1105_PORTn_QLEVEL2_QLEVEL                             0x000001FF

//Port N QLEVEL3 register
#define SJA1105_PORTn_QLEVEL3_QLEVEL_HWM                         0x01FF0000
#define SJA1105_PORTn_QLEVEL3_QLEVEL                             0x000001FF

//Port N QLEVEL4 register
#define SJA1105_PORTn_QLEVEL4_QLEVEL_HWM                         0x01FF0000
#define SJA1105_PORTn_QLEVEL4_QLEVEL                             0x000001FF

//Port N QLEVEL5 register
#define SJA1105_PORTn_QLEVEL5_QLEVEL_HWM                         0x01FF0000
#define SJA1105_PORTn_QLEVEL5_QLEVEL                             0x000001FF

//Port N QLEVEL6 register
#define SJA1105_PORTn_QLEVEL6_QLEVEL_HWM                         0x01FF0000
#define SJA1105_PORTn_QLEVEL6_QLEVEL                             0x000001FF

//Port N QLEVEL7 register
#define SJA1105_PORTn_QLEVEL7_QLEVEL_HWM                         0x01FF0000
#define SJA1105_PORTn_QLEVEL7_QLEVEL                             0x000001FF

//Virtual Link Status 0 register
#define SJA1105_VIRTUAL_LINK_STATUS0_TIMINGERR                   0xFFFF0000
#define SJA1105_VIRTUAL_LINK_STATUS0_UNRELEASED                  0x0000FFFF

//Virtual Link Status 1 register
#define SJA1105_VIRTUAL_LINK_STATUS1_LENGTHERR                   0x0000FFFF

//RAM Parity Check Configuration 0 register
#define SJA1105_RAM_PARITY_CHECK_CONFIG0_RPARINITL               0x007FFFFF

//RAM Parity Check Configuration 1 register
#define SJA1105_RAM_PARITY_CHECK_CONFIG1_RPARINITU               0x0000001F

//Ethernet Port Status Control 0 register
#define SJA1105_PORT_STATUS_CTRL0_CLEARPORT                      0x0000001F

//VL Status Control register
#define SJA1105_VL_STATUS_CTRL_CLEARVLS                          0x0000001F

//Ethernet Port Status Control 1 register
#define SJA1105_PORT_STATUS_CTRL1_INHIBITTX                      0x0000001F

//PTP Control 1 register
#define SJA1105_PTP_CTRL1_VALID                                  0x80000000
#define SJA1105_PTP_CTRL1_PTPSTRTSCH                             0x40000000
#define SJA1105_PTP_CTRL1_PTPSTOPSCH                             0x20000000
#define SJA1105_PTP_CTRL1_STARTPTPCP                             0x10000000
#define SJA1105_PTP_CTRL1_STOPPTPCP                              0x08000000
#define SJA1105_PTP_CTRL1_CASSYNC                                0x04000000
#define SJA1105_PTP_CTRL1_RESPTP                                 0x00000008
#define SJA1105_PTP_CTRL1_CORRCLK4TS                             0x00000004
#define SJA1105_PTP_CTRL1_PTPCLKSUB                              0x00000002
#define SJA1105_PTP_CTRL1_PTPCLKADD                              0x00000001

//PTP Control 2 register
#define SJA1105_PTP_CTRL2_PTPSCHTML                              0xFFFFFFFF

//PTP Control 3 register
#define SJA1105_PTP_CTRL3_PTPSCHTMU                              0xFFFFFFFF

//PTP Control 4 register
#define SJA1105_PTP_CTRL4_PTPPINSTL                              0xFFFFFFFF

//PTP Control 5 register
#define SJA1105_PTP_CTRL5_PTPPINSTU                              0xFFFFFFFF

//PTP Control 6 register
#define SJA1105_PTP_CTRL6_PTPPINDUR                              0xFFFFFFFF

//PTP Control 7 register
#define SJA1105_PTP_CTRL7_PTPCLKVALL                             0xFFFFFFFF

//PTP Control 8 register
#define SJA1105_PTP_CTRL8_PTPCLKVALU                             0xFFFFFFFF

//PTP Control 9 register
#define SJA1105_PTP_CTRL9_PTPCLKRATE                             0xFFFFFFFF

//PTP Control 10 register
#define SJA1105_PTP_CTRL10_PTPTSCLKL                             0xFFFFFFFF

//PTP Control 11 register
#define SJA1105_PTP_CTRL11_PTPTSCLKU                             0xFFFFFFFF

//PTP Control 12 register
#define SJA1105_PTP_CTRL12_PTPCLKCORP                            0xFFFFFFFF

//PTP Control 13 register
#define SJA1105_PTP_CTRL13_PTPSYNCTSL                            0xFFFFFFFF

//PTP Control 14 register
#define SJA1105_PTP_CTRL14_PTPSYNCTSU                            0xFFFFFFFF

//L2 Address Lookup Table Reconfiguration 0 register
#define SJA1105_L2_ADDR_LUT_RECONFIG0_VALID                      0x80000000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_RDWRSET                    0x40000000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_ERRORS                     0x20000000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_LOCKEDS                    0x10000000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_VALIDENT                   0x08000000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_MGMTROUTE                  0x04000000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_HOSTCMD                    0x03800000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_HOSTCMD_SEARCH             0x00800000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_HOSTCMD_READ               0x01000000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_HOSTCMD_WRITE              0x01800000
#define SJA1105_L2_ADDR_LUT_RECONFIG0_HOSTCMD_INVALIDATE         0x02000000

//L2 Address Lookup Table Reconfiguration 1 register
#define SJA1105_L2_ADDR_LUT_RECONFIG1_ENTRY_25_0                 0xFFFFFFC0

//L2 Address Lookup Table Reconfiguration 2 register
#define SJA1105_L2_ADDR_LUT_RECONFIG2_ENTRY_57_26                0xFFFFFFC0

//L2 Address Lookup Table Reconfiguration 3 register
#define SJA1105_L2_ADDR_LUT_RECONFIG3_ENTRY_89_58                0xFFFFFFC0

//L2 Address Lookup Table Reconfiguration 4 register
#define SJA1105_L2_ADDR_LUT_RECONFIG4_ENTRY_121_90               0xFFFFFFC0

//L2 Address Lookup Table Reconfiguration 5 register
#define SJA1105_L2_ADDR_LUT_RECONFIG5_ENTRY_153_122              0xFFFFFFC0

//L2 Forwarding Table Reconfiguration 0 register
#define SJA1105_L2_FWD_TABLE_RECONFIG0_VALID                     0x80000000
#define SJA1105_L2_FWD_TABLE_RECONFIG0_ERRORS                    0x40000000
#define SJA1105_L2_FWD_TABLE_RECONFIG0_RDWRSET                   0x20000000
#define SJA1105_L2_FWD_TABLE_RECONFIG0_INDEX                     0x0000001F

//L2 Forwarding Table Reconfiguration 1 register
#define SJA1105_L2_FWD_TABLE_RECONFIG1_ENTRY_31_25               0xFE000000

//L2 Forwarding Table Reconfiguration 2 register
#define SJA1105_L2_FWD_TABLE_RECONFIG2_ENTRY_63_32               0xFFFFFFFF

//VLAN Lookup Table Reconfiguration 0 register
#define SJA1105_VLAN_LUT_RECONFIG0_VALID                         0x80000000
#define SJA1105_VLAN_LUT_RECONFIG0_RDWRSET                       0x40000000
#define SJA1105_VLAN_LUT_RECONFIG0_VALIDENT                      0x08000000

//VLAN Lookup Table Reconfiguration 1 register
#define SJA1105_VLAN_LUT_RECONFIG1_ENTRY_4_0                     0xF8000000

//VLAN Lookup Table Reconfiguration 2 register
#define SJA1105_VLAN_LUT_RECONFIG2_ENTRY_36_5                    0xFFFFFFFF

//MAC Configuration Table Reconfiguration 0 register
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG0_VALID                 0x80000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG0_ERRORS                0x40000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG0_RDWRSET               0x20000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG0_PORTIDX               0x00000007

//MAC Configuration Table Reconfiguration 1 register
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG1_ENTRY_18_0            0xFFFFE000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG1_INGRESS               0x80000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG1_MIRRCIE               0x40000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG1_MIRRCETAG             0x20000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG1_INGMIRRVID            0x1FFE0000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG1_INGMIRRPCP            0x0001C000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG1_INGMIRRDEI            0x00002000

//MAC Configuration Table Reconfiguration 2 register
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_ENTRY_50_19           0xFFFFFFFF
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_MAXAGE_LSB            0xFE000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_VLANPRIO              0x01C00000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_VLANID                0x003FFC00
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_ING_MIRR              0x00000200
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_EGR_MIRR              0x00000100
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_DRPNONA664            0x00000080
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_DRPDTAG               0x00000040
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_DRPSOTAG              0x00000020
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_DRPSITAG              0x00000010
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_DRPUNTAG              0x00000008
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_RETAG                 0x00000004
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_DYN_LEARN             0x00000002
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG2_EGRESS                0x00000001

//MAC Configuration Table Reconfiguration 3 register
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG3_ENTRY_82_51           0xFFFFFFFF
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG3_TP_DELIN_LSB          0xFFFE0000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG3_TP_DELOUT             0x0001FFFE
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG3_MAXAGE_MSB            0x00000001

//MAC Configuration Table Reconfiguration 4 register
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_ENTRY_114_83          0xFFFFFFFF
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_BASE1_LSB             0xF0000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_ENABLED1              0x08000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_TOP0                  0x07FC0000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_BASE0                 0x0003FE00
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_ENABLED0              0x00000100
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_IFG                   0x000000F8
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_SPEED                 0x00000006
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_SPEED_1GBPS           0x00000002
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_SPEED_100MBPS         0x00000004
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_SPEED_10MBPS          0x00000006
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG4_TP_DELIN_MSB          0x00000001

//MAC Configuration Table Reconfiguration 5 register
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG5_ENTRY_146_115         0xFFFFFFFF
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG5_TOP2_LSB              0xFF000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG5_BASE2                 0x00FF8000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG5_ENABLED2              0x00004000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG5_TOP1                  0x00003FE0
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG5_BASE1_MSB             0x0000001F

//MAC Configuration Table Reconfiguration 6 register
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG6_ENTRY_178_147         0xFFFFFFFF
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG6_TOP4_LSB              0xC0000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG6_BASE4                 0x3FE00000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG6_ENABLED4              0x00100000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG6_TOP3                  0x000FF800
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG6_BASE3                 0x000007FC
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG6_ENABLED3              0x00000002
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG6_TOP2_MSB              0x00000001

//MAC Configuration Table Reconfiguration 7 register
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG7_ENTRY_210_179         0xFFFFFFFF
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG7_BASE6_LSB             0xF8000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG7_ENABLED6              0x04000000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG7_TOP5                  0x03FE0000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG7_BASE5                 0x0001FF00
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG7_ENABLED5              0x00000080
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG7_TOP4_MSB              0x0000007F

//MAC Configuration Table Reconfiguration 8 register
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG8_ENTRY_242_211         0xFFFFFFFF
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG8_TOP7                  0xFF800000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG8_BASE7                 0x007FC000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG8_ENABLED7              0x00002000
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG8_TOP6                  0x00001FF0
#define SJA1105_MAC_CONFIG_TABLE_RECONFIG8_BASE6_MSB             0x0000000F

//Retagging Table Reconfiguration 0 register
#define SJA1105_RETAGGING_TABLE_RECONFIG0_VALID                  0x80000000
#define SJA1105_RETAGGING_TABLE_RECONFIG0_ERRORS                 0x40000000
#define SJA1105_RETAGGING_TABLE_RECONFIG0_VALIDENT               0x20000000
#define SJA1105_RETAGGING_TABLE_RECONFIG0_RDWRSET                0x10000000
#define SJA1105_RETAGGING_TABLE_RECONFIG0_INDEX                  0x0000003F

//Retagging Table Reconfiguration 1 register
#define SJA1105_RETAGGING_TABLE_RECONFIG1_ENTRY_8_0              0xFF800000

//Retagging Table Reconfiguration 2 register
#define SJA1105_RETAGGING_TABLE_RECONFIG2_ENTRY_40_9             0xFFFFFFFF

//General Parameters Reconfiguration 0 register
#define SJA1105_GENERAL_PARAMS_RECONFIG0_VALID                   0x80000000
#define SJA1105_GENERAL_PARAMS_RECONFIG0_ERRORS                  0x40000000
#define SJA1105_GENERAL_PARAMS_RECONFIG0_MIRR_ERR                0x20000000
#define SJA1105_GENERAL_PARAMS_RECONFIG0_RDWRSET                 0x10000000

//General Parameters Reconfiguration 1 register
#define SJA1105_GENERAL_PARAMS_RECONFIG1_ENTRY_9_0               0xFFC00000

//General Parameters Reconfiguration 2 register
#define SJA1105_GENERAL_PARAMS_RECONFIG2_ENTRY_41_10             0xFFFFFFFF

//General Parameters Reconfiguration 3 register
#define SJA1105_GENERAL_PARAMS_RECONFIG3_ENTRY_73_42             0xFFFFFFFF

//General Parameters Reconfiguration 4 register
#define SJA1105_GENERAL_PARAMS_RECONFIG4_ENTRY_105_74            0xFFFFFFFF

//General Parameters Reconfiguration 5 register
#define SJA1105_GENERAL_PARAMS_RECONFIG5_ENTRY_137_106           0xFFFFFFFF

//General Parameters Reconfiguration 6 register
#define SJA1105_GENERAL_PARAMS_RECONFIG6_ENTRY_169_138           0xFFFFFFFF

//General Parameters Reconfiguration 7 register
#define SJA1105_GENERAL_PARAMS_RECONFIG7_ENTRY_201_170           0xFFFFFFFF

//General Parameters Reconfiguration 8 register
#define SJA1105_GENERAL_PARAMS_RECONFIG8_ENTRY_233_202           0xFFFFFFFF

//General Parameters Reconfiguration 9 register
#define SJA1105_GENERAL_PARAMS_RECONFIG9_ENTRY_265_234           0xFFFFFFFF

//General Parameters Reconfiguration 10 register
#define SJA1105_GENERAL_PARAMS_RECONFIG10_ENTRY_297_266          0xFFFFFFFF

//General Parameters Reconfiguration 11 register
#define SJA1105_GENERAL_PARAMS_RECONFIG11_ENTRY_329_298          0xFFFFFFFF

//VL Lookup Table Reconfiguration 0 register
#define SJA1105_VL_LUT_RECONFIG0_VALID                           0x80000000
#define SJA1105_VL_LUT_RECONFIG0_ERRORS                          0x40000000
#define SJA1105_VL_LUT_RECONFIG0_RDWRSET                         0x20000000
#define SJA1105_VL_LUT_RECONFIG0_INDEX                           0x000003FF

//VL Lookup Table Reconfiguration 1 register
#define SJA1105_VL_LUT_RECONFIG1_ENTRY_7_0                       0xFF000000

//VL Lookup Table Reconfiguration 2 register
#define SJA1105_VL_LUT_RECONFIG2_ENTRY_39_8                      0xFFFFFFFF

//VL Lookup Table Reconfiguration 3 register
#define SJA1105_VL_LUT_RECONFIG3_ENTRY_71_40                     0xFFFFFFFF

//L2 Lookup Parameters 0 register
#define SJA1105_L2_LOOKUP_PARAMS0_VALID                          0x80000000
#define SJA1105_L2_LOOKUP_PARAMS0_RDWRSET                        0x40000000

//L2 Lookup Parameters 1 register
#define SJA1105_L2_LOOKUP_PARAMS1_ENTRY_9_0                      0xFFC00000

//L2 Lookup Parameters 2 register
#define SJA1105_L2_LOOKUP_PARAMS2_ENTRY_41_10                    0xFFFFFFFF

//L2 Lookup Parameters 3 register
#define SJA1105_L2_LOOKUP_PARAMS3_ENTRY_73_42                    0xFFFFFFFF

//L2 Lookup Parameters 4 register
#define SJA1105_L2_LOOKUP_PARAMS4_ENTRY_105_74                   0xFFFFFFFF

//AVB Parameters Reconfiguration 0 register
#define SJA1105_AVB_PARAMS_RECONFIG0_VALID                       0x80000000
#define SJA1105_AVB_PARAMS_RECONFIG0_ERRORS                      0x40000000
#define SJA1105_AVB_PARAMS_RECONFIG0_RDWRSET                     0x20000000

//AVB Parameters Reconfiguration 1 register
#define SJA1105_AVB_PARAMS_RECONFIG1_ENTRY_1_0                   0xC0000000

//AVB Parameters Reconfiguration 2 register
#define SJA1105_AVB_PARAMS_RECONFIG2_ENTRY_33_2                  0xFFFFFFFF

//AVB Parameters Reconfiguration 3 register
#define SJA1105_AVB_PARAMS_RECONFIG3_ENTRY_65_34                 0xFFFFFFFF

//AVB Parameters Reconfiguration 4 register
#define SJA1105_AVB_PARAMS_RECONFIG4_ENTRY_97_66                 0xFFFFFFFF

//Credit-Based Shaping Reconfiguration 0 register
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG0_VALID             0x80000000
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG0_RDWRSET           0x40000000
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG0_ERRORS            0x20000000
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG0_CBSIDX            0x0000000F

//Credit-Based Shaping Reconfiguration 1 register
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG1_ENTRY_5_0         0xFC000000

//Credit-Based Shaping Reconfiguration 2 register
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG2_ENTRY_37_6        0xFFFFFFFF

//Credit-Based Shaping Reconfiguration 3 register
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG3_ENTRY_69_38       0xFFFFFFFF

//Credit-Based Shaping Reconfiguration 4 register
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG4_ENTRY_101_70      0xFFFFFFFF

//Credit-Based Shaping Reconfiguration 5 register
#define SJA1105_CREDIT_BASED_SHAPING_RECONFIG5_ENTRY_133_102     0xFFFFFFFF

//L2 Forwarding Parameters Reconfiguration 0 register
#define SJA1105_L2_FWD_PARAMS_RECONFIG0_ENTRY_18_0               0xFFFFE000

//L2 Forwarding Parameters Reconfiguration 1 register
#define SJA1105_L2_FWD_PARAMS_RECONFIG1_ENTRY_50_19              0xFFFFFFFF

//L2 Forwarding Parameters Reconfiguration 2 register
#define SJA1105_L2_FWD_PARAMS_RECONFIG2_ENTRY_82_51              0xFFFFFFFF

//Reset Control register
#define SJA1105_RESET_CTRL_SWITCH_RST                            0x00000100
#define SJA1105_RESET_CTRL_CFG_RST                               0x00000080
#define SJA1105_RESET_CTRL_CAR_RST                               0x00000020
#define SJA1105_RESET_CTRL_OTP_RST                               0x00000010
#define SJA1105_RESET_CTRL_WARM_RST                              0x00000008
#define SJA1105_RESET_CTRL_COLD_RST                              0x00000004
#define SJA1105_RESET_CTRL_POR_RST                               0x00000002

//RGU Unit Disable register
#define SJA1105_RGU_UNIT_DISABLE_DISABLE_FLAG                    0x00000001

//PLLx Status register
#define SJA1105_PLL_x_S_LOCK                                     0x00000001

//PLLx Control register
#define SJA1105_PLL_x_C_PLLCLKSRC                                0x1F000000
#define SJA1105_PLL_x_C_PLLCLKSRC_XO66M_0                        0x0A000000
#define SJA1105_PLL_x_C_MSEL                                     0x00FF0000
#define SJA1105_PLL_x_C_MSEL_DIV1                                0x00000000
#define SJA1105_PLL_x_C_MSEL_DIV2                                0x00010000
#define SJA1105_PLL_x_C_MSEL_DIV3                                0x00020000
#define SJA1105_PLL_x_C_MSEL_DIV4                                0x00030000
#define SJA1105_PLL_x_C_NSEL                                     0x00003000
#define SJA1105_PLL_x_C_NSEL_DIV1                                0x00000000
#define SJA1105_PLL_x_C_NSEL_DIV2                                0x00001000
#define SJA1105_PLL_x_C_NSEL_DIV3                                0x00002000
#define SJA1105_PLL_x_C_NSEL_DIV4                                0x00003000
#define SJA1105_PLL_x_C_AUTOBLOCK                                0x00000800
#define SJA1105_PLL_x_C_PSEL                                     0x00000300
#define SJA1105_PLL_x_C_PSEL_DIV1                                0x00000000
#define SJA1105_PLL_x_C_PSEL_DIV2                                0x00000100
#define SJA1105_PLL_x_C_PSEL_DIV3                                0x00000200
#define SJA1105_PLL_x_C_PSEL_DIV4                                0x00000300
#define SJA1105_PLL_x_C_DIRECT                                   0x00000080
#define SJA1105_PLL_x_C_FBSEL                                    0x00000040
#define SJA1105_PLL_x_C_P23EN                                    0x00000004
#define SJA1105_PLL_x_C_BYPASS                                   0x00000002
#define SJA1105_PLL_x_C_PD                                       0x00000001

//IDIVx Control register
#define SJA1105_IDIV_x_C_CLKSRC                                  0x1F000000
#define SJA1105_IDIV_x_C_CLKSRC_XO66M_0                          0x0A000000
#define SJA1105_IDIV_x_C_AUTOBLOCK                               0x00000800
#define SJA1105_IDIV_x_C_IDIV                                    0x000003FC
#define SJA1105_IDIV_x_C_IDIV_DIV1                               0x00000000
#define SJA1105_IDIV_x_C_IDIV_DIV2                               0x00000004
#define SJA1105_IDIV_x_C_IDIV_DIV3                               0x00000008
#define SJA1105_IDIV_x_C_IDIV_DIV4                               0x0000000C
#define SJA1105_IDIV_x_C_IDIV_DIV5                               0x00000010
#define SJA1105_IDIV_x_C_IDIV_DIV6                               0x00000014
#define SJA1105_IDIV_x_C_IDIV_DIV7                               0x00000018
#define SJA1105_IDIV_x_C_IDIV_DIV8                               0x0000001C
#define SJA1105_IDIV_x_C_IDIV_DIV9                               0x00000020
#define SJA1105_IDIV_x_C_IDIV_DIV10                              0x00000024
#define SJA1105_IDIV_x_C_PD                                      0x00000001

//MIIx Clock Control register
#define SJA1105_MIIx_CLK_CTRL_CLKSRC                             0x1F000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_TX_CLK_0                    0x00000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_RX_CLK_0                    0x01000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_TX_CLK_1                    0x02000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_RX_CLK_1                    0x03000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_TX_CLK_2                    0x04000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_RX_CLK_2                    0x05000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_TX_CLK_3                    0x06000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_RX_CLK_3                    0x07000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_TX_CLK_4                    0x08000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_RX_CLK_4                    0x09000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_XO66M_0                     0x0A000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_PLL0                        0x0B000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_PLL0_120                    0x0C000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_PLL0_240                    0x0D000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_PLL1                        0x0E000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_PLL1_120                    0x0F000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_PLL1_240                    0x10000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIV0                       0x11000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIV1                       0x12000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIV2                       0x13000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIV3                       0x14000000
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIV4                       0x15000000
#define SJA1105_MIIx_CLK_CTRL_AUTOBLOCK                          0x00000800
#define SJA1105_MIIx_CLK_CTRL_PD                                 0x00000001

//Internal clock selection
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_TX_CLK_x(n)                 (((n) * 2) << 24)
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_RX_CLK_x(n)                 (((n) * 2 + 0x01) << 24)
#define SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIVx(n)                    (((n) + 0x11) << 24)

//CFG_PAD_MIIx_TX register
#define SJA1105_CFG_PAD_MIIx_TX_D32_OS                           0x18000000
#define SJA1105_CFG_PAD_MIIx_TX_D32_OS_LOW                       0x00000000
#define SJA1105_CFG_PAD_MIIx_TX_D32_OS_MEDIUM                    0x08000000
#define SJA1105_CFG_PAD_MIIx_TX_D32_OS_FAST                      0x10000000
#define SJA1105_CFG_PAD_MIIx_TX_D32_OS_HIGH                      0x18000000
#define SJA1105_CFG_PAD_MIIx_TX_D32_IPUD                         0x03000000
#define SJA1105_CFG_PAD_MIIx_TX_D32_IPUD_PULL_UP                 0x00000000
#define SJA1105_CFG_PAD_MIIx_TX_D32_IPUD_REPEATER                0x01000000
#define SJA1105_CFG_PAD_MIIx_TX_D32_IPUD_PLAIN                   0x02000000
#define SJA1105_CFG_PAD_MIIx_TX_D32_IPUD_PULL_DOWN               0x03000000
#define SJA1105_CFG_PAD_MIIx_TX_D10_OS                           0x00180000
#define SJA1105_CFG_PAD_MIIx_TX_D10_OS_LOW                       0x00000000
#define SJA1105_CFG_PAD_MIIx_TX_D10_OS_MEDIUM                    0x00080000
#define SJA1105_CFG_PAD_MIIx_TX_D10_OS_FAST                      0x00100000
#define SJA1105_CFG_PAD_MIIx_TX_D10_OS_HIGH                      0x00180000
#define SJA1105_CFG_PAD_MIIx_TX_D10_IPUD                         0x00030000
#define SJA1105_CFG_PAD_MIIx_TX_D10_IPUD_PULL_UP                 0x00000000
#define SJA1105_CFG_PAD_MIIx_TX_D10_IPUD_REPEATER                0x00010000
#define SJA1105_CFG_PAD_MIIx_TX_D10_IPUD_PLAIN                   0x00020000
#define SJA1105_CFG_PAD_MIIx_TX_D10_IPUD_PULL_DOWN               0x00030000
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_OS                          0x00001800
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_OS_LOW                      0x00000000
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_OS_MEDIUM                   0x00000800
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_OS_FAST                     0x00001000
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_OS_HIGH                     0x00001800
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_IPUD                        0x00000300
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_IPUD_PULL_UP                0x00000000
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_IPUD_REPEATER               0x00000100
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_IPUD_PLAIN                  0x00000200
#define SJA1105_CFG_PAD_MIIx_TX_CTRL_IPUD_PULL_DOWN              0x00000300
#define SJA1105_CFG_PAD_MIIx_TX_CLK_OS                           0x00000018
#define SJA1105_CFG_PAD_MIIx_TX_CLK_OS_LOW                       0x00000000
#define SJA1105_CFG_PAD_MIIx_TX_CLK_OS_MEDIUM                    0x00000008
#define SJA1105_CFG_PAD_MIIx_TX_CLK_OS_FAST                      0x00000010
#define SJA1105_CFG_PAD_MIIx_TX_CLK_OS_HIGH                      0x00000018
#define SJA1105_CFG_PAD_MIIx_TX_CLK_IH                           0x00000004
#define SJA1105_CFG_PAD_MIIx_TX_CLK_IPUD                         0x00000003
#define SJA1105_CFG_PAD_MIIx_TX_CLK_IPUD_PULL_UP                 0x00000000
#define SJA1105_CFG_PAD_MIIx_TX_CLK_IPUD_REPEATER                0x00000001
#define SJA1105_CFG_PAD_MIIx_TX_CLK_IPUD_PLAIN                   0x00000002
#define SJA1105_CFG_PAD_MIIx_TX_CLK_IPUD_PULL_DOWN               0x00000003

//CFG_PAD_MIIx_RX register
#define SJA1105_CFG_PAD_MIIx_RX_D32_IH                           0x04000000
#define SJA1105_CFG_PAD_MIIx_RX_D32_IPUD                         0x03000000
#define SJA1105_CFG_PAD_MIIx_RX_D32_IPUD_PULL_UP                 0x00000000
#define SJA1105_CFG_PAD_MIIx_RX_D32_IPUD_REPEATER                0x01000000
#define SJA1105_CFG_PAD_MIIx_RX_D32_IPUD_PLAIN                   0x02000000
#define SJA1105_CFG_PAD_MIIx_RX_D32_IPUD_PULL_DOWN               0x03000000
#define SJA1105_CFG_PAD_MIIx_RX_D10_IH                           0x00040000
#define SJA1105_CFG_PAD_MIIx_RX_D10_IPUD                         0x00030000
#define SJA1105_CFG_PAD_MIIx_RX_D10_IPUD_PULL_UP                 0x00000000
#define SJA1105_CFG_PAD_MIIx_RX_D10_IPUD_REPEATER                0x00010000
#define SJA1105_CFG_PAD_MIIx_RX_D10_IPUD_PLAIN                   0x00020000
#define SJA1105_CFG_PAD_MIIx_RX_D10_IPUD_PULL_DOWN               0x00030000
#define SJA1105_CFG_PAD_MIIx_RX_CTRL_IH                          0x00000400
#define SJA1105_CFG_PAD_MIIx_RX_CTRL_IPUD                        0x00000300
#define SJA1105_CFG_PAD_MIIx_RX_CTRL_IPUD_PULL_UP                0x00000000
#define SJA1105_CFG_PAD_MIIx_RX_CTRL_IPUD_REPEATER               0x00000100
#define SJA1105_CFG_PAD_MIIx_RX_CTRL_IPUD_PLAIN                  0x00000200
#define SJA1105_CFG_PAD_MIIx_RX_CTRL_IPUD_PULL_DOWN              0x00000300
#define SJA1105_CFG_PAD_MIIx_RX_CLK_OS                           0x00000018
#define SJA1105_CFG_PAD_MIIx_RX_CLK_OS_LOW                       0x00000000
#define SJA1105_CFG_PAD_MIIx_RX_CLK_OS_MEDIUM                    0x00000008
#define SJA1105_CFG_PAD_MIIx_RX_CLK_OS_FAST                      0x00000010
#define SJA1105_CFG_PAD_MIIx_RX_CLK_OS_HIGH                      0x00000018
#define SJA1105_CFG_PAD_MIIx_RX_CLK_IH                           0x00000004
#define SJA1105_CFG_PAD_MIIx_RX_CLK_IPUD                         0x00000003
#define SJA1105_CFG_PAD_MIIx_RX_CLK_IPUD_PULL_UP                 0x00000000
#define SJA1105_CFG_PAD_MIIx_RX_CLK_IPUD_REPEATER                0x00000001
#define SJA1105_CFG_PAD_MIIx_RX_CLK_IPUD_PLAIN                   0x00000002
#define SJA1105_CFG_PAD_MIIx_RX_CLK_IPUD_PULL_DOWN               0x00000003

//CFG_PAD_MIIx_ID register
#define SJA1105_CFG_PAD_MIIx_ID_RXC_STABLE_OVR                   0x00008000
#define SJA1105_CFG_PAD_MIIx_ID_RXC_DELAY                        0x00007C00
#define SJA1105_CFG_PAD_MIIx_ID_RXC_DELAY_DEFAULT                0x00002000
#define SJA1105_CFG_PAD_MIIx_ID_RXC_BYPASS                       0x00000200
#define SJA1105_CFG_PAD_MIIx_ID_RXC_PD                           0x00000100
#define SJA1105_CFG_PAD_MIIx_ID_TXC_STABLE_OVR                   0x00000080
#define SJA1105_CFG_PAD_MIIx_ID_TXC_DELAY                        0x0000007C
#define SJA1105_CFG_PAD_MIIx_ID_TXC_DELAY_DEFAULT                0x00000020
#define SJA1105_CFG_PAD_MIIx_ID_TXC_BYPASS                       0x00000002
#define SJA1105_CFG_PAD_MIIx_ID_TXC_PD                           0x00000001

//CFG_PAD_MISC register
#define SJA1105_CFG_PAD_MISC_PTPCLK_OS                           0x00180000
#define SJA1105_CFG_PAD_MISC_PTPCLK_OS_LOW                       0x00000000
#define SJA1105_CFG_PAD_MISC_PTPCLK_OS_MEDIUM                    0x00080000
#define SJA1105_CFG_PAD_MISC_PTPCLK_OS_FAST                      0x00100000
#define SJA1105_CFG_PAD_MISC_PTPCLK_OS_HIGH                      0x00180000
#define SJA1105_CFG_PAD_MISC_PTPCLK_IH                           0x00040000
#define SJA1105_CFG_PAD_MISC_PTPCLK_IPUD                         0x00030000
#define SJA1105_CFG_PAD_MISC_PTPCLK_IPUD_PULL_UP                 0x00000000
#define SJA1105_CFG_PAD_MISC_PTPCLK_IPUD_REPEATER                0x00010000
#define SJA1105_CFG_PAD_MISC_PTPCLK_IPUD_PLAIN                   0x00020000
#define SJA1105_CFG_PAD_MISC_PTPCLK_IPUD_PULL_DOWN               0x00030000
#define SJA1105_CFG_PAD_MISC_RSTN_IH                             0x00000400
#define SJA1105_CFG_PAD_MISC_RSTN_IPUD                           0x00000300
#define SJA1105_CFG_PAD_MISC_RSTN_IPUD_PULL_UP                   0x00000000
#define SJA1105_CFG_PAD_MISC_RSTN_IPUD_REPEATER                  0x00000100
#define SJA1105_CFG_PAD_MISC_RSTN_IPUD_PLAIN                     0x00000200
#define SJA1105_CFG_PAD_MISC_RSTN_IPUD_PULL_DOWN                 0x00000300
#define SJA1105_CFG_PAD_MISC_CLKOUT_EN                           0x00000020
#define SJA1105_CFG_PAD_MISC_CLKOUT_OS                           0x00000018
#define SJA1105_CFG_PAD_MISC_CLKOUT_OS_LOW                       0x00000000
#define SJA1105_CFG_PAD_MISC_CLKOUT_OS_MEDIUM                    0x00000008
#define SJA1105_CFG_PAD_MISC_CLKOUT_OS_FAST                      0x00000010
#define SJA1105_CFG_PAD_MISC_CLKOUT_OS_HIGH                      0x00000018
#define SJA1105_CFG_PAD_MISC_CLKOUT_IPUD                         0x00000003
#define SJA1105_CFG_PAD_MISC_CLKOUT_IPUD_PULL_UP                 0x00000000
#define SJA1105_CFG_PAD_MISC_CLKOUT_IPUD_REPEATER                0x00000001
#define SJA1105_CFG_PAD_MISC_CLKOUT_IPUD_PLAIN                   0x00000002
#define SJA1105_CFG_PAD_MISC_CLKOUT_IPUD_PULL_DOWN               0x00000003

//CFG_PAD_SPI register
#define SJA1105_CFG_PAD_SPI_SDO_OS                               0x18000000
#define SJA1105_CFG_PAD_SPI_SDO_OS_LOW                           0x00000000
#define SJA1105_CFG_PAD_SPI_SDO_OS_MEDIUM                        0x08000000
#define SJA1105_CFG_PAD_SPI_SDO_OS_FAST                          0x10000000
#define SJA1105_CFG_PAD_SPI_SDO_OS_HIGH                          0x18000000
#define SJA1105_CFG_PAD_SPI_SDO_IPUD                             0x03000000
#define SJA1105_CFG_PAD_SPI_SDO_IPUD_PULL_UP                     0x00000000
#define SJA1105_CFG_PAD_SPI_SDO_IPUD_REPEATER                    0x01000000
#define SJA1105_CFG_PAD_SPI_SDO_IPUD_PLAIN                       0x02000000
#define SJA1105_CFG_PAD_SPI_SDO_IPUD_PULL_DOWN                   0x03000000
#define SJA1105_CFG_PAD_SPI_SDI_IH                               0x00040000
#define SJA1105_CFG_PAD_SPI_SDI_IPUD                             0x00030000
#define SJA1105_CFG_PAD_SPI_SDI_IPUD_PULL_UP                     0x00000000
#define SJA1105_CFG_PAD_SPI_SDI_IPUD_REPEATER                    0x00010000
#define SJA1105_CFG_PAD_SPI_SDI_IPUD_PLAIN                       0x00020000
#define SJA1105_CFG_PAD_SPI_SDI_IPUD_PULL_DOWN                   0x00030000
#define SJA1105_CFG_PAD_SPI_SSN_IH                               0x00000400
#define SJA1105_CFG_PAD_SPI_SSN_IPUD                             0x00000300
#define SJA1105_CFG_PAD_SPI_SSN_IPUD_PULL_UP                     0x00000000
#define SJA1105_CFG_PAD_SPI_SSN_IPUD_REPEATER                    0x00000100
#define SJA1105_CFG_PAD_SPI_SSN_IPUD_PLAIN                       0x00000200
#define SJA1105_CFG_PAD_SPI_SSN_IPUD_PULL_DOWN                   0x00000300
#define SJA1105_CFG_PAD_SPI_SCK_IH                               0x00000004
#define SJA1105_CFG_PAD_SPI_SCK_IH_PULL_UP                       0x00000000
#define SJA1105_CFG_PAD_SPI_SCK_IH_REPEATER                      0x00000004
#define SJA1105_CFG_PAD_SPI_SCK_IH_PLAIN                         0x00000008
#define SJA1105_CFG_PAD_SPI_SCK_IH_PULL_DOWN                     0x0000000C

//CFG_PAD_JTAG register
#define SJA1105_CFG_PAD_JTAG_TDO_IPUD                            0x03000000
#define SJA1105_CFG_PAD_JTAG_TDO_IPUD_PULL_UP                    0x00000000
#define SJA1105_CFG_PAD_JTAG_TDO_IPUD_REPEATER                   0x01000000
#define SJA1105_CFG_PAD_JTAG_TDO_IPUD_PLAIN                      0x02000000
#define SJA1105_CFG_PAD_JTAG_TDO_IPUD_PULL_DOWN                  0x03000000
#define SJA1105_CFG_PAD_JTAG_TDI_IPUD                            0x00030000
#define SJA1105_CFG_PAD_JTAG_TDI_IPUD_PULL_UP                    0x00000000
#define SJA1105_CFG_PAD_JTAG_TDI_IPUD_REPEATER                   0x00010000
#define SJA1105_CFG_PAD_JTAG_TDI_IPUD_PLAIN                      0x00020000
#define SJA1105_CFG_PAD_JTAG_TDI_IPUD_PULL_DOWN                  0x00030000
#define SJA1105_CFG_PAD_JTAG_TRSTNTMS_IPUD                       0x00000300
#define SJA1105_CFG_PAD_JTAG_TRSTNTMS_IPUD_PULL_UP               0x00000000
#define SJA1105_CFG_PAD_JTAG_TRSTNTMS_IPUD_REPEATER              0x00000100
#define SJA1105_CFG_PAD_JTAG_TRSTNTMS_IPUD_PLAIN                 0x00000200
#define SJA1105_CFG_PAD_JTAG_TRSTNTMS_IPUD_PULL_DOWN             0x00000300
#define SJA1105_CFG_PAD_JTAG_TCK_IPUD                            0x00000003
#define SJA1105_CFG_PAD_JTAG_TCK_IPUD_PULL_UP                    0x00000000
#define SJA1105_CFG_PAD_JTAG_TCK_IPUD_REPEATER                   0x00000001
#define SJA1105_CFG_PAD_JTAG_TCK_IPUD_PLAIN                      0x00000002
#define SJA1105_CFG_PAD_JTAG_TCK_IPUD_PULL_DOWN                  0x00000003

//PORT_STATUS_MIIx register
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_FREQ_MODE                0x00300000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_FREQ_MODE_2_5MHZ         0x00000000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_FREQ_MODE_25MHZ          0x00100000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_FREQ_MODE_50MHZ          0x00200000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_FREQ_MODE_125MHZ         0x00300000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_CTL_MODE                 0x000C0000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_CTL_MODE_BYPASS          0x00000000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_CTL_MODE_DELAYED         0x00040000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_CTL_MODE_DISABLED        0x00080000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_STABLE                   0x00020000
#define SJA1105_PORT_STATUS_MIIx_RXC_ID_PD                       0x00010000
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_FREQ_MODE                0x00003000
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_FREQ_MODE_2_5MHZ         0x00000000
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_FREQ_MODE_25MHZ          0x00001000
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_FREQ_MODE_50MHZ          0x00002000
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_FREQ_MODE_125MHZ         0x00003000
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_CTL_MODE                 0x00000C00
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_CTL_MODE_BYPASS          0x00000000
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_CTL_MODE_DELAYED         0x00000400
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_CTL_MODE_DISABLED        0x00000800
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_STABLE                   0x00000200
#define SJA1105_PORT_STATUS_MIIx_TXC_ID_PD                       0x00000100
#define SJA1105_PORT_STATUS_MIIx_SPEED                           0x00000018
#define SJA1105_PORT_STATUS_MIIx_SPEED_10MBPS                    0x00000000
#define SJA1105_PORT_STATUS_MIIx_SPEED_100MBPS                   0x00000008
#define SJA1105_PORT_STATUS_MIIx_SPEED_1GBPS                     0x00000010
#define SJA1105_PORT_STATUS_MIIx_MODE                            0x00000007
#define SJA1105_PORT_STATUS_MIIx_MODE_MII_MAC                    0x00000000
#define SJA1105_PORT_STATUS_MIIx_MODE_RMII_MAC                   0x00000001
#define SJA1105_PORT_STATUS_MIIx_MODE_MII_PHY                    0x00000004
#define SJA1105_PORT_STATUS_MIIx_MODE_RMII_PHY                   0x00000005
#define SJA1105_PORT_STATUS_MIIx_MODE_RGMII                      0x00000002
#define SJA1105_PORT_STATUS_MIIx_MODE_SGMII                      0x00000003
#define SJA1105_PORT_STATUS_MIIx_MODE_INACTIVE                   0x00000007

//TS_CONFIG register
#define SJA1105_TS_CONFIG_PD                                     0x00000040
#define SJA1105_TS_CONFIG_THRESHOLD                              0x0000003F
#define SJA1105_TS_CONFIG_THRESHOLD_DEFAULT                      0x00000025

//TS_STATUS register
#define SJA1105_TS_STATUS_EXCEEDED                               0x00000001

//PROD_CFG register
#define SJA1105_PROD_CFG_DISABLE_TTETH                           0x00000001

//PROD_ID register
#define SJA1105_PROD_ID_PART_NR                                  0x000FFFF0
#define SJA1105_PROD_ID_PART_NR_SJA1105P                         0x0009A840
#define SJA1105_PROD_ID_PART_NR_SJA1105Q                         0x0009A850
#define SJA1105_PROD_ID_PART_NR_SJA1105R                         0x0009A860
#define SJA1105_PROD_ID_PART_NR_SJA1105S                         0x0009A870
#define SJA1105_PROD_ID_VERSION                                  0x0000000F
#define SJA1105_PROD_ID_VERSION_DEFAULT                          0x00000001

//ACU Unit Disable register
#define SJA1105_ACU_UNIT_DISABLE_DISABLE_FLAG                    0x00000001

//BASIC_CONTROL register
#define SJA1105_BASIC_CONTROL_RESET                              0x8000
#define SJA1105_BASIC_CONTROL_LOOPBACK                           0x4000
#define SJA1105_BASIC_CONTROL_SPEED_SEL_LSB                      0x2000
#define SJA1105_BASIC_CONTROL_AUTONEG_ENABLE                     0x1000
#define SJA1105_BASIC_CONTROL_POWER_DOWN                         0x0800
#define SJA1105_BASIC_CONTROL_RE_AUTONEG                         0x0200
#define SJA1105_BASIC_CONTROL_DUPLEX_MODE                        0x0100
#define SJA1105_BASIC_CONTROL_SPEED_SEL_MSB                      0x0040

//BASIC_STATUS register
#define SJA1105_BASIC_STATUS_100BT4                              0x8000
#define SJA1105_BASIC_STATUS_100BTX_FD                           0x4000
#define SJA1105_BASIC_STATUS_100BTX_HD                           0x2000
#define SJA1105_BASIC_STATUS_10BT_FD                             0x1000
#define SJA1105_BASIC_STATUS_10BT_HD                             0x0800
#define SJA1105_BASIC_STATUS_100BT2_FD                           0x0400
#define SJA1105_BASIC_STATUS_100BT2_HD                           0x0200
#define SJA1105_BASIC_STATUS_EXTENDED_STATUS                     0x0100
#define SJA1105_BASIC_STATUS_UNIDIRECT_ABILITY                   0x0080
#define SJA1105_BASIC_STATUS_MF_PREAMBLE_SUPPR                   0x0040
#define SJA1105_BASIC_STATUS_AUTONEG_COMPLETE                    0x0020
#define SJA1105_BASIC_STATUS_REMOTE_FAULT                        0x0010
#define SJA1105_BASIC_STATUS_AUTONEG_ABILITY                     0x0008
#define SJA1105_BASIC_STATUS_LINK_STATUS                         0x0004
#define SJA1105_BASIC_STATUS_EXTENDED_CAPABILITY                 0x0001

//PHY_IDENTIFIER_1 register
#define SJA1105_PHY_IDENTIFIER_1_PHY_ID_MSB                      0xFFFF
#define SJA1105_PHY_IDENTIFIER_1_PHY_ID_MSB_DEFAULT              0xB018

//PHY_IDENTIFIER_2 register
#define SJA1105_PHY_IDENTIFIER_2_PHY_ID_LSB                      0xFC00
#define SJA1105_PHY_IDENTIFIER_2_PHY_ID_LSB_DEFAULT              0x0000
#define SJA1105_PHY_IDENTIFIER_2_TYPE_NO                         0x03F0
#define SJA1105_PHY_IDENTIFIER_2_TYPE_NO_DEFAULT                 0x02D0
#define SJA1105_PHY_IDENTIFIER_2_REVISION_NO                     0x000F
#define SJA1105_PHY_IDENTIFIER_2_REVISION_NO_DEFAULT             0x0000

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//SJA1105 Ethernet switch driver
extern const SwitchDriver sja1105SwitchDriver;

//SJA1105 related functions
error_t sja1105Init(NetInterface *interface);
error_t sja1105InitHook(NetInterface *interface);

error_t sja1105LoadStaticConfig(NetInterface *interface, const char_t *data,
   size_t length);

void sja1105ConfigurePll1(NetInterface *interface);
error_t sja1105ConfigureCgu(NetInterface *interface, uint8_t port);

void sja1105Tick(NetInterface *interface);

void sja1105EnableIrq(NetInterface *interface);
void sja1105DisableIrq(NetInterface *interface);

void sja1105EventHandler(NetInterface *interface);

error_t sja1105TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, NetTxAncillary *ancillary);

error_t sja1105UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, NetRxAncillary *ancillary);

bool_t sja1105GetLinkState(NetInterface *interface, uint8_t port);
uint32_t sja1105GetLinkSpeed(NetInterface *interface, uint8_t port);
NicDuplexMode sja1105GetDuplexMode(NetInterface *interface, uint8_t port);

void sja1105SetPortSpeed(NetInterface *interface, uint8_t port, uint32_t speed);

void sja1105SetPortState(NetInterface *interface, uint8_t port,
   SwitchPortState state);

SwitchPortState sja1105GetPortState(NetInterface *interface, uint8_t port);

void sja1105SetAgingTime(NetInterface *interface, uint32_t agingTime);

void sja1105EnableIgmpSnooping(NetInterface *interface, bool_t enable);
void sja1105EnableMldSnooping(NetInterface *interface, bool_t enable);
void sja1105EnableRsvdMcastTable(NetInterface *interface, bool_t enable);

error_t sja1105AddStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry);

error_t sja1105DeleteStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry);

error_t sja1105GetStaticFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry);

void sja1105FlushStaticFdbTable(NetInterface *interface);

error_t sja1105GetDynamicFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry);

void sja1105FlushDynamicFdbTable(NetInterface *interface, uint8_t port);

void sja1105SetUnknownMcastFwdPorts(NetInterface *interface,
   bool_t enable, uint32_t forwardPorts);

error_t sja1105WriteMacConfigEntry(NetInterface *interface, uint8_t port);
error_t sja1105ReadMacConfigEntry(NetInterface *interface, uint8_t port);

void sja1105WriteSingleReg(NetInterface *interface, uint32_t address,
   uint32_t data);

uint32_t sja1105ReadSingleReg(NetInterface *interface, uint32_t address);

void sja1105WriteMultipleRegs(NetInterface *interface, uint32_t address,
   const uint32_t *data, uint_t count);

void sja1105ReadMultipleRegs(NetInterface *interface, uint32_t address,
   uint32_t *data, uint_t count);

void sja1105DumpReg(NetInterface *interface);

void sja1105WritePhyReg(NetInterface *interface, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t sja1105ReadPhyReg(NetInterface *interface, uint8_t phyAddr,
   uint8_t regAddr);

void sja1105DumpPhyReg(NetInterface *interface, uint8_t phyAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
