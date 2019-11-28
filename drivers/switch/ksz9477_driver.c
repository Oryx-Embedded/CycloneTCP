/**
 * @file ksz9477_driver.c
 * @brief KSZ9477 7-port Gigabit Ethernet switch
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.6
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/ethernet_misc.h"
#include "drivers/switch/ksz9477_driver.h"
#include "debug.h"


/**
 * @brief KSZ9477 Ethernet switch driver
 **/

const PhyDriver ksz9477PhyDriver =
{
   ksz9477Init,
   ksz9477Tick,
   ksz9477EnableIrq,
   ksz9477DisableIrq,
   ksz9477EventHandler,
   ksz9477TagFrame,
   ksz9477UntagFrame
};


/**
 * @brief Tail tag rules (host to KSZ9477)
 **/

const uint16_t ksz9477IngressTailTag[6] =
{
   0,
   KSZ9477_TAIL_TAG_ENCODE(1),
   KSZ9477_TAIL_TAG_ENCODE(2),
   KSZ9477_TAIL_TAG_ENCODE(3),
   KSZ9477_TAIL_TAG_ENCODE(4),
   KSZ9477_TAIL_TAG_ENCODE(5)
};


/**
 * @brief KSZ9477 Ethernet switch initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ksz9477Init(NetInterface *interface)
{
   uint_t port;
   uint8_t temp;

   //Debug message
   TRACE_INFO("Initializing KSZ9477...\r\n");

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Initialize SPI
      interface->spiDriver->init();

      //Wait for the serial interface to be ready
      do
      {
         //Read CHIP_ID1 register
         temp = ksz9477ReadSwitchReg(interface, KSZ9477_CHIP_ID1);

         //The returned data is invalid until the serial interface is ready
      } while(temp != KSZ9477_CHIP_ID1_DEFAULT);

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
      //Tail tagging mode?
      if(interface->port != 0)
      {
         //Enable tail tag feature
         temp = ksz9477ReadSwitchReg(interface, KSZ9477_PORT6_OP_CTRL0);
         temp |= KSZ9477_PORTn_OP_CTRL0_TAIL_TAG_EN;
         ksz9477WriteSwitchReg(interface, KSZ9477_PORT6_OP_CTRL0, temp);

         //Loop through ports
         for(port = KSZ9477_PORT1; port <= KSZ9477_PORT5; port++)
         {
            //Disable packet transmission and switch address learning
            temp = ksz9477ReadSwitchReg(interface, KSZ9477_PORTn_MSTP_STATE(port));
            temp &= ~KSZ9477_PORTn_MSTP_STATE_TRANSMIT_EN;
            temp |= KSZ9477_PORTn_MSTP_STATE_RECEIVE_EN;
            temp |= KSZ9477_PORTn_MSTP_STATE_LEARNING_DIS;
            ksz9477WriteSwitchReg(interface, KSZ9477_PORTn_MSTP_STATE(port), temp);
         }
      }
      else
#endif
      {
         //Disable tail tag feature
         temp = ksz9477ReadSwitchReg(interface, KSZ9477_PORT6_OP_CTRL0);
         temp &= ~KSZ9477_PORTn_OP_CTRL0_TAIL_TAG_EN;
         ksz9477WriteSwitchReg(interface, KSZ9477_PORT6_OP_CTRL0, temp);

         //Loop through ports
         for(port = KSZ9477_PORT1; port <= KSZ9477_PORT5; port++)
         {
            //Enable transmission, reception and switch address learning
            temp = ksz9477ReadSwitchReg(interface, KSZ9477_PORTn_MSTP_STATE(port));
            temp |= KSZ9477_PORTn_MSTP_STATE_TRANSMIT_EN;
            temp |= KSZ9477_PORTn_MSTP_STATE_RECEIVE_EN;
            temp &= ~KSZ9477_PORTn_MSTP_STATE_LEARNING_DIS;
            ksz9477WriteSwitchReg(interface, KSZ9477_PORTn_MSTP_STATE(port), temp);
         }
      }

      //Add internal delay to ingress and egress RGMII clocks
      temp = ksz9477ReadSwitchReg(interface, KSZ9477_PORT6_XMII_CTRL1);
      temp |= KSZ9477_PORTn_XMII_CTRL1_RGMII_ID_IG;
      temp |= KSZ9477_PORTn_XMII_CTRL1_RGMII_ID_EG;
      ksz9477WriteSwitchReg(interface, KSZ9477_PORT6_XMII_CTRL1, temp);

      //Start switch operation
      ksz9477WriteSwitchReg(interface, KSZ9477_SWITCH_OP,
         KSZ9477_SWITCH_OP_START_SWITCH);
   }

   //Loop through ports
   for(port = KSZ9477_PORT1; port <= KSZ9477_PORT5; port++)
   {
      //Debug message
      TRACE_DEBUG("Port %u:\r\n", port);
      //Dump PHY registers for debugging purpose
      ksz9477DumpPhyReg(interface, port);
   }

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Get link state
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Link state
 **/

