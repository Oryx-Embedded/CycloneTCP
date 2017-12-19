/**
 * @file same70_eth_driver.c
 * @brief SAME70 Ethernet MAC controller
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.8.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include <limits.h>
#include "same70.h"
#include "core/net.h"
#include "drivers/mac/same70_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//TX buffer
#pragma data_alignment = 8
#pragma location = ".ram_no_cache"
static uint8_t txBuffer[SAME70_ETH_TX_BUFFER_COUNT][SAME70_ETH_TX_BUFFER_SIZE];
//RX buffer
#pragma data_alignment = 8
#pragma location = ".ram_no_cache"
static uint8_t rxBuffer[SAME70_ETH_RX_BUFFER_COUNT][SAME70_ETH_RX_BUFFER_SIZE];
//TX buffer descriptors
#pragma data_alignment = 4
#pragma location = ".ram_no_cache"
static Same70TxBufferDesc txBufferDesc[SAME70_ETH_TX_BUFFER_COUNT];
//RX buffer descriptors
#pragma data_alignment = 4
#pragma location = ".ram_no_cache"
static Same70RxBufferDesc rxBufferDesc[SAME70_ETH_RX_BUFFER_COUNT];

//Dummy TX buffer
#pragma data_alignment = 8
#pragma location = ".ram_no_cache"
static uint8_t dummyTxBuffer[SAME70_ETH_DUMMY_BUFFER_COUNT][SAME70_ETH_DUMMY_BUFFER_SIZE];
//Dummy RX buffer
#pragma data_alignment = 8
#pragma location = ".ram_no_cache"
static uint8_t dummyRxBuffer[SAME70_ETH_DUMMY_BUFFER_COUNT][SAME70_ETH_DUMMY_BUFFER_SIZE];
//Dummy TX buffer descriptors
#pragma data_alignment = 4
#pragma location = ".ram_no_cache"
static Same70TxBufferDesc dummyTxBufferDesc[SAME70_ETH_DUMMY_BUFFER_COUNT];
//Dummy RX buffer descriptors
#pragma data_alignment = 4
#pragma location = ".ram_no_cache"
static Same70RxBufferDesc dummyRxBufferDesc[SAME70_ETH_DUMMY_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//TX buffer
static uint8_t txBuffer[SAME70_ETH_TX_BUFFER_COUNT][SAME70_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(".ram_no_cache")));
//RX buffer
static uint8_t rxBuffer[SAME70_ETH_RX_BUFFER_COUNT][SAME70_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(".ram_no_cache")));
//TX buffer descriptors
static Same70TxBufferDesc txBufferDesc[SAME70_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(".ram_no_cache")));
//RX buffer descriptors
static Same70RxBufferDesc rxBufferDesc[SAME70_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(".ram_no_cache")));

//Dummy TX buffer
static uint8_t dummyTxBuffer[SAME70_ETH_DUMMY_BUFFER_COUNT][SAME70_ETH_DUMMY_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(".ram_no_cache")));
//Dummy RX buffer
static uint8_t dummyRxBuffer[SAME70_ETH_DUMMY_BUFFER_COUNT][SAME70_ETH_DUMMY_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(".ram_no_cache")));
//Dummy TX buffer descriptors
static Same70TxBufferDesc dummyTxBufferDesc[SAME70_ETH_DUMMY_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(".ram_no_cache")));
//Dummy RX buffer descriptors
static Same70RxBufferDesc dummyRxBufferDesc[SAME70_ETH_DUMMY_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(".ram_no_cache")));

#endif

//TX buffer index
static uint_t txBufferIndex;
//RX buffer index
static uint_t rxBufferIndex;


/**
 * @brief SAME70 Ethernet MAC driver
 **/

