/**
 * @file apm32f4xx_eth_driver.h
 * @brief APM32F4 Ethernet MAC driver
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

#ifndef _APM32F4XX_ETH_DRIVER_H
#define _APM32F4XX_ETH_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef APM32F4XX_ETH_TX_BUFFER_COUNT
   #define APM32F4XX_ETH_TX_BUFFER_COUNT 3
#elif (APM32F4XX_ETH_TX_BUFFER_COUNT < 1)
   #error APM32F4XX_ETH_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef APM32F4XX_ETH_TX_BUFFER_SIZE
   #define APM32F4XX_ETH_TX_BUFFER_SIZE 1536
#elif (APM32F4XX_ETH_TX_BUFFER_SIZE != 1536)
   #error APM32F4XX_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef APM32F4XX_ETH_RX_BUFFER_COUNT
   #define APM32F4XX_ETH_RX_BUFFER_COUNT 6
#elif (APM32F4XX_ETH_RX_BUFFER_COUNT < 1)
   #error APM32F4XX_ETH_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef APM32F4XX_ETH_RX_BUFFER_SIZE
   #define APM32F4XX_ETH_RX_BUFFER_SIZE 1536
#elif (APM32F4XX_ETH_RX_BUFFER_SIZE != 1536)
   #error APM32F4XX_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Interrupt priority grouping
#ifndef APM32F4XX_ETH_IRQ_PRIORITY_GROUPING
   #define APM32F4XX_ETH_IRQ_PRIORITY_GROUPING 3
#elif (APM32F4XX_ETH_IRQ_PRIORITY_GROUPING < 0)
   #error APM32F4XX_ETH_IRQ_PRIORITY_GROUPING parameter is not valid
#endif

//Ethernet interrupt group priority
#ifndef APM32F4XX_ETH_IRQ_GROUP_PRIORITY
   #define APM32F4XX_ETH_IRQ_GROUP_PRIORITY 12
#elif (APM32F4XX_ETH_IRQ_GROUP_PRIORITY < 0)
   #error APM32F4XX_ETH_IRQ_GROUP_PRIORITY parameter is not valid
#endif

//Ethernet interrupt subpriority
#ifndef APM32F4XX_ETH_IRQ_SUB_PRIORITY
   #define APM32F4XX_ETH_IRQ_SUB_PRIORITY 0
#elif (APM32F4XX_ETH_IRQ_SUB_PRIORITY < 0)
   #error APM32F4XX_ETH_IRQ_SUB_PRIORITY parameter is not valid
#endif

//MAC Configuration register
#define ETH_CFG_CST               0x02000000
#define ETH_CFG_WDTDIS            0x00800000
#define ETH_CFG_JDIS              0x00400000
#define ETH_CFG_IFG               0x000E0000
#define ETH_CFG_DISCRS            0x00010000
#define ETH_CFG_RESERVED15        0x00008000
#define ETH_CFG_SSEL              0x00004000
#define ETH_CFG_DISRXO            0x00002000
#define ETH_CFG_LBM               0x00001000
#define ETH_CFG_DM                0x00000800
#define ETH_CFG_IPC               0x00000400
#define ETH_CFG_DISR              0x00000200
#define ETH_CFG_ACS               0x00000080
#define ETH_CFG_BL                0x00000060
#define ETH_CFG_DC                0x00000010
#define ETH_CFG_TXEN              0x00000008
#define ETH_CFG_RXEN              0x00000004

//MAC Frame Filter register
#define ETH_FRAF_RXA              0x80000000
#define ETH_FRAF_HPF              0x00000400
#define ETH_FRAF_SAFEN            0x00000200
#define ETH_FRAF_SAIF             0x00000100
#define ETH_FRAF_PCTRLF           0x000000C0
#define ETH_FRAF_DISBF            0x00000020
#define ETH_FRAF_PM               0x00000010
#define ETH_FRAF_DAIF             0x00000008
#define ETH_FRAF_HMC              0x00000004
#define ETH_FRAF_HUC              0x00000002
#define ETH_FRAF_PR               0x00000001

//MII Address register
#define ETH_ADDR_PA               0x0000F800
#define ETH_ADDR_MR               0x000007C0
#define ETH_ADDR_CR               0x0000003C
#define ETH_ADDR_CR_DIV_42        0x00000000
#define ETH_ADDR_CR_DIV_62        0x00000004
#define ETH_ADDR_CR_DIV_16        0x00000008
#define ETH_ADDR_CR_DIV_26        0x0000000C
#define ETH_ADDR_CR_DIV_102       0x00000010
#define ETH_ADDR_MW               0x00000002
#define ETH_ADDR_MB               0x00000001

//MII Data register
#define ETH_DATA_MD               0x0000FFFF

//MAC Interrupt Mask register
#define ETH_IMASK_TSTIM           0x00000200
#define ETH_IMASK_PMTIM           0x00000008

//MAC Address 0 High register
#define ETH_ADDR0H_AL1            0x80000000
#define ETH_ADDR0H_ADDR0H         0x0000FFFF

//MAC Address 1 High register
#define ETH_ADDR1H_ADDREN         0x80000000
#define ETH_ADDR1H_ADDRSEL        0x40000000
#define ETH_ADDR1H_MASKBCTRL      0x3F000000
#define ETH_ADDR1H_ADDR1H         0x0000FFFF

//MAC Address 2 High register
#define ETH_ADDR2H_ADDREN         0x80000000
#define ETH_ADDR2H_ADDRSEL        0x40000000
#define ETH_ADDR2H_MASKBCTRL      0x3F000000
#define ETH_ADDR2H_ADDR2H         0x0000FFFF

//MAC Address 3 High register
#define ETH_ADDR3H_ADDREN         0x80000000
#define ETH_ADDR3H_ADDRSEL        0x40000000
#define ETH_ADDR3H_MASKBCTRL      0x3F000000
#define ETH_ADDR3H_ADDR3H         0x0000FFFF

//MMC Receive Interrupt register
#define ETH_RXINT_RXGUNF          0x00020000
#define ETH_RXINT_RXFAE           0x00000040
#define ETH_RXINT_RXFCE           0x00000020

//MMC Transmit Interrupt register
#define ETH_TXINT_TXGF            0x00200000
#define ETH_TXINT_TXGFMCOL        0x00008000
#define ETH_TXINT_TXGFSCOL        0x00004000

//DMA Bus Mode register
#define ETH_DMABMOD_MB            0x04000000
#define ETH_DMABMOD_AAL           0x02000000
#define ETH_DMABMOD_PBLX4         0x01000000
#define ETH_DMABMOD_USP           0x00800000
#define ETH_DMABMOD_RPBL          0x007E0000
#define ETH_DMABMOD_RPBL_1        0x00020000
#define ETH_DMABMOD_RPBL_2        0x00040000
#define ETH_DMABMOD_RPBL_4        0x00080000
#define ETH_DMABMOD_RPBL_8        0x00100000
#define ETH_DMABMOD_RPBL_16       0x00200000
#define ETH_DMABMOD_RPBL_32       0x00400000
#define ETH_DMABMOD_FB            0x00010000
#define ETH_DMABMOD_PR            0x0000C000
#define ETH_DMABMOD_PR_1_1        0x00000000
#define ETH_DMABMOD_PR_2_1        0x00004000
#define ETH_DMABMOD_PR_3_1        0x00008000
#define ETH_DMABMOD_PR_4_1        0x0000C000
#define ETH_DMABMOD_PBL           0x00003F00
#define ETH_DMABMOD_PBL_1         0x00000100
#define ETH_DMABMOD_PBL_2         0x00000200
#define ETH_DMABMOD_PBL_4         0x00000400
#define ETH_DMABMOD_PBL_8         0x00000800
#define ETH_DMABMOD_PBL_16        0x00001000
#define ETH_DMABMOD_PBL_32        0x00002000
#define ETH_DMABMOD_EDFEN         0x00000080
#define ETH_DMABMOD_DSL           0x0000007C
#define ETH_DMABMOD_DSL_0         0x00000000
#define ETH_DMABMOD_DSL_1         0x00000004
#define ETH_DMABMOD_DSL_2         0x00000008
#define ETH_DMABMOD_DSL_4         0x00000010
#define ETH_DMABMOD_DSL_8         0x00000020
#define ETH_DMABMOD_DSL_16        0x00000040
#define ETH_DMABMOD_DAS           0x00000002
#define ETH_DMABMOD_SWR           0x00000001

//DMA Status register
#define ETH_DMASTS_TSTFLG         0x20000000
#define ETH_DMASTS_PMTFLG         0x10000000
#define ETH_DMASTS_MMCFLG         0x08000000
#define ETH_DMASTS_ERRB           0x03800000
#define ETH_DMASTS_TXSTS          0x00700000
#define ETH_DMASTS_RXSTS          0x000E0000
#define ETH_DMASTS_NINTS          0x00010000
#define ETH_DMASTS_AINTS          0x00008000
#define ETH_DMASTS_ERXFLG         0x00004000
#define ETH_DMASTS_FBERRFLG       0x00002000
#define ETH_DMASTS_ETXFLG         0x00000400
#define ETH_DMASTS_RXWTOFLG       0x00000200
#define ETH_DMASTS_RXSFLG         0x00000100
#define ETH_DMASTS_RXBU           0x00000080
#define ETH_DMASTS_RXFLG          0x00000040
#define ETH_DMASTS_TXUNF          0x00000020
#define ETH_DMASTS_RXOVF          0x00000010
#define ETH_DMASTS_TXJTO          0x00000008
#define ETH_DMASTS_TXBU           0x00000004
#define ETH_DMASTS_TXSFLG         0x00000002
#define ETH_DMASTS_TXFLG          0x00000001

//DMA Operation Mode register
#define ETH_DMAOPMOD_DISDT        0x04000000
#define ETH_DMAOPMOD_RXSF         0x02000000
#define ETH_DMAOPMOD_DISFRXF      0x01000000
#define ETH_DMAOPMOD_TXSF         0x00200000
#define ETH_DMAOPMOD_FTXF         0x00100000
#define ETH_DMAOPMOD_TXTHCTRL     0x0001C000
#define ETH_DMAOPMOD_STTX         0x00002000
#define ETH_DMAOPMOD_FERRF        0x00000080
#define ETH_DMAOPMOD_FUF          0x00000040
#define ETH_DMAOPMOD_RXTHCTRL     0x00000018
#define ETH_DMAOPMOD_OSECF        0x00000004
#define ETH_DMAOPMOD_STRX         0x00000002

//DMA Interrupt Enable register
#define ETH_DMAINTEN_NINTSEN      0x00010000
#define ETH_DMAINTEN_AINTSEN      0x00008000
#define ETH_DMAINTEN_ERXIEN       0x00004000
#define ETH_DMAINTEN_FBERREN      0x00002000
#define ETH_DMAINTEN_ETXIEN       0x00000400
#define ETH_DMAINTEN_RXWTOEN      0x00000200
#define ETH_DMAINTEN_RXSEN        0x00000100
#define ETH_DMAINTEN_RXBUEN       0x00000080
#define ETH_DMAINTEN_RXIEN        0x00000040
#define ETH_DMAINTEN_TXUNFEN      0x00000020
#define ETH_DMAINTEN_RXOVFEN      0x00000010
#define ETH_DMAINTEN_TXJTOEN      0x00000008
#define ETH_DMAINTEN_TXBUEN       0x00000004
#define ETH_DMAINTEN_TXSEN        0x00000002
#define ETH_DMAINTEN_TXIEN        0x00000001

//Transmit DMA descriptor flags
#define ETH_TXDES0_OWN            0x80000000
#define ETH_TXDES0_INTC           0x40000000
#define ETH_TXDES0_LS             0x20000000
#define ETH_TXDES0_FS             0x10000000
#define ETH_TXDES0_DISC           0x08000000
#define ETH_TXDES0_DISP           0x04000000
#define ETH_TXDES0_TXTSEN         0x02000000
#define ETH_TXDES0_CHINS          0x00C00000
#define ETH_TXDES0_TXENDR         0x00200000
#define ETH_TXDES0_TXCH           0x00100000
#define ETH_TXDES0_TXTSS          0x00020000
#define ETH_TXDES0_IHERR          0x00010000
#define ETH_TXDES0_ERRS           0x00008000
#define ETH_TXDES0_JTO            0x00004000
#define ETH_TXDES0_FF             0x00002000
#define ETH_TXDES0_IPERR          0x00001000
#define ETH_TXDES0_LSC            0x00000800
#define ETH_TXDES0_NC             0x00000400
#define ETH_TXDES0_LC             0x00000200
#define ETH_TXDES0_EC             0x00000100
#define ETH_TXDES0_VLANF          0x00000080
#define ETH_TXDES0_CCNT           0x00000078
#define ETH_TXDES0_EDEF           0x00000004
#define ETH_TXDES0_UFERR          0x00000002
#define ETH_TXDES0_DEF            0x00000001
#define ETH_TXDES1_TXBS2          0x1FFF0000
#define ETH_TXDES1_TXBS1          0x00001FFF
#define ETH_TXDES2_TXADDR1_TXFTSL 0xFFFFFFFF
#define ETH_TXDES3_TXADDR2_TXFTSH 0xFFFFFFFF
#define ETH_TXDES6_TXFTSL         0xFFFFFFFF
#define ETH_TXDES7_TXFTSH         0xFFFFFFFF

//Receive DMA descriptor flags
#define ETH_RXDES0_OWN            0x80000000
#define ETH_RXDES0_ADDRF          0x40000000
#define ETH_RXDES0_FL             0x3FFF0000
#define ETH_RXDES0_ERRS           0x00008000
#define ETH_RXDES0_DESERR         0x00004000
#define ETH_RXDES0_SADDRF         0x00002000
#define ETH_RXDES0_LERR           0x00001000
#define ETH_RXDES0_OFERR          0x00000800
#define ETH_RXDES0_VLANF          0x00000400
#define ETH_RXDES0_FDES           0x00000200
#define ETH_RXDES0_LDES           0x00000100
#define ETH_RXDES0_IPCERR_TSV     0x00000080
#define ETH_RXDES0_LC             0x00000040
#define ETH_RXDES0_FT             0x00000020
#define ETH_RXDES0_RXWDTTO        0x00000010
#define ETH_RXDES0_RERR           0x00000008
#define ETH_RXDES0_DERR           0x00000004
#define ETH_RXDES0_CERR           0x00000002
#define ETH_RXDES0_PERR_ESA       0x00000001
#define ETH_RXDES1_DINTC          0x80000000
#define ETH_RXDES1_RBS2           0x1FFF0000
#define ETH_RXDES1_RXER           0x00008000
#define ETH_RXDES1_RXCH           0x00004000
#define ETH_RXDES1_RBS1           0x00001FFF
#define ETH_RXDES2_RXADDR1_RXFTSL 0xFFFFFFFF
#define ETH_RXDES3_RXADDR2_RXFTSH 0xFFFFFFFF
#define ETH_RXDES4_PTPV           0x00002000
#define ETH_RXDES4_PTPFT          0x00001000
#define ETH_RXDES4_PTPMT          0x00000F00
#define ETH_RXDES4_IPV6P          0x00000080
#define ETH_RXDES4_IPV4P          0x00000040
#define ETH_RXDES4_IPCBP          0x00000020
#define ETH_RXDES4_IPPERR         0x00000010
#define ETH_RXDES4_IPHERR         0x00000008
#define ETH_RXDES4_IPPT           0x00000007
#define ETH_RXDES6_RXFTSL         0xFFFFFFFF
#define ETH_RXDES7_RXFTSH         0xFFFFFFFF

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
} Apm32f4xxTxDmaDesc;


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
} Apm32f4xxRxDmaDesc;


//APM32F4 Ethernet MAC driver
extern const NicDriver apm32f4xxEthDriver;

//APM32F4 Ethernet MAC related functions
error_t apm32f4xxEthInit(NetInterface *interface);
void apm32f4xxEthInitGpio(NetInterface *interface);
void apm32f4xxEthInitDmaDesc(NetInterface *interface);

void apm32f4xxEthTick(NetInterface *interface);

void apm32f4xxEthEnableIrq(NetInterface *interface);
void apm32f4xxEthDisableIrq(NetInterface *interface);
void apm32f4xxEthEventHandler(NetInterface *interface);

error_t apm32f4xxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t apm32f4xxEthReceivePacket(NetInterface *interface);

error_t apm32f4xxEthUpdateMacAddrFilter(NetInterface *interface);
error_t apm32f4xxEthUpdateMacConfig(NetInterface *interface);

void apm32f4xxEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t apm32f4xxEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

uint32_t apm32f4xxEthCalcCrc(const void *data, size_t length);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
