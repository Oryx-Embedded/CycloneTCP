/**
 * @file mcxn547_eth_driver.c
 * @brief NXP MCX N547 Ethernet MAC driver
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
#include "fsl_device_registers.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_reset.h"
#include "core/net.h"
#include "drivers/mac/mcxn547_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
static uint8_t txBuffer[MCXN547_ETH_TX_BUFFER_COUNT][MCXN547_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
static uint8_t rxBuffer[MCXN547_ETH_RX_BUFFER_COUNT][MCXN547_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
static Mcxn547TxDmaDesc txDmaDesc[MCXN547_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
static Mcxn547RxDmaDesc rxDmaDesc[MCXN547_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[MCXN547_ETH_TX_BUFFER_COUNT][MCXN547_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Receive buffer
static uint8_t rxBuffer[MCXN547_ETH_RX_BUFFER_COUNT][MCXN547_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Transmit DMA descriptors
static Mcxn547TxDmaDesc txDmaDesc[MCXN547_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//Receive DMA descriptors
static Mcxn547RxDmaDesc rxDmaDesc[MCXN547_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//Current transmit descriptor
static uint_t txIndex;
//Current receive descriptor
static uint_t rxIndex;


/**
 * @brief MCX N547 Ethernet MAC driver
 **/

const NicDriver mcxn547EthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   mcxn547EthInit,
   mcxn547EthTick,
   mcxn547EthEnableIrq,
   mcxn547EthDisableIrq,
   mcxn547EthEventHandler,
   mcxn547EthSendPacket,
   mcxn547EthUpdateMacAddrFilter,
   mcxn547EthUpdateMacConfig,
   mcxn547EthWritePhyReg,
   mcxn547EthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief MCX N547 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mcxn547EthInit(NetInterface *interface)
{
   error_t error;
   uint32_t temp;

   //Debug message
   TRACE_INFO("Initializing MCX N547 Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Attach NONE clock to ENETRMII
   CLOCK_EnableClock(kCLOCK_InputMux);
   CLOCK_AttachClk(kNONE_to_ENETRMII);

   //Enable ENET peripheral clock
   CLOCK_EnableClock(kCLOCK_Enet);

   //Reset ENET module
   SYSCON0->PRESETCTRL2 = SYSCON_PRESETCTRL2_ENET_RST_MASK;
   SYSCON0->PRESETCTRL2 &= ~SYSCON_PRESETCTRL2_ENET_RST_MASK;

   //GPIO configuration
   mcxn547EthInitGpio(interface);

   //Perform a software reset
   ENET->DMA_MODE |= ENET_DMA_MODE_SWR_MASK;
   //Wait for the reset to complete
   while((ENET->DMA_MODE & ENET_DMA_MODE_SWR_MASK) != 0)
   {
   }

   //Adjust MDC clock range depending on CSR frequency
   ENET->MAC_MDIO_ADDRESS = ENET_MAC_MDIO_ADDRESS_CR(4);

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
   ENET->MAC_CONFIGURATION = ENET_MAC_CONFIGURATION_GPSLCE_MASK |
      ENET_MAC_CONFIGURATION_PS_MASK | ENET_MAC_CONFIGURATION_DO_MASK;

   //Set the maximum packet size that can be accepted
   temp = ENET->MAC_EXT_CONFIGURATION & ~ENET_MAC_EXT_CONFIGURATION_GPSL_MASK;
   ENET->MAC_EXT_CONFIGURATION = temp | MCXN547_ETH_RX_BUFFER_SIZE;

   //Set the MAC address of the station
   ENET->MAC_ADDRESS0_LOW = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ENET->MAC_ADDRESS0_HIGH = interface->macAddr.w[2];

   //Configure the receive filter
   ENET->MAC_PACKET_FILTER = 0;

   //Disable flow control
   ENET->MAC_TX_FLOW_CTRL_Q[0] = 0;
   ENET->MAC_RX_FLOW_CTRL = 0;

   //Enable the first RX queue
   ENET->MAC_RXQ_CTRL[0] = ENET_MAC_RXQ_CTRL_RXQ0EN(2);

   //Configure DMA operating mode
   ENET->DMA_MODE = ENET_DMA_MODE_PR(0);
   //Configure system bus mode
   ENET->DMA_SYSBUS_MODE |= ENET_DMA_SYSBUS_MODE_AAL_MASK;

   //The DMA takes the descriptor table as contiguous
   ENET->DMA_CH[0].DMA_CHX_CTRL = ENET_DMA_CH_DMA_CHX_CTRL_DSL(0);
   //Configure TX features
   ENET->DMA_CH[0].DMA_CHX_TX_CTRL = ENET_DMA_CH_DMA_CHX_TX_CTRL_TxPBL(32);

   //Configure RX features
   ENET->DMA_CH[0].DMA_CHX_RX_CTRL = ENET_DMA_CH_DMA_CHX_RX_CTRL_RxPBL(32) |
      ENET_DMA_CH_DMA_CHX_RX_CTRL_RBSZ_13_Y(MCXN547_ETH_RX_BUFFER_SIZE / 4);

   //Enable store and forward mode for transmission
   ENET->MTL_QUEUE[0].MTL_TXQX_OP_MODE |= ENET_MTL_QUEUE_MTL_TXQX_OP_MODE_TQS(7) |
      ENET_MTL_QUEUE_MTL_TXQX_OP_MODE_TXQEN(2) |
      ENET_MTL_QUEUE_MTL_TXQX_OP_MODE_TSF_MASK;

   //Enable store and forward mode for reception
   ENET->MTL_QUEUE[0].MTL_RXQX_OP_MODE |= ENET_MTL_QUEUE_MTL_RXQX_OP_MODE_RQS(7) |
      ENET_MTL_QUEUE_MTL_RXQX_OP_MODE_RSF_MASK;

   //Initialize DMA descriptor lists
   mcxn547EthInitDmaDesc(interface);

   //Disable MAC interrupts
   ENET->MAC_INTERRUPT_ENABLE = 0;

   //Enable the desired DMA interrupts
   ENET->DMA_CH[0].DMA_CHX_INT_EN = ENET_DMA_CH_DMA_CHX_INT_EN_NIE_MASK |
      ENET_DMA_CH_DMA_CHX_INT_EN_RIE_MASK | ENET_DMA_CH_DMA_CHX_INT_EN_TIE_MASK;

   //Set priority grouping (3 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(MCXN547_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(ETHERNET_IRQn, NVIC_EncodePriority(MCXN547_ETH_IRQ_PRIORITY_GROUPING,
      MCXN547_ETH_IRQ_GROUP_PRIORITY, MCXN547_ETH_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   ENET->MAC_CONFIGURATION |= ENET_MAC_CONFIGURATION_TE_MASK |
      ENET_MAC_CONFIGURATION_RE_MASK;

   //Enable DMA transmission and reception
   ENET->DMA_CH[0].DMA_CHX_TX_CTRL |= ENET_DMA_CH_DMA_CHX_TX_CTRL_ST_MASK;
   ENET->DMA_CH[0].DMA_CHX_RX_CTRL |= ENET_DMA_CH_DMA_CHX_RX_CTRL_SR_MASK;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void mcxn547EthInitGpio(NetInterface *interface)
{
//MCX-N5XX-EVK evaluation board?
#if defined(USE_MCX_N5XX_EVK)
   port_pin_config_t portPinConfig;

   //Enable PORT1 clock
   CLOCK_EnableClock(kCLOCK_Port1);

   //Select RMII interface mode
   SYSCON0->ENET_PHY_INTF_SEL |= SYSCON_ENET_PHY_INTF_SEL_PHY_SEL_MASK;

   //Configure RMII pins
   portPinConfig.pullSelect = kPORT_PullDisable;
   portPinConfig.pullValueSelect = kPORT_LowPullResistor;
   portPinConfig.slewRate = kPORT_FastSlewRate;
   portPinConfig.passiveFilterEnable = kPORT_PassiveFilterDisable;
   portPinConfig.openDrainEnable = kPORT_OpenDrainDisable;
   portPinConfig.driveStrength = kPORT_LowDriveStrength;
   portPinConfig.mux = kPORT_MuxAlt9;
   portPinConfig.inputBuffer = kPORT_InputBufferEnable;
   portPinConfig.invertInput = kPORT_InputNormal;
   portPinConfig.lockRegister = kPORT_UnlockRegister;

   //Configure PORT1_4 as ENET0_TX_CLK
   PORT_SetPinConfig(PORT1, 4, &portPinConfig);
   //Configure PORT1_5 as ENET0_TXEN
   PORT_SetPinConfig(PORT1, 5, &portPinConfig);
   //Configure PORT1_6 as ENET0_TXD0
   PORT_SetPinConfig(PORT1, 6, &portPinConfig);
   //Configure PORT1_7 as ENET0_TXD1
   PORT_SetPinConfig(PORT1, 7, &portPinConfig);

   //Configure PORT1_13 as ENET0_RXDV
   PORT_SetPinConfig(PORT1, 13, &portPinConfig);
   //Configure PORT1_14 as ENET0_RXD0
   PORT_SetPinConfig(PORT1, 14, &portPinConfig);
   //Configure PORT1_15 as ENET0_RXD1
   PORT_SetPinConfig(PORT1, 15, &portPinConfig);

   //Configure PORT1_20 as ENET0_MDC
   PORT_SetPinConfig(PORT1, 20, &portPinConfig);
   //Configure PORT1_21 as ENET0_MDIO
   PORT_SetPinConfig(PORT1, 21, &portPinConfig);
#endif
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void mcxn547EthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < MCXN547_ETH_TX_BUFFER_COUNT; i++)
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
   for(i = 0; i < MCXN547_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = (uint32_t) rxBuffer[i];
      rxDmaDesc[i].rdes1 = 0;
      rxDmaDesc[i].rdes2 = 0;
      rxDmaDesc[i].rdes3 = ENET_RDES3_OWN | ENET_RDES3_IOC | ENET_RDES3_BUF1V;
   }

   //Initialize RX descriptor index
   rxIndex = 0;

   //Start location of the TX descriptor list
   ENET->DMA_CH[0].DMA_CHX_TXDESC_LIST_ADDR = (uint32_t) &txDmaDesc[0];
   //Length of the transmit descriptor ring
   ENET->DMA_CH[0].DMA_CHX_TXDESC_RING_LENGTH = MCXN547_ETH_TX_BUFFER_COUNT - 1;

   //Start location of the RX descriptor list
   ENET->DMA_CH[0].DMA_CHX_RXDESC_LIST_ADDR = (uint32_t) &rxDmaDesc[0];
   //Length of the receive descriptor ring
   ENET->DMA_CH[0].DMA_CHX_RX_CONTROL2 = MCXN547_ETH_RX_BUFFER_COUNT - 1;
}


/**
 * @brief MCX N547 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void mcxn547EthTick(NetInterface *interface)
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

void mcxn547EthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(ETHERNET_IRQn);

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

void mcxn547EthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(ETHERNET_IRQn);

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
 * @brief MCX N547 Ethernet MAC interrupt service routine
 **/

void ETHERNET_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = ENET->DMA_CH[0].DMA_CHX_STAT;

   //Packet transmitted?
   if((status & ENET_DMA_CH_DMA_CHX_STAT_TI_MASK) != 0)
   {
      //Clear TI interrupt flag
      ENET->DMA_CH[0].DMA_CHX_STAT = ENET_DMA_CH_DMA_CHX_STAT_TI_MASK;

      //Check whether the TX buffer is available for writing
      if((txDmaDesc[txIndex].tdes3 & ENET_TDES3_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & ENET_DMA_CH_DMA_CHX_STAT_RI_MASK) != 0)
   {
      //Clear RI interrupt flag
      ENET->DMA_CH[0].DMA_CHX_STAT = ENET_DMA_CH_DMA_CHX_STAT_RI_MASK;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   ENET->DMA_CH[0].DMA_CHX_STAT = ENET_DMA_CH_DMA_CHX_STAT_NIS_MASK;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief MCX N547 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void mcxn547EthEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = mcxn547EthReceivePacket(interface);

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

error_t mcxn547EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > MCXN547_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txDmaDesc[txIndex].tdes3 & ENET_TDES3_OWN) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txIndex], buffer, offset, length);

   //Set the start address of the buffer
   txDmaDesc[txIndex].tdes0 = (uint32_t) txBuffer[txIndex];
   //Write the number of bytes to send
   txDmaDesc[txIndex].tdes2 = ENET_TDES2_IOC | (length & ENET_TDES2_B1L);
   //Give the ownership of the descriptor to the DMA
   txDmaDesc[txIndex].tdes3 = ENET_TDES3_OWN | ENET_TDES3_FD | ENET_TDES3_LD;

   //Clear TBU flag to resume processing
   ENET->DMA_CH[0].DMA_CHX_STAT = ENET_DMA_CH_DMA_CHX_STAT_TBU_MASK;
   //Instruct the DMA to poll the transmit descriptor list
   ENET->DMA_CH[0].DMA_CHX_TXDESC_TAIL_PTR = 0;

   //Increment index and wrap around if necessary
   if(++txIndex >= MCXN547_ETH_TX_BUFFER_COUNT)
   {
      txIndex = 0;
   }

   //Check whether the next buffer is available for writing
   if((txDmaDesc[txIndex].tdes3 & ENET_TDES3_OWN) == 0)
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

