/**
 * @file f2838x_eth_driver.c
 * @brief TMS320F2838xD Ethernet MAC driver
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
#include "inc/hw_emac.h"
#include "inc/hw_emac_ss.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib_cm/interrupt.h"
#include "driverlib_cm/sysctl.h"
#include "core/net.h"
#include "drivers/mac/f2838x_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
static uint8_t txBuffer[F2838X_ETH_TX_BUFFER_COUNT][F2838X_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
static uint8_t rxBuffer[F2838X_ETH_RX_BUFFER_COUNT][F2838X_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
static F2838xTxDmaDesc txDmaDesc[F2838X_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
static F2838xRxDmaDesc rxDmaDesc[F2838X_ETH_RX_BUFFER_COUNT];

//GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[F2838X_ETH_TX_BUFFER_COUNT][F2838X_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Receive buffer
static uint8_t rxBuffer[F2838X_ETH_RX_BUFFER_COUNT][F2838X_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Transmit DMA descriptors
static F2838xTxDmaDesc txDmaDesc[F2838X_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//Receive DMA descriptors
static F2838xRxDmaDesc rxDmaDesc[F2838X_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//Current transmit descriptor
static uint_t txIndex;
//Current receive descriptor
static uint_t rxIndex;


/**
 * @brief TMS320F2838xD Ethernet MAC driver
 **/

