/**
 * @file mcxe31b_eth_driver.c
 * @brief NXP MCX E31B Ethernet MAC driver
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
#include "fsl_device_registers.h"
#include "fsl_clock.h"
#include "fsl_memory.h"
#include "fsl_siul2.h"
#include "core/net.h"
#include "drivers/mac/mcxe31b_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
#pragma location = MCXE31B_ETH_RAM_SECTION
static uint8_t txBuffer[MCXE31B_ETH_TX_BUFFER_COUNT][MCXE31B_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
#pragma location = MCXE31B_ETH_RAM_SECTION
static uint8_t rxBuffer[MCXE31B_ETH_RX_BUFFER_COUNT][MCXE31B_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
#pragma location = MCXE31B_ETH_RAM_SECTION
static Mcxe31bTxDmaDesc txDmaDesc[MCXE31B_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
#pragma location = MCXE31B_ETH_RAM_SECTION
static Mcxe31bRxDmaDesc rxDmaDesc[MCXE31B_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[MCXE31B_ETH_TX_BUFFER_COUNT][MCXE31B_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(MCXE31B_ETH_RAM_SECTION)));
//Receive buffer
static uint8_t rxBuffer[MCXE31B_ETH_RX_BUFFER_COUNT][MCXE31B_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(MCXE31B_ETH_RAM_SECTION)));
//Transmit DMA descriptors
static Mcxe31bTxDmaDesc txDmaDesc[MCXE31B_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(MCXE31B_ETH_RAM_SECTION)));
//Receive DMA descriptors
static Mcxe31bRxDmaDesc rxDmaDesc[MCXE31B_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(MCXE31B_ETH_RAM_SECTION)));

#endif

//Current transmit descriptor
static uint_t txIndex;
//Current receive descriptor
static uint_t rxIndex;


/**
 * @brief MCX E31B Ethernet MAC driver
 **/

