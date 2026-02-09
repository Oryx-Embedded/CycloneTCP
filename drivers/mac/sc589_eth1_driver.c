/**
 * @file sc589_eth1_driver.c
 * @brief ADSP-SC589 Ethernet MAC driver (EMAC0 instance)
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
#include "drivers/mac/sc589_eth1_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
#pragma location = SC589_ETH1_RAM_SECTION
static uint8_t txBuffer[SC589_ETH1_TX_BUFFER_COUNT][SC589_ETH1_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
#pragma location = SC589_ETH1_RAM_SECTION
static uint8_t rxBuffer[SC589_ETH1_RX_BUFFER_COUNT][SC589_ETH1_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 8
#pragma location = SC589_ETH1_RAM_SECTION
static Sc589Eth1TxDmaDesc txDmaDesc[SC589_ETH1_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 8
#pragma location = SC589_ETH1_RAM_SECTION
static Sc589Eth1RxDmaDesc rxDmaDesc[SC589_ETH1_RX_BUFFER_COUNT];

//GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[SC589_ETH1_TX_BUFFER_COUNT][SC589_ETH1_TX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(SC589_ETH1_RAM_SECTION)));
//Receive buffer
static uint8_t rxBuffer[SC589_ETH1_RX_BUFFER_COUNT][SC589_ETH1_RX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(SC589_ETH1_RAM_SECTION)));
//Transmit DMA descriptors
static Sc589Eth1TxDmaDesc txDmaDesc[SC589_ETH1_TX_BUFFER_COUNT]
   __attribute__((aligned(8), __section__(SC589_ETH1_RAM_SECTION)));
//Receive DMA descriptors
static Sc589Eth1RxDmaDesc rxDmaDesc[SC589_ETH1_RX_BUFFER_COUNT]
   __attribute__((aligned(8), __section__(SC589_ETH1_RAM_SECTION)));

#endif

//Pointer to the current TX DMA descriptor
static Sc589Eth1TxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static Sc589Eth1RxDmaDesc *rxCurDmaDesc;


/**
 * @brief ADSP-SC589 Ethernet MAC driver (EMAC0 instance)
 **/

