/**
 * @file stm32f1xx_eth_driver.c
 * @brief STM32F1 Ethernet MAC driver
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

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#ifdef USE_STDPERIPH_DRIVER
   #include "stm32f10x.h"
#else
   #include "stm32f1xx.h"
   #include "stm32f1xx_hal.h"
#endif

#include "core/net.h"
#include "drivers/mac/stm32f1xx_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
static uint8_t txBuffer[STM32F1XX_ETH_TX_BUFFER_COUNT][STM32F1XX_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
static uint8_t rxBuffer[STM32F1XX_ETH_RX_BUFFER_COUNT][STM32F1XX_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
static Stm32f1xxTxDmaDesc txDmaDesc[STM32F1XX_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
static Stm32f1xxRxDmaDesc rxDmaDesc[STM32F1XX_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[STM32F1XX_ETH_TX_BUFFER_COUNT][STM32F1XX_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Receive buffer
static uint8_t rxBuffer[STM32F1XX_ETH_RX_BUFFER_COUNT][STM32F1XX_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Transmit DMA descriptors
static Stm32f1xxTxDmaDesc txDmaDesc[STM32F1XX_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//Receive DMA descriptors
static Stm32f1xxRxDmaDesc rxDmaDesc[STM32F1XX_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//Pointer to the current TX DMA descriptor
static Stm32f1xxTxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static Stm32f1xxRxDmaDesc *rxCurDmaDesc;


/**
 * @brief STM32F1 Ethernet MAC driver
 **/

