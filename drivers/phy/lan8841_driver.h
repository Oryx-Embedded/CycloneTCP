/**
 * @file lan8841_driver.h
 * @brief LAN8841 Gigabit Ethernet PHY driver
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

#ifndef _LAN8841_DRIVER_H
#define _LAN8841_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef LAN8841_PHY_ADDR
   #define LAN8841_PHY_ADDR 3
#elif (LAN8841_PHY_ADDR < 0 || LAN8841_PHY_ADDR > 31)
   #error LAN8841_PHY_ADDR parameter is not valid
#endif

//LAN8841 PHY registers
#define LAN8841_BMCR                                       0x00
#define LAN8841_BMSR                                       0x01
#define LAN8841_PHYID1                                     0x02
#define LAN8841_PHYID2                                     0x03
#define LAN8841_ANAR                                       0x04
#define LAN8841_ANLPAR                                     0x05
#define LAN8841_ANER                                       0x06
#define LAN8841_ANNPTR                                     0x07
#define LAN8841_ANNPRR                                     0x08
#define LAN8841_GBCR                                       0x09
#define LAN8841_GBSR                                       0x0A
#define LAN8841_MMDACR                                     0x0D
#define LAN8841_MMDAADR                                    0x0E
#define LAN8841_GBESR                                      0x0F
#define LAN8841_RLB                                        0x11
#define LAN8841_LINKMD                                     0x12
#define LAN8841_DPMAPCSS                                   0x13
#define LAN8841_RXERCTR                                    0x15
#define LAN8841_LED_MODE_SEL                               0x16
#define LAN8841_LED_BEHAVIOR                               0x17
#define LAN8841_OUT_CTRL                                   0x19
#define LAN8841_LEGACY_LED_MODE                            0x1A
#define LAN8841_ISR                                        0x1B
#define LAN8841_AUTOMDI                                    0x1C
#define LAN8841_SPDC                                       0x1D
#define LAN8841_EXT_LOOPBACK                               0x1E
#define LAN8841_PHYCON                                     0x1F

//LAN8841 MMD registers
#define LAN8841_MEAN_SLICER_ERROR                          0x01, 0x00E1
#define LAN8841_DCQ_MEAN_SQUARE_ERROR                      0x01, 0x00E2
#define LAN8841_DCQ_MEAN_SQUARE_ERROR_WORST_CASE           0x01, 0x00E3
#define LAN8841_DCQ_SQI                                    0x01, 0x00E4
#define LAN8841_DCQ_PEAK_MSE                               0x01, 0x00E5
#define LAN8841_DCQ_CTRL                                   0x01, 0x00E6
#define LAN8841_DCQ_CONFIG                                 0x01, 0x00E7
#define LAN8841_DCQ_SQI_TABLE                              0x01, 0x00E8
#define LAN8841_COMMON_CTRL                                0x02, 0x0000
#define LAN8841_STRAP_STAT                                 0x02, 0x0001
#define LAN8841_OP_MODE_STRAP_OVERRIDE                     0x02, 0x0002
#define LAN8841_OP_MODE_STRAP                              0x02, 0x0003
#define LAN8841_CLOCK_INV_CTRL_SIGNAL_PAD_SKEW             0x02, 0x0004
#define LAN8841_RGMII_RX_DATA_PAD_SKEW                     0x02, 0x0005
#define LAN8841_RGMII_TX_DATA_PAD_SKEW                     0x02, 0x0006
#define LAN8841_CLK_PAD_SKEW                               0x02, 0x0008
#define LAN8841_SELF_TEST_PKT_COUNT_LO                     0x02, 0x0009
#define LAN8841_SELF_TEST_PKT_COUNT_HI                     0x02, 0x000A
#define LAN8841_SELF_TEST_STAT                             0x02, 0x000B
#define LAN8841_SELF_TEST_FRAME_COUNT_EN                   0x02, 0x000C
#define LAN8841_SELF_TEST_PGEN_EN                          0x02, 0x000D
#define LAN8841_SELF_TEST_EN                               0x02, 0x000E
#define LAN8841_WOL_CTRL                                   0x02, 0x0010
#define LAN8841_WOL_MAC_LO                                 0x02, 0x0011
#define LAN8841_WOL_MAC_MI                                 0x02, 0x0012
#define LAN8841_WOL_MAC_HI                                 0x02, 0x0013
#define LAN8841_CUSTOM_PKT0_CRC_LO                         0x02, 0x0014
#define LAN8841_CUSTOM_PKT0_CRC_HI                         0x02, 0x0015
#define LAN8841_CUSTOM_PKT1_CRC_LO                         0x02, 0x0016
#define LAN8841_CUSTOM_PKT1_CRC_HI                         0x02, 0x0017
#define LAN8841_CUSTOM_PKT2_CRC_LO                         0x02, 0x0018
#define LAN8841_CUSTOM_PKT2_CRC_HI                         0x02, 0x0019
#define LAN8841_CUSTOM_PKT3_CRC_LO                         0x02, 0x001A
#define LAN8841_CUSTOM_PKT3_CRC_HI                         0x02, 0x001B
#define LAN8841_CUSTOM_PKT0_MASK_LL                        0x02, 0x001C
#define LAN8841_CUSTOM_PKT0_MASK_LH                        0x02, 0x001D
#define LAN8841_CUSTOM_PKT0_MASK_HL                        0x02, 0x001E
#define LAN8841_CUSTOM_PKT0_MASK_HH                        0x02, 0x001F
#define LAN8841_CUSTOM_PKT1_MASK_LL                        0x02, 0x0020
#define LAN8841_CUSTOM_PKT1_MASK_LH                        0x02, 0x0021
#define LAN8841_CUSTOM_PKT1_MASK_HL                        0x02, 0x0022
#define LAN8841_CUSTOM_PKT1_MASK_HH                        0x02, 0x0023
#define LAN8841_CUSTOM_PKT2_MASK_LL                        0x02, 0x0024
#define LAN8841_CUSTOM_PKT2_MASK_LH                        0x02, 0x0025
#define LAN8841_CUSTOM_PKT2_MASK_HL                        0x02, 0x0026
#define LAN8841_CUSTOM_PKT2_MASK_HH                        0x02, 0x0027
#define LAN8841_CUSTOM_PKT3_MASK_LL                        0x02, 0x0028
#define LAN8841_CUSTOM_PKT3_MASK_LH                        0x02, 0x0029
#define LAN8841_CUSTOM_PKT3_MASK_HL                        0x02, 0x002A
#define LAN8841_CUSTOM_PKT3_MASK_HH                        0x02, 0x002B
#define LAN8841_WOL_CTRL_STAT                              0x02, 0x002C
#define LAN8841_WOL_CUSTOM_PKT_RECEIVE_STAT                0x02, 0x002D
#define LAN8841_WOL_MAGIC_PKT_RECEIVE_STAT                 0x02, 0x002E
#define LAN8841_WOL_DATA_MODULE_STAT                       0x02, 0x002F
#define LAN8841_CUSTOM_PKT0_RCVD_CRC_L                     0x02, 0x0030
#define LAN8841_CUSTOM_PKT0_RCVD_CRC_H                     0x02, 0x0031
#define LAN8841_CUSTOM_PKT1_RCVD_CRC_L                     0x02, 0x0032
#define LAN8841_CUSTOM_PKT1_RCVD_CRC_H                     0x02, 0x0033
#define LAN8841_CUSTOM_PKT2_RCVD_CRC_L                     0x02, 0x0034
#define LAN8841_CUSTOM_PKT2_RCVD_CRC_H                     0x02, 0x0035
#define LAN8841_CUSTOM_PKT3_RCVD_CRC_L                     0x02, 0x0036
#define LAN8841_CUSTOM_PKT3_RCVD_CRC_H                     0x02, 0x0037
#define LAN8841_SELF_TEST_CORRECT_COUNT_LO                 0x02, 0x003C
#define LAN8841_SELF_TEST_CORRECT_COUNT_HI                 0x02, 0x003D
#define LAN8841_SELF_TEST_ERROR_COUNT_LO                   0x02, 0x003E
#define LAN8841_SELF_TEST_ERROR_COUNT_HI                   0x02, 0x003F
#define LAN8841_RX_DLL_CTRL                                0x02, 0x004C
#define LAN8841_TX_DLL_CTRL                                0x02, 0x004D
#define LAN8841_DRIVING_STRENGTH                           0x02, 0x006F
#define LAN8841_GPIO_EN                                    0x02, 0x0080
#define LAN8841_GPIO_DIR                                   0x02, 0x0081
#define LAN8841_GPIO_BUF                                   0x02, 0x0082
#define LAN8841_GPIO_DATA_SEL1                             0x02, 0x0083
#define LAN8841_GPIO_DATA_SEL2                             0x02, 0x0084
#define LAN8841_GPIO_DATA                                  0x02, 0x0085
#define LAN8841_GPIO_INT_STS                               0x02, 0x0086
#define LAN8841_GPIO_INT_EN                                0x02, 0x0087
#define LAN8841_GPIO_INT_POL                               0x02, 0x0088
#define LAN8841_PTP_CMD_CTL                                0x02, 0x0100
#define LAN8841_PTP_GENERAL_CONFIG                         0x02, 0x0101
#define LAN8841_PTP_REF_CLK_CFG                            0x02, 0x0102
#define LAN8841_PTP_INT_STS                                0x02, 0x0103
#define LAN8841_PTP_INT_EN                                 0x02, 0x0104
#define LAN8841_PTP_MOD_ERR                                0x02, 0x0105
#define LAN8841_PTP_LTC_SET_SEC_HI                         0x02, 0x0106
#define LAN8841_PTP_LTC_SET_SEC_MID                        0x02, 0x0107
#define LAN8841_PTP_LTC_SET_SEC_LO                         0x02, 0x0108
#define LAN8841_PTP_LTC_SET_NS_HI                          0x02, 0x0109
#define LAN8841_PTP_LTC_SET_NS_LO                          0x02, 0x010A
#define LAN8841_PTP_LTC_SET_SUBNS_HI                       0x02, 0x010B
#define LAN8841_PTP_LTC_SET_SUBNS_LO                       0x02, 0x010C
#define LAN8841_PTP_LTC_RATE_ADJ_HI                        0x02, 0x010D
#define LAN8841_PTP_LTC_RATE_ADJ_LO                        0x02, 0x010E
#define LAN8841_PTP_LTC_TEMP_RATE_ADJ_HI                   0x02, 0x010F
#define LAN8841_PTP_LTC_TEMP_RATE_ADJ_LO                   0x02, 0x0110
#define LAN8841_PTP_LTC_TEMP_RATE_DURATION_HI              0x02, 0x0111
#define LAN8841_PTP_LTC_TEMP_RATE_DURATION_LO              0x02, 0x0112
#define LAN8841_PTP_LTC_STEP_ADJ_HI                        0x02, 0x0113
#define LAN8841_PTP_LTC_STEP_ADJ_LO                        0x02, 0x0114
#define LAN8841_PTP_LTC_EXT_ADJ_CFG                        0x02, 0x0115
#define LAN8841_PTP_LTC_TARGET_SEC_HI_A                    0x02, 0x0116
#define LAN8841_PTP_LTC_TARGET_SEC_LO_A                    0x02, 0x0117
#define LAN8841_PTP_LTC_TARGET_NS_HI_A                     0x02, 0x0118
#define LAN8841_PTP_LTC_TARGET_NS_LO_A                     0x02, 0x0119
#define LAN8841_PTP_LTC_TARGET_RELOAD_SEC_HI_A             0x02, 0x011A
#define LAN8841_PTP_LTC_TARGET_RELOAD_SEC_LO_A             0x02, 0x011B
#define LAN8841_PTP_LTC_TARGET_RELOAD_NS_HI_A              0x02, 0x011C
#define LAN8841_PTP_LTC_TARGET_RELOAD_NS_LO_A              0x02, 0x011D
#define LAN8841_PTP_LTC_TARGET_ACT_NS_HI_A                 0x02, 0x011E
#define LAN8841_PTP_LTC_TARGET_ACT_NS_LO_A                 0x02, 0x011F
#define LAN8841_PTP_LTC_TARGET_SEC_HI_B                    0x02, 0x0120
#define LAN8841_PTP_LTC_TARGET_SEC_LO_B                    0x02, 0x0121
#define LAN8841_PTP_LTC_TARGET_NS_HI_B                     0x02, 0x0122
#define LAN8841_PTP_LTC_TARGET_NS_LO_B                     0x02, 0x0123
#define LAN8841_PTP_LTC_TARGET_RELOAD_SEC_HI_B             0x02, 0x0124
#define LAN8841_PTP_LTC_TARGET_RELOAD_SEC_LO_B             0x02, 0x0125
#define LAN8841_PTP_LTC_TARGET_RELOAD_NS_HI_B              0x02, 0x0126
#define LAN8841_PTP_LTC_TARGET_RELOAD_NS_LO_B              0x02, 0x0127
#define LAN8841_PTP_LTC_TARGET_ACT_NS_HI_B                 0x02, 0x0128
#define LAN8841_PTP_LTC_TARGET_ACT_NS_LO_B                 0x02, 0x0129
#define LAN8841_PTP_RX_USER_MAC_HI                         0x02, 0x012A
#define LAN8841_PTP_RX_USER_MAC_MID                        0x02, 0x012B
#define LAN8841_PTP_RX_USER_MAC_LO                         0x02, 0x012C
#define LAN8841_PTP_RX_USER_IP_ADDR0                       0x02, 0x012D
#define LAN8841_PTP_RX_USER_IP_ADDR1                       0x02, 0x012E
#define LAN8841_PTP_RX_USER_IP_ADDR2                       0x02, 0x012F
#define LAN8841_PTP_RX_USER_IP_ADDR3                       0x02, 0x0130
#define LAN8841_PTP_RX_USER_IP_ADDR4                       0x02, 0x0131
#define LAN8841_PTP_RX_USER_IP_ADDR5                       0x02, 0x0132
#define LAN8841_PTP_RX_USER_IP_ADDR6                       0x02, 0x0133
#define LAN8841_PTP_RX_USER_IP_ADDR7                       0x02, 0x0134
#define LAN8841_PTP_RX_USER_IP_MASK0                       0x02, 0x0135
#define LAN8841_PTP_RX_USER_IP_MASK1                       0x02, 0x0136
#define LAN8841_PTP_RX_USER_IP_MASK2                       0x02, 0x0137
#define LAN8841_PTP_RX_USER_IP_MASK3                       0x02, 0x0138
#define LAN8841_PTP_RX_USER_IP_MASK4                       0x02, 0x0139
#define LAN8841_PTP_RX_USER_IP_MASK5                       0x02, 0x013A
#define LAN8841_PTP_RX_USER_IP_MASK6                       0x02, 0x013B
#define LAN8841_PTP_RX_USER_IP_MASK7                       0x02, 0x013C
#define LAN8841_VLAN_TYPE_ID                               0x02, 0x013D
#define LAN8841_VLAN1_TYPE_ID                              0x02, 0x013E
#define LAN8841_VLAN1_ID_MASK                              0x02, 0x013F
#define LAN8841_VLAN1_VID_RANGE_UP                         0x02, 0x0140
#define LAN8841_VLAN1_VID_RANGE_LO                         0x02, 0x0141
#define LAN8841_VLAN2_TYPE_ID                              0x02, 0x0142
#define LAN8841_VLAN2_ID_MASK                              0x02, 0x0143
#define LAN8841_VLAN2_VID_RANGE_UP                         0x02, 0x0144
#define LAN8841_VLAN2_VID_RANGE_LO                         0x02, 0x0145
#define LAN8841_LLC_TYPE_ID                                0x02, 0x0146
#define LAN8841_PTP_GPIO_SEL                               0x02, 0x0147
#define LAN8841_PTP_RX_LATENCY_10                          0x02, 0x0148
#define LAN8841_PTP_TX_LATENCY_10                          0x02, 0x0149
#define LAN8841_PTP_RX_LATENCY_100                         0x02, 0x014A
#define LAN8841_PTP_TX_LATENCY_100                         0x02, 0x014B
#define LAN8841_PTP_RX_LATENCY_1000                        0x02, 0x014C
#define LAN8841_PTP_TX_LATENCY_1000                        0x02, 0x014D
#define LAN8841_PTP_ASYM_DLY_HI                            0x02, 0x014E
#define LAN8841_PTP_ASYM_DLY_LO                            0x02, 0x014F
#define LAN8841_PTP_PEERDLY_HI                             0x02, 0x0150
#define LAN8841_PTP_PEERDLY_LO                             0x02, 0x0151
#define LAN8841_PTP_CAP_INFO                               0x02, 0x0152
#define LAN8841_PTP_TX_USER_MAC_HI                         0x02, 0x0153
#define LAN8841_PTP_TX_USER_MAC_MID                        0x02, 0x0154
#define LAN8841_PTP_TX_USER_MAC_LO                         0x02, 0x0155
#define LAN8841_PTP_TX_USER_IP_ADDR0                       0x02, 0x0156
#define LAN8841_PTP_TX_USER_IP_ADDR1                       0x02, 0x0157
#define LAN8841_PTP_TX_USER_IP_ADDR2                       0x02, 0x0158
#define LAN8841_PTP_TX_USER_IP_ADDR3                       0x02, 0x0159
#define LAN8841_PTP_TX_USER_IP_ADDR4                       0x02, 0x015A
#define LAN8841_PTP_TX_USER_IP_ADDR5                       0x02, 0x015B
#define LAN8841_PTP_TX_USER_IP_ADDR6                       0x02, 0x015C
#define LAN8841_PTP_TX_USER_IP_ADDR7                       0x02, 0x015D
#define LAN8841_PTP_TX_USER_IP_MASK0                       0x02, 0x015E
#define LAN8841_PTP_TX_USER_IP_MASK1                       0x02, 0x015F
#define LAN8841_PTP_TX_USER_IP_MASK2                       0x02, 0x0160
#define LAN8841_PTP_TX_USER_IP_MASK3                       0x02, 0x0161
#define LAN8841_PTP_TX_USER_IP_MASK4                       0x02, 0x0162
#define LAN8841_PTP_TX_USER_IP_MASK5                       0x02, 0x0163
#define LAN8841_PTP_TX_USER_IP_MASK6                       0x02, 0x0164
#define LAN8841_PTP_TX_USER_IP_MASK7                       0x02, 0x0165
#define LAN8841_PTP_LTC_RD_SEC_HI                          0x02, 0x0166
#define LAN8841_PTP_LTC_RD_SEC_MID                         0x02, 0x0167
#define LAN8841_PTP_LTC_RD_SEC_LO                          0x02, 0x0168
#define LAN8841_PTP_LTC_RD_NS_HI                           0x02, 0x0169
#define LAN8841_PTP_LTC_RD_NS_LO                           0x02, 0x016A
#define LAN8841_PTP_LTC_RD_SUBNS_HI                        0x02, 0x016B
#define LAN8841_PTP_LTC_RD_SUBNS_LO                        0x02, 0x016C
#define LAN8841_PTP_REV                                    0x02, 0x016D
#define LAN8841_PTP_SPARE                                  0x02, 0x016E
#define LAN8841_PTP_RX_PARSE_CONFIG                        0x02, 0x0170
#define LAN8841_PTP_RX_PARSE_VLAN_CONFIG                   0x02, 0x0171
#define LAN8841_PTP_RX_PARSE_L2_ADDR_EN                    0x02, 0x0172
#define LAN8841_PTP_RX_PARSE_IP_ADDR_EN                    0x02, 0x0173
#define LAN8841_PTP_RX_PARSE_UDP_SRC_PORT                  0x02, 0x0174
#define LAN8841_PTP_RX_PARSE_UDP_DEST_PORT                 0x02, 0x0175
#define LAN8841_PTP_RX_VERSION                             0x02, 0x0176
#define LAN8841_PTP_RX_DOMAIN_DOMAIN_LO                    0x02, 0x0177
#define LAN8841_PTP_RX_DOMAIN_MASK_DOMAIN_UP               0x02, 0x0178
#define LAN8841_PTP_RX_SDOID_SDOID_LO                      0x02, 0x0179
#define LAN8841_PTP_RX_SDOID_MASK_SDOID_UP                 0x02, 0x017A
#define LAN8841_PTP_RX_TIMESTAMP_EN                        0x02, 0x017B
#define LAN8841_PTP_RX_TIMESTAMP_CONFIG                    0x02, 0x017C
#define LAN8841_PTP_RX_MOD                                 0x02, 0x017D
#define LAN8841_PTP_RX_RSVD_BYTE_CFG                       0x02, 0x017E
#define LAN8841_PTP_RX_TAIL_TAG                            0x02, 0x017F
#define LAN8841_PTP_RX_CF_MOD_EN                           0x02, 0x0180
#define LAN8841_PTP_RX_CF_CFG                              0x02, 0x0181
#define LAN8841_PTP_RX_INGRESS_NS_HI                       0x02, 0x0182
#define LAN8841_PTP_RX_INGRESS_NS_LO                       0x02, 0x0183
#define LAN8841_PTP_RX_INGRESS_SEC_HI                      0x02, 0x0184
#define LAN8841_PTP_RX_INGRESS_SEC_LO                      0x02, 0x0185
#define LAN8841_PTP_RX_MSG_HEADER1                         0x02, 0x0186
#define LAN8841_PTP_RX_MSG_HEADER2                         0x02, 0x0187
#define LAN8841_PTP_RX_PDREQ_SEC_HI                        0x02, 0x0188
#define LAN8841_PTP_RX_PDREQ_SEC_MID                       0x02, 0x0189
#define LAN8841_PTP_RX_PDREQ_SEC_LOW                       0x02, 0x018A
#define LAN8841_PTP_RX_PDREQ_NS_HI                         0x02, 0x018B
#define LAN8841_PTP_RX_PDREQ_NS_LO                         0x02, 0x018C
#define LAN8841_PTP_RX_RAW_TS_SEC                          0x02, 0x018D
#define LAN8841_PTP_RX_RAW_TS_NS_HI                        0x02, 0x018E
#define LAN8841_PTP_RX_RAW_TS_NS_LO                        0x02, 0x018F
#define LAN8841_PTP_RX_CHKSUM_DROPPED_CNT_HI               0x02, 0x0190
#define LAN8841_PTP_RX_CHKSUM_DROPPED_CNT_LO               0x02, 0x0191
#define LAN8841_PTP_RX_FRMS_MOD_CNT_HI                     0x02, 0x0192
#define LAN8841_PTP_RX_FRMS_MOD_CNT_LO                     0x02, 0x0193
#define LAN8841_PTP_TX_PARSE_CONFIG                        0x02, 0x01B0
#define LAN8841_PTP_TX_PARSE_VLAN_CONFIG                   0x02, 0x01B1
#define LAN8841_PTP_TX_PARSE_L2_ADDR_EN                    0x02, 0x01B2
#define LAN8841_PTP_TX_PARSE_IP_ADDR_EN                    0x02, 0x01B3
#define LAN8841_PTP_TX_PARSE_UDP_SRC_PORT                  0x02, 0x01B4
#define LAN8841_PTP_TX_PARSE_UDP_DEST_PORT                 0x02, 0x01B5
#define LAN8841_PTP_TX_VERSION                             0x02, 0x01B6
#define LAN8841_PTP_TX_DOMAIN_DOMAIN_LO                    0x02, 0x01B7
#define LAN8841_PTP_TX_DOMAIN_MASK_DOMAIN_UP               0x02, 0x01B8
#define LAN8841_PTP_TX_SDOID_SDOID_LO                      0x02, 0x01B9
#define LAN8841_PTP_TX_SDOID_MASK_SDOID_UP                 0x02, 0x01BA
#define LAN8841_PTP_TX_TIMESTAMP_EN                        0x02, 0x01BB
#define LAN8841_PTP_TX_TIMESTAMP_CONFIG                    0x02, 0x01BC
#define LAN8841_PTP_TX_MOD                                 0x02, 0x01BD
#define LAN8841_PTP_TX_RSVD_BYTE_CFG                       0x02, 0x01BE
#define LAN8841_PTP_TX_TAIL_TAG                            0x02, 0x01BF
#define LAN8841_PTP_TX_CF_MOD_EN                           0x02, 0x01C0
#define LAN8841_PTP_TX_CF_CFG                              0x02, 0x01C1
#define LAN8841_PTP_TX_EGRESS_NS_HI                        0x02, 0x01C2
#define LAN8841_PTP_TX_EGRESS_NS_LO                        0x02, 0x01C3
#define LAN8841_PTP_TX_EGRESS_SEC_HI                       0x02, 0x01C4
#define LAN8841_PTP_TX_EGRESS_SEC_LO                       0x02, 0x01C5
#define LAN8841_PTP_TX_MSG_HEADER1                         0x02, 0x01C6
#define LAN8841_PTP_TX_MSG_HEADER2                         0x02, 0x01C7
#define LAN8841_PTP_TX_SYNC_SEC_HI                         0x02, 0x01C8
#define LAN8841_PTP_TX_SYNC_SEC_MID                        0x02, 0x01C9
#define LAN8841_PTP_TX_SYNC_SEC_LOW                        0x02, 0x01CA
#define LAN8841_PTP_TX_SYNC_NS_HI                          0x02, 0x01CB
#define LAN8841_PTP_TX_SYNC_NS_LO                          0x02, 0x01CC
#define LAN8841_PTP_TX_PDRESP_SEC_HI                       0x02, 0x01CD
#define LAN8841_PTP_TX_PDRESP_SEC_MID                      0x02, 0x01CE
#define LAN8841_PTP_TX_PDRESP_SEC_LOW                      0x02, 0x01CF
#define LAN8841_PTP_TX_PDRESP_NS_HI                        0x02, 0x01D0
#define LAN8841_PTP_TX_PDRESP_NS_LO                        0x02, 0x01D1
#define LAN8841_PTP_TX_RAW_TS_SEC                          0x02, 0x01D2
#define LAN8841_PTP_TX_RAW_TS_NS_HI                        0x02, 0x01D3
#define LAN8841_PTP_TX_RAW_TS_NS_LO                        0x02, 0x01D4
#define LAN8841_PTP_TX_CHKSUM_DROPPED_CNT_HI               0x02, 0x01D5
#define LAN8841_PTP_TX_CHKSUM_DROPPED_CNT_LO               0x02, 0x01D6
#define LAN8841_PTP_TX_FRMS_MOD_CNT_HI                     0x02, 0x01D7
#define LAN8841_PTP_TX_FRMS_MOD_CNT_LO                     0x02, 0x01D8
#define LAN8841_PTP_GPIO_CAP_EN                            0x02, 0x01F0
#define LAN8841_PTP_GPIO_CAP_LOCK                          0x02, 0x01F1
#define LAN8841_PTP_GPIO_RE_LTC_SEC_HI_CAP_X               0x02, 0x01F2
#define LAN8841_PTP_GPIO_RE_LTC_SEC_LO_CAP_X               0x02, 0x01F3
#define LAN8841_PTP_GPIO_RE_LTC_NS_HI_CAP_X                0x02, 0x01F4
#define LAN8841_PTP_GPIO_RE_LTC_NS_LO_CAP_X                0x02, 0x01F5
#define LAN8841_PTP_GPIO_FE_LTC_SEC_HI_CAP_X               0x02, 0x01F6
#define LAN8841_PTP_GPIO_FE_LTC_SEC_LO_CAP_X               0x02, 0x01F7
#define LAN8841_PTP_GPIO_FE_LTC_NS_HI_CAP_X                0x02, 0x01F8
#define LAN8841_PTP_GPIO_FE_LTC_NS_LO_CAP_X                0x02, 0x01F9
#define LAN8841_PTP_GPIO_CAP_STS                           0x02, 0x01FA
#define LAN8841_PTP_GPIO_INT_CLR_CFG                       0x02, 0x01FB
#define LAN8841_PTP_DEBUG_SEL                              0x02, 0x01FE
#define LAN8841_PCS_CTRL1                                  0x03, 0x0000
#define LAN8841_PCS_STAT1                                  0x03, 0x0001
#define LAN8841_EEE_QUIET_TIMER                            0x03, 0x0008
#define LAN8841_EEE_UPDATE_TIMER                           0x03, 0x0009
#define LAN8841_EEE_LINK_FAIL_TIMER                        0x03, 0x000A
#define LAN8841_EEE_POST_UPDATE_TIMER                      0x03, 0x000B
#define LAN8841_EEE_WAIT_WQ_TIMER                          0x03, 0x000C
#define LAN8841_EEE_WAKE_TIMER                             0x03, 0x000D
#define LAN8841_EEE_WAKE_TX_TIMER                          0x03, 0x000E
#define LAN8841_EEE_WAKE_MZ_TIMER                          0x03, 0x000F
#define LAN8841_EEE_CTRL_CAPABILITY                        0x03, 0x0014
#define LAN8841_EEE_WAKE_ERROR_COUNTER                     0x03, 0x0016
#define LAN8841_EEE_100_TIMER0                             0x03, 0x0018
#define LAN8841_EEE_100_TIMER1                             0x03, 0x0019
#define LAN8841_EEE_100_TIMER2                             0x03, 0x001A
#define LAN8841_EEE_100_TIMER3                             0x03, 0x001B
#define LAN8841_EEE_ADV                                    0x07, 0x003C
#define LAN8841_EEE_LP_ABILITY                             0x07, 0x003D
#define LAN8841_EEE_LP_ABILITY_OVERRIDE                    0x07, 0x003E
#define LAN8841_EEE_MSG_CODE                               0x07, 0x003F
#define LAN8841_XTAL_CTRL                                  0x1C, 0x0001
#define LAN8841_AFED_CTRL                                  0x1C, 0x0009
#define LAN8841_LDO_CTRL                                   0x1C, 0x000E
#define LAN8841_EDPD_CTRL                                  0x1C, 0x0024
#define LAN8841_EMITX_CTRL                                 0x1C, 0x0025
#define LAN8841_EMITX_COEFF                                0x1C, 0x0026

//Basic Control register
#define LAN8841_BMCR_RESET                                 0x8000
#define LAN8841_BMCR_LOOPBACK                              0x4000
#define LAN8841_BMCR_SPEED_SEL_LSB                         0x2000
#define LAN8841_BMCR_AN_EN                                 0x1000
#define LAN8841_BMCR_POWER_DOWN                            0x0800
#define LAN8841_BMCR_ISOLATE                               0x0400
#define LAN8841_BMCR_RESTART_AN                            0x0200
#define LAN8841_BMCR_DUPLEX_MODE                           0x0100
#define LAN8841_BMCR_COL_TEST                              0x0080
#define LAN8841_BMCR_SPEED_SEL_MSB                         0x0040

//Basic Status register
#define LAN8841_BMSR_100BT4                                0x8000
#define LAN8841_BMSR_100BTX_FD                             0x4000
#define LAN8841_BMSR_100BTX_HD                             0x2000
#define LAN8841_BMSR_10BT_FD                               0x1000
#define LAN8841_BMSR_10BT_HD                               0x0800
#define LAN8841_BMSR_100BT2_FD                             0x0400
#define LAN8841_BMSR_100BT2_HD                             0x0200
#define LAN8841_BMSR_EXTENDED_STATUS                       0x0100
#define LAN8841_BMSR_UNIDIRECTIONAL_ABLE                   0x0080
#define LAN8841_BMSR_MF_PREAMBLE_SUPPR                     0x0040
#define LAN8841_BMSR_AN_COMPLETE                           0x0020
#define LAN8841_BMSR_REMOTE_FAULT                          0x0010
#define LAN8841_BMSR_AN_CAPABLE                            0x0008
#define LAN8841_BMSR_LINK_STATUS                           0x0004
#define LAN8841_BMSR_JABBER_DETECT                         0x0002
#define LAN8841_BMSR_EXTENDED_CAPABLE                      0x0001

//Device Identifier 1 register
#define LAN8841_PHYID1_PHY_ID_MSB                          0xFFFF
#define LAN8841_PHYID1_PHY_ID_MSB_DEFAULT                  0x0022

//Device Identifier 2 register
#define LAN8841_PHYID2_PHY_ID_LSB                          0xFC00
#define LAN8841_PHYID2_PHY_ID_LSB_DEFAULT                  0x1400
#define LAN8841_PHYID2_MODEL_NUM                           0x03F0
#define LAN8841_PHYID2_MODEL_NUM_DEFAULT                   0x0250
#define LAN8841_PHYID2_REVISION_NUM                        0x000F

//Auto-Negotiation Advertisement register
#define LAN8841_ANAR_NEXT_PAGE                             0x8000
#define LAN8841_ANAR_REMOTE_FAULT                          0x2000
#define LAN8841_ANAR_EXTENDED_NEXT_PAGE                    0x1000
#define LAN8841_ANAR_ASYM_PAUSE                            0x0800
#define LAN8841_ANAR_SYM_PAUSE                             0x0400
#define LAN8841_ANAR_100BT4                                0x0200
#define LAN8841_ANAR_100BTX_FD                             0x0100
#define LAN8841_ANAR_100BTX_HD                             0x0080
#define LAN8841_ANAR_10BT_FD                               0x0040
#define LAN8841_ANAR_10BT_HD                               0x0020
#define LAN8841_ANAR_SELECTOR                              0x001F
#define LAN8841_ANAR_SELECTOR_DEFAULT                      0x0001

//Auto-Negotiation Link Partner Base Page Ability register
#define LAN8841_ANLPAR_NEXT_PAGE                           0x8000
#define LAN8841_ANLPAR_ACK                                 0x4000
#define LAN8841_ANLPAR_REMOTE_FAULT                        0x2000
#define LAN8841_ANLPAR_EXTENDED_NEXT_PAGE                  0x1000
#define LAN8841_ANLPAR_ASYM_PAUSE                          0x0800
#define LAN8841_ANLPAR_PAUSE                               0x0400
#define LAN8841_ANLPAR_100BT4                              0x0200
#define LAN8841_ANLPAR_100BTX_FD                           0x0100
#define LAN8841_ANLPAR_100BTX_HD                           0x0080
#define LAN8841_ANLPAR_10BT_FD                             0x0040
#define LAN8841_ANLPAR_10BT_HD                             0x0020
#define LAN8841_ANLPAR_SELECTOR                            0x001F
#define LAN8841_ANLPAR_SELECTOR_DEFAULT                    0x0001

//Auto-Negotiation Expansion register
#define LAN8841_ANER_RECEIVE_NP_LOC_ABLE                   0x0040
#define LAN8841_ANER_RECEIVE_NP_STOR_LOC                   0x0020
#define LAN8841_ANER_PAR_DETECT_FAULT                      0x0010
#define LAN8841_ANER_LP_NEXT_PAGE_ABLE                     0x0008
#define LAN8841_ANER_NEXT_PAGE_ABLE                        0x0004
#define LAN8841_ANER_PAGE_RECEIVED                         0x0002
#define LAN8841_ANER_LP_AN_ABLE                            0x0001

//Auto-Negotiation Next Page TX register
#define LAN8841_ANNPTR_NEXT_PAGE                           0x8000
#define LAN8841_ANNPTR_MSG_PAGE                            0x2000
#define LAN8841_ANNPTR_ACK2                                0x1000
#define LAN8841_ANNPTR_TOGGLE                              0x0800
#define LAN8841_ANNPTR_MESSAGE                             0x07FF

//Auto-Negotiation Next Page RX register
#define LAN8841_ANNPRR_NEXT_PAGE                           0x8000
#define LAN8841_ANNPRR_ACK                                 0x4000
#define LAN8841_ANNPRR_MSG_PAGE                            0x2000
#define LAN8841_ANNPRR_ACK2                                0x1000
#define LAN8841_ANNPRR_TOGGLE                              0x0800
#define LAN8841_ANNPRR_MESSAGE                             0x07FF

//Auto-Negotiation Master Slave Control register
#define LAN8841_GBCR_TEST_MODE                             0xE000
#define LAN8841_GBCR_MS_MAN_CONF_EN                        0x1000
#define LAN8841_GBCR_MS_MAN_CONF_VAL                       0x0800
#define LAN8841_GBCR_PORT_TYPE                             0x0400
#define LAN8841_GBCR_1000BT_FD                             0x0200
#define LAN8841_GBCR_1000BT_HD                             0x0100

//Auto-Negotiation Master Slave Status register
#define LAN8841_GBSR_MS_CONF_FAULT                         0x8000
#define LAN8841_GBSR_MS_CONF_RES                           0x4000
#define LAN8841_GBSR_LOCAL_RECEIVER_STATUS                 0x2000
#define LAN8841_GBSR_REMOTE_RECEIVER_STATUS                0x1000
#define LAN8841_GBSR_LP_1000BT_FD                          0x0800
#define LAN8841_GBSR_LP_1000BT_HD                          0x0400
#define LAN8841_GBSR_IDLE_ERR_COUNT                        0x00FF

//MMD Access Control register
#define LAN8841_MMDACR_FUNC                                0xC000
#define LAN8841_MMDACR_FUNC_ADDR                           0x0000
#define LAN8841_MMDACR_FUNC_DATA_NO_POST_INC               0x4000
#define LAN8841_MMDACR_FUNC_DATA_POST_INC_RW               0x8000
#define LAN8841_MMDACR_FUNC_DATA_POST_INC_W                0xC000
#define LAN8841_MMDACR_DEVAD                               0x001F

//Extended Status register
#define LAN8841_GBESR_1000BX_FD                            0x8000
#define LAN8841_GBESR_1000BX_HD                            0x4000
#define LAN8841_GBESR_1000BT_FD                            0x2000
#define LAN8841_GBESR_1000BT_HD                            0x1000

//Remote Loopback register
#define LAN8841_RLB_REMOTE_LOOPBACK                        0x0100

//LinkMD Cable Diagnostic register
#define LAN8841_LINKMD_TEST_EN                             0x8000
#define LAN8841_LINKMD_TX_DIS                              0x4000
#define LAN8841_LINKMD_PAIR                                0x3000
#define LAN8841_LINKMD_PAIR_A                              0x0000
#define LAN8841_LINKMD_PAIR_B                              0x1000
#define LAN8841_LINKMD_PAIR_C                              0x2000
#define LAN8841_LINKMD_PAIR_D                              0x3000
#define LAN8841_LINKMD_SEL                                 0x0C00
#define LAN8841_LINKMD_STATUS                              0x0300
#define LAN8841_LINKMD_STATUS_NORMAL                       0x0000
#define LAN8841_LINKMD_STATUS_OPEN                         0x0100
#define LAN8841_LINKMD_STATUS_SHORT                        0x0200
#define LAN8841_LINKMD_STATUS_TEST_FAILED                  0x0300
#define LAN8841_LINKMD_FAULT_DATA                          0x00FF

//Digital PMA/PCS Status register
#define LAN8841_DPMAPCSS_1000BT_LINK_STATUS                0x0002
#define LAN8841_DPMAPCSS_100BTX_LINK_STATUS                0x0001

//LED Mode Select register
#define LAN8841_LED_MODE_SEL_LED4_CONFIG                   0xF000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_LINK_ACT          0x0000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_LINK_1000_ACT     0x1000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_LINK_100_ACT      0x2000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_LINK_10_ACT       0x3000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_LINK_100_1000_ACT 0x4000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_LINK_10_1000_ACT  0x5000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_LINK_10_100_ACT   0x6000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_DUPLEX_COL        0x8000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_COL               0x9000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_ACT               0xA000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_AN_FAULT          0xC000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_OFF               0xE000
#define LAN8841_LED_MODE_SEL_LED4_CONFIG_ON                0xF000
#define LAN8841_LED_MODE_SEL_LED3_CONFIG                   0x0F00
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_LINK_ACT          0x0000
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_LINK_1000_ACT     0x0100
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_LINK_100_ACT      0x0200
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_LINK_10_ACT       0x0300
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_LINK_100_1000_ACT 0x0400
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_LINK_10_1000_ACT  0x0500
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_LINK_10_100_ACT   0x0600
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_DUPLEX_COL        0x0800
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_COL               0x0900
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_ACT               0x0A00
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_AN_FAULT          0x0C00
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_OFF               0x0E00
#define LAN8841_LED_MODE_SEL_LED3_CONFIG_ON                0x0F00
#define LAN8841_LED_MODE_SEL_LED2_CONFIG                   0x00F0
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_LINK_ACT          0x0000
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_LINK_1000_ACT     0x0010
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_LINK_100_ACT      0x0020
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_LINK_10_ACT       0x0030
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_LINK_100_1000_ACT 0x0040
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_LINK_10_1000_ACT  0x0050
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_LINK_10_100_ACT   0x0060
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_DUPLEX_COL        0x0080
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_COL               0x0090
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_ACT               0x00A0
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_AN_FAULT          0x00C0
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_OFF               0x00E0
#define LAN8841_LED_MODE_SEL_LED2_CONFIG_ON                0x00F0
#define LAN8841_LED_MODE_SEL_LED1_CONFIG                   0x000F
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_LINK_ACT          0x0000
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_LINK_1000_ACT     0x0001
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_LINK_100_ACT      0x0002
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_LINK_10_ACT       0x0003
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_LINK_100_1000_ACT 0x0004
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_LINK_10_1000_ACT  0x0005
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_LINK_10_100_ACT   0x0006
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_DUPLEX_COL        0x0008
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_COL               0x0009
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_ACT               0x000A
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_AN_FAULT          0x000C
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_OFF               0x000E
#define LAN8841_LED_MODE_SEL_LED1_CONFIG_ON                0x000F

//LED Behavior register
#define LAN8841_LED_BEHAVIOR_LED_ACT_OUT_SEL               0x4000
#define LAN8841_LED_BEHAVIOR_LED_PULSING_EN                0x1000
#define LAN8841_LED_BEHAVIOR_LED_BLINK_RATE                0x0C00
#define LAN8841_LED_BEHAVIOR_LED_BLINK_RATE_2_5HZ          0x0000
#define LAN8841_LED_BEHAVIOR_LED_BLINK_RATE_5HZ            0x0400
#define LAN8841_LED_BEHAVIOR_LED_BLINK_RATE_10HZ           0x0800
#define LAN8841_LED_BEHAVIOR_LED_BLINK_RATE_20HZ           0x0C00
#define LAN8841_LED_BEHAVIOR_LED_PULSE_STRECH_EN           0x01E0
#define LAN8841_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED4      0x0100
#define LAN8841_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED3      0x0080
#define LAN8841_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED2      0x0040
#define LAN8841_LED_BEHAVIOR_LED_PULSE_STRECH_EN_LED1      0x0020
#define LAN8841_LED_BEHAVIOR_LED_COMBINATION_DIS           0x000F
#define LAN8841_LED_BEHAVIOR_LED_COMBINATION_DIS_LED4      0x0008
#define LAN8841_LED_BEHAVIOR_LED_COMBINATION_DIS_LED3      0x0004
#define LAN8841_LED_BEHAVIOR_LED_COMBINATION_DIS_LED2      0x0002
#define LAN8841_LED_BEHAVIOR_LED_COMBINATION_DIS_LED1      0x0001

//Output Control register
#define LAN8841_OUT_CTRL_MDIO_BUFFER_TYPE                  0x8000
#define LAN8841_OUT_CTRL_INT_BUFFER_TYPE                   0x4000
#define LAN8841_OUT_CTRL_LED_BUFFER_TYPE                   0x3F00
#define LAN8841_OUT_CTRL_LED_BUFFER_TYPE_LED4              0x0800
#define LAN8841_OUT_CTRL_LED_BUFFER_TYPE_LED3              0x0400
#define LAN8841_OUT_CTRL_LED_BUFFER_TYPE_LED2              0x0200
#define LAN8841_OUT_CTRL_LED_BUFFER_TYPE_LED1              0x0100
#define LAN8841_OUT_CTRL_PME_POLARITY                      0x0080
#define LAN8841_OUT_CTRL_LED_POLARITY                      0x003F
#define LAN8841_OUT_CTRL_LED_POLARITY_LED4                 0x0008
#define LAN8841_OUT_CTRL_LED_POLARITY_LED3                 0x0004
#define LAN8841_OUT_CTRL_LED_POLARITY_LED2                 0x0002
#define LAN8841_OUT_CTRL_LED_POLARITY_LED1                 0x0001

//KSZ9031 LED Mode register
#define LAN8841_LEGACY_LED_MODE_KSZ9031_LED_MODE           0x4000

//Interrupt Status register
#define LAN8841_ISR_ENERGY_NOT_DETECT                      0x0800
#define LAN8841_ISR_ENERGY_DETECT                          0x0400
#define LAN8841_ISR_1588                                   0x0200
#define LAN8841_ISR_GPIO                                   0x0100
#define LAN8841_ISR_JABBER                                 0x0080
#define LAN8841_ISR_RECEIVE_ERROR                          0x0040
#define LAN8841_ISR_PAGE_RECEIVED                          0x0020
#define LAN8841_ISR_PAR_DETECT_FAULT                       0x0010
#define LAN8841_ISR_LP_ACK                                 0x0008
#define LAN8841_ISR_LINK_DOWN                              0x0004
#define LAN8841_ISR_ADC_FIFO_ERROR                         0x0002
#define LAN8841_ISR_LINK_UP                                0x0001

//Auto-MDI/MDI-X register
#define LAN8841_AUTOMDI_MDI_SET                            0x0080
#define LAN8841_AUTOMDI_SWAP_OFF                           0x0040

//Software Power Down Control register
#define LAN8841_SPDC_CLK_GATE_OVERRIDE                     0x0800
#define LAN8841_SPDC_PLL_DIS                               0x0400
#define LAN8841_SPDC_IO_DC_TEST_EN                         0x0080
#define LAN8841_SPDC_VOH                                   0x0040

//External Loopback register
#define LAN8841_EXT_LOOPBACK_EXT_LPBK                      0x0008

//Control register
#define LAN8841_PHYCON_INT_POL_INVERT                      0x4000
#define LAN8841_PHYCON_JABBER_EN                           0x0200
#define LAN8841_PHYCON_SQE_TEST_EN                         0x0100
#define LAN8841_PHYCON_SPEED_1000BT                        0x0040
#define LAN8841_PHYCON_SPEED_100BTX                        0x0020
#define LAN8841_PHYCON_SPEED_10BT                          0x0010
#define LAN8841_PHYCON_DUPLEX_STATUS                       0x0008
#define LAN8841_PHYCON_1000BT_MS_STATUS                    0x0004
#define LAN8841_PHYCON_SOFT_RESET                          0x0002
#define LAN8841_PHYCON_LINK_STATUS_CHECK_FAIL              0x0001

//RX DLL Control register
#define LAN8841_RX_DLL_CTRL_RXDLL_TUNE_DIS                 0x8000
#define LAN8841_RX_DLL_CTRL_BYPASS_RXDLL                   0x4000
#define LAN8841_RX_DLL_CTRL_RXDLL_TAP_SEL                  0x3F80
#define LAN8841_RX_DLL_CTRL_RXDLL_TAP_SEL_DEFAULT          0x0D80
#define LAN8841_RX_DLL_CTRL_RXDLL_TAP_ADJ                  0x007F
#define LAN8841_RX_DLL_CTRL_RXDLL_TAP_ADJ_DEFAULT          0x001B

//TX DLL Control register
#define LAN8841_TX_DLL_CTRL_TXDLL_TUNE_DIS                 0x8000
#define LAN8841_TX_DLL_CTRL_BYPASS_TXDLL                   0x4000
#define LAN8841_TX_DLL_CTRL_TXDLL_TAP_SEL                  0x3F80
#define LAN8841_TX_DLL_CTRL_TXDLL_TAP_SEL_DEFAULT          0x0D80
#define LAN8841_TX_DLL_CTRL_TXDLL_TAP_ADJ                  0x007F
#define LAN8841_TX_DLL_CTRL_TXDLL_TAP_ADJ_DEFAULT          0x001B

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//LAN8841 Ethernet PHY driver
extern const PhyDriver lan8841PhyDriver;

//LAN8841 related functions
error_t lan8841Init(NetInterface *interface);
void lan8841InitHook(NetInterface *interface);

void lan8841Tick(NetInterface *interface);

void lan8841EnableIrq(NetInterface *interface);
void lan8841DisableIrq(NetInterface *interface);

void lan8841EventHandler(NetInterface *interface);

void lan8841WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t lan8841ReadPhyReg(NetInterface *interface, uint8_t address);

void lan8841DumpPhyReg(NetInterface *interface);

void lan8841WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t lan8841ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
