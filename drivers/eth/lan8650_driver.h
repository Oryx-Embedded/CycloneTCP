/**
 * @file lan8650_driver.h
 * @brief LAN8650 10Base-T1S Ethernet controller
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

#ifndef _LAN8650_DRIVER_H
#define _LAN8650_DRIVER_H

//Dependencies
#include "core/nic.h"

//PLCA support
#ifndef LAN8650_PLCA_SUPPORT
   #define LAN8650_PLCA_SUPPORT ENABLED
#elif (LAN8650_PLCA_SUPPORT != ENABLED && LAN8650_PLCA_SUPPORT != DISABLED)
   #error LAN8650_PLCA_SUPPORT parameter is not valid
#endif

//Node count
#ifndef LAN8650_NODE_COUNT
   #define LAN8650_NODE_COUNT 8
#elif (LAN8650_NODE_COUNT < 0 || LAN8650_NODE_COUNT > 255)
   #error LAN8650_NODE_COUNT parameter is not valid
#endif

//Local ID
#ifndef LAN8650_LOCAL_ID
   #define LAN8650_LOCAL_ID 1
#elif (LAN8650_LOCAL_ID < 0 || LAN8650_LOCAL_ID > 255)
   #error LAN8650_LOCAL_ID parameter is not valid
#endif

//RX buffer size
#ifndef LAN8650_ETH_RX_BUFFER_SIZE
   #define LAN8650_ETH_RX_BUFFER_SIZE 1536
#elif (LAN8650_ETH_RX_BUFFER_SIZE != 1536)
   #error LAN8650_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Chunk payload size
#define LAN8650_CHUNK_PAYLOAD_SIZE 64

//Transmit data header
#define LAN8650_TX_HEADER_DNC    0x80000000
#define LAN8650_TX_HEADER_SEQ    0x40000000
#define LAN8650_TX_HEADER_NORX   0x20000000
#define LAN8650_TX_HEADER_VS     0x00C00000
#define LAN8650_TX_HEADER_DV     0x00200000
#define LAN8650_TX_HEADER_SV     0x00100000
#define LAN8650_TX_HEADER_SWO    0x000F0000
#define LAN8650_TX_HEADER_EV     0x00004000
#define LAN8650_TX_HEADER_EBO    0x00003F00
#define LAN8650_TX_HEADER_TSC    0x000000C0
#define LAN8650_TX_HEADER_P      0x00000001

//Receive data footer
#define LAN8650_RX_FOOTER_EXST   0x80000000
#define LAN8650_RX_FOOTER_HDRB   0x40000000
#define LAN8650_RX_FOOTER_SYNC   0x20000000
#define LAN8650_RX_FOOTER_RCA    0x1F000000
#define LAN8650_RX_FOOTER_VS     0x00C00000
#define LAN8650_RX_FOOTER_DV     0x00200000
#define LAN8650_RX_FOOTER_SV     0x00100000
#define LAN8650_RX_FOOTER_SWO    0x000F0000
#define LAN8650_RX_FOOTER_FD     0x00008000
#define LAN8650_RX_FOOTER_EV     0x00004000
#define LAN8650_RX_FOOTER_EBO    0x00003F00
#define LAN8650_RX_FOOTER_RTSA   0x00000080
#define LAN8650_RX_FOOTER_RTSP   0x00000040
#define LAN8650_RX_FOOTER_TXC    0x0000003E
#define LAN8650_RX_FOOTER_P      0x00000001

//Control command header
#define LAN8650_CTRL_HEADER_DNC  0x80000000
#define LAN8650_CTRL_HEADER_HDRB 0x40000000
#define LAN8650_CTRL_HEADER_WNR  0x20000000
#define LAN8650_CTRL_HEADER_AID  0x10000000
#define LAN8650_CTRL_HEADER_MMS  0x0F000000
#define LAN8650_CTRL_HEADER_ADDR 0x00FFFF00
#define LAN8650_CTRL_HEADER_LEN  0x000000FE
#define LAN8650_CTRL_HEADER_P    0x00000001

//Memory map selectors
#define LAN8650_MMS_STD          0x00
#define LAN8650_MMS_MAC          0x01
#define LAN8650_MMS_PHY_PCS      0x02
#define LAN8650_MMS_PHY_PMA_PMD  0x03
#define LAN8650_MMS_PHY_VS       0x04
#define LAN8650_MMS_PHY_MISC     0x0A

//LAN8650 Open Alliance Standard registers (MMS 0)
#define LAN8650_OA_ID                           0x00, 0x0000
#define LAN8650_OA_PHYID                        0x00, 0x0001
#define LAN8650_OA_STDCAP                       0x00, 0x0002
#define LAN8650_OA_RESET                        0x00, 0x0003
#define LAN8650_OA_CONFIG0                      0x00, 0x0004
#define LAN8650_OA_STATUS0                      0x00, 0x0008
#define LAN8650_OA_STATUS1                      0x00, 0x0009
#define LAN8650_OA_BUFSTS                       0x00, 0x000B
#define LAN8650_OA_IMASK0                       0x00, 0x000C
#define LAN8650_OA_MASK1                        0x00, 0x000D
#define LAN8650_TTSCAH                          0x00, 0x0010
#define LAN8650_TTSCAL                          0x00, 0x0011
#define LAN8650_TTSCBH                          0x00, 0x0012
#define LAN8650_TTSCBL                          0x00, 0x0013
#define LAN8650_TTSCCH                          0x00, 0x0014
#define LAN8650_TTSCCL                          0x00, 0x0015
#define LAN8650_BASIC_CONTROL                   0x00, 0xFF00
#define LAN8650_BASIC_STATUS                    0x00, 0xFF01
#define LAN8650_PHY_ID1                         0x00, 0xFF02
#define LAN8650_PHY_ID2                         0x00, 0xFF03
#define LAN8650_MMDCTRL                         0x00, 0xFF0D
#define LAN8650_MMDAD                           0x00, 0xFF0E

//LAN8650 MAC registers (MMS 1)
#define LAN8650_MAC_NCR                         0x01, 0x0000
#define LAN8650_MAC_NCFGR                       0x01, 0x0001
#define LAN8650_MAC_HRB                         0x01, 0x0020
#define LAN8650_MAC_HRT                         0x01, 0x0021
#define LAN8650_MAC_SAB1                        0x01, 0x0022
#define LAN8650_MAC_SAT1                        0x01, 0x0023
#define LAN8650_MAC_SAB2                        0x01, 0x0024
#define LAN8650_MAC_SAT2                        0x01, 0x0025
#define LAN8650_MAC_SAB3                        0x01, 0x0026
#define LAN8650_MAC_SAT3                        0x01, 0x0027
#define LAN8650_MAC_SAB4                        0x01, 0x0028
#define LAN8650_MAC_SAT4                        0x01, 0x0029
#define LAN8650_MAC_TIDM1                       0x01, 0x002A
#define LAN8650_MAC_TIDM2                       0x01, 0x002B
#define LAN8650_MAC_TIDM3                       0x01, 0x002C
#define LAN8650_MAC_TIDM4                       0x01, 0x002D
#define LAN8650_MAC_SAMB1                       0x01, 0x0032
#define LAN8650_MAC_SAMT1                       0x01, 0x0033
#define LAN8650_MAC_TISUBN                      0x01, 0x006F
#define LAN8650_MAC_TSH                         0x01, 0x0070
#define LAN8650_MAC_TSL                         0x01, 0x0074
#define LAN8650_MAC_TN                          0x01, 0x0075
#define LAN8650_MAC_TA                          0x01, 0x0076
#define LAN8650_MAC_TI                          0x01, 0x0077
#define LAN8650_BMGR_CTL                        0x01, 0x0280
#define LAN8650_STATS0                          0x01, 0x0288
#define LAN8650_STATS1                          0x01, 0x0289
#define LAN8650_STATS2                          0x01, 0x028A
#define LAN8650_STATS3                          0x01, 0x028B
#define LAN8650_STATS4                          0x01, 0x028C
#define LAN8650_STATS5                          0x01, 0x028D
#define LAN8650_STATS6                          0x01, 0x028E
#define LAN8650_STATS7                          0x01, 0x028F
#define LAN8650_STATS8                          0x01, 0x0290
#define LAN8650_STATS9                          0x01, 0x0291
#define LAN8650_STATS10                         0x01, 0x0292
#define LAN8650_STATS11                         0x01, 0x0293
#define LAN8650_STATS12                         0x01, 0x0294

//LAN8650 PHY PCS registers (MMS 2)
#define LAN8650_T1SPCSCTL                       0x02, 0x08F3
#define LAN8650_T1SPCSSTS                       0x02, 0x08F4
#define LAN8650_T1SPCSDIAG1                     0x02, 0x08F5
#define LAN8650_T1SPCSDIAG2                     0x02, 0x08F6

//LAN8650 PHY PMA/PMD registers (MMS 3)
#define LAN8650_T1PMAPMDEXTA                    0x03, 0x0012
#define LAN8650_T1PMAPMDCTL                     0x03, 0x0834
#define LAN8650_T1SPMACTL                       0x03, 0x08F9
#define LAN8650_T1SPMASTS                       0x03, 0x08FA
#define LAN8650_T1STSTCTL                       0x03, 0x08FB

//LAN8650 Vendor Specific registers (MMS 4)
#define LAN8650_CTRL1                           0x04, 0x0010
#define LAN8650_STS1                            0x04, 0x0018
#define LAN8650_STS2                            0x04, 0x0019
#define LAN8650_STS3                            0x04, 0x001A
#define LAN8650_IMSK1                           0x04, 0x001C
#define LAN8650_IMSK2                           0x04, 0x001D
#define LAN8650_CTRCTRL                         0x04, 0x0020
#define LAN8650_TOCNTH                          0x04, 0x0024
#define LAN8650_TOCNTL                          0x04, 0x0025
#define LAN8650_BCNCNTH                         0x04, 0x0026
#define LAN8650_BCNCNTL                         0x04, 0x0027
#define LAN8650_PRTMGMT2                        0x04, 0x003D
#define LAN8650_IWDTOH                          0x04, 0x003E
#define LAN8650_IWDTOL                          0x04, 0x003F
#define LAN8650_SLPCTL0                         0x04, 0x0080
#define LAN8650_SLPCTL1                         0x04, 0x0081
#define LAN8650_ANALOG5                         0x04, 0x00D5
#define LAN8650_MIDVER                          0x04, 0xCA00
#define LAN8650_PLCA_CTRL0                      0x04, 0xCA01
#define LAN8650_PLCA_CTRL1                      0x04, 0xCA02
#define LAN8650_PLCA_STS                        0x04, 0xCA03
#define LAN8650_PLCA_TOTMR                      0x04, 0xCA04
#define LAN8650_PLCA_BURST                      0x04, 0xCA05

//LAN8650 Miscellaneous registers (MMS 10)
#define LAN8650_QTXCFG                          0x0A, 0x81
#define LAN8650_QRXCFG                          0x0A, 0x82
#define LAN8650_PADCTRL                         0x0A, 0x88
#define LAN8650_MISC                            0x0A, 0x8C
#define LAN8650_DEVID                           0x0A, 0x94

//OA_ID register
#define LAN8650_OA_ID_MAJVER                    0x000000F0
#define LAN8650_OA_ID_MAJVER_DEFAULT            0x00000010
#define LAN8650_OA_ID_MINVER                    0x0000000F
#define LAN8650_OA_ID_MINVER_DEFAULT            0x00000001

//OA_PHYID register
#define LAN8650_OA_PHYID_OUI                    0xFFFFFC00
#define LAN8650_OA_PHYID_OUI_DEFAULT            0x02003C00
#define LAN8650_OA_PHYID_MODEL                  0x000003F0
#define LAN8650_OA_PHYID_MODEL_DEFAULT          0x000001B0
#define LAN8650_OA_PHYID_REV                    0x0000000F
#define LAN8650_OA_PHYID_REV_DEFAULT            0x00000003

//OA_STDCAP register
#define LAN8650_OA_STDCAP_TXFCSVC               0x00000400
#define LAN8650_OA_STDCAP_IPRAC                 0x00000200
#define LAN8650_OA_STDCAP_DPRAC                 0x00000100
#define LAN8650_OA_STDCAP_CTC                   0x00000080
#define LAN8650_OA_STDCAP_FTSC                  0x00000040
#define LAN8650_OA_STDCAP_AIDC                  0x00000020
#define LAN8650_OA_STDCAP_SEQC                  0x00000010
#define LAN8650_OA_STDCAP_MINCPS                0x00000007

//OA_RESET register
#define LAN8650_OA_RESET_SWRESET                0x00000001

//OA_CONFIG0 register
#define LAN8650_OA_CONFIG0_SYNC                 0x00008000
#define LAN8650_OA_CONFIG0_TXFCSVE              0x00004000
#define LAN8650_OA_CONFIG0_RFA                  0x00003000
#define LAN8650_OA_CONFIG0_RFA_DEFAULT          0x00000000
#define LAN8650_OA_CONFIG0_RFA_ZARFE            0x00001000
#define LAN8650_OA_CONFIG0_RFA_CSARFE           0x00002000
#define LAN8650_OA_CONFIG0_RFA_INVALID          0x00003000
#define LAN8650_OA_CONFIG0_TXCTHRESH            0x00000C00
#define LAN8650_OA_CONFIG0_TXCTHRESH_1_CREDIT   0x00000000
#define LAN8650_OA_CONFIG0_TXCTHRESH_4_CREDITS  0x00000400
#define LAN8650_OA_CONFIG0_TXCTHRESH_8_CREDITS  0x00000800
#define LAN8650_OA_CONFIG0_TXCTHRESH_16_CREDITS 0x00000C00
#define LAN8650_OA_CONFIG0_TXCTE                0x00000200
#define LAN8650_OA_CONFIG0_RXCTE                0x00000100
#define LAN8650_OA_CONFIG0_FTSE                 0x00000080
#define LAN8650_OA_CONFIG0_FTSS                 0x00000040
#define LAN8650_OA_CONFIG0_PROTE                0x00000020
#define LAN8650_OA_CONFIG0_SEQE                 0x00000010
#define LAN8650_OA_CONFIG0_CPS                  0x00000007
#define LAN8650_OA_CONFIG0_CPS_32_BYTES         0x00000005
#define LAN8650_OA_CONFIG0_CPS_64_BYTES         0x00000006

//OA_STATUS0 register
#define LAN8650_OA_STATUS0_CPDE                 0x00001000
#define LAN8650_OA_STATUS0_TXFSE                0x00000800
#define LAN8650_OA_STATUS0_TTSCAC               0x00000400
#define LAN8650_OA_STATUS0_TTSCAB               0x00000200
#define LAN8650_OA_STATUS0_TTSCAA               0x00000100
#define LAN8650_OA_STATUS0_PHYINT               0x00000080
#define LAN8650_OA_STATUS0_RESETC               0x00000040
#define LAN8650_OA_STATUS0_HDRE                 0x00000020
#define LAN8650_OA_STATUS0_LOFE                 0x00000010
#define LAN8650_OA_STATUS0_RXBOE                0x00000008
#define LAN8650_OA_STATUS0_TXBUE                0x00000004
#define LAN8650_OA_STATUS0_TXBOE                0x00000002
#define LAN8650_OA_STATUS0_TXPE                 0x00000001

//OA_STATUS1 register
#define LAN8650_OA_STATUS1_UV18                 0x00080000

//OA_BUFSTS register
#define LAN8650_OA_BUFSTS_TXC                   0x0000FF00
#define LAN8650_OA_BUFSTS_RCA                   0x000000FF

//OA_IMASK0 register
#define LAN8650_OA_IMASK0_CPDEM                 0x00001000
#define LAN8650_OA_IMASK0_TXFCSEM               0x00000800
#define LAN8650_OA_IMASK0_TTSCACM               0x00000400
#define LAN8650_OA_IMASK0_TTSCABM               0x00000200
#define LAN8650_OA_IMASK0_TTSCAAM               0x00000100
#define LAN8650_OA_IMASK0_PHYINTM               0x00000080
#define LAN8650_OA_IMASK0_RESETCM               0x00000040
#define LAN8650_OA_IMASK0_HDREM                 0x00000020
#define LAN8650_OA_IMASK0_LOFEM                 0x00000010
#define LAN8650_OA_IMASK0_RXBOEM                0x00000008
#define LAN8650_OA_IMASK0_TXBUEM                0x00000004
#define LAN8650_OA_IMASK0_TXBOEM                0x00000002
#define LAN8650_OA_IMASK0_TXPEM                 0x00000001

//OA_MASK1 register
#define LAN8650_OA_MASK1_UV18                   0x00080000

//TTSCAH register
#define LAN8650_TTSCAH_TIMESTAMPA_63_32         0xFFFFFFFF

//TTSCAL register
#define LAN8650_TTSCAL_TIMESTAMPA_31_0          0xFFFFFFFF

//TTSCBH register
#define LAN8650_TTSCBH_TIMESTAMPB_63_32         0xFFFFFFFF

//TTSCBL register
#define LAN8650_TTSCBL_TIMESTAMPB_31_0          0xFFFFFFFF

//TTSCCH register
#define LAN8650_TTSCCH_TIMESTAMPC_63_32         0xFFFFFFFF

//TTSCCL register
#define LAN8650_TTSCCL_TIMESTAMPC_31_0          0xFFFFFFFF

//BASIC_CONTROL register
#define LAN8650_BASIC_CONTROL_SW_RESET          0x8000
#define LAN8650_BASIC_CONTROL_LOOPBACK          0x4000
#define LAN8650_BASIC_CONTROL_SPD_SEL_LSB       0x2000
#define LAN8650_BASIC_CONTROL_AUTONEGEN         0x1000
#define LAN8650_BASIC_CONTROL_PD                0x0800
#define LAN8650_BASIC_CONTROL_REAUTONEG         0x0200
#define LAN8650_BASIC_CONTROL_DUPLEXMD          0x0100
#define LAN8650_BASIC_CONTROL_SPD_SEL_MSB       0x0040

//BASIC_STATUS register
#define LAN8650_BASIC_STATUS_100BT4A            0x8000
#define LAN8650_BASIC_STATUS_100BTXFDA          0x4000
#define LAN8650_BASIC_STATUS_100BTXHDA          0x2000
#define LAN8650_BASIC_STATUS_10BTFDA            0x1000
#define LAN8650_BASIC_STATUS_10BTHDA            0x0800
#define LAN8650_BASIC_STATUS_100BT2FDA          0x0400
#define LAN8650_BASIC_STATUS_100BT2HDA          0x0200
#define LAN8650_BASIC_STATUS_EXTSTS             0x0100
#define LAN8650_BASIC_STATUS_AUTONEGC           0x0020
#define LAN8650_BASIC_STATUS_RMTFLTD            0x0010
#define LAN8650_BASIC_STATUS_AUTONEGA           0x0008
#define LAN8650_BASIC_STATUS_LNKSTS             0x0004
#define LAN8650_BASIC_STATUS_JABDET             0x0002
#define LAN8650_BASIC_STATUS_EXTCAPA            0x0001

//PHY_ID1 register
#define LAN8650_PHY_ID1_OUI_2_9                 0x0000FF00
#define LAN8650_PHY_ID1_OUI_2_9_DEFAULT         0x00000000
#define LAN8650_PHY_ID1_OUI_10_17               0x000000FF
#define LAN8650_PHY_ID1_OUI_10_17_DEFAULT       0x00000007

//PHY_ID2 register
#define LAN8650_PHY_ID2_OUI_18_23               0x0000FC00
#define LAN8650_PHY_ID2_OUI_18_23_DEFAULT       0x0000C000
#define LAN8650_PHY_ID2_MODEL                   0x000003F0
#define LAN8650_PHY_ID2_MODEL_DEFAULT           0x000001B0
#define LAN8650_PHY_ID2_REV                     0x0000000F
#define LAN8650_PHY_ID2_REV_3                   0x00000003

//MMDCTRL register
#define LAN8650_MMDCTRL_FNCTN                   0xC000
#define LAN8650_MMDCTRL_FNCTN_ADDR              0x0000
#define LAN8650_MMDCTRL_FNCTN_DATA_NO_POST_INC  0x4000
#define LAN8650_MMDCTRL_FNCTN_DATA_POST_INC_RW  0x8000
#define LAN8650_MMDCTRL_FNCTN_DATA_POST_INC_W   0xC000
#define LAN8650_MMDCTRL_DEVAD                   0x001F
#define LAN8650_MMDCTRL_DEVAD_PMA_PMD           0x0001
#define LAN8650_MMDCTRL_DEVAD_PCS               0x0002
#define LAN8650_MMDCTRL_DEVAD_VENDOR_SPECIFIC_2 0x001F

//MMDAD register
#define LAN8650_MMDAD_ADR_DATA                  0xFFFF

//MAC_NCR register
#define LAN8650_MAC_NCR_TXEN                    0x00000008
#define LAN8650_MAC_NCR_RXEN                    0x00000004
#define LAN8650_MAC_NCR_LBL                     0x00000002

//MAC_NCFGR register
#define LAN8650_MAC_NCFGR_RXBP                  0x20000000
#define LAN8650_MAC_NCFGR_IRXFCS                0x04000000
#define LAN8650_MAC_NCFGR_EFRHD                 0x02000000
#define LAN8650_MAC_NCFGR_RFCS                  0x00020000
#define LAN8650_MAC_NCFGR_LFERD                 0x00010000
#define LAN8650_MAC_NCFGR_MAXFS                 0x00000100
#define LAN8650_MAC_NCFGR_UNIHEN                0x00000080
#define LAN8650_MAC_NCFGR_MTIHEN                0x00000040
#define LAN8650_MAC_NCFGR_NBC                   0x00000020
#define LAN8650_MAC_NCFGR_CAF                   0x00000010
#define LAN8650_MAC_NCFGR_DNVLAN                0x00000004

//MAC_SAB1 register
#define LAN8650_MAC_SAB1_ADDR_31_0              0xFFFFFFFF

//MAC_SAT1 register
#define LAN8650_MAC_SAT1_FLTTYP                 0x00010000
#define LAN8650_MAC_SAT1_ADDR_47_32             0x0000FFFF

//MAC_SAB2 register
#define LAN8650_MAC_SAB2_ADDR_31_0              0xFFFFFFFF

//MAC_SAT2 register
#define LAN8650_MAC_SAT2_FLTTYP                 0x00010000
#define LAN8650_MAC_SAT2_ADDR_47_32             0x0000FFFF

//MAC_SAB3 register
#define LAN8650_MAC_SAB3_ADDR_31_0              0xFFFFFFFF

//MAC_SAT3 register
#define LAN8650_MAC_SAT3_FLTTYP                 0x00010000
#define LAN8650_MAC_SAT3_ADDR_47_32             0x0000FFFF

//MAC_SAB4 register
#define LAN8650_MAC_SAB4_ADDR_31_0              0xFFFFFFFF

//MAC_SAT4 register
#define LAN8650_MAC_SAT4_FLTTYP                 0x00010000
#define LAN8650_MAC_SAT4_ADDR_47_32             0x0000FFFF

//MAC_TIDM1 register
#define LAN8650_MAC_TIDM1_ENID                  0x80000000
#define LAN8650_MAC_TIDM1_TID                   0x0000FFFF

//MAC_TIDM2 register
#define LAN8650_MAC_TIDM2_ENID                  0x80000000
#define LAN8650_MAC_TIDM2_TID                   0x0000FFFF

//MAC_TIDM3 register
#define LAN8650_MAC_TIDM3_ENID                  0x80000000
#define LAN8650_MAC_TIDM3_TID                   0x0000FFFF

//MAC_TIDM4 register
#define LAN8650_MAC_TIDM4_ENID                  0x80000000
#define LAN8650_MAC_TIDM4_TID                   0x0000FFFF

//MAC_SAMB1 register
#define LAN8650_MAC_SAMB1_ADDR_31_0             0xFFFFFFFF

//MAC_SAMT1 register
#define LAN8650_MAC_SAMT1_ADDR_47_32            0x0000FFFF

//MAC_TISUBN register
#define LAN8650_MAC_TISUBN_LSBTIR               0xFF000000
#define LAN8650_MAC_TISUBN_MSBTIR               0x0000FFFF

//MAC_TSH register
#define LAN8650_MAC_TSH_TCS_47_32               0x0000FFFF

//MAC_TSL register
#define LAN8650_MAC_TSL_TCS_31_0                0xFFFFFFFF

//MAC_TN register
#define LAN8650_MAC_TN_TNS                      0x3FFFFFFF

//MAC_TA register
#define LAN8650_MAC_TA_ADJ                      0x80000000
#define LAN8650_MAC_TA_ITDT                     0x3FFFFFFF

//MAC_TI register
#define LAN8650_MAC_TI_CNS                      0x000000FF

//BMGR_CTL register
#define LAN8650_BMGR_CTL_SNAPSTATS              0x00000020
#define LAN8650_BMGR_CTL_CLRSTATS               0x00000010

//STATS0 register
#define LAN8650_STATS0_RXSE                     0xFF000000
#define LAN8650_STATS0_LFER                     0x00FF0000
#define LAN8650_STATS0_OFRX                     0x0000FF00
#define LAN8650_STATS0_UFRX                     0x000000FF

//STATS1 register
#define LAN8650_STATS1_RXRER                    0xFF000000
#define LAN8650_STATS1_RXBOVR                   0x00FF0000
#define LAN8650_STATS1_RXFOVR                   0x0000FF00

//STATS2 register
#define LAN8650_STATS2_FCSE                     0x000000FF

//STATS3 register
#define LAN8650_STATS3_TID4MCNT                 0xFF000000
#define LAN8650_STATS3_TID3MCNT                 0x00FF0000
#define LAN8650_STATS3_TID2MCNT                 0x0000FF00
#define LAN8650_STATS3_TID1MCNT                 0x000000FF

//STATS4 register
#define LAN8650_STATS4_SA4MCNT                  0xFF000000
#define LAN8650_STATS4_SA3MCNT                  0x00FF0000
#define LAN8650_STATS4_SA2MCNT                  0x0000FF00
#define LAN8650_STATS4_SA1MCNT                  0x000000FF

//STATS5 register
#define LAN8650_STATS5_UHMFRX                   0xFF000000
#define LAN8650_STATS5_MHMFRX                   0x00FF0000
#define LAN8650_STATS5_BFRX                     0x0000FF00
#define LAN8650_STATS5_VTRX                     0x000000FF

//STATS6 register
#define LAN8650_STATS6_TFRX                     0xFFFFFFFF

//STATS7 register
#define LAN8650_STATS7_FRX                      0xFFFFFFFF

//STATS8 register
#define LAN8650_STATS8_TXAIE                    0x000000FF

//STATS9 register
#define LAN8650_STATS9_TXAEE                    0xFF000000
#define LAN8650_STATS9_TXFUR                    0x00FF0000
#define LAN8650_STATS9_TXBUR                    0x0000FF00

//STATS10 register
#define LAN8650_STATS10_XCOL                    0x000000FF

//STATS11 register
#define LAN8650_STATS11_TFTX                    0xFFFFFFFF

//STATS12 register
#define LAN8650_STATS12_FTX                     0xFFFFFFFF

//T1SPCSCTL register
#define LAN8650_T1SPCSCTL_RST                   0x8000
#define LAN8650_T1SPCSCTL_LBE                   0x4000
#define LAN8650_T1SPCSCTL_DUPLEX                0x0100

//T1SPCSSTS register
#define LAN8650_T1SPCSSTS_FAULT                 0x0080

//T1SPCSDIAG1 register
#define LAN8650_T1SPCSDIAG1_RMTJABCNT           0xFFFF

//T1SPCSDIAG2 register
#define LAN8650_T1SPCSDIAG2_CORTXCNT            0xFFFF

//T1PMAPMDEXTA register
#define LAN8650_T1PMAPMDEXTA_T1SABL             0x0008
#define LAN8650_T1PMAPMDEXTA_T1LABL             0x0004

//T1PMAPMDCTL register
#define LAN8650_T1PMAPMDCTL_TYPSEL              0x000F
#define LAN8650_T1PMAPMDCTL_TYPSEL_100BT1       0x0000
#define LAN8650_T1PMAPMDCTL_TYPSEL_1000BT1      0x0001
#define LAN8650_T1PMAPMDCTL_TYPSEL_10BT1L       0x0002
#define LAN8650_T1PMAPMDCTL_TYPSEL_10BT1S       0x0003

//T1SPMACTL register
#define LAN8650_T1SPMACTL_RST                   0x8000
#define LAN8650_T1SPMACTL_TXD                   0x4000
#define LAN8650_T1SPMACTL_LPE                   0x0800
#define LAN8650_T1SPMACTL_MDE                   0x0400
#define LAN8650_T1SPMACTL_LBE                   0x0001

//T1SPMASTS register
#define LAN8650_T1SPMASTS_LBA                   0x2000
#define LAN8650_T1SPMASTS_LPA                   0x0800
#define LAN8650_T1SPMASTS_MDA                   0x0400
#define LAN8650_T1SPMASTS_RXFA                  0x0200
#define LAN8650_T1SPMASTS_RXFD                  0x0002

//T1STSTCTL register
#define LAN8650_T1STSTCTL_TSTCTL                0xE000
#define LAN8650_T1STSTCTL_TSTCTL_NORMAL         0x0000
#define LAN8650_T1STSTCTL_TSTCTL_TEST_MODE_1    0x2000
#define LAN8650_T1STSTCTL_TSTCTL_TEST_MODE_2    0x4000
#define LAN8650_T1STSTCTL_TSTCTL_TEST_MODE_3    0x6000
#define LAN8650_T1STSTCTL_TSTCTL_TEST_MODE_4    0x8000

//CTRL1 register
#define LAN8650_CTRL1_IWDE                      0x0008
#define LAN8650_CTRL1_DIGLBE                    0x0002

//STS1 register
#define LAN8650_STS1_PSTC                       0x0800
#define LAN8650_STS1_TXCOL                      0x0400
#define LAN8650_STS1_TXJAB                      0x0200
#define LAN8650_STS1_EMPCYC                     0x0080
#define LAN8650_STS1_RXINTO                     0x0040
#define LAN8650_STS1_UNEXPB                     0x0020
#define LAN8650_STS1_BCNBFTO                    0x0010
#define LAN8650_STS1_PLCASYM                    0x0004
#define LAN8650_STS1_ESDERR                     0x0002
#define LAN8650_STS1_DEC5B                      0x0001

//STS2 register
#define LAN8650_STS2_WKEMDI                     0x0400
#define LAN8650_STS2_WKEWI                      0x0200
#define LAN8650_STS2_UV33                       0x0100
#define LAN8650_STS2_OT                         0x0040
#define LAN8650_STS2_IWDTO                      0x0020

//STS3 register
#define LAN8650_STS3_ERRTOID                    0x00FF

//IMSK1 register
#define LAN8650_IMSK1_PSTCM                     0x0800
#define LAN8650_IMSK1_TXCOLM                    0x0400
#define LAN8650_IMSK1_TXJABM                    0x0200
#define LAN8650_IMSK1_EMPCYCM                   0x0080
#define LAN8650_IMSK1_RXINTOM                   0x0040
#define LAN8650_IMSK1_UNEXPBM                   0x0020
#define LAN8650_IMSK1_BCNBFTOM                  0x0010
#define LAN8650_IMSK1_PLCASYMM                  0x0004
#define LAN8650_IMSK1_ESDERRM                   0x0002
#define LAN8650_IMSK1_DEC5BM                    0x0001

//IMSK2 register
#define LAN8650_IMSK2_WKEMDIM                   0x0400
#define LAN8650_IMSK2_WKEWIM                    0x0200
#define LAN8650_IMSK2_UV33M                     0x0100
#define LAN8650_IMSK2_OTM                       0x0040
#define LAN8650_IMSK2_IWDTOM                    0x0020

//CTRCTRL register
#define LAN8650_CTRCTRL_TOCTRE                  0x0002
#define LAN8650_CTRCTRL_BCNCTRE                 0x0001

//TOCNTH register
#define LAN8650_TOCNTH_TOCNT_31_16              0xFFFF

//TOCNTL register
#define LAN8650_TOCNTL_TOCNT_15_0               0xFFFF

//BCNCNTH register
#define LAN8650_BCNCNTH_BCNCNT_31_16            0xFFFF

//BCNCNTL register
#define LAN8650_BCNCNTL_BCNCNT_15_0             0xFFFF

//PRTMGMT2 register
#define LAN8650_PRTMGMT2_MIRXWDEN               0x2000
#define LAN8650_PRTMGMT2_PRIWDEN                0x1000
#define LAN8650_PRTMGMT2_MITXWDEN               0x0800

//IWDTOH register
#define LAN8650_IWDTOH_TIMEOUT_31_16            0xFFFF

//IWDTOL register
#define LAN8650_IWDTOL_TIMEOUT_15_0             0xFFFF

//SLPCTL0 register
#define LAN8650_SLPCTL0_SLPEN                   0x8000
#define LAN8650_SLPCTL0_WKINEN                  0x4000
#define LAN8650_SLPCTL0_MDIWKEN                 0x2000
#define LAN8650_SLPCTL0_SLPINHDLY               0x1800
#define LAN8650_SLPCTL0_SLPINHDLY_0MS           0x0000
#define LAN8650_SLPCTL0_SLPINHDLY_50MS          0x0800
#define LAN8650_SLPCTL0_SLPINHDLY_100MS         0x1000
#define LAN8650_SLPCTL0_SLPINHDLY_200MS         0x1800

//SLPCTL1 register
#define LAN8650_SLPCTL1_WIPOL                   0x0020
#define LAN8650_SLPCTL1_WAKEIND                 0x0010
#define LAN8650_SLPCTL1_CLRWKI                  0x0008
#define LAN8650_SLPCTL1_MWKFWD                  0x0004
#define LAN8650_SLPCTL1_WKOFWDEN                0x0002
#define LAN8650_SLPCTL1_MDIFWDEN                0x0001

//ANALOG5 register
#define LAN8650_ANALOG5_UV33FTM                 0xFF00
#define LAN8650_ANALOG5_UV33FTM_DEFAULT         0x1400

//MIDVER register
#define LAN8650_MIDVER_IDM                      0xFF00
#define LAN8650_MIDVER_IDM_DEFAULT              0x0A00
#define LAN8650_MIDVER_VER                      0x00FF
#define LAN8650_MIDVER_VER_DEFAULT              0x0010

//PLCA_CTRL0 register
#define LAN8650_PLCA_CTRL0_EN                   0x8000
#define LAN8650_PLCA_CTRL0_RST                  0x4000

//PLCA_CTRL1 register
#define LAN8650_PLCA_CTRL1_NCNT                 0xFF00
#define LAN8650_PLCA_CTRL1_ID                   0x00FF

//PLCA_STS register
#define LAN8650_PLCA_STS_PST                    0x8000

//PLCA_TOTMR register
#define LAN8650_PLCA_TOTMR_TOTMR                0x00FF
#define LAN8650_PLCA_TOTMR_TOTMR_DEFAULT        0x0020

//PLCA_BURST register
#define LAN8650_PLCA_BURST_MAXBC                0xFF00
#define LAN8650_PLCA_BURST_MAXBC_DEFAULT        0x0000
#define LAN8650_PLCA_BURST_BTMR                 0x00FF
#define LAN8650_PLCA_BURST_BTMR_DEFAULT         0x0080

//QTXCFG register
#define LAN8650_QTXCFG_CTTHR                    0xC0000000
#define LAN8650_QTXCFG_CTTHR_1_CHUNK            0x00000000
#define LAN8650_QTXCFG_CTTHR_2_CHUNKS           0x40000000
#define LAN8650_QTXCFG_CTTHR_3_CHUNKS           0x80000000
#define LAN8650_QTXCFG_CTTHR_4_CHUNKS           0xC0000000
#define LAN8650_QTXCFG_BUFSZ                    0x00700000
#define LAN8650_QTXCFG_BUFSZ_32_BYTES           0x00000000
#define LAN8650_QTXCFG_BUFSZ_64_BYTES           0x00100000
#define LAN8650_QTXCFG_MACFCSDIS                0x00080000

//QRXCFG register
#define LAN8650_QRXCFG_BUFSZ                    0x00700000
#define LAN8650_QRXCFG_BUFSZ_32_BYTES           0x00000000
#define LAN8650_QRXCFG_BUFSZ_64_BYTES           0x00100000

//PADCTRL register
#define LAN8650_PADCTRL_PDRV3                   0xC0000000
#define LAN8650_PADCTRL_PDRV3_LOW               0x00000000
#define LAN8650_PADCTRL_PDRV3_MEDIUM_LOW        0x40000000
#define LAN8650_PADCTRL_PDRV3_MEDIUM_HIGH       0x80000000
#define LAN8650_PADCTRL_PDRV3_HIGH              0xC0000000
#define LAN8650_PADCTRL_PDRV2                   0x30000000
#define LAN8650_PADCTRL_PDRV2_LOW               0x00000000
#define LAN8650_PADCTRL_PDRV2_MEDIUM_LOW        0x10000000
#define LAN8650_PADCTRL_PDRV2_MEDIUM_HIGH       0x20000000
#define LAN8650_PADCTRL_PDRV2_HIGH              0x30000000
#define LAN8650_PADCTRL_PDRV1                   0x0C000000
#define LAN8650_PADCTRL_PDRV1_LOW               0x00000000
#define LAN8650_PADCTRL_PDRV1_MEDIUM_LOW        0x04000000
#define LAN8650_PADCTRL_PDRV1_MEDIUM_HIGH       0x08000000
#define LAN8650_PADCTRL_PDRV1_HIGH              0x0C000000

//MISC register
#define LAN8650_MISC_UV18FEN                    0x00001000
#define LAN8650_MISC_UV18FTM                    0x00000FFF
#define LAN8650_MISC_UV18FTM_DEFAULT            0x00000040

//DEVID register
#define LAN8650_DEVID_MODEL                     0x000FFFF0
#define LAN8650_DEVID_MODEL_DEFAULT             0x00086500
#define LAN8650_DEVID_REV                       0x0000000F
#define LAN8650_DEVID_REV_1                     0x00000001

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//LAN8650 driver
extern const NicDriver lan8650Driver;

//LAN8650 related functions
error_t lan8650Init(NetInterface *interface);
void lan8650InitHook(NetInterface *interface);

void lan8650Config(NetInterface *interface);

void lan8650Tick(NetInterface *interface);

void lan8650EnableIrq(NetInterface *interface);
void lan8650DisableIrq(NetInterface *interface);
bool_t lan8650IrqHandler(NetInterface *interface);
void lan8650EventHandler(NetInterface *interface);

error_t lan8650SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t lan8650ReceivePacket(NetInterface *interface);

error_t lan8650UpdateMacAddrFilter(NetInterface *interface);

void lan8650WriteReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint32_t data);

uint32_t lan8650ReadReg(NetInterface *interface, uint8_t mms,
   uint16_t address);

void lan8650DumpReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint_t num);

void lan8650WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data);

uint16_t lan8650ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr);

int8_t lan8650ReadIndirectReg(NetInterface *interface, uint8_t address);

uint32_t lan8650CalcParity(uint32_t data);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
