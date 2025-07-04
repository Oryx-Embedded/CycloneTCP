/**
 * @file sam9x6_eth2_driver.c
 * @brief SAM9X60 Ethernet MAC driver (EMAC1 instance)
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
#include <limits.h>
#include "sam.h"
#include "core/net.h"
#include "drivers/mac/sam9x6_eth2_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//TX buffer
#pragma data_alignment = 8
#pragma location = SAM9X6_ETH2_RAM_SECTION
static uint8_t txBuffer[SAM9X6_ETH2_TX_BUFFER_COUNT][SAM9X6_ETH2_TX_BUFFER_SIZE];
//RX buffer
#pragma data_alignment = 8
#pragma location = SAM9X6_ETH2_RAM_SECTION
static uint8_t rxBuffer[SAM9X6_ETH2_RX_BUFFER_COUNT][SAM9X6_ETH2_RX_BUFFER_SIZE];
//TX buffer descriptors
#pragma data_alignment = 4
#pragma location = SAM9X6_ETH2_RAM_SECTION
static Sam9x6Eth2TxBufferDesc txBufferDesc[SAM9X6_ETH2_TX_BUFFER_COUNT];
//RX buffer descriptors
#pragma data_alignment = 4
#pragma location = SAM9X6_ETH2_RAM_SECTION
static Sam9x6Eth2RxBufferDesc rxBufferDesc[SAM9X6_ETH2_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//TX buffer
static uint8_t txBuffer[SAM9X6_ETH2_TX_BUFFER_COUNT][SAM9X6_ETH2_TX_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(SAM9X6_ETH2_RAM_SECTION)));
//RX buffer
static uint8_t rxBuffer[SAM9X6_ETH2_RX_BUFFER_COUNT][SAM9X6_ETH2_RX_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(SAM9X6_ETH2_RAM_SECTION)));
//TX buffer descriptors
static Sam9x6Eth2TxBufferDesc txBufferDesc[SAM9X6_ETH2_TX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(SAM9X6_ETH2_RAM_SECTION)));
//RX buffer descriptors
static Sam9x6Eth2RxBufferDesc rxBufferDesc[SAM9X6_ETH2_RX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(SAM9X6_ETH2_RAM_SECTION)));

#endif

//TX buffer index
static uint_t txBufferIndex;
//RX buffer index
static uint_t rxBufferIndex;


/**
 * @brief SAM9X6 Ethernet MAC driver (EMAC1 instance)
 **/