const NicDriver same70EthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   same70EthInit,
   same70EthTick,
   same70EthEnableIrq,
   same70EthDisableIrq,
   same70EthEventHandler,
   same70EthSendPacket,
   same70EthSetMulticastFilter,
   same70EthUpdateMacConfig,
   same70EthWritePhyReg,
   same70EthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief SAME70 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t same70EthInit(NetInterface *interface)
{
   error_t error;
   volatile uint32_t status;

   //Debug message
   TRACE_INFO("Initializing SAME70 Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable GMAC peripheral clock
   PMC->PMC_PCER1 = (1 << (ID_GMAC - 32));

   //GPIO configuration
   same70EthInitGpio(interface);

   //Configure MDC clock speed
   GMAC->GMAC_NCFGR = GMAC_NCFGR_CLK_MCK_96;
   //Enable management port (MDC and MDIO)
   GMAC->GMAC_NCR |= GMAC_NCR_MPE;

   //PHY transceiver initialization
   error = interface->phyDriver->init(interface);
   //Failed to initialize PHY transceiver?
   if(error)
      return error;

   //Set the MAC address
   GMAC->GMAC_SA[0].GMAC_SAB = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   GMAC->GMAC_SA[0].GMAC_SAT = interface->macAddr.w[2];

   //Configure the receive filter
   GMAC->GMAC_NCFGR |= GMAC_NCFGR_UNIHEN | GMAC_NCFGR_MTIHEN;

   //DMA configuration
   GMAC->GMAC_DCFGR = GMAC_DCFGR_DRBS(SAME70_ETH_RX_BUFFER_SIZE / 64) |
      GMAC_DCFGR_TXPBMS | GMAC_DCFGR_RXBMS_FULL | GMAC_DCFGR_FBLDO_INCR4;

   GMAC->GMAC_RBSRPQ[0] = GMAC_RBSRPQ_RBS(SAME70_ETH_DUMMY_BUFFER_SIZE / 64);
   GMAC->GMAC_RBSRPQ[1] = GMAC_RBSRPQ_RBS(SAME70_ETH_DUMMY_BUFFER_SIZE / 64);

   //Initialize hash table
   GMAC->GMAC_HRB = 0;
   GMAC->GMAC_HRT = 0;

   //Initialize buffer descriptors
   same70EthInitBufferDesc(interface);

   //Clear transmit status register
   GMAC->GMAC_TSR = GMAC_TSR_HRESP | GMAC_TSR_TXCOMP | GMAC_TSR_TFC |
      GMAC_TSR_TXGO | GMAC_TSR_RLE | GMAC_TSR_COL | GMAC_TSR_UBR;
   //Clear receive status register
   GMAC->GMAC_RSR = GMAC_RSR_HNO | GMAC_RSR_RXOVR | GMAC_RSR_REC | GMAC_RSR_BNA;

   //First disable all GMAC interrupts
   GMAC->GMAC_IDR = 0xFFFFFFFF;
   GMAC->GMAC_IDRPQ[0] = 0xFFFFFFFF;
   GMAC->GMAC_IDRPQ[1] = 0xFFFFFFFF;

   //Only the desired ones are enabled
   GMAC->GMAC_IER = GMAC_IER_HRESP | GMAC_IER_ROVR | GMAC_IER_TCOMP | GMAC_IER_TFC |
      GMAC_IER_RLEX | GMAC_IER_TUR | GMAC_IER_RXUBR | GMAC_IER_RCOMP;

   //Read GMAC ISR register to clear any pending interrupt
   status = GMAC->GMAC_ISR;

   //Set priority grouping (3 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(SAME70_ETH_IRQ_PRIORITY_GROUPING);

   //Configure GMAC interrupt priority
   NVIC_SetPriority(GMAC_IRQn, NVIC_EncodePriority(SAME70_ETH_IRQ_PRIORITY_GROUPING,
      SAME70_ETH_IRQ_GROUP_PRIORITY, SAME70_ETH_IRQ_SUB_PRIORITY));

   //Enable the GMAC to transmit and receive data
   GMAC->GMAC_NCR |= GMAC_NCR_TXEN | GMAC_NCR_RXEN;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


//SAME70-Xplained evaluation board?
#if defined(USE_SAME70_XPLAINED)

/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

void same70EthInitGpio(NetInterface *interface)
{
   //Enable PIO peripheral clocks
   PMC->PMC_PCER0 = (1 << ID_PIOC) | (1 << ID_PIOD);

   //Disable pull-up resistors on RMII pins
   PIOD->PIO_PUDR = GMAC_RMII_MASK;
   //Disable interrupts-on-change
   PIOD->PIO_IDR = GMAC_RMII_MASK;
   //Assign RMII pins to peripheral A function
   PIOD->PIO_ABCDSR[0] &= ~GMAC_RMII_MASK;
   PIOD->PIO_ABCDSR[1] &= ~GMAC_RMII_MASK;
   //Disable the PIO from controlling the corresponding pins
   PIOD->PIO_PDR = GMAC_RMII_MASK;

   //Select RMII operation mode
   GMAC->GMAC_UR &= ~GMAC_UR_RMII;

   //Configure PHY_RESET as an output
   PIOC->PIO_PER = PIO_PC10;
   PIOC->PIO_OER = PIO_PC10;

   //Reset PHY transceiver
   PIOC->PIO_CODR = PIO_PC10;
   sleep(10);

   //Take the PHY transceiver out of reset
   PIOC->PIO_SODR = PIO_PC10;
   sleep(10);
}

#endif


/**
 * @brief Initialize buffer descriptors
 * @param[in] interface Underlying network interface
 **/

void same70EthInitBufferDesc(NetInterface *interface)
{
   uint_t i;
   uint32_t address;

   //Initialize TX buffer descriptors
   for(i = 0; i < SAME70_ETH_TX_BUFFER_COUNT; i++)
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
   for(i = 0; i < SAME70_ETH_RX_BUFFER_COUNT; i++)
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

   //Initialize dummy TX buffer descriptors
   for(i = 0; i < SAME70_ETH_DUMMY_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current TX buffer
      address = (uint32_t) dummyTxBuffer[i];
      //Write the address to the descriptor entry
      dummyTxBufferDesc[i].address = address;
      //Initialize status field
      dummyTxBufferDesc[i].status = GMAC_TX_USED;
   }

   //Mark the last descriptor entry with the wrap flag
   dummyTxBufferDesc[i - 1].status |= GMAC_TX_WRAP;

   //Initialize dummy RX buffer descriptors
   for(i = 0; i < SAME70_ETH_DUMMY_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current RX buffer
      address = (uint32_t) dummyRxBuffer[i];
      //Write the address to the descriptor entry
      dummyRxBufferDesc[i].address = (address & GMAC_RX_ADDRESS) | GMAC_RX_OWNERSHIP;
      //Clear status field
      dummyRxBufferDesc[i].status = 0;
   }

   //Mark the last descriptor entry with the wrap flag
   dummyRxBufferDesc[i - 1].address |= GMAC_RX_WRAP;

   //Start location of the TX descriptor list
   GMAC->GMAC_TBQB = (uint32_t) txBufferDesc;
   GMAC->GMAC_TBQBAPQ[0] = (uint32_t) dummyTxBufferDesc;
   GMAC->GMAC_TBQBAPQ[1] = (uint32_t) dummyTxBufferDesc;

   //Start location of the RX descriptor list
   GMAC->GMAC_RBQB = (uint32_t) rxBufferDesc;
   GMAC->GMAC_RBQBAPQ[0] = (uint32_t) dummyRxBufferDesc;
   GMAC->GMAC_RBQBAPQ[1] = (uint32_t) dummyRxBufferDesc;
}


/**
 * @brief SAME70 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void same70EthTick(NetInterface *interface)
{
   //Handle periodic operations
   interface->phyDriver->tick(interface);
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void same70EthEnableIrq(NetInterface *interface)
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

void same70EthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(GMAC_IRQn);
   //Disable Ethernet PHY interrupts
   interface->phyDriver->disableIrq(interface);
}


/**
 * @brief SAME70 Ethernet MAC interrupt service routine
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
   isr = GMAC->GMAC_ISRPQ[0];
   isr = GMAC->GMAC_ISRPQ[1];
   isr = GMAC->GMAC_ISR;
   tsr = GMAC->GMAC_TSR;
   rsr = GMAC->GMAC_RSR;

   //A packet has been transmitted?
   if(tsr & (GMAC_TSR_HRESP | GMAC_TSR_TXCOMP | GMAC_TSR_TFC |
      GMAC_TSR_TXGO | GMAC_TSR_RLE | GMAC_TSR_COL | GMAC_TSR_UBR))
   {
      //Only clear TSR flags that are currently set
      GMAC->GMAC_TSR = tsr;

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
 * @brief SAME70 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void same70EthEventHandler(NetInterface *interface)
{
   error_t error;
   uint32_t rsr;

   //Read receive status
   rsr = GMAC->GMAC_RSR;

   //Packet received?
   if(rsr & (GMAC_RSR_HNO | GMAC_RSR_RXOVR | GMAC_RSR_REC | GMAC_RSR_BNA))
   {
      //Only clear RSR flags that are currently set
      GMAC->GMAC_RSR = rsr;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = same70EthReceivePacket(interface);

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

error_t same70EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
{
   static uint8_t temp[SAME70_ETH_TX_BUFFER_SIZE];
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > SAME70_ETH_TX_BUFFER_SIZE)
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
   netBufferRead(temp, buffer, offset, length);
   memcpy(txBuffer[txBufferIndex], temp, (length + 3) & ~3UL);

   //Set the necessary flags in the descriptor entry
   if(txBufferIndex < (SAME70_ETH_TX_BUFFER_COUNT - 1))
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
   GMAC->GMAC_NCR |= GMAC_NCR_TSTART;

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

error_t same70EthReceivePacket(NetInterface *interface)
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
   for(i = 0; i < SAME70_ETH_RX_BUFFER_COUNT; i++)
   {
      //Point to the current entry
      j = rxBufferIndex + i;

      //Wrap around to the beginning of the buffer if necessary
      if(j >= SAME70_ETH_RX_BUFFER_COUNT)
         j -= SAME70_ETH_RX_BUFFER_COUNT;

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
         n = MIN(size, SAME70_ETH_RX_BUFFER_SIZE);
         //Copy data from receive buffer
         memcpy(temp + length, rxBuffer[rxBufferIndex], (n + 3) & ~3UL);
         //Update byte counters
         length += n;
         size -= n;
      }

      //Mark the current buffer as free
      rxBufferDesc[rxBufferIndex].address &= ~GMAC_RX_OWNERSHIP;

      //Point to the following entry
      rxBufferIndex++;

      //Wrap around to the beginning of the buffer if necessary
      if(rxBufferIndex >= SAME70_ETH_RX_BUFFER_COUNT)
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
 * @brief Configure multicast MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t same70EthSetMulticastFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint8_t *p;
   uint32_t hashTable[2];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating SAME70 hash table...\r\n");

   //Clear hash table
   hashTable[0] = 0;
   hashTable[1] = 0;

   //The MAC filter table contains the multicast MAC addresses
   //to accept when receiving an Ethernet frame
   for(i = 0; i < MAC_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->macMulticastFilter[i];

      //Valid entry?
      if(entry->refCount > 0)
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
   }

   //Write the hash table
   GMAC->GMAC_HRB = hashTable[0];
   GMAC->GMAC_HRT = hashTable[1];

   //Debug message
   TRACE_DEBUG("  HRB = %08" PRIX32 "\r\n", GMAC->GMAC_HRB);
   TRACE_DEBUG("  HRT = %08" PRIX32 "\r\n", GMAC->GMAC_HRT);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t same70EthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read network configuration register
   config = GMAC->GMAC_NCFGR;

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
   GMAC->GMAC_NCFGR = config;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @param[in] data Register value
 **/

void same70EthWritePhyReg(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
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
   GMAC->GMAC_MAN = value;
   //Wait for the write to complete
   while(!(GMAC->GMAC_NSR & GMAC_NSR_IDLE));
}


/**
 * @brief Read PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t same70EthReadPhyReg(uint8_t phyAddr, uint8_t regAddr)
{
   uint32_t value;

   //Set up a read operation
   value = GMAC_MAN_CLTTO | GMAC_MAN_OP(2) | GMAC_MAN_WTN(2);
   //PHY address
   value |= GMAC_MAN_PHYA(phyAddr);
   //Register address
   value |= GMAC_MAN_REGA(regAddr);

   //Start a read operation
   GMAC->GMAC_MAN = value;
   //Wait for the read to complete
   while(!(GMAC->GMAC_NSR & GMAC_NSR_IDLE));

   //Return PHY register contents
   return GMAC->GMAC_MAN & GMAC_MAN_DATA_Msk;
}
