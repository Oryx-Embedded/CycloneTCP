/**
 * @file gd32h7xx_eth2_driver.c
 * @brief GigaDevice GD32H7 Ethernet MAC driver (ENET1 instance)
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
#include "gd32h7xx.h"
#include "core/net.h"
#include "drivers/mac/gd32h7xx_eth2_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
#pragma location = STM32H7XX_ETH2_RAM_SECTION
static uint8_t txBuffer[GD32H7XX_ETH2_TX_BUFFER_COUNT][GD32H7XX_ETH2_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
#pragma location = STM32H7XX_ETH2_RAM_SECTION
static uint8_t rxBuffer[GD32H7XX_ETH2_RX_BUFFER_COUNT][GD32H7XX_ETH2_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
#pragma location = STM32H7XX_ETH2_RAM_SECTION
static Gd32h7xxEth2TxDmaDesc txDmaDesc[GD32H7XX_ETH2_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
#pragma location = STM32H7XX_ETH2_RAM_SECTION
static Gd32h7xxEth2RxDmaDesc rxDmaDesc[GD32H7XX_ETH2_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[GD32H7XX_ETH2_TX_BUFFER_COUNT][GD32H7XX_ETH2_TX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(STM32H7XX_ETH2_RAM_SECTION)));
//Receive buffer
static uint8_t rxBuffer[GD32H7XX_ETH2_RX_BUFFER_COUNT][GD32H7XX_ETH2_RX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(STM32H7XX_ETH2_RAM_SECTION)));
//Transmit DMA descriptors
static Gd32h7xxEth2TxDmaDesc txDmaDesc[GD32H7XX_ETH2_TX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(STM32H7XX_ETH2_RAM_SECTION)));
//Receive DMA descriptors
static Gd32h7xxEth2RxDmaDesc rxDmaDesc[GD32H7XX_ETH2_RX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(STM32H7XX_ETH2_RAM_SECTION)));

#endif

//Pointer to the current TX DMA descriptor
static Gd32h7xxEth2TxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static Gd32h7xxEth2RxDmaDesc *rxCurDmaDesc;


/**
 * @brief GD32H7 Ethernet MAC driver (ENET1 instance)
 **/