const NicDriver mcxe31bEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   mcxe31bEthInit,
   mcxe31bEthTick,
   mcxe31bEthEnableIrq,
   mcxe31bEthDisableIrq,
   mcxe31bEthEventHandler,
   mcxe31bEthSendPacket,
   mcxe31bEthUpdateMacAddrFilter,
   mcxe31bEthUpdateMacConfig,
   mcxe31bEthWritePhyReg,
   mcxe31bEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief MCX E31B Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mcxe31bEthInit(NetInterface *interface)
{
   error_t error;
   uint32_t temp;

   //Debug message
   TRACE_INFO("Initializing MCX E31B Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   mcxe31bEthInitGpio(interface);

   //Enable EMAC peripheral clock
   CLOCK_EnableClock(kCLOCK_Emac);

   //Perform a software reset
   EMAC->DMA_MODE |= EMAC_DMA_MODE_SWR_MASK;
   //Wait for the reset to complete
   while((EMAC->DMA_MODE & EMAC_DMA_MODE_SWR_MASK) != 0)
   {
   }

   //Adjust MDC clock range depending on CSR frequency
   EMAC->MAC_MDIO_ADDRESS = EMAC_MAC_MDIO_ADDRESS_CR(4);

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
   EMAC->MAC_CONFIGURATION = EMAC_MAC_CONFIGURATION_GPSLCE_MASK |
      EMAC_MAC_CONFIGURATION_PS_MASK | EMAC_MAC_CONFIGURATION_DO_MASK;

   //Set the maximum packet size that can be accepted
   temp = EMAC->MAC_EXT_CONFIGURATION & ~EMAC_MAC_EXT_CONFIGURATION_GPSL_MASK;
   EMAC->MAC_EXT_CONFIGURATION = temp | MCXE31B_ETH_RX_BUFFER_SIZE;

   //Configure MAC address filtering
   mcxe31bEthUpdateMacAddrFilter(interface);

   //Disable flow control
   EMAC->MAC_TX_FLOW_CTRL_Q[0] = 0;
   EMAC->MAC_RX_FLOW_CTRL = 0;

   //Enable the first RX queue
   EMAC->MAC_RXQ_CTRL[0] = EMAC_MAC_RXQ_CTRL_RXQ0EN(2);

   //Configure DMA operating mode
   EMAC->DMA_MODE = EMAC_DMA_MODE_PR(0);
   //Configure system bus mode
   EMAC->DMA_SYSBUS_MODE |= EMAC_DMA_SYSBUS_MODE_AAL_MASK;

   //The DMA takes the descriptor table as contiguous
   EMAC->DMA_CH[0].DMA_CHX_CTRL = EMAC_DMA_CHX_CTRL_DSL(0);
   //Configure TX features
   EMAC->DMA_CH[0].DMA_CHX_TX_CTRL = EMAC_DMA_CHX_TX_CTRL_TxPBL(32);

   //Configure RX features
   EMAC->DMA_CH[0].DMA_CHX_RX_CTRL = EMAC_DMA_CHX_RX_CTRL_RxPBL(32) |
      EMAC_DMA_CHX_RX_CTRL_RBSZ_13_y(MCXE31B_ETH_RX_BUFFER_SIZE / 4);

   //Enable store and forward mode for transmission
   EMAC->MTL_QUEUE[0].MTL_TXQX_OP_MODE |= EMAC_MTL_TXQX_OP_MODE_TQS(7) |
      EMAC_MTL_TXQX_OP_MODE_TXQEN(2) | EMAC_MTL_TXQX_OP_MODE_TSF_MASK;

   //Enable store and forward mode for reception
   EMAC->MTL_QUEUE[0].MTL_RXQX_OP_MODE |= EMAC_MTL_RXQX_OP_MODE_RQS(7) |
      EMAC_MTL_RXQX_OP_MODE_RSF_MASK;

   //Initialize DMA descriptor lists
   mcxe31bEthInitDmaDesc(interface);

   //Prevent interrupts from being generated when statistic counters reach
   //half their maximum value
   EMAC->MMC_TX_INTERRUPT_MASK = 0xFFFFFFFF;
   EMAC->MMC_RX_INTERRUPT_MASK = 0xFFFFFFFF;
   EMAC->MMC_FPE_TX_INTERRUPT_MASK = 0xFFFFFFFF;
   EMAC->MMC_FPE_RX_INTERRUPT_MASK = 0xFFFFFFFF;

   //Disable MAC interrupts
   EMAC->MAC_INTERRUPT_ENABLE = 0;

   //Enable the desired DMA interrupts
   EMAC->DMA_CH[0].DMA_CHX_INT_EN = EMAC_DMA_CHX_INT_EN_NIE_MASK |
      EMAC_DMA_CHX_INT_EN_RIE_MASK | EMAC_DMA_CHX_INT_EN_TIE_MASK;

   //Set priority grouping (3 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(MCXE31B_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(EMAC_0_IRQn, NVIC_EncodePriority(MCXE31B_ETH_IRQ_PRIORITY_GROUPING,
      MCXE31B_ETH_IRQ_GROUP_PRIORITY, MCXE31B_ETH_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   EMAC->MAC_CONFIGURATION |= EMAC_MAC_CONFIGURATION_TE_MASK |
      EMAC_MAC_CONFIGURATION_RE_MASK;

   //Enable DMA transmission and reception
   EMAC->DMA_CH[0].DMA_CHX_TX_CTRL |= EMAC_DMA_CHX_TX_CTRL_ST_MASK;
   EMAC->DMA_CH[0].DMA_CHX_RX_CTRL |= EMAC_DMA_CHX_RX_CTRL_SR_MASK;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void mcxe31bEthInitGpio(NetInterface *interface)
{
//FRDM-MCXE31B evaluation board?
#if defined(USE_FRDM_MCXE31B)
   //Select RMII interface mode
   DCM_GPR->DCMRWF1 |= DCM_GPR_DCMRWF1_RMII_MII_SEL_MASK;

   //Configure MII_RMII_TX_CLK (PTD11)
   SIUL2_SetPinInputBuffer(SIUL2, 107, true, 296, kPORT_INPUT_MUX_ALT1);

   //Configure MII_RMII_TX_EN (PTD12)
   SIUL2_SetPinOutputBuffer(SIUL2, 108, true, kPORT_MUX_ALT5);
   //Configure MII_RMII_TXD0 (PTC2)
   SIUL2_SetPinOutputBuffer(SIUL2, 66, true, kPORT_MUX_ALT5);
   //Configure MII_RMII_TXD1 (PTD7)
   SIUL2_SetPinOutputBuffer(SIUL2, 103, true, kPORT_MUX_ALT5);

   //Configure MII_RMII_RX_DV (PTC17)
   SIUL2_SetPinInputBuffer(SIUL2, 81, true, 292, kPORT_INPUT_MUX_ALT1);
   //Configure MII_RMII_RXD0 (PTC1)
   SIUL2_SetPinInputBuffer(SIUL2, 65, true, 294, kPORT_INPUT_MUX_ALT1);
   //Configure MII_RMII_RXD1 (PTC0)
   SIUL2_SetPinInputBuffer(SIUL2, 64, true, 295, kPORT_INPUT_MUX_ALT1);

   //Configure MII_RMII_MDC (PTB5)
   SIUL2_SetPinOutputBuffer(SIUL2, 37, true, kPORT_MUX_ALT7);

   //Configure MII_RMII_MDIO (PTB4)
   SIUL2_SetPinOutputBuffer(SIUL2, 36, true, kPORT_MUX_ALT5);
   SIUL2_SetPinInputBuffer(SIUL2, 36, true, 291, kPORT_INPUT_MUX_ALT1);

   //Configure ENET_PHY_RST (PTC3) as an output
   SIUL2_SetPinOutputBuffer(SIUL2, 67, true, kPORT_MUX_AS_GPIO);

   //Reset PHY transceiver (hard reset)
   SIUL2_PortPinWrite(SIUL2, kSIUL2_PTC, 3, 0);
   sleep(10);
   SIUL2_PortPinWrite(SIUL2, kSIUL2_PTC, 3, 1);
   sleep(10);

   //Configure RMII clocks
   CLOCK_SetEmacRmiiTxClkFreq(50000000);
   CLOCK_AttachClk(kEMAC_RMII_TX_CLK_to_EMAC_TX);
   CLOCK_AttachClk(kEMAC_RMII_TX_CLK_to_EMAC_RX);
   CLOCK_AttachClk(kEMAC_RMII_TX_CLK_to_EMAC_TS);
   CLOCK_SetClkDiv(kCLOCK_DivEmacRxClk, 2);
   CLOCK_SetClkDiv(kCLOCK_DivEmacTxClk, 2);
   CLOCK_SetClkDiv(kCLOCK_DivEmacTsClk, 1);
#endif
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void mcxe31bEthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < MCXE31B_ETH_TX_BUFFER_COUNT; i++)
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
   for(i = 0; i < MCXE31B_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = MEMORY_ConvertMemoryMapAddress(
         (uint32_t) rxBuffer[i], kMEMORY_Local2DMA);

      rxDmaDesc[i].rdes1 = 0;
      rxDmaDesc[i].rdes2 = 0;
      rxDmaDesc[i].rdes3 = EMAC_RDES3_OWN | EMAC_RDES3_IOC | EMAC_RDES3_BUF1V;
   }

   //Initialize RX descriptor index
   rxIndex = 0;

   //Start location of the TX descriptor list
   EMAC->DMA_CH[0].DMA_CHX_TXDESC_LIST_ADDR = MEMORY_ConvertMemoryMapAddress(
      (uint32_t) &txDmaDesc[0], kMEMORY_Local2DMA);

   //Length of the transmit descriptor ring
   EMAC->DMA_CH[0].DMA_CHX_TXDESC_RING_LENGTH = MCXE31B_ETH_TX_BUFFER_COUNT - 1;

   //Start location of the RX descriptor list
   EMAC->DMA_CH[0].DMA_CHX_RXDESC_LIST_ADDR = MEMORY_ConvertMemoryMapAddress(
      (uint32_t) &rxDmaDesc[0], kMEMORY_Local2DMA);

   //Length of the receive descriptor ring
   EMAC->DMA_CH[0].DMA_CHX_RXDESC_RING_LENGTH = MCXE31B_ETH_RX_BUFFER_COUNT - 1;
}


/**
 * @brief MCX E31B Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void mcxe31bEthTick(NetInterface *interface)
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

void mcxe31bEthEnableIrq(NetInterface *interface)
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

void mcxe31bEthDisableIrq(NetInterface *interface)
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
 * @brief MCX E31B Ethernet MAC interrupt service routine
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
   status = EMAC->DMA_CH[0].DMA_CHX_STAT;

   //Packet transmitted?
   if((status & EMAC_DMA_CHX_STAT_TI_MASK) != 0)
   {
      //Clear TI interrupt flag
      EMAC->DMA_CH[0].DMA_CHX_STAT = EMAC_DMA_CHX_STAT_TI_MASK;

      //Check whether the TX buffer is available for writing
      if((txDmaDesc[txIndex].tdes3 & EMAC_TDES3_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & EMAC_DMA_CHX_STAT_RI_MASK) != 0)
   {
      //Clear RI interrupt flag
      EMAC->DMA_CH[0].DMA_CHX_STAT = EMAC_DMA_CHX_STAT_RI_MASK;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&nicDriverInterface->netContext->event);
   }

   //Clear NIS interrupt flag
   EMAC->DMA_CH[0].DMA_CHX_STAT = EMAC_DMA_CHX_STAT_NIS_MASK;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief MCX E31B Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void mcxe31bEthEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = mcxe31bEthReceivePacket(interface);

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

error_t mcxe31bEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > MCXE31B_ETH_TX_BUFFER_SIZE)
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
   txDmaDesc[txIndex].tdes0 = MEMORY_ConvertMemoryMapAddress(
      (uint32_t) txBuffer[txIndex], kMEMORY_Local2DMA);

   //Write the number of bytes to send
   txDmaDesc[txIndex].tdes2 = EMAC_TDES2_IOC | (length & EMAC_TDES2_B1L);
   //Give the ownership of the descriptor to the DMA
   txDmaDesc[txIndex].tdes3 = EMAC_TDES3_OWN | EMAC_TDES3_FD | EMAC_TDES3_LD;

   //Data synchronization barrier
   __DSB();

   //Clear TBU flag to resume processing
   EMAC->DMA_CH[0].DMA_CHX_STAT = EMAC_DMA_CHX_STAT_TBU_MASK;
   //Instruct the DMA to poll the transmit descriptor list
   EMAC->DMA_CH[0].DMA_CHX_TXDESC_TAIL_PTR = 0;

   //Increment index and wrap around if necessary
   if(++txIndex >= MCXE31B_ETH_TX_BUFFER_COUNT)
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

error_t mcxe31bEthReceivePacket(NetInterface *interface)
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
            n = MIN(n, MCXE31B_ETH_RX_BUFFER_SIZE);

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
      rxDmaDesc[rxIndex].rdes0 = MEMORY_ConvertMemoryMapAddress(
         (uint32_t) rxBuffer[rxIndex], kMEMORY_Local2DMA);

      //Give the ownership of the descriptor back to the DMA
      rxDmaDesc[rxIndex].rdes3 = EMAC_RDES3_OWN | EMAC_RDES3_IOC | EMAC_RDES3_BUF1V;

      //Increment index and wrap around if necessary
      if(++rxIndex >= MCXE31B_ETH_RX_BUFFER_COUNT)
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
   EMAC->DMA_CH[0].DMA_CHX_STAT = EMAC_DMA_CHX_STAT_RBU_MASK;
   //Instruct the DMA to poll the receive descriptor list
   EMAC->DMA_CH[0].DMA_CHX_RXDESC_TAIL_PTR = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mcxe31bEthUpdateMacAddrFilter(NetInterface *interface)
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
      EMAC->MAC_PACKET_FILTER = EMAC_MAC_PACKET_FILTER_PR_MASK;
   }
   else
   {
      //Set the MAC address of the station
      EMAC->MAC_ADDRESS[0].LOW = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
      EMAC->MAC_ADDRESS[0].HIGH = interface->macAddr.w[2];

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
               crc = mcxe31bEthCalcCrc(&entry->addr, sizeof(MacAddr));

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
         EMAC->MAC_ADDRESS[1].LOW = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
         EMAC->MAC_ADDRESS[1].HIGH = unicastMacAddr[0].w[2] | EMAC_HIGH_AE_MASK;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         EMAC->MAC_ADDRESS[1].LOW = 0;
         EMAC->MAC_ADDRESS[1].HIGH = 0;
      }

      //Configure the second unicast address filter
      if(j >= 2)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         EMAC->MAC_ADDRESS[2].LOW = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
         EMAC->MAC_ADDRESS[2].HIGH = unicastMacAddr[1].w[2] | EMAC_HIGH_AE_MASK;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         EMAC->MAC_ADDRESS[2].LOW = 0;
         EMAC->MAC_ADDRESS[2].HIGH = 0;
      }

      //Check whether frames with a multicast destination address should be
      //accepted
      if(interface->acceptAllMulticast)
      {
         //Configure the receive filter
         EMAC->MAC_PACKET_FILTER = EMAC_MAC_PACKET_FILTER_HPF_MASK |
            EMAC_MAC_PACKET_FILTER_PM_MASK;
      }
      else
      {
         //Configure the receive filter
         EMAC->MAC_PACKET_FILTER = EMAC_MAC_PACKET_FILTER_HPF_MASK |
            EMAC_MAC_PACKET_FILTER_HMC_MASK;

         //Configure the multicast hash table
         EMAC->MAC_HASH_TABLE_REG0 = hashTable[0];
         EMAC->MAC_HASH_TABLE_REG1 = hashTable[1];

         //Debug message
         TRACE_DEBUG("  MAC_HASH_TABLE_REG0 = 0x%08" PRIX32 "\r\n", EMAC->MAC_HASH_TABLE_REG0);
         TRACE_DEBUG("  MAC_HASH_TABLE_REG1 = 0x%08" PRIX32 "\r\n", EMAC->MAC_HASH_TABLE_REG1);
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

error_t mcxe31bEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = EMAC->MAC_CONFIGURATION;

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
   EMAC->MAC_CONFIGURATION = config;

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

void mcxe31bEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = EMAC->MAC_MDIO_ADDRESS & EMAC_MAC_MDIO_ADDRESS_CR_MASK;
      //Set up a write operation
      temp |= EMAC_MAC_MDIO_ADDRESS_GOC_0_MASK | EMAC_MAC_MDIO_ADDRESS_GB_MASK;

      //PHY address
      temp |= EMAC_MAC_MDIO_ADDRESS_PA(phyAddr);
      //Register address
      temp |= EMAC_MAC_MDIO_ADDRESS_RDA(regAddr);

      //Data to be written in the PHY register
      EMAC->MAC_MDIO_DATA = data & EMAC_MAC_MDIO_DATA_GD_MASK;

      //Start a write operation
      EMAC->MAC_MDIO_ADDRESS = temp;
      //Wait for the write to complete
      while((EMAC->MAC_MDIO_ADDRESS & EMAC_MAC_MDIO_ADDRESS_GB_MASK) != 0)
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

uint16_t mcxe31bEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = EMAC->MAC_MDIO_ADDRESS & EMAC_MAC_MDIO_ADDRESS_CR_MASK;

      //Set up a read operation
      temp |= EMAC_MAC_MDIO_ADDRESS_GOC_1_MASK |
         EMAC_MAC_MDIO_ADDRESS_GOC_0_MASK | EMAC_MAC_MDIO_ADDRESS_GB_MASK;

      //PHY address
      temp |= EMAC_MAC_MDIO_ADDRESS_PA(phyAddr);
      //Register address
      temp |= EMAC_MAC_MDIO_ADDRESS_RDA(regAddr);

      //Start a read operation
      EMAC->MAC_MDIO_ADDRESS = temp;
      //Wait for the read to complete
      while((EMAC->MAC_MDIO_ADDRESS & EMAC_MAC_MDIO_ADDRESS_GB_MASK) != 0)
      {
      }

      //Get register value
      data = EMAC->MAC_MDIO_DATA & EMAC_MAC_MDIO_DATA_GD_MASK;
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

uint32_t mcxe31bEthCalcCrc(const void *data, size_t length)
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
