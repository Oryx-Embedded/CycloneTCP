/**
 * @file esp32_eth_driver.c
 * @brief ESP32 Ethernet MAC driver
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
#include "esp_intr_alloc.h"
#include "soc/dport_reg.h"
#include "soc/dport_access.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_sig_map.h"
#include "driver/gpio.h"
#include "rom/gpio.h"
#include "esp_private/periph_ctrl.h"
#include "core/net.h"
#include "drivers/mac/esp32_eth_driver.h"
#include "debug.h"

//Transmit buffer
static uint8_t txBuffer[ESP32_ETH_TX_BUFFER_COUNT][ESP32_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(1024)));
//Receive buffer
static uint8_t rxBuffer[ESP32_ETH_RX_BUFFER_COUNT][ESP32_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(1024)));
//Transmit DMA descriptors
static Esp32EthTxDmaDesc txDmaDesc[ESP32_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(1024)));
//Receive DMA descriptors
static Esp32EthRxDmaDesc rxDmaDesc[ESP32_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(1024)));

//Pointer to the current TX DMA descriptor
static Esp32EthTxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static Esp32EthRxDmaDesc *rxCurDmaDesc;


/**
 * @brief ESP32 Ethernet MAC driver
 **/

const NicDriver esp32EthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   esp32EthInit,
   esp32EthTick,
   esp32EthEnableIrq,
   esp32EthDisableIrq,
   esp32EthEventHandler,
   esp32EthSendPacket,
   esp32EthUpdateMacAddrFilter,
   esp32EthUpdateMacConfig,
   esp32EthWritePhyReg,
   esp32EthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief ESP32 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t esp32EthInit(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing ESP32 Ethernet MAC...\r\n");

   //Enable EMAC module
   periph_module_enable(PERIPH_EMAC_MODULE);

   //Enable Ethernet MAC clock
   DPORT_SET_PERI_REG_MASK(DPORT_WIFI_CLK_EN_REG, DPORT_WIFI_CLK_EMAC_EN);

   //GPIO configuration
   esp32EthInitGpio(interface);

   //Perform a software reset
   EMAC_DMABUSMODE_REG |= EMAC_DMABUSMODE_SW_RST;
   //Wait for the reset to complete
   while((EMAC_DMABUSMODE_REG & EMAC_DMABUSMODE_SW_RST) != 0)
   {
   }

   //Adjust MDC clock range
   EMAC_MIIADDR_REG = EMAC_MIIADDR_MIICSRCLK_DIV_62;

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
   EMAC_CONFIG_REG = EMAC_CONFIG_EMACMII | EMAC_CONFIG_EMACRXOWN;

   //Set the MAC address of the station
   EMAC_ADDR0HIGH_REG = interface->macAddr.w[2];
   EMAC_ADDR0LOW_REG = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);

   //The MAC supports 3 additional addresses for unicast perfect filtering
   EMAC_ADDR1HIGH_REG = 0;
   EMAC_ADDR1LOW_REG = 0;
   EMAC_ADDR2HIGH_REG = 0;
   EMAC_ADDR2LOW_REG = 0;
   EMAC_ADDR3HIGH_REG = 0;
   EMAC_ADDR3LOW_REG = 0;

   //Configure the receive filter
   EMAC_FF_REG = 0;
   //Disable flow control
   EMAC_FC_REG = 0;

   //Enable store and forward mode
   EMAC_DMAOPERATION_MODE_REG = EMAC_DMAOPERATION_MODE_RX_STORE_FORWARD |
      EMAC_DMAOPERATION_MODE_TX_STORE_FORWARD;

   //Configure DMA bus mode
   EMAC_DMABUSMODE_REG = EMAC_DMABUSMODE_RX_DMA_PBL_32 |
      EMAC_DMABUSMODE_PROG_BURST_LEN_32 | EMAC_DMABUSMODE_ALT_DESC_SIZE;

   //Initialize DMA descriptor lists
   esp32EthInitDmaDesc(interface);

   //Disable MAC interrupts
   EMAC_INTMASK_REG |= EMAC_INTMASK_LPIINTMASK | EMAC_INTMASK_PMTINTMASK;

   //Enable the desired DMA interrupts
   EMAC_DMAIN_EN_REG = EMAC_DMAIN_EN_DMAIN_NISE | EMAC_DMAIN_EN_DMAIN_RIE |
      EMAC_DMAIN_EN_DMAIN_TIE;

   //Register interrupt handler
   esp_intr_alloc(ETS_ETH_MAC_INTR_SOURCE, ESP32_ETH_IRQ_FLAGS,
      esp32EthIrqHandler, interface, NULL);

   //Enable MAC transmission and reception
   EMAC_CONFIG_REG |= EMAC_CONFIG_EMACTX | EMAC_CONFIG_EMACRX;

   //Enable DMA transmission and reception
   EMAC_DMAOPERATION_MODE_REG |= EMAC_DMAOPERATION_MODE_START_STOP_TX |
      EMAC_DMAOPERATION_MODE_START_STOP_RX;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void esp32EthInitGpio(NetInterface *interface)
{
//ESP32-Ethernet-Kit?
#if defined(ESP32_ETHERNET_KIT)
   //Select RMII interface mode
   EMAC_EX_PHYINF_CONF_REG = EMAC_EX_PHYINF_CONF_EMAC_PHY_INTF_SEL_RMII;

   //Select clock source
   EMAC_EX_CLK_CTRL_REG |= EMAC_EX_CLK_CTRL_EMAC_EXT_OSC_EN;
   //Enable clock
   EMAC_EX_OSCCLK_CONF_REG |= EMAC_EX_OSCCLK_CONF_EMAC_OSC_CLK_SEL;

   //Configure RMII CLK (GPIO0)
   gpio_set_direction(0, GPIO_MODE_INPUT);

   //Configure TXD0 (GPIO19)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO19_U, FUNC_GPIO19_EMAC_TXD0);
   //Configure TX_EN (GPIO21)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO21_U, FUNC_GPIO21_EMAC_TX_EN);
   //Configure TXD1 (GPIO22)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO22_U, FUNC_GPIO22_EMAC_TXD1);

   //Configure RXD0 (GPIO25)
   gpio_set_direction(25, GPIO_MODE_INPUT);
   //Configure RXD1 (GPIO26)
   gpio_set_direction(26, GPIO_MODE_INPUT);
   //Configure CRS_DRV (GPIO27)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO27_U, FUNC_GPIO27_EMAC_RX_DV);

   //Configure MDC (GPIO23)
   gpio_matrix_out(23, EMAC_MDC_O_IDX, 0, 0);

   //Configure MDIO (GPIO18)
   gpio_matrix_out(18, EMAC_MDO_O_IDX, 0, 0);
   gpio_matrix_in(18, EMAC_MDI_I_IDX, 0);

   //Configure PHY_RST (GPIO5)
   gpio_pad_select_gpio(5);
   gpio_set_direction(5, GPIO_MODE_OUTPUT);

   //Reset PHY transceiver
   gpio_set_level(5, 0);
   sleep(10);
   gpio_set_level(5, 1);
   sleep(10);

//EVK-NINA-W10 evaluation board?
#elif defined(EVK_NINA_W10)
   //Select RMII interface mode
   EMAC_EX_PHYINF_CONF_REG = EMAC_EX_PHYINF_CONF_EMAC_PHY_INTF_SEL_RMII;

   //Select clock source
   EMAC_EX_CLK_CTRL_REG |= EMAC_EX_CLK_CTRL_EMAC_EXT_OSC_EN;
   //Enable clock
   EMAC_EX_OSCCLK_CONF_REG |= EMAC_EX_OSCCLK_CONF_EMAC_OSC_CLK_SEL;

   //Configure RMII CLK (GPIO0)
   gpio_set_direction(0, GPIO_MODE_INPUT);

   //Configure TXD0 (GPIO19)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO19_U, FUNC_GPIO19_EMAC_TXD0);
   //Configure TX_EN (GPIO21)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO21_U, FUNC_GPIO21_EMAC_TX_EN);
   //Configure TXD1 (GPIO22)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO22_U, FUNC_GPIO22_EMAC_TXD1);

   //Configure RXD0 (GPIO25)
   gpio_set_direction(25, GPIO_MODE_INPUT);
   //Configure RXD1 (GPIO26)
   gpio_set_direction(26, GPIO_MODE_INPUT);
   //Configure CRS_DRV (GPIO27)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO27_U, FUNC_GPIO27_EMAC_RX_DV);

   //Configure MDC (GPIO2)
   gpio_matrix_out(2, EMAC_MDC_O_IDX, 0, 0);

   //Configure MDIO (GPIO4)
   gpio_matrix_out(4, EMAC_MDO_O_IDX, 0, 0);
   gpio_matrix_in(4, EMAC_MDI_I_IDX, 0);

//ESP32-EVB or ESP32-GATEWAY evaluation board?
#elif defined(ESP32_EVB) || defined(ESP32_GATEWAY)
   //Select RMII interface mode
   EMAC_EX_PHYINF_CONF_REG = EMAC_EX_PHYINF_CONF_EMAC_PHY_INTF_SEL_RMII;

   //Select clock source
   EMAC_EX_CLK_CTRL_REG |= EMAC_EX_CLK_CTRL_EMAC_EXT_OSC_EN;
   //Enable clock
   EMAC_EX_OSCCLK_CONF_REG |= EMAC_EX_OSCCLK_CONF_EMAC_OSC_CLK_SEL;

   //Configure RMII CLK (GPIO0)
   gpio_set_direction(0, GPIO_MODE_INPUT);

   //Configure TXD0 (GPIO19)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO19_U, FUNC_GPIO19_EMAC_TXD0);
   //Configure TX_EN (GPIO21)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO21_U, FUNC_GPIO21_EMAC_TX_EN);
   //Configure TXD1 (GPIO22)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO22_U, FUNC_GPIO22_EMAC_TXD1);

   //Configure RXD0 (GPIO25)
   gpio_set_direction(25, GPIO_MODE_INPUT);
   //Configure RXD1 (GPIO26)
   gpio_set_direction(26, GPIO_MODE_INPUT);
   //Configure CRS_DRV (GPIO27)
   PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO27_U, FUNC_GPIO27_EMAC_RX_DV);

   //Configure MDC (GPIO23)
   gpio_matrix_out(23, EMAC_MDC_O_IDX, 0, 0);

   //Configure MDIO (GPIO18)
   gpio_matrix_out(18, EMAC_MDO_O_IDX, 0, 0);
   gpio_matrix_in(18, EMAC_MDI_I_IDX, 0);