const NicDriver stm32f1xxEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   stm32f1xxEthInit,
   stm32f1xxEthTick,
   stm32f1xxEthEnableIrq,
   stm32f1xxEthDisableIrq,
   stm32f1xxEthEventHandler,
   stm32f1xxEthSendPacket,
   stm32f1xxEthUpdateMacAddrFilter,
   stm32f1xxEthUpdateMacConfig,
   stm32f1xxEthWritePhyReg,
   stm32f1xxEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief STM32F1 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t stm32f1xxEthInit(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing STM32F1 Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   stm32f1xxEthInitGpio(interface);

#ifdef USE_STDPERIPH_DRIVER
   //Enable Ethernet MAC clock
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC |
      RCC_AHBPeriph_ETH_MAC_Tx | RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

   //Reset Ethernet MAC peripheral
   RCC_AHBPeriphResetCmd(RCC_AHBPeriph_ETH_MAC, ENABLE);
   RCC_AHBPeriphResetCmd(RCC_AHBPeriph_ETH_MAC, DISABLE);
#else
   //Enable Ethernet MAC clock
   __HAL_RCC_ETHMAC_CLK_ENABLE();
   __HAL_RCC_ETHMACTX_CLK_ENABLE();
   __HAL_RCC_ETHMACRX_CLK_ENABLE();

   //Reset Ethernet MAC peripheral
   __HAL_RCC_ETHMAC_FORCE_RESET();
   __HAL_RCC_ETHMAC_RELEASE_RESET();
#endif

   //Perform a software reset
   ETH->DMABMR |= ETH_DMABMR_SR;
   //Wait for the reset to complete
   while((ETH->DMABMR & ETH_DMABMR_SR) != 0)
   {
   }

   //Adjust MDC clock range depending on HCLK frequency
   ETH->MACMIIAR = ETH_MACMIIAR_CR_DIV42;

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
   ETH->MACCR = ETH_MACCR_RESERVED15 | ETH_MACCR_ROD;

   //Configure MAC address filtering
   stm32f1xxEthUpdateMacAddrFilter(interface);

   //Disable flow control
   ETH->MACFCR = 0;
   //Enable store and forward mode
   ETH->DMAOMR = ETH_DMAOMR_RSF | ETH_DMAOMR_TSF;

   //Configure DMA bus mode
   ETH->DMABMR = ETH_DMABMR_AAB | ETH_DMABMR_USP | ETH_DMABMR_RDP_32Beat |
      ETH_DMABMR_RTPR_1_1 | ETH_DMABMR_PBL_32Beat;

   //Initialize DMA descriptor lists
   stm32f1xxEthInitDmaDesc(interface);

   //Prevent interrupts from being generated when the transmit statistic
   //counters reach half their maximum value
   ETH->MMCTIMR = ETH_MMCTIMR_TGFM | ETH_MMCTIMR_TGFMSCM | ETH_MMCTIMR_TGFSCM;

   //Prevent interrupts from being generated when the receive statistic
   //counters reach half their maximum value
   ETH->MMCRIMR = ETH_MMCRIMR_RGUFM | ETH_MMCRIMR_RFAEM | ETH_MMCRIMR_RFCEM;

   //Disable MAC interrupts
   ETH->MACIMR = ETH_MACIMR_TSTIM | ETH_MACIMR_PMTIM;
   //Enable the desired DMA interrupts
   ETH->DMAIER = ETH_DMAIER_NISE | ETH_DMAIER_RIE | ETH_DMAIER_TIE;

   //Set priority grouping (4 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(STM32F1XX_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(ETH_IRQn, NVIC_EncodePriority(STM32F1XX_ETH_IRQ_PRIORITY_GROUPING,
      STM32F1XX_ETH_IRQ_GROUP_PRIORITY, STM32F1XX_ETH_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   ETH->MACCR |= ETH_MACCR_TE | ETH_MACCR_RE;
   //Enable DMA transmission and reception
   ETH->DMAOMR |= ETH_DMAOMR_ST | ETH_DMAOMR_SR;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void stm32f1xxEthInitGpio(NetInterface *interface)
{
//STM3210C-EVAL evaluation board?
#if defined(USE_STM3210C_EVAL)
   GPIO_InitTypeDef GPIO_InitStructure;

   //Enable AFIO clock
   __HAL_RCC_AFIO_CLK_ENABLE();

   //Enable GPIO clocks
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();

   //Configure MCO (PA8) as an output
   GPIO_InitStructure.Pin = GPIO_PIN_8;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   //Configure MCO pin to output the HSE clock (25MHz)
   HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, 1);

   //Select MII interface mode
   __HAL_AFIO_ETH_MII();

   //Configure MII_MDIO (PA2)
   GPIO_InitStructure.Pin = GPIO_PIN_2;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   //Configure ETH_MII_TXD3 (PB8), MII_TX_EN (PB11), MII_TXD0 (PB12)
   //and MII_TXD1 (PB13)
   GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

   //Configure MII_MDC (PC1) and MII_TXD2 (PC2)
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

   //Configure ETH_MII_CRS (PA0), ETH_MII_RX_CLK (PA1) and ETH_MII_COL (PA3)
   GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_INPUT;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   //Configure ETH_MII_RX_ER (PB10)
   GPIO_InitStructure.Pin = GPIO_PIN_10;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_INPUT;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

   //Configure ETH_MII_TX_CLK (PC3)
   GPIO_InitStructure.Pin = GPIO_PIN_3;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_INPUT;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

   //Configure ETH_MII_RX_DV (PD8), ETH_MII_RXD0 (PD9), ETH_MII_RXD1 (PD10),
   //ETH_MII_RXD2 (PD11) and ETH_MII_RXD3 (PD12)
   GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_INPUT;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

   //Remap Ethernet pins
   __HAL_AFIO_REMAP_ETH_ENABLE();

//STM32-P107 evaluation board?
#elif defined(USE_STM32_P107)
   uint32_t temp;
   GPIO_InitTypeDef GPIO_InitStructure;

   //Enable AFIO clock
   __HAL_RCC_AFIO_CLK_ENABLE();

   //Enable GPIO clocks
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();

   //Configure MCO (PA8) as an output
   GPIO_InitStructure.Pin = GPIO_PIN_8;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   //Configure PLL3 to output a 50MHz clock
   temp = RCC->CFGR2 & ~RCC_CFGR2_PLL3MUL;
   RCC->CFGR2 = temp | RCC_CFGR2_PLL3MUL10;

   //Enable PLL3
   RCC->CR |= RCC_CR_PLL3ON;

   //Wait for the PLL3 to lock
   while((RCC->CR & RCC_CR_PLL3RDY) == 0)
   {
   }

   //Configure MCO pin to output the PLL3 clock
   HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLL3CLK, 1);

   //Select RMII interface mode
   __HAL_AFIO_ETH_RMII();

   //Configure MII_MDIO (PA2)
   GPIO_InitStructure.Pin = GPIO_PIN_2;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   //Configure ETH_RMII_TX_EN (PB11), ETH_RMII_TXD0 (PB12) and ETH_RMII_TXD1 (PB13)
   GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

   //Configure ETH_MDC (PC1)
   GPIO_InitStructure.Pin = GPIO_PIN_1;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

   //Configure ETH_RMII_REF_CLK (PA1) and ETH_RMII_CRS_DV (PA7)
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_7;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_INPUT;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   //Configure ETH_RMII_RXD0 (PC4) and ETH_RMII_RXD1 (PC5)
   GPIO_InitStructure.Pin = GPIO_PIN_4 | GPIO_PIN_5;
   GPIO_InitStructure.Mode = GPIO_MODE_AF_INPUT;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

   //Do not remap Ethernet pins
   __HAL_AFIO_REMAP_ETH_DISABLE();
