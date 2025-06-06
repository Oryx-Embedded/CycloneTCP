/**
 * @file m467_eth_driver.h
 * @brief Nuvoton M467 Ethernet MAC driver
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

#ifndef _M467_ETH_DRIVER_H
#define _M467_ETH_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef M467_ETH_TX_BUFFER_COUNT
   #define M467_ETH_TX_BUFFER_COUNT 3
#elif (M467_ETH_TX_BUFFER_COUNT < 1)
   #error M467_ETH_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef M467_ETH_TX_BUFFER_SIZE
   #define M467_ETH_TX_BUFFER_SIZE 1536
#elif (M467_ETH_TX_BUFFER_SIZE != 1536)
   #error M467_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef M467_ETH_RX_BUFFER_COUNT
   #define M467_ETH_RX_BUFFER_COUNT 6
#elif (M467_ETH_RX_BUFFER_COUNT < 1)
   #error M467_ETH_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef M467_ETH_RX_BUFFER_SIZE
   #define M467_ETH_RX_BUFFER_SIZE 1536
#elif (M467_ETH_RX_BUFFER_SIZE != 1536)
   #error M467_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Interrupt priority grouping
#ifndef M467_ETH_IRQ_PRIORITY_GROUPING
   #define M467_ETH_IRQ_PRIORITY_GROUPING 3
#elif (M467_ETH_IRQ_PRIORITY_GROUPING < 0)
   #error M467_ETH_IRQ_PRIORITY_GROUPING parameter is not valid
#endif

//Ethernet interrupt group priority
#ifndef M467_ETH_IRQ_GROUP_PRIORITY
   #define M467_ETH_IRQ_GROUP_PRIORITY 12
#elif (M467_ETH_IRQ_GROUP_PRIORITY < 0)
   #error M467_ETH_IRQ_GROUP_PRIORITY parameter is not valid
#endif

//Ethernet interrupt subpriority
#ifndef M467_ETH_IRQ_SUB_PRIORITY
   #define M467_ETH_IRQ_SUB_PRIORITY 0
#elif (M467_ETH_IRQ_SUB_PRIORITY < 0)
   #error M467_ETH_IRQ_SUB_PRIORITY parameter is not valid
#endif

//EMAC registers
#define EMAC_MAC_CONFIG                                     *((volatile uint32_t *) (EMAC_BASE + 0x0000))
#define EMAC_MAC_FRAME_FILTER                               *((volatile uint32_t *) (EMAC_BASE + 0x0004))
#define EMAC_GMII_ADDR                                      *((volatile uint32_t *) (EMAC_BASE + 0x0010))
#define EMAC_GMII_DATA                                      *((volatile uint32_t *) (EMAC_BASE + 0x0014))
#define EMAC_FLOW_CONTROL                                   *((volatile uint32_t *) (EMAC_BASE + 0x0018))
#define EMAC_VLAN_TAG                                       *((volatile uint32_t *) (EMAC_BASE + 0x001C))
#define EMAC_VERSION                                        *((volatile uint32_t *) (EMAC_BASE + 0x0020))
#define EMAC_DEBUG                                          *((volatile uint32_t *) (EMAC_BASE + 0x0024))
#define EMAC_PMT_CONTROL_STATUS                             *((volatile uint32_t *) (EMAC_BASE + 0x002C))
#define EMAC_INTERRUPT_STATUS                               *((volatile uint32_t *) (EMAC_BASE + 0x0038))
#define EMAC_INTERRUPT_MASK                                 *((volatile uint32_t *) (EMAC_BASE + 0x003C))
#define EMAC_MAC_ADDR0_HIGH                                 *((volatile uint32_t *) (EMAC_BASE + 0x0040))
#define EMAC_MAC_ADDR0_LOW                                  *((volatile uint32_t *) (EMAC_BASE + 0x0044))
#define EMAC_MAC_ADDR1_HIGH                                 *((volatile uint32_t *) (EMAC_BASE + 0x0048))
#define EMAC_MAC_ADDR1_LOW                                  *((volatile uint32_t *) (EMAC_BASE + 0x004C))
#define EMAC_MAC_ADDR2_HIGH                                 *((volatile uint32_t *) (EMAC_BASE + 0x0050))
#define EMAC_MAC_ADDR2_LOW                                  *((volatile uint32_t *) (EMAC_BASE + 0x0054))
#define EMAC_MAC_ADDR3_HIGH                                 *((volatile uint32_t *) (EMAC_BASE + 0x0058))
#define EMAC_MAC_ADDR3_LOW                                  *((volatile uint32_t *) (EMAC_BASE + 0x005C))
#define EMAC_MAC_ADDR4_HIGH                                 *((volatile uint32_t *) (EMAC_BASE + 0x0060))
#define EMAC_MAC_ADDR4_LOW                                  *((volatile uint32_t *) (EMAC_BASE + 0x0064))
#define EMAC_MAC_ADDR5_HIGH                                 *((volatile uint32_t *) (EMAC_BASE + 0x0068))
#define EMAC_MAC_ADDR5_LOW                                  *((volatile uint32_t *) (EMAC_BASE + 0x006C))
#define EMAC_MAC_ADDR6_HIGH                                 *((volatile uint32_t *) (EMAC_BASE + 0x0070))
#define EMAC_MAC_ADDR6_LOW                                  *((volatile uint32_t *) (EMAC_BASE + 0x0074))
#define EMAC_MAC_ADDR7_HIGH                                 *((volatile uint32_t *) (EMAC_BASE + 0x0078))
#define EMAC_MAC_ADDR7_LOW                                  *((volatile uint32_t *) (EMAC_BASE + 0x007C))
#define EMAC_MAC_ADDR8_HIGH                                 *((volatile uint32_t *) (EMAC_BASE + 0x0080))
#define EMAC_MAC_ADDR8_LOW                                  *((volatile uint32_t *) (EMAC_BASE + 0x0084))
#define EMAC_WDOG_TIMEOUT                                   *((volatile uint32_t *) (EMAC_BASE + 0x00DC))
#define EMAC_VLAN_INCL_REG                                  *((volatile uint32_t *) (EMAC_BASE + 0x0584))
#define EMAC_TIMESTAMP_CONTROL                              *((volatile uint32_t *) (EMAC_BASE + 0x0700))
#define EMAC_SUB_SECOND_INCREMENT                           *((volatile uint32_t *) (EMAC_BASE + 0x0704))
#define EMAC_SYSTEM_TIME_SECONDS                            *((volatile uint32_t *) (EMAC_BASE + 0x0708))
#define EMAC_SYSTEM_TIME_NANOSECONDS                        *((volatile uint32_t *) (EMAC_BASE + 0x070C))
#define EMAC_SYSTEM_TIME_SECONDS_UPDATE                     *((volatile uint32_t *) (EMAC_BASE + 0x0710))
#define EMAC_SYSTEM_TIME_NANOSECONDS_UPDATE                 *((volatile uint32_t *) (EMAC_BASE + 0x0714))
#define EMAC_TIMESTAMP_ADDEND                               *((volatile uint32_t *) (EMAC_BASE + 0x0718))
#define EMAC_TARGET_TIME_SECONDS                            *((volatile uint32_t *) (EMAC_BASE + 0x071C))
#define EMAC_TARGET_TIME_NANOSECONDS                        *((volatile uint32_t *) (EMAC_BASE + 0x0720))
#define EMAC_SYSTEM_TIME_HIGHER_WORD_SECONDS                *((volatile uint32_t *) (EMAC_BASE + 0x0724))
#define EMAC_TIMESTAMP_STATUS                               *((volatile uint32_t *) (EMAC_BASE + 0x0728))
#define EMAC_PPS_CONTROL                                    *((volatile uint32_t *) (EMAC_BASE + 0x072C))
#define EMAC_PPS0_INTERVAL                                  *((volatile uint32_t *) (EMAC_BASE + 0x0760))
#define EMAC_PPS0_WIDTH                                     *((volatile uint32_t *) (EMAC_BASE + 0x0764))
#define EMAC_BUS_MODE                                       *((volatile uint32_t *) (EMAC_BASE + 0x1000))
#define EMAC_TRANSMIT_POLL_DEMAND                           *((volatile uint32_t *) (EMAC_BASE + 0x1004))
#define EMAC_RECEIVE_POLL_DEMAND                            *((volatile uint32_t *) (EMAC_BASE + 0x1008))
#define EMAC_RECEIVE_DESCRIPTOR_LIST_ADDR                   *((volatile uint32_t *) (EMAC_BASE + 0x100C))
#define EMAC_TRANSMIT_DESCRIPTOR_LIST_ADDR                  *((volatile uint32_t *) (EMAC_BASE + 0x1010))
#define EMAC_STATUS                                         *((volatile uint32_t *) (EMAC_BASE + 0x1014))
#define EMAC_OPERATION_MODE                                 *((volatile uint32_t *) (EMAC_BASE + 0x1018))
#define EMAC_INTERRUPT_ENABLE                               *((volatile uint32_t *) (EMAC_BASE + 0x101C))
#define EMAC_MISSED_FRAME_AND_BUFFER_OVERFLOW_CNT           *((volatile uint32_t *) (EMAC_BASE + 0x1020))
#define EMAC_RECEIVE_INTERRUPT_WATCHDOG_TIMER               *((volatile uint32_t *) (EMAC_BASE + 0x1024))
#define EMAC_AHB_STATUS                                     *((volatile uint32_t *) (EMAC_BASE + 0x102C))
#define EMAC_CURRENT_HOST_TRANSMIT_DESCRIPTOR               *((volatile uint32_t *) (EMAC_BASE + 0x1048))
#define EMAC_CURRENT_HOST_RECEIVE_DESCRIPTOR                *((volatile uint32_t *) (EMAC_BASE + 0x104C))
#define EMAC_CURRENT_HOST_TRANSMIT_BUFFER_ADDR              *((volatile uint32_t *) (EMAC_BASE + 0x1050))
#define EMAC_CURRENT_HOST_RECEIVE_BUFFER_ADDR               *((volatile uint32_t *) (EMAC_BASE + 0x1054))
#define EMAC_HW_FEATURE                                     *((volatile uint32_t *) (EMAC_BASE + 0x1058))

//MAC Configuration register
#define EMAC_MAC_CONFIG_SARC                                0xF0000000
#define EMAC_MAC_CONFIG_TWOKPE                              0x08000000
#define EMAC_MAC_CONFIG_CST                                 0x02000000
#define EMAC_MAC_CONFIG_WD                                  0x00800000
#define EMAC_MAC_CONFIG_JD                                  0x00400000
#define EMAC_MAC_CONFIG_JE                                  0x00100000
#define EMAC_MAC_CONFIG_IFG                                 0x000E0000
#define EMAC_MAC_CONFIG_DCRS                                0x00010000
#define EMAC_MAC_CONFIG_FES                                 0x00004000
#define EMAC_MAC_CONFIG_DO                                  0x00002000
#define EMAC_MAC_CONFIG_LM                                  0x00001000
#define EMAC_MAC_CONFIG_DM                                  0x00000800
#define EMAC_MAC_CONFIG_IPC                                 0x00000400
#define EMAC_MAC_CONFIG_DR                                  0x00000200
#define EMAC_MAC_CONFIG_ACS                                 0x00000080
#define EMAC_MAC_CONFIG_BL                                  0x00000060
#define EMAC_MAC_CONFIG_DC                                  0x00000010
#define EMAC_MAC_CONFIG_TE                                  0x00000008
#define EMAC_MAC_CONFIG_RE                                  0x00000004
#define EMAC_MAC_CONFIG_PRELEN                              0x00000003

//MAC Frame Filter register
#define EMAC_MAC_FRAME_FILTER_RA                            0x80000000
#define EMAC_MAC_FRAME_FILTER_VTFE                          0x00010000
#define EMAC_MAC_FRAME_FILTER_SAF                           0x00000200
#define EMAC_MAC_FRAME_FILTER_SAIF                          0x00000100
#define EMAC_MAC_FRAME_FILTER_PCF                           0x000000C0
#define EMAC_MAC_FRAME_FILTER_DBF                           0x00000020
#define EMAC_MAC_FRAME_FILTER_PM                            0x00000010
#define EMAC_MAC_FRAME_FILTER_DAIF                          0x00000008
#define EMAC_MAC_FRAME_FILTER_PR                            0x00000001

//GMII Address register
#define EMAC_GMII_ADDR_PA                                   0x0000F800
#define EMAC_GMII_ADDR_GR                                   0x000007C0
#define EMAC_GMII_ADDR_CR                                   0x0000003C
#define EMAC_GMII_ADDR_CR_DIV_42                            0x00000000
#define EMAC_GMII_ADDR_CR_DIV_62                            0x00000004
#define EMAC_GMII_ADDR_CR_DIV_16                            0x00000008
#define EMAC_GMII_ADDR_CR_DIV_26                            0x0000000C
#define EMAC_GMII_ADDR_CR_DIV_102                           0x00000010
#define EMAC_GMII_ADDR_CR_DIV_124                           0x00000014
#define EMAC_GMII_ADDR_GW                                   0x00000002
#define EMAC_GMII_ADDR_GB                                   0x00000001

//GMII Data register
#define EMAC_GMII_DATA_GD                                   0x0000FFFF

//Flow Control register
#define EMAC_FLOW_CONTROL_PT                                0xFFFF0000
#define EMAC_FLOW_CONTROL_DZQP                              0x00000080
#define EMAC_FLOW_CONTROL_PLT                               0x00000030
#define EMAC_FLOW_CONTROL_UP                                0x00000008
#define EMAC_FLOW_CONTROL_RFE                               0x00000004
#define EMAC_FLOW_CONTROL_TFE                               0x00000002
#define EMAC_FLOW_CONTROL_FCA_BPA                           0x00000001

//VLAN Tag register
#define EMAC_VLAN_TAG_ESVL                                  0x00040000
#define EMAC_VLAN_TAG_VTIM                                  0x00020000
#define EMAC_VLAN_TAG_ETV                                   0x00010000
#define EMAC_VLAN_TAG_VL                                    0x0000FFFF

//Debug register
#define EMAC_DEBUG_TXSTSFSTS                                0x02000000
#define EMAC_DEBUG_TXFSTS                                   0x01000000
#define EMAC_DEBUG_TWCSTS                                   0x00400000
#define EMAC_DEBUG_TRCSTS                                   0x00300000
#define EMAC_DEBUG_TXPAUSED                                 0x00080000
#define EMAC_DEBUG_TFCSTS                                   0x00060000
#define EMAC_DEBUG_TPESTS                                   0x00010000
#define EMAC_DEBUG_RXFSTS                                   0x00000300
#define EMAC_DEBUG_RRCSTS                                   0x00000060
#define EMAC_DEBUG_RWCSTS                                   0x00000010
#define EMAC_DEBUG_RFCFCSTS                                 0x00000006
#define EMAC_DEBUG_RPESTS                                   0x00000001

//PMT Control and Status register
#define EMAC_PMT_CONTROL_STATUS_MGKPRCVD                    0x00000020
#define EMAC_PMT_CONTROL_STATUS_MGKPKTEN                    0x00000002
#define EMAC_PMT_CONTROL_STATUS_PWRDWN                      0x00000001

//Interrupt register
#define EMAC_INTERRUPT_STATUS_TSIS                          0x00000200
#define EMAC_INTERRUPT_STATUS_PMTIS                         0x00000008

//Interrupt Mask register
#define EMAC_INTERRUPT_MASK_TSIM                            0x00000200
#define EMAC_INTERRUPT_MASK_PMTIM                           0x00000008

//MAC Address0 High register
#define EMAC_MAC_ADDR0_HIGH_AE                              0x80000000
#define EMAC_MAC_ADDR0_HIGH_ADDRHI                          0x0000FFFF

//MAC Address0 Low register
#define EMAC_MAC_ADDR0_LOW_ADDRLO                           0xFFFFFFFF

//MAC Address1 High register
#define EMAC_MAC_ADDR1_HIGH_AE                              0x80000000
#define EMAC_MAC_ADDR1_HIGH_SA                              0x40000000
#define EMAC_MAC_ADDR1_HIGH_MBC                             0x3F000000
#define EMAC_MAC_ADDR1_HIGH_ADDRHI                          0x0000FFFF

//MAC Address1 Low register
#define EMAC_MAC_ADDR1_LOW_ADDRLO                           0xFFFFFFFF

//MAC Address2 High register
#define EMAC_MAC_ADDR2_HIGH_AE                              0x80000000
#define EMAC_MAC_ADDR2_HIGH_SA                              0x40000000
#define EMAC_MAC_ADDR2_HIGH_MBC                             0x3F000000
#define EMAC_MAC_ADDR2_HIGH_ADDRHI                          0x0000FFFF

//MAC Address2 Low register
#define EMAC_MAC_ADDR2_LOW_ADDRLO                           0xFFFFFFFF

//MAC Address3 High register
#define EMAC_MAC_ADDR3_HIGH_AE                              0x80000000
#define EMAC_MAC_ADDR3_HIGH_SA                              0x40000000
#define EMAC_MAC_ADDR3_HIGH_MBC                             0x3F000000
#define EMAC_MAC_ADDR3_HIGH_ADDRHI                          0x0000FFFF

//MAC Address3 Low register
#define EMAC_MAC_ADDR3_LOW_ADDRLO                           0xFFFFFFFF

//MAC Address4 High register
#define EMAC_MAC_ADDR4_HIGH_AE                              0x80000000
#define EMAC_MAC_ADDR4_HIGH_SA                              0x40000000
#define EMAC_MAC_ADDR4_HIGH_MBC                             0x3F000000
#define EMAC_MAC_ADDR4_HIGH_ADDRHI                          0x0000FFFF

//MAC Address4 Low register
#define EMAC_MAC_ADDR4_LOW_ADDRLO                           0xFFFFFFFF

//MAC Address5 High register
#define EMAC_MAC_ADDR5_HIGH_AE                              0x80000000
#define EMAC_MAC_ADDR5_HIGH_SA                              0x40000000
#define EMAC_MAC_ADDR5_HIGH_MBC                             0x3F000000
#define EMAC_MAC_ADDR5_HIGH_ADDRHI                          0x0000FFFF

//MAC Address5 Low register
#define EMAC_MAC_ADDR5_LOW_ADDRLO                           0xFFFFFFFF

//MAC Address6 High register
#define EMAC_MAC_ADDR6_HIGH_AE                              0x80000000
#define EMAC_MAC_ADDR6_HIGH_SA                              0x40000000
#define EMAC_MAC_ADDR6_HIGH_MBC                             0x3F000000
#define EMAC_MAC_ADDR6_HIGH_ADDRHI                          0x0000FFFF

//MAC Address6 Low register
#define EMAC_MAC_ADDR6_LOW_ADDRLO                           0xFFFFFFFF

//MAC Address7 High register
#define EMAC_MAC_ADDR7_HIGH_AE                              0x80000000
#define EMAC_MAC_ADDR7_HIGH_SA                              0x40000000
#define EMAC_MAC_ADDR7_HIGH_MBC                             0x3F000000
#define EMAC_MAC_ADDR7_HIGH_ADDRHI                          0x0000FFFF

//MAC Address7 Low register
#define EMAC_MAC_ADDR7_LOW_ADDRLO                           0xFFFFFFFF

//MAC Address8 High register
#define EMAC_MAC_ADDR8_HIGH_AE                              0x80000000
#define EMAC_MAC_ADDR8_HIGH_SA                              0x40000000
#define EMAC_MAC_ADDR8_HIGH_MBC                             0x3F000000
#define EMAC_MAC_ADDR8_HIGH_ADDRHI                          0x0000FFFF

//MAC Address8 Low register
#define EMAC_MAC_ADDR8_LOW_ADDRLO                           0xFFFFFFFF

//Watchdog Timeout register
#define EMAC_WDOG_TIMEOUT_PWE                               0x00010000
#define EMAC_WDOG_TIMEOUT_WTO                               0x00003FFF

//VLAN Tag Inclusion or Replacement register
#define EMAC_VLAN_INCL_REG_CSVL                             0x00080000
#define EMAC_VLAN_INCL_REG_VLP                              0x00040000
#define EMAC_VLAN_INCL_REG_VLC                              0x00030000
#define EMAC_VLAN_INCL_REG_VLT                              0x0000FFFF

//Timestamp Control register
#define EMAC_TIMESTAMP_CONTROL_TSENMACADDR                  0x00040000
#define EMAC_TIMESTAMP_CONTROL_SNAPTYPSEL                   0x00030000
#define EMAC_TIMESTAMP_CONTROL_TSMSTRENA                    0x00008000
#define EMAC_TIMESTAMP_CONTROL_TSEVNTENA                    0x00004000
#define EMAC_TIMESTAMP_CONTROL_TSIPV4ENA                    0x00002000
#define EMAC_TIMESTAMP_CONTROL_TSIPV6ENA                    0x00001000
#define EMAC_TIMESTAMP_CONTROL_TSIPENA                      0x00000800
#define EMAC_TIMESTAMP_CONTROL_TSVER2ENA                    0x00000400
#define EMAC_TIMESTAMP_CONTROL_TSCTRLSSR                    0x00000200
#define EMAC_TIMESTAMP_CONTROL_TSENALL                      0x00000100
#define EMAC_TIMESTAMP_CONTROL_TSADDREG                     0x00000020
#define EMAC_TIMESTAMP_CONTROL_TSTRIG                       0x00000010
#define EMAC_TIMESTAMP_CONTROL_TSUPDT                       0x00000008
#define EMAC_TIMESTAMP_CONTROL_TSINIT                       0x00000004
#define EMAC_TIMESTAMP_CONTROL_TSCFUPDT                     0x00000002
#define EMAC_TIMESTAMP_CONTROL_TSENA                        0x00000001

//Sub-Second Increment register
#define EMAC_SUB_SECOND_INCREMENT_SSINC                     0x000000FF

//System Time Seconds register
#define EMAC_SYSTEM_TIME_SECONDS_TSS                        0xFFFFFFFF

//System Time Nanoseconds register
#define EMAC_SYSTEM_TIME_NANOSECONDS_TSSS                   0x7FFFFFFF

//System Time Seconds Update register
#define EMAC_SYSTEM_TIME_SECONDS_UPDATE_TSS                 0xFFFFFFFF

//System Time Nanoseconds Update register
#define EMAC_SYSTEM_TIME_NANOSECONDS_UPDATE_ADDSUB          0x80000000
#define EMAC_SYSTEM_TIME_NANOSECONDS_UPDATE_TSSS            0x7FFFFFFF

//Timestamp Addend register
#define EMAC_TIMESTAMP_ADDEND_TSAR                          0xFFFFFFFF

//Target Time Seconds register
#define EMAC_TARGET_TIME_SECONDS_TSTR                       0xFFFFFFFF

//Target Time Nanoseconds register
#define EMAC_TARGET_TIME_NANOSECONDS_TTSLO                  0xFFFFFFFF

//System Time Higher Word Seconds register
#define EMAC_SYSTEM_TIME_HIGHER_WORD_SECONDS_TSHWR          0x0000FFFF

//Timestamp Status register
#define EMAC_TIMESTAMP_STATUS_ATSSTN                        0x000F0000
#define EMAC_TIMESTAMP_STATUS_TSTRGTERR                     0x00000008
#define EMAC_TIMESTAMP_STATUS_TSTARGT                       0x00000002
#define EMAC_TIMESTAMP_STATUS_TSSOVF                        0x00000001

//PPS Control register
#define EMAC_PPS_CONTROL_TRGTMODSEL0                        0x00000060
#define EMAC_PPS_CONTROL_PPSEN0                             0x00000010
#define EMAC_PPS_CONTROL_PPSCTRL_PPSCMD                     0x0000000F

//PPS0 Interval register
#define EMAC_PPS0_INTERVAL_PPSINT                           0xFFFFFFFF

//PPS0 Width register
#define EMAC_PPS0_WIDTH_PPSWIDTH                            0xFFFFFFFF

//Bus Mode register
#define EMAC_BUS_MODE_AAB                                   0x02000000
#define EMAC_BUS_MODE_PBLX8                                 0x01000000
#define EMAC_BUS_MODE_USP                                   0x00800000
#define EMAC_BUS_MODE_RPBL                                  0x007E0000
#define EMAC_BUS_MODE_RPBL_1                                0x00020000
#define EMAC_BUS_MODE_RPBL_2                                0x00040000
#define EMAC_BUS_MODE_RPBL_4                                0x00080000
#define EMAC_BUS_MODE_RPBL_8                                0x00100000
#define EMAC_BUS_MODE_RPBL_16                               0x00200000
#define EMAC_BUS_MODE_RPBL_32                               0x00400000
#define EMAC_BUS_MODE_FB                                    0x00010000
#define EMAC_BUS_MODE_PBL                                   0x00003F00
#define EMAC_BUS_MODE_PBL_1                                 0x00000100
#define EMAC_BUS_MODE_PBL_2                                 0x00000200
#define EMAC_BUS_MODE_PBL_4                                 0x00000400
#define EMAC_BUS_MODE_PBL_8                                 0x00000800
#define EMAC_BUS_MODE_PBL_16                                0x00001000
#define EMAC_BUS_MODE_PBL_32                                0x00002000
#define EMAC_BUS_MODE_ATDS                                  0x00000080
#define EMAC_BUS_MODE_DSL                                   0x0000007C
#define EMAC_BUS_MODE_DSL_0                                 0x00000000
#define EMAC_BUS_MODE_DSL_1                                 0x00000004
#define EMAC_BUS_MODE_DSL_2                                 0x00000008
#define EMAC_BUS_MODE_DSL_4                                 0x00000010
#define EMAC_BUS_MODE_DSL_8                                 0x00000020
#define EMAC_BUS_MODE_DSL_16                                0x00000040
#define EMAC_BUS_MODE_SWR                                   0x00000001

//Transmit Poll Demand register
#define EMAC_TRANSMIT_POLL_DEMAND_TPD                       0xFFFFFFFF

//Receive Poll Demand register
#define EMAC_RECEIVE_POLL_DEMAND_RPD                        0xFFFFFFFF

//Receive Descriptor List Address register
#define EMAC_RECEIVE_DESCRIPTOR_LIST_ADDR_RDESLA            0xFFFFFFFF

//Transmit Descriptor List Address register
#define EMAC_TRANSMIT_DESCRIPTOR_LIST_ADDR_TDESLA           0xFFFFFFFF

//Status register
#define EMAC_STATUS_TTI                                     0x20000000
#define EMAC_STATUS_GPI                                     0x10000000
#define EMAC_STATUS_EB                                      0x03800000
#define EMAC_STATUS_TS                                      0x00700000
#define EMAC_STATUS_RS                                      0x000E0000
#define EMAC_STATUS_NIS                                     0x00010000
#define EMAC_STATUS_AIS                                     0x00008000
#define EMAC_STATUS_ERI                                     0x00004000
#define EMAC_STATUS_FBI                                     0x00002000
#define EMAC_STATUS_ETI                                     0x00000400
#define EMAC_STATUS_RWT                                     0x00000200
#define EMAC_STATUS_RPS                                     0x00000100
#define EMAC_STATUS_RU                                      0x00000080
#define EMAC_STATUS_RI                                      0x00000040
#define EMAC_STATUS_UNF                                     0x00000020
#define EMAC_STATUS_OVF                                     0x00000010
#define EMAC_STATUS_TJT                                     0x00000008
#define EMAC_STATUS_TU                                      0x00000004
#define EMAC_STATUS_TPS                                     0x00000002
#define EMAC_STATUS_TI                                      0x00000001

//Operation Mode register
#define EMAC_OPERATION_MODE_DT                              0x04000000
#define EMAC_OPERATION_MODE_RSF                             0x02000000
#define EMAC_OPERATION_MODE_DFF                             0x01000000
#define EMAC_OPERATION_MODE_TSF                             0x00200000
#define EMAC_OPERATION_MODE_FTF                             0x00100000
#define EMAC_OPERATION_MODE_TTC                             0x0001C000
#define EMAC_OPERATION_MODE_ST                              0x00002000
#define EMAC_OPERATION_MODE_RFD                             0x00001800
#define EMAC_OPERATION_MODE_RFA                             0x00000600
#define EMAC_OPERATION_MODE_EFC                             0x00000100
#define EMAC_OPERATION_MODE_FEF                             0x00000080
#define EMAC_OPERATION_MODE_FUF                             0x00000040
#define EMAC_OPERATION_MODE_DGF                             0x00000020
#define EMAC_OPERATION_MODE_RTC                             0x00000018
#define EMAC_OPERATION_MODE_OSF                             0x00000004
#define EMAC_OPERATION_MODE_SR                              0x00000002

//Interrupt Enable register
#define EMAC_INTERRUPT_ENABLE_NIE                           0x00010000
#define EMAC_INTERRUPT_ENABLE_AIE                           0x00008000
#define EMAC_INTERRUPT_ENABLE_ERE                           0x00004000
#define EMAC_INTERRUPT_ENABLE_FBE                           0x00002000
#define EMAC_INTERRUPT_ENABLE_ETE                           0x00000400
#define EMAC_INTERRUPT_ENABLE_RWE                           0x00000200
#define EMAC_INTERRUPT_ENABLE_RSE                           0x00000100
#define EMAC_INTERRUPT_ENABLE_RUE                           0x00000080
#define EMAC_INTERRUPT_ENABLE_RIE                           0x00000040
#define EMAC_INTERRUPT_ENABLE_UNE                           0x00000020
#define EMAC_INTERRUPT_ENABLE_OVE                           0x00000010
#define EMAC_INTERRUPT_ENABLE_TJE                           0x00000008
#define EMAC_INTERRUPT_ENABLE_TUE                           0x00000004
#define EMAC_INTERRUPT_ENABLE_TSE                           0x00000002
#define EMAC_INTERRUPT_ENABLE_TIE                           0x00000001

//Missed Frame and Buffer Overflow Counter register
#define EMAC_MISSED_FRAME_AND_BUFFER_OVERFLOW_CNT_OVFCNTOVF 0x10000000
#define EMAC_MISSED_FRAME_AND_BUFFER_OVERFLOW_CNT_OVFFRMCNT 0x0FFE0000
#define EMAC_MISSED_FRAME_AND_BUFFER_OVERFLOW_CNT_MISCNTOVF 0x00010000
#define EMAC_MISSED_FRAME_AND_BUFFER_OVERFLOW_CNT_MISFRMCNT 0x0000FFFF

//Receive Interrupt Watchdog Timer register
#define EMAC_RECEIVE_INTERRUPT_WATCHDOG_TIMER_RIWT          0x000000FF

//AHB Status register
#define EMAC_AHB_STATUS_AXIRDSTS                            0x00000002
#define EMAC_AHB_STATUS_AXWHSTS                             0x00000001

//Current Host Transmit Descriptor register
#define EMAC_CURRENT_HOST_TRANSMIT_DESCRIPTOR_CURTDESAPTR   0xFFFFFFFF

//Current Host Receive Descriptor register
#define EMAC_CURRENT_HOST_RECEIVE_DESCRIPTOR_CURRDESAPTR    0xFFFFFFFF

//Current Host Transmit Buffer Address register
#define EMAC_CURRENT_HOST_TRANSMIT_BUFFER_ADDR_CURTBUFAPTR  0xFFFFFFFF

//Current Host Receive Buffer Address register
#define EMAC_CURRENT_HOST_RECEIVE_BUFFER_ADDR_CURRBUFAPTR   0xFFFFFFFF

//HW Feature register
#define EMAC_HW_FEATURE_ACTPHYIF                            0x70000000
#define EMAC_HW_FEATURE_SAVLANINS                           0x08000000
#define EMAC_HW_FEATURE_FLEXIPPSEN                          0x04000000
#define EMAC_HW_FEATURE_INTTSEN                             0x02000000
#define EMAC_HW_FEATURE_ENHDESSEL                           0x01000000
#define EMAC_HW_FEATURE_TXCHCNT                             0x00C00000
#define EMAC_HW_FEATURE_RXCHCNT                             0x00300000
#define EMAC_HW_FEATURE_RXFIFOSIZE                          0x00080000
#define EMAC_HW_FEATURE_RXTYP2COE                           0x00040000
#define EMAC_HW_FEATURE_RXTYP1COE                           0x00020000
#define EMAC_HW_FEATURE_TXCOESEL                            0x00010000
#define EMAC_HW_FEATURE_EEESEL                              0x00004000
#define EMAC_HW_FEATURE_TSVER2SEL                           0x00002000
#define EMAC_HW_FEATURE_TSVER1SEL                           0x00001000
#define EMAC_HW_FEATURE_MMCSEL                              0x00000800
#define EMAC_HW_FEATURE_MGKSEL                              0x00000400
#define EMAC_HW_FEATURE_RWKSEL                              0x00000200
#define EMAC_HW_FEATURE_SMASEL                              0x00000100
#define EMAC_HW_FEATURE_L3L4FLTREN                          0x00000080
#define EMAC_HW_FEATURE_PCSSEL                              0x00000040
#define EMAC_HW_FEATURE_ADDMACADRSEL                        0x00000020
#define EMAC_HW_FEATURE_HASHSEL                             0x00000010
#define EMAC_HW_FEATURE_EXTHASHEN                           0x00000008
#define EMAC_HW_FEATURE_HDSEL                               0x00000004
#define EMAC_HW_FEATURE_MIISEL                              0x00000001

//Transmit DMA descriptor flags
#define EMAC_TDES0_OWN                                      0x80000000
#define EMAC_TDES0_IC                                       0x40000000
#define EMAC_TDES0_LS                                       0x20000000
#define EMAC_TDES0_FS                                       0x10000000
#define EMAC_TDES0_DC                                       0x08000000
#define EMAC_TDES0_DP                                       0x04000000
#define EMAC_TDES0_TTSE                                     0x02000000
#define EMAC_TDES0_CRCR                                     0x01000000
#define EMAC_TDES0_CIC                                      0x00C00000
#define EMAC_TDES0_TER                                      0x00200000
#define EMAC_TDES0_TCH                                      0x00100000
#define EMAC_TDES0_VLIC                                     0x000C0000
#define EMAC_TDES0_TTSS                                     0x00020000
#define EMAC_TDES0_IHE                                      0x00010000
#define EMAC_TDES0_ES                                       0x00008000
#define EMAC_TDES0_JT                                       0x00004000
#define EMAC_TDES0_FF                                       0x00002000
#define EMAC_TDES0_IPE                                      0x00001000
#define EMAC_TDES0_LOC                                      0x00000800
#define EMAC_TDES0_NC                                       0x00000400
#define EMAC_TDES0_LC                                       0x00000200
#define EMAC_TDES0_EC                                       0x00000100
#define EMAC_TDES0_VF                                       0x00000080
#define EMAC_TDES0_CC                                       0x00000078
#define EMAC_TDES0_ED                                       0x00000004
#define EMAC_TDES0_UF                                       0x00000002
#define EMAC_TDES0_DB                                       0x00000001
#define EMAC_TDES1_SAIC                                     0xE0000000
#define EMAC_TDES1_TBS2                                     0x1FFF0000
#define EMAC_TDES1_TBS1                                     0x00001FFF
#define EMAC_TDES2_TBAP1                                    0xFFFFFFFF
#define EMAC_TDES3_TBAP2                                    0xFFFFFFFF
#define EMAC_TDES6_TTSL                                     0xFFFFFFFF
#define EMAC_TDES7_TTSH                                     0xFFFFFFFF

//Receive DMA descriptor flags
#define EMAC_RDES0_OWN                                      0x80000000
#define EMAC_RDES0_AFM                                      0x40000000
#define EMAC_RDES0_FL                                       0x3FFF0000
#define EMAC_RDES0_ES                                       0x00008000
#define EMAC_RDES0_DE                                       0x00004000
#define EMAC_RDES0_SAF                                      0x00002000
#define EMAC_RDES0_LE                                       0x00001000
#define EMAC_RDES0_OE                                       0x00000800
#define EMAC_RDES0_VLAN                                     0x00000400
#define EMAC_RDES0_FS                                       0x00000200
#define EMAC_RDES0_LS                                       0x00000100
#define EMAC_RDES0_IPHCE_TSV                                0x00000080
#define EMAC_RDES0_LC                                       0x00000040
#define EMAC_RDES0_FT                                       0x00000020
#define EMAC_RDES0_RWT                                      0x00000010
#define EMAC_RDES0_RE                                       0x00000008
#define EMAC_RDES0_DBE                                      0x00000004
#define EMAC_RDES0_CE                                       0x00000002
#define EMAC_RDES0_PCE_ESA                                  0x00000001
#define EMAC_RDES1_DIC                                      0x80000000
#define EMAC_RDES1_RBS2                                     0x1FFF0000
#define EMAC_RDES1_RER                                      0x00008000
#define EMAC_RDES1_RCH                                      0x00004000
#define EMAC_RDES1_RBS1                                     0x00001FFF
#define EMAC_RDES2_RBAP1                                    0xFFFFFFFF
#define EMAC_RDES3_RBAP2                                    0xFFFFFFFF
#define EMAC_RDES4_L3L4FNM                                  0x0C000000
#define EMAC_RDES4_L4FM                                     0x02000000
#define EMAC_RDES4_L3FM                                     0x01000000
#define EMAC_RDES4_TSD                                      0x00004000
#define EMAC_RDES4_PV                                       0x00002000
#define EMAC_RDES4_PFT                                      0x00001000
#define EMAC_RDES4_PMT                                      0x00000F00
#define EMAC_RDES4_IPV6PR                                   0x00000080
#define EMAC_RDES4_IPV4PR                                   0x00000040
#define EMAC_RDES4_IPCB                                     0x00000020
#define EMAC_RDES4_IPPE                                     0x00000010
#define EMAC_RDES4_IPHE                                     0x00000008
#define EMAC_RDES4_IPPT                                     0x00000007
#define EMAC_RDES6_RTSL                                     0xFFFFFFFF
#define EMAC_RDES7_RTSH                                     0xFFFFFFFF

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Enhanced TX DMA descriptor
 **/

