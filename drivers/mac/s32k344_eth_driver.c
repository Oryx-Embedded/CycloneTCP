/**
 * @file s32k344_eth_driver.c
 * @brief NXP S32K344 Ethernet MAC driver
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

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include "S32K344.h"
#include "core/net.h"
#include "drivers/mac/s32k344_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
#pragma location = S32K344_ETH_RAM_SECTION
static uint8_t txBuffer[S32K344_ETH_TX_BUFFER_COUNT][S32K344_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
#pragma location = S32K344_ETH_RAM_SECTION
static uint8_t rxBuffer[S32K344_ETH_RX_BUFFER_COUNT][S32K344_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
#pragma location = S32K344_ETH_RAM_SECTION
static S32k344TxDmaDesc txDmaDesc[S32K344_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
#pragma location = S32K344_ETH_RAM_SECTION
static S32k344RxDmaDesc rxDmaDesc[S32K344_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[S32K344_ETH_TX_BUFFER_COUNT][S32K344_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(S32K344_ETH_RAM_SECTION)));
//Receive buffer
static uint8_t rxBuffer[S32K344_ETH_RX_BUFFER_COUNT][S32K344_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(S32K344_ETH_RAM_SECTION)));
//Transmit DMA descriptors
static S32k344TxDmaDesc txDmaDesc[S32K344_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(S32K344_ETH_RAM_SECTION)));
//Receive DMA descriptors
static S32k344RxDmaDesc rxDmaDesc[S32K344_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(S32K344_ETH_RAM_SECTION)));

#endif

//Current transmit descriptor
static uint_t txIndex;
//Current receive descriptor
static uint_t rxIndex;


/**
 * @brief S32K344 Ethernet MAC driver
 **/

