/**
 * @file sc598_eth1_driver.c
 * @brief ADSP-SC598 Ethernet MAC driver (EMAC0 instance)
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
#include <sys/platform.h>
#include <services/int/adi_int.h>
#include "core/net.h"
#include "drivers/mac/sc598_eth1_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
#pragma location = SC598_ETH1_RAM_SECTION
static uint8_t txBuffer[SC598_ETH1_TX_BUFFER_COUNT][SC598_ETH1_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
#pragma location = SC598_ETH1_RAM_SECTION
static uint8_t rxBuffer[SC598_ETH1_RX_BUFFER_COUNT][SC598_ETH1_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 8
#pragma location = SC598_ETH1_RAM_SECTION
static Sc598Eth1TxDmaDesc txDmaDesc[SC598_ETH1_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 8
#pragma location = SC598_ETH1_RAM_SECTION
static Sc598Eth1RxDmaDesc rxDmaDesc[SC598_ETH1_RX_BUFFER_COUNT];

//GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[SC598_ETH1_TX_BUFFER_COUNT][SC598_ETH1_TX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(SC598_ETH1_RAM_SECTION)));
//Receive buffer
static uint8_t rxBuffer[SC598_ETH1_RX_BUFFER_COUNT][SC598_ETH1_RX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(SC598_ETH1_RAM_SECTION)));
//Transmit DMA descriptors
static Sc598Eth1TxDmaDesc txDmaDesc[SC598_ETH1_TX_BUFFER_COUNT]
   __attribute__((aligned(8), __section__(SC598_ETH1_RAM_SECTION)));
//Receive DMA descriptors
static Sc598Eth1RxDmaDesc rxDmaDesc[SC598_ETH1_RX_BUFFER_COUNT]
   __attribute__((aligned(8), __section__(SC598_ETH1_RAM_SECTION)));

#endif

//Current transmit descriptor
static uint_t txIndex;
//Current receive descriptor
static uint_t rxIndex;


/**
 * @brief ADSP-SC598 Ethernet MAC driver (EMAC0 instance)
 **/

