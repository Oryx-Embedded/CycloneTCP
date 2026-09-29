/**
 * @file pic32cx_eth_driver.h
 * @brief PIC32CX SG41/SG60/SG61 Ethernet MAC driver
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2026 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.6.6
 **/

#ifndef _PIC32CX_ETH_DRIVER_H
#define _PIC32CX_ETH_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef PIC32CX_ETH_TX_BUFFER_COUNT
   #define PIC32CX_ETH_TX_BUFFER_COUNT 3
#elif (PIC32CX_ETH_TX_BUFFER_COUNT < 1)
   #error PIC32CX_ETH_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef PIC32CX_ETH_TX_BUFFER_SIZE
   #define PIC32CX_ETH_TX_BUFFER_SIZE 1536
#elif (PIC32CX_ETH_TX_BUFFER_SIZE != 1536)
   #error PIC32CX_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef PIC32CX_ETH_RX_BUFFER_COUNT
   #define PIC32CX_ETH_RX_BUFFER_COUNT 72
#elif (PIC32CX_ETH_RX_BUFFER_COUNT < 12)
   #error PIC32CX_ETH_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef PIC32CX_ETH_RX_BUFFER_SIZE
   #define PIC32CX_ETH_RX_BUFFER_SIZE 128
#elif (PIC32CX_ETH_RX_BUFFER_SIZE != 128)
   #error PIC32CX_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Interrupt priority grouping
#ifndef PIC32CX_ETH_IRQ_PRIORITY_GROUPING
   #define PIC32CX_ETH_IRQ_PRIORITY_GROUPING 4
#elif (PIC32CX_ETH_IRQ_PRIORITY_GROUPING < 0)
   #error PIC32CX_ETH_IRQ_PRIORITY_GROUPING parameter is not valid
#endif

//Ethernet interrupt group priority
#ifndef PIC32CX_ETH_IRQ_GROUP_PRIORITY
   #define PIC32CX_ETH_IRQ_GROUP_PRIORITY 6
#elif (PIC32CX_ETH_IRQ_GROUP_PRIORITY < 0)
   #error PIC32CX_ETH_IRQ_GROUP_PRIORITY parameter is not valid
#endif

//Ethernet interrupt subpriority
#ifndef PIC32CX_ETH_IRQ_SUB_PRIORITY
   #define PIC32CX_ETH_IRQ_SUB_PRIORITY 0
#elif (PIC32CX_ETH_IRQ_SUB_PRIORITY < 0)
   #error PIC32CX_ETH_IRQ_SUB_PRIORITY parameter is not valid
#endif

//TX buffer descriptor flags
#define GMAC_TX_USED           0x80000000
#define GMAC_TX_WRAP           0x40000000
#define GMAC_TX_RLE_ERROR      0x20000000
#define GMAC_TX_UNDERRUN_ERROR 0x10000000
#define GMAC_TX_AHB_ERROR      0x08000000
#define GMAC_TX_LATE_COL_ERROR 0x04000000
#define GMAC_TX_CHECKSUM_ERROR 0x00700000
#define GMAC_TX_NO_CRC         0x00010000
#define GMAC_TX_LAST           0x00008000
#define GMAC_TX_LENGTH         0x00003FFF

//RX buffer descriptor flags
#define GMAC_RX_ADDRESS        0xFFFFFFFC
#define GMAC_RX_WRAP           0x00000002
#define GMAC_RX_OWNERSHIP      0x00000001
#define GMAC_RX_BROADCAST      0x80000000
#define GMAC_RX_MULTICAST_HASH 0x40000000
#define GMAC_RX_UNICAST_HASH   0x20000000
#define GMAC_RX_SAR            0x08000000
#define GMAC_RX_SAR_MASK       0x06000000
#define GMAC_RX_TYPE_ID        0x01000000
#define GMAC_RX_SNAP           0x01000000
#define GMAC_RX_TYPE_ID_MASK   0x00C00000
#define GMAC_RX_CHECKSUM_VALID 0x00C00000
#define GMAC_RX_VLAN_TAG       0x00200000
#define GMAC_RX_PRIORITY_TAG   0x00100000
#define GMAC_RX_VLAN_PRIORITY  0x000E0000
#define GMAC_RX_CFI            0x00010000
#define GMAC_RX_EOF            0x00008000
#define GMAC_RX_SOF            0x00004000
#define GMAC_RX_LENGTH_MSB     0x00002000
#define GMAC_RX_BAD_FCS        0x00002000
#define GMAC_RX_LENGTH         0x00001FFF