bool_t ksz9477GetLinkState(NetInterface *interface, uint8_t port)
{
   uint16_t value;
   bool_t linkState;

   //Check port number
   if(port >= KSZ9477_PORT1 && port <= KSZ9477_PORT5)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Any link failure condition is latched in the BMSR register. Reading
      //the register twice will always return the actual link status
      value = ksz9477ReadPhyReg(interface, port, KSZ9477_BMSR);
      value = ksz9477ReadPhyReg(interface, port, KSZ9477_BMSR);

      //Retrieve current link state
      linkState = (value & KSZ9477_BMSR_LINK_STATUS) ? TRUE : FALSE;

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }
   else
   {
      //The specified port number is not valid
      linkState = FALSE;
   }

   //Return link status
   return linkState;
}


/**
 * @brief KSZ9477 timer handler
 * @param[in] interface Underlying network interface
 **/

void ksz9477Tick(NetInterface *interface)
{
   uint_t port;
   uint16_t value;
   bool_t linkState;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Tail tagging mode?
   if(interface->port != 0)
   {
      uint_t i;
      NetInterface *virtualInterface;

      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Point to the current interface
         virtualInterface = &netInterface[i];

         //Check whether the current virtual interface is attached to the
         //physical interface
         if(virtualInterface == interface || virtualInterface->parent == interface)
         {
            //The tail tag is used to indicate the source/destination port
            port = virtualInterface->port;

            //Valid port?
            if(port >= KSZ9477_PORT1 && port <= KSZ9477_PORT5)
            {
               //Read status register
               value = ksz9477ReadPhyReg(interface, port, KSZ9477_BMSR);

               //Retrieve current link state
               linkState = (value & KSZ9477_BMSR_LINK_STATUS) ? TRUE : FALSE;

               //Link up or link down event?
               if(linkState != virtualInterface->linkState)
               {
                  //Set event flag
                  interface->phyEvent = TRUE;
                  //Notify the TCP/IP stack of the event
                  osSetEvent(&netEvent);
               }
            }
         }
      }
   }
   else
#endif
   {
      //Initialize link state
      linkState = FALSE;

      //Loop through ports
      for(port = KSZ9477_PORT1; port <= KSZ9477_PORT5; port++)
      {
         //Read status register
         value = ksz9477ReadPhyReg(interface, port, KSZ9477_BMSR);

         //Retrieve current link state
         if(value & KSZ9477_BMSR_LINK_STATUS)
            linkState = TRUE;
      }

      //Link up or link down event?
      if(linkState != interface->linkState)
      {
         //Set event flag
         interface->phyEvent = TRUE;
         //Notify the TCP/IP stack of the event
         osSetEvent(&netEvent);
      }
   }
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void ksz9477EnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void ksz9477DisableIrq(NetInterface *interface)
{
}


/**
 * @brief KSZ9477 event handler
 * @param[in] interface Underlying network interface
 **/

