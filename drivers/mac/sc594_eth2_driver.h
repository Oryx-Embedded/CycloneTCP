/**
 * @file sc594_eth2_driver.h
 * @brief ADSP-SC594 Ethernet MAC driver (EMAC1 instance)
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

#ifndef _SC594_ETH2_DRIVER_H
#define _SC594_ETH2_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef SC594_ETH2_TX_BUFFER_COUNT
   #define SC594_ETH2_TX_BUFFER_COUNT 8
#elif (SC594_ETH2_TX_BUFFER_COUNT < 1)
   #error SC594_ETH2_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef SC594_ETH2_TX_BUFFER_SIZE
   #define SC594_ETH2_TX_BUFFER_SIZE 1536
#elif (SC594_ETH2_TX_BUFFER_SIZE != 1536)
   #error SC594_ETH2_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef SC594_ETH2_RX_BUFFER_COUNT
   #define SC594_ETH2_RX_BUFFER_COUNT 8
#elif (SC594_ETH2_RX_BUFFER_COUNT < 1)
   #error SC594_ETH2_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef SC594_ETH2_RX_BUFFER_SIZE
   #define SC594_ETH2_RX_BUFFER_SIZE 1536
#elif (SC594_ETH2_RX_BUFFER_SIZE != 1536)
   #error SC594_ETH2_RX_BUFFER_SIZE parameter is not valid
#endif

//Ethernet interrupt priority
#ifndef SC594_ETH2_IRQ_PRIORITY
   #define SC594_ETH2_IRQ_PRIORITY 0
#elif (SC594_ETH2_IRQ_PRIORITY < 0)
   #error SC594_ETH2_IRQ_PRIORITY parameter is not valid
#endif

//Name of the section where to place DMA buffers
#ifndef SC594_ETH2_RAM_SECTION
   #define SC594_ETH2_RAM_SECTION ".l2_uncached_data"
#endif

//EMAC1 peripheral ID
#define EMAC1_SPU_PID 106

//EMAC_SMI_ADDR register
#define ENUM_EMAC_SMI_ADDR_CR_DIV42   (0 << BITP_EMAC_SMI_ADDR_CR)
#define ENUM_EMAC_SMI_ADDR_CR_DIV62   (1 << BITP_EMAC_SMI_ADDR_CR)
#define ENUM_EMAC_SMI_ADDR_CR_DIV16   (2 << BITP_EMAC_SMI_ADDR_CR)
#define ENUM_EMAC_SMI_ADDR_CR_DIV26   (3 << BITP_EMAC_SMI_ADDR_CR)

//EMAC_DMA_BUSMODE register
#define ENUM_EMAC_DMA_BUSMODE_RPBL_1  (1 << BITP_EMAC_DMA0_BUSMODE_RPBL)
#define ENUM_EMAC_DMA_BUSMODE_RPBL_2  (2 << BITP_EMAC_DMA0_BUSMODE_RPBL)
#define ENUM_EMAC_DMA_BUSMODE_RPBL_4  (4 << BITP_EMAC_DMA0_BUSMODE_RPBL)
#define ENUM_EMAC_DMA_BUSMODE_RPBL_8  (8 << BITP_EMAC_DMA0_BUSMODE_RPBL)
#define ENUM_EMAC_DMA_BUSMODE_RPBL_16 (16 << BITP_EMAC_DMA0_BUSMODE_RPBL)
#define ENUM_EMAC_DMA_BUSMODE_RPBL_32 (32 << BITP_EMAC_DMA0_BUSMODE_RPBL)

#define ENUM_EMAC_DMA_BUSMODE_PBL_1   (1 << BITP_EMAC_DMA0_BUSMODE_PBL)
#define ENUM_EMAC_DMA_BUSMODE_PBL_2   (2 << BITP_EMAC_DMA0_BUSMODE_PBL)
#define ENUM_EMAC_DMA_BUSMODE_PBL_4   (4 << BITP_EMAC_DMA0_BUSMODE_PBL)
#define ENUM_EMAC_DMA_BUSMODE_PBL_8   (8 << BITP_EMAC_DMA0_BUSMODE_PBL)
#define ENUM_EMAC_DMA_BUSMODE_PBL_16  (16 << BITP_EMAC_DMA0_BUSMODE_PBL)
#define ENUM_EMAC_DMA_BUSMODE_PBL_32  (32 << BITP_EMAC_DMA0_BUSMODE_PBL)

//Transmit DMA descriptor flags
#define EMAC_TDES0_OWN       0x80000000
#define EMAC_TDES0_IC        0x40000000
#define EMAC_TDES0_LS        0x20000000
#define EMAC_TDES0_FS        0x10000000
#define EMAC_TDES0_DC        0x08000000
#define EMAC_TDES0_DP        0x04000000
#define EMAC_TDES0_TTSE      0x02000000
#define EMAC_TDES0_CRCR      0x01000000
#define EMAC_TDES0_CIC       0x00C00000
#define EMAC_TDES0_TER       0x00200000
#define EMAC_TDES0_TCH       0x00100000
#define EMAC_TDES0_VLIC      0x000C0000
#define EMAC_TDES0_TTSS      0x00020000
#define EMAC_TDES0_IHE       0x00010000
#define EMAC_TDES0_ES        0x00008000
#define EMAC_TDES0_JT        0x00004000
#define EMAC_TDES0_FF        0x00002000
#define EMAC_TDES0_IPE       0x00001000
#define EMAC_TDES0_LCA       0x00000800
#define EMAC_TDES0_NC        0x00000400
#define EMAC_TDES0_LCO       0x00000200
#define EMAC_TDES0_EC        0x00000100
#define EMAC_TDES0_VF        0x00000080
#define EMAC_TDES0_CC        0x00000078
#define EMAC_TDES0_ED        0x00000004
#define EMAC_TDES0_UF        0x00000002
#define EMAC_TDES0_DB        0x00000001
#define EMAC_TDES1_SAIC      0xE0000000
#define EMAC_TDES1_TBS2      0x1FFF0000
#define EMAC_TDES1_TBS1      0x00001FFF
#define EMAC_TDES2_TBAP1     0xFFFFFFFF
#define EMAC_TDES3_TBAP2     0xFFFFFFFF

//Receive DMA descriptor flags
#define EMAC_RDES0_OWN       0x80000000
#define EMAC_RDES0_AFM       0x40000000
#define EMAC_RDES0_FL        0x3FFF0000
#define EMAC_RDES0_ES        0x00008000
#define EMAC_RDES0_DE        0x00004000
#define EMAC_RDES0_SAF       0x00002000
#define EMAC_RDES0_LE        0x00001000
#define EMAC_RDES0_OE        0x00000800
#define EMAC_RDES0_VLAN      0x00000400
#define EMAC_RDES0_FS        0x00000200
#define EMAC_RDES0_LS        0x00000100
#define EMAC_RDES0_TSV       0x00000080
#define EMAC_RDES0_LCO       0x00000040
#define EMAC_RDES0_FT        0x00000020
#define EMAC_RDES0_RWT       0x00000010
#define EMAC_RDES0_RE        0x00000008
#define EMAC_RDES0_DBE       0x00000004
#define EMAC_RDES0_CE        0x00000002
#define EMAC_RDES0_ESA       0x00000001
#define EMAC_RDES1_DIC       0x80000000
#define EMAC_RDES1_RBS2      0x1FFF0000
#define EMAC_RDES1_RER       0x00008000
#define EMAC_RDES1_RCH       0x00004000
#define EMAC_RDES1_RBS1      0x00001FFF
#define EMAC_RDES2_RBAP1     0xFFFFFFFF
#define EMAC_RDES3_RBAP2     0xFFFFFFFF

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
} Sc594Eth2TxDmaDesc;


/**
 * @brief Enhanced RX DMA descriptor
 **/

