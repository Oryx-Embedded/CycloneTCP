/**
 * @file dm9162_driver.c
 * @brief DM9162 Ethernet PHY driver
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
#include "drivers/phy/dm9162_driver.h"
#include "debug.h"


/**
 * @brief DM9162 Ethernet PHY driver
 **/

const PhyDriver dm9162PhyDriver =
{
   dm9162Init,
   dm9162Tick,
   dm9162EnableIrq,
   dm9162DisableIrq,
   dm9162EventHandler
};


/**
 * @brief DM9162 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t dm9162Init(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing DM9162...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = DM9162_PHY_ADDR;
   }

   //Initialize serial management interface
   if(interface->smiDriver != NULL)
   {
      interface->smiDriver->init();
   }

   //Initialize external interrupt line driver
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->init();
   }

   //Reset PHY transceiver
   dm9162WritePhyReg(interface, DM9162_BMCR, DM9162_BMCR_RESET);

   //Wait for the reset to complete
   while(dm9162ReadPhyReg(interface, DM9162_BMCR) & DM9162_BMCR_RESET)
   {
   }

   //Dump PHY registers for debugging purpose
   dm9162DumpPhyReg(interface);

   //The PHY will generate interrupts when link status changes are detected
   dm9162WritePhyReg(interface, DM9162_MDINTR, ~(DM9162_MDINTR_LINK_MASK |
      DM9162_MDINTR_INTR_MASK));

   //Perform custom configuration
   dm9162InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief DM9162 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void dm9162InitHook(NetInterface *interface)
{
}


/**
 * @brief DM9162 timer handler
 * @param[in] interface Underlying network interface
 **/

void dm9162Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read basic status register
      value = dm9162ReadPhyReg(interface, DM9162_BMSR);
      //Retrieve current link state
      linkState = (value & DM9162_BMSR_LINK_STATUS) ? TRUE : FALSE;

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
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void dm9162EnableIrq(NetInterface *interface)
{
   //Enable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->enableIrq();
   }
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void dm9162DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief DM9162 event handler
 * @param[in] interface Underlying network interface
 **/

void dm9162EventHandler(NetInterface *interface)
{
   uint16_t value;
   bool_t end;

   //Read status register to acknowledge the interrupt
   value = dm9162ReadPhyReg(interface, DM9162_MDINTR);

   //Link status change?
   if((value & DM9162_MDINTR_LINK_CHANGE) != 0)
   {
      //Any link failure condition is latched in the BMSR register. Reading
      //the register twice will always return the actual link status
      value = dm9162ReadPhyReg(interface, DM9162_BMSR);
      value = dm9162ReadPhyReg(interface, DM9162_BMSR);

      //Link is up?
      if((value & DM9162_BMSR_LINK_STATUS) != 0)
      {
         //Wait for the auto-negotiation to complete
         do
         {
            //Read DSCSR register
            value = dm9162ReadPhyReg(interface, DM9162_DSCSR);

            //Check current state
            switch(value & DM9162_DSCSR_ANMB)
            {
            //Auto-negotiation is still in progress?
            case DM9162_DSCSR_ANMB_ABILITY_MATCH:
            case DM9162_DSCSR_ANMB_ACK_MATCH:
            case DM9162_DSCSR_ANMB_CONSIST_MATCH:
            case DM9162_DSCSR_ANMB_LINK_READY:
               end = FALSE;
               break;
            //Auto-negotiation is complete?
            default:
               end = TRUE;
               break;
            }

            //Check loop condition variable
         } while(!end);

         //Read DSCSR register
         value = dm9162ReadPhyReg(interface, DM9162_DSCSR);

         //Check current operation mode
         if((value & DM9162_DSCSR_10HDX) != 0)
         {
            //10BASE-T half-duplex
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;
         }
         else if((value & DM9162_DSCSR_10FDX) != 0)
         {
            //10BASE-T full-duplex
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
         }
         else if((value & DM9162_DSCSR_100HDX) != 0)
         {
            //100BASE-TX half-duplex
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;
         }
         else if((value & DM9162_DSCSR_100FDX) != 0)
         {
            //100BASE-TX full-duplex
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
         }
         else
         {
            //Debug message
            TRACE_WARNING("Invalid operation mode!\r\n");
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
}


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void dm9162WritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t dm9162ReadPhyReg(NetInterface *interface, uint8_t address)
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

void dm9162DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         dm9162ReadPhyReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