const NicDriver f2838xEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   f2838xEthInit,
   f2838xEthTick,
   f2838xEthEnableIrq,
   f2838xEthDisableIrq,
   f2838xEthEventHandler,
   f2838xEthSendPacket,
   f2838xEthUpdateMacAddrFilter,
   f2838xEthUpdateMacConfig,
   f2838xEthWritePhyReg,
   f2838xEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief TMS320F2838xD Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t f2838xEthInit(NetInterface *interface)
{
   error_t error;
   uint32_t temp;
#ifdef ti_sysbios_BIOS___VERS
   Hwi_Params hwiParams;
#endif

   //Debug message
   TRACE_INFO("Initializing TMS320F2838xD Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable Ethernet peripheral clock
   SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_ENET);
   //Reset Ethernet peripheral
   SysCtl_resetPeripheral(SYSCTL_PERIPH_RES_ENET);

   //GPIO configuration
   f2838xEthInitGpio(interface);

   //Perform a software reset
   ETHERNET_DMA_MODE_R |= ETHERNET_DMA_MODE_SWR;
   //Wait for the reset to complete
   while((ETHERNET_DMA_MODE_R & ETHERNET_DMA_MODE_SWR) != 0)
   {
   }

   //Adjust MDC clock range depending on CSR frequency
   ETHERNET_MAC_MDIO_ADDRESS_R = (4 << ETHERNET_MAC_MDIO_ADDRESS_CR_S);

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

   //Startup delay
   sleep(10);

   //Use default MAC configuration
   ETHERNET_MAC_CONFIGURATION_R = ETHERNET_MAC_CONFIGURATION_GPSLCE |
      ETHERNET_MAC_CONFIGURATION_PS | ETHERNET_MAC_CONFIGURATION_DO;

   //Set the maximum packet size that can be accepted
   temp = ETHERNET_MAC_EXT_CONFIGURATION_R & ~ETHERNET_MAC_EXT_CONFIGURATION_GPSL_M;
   ETHERNET_MAC_EXT_CONFIGURATION_R = temp | F2838X_ETH_RX_BUFFER_SIZE;

   //Configure MAC address filtering
   f2838xEthUpdateMacAddrFilter(interface);

   //Disable flow control
   ETHERNET_MAC_Q0_TX_FLOW_CTRL_R = 0;
   ETHERNET_MAC_RX_FLOW_CTRL_R = 0;

   //Enable the first RX queue
   ETHERNET_MAC_RXQ_CTRL0_R = (2 << ETHERNET_MAC_RXQ_CTRL0_RXQ0EN_S);

   //Configure DMA operating mode
   ETHERNET_DMA_MODE_R = (0 << ETHERNET_DMA_MODE_INTM_S) |
      (0 << ETHERNET_DMA_MODE_PR_S);

   //Configure system bus mode
   ETHERNET_DMA_SYSBUS_MODE_R |= ETHERNET_DMA_SYSBUS_MODE_AAL;

   //The DMA takes the descriptor table as contiguous
   ETHERNET_DMA_CH0_CONTROL_R = (0 << ETHERNET_DMA_CH0_CONTROL_DSL_S);
   //Configure TX features
   ETHERNET_DMA_CH0_TX_CONTROL_R = (32 << ETHERNET_DMA_CH0_TX_CONTROL_TXPBL_S);

   //Configure RX features
   ETHERNET_DMA_CH0_RX_CONTROL_R = (32 << ETHERNET_DMA_CH0_RX_CONTROL_RXPBL_S) |
      ((F2838X_ETH_RX_BUFFER_SIZE / 4) << ETHERNET_DMA_CH0_RX_CONTROL_RBSZ_S);

   //Enable store and forward mode for transmission
   ETHERNET_MTL_TXQ0_OPERATION_MODE_R |= (7 << ETHERNET_MTL_TXQ0_OPERATION_MODE_TQS_S) |
      (2 << ETHERNET_MTL_TXQ0_OPERATION_MODE_TXQEN_S) |
      ETHERNET_MTL_TXQ0_OPERATION_MODE_TSF;

   //Enable store and forward mode for reception
   ETHERNET_MTL_RXQ0_OPERATION_MODE_R |= (7 << ETHERNET_MTL_RXQ0_OPERATION_MODE_RQS_S) |
      ETHERNET_MTL_RXQ0_OPERATION_MODE_RSF;

   //Initialize DMA descriptor lists
   f2838xEthInitDmaDesc(interface);

   //Prevent interrupts from being generated when statistic counters reach
   //half their maximum value
   ETHERNET_MMC_TX_INTERRUPT_MASK_R = 0xFFFFFFFF;
   ETHERNET_MMC_RX_INTERRUPT_MASK_R = 0xFFFFFFFF;
   ETHERNET_MMC_IPC_RX_INTERRUPT_MASK_R = 0xFFFFFFFF;

   //Disable MAC interrupts
   ETHERNET_MAC_INTERRUPT_ENABLE_R = 0;

   //Enable the desired DMA interrupts
   ETHERNET_DMA_CH0_INTERRUPT_ENABLE_R = ETHERNET_DMA_CH0_INTERRUPT_ENABLE_NIE |
      ETHERNET_DMA_CH0_INTERRUPT_ENABLE_RIE | ETHERNET_DMA_CH0_INTERRUPT_ENABLE_TIE;

#ifdef ti_sysbios_BIOS___VERS
   //Configure Ethernet interrupt
   Hwi_Params_init(&hwiParams);
   hwiParams.enableInt = FALSE;
   hwiParams.priority = F2838X_ETH_IRQ_PRIORITY;

   //Register interrupt handler
   Hwi_create(INT_EMAC, (Hwi_FuncPtr) f2838xEthIrqHandler, &hwiParams, NULL);
#else
   //Register interrupt handler
   Interrupt_registerHandler(INT_EMAC, f2838xEthIrqHandler);
   //Configure Ethernet interrupt priority
   Interrupt_setPriority(INT_EMAC, F2838X_ETH_IRQ_PRIORITY);
#endif

   //Enable MAC transmission and reception
   ETHERNET_MAC_CONFIGURATION_R |= ETHERNET_MAC_CONFIGURATION_TE |
      ETHERNET_MAC_CONFIGURATION_RE;

   //Enable DMA transmission and reception
   ETHERNET_DMA_CH0_TX_CONTROL_R |= ETHERNET_DMA_CH0_TX_CONTROL_ST;
   ETHERNET_DMA_CH0_RX_CONTROL_R |= ETHERNET_DMA_CH0_RX_CONTROL_SR;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void f2838xEthInitGpio(NetInterface *interface)
{
   //Select MII interface mode
   ETHERNETSS_CTRLSTS_R = (0xA5U << ETHERNETSS_CTRLSTS_WRITE_KEY_S) |
      (0 << ETHERNETSS_CTRLSTS_PHY_INTF_SEL_S);
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void f2838xEthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < F2838X_ETH_TX_BUFFER_COUNT; i++)
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
   for(i = 0; i < F2838X_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = (uint32_t) rxBuffer[i];
      rxDmaDesc[i].rdes1 = 0;
      rxDmaDesc[i].rdes2 = 0;
      rxDmaDesc[i].rdes3 = ETH_RDES3_OWN | ETH_RDES3_IOC | ETH_RDES3_BUF1V;
   }

   //Initialize RX descriptor index
   rxIndex = 0;

   //Start location of the TX descriptor list
   ETHERNET_DMA_CH0_TXDESC_LIST_ADDRESS_R = (uint32_t) &txDmaDesc[0];
   //Length of the transmit descriptor ring
   ETHERNET_DMA_CH0_TXDESC_RING_LENGTH_R = F2838X_ETH_TX_BUFFER_COUNT - 1;

   //Start location of the RX descriptor list
   ETHERNET_DMA_CH0_RXDESC_LIST_ADDRESS_R = (uint32_t) &rxDmaDesc[0];
   //Length of the receive descriptor ring
   ETHERNET_DMA_CH0_RXDESC_RING_LENGTH_R = F2838X_ETH_RX_BUFFER_COUNT - 1;
}