#endif
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void stm32f1xxEthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < STM32F1XX_ETH_TX_BUFFER_COUNT; i++)
   {
      //Use chain structure rather than ring structure
      txDmaDesc[i].tdes0 = ETH_TDES0_IC | ETH_TDES0_TCH;
      //Initialize transmit buffer size
      txDmaDesc[i].tdes1 = 0;
      //Transmit buffer address
      txDmaDesc[i].tdes2 = (uint32_t) txBuffer[i];
      //Next descriptor address
      txDmaDesc[i].tdes3 = (uint32_t) &txDmaDesc[i + 1];
   }

   //The last descriptor is chained to the first entry
   txDmaDesc[i - 1].tdes3 = (uint32_t) &txDmaDesc[0];
   //Point to the very first descriptor
   txCurDmaDesc = &txDmaDesc[0];

   //Initialize RX DMA descriptor list
   for(i = 0; i < STM32F1XX_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = ETH_RDES0_OWN;
      //Use chain structure rather than ring structure
      rxDmaDesc[i].rdes1 = ETH_RDES1_RCH | (STM32F1XX_ETH_RX_BUFFER_SIZE & ETH_RDES1_RBS1);
      //Receive buffer address
      rxDmaDesc[i].rdes2 = (uint32_t) rxBuffer[i];
      //Next descriptor address
      rxDmaDesc[i].rdes3 = (uint32_t) &rxDmaDesc[i + 1];
   }

   //The last descriptor is chained to the first entry
   rxDmaDesc[i - 1].rdes3 = (uint32_t) &rxDmaDesc[0];
   //Point to the very first descriptor
   rxCurDmaDesc = &rxDmaDesc[0];

   //Start location of the TX descriptor list
   ETH->DMATDLAR = (uint32_t) txDmaDesc;
   //Start location of the RX descriptor list
   ETH->DMARDLAR = (uint32_t) rxDmaDesc;
}


/**
 * @brief STM32F1 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void stm32f1xxEthTick(NetInterface *interface)
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

void stm32f1xxEthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(ETH_IRQn);

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

void stm32f1xxEthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(ETH_IRQn);

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
 * @brief STM32F1 Ethernet MAC interrupt service routine
 **/

