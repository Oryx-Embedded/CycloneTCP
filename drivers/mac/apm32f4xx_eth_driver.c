/**
 * @file apm32f4xx_eth_driver.c
 * @brief APM32F4 Ethernet MAC driver
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
#include "apm32f4xx.h"
#include "apm32f4xx_rcm.h"
#include "apm32f4xx_syscfg.h"
#include "apm32f4xx_gpio.h"
#include "core/net.h"
#include "drivers/mac/apm32f4xx_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
static uint8_t txBuffer[APM32F4XX_ETH_TX_BUFFER_COUNT][APM32F4XX_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
static uint8_t rxBuffer[APM32F4XX_ETH_RX_BUFFER_COUNT][APM32F4XX_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
static Apm32f4xxTxDmaDesc txDmaDesc[APM32F4XX_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
static Apm32f4xxRxDmaDesc rxDmaDesc[APM32F4XX_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[APM32F4XX_ETH_TX_BUFFER_COUNT][APM32F4XX_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Receive buffer
static uint8_t rxBuffer[APM32F4XX_ETH_RX_BUFFER_COUNT][APM32F4XX_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Transmit DMA descriptors
static Apm32f4xxTxDmaDesc txDmaDesc[APM32F4XX_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//Receive DMA descriptors
static Apm32f4xxRxDmaDesc rxDmaDesc[APM32F4XX_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//Pointer to the current TX DMA descriptor
static Apm32f4xxTxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static Apm32f4xxRxDmaDesc *rxCurDmaDesc;


/**
 * @brief APM32F4 Ethernet MAC driver
 **/