#endif
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void esp32EthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < ESP32_ETH_TX_BUFFER_COUNT; i++)
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
   for(i = 0; i < ESP32_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = EMAC_RDES0_OWN;
      //Use chain structure rather than ring structure
      rxDmaDesc[i].rdes1 = EMAC_RDES1_RCH | (ESP32_ETH_RX_BUFFER_SIZE & EMAC_RDES1_RBS1);
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
   EMAC_DMATXBASEADDR_REG = (uint32_t) txDmaDesc;
   //Start location of the RX descriptor list
   EMAC_DMARXBASEADDR_REG = (uint32_t) rxDmaDesc;
}


/**
 * @brief ESP32 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void esp32EthTick(NetInterface *interface)
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

void esp32EthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   esp_intr_enable_source(ETS_ETH_MAC_INTR_SOURCE);

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

void esp32EthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   esp_intr_disable_source(ETS_ETH_MAC_INTR_SOURCE);

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
 * @brief ESP32 Ethernet MAC interrupt service routine
 * @param[in] arg Unused parameter
 **/

void IRAM_ATTR esp32EthIrqHandler(void *arg)
{
   bool_t flag;
   uint32_t status;
   NetInterface *interface;

   //Interrupt service routine prologue
   osEnterIsr();

   //Point to the underlying network interface
   interface = (NetInterface *) arg;

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = EMAC_DMASTATUS_REG;

   //Packet transmitted?
   if((status & EMAC_DMASTATUS_TRANS_INT) != 0)
   {
      //Clear TI interrupt flag
      EMAC_DMASTATUS_REG = EMAC_DMASTATUS_TRANS_INT;

      //Check whether the TX buffer is available for writing
      if((txCurDmaDesc->tdes0 & EMAC_TDES0_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&interface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & EMAC_DMASTATUS_RECV_INT) != 0)
   {
      //Clear RI interrupt flag
      EMAC_DMASTATUS_REG = EMAC_DMASTATUS_RECV_INT;

      //Set event flag
      interface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   EMAC_DMASTATUS_REG = EMAC_DMASTATUS_NORM_INT_SUMM;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief ESP32 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void esp32EthEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = esp32EthReceivePacket(interface);

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

error_t esp32EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > ESP32_ETH_TX_BUFFER_SIZE)
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

   //Clear TBUS flag to resume processing
   EMAC_DMASTATUS_REG = EMAC_DMASTATUS_TRANS_BUF_UNAVAIL;
   //Instruct the DMA to poll the transmit descriptor list
   EMAC_DMATXPOLLDEMAND_REG = 0;

   //Point to the next descriptor in the list
   txCurDmaDesc = (Esp32EthTxDmaDesc *) txCurDmaDesc->tdes3;

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

error_t esp32EthReceivePacket(NetInterface *interface)
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
            n = MIN(n, ESP32_ETH_RX_BUFFER_SIZE);

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
      rxCurDmaDesc = (Esp32EthRxDmaDesc *) rxCurDmaDesc->rdes3;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RBUS flag to resume processing
   EMAC_DMASTATUS_REG = EMAC_DMASTATUS_RECV_BUF_UNAVAIL;
   //Instruct the DMA to poll the receive descriptor list
   EMAC_DMARXPOLLDEMAND_REG = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t esp32EthUpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   bool_t acceptMulticast;
   MacAddr unicastMacAddr[3];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Set the MAC address of the station
   EMAC_ADDR0HIGH_REG = interface->macAddr.w[2];
   EMAC_ADDR0LOW_REG = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);

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
      EMAC_ADDR1HIGH_REG = unicastMacAddr[0].w[2] | EMAC_ADDR1HIGH_ADDRESS_ENABLE1;
      EMAC_ADDR1LOW_REG = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
   }
   else
   {
      EMAC_ADDR1HIGH_REG = 0;
      EMAC_ADDR1LOW_REG = 0;
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      EMAC_ADDR2HIGH_REG = unicastMacAddr[1].w[2] | EMAC_ADDR2HIGH_ADDRESS_ENABLE2;
      EMAC_ADDR2LOW_REG = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
   }
   else
   {
      EMAC_ADDR2HIGH_REG = 0;
      EMAC_ADDR2LOW_REG = 0;
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      EMAC_ADDR3HIGH_REG = unicastMacAddr[2].w[2] | EMAC_ADDR3HIGH_ADDRESS_ENABLE3;
      EMAC_ADDR3LOW_REG = unicastMacAddr[2].w[0] | (unicastMacAddr[0].w[2] << 16);
   }
   else
   {
      EMAC_ADDR3HIGH_REG = 0;
      EMAC_ADDR3LOW_REG = 0;
   }

   //Enable or disable the reception of multicast frames
   if(acceptMulticast)
   {
      EMAC_FF_REG |= EMAC_FF_PAM;
   }
   else
   {
      EMAC_FF_REG &= ~EMAC_FF_PAM;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t esp32EthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = EMAC_CONFIG_REG;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= EMAC_CONFIG_EMACFESPEED;
   }
   else
   {
      config &= ~EMAC_CONFIG_EMACFESPEED;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= EMAC_CONFIG_EMACDUPLEX;
   }
   else
   {
      config &= ~EMAC_CONFIG_EMACDUPLEX;
   }

   //Update MAC configuration register
   EMAC_CONFIG_REG = config;

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

