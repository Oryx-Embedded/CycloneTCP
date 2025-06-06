/**
 * @file gd32h7xx_eth1_driver.h
 * @brief GigaDevice GD32H7 Ethernet MAC driver (ENET0 instance)
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

#ifndef _GD32H7XX_ETH1_DRIVER_H
#define _GD32H7XX_ETH1_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef GD32H7XX_ETH1_TX_BUFFER_COUNT
   #define GD32H7XX_ETH1_TX_BUFFER_COUNT 8
#elif (GD32H7XX_ETH1_TX_BUFFER_COUNT < 1)
   #error GD32H7XX_ETH1_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef GD32H7XX_ETH1_TX_BUFFER_SIZE
   #define GD32H7XX_ETH1_TX_BUFFER_SIZE 1536
#elif (GD32H7XX_ETH1_TX_BUFFER_SIZE != 1536)
   #error GD32H7XX_ETH1_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef GD32H7XX_ETH1_RX_BUFFER_COUNT
   #define GD32H7XX_ETH1_RX_BUFFER_COUNT 8
#elif (GD32H7XX_ETH1_RX_BUFFER_COUNT < 1)
   #error GD32H7XX_ETH1_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef GD32H7XX_ETH1_RX_BUFFER_SIZE
   #define GD32H7XX_ETH1_RX_BUFFER_SIZE 1536
#elif (GD32H7XX_ETH1_RX_BUFFER_SIZE != 1536)
   #error GD32H7XX_ETH1_RX_BUFFER_SIZE parameter is not valid
#endif

//Interrupt priority grouping
#ifndef GD32H7XX_ETH1_IRQ_PRIORITY_GROUPING
   #define GD32H7XX_ETH1_IRQ_PRIORITY_GROUPING 3
#elif (GD32H7XX_ETH1_IRQ_PRIORITY_GROUPING < 0)
   #error GD32H7XX_ETH1_IRQ_PRIORITY_GROUPING parameter is not valid
#endif

//Ethernet interrupt group priority
#ifndef GD32H7XX_ETH1_IRQ_GROUP_PRIORITY
   #define GD32H7XX_ETH1_IRQ_GROUP_PRIORITY 12
#elif (GD32H7XX_ETH1_IRQ_GROUP_PRIORITY < 0)
   #error GD32H7XX_ETH1_IRQ_GROUP_PRIORITY parameter is not valid
#endif

//Ethernet interrupt subpriority
#ifndef GD32H7XX_ETH1_IRQ_SUB_PRIORITY
   #define GD32H7XX_ETH1_IRQ_SUB_PRIORITY 0
#elif (GD32H7XX_ETH1_IRQ_SUB_PRIORITY < 0)
   #error GD32H7XX_ETH1_IRQ_SUB_PRIORITY parameter is not valid
#endif

//Name of the section where to place DMA buffers
#ifndef STM32H7XX_ETH1_RAM_SECTION
   #define STM32H7XX_ETH1_RAM_SECTION ".ram_no_cache"
#endif
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
} Gd32h7xxEth1TxDmaDesc;


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
} Gd32h7xxEth1RxDmaDesc;


//GD32H7 Ethernet MAC driver
extern const NicDriver gd32h7xxEth1Driver;

//GD32H7 Ethernet MAC related functions
error_t gd32h7xxEth1Init(NetInterface *interface);
void gd32h7xxEth1InitGpio(NetInterface *interface);
void gd32h7xxEth1InitDmaDesc(NetInterface *interface);

void gd32h7xxEth1Tick(NetInterface *interface);

void gd32h7xxEth1EnableIrq(NetInterface *interface);
void gd32h7xxEth1DisableIrq(NetInterface *interface);
void gd32h7xxEth1EventHandler(NetInterface *interface);

error_t gd32h7xxEth1SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t gd32h7xxEth1ReceivePacket(NetInterface *interface);

error_t gd32h7xxEth1UpdateMacAddrFilter(NetInterface *interface);
error_t gd32h7xxEth1UpdateMacConfig(NetInterface *interface);

void gd32h7xxEth1WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t gd32h7xxEth1ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

uint32_t gd32h7xxEth1CalcCrc(const void *data, size_t length);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