const NicDriver apm32f4xxEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   apm32f4xxEthInit,
   apm32f4xxEthTick,
   apm32f4xxEthEnableIrq,
   apm32f4xxEthDisableIrq,
   apm32f4xxEthEventHandler,
   apm32f4xxEthSendPacket,
   apm32f4xxEthUpdateMacAddrFilter,
   apm32f4xxEthUpdateMacConfig,
   apm32f4xxEthWritePhyReg,
   apm32f4xxEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief APM32F4 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t apm32f4xxEthInit(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing APM32F4 Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   apm32f4xxEthInitGpio(interface);

   //Enable Ethernet MAC clock
   RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_ETH_MAC |
      RCM_AHB1_PERIPH_ETH_MAC_Tx | RCM_AHB1_PERIPH_ETH_MAC_Rx);

   //Reset Ethernet MAC peripheral
   RCM_EnableAHB1PeriphReset(RCM_AHB1_PERIPH_ETH_MAC);
   RCM_DisableAHB1PeriphReset(RCM_AHB1_PERIPH_ETH_MAC);

   //Perform a software reset
   ETH->DMABMOD |= ETH_DMABMOD_SWR;
   //Wait for the reset to complete
   while((ETH->DMABMOD & ETH_DMABMOD_SWR) != 0)
   {
   }

   //Adjust MDC clock range depending on HCLK frequency
   ETH->ADDR = ETH_ADDR_CR_DIV_102;

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
   ETH->CFG = ETH_CFG_RESERVED15 | ETH_CFG_DISRXO;

   //Set the MAC address of the station
   ETH->ADDR0L = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ETH->ADDR0H = interface->macAddr.w[2] | ETH_ADDR0H_AL1;

   //The MAC supports 3 additional addresses for unicast perfect filtering
   ETH->ADDR1L = 0;
   ETH->ADDR1H = 0;
   ETH->ADDR2L = 0;
   ETH->ADDR2H = 0;
   ETH->ADDR3L = 0;
   ETH->ADDR3H = 0;

   //Initialize hash table
   ETH->HTL = 0;
   ETH->HTH = 0;

   //Configure the receive filter
   ETH->FRAF = ETH_FRAF_HPF | ETH_FRAF_HMC;
   //Disable flow control
   ETH->FCTRL = 0;
   //Enable store and forward mode
   ETH->DMAOPMOD = ETH_DMAOPMOD_RXSF | ETH_DMAOPMOD_TXSF;

   //Configure DMA bus mode
   ETH->DMABMOD = ETH_DMABMOD_AAL | ETH_DMABMOD_USP | ETH_DMABMOD_RPBL_32 |
      ETH_DMABMOD_PR_1_1 | ETH_DMABMOD_PBL_32 | ETH_DMABMOD_EDFEN;

   //Initialize DMA descriptor lists
   apm32f4xxEthInitDmaDesc(interface);

   //Prevent interrupts from being generated when the transmit statistic
   //counters reach half their maximum value
   ETH->TXINT = ETH_TXINT_TXGF | ETH_TXINT_TXGFMCOL | ETH_TXINT_TXGFSCOL;

   //Prevent interrupts from being generated when the receive statistic
   //counters reach half their maximum value
   ETH->RXINT = ETH_RXINT_RXGUNF | ETH_RXINT_RXFAE | ETH_RXINT_RXFCE;

   //Disable MAC interrupts
   ETH->IMASK = ETH_IMASK_TSTIM | ETH_IMASK_PMTIM;
   //Enable the desired DMA interrupts
   ETH->DMAINTEN = ETH_DMAINTEN_NINTSEN | ETH_DMAINTEN_RXIEN | ETH_DMAINTEN_TXIEN;

   //Set priority grouping (4 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(APM32F4XX_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(ETH_IRQn, NVIC_EncodePriority(APM32F4XX_ETH_IRQ_PRIORITY_GROUPING,
      APM32F4XX_ETH_IRQ_GROUP_PRIORITY, APM32F4XX_ETH_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   ETH->CFG |= ETH_CFG_TXEN | ETH_CFG_RXEN;
   //Enable DMA transmission and reception
   ETH->DMAOPMOD |= ETH_DMAOPMOD_STTX | ETH_DMAOPMOD_STRX;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void apm32f4xxEthInitGpio(NetInterface *interface)
{
//APM32F407IG Tiny Board?
#if defined(USE_APM32F407IG_TINY_BOARD)
   GPIO_Config_T gpioConfig;

   //Enable SYSCFG clock
   RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_SYSCFG);

   //Enable GPIO clocks
   RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOA);
   RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOC);
   RCM_EnableAHB1PeriphClock(RCM_AHB1_PERIPH_GPIOG);

   //Select RMII interface mode
   SYSCFG_ConfigMediaInterface(SYSCFG_INTERFACE_RMII);

   //Configure RMII pins
   gpioConfig.mode = GPIO_MODE_AF;
   gpioConfig.otype = GPIO_OTYPE_PP;
   gpioConfig.pupd = GPIO_PUPD_NOPULL;
   gpioConfig.speed = GPIO_SPEED_100MHz;

   //Configure ETH_RMII_REF_CLK (PA1), ETH_MDIO (PA2) and ETH_RMII_CRS_DV (PA7)
   gpioConfig.pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
   GPIO_Config(GPIOA, &gpioConfig);

   //Configure ETH_MDC (PC1), ETH_RMII_RXD0 (PC4) and ETH_RMII_RXD1 (PC5)
   gpioConfig.pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
   GPIO_Config(GPIOC, &gpioConfig);

   //Configure ETH_RMII_TX_EN (PG11), ETH_RMII_TXD0 (PG13) and
   //ETH_RMII_TXD1 (PG14)
   gpioConfig.pin = GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
   GPIO_Config(GPIOG, &gpioConfig);

   //Remap Ethernet pins
   GPIO_ConfigPinAF(GPIOA, GPIO_PIN_SOURCE_1, GPIO_AF_ETH);
   GPIO_ConfigPinAF(GPIOA, GPIO_PIN_SOURCE_2, GPIO_AF_ETH);
   GPIO_ConfigPinAF(GPIOA, GPIO_PIN_SOURCE_7, GPIO_AF_ETH);
   GPIO_ConfigPinAF(GPIOC, GPIO_PIN_SOURCE_1, GPIO_AF_ETH);
   GPIO_ConfigPinAF(GPIOC, GPIO_PIN_SOURCE_4, GPIO_AF_ETH);
   GPIO_ConfigPinAF(GPIOC, GPIO_PIN_SOURCE_5, GPIO_AF_ETH);
   GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_11, GPIO_AF_ETH);
   GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_13, GPIO_AF_ETH);
   GPIO_ConfigPinAF(GPIOG, GPIO_PIN_SOURCE_14, GPIO_AF_ETH);
