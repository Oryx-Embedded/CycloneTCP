/**
 * @file ksz8863_driver.c
 * @brief KSZ8863 3-port Ethernet switch
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
#include "drivers/switch/ksz8863_driver.h"
#include "debug.h"


/**
 * @brief KSZ8863 Ethernet switch driver
 **/

const PhyDriver ksz8863PhyDriver =
{
   ksz8863Init,
   ksz8863Tick,
   ksz8863EnableIrq,
   ksz8863DisableIrq,
   ksz8863EventHandler,
   ksz8863TagFrame,
   ksz8863UntagFrame
};


/**
 * @brief Tail tag rules (host to KSZ8863)
 **/

const uint8_t ksz8863IngressTailTag[3] =
{
   0,
   KSZ8863_TAIL_TAG_ENCODE(1),
   KSZ8863_TAIL_TAG_ENCODE(2)
};


/**
 * @brief KSZ8863 Ethernet switch initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ksz8863Init(NetInterface *interface)
{
   uint_t port;
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   uint8_t temp;
#endif

   //Debug message
   TRACE_INFO("Initializing KSZ8863...\r\n");

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Initialize SPI
      interface->spiDriver->init();
   }

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Wait for the serial interface to be ready
   do
   {
      //Read CHIP_ID0 register
      temp = ksz8863ReadSwitchReg(interface, KSZ8863_CHIP_ID0);

      //The returned data is invalid until the serial interface is ready
   } while(temp != KSZ8863_CHIP_ID0_FAMILY_ID_DEFAULT);

   //Tail tagging mode?
   if(interface->port != 0)
   {
      //Enable tail tag feature
      temp = ksz8863ReadSwitchReg(interface, KSZ8863_GLOBAL_CTRL1);
      temp |= KSZ8863_GLOBAL_CTRL1_TAIL_TAG_EN;
      ksz8863WriteSwitchReg(interface, KSZ8863_GLOBAL_CTRL1, temp);

      //Loop through ports
      for(port = KSZ8863_PORT1; port <= KSZ8863_PORT2; port++)
      {
         //Disable packet transmission and switch address learning
         temp = ksz8863ReadSwitchReg(interface, KSZ8863_PORTn_CTRL2(port));
         temp &= ~KSZ8863_PORTn_CTRL2_TRANSMIT_EN;
         temp |= KSZ8863_PORTn_CTRL2_RECEIVE_EN;
         temp |= KSZ8863_PORTn_CTRL2_LEARNING_DIS;
         ksz8863WriteSwitchReg(interface, KSZ8863_PORTn_CTRL2(port), temp);
      }
   }
   else
   {
      //Disable tail tag feature
      temp = ksz8863ReadSwitchReg(interface, KSZ8863_GLOBAL_CTRL1);
      temp &= ~KSZ8863_GLOBAL_CTRL1_TAIL_TAG_EN;
      ksz8863WriteSwitchReg(interface, KSZ8863_GLOBAL_CTRL1, temp);

      //Loop through ports
      for(port = KSZ8863_PORT1; port <= KSZ8863_PORT2; port++)
      {
         //Enable transmission, reception and switch address learning
         temp = ksz8863ReadSwitchReg(interface, KSZ8863_PORTn_CTRL2(port));
         temp |= KSZ8863_PORTn_CTRL2_TRANSMIT_EN;
         temp |= KSZ8863_PORTn_CTRL2_RECEIVE_EN;
         temp &= ~KSZ8863_PORTn_CTRL2_LEARNING_DIS;
         ksz8863WriteSwitchReg(interface, KSZ8863_PORTn_CTRL2(port), temp);
      }
   }

   //Dump switch registers for debugging purpose
   ksz8863DumpSwitchReg(interface);
#endif

   //SMI interface mode?
   if(interface->spiDriver == NULL)
   {
      //Loop through ports
      for(port = KSZ8863_PORT1; port <= KSZ8863_PORT2; port++)
      {
         //Debug message
         TRACE_DEBUG("Port %u:\r\n", port);
         //Dump PHY registers for debugging purpose
         ksz8863DumpPhyReg(interface, port);
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

bool_t ksz8863GetLinkState(NetInterface *interface, uint8_t port)
{
   uint16_t status;
   bool_t linkState;

   //Check port number
   if(port >= KSZ8863_PORT1 && port <= KSZ8863_PORT2)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //SPI slave mode?
      if(interface->spiDriver != NULL)
      {
         //Read port status 0 register
         status = ksz8863ReadSwitchReg(interface, KSZ8863_PORTn_STAT0(port));

         //Retrieve current link state
         linkState = (status & KSZ8863_PORTn_STAT0_LINK_GOOD) ? TRUE : FALSE;
      }
      else
      {
         //Read status register
         status = ksz8863ReadPhyReg(interface, port, KSZ8863_BMSR);

         //Retrieve current link state
         linkState = (status & KSZ8863_BMSR_LINK_STATUS) ? TRUE : FALSE;
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
 * @brief KSZ8863 timer handler
 * @param[in] interface Underlying network interface
 **/

