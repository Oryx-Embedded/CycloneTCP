/**
 * @file esp32_eth_driver.c
 * @brief ESP32 Ethernet MAC controller
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
#include "soc/soc.h"
#include "soc/dport_reg.h"
#include "soc/emac_ex_reg.h"
#include "soc/emac_reg_v2.h"
#include "driver/periph_ctrl.h"
#include "core/net.h"
#include "drivers/mac/esp32_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

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
   esp32EthSetMulticastFilter,
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

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable EMAC module
   periph_module_enable(PERIPH_EMAC_MODULE);
   //Enable Ethernet MAC clock
   DPORT_REG_SET_BIT(EMAC_CLK_EN_REG, EMAC_CLK_EN);

   //GPIO configuration
   esp32EthInitGpio(interface);

   //Perform a software reset
   REG_SET_BIT(EMAC_DMABUSMODE_REG, EMAC_SW_RST);
   //Wait for the reset to complete
   while(REG_GET_BIT(EMAC_DMABUSMODE_REG, EMAC_SW_RST));

   //Adjust MDC clock range
   REG_SET_FIELD(EMAC_GMACGMIIDATA_REG, EMAC_GMIICSRCLK, 1);

   //PHY transceiver initialization
   error = interface->phyDriver->init(interface);
   //Failed to initialize PHY transceiver?
   if(error)
      return error;

   //Use default MAC configuration
   REG_WRITE(EMAC_GMACCONFIG_REG, 0);
   REG_SET_BIT(EMAC_GMACCONFIG_REG, EMAC_GMACMIIGMII);
   REG_SET_BIT(EMAC_GMACCONFIG_REG, EMAC_GMACRXOWN);

   //Set the MAC address
   REG_WRITE(EMAC_GMACADDR0HIGH_REG, interface->macAddr.w[2] | EMAC_ADDRESS_ENABLE0_M);
   REG_WRITE(EMAC_GMACADDR0LOW_REG, interface->macAddr.w[0] | (interface->macAddr.w[1] << 16));

   //Configure the receive filter
   REG_WRITE(EMAC_GMACFRAMEFILTER_REG, 0);
   //Disable flow control
   REG_WRITE(EMAC_GMACFLOWCONTROL_REG, 0);

   //Enable store and forward mode
   REG_WRITE(EMAC_DMAOPERATION_MODE_REG, 0);
   REG_SET_BIT(EMAC_DMAOPERATION_MODE_REG, EMAC_RECV_STORE_FORWARD);
   REG_SET_BIT(EMAC_DMAOPERATION_MODE_REG, EMAC_TRANSMIT_STORE_FORWARD);

   //Configure DMA bus mode
   REG_SET_FIELD(EMAC_DMABUSMODE_REG, EMAC_RX_DMA_PBL, 1);
   REG_SET_FIELD(EMAC_DMABUSMODE_REG, EMAC_PROG_BURST_LEN, 1);
   REG_SET_BIT(EMAC_DMABUSMODE_REG, EMAC_ALT_DESC_SIZE);

   //Initialize DMA descriptor lists
   esp32EthInitDmaDesc(interface);

   //Disable MAC interrupts
   REG_SET_BIT(EMAC_GMACINTERRUPTMASK_REG, EMAC_LPI_INTERRUPT_MASK);
   REG_SET_BIT(EMAC_GMACINTERRUPTMASK_REG, EMAC_PMT_INTERRUPT_MASK);
   REG_SET_BIT(EMAC_GMACINTERRUPTMASK_REG, EMAC_INTERRUPT_MASK);

   //Enable the desired DMA interrupts
   REG_WRITE(EMAC_DMAINTERRUPT_EN_REG, 0);
   REG_SET_BIT(EMAC_DMAINTERRUPT_EN_REG, EMAC_NORMAL_INTERRUPT_SUMMARY_ENABLE);
   REG_SET_BIT(EMAC_DMAINTERRUPT_EN_REG, EMAC_RECEIVE_INTERRUPT_ENABLE);
   REG_SET_BIT(EMAC_DMAINTERRUPT_EN_REG, EMAC_TRANSMIT_INTERRUPT_ENABLE);

   //Register interrupt handler
   esp_intr_alloc(ETS_ETH_MAC_INTR_SOURCE, 0, esp32EthIrqHandler, NULL, NULL);

   //Enable MAC transmission and reception
   REG_SET_BIT(EMAC_GMACCONFIG_REG, EMAC_GMACTX);
   REG_SET_BIT(EMAC_GMACCONFIG_REG, EMAC_GMACRX);

   //Enable DMA transmission and reception
   REG_SET_BIT(EMAC_DMAOPERATION_MODE_REG, EMAC_START_STOP_TRANSMISSION_COMMAND);
   REG_SET_BIT(EMAC_DMAOPERATION_MODE_REG, EMAC_START_STOP_RECEIVE);

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


//ESP32-EVB or ESP32-GATEWAY evaluation board?
#if defined(ESP32_EVB) || defined(ESP32_GATEWAY)

/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

void esp32EthInitGpio(NetInterface *interface)
{
   //Select RMII interface mode
   REG_SET_FIELD(EMAC_EX_PHYINF_CONF_REG, EMAC_EX_PHY_INTF_SEL, EMAC_EX_PHY_INTF_RMII);

   //Select clock source
   REG_SET_BIT(EMAC_EX_CLK_CTRL_REG, EMAC_EX_EXT_OSC_EN);
   //Enable clock
   REG_SET_BIT(EMAC_EX_OSCCLK_CONF_REG, EMAC_EX_OSC_CLK_SEL);

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
}

#endif


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
   REG_WRITE(EMAC_DMATXBASEADDR_REG, (uint32_t) txDmaDesc);
   //Start location of the RX descriptor list
   REG_WRITE(EMAC_DMARXBASEADDR_REG, (uint32_t) rxDmaDesc);
}


/**
 * @brief ESP32 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void esp32EthTick(NetInterface *interface)
{
   //Handle periodic operations
   interface->phyDriver->tick(interface);
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void esp32EthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts

   //Enable Ethernet PHY interrupts
   interface->phyDriver->enableIrq(interface);
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void esp32EthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts

   //Disable Ethernet PHY interrupts
   interface->phyDriver->disableIrq(interface);
}


/**
 * @brief ESP32 Ethernet MAC interrupt service routine
 * @param[in] arg Unused parameter
 **/

