/**
 * @file ksz8051_driver.c
 * @brief KSZ8051 Ethernet PHY driver
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
#include "drivers/phy/ksz8051_driver.h"
#include "debug.h"


/**
 * @brief KSZ8051 Ethernet PHY driver
 **/

const PhyDriver ksz8051PhyDriver =
{
   ksz8051Init,
   ksz8051Tick,
   ksz8051EnableIrq,
   ksz8051DisableIrq,
   ksz8051EventHandler
};


/**
 * @brief KSZ8051 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ksz8051Init(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing KSZ8051...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = KSZ8051_PHY_ADDR;
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
   ksz8051WritePhyReg(interface, KSZ8051_BMCR, KSZ8051_BMCR_RESET);

   //Wait for the reset to complete
   while(ksz8051ReadPhyReg(interface, KSZ8051_BMCR) & KSZ8051_BMCR_RESET)
   {
   }

   //Dump PHY registers for debugging purpose
   ksz8051DumpPhyReg(interface);

   //The PHY will generate interrupts when link status changes are detected
   ksz8051WritePhyReg(interface, KSZ8051_ICSR, KSZ8051_ICSR_LINK_DOWN_IE |
      KSZ8051_ICSR_LINK_UP_IE);

   //Perform custom configuration
   ksz8051InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief KSZ8051 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void ksz8051InitHook(NetInterface *interface)
{
}


/**
 * @brief KSZ8051 timer handler
 * @param[in] interface Underlying network interface
 **/

void ksz8051Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read basic status register
      value = ksz8051ReadPhyReg(interface, KSZ8051_BMSR);
      //Retrieve current link state
      linkState = (value & KSZ8051_BMSR_LINK_STATUS) ? TRUE : FALSE;

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

void ksz8051EnableIrq(NetInterface *interface)
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

void ksz8051DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief KSZ8051 event handler
 * @param[in] interface Underlying network interface
 **/

void ksz8051EventHandler(NetInterface *interface)
{
   uint16_t value;

   //Read status register to acknowledge the interrupt
   value = ksz8051ReadPhyReg(interface, KSZ8051_ICSR);

   //Link status change?
   if((value & (KSZ8051_ICSR_LINK_DOWN_IF | KSZ8051_ICSR_LINK_UP_IF)) != 0)
   {
      //Any link failure condition is latched in the BMSR register. Reading
      //the register twice will always return the actual link status
      value = ksz8051ReadPhyReg(interface, KSZ8051_BMSR);
      value = ksz8051ReadPhyReg(interface, KSZ8051_BMSR);

      //Link is up?
      if((value & KSZ8051_BMSR_LINK_STATUS) != 0)
      {
         //Read PHY control register
         value = ksz8051ReadPhyReg(interface, KSZ8051_PHYCON1);

         //Check current operation mode
         switch(value & KSZ8051_PHYCON1_OP_MODE)
         {
         //10BASE-T half-duplex
         case KSZ8051_PHYCON1_OP_MODE_10BT_HD:
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;
            break;

         //10BASE-T full-duplex
         case KSZ8051_PHYCON1_OP_MODE_10BT_FD:
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
            break;

         //100BASE-TX half-duplex
         case KSZ8051_PHYCON1_OP_MODE_100BTX_HD:
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;
            break;

         //100BASE-TX full-duplex
         case KSZ8051_PHYCON1_OP_MODE_100BTX_FD:
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
            break;

         //Unknown operation mode
         default:
            //Debug message
            TRACE_WARNING("Invalid operation mode!\r\n");
            break;
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

void ksz8051WritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t ksz8051ReadPhyReg(NetInterface *interface, uint8_t address)
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

void ksz8051DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         ksz8051ReadPhyReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
