/**
 * @file ksz8873_driver.c
 * @brief KSZ8873 Ethernet switch
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
#include "core/net.h"
#include "drivers/switch/ksz8873_driver.h"
#include "debug.h"


/**
 * @brief KSZ8873 Ethernet switch driver
 **/

const PhyDriver ksz8873PhyDriver =
{
   ksz8873Init,
   ksz8873Tick,
   ksz8873EnableIrq,
   ksz8873DisableIrq,
   ksz8873EventHandler,
};


/**
 * @brief KSZ8873 Ethernet switch initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ksz8873Init(NetInterface *interface)
{
   uint_t port;

   //Debug message
   TRACE_INFO("Initializing KSZ8873...\r\n");

   //Loop through ports
   for(port = KSZ8873_PORT1; port <= KSZ8873_PORT2; port++)
   {
      //Debug message
      TRACE_INFO("Port %u:\r\n", port);
      //Dump PHY registers for debugging purpose
      ksz8873DumpPhyReg(interface, port);
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

bool_t ksz8873GetLinkState(NetInterface *interface, uint8_t port)
{
   uint16_t status;
   bool_t linkState;

   //Check port number
   if(port >= KSZ8873_PORT1 && port <= KSZ8873_PORT2)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);
      //Read status register
      status = ksz8873ReadPhyReg(interface, port, KSZ8873_PHY_REG_BMSR);
      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Retrieve current link state
      linkState = (status & BMSR_LINK_STATUS) ? TRUE : FALSE;
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
 * @brief KSZ8873 timer handler
 * @param[in] interface Underlying network interface
 **/

void ksz8873Tick(NetInterface *interface)
{
   uint_t port;
   uint16_t status;
   bool_t linkState;

   //Initialize link state
   linkState = FALSE;

   //Loop through ports
   for(port = KSZ8873_PORT1; port <= KSZ8873_PORT2; port++)
   {
      //Read status register
      status = ksz8873ReadPhyReg(interface, port, KSZ8873_PHY_REG_BMSR);

      //Retrieve current link state
      if(status & BMSR_LINK_STATUS)
         linkState = TRUE;
   }

   //Link up event?
   if(linkState)
   {
      if(!interface->linkState)
      {
         //Set event flag
         interface->phyEvent = TRUE;
         //Notify the TCP/IP stack of the event
         osSetEvent(&netEvent);
      }
   }
   //Link down event?
   else
   {
      if(interface->linkState)
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

void ksz8873EnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void ksz8873DisableIrq(NetInterface *interface)
{
}


/**
 * @brief KSZ8873 event handler
 * @param[in] interface Underlying network interface
 **/

void ksz8873EventHandler(NetInterface *interface)
{
   uint_t port;
   uint16_t status;
   bool_t linkState;

   //Initialize link state
   linkState = FALSE;

   //Loop through ports
   for(port = KSZ8873_PORT1; port <= KSZ8873_PORT2; port++)
   {
      //Read status register
      status = ksz8873ReadPhyReg(interface, port, KSZ8873_PHY_REG_BMSR);

      //Retrieve current link state
      if(status & BMSR_LINK_STATUS)
         linkState = TRUE;
   }

   //Link up event?
   if(linkState)
   {
      //Set current speed
      interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
      //Set duplex mode
      interface->duplexMode = NIC_FULL_DUPLEX_MODE;

      //Update link state
      interface->linkState = TRUE;

      //Adjust MAC configuration parameters for proper operation
      interface->nicDriver->updateMacConfig(interface);
   }
   else
   {
      //Update link state
      interface->linkState = FALSE;
   }

   //Process link state change event
   nicNotifyLinkChange(interface);
}


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void ksz8873WritePhyReg(NetInterface *interface,
   uint8_t port, uint8_t address, uint16_t data)
{
   //Write the specified PHY register
   interface->nicDriver->writePhyReg(port, address, data);
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t ksz8873ReadPhyReg(NetInterface *interface,
   uint8_t port, uint8_t address)
{
   //Read the specified PHY register
   return interface->nicDriver->readPhyReg(port, address);
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 **/

void ksz8873DumpPhyReg(NetInterface *interface, uint8_t port)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i, ksz8873ReadPhyReg(interface, port, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
