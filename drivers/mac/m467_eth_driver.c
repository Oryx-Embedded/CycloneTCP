/**
 * @file m467_eth_driver.c
 * @brief Nuvoton M467 Ethernet MAC driver
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
#include "m460.h"
#include "core/net.h"
#include "drivers/mac/m467_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
static uint8_t txBuffer[M467_ETH_TX_BUFFER_COUNT][M467_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
static uint8_t rxBuffer[M467_ETH_RX_BUFFER_COUNT][M467_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
static M467TxDmaDesc txDmaDesc[M467_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
static M467RxDmaDesc rxDmaDesc[M467_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[M467_ETH_TX_BUFFER_COUNT][M467_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Receive buffer
static uint8_t rxBuffer[M467_ETH_RX_BUFFER_COUNT][M467_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Transmit DMA descriptors
static M467TxDmaDesc txDmaDesc[M467_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//Receive DMA descriptors
static M467RxDmaDesc rxDmaDesc[M467_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//Pointer to the current TX DMA descriptor
static M467TxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static M467RxDmaDesc *rxCurDmaDesc;


/**
 * @brief M467 Ethernet MAC driver
 **/

const NicDriver m467EthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   m467EthInit,
   m467EthTick,
   m467EthEnableIrq,
   m467EthDisableIrq,
   m467EthEventHandler,
   m467EthSendPacket,
   m467EthUpdateMacAddrFilter,
   m467EthUpdateMacConfig,
   m467EthWritePhyReg,
   m467EthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief M467 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t m467EthInit(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing M467 Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   m467EthInitGpio(interface);

   //Reset EMAC module
   SYS_ResetModule(EMAC0_RST);
   //Enable EMAC clock
   CLK_EnableModuleClock(EMAC0_MODULE);

   //Perform a software reset
   EMAC_BUS_MODE |= EMAC_BUS_MODE_SWR;
   //Wait for the reset to complete
   while((EMAC_BUS_MODE & EMAC_BUS_MODE_SWR) != 0)
   {
   }

   //Adjust MDC clock range depending on CSR frequency
   EMAC_GMII_ADDR = EMAC_GMII_ADDR_CR_DIV_102;

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
   EMAC_MAC_CONFIG = EMAC_MAC_CONFIG_DO;

   //Configure MAC address filtering
   m467EthUpdateMacAddrFilter(interface);

   //Disable flow control
   EMAC_FLOW_CONTROL = 0;
   //Enable store and forward mode
   EMAC_OPERATION_MODE = EMAC_OPERATION_MODE_RSF | EMAC_OPERATION_MODE_TSF;

   //Configure DMA bus mode
   EMAC_BUS_MODE = EMAC_BUS_MODE_AAB | EMAC_BUS_MODE_USP |
      EMAC_BUS_MODE_RPBL_1 | EMAC_BUS_MODE_PBL_1 | EMAC_BUS_MODE_ATDS;

   //Initialize DMA descriptor lists
   m467EthInitDmaDesc(interface);

   //Disable MAC interrupts
   EMAC_INTERRUPT_MASK = EMAC_INTERRUPT_MASK_TSIM | EMAC_INTERRUPT_MASK_PMTIM;
   //Enable the desired DMA interrupts
   EMAC_INTERRUPT_ENABLE = EMAC_INTERRUPT_ENABLE_NIE |
      EMAC_INTERRUPT_ENABLE_RIE | EMAC_INTERRUPT_ENABLE_TIE;

   //Set priority grouping (4 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(M467_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(EMAC0_TXRX_IRQn, NVIC_EncodePriority(M467_ETH_IRQ_PRIORITY_GROUPING,
      M467_ETH_IRQ_GROUP_PRIORITY, M467_ETH_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   EMAC_MAC_CONFIG |= EMAC_MAC_CONFIG_TE | EMAC_MAC_CONFIG_RE;
   //Enable DMA transmission and reception
   EMAC_OPERATION_MODE |= EMAC_OPERATION_MODE_ST | EMAC_OPERATION_MODE_SR;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void m467EthInitGpio(NetInterface *interface)
{
//NuMaker-IoT-M467 evaluation board?
#if defined(USE_NUMAKER_IOT_M467)
   uint32_t temp;

   //Enable GPIO clocks
   CLK->AHBCLK0 |= CLK_AHBCLK0_GPACKEN_Msk;
   CLK->AHBCLK0 |= CLK_AHBCLK0_GPCCKEN_Msk;
   CLK->AHBCLK0 |= CLK_AHBCLK0_GPECKEN_Msk;

   //Configure EMAC_RMII_RXERR (PA.6)
   SET_EMAC0_RMII_RXERR_PA6();
   //Configure EMAC_RMII_CRSDV (PA.7)
   SET_EMAC0_RMII_CRSDV_PA7();
   //Configure EMAC_RMII_RXD1 (PC.6)
   SET_EMAC0_RMII_RXD1_PC6();
   //Configure EMAC_RMII_RXD0 (PC.7)
   SET_EMAC0_RMII_RXD0_PC7();
   //Configure EMAC_RMII_REFCLK (PC.8)
   SET_EMAC0_RMII_REFCLK_PC8();
   //Configure EMAC_RMII_MDC (PE.8)
   SET_EMAC0_RMII_MDC_PE8();
   //Configure EMAC_RMII_MDIO (PE.9)
   SET_EMAC0_RMII_MDIO_PE9();
   //Configure EMAC_RMII_TXD0 (PE.10)
   SET_EMAC0_RMII_TXD0_PE10();
   //Configure EMAC_RMII_TXD1 (PE.11)
   SET_EMAC0_RMII_TXD1_PE11();
   //Configure EMAC_RMII_TXEN (PE.12)
   SET_EMAC0_RMII_TXEN_PE12();

   //Enable high slew rate on RMII output pins
   temp = PE->SLEWCTL;
   temp = (temp & ~GPIO_SLEWCTL_HSREN10_Msk) | (GPIO_SLEWCTL_HIGH << GPIO_SLEWCTL_HSREN10_Pos);
   temp = (temp & ~GPIO_SLEWCTL_HSREN11_Msk) | (GPIO_SLEWCTL_HIGH << GPIO_SLEWCTL_HSREN11_Pos);
   temp = (temp & ~GPIO_SLEWCTL_HSREN12_Msk) | (GPIO_SLEWCTL_HIGH << GPIO_SLEWCTL_HSREN11_Pos);
   PE->SLEWCTL = temp;
#endif
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void m467EthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < M467_ETH_TX_BUFFER_COUNT; i++)
   {
      //Use chain structure rather than ring structure
      txDmaDesc[i].tdes0 = EMAC_TDES0_IC | EMAC_TDES0_TCH;
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
   for(i = 0; i < M467_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = EMAC_RDES0_OWN;
      //Use chain structure rather than ring structure
      rxDmaDesc[i].rdes1 = EMAC_RDES1_RCH | (M467_ETH_RX_BUFFER_SIZE & EMAC_RDES1_RBS1);
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
   EMAC_TRANSMIT_DESCRIPTOR_LIST_ADDR = (uint32_t) txDmaDesc;
   //Start location of the RX descriptor list
   EMAC_RECEIVE_DESCRIPTOR_LIST_ADDR = (uint32_t) rxDmaDesc;
}


/**
 * @brief M467 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void m467EthTick(NetInterface *interface)
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

void m467EthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(EMAC0_TXRX_IRQn);

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

void m467EthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(EMAC0_TXRX_IRQn);

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
 * @brief M467 Ethernet MAC interrupt service routine
 **/

void EMAC0_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = EMAC_STATUS;

   //Packet transmitted?
   if((status & EMAC_STATUS_TI) != 0)
   {
      //Clear TI interrupt flag
      EMAC_STATUS = EMAC_STATUS_TI;

      //Check whether the TX buffer is available for writing
      if((txCurDmaDesc->tdes0 & EMAC_TDES0_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & EMAC_STATUS_RI) != 0)
   {
      //Clear RI interrupt flag
      EMAC_STATUS = EMAC_STATUS_RI;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   EMAC_STATUS = EMAC_STATUS_NIS;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief M467 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void m467EthEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = m467EthReceivePacket(interface);

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

error_t m467EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > M467_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txCurDmaDesc->tdes0 & EMAC_TDES0_OWN) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead((uint8_t *) txCurDmaDesc->tdes2, buffer, offset, length);

   //Write the number of bytes to send
   txCurDmaDesc->tdes1 = length & EMAC_TDES1_TBS1;
   //Set LS and FS flags as the data fits in a single buffer
   txCurDmaDesc->tdes0 |= EMAC_TDES0_LS | EMAC_TDES0_FS;
   //Give the ownership of the descriptor to the DMA
   txCurDmaDesc->tdes0 |= EMAC_TDES0_OWN;

   //Clear TU flag to resume processing
   EMAC_STATUS = EMAC_STATUS_TU;
   //Instruct the DMA to poll the transmit descriptor list
   EMAC_TRANSMIT_POLL_DEMAND = 0;

   //Point to the next descriptor in the list
   txCurDmaDesc = (M467TxDmaDesc *) txCurDmaDesc->tdes3;

   //Check whether the next buffer is available for writing
   if((txCurDmaDesc->tdes0 & EMAC_TDES0_OWN) == 0)
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

error_t m467EthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxCurDmaDesc->rdes0 & EMAC_RDES0_OWN) == 0)
   {
      //FS and LS flags should be set
      if((rxCurDmaDesc->rdes0 & EMAC_RDES0_FS) != 0 &&
         (rxCurDmaDesc->rdes0 & EMAC_RDES0_LS) != 0)
      {
         //Make sure no error occurred
         if((rxCurDmaDesc->rdes0 & EMAC_RDES0_ES) == 0)
         {
            //Retrieve the length of the frame
            n = (rxCurDmaDesc->rdes0 & EMAC_RDES0_FL) >> 16;
            //Limit the number of data to read
            n = MIN(n, M467_ETH_RX_BUFFER_SIZE);

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
      rxCurDmaDesc->rdes0 = EMAC_RDES0_OWN;
      //Point to the next descriptor in the list
      rxCurDmaDesc = (M467RxDmaDesc *) rxCurDmaDesc->rdes3;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RU flag to resume processing
   EMAC_STATUS = EMAC_STATUS_RU;
   //Instruct the DMA to poll the receive descriptor list
   EMAC_RECEIVE_POLL_DEMAND = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t m467EthUpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   bool_t acceptMulticast;
   MacAddr unicastMacAddr[3];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Set the MAC address of the station
   EMAC_MAC_ADDR0_HIGH = interface->macAddr.w[2];
   EMAC_MAC_ADDR0_LOW = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);

   //The MAC supports 3 additional addresses for unicast perfect filtering
   unicastMacAddr[0] = MAC_UNSPECIFIED_ADDR;
   unicastMacAddr[1] = MAC_UNSPECIFIED_ADDR;
   unicastMacAddr[2] = MAC_UNSPECIFIED_ADDR;

   //This flag will be set if multicast addresses should be accepted
   acceptMulticast = FALSE;

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
            //Accept multicast addresses
            acceptMulticast = TRUE;
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
      EMAC_MAC_ADDR1_HIGH = unicastMacAddr[0].w[2] | EMAC_MAC_ADDR1_HIGH_AE;
      EMAC_MAC_ADDR1_LOW = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
   }
   else
   {
      EMAC_MAC_ADDR1_HIGH = 0;
      EMAC_MAC_ADDR1_LOW = 0;
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      EMAC_MAC_ADDR2_HIGH = unicastMacAddr[1].w[2] | EMAC_MAC_ADDR2_HIGH_AE;
      EMAC_MAC_ADDR2_LOW = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
   }
   else
   {
      EMAC_MAC_ADDR2_HIGH = 0;
      EMAC_MAC_ADDR2_LOW = 0;
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      EMAC_MAC_ADDR3_HIGH = unicastMacAddr[2].w[2] | EMAC_MAC_ADDR3_HIGH_AE;
      EMAC_MAC_ADDR3_LOW = unicastMacAddr[2].w[0] | (unicastMacAddr[0].w[2] << 16);
   }
   else
   {
      EMAC_MAC_ADDR3_HIGH = 0;
      EMAC_MAC_ADDR3_LOW = 0;
   }

   //Enable or disable the reception of multicast frames
   if(acceptMulticast)
   {
      EMAC_MAC_FRAME_FILTER |= EMAC_MAC_FRAME_FILTER_PM;
   }
   else
   {
      EMAC_MAC_FRAME_FILTER &= ~EMAC_MAC_FRAME_FILTER_PM;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t m467EthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = EMAC_MAC_CONFIG;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= EMAC_MAC_CONFIG_FES;
   }
   else
   {
      config &= ~EMAC_MAC_CONFIG_FES;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= EMAC_MAC_CONFIG_DM;
   }
   else
   {
      config &= ~EMAC_MAC_CONFIG_DM;
   }

   //Update MAC configuration register
   EMAC_MAC_CONFIG = config;

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

void m467EthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = EMAC_GMII_ADDR & EMAC_GMII_ADDR_CR;
      //Set up a write operation
      temp |= EMAC_GMII_ADDR_GW | EMAC_GMII_ADDR_GB;
      //PHY address
      temp |= (phyAddr << 11) & EMAC_GMII_ADDR_PA;
      //Register address
      temp |= (regAddr << 6) & EMAC_GMII_ADDR_GR;

      //Data to be written in the PHY register
      EMAC_GMII_DATA = data & EMAC_GMII_DATA_GD;

      //Start a write operation
      EMAC_GMII_ADDR = temp;
      //Wait for the write to complete
      while((EMAC_GMII_ADDR & EMAC_GMII_ADDR_GB) != 0)
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

uint16_t m467EthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = EMAC_GMII_ADDR & EMAC_GMII_ADDR_CR;
      //Set up a read operation
      temp |= EMAC_GMII_ADDR_GB;
      //PHY address
      temp |= (phyAddr << 11) & EMAC_GMII_ADDR_PA;
      //Register address
      temp |= (regAddr << 6) & EMAC_GMII_ADDR_GR;

      //Start a read operation
      EMAC_GMII_ADDR = temp;
      //Wait for the read to complete
      while((EMAC_GMII_ADDR & EMAC_GMII_ADDR_GB) != 0)
      {
      }

      //Get register value
      data = EMAC_GMII_DATA & EMAC_GMII_DATA_GD;
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
      data = 0;
   }

   //Return the value of the PHY register
   return data;
}