void ksz9477EventHandler(NetInterface *interface)
{
   uint_t port;
   uint16_t value;
   bool_t linkState;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Tail tagging mode?
   if(interface->port != 0)
   {
      uint_t i;
      NetInterface *virtualInterface;

      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Point to the current interface
         virtualInterface = &netInterface[i];

         //Check whether the current virtual interface is attached to the
         //physical interface
         if(virtualInterface == interface ||
            virtualInterface->parent == interface)
         {
            //The tail tag is used to indicate the source/destination port
            port = virtualInterface->port;

            //Valid port?
            if(port >= KSZ9477_PORT1 && port <= KSZ9477_PORT5)
            {
               //Any link failure condition is latched in the BMSR register. Reading
               //the register twice will always return the actual link status
               value = ksz9477ReadPhyReg(interface, port, KSZ9477_BMSR);
               value = ksz9477ReadPhyReg(interface, port, KSZ9477_BMSR);

               //Retrieve current link state
               linkState = (value & KSZ9477_BMSR_LINK_STATUS) ? TRUE : FALSE;

               //Link up event?
               if(linkState && !virtualInterface->linkState)
               {
                  //Adjust MAC configuration parameters for proper operation
                  interface->linkSpeed = NIC_LINK_SPEED_1GBPS;
                  interface->duplexMode = NIC_FULL_DUPLEX_MODE;
                  interface->nicDriver->updateMacConfig(interface);

                  //Read PHY control register
                  value = ksz9477ReadPhyReg(interface, port, KSZ9477_PHYCON);

                  //Check current speed
                  if(value & KSZ9477_PHYCON_SPEED_1000BT)
                  {
                     //1000BASE-T
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_1GBPS;
                  }
                  else if(value & KSZ9477_PHYCON_SPEED_100BTX)
                  {
                     //100BASE-TX
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                  }
                  else if(value & KSZ9477_PHYCON_SPEED_10BT)
                  {
                     //10BASE-T
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_10MBPS;
                  }
                  else
                  {
                     //Debug message
                     TRACE_WARNING("Invalid speed!\r\n");
                  }

                  //Check current duplex mode
                  if(value & KSZ9477_PHYCON_DUPLEX_STATUS)
                     virtualInterface->duplexMode = NIC_FULL_DUPLEX_MODE;
                  else
                     virtualInterface->duplexMode = NIC_HALF_DUPLEX_MODE;

                  //Update link state
                  virtualInterface->linkState = TRUE;

                  //Process link state change event
                  nicNotifyLinkChange(virtualInterface);
               }
               //Link down event
               else if(!linkState && virtualInterface->linkState)
               {
                  //Update link state
                  virtualInterface->linkState = FALSE;

                  //Process link state change event
                  nicNotifyLinkChange(virtualInterface);
               }
            }
         }
      }
   }
   else
#endif
   {
      //Initialize link state
      linkState = FALSE;

      //Loop through ports
      for(port = KSZ9477_PORT1; port <= KSZ9477_PORT5; port++)
      {
         //Any link failure condition is latched in the BMSR register. Reading
         //the register twice will always return the actual link status
         value = ksz9477ReadPhyReg(interface, port, KSZ9477_BMSR);
         value = ksz9477ReadPhyReg(interface, port, KSZ9477_BMSR);

         //Retrieve current link state
         if(value & KSZ9477_BMSR_LINK_STATUS)
            linkState = TRUE;
      }

      //Link up event?
      if(linkState)
      {
         //Adjust MAC configuration parameters for proper operation
         interface->linkSpeed = NIC_LINK_SPEED_1GBPS;
         interface->duplexMode = NIC_FULL_DUPLEX_MODE;
         interface->nicDriver->updateMacConfig(interface);

         //Update link state
         interface->linkState = TRUE;
      }
      else
      {
         //Update link state
         interface->linkState = FALSE;
      }

      //Process link state change event
      nicNotifyLinkChange(interface);
   }
}


/**
 * @brief Add tail tag to Ethernet frame
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the payload
 * @param[in,out] offset Offset to the first payload byte
 * @param[in] port Switch port identifier
 * @param[in,out] type Ethernet type
 * @return Error code
 **/