void esp32EthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = EMAC_MIIADDR_REG & EMAC_MIIADDR_MIICSRCLK;
      //Set up a write operation
      temp |= EMAC_MIIADDR_MIIWRITE | EMAC_MIIADDR_MIIBUSY;
      //PHY address
      temp |= (phyAddr << 11) & EMAC_MIIADDR_MIIDEV;
      //Register address
      temp |= (regAddr << 6) & EMAC_MIIADDR_MIIREG;

      //Data to be written in the PHY register
      EMAC_MIIDATA_REG = data & EMAC_MIIDATA_MII_DATA;

      //Start a write operation
      EMAC_MIIADDR_REG = temp;
      //Wait for the write to complete
      while((EMAC_MIIADDR_REG & EMAC_MIIADDR_MIIBUSY) != 0)
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

uint16_t esp32EthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = EMAC_MIIADDR_REG & EMAC_MIIADDR_MIICSRCLK;
      //Set up a read operation
      temp |= EMAC_MIIADDR_MIIBUSY;
      //PHY address
      temp |= (phyAddr << 11) & EMAC_MIIADDR_MIIDEV;
      //Register address
      temp |= (regAddr << 6) & EMAC_MIIADDR_MIIREG;

      //Start a read operation
      EMAC_MIIADDR_REG = temp;
      //Wait for the read to complete
      while((EMAC_MIIADDR_REG & EMAC_MIIADDR_MIIBUSY) != 0)
      {
      }

      //Get register value
      data = EMAC_MIIDATA_REG & EMAC_MIIDATA_MII_DATA;
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
      data = 0;
   }

   //Return the value of the PHY register
   return data;
}