void ksz8863Tick(NetInterface *interface)
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
            if(port >= KSZ8863_PORT1 && port <= KSZ8863_PORT2)
            {
               //Read port status 0 register
               status = ksz8863ReadSwitchReg(interface,
                  KSZ8863_PORTn_STAT0(port));

               //Retrieve current link state
               linkState = (status & KSZ8863_PORTn_STAT0_LINK_GOOD) ? TRUE : FALSE;

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
      for(port = KSZ8863_PORT1; port <= KSZ8863_PORT2; port++)
      {
         //SPI slave mode?
         if(interface->spiDriver != NULL)
         {
            //Read port status 0 register
            status = ksz8863ReadSwitchReg(interface, KSZ8863_PORTn_STAT0(port));

            //Retrieve current link state
            if(status & KSZ8863_PORTn_STAT0_LINK_GOOD)
               linkState = TRUE;
         }
         else
         {
            //Read status register
            status = ksz8863ReadPhyReg(interface, port, KSZ8863_BMSR);

            //Retrieve current link state
            if(status & KSZ8863_BMSR_LINK_STATUS)
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

void ksz8863EnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void ksz8863DisableIrq(NetInterface *interface)
{
}


/**
 * @brief KSZ8863 event handler
 * @param[in] interface Underlying network interface
 **/

void ksz8863EventHandler(NetInterface *interface)
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
            if(port >= KSZ8863_PORT1 && port <= KSZ8863_PORT2)
            {
               //Read port status 0 register
               status = ksz8863ReadSwitchReg(interface,
                  KSZ8863_PORTn_STAT0(port));

               //Retrieve current link state
               linkState = (status & KSZ8863_PORTn_STAT0_LINK_GOOD) ? TRUE : FALSE;

               //Link up event?
               if(linkState && !virtualInterface->linkState)
               {
                  //Adjust MAC configuration parameters for proper operation
                  interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                  interface->duplexMode = NIC_FULL_DUPLEX_MODE;
                  interface->nicDriver->updateMacConfig(interface);

                  //Read port status 1 register
                  status = ksz8863ReadSwitchReg(interface,
                     KSZ8863_PORTn_STAT1(port));

                  //Check current speed
                  if(status & KSZ8863_PORTn_STAT1_OP_SPEED)
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                  else
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_10MBPS;

                  //Check duplex mode
                  if(status & KSZ8863_PORTn_STAT1_OP_DUPLEX)
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
      for(port = KSZ8863_PORT1; port <= KSZ8863_PORT2; port++)
      {
         //SPI slave mode?
         if(interface->spiDriver != NULL)
         {
            //Read port status 0 register
            status = ksz8863ReadSwitchReg(interface, KSZ8863_PORTn_STAT0(port));

            //Retrieve current link state
            if(status & KSZ8863_PORTn_STAT0_LINK_GOOD)
               linkState = TRUE;
         }
         else
         {
            //Read status register
            status = ksz8863ReadPhyReg(interface, port, KSZ8863_BMSR);

            //Retrieve current link state
            if(status & KSZ8863_BMSR_LINK_STATUS)
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

error_t ksz8863TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type)
{
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   error_t error;
   size_t length;
   const uint8_t *tailTag;

   //Valid port?
   if(port >= KSZ8863_PORT1 && port <= KSZ8863_PORT2)
   {
      //The one byte tail tagging is used to indicate the destination port
      tailTag = &ksz8863IngressTailTag[port];

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

error_t ksz8863UntagFrame(NetInterface *interface, uint8_t **frame,
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
      *port = KSZ8863_TAIL_TAG_DECODE(*tailTag);

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

void ksz8863WritePhyReg(NetInterface *interface, uint8_t port,
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

uint16_t ksz8863ReadPhyReg(NetInterface *interface, uint8_t port,
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

void ksz8863DumpPhyReg(NetInterface *interface, uint8_t port)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         ksz8863ReadPhyReg(interface, port, i));
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

void ksz8863WriteSwitchReg(NetInterface *interface, uint8_t address,
   uint8_t data)
{
   uint8_t phyAddr;
   uint8_t regAddr;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Set up a write operation
      interface->spiDriver->transfer(KSZ8863_SPI_CMD_WRITE);
      //Write register address
      interface->spiDriver->transfer(address);

      //Write data
      interface->spiDriver->transfer(data);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //SMI register read access is selected when opcode is set to 0 and
      //bit 4 of the PHY address is set to 0
      phyAddr = (address >> 5) & 0x07;

      //Register address field forms register address bits 4:0
      regAddr = address & 0x1F;

      //Registers are 8 data bits wide. For write operation, data bits 15:8
      //are not defined, and hence can be set to either zeroes or ones
      interface->nicDriver->writePhyReg(SMI_OPCODE_0, phyAddr, regAddr,
         data);
   }
}


/**
 * @brief Read switch register
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @return Register value
 **/

uint8_t ksz8863ReadSwitchReg(NetInterface *interface, uint8_t address)
{
   uint8_t phyAddr;
   uint8_t regAddr;
   uint8_t data;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Set up a read operation
      interface->spiDriver->transfer(KSZ8863_SPI_CMD_READ);
      //Write register address
      interface->spiDriver->transfer(address);

      //Read data
      data = interface->spiDriver->transfer(0xFF);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //SMI register read access is selected when opcode is set to 0 and
      //bit 4 of the PHY address is set to 1
      phyAddr = 0x10 | ((address >> 5) & 0x07);

      //Register address field forms register address bits 4:0
      regAddr = address & 0x1F;

      //Registers are 8 data bits wide. For read operation, data bits 15:8
      //are read back as zeroes
      data = interface->nicDriver->readPhyReg(SMI_OPCODE_0, phyAddr,
         regAddr) & 0xFF;
   }

   //Return register value
   return data;
}


/**
 * @brief Dump switch registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void ksz8863DumpSwitchReg(NetInterface *interface)
{
   uint16_t i;

   //Loop through switch registers
   for(i = 0; i < 256; i++)
   {
      //Display current switch register
      TRACE_DEBUG("0x%02" PRIX16 " (%02" PRIu16 ") : 0x%02" PRIX8 "\r\n",
         i, i, ksz8863ReadSwitchReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