const NicDriver gd32h7xxEth2Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   gd32h7xxEth2Init,
   gd32h7xxEth2Tick,
   gd32h7xxEth2EnableIrq,
   gd32h7xxEth2DisableIrq,
   gd32h7xxEth2EventHandler,
   gd32h7xxEth2SendPacket,
   gd32h7xxEth2UpdateMacAddrFilter,
   gd32h7xxEth2UpdateMacConfig,
   gd32h7xxEth2WritePhyReg,
   gd32h7xxEth2ReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief GD32H7 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t gd32h7xxEth2Init(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing GD32H7 Ethernet MAC (ENET1)...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   gd32h7xxEth2InitGpio(interface);

   //Enable Ethernet MAC clock
   rcu_periph_clock_enable(RCU_ENET1);
   rcu_periph_clock_enable(RCU_ENET1TX);
   rcu_periph_clock_enable(RCU_ENET1RX);

   //Reset Ethernet MAC peripheral
   rcu_periph_reset_enable(RCU_ENET1RST);
   rcu_periph_reset_disable(RCU_ENET1RST);

   //Perform a software reset
   ENET_DMA_BCTL(ENET1) |= ENET_DMA_BCTL_SWR;
   //Wait for the reset to complete
   while((ENET_DMA_BCTL(ENET1) & ENET_DMA_BCTL_SWR) != 0)
   {
   }

   //Adjust MDC clock range depending on HCLK frequency
   ENET_MAC_PHY_CTL(ENET1) = ENET_MDC_HCLK_DIV62;

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
   ENET_MAC_CFG(ENET1) = ENET_MAC_CFG_ROD;

   //Set the MAC address of the station
   ENET_MAC_ADDR0L(ENET1) = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ENET_MAC_ADDR0H(ENET1) = interface->macAddr.w[2] | ENET_MAC_ADDR0H_MO;

   //The MAC supports 3 additional addresses for unicast perfect filtering
   ENET_MAC_ADDR1L(ENET1) = 0;
   ENET_MAC_ADDR1H(ENET1) = 0;
   ENET_MAC_ADDR2L(ENET1) = 0;
   ENET_MAC_ADDT2H(ENET1) = 0;
   ENET_MAC_ADDR3L(ENET1) = 0;
   ENET_MAC_ADDR3H(ENET1) = 0;

   //Initialize hash table
   ENET_MAC_HLL(ENET1) = 0;
   ENET_MAC_HLH(ENET1) = 0;

   //Configure the receive filter
   ENET_MAC_FRMF(ENET1) = ENET_MAC_FRMF_HPFLT | ENET_MAC_FRMF_HMF;
   //Disable flow control
   ENET_MAC_FCTL(ENET1) = 0;
   //Enable store and forward mode
   ENET_DMA_CTL(ENET1) = ENET_DMA_CTL_RSFD | ENET_DMA_CTL_TSFD;

   //Configure DMA bus mode
   ENET_DMA_BCTL(ENET1) = ENET_DMA_BCTL_AA | ENET_DMA_BCTL_UIP | ENET_RXDP_32BEAT |
      ENET_ARBITRATION_RXTX_1_1 | ENET_PGBL_32BEAT | ENET_DMA_BCTL_DFM;

   //Initialize DMA descriptor lists
   gd32h7xxEth2InitDmaDesc(interface);

   //Prevent interrupts from being generated when the transmit statistic
   //counters reach half their maximum value
   ENET_MSC_TINTMSK(ENET1) = ENET_MSC_TINTMSK_TGFIM | ENET_MSC_TINTMSK_TGFMSCIM |
      ENET_MSC_TINTMSK_TGFSCIM;

   //Prevent interrupts from being generated when the receive statistic
   //counters reach half their maximum value
   ENET_MSC_RINTMSK(ENET1) = ENET_MSC_RINTMSK_RGUFIM | ENET_MSC_RINTMSK_RFAEIM |
      ENET_MSC_RINTMSK_RFCEIM;

   //Disable MAC interrupts
   ENET_MAC_INTMSK(ENET1) = ENET_MAC_INTMSK_TMSTIM | ENET_MAC_INTMSK_WUMIM;
   //Enable the desired DMA interrupts
   ENET_DMA_INTEN(ENET1) = ENET_DMA_INTEN_NIE | ENET_DMA_INTEN_RIE | ENET_DMA_INTEN_TIE;

   //Set priority grouping (4 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(GD32H7XX_ETH2_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(ENET1_IRQn, NVIC_EncodePriority(GD32H7XX_ETH2_IRQ_PRIORITY_GROUPING,
      GD32H7XX_ETH2_IRQ_GROUP_PRIORITY, GD32H7XX_ETH2_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   ENET_MAC_CFG(ENET1) |= ENET_MAC_CFG_TEN | ENET_MAC_CFG_REN;
   //Enable DMA transmission and reception
   ENET_DMA_CTL(ENET1) |= ENET_DMA_CTL_STE | ENET_DMA_CTL_SRE;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void gd32h7xxEth2InitGpio(NetInterface *interface)
{
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void gd32h7xxEth2InitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < GD32H7XX_ETH2_TX_BUFFER_COUNT; i++)
   {
      //Use chain structure rather than ring structure
      txDmaDesc[i].tdes0 = ENET_TDES0_INTC | ENET_TDES0_TCHM;
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
   for(i = 0; i < GD32H7XX_ETH2_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = ENET_RDES0_DAV;
      //Use chain structure rather than ring structure
      rxDmaDesc[i].rdes1 = ENET_RDES1_RCHM | (GD32H7XX_ETH2_RX_BUFFER_SIZE & ENET_RDES1_RB1S);
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
   ENET_DMA_TDTADDR(ENET1) = (uint32_t) txDmaDesc;
   //Start location of the RX descriptor list
   ENET_DMA_RDTADDR(ENET1) = (uint32_t) rxDmaDesc;
}


/**
 * @brief GD32H7 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void gd32h7xxEth2Tick(NetInterface *interface)
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

void gd32h7xxEth2EnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(ENET1_IRQn);

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

void gd32h7xxEth2DisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(ENET1_IRQn);

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
 * @brief GD32H7 Ethernet MAC interrupt service routine
 **/

void ENET1_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = ENET_DMA_STAT(ENET1);

   //Packet transmitted?
   if((status & ENET_DMA_STAT_TS) != 0)
   {
      //Clear TS interrupt flag
      ENET_DMA_STAT(ENET1) = ENET_DMA_STAT_TS;

      //Check whether the TX buffer is available for writing
      if((txCurDmaDesc->tdes0 & ENET_TDES0_DAV) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & ENET_DMA_STAT_RS) != 0)
   {
      //Clear RS interrupt flag
      ENET_DMA_STAT(ENET1) = ENET_DMA_STAT_RS;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   ENET_DMA_STAT(ENET1) = ENET_DMA_STAT_NI;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief GD32H7 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void gd32h7xxEth2EventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = gd32h7xxEth2ReceivePacket(interface);

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

error_t gd32h7xxEth2SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > GD32H7XX_ETH2_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txCurDmaDesc->tdes0 & ENET_TDES0_DAV) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead((uint8_t *) txCurDmaDesc->tdes2, buffer, offset, length);

   //Write the number of bytes to send
   txCurDmaDesc->tdes1 = length & ENET_TDES1_TB1S;
   //Set LS and FS flags as the data fits in a single buffer
   txCurDmaDesc->tdes0 |= ENET_TDES0_LSG | ENET_TDES0_FSG;
   //Give the ownership of the descriptor to the DMA
   txCurDmaDesc->tdes0 |= ENET_TDES0_DAV;

   //Data synchronization barrier
   __DSB();

   //Clear TBU flag to resume processing
   ENET_DMA_STAT(ENET1) = ENET_DMA_STAT_TBU;
   //Instruct the DMA to poll the transmit descriptor list
   ENET_DMA_TPEN(ENET1) = 0;

   //Point to the next descriptor in the list
   txCurDmaDesc = (Gd32h7xxEth2TxDmaDesc *) txCurDmaDesc->tdes3;

   //Check whether the next buffer is available for writing
   if((txCurDmaDesc->tdes0 & ENET_TDES0_DAV) == 0)
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

error_t gd32h7xxEth2ReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxCurDmaDesc->rdes0 & ENET_RDES0_DAV) == 0)
   {
      //FS and LS flags should be set
      if((rxCurDmaDesc->rdes0 & ENET_RDES0_FDES) != 0 &&
         (rxCurDmaDesc->rdes0 & ENET_RDES0_LDES) != 0)
      {
         //Make sure no error occurred
         if((rxCurDmaDesc->rdes0 & ENET_RDES0_ERRS) == 0)
         {
            //Retrieve the length of the frame
            n = (rxCurDmaDesc->rdes0 & ENET_RDES0_FRML) >> 16;
            //Limit the number of data to read
            n = MIN(n, GD32H7XX_ETH2_RX_BUFFER_SIZE);

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
      rxCurDmaDesc->rdes0 = ENET_RDES0_DAV;
      //Point to the next descriptor in the list
      rxCurDmaDesc = (Gd32h7xxEth2RxDmaDesc *) rxCurDmaDesc->rdes3;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RBU flag to resume processing
   ENET_DMA_STAT(ENET1) = ENET_DMA_STAT_RBU;
   //Instruct the DMA to poll the receive descriptor list
   ENET_DMA_RPEN(ENET1) = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t gd32h7xxEth2UpdateMacAddrFilter(NetInterface *interface)
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
   ENET_MAC_ADDR0L(ENET1) = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ENET_MAC_ADDR0H(ENET1) = interface->macAddr.w[2] | ENET_MAC_ADDR0H_MO;

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
            crc = gd32h7xxEth2CalcCrc(&entry->addr, sizeof(MacAddr));

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
      ENET_MAC_ADDR1L(ENET1) = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
      ENET_MAC_ADDR1H(ENET1) = unicastMacAddr[0].w[2] | ENET_MAC_ADDR1H_AFE;
   }
   else
   {
      //When the AE bit is cleared, the entry is ignored
      ENET_MAC_ADDR1L(ENET1) = 0;
      ENET_MAC_ADDR1H(ENET1) = 0;
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      //When the AE bit is set, the entry is used for perfect filtering
      ENET_MAC_ADDR2L(ENET1) = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
      ENET_MAC_ADDT2H(ENET1) = unicastMacAddr[1].w[2] | ENET_MAC_ADDR2H_AFE;
   }
   else
   {
      //When the AE bit is cleared, the entry is ignored
      ENET_MAC_ADDR2L(ENET1) = 0;
      ENET_MAC_ADDT2H(ENET1) = 0;
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      //When the AE bit is set, the entry is used for perfect filtering
      ENET_MAC_ADDR3L(ENET1) = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
      ENET_MAC_ADDR3H(ENET1) = unicastMacAddr[2].w[2] | ENET_MAC_ADDR3H_AFE;
   }
   else
   {
      //When the AE bit is cleared, the entry is ignored
      ENET_MAC_ADDR3L(ENET1) = 0;
      ENET_MAC_ADDR3H(ENET1) = 0;
   }

   //Configure the multicast hash table
   ENET_MAC_HLL(ENET1) = hashTable[0];
   ENET_MAC_HLH(ENET1) = hashTable[1];

   //Debug message
   TRACE_DEBUG("  ENET_MAC_HLL = %08" PRIX32 "\r\n", ENET_MAC_HLL(ENET1));
   TRACE_DEBUG("  ENET_MAC_HLH = %08" PRIX32 "\r\n", ENET_MAC_HLH(ENET1));

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t gd32h7xxEth2UpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = ENET_MAC_CFG(ENET1);

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= ENET_MAC_CFG_SPD;
   }
   else
   {
      config &= ~ENET_MAC_CFG_SPD;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= ENET_MAC_CFG_DPM;
   }
   else
   {
      config &= ~ENET_MAC_CFG_DPM;
   }

   //Update MAC configuration register
   ENET_MAC_CFG(ENET1) = config;

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

void gd32h7xxEth2WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = ENET_MAC_PHY_CTL(ENET1) & ENET_MAC_PHY_CTL_CLR;
      //Set up a write operation
      temp |= ENET_MAC_PHY_CTL_PW | ENET_MAC_PHY_CTL_PB;
      //PHY address
      temp |= MAC_PHY_CTL_PA(phyAddr);
      //Register address
      temp |= MAC_PHY_CTL_PR(regAddr);

      //Data to be written in the PHY register
      ENET_MAC_PHY_DATA(ENET1) = data & ENET_MAC_PHY_DATA_PD;

      //Start a write operation
      ENET_MAC_PHY_CTL(ENET1) = temp;
      //Wait for the write to complete
      while((ENET_MAC_PHY_CTL(ENET1) & ENET_MAC_PHY_CTL_PB) != 0)
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

uint16_t gd32h7xxEth2ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = ENET_MAC_PHY_CTL(ENET1) & ENET_MAC_PHY_CTL_CLR;
      //Set up a read operation
      temp |= ENET_MAC_PHY_CTL_PB;
      //PHY address
      temp |= MAC_PHY_CTL_PA(phyAddr);
      //Register address
      temp |= MAC_PHY_CTL_PR(regAddr);

      //Start a read operation
      ENET_MAC_PHY_CTL(ENET1) = temp;
      //Wait for the read to complete
      while((ENET_MAC_PHY_CTL(ENET1) & ENET_MAC_PHY_CTL_PB) != 0)
      {
      }

      //Get register value
      data = ENET_MAC_PHY_DATA(ENET1) & ENET_MAC_PHY_DATA_PD;
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

uint32_t gd32h7xxEth2CalcCrc(const void *data, size_t length)
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