//Processor-specific definitions
#if defined(__PIC32CX2051BZ62132__) || defined(__PIC32CX2051BZ62132__)
   //GMAC interrupt
   #define GMAC_IRQn             ETH_IRQn
   #define GMAC_Handler          ETH_Handler

   //GMAC peripheral base address
   #define GMAC_REGS             ETH_REGS

   //GMAC registers
   #define GMAC_NCR              ETH_NCR
   #define GMAC_NCFGR            ETH_NCFGR
   #define GMAC_NSR              ETH_NSR
   #define GMAC_TSR              ETH_TSR
   #define GMAC_RBQB             ETH_RBQB
   #define GMAC_TBQB             ETH_TBQB
   #define GMAC_RSR              ETH_RSR
   #define GMAC_ISR              ETH_ISR
   #define GMAC_IER              ETH_IER
   #define GMAC_IDR              ETH_IDR
   #define GMAC_MAN              ETH_MAN
   #define GMAC_HRB              ETH_HRB
   #define GMAC_HRT              ETH_HRT
   #define GMAC_SAB              ETH_SAB
   #define GMAC_SAT              ETH_SAT

   //GMAC_NCR register
   #define GMAC_NCR_TSTART_Msk   ETH_NCR_TSTART_Msk
   #define GMAC_NCR_MPE_Msk      ETH_NCR_MPE_Msk
   #define GMAC_NCR_TXEN_Msk     ETH_NCR_TXEN_Msk
   #define GMAC_NCR_RXEN_Msk     ETH_NCR_RXEN_Msk

   //GMAC_NCFGR register
   #define GMAC_NCFGR_DBW        ETH_NCFGR_DBW
   #define GMAC_NCFGR_CLK        ETH_NCFGR_CLK
   #define GMAC_NCFGR_GIGE_Msk   ETH_NCFGR_GIGE_Msk
   #define GMAC_NCFGR_MAXFS_Msk  ETH_NCFGR_MAXFS_Msk
   #define GMAC_NCFGR_UNIHEN_Msk ETH_NCFGR_UNIHEN_Msk
   #define GMAC_NCFGR_MTIHEN_Msk ETH_NCFGR_MTIHEN_Msk
   #define GMAC_NCFGR_FD_Msk     ETH_NCFGR_FD_Msk
   #define GMAC_NCFGR_SPD_Msk    ETH_NCFGR_SPD_Msk

   //GMAC_NSR register
   #define GMAC_NSR_IDLE_Msk     ETH_NSR_IDLE_Msk

   //GMAC_TSR register
   #define GMAC_TSR_HRESP_Msk    ETH_TSR_HRESP_Msk
   #define GMAC_TSR_UND_Msk      ETH_TSR_UND_Msk
   #define GMAC_TSR_TXCOMP_Msk   ETH_TSR_TXCOMP_Msk
   #define GMAC_TSR_TFC_Msk      ETH_TSR_TFC_Msk
   #define GMAC_TSR_TXGO_Msk     ETH_TSR_TXGO_Msk
   #define GMAC_TSR_RLE_Msk      ETH_TSR_RLE_Msk
   #define GMAC_TSR_COL_Msk      ETH_TSR_COL_Msk
   #define GMAC_TSR_UBR_Msk      ETH_TSR_UBR_Msk

   //GMAC_RSR register
   #define GMAC_RSR_HNO_Msk      ETH_RSR_HNO_Msk
   #define GMAC_RSR_RXOVR_Msk    ETH_RSR_RXOVR_Msk
   #define GMAC_RSR_REC_Msk      ETH_RSR_REC_Msk
   #define GMAC_RSR_BNA_Msk      ETH_RSR_BNA_Msk

   //GMAC_IER register
   #define GMAC_IER_HRESP_Msk    ETH_IER_HRESP_Msk
   #define GMAC_IER_ROVR_Msk     ETH_IER_ROVR_Msk
   #define GMAC_IER_TCOMP_Msk    ETH_IER_TCOMP_Msk
   #define GMAC_IER_TFC_Msk      ETH_IER_TFC_Msk
   #define GMAC_IER_RLEX_Msk     ETH_IER_RLEX_Msk
   #define GMAC_IER_TUR_Msk      ETH_IER_TUR_Msk
   #define GMAC_IER_RXUBR_Msk    ETH_IER_RXUBR_Msk
   #define GMAC_IER_RCOMP_Msk    ETH_IER_RCOMP_Msk

   //GMAC_MAN register
   #define GMAC_MAN_CLTTO_Msk    ETH_MAN_CLTTO_Msk
   #define GMAC_MAN_OP           ETH_MAN_OP
   #define GMAC_MAN_PHYA         ETH_MAN_PHYA
   #define GMAC_MAN_REGA         ETH_MAN_REGA
   #define GMAC_MAN_WTN          ETH_MAN_WTN
   #define GMAC_MAN_DATA         ETH_MAN_DATA
   #define GMAC_MAN_DATA_Msk     ETH_MAN_DATA_Msk
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Transmit buffer descriptor
 **/

typedef struct
{
   uint32_t address;
   uint32_t status;
} Pic32cxTxBufferDesc;


/**
 * @brief Receive buffer descriptor
 **/

typedef struct
{
   uint32_t address;
   uint32_t status;
} Pic32cxRxBufferDesc;


//PIC32CX Ethernet MAC driver
extern const NicDriver pic32cxEthDriver;

//PIC32CX Ethernet MAC related functions
error_t pic32cxEthInit(NetInterface *interface);
void pic32cxEthInitGpio(NetInterface *interface);
void pic32cxEthInitBufferDesc(NetInterface *interface);

void pic32cxEthTick(NetInterface *interface);

void pic32cxEthEnableIrq(NetInterface *interface);
void pic32cxEthDisableIrq(NetInterface *interface);
void pic32cxEthEventHandler(NetInterface *interface);

error_t pic32cxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t pic32cxEthReceivePacket(NetInterface *interface);

error_t pic32cxEthUpdateMacAddrFilter(NetInterface *interface);
error_t pic32cxEthUpdateMacConfig(NetInterface *interface);

void pic32cxEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t pic32cxEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
