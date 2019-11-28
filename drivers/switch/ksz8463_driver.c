/**
 * @file ksz8463_driver.c
 * @brief KSZ8463 3-port Ethernet switch
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
#include "drivers/switch/ksz8463_driver.h"
#include "debug.h"


/**
 * @brief KSZ8463 Ethernet switch driver
 **/

const PhyDriver ksz8463PhyDriver =
{
   ksz8463Init,
   ksz8463Tick,
   ksz8463EnableIrq,
   ksz8463DisableIrq,
   ksz8463EventHandler,
   ksz8463TagFrame,
   ksz8463UntagFrame
};


/**
 * @brief Tail tag rules (host to KSZ8463)
 **/

const uint8_t ksz8463IngressTailTag[3] =
{
   0,
   KSZ8463_TAIL_TAG_ENCODE(1),
   KSZ8463_TAIL_TAG_ENCODE(2)
};


/**
 * @brief KSZ8463 Ethernet switch initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ksz8463Init(NetInterface *interface)
{
   uint_t port;
   uint16_t temp;

   //Debug message
   TRACE_INFO("Initializing KSZ8463...\r\n");

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Initialize SPI
      interface->spiDriver->init();

      //Wait for the serial interface to be ready
      do
      {
         //Read CIDER register
         temp = ksz8463ReadSwitchReg(interface, KSZ8463_CIDER);

         //The returned data is invalid until the serial interface is ready
      } while((temp & KSZ8463_CIDER_FAMILY_ID) != KSZ8463_CIDER_FAMILY_ID_DEFAULT);

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
      //Tail tagging mode?
      if(interface->port != 0)
      {
         //Enable tail tag feature
         temp = ksz8463ReadSwitchReg(interface, KSZ8463_SGCR8);
         temp |= KSZ8463_SGCR8_TAIL_TAG_EN;
         ksz8463WriteSwitchReg(interface, KSZ8463_SGCR8, temp);

         //Loop through ports
         for(port = KSZ8463_PORT1; port <= KSZ8463_PORT2; port++)
         {
            //Disable packet transmission and switch address learning
            temp = ksz8463ReadSwitchReg(interface, KSZ8463_PnCR2(port));
            temp &= ~KSZ8463_PnCR2_TRANSMIT_EN;
            temp |= KSZ8463_PnCR2_RECEIVE_EN | KSZ8463_PnCR2_LEARNING_DIS;
            ksz8463WriteSwitchReg(interface, KSZ8463_PnCR2(port), temp);
         }
      }
      else
#endif
      {
         //Disable tail tag feature
         temp = ksz8463ReadSwitchReg(interface, KSZ8463_SGCR8);
         temp &= ~KSZ8463_SGCR8_TAIL_TAG_EN;
         ksz8463WriteSwitchReg(interface, KSZ8463_SGCR8, temp);

         //Loop through ports
         for(port = KSZ8463_PORT1; port <= KSZ8463_PORT2; port++)
         {
            //Enable transmission, reception and switch address learning
            temp = ksz8463ReadSwitchReg(interface, KSZ8463_PnCR2(port));
            temp |= KSZ8463_PnCR2_TRANSMIT_EN | KSZ8463_PnCR2_RECEIVE_EN;
            temp &= ~KSZ8463_PnCR2_LEARNING_DIS;
            ksz8463WriteSwitchReg(interface, KSZ8463_PnCR2(port), temp);
         }
      }

      //Dump switch registers for debugging purpose
      ksz8463DumpSwitchReg(interface);
   }
   else
   {
      //Loop through ports
      for(port = KSZ8463_PORT1; port <= KSZ8463_PORT2; port++)
      {
         //Debug message
         TRACE_DEBUG("Port %u:\r\n", port);
         //Dump PHY registers for debugging purpose
         ksz8463DumpPhyReg(interface, port);
      }
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

bool_t ksz8463GetLinkState(NetInterface *interface, uint8_t port)
{
   uint16_t status;
   bool_t linkState;

   //Check port number
   if(port >= KSZ8463_PORT1 && port <= KSZ8463_PORT2)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //SPI slave mode?
      if(interface->spiDriver != NULL)
      {
         //Read port status register
         status = ksz8463ReadSwitchReg(interface, KSZ8463_PnSR(port));

         //Retrieve current link state
         linkState = (status & KSZ8463_PnSR_LINK_STATUS) ? TRUE : FALSE;
      }
      else
      {
         //Read status register
         status = ksz8463ReadPhyReg(interface, port, KSZ8463_BMSR);

         //Retrieve current link state
         linkState = (status & KSZ8463_BMSR_LINK_STATUS) ? TRUE : FALSE;
      }

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
 * @brief KSZ8463 timer handler
 * @param[in] interface Underlying network interface
 **/