typedef struct
{
   uint32_t rdes0;
   uint32_t rdes1;
   uint32_t rdes2;
   uint32_t rdes3;
} Sc594Eth2RxDmaDesc;


//ADSP-SC594 Ethernet MAC driver (EMAC1 instance)
extern const NicDriver sc594Eth2Driver;

//ADSP-SC594 Ethernet MAC related functions
error_t sc594Eth2Init(NetInterface *interface);
void sc594Eth2InitGpio(NetInterface *interface);
void sc594Eth2ResetPhy(NetInterface *interface);
void sc594Eth2InitDmaDesc(NetInterface *interface);

void sc594Eth2Tick(NetInterface *interface);

void sc594Eth2EnableIrq(NetInterface *interface);
void sc594Eth2DisableIrq(NetInterface *interface);
void sc594Eth2IrqHandler(uint32_t id, void *param);
void sc594Eth2EventHandler(NetInterface *interface);

error_t sc594Eth2SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t sc594Eth2ReceivePacket(NetInterface *interface);

error_t sc594Eth2UpdateMacAddrFilter(NetInterface *interface);
error_t sc594Eth2UpdateMacConfig(NetInterface *interface);

void sc594Eth2WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t sc594Eth2ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

uint32_t sc594Eth2CalcCrc(const void *data, size_t length);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
