/**
 * @file pic32mz_eth_driver.h
 * @brief PIC32MZ Ethernet MAC driver
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

#ifndef _PIC32MZ_ETH_DRIVER_H
#define _PIC32MZ_ETH_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef PIC32MZ_ETH_TX_BUFFER_COUNT
   #define PIC32MZ_ETH_TX_BUFFER_COUNT 3
#elif (PIC32MZ_ETH_TX_BUFFER_COUNT < 1)
   #error PIC32MZ_ETH_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef PIC32MZ_ETH_TX_BUFFER_SIZE
   #define PIC32MZ_ETH_TX_BUFFER_SIZE 1536
#elif (PIC32MZ_ETH_TX_BUFFER_SIZE != 1536)
   #error PIC32MZ_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef PIC32MZ_ETH_RX_BUFFER_COUNT
   #define PIC32MZ_ETH_RX_BUFFER_COUNT 6
#elif (PIC32MZ_ETH_RX_BUFFER_COUNT < 1)
   #error PIC32MZ_ETH_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef PIC32MZ_ETH_RX_BUFFER_SIZE
   #define PIC32MZ_ETH_RX_BUFFER_SIZE 1536
#elif (PIC32MZ_ETH_RX_BUFFER_SIZE != 1536)
   #error PIC32MZ_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Ethernet interrupt priority
#ifndef PIC32MZ_ETH_IRQ_PRIORITY
   #define PIC32MZ_ETH_IRQ_PRIORITY 2
#elif (PIC32MZ_ETH_IRQ_PRIORITY < 0)
   #error PIC32MZ_ETH_IRQ_PRIORITY parameter is not valid
#endif

//Ethernet interrupt subpriority
#ifndef PIC32MZ_ETH_IRQ_SUB_PRIORITY
   #define PIC32MZ_ETH_IRQ_SUB_PRIORITY 0
#elif (PIC32MZ_ETH_IRQ_SUB_PRIORITY < 0)
   #error PIC32MZ_ETH_IRQ_SUB_PRIORITY parameter is not valid
#endif

//Processor-specific definitions
#if defined(__32MZ1025W104132__) || defined(__32MZ2051W104132__) || \
   defined(__WFI32E01__) || defined(__WFI32E02__) || defined(__WFI32E03__)
   //Enable Ethernet interrupts
   #define ETH_SET_ETHIE() IEC5SET = _IEC5_ETHIE_MASK
   //Disable Ethernet interrupts
   #define ETH_CLEAR_ETHIE() IEC5CLR = _IEC5_ETHIE_MASK
   //Clear Ethernet interrupt flag
   #define ETH_CLEAR_ETHIF() IFS5CLR = _IFS5_ETHIF_MASK

   //Set Ethernet interrupt priority
   #define ETH_SET_ETHIP(n) \
      IPC41CLR = _IPC41_ETHIP_MASK, \
      IPC41SET = ((n) << _IPC41_ETHIP_POSITION)

   //Set Ethernet interrupt subpriority
   #define ETH_SET_ETHIS(n) \
      IPC41CLR = _IPC41_ETHIS_MASK, \
      IPC41SET = ((n) << _IPC41_ETHIS_POSITION)
#else
   //Enable Ethernet interrupts
   #define ETH_SET_ETHIE() IEC4SET = _IEC4_ETHIE_MASK
   //Disable Ethernet interrupts
   #define ETH_CLEAR_ETHIE() IEC4CLR = _IEC4_ETHIE_MASK
   //Clear Ethernet interrupt flag
   #define ETH_CLEAR_ETHIF() IFS4CLR = _IFS4_ETHIF_MASK

   //Set Ethernet interrupt priority
   #define ETH_SET_ETHIP(n) \
      IPC38CLR = _IPC38_ETHIP_MASK, \
      IPC38SET = ((n) << _IPC38_ETHIP_POSITION)

   //Set Ethernet interrupt subpriority
   #define ETH_SET_ETHIS(n) \
      IPC38CLR = _IPC38_ETHIS_MASK, \
      IPC38SET = ((n) << _IPC38_ETHIS_POSITION)
#endif

//EMAC1MCFG register
#define _EMAC1MCFG_CLKSEL_DIV4  (0 << _EMAC1MCFG_CLKSEL_POSITION)
#define _EMAC1MCFG_CLKSEL_DIV6  (2 << _EMAC1MCFG_CLKSEL_POSITION)
#define _EMAC1MCFG_CLKSEL_DIV8  (3 << _EMAC1MCFG_CLKSEL_POSITION)
#define _EMAC1MCFG_CLKSEL_DIV10 (4 << _EMAC1MCFG_CLKSEL_POSITION)
#define _EMAC1MCFG_CLKSEL_DIV14 (5 << _EMAC1MCFG_CLKSEL_POSITION)
#define _EMAC1MCFG_CLKSEL_DIV20 (6 << _EMAC1MCFG_CLKSEL_POSITION)
#define _EMAC1MCFG_CLKSEL_DIV28 (7 << _EMAC1MCFG_CLKSEL_POSITION)
#define _EMAC1MCFG_CLKSEL_DIV40 (8 << _EMAC1MCFG_CLKSEL_POSITION)
#define _EMAC1MCFG_CLKSEL_DIV48 (9 << _EMAC1MCFG_CLKSEL_POSITION)
#define _EMAC1MCFG_CLKSEL_DIV50 (10 << _EMAC1MCFG_CLKSEL_POSITION)

//Transmit buffer descriptor flags
#define ETH_TX_CTRL_SOP                   0x80000000
#define ETH_TX_CTRL_EOP                   0x40000000
#define ETH_TX_CTRL_BYTE_COUNT            0x07FF0000
#define ETH_TX_CTRL_NPV                   0x00000100
#define ETH_TX_CTRL_EOWN                  0x00000080
#define ETH_TX_STATUS1_VLAN               0x00080000
#define ETH_TX_STATUS1_BACKPRESSURE       0x00040000
#define ETH_TX_STATUS1_PAUSE              0x00020000
#define ETH_TX_STATUS1_CONTROL            0x00010000
#define ETH_TX_STATUS1_TOTAL_BYTES        0x0000FFFF
#define ETH_TX_STATUS2_UNDERRUN           0x80000000
#define ETH_TX_STATUS2_GIANT              0x40000000
#define ETH_TX_STATUS2_LATE_COL           0x20000000
#define ETH_TX_STATUS2_MAX_COL            0x10000000
#define ETH_TX_STATUS2_EXCESSIVE_DEFER    0x08000000
#define ETH_TX_STATUS2_PACKET_DEFER       0x04000000
#define ETH_TX_STATUS2_BROADCAST          0x02000000
#define ETH_TX_STATUS2_MULTICAST          0x01000000
#define ETH_TX_STATUS2_DONE               0x00800000
#define ETH_TX_STATUS2_LEN_OUT_OF_RANGE   0x00400000
#define ETH_TX_STATUS2_LEN_CHECK_ERROR    0x00200000
#define ETH_TX_STATUS2_CRC_ERROR          0x00100000
#define ETH_TX_STATUS2_COL_COUNT          0x000F0000
#define ETH_TX_STATUS2_BYTE_COUNT         0x0000FFFF

//Receive buffer descriptor flags
#define ETH_RX_CTRL_SOP                   0x80000000
#define ETH_RX_CTRL_EOP                   0x40000000
#define ETH_RX_CTRL_BYTE_COUNT            0x07FF0000
#define ETH_RX_CTRL_NPV                   0x00000100
#define ETH_RX_CTRL_EOWN                  0x00000080
#define ETH_RX_STATUS1_MULTICAST_MATCH    0x80000000
#define ETH_RX_STATUS1_BROADCAST_MATCH    0x40000000
#define ETH_RX_STATUS1_UNICAST_MATCH      0x20000000
#define ETH_RX_STATUS1_PATTERN_MATCH      0x10000000
#define ETH_RX_STATUS1_MAGIC_PACKET_MATCH 0x08000000
#define ETH_RX_STATUS1_HASH_TABLE_MATCH   0x04000000
#define ETH_RX_STATUS1_NOT_MATCH          0x02000000
#define ETH_RX_STATUS1_RUNT_PACKET        0x01000000
#define ETH_RX_STATUS1_PACKET_CHECKSUM    0x0000FFFF
#define ETH_RX_STATUS2_VLAN               0x40000000
#define ETH_RX_STATUS2_UNKNOWN_OP_CODE    0x20000000
#define ETH_RX_STATUS2_PAUSE              0x10000000
#define ETH_RX_STATUS2_CONTROL            0x08000000
#define ETH_RX_STATUS2_DRIBBLE_NIBBLE     0x04000000
#define ETH_RX_STATUS2_BROADCAST          0x02000000
#define ETH_RX_STATUS2_MULTICAST          0x01000000
#define ETH_RX_STATUS2_OK                 0x00800000
#define ETH_RX_STATUS2_LEN_OUT_OF_RANGE   0x00400000
#define ETH_RX_STATUS2_LEN_CHECK_ERROR    0x00200000
#define ETH_RX_STATUS2_CRC_ERROR          0x00100000
#define ETH_RX_STATUS2_CODE_VIOLATION     0x00080000
#define ETH_RX_STATUS2_CARRIER_EVENT      0x00040000
#define ETH_RX_STATUS2_RXDV_EVENT         0x00020000
#define ETH_RX_STATUS2_LONG_EVENT         0x00010000
#define ETH_RX_STATUS2_BYTE_COUNT         0x0000FFFF

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief TX buffer descriptor
 **/

