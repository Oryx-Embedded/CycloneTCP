/**
 * @file ics1894_driver.c
 * @brief ICS1894-32 Ethernet PHY driver
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
#include "core/net.h"
#include "drivers/phy/ics1894_driver.h"
#include "debug.h"


/**
 * @brief ICS1894 Ethernet PHY driver
 **/

const PhyDriver ics1894PhyDriver =
{
   ics1894Init,
   ics1894Tick,
   ics1894EnableIrq,
   ics1894DisableIrq,
   ics1894EventHandler
};


/**
 * @brief ICS1894 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ics1894Init(NetInterface *interface)
{
   uint16_t temp;

   //Debug message
   TRACE_INFO("Initializing ICS1894...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = ICS1894_PHY_ADDR;
   }

   //Initialize serial management interface
   if(interface->smiDriver != NULL)
   {
      interface->smiDriver->init();
   }

   //Reset PHY transceiver
   ics1894WritePhyReg(interface, ICS1894_BMCR, ICS1894_BMCR_RESET);

   //Wait for the reset to complete
   while(ics1894ReadPhyReg(interface, ICS1894_BMCR) & ICS1894_BMCR_RESET)
   {
   }

   //Dump PHY registers for debugging purpose
   ics1894DumpPhyReg(interface);

   //The PHY supports full-duplex operation only
   ics1894WritePhyReg(interface, ICS1894_ANAR, ICS1894_ANAR_100BTX_FD |
      ICS1894_ANAR_10BT_FD | ICS1894_ANAR_SELECTOR_DEFAULT);

   //Configure LED0 mode
   temp = ics1894ReadPhyReg(interface, ICS1894_ECR3);
   temp &= ~ICS1894_ECR3_LED0_MODE;
   temp |= ICS1894_ECR3_LED0_MODE_LINK_STAT;
   ics1894WritePhyReg(interface, ICS1894_ECR3, temp);

   //Configure LED1 mode
   temp = ics1894ReadPhyReg(interface, ICS1894_ECR3);
   temp &= ~ICS1894_ECR3_LED1_MODE;
   temp |= ICS1894_ECR3_LED1_MODE_ACT;
   ics1894WritePhyReg(interface, ICS1894_ECR3, temp);

   //Perform custom configuration
   ics1894InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief ICS1894 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void ics1894InitHook(NetInterface *interface)
{
}


/**
 * @brief ICS1894 timer handler
 * @param[in] interface Underlying network interface
 **/

void ics1894Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //Read basic status register
   value = ics1894ReadPhyReg(interface, ICS1894_BMSR);
   //Retrieve current link state
   linkState = (value & ICS1894_BMSR_LINK_STATUS) ? TRUE : FALSE;

   //Link up event?
   if(linkState && !interface->linkState)
   {
      //Set event flag
      interface->phyEvent = TRUE;
      //Notify the TCP/IP stack of the event
      osSetEvent(&netEvent);
   }
   //Link down event?
   else if(!linkState && interface->linkState)
   {
      //Set event flag
      interface->phyEvent = TRUE;
      //Notify the TCP/IP stack of the event
      osSetEvent(&netEvent);
   }
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void ics1894EnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void ics1894DisableIrq(NetInterface *interface)
{
}


/**
 * @brief ICS1894 event handler
 * @param[in] interface Underlying network interface
 **/

void ics1894EventHandler(NetInterface *interface)
{
   uint16_t value;

   //Read Quick Poll Detailed Status register
   value = ics1894ReadPhyReg(interface, ICS1894_QPDSR);

   //Link is up?
   if((value & ICS1894_QPDSR_LINK_STATUS) != 0)
   {
      //Check current speed
      if((value & ICS1894_QPDSR_DATA_RATE) != 0)
      {
         interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
      }
      else
      {
         interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
      }

      //Check current duplex mode
      if((value & ICS1894_QPDSR_DUPLEX) != 0)
      {
         interface->duplexMode = NIC_FULL_DUPLEX_MODE;
      }
      else
      {
         interface->duplexMode = NIC_HALF_DUPLEX_MODE;
      }

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
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void ics1894WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data)
{
   //Write the specified PHY register
   if(interface->smiDriver != NULL)
   {
      interface->smiDriver->writePhyReg(SMI_OPCODE_WRITE,
         interface->phyAddr, address, data);
   }
   else
   {
      interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE,
         interface->phyAddr, address, data);
   }
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t ics1894ReadPhyReg(NetInterface *interface, uint8_t address)
{
   uint16_t data;

   //Read the specified PHY register
   if(interface->smiDriver != NULL)
   {
      data = interface->smiDriver->readPhyReg(SMI_OPCODE_READ,
         interface->phyAddr, address);
   }
   else
   {
      data = interface->nicDriver->readPhyReg(SMI_OPCODE_READ,
         interface->phyAddr, address);
   }

   //Return the value of the PHY register
   return data;
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void ics1894DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         ics1894ReadPhyReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
