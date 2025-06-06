/**
 * @file mpc5777_eth_driver.h
 * @brief NXP MPC5777 Ethernet MAC driver
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

#ifndef _MPC5777_ETH_DRIVER_H
#define _MPC5777_ETH_DRIVER_H

//Number of TX buffers
#ifndef MPC5777_ETH_TX_BUFFER_COUNT
   #define MPC5777_ETH_TX_BUFFER_COUNT 3
#elif (MPC5777_ETH_TX_BUFFER_COUNT < 1)
   #error MPC5777_ETH_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef MPC5777_ETH_TX_BUFFER_SIZE
   #define MPC5777_ETH_TX_BUFFER_SIZE 1536
#elif (MPC5777_ETH_TX_BUFFER_SIZE != 1536)
   #error MPC5777_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef MPC5777_ETH_RX_BUFFER_COUNT
   #define MPC5777_ETH_RX_BUFFER_COUNT 6
#elif (MPC5777_ETH_RX_BUFFER_COUNT < 1)
   #error MPC5777_ETH_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef MPC5777_ETH_RX_BUFFER_SIZE
   #define MPC5777_ETH_RX_BUFFER_SIZE 1536
#elif (MPC5777_ETH_RX_BUFFER_SIZE != 1536)
   #error MPC5777_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Ethernet interrupt priority
#ifndef MPC5777_ETH_IRQ_PRIORITY
   #define MPC5777_ETH_IRQ_PRIORITY 10
#elif (MPC5777_ETH_IRQ_PRIORITY < 0)
   #error MPC5777_ETH_IRQ_PRIORITY parameter is not valid
#endif

//Enhanced transmit buffer descriptor
#define FEC_TBD0_R                0x80000000
#define FEC_TBD0_TO1              0x40000000
#define FEC_TBD0_W                0x20000000
#define FEC_TBD0_TO2              0x10000000
#define FEC_TBD0_L                0x08000000
#define FEC_TBD0_TC               0x04000000
#define FEC_TBD0_DATA_LENGTH      0x0000FFFF
#define FEC_TBD1_DATA_POINTER     0xFFFFFFFF

//Enhanced receive buffer descriptor
#define FEC_RBD0_E                0x80000000
#define FEC_RBD0_RO1              0x40000000
#define FEC_RBD0_W                0x20000000
#define FEC_RBD0_RO2              0x10000000
#define FEC_RBD0_L                0x08000000
#define FEC_RBD0_M                0x01000000
#define FEC_RBD0_BC               0x00800000
#define FEC_RBD0_MC               0x00400000
#define FEC_RBD0_LG               0x00200000
#define FEC_RBD0_NO               0x00100000
#define FEC_RBD0_CR               0x00040000
#define FEC_RBD0_OV               0x00020000
#define FEC_RBD0_TR               0x00010000
#define FEC_RBD0_DATA_LENGTH      0x0000FFFF
#define FEC_RBD1_DATA_POINTER     0xFFFFFFFF

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//MPC5777 Ethernet MAC driver
extern const NicDriver mpc5777EthDriver;

//MPC5777 Ethernet MAC related functions
error_t mpc5777EthInit(NetInterface *interface);
void mpc5777EthInitGpio(NetInterface *interface);
void mpc5777EthInitBufferDesc(NetInterface *interface);

void mpc5777EthTick(NetInterface *interface);

void mpc5777EthEnableIrq(NetInterface *interface);
void mpc5777EthDisableIrq(NetInterface *interface);
void mpc5777EthEventHandler(NetInterface *interface);

error_t mpc5777EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t mpc5777EthReceivePacket(NetInterface *interface);

error_t mpc5777EthUpdateMacAddrFilter(NetInterface *interface);
error_t mpc5777EthUpdateMacConfig(NetInterface *interface);

void mpc5777EthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t mpc5777EthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

uint32_t mpc5777EthCalcCrc(const void *data, size_t length);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