#endif
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void apm32f4xxEthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < APM32F4XX_ETH_TX_BUFFER_COUNT; i++)
   {
      //Use chain structure rather than ring structure
      txDmaDesc[i].tdes0 = ETH_TXDES0_INTC | ETH_TXDES0_TXCH;
      //Initialize transmit buffer size
      txDmaDesc[i].tdes1 = 0;
      //Transmit buffer address
      txDmaDesc[i].tdes2 = (uint32_t) txBuffer[i];
      //Next descriptor address
      txDmaDesc[i].tdes3 = (uint32_t) &txDmaDesc[i + 1];
      //Reserved fields
      txDmaDesc[i].tdes4 = 0;
      txDmaDesc[i].tdes5 = 0;
      //Transmit frame time stamp
      txDmaDesc[i].tdes6 = 0;
      txDmaDesc[i].tdes7 = 0;
   }

   //The last descriptor is chained to the first entry
   txDmaDesc[i - 1].tdes3 = (uint32_t) &txDmaDesc[0];
   //Point to the very first descriptor
   txCurDmaDesc = &txDmaDesc[0];

   //Initialize RX DMA descriptor list
   for(i = 0; i < APM32F4XX_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = ETH_RXDES0_OWN;
      //Use chain structure rather than ring structure
      rxDmaDesc[i].rdes1 = ETH_RXDES1_RXCH | (APM32F4XX_ETH_RX_BUFFER_SIZE & ETH_RXDES1_RBS1);
      //Receive buffer address
      rxDmaDesc[i].rdes2 = (uint32_t) rxBuffer[i];
      //Next descriptor address
      rxDmaDesc[i].rdes3 = (uint32_t) &rxDmaDesc[i + 1];
      //Extended status
      rxDmaDesc[i].rdes4 = 0;
      //Reserved field
      rxDmaDesc[i].rdes5 = 0;
      //Receive frame time stamp
      rxDmaDesc[i].rdes6 = 0;
      rxDmaDesc[i].rdes7 = 0;
   }

   //The last descriptor is chained to the first entry
   rxDmaDesc[i - 1].rdes3 = (uint32_t) &rxDmaDesc[0];
   //Point to the very first descriptor
   rxCurDmaDesc = &rxDmaDesc[0];

   //Start location of the TX descriptor list
   ETH->DMATXDLADDR = (uint32_t) txDmaDesc;
   //Start location of the RX descriptor list
   ETH->DMARXDLADDR = (uint32_t) rxDmaDesc;
}