void IRAM_ATTR esp32EthIrqHandler(void *arg)
{
   bool_t flag;
   uint32_t status;

   //Enter interrupt service routine
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = REG_READ(EMAC_DMASTATUS_REG);

   //A packet has been transmitted?
   if(status & EMAC_TRANS_INT_M)
   {
      //Clear TI interrupt flag
      REG_WRITE(EMAC_DMASTATUS_REG, EMAC_TRANS_INT_M);

      //Check whether the TX buffer is available for writing
      if(!(txCurDmaDesc->tdes0 & EMAC_TDES0_OWN))
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //A packet has been received?
   if(status & EMAC_RECV_INT_M)
   {
      //Disable RIE interrupt
      REG_CLR_BIT(EMAC_DMAINTERRUPT_EN_REG, EMAC_RECEIVE_INTERRUPT_ENABLE);

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   REG_WRITE(EMAC_DMASTATUS_REG, EMAC_NORM_INT_SUMM_M);

   //Leave interrupt service routine
   osExitIsr(flag);
}


/**
 * @brief ESP32 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void esp32EthEventHandler(NetInterface *interface)
{
   error_t error;

   //Packet received?
   if(REG_GET_BIT(EMAC_DMASTATUS_REG, EMAC_RECV_INT))
   {
      //Clear interrupt flag
      REG_WRITE(EMAC_DMASTATUS_REG, EMAC_RECV_INT_M);

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = esp32EthReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }

   //Re-enable DMA interrupts
   REG_SET_BIT(EMAC_DMAINTERRUPT_EN_REG, EMAC_NORMAL_INTERRUPT_SUMMARY_ENABLE);
   REG_SET_BIT(EMAC_DMAINTERRUPT_EN_REG, EMAC_RECEIVE_INTERRUPT_ENABLE);
   REG_SET_BIT(EMAC_DMAINTERRUPT_EN_REG, EMAC_TRANSMIT_INTERRUPT_ENABLE);
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t esp32EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
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
   if(txCurDmaDesc->tdes0 & EMAC_TDES0_OWN)
      return ERROR_FAILURE;

   //Copy user data to the transmit buffer
   netBufferRead((uint8_t *) txCurDmaDesc->tdes2, buffer, offset, length);

   //Write the number of bytes to send
   txCurDmaDesc->tdes1 = length & EMAC_TDES1_TBS1;
   //Set LS and FS flags as the data fits in a single buffer
   txCurDmaDesc->tdes0 |= EMAC_TDES0_LS | EMAC_TDES0_FS;
   //Give the ownership of the descriptor to the DMA
   txCurDmaDesc->tdes0 |= EMAC_TDES0_OWN;

   //Clear TBUS flag to resume processing
   REG_WRITE(EMAC_DMASTATUS_REG, EMAC_TRANS_BUF_UNAVAIL_M);
   //Instruct the DMA to poll the transmit descriptor list
   REG_WRITE(EMAC_DMATXPOLLDEMAND_REG, 0);

   //Point to the next descriptor in the list
   txCurDmaDesc = (Esp32EthTxDmaDesc *) txCurDmaDesc->tdes3;

   //Check whether the next buffer is available for writing
   if(!(txCurDmaDesc->tdes0 & EMAC_TDES0_OWN))
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

   //The current buffer is available for reading?
   if(!(rxCurDmaDesc->rdes0 & EMAC_RDES0_OWN))
   {
      //FS and LS flags should be set
      if((rxCurDmaDesc->rdes0 & EMAC_RDES0_FS) && (rxCurDmaDesc->rdes0 & EMAC_RDES0_LS))
      {
         //Make sure no error occurred
         if(!(rxCurDmaDesc->rdes0 & EMAC_RDES0_ES))
         {
            //Retrieve the length of the frame
            n = (rxCurDmaDesc->rdes0 & EMAC_RDES0_FL) >> 16;
            //Limit the number of data to read
            n = MIN(n, ESP32_ETH_RX_BUFFER_SIZE);

            //Pass the packet to the upper layer
            nicProcessPacket(interface, (uint8_t *) rxCurDmaDesc->rdes2, n);

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
   REG_WRITE(EMAC_DMASTATUS_REG, EMAC_RECV_BUF_UNAVAIL_M);
   //Instruct the DMA to poll the receive descriptor list
   REG_WRITE(EMAC_DMARXPOLLDEMAND_REG, 0);

   //Return status code
   return error;
}


/**
 * @brief Configure multicast MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t esp32EthSetMulticastFilter(NetInterface *interface)
{
   uint_t i;
   bool_t acceptMulticast;

   //This flag will be set if multicast addresses should be accepted
   acceptMulticast = FALSE;

   //The MAC filter table contains the multicast MAC addresses
   //to accept when receiving an Ethernet frame
   for(i = 0; i < MAC_MULTICAST_FILTER_SIZE; i++)
   {
      //Valid entry?
      if(interface->macMulticastFilter[i].refCount > 0)
      {
         //Accept multicast addresses
         acceptMulticast = TRUE;
         //We are done
         break;
      }
   }

   //Enable the reception of multicast frames if necessary
   if(acceptMulticast)
      REG_SET_BIT(EMAC_GMACFRAMEFILTER_REG, EMAC_PASS_ALL_MULTICAST);
   else
      REG_CLR_BIT(EMAC_GMACFRAMEFILTER_REG, EMAC_PASS_ALL_MULTICAST);

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
   config = REG_READ(EMAC_GMACCONFIG_REG);

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
      config |= EMAC_GMACFESPEED_M;
   else
      config &= ~EMAC_GMACFESPEED_M;

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
      config |= EMAC_GMACDUPLEX_M;
   else
      config &= ~EMAC_GMACDUPLEX_M;

   //Update MAC configuration register
   REG_WRITE(EMAC_GMACCONFIG_REG, config);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @param[in] data Register value
 **/

void esp32EthWritePhyReg(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
   uint32_t value;

   //Take care not to alter MDC clock configuration
   value = REG_READ(EMAC_GMACGMIIADDR_REG) & EMAC_GMIICSRCLK_M;
   //Set up a write operation
   value |= EMAC_GMIIWRITE_M | EMAC_GMIIBUSY_M;
   //PHY address
   value |= (phyAddr << EMAC_GMIIDEV_S) & EMAC_GMIIDEV_M;
   //Register address
   value |= (regAddr << EMAC_GMIIREG_S) & EMAC_GMIIREG_M;

   //Data to be written in the PHY register
   REG_WRITE(EMAC_GMACGMIIDATA_REG, data);

   //Start a write operation
   REG_WRITE(EMAC_GMACGMIIADDR_REG, value);
   //Wait for the write to complete
   while(REG_GET_BIT(EMAC_GMACGMIIADDR_REG, EMAC_GMIIBUSY));
}


/**
 * @brief Read PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t esp32EthReadPhyReg(uint8_t phyAddr, uint8_t regAddr)
{
   uint32_t value;

   //Take care not to alter MDC clock configuration
   value = REG_READ(EMAC_GMACGMIIADDR_REG) & EMAC_GMIICSRCLK_M;
   //Set up a read operation
   value |= EMAC_GMIIBUSY_M;
   //PHY address
   value |= (phyAddr << EMAC_GMIIDEV_S) & EMAC_GMIIDEV_M;
   //Register address
   value |= (regAddr << EMAC_GMIIREG_S) & EMAC_GMIIREG_M;

   //Start a read operation
   REG_WRITE(EMAC_GMACGMIIADDR_REG, value);
   //Wait for the read to complete
   while(REG_GET_BIT(EMAC_GMACGMIIADDR_REG, EMAC_GMIIBUSY));

   //Return PHY register contents
   return REG_READ(EMAC_GMACGMIIDATA_REG);
}