error_t mcxn547EthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_OWN) == 0)
   {
      //FD and LD flags should be set
      if((rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_FD) != 0 &&
         (rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_LD) != 0)
      {
         //Make sure no error occurred
         if((rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_ES) == 0)
         {
            //Retrieve the length of the frame
            n = rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_PL;
            //Limit the number of data to read
            n = MIN(n, MCXN547_ETH_RX_BUFFER_SIZE);

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
      rxDmaDesc[rxIndex].rdes3 = ENET_RDES3_OWN | ENET_RDES3_IOC | ENET_RDES3_BUF1V;

      //Increment index and wrap around if necessary
      if(++rxIndex >= MCXN547_ETH_RX_BUFFER_COUNT)
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
   ENET->DMA_CH[0].DMA_CHX_STAT = ENET_DMA_CH_DMA_CHX_STAT_RBU_MASK;
   //Instruct the DMA to poll the receive descriptor list
   ENET->DMA_CH[0].DMA_CHX_RXDESC_TAIL_PTR = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mcxn547EthUpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   bool_t acceptMulticast;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Set the MAC address of the station
   ENET->MAC_ADDRESS0_LOW = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ENET->MAC_ADDRESS0_HIGH = interface->macAddr.w[2];

   //This flag will be set if multicast addresses should be accepted
   acceptMulticast = FALSE;

   //The MAC address filter contains the list of MAC addresses to accept
   //when receiving an Ethernet frame
   for(i = 0; i < MAC_ADDR_FILTER_SIZE; i++)
   {
      //Valid entry?
      if(interface->macAddrFilter[i].refCount > 0)
      {
         //Accept multicast addresses
         acceptMulticast = TRUE;
         //We are done
         break;
      }
   }

   //Enable or disable the reception of multicast frames
   if(acceptMulticast)
   {
      ENET->MAC_PACKET_FILTER |= ENET_MAC_PACKET_FILTER_PM_MASK;
   }
   else
   {
      ENET->MAC_PACKET_FILTER &= ~ENET_MAC_PACKET_FILTER_PM_MASK;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mcxn547EthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = ENET->MAC_CONFIGURATION;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= ENET_MAC_CONFIGURATION_FES_MASK;
   }
   else
   {
      config &= ~ENET_MAC_CONFIGURATION_FES_MASK;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= ENET_MAC_CONFIGURATION_DM_MASK;
   }
   else
   {
      config &= ~ENET_MAC_CONFIGURATION_DM_MASK;
   }

   //Update MAC configuration register
   ENET->MAC_CONFIGURATION = config;

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

void mcxn547EthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = ENET->MAC_MDIO_ADDRESS & ENET_MAC_MDIO_ADDRESS_CR_MASK;
      //Set up a write operation
      temp |= ENET_MAC_MDIO_ADDRESS_GOC_0_MASK | ENET_MAC_MDIO_ADDRESS_GB_MASK;

      //PHY address
      temp |= ENET_MAC_MDIO_ADDRESS_PA(phyAddr);
      //Register address
      temp |= ENET_MAC_MDIO_ADDRESS_RDA(regAddr);

      //Data to be written in the PHY register
      ENET->MAC_MDIO_DATA = data & ENET_MAC_MDIO_DATA_GD_MASK;

      //Start a write operation
      ENET->MAC_MDIO_ADDRESS = temp;
      //Wait for the write to complete
      while((ENET->MAC_MDIO_ADDRESS & ENET_MAC_MDIO_ADDRESS_GB_MASK) != 0)
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

uint16_t mcxn547EthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = ENET->MAC_MDIO_ADDRESS & ENET_MAC_MDIO_ADDRESS_CR_MASK;

      //Set up a read operation
      temp |= ENET_MAC_MDIO_ADDRESS_GOC_1_MASK |
         ENET_MAC_MDIO_ADDRESS_GOC_0_MASK | ENET_MAC_MDIO_ADDRESS_GB_MASK;

      //PHY address
      temp |= ENET_MAC_MDIO_ADDRESS_PA(phyAddr);
      //Register address
      temp |= ENET_MAC_MDIO_ADDRESS_RDA(regAddr);

      //Start a read operation
      ENET->MAC_MDIO_ADDRESS = temp;
      //Wait for the read to complete
      while((ENET->MAC_MDIO_ADDRESS & ENET_MAC_MDIO_ADDRESS_GB_MASK) != 0)
      {
      }

      //Get register value
      data = ENET->MAC_MDIO_DATA & ENET_MAC_MDIO_DATA_GD_MASK;
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
      data = 0;
   }

   //Return the value of the PHY register
   return data;
}
