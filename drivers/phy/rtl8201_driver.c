/**
 * @file rtl8201_driver.c
 * @brief RTL8201 Gigabit Ethernet PHY driver
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
#include "drivers/phy/rtl8201_driver.h"
#include "debug.h"


/**
 * @brief RTL8201 Ethernet PHY driver
 **/

const PhyDriver rtl8201PhyDriver =
{
   rtl8201Init,
   rtl8201Tick,
   rtl8201EnableIrq,
   rtl8201DisableIrq,
   rtl8201EventHandler
};


/**
 * @brief RTL8201 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t rtl8201Init(NetInterface *interface)
{
   uint16_t temp;

   //Debug message
   TRACE_INFO("Initializing RTL8201...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = RTL8201_PHY_ADDR;
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
   rtl8201WritePhyReg(interface, RTL8201_BMCR, RTL8201_BMCR_RESET);

   //Wait for the reset to complete
   while(rtl8201ReadPhyReg(interface, RTL8201_BMCR) & RTL8201_BMCR_RESET)
   {
   }

   //Dump PHY registers for debugging purpose
   rtl8201DumpPhyReg(interface);

   //Select page 7
   rtl8201WritePhyReg(interface, RTL8201_PDR, 7);

   //The PHY will generate interrupts when link status changes are detected
   temp = rtl8201ReadPhyReg(interface, RTL8201_IWELFR);
   temp |= RTL8201_IWELFR_INT_LINK_CHG;
   rtl8201WritePhyReg(interface, RTL8201_IWELFR, temp);

   //Select page 0
   rtl8201WritePhyReg(interface, RTL8201_PDR, 0);

   //Perform custom configuration
   rtl8201InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief RTL8201 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void rtl8201InitHook(NetInterface *interface)
{
}


/**
 * @brief RTL8201 timer handler
 * @param[in] interface Underlying network interface
 **/

void rtl8201Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read basic status register
      value = rtl8201ReadPhyReg(interface, RTL8201_BMSR);
      //Retrieve current link state
      linkState = (value & RTL8201_BMSR_LINK_STATUS) ? TRUE : FALSE;

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

void rtl8201EnableIrq(NetInterface *interface)
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

void rtl8201DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief RTL8201 event handler
 * @param[in] interface Underlying network interface
 **/

void rtl8201EventHandler(NetInterface *interface)
{
   uint16_t value;

   //Read status register to acknowledge the interrupt
   value = rtl8201ReadPhyReg(interface, RTL8201_IISDR);

   //Link status change?
   if((value & RTL8201_IISDR_LINK_STATUS_CHG) != 0)
   {
      //Any link failure condition is latched in the BMSR register. Reading
      //the register twice will always return the actual link status
      value = rtl8201ReadPhyReg(interface, RTL8201_BMSR);
      value = rtl8201ReadPhyReg(interface, RTL8201_BMSR);

      //Link is up?
      if((value & RTL8201_BMSR_LINK_STATUS) != 0)
      {
         //Read BMCR register
         value = rtl8201ReadPhyReg(interface, RTL8201_BMCR);

         //Check current speed
         if((value & RTL8201_BMCR_SPEED_SEL_LSB) != 0)
         {
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
         }
         else
         {
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
         }

         //Check current duplex mode
         if((value & RTL8201_BMCR_DUPLEX_MODE) != 0)
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
}


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void rtl8201WritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t rtl8201ReadPhyReg(NetInterface *interface, uint8_t address)
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

void rtl8201DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         rtl8201ReadPhyReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Write MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @param[in] data MMD register value
 **/

void rtl8201WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data)
{
   //Select register operation
   rtl8201WritePhyReg(interface, RTL8201_MACR,
      RTL8201_MACR_FUNC_ADDR | (devAddr & RTL8201_MACR_DEVAD));

   //Write MMD register address
   rtl8201WritePhyReg(interface, RTL8201_MAADR, regAddr);

   //Select data operation
   rtl8201WritePhyReg(interface, RTL8201_MACR,
      RTL8201_MACR_FUNC_DATA_NO_POST_INC | (devAddr & RTL8201_MACR_DEVAD));

   //Write the content of the MMD register
   rtl8201WritePhyReg(interface, RTL8201_MAADR, data);
}


/**
 * @brief Read MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return MMD register value
 **/

uint16_t rtl8201ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr)
{
   //Select register operation
   rtl8201WritePhyReg(interface, RTL8201_MACR,
      RTL8201_MACR_FUNC_ADDR | (devAddr & RTL8201_MACR_DEVAD));

   //Write MMD register address
   rtl8201WritePhyReg(interface, RTL8201_MAADR, regAddr);

   //Select data operation
   rtl8201WritePhyReg(interface, RTL8201_MACR,
      RTL8201_MACR_FUNC_DATA_NO_POST_INC | (devAddr & RTL8201_MACR_DEVAD));

   //Read the content of the MMD register
   return rtl8201ReadPhyReg(interface, RTL8201_MAADR);
}
