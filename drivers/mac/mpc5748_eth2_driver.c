/**
 * @file mpc5748_eth2_driver.c
 * @brief NXP MPC5748 Ethernet MAC driver (ENET1 instance)
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
#include "device_registers.h"
#include "interrupt_manager.h"
#include "core/net.h"
#include "drivers/mac/mpc5748_eth2_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//TX buffer
static uint8_t txBuffer[MPC5748_ETH2_TX_BUFFER_COUNT][MPC5748_ETH2_TX_BUFFER_SIZE]
   __attribute__((aligned(64)));
//RX buffer
static uint8_t rxBuffer[MPC5748_ETH2_RX_BUFFER_COUNT][MPC5748_ETH2_RX_BUFFER_SIZE]
   __attribute__((aligned(64)));
//TX buffer descriptors
static uint32_t txBufferDesc[MPC5748_ETH2_TX_BUFFER_COUNT][8]
   __attribute__((aligned(64)));
//RX buffer descriptors
static uint32_t rxBufferDesc[MPC5748_ETH2_RX_BUFFER_COUNT][8]
   __attribute__((aligned(64)));

//TX buffer index
static uint_t txBufferIndex;
//RX buffer index
static uint_t rxBufferIndex;


/**
 * @brief MPC5748 Ethernet MAC driver (ENET1 instance)
 **/

