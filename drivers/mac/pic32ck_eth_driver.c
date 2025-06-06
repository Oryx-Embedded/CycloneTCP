/**
 * @file pic32ck_eth_driver.c
 * @brief PIC32CK GC01/SG01 Ethernet MAC driver
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
#include "pic32c.h"
#include "core/net.h"
#include "drivers/mac/pic32ck_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//TX buffer
#pragma data_alignment = 8
static uint8_t txBuffer[PIC32CK_ETH_TX_BUFFER_COUNT][PIC32CK_ETH_TX_BUFFER_SIZE];
//RX buffer
#pragma data_alignment = 8
static uint8_t rxBuffer[PIC32CK_ETH_RX_BUFFER_COUNT][PIC32CK_ETH_RX_BUFFER_SIZE];
//TX buffer descriptors
#pragma data_alignment = 8
static Pic32ckTxBufferDesc txBufferDesc[PIC32CK_ETH_TX_BUFFER_COUNT];
//RX buffer descriptors
#pragma data_alignment = 8
static Pic32ckRxBufferDesc rxBufferDesc[PIC32CK_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//TX buffer
static uint8_t txBuffer[PIC32CK_ETH_TX_BUFFER_COUNT][PIC32CK_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(8)));
//RX buffer
static uint8_t rxBuffer[PIC32CK_ETH_RX_BUFFER_COUNT][PIC32CK_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(8)));
//TX buffer descriptors
static Pic32ckTxBufferDesc txBufferDesc[PIC32CK_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(8)));
//RX buffer descriptors
static Pic32ckRxBufferDesc rxBufferDesc[PIC32CK_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(8)));

#endif

//TX buffer index
static uint_t txBufferIndex;
//RX buffer index
static uint_t rxBufferIndex;


/**
 * @brief PIC32CK Ethernet MAC driver
 **/