const NicDriver sc598Eth1Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   sc598Eth1Init,
   sc598Eth1Tick,
   sc598Eth1EnableIrq,
   sc598Eth1DisableIrq,
   sc598Eth1EventHandler,
   sc598Eth1SendPacket,
   sc598Eth1UpdateMacAddrFilter,
   sc598Eth1UpdateMacConfig,
   sc598Eth1WritePhyReg,
   sc598Eth1ReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief ADSP-SC598 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t sc598Eth1Init(NetInterface *interface)
{
   error_t error;
   uint32_t temp;

   //Debug message
   TRACE_INFO("Initializing ADSP-SC598 Ethernet MAC (EMAC0)...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   sc598Eth1InitGpio(interface);

   //Set DMA transfer format (little-endian)
   *pREG_PADS0_PCFG0 &= ~BITM_PADS_PCFG0_EMAC0_ENDIANNESS;

   //Perform a software reset
   *pREG_EMAC0_DMA_MODE |= BITM_EMAC_DMA_MODE_SWR;
   //Wait for the reset to complete
   while((*pREG_EMAC0_DMA_MODE & BITM_EMAC_DMA_MODE_SWR) != 0)
   {
   }

   //Adjust MDC clock range depending on SCLK0 frequency
   *pREG_EMAC0_MDIO_ADDR = (4 << BITP_EMAC_MDIO_ADDR_CR);

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
   *pREG_EMAC0_MAC_CFG = BITM_EMAC_MAC_CFG_GPSLCE | BITM_EMAC_MAC_CFG_PS |
      BITM_EMAC_MAC_CFG_DO;

   //Set the maximum packet size that can be accepted
   temp = *pREG_EMAC0_MAC_EXT_CFG & ~BITM_EMAC_MAC_EXT_CFG_GPSL;
   *pREG_EMAC0_MAC_EXT_CFG = temp | SC598_ETH1_RX_BUFFER_SIZE;

   //Configure MAC address filtering
   sc598Eth1UpdateMacAddrFilter(interface);

   //Disable flow control
   *pREG_EMAC0_Q0_TXFLOW_CTL = 0;
   *pREG_EMAC0_RXFLOW_CTL = 0;

   //Enable the first RX queue
   *pREG_EMAC0_RXQ_CTL0 = ENUM_EMAC_RXQ_CTL0_RXQ0EN_EN_DCB_GEN;

   //Configure DMA operating mode
   *pREG_EMAC0_DMA_MODE = ENUM_EMAC_DMA_MODE_MODE0 |
      ENUM_EMAC_DMA_MODE_DSPW_DISABLE;

   //Configure system bus mode
   *pREG_EMAC0_DMA_SYSBMODE |= BITM_EMAC_DMA_SYSBMODE_AAL;

   //The DMA takes the descriptor table as contiguous
   *pREG_EMAC0_DMA0_CTL = (0 << BITP_EMAC_DMA_CTL_DSL);
   //Configure TX features
   *pREG_EMAC0_DMA0_TXCTL = (32 << BITP_EMAC_DMA_TXCTL_TXPBL);

   //Configure RX features
   *pREG_EMAC0_DMA0_RXCTL = (32 << BITP_EMAC_DMA_RXCTL_RXPBL) |
      ((SC598_ETH1_RX_BUFFER_SIZE / 4) << BITP_EMAC_DMA_RXCTL_RBSZ_13_Y);

   //Enable store and forward mode for transmission
   *pREG_EMAC0_TQ0_OPMODE |= (7 << BITP_EMAC_TQ_OPMODE_TQS) |
      ENUM_EMAC_TQ_OPMODE_TXQEN_ENABLE | BITM_EMAC_TQ_OPMODE_TSF;

   //Enable store and forward mode for reception
   *pREG_EMAC0_RQ0_OPMODE |= (7 << BITP_EMAC_RQ_OPMODE_RQS) |
      BITM_EMAC_RQ_OPMODE_RSF;

   //Initialize DMA descriptor lists
   sc598Eth1InitDmaDesc(interface);

   //Prevent interrupts from being generated when statistic counters reach
   //half their maximum value
   *pREG_EMAC0_MMC_TXIMSK = 0x0FFFFFFF;
   *pREG_EMAC0_MMC_RXIMSK = 0x0FFFFFFF;
   *pREG_EMAC0_MMC_IPC_RXIMSK = 0x3FFFFFFF;
   *pREG_EMAC0_MMC_FPE_TXIMSK = 0x00000003;
   *pREG_EMAC0_MMC_FPE_RXIMSK = 0x0000000F;

   //Disable MAC interrupts
   *pREG_EMAC0_MAC_IEN = 0;

   //Enable the desired DMA interrupts
   *pREG_EMAC0_DMA0_IEN = BITM_EMAC_DMA_IEN_NIE | BITM_EMAC_DMA_IEN_RIE |
      BITM_EMAC_DMA_IEN_TIE;

   //Register interrupt handler
   adi_int_InstallHandler(INTR_EMAC0_STAT, sc598Eth1IrqHandler, interface,
      false);

   //Enable MAC transmission and reception
   *pREG_EMAC0_MAC_CFG |= BITM_EMAC_MAC_CFG_TE | BITM_EMAC_MAC_CFG_RE;

   //Enable DMA transmission and reception
   *pREG_EMAC0_DMA0_TXCTL |= BITM_EMAC_DMA_TXCTL_ST;
   *pREG_EMAC0_DMA0_RXCTL |= BITM_EMAC_DMA_RXCTL_SR;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void sc598Eth1InitGpio(NetInterface *interface)
{
//EV-SC598-SOM evaluation board?
#if defined(USE_EV_SC598_SOM)
   uint32_t temp;

   //Configure PH_03 (ETH0_MDC), PH_04 (ETH0_MDIO), PH_05 (ETH0_RXD0),
   //PH_06 (ETH0_RXD1), PH_07 (ETH0_RXCLK_REFCLK), PH_08 (ETH0_RXCTL_RXDV),
   //PH_09 (ETH0_TXD0), PH_10 (ETH0_TXD1), PH_11 (ETH0_RXD2), PH_12 (ETH0_RXD3),
   //PH_13 (ETH0_TXCTL_TXEN), PH_14 (ETH0_TXCLK) and PH_15 (ETH0_TXD2)
   temp = *pREG_PORTH_MUX;
   temp = (temp & ~BITM_PORT_MUX_MUX3) | (0 << BITP_PORT_MUX_MUX3);
   temp = (temp & ~BITM_PORT_MUX_MUX4) | (0 << BITP_PORT_MUX_MUX4);
   temp = (temp & ~BITM_PORT_MUX_MUX5) | (0 << BITP_PORT_MUX_MUX5);
   temp = (temp & ~BITM_PORT_MUX_MUX6) | (0 << BITP_PORT_MUX_MUX6);
   temp = (temp & ~BITM_PORT_MUX_MUX7) | (0 << BITP_PORT_MUX_MUX7);
   temp = (temp & ~BITM_PORT_MUX_MUX8) | (0 << BITP_PORT_MUX_MUX8);
   temp = (temp & ~BITM_PORT_MUX_MUX9) | (0 << BITP_PORT_MUX_MUX9);
   temp = (temp & ~BITM_PORT_MUX_MUX10) | (0 << BITP_PORT_MUX_MUX10);
   temp = (temp & ~BITM_PORT_MUX_MUX11) | (0 << BITP_PORT_MUX_MUX11);
   temp = (temp & ~BITM_PORT_MUX_MUX12) | (0 << BITP_PORT_MUX_MUX12);
   temp = (temp & ~BITM_PORT_MUX_MUX13) | (0 << BITP_PORT_MUX_MUX13);
   temp = (temp & ~BITM_PORT_MUX_MUX14) | (0 << BITP_PORT_MUX_MUX14);
   temp = (temp & ~BITM_PORT_MUX_MUX15) | (0 << BITP_PORT_MUX_MUX15);
   *pREG_PORTH_MUX = temp;

   //Select peripheral mode
   *pREG_PORTH_FER_SET = BITM_PORT_FER_PX3 | BITM_PORT_FER_PX4 |
      BITM_PORT_FER_PX5 | BITM_PORT_FER_PX6 | BITM_PORT_FER_PX7 |
      BITM_PORT_FER_PX8 | BITM_PORT_FER_PX9 | BITM_PORT_FER_PX10 |
      BITM_PORT_FER_PX11 | BITM_PORT_FER_PX12 | BITM_PORT_FER_PX13 |
      BITM_PORT_FER_PX14 | BITM_PORT_FER_PX15;

   //Configure PI_00 (ETH0_TXD3)
   temp = *pREG_PORTI_MUX;
   temp = (temp & ~BITM_PORT_MUX_MUX0) | (0 << BITP_PORT_MUX_MUX0);
   *pREG_PORTI_MUX = temp;

   //Select peripheral mode
   *pREG_PORTI_FER_SET = BITM_PORT_FER_PX0;

   //Reset PHY transceiver (hard reset)
   sc598Eth1ResetPhy(interface);

   //Select RGMII interface mode
   temp = *pREG_PADS0_PCFG0 & ~BITM_PADS_PCFG0_EMACPHYISEL;
   *pREG_PADS0_PCFG0 = temp | ENUM_PADS_PCFG0_EMACPHY_RGMII;

   //Reset PHY interface
   *pREG_PADS0_PCFG0 |= BITM_PADS_PCFG0_EMACRESET;
#endif
}


/**
 * @brief Reset PHY transceiver
 * @param[in] interface Underlying network interface
 **/

__weak_func void sc598Eth1ResetPhy(NetInterface *interface)
{
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void sc598Eth1InitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < SC598_ETH1_TX_BUFFER_COUNT; i++)
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
   for(i = 0; i < SC598_ETH1_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = adi_rtl_internal_to_system_addr(
         (uint32_t)(uintptr_t) rxBuffer[i], 1);

      rxDmaDesc[i].rdes1 = 0;
      rxDmaDesc[i].rdes2 = 0;
      rxDmaDesc[i].rdes3 = EMAC_RDES3_OWN | EMAC_RDES3_IOC | EMAC_RDES3_BUF1V;
   }

   //Initialize RX descriptor index
   rxIndex = 0;

   //Start location of the TX descriptor list
   *pREG_EMAC0_DMA0_TXDSC_ADDR = adi_rtl_internal_to_system_addr(
      (uint32_t)(uintptr_t) &txDmaDesc[0], 1);

   //Length of the transmit descriptor ring
   *pREG_EMAC0_DMA0_TXDSC_RLEN = SC598_ETH1_TX_BUFFER_COUNT - 1;

   //Start location of the RX descriptor list
   *pREG_EMAC0_DMA0_RXDSC_ADDR = adi_rtl_internal_to_system_addr(
      (uint32_t)(uintptr_t) &rxDmaDesc[0], 1);

   //Length of the receive descriptor ring
   *pREG_EMAC0_DMA0_RXCTL2 = SC598_ETH1_RX_BUFFER_COUNT - 1;
}


/**
 * @brief ADSP-SC598 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void sc598Eth1Tick(NetInterface *interface)
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

void sc598Eth1EnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   adi_int_EnableInt(INTR_EMAC0_STAT, true);

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

void sc598Eth1DisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   adi_int_EnableInt(INTR_EMAC0_STAT, false);

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
 * @brief ADSP-SC598 Ethernet MAC interrupt service routine
 * @param id Interrupt identifier
 * @param param Unused parameter
 **/

void sc598Eth1IrqHandler(uint32_t id, void *param)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = *pREG_EMAC0_DMA0_STAT;

   //Packet transmitted?
   if((status & BITM_EMAC_DMA_STAT_TI) != 0)
   {
      //Clear TI interrupt flag
      *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA_STAT_TI;

      //Check whether the TX buffer is available for writing
      if((txDmaDesc[txIndex].tdes3 & EMAC_TDES3_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & BITM_EMAC_DMA_STAT_RI) != 0)
   {
      //Clear RI interrupt flag
      *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA_STAT_RI;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&nicDriverInterface->netContext->event);
   }

   //Clear NIS interrupt flag
   *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA_STAT_NIS;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief ADSP-SC598 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void sc598Eth1EventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = sc598Eth1ReceivePacket(interface);

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

error_t sc598Eth1SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > SC598_ETH1_TX_BUFFER_SIZE)
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
   txDmaDesc[txIndex].tdes0 = adi_rtl_internal_to_system_addr(
      (uint32_t)(uintptr_t) txBuffer[txIndex], 1);

   //Write the number of bytes to send
   txDmaDesc[txIndex].tdes2 = EMAC_TDES2_IOC | (length & EMAC_TDES2_HL_B1L);
   //Give the ownership of the descriptor to the DMA
   txDmaDesc[txIndex].tdes3 = EMAC_TDES3_OWN | EMAC_TDES3_FD | EMAC_TDES3_LD;

   //Data synchronization barrier
   __asm("dsb sy");

   //Clear TBU flag to resume processing
   *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA_STAT_TBU;
   //Instruct the DMA to poll the transmit descriptor list
   *pREG_EMAC0_DMA0_TXDSC_TLPTR = 0;

   //Increment index and wrap around if necessary
   if(++txIndex >= SC598_ETH1_TX_BUFFER_COUNT)
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