void ETH_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = ETH->DMASR;

   //Packet transmitted?
   if((status & ETH_DMASR_TS) != 0)
   {
      //Clear TS interrupt flag
      ETH->DMASR = ETH_DMASR_TS;

      //Check whether the TX buffer is available for writing
      if((txCurDmaDesc->tdes0 & ETH_TDES0_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & ETH_DMASR_RS) != 0)
   {
      //Clear RS interrupt flag
      ETH->DMASR = ETH_DMASR_RS;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   ETH->DMASR = ETH_DMASR_NIS;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief STM32F1 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void stm32f1xxEthEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = stm32f1xxEthReceivePacket(interface);

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

error_t stm32f1xxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > STM32F1XX_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txCurDmaDesc->tdes0 & ETH_TDES0_OWN) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead((uint8_t *) txCurDmaDesc->tdes2, buffer, offset, length);

   //Write the number of bytes to send
   txCurDmaDesc->tdes1 = length & ETH_TDES1_TBS1;
   //Set LS and FS flags as the data fits in a single buffer
   txCurDmaDesc->tdes0 |= ETH_TDES0_LS | ETH_TDES0_FS;
   //Give the ownership of the descriptor to the DMA
   txCurDmaDesc->tdes0 |= ETH_TDES0_OWN;

   //Clear TBUS flag to resume processing
   ETH->DMASR = ETH_DMASR_TBUS;
   //Instruct the DMA to poll the transmit descriptor list
   ETH->DMATPDR = 0;

   //Point to the next descriptor in the list
   txCurDmaDesc = (Stm32f1xxTxDmaDesc *) txCurDmaDesc->tdes3;

   //Check whether the next buffer is available for writing
   if((txCurDmaDesc->tdes0 & ETH_TDES0_OWN) == 0)
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

error_t stm32f1xxEthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxCurDmaDesc->rdes0 & ETH_RDES0_OWN) == 0)
   {
      //FS and LS flags should be set
      if((rxCurDmaDesc->rdes0 & ETH_RDES0_FS) != 0 &&
         (rxCurDmaDesc->rdes0 & ETH_RDES0_LS) != 0)
      {
         //Make sure no error occurred
         if((rxCurDmaDesc->rdes0 & ETH_RDES0_ES) == 0)
         {
            //Retrieve the length of the frame
            n = (rxCurDmaDesc->rdes0 & ETH_RDES0_FL) >> 16;
            //Limit the number of data to read
            n = MIN(n, STM32F1XX_ETH_RX_BUFFER_SIZE);

            //Additional options can be passed to the stack along with the packet
            ancillary = NET_DEFAULT_RX_ANCILLARY;

            //Pass the packet to the upper layer
            nicProcessPacket(interface, (uint8_t *) rxCurDmaDesc->rdes2, n,
               &ancillary);

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

      //Give the ownership of the descriptor back to the DMA
      rxCurDmaDesc->rdes0 = ETH_RDES0_OWN;
      //Point to the next descriptor in the list
      rxCurDmaDesc = (Stm32f1xxRxDmaDesc *) rxCurDmaDesc->rdes3;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RBUS flag to resume processing
   ETH->DMASR = ETH_DMASR_RBUS;
   //Instruct the DMA to poll the receive descriptor list
   ETH->DMARPDR = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t stm32f1xxEthUpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   uint_t k;
   uint32_t crc;
   uint32_t hashTable[2];
   MacAddr unicastMacAddr[3];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Promiscuous mode?
   if(interface->promiscuous)
   {
      //Pass all incoming frames regardless of their destination address
      ETH->MACFFR = ETH_MACFFR_PM;
   }
   else
   {
      //Set the MAC address of the station
      ETH->MACA0LR = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
      ETH->MACA0HR = interface->macAddr.w[2];

      //The MAC supports 3 additional addresses for unicast perfect filtering
      unicastMacAddr[0] = MAC_UNSPECIFIED_ADDR;
      unicastMacAddr[1] = MAC_UNSPECIFIED_ADDR;
      unicastMacAddr[2] = MAC_UNSPECIFIED_ADDR;

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
               crc = stm32f1xxEthCalcCrc(&entry->addr, sizeof(MacAddr));

               //The upper 6 bits in the CRC register are used to index the
               //contents of the hash table
               k = (crc >> 26) & 0x3F;

               //Update hash table contents
               hashTable[k / 32] |= (1 << (k % 32));
            }
            else
            {
               //Up to 3 additional MAC addresses can be specified
               if(j < 3)
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
         ETH->MACA1LR = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
         ETH->MACA1HR = unicastMacAddr[0].w[2] | ETH_MACA1HR_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         ETH->MACA1LR = 0;
         ETH->MACA1HR = 0;
      }

      //Configure the second unicast address filter
      if(j >= 2)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         ETH->MACA2LR = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
         ETH->MACA2HR = unicastMacAddr[1].w[2] | ETH_MACA2HR_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         ETH->MACA2LR = 0;
         ETH->MACA2HR = 0;
      }

      //Configure the third unicast address filter
      if(j >= 3)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         ETH->MACA3LR = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
         ETH->MACA3HR = unicastMacAddr[2].w[2] | ETH_MACA3HR_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         ETH->MACA3LR = 0;
         ETH->MACA3HR = 0;
      }

      //Check whether frames with a multicast destination address should be
      //accepted
      if(interface->acceptAllMulticast)
      {
         //Configure the receive filter
         ETH->MACFFR = ETH_MACFFR_HPF | ETH_MACFFR_PAM;
      }
      else
      {
         //Configure the receive filter
         ETH->MACFFR = ETH_MACFFR_HPF | ETH_MACFFR_HM;

         //Configure the multicast hash table
         ETH->MACHTLR = hashTable[0];
         ETH->MACHTHR = hashTable[1];

         //Debug message
         TRACE_DEBUG("  MACHTLR = %08" PRIX32 "\r\n", ETH->MACHTLR);
         TRACE_DEBUG("  MACHTHR = %08" PRIX32 "\r\n", ETH->MACHTHR);
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

error_t stm32f1xxEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = ETH->MACCR;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= ETH_MACCR_FES;
   }
   else
   {
      config &= ~ETH_MACCR_FES;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= ETH_MACCR_DM;
   }
   else
   {
      config &= ~ETH_MACCR_DM;
   }

   //Update MAC configuration register
   ETH->MACCR = config;

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

void stm32f1xxEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = ETH->MACMIIAR & ETH_MACMIIAR_CR;
      //Set up a write operation
      temp |= ETH_MACMIIAR_MW | ETH_MACMIIAR_MB;
      //PHY address
      temp |= (phyAddr << 11) & ETH_MACMIIAR_PA;
      //Register address
      temp |= (regAddr << 6) & ETH_MACMIIAR_MR;

      //Data to be written in the PHY register
      ETH->MACMIIDR = data & ETH_MACMIIDR_MD;

      //Start a write operation
      ETH->MACMIIAR = temp;
      //Wait for the write to complete
      while((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
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

uint16_t stm32f1xxEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = ETH->MACMIIAR & ETH_MACMIIAR_CR;
      //Set up a read operation
      temp |= ETH_MACMIIAR_MB;
      //PHY address
      temp |= (phyAddr << 11) & ETH_MACMIIAR_PA;
      //Register address
      temp |= (regAddr << 6) & ETH_MACMIIAR_MR;

      //Start a read operation
      ETH->MACMIIAR = temp;
      //Wait for the read to complete
      while((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0)
      {
      }

      //Get register value
      data = ETH->MACMIIDR & ETH_MACMIIDR_MD;
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

uint32_t stm32f1xxEthCalcCrc(const void *data, size_t length)
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