/**
 * @brief APM32F4 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void apm32f4xxEthTick(NetInterface *interface)
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

void apm32f4xxEthEnableIrq(NetInterface *interface)
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

void apm32f4xxEthDisableIrq(NetInterface *interface)
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
 * @brief APM32F4 Ethernet MAC interrupt service routine
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
   status = ETH->DMASTS;

   //Packet transmitted?
   if((status & ETH_DMASTS_TXFLG) != 0)
   {
      //Clear TXFLG interrupt flag
      ETH->DMASTS = ETH_DMASTS_TXFLG;

      //Check whether the TX buffer is available for writing
      if((txCurDmaDesc->tdes0 & ETH_TXDES0_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & ETH_DMASTS_RXFLG) != 0)
   {
      //Clear RXFLG interrupt flag
      ETH->DMASTS = ETH_DMASTS_RXFLG;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   ETH->DMASTS = ETH_DMASTS_NINTS;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief APM32F4 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void apm32f4xxEthEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = apm32f4xxEthReceivePacket(interface);

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

error_t apm32f4xxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > APM32F4XX_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txCurDmaDesc->tdes0 & ETH_TXDES0_OWN) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead((uint8_t *) txCurDmaDesc->tdes2, buffer, offset, length);

   //Write the number of bytes to send
   txCurDmaDesc->tdes1 = length & ETH_TXDES1_TXBS1;
   //Set LS and FS flags as the data fits in a single buffer
   txCurDmaDesc->tdes0 |= ETH_TXDES0_LS | ETH_TXDES0_FS;
   //Give the ownership of the descriptor to the DMA
   txCurDmaDesc->tdes0 |= ETH_TXDES0_OWN;

   //Clear TXBU flag to resume processing
   ETH->DMASTS = ETH_DMASTS_TXBU;
   //Instruct the DMA to poll the transmit descriptor list
   ETH->DMATXPD = 0;

   //Point to the next descriptor in the list
   txCurDmaDesc = (Apm32f4xxTxDmaDesc *) txCurDmaDesc->tdes3;

   //Check whether the next buffer is available for writing
   if((txCurDmaDesc->tdes0 & ETH_TXDES0_OWN) == 0)
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

error_t apm32f4xxEthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxCurDmaDesc->rdes0 & ETH_RXDES0_OWN) == 0)
   {
      //FS and LS flags should be set
      if((rxCurDmaDesc->rdes0 & ETH_RXDES0_FDES) != 0 &&
         (rxCurDmaDesc->rdes0 & ETH_RXDES0_LDES) != 0)
      {
         //Make sure no error occurred
         if((rxCurDmaDesc->rdes0 & ETH_RXDES0_ERRS) == 0)
         {
            //Retrieve the length of the frame
            n = (rxCurDmaDesc->rdes0 & ETH_RXDES0_FL) >> 16;
            //Limit the number of data to read
            n = MIN(n, APM32F4XX_ETH_RX_BUFFER_SIZE);

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
      rxCurDmaDesc->rdes0 = ETH_RXDES0_OWN;
      //Point to the next descriptor in the list
      rxCurDmaDesc = (Apm32f4xxRxDmaDesc *) rxCurDmaDesc->rdes3;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RXBU flag to resume processing
   ETH->DMASTS = ETH_DMASTS_RXBU;
   //Instruct the DMA to poll the receive descriptor list
   ETH->DMARXPD = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t apm32f4xxEthUpdateMacAddrFilter(NetInterface *interface)
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

   //Set the MAC address of the station
   ETH->ADDR0L = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ETH->ADDR0H = interface->macAddr.w[2] | ETH_ADDR0H_AL1;

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
            crc = apm32f4xxEthCalcCrc(&entry->addr, sizeof(MacAddr));

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
      ETH->ADDR1L = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
      ETH->ADDR1H = unicastMacAddr[0].w[2] | ETH_ADDR1H_ADDREN;
   }
   else
   {
      //When the AE bit is cleared, the entry is ignored
      ETH->ADDR1L = 0;
      ETH->ADDR1H = 0;
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      //When the AE bit is set, the entry is used for perfect filtering
      ETH->ADDR2L = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
      ETH->ADDR2H = unicastMacAddr[1].w[2] | ETH_ADDR2H_ADDREN;
   }
   else
   {
      //When the AE bit is cleared, the entry is ignored
      ETH->ADDR2L = 0;
      ETH->ADDR2H = 0;
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      //When the AE bit is set, the entry is used for perfect filtering
      ETH->ADDR3L = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
      ETH->ADDR3H = unicastMacAddr[2].w[2] | ETH_ADDR3H_ADDREN;
   }
   else
   {
      //When the AE bit is cleared, the entry is ignored
      ETH->ADDR3L = 0;
      ETH->ADDR3H = 0;
   }

   //Configure the multicast hash table
   ETH->HTL = hashTable[0];
   ETH->HTH = hashTable[1];

   //Debug message
   TRACE_DEBUG("  HTL = %08" PRIX32 "\r\n", ETH->HTL);
   TRACE_DEBUG("  HTH = %08" PRIX32 "\r\n", ETH->HTH);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t apm32f4xxEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = ETH->CFG;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= ETH_CFG_SSEL;
   }
   else
   {
      config &= ~ETH_CFG_SSEL;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= ETH_CFG_DM;
   }
   else
   {
      config &= ~ETH_CFG_DM;
   }

   //Update MAC configuration register
   ETH->CFG = config;

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

void apm32f4xxEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = ETH->ADDR & ETH_ADDR_CR;
      //Set up a write operation
      temp |= ETH_ADDR_MW | ETH_ADDR_MB;
      //PHY address
      temp |= (phyAddr << 11) & ETH_ADDR_PA;
      //Register address
      temp |= (regAddr << 6) & ETH_ADDR_MR;

      //Data to be written in the PHY register
      ETH->DATA = data & ETH_DATA_MD;

      //Start a write operation
      ETH->ADDR = temp;
      //Wait for the write to complete
      while((ETH->ADDR & ETH_ADDR_MB) != 0)
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

uint16_t apm32f4xxEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = ETH->ADDR & ETH_ADDR_CR;
      //Set up a read operation
      temp |= ETH_ADDR_MB;
      //PHY address
      temp |= (phyAddr << 11) & ETH_ADDR_PA;
      //Register address
      temp |= (regAddr << 6) & ETH_ADDR_MR;

      //Start a read operation
      ETH->ADDR = temp;
      //Wait for the read to complete
      while((ETH->ADDR & ETH_ADDR_MB) != 0)
      {
      }

      //Get register value
      data = ETH->DATA & ETH_DATA_MD;
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

uint32_t apm32f4xxEthCalcCrc(const void *data, size_t length)
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
