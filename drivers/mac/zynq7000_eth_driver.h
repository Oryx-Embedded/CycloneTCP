/**
 * @file zynq7000_eth_driver.h
 * @brief Zynq-7000 Gigabit Ethernet MAC driver
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

#ifndef _ZYNQ7000_ETH_DRIVER_H
#define _ZYNQ7000_ETH_DRIVER_H

//Number of TX buffers
#ifndef ZYNQ7000_ETH_TX_BUFFER_COUNT
   #define ZYNQ7000_ETH_TX_BUFFER_COUNT 16
#elif (ZYNQ7000_ETH_TX_BUFFER_COUNT < 1)
   #error ZYNQ7000_ETH_TX_BUFFER_COUNT parameter is not valid
#endif

//TX buffer size
#ifndef ZYNQ7000_ETH_TX_BUFFER_SIZE
   #define ZYNQ7000_ETH_TX_BUFFER_SIZE 1536
#elif (ZYNQ7000_ETH_TX_BUFFER_SIZE != 1536)
   #error ZYNQ7000_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//Number of RX buffers
#ifndef ZYNQ7000_ETH_RX_BUFFER_COUNT
   #define ZYNQ7000_ETH_RX_BUFFER_COUNT 16
#elif (ZYNQ7000_ETH_RX_BUFFER_COUNT < 1)
   #error ZYNQ7000_ETH_RX_BUFFER_COUNT parameter is not valid
#endif

//RX buffer size
#ifndef ZYNQ7000_ETH_RX_BUFFER_SIZE
   #define ZYNQ7000_ETH_RX_BUFFER_SIZE 1536
#elif (ZYNQ7000_ETH_RX_BUFFER_SIZE != 1536)
   #error ZYNQ7000_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Ethernet interrupt priority
#ifndef ZYNQ7000_ETH_IRQ_PRIORITY
   #define ZYNQ7000_ETH_IRQ_PRIORITY 160
#elif (ZYNQ7000_ETH_IRQ_PRIORITY < 0)
   #error ZYNQ7000_ETH_IRQ_PRIORITY parameter is not valid
#endif

//Name of the section where to place DMA buffers
#ifndef ZYNQ7000_ETH_RAM_SECTION
   #define ZYNQ7000_ETH_RAM_SECTION ".ram_no_cache"
#endif

//Macro for hardware access
#define _HW_REG(address) *((volatile uint32_t *) (address))

//XEMACPS registers
#define XSLCR_LOCK             _HW_REG(XSLCR_UNLOCK_ADDR - 4)
#define XSLCR_UNLOCK           _HW_REG(XSLCR_UNLOCK_ADDR)
#define XSLCR_GEM0_RCLK_CTRL   _HW_REG(XSLCR_GEM0_RCLK_CTRL_ADDR)
#define XSLCR_GEM0_CLK_CTRL    _HW_REG(XSLCR_GEM0_CLK_CTRL_ADDR)
#define XEMACPS_NWCTRL         _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_NWCTRL_OFFSET)
#define XEMACPS_NWCFG          _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_NWCFG_OFFSET)
#define XEMACPS_NWSR           _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_NWSR_OFFSET)
#define XEMACPS_DMACR          _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_DMACR_OFFSET)
#define XEMACPS_TXSR           _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXSR_OFFSET)
#define XEMACPS_RXQBASE        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXQBASE_OFFSET)
#define XEMACPS_TXQBASE        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXQBASE_OFFSET)
#define XEMACPS_RXSR           _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXSR_OFFSET)
#define XEMACPS_ISR            _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_ISR_OFFSET)
#define XEMACPS_IER            _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_IER_OFFSET)
#define XEMACPS_IDR            _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_IDR_OFFSET)
#define XEMACPS_IMR            _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_IMR_OFFSET)
#define XEMACPS_PHYMNTNC       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_PHYMNTNC_OFFSET)
#define XEMACPS_RXPAUSE        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXPAUSE_OFFSET)
#define XEMACPS_TXPAUSE        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXPAUSE_OFFSET)
#define XEMACPS_JUMBOMAXLEN    _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_JUMBOMAXLEN_OFFSET)
#define XEMACPS_HASHL          _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_HASHL_OFFSET)
#define XEMACPS_HASHH          _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_HASHH_OFFSET)
#define XEMACPS_LADDR1L        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LADDR1L_OFFSET)
#define XEMACPS_LADDR1H        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LADDR1H_OFFSET)
#define XEMACPS_LADDR2L        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LADDR2L_OFFSET)
#define XEMACPS_LADDR2H        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LADDR2H_OFFSET)
#define XEMACPS_LADDR3L        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LADDR3L_OFFSET)
#define XEMACPS_LADDR3H        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LADDR3H_OFFSET)
#define XEMACPS_LADDR4L        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LADDR4L_OFFSET)
#define XEMACPS_LADDR4H        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LADDR4H_OFFSET)
#define XEMACPS_MATCH1         _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_MATCH1_OFFSET)
#define XEMACPS_MATCH2         _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_MATCH2_OFFSET)
#define XEMACPS_MATCH3         _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_MATCH3_OFFSET)
#define XEMACPS_MATCH4         _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_MATCH4_OFFSET)
#define XEMACPS_STRETCH        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_STRETCH_OFFSET)
#define XEMACPS_OCTTXL         _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_OCTTXL_OFFSET)
#define XEMACPS_OCTTXH         _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_OCTTXH_OFFSET)
#define XEMACPS_TXCNT          _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXCNT_OFFSET)
#define XEMACPS_TXBCCNT        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXBCCNT_OFFSET)
#define XEMACPS_TXMCCNT        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXMCCNT_OFFSET)
#define XEMACPS_TXPAUSECNT     _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXPAUSECNT_OFFSET)
#define XEMACPS_TX64CNT        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TX64CNT_OFFSET)
#define XEMACPS_TX65CNT        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TX65CNT_OFFSET)
#define XEMACPS_TX128CNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TX128CNT_OFFSET)
#define XEMACPS_TX256CNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TX256CNT_OFFSET)
#define XEMACPS_TX512CNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TX512CNT_OFFSET)
#define XEMACPS_TX1024CNT      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TX1024CNT_OFFSET)
#define XEMACPS_TX1519CNT      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TX1519CNT_OFFSET)
#define XEMACPS_TXURUNCNT      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXURUNCNT_OFFSET)
#define XEMACPS_SNGLCOLLCNT    _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_SNGLCOLLCNT_OFFSET)
#define XEMACPS_MULTICOLLCNT   _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_MULTICOLLCNT_OFFSET)
#define XEMACPS_EXCESSCOLLCNT  _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_EXCESSCOLLCNT_OFFSET)
#define XEMACPS_LATECOLLCNT    _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LATECOLLCNT_OFFSET)
#define XEMACPS_TXDEFERCNT     _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXDEFERCNT_OFFSET)
#define XEMACPS_TXCSENSECNT    _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXCSENSECNT_OFFSET)
#define XEMACPS_OCTRXL         _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_OCTRXL_OFFSET)
#define XEMACPS_OCTRXH         _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_OCTRXH_OFFSET)
#define XEMACPS_RXCNT          _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXCNT_OFFSET)
#define XEMACPS_RXBROADCNT     _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXBROADCNT_OFFSET)
#define XEMACPS_RXMULTICNT     _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXMULTICNT_OFFSET)
#define XEMACPS_RXPAUSECNT     _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXPAUSECNT_OFFSET)
#define XEMACPS_RX64CNT        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RX64CNT_OFFSET)
#define XEMACPS_RX65CNT        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RX65CNT_OFFSET)
#define XEMACPS_RX128CNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RX128CNT_OFFSET)
#define XEMACPS_RX256CNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RX256CNT_OFFSET)
#define XEMACPS_RX512CNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RX512CNT_OFFSET)
#define XEMACPS_RX1024CNT      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RX1024CNT_OFFSET)
#define XEMACPS_RX1519CNT      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RX1519CNT_OFFSET)
#define XEMACPS_RXUNDRCNT      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXUNDRCNT_OFFSET)
#define XEMACPS_RXOVRCNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXOVRCNT_OFFSET)
#define XEMACPS_RXJABCNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXJABCNT_OFFSET)
#define XEMACPS_RXFCSCNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXFCSCNT_OFFSET)
#define XEMACPS_RXLENGTHCNT    _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXLENGTHCNT_OFFSET)
#define XEMACPS_RXSYMBCNT      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXSYMBCNT_OFFSET)
#define XEMACPS_RXALIGNCNT     _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXALIGNCNT_OFFSET)
#define XEMACPS_RXRESERRCNT    _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXRESERRCNT_OFFSET)
#define XEMACPS_RXORCNT        _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXORCNT_OFFSET)
#define XEMACPS_RXIPCCNT       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXIPCCNT_OFFSET)
#define XEMACPS_RXTCPCCNT      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXTCPCCNT_OFFSET)
#define XEMACPS_RXUDPCCNT      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXUDPCCNT_OFFSET)
#define XEMACPS_LAST           _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_LAST_OFFSET)
#define XEMACPS_1588_SEC       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_1588_SEC_OFFSET)
#define XEMACPS_1588_NANOSEC   _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_1588_NANOSEC_OFFSET)
#define XEMACPS_1588_ADJ       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_1588_ADJ_OFFSET)
#define XEMACPS_1588_INC       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_1588_INC_OFFSET)
#define XEMACPS_PTP_TXSEC      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_PTP_TXSEC_OFFSET)
#define XEMACPS_PTP_TXNANOSEC  _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_PTP_TXNANOSEC_OFFSET)
#define XEMACPS_PTP_RXSEC      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_PTP_RXSEC_OFFSET)
#define XEMACPS_PTP_RXNANOSEC  _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_PTP_RXNANOSEC_OFFSET)
#define XEMACPS_PTPP_TXSEC     _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_PTPP_TXSEC_OFFSET)
#define XEMACPS_PTPP_TXNANOSEC _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_PTPP_TXNANOSEC_OFFSET)
#define XEMACPS_PTPP_RXSEC     _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_PTPP_RXSEC_OFFSET)
#define XEMACPS_PTPP_RXNANOSEC _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_PTPP_RXNANOSEC_OFFSET)
#define XEMACPS_INTQ1_STS      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_INTQ1_STS_OFFSET)
#define XEMACPS_TXQ1BASE       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_TXQ1BASE_OFFSET)
#define XEMACPS_RXQ1BASE       _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_RXQ1BASE_OFFSET)
#define XEMACPS_MSBBUF_TXQBASE _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_MSBBUF_TXQBASE_OFFSET)
#define XEMACPS_MSBBUF_RXQBASE _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_MSBBUF_RXQBASE_OFFSET)
#define XEMACPS_INTQ1_IER      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_INTQ1_IER_OFFSET)
#define XEMACPS_INTQ1_IDR      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_INTQ1_IDR_OFFSET)
#define XEMACPS_INTQ1_IMR      _HW_REG(XPAR_XEMACPS_0_BASEADDR + XEMACPS_INTQ1_IMR_OFFSET)

//SLCR_LOCK register
#define XSLCR_LOCK_KEY_VALUE             0x0000767B;

//SLCR_UNLOCK register
#define XSLCR_UNLOCK_KEY_VALUE           0x0000DF0D;

//SLCR_GEM0_RCLK_CTRL register
#define XSLCR_GEM0_RCLK_CTRL_SRCSEL_MASK 0x00000010
#define XSLCR_GEM0_RCLK_CTRL_CLKACT_MASK 0x00000001

//SLCR_GEM0_CLK_CTRL register
#define XSLCR_GEM0_CLK_CTRL_DIV1_MASK    0x03F00000
#define XSLCR_GEM0_CLK_CTRL_DIV0_MASK    0x00003F00
#define XSLCR_GEM0_CLK_CTRL_SRCSEL_MASK  0x00000070
#define XSLCR_GEM0_CLK_CTRL_CLKACT_MASK  0x00000001

//PHYMNTNC register
#ifdef XEMACPS_PHYMNTNC_DATA_MASK
   #undef XEMACPS_PHYMNTNC_DATA_MASK
   #define XEMACPS_PHYMNTNC_DATA_MASK    0x0000FFFF
#endif

//TX buffer descriptor flags
#define XEMACPS_TX_USED                  0x80000000
#define XEMACPS_TX_WRAP                  0x40000000
#define XEMACPS_TX_RLE_ERROR             0x20000000
#define XEMACPS_TX_UNDERRUN_ERROR        0x10000000
#define XEMACPS_TX_AHB_ERROR             0x08000000
#define XEMACPS_TX_LATE_COL_ERROR        0x04000000
#define XEMACPS_TX_CHECKSUM_ERROR        0x00700000
#define XEMACPS_TX_NO_CRC                0x00010000
#define XEMACPS_TX_LAST                  0x00008000
#define XEMACPS_TX_LENGTH                0x00003FFF

//RX buffer descriptor flags
#define XEMACPS_RX_ADDRESS               0xFFFFFFFC
#define XEMACPS_RX_WRAP                  0x00000002
#define XEMACPS_RX_OWNERSHIP             0x00000001
#define XEMACPS_RX_BROADCAST             0x80000000
#define XEMACPS_RX_MULTICAST_HASH        0x40000000
#define XEMACPS_RX_UNICAST_HASH          0x20000000
#define XEMACPS_RX_SAR                   0x08000000
#define XEMACPS_RX_SAR_MASK              0x06000000
#define XEMACPS_RX_TYPE_ID               0x01000000
#define XEMACPS_RX_SNAP                  0x01000000
#define XEMACPS_RX_TYPE_ID_MASK          0x00C00000
#define XEMACPS_RX_CHECKSUM_VALID        0x00C00000
#define XEMACPS_RX_VLAN_TAG              0x00200000
#define XEMACPS_RX_PRIORITY_TAG          0x00100000
#define XEMACPS_RX_VLAN_PRIORITY         0x000E0000
#define XEMACPS_RX_CFI                   0x00010000
#define XEMACPS_RX_EOF                   0x00008000
#define XEMACPS_RX_SOF                   0x00004000
#define XEMACPS_RX_LENGTH_MSB            0x00002000
#define XEMACPS_RX_BAD_FCS               0x00002000
#define XEMACPS_RX_LENGTH                0x00001FFF

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
} Zynq7000TxBufferDesc;


/**
 * @brief Receive buffer descriptor
 **/

typedef struct
{
   uint32_t address;
   uint32_t status;
} Zynq7000RxBufferDesc;


//Zynq-7000 Ethernet MAC driver
extern const NicDriver zynq7000EthDriver;

//Zynq-7000 Ethernet MAC related functions
error_t zynq7000EthInit(NetInterface *interface);
void zynq7000EthInitBufferDesc(NetInterface *interface);

void zynq7000EthTick(NetInterface *interface);

void zynq7000EthEnableIrq(NetInterface *interface);
void zynq7000EthDisableIrq(NetInterface *interface);
void zynq7000EthIrqHandler(NetInterface *interface);
void zynq7000EthEventHandler(NetInterface *interface);

error_t zynq7000EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t zynq7000EthReceivePacket(NetInterface *interface);

error_t zynq7000EthUpdateMacAddrFilter(NetInterface *interface);
error_t zynq7000EthUpdateMacConfig(NetInterface *interface);

void zynq7000EthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data);

uint16_t zynq7000EthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