error_t sc598Eth1ReceivePacket(NetInterface *interface)
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
            n = MIN(n, SC598_ETH1_RX_BUFFER_SIZE);

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
      rxDmaDesc[rxIndex].rdes0 = adi_rtl_internal_to_system_addr(
         (uint32_t)(uintptr_t) rxBuffer[rxIndex], 1);

      //Give the ownership of the descriptor back to the DMA
      rxDmaDesc[rxIndex].rdes3 = EMAC_RDES3_OWN | EMAC_RDES3_IOC | EMAC_RDES3_BUF1V;

      //Increment index and wrap around if necessary
      if(++rxIndex >= SC598_ETH1_RX_BUFFER_COUNT)
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
   *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA_STAT_RBU;
   //Instruct the DMA to poll the receive descriptor list
   *pREG_EMAC0_DMA0_RXDSC_TLPTR = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t sc598Eth1UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   uint_t k;
   uint32_t crc;
   uint32_t hashTable[8];
   MacAddr unicastMacAddr[3];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Promiscuous mode?
   if(interface->promiscuous)
   {
      //Pass all incoming frames regardless of their destination address
      *pREG_EMAC0_MACPKT_FILT = BITM_EMAC_MACPKT_FILT_PR;
   }
   else
   {
      //Set the MAC address of the station
      *pREG_EMAC0_ADDR0_LO = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
      *pREG_EMAC0_ADDR0_HI = interface->macAddr.w[2];

      //The MAC supports 3 additional addresses for unicast perfect filtering
      unicastMacAddr[0] = MAC_UNSPECIFIED_ADDR;
      unicastMacAddr[1] = MAC_UNSPECIFIED_ADDR;
      unicastMacAddr[2] = MAC_UNSPECIFIED_ADDR;

      //The hash table is used for multicast address filtering
      hashTable[0] = 0;
      hashTable[1] = 0;
      hashTable[2] = 0;
      hashTable[3] = 0;
      hashTable[4] = 0;
      hashTable[5] = 0;
      hashTable[6] = 0;
      hashTable[7] = 0;

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
               crc = sc598Eth1CalcCrc(&entry->addr, sizeof(MacAddr));

               //The upper 8 bits in the CRC register are used to index the
               //contents of the hash table
               k = (crc >> 24) & 0xFF;

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
         *pREG_EMAC0_ADDR1_LO = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
         *pREG_EMAC0_ADDR1_HI = unicastMacAddr[0].w[2] | BITM_EMAC_ADDR_HI_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         *pREG_EMAC0_ADDR1_LO = 0;
         *pREG_EMAC0_ADDR1_HI = 0;
      }

      //Configure the second unicast address filter
      if(j >= 2)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         *pREG_EMAC0_ADDR2_LO = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
         *pREG_EMAC0_ADDR2_HI = unicastMacAddr[1].w[2] | BITM_EMAC_ADDR_HI_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         *pREG_EMAC0_ADDR2_LO = 0;
         *pREG_EMAC0_ADDR2_HI = 0;
      }

      //Configure the third unicast address filter
      if(j >= 3)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         *pREG_EMAC0_ADDR3_LO = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
         *pREG_EMAC0_ADDR3_HI = unicastMacAddr[2].w[2] | BITM_EMAC_ADDR_HI_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         *pREG_EMAC0_ADDR3_LO = 0;
         *pREG_EMAC0_ADDR3_HI = 0;
      }

      //Check whether frames with a multicast destination address should be
      //accepted
      if(interface->acceptAllMulticast)
      {
         //Configure the receive filter
         *pREG_EMAC0_MACPKT_FILT = BITM_EMAC_MACPKT_FILT_HPF | BITM_EMAC_MACPKT_FILT_PM;
      }
      else
      {
         //Configure the receive filter
         *pREG_EMAC0_MACPKT_FILT = BITM_EMAC_MACPKT_FILT_HPF | BITM_EMAC_MACPKT_FILT_HMC;

         //Configure the multicast hash table
         *pREG_EMAC0_HASHTBL_REG0 = hashTable[0];
         *pREG_EMAC0_HASHTBL_REG1 = hashTable[1];
         *pREG_EMAC0_HASHTBL_REG2 = hashTable[2];
         *pREG_EMAC0_HASHTBL_REG3 = hashTable[3];
         *pREG_EMAC0_HASHTBL_REG4 = hashTable[4];
         *pREG_EMAC0_HASHTBL_REG5 = hashTable[5];
         *pREG_EMAC0_HASHTBL_REG6 = hashTable[6];
         *pREG_EMAC0_HASHTBL_REG7 = hashTable[7];

         //Debug message
         TRACE_DEBUG("  EMAC_HASHTBL_REG0 = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_REG0);
         TRACE_DEBUG("  EMAC_HASHTBL_REG1 = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_REG1);
         TRACE_DEBUG("  EMAC_HASHTBL_REG2 = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_REG2);
         TRACE_DEBUG("  EMAC_HASHTBL_REG3 = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_REG3);
         TRACE_DEBUG("  EMAC_HASHTBL_REG4 = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_REG4);
         TRACE_DEBUG("  EMAC_HASHTBL_REG5 = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_REG5);
         TRACE_DEBUG("  EMAC_HASHTBL_REG6 = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_REG6);
         TRACE_DEBUG("  EMAC_HASHTBL_REG7 = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_REG7);
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

error_t sc598Eth1UpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = *pREG_EMAC0_MAC_CFG;

   //1000BASE-T operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_1GBPS)
   {
      config &= ~BITM_EMAC_MAC_CFG_PS;
      config &= ~BITM_EMAC_MAC_CFG_FES;
   }
   //100BASE-TX operation mode?
   else if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= BITM_EMAC_MAC_CFG_PS;
      config |= BITM_EMAC_MAC_CFG_FES;
   }
   //10BASE-T operation mode?
   else
   {
      config |= BITM_EMAC_MAC_CFG_PS;
      config &= ~BITM_EMAC_MAC_CFG_FES;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= BITM_EMAC_MAC_CFG_DM;
   }
   else
   {
      config &= ~BITM_EMAC_MAC_CFG_DM;
   }

   //Update MAC configuration register
   *pREG_EMAC0_MAC_CFG = config;

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

void sc598Eth1WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = *pREG_EMAC0_MDIO_ADDR & BITM_EMAC_MDIO_ADDR_CR;
      //Set up a write operation
      temp |= BITM_EMAC_MDIO_ADDR_GOC_0 | BITM_EMAC_MDIO_ADDR_GB;
      //PHY address
      temp |= (phyAddr << BITP_EMAC_MDIO_ADDR_PA) & BITM_EMAC_MDIO_ADDR_PA;
      //Register address
      temp |= (regAddr << BITP_EMAC_MDIO_ADDR_RDA) & BITM_EMAC_MDIO_ADDR_RDA;

      //Data to be written in the PHY register
      *pREG_EMAC0_MDIO_DATA = data & BITM_EMAC_MDIO_DATA_GD;

      //Start a write operation
      *pREG_EMAC0_MDIO_ADDR = temp;
      //Wait for the write to complete
      while((*pREG_EMAC0_MDIO_ADDR & BITM_EMAC_MDIO_ADDR_GB) != 0)
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

uint16_t sc598Eth1ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = *pREG_EMAC0_MDIO_ADDR & BITM_EMAC_MDIO_ADDR_CR;

      //Set up a read operation
      temp |= BITM_EMAC_MDIO_ADDR_GOC_1 | BITM_EMAC_MDIO_ADDR_GOC_0 |
         BITM_EMAC_MDIO_ADDR_GB;

      //PHY address
      temp |= (phyAddr << BITP_EMAC_MDIO_ADDR_PA) & BITM_EMAC_MDIO_ADDR_PA;
      //Register address
      temp |= (regAddr << BITP_EMAC_MDIO_ADDR_RDA) & BITM_EMAC_MDIO_ADDR_RDA;

      //Start a read operation
      *pREG_EMAC0_MDIO_ADDR = temp;
      //Wait for the read to complete
      while((*pREG_EMAC0_MDIO_ADDR & BITM_EMAC_MDIO_ADDR_GB) != 0)
      {
      }

      //Get register value
      data = *pREG_EMAC0_MDIO_DATA & BITM_EMAC_MDIO_DATA_GD;
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

uint32_t sc598Eth1CalcCrc(const void *data, size_t length)
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