const NicDriver mpc5748Eth2Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   mpc5748Eth2Init,
   mpc5748Eth2Tick,
   mpc5748Eth2EnableIrq,
   mpc5748Eth2DisableIrq,
   mpc5748Eth2EventHandler,
   mpc5748Eth2SendPacket,
   mpc5748Eth2UpdateMacAddrFilter,
   mpc5748Eth2UpdateMacConfig,
   mpc5748Eth2WritePhyReg,
   mpc5748Eth2ReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief MPC5748 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mpc5748Eth2Init(NetInterface *interface)
{
   error_t error;
   uint32_t value;

   //Debug message
   TRACE_INFO("Initializing MPC5748 Ethernet MAC (ENET1)...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   mpc5748Eth2InitGpio(interface);

   //Reset ENET module
   ENET_1->ECR = ENET_ECR_RESET_MASK;
   //Wait for the reset to complete
   while((ENET_1->ECR & ENET_ECR_RESET_MASK) != 0)
   {
   }

   //Configure MAC for MII mode
   ENET_1->RCR = ENET_RCR_MAX_FL(MPC5748_ETH2_RX_BUFFER_SIZE) |
      ENET_RCR_MII_MODE_MASK;

   //Transmit control register
   ENET_1->TCR = 0;
   //Configure MDC clock frequency
   ENET_1->MSCR = ENET_MSCR_MII_SPEED(19);

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

   //Set the MAC address of the station (upper 16 bits)
   value = interface->macAddr.b[5];
   value |= (interface->macAddr.b[4] << 8);
   ENET_1->PAUR = ENET_PAUR_PADDR2(value) | ENET_PAUR_TYPE(0x8808);

   //Set the MAC address of the station (lower 32 bits)
   value = interface->macAddr.b[3];
   value |= (interface->macAddr.b[2] << 8);
   value |= (interface->macAddr.b[1] << 16);
   value |= (interface->macAddr.b[0] << 24);
   ENET_1->PALR = ENET_PALR_PADDR1(value);

   //Hash table for unicast address filtering
   ENET_1->IALR = 0;
   ENET_1->IAUR = 0;
   //Hash table for multicast address filtering
   ENET_1->GALR = 0;
   ENET_1->GAUR = 0;

   //Disable transmit accelerator functions
   ENET_1->TACC = 0;
   //Disable receive accelerator functions
   ENET_1->RACC = 0;

   //Use enhanced buffer descriptors
   ENET_1->ECR = ENET_ECR_EN1588_MASK;

   //Reset statistics counters
   ENET_1->MIBC = ENET_MIBC_MIB_CLEAR_MASK;
   ENET_1->MIBC = 0;

   //Initialize buffer descriptors
   mpc5748Eth2InitBufferDesc(interface);

   //Clear any pending interrupts
   ENET_1->EIR = 0xFFFFFFFF;
   //Enable desired interrupts
   ENET_1->EIMR = ENET_EIMR_TXF_MASK | ENET_EIMR_RXF_MASK | ENET_EIMR_EBERR_MASK;

   //Configure ENET transmit interrupt priority
   INT_SYS_SetPriority(ENET1_GROUP2_IRQn, MPC5748_ETH2_IRQ_PRIORITY);
   //Configure ENET receive interrupt priority
   INT_SYS_SetPriority(ENET1_GROUP1_IRQn, MPC5748_ETH2_IRQ_PRIORITY);
   //Configure ENET error interrupt priority
   INT_SYS_SetPriority(ENET1_GROUP0_IRQn, MPC5748_ETH2_IRQ_PRIORITY);

   //Enable Ethernet MAC
   ENET_1->ECR |= ENET_ECR_ETHEREN_MASK;
   //Instruct the DMA to poll the receive descriptor list
   ENET_1->RDAR = ENET_RDAR_RDAR_MASK;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void mpc5748Eth2InitGpio(NetInterface *interface)
{
//MPC5748G-GW-RDB evaluation board?
#if defined(USE_MPC5748G_GW_RDB)
   //Configure MII_1_TX_CLK (PE12)
   SIUL2->MSCR[76] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[460] = SIUL2_IMCR_SSS(1);

   //Configure MII_1_TX_EN (PI12)
   SIUL2->MSCR[140] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(3);

   //Configure MII_1_TXD0 (PA11)
   SIUL2->MSCR[11] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(3);

   //Configure MII_1_TXD1 (PA10)
   SIUL2->MSCR[10] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(4);

   //Configure MII_1_TXD2 (PH3)
   SIUL2->MSCR[115] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(4);

   //Configure MII_1_TXD3 (PI13)
   SIUL2->MSCR[141] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(3);

   //Configure MII_1_RX_CLK (PB11)
   SIUL2->MSCR[27] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[459] = SIUL2_IMCR_SSS(2);

   //Configure MII_1_RX_DV (PB5)
   SIUL2->MSCR[21] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[465] = SIUL2_IMCR_SSS(2);

   //Configure MII_1_RXD0 (PD10)
   SIUL2->MSCR[58] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[461] = SIUL2_IMCR_SSS(2);

   //Configure MII_1_RXD1 (PD9)
   SIUL2->MSCR[57] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[462] = SIUL2_IMCR_SSS(2);

   //Configure MII_1_RXD2 (PB7)
   SIUL2->MSCR[23] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[463] = SIUL2_IMCR_SSS(2);

   //Configure MII_1_RXD3 (PB6)
   SIUL2->MSCR[22] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[464] = SIUL2_IMCR_SSS(2);

   //Configure PHY reset pin (PI11)
   SIUL2->MSCR[139] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;

   //Reset PHY transceiver
   SIUL2->GPDO[139] = 0;
   sleep(10);
   SIUL2->GPDO[139] = 1;
   sleep(10);

//SJA1105SMBEVM evaluation board?
#elif defined(USE_SJA1105SMBEVM)
   //Configure MII_1_TX_CLK (PE12)
   SIUL2->MSCR[76] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[460] = SIUL2_IMCR_SSS(1);

   //Configure MII_1_TX_EN (PI12)
   SIUL2->MSCR[140] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(3);

   //Configure MII_1_TXD0 (PA11)
   SIUL2->MSCR[11] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(3);

   //Configure MII_1_TXD1 (PA10)
   SIUL2->MSCR[10] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(4);

   //Configure MII_1_TXD2 (PH3)
   SIUL2->MSCR[115] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(4);

   //Configure MII_1_TXD3 (PI13)
   SIUL2->MSCR[141] = SIUL2_MSCR_SRC(3) | SIUL2_MSCR_OBE_MASK |
      SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_SSS(3);

   //Configure MII_1_RX_CLK (PB11)
   SIUL2->MSCR[27] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[459] = SIUL2_IMCR_SSS(2);

   //Configure MII_1_RX_DV (PB5)
   SIUL2->MSCR[21] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[465] = SIUL2_IMCR_SSS(2);

   //Configure MII_1_RXD0 (PB15)
   SIUL2->MSCR[31] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[461] = SIUL2_IMCR_SSS(1);

   //Configure MII_1_RXD1 (PD9)
   SIUL2->MSCR[57] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[462] = SIUL2_IMCR_SSS(2);

   //Configure MII_1_RXD2 (PB7)
   SIUL2->MSCR[23] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[463] = SIUL2_IMCR_SSS(2);

   //Configure MII_1_RXD3 (PB6)
   SIUL2->MSCR[22] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->IMCR[464] = SIUL2_IMCR_SSS(2);

   //Configure PHY reset pin (PI14)
   SIUL2->MSCR[142] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;

   //Reset PHY transceiver
   SIUL2->GPDO[142] = 0;
   sleep(10);
   SIUL2->GPDO[142] = 1;
   sleep(10);
#endif
}


/**
 * @brief Initialize buffer descriptors
 * @param[in] interface Underlying network interface
 **/

void mpc5748Eth2InitBufferDesc(NetInterface *interface)
{
   uint_t i;
   uint32_t address;

   //Clear TX and RX buffer descriptors
   osMemset(txBufferDesc, 0, sizeof(txBufferDesc));
   osMemset(rxBufferDesc, 0, sizeof(rxBufferDesc));

   //Initialize TX buffer descriptors
   for(i = 0; i < MPC5748_ETH2_TX_BUFFER_COUNT; i++)
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
   for(i = 0; i < MPC5748_ETH2_RX_BUFFER_COUNT; i++)
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
   ENET_1->TDSR = (uint32_t) txBufferDesc;
   //Start location of the RX descriptor list
   ENET_1->RDSR = (uint32_t) rxBufferDesc;
   //Maximum receive buffer size
   ENET_1->MRBR = MPC5748_ETH2_RX_BUFFER_SIZE;
}


/**
 * @brief MPC5748 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void mpc5748Eth2Tick(NetInterface *interface)
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

void mpc5748Eth2EnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   INT_SYS_EnableIRQ(ENET1_GROUP2_IRQn);
   INT_SYS_EnableIRQ(ENET1_GROUP1_IRQn);
   INT_SYS_EnableIRQ(ENET1_GROUP0_IRQn);

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

void mpc5748Eth2DisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   INT_SYS_DisableIRQ(ENET1_GROUP2_IRQn);
   INT_SYS_DisableIRQ(ENET1_GROUP1_IRQn);
   INT_SYS_DisableIRQ(ENET1_GROUP0_IRQn);

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
 * @brief Ethernet MAC transmit interrupt
 **/

void ENET1_Tx_IRQHandler(void)
{
   bool_t flag;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Packet transmitted?
   if((ENET_1->EIR & ENET_EIR_TXF_MASK) != 0)
   {
      //Clear TXF interrupt flag
      ENET_1->EIR = ENET_EIR_TXF_MASK;

      //Check whether the TX buffer is available for writing
      if((txBufferDesc[txBufferIndex][0] & ENET_TBD0_R) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag = osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }

      //Instruct the DMA to poll the transmit descriptor list
      ENET_1->TDAR = ENET_TDAR_TDAR_MASK;
   }

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief Ethernet MAC receive interrupt
 **/

void ENET1_Rx_IRQHandler(void)
{
   bool_t flag;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Packet received?
   if((ENET_1->EIR & ENET_EIR_RXF_MASK) != 0)
   {
      //Disable RXF interrupt
      ENET_1->EIMR &= ~ENET_EIMR_RXF_MASK;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag = osSetEventFromIsr(&netEvent);
   }

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief Ethernet MAC error interrupt
 **/

void ENET1_Err_IRQHandler(void)
{
   bool_t flag;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //System bus error?
   if((ENET_1->EIR & ENET_EIR_EBERR_MASK) != 0)
   {
      //Disable EBERR interrupt
      ENET_1->EIMR &= ~ENET_EIMR_EBERR_MASK;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief MPC5748 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void mpc5748Eth2EventHandler(NetInterface *interface)
{
   error_t error;
   uint32_t status;

   //Read interrupt event register
   status = ENET_1->EIR;

   //Packet received?
   if((status & ENET_EIR_RXF_MASK) != 0)
   {
      //Clear RXF interrupt flag
      ENET_1->EIR = ENET_EIR_RXF_MASK;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = mpc5748Eth2ReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }

   //System bus error?
   if((status & ENET_EIR_EBERR_MASK) != 0)
   {
      //Clear EBERR interrupt flag
      ENET_1->EIR = ENET_EIR_EBERR_MASK;

      //Disable Ethernet MAC
      ENET_1->ECR &= ~ENET_ECR_ETHEREN_MASK;
      //Reset buffer descriptors
      mpc5748Eth2InitBufferDesc(interface);
      //Resume normal operation
      ENET_1->ECR |= ENET_ECR_ETHEREN_MASK;
      //Instruct the DMA to poll the receive descriptor list
      ENET_1->RDAR = ENET_RDAR_RDAR_MASK;
   }

   //Re-enable Ethernet MAC interrupts
   ENET_1->EIMR = ENET_EIMR_TXF_MASK | ENET_EIMR_RXF_MASK | ENET_EIMR_EBERR_MASK;
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

error_t mpc5748Eth2SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > MPC5748_ETH2_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txBufferDesc[txBufferIndex][0] & ENET_TBD0_R) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txBufferIndex], buffer, offset, length);

   //Clear BDU flag
   txBufferDesc[txBufferIndex][4] = 0;

   //Check current index
   if(txBufferIndex < (MPC5748_ETH2_TX_BUFFER_COUNT - 1))
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
   ENET_1->TDAR = ENET_TDAR_TDAR_MASK;

   //Check whether the next buffer is available for writing
   if((txBufferDesc[txBufferIndex][0] & ENET_TBD0_R) == 0)
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

error_t mpc5748Eth2ReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxBufferDesc[rxBufferIndex][0] & ENET_RBD0_E) == 0)
   {
      //The frame should not span multiple buffers
      if((rxBufferDesc[rxBufferIndex][0] & ENET_RBD0_L) != 0)
      {
         //Check whether an error occurred
         if((rxBufferDesc[rxBufferIndex][0] & (ENET_RBD0_LG | ENET_RBD0_NO |
            ENET_RBD0_CR | ENET_RBD0_OV | ENET_RBD0_TR)) == 0)
         {
            //Retrieve the length of the frame
            n = rxBufferDesc[rxBufferIndex][0] & ENET_RBD0_DATA_LENGTH;
            //Limit the number of data to read
            n = MIN(n, MPC5748_ETH2_RX_BUFFER_SIZE);

            //Additional options can be passed to the stack along with the packet
            ancillary = NET_DEFAULT_RX_ANCILLARY;

            //Pass the packet to the upper layer
            nicProcessPacket(interface, rxBuffer[rxBufferIndex], n, &ancillary);

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
      if(rxBufferIndex < (MPC5748_ETH2_RX_BUFFER_COUNT - 1))
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
      ENET_1->RDAR = ENET_RDAR_RDAR_MASK;
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

error_t mpc5748Eth2UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint32_t value;
   uint32_t unicastHashTable[2];
   uint32_t multicastHashTable[2];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Set the MAC address of the station (upper 16 bits)
   value = interface->macAddr.b[5];
   value |= (interface->macAddr.b[4] << 8);
   ENET_1->PAUR = ENET_PAUR_PADDR2(value) | ENET_PAUR_TYPE(0x8808);

   //Set the MAC address of the station (lower 32 bits)
   value = interface->macAddr.b[3];
   value |= (interface->macAddr.b[2] << 8);
   value |= (interface->macAddr.b[1] << 16);
   value |= (interface->macAddr.b[0] << 24);
   ENET_1->PALR = ENET_PALR_PADDR1(value);

   //Clear hash table (unicast address filtering)
   unicastHashTable[0] = 0;
   unicastHashTable[1] = 0;

   //Clear hash table (multicast address filtering)
   multicastHashTable[0] = 0;
   multicastHashTable[1] = 0;

   //The MAC address filter contains the list of MAC addresses to accept
   //when receiving an Ethernet frame
   for(i = 0; i < MAC_ADDR_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->macAddrFilter[i];

      //Valid entry?
      if(entry->refCount > 0)
      {
         //Compute CRC over the current MAC address
         crc = mpc5748Eth2CalcCrc(&entry->addr, sizeof(MacAddr));

         //The upper 6 bits in the CRC register are used to index the
         //contents of the hash table
         k = (crc >> 26) & 0x3F;

         //Multicast address?
         if(macIsMulticastAddr(&entry->addr))
         {
            //Update the multicast hash table
            multicastHashTable[k / 32] |= (1 << (k % 32));
         }
         else
         {
            //Update the unicast hash table
            unicastHashTable[k / 32] |= (1 << (k % 32));
         }
      }
   }

   //Write the hash table (unicast address filtering)
   ENET_1->IALR = unicastHashTable[0];
   ENET_1->IAUR = unicastHashTable[1];

   //Write the hash table (multicast address filtering)
   ENET_1->GALR = multicastHashTable[0];
   ENET_1->GAUR = multicastHashTable[1];

   //Debug message
   TRACE_DEBUG("  IALR = %08" PRIX32 "\r\n", ENET_1->IALR);
   TRACE_DEBUG("  IAUR = %08" PRIX32 "\r\n", ENET_1->IAUR);
   TRACE_DEBUG("  GALR = %08" PRIX32 "\r\n", ENET_1->GALR);
   TRACE_DEBUG("  GAUR = %08" PRIX32 "\r\n", ENET_1->GAUR);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mpc5748Eth2UpdateMacConfig(NetInterface *interface)
{
   //Disable Ethernet MAC while modifying configuration registers
   ENET_1->ECR &= ~ENET_ECR_ETHEREN_MASK;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      //100 Mbps operation
      ENET_1->RCR &= ~ENET_RCR_RMII_10T_MASK;
   }
   else
   {
      //10 Mbps operation
      ENET_1->RCR |= ENET_RCR_RMII_10T_MASK;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      //Full-duplex mode
      ENET_1->TCR |= ENET_TCR_FDEN_MASK;
      //Receive path operates independently of transmit
      ENET_1->RCR &= ~ENET_RCR_DRT_MASK;
   }
   else
   {
      //Half-duplex mode
      ENET_1->TCR &= ~ENET_TCR_FDEN_MASK;
      //Disable reception of frames while transmitting
      ENET_1->RCR |= ENET_RCR_DRT_MASK;
   }

   //Reset buffer descriptors
   mpc5748Eth2InitBufferDesc(interface);

   //Re-enable Ethernet MAC
   ENET_1->ECR |= ENET_ECR_ETHEREN_MASK;
   //Instruct the DMA to poll the receive descriptor list
   ENET_1->RDAR = ENET_RDAR_RDAR_MASK;

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

void mpc5748Eth2WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Set up a write operation
      temp = ENET_MMFR_ST(1) | ENET_MMFR_OP(1) | ENET_MMFR_TA(2);
      //PHY address
      temp |= ENET_MMFR_PA(phyAddr);
      //Register address
      temp |= ENET_MMFR_RA(regAddr);
      //Register value
      temp |= ENET_MMFR_DATA(data);

      //Clear MII interrupt flag
      ENET_1->EIR = ENET_EIR_MII_MASK;
      //Start a write operation
      ENET_1->MMFR = temp;

      //Wait for the write to complete
      while((ENET_1->EIR & ENET_EIR_MII_MASK) == 0)
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

uint16_t mpc5748Eth2ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Set up a read operation
      temp = ENET_MMFR_ST(1) | ENET_MMFR_OP(2) | ENET_MMFR_TA(2);
      //PHY address
      temp |= ENET_MMFR_PA(phyAddr);
      //Register address
      temp |= ENET_MMFR_RA(regAddr);

      //Clear MII interrupt flag
      ENET_1->EIR = ENET_EIR_MII_MASK;
      //Start a read operation
      ENET_1->MMFR = temp;

      //Wait for the read to complete
      while((ENET_1->EIR & ENET_EIR_MII_MASK) == 0)
      {
      }

      //Get register value
      data = ENET_1->MMFR & ENET_MMFR_DATA_MASK;
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

uint32_t mpc5748Eth2CalcCrc(const void *data, size_t length)
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
      //Update CRC value
      crc ^= p[i];

      //The message is processed bit by bit
      for(j = 0; j < 8; j++)
      {
         if((crc & 0x01) != 0)
         {
            crc = (crc >> 1) ^ 0xEDB88320;
         }
         else
         {
            crc = crc >> 1;
         }
      }
   }

   //Return CRC value
   return crc;
}
