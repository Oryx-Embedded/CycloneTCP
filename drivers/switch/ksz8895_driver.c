/**
 * @file ksz8895_driver.c
 * @brief KSZ8895 5-port Ethernet switch
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
#include "drivers/switch/ksz8895_driver.h"
#include "debug.h"


/**
 * @brief KSZ8895 Ethernet switch driver
 **/

const PhyDriver ksz8895PhyDriver =
{
   ksz8895Init,
   ksz8895Tick,
   ksz8895EnableIrq,
   ksz8895DisableIrq,
   ksz8895EventHandler,
   ksz8895TagFrame,
   ksz8895UntagFrame
};


/**
 * @brief Tail tag rules (host to KSZ8895)
 **/

const uint8_t ksz8895IngressTailTag[5] =
{
   0,
   KSZ8895_TAIL_TAG_ENCODE(1),
   KSZ8895_TAIL_TAG_ENCODE(2),
   KSZ8895_TAIL_TAG_ENCODE(3),
   KSZ8895_TAIL_TAG_ENCODE(4)
};


/**
 * @brief KSZ8895 Ethernet switch initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ksz8895Init(NetInterface *interface)
{
   uint_t port;
   uint8_t temp;

   //Debug message
   TRACE_INFO("Initializing KSZ8895...\r\n");

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Initialize SPI
      interface->spiDriver->init();
   }

   //Wait for the serial interface to be ready
   do
   {
      //Read CHIP_ID0 register
      temp = ksz8895ReadSwitchReg(interface, KSZ8895_CHIP_ID0);

      //The returned data is invalid until the serial interface is ready
   } while(temp != KSZ8895_CHIP_ID0_FAMILY_ID_DEFAULT);

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Tail tagging mode?
   if(interface->port != 0)
   {
      //Enable tail tag feature
      temp = ksz8895ReadSwitchReg(interface, KSZ8895_GLOBAL_CTRL10);
      temp |= KSZ8895_GLOBAL_CTRL10_TAIL_TAG_EN;
      ksz8895WriteSwitchReg(interface, KSZ8895_GLOBAL_CTRL10, temp);

      //Loop through ports
      for(port = KSZ8895_PORT1; port <= KSZ8895_PORT4; port++)
      {
         //Disable packet transmission and switch address learning
         temp = ksz8895ReadSwitchReg(interface, KSZ8895_PORTn_CTRL2(port));
         temp &= ~KSZ8895_PORTn_CTRL2_TRANSMIT_EN;
         temp |= KSZ8895_PORTn_CTRL2_RECEIVE_EN;
         temp |= KSZ8895_PORTn_CTRL2_LEARNING_DIS;
         ksz8895WriteSwitchReg(interface, KSZ8895_PORTn_CTRL2(port), temp);
      }
   }
   else
#endif
   {
      //Disable tail tag feature
      temp = ksz8895ReadSwitchReg(interface, KSZ8895_GLOBAL_CTRL10);
      temp &= ~KSZ8895_GLOBAL_CTRL10_TAIL_TAG_EN;
      ksz8895WriteSwitchReg(interface, KSZ8895_GLOBAL_CTRL10, temp);

      //Loop through ports
      for(port = KSZ8895_PORT1; port <= KSZ8895_PORT4; port++)
      {
         //Enable transmission, reception and switch address learning
         temp = ksz8895ReadSwitchReg(interface, KSZ8895_PORTn_CTRL2(port));
         temp |= KSZ8895_PORTn_CTRL2_TRANSMIT_EN;
         temp |= KSZ8895_PORTn_CTRL2_RECEIVE_EN;
         temp &= ~KSZ8895_PORTn_CTRL2_LEARNING_DIS;
         ksz8895WriteSwitchReg(interface, KSZ8895_PORTn_CTRL2(port), temp);
      }
   }

   //Start switch operation
   ksz8895WriteSwitchReg(interface, KSZ8895_CHIP_ID1,
      KSZ8895_CHIP_ID1_START_SWITCH);

   //Dump switch registers for debugging purpose
   ksz8895DumpSwitchReg(interface);

   //SMI interface mode?
   if(interface->spiDriver == NULL)
   {
      //Loop through ports
      for(port = KSZ8895_PORT1; port <= KSZ8895_PORT4; port++)
      {
         //Debug message
         TRACE_DEBUG("Port %u:\r\n", port);
         //Dump PHY registers for debugging purpose
         ksz8895DumpPhyReg(interface, port);
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

bool_t ksz8895GetLinkState(NetInterface *interface, uint8_t port)
{
   uint16_t status;
   bool_t linkState;

   //Check port number
   if(port >= KSZ8895_PORT1 && port <= KSZ8895_PORT4)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);
      //Read port status 1 register
      status = ksz8895ReadSwitchReg(interface, KSZ8895_PORTn_STAT1(port));
      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Retrieve current link state
      linkState = (status & KSZ8895_PORTn_STAT1_LINK_GOOD) ? TRUE : FALSE;
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
 * @brief KSZ8895 timer handler
 * @param[in] interface Underlying network interface
 **/