const NicDriver pic32ckEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   pic32ckEthInit,
   pic32ckEthTick,
   pic32ckEthEnableIrq,
   pic32ckEthDisableIrq,
   pic32ckEthEventHandler,
   pic32ckEthSendPacket,
   pic32ckEthUpdateMacAddrFilter,
   pic32ckEthUpdateMacConfig,
   pic32ckEthWritePhyReg,
   pic32ckEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief PIC32CK Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t pic32ckEthInit(NetInterface *interface)
{
   error_t error;
   volatile uint32_t status;

   //Debug message
   TRACE_INFO("Initializing PIC32CK Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable CLK_ETH_TX core clock
   GCLK_REGS->GCLK_PCHCTRL[ETH_GCLK_ID_TX] = GCLK_PCHCTRL_GEN_GCLK0 |
      GCLK_PCHCTRL_CHEN_Msk;

   //Wait for synchronization
   while((GCLK_REGS->GCLK_PCHCTRL[ETH_GCLK_ID_TX] & GCLK_PCHCTRL_CHEN_Msk) == 0)
   {
   }

   //Enable CLK_ETH_TSU core clock
   GCLK_REGS->GCLK_PCHCTRL[ETH_GCLK_ID_TSU] = GCLK_PCHCTRL_GEN_GCLK0 |
      GCLK_PCHCTRL_CHEN_Msk;

   //Wait for synchronization
   while((GCLK_REGS->GCLK_PCHCTRL[ETH_GCLK_ID_TSU] & GCLK_PCHCTRL_CHEN_Msk) == 0)
   {
   }

   //Enable ETH bus clocks (CLK_ETH_APB and CLK_ETH_AHB)
   MCLK_REGS->MCLK_CLKMSK[ETH_MCLK_ID_APB / 32] |= (1U << (ETH_MCLK_ID_APB % 32));
   MCLK_REGS->MCLK_CLKMSK[ETH_MCLK_ID_AHB / 32] |= (1U << (ETH_MCLK_ID_AHB % 32));

   //Enable ETH module
   ETH_REGS->ETH_CTRLA = ETH_CTRLA_ENABLE_Msk;

   //Wait for synchronization
   while(ETH_REGS->ETH_SYNCB != 0)
   {
   }

   //Disable transmit and receive circuits
   ETH_REGS->ETH_NCR = 0;

   //GPIO configuration
   pic32ckEthInitGpio(interface);

   //Configure MDC clock speed
   ETH_REGS->ETH_NCFGR = ETH_NCFGR_DBW(1) | ETH_NCFGR_CLK(5);
   //Enable management port (MDC and MDIO)
   ETH_REGS->ETH_NCR |= ETH_NCR_MPE_Msk;

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
   ETH_REGS->SA[0].ETH_SAB = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ETH_REGS->SA[0].ETH_SAT = interface->macAddr.w[2];

   //The MAC supports 3 additional addresses for unicast perfect filtering
   ETH_REGS->SA[1].ETH_SAB = 0;
   ETH_REGS->SA[2].ETH_SAB = 0;
   ETH_REGS->SA[3].ETH_SAB = 0;

   //Initialize hash table
   ETH_REGS->ETH_HRB = 0;
   ETH_REGS->ETH_HRT = 0;

   //Configure the receive filter
   ETH_REGS->ETH_NCFGR |= ETH_NCFGR_MAXFS_Msk | ETH_NCFGR_MTIHEN_Msk;

   //Initialize buffer descriptors
   pic32ckEthInitBufferDesc(interface);

   //Clear transmit status register
   ETH_REGS->ETH_TSR = ETH_TSR_HRESP_Msk | ETH_TSR_UND_Msk |
      ETH_TSR_TXCOMP_Msk | ETH_TSR_TFC_Msk | ETH_TSR_TXGO_Msk |
      ETH_TSR_RLE_Msk | ETH_TSR_COL_Msk | ETH_TSR_UBR_Msk;

   //Clear receive status register
   ETH_REGS->ETH_RSR = ETH_RSR_HNO_Msk | ETH_RSR_RXOVR_Msk |
      ETH_RSR_REC_Msk | ETH_RSR_BNA_Msk;

   //First disable all ETH interrupts
   ETH_REGS->ETH_IDR = 0xFFFFFFFF;

   //Only the desired ones are enabled
   ETH_REGS->ETH_IER = ETH_IER_HRESP_Msk | ETH_IER_ROVR_Msk |
      ETH_IER_TCOMP_Msk | ETH_IER_TFC_Msk | ETH_IER_RLEX_Msk |
      ETH_IER_TUR_Msk | ETH_IER_RXUBR_Msk | ETH_IER_RCOMP_Msk;

   //Read ETH_ISR register to clear any pending interrupt
   status = ETH_REGS->ETH_ISR;
   (void) status;

   //Set priority grouping (3 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(PIC32CK_ETH_IRQ_PRIORITY_GROUPING);

   //Configure ETH interrupt priority
   NVIC_SetPriority(ETH_IRQn, NVIC_EncodePriority(PIC32CK_ETH_IRQ_PRIORITY_GROUPING,
      PIC32CK_ETH_IRQ_GROUP_PRIORITY, PIC32CK_ETH_IRQ_SUB_PRIORITY));

   //Enable the ETH to transmit and receive data
   ETH_REGS->ETH_NCR |= ETH_NCR_TXEN_Msk | ETH_NCR_RXEN_Msk;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void pic32ckEthInitGpio(NetInterface *interface)
{
//PIC32CK GC01/SG01 Curiosity Ultra evaluation board?
#if defined(USE_PIC32CK_GC01_CURIOSITY_ULTRA) || \
   defined(USE_PIC32CK_SG01_CURIOSITY_ULTRA)
   uint32_t temp;

   //Enable PORT bus clock (CLK_PORT_APB)
   MCLK_REGS->MCLK_CLKMSK[PORT_MCLK_ID_APB / 32] |= (1U << (PORT_MCLK_ID_APB % 32));

   //Configure ETH_REF_CLK (PC0)
   PORT_REGS->GROUP[2].PORT_PINCFG[0] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[0] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[0] = temp | PORT_PMUX_PMUXE(MUX_PC00L_ETH_REF_CLK);

   //Configure ETH_MDC (PC3)
   PORT_REGS->GROUP[2].PORT_PINCFG[3] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[1] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[1] = temp | PORT_PMUX_PMUXO(MUX_PC03L_ETH_MDC);

   //Configure ETH_MDIO (PC4)
   PORT_REGS->GROUP[2].PORT_PINCFG[4] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[2] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[2] = temp | PORT_PMUX_PMUXE(MUX_PC04L_ETH_MDIO);

   //Configure ETH_RXD1 (PC6)
   PORT_REGS->GROUP[2].PORT_PINCFG[6] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[3] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[3] = temp | PORT_PMUX_PMUXE(MUX_PC06L_ETH_RXD1);

   //Configure ETH_RXD0 (PC7)
   PORT_REGS->GROUP[2].PORT_PINCFG[7] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[3] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[3] = temp | PORT_PMUX_PMUXO(MUX_PC07L_ETH_RXD0);

   //Configure ETH_RXER (PC9)
   PORT_REGS->GROUP[2].PORT_PINCFG[9] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[4] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[4] = temp | PORT_PMUX_PMUXO(MUX_PC09L_ETH_RXER);

   //Configure ETH_RXDV (PC10)
   PORT_REGS->GROUP[2].PORT_PINCFG[10] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[5] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[5] = temp | PORT_PMUX_PMUXE(MUX_PC10L_ETH_RXDV);

   //Configure ETH_TXEN (PC11)
   PORT_REGS->GROUP[2].PORT_PINCFG[11] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[5] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[5] = temp | PORT_PMUX_PMUXO(MUX_PC11L_ETH_TXEN);

   //Configure ETH_TXD0 (PC12)
   PORT_REGS->GROUP[2].PORT_PINCFG[12] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[6] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[6] = temp | PORT_PMUX_PMUXE(MUX_PC12L_ETH_TXD0);

   //Configure ETH_TXD1 (PC13)
   PORT_REGS->GROUP[2].PORT_PINCFG[13] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[2].PORT_PMUX[6] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[2].PORT_PMUX[6] = temp | PORT_PMUX_PMUXO(MUX_PC13L_ETH_TXD1);

   //Select RMII operation mode
   ETH_REGS->ETH_UR &= ~ETH_UR_MII_Msk;

   //Configure PHY_RESET (PD21) as an output
   PORT_REGS->GROUP[3].PORT_DIRSET = PORT_PD21;

   //Reset PHY transceiver
   PORT_REGS->GROUP[3].PORT_OUTCLR = PORT_PD21;
   sleep(10);
   PORT_REGS->GROUP[3].PORT_OUTSET = PORT_PD21;
   sleep(10);
#endif
}


/**
 * @brief Initialize buffer descriptors
 * @param[in] interface Underlying network interface
 **/

void pic32ckEthInitBufferDesc(NetInterface *interface)
{
   uint_t i;
   uint32_t address;

   //Initialize TX buffer descriptors
   for(i = 0; i < PIC32CK_ETH_TX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current TX buffer
      address = (uint32_t) txBuffer[i];
      //Write the address to the descriptor entry
      txBufferDesc[i].address = address;
      //Initialize status field
      txBufferDesc[i].status = ETH_TX_USED;
   }

   //Mark the last descriptor entry with the wrap flag
   txBufferDesc[i - 1].status |= ETH_TX_WRAP;
   //Initialize TX buffer index
   txBufferIndex = 0;

   //Initialize RX buffer descriptors
   for(i = 0; i < PIC32CK_ETH_RX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current RX buffer
      address = (uint32_t) rxBuffer[i];
      //Write the address to the descriptor entry
      rxBufferDesc[i].address = address & ETH_RX_ADDRESS;
      //Clear status field
      rxBufferDesc[i].status = 0;
   }

   //Mark the last descriptor entry with the wrap flag
   rxBufferDesc[i - 1].address |= ETH_RX_WRAP;
   //Initialize RX buffer index
   rxBufferIndex = 0;

   //Start location of the TX descriptor list
   ETH_REGS->ETH_TBQB = (uint32_t) txBufferDesc;
   //Start location of the RX descriptor list
   ETH_REGS->ETH_RBQB = (uint32_t) rxBufferDesc;
}


/**
 * @brief PIC32CK Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void pic32ckEthTick(NetInterface *interface)
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

void pic32ckEthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(ETH_IRQn);

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

void pic32ckEthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(ETH_IRQn);

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
 * @brief PIC32CK Ethernet MAC interrupt service routine
 **/

void ETH_Handler(void)
{
   bool_t flag;
   volatile uint32_t isr;
   volatile uint32_t tsr;
   volatile uint32_t rsr;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Each time the software reads ETH_ISR, it has to check the contents
   //of ETH_TSR, ETH_RSR and ETH_NSR
   isr = ETH_REGS->ETH_ISR;
   tsr = ETH_REGS->ETH_TSR;
   rsr = ETH_REGS->ETH_RSR;

   //Clear interrupt flags
   ETH_REGS->ETH_ISR = isr;

   //Packet transmitted?
   if((tsr & (ETH_TSR_HRESP_Msk | ETH_TSR_UND_Msk |
      ETH_TSR_TXCOMP_Msk | ETH_TSR_TFC_Msk | ETH_TSR_TXGO_Msk |
      ETH_TSR_RLE_Msk | ETH_TSR_COL_Msk | ETH_TSR_UBR_Msk)) != 0)
   {
      //Only clear TSR flags that are currently set
      ETH_REGS->ETH_TSR = tsr;

      //Check whether the TX buffer is available for writing
      if((txBufferDesc[txBufferIndex].status & ETH_TX_USED) != 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((rsr & (ETH_RSR_HNO_Msk | ETH_RSR_RXOVR_Msk | ETH_RSR_REC_Msk |
      ETH_RSR_BNA_Msk)) != 0)
   {
      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief PIC32CK Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void pic32ckEthEventHandler(NetInterface *interface)
{
   error_t error;
   uint32_t rsr;

   //Read receive status
   rsr = ETH_REGS->ETH_RSR;

   //Packet received?
   if((rsr & (ETH_RSR_HNO_Msk | ETH_RSR_RXOVR_Msk | ETH_RSR_REC_Msk |
      ETH_RSR_BNA_Msk)) != 0)
   {
      //Only clear RSR flags that are currently set
      ETH_REGS->ETH_RSR = rsr;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = pic32ckEthReceivePacket(interface);

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

error_t pic32ckEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > PIC32CK_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txBufferDesc[txBufferIndex].status & ETH_TX_USED) == 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txBufferIndex], buffer, offset, length);

   //Set the necessary flags in the descriptor entry
   if(txBufferIndex < (PIC32CK_ETH_TX_BUFFER_COUNT - 1))
   {
      //Write the status word
      txBufferDesc[txBufferIndex].status = ETH_TX_LAST |
         (length & ETH_TX_LENGTH);

      //Point to the next buffer
      txBufferIndex++;
   }
   else
   {
      //Write the status word
      txBufferDesc[txBufferIndex].status = ETH_TX_WRAP | ETH_TX_LAST |
         (length & ETH_TX_LENGTH);

      //Wrap around
      txBufferIndex = 0;
   }

   //Data synchronization barrier
   __DSB();

   //Set the TSTART bit to initiate transmission
   ETH_REGS->ETH_NCR |= ETH_NCR_TSTART_Msk;

   //Check whether the next buffer is available for writing
   if((txBufferDesc[txBufferIndex].status & ETH_TX_USED) != 0)
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

error_t pic32ckEthReceivePacket(NetInterface *interface)
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
   for(i = 0; i < PIC32CK_ETH_RX_BUFFER_COUNT; i++)
   {
      //Point to the current entry
      j = rxBufferIndex + i;

      //Wrap around to the beginning of the buffer if necessary
      if(j >= PIC32CK_ETH_RX_BUFFER_COUNT)
      {
         j -= PIC32CK_ETH_RX_BUFFER_COUNT;
      }

      //No more entries to process?
      if((rxBufferDesc[j].address & ETH_RX_OWNERSHIP) == 0)
      {
         //Stop processing
         break;
      }

      //A valid SOF has been found?
      if((rxBufferDesc[j].status & ETH_RX_SOF) != 0)
      {
         //Save the position of the SOF
         sofIndex = i;
      }

      //A valid EOF has been found?
      if((rxBufferDesc[j].status & ETH_RX_EOF) != 0 && sofIndex != UINT_MAX)
      {
         //Save the position of the EOF
         eofIndex = i;
         //Retrieve the length of the frame
         size = rxBufferDesc[j].status & ETH_RX_LENGTH;
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
         n = MIN(size, PIC32CK_ETH_RX_BUFFER_SIZE);
         //Copy data from receive buffer
         osMemcpy((uint8_t *) temp + length, rxBuffer[rxBufferIndex], n);
         //Update byte counters
         length += n;
         size -= n;
      }

      //Mark the current buffer as free
      rxBufferDesc[rxBufferIndex].address &= ~ETH_RX_OWNERSHIP;

      //Point to the following entry
      rxBufferIndex++;

      //Wrap around to the beginning of the buffer if necessary
      if(rxBufferIndex >= PIC32CK_ETH_RX_BUFFER_COUNT)
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

error_t pic32ckEthUpdateMacAddrFilter(NetInterface *interface)
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
   ETH_REGS->SA[0].ETH_SAB = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ETH_REGS->SA[0].ETH_SAT = interface->macAddr.w[2];

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
      //The address is activated when SAT register is written
      ETH_REGS->SA[1].ETH_SAB = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
      ETH_REGS->SA[1].ETH_SAT = unicastMacAddr[0].w[2];
   }
   else
   {
      //The address is deactivated when SAB register is written
      ETH_REGS->SA[1].ETH_SAB = 0;
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      //The address is activated when SAT register is written
      ETH_REGS->SA[2].ETH_SAB = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
      ETH_REGS->SA[2].ETH_SAT = unicastMacAddr[1].w[2];
   }
   else
   {
      //The address is deactivated when SAB register is written
      ETH_REGS->SA[2].ETH_SAB = 0;
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      //The address is activated when SAT register is written
      ETH_REGS->SA[3].ETH_SAB = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
      ETH_REGS->SA[3].ETH_SAT = unicastMacAddr[2].w[2];
   }
   else
   {
      //The address is deactivated when SAB register is written
      ETH_REGS->SA[3].ETH_SAB = 0;
   }

   //Configure the multicast hash table
   ETH_REGS->ETH_HRB = hashTable[0];
   ETH_REGS->ETH_HRT = hashTable[1];

   //Debug message
   TRACE_DEBUG("  HRB = %08" PRIX32 "\r\n", ETH_REGS->ETH_HRB);
   TRACE_DEBUG("  HRT = %08" PRIX32 "\r\n", ETH_REGS->ETH_HRT);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t pic32ckEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read network configuration register
   config = ETH_REGS->ETH_NCFGR;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= ETH_NCFGR_SPD_Msk;
   }
   else
   {
      config &= ~ETH_NCFGR_SPD_Msk;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= ETH_NCFGR_FD_Msk;
   }
   else
   {
      config &= ~ETH_NCFGR_FD_Msk;
   }

   //Write configuration value back to NCFGR register
   ETH_REGS->ETH_NCFGR = config;

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

void pic32ckEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Set up a write operation
      temp = ETH_MAN_CLTTO_Msk | ETH_MAN_OP(1) | ETH_MAN_WTN(2);
      //PHY address
      temp |= ETH_MAN_PHYA(phyAddr);
      //Register address
      temp |= ETH_MAN_REGA(regAddr);
      //Register value
      temp |= ETH_MAN_DATA(data);

      //Start a write operation
      ETH_REGS->ETH_MAN = temp;
      //Wait for the write to complete
      while((ETH_REGS->ETH_NSR & ETH_NSR_IDLE_Msk) == 0)
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

uint16_t pic32ckEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Set up a read operation
      temp = ETH_MAN_CLTTO_Msk | ETH_MAN_OP(2) | ETH_MAN_WTN(2);
      //PHY address
      temp |= ETH_MAN_PHYA(phyAddr);
      //Register address
      temp |= ETH_MAN_REGA(regAddr);

      //Start a read operation
      ETH_REGS->ETH_MAN = temp;
      //Wait for the read to complete
      while((ETH_REGS->ETH_NSR & ETH_NSR_IDLE_Msk) == 0)
      {
      }

      //Get register value
      data = ETH_REGS->ETH_MAN & ETH_MAN_DATA_Msk;
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
      data = 0;
   }

   //Return the value of the PHY register
   return data;
}