const NicDriver sc589Eth1Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   sc589Eth1Init,
   sc589Eth1Tick,
   sc589Eth1EnableIrq,
   sc589Eth1DisableIrq,
   sc589Eth1EventHandler,
   sc589Eth1SendPacket,
   sc589Eth1UpdateMacAddrFilter,
   sc589Eth1UpdateMacConfig,
   sc589Eth1WritePhyReg,
   sc589Eth1ReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief ADSP-SC589 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t sc589Eth1Init(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing ADSP-SC589 Ethernet MAC (EMAC0)...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   sc589Eth1InitGpio(interface);

   //Perform a software reset
   *pREG_EMAC0_DMA0_BUSMODE |= BITM_EMAC_DMA0_BUSMODE_SWR;
   //Wait for the reset to complete
   while((*pREG_EMAC0_DMA0_BUSMODE & BITM_EMAC_DMA0_BUSMODE_SWR) != 0)
   {
   }

   //Adjust MDC clock range depending on CLKO7 frequency
   *pREG_EMAC0_SMI_ADDR = ENUM_EMAC_SMI_ADDR_CR_DIV62;

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
   *pREG_EMAC0_MACCFG = BITM_EMAC_MACCFG_PS | BITM_EMAC_MACCFG_DO;

   //Configure MAC address filtering
   sc589Eth1UpdateMacAddrFilter(interface);

   //Disable flow control
   *pREG_EMAC0_FLOWCTL = 0;

   //Enable store and forward mode
   *pREG_EMAC0_DMA0_OPMODE = BITM_EMAC_DMA0_OPMODE_RSF |
      BITM_EMAC_DMA0_OPMODE_TSF;

   //Configure DMA bus mode
   *pREG_EMAC0_DMA0_BUSMODE = BITM_EMAC_DMA0_BUSMODE_AAL |
      BITM_EMAC_DMA0_BUSMODE_USP | ENUM_EMAC_DMA_BUSMODE_RPBL_32 |
      ENUM_EMAC_DMA_BUSMODE_PBL_32 | BITM_EMAC_DMA0_BUSMODE_ATDS;

   //Initialize DMA descriptor lists
   sc589Eth1InitDmaDesc(interface);

   //Prevent interrupts from being generated when statistic counters reach
   //half their maximum value
   *pREG_EMAC0_MMC_TXIMSK = 0x01FFFFFF;
   *pREG_EMAC0_MMC_RXIMSK = 0x01FFFFFF;
   *pREG_EMAC0_IPC_RXIMSK = 0x3FFFFFFF;

   //Disable MAC interrupts
   *pREG_EMAC0_IMSK = BITM_EMAC_IMSK_LPIIM | BITM_EMAC_IMSK_TS;

   //Enable the desired DMA interrupts
   *pREG_EMAC0_DMA0_IEN = BITM_EMAC_DMA0_IEN_NIE | BITM_EMAC_DMA0_IEN_RIE |
      BITM_EMAC_DMA0_IEN_TIE;

   //Register interrupt handler
   adi_int_InstallHandler(INTR_EMAC0_STAT, sc589Eth1IrqHandler, interface,
      false);

   //Enable MAC transmission and reception
   *pREG_EMAC0_MACCFG |= BITM_EMAC_MACCFG_TE | BITM_EMAC_MACCFG_RE;
   //Enable DMA transmission and reception
   *pREG_EMAC0_DMA0_OPMODE |= BITM_EMAC_DMA0_OPMODE_ST | BITM_EMAC_DMA0_OPMODE_SR;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void sc589Eth1InitGpio(NetInterface *interface)
{
//ADZS-SC589-EZLITE evaluation board?
#if defined(USE_ADZS_SC589_EZLITE)
   uint32_t temp;

   //Configure PA_00 (ETH0_TXD0), PA_01 (ETH0_TXD1), PA_02 (ETH0_MDC),
   //PA_03 (ETH0_MDIO), PA_04 (ETH0_RXD0), PA_05 (ETH0_RXD1),
   //PA_06 (ETH0_RXCLK_REFCLK), PA_07 (ETH0_CRS), PA_08 (ETH0_RXD2),
   //PA_09 (ETH0_RXD3), PA_10 (ETH0_TXEN), PA_11 (ETH0_TXCLK),
   //PA_12 (ETH0_TXD2) and PA_13 (ETH0_TXD3)
   temp = *pREG_PORTA_MUX;
   temp = (temp & ~BITM_PORT_MUX_MUX0) | (0 << BITP_PORT_MUX_MUX0);
   temp = (temp & ~BITM_PORT_MUX_MUX1) | (0 << BITP_PORT_MUX_MUX1);
   temp = (temp & ~BITM_PORT_MUX_MUX2) | (0 << BITP_PORT_MUX_MUX2);
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
   *pREG_PORTA_MUX = temp;

   //Select peripheral mode
   *pREG_PORTA_FER_SET = BITM_PORT_FER_PX0 | BITM_PORT_FER_PX1 |
      BITM_PORT_FER_PX2 | BITM_PORT_FER_PX3 | BITM_PORT_FER_PX4 |
      BITM_PORT_FER_PX5 | BITM_PORT_FER_PX6 | BITM_PORT_FER_PX7 |
      BITM_PORT_FER_PX8 | BITM_PORT_FER_PX9 | BITM_PORT_FER_PX10 |
      BITM_PORT_FER_PX11 | BITM_PORT_FER_PX12 | BITM_PORT_FER_PX13;

   //Configure ETH0_MD_INT (PC_15) as an input
   *pREG_PORTC_FER_CLR = BITM_PORT_FER_PX15;
   *pREG_PORTC_DIR_CLR = BITM_PORT_DIR_PX15;
   *pREG_PORTC_INEN_SET = BITM_PORT_INEN_PX15;

   //Configure ETH0_RESET (PB_14) as an output
   *pREG_PORTB_FER_CLR = BITM_PORT_FER_PX14;
   *pREG_PORTB_DIR_SET = BITM_PORT_DIR_PX14;

   //Reset PHY transceiver (hard reset)
   *pREG_PORTB_DATA_CLR = BITM_PORT_DATA_PX14;
   sleep(10);
   *pREG_PORTB_DATA_SET = BITM_PORT_DATA_PX14;
   sleep(10);

   //Select RGMII interface mode
   *pREG_PADS0_PCFG0 |= BITM_PADS_PCFG0_EMACPHYISEL;
   //Reset PHY interface
   *pREG_PADS0_PCFG0 |= BITM_PADS_PCFG0_EMACRESET;
#endif
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void sc589Eth1InitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < SC589_ETH1_TX_BUFFER_COUNT; i++)
   {
      //Use chain structure rather than ring structure
      txDmaDesc[i].tdes0 = EMAC_TDES0_IC | EMAC_TDES0_TCH;
      //Initialize transmit buffer size
      txDmaDesc[i].tdes1 = 0;

      //Transmit buffer address
      txDmaDesc[i].tdes2 = adi_rtl_internal_to_system_addr(
         (uint32_t) txBuffer[i], 1);

      //Next descriptor address
      txDmaDesc[i].tdes3 = adi_rtl_internal_to_system_addr(
         (uint32_t) &txDmaDesc[i + 1], 1);

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
   for(i = 0; i < SC589_ETH1_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = EMAC_RDES0_OWN;
      //Use chain structure rather than ring structure
      rxDmaDesc[i].rdes1 = EMAC_RDES1_RCH | (SC589_ETH1_RX_BUFFER_SIZE & EMAC_RDES1_RBS1);

      //Receive buffer address
      rxDmaDesc[i].rdes2 = adi_rtl_internal_to_system_addr(
         (uint32_t) rxBuffer[i], 1);

      //Next descriptor address
      rxDmaDesc[i].rdes3 = adi_rtl_internal_to_system_addr(
         (uint32_t) &rxDmaDesc[i + 1], 1);

      //Extended status
      rxDmaDesc[i].rdes4 = 0;
      //Reserved field
      rxDmaDesc[i].rdes5 = 0;
      //Receive frame time stamp
      rxDmaDesc[i].rdes6 = 0;
      rxDmaDesc[i].rdes7 = 0;
   }

   //The last descriptor is chained to the first entry
   rxDmaDesc[i - 1].rdes3 = adi_rtl_internal_to_system_addr(
      (uint32_t) &rxDmaDesc[0], 1);

   //Point to the very first descriptor
   rxCurDmaDesc = &rxDmaDesc[0];

   //Start location of the TX descriptor list
   *pREG_EMAC0_DMA0_TXDSC_ADDR = adi_rtl_internal_to_system_addr(
      (uint32_t) txDmaDesc, 1);

   //Start location of the RX descriptor list
   *pREG_EMAC0_DMA0_RXDSC_ADDR = adi_rtl_internal_to_system_addr(
      (uint32_t) rxDmaDesc, 1);
}


/**
 * @brief ADSP-SC589 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void sc589Eth1Tick(NetInterface *interface)
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

void sc589Eth1EnableIrq(NetInterface *interface)
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

void sc589Eth1DisableIrq(NetInterface *interface)
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
 * @brief ADSP-SC589 Ethernet MAC interrupt service routine
 * @param id Interrupt identifier
 * @param param Unused parameter
 **/

void sc589Eth1IrqHandler(uint32_t id, void *param)
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
   if((status & BITM_EMAC_DMA0_STAT_TI) != 0)
   {
      //Clear TI interrupt flag
      *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA0_STAT_TI;

      //Check whether the TX buffer is available for writing
      if((txCurDmaDesc->tdes0 & EMAC_TDES0_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & BITM_EMAC_DMA0_STAT_RI) != 0)
   {
      //Clear RI interrupt flag
      *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA0_STAT_RI;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&nicDriverInterface->netContext->event);
   }

   //Clear NIS interrupt flag
   *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA0_STAT_NIS;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief ADSP-SC589 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void sc589Eth1EventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = sc589Eth1ReceivePacket(interface);

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

error_t sc589Eth1SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > SC589_ETH1_TX_BUFFER_SIZE)
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
   netBufferRead((uint8_t *) adi_rtl_system_to_internal_addr(
      txCurDmaDesc->tdes2), buffer, offset, length);

   //Write the number of bytes to send
   txCurDmaDesc->tdes1 = length & EMAC_TDES1_TBS1;
   //Set LS and FS flags as the data fits in a single buffer
   txCurDmaDesc->tdes0 |= EMAC_TDES0_LS | EMAC_TDES0_FS;
   //Give the ownership of the descriptor to the DMA
   txCurDmaDesc->tdes0 |= EMAC_TDES0_OWN;

   //Data synchronization barrier
   __asm("dsb");

   //Clear TU flag to resume processing
   *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA0_STAT_TU;
   //Instruct the DMA to poll the transmit descriptor list
   *pREG_EMAC0_DMA0_TXPOLL = 0;

   //Point to the next descriptor in the list
   txCurDmaDesc = (Sc589Eth1TxDmaDesc *) adi_rtl_system_to_internal_addr(
      txCurDmaDesc->tdes3);

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