void ksz8895Tick(NetInterface *interface)
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
            if(port >= KSZ8895_PORT1 && port <= KSZ8895_PORT4)
            {
               //Read port status 1 register
               status = ksz8895ReadSwitchReg(interface,
                  KSZ8895_PORTn_STAT1(port));

               //Retrieve current link state
               linkState = (status & KSZ8895_PORTn_STAT1_LINK_GOOD) ? TRUE : FALSE;

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
      for(port = KSZ8895_PORT1; port <= KSZ8895_PORT4; port++)
      {
         //Read port status 1 register
         status = ksz8895ReadSwitchReg(interface, KSZ8895_PORTn_STAT1(port));

         //Retrieve current link state
         if(status & KSZ8895_PORTn_STAT1_LINK_GOOD)
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

void ksz8895EnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void ksz8895DisableIrq(NetInterface *interface)
{
}


/**
 * @brief KSZ8895 event handler
 * @param[in] interface Underlying network interface
 **/

void ksz8895EventHandler(NetInterface *interface)
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
            if(port >= KSZ8895_PORT1 && port <= KSZ8895_PORT4)
            {
               //Read port status 1 register
               status = ksz8895ReadSwitchReg(interface,
                  KSZ8895_PORTn_STAT1(port));

               //Retrieve current link state
               linkState = (status & KSZ8895_PORTn_STAT1_LINK_GOOD) ? TRUE : FALSE;

               //Link up event?
               if(linkState && !virtualInterface->linkState)
               {
                  //Adjust MAC configuration parameters for proper operation
                  interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                  interface->duplexMode = NIC_FULL_DUPLEX_MODE;
                  interface->nicDriver->updateMacConfig(interface);

                  //Read port status 2 register
                  status = ksz8895ReadSwitchReg(interface,
                     KSZ8895_PORTn_CTRL7_STAT2(port));

                  //Check current operation mode
                  switch(status & KSZ8895_PORTn_CTRL7_STAT2_OP_MODE)
                  {
                  //10BASE-T half-duplex
                  case KSZ8895_PORTn_CTRL7_STAT2_OP_MODE_10BT_HD:
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_10MBPS;
                     virtualInterface->duplexMode = NIC_HALF_DUPLEX_MODE;
                     break;
                  //10BASE-T full-duplex
                  case KSZ8895_PORTn_CTRL7_STAT2_OP_MODE_10BT_FD:
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_10MBPS;
                     virtualInterface->duplexMode = NIC_FULL_DUPLEX_MODE;
                     break;
                  //100BASE-TX half-duplex
                  case KSZ8895_PORTn_CTRL7_STAT2_OP_MODE_100BTX_HD:
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                     virtualInterface->duplexMode = NIC_HALF_DUPLEX_MODE;
                     break;
                  //100BASE-TX full-duplex
                  case KSZ8895_PORTn_CTRL7_STAT2_OP_MODE_100BTX_FD:
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                     virtualInterface->duplexMode = NIC_FULL_DUPLEX_MODE;
                     break;
                  //Unknown operation mode
                  default:
                     //Debug message
                     TRACE_WARNING("Invalid operation mode!\r\n");
                     break;
                  }

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
      for(port = KSZ8895_PORT1; port <= KSZ8895_PORT4; port++)
      {
         //Read port status 1 register
         status = ksz8895ReadSwitchReg(interface, KSZ8895_PORTn_STAT1(port));

         //Retrieve current link state
         if(status & KSZ8895_PORTn_STAT1_LINK_GOOD)
            linkState = TRUE;
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

error_t ksz8895TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type)
{
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   error_t error;
   size_t length;
   const uint8_t *tailTag;

   //Valid port?
   if(port >= KSZ8895_PORT1 && port <= KSZ8895_PORT4)
   {
      //The one byte tail tagging is used to indicate the destination port
      tailTag = &ksz8895IngressTailTag[port];

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

error_t ksz8895UntagFrame(NetInterface *interface, uint8_t **frame,
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
      *port = KSZ8895_TAIL_TAG_DECODE(*tailTag);

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

void ksz8895WritePhyReg(NetInterface *interface, uint8_t port,
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

uint16_t ksz8895ReadPhyReg(NetInterface *interface, uint8_t port,
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

void ksz8895DumpPhyReg(NetInterface *interface, uint8_t port)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         ksz8895ReadPhyReg(interface, port, i));
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

void ksz8895WriteSwitchReg(NetInterface *interface, uint8_t address,
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
      interface->spiDriver->transfer(KSZ8895_SPI_CMD_WRITE);
      //Write register address
      interface->spiDriver->transfer(address);

      //Write data
      interface->spiDriver->transfer(data);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //SMI register write access is selected when opcode is set to 10 and
      //bits 2:1 of the PHY address are set to 11
      phyAddr = 0x06 | ((address >> 3) & 0x18) | ((address >> 5) & 0x01);

      //Register address field forms register address bits 4:0
      regAddr = address & 0x1F;

      //Registers are 8 data bits wide. For write operation, data bits 15:8
      //are not defined, and hence can be set to either zeroes or ones
      interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE, phyAddr, regAddr,
         data);
   }
}


/**
 * @brief Read switch register
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @return Register value
 **/

uint8_t ksz8895ReadSwitchReg(NetInterface *interface, uint8_t address)
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
      interface->spiDriver->transfer(KSZ8895_SPI_CMD_READ);
      //Write register address
      interface->spiDriver->transfer(address);

      //Read data
      data = interface->spiDriver->transfer(0xFF);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //SMI register read access is selected when opcode is set to 10 and
      //bits 2:1 of the PHY address are set to 11
      phyAddr = 0x06 | ((address >> 3) & 0x18) | ((address >> 5) & 0x01);

      //Register address field forms register address bits 4:0
      regAddr = address & 0x1F;

      //Registers are 8 data bits wide. For read operation, data bits 15:8
      //are read back as zeroes
      data = interface->nicDriver->readPhyReg(SMI_OPCODE_READ, phyAddr,
         regAddr) & 0xFF;
   }

   //Return register value
   return data;
}


/**
 * @brief Dump switch registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void ksz8895DumpSwitchReg(NetInterface *interface)
{
   uint16_t i;

   //Loop through switch registers
   for(i = 0; i < 256; i++)
   {
      //Display current switch register
      TRACE_DEBUG("0x%02" PRIX16 " (%02" PRIu16 ") : 0x%02" PRIX8 "\r\n",
         i, i, ksz8895ReadSwitchReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