/**
 * @brief TMS320F2838xD Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void f2838xEthTick(NetInterface *interface)
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

void f2838xEthEnableIrq(NetInterface *interface)
{
#ifdef ti_sysbios_BIOS___VERS
   //Enable Ethernet MAC interrupts
   Hwi_enableInterrupt(INT_EMAC);
#else
   //Enable Ethernet MAC interrupts
   Interrupt_enable(INT_EMAC);
#endif

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

void f2838xEthDisableIrq(NetInterface *interface)
{
#ifdef ti_sysbios_BIOS___VERS
   //Disable Ethernet MAC interrupts
   Hwi_disableInterrupt(INT_EMAC);
#else
   //Disable Ethernet MAC interrupts
   Interrupt_disable(INT_EMAC);
#endif

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
 * @brief TMS320F2838xD Ethernet MAC interrupt service routine
 **/

__interrupt void f2838xEthIrqHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = ETHERNET_DMA_CH0_STATUS_R;

   //Packet transmitted?
   if((status & ETHERNET_DMA_CH0_STATUS_TI) != 0)
   {
      //Clear TI interrupt flag
      ETHERNET_DMA_CH0_STATUS_R = ETHERNET_DMA_CH0_STATUS_TI;

      //Check whether the TX buffer is available for writing
      if((txDmaDesc[txIndex].tdes3 & ETH_TDES3_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & ETHERNET_DMA_CH0_STATUS_RI) != 0)
   {
      //Clear RI interrupt flag
      ETHERNET_DMA_CH0_STATUS_R = ETHERNET_DMA_CH0_STATUS_RI;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   ETHERNET_DMA_CH0_STATUS_R = ETHERNET_DMA_CH0_STATUS_NIS;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief TMS320F2838xD Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void f2838xEthEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = f2838xEthReceivePacket(interface);

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

error_t f2838xEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > F2838X_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txDmaDesc[txIndex].tdes3 & ETH_TDES3_OWN) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txIndex], buffer, offset, length);

   //Set the start address of the buffer
   txDmaDesc[txIndex].tdes0 = (uint32_t) txBuffer[txIndex];
   //Write the number of bytes to send
   txDmaDesc[txIndex].tdes2 = ETH_TDES2_IOC | (length & ETH_TDES2_B1L);
   //Give the ownership of the descriptor to the DMA
   txDmaDesc[txIndex].tdes3 = ETH_TDES3_OWN | ETH_TDES3_FD | ETH_TDES3_LD;

   //Clear TBU flag to resume processing
   ETHERNET_DMA_CH0_STATUS_R = ETHERNET_DMA_CH0_STATUS_TBU;
   //Instruct the DMA to poll the transmit descriptor list
   ETHERNET_DMA_CH0_TXDESC_TAIL_POINTER_R = 0;

   //Increment index and wrap around if necessary
   if(++txIndex >= F2838X_ETH_TX_BUFFER_COUNT)
   {
      txIndex = 0;
   }

   //Check whether the next buffer is available for writing
   if((txDmaDesc[txIndex].tdes3 & ETH_TDES3_OWN) == 0)
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

