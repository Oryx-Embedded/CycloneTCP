/**
 * @file m2sxxx_eth_driver.c
 * @brief SmartFusion2 (M2Sxxx) Ethernet MAC controller
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
#include "m2sxxx.h"
#include "core/net.h"
#include "drivers/mac/m2sxxx_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
static uint8_t txBuffer[M2SXXX_ETH_TX_BUFFER_COUNT][M2SXXX_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
static uint8_t rxBuffer[M2SXXX_ETH_RX_BUFFER_COUNT][M2SXXX_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
static M2sxxxTxDmaDesc txDmaDesc[M2SXXX_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
static M2sxxxRxDmaDesc rxDmaDesc[M2SXXX_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[M2SXXX_ETH_TX_BUFFER_COUNT][M2SXXX_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Receive buffer
static uint8_t rxBuffer[M2SXXX_ETH_RX_BUFFER_COUNT][M2SXXX_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Transmit DMA descriptors
static M2sxxxTxDmaDesc txDmaDesc[M2SXXX_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//Receive DMA descriptors
static M2sxxxRxDmaDesc rxDmaDesc[M2SXXX_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//Pointer to the current TX DMA descriptor
static M2sxxxTxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static M2sxxxRxDmaDesc *rxCurDmaDesc;


/**
 * @brief M2Sxxx Ethernet MAC driver
 **/

const NicDriver m2sxxxEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   m2sxxxEthInit,
   m2sxxxEthTick,
   m2sxxxEthEnableIrq,
   m2sxxxEthDisableIrq,
   m2sxxxEthEventHandler,
   m2sxxxEthSendPacket,
   m2sxxxEthSetMulticastFilter,
   m2sxxxEthUpdateMacConfig,
   m2sxxxEthWritePhyReg,
   m2sxxxEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief M2Sxxx Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t m2sxxxEthInit(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing M2Sxxx Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Disable EDAC feature
   SYSREG->EDAC_CR &= ~(EDAC_CR_MAC_EDAC_RX_EN | EDAC_CR_MAC_EDAC_TX_EN);

   //Reset the MAC module
   MAC->CFG1 = CFG1_SOFT_RESET | CFG1_RESET_RX_MAC_CTRL |
      CFG1_RESET_TX_MAC_CTRL | CFG1_RESET_RX_FUNCTION | CFG1_RESET_TX_FUNCTION;

   //Reset the interface module
   MAC->INTERFACE_CTRL = INTERFACE_CTRL_RESET;

   //Reset FIFOs
   MAC->FIFO_CFG0 = FIFO_CFG0_HSTRSTFT | FIFO_CFG0_HSTRSTST |
      FIFO_CFG0_HSTRSTFR | FIFO_CFG0_HSTRSTSR | FIFO_CFG0_HSTRSTWT;

   //Take the MAC module out of reset
   MAC->CFG1 = 0;
   //Take the interface module out of reset
   MAC->INTERFACE_CTRL = 0;
   //Take the FIFOs out of reset
   MAC->FIFO_CFG0 = 0;

   //Select interface mode (MII, RMII, GMII or TBI)
   m2sxxxEthInitGpio(interface);

   //Select the proper divider for the MDC clock
   MAC->MII_CONFIG = MII_CONFIG_CLKSEL_DIV28;

   //PHY transceiver initialization
   error = interface->phyDriver->init(interface);
   //Failed to initialize PHY transceiver?
   if(error)
      return error;

   //Set the upper 16 bits of the MAC address
   MAC->STATION_ADDRESS2 = (interface->macAddr.b[0] << 16) |
      (interface->macAddr.b[1] << 24);

   //Set the lower 32 bits of the MAC address
   MAC->STATION_ADDRESS1 = interface->macAddr.b[2] |
      (interface->macAddr.b[3] << 8) |
      (interface->macAddr.b[4] << 16) |
      (interface->macAddr.b[5] << 24);

   //Maximum frame length to be accepted
   MAC->MAX_FRAME_LENGTH = 1518;

   //Disable flow control
   MAC->CFG1 = 0;

   //All short frames will be zero-padded to 60 bytes and a valid CRC is then appended
   MAC->CFG2 = CFG2_PREAMBLE_7 | CFG2_INTERFACE_MODE_NIBBLE |
      CFG2_LENGTH_FIELD_CHECK | CFG2_PAD_CRC_EN | CFG2_CRC_EN;

   //Enable TX and RX FIFOs
   MAC->FIFO_CFG0 = FIFO_CFG0_FTFENREQ | FIFO_CFG0_STFENREQ |
      FIFO_CFG0_FRFENREQ | FIFO_CFG0_SRFENREQ | FIFO_CFG0_WTMENREQ;

   //Use default FIFO configuration
   MAC->FIFO_CFG1 = FIFO_CFG1_DEFAULT_VALUE;
   MAC->FIFO_CFG2 = FIFO_CFG2_DEFAULT_VALUE;
   MAC->FIFO_CFG3 = FIFO_CFG3_DEFAULT_VALUE;

   //Drop frames less than 64 bytes
   MAC->FIFO_CFG5 = FIFO_CFG5_HSTDRPLT64 | FIFO_CFG5_HSTFLTRFRMDC;

   //Specify the statistics vectors that will be checked
   MAC->FIFO_CFG5 &= ~(FIFO_CFG5_TRUNCATED | FIFO_CFG5_RECEPTION_OK |
      FIFO_CFG5_INVALID_CRC | FIFO_CFG5_RECEIVE_ERROR);

   //Configure frame filtering
   MAC->FIFO_CFG4 = FIFO_CFG4_TRUNCATED |
      FIFO_CFG4_INVALID_CRC | FIFO_CFG4_RECEIVE_ERROR;

   //Initialize DMA descriptor lists
   m2sxxxEthInitDmaDesc(interface);

   //Enable the desired Ethernet interrupts
   MAC->DMA_IRQ_MASK = DMA_IRQ_MASK_RX_PKT_RECEIVED | DMA_IRQ_MASK_TX_PKT_SENT;

   //Set priority grouping (4 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(M2SXXX_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(EthernetMAC_IRQn, NVIC_EncodePriority(M2SXXX_ETH_IRQ_PRIORITY_GROUPING,
      M2SXXX_ETH_IRQ_GROUP_PRIORITY, M2SXXX_ETH_IRQ_SUB_PRIORITY));

   //Enable transmission and reception
   MAC->CFG1 |= CFG1_TX_EN | CFG1_RX_EN;
   //Enable the DMA transfer of received packets
   MAC->DMA_RX_CTRL = DMA_RX_CTRL_RX_EN;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