const NicDriver s32k344EthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   s32k344EthInit,
   s32k344EthTick,
   s32k344EthEnableIrq,
   s32k344EthDisableIrq,
   s32k344EthEventHandler,
   s32k344EthSendPacket,
   s32k344EthUpdateMacAddrFilter,
   s32k344EthUpdateMacConfig,
   s32k344EthWritePhyReg,
   s32k344EthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief S32K344 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t s32k344EthInit(NetInterface *interface)
{
   error_t error;
   uint32_t temp;

   //Debug message
   TRACE_INFO("Initializing S32K344 Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   s32k344EthInitGpio(interface);

   //Enable EMAC clock
   IP_MC_ME->PRTN2_COFB1_CLKEN |= MC_ME_PRTN2_COFB1_CLKEN_REQ32_MASK;
   IP_MC_ME->PRTN2_PCONF |= MC_ME_PRTN2_PCONF_PCE_MASK;
   IP_MC_ME->PRTN2_PUPD |= MC_ME_PRTN2_PUPD_PCUD_MASK;
   IP_MC_ME->CTL_KEY = MC_ME_CTL_KEY_KEY(0x5AF0);
   IP_MC_ME->CTL_KEY = MC_ME_CTL_KEY_KEY(0xA50F);

   //Wait for the hardware update process to complete
   while((IP_MC_ME->PRTN2_PUPD & MC_ME_PRTN2_PUPD_PCUD_MASK) != 0)
   {
   }

   //Perform a software reset
   IP_EMAC->DMA_MODE |= EMAC_DMA_MODE_SWR_MASK;
   //Wait for the reset to complete
   while((IP_EMAC->DMA_MODE & EMAC_DMA_MODE_SWR_MASK) != 0)
   {
   }

   //Adjust MDC clock range depending on CSR frequency
   IP_EMAC->MAC_MDIO_ADDRESS = EMAC_MAC_MDIO_ADDRESS_CR(4);

   //Valid Ethernet PHY or switch driver?
   if(interface->phyDriver != NULL)
   {
      //Ethernet PHY initialization
      error = interface->phyDriver->init(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      //Ethernet switch initialization
      error = interface->switchDriver->init(interface);
   }
   else
   {
      //The interface is not properly configured
      error = ERROR_FAILURE;
   }

   //Any error to report?
   if(error)
   {
      return error;
   }

   //Use default MAC configuration
   IP_EMAC->MAC_CONFIGURATION = EMAC_MAC_CONFIGURATION_GPSLCE_MASK |
      EMAC_MAC_CONFIGURATION_PS_MASK | EMAC_MAC_CONFIGURATION_DO_MASK;

   //Set the maximum packet size that can be accepted
   temp = IP_EMAC->MAC_EXT_CONFIGURATION & ~EMAC_MAC_EXT_CONFIGURATION_GPSL_MASK;
   IP_EMAC->MAC_EXT_CONFIGURATION = temp | S32K344_ETH_RX_BUFFER_SIZE;

   //Configure MAC address filtering
   s32k344EthUpdateMacAddrFilter(interface);

   //Disable flow control
   IP_EMAC->MAC_Q0_TX_FLOW_CTRL = 0;
   IP_EMAC->MAC_RX_FLOW_CTRL = 0;

   //Enable the first RX queue
   IP_EMAC->MAC_RXQ_CTRL0 = EMAC_MAC_RXQ_CTRL0_RXQ0EN(2);

   //Configure DMA operating mode
   IP_EMAC->DMA_MODE = EMAC_DMA_MODE_PR(0);
   //Configure system bus mode
   IP_EMAC->DMA_SYSBUS_MODE |= EMAC_DMA_SYSBUS_MODE_AAL_MASK;

   //The DMA takes the descriptor table as contiguous
   IP_EMAC->DMA_CH0_CONTROL = EMAC_DMA_CH0_CONTROL_DSL(0);
   //Configure TX features
   IP_EMAC->DMA_CH0_TX_CONTROL = EMAC_DMA_CH0_TX_CONTROL_TxPBL(32);

   //Configure RX features
   IP_EMAC->DMA_CH0_RX_CONTROL = EMAC_DMA_CH0_RX_CONTROL_RxPBL(32) |
      EMAC_DMA_CH0_RX_CONTROL_RBSZ_13_y(S32K344_ETH_RX_BUFFER_SIZE / 4);

   //Enable store and forward mode for transmission
   IP_EMAC->MTL_TXQ0_OPERATION_MODE |= EMAC_MTL_TXQ0_OPERATION_MODE_TQS(7) |
      EMAC_MTL_TXQ0_OPERATION_MODE_TXQEN(2) | EMAC_MTL_TXQ0_OPERATION_MODE_TSF_MASK;

   //Enable store and forward mode for reception
   IP_EMAC->MTL_RXQ0_OPERATION_MODE |= EMAC_MTL_RXQ0_OPERATION_MODE_RQS(7) |
      EMAC_MTL_RXQ0_OPERATION_MODE_RSF_MASK;

   //Initialize DMA descriptor lists
   s32k344EthInitDmaDesc(interface);

   //Prevent interrupts from being generated when statistic counters reach
   //half their maximum value
   IP_EMAC->MMC_TX_INTERRUPT_MASK = 0xFFFFFFFF;
   IP_EMAC->MMC_RX_INTERRUPT_MASK = 0xFFFFFFFF;
   IP_EMAC->MMC_FPE_TX_INTERRUPT_MASK = 0xFFFFFFFF;
   IP_EMAC->MMC_FPE_RX_INTERRUPT_MASK = 0xFFFFFFFF;

   //Disable MAC interrupts
   IP_EMAC->MAC_INTERRUPT_ENABLE = 0;

   //Enable the desired DMA interrupts
   IP_EMAC->DMA_CH0_INTERRUPT_ENABLE = EMAC_DMA_CH0_INTERRUPT_ENABLE_NIE_MASK |
      EMAC_DMA_CH0_INTERRUPT_ENABLE_RIE_MASK | EMAC_DMA_CH0_INTERRUPT_ENABLE_TIE_MASK;

   //Set priority grouping (3 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(S32K344_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(EMAC_0_IRQn, NVIC_EncodePriority(S32K344_ETH_IRQ_PRIORITY_GROUPING,
      S32K344_ETH_IRQ_GROUP_PRIORITY, S32K344_ETH_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   IP_EMAC->MAC_CONFIGURATION |= EMAC_MAC_CONFIGURATION_TE_MASK |
      EMAC_MAC_CONFIGURATION_RE_MASK;

   //Enable DMA transmission and reception
   IP_EMAC->DMA_CH0_TX_CONTROL |= EMAC_DMA_CH0_TX_CONTROL_ST_MASK;
   IP_EMAC->DMA_CH0_RX_CONTROL |= EMAC_DMA_CH0_RX_CONTROL_SR_MASK;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void s32k344EthInitGpio(NetInterface *interface)
{
//S32K344MINI-EVB evaluation board?
#if defined(USE_S32K344MINI_EVB)
   //Select RMII interface mode
   IP_DCM_GPR->DCMRWF1 |= DCM_GPR_DCMRWF1_RMII_MII_SEL_MASK;

   //Configure EMAC_MII_RMII_MDIO (PTD16)
   IP_SIUL2->MSCR[112] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_IBE_MASK |
      SIUL2_MSCR_PUS_MASK | SIUL2_MSCR_PUE_MASK | SIUL2_MSCR_SSS_2(0) |
      SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(1);
   IP_SIUL2->IMCR[291] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_MDC (PTD17)
   IP_SIUL2->MSCR[113] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SSS_2(0) |
      SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TX_CLK (PTC0)
   IP_SIUL2->MSCR[64] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[296] = SIUL2_IMCR_SSS(4);

   //Configure EMAC_MII_RMII_TX_EN (PTE9)
   IP_SIUL2->MSCR[137] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(1) | SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(0);

   //Configure EMAC_MII_RMII_TXD0 (PTB5)
   IP_SIUL2->MSCR[37] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(0) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TXD1 (PTB4)
   IP_SIUL2->MSCR[36] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(0) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_RX_DV (PTC15)
   IP_SIUL2->MSCR[79] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[292] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_RXD0 (PTD9)
   IP_SIUL2->MSCR[105] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[294] = SIUL2_IMCR_SSS(3);

   //Configure EMAC_MII_RMII_RXD1 (PTD8)
   IP_SIUL2->MSCR[104] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[295] = SIUL2_IMCR_SSS(3);

   //Configure ENET_RESET (PTE21) as an output
   IP_SIUL2->MSCR[149] = SIUL2_MSCR_OBE_MASK;

   //Reset PHY transceiver (hard reset)
   IP_SIUL2->GPDO149 = 0;
   sleep(10);
   IP_SIUL2->GPDO149 = 1;
   sleep(100);

   //Configure RMII clocks
   s32k344EthInitRmiiClock();

//S32K3X4EVB-Q172 evaluation board?
#elif defined(USE_S32K3X4EVB_Q172)
   //Select RMII interface mode
   IP_DCM_GPR->DCMRWF1 |= DCM_GPR_DCMRWF1_RMII_MII_SEL_MASK;

   //Configure EMAC_MII_RMII_MDIO (PTD16)
   IP_SIUL2->MSCR[112] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_IBE_MASK |
      SIUL2_MSCR_PUS_MASK | SIUL2_MSCR_PUE_MASK | SIUL2_MSCR_SSS_2(0) |
      SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(1);
   IP_SIUL2->IMCR[291] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_MDC (PTD17)
   IP_SIUL2->MSCR[113] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SSS_2(0) |
      SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TX_CLK (PTC0)
   IP_SIUL2->MSCR[64] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[296] = SIUL2_IMCR_SSS(4);

   //Configure EMAC_MII_RMII_TX_EN (PTE9)
   IP_SIUL2->MSCR[137] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(1) | SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(0);

   //Configure EMAC_MII_RMII_TXD0 (PTB5)
   IP_SIUL2->MSCR[37] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(0) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TXD1 (PTB4)
   IP_SIUL2->MSCR[36] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(0) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_RX_DV (PTC17)
   IP_SIUL2->MSCR[81] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[292] = SIUL2_IMCR_SSS(1);

   //Configure EMAC_MII_RMII_RXD0 (PTD9)
   IP_SIUL2->MSCR[105] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[294] = SIUL2_IMCR_SSS(3);

   //Configure EMAC_MII_RMII_RXD1 (PTD8)
   IP_SIUL2->MSCR[104] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[295] = SIUL2_IMCR_SSS(3);

   //Configure ENET_RESET (PTE21) as an output
   IP_SIUL2->MSCR[149] = SIUL2_MSCR_OBE_MASK;

   //Reset PHY transceiver (hard reset)
   IP_SIUL2->GPDO149 = 0;
   sleep(10);
   IP_SIUL2->GPDO149 = 1;
   sleep(100);

   //Configure RMII clocks
   s32k344EthInitRmiiClock();

//S32K3X4EVB-T172 evaluation board?
#elif defined(USE_S32K3X4EVB_T172)
   //Select RMII interface mode
   IP_DCM_GPR->DCMRWF1 |= DCM_GPR_DCMRWF1_RMII_MII_SEL_MASK;

   //Configure EMAC_MII_RMII_MDIO (PTB4)
   IP_SIUL2->MSCR[36] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_IBE_MASK |
      SIUL2_MSCR_PUS_MASK | SIUL2_MSCR_PUE_MASK | SIUL2_MSCR_SSS_2(1) |
      SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);
   IP_SIUL2->IMCR[291] = SIUL2_IMCR_SSS(1);

   //Configure EMAC_MII_RMII_MDC (PTB5)
   IP_SIUL2->MSCR[37] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SSS_2(1) |
      SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TX_CLK (PTD11)
   IP_SIUL2->MSCR[107] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[296] = SIUL2_IMCR_SSS(1);

   //Configure EMAC_MII_RMII_TX_EN (PTD12)
   IP_SIUL2->MSCR[108] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(1) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TXD0 (PTC2)
   IP_SIUL2->MSCR[66] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(1) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TXD1 (PTD7)
   IP_SIUL2->MSCR[103] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(1) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_RX_DV (PTC17)
   IP_SIUL2->MSCR[81] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[292] = SIUL2_IMCR_SSS(1);

   //Configure EMAC_MII_RMII_RX_ER (PTC16)
   IP_SIUL2->MSCR[80] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[293] = SIUL2_IMCR_SSS(1);

   //Configure EMAC_MII_RMII_RXD0 (PTC1)
   IP_SIUL2->MSCR[65] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[294] = SIUL2_IMCR_SSS(1);

   //Configure EMAC_MII_RMII_RXD1 (PTC0)
   IP_SIUL2->MSCR[64] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[295] = SIUL2_IMCR_SSS(1);

   //Configure ENET_RSTN (PTA28) as an output
   IP_SIUL2->MSCR[28] = SIUL2_MSCR_OBE_MASK;

   //Reset PHY transceiver (hard reset)
   IP_SIUL2->GPDO28 = 0;
   sleep(10);
   IP_SIUL2->GPDO28 = 1;
   sleep(100);

   //Configure RMII clocks
   s32k344EthInitRmiiClock();

//S32K344-WB evaluation board?
#elif defined(USE_S32K344_WB)
   //Select RMII interface mode
   IP_DCM_GPR->DCMRWF1 |= DCM_GPR_DCMRWF1_RMII_MII_SEL_MASK;

   //Configure EMAC_MII_RMII_MDIO (PTD16)
   IP_SIUL2->MSCR[112] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_IBE_MASK |
      SIUL2_MSCR_PUS_MASK | SIUL2_MSCR_PUE_MASK | SIUL2_MSCR_SSS_2(0) |
      SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(1);
   IP_SIUL2->IMCR[291] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_MDC (PTE8)
   IP_SIUL2->MSCR[136] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SSS_2(1) |
      SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TX_CLK (PTD6)
   IP_SIUL2->MSCR[102] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[296] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_TX_EN (PTE9)
   IP_SIUL2->MSCR[137] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(1) | SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(0);

   //Configure EMAC_MII_RMII_TXD0 (PTB5)
   IP_SIUL2->MSCR[37] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(0) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TXD1 (PTB4)
   IP_SIUL2->MSCR[36] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(0) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_RX_DV (PTC17)
   IP_SIUL2->MSCR[81] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[292] = SIUL2_IMCR_SSS(1);

   //Configure EMAC_MII_RMII_RX_ER (PTC16)
   IP_SIUL2->MSCR[80] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[293] = SIUL2_IMCR_SSS(1);

   //Configure EMAC_MII_RMII_RXD0 (PTC0)
   IP_SIUL2->MSCR[64] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[294] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_RXD1 (PTC1)
   IP_SIUL2->MSCR[65] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[295] = SIUL2_IMCR_SSS(2);

   //Configure RMII clocks
   s32k344EthInitRmiiClock();

//MR-CANHUBK344 evaluation board?
#elif defined(USE_MR_CANHUBK344)
   //Select RMII interface mode
   IP_DCM_GPR->DCMRWF1 |= DCM_GPR_DCMRWF1_RMII_MII_SEL_MASK;

   //Configure EMAC_MII_RMII_MDIO (PTD16)
   IP_SIUL2->MSCR[112] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_IBE_MASK |
      SIUL2_MSCR_PUS_MASK | SIUL2_MSCR_PUE_MASK | SIUL2_MSCR_SSS_2(0) |
      SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(1);
   IP_SIUL2->IMCR[291] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_MDC (PTE8)
   IP_SIUL2->MSCR[136] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SSS_2(1) |
      SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TX_CLK (PTD6)
   IP_SIUL2->MSCR[102] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[296] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_TX_EN (PTE9)
   IP_SIUL2->MSCR[137] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(1) | SIUL2_MSCR_SSS_1(1) | SIUL2_MSCR_SSS_0(0);

   //Configure EMAC_MII_RMII_TXD0 (PTB5)
   IP_SIUL2->MSCR[37] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(0) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_TXD1 (PTB4)
   IP_SIUL2->MSCR[36] = SIUL2_MSCR_SRC(1) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SSS_2(0) | SIUL2_MSCR_SSS_1(0) | SIUL2_MSCR_SSS_0(1);

   //Configure EMAC_MII_RMII_RX_DV (PTC15)
   IP_SIUL2->MSCR[79] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[292] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_RX_ER (PTC14)
   IP_SIUL2->MSCR[78] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[293] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_RXD0 (PTC0)
   IP_SIUL2->MSCR[64] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[294] = SIUL2_IMCR_SSS(2);

   //Configure EMAC_MII_RMII_RXD1 (PTC1)
   IP_SIUL2->MSCR[65] = SIUL2_MSCR_IBE_MASK;
   IP_SIUL2->IMCR[295] = SIUL2_IMCR_SSS(2);

   //Configure ENET_RST_N (PTB22) as an output
   IP_SIUL2->MSCR[54] = SIUL2_MSCR_OBE_MASK;

   //Reset PHY transceiver (hard reset)
   IP_SIUL2->GPDO54 = 0;
   sleep(10);
   IP_SIUL2->GPDO54 = 1;
   sleep(100);

   //Configure RMII clocks
   s32k344EthInitRmiiClock();
#endif
}


/**
 * @brief RMII clock configuration
 * @param[in] interface Underlying network interface
 **/

void s32k344EthInitRmiiClock(void)
{
   //Configure clock mux 7 (EMAC_TX_CLK)
   while((IP_MC_CGM->MUX_7_CSS & MC_CGM_MUX_7_CSS_SWIP_MASK) != 0)
   {
   }

   //Set 1:2 clock divider for clock mux 7
   IP_MC_CGM->MUX_7_DC_0 = MC_CGM_MUX_7_DC_0_DE_MASK | MC_CGM_MUX_7_DC_0_DIV(1);
   //Wait until the divider configuration update is complete
   while((IP_MC_CGM->MUX_7_DIV_UPD_STAT & MC_CGM_MUX_7_DIV_UPD_STAT_DIV_STAT_MASK) != 0)
   {
   }

   //Select EMAC_RMII_TX_CLK as source clock for clock mux 7
   IP_MC_CGM->MUX_7_CSC = MC_CGM_MUX_7_CSC_SELCTL(24) | MC_CGM_MUX_7_CSC_CLK_SW_MASK;
   //Wait until the CLK_SW bit is set in the status register
   while((IP_MC_CGM->MUX_7_CSS & MC_CGM_MUX_7_CSS_CLK_SW_MASK) == 0)
   {
   }

   //Wait until the clock source switching is complete
   while((IP_MC_CGM->MUX_7_CSS & MC_CGM_MUX_7_CSS_SWIP_MASK) != 0)
   {
   }

   //Configure clock mux 8 (EMAC_RX_CLK)
   while((IP_MC_CGM->MUX_8_CSS & MC_CGM_MUX_8_CSS_SWIP_MASK) != 0)
   {
   }

   //Set 1:2 clock divider for clock mux 8
   IP_MC_CGM->MUX_8_DC_0 = MC_CGM_MUX_8_DC_0_DE_MASK | MC_CGM_MUX_8_DC_0_DIV(1);
   //Wait until the divider configuration update is complete
   while((IP_MC_CGM->MUX_8_DIV_UPD_STAT & MC_CGM_MUX_8_DIV_UPD_STAT_DIV_STAT_MASK) != 0)
   {
   }

   //Select EMAC_RMII_TX_CLK as source clock for clock mux 8
   IP_MC_CGM->MUX_8_CSC = MC_CGM_MUX_8_CSC_SELCTL(24) | MC_CGM_MUX_8_CSC_CLK_SW_MASK;
   //Wait until the CLK_SW bit is set in the status register
   while((IP_MC_CGM->MUX_8_CSS & MC_CGM_MUX_8_CSS_CLK_SW_MASK) == 0)
   {
   }

   //Wait until the clock source switching is complete
   while((IP_MC_CGM->MUX_8_CSS & MC_CGM_MUX_8_CSS_SWIP_MASK) != 0)
   {
   }

   //Configure clock mux 9 (EMAC_TS_CLK)
   while((IP_MC_CGM->MUX_9_CSS & MC_CGM_MUX_9_CSS_SWIP_MASK) != 0)
   {
   }

   //Set 1:1 clock divider for clock mux 9
   IP_MC_CGM->MUX_9_DC_0 = MC_CGM_MUX_9_DC_0_DE_MASK | MC_CGM_MUX_9_DC_0_DIV(0);
   //Wait until the divider configuration update is complete
   while((IP_MC_CGM->MUX_9_DIV_UPD_STAT & MC_CGM_MUX_9_DIV_UPD_STAT_DIV_STAT_MASK) != 0)
   {
   }

   //Select EMAC_RMII_TX_CLK as source clock for clock mux 9
   IP_MC_CGM->MUX_9_CSC = MC_CGM_MUX_9_CSC_SELCTL(24) | MC_CGM_MUX_9_CSC_CLK_SW_MASK;
   //Wait until the CLK_SW bit is set in the status register
   while((IP_MC_CGM->MUX_9_CSS & MC_CGM_MUX_9_CSS_CLK_SW_MASK) == 0)
   {
   }

   //Wait until the clock source switching is complete
   while((IP_MC_CGM->MUX_9_CSS & MC_CGM_MUX_9_CSS_SWIP_MASK) != 0)
   {
   }
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void s32k344EthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < S32K344_ETH_TX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the application
      txDmaDesc[i].tdes0 = 0;
      txDmaDesc[i].tdes1 = 0;
      txDmaDesc[i].tdes2 = 0;
      txDmaDesc[i].tdes3 = 0;
   }

   //Initialize TX descriptor index
   txIndex = 0;

   //Initialize RX DMA descriptor list
   for(i = 0; i < S32K344_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = S32K344_ETH_REMAP_ADDR(rxBuffer[i]);
      rxDmaDesc[i].rdes1 = 0;
      rxDmaDesc[i].rdes2 = 0;
      rxDmaDesc[i].rdes3 = EMAC_RDES3_OWN | EMAC_RDES3_IOC | EMAC_RDES3_BUF1V;
   }

   //Initialize RX descriptor index
   rxIndex = 0;

   //Start location of the TX descriptor list
   IP_EMAC->DMA_CH0_TXDESC_LIST_ADDRESS = S32K344_ETH_REMAP_ADDR(&txDmaDesc[0]);
   //Length of the transmit descriptor ring
   IP_EMAC->DMA_CH0_TXDESC_RING_LENGTH = S32K344_ETH_TX_BUFFER_COUNT - 1;

   //Start location of the RX descriptor list
   IP_EMAC->DMA_CH0_RXDESC_LIST_ADDRESS = S32K344_ETH_REMAP_ADDR(&rxDmaDesc[0]);
   //Length of the receive descriptor ring
   IP_EMAC->DMA_CH0_RXDESC_RING_LENGTH = S32K344_ETH_RX_BUFFER_COUNT - 1;
}


/**
 * @brief S32K344 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void s32k344EthTick(NetInterface *interface)
{
   //Valid Ethernet PHY or switch driver?
   if(interface->phyDriver != NULL)
   {
      //Handle periodic operations
      interface->phyDriver->tick(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      //Handle periodic operations
      interface->switchDriver->tick(interface);
   }
   else
   {
      //Just for sanity
   }
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void s32k344EthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(EMAC_0_IRQn);

   //Valid Ethernet PHY or switch driver?
   if(interface->phyDriver != NULL)
   {
      //Enable Ethernet PHY interrupts
      interface->phyDriver->enableIrq(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      //Enable Ethernet switch interrupts
      interface->switchDriver->enableIrq(interface);
   }
   else
   {
      //Just for sanity
   }
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void s32k344EthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(EMAC_0_IRQn);

   //Valid Ethernet PHY or switch driver?
   if(interface->phyDriver != NULL)
   {
      //Disable Ethernet PHY interrupts
      interface->phyDriver->disableIrq(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      //Disable Ethernet switch interrupts
      interface->switchDriver->disableIrq(interface);
   }
   else
   {
      //Just for sanity
   }
}


/**
 * @brief S32K344 Ethernet MAC interrupt service routine
 **/

void EMAC_0_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = IP_EMAC->DMA_CH0_STATUS;

   //Packet transmitted?
   if((status & EMAC_DMA_CH0_STATUS_TI_MASK) != 0)
   {
      //Clear TI interrupt flag
      IP_EMAC->DMA_CH0_STATUS = EMAC_DMA_CH0_STATUS_TI_MASK;

      //Check whether the TX buffer is available for writing
      if((txDmaDesc[txIndex].tdes3 & EMAC_TDES3_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & EMAC_DMA_CH0_STATUS_RI_MASK) != 0)
   {
      //Clear RI interrupt flag
      IP_EMAC->DMA_CH0_STATUS = EMAC_DMA_CH0_STATUS_RI_MASK;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&nicDriverInterface->netContext->event);
   }

   //Clear NIS interrupt flag
   IP_EMAC->DMA_CH0_STATUS = EMAC_DMA_CH0_STATUS_NIS_MASK;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief S32K344 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void s32k344EthEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = s32k344EthReceivePacket(interface);

      //No more data in the receive buffer?
   } while(error != ERROR_BUFFER_EMPTY);
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 * @return Error code
 **/