error_t f2838xEthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_OWN) == 0)
   {
      //FD and LD flags should be set
      if((rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_FD) != 0 &&
         (rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_LD) != 0)
      {
         //Make sure no error occurred
         if((rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_ES) == 0)
         {
            //Retrieve the length of the frame
            n = rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_PL;
            //Limit the number of data to read
            n = MIN(n, F2838X_ETH_RX_BUFFER_SIZE);

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
      rxDmaDesc[rxIndex].rdes0 = (uint32_t) rxBuffer[rxIndex];
      //Give the ownership of the descriptor back to the DMA
      rxDmaDesc[rxIndex].rdes3 = ETH_RDES3_OWN | ETH_RDES3_IOC | ETH_RDES3_BUF1V;

      //Increment index and wrap around if necessary
      if(++rxIndex >= F2838X_ETH_RX_BUFFER_COUNT)
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
   ETHERNET_DMA_CH0_STATUS_R = ETHERNET_DMA_CH0_STATUS_RBU;
   //Instruct the DMA to poll the receive descriptor list
   ETHERNET_DMA_CH0_RXDESC_TAIL_POINTER_R = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t f2838xEthUpdateMacAddrFilter(NetInterface *interface)
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
      ETHERNET_MAC_PACKET_FILTER_R = ETHERNET_MAC_PACKET_FILTER_PR;
   }
   else
   {
      //Set the MAC address of the station
      ETHERNET_MAC_ADDRESS0_LOW_R = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
      ETHERNET_MAC_ADDRESS0_HIGH_R = interface->macAddr.w[2];

      //The MAC supports 7 additional addresses for unicast perfect filtering
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
               crc = f2838xEthCalcCrc(&entry->addr, sizeof(MacAddr));

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
         ETHERNET_MAC_ADDRESS1_LOW_R = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
         ETHERNET_MAC_ADDRESS1_HIGH_R = unicastMacAddr[0].w[2] | ETHERNET_MAC_ADDRESS1_HIGH_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         ETHERNET_MAC_ADDRESS1_LOW_R = 0;
         ETHERNET_MAC_ADDRESS1_HIGH_R = 0;
      }

      //Configure the second unicast address filter
      if(j >= 2)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         ETHERNET_MAC_ADDRESS2_LOW_R = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
         ETHERNET_MAC_ADDRESS2_HIGH_R = unicastMacAddr[1].w[2] | ETHERNET_MAC_ADDRESS2_HIGH_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         ETHERNET_MAC_ADDRESS2_LOW_R = 0;
         ETHERNET_MAC_ADDRESS2_HIGH_R = 0;
      }

      //Configure the third unicast address filter
      if(j >= 3)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         ETHERNET_MAC_ADDRESS3_LOW_R = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
         ETHERNET_MAC_ADDRESS3_HIGH_R = unicastMacAddr[2].w[2] | ETHERNET_MAC_ADDRESS3_HIGH_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         ETHERNET_MAC_ADDRESS3_LOW_R = 0;
         ETHERNET_MAC_ADDRESS3_HIGH_R = 0;
      }

      //Check whether frames with a multicast destination address should be
      //accepted
      if(interface->acceptAllMulticast)
      {
         //Configure the receive filter
         ETHERNET_MAC_PACKET_FILTER_R = ETHERNET_MAC_PACKET_FILTER_HPF |
            ETHERNET_MAC_PACKET_FILTER_PM;
      }
      else
      {
         //Configure the receive filter
         ETHERNET_MAC_PACKET_FILTER_R = ETHERNET_MAC_PACKET_FILTER_HPF |
            ETHERNET_MAC_PACKET_FILTER_HMC;

         //Configure the multicast hash table
         ETHERNET_MAC_HASH_TABLE_REG0_R = hashTable[0];
         ETHERNET_MAC_HASH_TABLE_REG1_R = hashTable[1];

         //Debug message
         TRACE_DEBUG("  MAC_HASH_TABLE_REG0 = %08" PRIX32 "\r\n", ETHERNET_MAC_HASH_TABLE_REG0_R);
         TRACE_DEBUG("  MAC_HASH_TABLE_REG1 = %08" PRIX32 "\r\n", ETHERNET_MAC_HASH_TABLE_REG1_R);
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

error_t f2838xEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = ETHERNET_MAC_CONFIGURATION_R;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= ETHERNET_MAC_CONFIGURATION_FES;
   }
   else
   {
      config &= ~ETHERNET_MAC_CONFIGURATION_FES;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= ETHERNET_MAC_CONFIGURATION_DM;
   }
   else
   {
      config &= ~ETHERNET_MAC_CONFIGURATION_DM;
   }

   //Update MAC configuration register
   ETHERNET_MAC_CONFIGURATION_R = config;

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