const NicDriver sam9x6Eth2Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   sam9x6Eth2Init,
   sam9x6Eth2Tick,
   sam9x6Eth2EnableIrq,
   sam9x6Eth2DisableIrq,
   sam9x6Eth2EventHandler,
   sam9x6Eth2SendPacket,
   sam9x6Eth2UpdateMacAddrFilter,
   sam9x6Eth2UpdateMacConfig,
   sam9x6Eth2WritePhyReg,
   sam9x6Eth2ReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief SAM9X6 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t sam9x6Eth2Init(NetInterface *interface)
{
   error_t error;
   volatile uint32_t temp;

   //Debug message
   TRACE_INFO("Initializing SAM9X6 Ethernet MAC (EMAC1)...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable EMAC peripheral clock
   PMC_REGS->PMC_PCR = PMC_PCR_PID(ID_EMAC1);
   temp = PMC_REGS->PMC_PCR;
   PMC_REGS->PMC_PCR = temp | PMC_PCR_CMD_Msk | PMC_PCR_EN_Msk;

   //Disable transmit and receive circuits
   EMAC1_REGS->EMAC_NCR = 0;

   //GPIO configuration
   sam9x6Eth2InitGpio(interface);

   //Configure MDC clock speed
   EMAC1_REGS->EMAC_NCFGR = EMAC_NCFGR_CLK_MCK_64;
   //Enable management port (MDC and MDIO)
   EMAC1_REGS->EMAC_NCR |= EMAC_NCR_MPE_Msk;

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

   //Set the MAC address of the station
   EMAC1_REGS->EMAC_SA[0].EMAC_SAxB = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   EMAC1_REGS->EMAC_SA[0].EMAC_SAxT = interface->macAddr.w[2];

   //The MAC supports 3 additional addresses for unicast perfect filtering
   EMAC1_REGS->EMAC_SA[1].EMAC_SAxB = 0;
   EMAC1_REGS->EMAC_SA[2].EMAC_SAxB = 0;
   EMAC1_REGS->EMAC_SA[3].EMAC_SAxB = 0;

   //Initialize hash table
   EMAC1_REGS->EMAC_HRB = 0;
   EMAC1_REGS->EMAC_HRT = 0;

   //Configure the receive filter
   EMAC1_REGS->EMAC_NCFGR |= EMAC_NCFGR_BIG_Msk | EMAC_NCFGR_MTI_Msk;

   //Initialize buffer descriptors
   sam9x6Eth2InitBufferDesc(interface);

   //Clear transmit status register
   EMAC1_REGS->EMAC_TSR = EMAC_TSR_UND_Msk | EMAC_TSR_COMP_Msk |
      EMAC_TSR_BEX_Msk | EMAC_TSR_TGO_Msk | EMAC_TSR_RLES_Msk |
      EMAC_TSR_COL_Msk | EMAC_TSR_UBR_Msk;

   //Clear receive status register
   EMAC1_REGS->EMAC_RSR = EMAC_RSR_OVR_Msk | EMAC_RSR_REC_Msk |
      EMAC_RSR_BNA_Msk;

   //First disable all EMAC interrupts
   EMAC1_REGS->EMAC_IDR = 0xFFFFFFFF;

   //Only the desired ones are enabled
   EMAC1_REGS->EMAC_IER = EMAC_IER_ROVR_Msk | EMAC_IER_TCOMP_Msk |
      EMAC_IER_TXERR_Msk | EMAC_IER_RLE_Msk | EMAC_IER_TUND_Msk |
      EMAC_IER_RXUBR_Msk | EMAC_IER_RCOMP_Msk;

   //Read EMAC_ISR register to clear any pending interrupt
   temp = EMAC1_REGS->EMAC_ISR;
   (void) temp;

   //Configure interrupt controller
   AIC_REGS->AIC_SSR = ID_EMAC1;
   AIC_REGS->AIC_SMR = AIC_SMR_SRCTYPE_INT_LEVEL_SENSITIVE | AIC_SMR_PRIOR(SAM9X6_ETH2_IRQ_PRIORITY);
   AIC_REGS->AIC_SVR = (uint32_t) sam9x6Eth2IrqHandler;

   //Clear EMAC interrupt flag
   AIC_REGS->AIC_ICCR = (1 << ID_EMAC1);

   //Enable the EMAC to transmit and receive data
   EMAC1_REGS->EMAC_NCR |= EMAC_NCR_TE_Msk | EMAC_NCR_RE_Msk;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void sam9x6Eth2InitGpio(NetInterface *interface)
{
}


/**
 * @brief Initialize buffer descriptors
 * @param[in] interface Underlying network interface
 **/

void sam9x6Eth2InitBufferDesc(NetInterface *interface)
{
   uint_t i;
   uint32_t address;

   //Initialize TX buffer descriptors
   for(i = 0; i < SAM9X6_ETH2_TX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current TX buffer
      address = (uint32_t) txBuffer[i];
      //Write the address to the descriptor entry
      txBufferDesc[i].address = address;
      //Initialize status field
      txBufferDesc[i].status = EMAC_TX_USED;
   }

   //Mark the last descriptor entry with the wrap flag
   txBufferDesc[i - 1].status |= EMAC_TX_WRAP;
   //Initialize TX buffer index
   txBufferIndex = 0;

   //Initialize RX buffer descriptors
   for(i = 0; i < SAM9X6_ETH2_RX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current RX buffer
      address = (uint32_t) rxBuffer[i];
      //Write the address to the descriptor entry
      rxBufferDesc[i].address = address & EMAC_RX_ADDRESS;
      //Clear status field
      rxBufferDesc[i].status = 0;
   }

   //Mark the last descriptor entry with the wrap flag
   rxBufferDesc[i - 1].address |= EMAC_RX_WRAP;
   //Initialize RX buffer index
   rxBufferIndex = 0;

   //Start location of the TX descriptor list
   EMAC1_REGS->EMAC_TBQP = (uint32_t) txBufferDesc;
   //Start location of the RX descriptor list
   EMAC1_REGS->EMAC_RBQP = (uint32_t) rxBufferDesc;
}


/**
 * @brief SAM9X6 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void sam9x6Eth2Tick(NetInterface *interface)
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

void sam9x6Eth2EnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   AIC_REGS->AIC_SSR = AIC_SSR_INTSEL(ID_EMAC1);
   AIC_REGS->AIC_IECR = AIC_IECR_INTEN_Msk;

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

void sam9x6Eth2DisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   AIC_REGS->AIC_SSR = AIC_SSR_INTSEL(ID_EMAC1);
   AIC_REGS->AIC_IDCR = AIC_IDCR_INTD_Msk;

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
 * @brief SAM9X6 Ethernet MAC interrupt service routine
 **/

void sam9x6Eth2IrqHandler(void)
{
   bool_t flag;
   volatile uint32_t isr;
   volatile uint32_t tsr;
   volatile uint32_t rsr;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Each time the software reads EMAC_ISR, it has to check the contents
   //of EMAC_TSR, EMAC_RSR and EMAC_NSR
   isr = EMAC1_REGS->EMAC_ISR;
   tsr = EMAC1_REGS->EMAC_TSR;
   rsr = EMAC1_REGS->EMAC_RSR;
   (void) isr;

   //Packet transmitted?
   if((tsr & (EMAC_TSR_UND_Msk | EMAC_TSR_COMP_Msk | EMAC_TSR_BEX_Msk |
      EMAC_TSR_TGO_Msk | EMAC_TSR_RLES_Msk | EMAC_TSR_COL_Msk |
      EMAC_TSR_UBR_Msk)) != 0)
   {
      //Only clear TSR flags that are currently set
      EMAC1_REGS->EMAC_TSR = tsr;

      //Check whether the TX buffer is available for writing
      if((txBufferDesc[txBufferIndex].status & EMAC_TX_USED) != 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((rsr & (EMAC_RSR_OVR_Msk | EMAC_RSR_REC_Msk | EMAC_RSR_BNA_Msk)) != 0)
   {
      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

#if (NET_RTOS_SUPPORT == DISABLED)
   //Write AIC_EOICR register before exiting
   AIC_REGS->AIC_EOICR = 0;
#endif

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief SAM9X6 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void sam9x6Eth2EventHandler(NetInterface *interface)
{
   error_t error;
   uint32_t rsr;

   //Read receive status
   rsr = EMAC1_REGS->EMAC_RSR;

   //Packet received?
   if((rsr & (EMAC_RSR_OVR_Msk | EMAC_RSR_REC_Msk | EMAC_RSR_BNA_Msk)) != 0)
   {
      //Only clear RSR flags that are currently set
      EMAC1_REGS->EMAC_RSR = rsr;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = sam9x6Eth2ReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }
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

error_t sam9x6Eth2SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > SAM9X6_ETH2_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txBufferDesc[txBufferIndex].status & EMAC_TX_USED) == 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txBufferIndex], buffer, offset, length);

   //Set the necessary flags in the descriptor entry
   if(txBufferIndex < (SAM9X6_ETH2_TX_BUFFER_COUNT - 1))
   {
      //Write the status word
      txBufferDesc[txBufferIndex].status = EMAC_TX_LAST |
         (length & EMAC_TX_LENGTH);

      //Point to the next buffer
      txBufferIndex++;
   }
   else
   {
      //Write the status word
      txBufferDesc[txBufferIndex].status = EMAC_TX_WRAP | EMAC_TX_LAST |
         (length & EMAC_TX_LENGTH);

      //Wrap around
      txBufferIndex = 0;
   }

   //Set the TSTART bit to initiate transmission
   EMAC1_REGS->EMAC_NCR |= EMAC_NCR_TSTART_Msk;

   //Check whether the next buffer is available for writing
   if((txBufferDesc[txBufferIndex].status & EMAC_TX_USED) != 0)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Receive a packet
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t sam9x6Eth2ReceivePacket(NetInterface *interface)
{
   static uint32_t temp[ETH_MAX_FRAME_SIZE / 4];
   error_t error;
   uint_t i;
   uint_t j;
   uint_t sofIndex;
   uint_t eofIndex;
   size_t n;
   size_t size;
   size_t length;

   //Initialize variables
   size = 0;
   sofIndex = UINT_MAX;
   eofIndex = UINT_MAX;

   //Search for SOF and EOF flags
   for(i = 0; i < SAM9X6_ETH2_RX_BUFFER_COUNT; i++)
   {
      //Point to the current entry
      j = rxBufferIndex + i;

      //Wrap around to the beginning of the buffer if necessary
      if(j >= SAM9X6_ETH2_RX_BUFFER_COUNT)
      {
         j -= SAM9X6_ETH2_RX_BUFFER_COUNT;
      }

      //No more entries to process?
      if((rxBufferDesc[j].address & EMAC_RX_OWNERSHIP) == 0)
      {
         //Stop processing
         break;
      }

      //A valid SOF has been found?
      if((rxBufferDesc[j].status & EMAC_RX_SOF) != 0)
      {
         //Save the position of the SOF
         sofIndex = i;
      }

      //A valid EOF has been found?
      if((rxBufferDesc[j].status & EMAC_RX_EOF) != 0 && sofIndex != UINT_MAX)
      {
         //Save the position of the EOF
         eofIndex = i;
         //Retrieve the length of the frame
         size = rxBufferDesc[j].status & EMAC_RX_LENGTH;
         //Limit the number of data to read
         size = MIN(size, ETH_MAX_FRAME_SIZE);
         //Stop processing since we have reached the end of the frame
         break;
      }
   }

   //Determine the number of entries to process
   if(eofIndex != UINT_MAX)
   {
      j = eofIndex + 1;
   }
   else if(sofIndex != UINT_MAX)
   {
      j = sofIndex;
   }
   else
   {
      j = i;
   }

   //Total number of bytes that have been copied from the receive buffer
   length = 0;

   //Process incoming frame
   for(i = 0; i < j; i++)
   {
      //Any data to copy from current buffer?
      if(eofIndex != UINT_MAX && i >= sofIndex && i <= eofIndex)
      {
         //Calculate the number of bytes to read at a time
         n = MIN(size, SAM9X6_ETH2_RX_BUFFER_SIZE);
         //Copy data from receive buffer
         osMemcpy((uint8_t *) temp + length, rxBuffer[rxBufferIndex], n);
         //Update byte counters
         length += n;
         size -= n;
      }

      //Mark the current buffer as free
      rxBufferDesc[rxBufferIndex].address &= ~EMAC_RX_OWNERSHIP;

      //Point to the following entry
      rxBufferIndex++;

      //Wrap around to the beginning of the buffer if necessary
      if(rxBufferIndex >= SAM9X6_ETH2_RX_BUFFER_COUNT)
      {
         rxBufferIndex = 0;
      }
   }

   //Any packet to process?
   if(length > 0)
   {
      NetRxAncillary ancillary;

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_RX_ANCILLARY;

      //Pass the packet to the upper layer
      nicProcessPacket(interface, (uint8_t *) temp, length, &ancillary);
      //Valid packet received
      error = NO_ERROR;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t sam9x6Eth2UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   uint_t k;
   uint8_t *p;
   uint32_t hashTable[2];
   MacAddr unicastMacAddr[3];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Set the MAC address of the station
   EMAC1_REGS->EMAC_SA[0].EMAC_SAxB = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   EMAC1_REGS->EMAC_SA[0].EMAC_SAxT = interface->macAddr.w[2];

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
            //Point to the MAC address
            p = entry->addr.b;

            //Apply the hash function
            k = (p[0] >> 6) ^ p[0];
            k ^= (p[1] >> 4) ^ (p[1] << 2);
            k ^= (p[2] >> 2) ^ (p[2] << 4);
            k ^= (p[3] >> 6) ^ p[3];
            k ^= (p[4] >> 4) ^ (p[4] << 2);
            k ^= (p[5] >> 2) ^ (p[5] << 4);

            //The hash value is reduced to a 6-bit index
            k &= 0x3F;

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
      //The address is activated when SAH register is written
      EMAC1_REGS->EMAC_SA[1].EMAC_SAxB = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
      EMAC1_REGS->EMAC_SA[1].EMAC_SAxT = unicastMacAddr[0].w[2];
   }
   else
   {
      //The address is deactivated when SAL register is written
      EMAC1_REGS->EMAC_SA[1].EMAC_SAxB = 0;
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      //The address is activated when SAH register is written
      EMAC1_REGS->EMAC_SA[2].EMAC_SAxB = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
      EMAC1_REGS->EMAC_SA[2].EMAC_SAxT = unicastMacAddr[1].w[2];
   }
   else
   {
      //The address is deactivated when SAL register is written
      EMAC1_REGS->EMAC_SA[2].EMAC_SAxB = 0;
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      //The address is activated when SAH register is written
      EMAC1_REGS->EMAC_SA[3].EMAC_SAxB = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
      EMAC1_REGS->EMAC_SA[3].EMAC_SAxT = unicastMacAddr[2].w[2];
   }
   else
   {
      //The address is deactivated when SAL register is written
      EMAC1_REGS->EMAC_SA[3].EMAC_SAxB = 0;
   }

   //Configure the multicast hash table
   EMAC1_REGS->EMAC_HRB = hashTable[0];
   EMAC1_REGS->EMAC_HRT = hashTable[1];

   //Debug message
   TRACE_DEBUG("  HRB = %08" PRIX32 "\r\n", EMAC1_REGS->EMAC_HRB);
   TRACE_DEBUG("  HRT = %08" PRIX32 "\r\n", EMAC1_REGS->EMAC_HRT);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t sam9x6Eth2UpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read network configuration register
   config = EMAC1_REGS->EMAC_NCFGR;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= EMAC_NCFGR_SPD_Msk;
   }
   else
   {
      config &= ~EMAC_NCFGR_SPD_Msk;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= EMAC_NCFGR_FD_Msk;
   }
   else
   {
      config &= ~EMAC_NCFGR_FD_Msk;
   }

   //Write configuration value back to NCFGR register
   EMAC1_REGS->EMAC_NCFGR = config;

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

void sam9x6Eth2WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Set up a write operation
      temp = EMAC_MAN_SOF(1) | EMAC_MAN_RW(1) | EMAC_MAN_CODE(2);
      //PHY address
      temp |= EMAC_MAN_PHYA(phyAddr);
      //Register address
      temp |= EMAC_MAN_REGA(regAddr);
      //Register value
      temp |= EMAC_MAN_DATA(data);

      //Start a write operation
      EMAC1_REGS->EMAC_MAN = temp;
      //Wait for the write to complete
      while((EMAC1_REGS->EMAC_NSR & EMAC_NSR_IDLE_Msk) == 0)
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

uint16_t sam9x6Eth2ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Set up a read operation
      temp = EMAC_MAN_SOF(1) | EMAC_MAN_RW(2) | EMAC_MAN_CODE(2);
      //PHY address
      temp |= EMAC_MAN_PHYA(phyAddr);
      //Register address
      temp |= EMAC_MAN_REGA(regAddr);

      //Start a read operation
      EMAC1_REGS->EMAC_MAN = temp;
      //Wait for the read to complete
      while((EMAC1_REGS->EMAC_NSR & EMAC_NSR_IDLE_Msk) == 0)
      {
      }

      //Get register value
      data = EMAC1_REGS->EMAC_MAN & EMAC_MAN_DATA_Msk;
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
      data = 0;
   }

   //Return the value of the PHY register
   return data;
}
