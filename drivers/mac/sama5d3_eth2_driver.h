/**
 * @file sama5d3_eth2_driver.h
 * @brief SAMA5D3 Gigabit Ethernet MAC driver (GMAC instance)
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

#ifndef _SAMA5D3_ETH2_DRIVER_H
#define _SAMA5D3_ETH2_DRIVER_H

//Number of TX buffers
#ifndef SAMA5D3_ETH2_TX_BUFFER_COUNT
   #define SAMA5D3_ETH2_TX_BUFFER_COUNT 2
#elif (SAMA5D3_ETH2_TX_BUFFER_COUNT != 2)
   #error SAMA5D3_ETH2_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef SAMA5D3_ETH2_TX_BUFFER_SIZE
   #define SAMA5D3_ETH2_TX_BUFFER_SIZE 1536
#elif (SAMA5D3_ETH2_TX_BUFFER_SIZE != 1536)
   #error SAMA5D3_ETH2_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef SAMA5D3_ETH2_RX_BUFFER_COUNT
   #define SAMA5D3_ETH2_RX_BUFFER_COUNT 96
#elif (SAMA5D3_ETH2_RX_BUFFER_COUNT < 12)
   #error SAMA5D3_ETH2_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef SAMA5D3_ETH2_RX_BUFFER_SIZE
   #define SAMA5D3_ETH2_RX_BUFFER_SIZE 128
#elif (SAMA5D3_ETH2_RX_BUFFER_SIZE != 128)
   #error SAMA5D3_ETH2_RX_BUFFER_SIZE parameter is not valid
#endif

//Ethernet interrupt priority
#ifndef SAMA5D3_ETH2_IRQ_PRIORITY
   #define SAMA5D3_ETH2_IRQ_PRIORITY 0
#elif (SAMA5D3_ETH2_IRQ_PRIORITY < 0)
   #error SAMA5D3_ETH2_IRQ_PRIORITY parameter is not valid
#endif

//Name of the section where to place DMA buffers
#ifndef SAMA5D3_ETH2_RAM_SECTION
   #define SAMA5D3_ETH2_RAM_SECTION ".ram_no_cache"
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
} Sama5d3Eth2TxBufferDesc;


/**
 * @brief Receive buffer descriptor
 **/

typedef struct
{
   uint32_t address;
   uint32_t status;
} Sama5d3Eth2RxBufferDesc;


//SAMA5D3 Ethernet MAC driver (GMAC instance)
extern const NicDriver sama5d3Eth2Driver;

//SAMA5D3 Ethernet MAC related functions
error_t sama5d3Eth2Init(NetInterface *interface);
void sama5d3Eth2InitGpio(NetInterface *interface);
void sama5d3Eth2InitBufferDesc(NetInterface *interface);

void sama5d3Eth2Tick(NetInterface *interface);

void sama5d3Eth2EnableIrq(NetInterface *interface);
void sama5d3Eth2DisableIrq(NetInterface *interface);
void sama5d3Eth2IrqHandler(void);
void sama5d3Eth2EventHandler(NetInterface *interface);

error_t sama5d3Eth2SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t sama5d3Eth2ReceivePacket(NetInterface *interface);

error_t sama5d3Eth2UpdateMacAddrFilter(NetInterface *interface);
error_t sama5d3Eth2UpdateMacConfig(NetInterface *interface);

void sama5d3Eth2WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t sama5d3Eth2ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