//SF2-STARTER-KIT-ES-2 evaluation board?
#if defined(USE_SF2_STARTER_KIT_ES_2)

/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

void m2sxxxEthInitGpio(NetInterface *interface)
{
   //Select MII interface mode
   SYSREG->MAC_CR = MAC_CR_ETH_PHY_MODE_MII;
}

#endif


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void m2sxxxEthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < M2SXXX_ETH_TX_BUFFER_COUNT; i++)
   {
      //Transmit buffer address
      txDmaDesc[i].addr = (uint32_t) txBuffer[i];
      //The descriptor is initially owned by the user
      txDmaDesc[i].size = DMA_DESC_EMPTY_FLAG;
      //Next descriptor address
      txDmaDesc[i].next = (uint32_t) &txDmaDesc[i + 1];
   }

   //The last descriptor is chained to the first entry
   txDmaDesc[i - 1].next = (uint32_t) &txDmaDesc[0];
   //Point to the very first descriptor
   txCurDmaDesc = &txDmaDesc[0];

   //Initialize RX DMA descriptor list
   for(i = 0; i < M2SXXX_ETH_RX_BUFFER_COUNT; i++)
   {
      //Receive buffer address
      rxDmaDesc[i].addr = (uint32_t) rxBuffer[i];
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].size = DMA_DESC_EMPTY_FLAG;
      //Next descriptor address
      rxDmaDesc[i].next = (uint32_t) &rxDmaDesc[i + 1];
   }

   //The last descriptor is chained to the first entry
   rxDmaDesc[i - 1].next = (uint32_t) &rxDmaDesc[0];
   //Point to the very first descriptor
   rxCurDmaDesc = &rxDmaDesc[0];

   //Start location of the TX descriptor list
   MAC->DMA_TX_DESC = (uint32_t) txDmaDesc;
   //Start location of the RX descriptor list
   MAC->DMA_RX_DESC = (uint32_t) rxDmaDesc;
}


