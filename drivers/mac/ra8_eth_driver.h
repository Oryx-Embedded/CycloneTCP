/**
 * @file ra8_eth_driver.h
 * @brief Renesas RA8M1 / RA8D1 / RA8T1 Ethernet MAC driver
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

#ifndef _RA8_ETH_DRIVER_H
#define _RA8_ETH_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef RA8_ETH_TX_BUFFER_COUNT
   #define RA8_ETH_TX_BUFFER_COUNT 3
#elif (RA8_ETH_TX_BUFFER_COUNT < 1)
   #error RA8_ETH_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef RA8_ETH_TX_BUFFER_SIZE
   #define RA8_ETH_TX_BUFFER_SIZE 1536
#elif (RA8_ETH_TX_BUFFER_SIZE != 1536)
   #error RA8_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef RA8_ETH_RX_BUFFER_COUNT
   #define RA8_ETH_RX_BUFFER_COUNT 6
#elif (RA8_ETH_RX_BUFFER_COUNT < 1)
   #error RA8_ETH_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef RA8_ETH_RX_BUFFER_SIZE
   #define RA8_ETH_RX_BUFFER_SIZE 1536
#elif (RA8_ETH_RX_BUFFER_SIZE != 1536)
   #error RA8_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Interrupt priority grouping
#ifndef RA8_ETH_IRQ_PRIORITY_GROUPING
   #define RA8_ETH_IRQ_PRIORITY_GROUPING 3
#elif (RA8_ETH_IRQ_PRIORITY_GROUPING < 0)
   #error RA8_ETH_IRQ_PRIORITY_GROUPING parameter is not valid
#endif

//Ethernet interrupt group priority
#ifndef RA8_ETH_IRQ_GROUP_PRIORITY
   #define RA8_ETH_IRQ_GROUP_PRIORITY 12
#elif (RA8_ETH_IRQ_GROUP_PRIORITY < 0)
   #error RA8_ETH_IRQ_GROUP_PRIORITY parameter is not valid
#endif

//Ethernet interrupt subpriority
#ifndef RA8_ETH_IRQ_SUB_PRIORITY
   #define RA8_ETH_IRQ_SUB_PRIORITY 0
#elif (RA8_ETH_IRQ_SUB_PRIORITY < 0)
   #error RA8_ETH_IRQ_SUB_PRIORITY parameter is not valid
#endif

//Name of the section where to place DMA buffers
#ifndef RA8_ETH_RAM_SECTION
   #define RA8_ETH_RAM_SECTION ".nocache"
#endif

//Ethernet DMA controller
#ifndef R_ETHERC_EDMAC
   #define R_ETHERC_EDMAC ((R_ETHERC_EDMAC_Type *) R_ETHERC_EDMAC_BASE)
#endif

//MSTPCRB15 bitfield
#ifndef R_MSTP_MSTPCRB_MSTPB15_Msk
   #define R_MSTP_MSTPCRB_MSTPB15_Msk (R_MSTP_MSTPCRB_MSTPB_Msk << 15)
#endif

//Transmit DMA descriptor flags
#define EDMAC_TD0_TACT     0x80000000
#define EDMAC_TD0_TDLE     0x40000000
#define EDMAC_TD0_TFP_SOF  0x20000000
#define EDMAC_TD0_TFP_EOF  0x10000000
#define EDMAC_TD0_TFE      0x08000000
#define EDMAC_TD0_TWBI     0x04000000
#define EDMAC_TD0_TFS_MASK 0x0000010F
#define EDMAC_TD0_TFS_TABT 0x00000100
#define EDMAC_TD0_TFS_CND  0x00000008
#define EDMAC_TD0_TFS_DLC  0x00000004
#define EDMAC_TD0_TFS_CD   0x00000002
#define EDMAC_TD0_TFS_TRO  0x00000001
#define EDMAC_TD1_TBL      0xFFFF0000
#define EDMAC_TD2_TBA      0xFFFFFFFF

//Receive DMA descriptor flags
#define EDMAC_RD0_RACT     0x80000000
#define EDMAC_RD0_RDLE     0x40000000
#define EDMAC_RD0_RFP_SOF  0x20000000
#define EDMAC_RD0_RFP_EOF  0x10000000
#define EDMAC_RD0_RFE      0x08000000
#define EDMAC_RD0_RFS_MASK 0x0000039F
#define EDMAC_RD0_RFS_RFOF 0x00000200
#define EDMAC_RD0_RFS_RABT 0x00000100
#define EDMAC_RD0_RFS_RMAF 0x00000080
#define EDMAC_RD0_RFS_RRF  0x00000010
#define EDMAC_RD0_RFS_RTLF 0x00000008
#define EDMAC_RD0_RFS_RTSF 0x00000004
#define EDMAC_RD0_RFS_PRE  0x00000002
#define EDMAC_RD0_RFS_CERF 0x00000001
#define EDMAC_RD1_RBL      0xFFFF0000
#define EDMAC_RD1_RFL      0x0000FFFF
#define EDMAC_RD2_RBA      0xFFFFFFFF

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Transmit DMA descriptor
 **/

typedef struct
{
   uint32_t td0;
   uint32_t td1;
   uint32_t td2;
   uint32_t padding;
} Ra8EthTxDmaDesc;


/**
 * @brief Receive DMA descriptor
 **/

typedef struct
{
   uint32_t rd0;
   uint32_t rd1;
   uint32_t rd2;
   uint32_t padding;
} Ra8EthRxDmaDesc;


//RA8 Ethernet MAC driver
extern const NicDriver ra8EthDriver;

//RA8 Ethernet MAC related functions
error_t ra8EthInit(NetInterface *interface);
void ra8EthInitGpio(NetInterface *interface);
void ra8EthInitDmaDesc(NetInterface *interface);

void ra8EthTick(NetInterface *interface);

void ra8EthEnableIrq(NetInterface *interface);
void ra8EthDisableIrq(NetInterface *interface);
void ra8EthEventHandler(NetInterface *interface);

error_t ra8EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t ra8EthReceivePacket(NetInterface *interface);

error_t ra8EthUpdateMacAddrFilter(NetInterface *interface);
error_t ra8EthUpdateMacConfig(NetInterface *interface);

void ra8EthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t ra8EthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

void ra8EthWriteSmi(uint32_t data, uint_t length);
uint32_t ra8EthReadSmi(uint_t length);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
