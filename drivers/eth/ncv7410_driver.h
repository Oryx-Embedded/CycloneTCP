/**
 * @file ncv7410_driver.h
 * @brief Onsemi NCV7410 10Base-T1S Ethernet controller
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

#ifndef _NCV7410_DRIVER_H
#define _NCV7410_DRIVER_H

//Dependencies
#include "core/nic.h"

//PLCA support
#ifndef NCV7410_PLCA_SUPPORT
   #define NCV7410_PLCA_SUPPORT ENABLED
#elif (NCV7410_PLCA_SUPPORT != ENABLED && NCV7410_PLCA_SUPPORT != DISABLED)
   #error NCV7410_PLCA_SUPPORT parameter is not valid
#endif

//Node count
#ifndef NCV7410_NODE_COUNT
   #define NCV7410_NODE_COUNT 8
#elif (NCV7410_NODE_COUNT < 0 || NCV7410_NODE_COUNT > 255)
   #error NCV7410_NODE_COUNT parameter is not valid
#endif

//Local ID
#ifndef NCV7410_LOCAL_ID
   #define NCV7410_LOCAL_ID 1
#elif (NCV7410_LOCAL_ID < 0 || NCV7410_LOCAL_ID > 255)
   #error NCV7410_LOCAL_ID parameter is not valid
#endif

//RX buffer size
#ifndef NCV7410_ETH_RX_BUFFER_SIZE
   #define NCV7410_ETH_RX_BUFFER_SIZE 1536
#elif (NCV7410_ETH_RX_BUFFER_SIZE != 1536)
   #error NCV7410_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Chunk payload size
#define NCV7410_CHUNK_PAYLOAD_SIZE 64

//Transmit data header
#define NCV7410_TX_HEADER_DNC    0x80000000
#define NCV7410_TX_HEADER_SEQ    0x40000000
#define NCV7410_TX_HEADER_NORX   0x20000000
#define NCV7410_TX_HEADER_VS     0x00C00000
#define NCV7410_TX_HEADER_DV     0x00200000
#define NCV7410_TX_HEADER_SV     0x00100000
#define NCV7410_TX_HEADER_SWO    0x000F0000
#define NCV7410_TX_HEADER_EV     0x00004000
#define NCV7410_TX_HEADER_EBO    0x00003F00
#define NCV7410_TX_HEADER_TSC    0x000000C0
#define NCV7410_TX_HEADER_P      0x00000001

//Receive data footer
#define NCV7410_RX_FOOTER_EXST   0x80000000
#define NCV7410_RX_FOOTER_HDRB   0x40000000
#define NCV7410_RX_FOOTER_SYNC   0x20000000
#define NCV7410_RX_FOOTER_RCA    0x1F000000
#define NCV7410_RX_FOOTER_VS     0x00C00000
#define NCV7410_RX_FOOTER_DV     0x00200000
#define NCV7410_RX_FOOTER_SV     0x00100000
#define NCV7410_RX_FOOTER_SWO    0x000F0000
#define NCV7410_RX_FOOTER_FD     0x00008000
#define NCV7410_RX_FOOTER_EV     0x00004000
#define NCV7410_RX_FOOTER_EBO    0x00003F00
#define NCV7410_RX_FOOTER_RTSA   0x00000080
#define NCV7410_RX_FOOTER_RTSP   0x00000040
#define NCV7410_RX_FOOTER_TXC    0x0000003E
#define NCV7410_RX_FOOTER_P      0x00000001

//Control command header
#define NCV7410_CTRL_HEADER_DNC  0x80000000
#define NCV7410_CTRL_HEADER_HDRB 0x40000000
#define NCV7410_CTRL_HEADER_WNR  0x20000000
#define NCV7410_CTRL_HEADER_AID  0x10000000
#define NCV7410_CTRL_HEADER_MMS  0x0F000000
#define NCV7410_CTRL_HEADER_ADDR 0x00FFFF00
#define NCV7410_CTRL_HEADER_LEN  0x000000FE
#define NCV7410_CTRL_HEADER_P    0x00000001

//Memory map selectors
#define NCV7410_MMS_STD          0x00
#define NCV7410_MMS_MAC          0x01
#define NCV7410_MMS_PHY_PCS      0x02
#define NCV7410_MMS_PHY_PMA_PMD  0x03
#define NCV7410_MMS_PHY_PLCA     0x04
#define NCV7410_MMS_PHY_VS       0x0C

//NCV7410 Standard Control and Status registers (MMS 0)
#define NCV7410_IDVER                                  0x00, 0x0000
#define NCV7410_PHYID                                  0x00, 0x0001
#define NCV7410_SPICAP                                 0x00, 0x0002
#define NCV7410_RESET                                  0x00, 0x0003
#define NCV7410_CONFIG0                                0x00, 0x0004
#define NCV7410_STATUS0                                0x00, 0x0008
#define NCV7410_BUFSTS                                 0x00, 0x000B
#define NCV7410_IMASK                                  0x00, 0x000C
#define NCV7410_PHYCTRL                                0x00, 0xFF00
#define NCV7410_PHYSTATUS                              0x00, 0xFF01
#define NCV7410_PHYID0                                 0x00, 0xFF02
#define NCV7410_PHYID1                                 0x00, 0xFF03

//NCV7410 MAC registers (MMS 1)
#define NCV7410_MACCTRL0                               0x01, 0x0000
#define NCV7410_ADDRFILT0L                             0x01, 0x0010
#define NCV7410_ADDRFILT0H                             0x01, 0x0011
#define NCV7410_ADDRFILT1L                             0x01, 0x0012
#define NCV7410_ADDRFILT1H                             0x01, 0x0013
#define NCV7410_ADDRFILT2L                             0x01, 0x0014
#define NCV7410_ADDRFILT2H                             0x01, 0x0015
#define NCV7410_ADDRFILT3L                             0x01, 0x0016
#define NCV7410_ADDRFILT3H                             0x01, 0x0017
#define NCV7410_ADDRMASK0L                             0x01, 0x0020
#define NCV7410_ADDRMASK0H                             0x01, 0x0021
#define NCV7410_ADDRMASK1L                             0x01, 0x0022
#define NCV7410_ADDRMASK1H                             0x01, 0x0023
#define NCV7410_ADDRMASK2L                             0x01, 0x0024
#define NCV7410_ADDRMASK2H                             0x01, 0x0025
#define NCV7410_ADDRMASK3L                             0x01, 0x0026
#define NCV7410_ADDRMASK3H                             0x01, 0x0027
#define NCV7410_STOCTETSTXL                            0x01, 0x0030
#define NCV7410_STOCTETSTXH                            0x01, 0x0031
#define NCV7410_STFRAMESTXOK                           0x01, 0x0032
#define NCV7410_STBCASTTXOK                            0x01, 0x0033
#define NCV7410_STMCASTTXOK                            0x01, 0x0034
#define NCV7410_STFRAMESTX64                           0x01, 0x0035
#define NCV7410_STFRAMESTX65                           0x01, 0x0036
#define NCV7410_STFRAMESTX128                          0x01, 0x0037
#define NCV7410_STFRAMESTX256                          0x01, 0x0038
#define NCV7410_STFRAMESTX512                          0x01, 0x0039
#define NCV7410_STFRAMESTX1024                         0x01, 0x003A
#define NCV7410_STUNDERFLOW                            0x01, 0x003B
#define NCV7410_STSINGLECOL                            0x01, 0x003C
#define NCV7410_STMULTICOL                             0x01, 0x003D
#define NCV7410_STEXCESSCOL                            0x01, 0x003E
#define NCV7410_STDEFERREDTX                           0x01, 0x003F
#define NCV7410_STCRSERR                               0x01, 0x0040
#define NCV7410_STOCTETSRXL                            0x01, 0x0041
#define NCV7410_STOCTETSRXH                            0x01, 0x0042
#define NCV7410_STFRAMESRXOK                           0x01, 0x0043
#define NCV7410_STBCASTRXOK                            0x01, 0x0044
#define NCV7410_STMCASTRXOK                            0x01, 0x0045
#define NCV7410_STFRAMESRX64                           0x01, 0x0046
#define NCV7410_STFRAMESRX65                           0x01, 0x0047
#define NCV7410_STFRAMESRX128                          0x01, 0x0048
#define NCV7410_STFRAMESRX256                          0x01, 0x0049
#define NCV7410_STFRAMESRX512                          0x01, 0x004A
#define NCV7410_STFRAMESRX1024                         0x01, 0x004B
#define NCV7410_STRUNTERR                              0x01, 0x004C
#define NCV7410_STRXTOOLONG                            0x01, 0x004D
#define NCV7410_STFCSERRS                              0x01, 0x004E
#define NCV7410_STSYMBOLERRS                           0x01, 0x004F
#define NCV7410_STALIGNERRS                            0x01, 0x0050
#define NCV7410_STRXOVERFLOW                           0x01, 0x0051
#define NCV7410_STRXDROPPED                            0x01, 0x0052

//NCV7410 PHY PCS registers (MMS 2)
#define NCV7410_PCS_DEVINPKG1                          0x02, 0x0005
#define NCV7410_PCS_DEVINPKG2                          0x02, 0x0006
#define NCV7410_T1SPCSCTRL                             0x02, 0x08F3
#define NCV7410_T1SPCSSTATUS                           0x02, 0x08F4
#define NCV7410_T1SPCSDIAG1                            0x02, 0x08F5
#define NCV7410_T1SPCSDIAG2                            0x02, 0x08F6

//NCV7410 PHY PMA/PMD registers (MMS 3)
#define NCV7410_PMA_DEVINPKG1                          0x03, 0x0005
#define NCV7410_PMA_DEVINPKG2                          0x03, 0x0006
#define NCV7410_BASET1EXTABLTY                         0x03, 0x0012
#define NCV7410_T1SPMACTRL                             0x03, 0x08F9
#define NCV7410_T1SPMASTS                              0x03, 0x08FA
#define NCV7410_T1STMCTL                               0x03, 0x08FB

//NCV7410 PHY PLCA registers (MMS 4)
#define NCV7410_CHIPREV                                0x04, 0x8000
#define NCV7410_PHYCFG1                                0x04, 0x8001
#define NCV7410_PLCAEXT                                0x04, 0x8002
#define NCV7410_PMATUNE0                               0x04, 0x8003
#define NCV7410_PMATUNE1                               0x04, 0x8004
#define NCV7410_PLCIDVER                               0x04, 0xCA00
#define NCV7410_PLCACTRL0                              0x04, 0xCA01
#define NCV7410_PLCACTRL1                              0x04, 0xCA02
#define NCV7410_PLCASTATUS                             0x04, 0xCA03
#define NCV7410_PLCATOTMR                              0x04, 0xCA04
#define NCV7410_PLCABURST                              0x04, 0xCA05

//NCV7410 Vendor Specific registers (MMS 12)
#define NCV7410_MIIMIRQCTRL                            0x0C, 0x0010
#define NCV7410_MIIMIRQSTS                             0x0C, 0x0011
#define NCV7410_DIOCFG                                 0x0C, 0x0012
#define NCV7410_TDCTRL                                 0x0C, 0x0016
#define NCV7410_TDSTATUS                               0x0C, 0x0017
#define NCV7410_TDRES                                  0x0C, 0x0018
#define NCV7410_TDPRES                                 0x0C, 0x0019
#define NCV7410_TRCT                                   0x0C, 0x001A
#define NCV7410_PHYCFG0                                0x0C, 0x1001
#define NCV7410_MACID0                                 0x0C, 0x1002
#define NCV7410_MACID1                                 0x0C, 0x1003
#define NCV7410_CHIPINFO                               0x0C, 0x1004
#define NCV7410_NVMHEALTH                              0x0C, 0x1005

//SPI Identification register
#define NCV7410_IDVER_MAJVER                           0x000000F0
#define NCV7410_IDVER_MAJVER_DEFAULT                   0x00000010
#define NCV7410_IDVER_MINVER                           0x0000000F
#define NCV7410_IDVER_MINVER_DEFAULT                   0x00000001

//SPI Identification register
#define NCV7410_PHYID_OUI                              0xFFFFFC00
#define NCV7410_PHYID_OUI_DEFAULT                      0x180FF400
#define NCV7410_PHYID_MODEL                            0x000003F0
#define NCV7410_PHYID_MODEL_DEFAULT                    0x000001A0
#define NCV7410_PHYID_REV                              0x0000000F
#define NCV7410_PHYID_REV_DEFAULT                      0x00000001

//SPI Capabilities register
#define NCV7410_SPICAP_TXFCSVC                         0x00000400
#define NCV7410_SPICAP_IPRAC                           0x00000200
#define NCV7410_SPICAP_DPRAC                           0x00000100
#define NCV7410_SPICAP_CTC                             0x00000080
#define NCV7410_SPICAP_FTC                             0x00000040
#define NCV7410_SPICAP_AIDC                            0x00000020
#define NCV7410_SPICAP_SEQ                             0x00000010
#define NCV7410_SPICAP_MINCPS                          0x00000007

//Reset Control And Status register
#define NCV7410_RESET_RESET                            0x00000001

//SPI Protocol Configuration 0 register
#define NCV7410_CONFIG0_SYNC                           0x00008000
#define NCV7410_CONFIG0_TXFCSVE                        0x00004000
#define NCV7410_CONFIG0_CSARFE                         0x00002000
#define NCV7410_CONFIG0_ZARFE                          0x00001000
#define NCV7410_CONFIG0_TXCTHRESH                      0x00000C00
#define NCV7410_CONFIG0_TXCTHRESH_1_CREDIT             0x00000000
#define NCV7410_CONFIG0_TXCTHRESH_4_CREDITS            0x00000400
#define NCV7410_CONFIG0_TXCTHRESH_8_CREDITS            0x00000800
#define NCV7410_CONFIG0_TXCTHRESH_16_CREDITS           0x00000C00
#define NCV7410_CONFIG0_TXCTE                          0x00000200
#define NCV7410_CONFIG0_RXCTE                          0x00000100
#define NCV7410_CONFIG0_FTSE                           0x00000080
#define NCV7410_CONFIG0_FTSS                           0x00000040
#define NCV7410_CONFIG0_PROTE                          0x00000020
#define NCV7410_CONFIG0_CPS                            0x00000007
#define NCV7410_CONFIG0_CPS_8_BYTES                    0x00000003
#define NCV7410_CONFIG0_CPS_16_BYTES                   0x00000004
#define NCV7410_CONFIG0_CPS_32_BYTES                   0x00000005
#define NCV7410_CONFIG0_CPS_64_BYTES                   0x00000006

//SPI Protocol Status 0 register
#define NCV7410_STATUS0_CDPE                           0x00001000
#define NCV7410_STATUS0_TXFCSE                         0x00000800
#define NCV7410_STATUS0_TTSCAC                         0x00000400
#define NCV7410_STATUS0_TTSCAB                         0x00000200
#define NCV7410_STATUS0_TTSCAA                         0x00000100
#define NCV7410_STATUS0_PHYINT                         0x00000080
#define NCV7410_STATUS0_RESETC                         0x00000040
#define NCV7410_STATUS0_HDRE                           0x00000020
#define NCV7410_STATUS0_LOFE                           0x00000010
#define NCV7410_STATUS0_RXBOE                          0x00000008
#define NCV7410_STATUS0_TXBUE                          0x00000004
#define NCV7410_STATUS0_TXBOE                          0x00000002
#define NCV7410_STATUS0_TXPE                           0x00000001

//Buffer Status register
#define NCV7410_BUFSTS_TXC                             0x0000FF00
#define NCV7410_BUFSTS_RCA                             0x000000FF

//Interrupt Mask register
#define NCV7410_IMASK_CDPEM                            0x00001000
#define NCV7410_IMASK_TXFCSEM                          0x00000800
#define NCV7410_IMASK_PHYINTM                          0x00000080
#define NCV7410_IMASK_RESETCM                          0x00000040
#define NCV7410_IMASK_HDREM                            0x00000020
#define NCV7410_IMASK_LOFEM                            0x00000010
#define NCV7410_IMASK_RXDOEM                           0x00000008
#define NCV7410_IMASK_TXBUEM                           0x00000004
#define NCV7410_IMASK_TXBOEM                           0x00000002
#define NCV7410_IMASK_TXPEM                            0x00000001

//PHY Control register
#define NCV7410_PHYCTRL_RESET                          0x00008000
#define NCV7410_PHYCTRL_LOOP                           0x00004000
#define NCV7410_PHYCTRL_SPD0                           0x00002000
#define NCV7410_PHYCTRL_LCTL                           0x00001000
#define NCV7410_PHYCTRL_ISOM                           0x00000400
#define NCV7410_PHYCTRL_LRST                           0x00000200
#define NCV7410_PHYCTRL_DUPL                           0x00000100
#define NCV7410_PHYCTRL_CTEST                          0x00000080
#define NCV7410_PHYCTRL_SPD1                           0x00000040

//PHY Status register
#define NCV7410_PHYSTATUS_S10M                         0x00000800
#define NCV7410_PHYSTATUS_UNIA                         0x00000080
#define NCV7410_PHYSTATUS_PRSUP                        0x00000040
#define NCV7410_PHYSTATUS_LNOK                         0x00000020
#define NCV7410_PHYSTATUS_RJAB                         0x00000010
#define NCV7410_PHYSTATUS_ANAB                         0x00000008
#define NCV7410_PHYSTATUS_LKST                         0x00000004
#define NCV7410_PHYSTATUS_LJAB                         0x00000002
#define NCV7410_PHYSTATUS_EXTC                         0x00000001

//PHY Identifier 0 register
#define NCV7410_PHYID0_OUI_MSB                         0x0000FFFF
#define NCV7410_PHYID0_OUI_MSB_DEFAULT                 0x0000180F

//PHY Identifier 1 register
#define NCV7410_PHYID1_OUI_LSB                         0x0000FC00
#define NCV7410_PHYID1_OUI_LSB_DEFAULT                 0x0000D400
#define NCV7410_PHYID1_MODEL_NUM                       0x000003F0
#define NCV7410_PHYID1_MODEL_NUM_DEFAULT               0x000001A0
#define NCV7410_PHYID1_REV_NUM                         0x0000000F
#define NCV7410_PHYID1_REV_NUM_DEFAULT                 0x00000001

//MAC Control 0 register
#define NCV7410_MACCTRL0_IPGNF                         0x00200000
#define NCV7410_MACCTRL0_BKOD                          0x00100000
#define NCV7410_MACCTRL0_NFCSF                         0x00080000
#define NCV7410_MACCTRL0_MCSF                          0x00040000
#define NCV7410_MACCTRL0_BCSF                          0x00020000
#define NCV7410_MACCTRL0_ADRF                          0x00010000
#define NCV7410_MACCTRL0_FCSA                          0x00000100
#define NCV7410_MACCTRL0_TXEN                          0x00000002
#define NCV7410_MACCTRL0_RXEN                          0x00000001

//Address Filter Low register
#define NCV7410_ADDRFILTnL_ADDRFILT_31_0               0xFFFFFFFF

//Address Filter High register
#define NCV7410_ADDRFILTnH_EN                          0x80000000
#define NCV7410_ADDRFILTnH_ADDRFILT_47_32              0x0000FFFF

//Address Mask Low register
#define NCV7410_ADDRMASKnL_ADDRMASK_31_0               0xFFFFFFFF

//Address Mask High register
#define NCV7410_ADDRMASKnH_ADDRMASK_47_32              0x00008000

//Statistic Sent Bytes Counter Low register
#define NCV7410_STOCTETSTXL_STOCTETSTXL_31_0           0xFFFFFFFF

//Statistic Sent Bytes Counter High register
#define NCV7410_STOCTETSTXH_STOCTETSTXL_47_32          0x0000FFFF

//Statistic Aborted Frames Due To TX-buffer Underflow register
#define NCV7410_STUNDERFLOW_STUNDERFLOW                0x000003FF

//Statistic Frames Transmitted After Single Collision register
#define NCV7410_STSINGLECOL_STSINGLECOL                0x0003FFFF

//Statistic Frames Transmitted After Multiple Collisions register
#define NCV7410_STMULTICOL_STMULTICOL                  0x0003FFFF

//Statistic Frames Transmitted After Excessive Collisions register
#define NCV7410_STEXCESSCOL_STEXCESSCOL                0x000003FF

//Statistic Frames Transmitted After Deferral register
#define NCV7410_STDEFERREDTX_STDEFERREDTX              0x0003FFFF

//Statistic Counter Of CRS De-assertion During Frame Transmission register
#define NCV7410_STCRSERR_STCRSERR                      0x000003FF

//Statistic Received Bytes Counter Low register
#define NCV7410_STOCTETSRXL_STOCTETSRX_31_0            0xFFFFFFFF

//Statistic Received Bytes Counter High register
#define NCV7410_STOCTETSRXH_STOCTETSRX_47_32           0x0000FFFF

//Statistic Dropped Too Short Frames register
#define NCV7410_STRUNTERR_STRUNTERR                    0x000003FF

//Statistic Dropped Too Long Frames register
#define NCV7410_STRXTOOLONG_STRXTOOLONG                0x000003FF

//Statistic Dropped FCS Error Frames register
#define NCV7410_STFCSERRS_STFCSERRS                    0x000003FF

//Statistic Symbol Errors During Frame Reception register
#define NCV7410_STSYMBOLERRS_STSYMBOLERRS              0x000003FF

//Statistic Align Errors During Frame Reception register
#define NCV7410_STALIGNERRS_STALIGNERRS                0x000003FF

//Statistic RX Buffer Overflow Errors register
#define NCV7410_STRXOVERFLOW_STRXOVERFLOW              0x000003FF

//Devices In Package 1 register
#define NCV7410_PCS_DEVINPKG1_PCS                      0x0008
#define NCV7410_PCS_DEVINPKG1_PMA                      0x0002
#define NCV7410_PCS_DEVINPKG1_CL22                     0x0001

//10BASE-T1S PCS Control register
#define NCV7410_T1SPCSCTRL_PCSRST                      0x8000
#define NCV7410_T1SPCSCTRL_LOOP                        0x4000

//10BASE-T1S PCS Status register
#define NCV7410_T1SPCSSTATUS_JAB                       0x0080

//10BASE-T1S PCS Diagnostics 1 register
#define NCV7410_T1SPCSDIAG1_REMJAB                     0xFFFF

//10BASE-T1S PCS Diagnostics 2 register
#define NCV7410_T1SPCSDIAG2_CTX                        0xFFFF

//Devices In Package 1 register
#define NCV7410_PMA_DEVINPKG1_PCS                      0x0008
#define NCV7410_PMA_DEVINPKG1_PMA                      0x0002
#define NCV7410_PMA_DEVINPKG1_CL22                     0x0001

//BASE-T1 Extended Ability register
#define NCV7410_BASET1EXTABLTY_10T1S                   0x0008

//10BASE-T1S PMA Control register
#define NCV7410_T1SPMACTRL_PMARST                      0x8000
#define NCV7410_T1SPMACTRL_TXDIS                       0x4000
#define NCV7410_T1SPMACTRL_MULT                        0x0400
#define NCV7410_T1SPMACTRL_LOOP                        0x0001

//10BASE-T1S PMA Status register
#define NCV7410_T1SPMASTS_LOOPA                        0x2000
#define NCV7410_T1SPMASTS_LPWRA                        0x0800
#define NCV7410_T1SPMASTS_MULTA                        0x0400
#define NCV7410_T1SPMASTS_RFLTA                        0x0200
#define NCV7410_T1SPMASTS_RJAB                         0x0002

//10BASE-T1S Test Mode Control register
#define NCV7410_T1STMCTL_TEST_MODE                     0xE000
#define NCV7410_T1STMCTL_TEST_MODE_NORMAL              0x0000
#define NCV7410_T1STMCTL_TEST_MODE_TX_OUT_VOLTAGE_TEST 0x2000
#define NCV7410_T1STMCTL_TEST_MODE_TX_OUT_DROP_TEST    0x4000
#define NCV7410_T1STMCTL_TEST_MODE_TX_PSD_MASK         0x6000
#define NCV7410_T1STMCTL_TEST_MODE_TX_HIGH_Z_TEST      0x8000

//Chip Revision register
#define NCV7410_CHIPREV_MAJ                            0xF000
#define NCV7410_CHIPREV_MAJ_DEFAULT                    0x1000
#define NCV7410_CHIPREV_MIN                            0x0F00
#define NCV7410_CHIPREV_MIN_DEFAULT                    0x0000
#define NCV7410_CHIPREV_STAGE                          0x00C0
#define NCV7410_CHIPREV_STAGE_DEFAULT                  0x00C0
#define NCV7410_CHIPREV_BUILD                          0x003F
#define NCV7410_CHIPREV_BUILD_DEFAULT                  0x0006

//PHY Configuration 1 register
#define NCV7410_PHYCFG1_PKTLOOP                        0x8000
#define NCV7410_PHYCFG1_ENIE                           0x0080
#define NCV7410_PHYCFG1_UNJT                           0x0040
#define NCV7410_PHYCFG1_SCRD                           0x0004
#define NCV7410_PHYCFG1_NCOLM                          0x0002
#define NCV7410_PHYCFG1_RXDLY                          0x0001

//PLCA Extensions register
#define NCV7410_PLCAEXT_PREN                           0x8000
#define NCV7410_PLCAEXT_MIIDIS                         0x0800
#define NCV7410_PLCAEXT_LDEN                           0x0002
#define NCV7410_PLCAEXT_LDR                            0x0001

//PMA Tune 0 register
#define NCV7410_PMATUNE0_BDT                           0x3F00
#define NCV7410_PMATUNE0_DCWS                          0x0007

//PMA Tune 1 register
#define NCV7410_PMATUNE1_PPDT                          0x3F00
#define NCV7410_PMATUNE1_CDT                           0x003F

//PLCA Register Map And Identification register
#define NCV7410_PLCIDVER_MAPID                         0xFF00
#define NCV7410_PLCIDVER_MAPVER                        0x00FF

//PLCA Control 0 register
#define NCV7410_PLCACTRL0_EN                           0x8000
#define NCV7410_PLCACTRL0_RST                          0x4000

//PLCA Control 1 register
#define NCV7410_PLCACTRL1_NCNT                         0xFF00
#define NCV7410_PLCACTRL1_ID                           0x00FF

//PLCA Status register
#define NCV7410_PLCASTATUS_PST                         0x8000

//PLCA Transmit Opportunity Timer register
#define NCV7410_PLCATOTMR_TOTMR                        0x00FF
#define NCV7410_PLCATOTMR_TOTMR_DEFAULT                0x0018

//PLCA Burst Mode register
#define NCV7410_PLCABURST_MAXBC                        0xFF00
#define NCV7410_PLCABURST_MAXBC_DEFAULT                0x0000
#define NCV7410_PLCABURST_BTMR                         0x00FF
#define NCV7410_PLCABURST_BTMR_DEFAULT                 0x0080

//MIIM Interrupt Control register
#define NCV7410_MIIMIRQCTRL_MIPCE                      0x0020
#define NCV7410_MIIMIRQCTRL_MIPRE                      0x0010
#define NCV7410_MIIMIRQCTRL_MIRJE                      0x0008
#define NCV7410_MIIMIRQCTRL_MILJE                      0x0004
#define NCV7410_MIIMIRQCTRL_MIPSE                      0x0002
#define NCV7410_MIIMIRQCTRL_MILSE                      0x0001

//MIIM Interrupt Status register
#define NCV7410_MIIMIRQSTS_RSTS                        0x8000
#define NCV7410_MIIMIRQSTS_MIPCL                       0x0020
#define NCV7410_MIIMIRQSTS_MIPRL                       0x0010
#define NCV7410_MIIMIRQSTS_MIRJL                       0x0008
#define NCV7410_MIIMIRQSTS_MILJL                       0x0004
#define NCV7410_MIIMIRQSTS_MIPSL                       0x0002
#define NCV7410_MIIMIRQSTS_MILSL                       0x0001

//DIO Configuration register
#define NCV7410_DIOCFG_SLR1                            0x8000
#define NCV7410_DIOCFG_PEN1                            0x4000
#define NCV7410_DIOCFG_PUD1                            0x2000
#define NCV7410_DIOCFG_PUD1_PULL_UP                    0x0000
#define NCV7410_DIOCFG_PUD1_PULL_DOWN                  0x2000
#define NCV7410_DIOCFG_FN1                             0x1E00
#define NCV7410_DIOCFG_FN1_DISABLE                     0x0000
#define NCV7410_DIOCFG_FN1_GPIO                        0x0200
#define NCV7410_DIOCFG_FN1_SFD_TX                      0x0400
#define NCV7410_DIOCFG_FN1_SFD_RX                      0x0600
#define NCV7410_DIOCFG_FN1_LED_LINK_CTRL               0x0800
#define NCV7410_DIOCFG_FN1_LED_PLCA_STATUS             0x0A00
#define NCV7410_DIOCFG_FN1_LED_TX                      0x0C00
#define NCV7410_DIOCFG_FN1_LED_RX                      0x0E00
#define NCV7410_DIOCFG_FN1_CLK25M                      0x1000
#define NCV7410_DIOCFG_FN1_SFD_RX_TX                   0x1600
#define NCV7410_DIOCFG_FN1_LED_TX_RX                   0x1E00
#define NCV7410_DIOCFG_VAL1                            0x0100
#define NCV7410_DIOCFG_SLR0                            0x0080
#define NCV7410_DIOCFG_PEN0                            0x0040
#define NCV7410_DIOCFG_PUD0                            0x0020
#define NCV7410_DIOCFG_PUD0_PULL_UP                    0x0000
#define NCV7410_DIOCFG_PUD0_PULL_DOWN                  0x0020
#define NCV7410_DIOCFG_FN0                             0x001E
#define NCV7410_DIOCFG_FN0_DISABLE                     0x0000
#define NCV7410_DIOCFG_FN0_GPIO                        0x0002
#define NCV7410_DIOCFG_FN0_SFD_TX                      0x0004
#define NCV7410_DIOCFG_FN0_SFD_RX                      0x0006
#define NCV7410_DIOCFG_FN0_LED_LINK_CTRL               0x0008
#define NCV7410_DIOCFG_FN0_LED_PLCA_STATUS             0x000A
#define NCV7410_DIOCFG_FN0_LED_TX                      0x000C
#define NCV7410_DIOCFG_FN0_LED_RX                      0x000E
#define NCV7410_DIOCFG_FN0_CLK25M                      0x0010
#define NCV7410_DIOCFG_FN0_SFD_RX_TX                   0x0016
#define NCV7410_DIOCFG_FN0_LED_TX_RX                   0x001E
#define NCV7410_DIOCFG_VAL0                            0x0001

//Topology Discovery Control register
#define NCV7410_TDCTRL_TD_ED                           0x8000
#define NCV7410_TDCTRL_CALM                            0x0400
#define NCV7410_TDCTRL_MANM                            0x0200
#define NCV7410_TDCTRL_STRT                            0x0100
#define NCV7410_TDCTRL_SCRDIR                          0x0010
#define NCV7410_TDCTRL_REFN                            0x0001

//Topology Discovery Status register
#define NCV7410_TDSTATUS_DONE                          0x8000
#define NCV7410_TDSTATUS_HNDE                          0x0004
#define NCV7410_TDSTATUS_MEAE                          0x0002
#define NCV7410_TDSTATUS_CALE                          0x0001

//Topology Discovery Result register
#define NCV7410_TDRES_CNTV                             0xFFFF

//Topology Discovery Precision register
#define NCV7410_TDPRES_RTMP                            0xFFFF

//Topology Reference Counter Timer register
#define NCV7410_TRCT_RCNT                              0xFFFF

//PHY Configuration 0 register
#define NCV7410_PHYCFG0_TX_GAIN                        0xC000
#define NCV7410_PHYCFG0_TX_GAIN_1000_MVPP              0x0000
#define NCV7410_PHYCFG0_TX_GAIN_1100_MVPP              0x4000
#define NCV7410_PHYCFG0_TX_GAIN_900_MVPP               0x8000
#define NCV7410_PHYCFG0_TX_GAIN_800_MVPP               0xC000
#define NCV7410_PHYCFG0_RX_CD                          0x3C00
#define NCV7410_PHYCFG0_RX_CD_150_MVPP                 0x0000
#define NCV7410_PHYCFG0_RX_CD_200_MVPP                 0x0400
#define NCV7410_PHYCFG0_RX_CD_250_MVPP                 0x0800
#define NCV7410_PHYCFG0_RX_CD_300_MVPP                 0x0C00
#define NCV7410_PHYCFG0_RX_CD_350_MVPP                 0x1000
#define NCV7410_PHYCFG0_RX_CD_400_MVPP                 0x1400
#define NCV7410_PHYCFG0_RX_CD_450_MVPP                 0x1800
#define NCV7410_PHYCFG0_RX_CD_500_MVPP                 0x1C00
#define NCV7410_PHYCFG0_RX_CD_550_MVPP                 0x2000
#define NCV7410_PHYCFG0_RX_CD_600_MVPP                 0x2400
#define NCV7410_PHYCFG0_RX_CD_650_MVPP                 0x2800
#define NCV7410_PHYCFG0_RX_CD_700_MVPP                 0x2C00
#define NCV7410_PHYCFG0_RX_CD_750_MVPP                 0x3000
#define NCV7410_PHYCFG0_RX_CD_800_MVPP                 0x3400
#define NCV7410_PHYCFG0_RX_CD_850_MVPP                 0x3800
#define NCV7410_PHYCFG0_RX_CD_900_MVPP                 0x3C00
#define NCV7410_PHYCFG0_RX_ED                          0x03C0
#define NCV7410_PHYCFG0_RX_ED_150_MVPP                 0x0000
#define NCV7410_PHYCFG0_RX_ED_200_MVPP                 0x0040
#define NCV7410_PHYCFG0_RX_ED_250_MVPP                 0x0080
#define NCV7410_PHYCFG0_RX_ED_300_MVPP                 0x00C0
#define NCV7410_PHYCFG0_RX_ED_350_MVPP                 0x0100
#define NCV7410_PHYCFG0_RX_ED_400_MVPP                 0x0140
#define NCV7410_PHYCFG0_RX_ED_450_MVPP                 0x0180
#define NCV7410_PHYCFG0_RX_ED_500_MVPP                 0x01C0
#define NCV7410_PHYCFG0_RX_ED_550_MVPP                 0x0200
#define NCV7410_PHYCFG0_RX_ED_600_MVPP                 0x0240
#define NCV7410_PHYCFG0_RX_ED_650_MVPP                 0x0280
#define NCV7410_PHYCFG0_RX_ED_700_MVPP                 0x02C0
#define NCV7410_PHYCFG0_RX_ED_750_MVPP                 0x0300
#define NCV7410_PHYCFG0_RX_ED_800_MVPP                 0x0340
#define NCV7410_PHYCFG0_RX_ED_850_MVPP                 0x0380
#define NCV7410_PHYCFG0_RX_ED_900_MVPP                 0x03C0
#define NCV7410_PHYCFG0_DSLEW                          0x0020
#define NCV7410_PHYCFG0_DSLEW_SLOW                     0x0000
#define NCV7410_PHYCFG0_DSLEW_FAST                     0x0020
#define NCV7410_PHYCFG0_CMC                            0x0018
#define NCV7410_PHYCFG0_CMC_0_TO_0_5_R                 0x0000
#define NCV7410_PHYCFG0_CMC_0_5_TO_2_25_R              0x0008
#define NCV7410_PHYCFG0_CMC_2_25_TO_3_75_R             0x0010
#define NCV7410_PHYCFG0_CMC_3_75_TO_5_R                0x0018
#define NCV7410_PHYCFG0_TXSLEW                         0x0004
#define NCV7410_PHYCFG0_TXSLEW_SLOW                    0x0000
#define NCV7410_PHYCFG0_TXSLEW_FAST                    0x0004
#define NCV7410_PHYCFG0_CLKO_EN                        0x0001

//MAC Identification 0 register
#define NCV7410_MACID0_MACID_15_0                      0xFFFF

//MAC Identification 1 register
#define NCV7410_MACID1_MACID_23_16                     0x00FF

//Chip Info register
#define NCV7410_CHIPINFO_WAFER_Y                       0x7F00
#define NCV7410_CHIPINFO_WAFER_X                       0x007F

//NVM Health register
#define NCV7410_NVMHEALTH_RED_ZONE_NVM_WARNING         0x8000
#define NCV7410_NVMHEALTH_RED_ZONE_NVM_ERROR           0x4000
#define NCV7410_NVMHEALTH_YELLOW_ZONE_NVM_WARNING      0x2000
#define NCV7410_NVMHEALTH_YELLOW_ZONE_NVM_ERROR        0x1000
#define NCV7410_NVMHEALTH_GREEN_ZONE_NVM_WARNING       0x0800
#define NCV7410_NVMHEALTH_GREEN_ZONE_NVM_ERROR         0x0400

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//NCV7410 driver
extern const NicDriver ncv7410Driver;

//NCV7410 related functions
error_t ncv7410Init(NetInterface *interface);
void ncv7410InitHook(NetInterface *interface);

void ncv7410Tick(NetInterface *interface);

void ncv7410EnableIrq(NetInterface *interface);
void ncv7410DisableIrq(NetInterface *interface);
bool_t ncv7410IrqHandler(NetInterface *interface);
void ncv7410EventHandler(NetInterface *interface);

error_t ncv7410SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t ncv7410ReceivePacket(NetInterface *interface);

error_t ncv7410UpdateMacAddrFilter(NetInterface *interface);

void ncv7410WriteReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint32_t data);

uint32_t ncv7410ReadReg(NetInterface *interface, uint8_t mms,
   uint16_t address);

void ncv7410DumpReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint_t num);

uint32_t ncv7410CalcParity(uint32_t data);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