error_t sc589Eth1ReceivePacket(NetInterface *interface)
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
            n = MIN(n, SC589_ETH1_RX_BUFFER_SIZE);

            //Additional options can be passed to the stack along with the packet
            ancillary = NET_DEFAULT_RX_ANCILLARY;

            //Pass the packet to the upper layer
            nicProcessPacket(interface, (uint8_t *) adi_rtl_system_to_internal_addr(
               rxCurDmaDesc->rdes2), n, &ancillary);

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
      rxCurDmaDesc = (Sc589Eth1RxDmaDesc *) adi_rtl_system_to_internal_addr(
         rxCurDmaDesc->rdes3);
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RU flag to resume processing
   *pREG_EMAC0_DMA0_STAT = BITM_EMAC_DMA0_STAT_RU;
   //Instruct the DMA to poll the receive descriptor list
   *pREG_EMAC0_DMA0_RXPOLL = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t sc589Eth1UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   uint_t k;
   uint32_t crc;
   uint32_t hashTable[2];
   MacAddr unicastMacAddr[1];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Promiscuous mode?
   if(interface->promiscuous)
   {
      //Pass all incoming frames regardless of their destination address
      *pREG_EMAC0_MACFRMFILT = BITM_EMAC_MACFRMFILT_PR;
   }
   else
   {
      //Set the MAC address of the station
      *pREG_EMAC0_ADDR0_LO = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
      *pREG_EMAC0_ADDR0_HI = interface->macAddr.w[2];

      //The MAC supports one additional address for unicast perfect filtering
      unicastMacAddr[0] = MAC_UNSPECIFIED_ADDR;

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
               crc = sc589Eth1CalcCrc(&entry->addr, sizeof(MacAddr));

               //The upper 6 bits in the CRC register are used to index the
               //contents of the hash table
               k = (crc >> 26) & 0x3F;

               //Update hash table contents
               hashTable[k / 32] |= (1 << (k % 32));
            }
            else
            {
               //One additional MAC address can be specified
               if(j < 1)
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
         *pREG_EMAC0_ADDR1_HI = unicastMacAddr[0].w[2] | BITM_EMAC_ADDR1_HI_AE;
      }
      else
      {
         //When the AE bit is cleared, the entry is ignored
         *pREG_EMAC0_ADDR1_LO = 0;
         *pREG_EMAC0_ADDR1_HI = 0;
      }

      //Check whether frames with a multicast destination address should be
      //accepted
      if(interface->acceptAllMulticast)
      {
         //Configure the receive filter
         *pREG_EMAC0_MACFRMFILT = BITM_EMAC_MACFRMFILT_HPF | BITM_EMAC_MACFRMFILT_PM;
      }
      else
      {
         //Configure the receive filter
         *pREG_EMAC0_MACFRMFILT = BITM_EMAC_MACFRMFILT_HPF | BITM_EMAC_MACFRMFILT_HMC;

         //Configure the multicast hash table
         *pREG_EMAC0_HASHTBL_LO = hashTable[0];
         *pREG_EMAC0_HASHTBL_HI = hashTable[1];

         //Debug message
         TRACE_DEBUG("  EMAC_HASHTBL_LO = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_LO);
         TRACE_DEBUG("  EMAC_HASHTBL_HI = 0x%08" PRIX32 "\r\n", *pREG_EMAC0_HASHTBL_HI);
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

error_t sc589Eth1UpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = *pREG_EMAC0_MACCFG;

   //1000BASE-T operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_1GBPS)
   {
      config &= ~BITM_EMAC_MACCFG_PS;
      config &= ~BITM_EMAC_MACCFG_FES;
   }
   //100BASE-TX operation mode?
   else if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= BITM_EMAC_MACCFG_PS;
      config |= BITM_EMAC_MACCFG_FES;
   }
   //10BASE-T operation mode?
   else
   {
      config |= BITM_EMAC_MACCFG_PS;
      config &= ~BITM_EMAC_MACCFG_FES;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= BITM_EMAC_MACCFG_DM;
   }
   else
   {
      config &= ~BITM_EMAC_MACCFG_DM;
   }

   //Update MAC configuration register
   *pREG_EMAC0_MACCFG = config;

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

void sc589Eth1WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = *pREG_EMAC0_SMI_ADDR & BITM_EMAC_SMI_ADDR_CR;
      //Set up a write operation
      temp |= BITM_EMAC_SMI_ADDR_SMIW | BITM_EMAC_SMI_ADDR_SMIB;
      //PHY address
      temp |= (phyAddr << BITP_EMAC_SMI_ADDR_PA) & BITM_EMAC_SMI_ADDR_PA;
      //Register address
      temp |= (regAddr << BITP_EMAC_SMI_ADDR_SMIR) & BITM_EMAC_SMI_ADDR_SMIR;

      //Data to be written in the PHY register
      *pREG_EMAC0_SMI_DATA = data & BITM_EMAC_SMI_DATA_SMID;

      //Start a write operation
      *pREG_EMAC0_SMI_ADDR = temp;
      //Wait for the write to complete
      while((*pREG_EMAC0_SMI_ADDR & BITM_EMAC_SMI_ADDR_SMIB) != 0)
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

uint16_t sc589Eth1ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = *pREG_EMAC0_SMI_ADDR & BITM_EMAC_SMI_ADDR_CR;
      //Set up a read operation
      temp |= BITM_EMAC_SMI_ADDR_SMIB;
      //PHY address
      temp |= (phyAddr << BITP_EMAC_SMI_ADDR_PA) & BITM_EMAC_SMI_ADDR_PA;
      //Register address
      temp |= (regAddr << BITP_EMAC_SMI_ADDR_SMIR) & BITM_EMAC_SMI_ADDR_SMIR;

      //Start a read operation
      *pREG_EMAC0_SMI_ADDR = temp;
      //Wait for the read to complete
      while((*pREG_EMAC0_SMI_ADDR & BITM_EMAC_SMI_ADDR_SMIB) != 0)
      {
      }

      //Get register value
      data = *pREG_EMAC0_SMI_DATA & BITM_EMAC_SMI_DATA_SMID;
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

uint32_t sc589Eth1CalcCrc(const void *data, size_t length)
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