error_t s32k344EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > S32K344_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txDmaDesc[txIndex].tdes3 & EMAC_TDES3_OWN) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txIndex], buffer, offset, length);

   //Set the start address of the buffer
   txDmaDesc[txIndex].tdes0 = S32K344_ETH_REMAP_ADDR(txBuffer[txIndex]);
   //Write the number of bytes to send
   txDmaDesc[txIndex].tdes2 = EMAC_TDES2_IOC | (length & EMAC_TDES2_B1L);
   //Give the ownership of the descriptor to the DMA
   txDmaDesc[txIndex].tdes3 = EMAC_TDES3_OWN | EMAC_TDES3_FD | EMAC_TDES3_LD;

   //Data synchronization barrier
   __DSB();

   //Clear TBU flag to resume processing
   IP_EMAC->DMA_CH0_STATUS = EMAC_DMA_CH0_STATUS_TBU_MASK;
   //Instruct the DMA to poll the transmit descriptor list
   IP_EMAC->DMA_CH0_TXDESC_TAIL_POINTER = 0;

   //Increment index and wrap around if necessary
   if(++txIndex >= S32K344_ETH_TX_BUFFER_COUNT)
   {
      txIndex = 0;
   }

   //Check whether the next buffer is available for writing
   if((txDmaDesc[txIndex].tdes3 & EMAC_TDES3_OWN) == 0)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
   }

   //Data successfully written
   return NO_ERROR;
}


