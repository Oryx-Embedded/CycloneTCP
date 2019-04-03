/**
 * @file same54_eth_driver.c
 * @brief SAME54 Ethernet MAC controller
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include <limits.h>
#include "same54.h"
#include "core/net.h"
#include "drivers/mac/same54_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//TX buffer
#pragma data_alignment = 8
static uint8_t txBuffer[SAME54_ETH_TX_BUFFER_COUNT][SAME54_ETH_TX_BUFFER_SIZE];
//RX buffer
#pragma data_alignment = 8
static uint8_t rxBuffer[SAME54_ETH_RX_BUFFER_COUNT][SAME54_ETH_RX_BUFFER_SIZE];
//TX buffer descriptors
#pragma data_alignment = 4
static Same54TxBufferDesc txBufferDesc[SAME54_ETH_TX_BUFFER_COUNT];
//RX buffer descriptors
#pragma data_alignment = 4
static Same54RxBufferDesc rxBufferDesc[SAME54_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//TX buffer
static uint8_t txBuffer[SAME54_ETH_TX_BUFFER_COUNT][SAME54_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(8)));
//RX buffer
static uint8_t rxBuffer[SAME54_ETH_RX_BUFFER_COUNT][SAME54_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(8)));
//TX buffer descriptors
static Same54TxBufferDesc txBufferDesc[SAME54_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//RX buffer descriptors
static Same54RxBufferDesc rxBufferDesc[SAME54_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//TX buffer index
static uint_t txBufferIndex;
//RX buffer index
static uint_t rxBufferIndex;


/**
 * @brief SAME54 Ethernet MAC driver
 **/

