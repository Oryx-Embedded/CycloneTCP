/**
 * @file mk7x_eth_driver.c
 * @brief Freescale Kinetis K70 Ethernet MAC controller
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

//MK70F12 device?
#if defined(MK70F12)
   #include "mk70f12.h"
//MK70F15 device?
#elif defined(MK70F15)
   #include "mk70f15.h"
#endif

//Dependencies
#include "core/net.h"
#include "drivers/mac/mk7x_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//TX buffer
#pragma data_alignment = 16
static uint8_t txBuffer[MK7X_ETH_TX_BUFFER_COUNT][MK7X_ETH_TX_BUFFER_SIZE];
//RX buffer
#pragma data_alignment = 16
static uint8_t rxBuffer[MK7X_ETH_RX_BUFFER_COUNT][MK7X_ETH_RX_BUFFER_SIZE];
//TX buffer descriptors
#pragma data_alignment = 16
static uint32_t txBufferDesc[MK7X_ETH_TX_BUFFER_COUNT][8];
//RX buffer descriptors
#pragma data_alignment = 16
static uint32_t rxBufferDesc[MK7X_ETH_RX_BUFFER_COUNT][8];

//ARM or GCC compiler?
#else

//TX buffer
static uint8_t txBuffer[MK7X_ETH_TX_BUFFER_COUNT][MK7X_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(16)));
//RX buffer
static uint8_t rxBuffer[MK7X_ETH_RX_BUFFER_COUNT][MK7X_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(16)));
//TX buffer descriptors
static uint32_t txBufferDesc[MK7X_ETH_TX_BUFFER_COUNT][8]
   __attribute__((aligned(16)));
//RX buffer descriptors
static uint32_t rxBufferDesc[MK7X_ETH_RX_BUFFER_COUNT][8]
   __attribute__((aligned(16)));

#endif

//TX buffer index
static uint_t txBufferIndex;
//RX buffer index
static uint_t rxBufferIndex;


/**
 * @brief Kinetis K7x Ethernet MAC driver
 **/

