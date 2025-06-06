/**
 * @file at32f4xx_eth_driver.h
 * @brief Artery AT32F4 Ethernet MAC driver
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

#ifndef _AT32F4XX_ETH_DRIVER_H
#define _AT32F4XX_ETH_DRIVER_H

//Dependencies
#include "core/nic.h"

//Number of TX buffers
#ifndef AT32F4XX_ETH_TX_BUFFER_COUNT
   #define AT32F4XX_ETH_TX_BUFFER_COUNT 3
#elif (AT32F4XX_ETH_TX_BUFFER_COUNT < 1)
   #error AT32F4XX_ETH_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef AT32F4XX_ETH_TX_BUFFER_SIZE
   #define AT32F4XX_ETH_TX_BUFFER_SIZE 1536
#elif (AT32F4XX_ETH_TX_BUFFER_SIZE != 1536)
   #error AT32F4XX_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef AT32F4XX_ETH_RX_BUFFER_COUNT
   #define AT32F4XX_ETH_RX_BUFFER_COUNT 6
#elif (AT32F4XX_ETH_RX_BUFFER_COUNT < 1)
   #error AT32F4XX_ETH_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef AT32F4XX_ETH_RX_BUFFER_SIZE
   #define AT32F4XX_ETH_RX_BUFFER_SIZE 1536
#elif (AT32F4XX_ETH_RX_BUFFER_SIZE != 1536)
   #error AT32F4XX_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Interrupt priority grouping
#ifndef AT32F4XX_ETH_IRQ_PRIORITY_GROUPING
   #define AT32F4XX_ETH_IRQ_PRIORITY_GROUPING 3
#elif (AT32F4XX_ETH_IRQ_PRIORITY_GROUPING < 0)
   #error AT32F4XX_ETH_IRQ_PRIORITY_GROUPING parameter is not valid
#endif

//Ethernet interrupt group priority
#ifndef AT32F4XX_ETH_IRQ_GROUP_PRIORITY
   #define AT32F4XX_ETH_IRQ_GROUP_PRIORITY 12
#elif (AT32F4XX_ETH_IRQ_GROUP_PRIORITY < 0)
   #error AT32F4XX_ETH_IRQ_GROUP_PRIORITY parameter is not valid
#endif

//Ethernet interrupt subpriority
#ifndef AT32F4XX_ETH_IRQ_SUB_PRIORITY
   #define AT32F4XX_ETH_IRQ_SUB_PRIORITY 0
#elif (AT32F4XX_ETH_IRQ_SUB_PRIORITY < 0)
   #error AT32F4XX_ETH_IRQ_SUB_PRIORITY parameter is not valid
#endif

//Ethernet MAC Configuration register
#define EMAC_CTRL_WD            0x00800000
#define EMAC_CTRL_JD            0x00400000
#define EMAC_CTRL_IFG           0x000E0000
#define EMAC_CTRL_DCS           0x00010000
#define EMAC_CTRL_RESERVED15    0x00008000
#define EMAC_CTRL_FES           0x00004000
#define EMAC_CTRL_DRO           0x00002000
#define EMAC_CTRL_LM            0x00001000
#define EMAC_CTRL_DM            0x00000800
#define EMAC_CTRL_IPC           0x00000400
#define EMAC_CTRL_DR            0x00000200
#define EMAC_CTRL_ACS           0x00000080
#define EMAC_CTRL_BL            0x00000060
#define EMAC_CTRL_DC            0x00000010
#define EMAC_CTRL_TE            0x00000008
#define EMAC_CTRL_RE            0x00000004

//Ethernet MAC Frame Filter register
#define EMAC_FRMF_RA            0x80000000
#define EMAC_FRMF_HPF           0x00000400
#define EMAC_FRMF_SAF           0x00000200
#define EMAC_FRMF_SAIF          0x00000100
#define EMAC_FRMF_PCF           0x000000C0
#define EMAC_FRMF_DBF           0x00000020
#define EMAC_FRMF_PMC           0x00000010
#define EMAC_FRMF_DAIF          0x00000008
#define EMAC_FRMF_HMC           0x00000004
#define EMAC_FRMF_HUC           0x00000002
#define EMAC_FRMF_PR            0x00000001

//Ethernet MAC MII Address register
#define EMAC_MIIADDR_PA         0x0000F800
#define EMAC_MIIADDR_MII        0x000007C0
#define EMAC_MIIADDR_CR         0x0000003C
#define EMAC_MIIADDR_CR_DIV_42  0x00000000
#define EMAC_MIIADDR_CR_DIV_62  0x00000004
#define EMAC_MIIADDR_CR_DIV_16  0x00000008
#define EMAC_MIIADDR_CR_DIV_26  0x0000000C
#define EMAC_MIIADDR_CR_DIV_102 0x00000010
#define EMAC_MIIADDR_CR_DIV_124 0x00000014
#define EMAC_MIIADDR_MW         0x00000002
#define EMAC_MIIADDR_MB         0x00000001

//Ethernet MAC MII Data register
#define EMAC_MIIDT_MD           0x0000FFFF

//Ethernet MAC Interrupt Mask register
#define EMAC_IMR_TIM            0x00000200
#define EMAC_IMR_PIM            0x00000008

//Ethernet MAC Address 0 High register
#define EMAC_A0H_AE             0x80000000
#define EMAC_A0H_MA0H           0x0000FFFF

//Ethernet MAC Address 1 High register
#define EMAC_A1H_AE             0x80000000
#define EMAC_A1H_SA             0x40000000
#define EMAC_A1H_MBC            0x3F000000
#define EMAC_A1H_MA1H           0x0000FFFF

//Ethernet MAC Address 2 High register
#define EMAC_A2H_AE             0x80000000
#define EMAC_A2H_SA             0x40000000
#define EMAC_A2H_MBC            0x3F000000
#define EMAC_A2H_MA2H           0x0000FFFF

//Ethernet MAC Address 3 High register
#define EMAC_A3H_AE             0x80000000
#define EMAC_A3H_SA             0x40000000
#define EMAC_A3H_MBC            0x3F000000
#define EMAC_A3H_MA3H           0x0000FFFF

//Ethernet MMC Receive Interrupt register
#define EMAC_MMC_RIM_RUGFCIM    0x00020000
#define EMAC_MMC_RIM_RAEFACIM   0x00000040
#define EMAC_MMC_RIM_RCEFCIM    0x00000020

//Ethernet MMC Transmit Interrupt register
#define EMAC_MMC_TIM_TGFCIM     0x00200000
#define EMAC_MMC_TIM_TMCGFCIM   0x00008000
#define EMAC_MMC_TIM_TSCGFCIM   0x00004000

//Ethernet DMA Bus Mode register
#define EMAC_DMA_BM_AAB         0x02000000
#define EMAC_DMA_BM_PBLX8       0x01000000
#define EMAC_DMA_BM_USP         0x00800000
#define EMAC_DMA_BM_RDP         0x007E0000
#define EMAC_DMA_BM_RDP_1       0x00020000
#define EMAC_DMA_BM_RDP_2       0x00040000
#define EMAC_DMA_BM_RDP_4       0x00080000
#define EMAC_DMA_BM_RDP_8       0x00100000
#define EMAC_DMA_BM_RDP_16      0x00200000
#define EMAC_DMA_BM_RDP_32      0x00400000
#define EMAC_DMA_BM_FB          0x00010000
#define EMAC_DMA_BM_PR          0x0000C000
#define EMAC_DMA_BM_PR_1_1      0x00000000
#define EMAC_DMA_BM_PR_2_1      0x00004000
#define EMAC_DMA_BM_PR_3_1      0x00008000
#define EMAC_DMA_BM_PR_4_1      0x0000C000
#define EMAC_DMA_BM_PBL         0x00003F00
#define EMAC_DMA_BM_PBL_1       0x00000100
#define EMAC_DMA_BM_PBL_2       0x00000200
#define EMAC_DMA_BM_PBL_4       0x00000400
#define EMAC_DMA_BM_PBL_8       0x00000800
#define EMAC_DMA_BM_PBL_16      0x00001000
#define EMAC_DMA_BM_PBL_32      0x00002000
#define EMAC_DMA_BM_DSL         0x0000007C
#define EMAC_DMA_BM_DSL_0       0x00000000
#define EMAC_DMA_BM_DSL_1       0x00000004
#define EMAC_DMA_BM_DSL_2       0x00000008
#define EMAC_DMA_BM_DSL_4       0x00000010
#define EMAC_DMA_BM_DSL_8       0x00000020
#define EMAC_DMA_BM_DSL_16      0x00000040
#define EMAC_DMA_BM_DA          0x00000002
#define EMAC_DMA_BM_SWR         0x00000001

//Ethernet DMA Status register
#define EMAC_DMA_STS_TTI        0x20000000
#define EMAC_DMA_STS_MPI        0x10000000
#define EMAC_DMA_STS_MMI        0x08000000
#define EMAC_DMA_STS_EB         0x03800000
#define EMAC_DMA_STS_TS         0x00700000
#define EMAC_DMA_STS_RS         0x000E0000
#define EMAC_DMA_STS_NIS        0x00010000
#define EMAC_DMA_STS_AIS        0x00008000
#define EMAC_DMA_STS_ERI        0x00004000
#define EMAC_DMA_STS_FBEI       0x00002000
#define EMAC_DMA_STS_ETI        0x00000400
#define EMAC_DMA_STS_RWT        0x00000200
#define EMAC_DMA_STS_RPS        0x00000100
#define EMAC_DMA_STS_RBU        0x00000080
#define EMAC_DMA_STS_RI         0x00000040
#define EMAC_DMA_STS_UNF        0x00000020
#define EMAC_DMA_STS_OVF        0x00000010
#define EMAC_DMA_STS_TJT        0x00000008
#define EMAC_DMA_STS_TBU        0x00000004
#define EMAC_DMA_STS_TPS        0x00000002
#define EMAC_DMA_STS_TI         0x00000001

//Ethernet DMA Operation Mode register
#define EMAC_DMA_OPM_DT         0x04000000
#define EMAC_DMA_OPM_RSF        0x02000000
#define EMAC_DMA_OPM_DFRF       0x01000000
#define EMAC_DMA_OPM_TSF        0x00200000
#define EMAC_DMA_OPM_FTF        0x00100000
#define EMAC_DMA_OPM_TTC        0x0001C000
#define EMAC_DMA_OPM_SSTC       0x00002000
#define EMAC_DMA_OPM_FEF        0x00000080
#define EMAC_DMA_OPM_FUGF       0x00000040
#define EMAC_DMA_OPM_RTC        0x00000018
#define EMAC_DMA_OPM_OSF        0x00000004
#define EMAC_DMA_OPM_SSR        0x00000002

//Ethernet DMA Interrupt Enable register
#define EMAC_DMA_IE_NIE         0x00010000
#define EMAC_DMA_IE_AIE         0x00008000
#define EMAC_DMA_IE_ERE         0x00004000
#define EMAC_DMA_IE_FBEE        0x00002000
#define EMAC_DMA_IE_EIE         0x00000400
#define EMAC_DMA_IE_RWTE        0x00000200
#define EMAC_DMA_IE_RSE         0x00000100
#define EMAC_DMA_IE_RBUE        0x00000080
#define EMAC_DMA_IE_RIE         0x00000040
#define EMAC_DMA_IE_UNE         0x00000020
#define EMAC_DMA_IE_OVE         0x00000010
#define EMAC_DMA_IE_TJE         0x00000008
#define EMAC_DMA_IE_TUE         0x00000004
#define EMAC_DMA_IE_TSE         0x00000002
#define EMAC_DMA_IE_TIE         0x00000001

//Transmit DMA descriptor flags
#define EMAC_TDES0_OWN          0x80000000
#define EMAC_TDES0_IC           0x40000000
#define EMAC_TDES0_LS           0x20000000
#define EMAC_TDES0_FS           0x10000000
#define EMAC_TDES0_DC           0x08000000
#define EMAC_TDES0_DP           0x04000000
#define EMAC_TDES0_TTSE         0x02000000
#define EMAC_TDES0_CIC          0x00C00000
#define EMAC_TDES0_TER          0x00200000
#define EMAC_TDES0_TCH          0x00100000
#define EMAC_TDES0_TTSS         0x00020000
#define EMAC_TDES0_IHE          0x00010000
#define EMAC_TDES0_ES           0x00008000
#define EMAC_TDES0_JT           0x00004000
#define EMAC_TDES0_FF           0x00002000
#define EMAC_TDES0_IPE          0x00001000
#define EMAC_TDES0_LOC          0x00000800
#define EMAC_TDES0_NC           0x00000400
#define EMAC_TDES0_LC           0x00000200
#define EMAC_TDES0_EC           0x00000100
#define EMAC_TDES0_VF           0x00000080
#define EMAC_TDES0_CC           0x00000078
#define EMAC_TDES0_ED           0x00000004
#define EMAC_TDES0_UF           0x00000002
#define EMAC_TDES0_DB           0x00000001
#define EMAC_TDES1_TBS2         0x1FFF0000
#define EMAC_TDES1_TBS1         0x00001FFF
#define EMAC_TDES2_TBAP1        0xFFFFFFFF
#define EMAC_TDES3_TBAP2        0xFFFFFFFF

//Receive DMA descriptor flags
#define EMAC_RDES0_OWN          0x80000000
#define EMAC_RDES0_AFM          0x40000000
#define EMAC_RDES0_FL           0x3FFF0000
#define EMAC_RDES0_ES           0x00008000
#define EMAC_RDES0_DE           0x00004000
#define EMAC_RDES0_SAF          0x00002000
#define EMAC_RDES0_LE           0x00001000
#define EMAC_RDES0_OE           0x00000800
#define EMAC_RDES0_VLAN         0x00000400
#define EMAC_RDES0_FS           0x00000200
#define EMAC_RDES0_LS           0x00000100
#define EMAC_RDES0_IPHCE        0x00000080
#define EMAC_RDES0_LC           0x00000040
#define EMAC_RDES0_FT           0x00000020
#define EMAC_RDES0_RWT          0x00000010
#define EMAC_RDES0_RE           0x00000008
#define EMAC_RDES0_DBE          0x00000004
#define EMAC_RDES0_CE           0x00000002
#define EMAC_RDES0_PCE          0x00000001
#define EMAC_RDES1_DIC          0x80000000
#define EMAC_RDES1_RBS2         0x1FFF0000
#define EMAC_RDES1_RER          0x00008000
#define EMAC_RDES1_RCH          0x00004000
#define EMAC_RDES1_RBS1         0x00001FFF
#define EMAC_RDES2_RBAP1        0xFFFFFFFF
#define EMAC_RDES3_RBAP2        0xFFFFFFFF

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
} At32f4xxTxDmaDesc;


/**
 * @brief Enhanced RX DMA descriptor
 **/

typedef struct
{
   uint32_t rdes0;
   uint32_t rdes1;
   uint32_t rdes2;
   uint32_t rdes3;
} At32f4xxRxDmaDesc;


//AT32F4 Ethernet MAC driver
extern const NicDriver at32f4xxEthDriver;

//AT32F4 Ethernet MAC related functions
error_t at32f4xxEthInit(NetInterface *interface);
void at32f4xxEthInitGpio(NetInterface *interface);
void at32f4xxEthInitDmaDesc(NetInterface *interface);

void at32f4xxEthTick(NetInterface *interface);

void at32f4xxEthEnableIrq(NetInterface *interface);
void at32f4xxEthDisableIrq(NetInterface *interface);
void at32f4xxEthEventHandler(NetInterface *interface);

error_t at32f4xxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t at32f4xxEthReceivePacket(NetInterface *interface);

error_t at32f4xxEthUpdateMacAddrFilter(NetInterface *interface);
error_t at32f4xxEthUpdateMacConfig(NetInterface *interface);

void at32f4xxEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t at32f4xxEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

uint32_t at32f4xxEthCalcCrc(const void *data, size_t length);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