typedef struct
{
   uint32_t control;
   uint32_t address;
   uint32_t status1;
   uint32_t status2;
   uint32_t next;
} Pic32mzTxBufferDesc;


/**
 * @brief RX buffer descriptor
 **/

typedef struct
{
   uint32_t control;
   uint32_t address;
   uint32_t status1;
   uint32_t status2;
   uint32_t next;
} Pic32mzRxBufferDesc;


//PIC32MZ Ethernet MAC driver
extern const NicDriver pic32mzEthDriver;

//PIC32MZ Ethernet MAC related functions
error_t pic32mzEthInit(NetInterface *interface);
void pic32mzEthInitGpio(NetInterface *interface);
void pic32mzEthInitBufferDesc(NetInterface *interface);

void pic32mzEthTick(NetInterface *interface);

void pic32mzEthEnableIrq(NetInterface *interface);
void pic32mzEthDisableIrq(NetInterface *interface);
void pic32mzEthIrqHandler(void);
void pic32mzEthEventHandler(NetInterface *interface);

error_t pic32mzEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t pic32mzEthReceivePacket(NetInterface *interface);

error_t pic32mzEthUpdateMacAddrFilter(NetInterface *interface);
error_t pic32mzEthUpdateMacConfig(NetInterface *interface);

void pic32mzEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t pic32mzEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

uint32_t pic32mzEthCalcCrc(const void *data, size_t length);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