/**
 * @brief Receive a packet
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t s32k344EthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxDmaDesc[rxIndex].rdes3 & EMAC_RDES3_OWN) == 0)
   {
      //FD and LD flags should be set
      if((rxDmaDesc[rxIndex].rdes3 & EMAC_RDES3_FD) != 0 &&
         (rxDmaDesc[rxIndex].rdes3 & EMAC_RDES3_LD) != 0)
      {
         //Make sure no error occurred
         if((rxDmaDesc[rxIndex].rdes3 & EMAC_RDES3_ES) == 0)
         {
            //Retrieve the length of the frame
            n = rxDmaDesc[rxIndex].rdes3 & EMAC_RDES3_PL;
            //Limit the number of data to read
            n = MIN(n, S32K344_ETH_RX_BUFFER_SIZE);

            //Additional options can be passed to the stack along with the packet
            ancillary = NET_DEFAULT_RX_ANCILLARY;

            //Pass the packet to the upper layer
            nicProcessPacket(interface, rxBuffer[rxIndex], n, &ancillary);

            //Valid packet received
            error = NO_ERROR;
         }
         else
         {
            //The received packet contains an error
            error = ERROR_INVALID_PACKET;
         }
      }
      else
      {
         //The packet is not valid
         error = ERROR_INVALID_PACKET;
      }

      //Set the start address of the buffer
      rxDmaDesc[rxIndex].rdes0 = S32K344_ETH_REMAP_ADDR(rxBuffer[rxIndex]);
      //Give the ownership of the descriptor back to the DMA
      rxDmaDesc[rxIndex].rdes3 = EMAC_RDES3_OWN | EMAC_RDES3_IOC | EMAC_RDES3_BUF1V;

      //Increment index and wrap around if necessary
      if(++rxIndex >= S32K344_ETH_RX_BUFFER_COUNT)
      {
         rxIndex = 0;
      }
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RBU flag to resume processing
   IP_EMAC->DMA_CH0_STATUS = EMAC_DMA_CH0_STATUS_RBU_MASK;
   //Instruct the DMA to poll the receive descriptor list
   IP_EMAC->DMA_CH0_RXDESC_TAIL_POINTER = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t s32k344EthUpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   uint_t k;
   uint32_t crc;
   uint32_t hashTable[2];
   MacAddr unicastMacAddr[2];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Promiscuous mode?
   if(interface->promiscuous)
   {
      //Pass all incoming frames regardless of their destination address
      IP_EMAC->MAC_PACKET_FILTER = EMAC_MAC_PACKET_FILTER_PR_MASK;
   }
   else
   {
      //Set the MAC address of the station
      IP_EMAC->MAC_ADDRESS0_LOW = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
      IP_EMAC->MAC_ADDRESS0_HIGH = interface->macAddr.w[2];

      //The MAC supports 2 additional addresses for unicast perfect filtering
      unicastMacAddr[0] = MAC_UNSPECIFIED_ADDR;
      unicastMacAddr[1] = MAC_UNSPECIFIED_ADDR;

      //The hash table is used for multicast address filtering
      hashTable[0] = 0;
      hashTable[1] = 0;

      //The MAC address filter contains the list of MAC addresses to accept
      //when receiving an Ethernet frame
      for(i = 0, j = 0; i < MAC_ADDR_FILTER_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->macAddrFilter[i];

         //Valid entry?
         if(entry->refCount > 0)
         {
            //Multicast address?
            if(macIsMulticastAddr(&entry->addr))
            {
               //Compute CRC over the current MAC address
               crc = s32k344EthCalcCrc(&entry->addr, sizeof(MacAddr));

               //The upper 6 bits in the CRC register are used to index the
               //contents of the hash table
               k = (crc >> 26) & 0x3F;

               //Update hash table contents
               hashTable[k / 32] |= (1 << (k % 32));
            }
            else
            {
               //Up to 2 additional MAC addresses can be specified
               if(j < 2)
               {
                  //Save the unicast address
                  unicastMacAddr[j++] = entry->addr;
               }
            }
         }
      }

      //Configure the first unicast address filter
      if(j >= 1)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         IP_EMAC->MAC_ADDRESS1_LOW = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
         IP_EMAC->MAC_ADDRESS1_HIGH = unicastMacAddr[0].w[2] | EMAC_MAC_ADDRESS1_HIGH_AE_MASK;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         IP_EMAC->MAC_ADDRESS1_LOW = 0;
         IP_EMAC->MAC_ADDRESS1_HIGH = 0;
      }

      //Configure the second unicast address filter
      if(j >= 2)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         IP_EMAC->MAC_ADDRESS2_LOW = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
         IP_EMAC->MAC_ADDRESS2_HIGH = unicastMacAddr[1].w[2] | EMAC_MAC_ADDRESS2_HIGH_AE_MASK;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         IP_EMAC->MAC_ADDRESS2_LOW = 0;
         IP_EMAC->MAC_ADDRESS2_HIGH = 0;
      }

      //Check whether frames with a multicast destination address should be
      //accepted
      if(interface->acceptAllMulticast)
      {
         //Configure the receive filter
         IP_EMAC->MAC_PACKET_FILTER = EMAC_MAC_PACKET_FILTER_HPF_MASK |
            EMAC_MAC_PACKET_FILTER_PM_MASK;
      }
      else
      {
         //Configure the receive filter
         IP_EMAC->MAC_PACKET_FILTER = EMAC_MAC_PACKET_FILTER_HPF_MASK |
            EMAC_MAC_PACKET_FILTER_HMC_MASK;

         //Configure the multicast hash table
         IP_EMAC->MAC_HASH_TABLE_REG0 = hashTable[0];
         IP_EMAC->MAC_HASH_TABLE_REG1 = hashTable[1];

         //Debug message
         TRACE_DEBUG("  MAC_HASH_TABLE_REG0 = 0x%08" PRIX32 "\r\n", IP_EMAC->MAC_HASH_TABLE_REG0);
         TRACE_DEBUG("  MAC_HASH_TABLE_REG1 = 0x%08" PRIX32 "\r\n", IP_EMAC->MAC_HASH_TABLE_REG1);
      }
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t s32k344EthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = IP_EMAC->MAC_CONFIGURATION;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= EMAC_MAC_CONFIGURATION_FES_MASK;
   }
   else
   {
      config &= ~EMAC_MAC_CONFIGURATION_FES_MASK;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= EMAC_MAC_CONFIGURATION_DM_MASK;
   }
   else
   {
      config &= ~EMAC_MAC_CONFIGURATION_DM_MASK;
   }

   //Update MAC configuration register
   IP_EMAC->MAC_CONFIGURATION = config;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write PHY register
 * @param[in] opcode Access type (2 bits)
 * @param[in] phyAddr PHY address (5 bits)
 * @param[in] regAddr Register address (5 bits)
 * @param[in] data Register value
 **/