void ksz8463Tick(NetInterface *interface)
{
   uint_t port;
   uint16_t status;
   bool_t linkState;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Tail tagging mode?
   if(interface->port != 0)
   {
      uint_t i;
      NetInterface *virtualInterface;

      //SPI slave mode?
      if(interface->spiDriver != NULL)
      {
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
               if(port >= KSZ8463_PORT1 && port <= KSZ8463_PORT2)
               {
                  //Read port status register
                  status = ksz8463ReadSwitchReg(interface, KSZ8463_PnSR(port));

                  //Retrieve current link state
                  linkState = (status & KSZ8463_PnSR_LINK_STATUS) ? TRUE : FALSE;

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
   }
   else
#endif
   {
      //Initialize link state
      linkState = FALSE;

      //Loop through ports
      for(port = KSZ8463_PORT1; port <= KSZ8463_PORT2; port++)
      {
         //SPI slave mode?
         if(interface->spiDriver != NULL)
         {
            //Read port status register
            status = ksz8463ReadSwitchReg(interface, KSZ8463_PnSR(port));

            //Retrieve current link state
            if(status & KSZ8463_PnSR_LINK_STATUS)
               linkState = TRUE;
         }
         else
         {
            //Read status register
            status = ksz8463ReadPhyReg(interface, port, KSZ8463_BMSR);

            //Retrieve current link state
            if(status & KSZ8463_BMSR_LINK_STATUS)
               linkState = TRUE;
         }
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

void ksz8463EnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void ksz8463DisableIrq(NetInterface *interface)
{
}


/**
 * @brief KSZ8463 event handler
 * @param[in] interface Underlying network interface
 **/

void ksz8463EventHandler(NetInterface *interface)
{
   uint_t port;
   uint16_t status;
   bool_t linkState;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Tail tagging mode?
   if(interface->port != 0)
   {
      uint_t i;
      NetInterface *virtualInterface;

      //SPI slave mode?
      if(interface->spiDriver != NULL)
      {
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
               if(port >= KSZ8463_PORT1 && port <= KSZ8463_PORT2)
               {
                  //Read port status register
                  status = ksz8463ReadSwitchReg(interface, KSZ8463_PnSR(port));

                  //Retrieve current link state
                  linkState = (status & KSZ8463_PnSR_LINK_STATUS) ? TRUE : FALSE;

                  //Link up event?
                  if(linkState && !virtualInterface->linkState)
                  {
                     //Adjust MAC configuration parameters for proper operation
                     interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                     interface->duplexMode = NIC_FULL_DUPLEX_MODE;
                     interface->nicDriver->updateMacConfig(interface);

                     //Check current speed
                     if(status & KSZ8463_PnSR_OP_SPEED)
                        virtualInterface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                     else
                        virtualInterface->linkSpeed = NIC_LINK_SPEED_10MBPS;

                     //Check duplex mode
                     if(status & KSZ8463_PnSR_OP_DUPLEX)
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
   }
   else
#endif
   {
      //Initialize link state
      linkState = FALSE;

      //Loop through ports
      for(port = KSZ8463_PORT1; port <= KSZ8463_PORT2; port++)
      {
         //SPI slave mode?
         if(interface->spiDriver != NULL)
         {
            //Read port status register
            status = ksz8463ReadSwitchReg(interface, KSZ8463_PnSR(port));

            //Retrieve current link state
            if(status & KSZ8463_PnSR_LINK_STATUS)
               linkState = TRUE;
         }
         else
         {
            //Read status register
            status = ksz8463ReadPhyReg(interface, port, KSZ8463_BMSR);

            //Retrieve current link state
            if(status & KSZ8463_BMSR_LINK_STATUS)
               linkState = TRUE;
         }
      }

      //Link up event?
      if(linkState)
      {
         //Adjust MAC configuration parameters for proper operation
         interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
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

error_t ksz8463TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type)
{
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   error_t error;
   size_t length;
   const uint8_t *tailTag;

   //Valid port?
   if(port >= KSZ8463_PORT1 && port <= KSZ8463_PORT2)
   {
      //The one byte tail tagging is used to indicate the destination port
      tailTag = &ksz8463IngressTailTag[port];

      //Retrieve the length of the frame
      length = netBufferGetLength(buffer) - *offset;

      //The host controller should manually add padding to the packet before
      //inserting the tail tag
      error = ethPadFrame(buffer, &length);

      //Check status code
      if(!error)
      {
         //The tail tag is inserted at the end of the packet, just before the CRC
         error = netBufferAppend(buffer, tailTag, sizeof(uint8_t));
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

error_t ksz8463UntagFrame(NetInterface *interface, uint8_t **frame,
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
      *port = KSZ8463_TAIL_TAG_DECODE(*tailTag);

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

void ksz8463WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data)
{
   //Write the specified PHY register
   interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE, port, address, data);
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t ksz8463ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address)
{
   //Read the specified PHY register
   return interface->nicDriver->readPhyReg(SMI_OPCODE_READ, port, address);
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 **/

void ksz8463DumpPhyReg(NetInterface *interface, uint8_t port)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         ksz8463ReadPhyReg(interface, port, i));
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

void ksz8463WriteSwitchReg(NetInterface *interface, uint16_t address,
   uint16_t data)
{
   uint16_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a write operation
      command = KSZ8463_SPI_CMD_WRITE;

      //The byte enable bits are set to indicate which bytes will be
      //transferred in the data phase
      if(address & 0x02)
         command |= KSZ8463_SPI_CMD_B3 | KSZ8463_SPI_CMD_B2;
      else
         command |= KSZ8463_SPI_CMD_B1 | KSZ8463_SPI_CMD_B0;

      //Set register address
      command |= (address << 4) & KSZ8463_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 16-bit command
      interface->spiDriver->transfer(MSB(command));
      interface->spiDriver->transfer(LSB(command));

      //Write 16-bit data
      interface->spiDriver->transfer(LSB(data));
      interface->spiDriver->transfer(MSB(data));

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

uint16_t ksz8463ReadSwitchReg(NetInterface *interface, uint16_t address)
{
   uint16_t data;
   uint16_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a read operation
      command = KSZ8463_SPI_CMD_READ;

      //The byte enable bits are set to indicate which bytes will be
      //transferred in the data phase
      if(address & 0x02)
         command |= KSZ8463_SPI_CMD_B3 | KSZ8463_SPI_CMD_B2;
      else
         command |= KSZ8463_SPI_CMD_B1 | KSZ8463_SPI_CMD_B0;

      //Set register address
      command |= (address << 4) & KSZ8463_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 16-bit command
      interface->spiDriver->transfer(MSB(command));
      interface->spiDriver->transfer(LSB(command));

      //Read 16-bit data
      data = interface->spiDriver->transfer(0xFF);
      data |= interface->spiDriver->transfer(0xFF) << 8;

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

void ksz8463DumpSwitchReg(NetInterface *interface)
{
   uint16_t i;

   //Loop through switch registers
   for(i = 0; i < 256; i += 2)
   {
      //Display current switch register
      TRACE_DEBUG("0x%02" PRIX16 " (%02" PRIu16 ") : 0x%04" PRIX16 "\r\n",
         i, i, ksz8463ReadSwitchReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
