/**
 * @file pic32ck_eth_driver.h
 * @brief PIC32CK GC01/SG01 Ethernet MAC driver
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

#ifndef _PIC32CK_ETH_DRIVER_H
#define _PIC32CK_ETH_DRIVER_H

//Number of TX buffers
#ifndef PIC32CK_ETH_TX_BUFFER_COUNT
   #define PIC32CK_ETH_TX_BUFFER_COUNT 3
#elif (PIC32CK_ETH_TX_BUFFER_COUNT < 1)
   #error PIC32CK_ETH_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef PIC32CK_ETH_TX_BUFFER_SIZE
   #define PIC32CK_ETH_TX_BUFFER_SIZE 1536
#elif (PIC32CK_ETH_TX_BUFFER_SIZE != 1536)
   #error PIC32CK_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef PIC32CK_ETH_RX_BUFFER_COUNT
   #define PIC32CK_ETH_RX_BUFFER_COUNT 72
#elif (PIC32CK_ETH_RX_BUFFER_COUNT < 12)
   #error PIC32CK_ETH_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef PIC32CK_ETH_RX_BUFFER_SIZE
   #define PIC32CK_ETH_RX_BUFFER_SIZE 128
#elif (PIC32CK_ETH_RX_BUFFER_SIZE != 128)
   #error PIC32CK_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Interrupt priority grouping
#ifndef PIC32CK_ETH_IRQ_PRIORITY_GROUPING
   #define PIC32CK_ETH_IRQ_PRIORITY_GROUPING 4
#elif (PIC32CK_ETH_IRQ_PRIORITY_GROUPING < 0)
   #error PIC32CK_ETH_IRQ_PRIORITY_GROUPING parameter is not valid
#endif

//Ethernet interrupt group priority
#ifndef PIC32CK_ETH_IRQ_GROUP_PRIORITY
   #define PIC32CK_ETH_IRQ_GROUP_PRIORITY 6
#elif (PIC32CK_ETH_IRQ_GROUP_PRIORITY < 0)
   #error PIC32CK_ETH_IRQ_GROUP_PRIORITY parameter is not valid
#endif

//Ethernet interrupt subpriority
#ifndef PIC32CK_ETH_IRQ_SUB_PRIORITY
   #define PIC32CK_ETH_IRQ_SUB_PRIORITY 0
#elif (PIC32CK_ETH_IRQ_SUB_PRIORITY < 0)
   #error PIC32CK_ETH_IRQ_SUB_PRIORITY parameter is not valid
#endif

//ETH_UR register
#ifndef ETH_UR_MII_Msk
   #define ETH_UR_MII_Msk 0x00000001
#endif

//TX buffer descriptor flags
#define ETH_TX_USED           0x80000000
#define ETH_TX_WRAP           0x40000000
#define ETH_TX_RLE_ERROR      0x20000000
#define ETH_TX_UNDERRUN_ERROR 0x10000000
#define ETH_TX_AHB_ERROR      0x08000000
#define ETH_TX_LATE_COL_ERROR 0x04000000
#define ETH_TX_CHECKSUM_ERROR 0x00700000
#define ETH_TX_NO_CRC         0x00010000
#define ETH_TX_LAST           0x00008000
#define ETH_TX_LENGTH         0x00003FFF

//RX buffer descriptor flags
#define ETH_RX_ADDRESS        0xFFFFFFFC
#define ETH_RX_WRAP           0x00000002
#define ETH_RX_OWNERSHIP      0x00000001
#define ETH_RX_BROADCAST      0x80000000
#define ETH_RX_MULTICAST_HASH 0x40000000
#define ETH_RX_UNICAST_HASH   0x20000000
#define ETH_RX_SAR            0x08000000
#define ETH_RX_SAR_MASK       0x06000000
#define ETH_RX_TYPE_ID        0x01000000
#define ETH_RX_SNAP           0x01000000
#define ETH_RX_TYPE_ID_MASK   0x00C00000
#define ETH_RX_CHECKSUM_VALID 0x00C00000
#define ETH_RX_VLAN_TAG       0x00200000
#define ETH_RX_PRIORITY_TAG   0x00100000
#define ETH_RX_VLAN_PRIORITY  0x000E0000
#define ETH_RX_CFI            0x00010000
#define ETH_RX_EOF            0x00008000
#define ETH_RX_SOF            0x00004000
#define ETH_RX_LENGTH_MSB     0x00002000
#define ETH_RX_BAD_FCS        0x00002000
#define ETH_RX_LENGTH         0x00001FFF

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
} Pic32ckTxBufferDesc;


/**
 * @brief Receive buffer descriptor
 **/

typedef struct
{
   uint32_t address;
   uint32_t status;
} Pic32ckRxBufferDesc;


//PIC32CK Ethernet MAC driver
extern const NicDriver pic32ckEthDriver;

//PIC32CK Ethernet MAC related functions
error_t pic32ckEthInit(NetInterface *interface);
void pic32ckEthInitGpio(NetInterface *interface);
void pic32ckEthInitBufferDesc(NetInterface *interface);

void pic32ckEthTick(NetInterface *interface);

void pic32ckEthEnableIrq(NetInterface *interface);
void pic32ckEthDisableIrq(NetInterface *interface);
void pic32ckEthEventHandler(NetInterface *interface);

error_t pic32ckEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t pic32ckEthReceivePacket(NetInterface *interface);

error_t pic32ckEthUpdateMacAddrFilter(NetInterface *interface);
error_t pic32ckEthUpdateMacConfig(NetInterface *interface);

void pic32ckEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t pic32ckEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