void s32k344EthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = IP_EMAC->MAC_MDIO_ADDRESS & EMAC_MAC_MDIO_ADDRESS_CR_MASK;
      //Set up a write operation
      temp |= EMAC_MAC_MDIO_ADDRESS_GOC_0_MASK | EMAC_MAC_MDIO_ADDRESS_GB_MASK;

      //PHY address
      temp |= EMAC_MAC_MDIO_ADDRESS_PA(phyAddr);
      //Register address
      temp |= EMAC_MAC_MDIO_ADDRESS_RDA(regAddr);

      //Data to be written in the PHY register
      IP_EMAC->MAC_MDIO_DATA = data & EMAC_MAC_MDIO_DATA_GD_MASK;

      //Start a write operation
      IP_EMAC->MAC_MDIO_ADDRESS = temp;
      //Wait for the write to complete
      while((IP_EMAC->MAC_MDIO_ADDRESS & EMAC_MAC_MDIO_ADDRESS_GB_MASK) != 0)
      {
      }
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
   }
}


/**
 * @brief Read PHY register
 * @param[in] opcode Access type (2 bits)
 * @param[in] phyAddr PHY address (5 bits)
 * @param[in] regAddr Register address (5 bits)
 * @return Register value
 **/

uint16_t s32k344EthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = IP_EMAC->MAC_MDIO_ADDRESS & EMAC_MAC_MDIO_ADDRESS_CR_MASK;

      //Set up a read operation
      temp |= EMAC_MAC_MDIO_ADDRESS_GOC_1_MASK |
         EMAC_MAC_MDIO_ADDRESS_GOC_0_MASK | EMAC_MAC_MDIO_ADDRESS_GB_MASK;

      //PHY address
      temp |= EMAC_MAC_MDIO_ADDRESS_PA(phyAddr);
      //Register address
      temp |= EMAC_MAC_MDIO_ADDRESS_RDA(regAddr);

      //Start a read operation
      IP_EMAC->MAC_MDIO_ADDRESS = temp;
      //Wait for the read to complete
      while((IP_EMAC->MAC_MDIO_ADDRESS & EMAC_MAC_MDIO_ADDRESS_GB_MASK) != 0)
      {
      }

      //Get register value
      data = IP_EMAC->MAC_MDIO_DATA & EMAC_MAC_MDIO_DATA_GD_MASK;
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
      data = 0;
   }

   //Return the value of the PHY register
   return data;
}


/**
 * @brief CRC calculation
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t s32k344EthCalcCrc(const void *data, size_t length)
{
   uint_t i;
   uint_t j;
   uint32_t crc;
   const uint8_t *p;

   //Point to the data over which to calculate the CRC
   p = (uint8_t *) data;
   //CRC preset value
   crc = 0xFFFFFFFF;

   //Loop through data
   for(i = 0; i < length; i++)
   {
      //The message is processed bit by bit
      for(j = 0; j < 8; j++)
      {
         //Update CRC value
         if((((crc >> 31) ^ (p[i] >> j)) & 0x01) != 0)
         {
            crc = (crc << 1) ^ 0x04C11DB7;
         }
         else
         {
            crc = crc << 1;
         }
      }
   }

   //Return CRC value
   return ~crc;
}