const NicDriver same54EthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   same54EthInit,
   same54EthTick,
   same54EthEnableIrq,
   same54EthDisableIrq,
   same54EthEventHandler,
   same54EthSendPacket,
   same54EthUpdateMacAddrFilter,
   same54EthUpdateMacConfig,
   same54EthWritePhyReg,
   same54EthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief SAME54 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t same54EthInit(NetInterface *interface)
{
   error_t error;
   volatile uint32_t status;

   //Debug message
   TRACE_INFO("Initializing SAME54 Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable GMAC bus clocks (CLK_GMAC_APB and CLK_GMAC_AHB)
   MCLK->APBCMASK.bit.GMAC_ = 1;
   MCLK->AHBMASK.bit.GMAC_ = 1;

   //GPIO configuration
   same54EthInitGpio(interface);

   //Configure MDC clock speed
   GMAC->NCFGR.reg = GMAC_NCFGR_CLK(5);
   //Enable management port (MDC and MDIO)
   GMAC->NCR.reg |= GMAC_NCR_MPE;

   //PHY transceiver initialization
   error = interface->phyDriver->init(interface);
   //Failed to initialize PHY transceiver?
   if(error)
      return error;

   //Set the MAC address of the station
   GMAC->Sa[0].SAB.reg = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   GMAC->Sa[0].SAT.reg = interface->macAddr.w[2];

   //The MAC supports 3 additional addresses for unicast perfect filtering
   GMAC->Sa[1].SAB.reg = 0;
   GMAC->Sa[2].SAB.reg = 0;
   GMAC->Sa[3].SAB.reg = 0;

   //Initialize hash table
   GMAC->HRB.reg = 0;
   GMAC->HRT.reg = 0;

   //Configure the receive filter
   GMAC->NCFGR.reg |= GMAC_NCFGR_MAXFS | GMAC_NCFGR_MTIHEN;

   //Initialize buffer descriptors
   same54EthInitBufferDesc(interface);

   //Clear transmit status register
   GMAC->TSR.reg = GMAC_TSR_HRESP | GMAC_TSR_UND | GMAC_TSR_TXCOMP | GMAC_TSR_TFC |
      GMAC_TSR_TXGO | GMAC_TSR_RLE | GMAC_TSR_COL | GMAC_TSR_UBR;
   //Clear receive status register
   GMAC->RSR.reg = GMAC_RSR_HNO | GMAC_RSR_RXOVR | GMAC_RSR_REC | GMAC_RSR_BNA;

   //First disable all GMAC interrupts
   GMAC->IDR.reg = 0xFFFFFFFF;
   //Only the desired ones are enabled
   GMAC->IER.reg = GMAC_IER_HRESP | GMAC_IER_ROVR | GMAC_IER_TCOMP | GMAC_IER_TFC |
      GMAC_IER_RLEX | GMAC_IER_TUR | GMAC_IER_RXUBR | GMAC_IER_RCOMP;

   //Read GMAC ISR register to clear any pending interrupt
   status = GMAC->ISR.reg;

   //Set priority grouping (3 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(SAME54_ETH_IRQ_PRIORITY_GROUPING);

   //Configure GMAC interrupt priority
   NVIC_SetPriority(GMAC_IRQn, NVIC_EncodePriority(SAME54_ETH_IRQ_PRIORITY_GROUPING,
      SAME54_ETH_IRQ_GROUP_PRIORITY, SAME54_ETH_IRQ_SUB_PRIORITY));

   //Enable the GMAC to transmit and receive data
   GMAC->NCR.reg |= GMAC_NCR_TXEN | GMAC_NCR_RXEN;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


//SAME54-Xplained-Pro evaluation board?
#if defined(USE_SAME54_XPLAINED_PRO)

/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

void same54EthInitGpio(NetInterface *interface)
{
   //Enable PORT bus clock (CLK_PORT_APB)
   MCLK->APBBMASK.bit.PORT_ = 1;

   //Configure GRX1 (PA12)
   PORT->Group[0].PINCFG[12].bit.PMUXEN = 1;
   PORT->Group[0].PMUX[6].bit.PMUXE = MUX_PA12L_GMAC_GRX1;

   //Configure GRX0 (PA13)
   PORT->Group[0].PINCFG[13].bit.PMUXEN = 1;
   PORT->Group[0].PMUX[6].bit.PMUXO = MUX_PA13L_GMAC_GRX0;

   //Configure GTXCK (PA14)
   PORT->Group[0].PINCFG[14].bit.PMUXEN = 1;
   PORT->Group[0].PMUX[7].bit.PMUXE = MUX_PA14L_GMAC_GTXCK;

   //Configure GRXER (PA15)
   PORT->Group[0].PINCFG[15].bit.PMUXEN = 1;
   PORT->Group[0].PMUX[7].bit.PMUXO = MUX_PA15L_GMAC_GRXER;

   //Configure GTXEN (PA17)
   PORT->Group[0].PINCFG[17].bit.DRVSTR = 1;
   PORT->Group[0].PINCFG[17].bit.PMUXEN = 1;
   PORT->Group[0].PMUX[8].bit.PMUXO = MUX_PA17L_GMAC_GTXEN;

   //Configure GTX0 (PA18)
   PORT->Group[0].PINCFG[18].bit.DRVSTR = 1;
   PORT->Group[0].PINCFG[18].bit.PMUXEN = 1;
   PORT->Group[0].PMUX[9].bit.PMUXE = MUX_PA18L_GMAC_GTX0;

   //Configure GTX1 (PA19)
   PORT->Group[0].PINCFG[19].bit.DRVSTR = 1;
   PORT->Group[0].PINCFG[19].bit.PMUXEN = 1;
   PORT->Group[0].PMUX[9].bit.PMUXO = MUX_PA19L_GMAC_GTX1;

   //Configure GMDC (PC11)
   PORT->Group[2].PINCFG[11].bit.PMUXEN = 1;
   PORT->Group[2].PMUX[5].bit.PMUXO = MUX_PC11L_GMAC_GMDC;

   //Configure GMDIO (PC12)
   PORT->Group[2].PINCFG[12].bit.PMUXEN = 1;
   PORT->Group[2].PMUX[6].bit.PMUXE = MUX_PC12L_GMAC_GMDIO;

   //Configure GRXDV (PC20)
   PORT->Group[2].PINCFG[20].bit.PMUXEN = 1;
   PORT->Group[2].PMUX[10].bit.PMUXE = MUX_PC20L_GMAC_GRXDV;

   //Select RMII operation mode
   GMAC->UR.bit.MII = 0;

   //Configure PHY_RESET (PC21) as an output
   PORT->Group[2].DIRSET.reg = PORT_PC21;

   //Reset PHY transceiver
   PORT->Group[2].OUTCLR.reg = PORT_PC21;
   sleep(10);

   //Take the PHY transceiver out of reset
   PORT->Group[2].OUTSET.reg = PORT_PC21;
   sleep(10);
}

#endif


/**
 * @brief Initialize buffer descriptors
 * @param[in] interface Underlying network interface
 **/

void same54EthInitBufferDesc(NetInterface *interface)
{
   uint_t i;
   uint32_t address;

   //Initialize TX buffer descriptors
   for(i = 0; i < SAME54_ETH_TX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current TX buffer
      address = (uint32_t) txBuffer[i];
      //Write the address to the descriptor entry
      txBufferDesc[i].address = address;
      //Initialize status field
      txBufferDesc[i].status = GMAC_TX_USED;
   }

   //Mark the last descriptor entry with the wrap flag
   txBufferDesc[i - 1].status |= GMAC_TX_WRAP;
   //Initialize TX buffer index
   txBufferIndex = 0;

   //Initialize RX buffer descriptors
   for(i = 0; i < SAME54_ETH_RX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current RX buffer
      address = (uint32_t) rxBuffer[i];
      //Write the address to the descriptor entry
      rxBufferDesc[i].address = address & GMAC_RX_ADDRESS;
      //Clear status field
      rxBufferDesc[i].status = 0;
   }

   //Mark the last descriptor entry with the wrap flag
   rxBufferDesc[i - 1].address |= GMAC_RX_WRAP;
   //Initialize RX buffer index
   rxBufferIndex = 0;

   //Start location of the TX descriptor list
   GMAC->TBQB.reg = (uint32_t) txBufferDesc;
   //Start location of the RX descriptor list
   GMAC->RBQB.reg = (uint32_t) rxBufferDesc;
}


/**
 * @brief SAME54 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void same54EthTick(NetInterface *interface)
{
   //Handle periodic operations
   interface->phyDriver->tick(interface);
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void same54EthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(GMAC_IRQn);
   //Enable Ethernet PHY interrupts
   interface->phyDriver->enableIrq(interface);
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void same54EthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(GMAC_IRQn);
   //Disable Ethernet PHY interrupts
   interface->phyDriver->disableIrq(interface);
}


/**
 * @brief SAME54 Ethernet MAC interrupt service routine
 **/

void GMAC_Handler(void)
{
   bool_t flag;
   volatile uint32_t isr;
   volatile uint32_t tsr;
   volatile uint32_t rsr;

   //Enter interrupt service routine
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Each time the software reads GMAC_ISR, it has to check the
   //contents of GMAC_TSR, GMAC_RSR and GMAC_NSR
   isr = GMAC->ISR.reg;
   tsr = GMAC->TSR.reg;
   rsr = GMAC->RSR.reg;

   //A packet has been transmitted?
   if(tsr & (GMAC_TSR_HRESP | GMAC_TSR_UND | GMAC_TSR_TXCOMP | GMAC_TSR_TFC |
      GMAC_TSR_TXGO | GMAC_TSR_RLE | GMAC_TSR_COL | GMAC_TSR_UBR))
   {
      //Only clear TSR flags that are currently set
      GMAC->TSR.reg = tsr;

      //Check whether the TX buffer is available for writing
      if(txBufferDesc[txBufferIndex].status & GMAC_TX_USED)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //A packet has been received?
   if(rsr & (GMAC_RSR_HNO | GMAC_RSR_RXOVR | GMAC_RSR_REC | GMAC_RSR_BNA))
   {
      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}


/**
 * @brief SAME54 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void same54EthEventHandler(NetInterface *interface)
{
   error_t error;
   uint32_t rsr;

   //Read receive status
   rsr = GMAC->RSR.reg;

   //Packet received?
   if(rsr & (GMAC_RSR_HNO | GMAC_RSR_RXOVR | GMAC_RSR_REC | GMAC_RSR_BNA))
   {
      //Only clear RSR flags that are currently set
      GMAC->RSR.reg = rsr;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = same54EthReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t same54EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > SAME54_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if(!(txBufferDesc[txBufferIndex].status & GMAC_TX_USED))
      return ERROR_FAILURE;

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txBufferIndex], buffer, offset, length);

   //Set the necessary flags in the descriptor entry
   if(txBufferIndex < (SAME54_ETH_TX_BUFFER_COUNT - 1))
   {
      //Write the status word
      txBufferDesc[txBufferIndex].status =
         GMAC_TX_LAST | (length & GMAC_TX_LENGTH);

      //Point to the next buffer
      txBufferIndex++;
   }
   else
   {
      //Write the status word
      txBufferDesc[txBufferIndex].status = GMAC_TX_WRAP |
         GMAC_TX_LAST | (length & GMAC_TX_LENGTH);

      //Wrap around
      txBufferIndex = 0;
   }

   //Data synchronization barrier
   __DSB();

   //Set the TSTART bit to initiate transmission
   GMAC->NCR.reg |= GMAC_NCR_TSTART;

   //Check whether the next buffer is available for writing
   if(txBufferDesc[txBufferIndex].status & GMAC_TX_USED)
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

error_t same54EthReceivePacket(NetInterface *interface)
{
   static uint8_t temp[ETH_MAX_FRAME_SIZE];
   error_t error;
   uint_t i;
   uint_t j;
   uint_t sofIndex;
   uint_t eofIndex;
   size_t n;
   size_t size;
   size_t length;

   //Initialize SOF and EOF indices
   sofIndex = UINT_MAX;
   eofIndex = UINT_MAX;

   //Search for SOF and EOF flags
   for(i = 0; i < SAME54_ETH_RX_BUFFER_COUNT; i++)
   {
      //Point to the current entry
      j = rxBufferIndex + i;

      //Wrap around to the beginning of the buffer if necessary
      if(j >= SAME54_ETH_RX_BUFFER_COUNT)
         j -= SAME54_ETH_RX_BUFFER_COUNT;

      //No more entries to process?
      if(!(rxBufferDesc[j].address & GMAC_RX_OWNERSHIP))
      {
         //Stop processing
         break;
      }
      //A valid SOF has been found?
      if(rxBufferDesc[j].status & GMAC_RX_SOF)
      {
         //Save the position of the SOF
         sofIndex = i;
      }
      //A valid EOF has been found?
      if((rxBufferDesc[j].status & GMAC_RX_EOF) && sofIndex != UINT_MAX)
      {
         //Save the position of the EOF
         eofIndex = i;
         //Retrieve the length of the frame
         size = rxBufferDesc[j].status & GMAC_RX_LENGTH;
         //Limit the number of data to read
         size = MIN(size, ETH_MAX_FRAME_SIZE);
         //Stop processing since we have reached the end of the frame
         break;
      }
   }

   //Determine the number of entries to process
   if(eofIndex != UINT_MAX)
      j = eofIndex + 1;
   else if(sofIndex != UINT_MAX)
      j = sofIndex;
   else
      j = i;

   //Total number of bytes that have been copied from the receive buffer
   length = 0;

   //Process incoming frame
   for(i = 0; i < j; i++)
   {
      //Any data to copy from current buffer?
      if(eofIndex != UINT_MAX && i >= sofIndex && i <= eofIndex)
      {
         //Calculate the number of bytes to read at a time
         n = MIN(size, SAME54_ETH_RX_BUFFER_SIZE);
         //Copy data from receive buffer
         memcpy(temp + length, rxBuffer[rxBufferIndex], n);
         //Update byte counters
         length += n;
         size -= n;
      }

      //Mark the current buffer as free
      rxBufferDesc[rxBufferIndex].address &= ~GMAC_RX_OWNERSHIP;

      //Point to the following entry
      rxBufferIndex++;

      //Wrap around to the beginning of the buffer if necessary
      if(rxBufferIndex >= SAME54_ETH_RX_BUFFER_COUNT)
         rxBufferIndex = 0;
   }

   //Any packet to process?
   if(length > 0)
   {
      //Pass the packet to the upper layer
      nicProcessPacket(interface, temp, length);
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

error_t same54EthUpdateMacAddrFilter(NetInterface *interface)
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
      //The addresse is activated when SAT register is written
      GMAC->Sa[1].SAB.reg = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
      GMAC->Sa[1].SAT.reg = unicastMacAddr[0].w[2];
   }
   else
   {
      //The addresse is activated when SAB register is written
      GMAC->Sa[1].SAB.reg = 0;
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      //The addresse is activated when SAT register is written
      GMAC->Sa[2].SAB.reg = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
      GMAC->Sa[2].SAT.reg = unicastMacAddr[1].w[2];
   }
   else
   {
      //The addresse is activated when SAB register is written
      GMAC->Sa[2].SAB.reg = 0;
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      //The addresse is activated when SAT register is written
      GMAC->Sa[3].SAB.reg = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
      GMAC->Sa[3].SAT.reg = unicastMacAddr[2].w[2];
   }
   else
   {
      //The addresse is activated when SAB register is written
      GMAC->Sa[3].SAB.reg = 0;
   }

   //Configure the multicast address filter
   GMAC->HRB.reg = hashTable[0];
   GMAC->HRT.reg = hashTable[1];

   //Debug message
   TRACE_DEBUG("  HRB = %08" PRIX32 "\r\n", GMAC->HRB.reg);
   TRACE_DEBUG("  HRT = %08" PRIX32 "\r\n", GMAC->HRT.reg);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t same54EthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read network configuration register
   config = GMAC->NCFGR.reg;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
      config |= GMAC_NCFGR_SPD;
   else
      config &= ~GMAC_NCFGR_SPD;

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
      config |= GMAC_NCFGR_FD;
   else
      config &= ~GMAC_NCFGR_FD;

   //Write configuration value back to NCFGR register
   GMAC->NCFGR.reg = config;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @param[in] data Register value
 **/

void same54EthWritePhyReg(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
   uint32_t value;

   //Set up a write operation
   value = GMAC_MAN_CLTTO | GMAC_MAN_OP(1) | GMAC_MAN_WTN(2);
   //PHY address
   value |= GMAC_MAN_PHYA(phyAddr);
   //Register address
   value |= GMAC_MAN_REGA(regAddr);
   //Register value
   value |= GMAC_MAN_DATA(data);

   //Start a write operation
   GMAC->MAN.reg = value;
   //Wait for the write to complete
   while(!(GMAC->NSR.reg & GMAC_NSR_IDLE));
}


/**
 * @brief Read PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t same54EthReadPhyReg(uint8_t phyAddr, uint8_t regAddr)
{
   uint32_t value;

   //Set up a read operation
   value = GMAC_MAN_CLTTO | GMAC_MAN_OP(2) | GMAC_MAN_WTN(2);
   //PHY address
   value |= GMAC_MAN_PHYA(phyAddr);
   //Register address
   value |= GMAC_MAN_REGA(regAddr);

   //Start a read operation
   GMAC->MAN.reg = value;
   //Wait for the read to complete
   while(!(GMAC->NSR.reg & GMAC_NSR_IDLE));

   //Return PHY register contents
   return GMAC->MAN.reg & GMAC_MAN_DATA_Msk;
}