const NicDriver mk7xEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   mk7xEthInit,
   mk7xEthTick,
   mk7xEthEnableIrq,
   mk7xEthDisableIrq,
   mk7xEthEventHandler,
   mk7xEthSendPacket,
   mk7xEthSetMulticastFilter,
   mk7xEthUpdateMacConfig,
   mk7xEthWritePhyReg,
   mk7xEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief Kinetis K7x Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mk7xEthInit(NetInterface *interface)
{
   error_t error;
   uint32_t value;

   //Debug message
   TRACE_INFO("Initializing Kinetis K7x Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Disable MPU
   MPU->CESR &= ~MPU_CESR_VLD_MASK;

   //Enable external reference clock
   OSC0->CR |= OSC_CR_ERCLKEN_MASK;
   //Enable ENET peripheral clock
   SIM->SCGC2 |= SIM_SCGC2_ENET_MASK;

   //GPIO configuration
   mk7xEthInitGpio(interface);

   //Reset ENET module
   ENET->ECR = ENET_ECR_RESET_MASK;
   //Wait for the reset to complete
   while(ENET->ECR & ENET_ECR_RESET_MASK);

   //Reveive control register
   ENET->RCR = ENET_RCR_MAX_FL(1518) | ENET_RCR_RMII_MODE_MASK |
      ENET_RCR_MII_MODE_MASK;

   //Transmit control register
   ENET->TCR = 0;
   //Configure MDC clock frequency
   ENET->MSCR = ENET_MSCR_MII_SPEED(59);

   //PHY transceiver initialization
   error = interface->phyDriver->init(interface);
   //Failed to initialize PHY transceiver?
   if(error)
      return error;

   //Set the MAC address (upper 16 bits)
   value = interface->macAddr.b[5];
   value |= (interface->macAddr.b[4] << 8);
   ENET->PAUR = ENET_PAUR_PADDR2(value) | ENET_PAUR_TYPE(0x8808);

   //Set the MAC address (lower 32 bits)
   value = interface->macAddr.b[3];
   value |= (interface->macAddr.b[2] << 8);
   value |= (interface->macAddr.b[1] << 16);
   value |= (interface->macAddr.b[0] << 24);
   ENET->PALR = ENET_PALR_PADDR1(value);

   //Hash table for unicast address filtering
   ENET->IALR = 0;
   ENET->IAUR = 0;
   //Hash table for multicast address filtering
   ENET->GALR = 0;
   ENET->GAUR = 0;

   //Disable transmit accelerator functions
   ENET->TACC = 0;
   //Disable receive accelerator functions
   ENET->RACC = 0;

   //Use enhanced buffer descriptors
   ENET->ECR = ENET_ECR_DBSWP_MASK | ENET_ECR_EN1588_MASK;
   //Clear MIC counters
   ENET->MIBC = ENET_MIBC_MIB_CLEAR_MASK;

   //Initialize buffer descriptors
   mk7xEthInitBufferDesc(interface);

   //Clear any pending interrupts
   ENET->EIR = 0xFFFFFFFF;
   //Enable desired interrupts
   ENET->EIMR = ENET_EIMR_TXF_MASK | ENET_EIMR_RXF_MASK | ENET_EIMR_EBERR_MASK;

   //Set priority grouping (4 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(MK7X_ETH_IRQ_PRIORITY_GROUPING);

   //Configure ENET transmit interrupt priority
   NVIC_SetPriority(ENET_Transmit_IRQn, NVIC_EncodePriority(MK7X_ETH_IRQ_PRIORITY_GROUPING,
      MK7X_ETH_IRQ_GROUP_PRIORITY, MK7X_ETH_IRQ_SUB_PRIORITY));

   //Configure ENET receive interrupt priority
   NVIC_SetPriority(ENET_Receive_IRQn, NVIC_EncodePriority(MK7X_ETH_IRQ_PRIORITY_GROUPING,
      MK7X_ETH_IRQ_GROUP_PRIORITY, MK7X_ETH_IRQ_SUB_PRIORITY));

   //Configure ENET error interrupt priority
   NVIC_SetPriority(ENET_Error_IRQn, NVIC_EncodePriority(MK7X_ETH_IRQ_PRIORITY_GROUPING,
      MK7X_ETH_IRQ_GROUP_PRIORITY, MK7X_ETH_IRQ_SUB_PRIORITY));

   //Enable Ethernet MAC
   ENET->ECR |= ENET_ECR_ETHEREN_MASK;
   //Instruct the DMA to poll the receive descriptor list
   ENET->RDAR = ENET_RDAR_RDAR_MASK;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


//TWR-K70F120M evaluation board?
#if defined(USE_TWR_K70F120M)

/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

void mk7xEthInitGpio(NetInterface *interface)
{
   //Enable PORTA and PORTB peripheral clocks
   SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK;

   //Configure RMII0_RXER (PTA5)
   PORTA->PCR[5] = PORT_PCR_MUX(4) | PORT_PCR_PE_MASK;
   //Configure RMII0_RXD1 (PTA12)
   PORTA->PCR[12] = PORT_PCR_MUX(4);
   //Configure RMII0_RXD0 (PTA13)
   PORTA->PCR[13] = PORT_PCR_MUX(4);
   //Configure RMII0_CRS_DV (PTA14)
   PORTA->PCR[14] = PORT_PCR_MUX(4);
   //Configure RMII0_TXEN (PTA15)
   PORTA->PCR[15] = PORT_PCR_MUX(4);
   //Configure RMII0_TXD0 (PTA16)
   PORTA->PCR[16] = PORT_PCR_MUX(4);
   //Configure RMII0_TXD1 (PTA17)
   PORTA->PCR[17] = PORT_PCR_MUX(4);

   //Configure RMII0_MDIO (PTB0)
   PORTB->PCR[0] = PORT_PCR_MUX(4) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
   //Configure RMII0_MDC (PTB1)
   PORTB->PCR[1] = PORT_PCR_MUX(4);
}

#endif


/**
 * @brief Initialize buffer descriptors
 * @param[in] interface Underlying network interface
 **/

void mk7xEthInitBufferDesc(NetInterface *interface)
{
   uint_t i;
   uint32_t address;

   //Clear TX and RX buffer descriptors
   memset(txBufferDesc, 0, sizeof(txBufferDesc));
   memset(rxBufferDesc, 0, sizeof(rxBufferDesc));

   //Initialize TX buffer descriptors
   for(i = 0; i < MK7X_ETH_TX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current TX buffer
      address = (uint32_t) txBuffer[i];
      //Transmit buffer address
      txBufferDesc[i][1] = address;
      //Generate interrupts
      txBufferDesc[i][2] = ENET_TBD2_INT;
   }

   //Mark the last descriptor entry with the wrap flag
   txBufferDesc[i - 1][0] |= ENET_TBD0_W;
   //Initialize TX buffer index
   txBufferIndex = 0;

   //Initialize RX buffer descriptors
   for(i = 0; i < MK7X_ETH_RX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current RX buffer
      address = (uint32_t) rxBuffer[i];
      //The descriptor is initially owned by the DMA
      rxBufferDesc[i][0] = ENET_RBD0_E;
      //Receive buffer address
      rxBufferDesc[i][1] = address;
      //Generate interrupts
      rxBufferDesc[i][2] = ENET_RBD2_INT;
   }

   //Mark the last descriptor entry with the wrap flag
   rxBufferDesc[i - 1][0] |= ENET_RBD0_W;
   //Initialize RX buffer index
   rxBufferIndex = 0;

   //Start location of the TX descriptor list
   ENET->TDSR = (uint32_t) txBufferDesc;
   //Start location of the RX descriptor list
   ENET->RDSR = (uint32_t) rxBufferDesc;
   //Maximum receive buffer size
   ENET->MRBR = MK7X_ETH_RX_BUFFER_SIZE;
}


/**
 * @brief Kinetis K7x Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void mk7xEthTick(NetInterface *interface)
{
   //Handle periodic operations
   interface->phyDriver->tick(interface);
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void mk7xEthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(ENET_Transmit_IRQn);
   NVIC_EnableIRQ(ENET_Receive_IRQn);
   NVIC_EnableIRQ(ENET_Error_IRQn);
   //Enable Ethernet PHY interrupts
   interface->phyDriver->enableIrq(interface);
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void mk7xEthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(ENET_Transmit_IRQn);
   NVIC_DisableIRQ(ENET_Receive_IRQn);
   NVIC_DisableIRQ(ENET_Error_IRQn);
   //Disable Ethernet PHY interrupts
   interface->phyDriver->disableIrq(interface);
}


/**
 * @brief Ethernet MAC transmit interrupt
 **/

void ENET_Transmit_IRQHandler(void)
{
   bool_t flag;

   //Enter interrupt service routine
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //A packet has been transmitted?
   if(ENET->EIR & ENET_EIR_TXF_MASK)
   {
      //Clear TXF interrupt flag
      ENET->EIR = ENET_EIR_TXF_MASK;

      //Check whether the TX buffer is available for writing
      if(!(txBufferDesc[txBufferIndex][0] & ENET_TBD0_R))
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag = osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }

      //Instruct the DMA to poll the transmit descriptor list
      ENET->TDAR = ENET_TDAR_TDAR_MASK;
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}


/**
 * @brief Ethernet MAC receive interrupt
 **/

void ENET_Receive_IRQHandler(void)
{
   bool_t flag;

   //Enter interrupt service routine
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //A packet has been received?
   if(ENET->EIR & ENET_EIR_RXF_MASK)
   {
      //Disable RXF interrupt
      ENET->EIMR &= ~ENET_EIMR_RXF_MASK;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag = osSetEventFromIsr(&netEvent);
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}


/**
 * @brief Ethernet MAC error interrupt
 **/

void ENET_Error_IRQHandler(void)
{
   bool_t flag;

   //Enter interrupt service routine
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //System bus error?
   if(ENET->EIR & ENET_EIR_EBERR_MASK)
   {
      //Disable EBERR interrupt
      ENET->EIMR &= ~ENET_EIMR_EBERR_MASK;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}


/**
 * @brief Kinetis K7x Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void mk7xEthEventHandler(NetInterface *interface)
{
   error_t error;
   uint32_t status;

   //Read interrupt event register
   status = ENET->EIR;

   //Packet received?
   if(status & ENET_EIR_RXF_MASK)
   {
      //Clear RXF interrupt flag
      ENET->EIR = ENET_EIR_RXF_MASK;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = mk7xEthReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }

   //System bus error?
   if(status & ENET_EIR_EBERR_MASK)
   {
      //Clear EBERR interrupt flag
      ENET->EIR = ENET_EIR_EBERR_MASK;

      //Disable Ethernet MAC
      ENET->ECR &= ~ENET_ECR_ETHEREN_MASK;
      //Reset buffer descriptors
      mk7xEthInitBufferDesc(interface);
      //Resume normal operation
      ENET->ECR |= ENET_ECR_ETHEREN_MASK;
      //Instruct the DMA to poll the receive descriptor list
      ENET->RDAR = ENET_RDAR_RDAR_MASK;
   }

   //Re-enable Ethernet MAC interrupts
   ENET->EIMR = ENET_EIMR_TXF_MASK | ENET_EIMR_RXF_MASK | ENET_EIMR_EBERR_MASK;
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t mk7xEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > MK7X_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if(txBufferDesc[txBufferIndex][0] & ENET_TBD0_R)
      return ERROR_FAILURE;

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txBufferIndex], buffer, offset, length);

   //Clear BDU flag
   txBufferDesc[txBufferIndex][4] = 0;

   //Check current index
   if(txBufferIndex < (MK7X_ETH_TX_BUFFER_COUNT - 1))
   {
      //Give the ownership of the descriptor to the DMA engine
      txBufferDesc[txBufferIndex][0] = ENET_TBD0_R | ENET_TBD0_L |
         ENET_TBD0_TC | (length & ENET_TBD0_DATA_LENGTH);

      //Point to the next buffer
      txBufferIndex++;
   }
   else
   {
      //Give the ownership of the descriptor to the DMA engine
      txBufferDesc[txBufferIndex][0] = ENET_TBD0_R | ENET_TBD0_W |
         ENET_TBD0_L | ENET_TBD0_TC | (length & ENET_TBD0_DATA_LENGTH);

      //Wrap around
      txBufferIndex = 0;
   }

   //Instruct the DMA to poll the transmit descriptor list
   ENET->TDAR = ENET_TDAR_TDAR_MASK;

   //Check whether the next buffer is available for writing
   if(!(txBufferDesc[txBufferIndex][0] & ENET_TBD0_R))
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

error_t mk7xEthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;

   //Make sure the current buffer is available for reading
   if(!(rxBufferDesc[rxBufferIndex][0] & ENET_RBD0_E))
   {
      //The frame should not span multiple buffers
      if(rxBufferDesc[rxBufferIndex][0] & ENET_RBD0_L)
      {
         //Check whether an error occurred
         if(!(rxBufferDesc[rxBufferIndex][0] & (ENET_RBD0_LG |
            ENET_RBD0_NO | ENET_RBD0_CR | ENET_RBD0_OV | ENET_RBD0_TR)))
         {
            //Retrieve the length of the frame
            n = rxBufferDesc[rxBufferIndex][0] & ENET_RBD0_DATA_LENGTH;
            //Limit the number of data to read
            n = MIN(n, MK7X_ETH_RX_BUFFER_SIZE);

            //Pass the packet to the upper layer
            nicProcessPacket(interface, rxBuffer[rxBufferIndex], n);

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

      //Clear BDU flag
      rxBufferDesc[rxBufferIndex][4] = 0;

      //Check current index
      if(rxBufferIndex < (MK7X_ETH_RX_BUFFER_COUNT - 1))
      {
         //Give the ownership of the descriptor back to the DMA engine
         rxBufferDesc[rxBufferIndex][0] = ENET_RBD0_E;
         //Point to the next buffer
         rxBufferIndex++;
      }
      else
      {
         //Give the ownership of the descriptor back to the DMA engine
         rxBufferDesc[rxBufferIndex][0] = ENET_RBD0_E | ENET_RBD0_W;
         //Wrap around
         rxBufferIndex = 0;
      }

      //Instruct the DMA to poll the receive descriptor list
      ENET->RDAR = ENET_RDAR_RDAR_MASK;
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

error_t mk7xEthSetMulticastFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint32_t hashTable[2];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating Kinetis K7x hash table...\r\n");

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
         //Compute CRC over the current MAC address
         crc = mk7xEthCalcCrc(&entry->addr, sizeof(MacAddr));

         //The upper 6 bits in the CRC register are used to index the
         //contents of the hash table
         k = (crc >> 26) & 0x3F;

         //Update hash table contents
         hashTable[k / 32] |= (1 << (k % 32));
      }
   }

   //Write the hash table
   ENET->GALR = hashTable[0];
   ENET->GAUR = hashTable[1];

   //Debug message
   TRACE_DEBUG("  GALR = %08" PRIX32 "\r\n", ENET->GALR);
   TRACE_DEBUG("  GAUR = %08" PRIX32 "\r\n", ENET->GAUR);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mk7xEthUpdateMacConfig(NetInterface *interface)
{
   //Disable Ethernet MAC while modifying configuration registers
   ENET->ECR &= ~ENET_ECR_ETHEREN_MASK;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      //100 Mbps operation
      ENET->RCR &= ~ENET_RCR_RMII_10T_MASK;
   }
   else
   {
      //10 Mbps operation
      ENET->RCR |= ENET_RCR_RMII_10T_MASK;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      //Full-duplex mode
      ENET->TCR |= ENET_TCR_FDEN_MASK;
      //Receive path operates independently of transmit
      ENET->RCR &= ~ENET_RCR_DRT_MASK;
   }
   else
   {
      //Half-duplex mode
      ENET->TCR &= ~ENET_TCR_FDEN_MASK;
      //Disable reception of frames while transmitting
      ENET->RCR |= ENET_RCR_DRT_MASK;
   }

   //Reset buffer descriptors
   mk7xEthInitBufferDesc(interface);

   //Re-enable Ethernet MAC
   ENET->ECR |= ENET_ECR_ETHEREN_MASK;
   //Instruct the DMA to poll the receive descriptor list
   ENET->RDAR = ENET_RDAR_RDAR_MASK;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @param[in] data Register value
 **/

void mk7xEthWritePhyReg(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
   uint32_t value;

   //Set up a write operation
   value = ENET_MMFR_ST(1) | ENET_MMFR_OP(1) | ENET_MMFR_TA(2);
   //PHY address
   value |= ENET_MMFR_PA(phyAddr);
   //Register address
   value |= ENET_MMFR_RA(regAddr);
   //Register value
   value |= ENET_MMFR_DATA(data);

   //Clear MII interrupt flag
   ENET->EIR = ENET_EIR_MII_MASK;
   //Start a write operation
   ENET->MMFR = value;
   //Wait for the write to complete
   while(!(ENET->EIR & ENET_EIR_MII_MASK));
}


/**
 * @brief Read PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t mk7xEthReadPhyReg(uint8_t phyAddr, uint8_t regAddr)
{
   uint32_t value;

   //Set up a read operation
   value = ENET_MMFR_ST(1) | ENET_MMFR_OP(2) | ENET_MMFR_TA(2);
   //PHY address
   value |= ENET_MMFR_PA(phyAddr);
   //Register address
   value |= ENET_MMFR_RA(regAddr);

   //Clear MII interrupt flag
   ENET->EIR = ENET_EIR_MII_MASK;
   //Start a read operation
   ENET->MMFR = value;
   //Wait for the read to complete
   while(!(ENET->EIR & ENET_EIR_MII_MASK));

   //Return PHY register contents
   return ENET->MMFR & ENET_MMFR_DATA_MASK;
}


/**
 * @brief CRC calculation
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t mk7xEthCalcCrc(const void *data, size_t length)
{
   uint_t i;
   uint_t j;

   //Point to the data over which to calculate the CRC
   const uint8_t *p = (uint8_t *) data;
   //CRC preset value
   uint32_t crc = 0xFFFFFFFF;

   //Loop through data
   for(i = 0; i < length; i++)
   {
      //Update CRC value
      crc ^= p[i];
      //The message is processed bit by bit
      for(j = 0; j < 8; j++)
      {
         if(crc & 0x00000001)
            crc = (crc >> 1) ^ 0xEDB88320;
         else
            crc = crc >> 1;
      }
   }

   //Return CRC value
   return crc;
}