void f2838xEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = ETHERNET_MAC_MDIO_ADDRESS_R & ETHERNET_MAC_MDIO_ADDRESS_CR_M;
      //Set up a write operation
      temp |= ETHERNET_MAC_MDIO_ADDRESS_GOC_0 | ETHERNET_MAC_MDIO_ADDRESS_GB;

      //PHY address
      temp |= (phyAddr <<ETHERNET_MAC_MDIO_ADDRESS_PA_S) &
         ETHERNET_MAC_MDIO_ADDRESS_PA_M;

      //Register address
      temp |= (regAddr << ETHERNET_MAC_MDIO_ADDRESS_RDA_S) &
         ETHERNET_MAC_MDIO_ADDRESS_RDA_M;

      //Data to be written in the PHY register
      ETHERNET_MAC_MDIO_DATA_R = data & ETHERNET_MAC_MDIO_DATA_GD_M;

      //Start a write operation
      ETHERNET_MAC_MDIO_ADDRESS_R = temp;
      //Wait for the write to complete
      while((ETHERNET_MAC_MDIO_ADDRESS_R & ETHERNET_MAC_MDIO_ADDRESS_GB) != 0)
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

uint16_t f2838xEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = ETHERNET_MAC_MDIO_ADDRESS_R & ETHERNET_MAC_MDIO_ADDRESS_CR_M;

      //Set up a read operation
      temp |= ETHERNET_MAC_MDIO_ADDRESS_GOC_1 |
         ETHERNET_MAC_MDIO_ADDRESS_GOC_0 | ETHERNET_MAC_MDIO_ADDRESS_GB;

      //PHY address
      temp |= (phyAddr <<ETHERNET_MAC_MDIO_ADDRESS_PA_S) &
         ETHERNET_MAC_MDIO_ADDRESS_PA_M;

      //Register address
      temp |= (regAddr << ETHERNET_MAC_MDIO_ADDRESS_RDA_S) &
         ETHERNET_MAC_MDIO_ADDRESS_RDA_M;

      //Start a read operation
      ETHERNET_MAC_MDIO_ADDRESS_R = temp;
      //Wait for the read to complete
      while((ETHERNET_MAC_MDIO_ADDRESS_R & ETHERNET_MAC_MDIO_ADDRESS_GB) != 0)
      {
      }

      //Get register value
      data = ETHERNET_MAC_MDIO_DATA_R & ETHERNET_MAC_MDIO_DATA_GD_M;
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

uint32_t f2838xEthCalcCrc(const void *data, size_t length)
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