error_t ksz9477TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type)
{
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   error_t error;
   size_t length;
   const uint16_t *tailTag;

   //Valid port?
   if(port >= KSZ9477_PORT1 && port <= KSZ9477_PORT5)
   {
      //The two-byte tail tagging is used to indicate the destination port
      tailTag = &ksz9477IngressTailTag[port];

      //Retrieve the length of the frame
      length = netBufferGetLength(buffer) - *offset;

      //The host controller should manually add padding to the packet before
      //inserting the tail tag
      error = ethPadFrame(buffer, &length);

      //Check status code
      if(!error)
      {
         //The tail tag is inserted at the end of the packet, just before the CRC
         error = netBufferAppend(buffer, tailTag, sizeof(uint16_t));
      }
   }
   else
   {
      //Invalid port identifier
      error = ERROR_WRONG_IDENTIFIER;
   }

   //Return status code
   return error;
#else
   //Tail tagging mode is not implemented
   return NO_ERROR;
#endif
}


/**
 * @brief Decode tail tag from incoming Ethernet frame
 * @param[in] interface Underlying network interface
 * @param[in,out] frame Pointer to the received Ethernet frame
 * @param[in,out] length Length of the frame, in bytes
 * @param[out] port Switch port identifier
 * @return Error code
 **/

error_t ksz9477UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, uint8_t *port)
{
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   error_t error;
   uint8_t *tailTag;

   //Valid Ethernet frame received?
   if(*length >= (sizeof(EthHeader) + sizeof(uint8_t)))
   {
      //The tail tag is inserted at the end of the packet, just before the CRC
      tailTag = *frame + *length - sizeof(uint8_t);

      //The one byte tail tagging is used to indicate the source port
      *port = KSZ9477_TAIL_TAG_DECODE(*tailTag);

      //Strip tail tag from Ethernet frame
      *length -= sizeof(uint8_t);

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //Drop the received frame
      error = ERROR_INVALID_LENGTH;
   }

   //Return status code
   return error;
#else
   //Tail tagging mode is not implemented
   return NO_ERROR;
#endif
}


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void ksz9477WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data)
{
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a write operation
      command = KSZ9477_SPI_CMD_WRITE;
      //Set register address
      command |= KSZ9477_PORTn_ETH_PHY_REG(port, address) << 5;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Write 16-bit data
      interface->spiDriver->transfer(MSB(data));
      interface->spiDriver->transfer(LSB(data));

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //Write the specified PHY register
      interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE, port, address, data);
   }
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t ksz9477ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address)
{
   uint16_t data;
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a read operation
      command = KSZ9477_SPI_CMD_READ;
      //Set register address
      command |= KSZ9477_PORTn_ETH_PHY_REG(port, address) << 5;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Read 16-bit data
      data = interface->spiDriver->transfer(0xFF) << 8;
      data |= interface->spiDriver->transfer(0xFF);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //Read the specified PHY register
      data = interface->nicDriver->readPhyReg(SMI_OPCODE_READ, port, address);
   }

   //Return register value
   return data;
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 **/

void ksz9477DumpPhyReg(NetInterface *interface, uint8_t port)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         ksz9477ReadPhyReg(interface, port, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Write switch register
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @param[in] data Register value
 **/

void ksz9477WriteSwitchReg(NetInterface *interface, uint16_t address,
   uint8_t data)
{
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a write operation
      command = KSZ9477_SPI_CMD_WRITE;
      //Set register address
      command |= (address << 5) & KSZ9477_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Write data
      interface->spiDriver->transfer(data);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //The MDC/MDIO interface does not have access to all the configuration
      //registers. It can only access the standard MIIM registers
   }
}


/**
 * @brief Read switch register
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @return Register value
 **/

uint8_t ksz9477ReadSwitchReg(NetInterface *interface, uint16_t address)
{
   uint8_t data;
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a read operation
      command = KSZ9477_SPI_CMD_READ;
      //Set register address
      command |= (address << 5) & KSZ9477_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Read data
      data = interface->spiDriver->transfer(0xFF);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //The MDC/MDIO interface does not have access to all the configuration
      //registers. It can only access the standard MIIM registers
      data = 0;
   }

   //Return register value
   return data;
}


/**
 * @brief Dump switch registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void ksz9477DumpSwitchReg(NetInterface *interface)
{
   uint16_t i;

   //Loop through switch registers
   for(i = 0; i < 256; i++)
   {
      //Display current switch register
      TRACE_DEBUG("0x%02" PRIX16 " (%02" PRIu16 ") : 0x%02" PRIX8 "\r\n",
         i, i, ksz9477ReadSwitchReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
