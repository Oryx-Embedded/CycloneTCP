/**
 * @file ncn26010_driver.h
 * @brief Onsemi NCN26010 10Base-T1S Ethernet controller
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

#ifndef _NCN26010_DRIVER_H
#define _NCN26010_DRIVER_H

//Dependencies
#include "core/nic.h"

//PLCA support
#ifndef NCN26010_PLCA_SUPPORT
   #define NCN26010_PLCA_SUPPORT ENABLED
#elif (NCN26010_PLCA_SUPPORT != ENABLED && NCN26010_PLCA_SUPPORT != DISABLED)
   #error NCN26010_PLCA_SUPPORT parameter is not valid
#endif

//Node count
#ifndef NCN26010_NODE_COUNT
   #define NCN26010_NODE_COUNT 8
#elif (NCN26010_NODE_COUNT < 0 || NCN26010_NODE_COUNT > 255)
   #error NCN26010_NODE_COUNT parameter is not valid
#endif

//Local ID
#ifndef NCN26010_LOCAL_ID
   #define NCN26010_LOCAL_ID 1
#elif (NCN26010_LOCAL_ID < 0 || NCN26010_LOCAL_ID > 255)
   #error NCN26010_LOCAL_ID parameter is not valid
#endif

//RX buffer size
#ifndef NCN26010_ETH_RX_BUFFER_SIZE
   #define NCN26010_ETH_RX_BUFFER_SIZE 1536
#elif (NCN26010_ETH_RX_BUFFER_SIZE != 1536)
   #error NCN26010_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Chunk payload size
#define NCN26010_CHUNK_PAYLOAD_SIZE 64

//Transmit data header
#define NCN26010_TX_HEADER_DNC    0x80000000
#define NCN26010_TX_HEADER_SEQ    0x40000000
#define NCN26010_TX_HEADER_NORX   0x20000000
#define NCN26010_TX_HEADER_VS     0x00C00000
#define NCN26010_TX_HEADER_DV     0x00200000
#define NCN26010_TX_HEADER_SV     0x00100000
#define NCN26010_TX_HEADER_SWO    0x000F0000
#define NCN26010_TX_HEADER_EV     0x00004000
#define NCN26010_TX_HEADER_EBO    0x00003F00
#define NCN26010_TX_HEADER_TSC    0x000000C0
#define NCN26010_TX_HEADER_P      0x00000001

//Receive data footer
#define NCN26010_RX_FOOTER_EXST   0x80000000
#define NCN26010_RX_FOOTER_HDRB   0x40000000
#define NCN26010_RX_FOOTER_SYNC   0x20000000
#define NCN26010_RX_FOOTER_RCA    0x1F000000
#define NCN26010_RX_FOOTER_VS     0x00C00000
#define NCN26010_RX_FOOTER_DV     0x00200000
#define NCN26010_RX_FOOTER_SV     0x00100000
#define NCN26010_RX_FOOTER_SWO    0x000F0000
#define NCN26010_RX_FOOTER_FD     0x00008000
#define NCN26010_RX_FOOTER_EV     0x00004000
#define NCN26010_RX_FOOTER_EBO    0x00003F00
#define NCN26010_RX_FOOTER_RTSA   0x00000080
#define NCN26010_RX_FOOTER_RTSP   0x00000040
#define NCN26010_RX_FOOTER_TXC    0x0000003E
#define NCN26010_RX_FOOTER_P      0x00000001

//Control command header
#define NCN26010_CTRL_HEADER_DNC  0x80000000
#define NCN26010_CTRL_HEADER_HDRB 0x40000000
#define NCN26010_CTRL_HEADER_WNR  0x20000000
#define NCN26010_CTRL_HEADER_AID  0x10000000
#define NCN26010_CTRL_HEADER_MMS  0x0F000000
#define NCN26010_CTRL_HEADER_ADDR 0x00FFFF00
#define NCN26010_CTRL_HEADER_LEN  0x000000FE
#define NCN26010_CTRL_HEADER_P    0x00000001

//Memory map selectors
#define NCN26010_MMS_STD          0x00
#define NCN26010_MMS_MAC          0x01
#define NCN26010_MMS_PHY_PCS      0x02
#define NCN26010_MMS_PHY_PMA_PMD  0x03
#define NCN26010_MMS_PHY_PLCA     0x04
#define NCN26010_MMS_PHY_VS       0x0C

//NCN26010 Standard Control and Status registers (MMS 0)
#define NCN26010_IDVER                                    0x00, 0x0000
#define NCN26010_PHYID                                    0x00, 0x0001
#define NCN26010_SPICAP                                   0x00, 0x0002
#define NCN26010_RESET                                    0x00, 0x0003
#define NCN26010_CONFIG0                                  0x00, 0x0004
#define NCN26010_STATUS0                                  0x00, 0x0008
#define NCN26010_BUFSTS                                   0x00, 0x000B
#define NCN26010_IMSK0                                    0x00, 0x000C
#define NCN26010_PHYCTRL                                  0x00, 0xFF00
#define NCN26010_PHYSTATUS                                0x00, 0xFF01
#define NCN26010_PHYID0                                   0x00, 0xFF02
#define NCN26010_PHYID1                                   0x00, 0xFF03

//NCN26010 MAC registers (MMS 1)
#define NCN26010_MACCTRL0                                 0x01, 0x0000
#define NCN26010_ADDRFILT0L                               0x01, 0x0010
#define NCN26010_ADDRFILT0H                               0x01, 0x0011
#define NCN26010_ADDRFILT1L                               0x01, 0x0012
#define NCN26010_ADDRFILT1H                               0x01, 0x0013
#define NCN26010_ADDRFILT2L                               0x01, 0x0014
#define NCN26010_ADDRFILT2H                               0x01, 0x0015
#define NCN26010_ADDRFILT3L                               0x01, 0x0016
#define NCN26010_ADDRFILT3H                               0x01, 0x0017
#define NCN26010_ADDRMASK0L                               0x01, 0x0020
#define NCN26010_ADDRMASK0H                               0x01, 0x0021
#define NCN26010_ADDRMASK1L                               0x01, 0x0022
#define NCN26010_ADDRMASK1H                               0x01, 0x0023
#define NCN26010_ADDRMASK2L                               0x01, 0x0024
#define NCN26010_ADDRMASK2H                               0x01, 0x0025
#define NCN26010_ADDRMASK3L                               0x01, 0x0026
#define NCN26010_ADDRMASK3H                               0x01, 0x0027
#define NCN26010_STOCTETSTXL                              0x01, 0x0030
#define NCN26010_STOCTETSTXH                              0x01, 0x0031
#define NCN26010_STFRAMESTXOK                             0x01, 0x0032
#define NCN26010_STBCASTTXOK                              0x01, 0x0033
#define NCN26010_STMCASTTXOK                              0x01, 0x0034
#define NCN26010_STFRAMESTX64                             0x01, 0x0035
#define NCN26010_STFRAMESTX65                             0x01, 0x0036
#define NCN26010_STFRAMESTX128                            0x01, 0x0037
#define NCN26010_STFRAMESTX256                            0x01, 0x0038
#define NCN26010_STFRAMESTX512                            0x01, 0x0039
#define NCN26010_STFRAMESTX1024                           0x01, 0x003A
#define NCN26010_STUNDERFLOW                              0x01, 0x003B
#define NCN26010_STSINGLECOL                              0x01, 0x003C
#define NCN26010_STMULTICOL                               0x01, 0x003D
#define NCN26010_STEXCESSCOL                              0x01, 0x003E
#define NCN26010_STDEFERREDTX                             0x01, 0x003F
#define NCN26010_STCRSERR                                 0x01, 0x0040
#define NCN26010_STOCTETSRXL                              0x01, 0x0041
#define NCN26010_STOCTETSRXH                              0x01, 0x0042
#define NCN26010_STFRAMESRXOK                             0x01, 0x0043
#define NCN26010_STBCASTRXOK                              0x01, 0x0044
#define NCN26010_STMCASTRXOK                              0x01, 0x0045
#define NCN26010_STFRAMESRX64                             0x01, 0x0046
#define NCN26010_STFRAMESRX65                             0x01, 0x0047
#define NCN26010_STFRAMESRX128                            0x01, 0x0048
#define NCN26010_STFRAMESRX256                            0x01, 0x0049
#define NCN26010_STFRAMESRX512                            0x01, 0x004A
#define NCN26010_STFRAMESRX1024                           0x01, 0x004B
#define NCN26010_STRUNTERR                                0x01, 0x004C
#define NCN26010_STRXTOOLONG                              0x01, 0x004D
#define NCN26010_STFCSERRS                                0x01, 0x004E
#define NCN26010_STSYMBOLERRS                             0x01, 0x004F
#define NCN26010_STALIGNERRS                              0x01, 0x0050
#define NCN26010_STRXOVERFLOW                             0x01, 0x0051
#define NCN26010_STRXDROPPED                              0x01, 0x0052

//NCN26010 PHY PCS registers (MMS 2)
#define NCN26010_PCS_DEVINPKG1                            0x02, 0x0005
#define NCN26010_PCS_DEVINPKG2                            0x02, 0x0006
#define NCN26010_T1SPCSCTRL                               0x02, 0x08F3
#define NCN26010_T1SPCSSTATUS                             0x02, 0x08F4
#define NCN26010_T1SPCSRMTJAB                             0x02, 0x08F5
#define NCN26010_T1SPCSPHYCOL                             0x02, 0x08F6

//NCN26010 PHY PMA/PMD registers (MMS 3)
#define NCN26010_PMA_DEVINPKG1                            0x03, 0x0005
#define NCN26010_PMA_DEVINPKG2                            0x03, 0x0006
#define NCN26010_BASET1EXTABLTY                           0x03, 0x0012
#define NCN26010_T1SPMACTRL                               0x03, 0x08F9
#define NCN26010_T1SPMASTS                                0x03, 0x08FA
#define NCN26010_T1STMCTL                                 0x03, 0x08FB

//NCN26010 PHY PLCA registers (MMS 4)
#define NCN26010_CHIPREV                                  0x04, 0x8000
#define NCN26010_PHYCFG1                                  0x04, 0x8001
#define NCN26010_PLCAEXT                                  0x04, 0x8002
#define NCN26010_PMATUNE0                                 0x04, 0x8003
#define NCN26010_PMATUNE1                                 0x04, 0x8004
#define NCN26010_PLCAREGMAP                               0x04, 0xCA00
#define NCN26010_PLCACTRL0                                0x04, 0xCA01
#define NCN26010_PLCACTRL1                                0x04, 0xCA02
#define NCN26010_PLCASTATUS                               0x04, 0xCA03
#define NCN26010_PLCATOTMR                                0x04, 0xCA04
#define NCN26010_PLCABURST                                0x04, 0xCA05

//NCN26010 Vendor Specific registers (MMS 12)
#define NCN26010_MIIMIRQCTRL                              0x0C, 0x0010
#define NCN26010_MIIMIRQSTS                               0x0C, 0x0011
#define NCN26010_DIOCFG                                   0x0C, 0x0012
#define NCN26010_PHYTWEAK                                 0x0C, 0x1001
#define NCN26010_MACID0                                   0x0C, 0x1002
#define NCN26010_MACID1                                   0x0C, 0x1003
#define NCN26010_CHIPINFO                                 0x0C, 0x1004
#define NCN26010_NVMHEALTH                                0x0C, 0x1005

//SPI Identification register
#define NCN26010_IDVER_MAJVER                             0x000000F0
#define NCN26010_IDVER_MAJVER_DEFAULT                     0x00000010
#define NCN26010_IDVER_MINVER                             0x0000000F
#define NCN26010_IDVER_MINVER_DEFAULT                     0x00000001

//SPI Identification register
#define NCN26010_PHYID_OUI                                0xFFFFFC00
#define NCN26010_PHYID_OUI_DEFAULT                        0x180FF400
#define NCN26010_PHYID_MODEL                              0x000003F0
#define NCN26010_PHYID_MODEL_DEFAULT                      0x000001A0
#define NCN26010_PHYID_REV                                0x0000000F
#define NCN26010_PHYID_REV_DEFAULT                        0x00000001

//SPI Capabilities register
#define NCN26010_SPICAP_TXFCSVC                           0x00000400
#define NCN26010_SPICAP_IPRAC                             0x00000200
#define NCN26010_SPICAP_DPRAC                             0x00000100
#define NCN26010_SPICAP_CTC                               0x00000080
#define NCN26010_SPICAP_FTC                               0x00000040
#define NCN26010_SPICAP_AIDC                              0x00000020
#define NCN26010_SPICAP_SEQ                               0x00000010
#define NCN26010_SPICAP_MINCPS                            0x00000007

//Reset Control And Status register
#define NCN26010_RESET_RESET                              0x00000001

//SPI Protocol Configuration 0 register
#define NCN26010_CONFIG0_SYNC                             0x00008000
#define NCN26010_CONFIG0_TXFCSVE                          0x00004000
#define NCN26010_CONFIG0_CSARFE                           0x00002000
#define NCN26010_CONFIG0_ZARFE                            0x00001000
#define NCN26010_CONFIG0_TXCTHRESH                        0x00000C00
#define NCN26010_CONFIG0_TXCTHRESH_1_CREDIT               0x00000000
#define NCN26010_CONFIG0_TXCTHRESH_4_CREDITS              0x00000400
#define NCN26010_CONFIG0_TXCTHRESH_8_CREDITS              0x00000800
#define NCN26010_CONFIG0_TXCTHRESH_16_CREDITS             0x00000C00
#define NCN26010_CONFIG0_TXCTE                            0x00000200
#define NCN26010_CONFIG0_RXCTE                            0x00000100
#define NCN26010_CONFIG0_FTSE                             0x00000080
#define NCN26010_CONFIG0_FTSS                             0x00000040
#define NCN26010_CONFIG0_PROTE                            0x00000020
#define NCN26010_CONFIG0_CPS                              0x00000007
#define NCN26010_CONFIG0_CPS_8_BYTES                      0x00000003
#define NCN26010_CONFIG0_CPS_16_BYTES                     0x00000004
#define NCN26010_CONFIG0_CPS_32_BYTES                     0x00000005
#define NCN26010_CONFIG0_CPS_64_BYTES                     0x00000006

//SPI Protocol Status 0 register
#define NCN26010_STATUS0_CDPE                             0x00001000
#define NCN26010_STATUS0_TXFCSE                           0x00000800
#define NCN26010_STATUS0_TTSCAC                           0x00000400
#define NCN26010_STATUS0_TTSCAB                           0x00000200
#define NCN26010_STATUS0_TTSCAA                           0x00000100
#define NCN26010_STATUS0_PHYINT                           0x00000080
#define NCN26010_STATUS0_RESETC                           0x00000040
#define NCN26010_STATUS0_HDRE                             0x00000020
#define NCN26010_STATUS0_LOFE                             0x00000010
#define NCN26010_STATUS0_RXBOE                            0x00000008
#define NCN26010_STATUS0_TXBUE                            0x00000004
#define NCN26010_STATUS0_TXBOE                            0x00000002
#define NCN26010_STATUS0_TXPE                             0x00000001

//Buffer Status register
#define NCN26010_BUFSTS_TXC                               0x0000FF00
#define NCN26010_BUFSTS_RCA                               0x000000FF

//Interrupt Mask 0 register
#define NCN26010_IMSK0_CDPEM                              0x00001000
#define NCN26010_IMSK0_TXFCSEM                            0x00000800
#define NCN26010_IMSK0_PHYINTM                            0x00000080
#define NCN26010_IMSK0_RESETCM                            0x00000040
#define NCN26010_IMSK0_HDREM                              0x00000020
#define NCN26010_IMSK0_LOFEM                              0x00000010
#define NCN26010_IMSK0_RXDOEM                             0x00000008
#define NCN26010_IMSK0_TXBUEM                             0x00000004
#define NCN26010_IMSK0_TXBOEM                             0x00000002
#define NCN26010_IMSK0_TXPEM                              0x00000001

//PHY Control register
#define NCN26010_PHYCTRL_RESET                            0x00008000
#define NCN26010_PHYCTRL_LOOPBACK                         0x00004000
#define NCN26010_PHYCTRL_SPEED_LSB                        0x00002000
#define NCN26010_PHYCTRL_LINK_CONTROL                     0x00001000
#define NCN26010_PHYCTRL_ISOLATE                          0x00000400
#define NCN26010_PHYCTRL_LINK_RESET                       0x00000200
#define NCN26010_PHYCTRL_DUPLEX_MODE                      0x00000100
#define NCN26010_PHYCTRL_COLLISION_TEST                   0x00000080
#define NCN26010_PHYCTRL_SPEED_MSB                        0x00000040

//PHY Status register
#define NCN26010_PHYSTATUS_10_HALF_DUPLEX                 0x00000800
#define NCN26010_PHYSTATUS_UNIDIRECTIONAL_ABILITY         0x00000080
#define NCN26010_PHYSTATUS_MF_PREAMBLE_SUPPR              0x00000040
#define NCN26010_PHYSTATUS_LINK_NEGOTIATION_COMPLETE      0x00000020
#define NCN26010_PHYSTATUS_REMOTE_FAULT                   0x00000010
#define NCN26010_PHYSTATUS_AUTO_NEGOTIATION_ABILITY       0x00000008
#define NCN26010_PHYSTATUS_LINK_STATUS                    0x00000004
#define NCN26010_PHYSTATUS_JABBER_DETECT                  0x00000002
#define NCN26010_PHYSTATUS_EXTENDED_CAPABILITY            0x00000001

//PHY Identifier 0 register
#define NCN26010_PHYID0_OUI_MSB                           0x0000FFFF
#define NCN26010_PHYID0_OUI_MSB_DEFAULT                   0x0000180F

//PHY Identifier 1 register
#define NCN26010_PHYID1_OUI_LSB                           0x0000FC00
#define NCN26010_PHYID1_OUI_LSB_DEFAULT                   0x0000D400
#define NCN26010_PHYID1_MODEL_NUM                         0x000003F0
#define NCN26010_PHYID1_MODEL_NUM_DEFAULT                 0x000001A0
#define NCN26010_PHYID1_REV_NUM                           0x0000000F
#define NCN26010_PHYID1_REV_NUM_DEFAULT                   0x00000001

//MAC Control 0 register
#define NCN26010_MACCTRL0_IPGNF                           0x00200000
#define NCN26010_MACCTRL0_BKOD                            0x00100000
#define NCN26010_MACCTRL0_NFCSF                           0x00080000
#define NCN26010_MACCTRL0_MCSF                            0x00040000
#define NCN26010_MACCTRL0_BCSF                            0x00020000
#define NCN26010_MACCTRL0_ADRF                            0x00010000
#define NCN26010_MACCTRL0_FCSA                            0x00000100
#define NCN26010_MACCTRL0_TXEN                            0x00000002
#define NCN26010_MACCTRL0_RXEN                            0x00000001

//Address Filter Low register
#define NCN26010_ADDRFILTnL_ADDRFILT_31_0                 0xFFFFFFFF

//Address Filter High register
#define NCN26010_ADDRFILTnH_EN                            0x80000000
#define NCN26010_ADDRFILTnH_ADDRFILT_47_32                0x0000FFFF

//Address Mask Low register
#define NCN26010_ADDRMASKnL_ADDRMASK_31_0                 0xFFFFFFFF

//Address Mask High register
#define NCN26010_ADDRMASKnH_ADDRMASK_47_32                0x00008000

//Statistic Sent Bytes Counter Low register
#define NCN26010_STOCTETSTXL_STOCTETSTXL_31_0             0xFFFFFFFF

//Statistic Sent Bytes Counter High register
#define NCN26010_STOCTETSTXH_STOCTETSTXL_47_32            0x0000FFFF

//Statistic Aborted Frames Due To TX-buffer Underflow register
#define NCN26010_STUNDERFLOW_STUNDERFLOW                  0x000003FF

//Statistic Frames Transmitted After Single Collision register
#define NCN26010_STSINGLECOL_STSINGLECOL                  0x0003FFFF

//Statistic Frames Transmitted After Multiple Collisions register
#define NCN26010_STMULTICOL_STMULTICOL                    0x0003FFFF

//Statistic Frames Transmitted After Excessive Collisions register
#define NCN26010_STEXCESSCOL_STEXCESSCOL                  0x000003FF

//Statistic Frames Transmitted After Deferral register
#define NCN26010_STDEFERREDTX_STDEFERREDTX                0x0003FFFF

//Statistic Counter Of CRS De-assertion During Frame Transmission register
#define NCN26010_STCRSERR_STCRSERR                        0x000003FF

//Statistic Received Bytes Counter Low register
#define NCN26010_STOCTETSRXL_STOCTETSRX_31_0              0xFFFFFFFF

//Statistic Received Bytes Counter High register
#define NCN26010_STOCTETSRXH_STOCTETSRX_47_32             0x0000FFFF

//Statistic Dropped Too Short Frames register
#define NCN26010_STRUNTERR_STRUNTERR                      0x000003FF

//Statistic Dropped Too Long Frames register
#define NCN26010_STRXTOOLONG_STRXTOOLONG                  0x000003FF

//Statistic Dropped FCS Error Frames register
#define NCN26010_STFCSERRS_STFCSERRS                      0x000003FF

//Statistic Symbol Errors During Frame Reception register
#define NCN26010_STSYMBOLERRS_STSYMBOLERRS                0x000003FF

//Statistic Align Errors During Frame Reception register
#define NCN26010_STALIGNERRS_STALIGNERRS                  0x000003FF

//Statistic RX Buffer Overflow Errors register
#define NCN26010_STRXOVERFLOW_STRXOVERFLOW                0x000003FF

//Devices In Package 1 register
#define NCN26010_PCS_DEVINPKG1_PCS_PRESENT                0x0008
#define NCN26010_PCS_DEVINPKG1_PMA_PRESENT                0x0002
#define NCN26010_PCS_DEVINPKG1_CLAUSE_22_REGS_PRESENT     0x0001

//10BASE-T1S PCS Control register
#define NCN26010_T1SPCSCTRL_PCS_RESET                     0x8000
#define NCN26010_T1SPCSCTRL_LOOPBACK                      0x4000

//10BASE-T1S PCS Status register
#define NCN26010_T1SPCSSTATUS_FAULT                       0x0080

//10BASE-T1S PCS Diagnostics 1 register
#define NCN26010_T1SPCSRMTJAB_PCS_REMOTE_JABBER_COUNT     0xFFFF

//10BASE-T1S PCS Diagnostics 2 register
#define NCN26010_T1SPCSPHYCOL_PCS_PHY_COL_COUNT           0xFFFF

//Devices In Package 1 register
#define NCN26010_PMA_DEVINPKG1_PCS_PRESENT                0x0008
#define NCN26010_PMA_DEVINPKG1_PMA_PRESENT                0x0002
#define NCN26010_PMA_DEVINPKG1_CLAUSE_22_REGS_PRESENT     0x0001

//BASE-T1 Extended Ability register
#define NCN26010_BASET1EXTABLTY_10BASE_T1S                0x0008

//10BASE-T1S PMA Control register
#define NCN26010_T1SPMACTRL_PMA_RESET                     0x8000
#define NCN26010_T1SPMACTRL_TX_DIS                        0x4000
#define NCN26010_T1SPMACTRL_LOW_POWER_MODE                0x0800
#define NCN26010_T1SPMACTRL_MULTI_DROP_EN                 0x0400
#define NCN26010_T1SPMACTRL_LOOPBACK_MODE                 0x0001

//10BASE-T1S PMA Status register
#define NCN26010_T1SPMASTS_LOOPBACK_ABILITY               0x2000
#define NCN26010_T1SPMASTS_LOW_POWER_ABILITY              0x0800
#define NCN26010_T1SPMASTS_MULTI_DROP_ABILITY             0x0400
#define NCN26010_T1SPMASTS_RECEIVE_FAULT_ABILITY          0x0200
#define NCN26010_T1SPMASTS_REMOTE_JABBER                  0x0002

//10BASE-T1S Test Mode Control register
#define NCN26010_T1STMCTL_TEST_MODE                       0xE000
#define NCN26010_T1STMCTL_TEST_MODE_NORMAL                0x0000
#define NCN26010_T1STMCTL_TEST_MODE_TX_OUT_VOLTAGE_TEST   0x2000
#define NCN26010_T1STMCTL_TEST_MODE_TX_OUT_DROP_TEST      0x4000
#define NCN26010_T1STMCTL_TEST_MODE_TX_PSD_MASK           0x6000
#define NCN26010_T1STMCTL_TEST_MODE_TX_HIGH_Z_TEST        0x8000

//Chip Revision register
#define NCN26010_CHIPREV_MAJOR_REV                        0xF000
#define NCN26010_CHIPREV_MAJOR_REV_DEFAULT                0x1000
#define NCN26010_CHIPREV_MINOR_REV                        0x0F00
#define NCN26010_CHIPREV_MINOR_REV_DEFAULT                0x0000
#define NCN26010_CHIPREV_STAGE                            0x00C0
#define NCN26010_CHIPREV_STAGE_DEFAULT                    0x00C0
#define NCN26010_CHIPREV_PATCH                            0x003F
#define NCN26010_CHIPREV_PATCH_DEFAULT                    0x0001

//PHY Configuration 1 register
#define NCN26010_PHYCFG1_ENHANCED_NOISE_IMMUNITY          0x0080
#define NCN26010_PHYCFG1_UNJAB_TIMER_EN                   0x0040
#define NCN26010_PHYCFG1_SCRAMBLER_DIS                    0x0004
#define NCN26010_PHYCFG1_NO_COL_MASKING                   0x0002
#define NCN26010_PHYCFG1_RX_DELAY                         0x0001

//PLCA Extensions register
#define NCN26010_PLCAEXT_PLCA_PRECEDENCE                  0x8000
#define NCN26010_PLCAEXT_COORDINATOR_MODE                 0x0002
#define NCN26010_PLCAEXT_COORDINATOR_ROLE                 0x0001

//PMA Tune 0 register
#define NCN26010_PMATUNE0_PLCA_BEACON_DETECT_THRESHOLD    0x3F00
#define NCN26010_PMATUNE0_DRIFT_COMPENSATION_WIN_SEL      0x0007

//PMA Tune 1 register
#define NCN26010_PMATUNE1_PKT_PREAMBLE_DETEC_THRESHOLD    0x3F00
#define NCN26010_PMATUNE1_COMMIT_DETECT_THRESHOLD         0x003F

//PLCA Register Map And Identification register
#define NCN26010_PLCAREGMAP_MAPID                         0xFF00
#define NCN26010_PLCAREGMAP_MAPVER                        0x00FF

//PLCA Control 0 register
#define NCN26010_PLCACTRL0_PLCA_EN                        0x8000
#define NCN26010_PLCACTRL0_PLCA_RESET                     0x4000

//PLCA Control 1 register
#define NCN26010_PLCACTRL1_NCNT                           0xFF00
#define NCN26010_PLCACTRL1_ID                             0x00FF

//PLCA Status register
#define NCN26010_PLCASTATUS_PST                           0x8000

//PLCA Transmit Opportunity Timer register
#define NCN26010_PLCATOTMR_TOTMR                          0x00FF
#define NCN26010_PLCATOTMR_TOTMR_DEFAULT                  0x0018

//PLCA Burst Mode register
#define NCN26010_PLCABURST_MAXBC                          0xFF00
#define NCN26010_PLCABURST_MAXBC_DEFAULT                  0x0000
#define NCN26010_PLCABURST_BTMR                           0x00FF
#define NCN26010_PLCABURST_BTMR_DEFAULT                   0x0080

//MIIM IRQ Control register
#define NCN26010_MIIMIRQCTRL_PHY_COL_REPORT               0x0020
#define NCN26010_MIIMIRQCTRL_PLCA_RECOVERY_REPORT         0x0010
#define NCN26010_MIIMIRQCTRL_REMOTE_JABBER_REPORT         0x0008
#define NCN26010_MIIMIRQCTRL_LOCAL_JABBER_REPORT          0x0004
#define NCN26010_MIIMIRQCTRL_PLCA_STATUS_CHANGE_REPORT    0x0002
#define NCN26010_MIIMIRQCTRL_LINK_STATUS_CHANGE_REPORT    0x0001

//MIIM IRQ Status register
#define NCN26010_MIIMIRQSTS_RESET_STATUS                  0x8000
#define NCN26010_MIIMIRQSTS_PHY_COL                       0x0020
#define NCN26010_MIIMIRQSTS_PLCA_RECOVERY                 0x0010
#define NCN26010_MIIMIRQSTS_REMOTE_JABBER                 0x0008
#define NCN26010_MIIMIRQSTS_LOCAL_JABBER                  0x0004
#define NCN26010_MIIMIRQSTS_PLCA_STATUS_CHANGE            0x0002
#define NCN26010_MIIMIRQSTS_LINK_STATUS_CHANGE            0x0001

//DIO Configuration register
#define NCN26010_DIOCFG_SLEW_RATE_1                       0x8000
#define NCN26010_DIOCFG_PULL_EN_1                         0x4000
#define NCN26010_DIOCFG_PULL_TYPE_1                       0x2000
#define NCN26010_DIOCFG_PULL_TYPE_1_PULL_UP               0x0000
#define NCN26010_DIOCFG_PULL_TYPE_1_PULL_DOWN             0x2000
#define NCN26010_DIOCFG_FN1                               0x1E00
#define NCN26010_DIOCFG_FN1_DISABLE                       0x0000
#define NCN26010_DIOCFG_FN1_GPIO                          0x0200
#define NCN26010_DIOCFG_FN1_SFD_TX                        0x0400
#define NCN26010_DIOCFG_FN1_SFD_RX                        0x0600
#define NCN26010_DIOCFG_FN1_LED_LINK_CTRL                 0x0800
#define NCN26010_DIOCFG_FN1_LED_PLCA_STATUS               0x0A00
#define NCN26010_DIOCFG_FN1_LED_TX                        0x0C00
#define NCN26010_DIOCFG_FN1_LED_RX                        0x0E00
#define NCN26010_DIOCFG_FN1_CLK25M                        0x1000
#define NCN26010_DIOCFG_FN1_SFD_RX_TX                     0x1600
#define NCN26010_DIOCFG_FN1_LED_TX_RX                     0x1E00
#define NCN26010_DIOCFG_VAL1                              0x0100
#define NCN26010_DIOCFG_SLEW_RATE_0                       0x0080
#define NCN26010_DIOCFG_PULL_EN_0                         0x0040
#define NCN26010_DIOCFG_PULL_TYPE_0                       0x0020
#define NCN26010_DIOCFG_PULL_TYPE_0_PULL_UP               0x0000
#define NCN26010_DIOCFG_PULL_TYPE_0_PULL_DOWN             0x0020
#define NCN26010_DIOCFG_FN0                               0x001E
#define NCN26010_DIOCFG_FN0_DISABLE                       0x0000
#define NCN26010_DIOCFG_FN0_GPIO                          0x0002
#define NCN26010_DIOCFG_FN0_SFD_TX                        0x0004
#define NCN26010_DIOCFG_FN0_SFD_RX                        0x0006
#define NCN26010_DIOCFG_FN0_LED_LINK_CTRL                 0x0008
#define NCN26010_DIOCFG_FN0_LED_PLCA_STATUS               0x000A
#define NCN26010_DIOCFG_FN0_LED_TX                        0x000C
#define NCN26010_DIOCFG_FN0_LED_RX                        0x000E
#define NCN26010_DIOCFG_FN0_CLK25M                        0x0010
#define NCN26010_DIOCFG_FN0_SFD_RX_TX                     0x0016
#define NCN26010_DIOCFG_FN0_LED_TX_RX                     0x001E
#define NCN26010_DIOCFG_VAL0                              0x0001

//PHY Tweaks register
#define NCN26010_PHYTWEAK_TX_GAIN                         0xC000
#define NCN26010_PHYTWEAK_TX_GAIN_1000_MVPP               0x0000
#define NCN26010_PHYTWEAK_TX_GAIN_1100_MVPP               0x4000
#define NCN26010_PHYTWEAK_TX_GAIN_900_MVPP                0x8000
#define NCN26010_PHYTWEAK_TX_GAIN_800_MVPP                0xC000
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD                 0x3C00
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_150_MVPP        0x0000
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_200_MVPP        0x0400
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_250_MVPP        0x0800
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_300_MVPP        0x0C00
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_350_MVPP        0x1000
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_400_MVPP        0x1400
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_450_MVPP        0x1800
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_500_MVPP        0x1C00
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_550_MVPP        0x2000
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_600_MVPP        0x2400
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_650_MVPP        0x2800
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_700_MVPP        0x2C00
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_750_MVPP        0x3000
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_800_MVPP        0x3400
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_850_MVPP        0x3800
#define NCN26010_PHYTWEAK_RX_CD_THRESHOLD_900_MVPP        0x3C00
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD                 0x03C0
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_150_MVPP        0x0000
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_200_MVPP        0x0040
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_250_MVPP        0x0080
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_300_MVPP        0x00C0
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_350_MVPP        0x0100
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_400_MVPP        0x0140
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_450_MVPP        0x0180
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_500_MVPP        0x01C0
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_550_MVPP        0x0200
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_600_MVPP        0x0240
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_650_MVPP        0x0280
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_700_MVPP        0x02C0
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_750_MVPP        0x0300
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_800_MVPP        0x0340
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_850_MVPP        0x0380
#define NCN26010_PHYTWEAK_RX_ED_THRESHOLD_900_MVPP        0x03C0
#define NCN26010_PHYTWEAK_DIGITAL_SLEW_RATE               0x0020
#define NCN26010_PHYTWEAK_DIGITAL_SLEW_RATE_SLOW          0x0000
#define NCN26010_PHYTWEAK_DIGITAL_SLEW_RATE_FAST          0x0020
#define NCN26010_PHYTWEAK_CMC_COMPENSATION                0x0018
#define NCN26010_PHYTWEAK_CMC_COMPENSATION_0_TO_0_5_R     0x0000
#define NCN26010_PHYTWEAK_CMC_COMPENSATION_0_5_TO_2_25_R  0x0008
#define NCN26010_PHYTWEAK_CMC_COMPENSATION_2_25_TO_3_75_R 0x0010
#define NCN26010_PHYTWEAK_CMC_COMPENSATION_3_75_TO_5_R    0x0018
#define NCN26010_PHYTWEAK_TX_SLEW                         0x0004
#define NCN26010_PHYTWEAK_TX_SLEW_SLOW                    0x0000
#define NCN26010_PHYTWEAK_TX_SLEW_FAST                    0x0004
#define NCN26010_PHYTWEAK_CLK_OUT_EN                      0x0001

//MAC Identification 0 register
#define NCN26010_MACID0_MACID_15_0                        0xFFFF

//MAC Identification 1 register
#define NCN26010_MACID1_MACID_23_16                       0x00FF

//Chip Info register
#define NCN26010_CHIPINFO_WAFER_Y                         0x7F00
#define NCN26010_CHIPINFO_WAFER_X                         0x007F

//NVM Health register
#define NCN26010_NVMHEALTH_RED_ZONE_NVM_WARNING           0x8000
#define NCN26010_NVMHEALTH_RED_ZONE_NVM_ERROR             0x4000
#define NCN26010_NVMHEALTH_YELLOW_ZONE_NVM_WARNING        0x2000
#define NCN26010_NVMHEALTH_YELLOW_ZONE_NVM_ERROR          0x1000
#define NCN26010_NVMHEALTH_GREEN_ZONE_NVM_WARNING         0x0800
#define NCN26010_NVMHEALTH_GREEN_ZONE_NVM_ERROR           0x0400

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//NCN26010 driver
extern const NicDriver ncn26010Driver;

//NCN26010 related functions
error_t ncn26010Init(NetInterface *interface);
void ncn26010InitHook(NetInterface *interface);

void ncn26010Tick(NetInterface *interface);

void ncn26010EnableIrq(NetInterface *interface);
void ncn26010DisableIrq(NetInterface *interface);
bool_t ncn26010IrqHandler(NetInterface *interface);
void ncn26010EventHandler(NetInterface *interface);

error_t ncn26010SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t ncn26010ReceivePacket(NetInterface *interface);

error_t ncn26010UpdateMacAddrFilter(NetInterface *interface);

void ncn26010WriteReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint32_t data);

uint32_t ncn26010ReadReg(NetInterface *interface, uint8_t mms,
   uint16_t address);

void ncn26010DumpReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint_t num);

uint32_t ncn26010CalcParity(uint32_t data);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