typedef struct
{
   uint32_t tdes0;
   uint32_t tdes1;
   uint32_t tdes2;
   uint32_t tdes3;
   uint32_t tdes4;
   uint32_t tdes5;
   uint32_t tdes6;
   uint32_t tdes7;
} M467TxDmaDesc;


/**
 * @brief Enhanced RX DMA descriptor
 **/

typedef struct
{
   uint32_t rdes0;
   uint32_t rdes1;
   uint32_t rdes2;
   uint32_t rdes3;
   uint32_t rdes4;
   uint32_t rdes5;
   uint32_t rdes6;
   uint32_t rdes7;
} M467RxDmaDesc;


//M467 Ethernet MAC driver
extern const NicDriver m467EthDriver;

//M467 Ethernet MAC related functions
error_t m467EthInit(NetInterface *interface);
void m467EthInitGpio(NetInterface *interface);
void m467EthInitDmaDesc(NetInterface *interface);

void m467EthTick(NetInterface *interface);

void m467EthEnableIrq(NetInterface *interface);
void m467EthDisableIrq(NetInterface *interface);
void m467EthEventHandler(NetInterface *interface);

error_t m467EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t m467EthReceivePacket(NetInterface *interface);

error_t m467EthUpdateMacAddrFilter(NetInterface *interface);
error_t m467EthUpdateMacConfig(NetInterface *interface);

void m467EthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t m467EthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