/**
 * @brief M2Sxxx Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void m2sxxxEthTick(NetInterface *interface)
{
   //Handle periodic operations
   interface->phyDriver->tick(interface);
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void m2sxxxEthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(EthernetMAC_IRQn);
   //Enable Ethernet PHY interrupts
   interface->phyDriver->enableIrq(interface);
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void m2sxxxEthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(EthernetMAC_IRQn);
   //Disable Ethernet PHY interrupts
   interface->phyDriver->disableIrq(interface);
}


/**
 * @brief M2Sxxx Ethernet MAC interrupt service routine
 **/

void EthernetMAC_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Enter interrupt service routine
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read interrupt status register
   status = MAC->DMA_IRQ;

   //A packet has been transmitted?
   if(status & DMA_IRQ_TX_PKT_SENT)
   {
      //Clear TX interrupt flag
      MAC->DMA_TX_STATUS = DMA_TX_STATUS_TX_PKT_SENT;

      //Check whether the TX buffer is available for writing
      if(txCurDmaDesc->size & DMA_DESC_EMPTY_FLAG)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //A packet has been received?
   if(status & DMA_IRQ_RX_PKT_RECEIVED)
   {
      //Disable RX interrupt
      MAC->DMA_IRQ_MASK &= ~DMA_IRQ_MASK_RX_PKT_RECEIVED;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}


/**
 * @brief M2Sxxx Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void m2sxxxEthEventHandler(NetInterface *interface)
{
   //Packet received?
   if(MAC->DMA_RX_STATUS & DMA_RX_STATUS_RX_PKT_RECEIVED)
   {
      //Process all the pending packets
      while(MAC->DMA_RX_STATUS & DMA_RX_STATUS_RX_PKT_RECEIVED)
      {
         //Clear RX interrupt flag
         MAC->DMA_RX_STATUS = DMA_RX_STATUS_RX_PKT_RECEIVED;
         //Read incoming packet
         m2sxxxEthReceivePacket(interface);
      }
   }

   //Re-enable Ethernet interrupts
   MAC->DMA_IRQ_MASK = DMA_IRQ_MASK_RX_PKT_RECEIVED | DMA_IRQ_MASK_TX_PKT_SENT;
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t m2sxxxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > M2SXXX_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if(!(txCurDmaDesc->size & DMA_DESC_EMPTY_FLAG))
      return ERROR_FAILURE;

   //Copy user data to the transmit buffer
   netBufferRead((uint8_t *) txCurDmaDesc->addr, buffer, offset, length);

   //Set the packet length and give the ownership of the descriptor to the DMA
   txCurDmaDesc->size = length & DMA_DESC_SIZE_MASK;

   //Check whether DMA transfers are suspended
   if(!(MAC->DMA_TX_CTRL & DMA_TX_CTRL_TX_EN))
   {
      //Set the start position in the ring buffer
      MAC->DMA_TX_DESC = (uint32_t) txCurDmaDesc;
   }

   //Instruct the DMA controller to transfer the packet
   MAC->DMA_TX_CTRL = DMA_TX_CTRL_TX_EN;

   //Point to the next descriptor in the list
   txCurDmaDesc = (M2sxxxTxDmaDesc *) txCurDmaDesc->next;

   //Check whether the next buffer is available for writing
   if(txCurDmaDesc->size & DMA_DESC_EMPTY_FLAG)
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

error_t m2sxxxEthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;

   //The current buffer is available for reading?
   if(!(rxCurDmaDesc->size & DMA_DESC_EMPTY_FLAG))
   {
      //Retrieve the length of the frame
      n = rxCurDmaDesc->size & DMA_DESC_SIZE_MASK;
      //Limit the number of data to read
      n = MIN(n, M2SXXX_ETH_RX_BUFFER_SIZE);

      //Pass the packet to the upper layer
      nicProcessPacket(interface, (uint8_t *) rxCurDmaDesc->addr, n);

      //Give the ownership of the descriptor back to the DMA
      rxCurDmaDesc->size = DMA_DESC_EMPTY_FLAG;

      //Check whether DMA transfers are suspended
      if(!(MAC->DMA_RX_CTRL & DMA_RX_CTRL_RX_EN))
      {
         //Set the start position in the ring buffer
         MAC->DMA_RX_DESC = (uint32_t) rxCurDmaDesc;
      }

      //Enable the DMA transfer of received packets
      MAC->DMA_RX_CTRL = DMA_RX_CTRL_RX_EN;
      //Point to the next descriptor in the list
      rxCurDmaDesc = (M2sxxxRxDmaDesc *) rxCurDmaDesc->next;

      //Valid packet received
      error = NO_ERROR;
   }
   else
   {
      //Check whether DMA transfers are suspended
      if(!(MAC->DMA_RX_CTRL & DMA_RX_CTRL_RX_EN))
      {
         //Set the start position in the ring buffer
         MAC->DMA_RX_DESC = (uint32_t) rxCurDmaDesc;
      }

      //Enable the DMA transfer of received packets
      MAC->DMA_RX_CTRL = DMA_RX_CTRL_RX_EN;

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

error_t m2sxxxEthSetMulticastFilter(NetInterface *interface)
{
   //SmartFusion2 Ethernet MAC does not implement any hash table
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t m2sxxxEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t temp;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      //The link operates at 100 Mbps
      temp = SYSREG->MAC_CR & ~MAC_CR_ETH_LINE_SPEED;
      SYSREG->MAC_CR = temp | MAC_CR_ETH_LINE_SPEED_100MBPS;

      //Configure the RMII module with the current operating speed
      MAC->INTERFACE_CTRL |= INTERFACE_CTRL_SPEED;

      //Use nibble mode
      temp = MAC->CFG2 & ~CFG2_INTERFACE_MODE;
      MAC->CFG2 = temp | CFG2_INTERFACE_MODE_NIBBLE;
   }
   else
   {
      //The link operates at 10 Mbps
      if((SYSREG->MAC_CR & MAC_CR_ETH_PHY_MODE) == MAC_CR_ETH_PHY_MODE_MII)
      {
         temp = SYSREG->MAC_CR & ~MAC_CR_ETH_LINE_SPEED;
         SYSREG->MAC_CR = temp | MAC_CR_ETH_LINE_SPEED_100MBPS;
      }
      else
      {
         temp = SYSREG->MAC_CR & ~MAC_CR_ETH_LINE_SPEED;
         SYSREG->MAC_CR = temp | MAC_CR_ETH_LINE_SPEED_10MBPS;
      }

      //Configure the RMII module with the current operating speed
      MAC->INTERFACE_CTRL &= ~INTERFACE_CTRL_SPEED;

      //Use nibble mode
      temp = MAC->CFG2 & ~CFG2_INTERFACE_MODE;
      MAC->CFG2 = temp | CFG2_INTERFACE_MODE_NIBBLE;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      //Configure MAC to operate in full-duplex mode
      MAC->CFG2 |= CFG2_FULL_DUPLEX;
      MAC->FIFO_CFG5 &= ~FIFO_CFG5_CFGHDPLX;
   }
   else
   {
      //Configure MAC to operate in half-duplex mode
      MAC->CFG2 &= ~CFG2_FULL_DUPLEX;
      MAC->FIFO_CFG5 |= FIFO_CFG5_CFGHDPLX;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @param[in] data Register value
 **/

void m2sxxxEthWritePhyReg(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
   //Set PHY address and register address
   MAC->MII_ADDRESS = (phyAddr << MII_ADDRESS_PHY_ADDR_POS) | regAddr;
   //Start a write operation
   MAC->MII_CTRL = data;

   //Wait for the write to complete
   while(MAC->MII_INDICATORS & MII_INDICATORS_BUSY);
}


/**
 * @brief Read PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t m2sxxxEthReadPhyReg(uint8_t phyAddr, uint8_t regAddr)
{
   //Set PHY address and register address
   MAC->MII_ADDRESS = (phyAddr << MII_ADDRESS_PHY_ADDR_POS) | regAddr;
   //Start a read operation
   MAC->MII_COMMAND = MII_COMMAND_READ;

   //Wait for the read to complete
   while(MAC->MII_INDICATORS & MII_INDICATORS_BUSY);

   //Clear command register
   MAC->MII_COMMAND = 0;
   //Return PHY register contents
   return MAC->MII_STATUS;
}
