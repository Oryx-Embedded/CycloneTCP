/**
 * @file sc598_eth1_driver.h
 * @brief ADSP-SC598 Ethernet MAC driver (EMAC0 instance)
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
 * @version 2.6.0
 **/

#ifndef _SC598_ETH1_DRIVER_H
#define _SC598_ETH1_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef SC598_ETH1_TX_BUFFER_COUNT
   #define SC598_ETH1_TX_BUFFER_COUNT 8
#elif (SC598_ETH1_TX_BUFFER_COUNT < 1)
   #error SC598_ETH1_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef SC598_ETH1_TX_BUFFER_SIZE
   #define SC598_ETH1_TX_BUFFER_SIZE 1536
#elif (SC598_ETH1_TX_BUFFER_SIZE != 1536)
   #error SC598_ETH1_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef SC598_ETH1_RX_BUFFER_COUNT
   #define SC598_ETH1_RX_BUFFER_COUNT 8
#elif (SC598_ETH1_RX_BUFFER_COUNT < 1)
   #error SC598_ETH1_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef SC598_ETH1_RX_BUFFER_SIZE
   #define SC598_ETH1_RX_BUFFER_SIZE 1536
#elif (SC598_ETH1_RX_BUFFER_SIZE != 1536)
   #error SC598_ETH1_RX_BUFFER_SIZE parameter is not valid
#endif

//Ethernet interrupt priority
#ifndef SC598_ETH1_IRQ_PRIORITY
   #define SC598_ETH1_IRQ_PRIORITY 0
#elif (SC598_ETH1_IRQ_PRIORITY < 0)
   #error SC598_ETH1_IRQ_PRIORITY parameter is not valid
#endif

//Name of the section where to place DMA buffers
#ifndef SC598_ETH1_RAM_SECTION
   #define SC598_ETH1_RAM_SECTION ".l2_uncached_data"
#endif

//EMAC0 peripheral ID
#define EMAC0_SPU_PID 105

//Transmit normal descriptor (read format)
#define EMAC_TDES0_BUF1AP        0xFFFFFFFF
#define EMAC_TDES1_BUF2AP        0xFFFFFFFF
#define EMAC_TDES2_IOC           0x80000000
#define EMAC_TDES2_TTSE_TMWD     0x40000000
#define EMAC_TDES2_B2L           0x3FFF0000
#define EMAC_TDES2_VTIR          0x0000C000
#define EMAC_TDES2_HL_B1L        0x00003FFF
#define EMAC_TDES3_OWN           0x80000000
#define EMAC_TDES3_CTXT          0x40000000
#define EMAC_TDES3_FD            0x20000000
#define EMAC_TDES3_LD            0x10000000
#define EMAC_TDES3_CPC           0x0C000000
#define EMAC_TDES3_SAIC          0x03800000
#define EMAC_TDES3_SLOTNUM_THL   0x00780000
#define EMAC_TDES3_TSE           0x00040000
#define EMAC_TDES3_CIC           0x00030000
#define EMAC_TDES3_FL            0x00007FFF
#define EMAC_TDES3_TPL           0w0003FFFF

//Transmit normal descriptor (write-back format)
#define EMAC_TDES0_TTSL          0xFFFFFFFF
#define EMAC_TDES1_TTSH          0xFFFFFFFF
#define EMAC_TDES3_OWN           0x80000000
#define EMAC_TDES3_CTXT          0x40000000
#define EMAC_TDES3_FD            0x20000000
#define EMAC_TDES3_LD            0x10000000
#define EMAC_TDES3_DE            0x08000000
#define EMAC_TDES3_TTSS          0x00020000
#define EMAC_TDES3_EUE           0x00010000
#define EMAC_TDES3_ES            0x00008000
#define EMAC_TDES3_JT            0x00004000
#define EMAC_TDES3_FF            0x00002000
#define EMAC_TDES3_PCE           0x00001000
#define EMAC_TDES3_LOC           0x00000800
#define EMAC_TDES3_NC            0x00000400
#define EMAC_TDES3_LC            0x00000200
#define EMAC_TDES3_EC            0x00000100
#define EMAC_TDES3_CC            0x000000F0
#define EMAC_TDES3_ED            0x00000008
#define EMAC_TDES3_UF            0x00000004
#define EMAC_TDES3_DB            0x00000002
#define EMAC_TDES3_IHE           0x00000001

//Transmit context descriptor
#define EMAC_TDES0_TTSL          0xFFFFFFFF
#define EMAC_TDES1_TTSH          0xFFFFFFFF
#define EMAC_TDES2_IVT           0xFFFF0000
#define EMAC_TDES2_MSS           0x00003FFF
#define EMAC_TDES3_OWN           0x80000000
#define EMAC_TDES3_CTXT          0x40000000
#define EMAC_TDES3_OSTC          0x08000000
#define EMAC_TDES3_TCMSSV        0x04000000
#define EMAC_TDES3_CDE           0x00800000
#define EMAC_TDES3_IVTIR         0x000C0000
#define EMAC_TDES3_IVLTV         0x00020000
#define EMAC_TDES3_VLTV          0x00010000
#define EMAC_TDES3_VT            0x0000FFFF

