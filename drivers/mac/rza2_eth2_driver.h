/**
 * @file rza2_eth2_driver.h
 * @brief RZ/A2 Ethernet MAC driver (ETHERC1 instance)
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

#ifndef _RZA2_ETH2_DRIVER_H
#define _RZA2_ETH2_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef RZA2_ETH2_TX_BUFFER_COUNT
   #define RZA2_ETH2_TX_BUFFER_COUNT 8
#elif (RZA2_ETH2_TX_BUFFER_COUNT < 1)
   #error RZA2_ETH2_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef RZA2_ETH2_TX_BUFFER_SIZE
   #define RZA2_ETH2_TX_BUFFER_SIZE 1536
#elif (RZA2_ETH2_TX_BUFFER_SIZE != 1536)
   #error RZA2_ETH2_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef RZA2_ETH2_RX_BUFFER_COUNT
   #define RZA2_ETH2_RX_BUFFER_COUNT 8
#elif (RZA2_ETH2_RX_BUFFER_COUNT < 1)
   #error RZA2_ETH2_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef RZA2_ETH2_RX_BUFFER_SIZE
   #define RZA2_ETH2_RX_BUFFER_SIZE 1536
#elif (RZA2_ETH2_RX_BUFFER_SIZE != 1536)
   #error RZA2_ETH2_RX_BUFFER_SIZE parameter is not valid
#endif

//Ethernet interrupt priority
#ifndef RZA2_ETH2_IRQ_PRIORITY
   #define RZA2_ETH2_IRQ_PRIORITY 25
#elif (RZA2_ETH2_IRQ_PRIORITY < 0)
   #error RZA2_ETH2_IRQ_PRIORITY parameter is not valid
#endif

//Name of the section where to place DMA buffers
#ifndef RZA2_ETH2_RAM_SECTION
   #define RZA2_ETH2_RAM_SECTION ".BSS_DMAC_SAMPLE_INTERNAL_RAM"
#endif

//Get physical address
#ifndef RZA2_ETH2_GET_PHYSICAL_ADDR
   #define RZA2_ETH2_GET_PHYSICAL_ADDR(addr) ((uint32_t) (addr) - 0x02000000U)
#endif

//EDMR register
#define EDMAC_EDMR_DL_16     0x00000000
#define EDMAC_EDMR_DL_32     0x00000010
#define EDMAC_EDMR_DL_64     0x00000020

//FDR register
#define EDMAC_FDR_TFD_2048   0x00000700
#define EDMAC_FDR_RFD_4096   0x0000000F

//Transmit DMA descriptor flags
#define EDMAC_TD0_TACT       0x80000000
#define EDMAC_TD0_TDLE       0x40000000
#define EDMAC_TD0_TFP_SOF    0x20000000
#define EDMAC_TD0_TFP_EOF    0x10000000
#define EDMAC_TD0_TFE        0x08000000
#define EDMAC_TD0_TWBI       0x04000000
#define EDMAC_TD0_TFS_MASK   0x0000010F
#define EDMAC_TD0_TFS_TABT   0x00000100
#define EDMAC_TD0_TFS_CND    0x00000008
#define EDMAC_TD0_TFS_DLC    0x00000004
#define EDMAC_TD0_TFS_CD     0x00000002
#define EDMAC_TD0_TFS_TRO    0x00000001
#define EDMAC_TD1_TBL        0xFFFF0000
#define EDMAC_TD2_TBA        0xFFFFFFFF

//Receive DMA descriptor flags
#define EDMAC_RD0_RACT       0x80000000
#define EDMAC_RD0_RDLE       0x40000000
#define EDMAC_RD0_RFP_SOF    0x20000000
#define EDMAC_RD0_RFP_EOF    0x10000000
#define EDMAC_RD0_RFE        0x08000000
#define EDMAC_RD0_RFS_MASK   0x0000039F
#define EDMAC_RD0_RFS_RFOF   0x00000200
#define EDMAC_RD0_RFS_RABT   0x00000100
#define EDMAC_RD0_RFS_RMAF   0x00000080
#define EDMAC_RD0_RFS_RRF    0x00000010
#define EDMAC_RD0_RFS_RTLF   0x00000008
#define EDMAC_RD0_RFS_RTSF   0x00000004
#define EDMAC_RD0_RFS_PRE    0x00000002
#define EDMAC_RD0_RFS_CERF   0x00000001
#define EDMAC_RD1_RBL        0xFFFF0000
#define EDMAC_RD1_RFL        0x0000FFFF
#define EDMAC_RD2_RBA        0xFFFFFFFF

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
} Rza2Eth2TxDmaDesc;


/**
 * @brief Receive DMA descriptor
 **/

typedef struct
{
   uint32_t rd0;
   uint32_t rd1;
   uint32_t rd2;
   uint32_t padding;
} Rza2Eth2RxDmaDesc;


//RZ/A2 Ethernet MAC driver (ETHERC1 instance)
extern const NicDriver rza2Eth2Driver;

//RZ/A2 Ethernet MAC related functions
error_t rza2Eth2Init(NetInterface *interface);
void rza2Eth2InitGpio(NetInterface *interface);
void rza2Eth2InitDmaDesc(NetInterface *interface);

void rza2Eth2Tick(NetInterface *interface);

void rza2Eth2EnableIrq(NetInterface *interface);
void rza2Eth2DisableIrq(NetInterface *interface);
void rza2Eth2IrqHandler(uint32_t intSense);
void rza2Eth2EventHandler(NetInterface *interface);

error_t rza2Eth2SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t rza2Eth2ReceivePacket(NetInterface *interface);

error_t rza2Eth2UpdateMacAddrFilter(NetInterface *interface);
error_t rza2Eth2UpdateMacConfig(NetInterface *interface);

void rza2Eth2WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t rza2Eth2ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

void rza2Eth2WriteSmi(uint32_t data, uint_t length);
uint32_t rza2Eth2ReadSmi(uint_t length);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