//Receive normal descriptor (read format)
#define EMAC_RDES0_BUF1AP        0xFFFFFFFF
#define EMAC_RDES2_BUF2AP        0xFFFFFFFF
#define EMAC_RDES3_OWN           0x80000000
#define EMAC_RDES3_IOC           0x40000000
#define EMAC_RDES3_BUF2V         0x02000000
#define EMAC_RDES3_BUF1V         0x01000000

//Receive normal descriptor (write-back format)
#define EMAC_RDES0_IVT           0xFFFF0000
#define EMAC_RDES0_OVT           0x0000FFFF
#define EMAC_RDES1_OPC           0xFFFF0000
#define EMAC_RDES1_TD            0x00008000
#define EMAC_RDES1_TSA           0x00004000
#define EMAC_RDES1_PV            0x00002000
#define EMAC_RDES1_PFT           0x00001000
#define EMAC_RDES1_PMT           0x00000F00
#define EMAC_RDES1_IPCE          0x00000080
#define EMAC_RDES1_IPCB          0x00000040
#define EMAC_RDES1_IPV6          0x00000020
#define EMAC_RDES1_IPV4          0x00000010
#define EMAC_RDES1_IPHE          0x00000008
#define EMAC_RDES1_PT            0x00000007
#define EMAC_RDES2_L3L4FM        0xE0000000
#define EMAC_RDES2_L4FM          0x10000000
#define EMAC_RDES2_L3FM          0x08000000
#define EMAC_RDES2_MADRM         0x07F80000
#define EMAC_RDES2_HF            0x00040000
#define EMAC_RDES2_DAF_RXPI      0x00020000
#define EMAC_RDES2_SAF_RXPD      0x00010000
#define EMAC_RDES2_OTS           0x00008000
#define EMAC_RDES2_ITS           0x00004000
#define EMAC_RDES2_RX_PARSER     0x00003800
#define EMAC_RDES2_ARPRN         0x00000400
#define EMAC_RDES2_HL            0x000003FF
#define EMAC_RDES3_OWN           0x80000000
#define EMAC_RDES3_CTXT          0x40000000
#define EMAC_RDES3_FD            0x20000000
#define EMAC_RDES3_LD            0x10000000
#define EMAC_RDES3_RS2V          0x08000000
#define EMAC_RDES3_RS1V          0x04000000
#define EMAC_RDES3_RS0V          0x02000000
#define EMAC_RDES3_CE            0x01000000
#define EMAC_RDES3_GP            0x00800000
#define EMAC_RDES3_RWT           0x00400000
#define EMAC_RDES3_OE            0x00200000
#define EMAC_RDES3_RE            0x00100000
#define EMAC_RDES3_DE            0x00080000
#define EMAC_RDES3_LT            0x00070000
#define EMAC_RDES3_ES            0x00008000
#define EMAC_RDES3_PL            0x00007FFF

//Receive context descriptor
#define EMAC_RDES0_RTSL          0xFFFFFFFF
#define EMAC_RDES1_RTSH          0xFFFFFFFF
#define EMAC_RDES3_OWN           0x80000000
#define EMAC_RDES3_CTXT          0x40000000

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Transmit descriptor
 **/

typedef struct
{
   uint32_t tdes0;
   uint32_t tdes1;
   uint32_t tdes2;
   uint32_t tdes3;
} Sc598Eth1TxDmaDesc;


/**
 * @brief Receive descriptor
 **/

typedef struct
{
   uint32_t rdes0;
   uint32_t rdes1;
   uint32_t rdes2;
   uint32_t rdes3;
} Sc598Eth1RxDmaDesc;


//ADSP-SC598 Ethernet MAC driver (EMAC0 instance)
extern const NicDriver sc598Eth1Driver;

//ADSP-SC598 Ethernet MAC related functions
error_t sc598Eth1Init(NetInterface *interface);
void sc598Eth1InitGpio(NetInterface *interface);
void sc598Eth1ResetPhy(NetInterface *interface);
void sc598Eth1InitDmaDesc(NetInterface *interface);

void sc598Eth1Tick(NetInterface *interface);

void sc598Eth1EnableIrq(NetInterface *interface);
void sc598Eth1DisableIrq(NetInterface *interface);
void sc598Eth1IrqHandler(uint32_t id, void *param);
void sc598Eth1EventHandler(NetInterface *interface);

error_t sc598Eth1SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t sc598Eth1ReceivePacket(NetInterface *interface);

error_t sc598Eth1UpdateMacAddrFilter(NetInterface *interface);
error_t sc598Eth1UpdateMacConfig(NetInterface *interface);

void sc598Eth1WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t sc598Eth1ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

uint32_t sc598Eth1CalcCrc(const void *data, size_t length);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
