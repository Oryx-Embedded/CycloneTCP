/**
 * @file rtl8211e_driver.c
 * @brief RTL8211E Gigabit Ethernet PHY driver
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
#include "drivers/phy/rtl8211e_driver.h"
#include "debug.h"


/**
 * @brief RTL8211E Ethernet PHY driver
 **/

const PhyDriver rtl8211ePhyDriver =
{
   rtl8211eInit,
   rtl8211eTick,
   rtl8211eEnableIrq,
   rtl8211eDisableIrq,
   rtl8211eEventHandler
};


/**
 * @brief RTL8211E PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t rtl8211eInit(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing RTL8211E...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = RTL8211E_PHY_ADDR;
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
   rtl8211eWritePhyReg(interface, RTL8211E_BMCR, RTL8211E_BMCR_RESET);

   //Wait for the reset to complete
   while(rtl8211eReadPhyReg(interface, RTL8211E_BMCR) & RTL8211E_BMCR_RESET)
   {
   }

   //Dump PHY registers for debugging purpose
   rtl8211eDumpPhyReg(interface);

   //The PHY will generate interrupts when link status changes are detected
   rtl8211eWritePhyReg(interface, RTL8211E_INER, RTL8211E_INER_AN_COMPLETE |
      RTL8211E_INER_LINK_STATUS);

   //Perform custom configuration
   rtl8211eInitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief RTL8211E custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void rtl8211eInitHook(NetInterface *interface)
{
}


/**
 * @brief RTL8211E timer handler
 * @param[in] interface Underlying network interface
 **/

void rtl8211eTick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read basic status register
      value = rtl8211eReadPhyReg(interface, RTL8211E_BMSR);
      //Retrieve current link state
      linkState = (value & RTL8211E_BMSR_LINK_STATUS) ? TRUE : FALSE;

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

void rtl8211eEnableIrq(NetInterface *interface)
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

void rtl8211eDisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief RTL8211E event handler
 * @param[in] interface Underlying network interface
 **/

void rtl8211eEventHandler(NetInterface *interface)
{
   uint16_t status;

   //Read status register to acknowledge the interrupt
   status = rtl8211eReadPhyReg(interface, RTL8211E_INSR);

   //Link status change?
   if((status & (RTL8211E_INSR_AN_COMPLETE | RTL8211E_INSR_LINK_STATUS)) != 0)
   {
      //Any link failure condition is latched in the BMSR register. Reading
      //the register twice will always return the actual link status
      status = rtl8211eReadPhyReg(interface, RTL8211E_BMSR);
      status = rtl8211eReadPhyReg(interface, RTL8211E_BMSR);

      //Link is up?
      if((status & RTL8211E_BMSR_LINK_STATUS) != 0)
      {
         //Read PHY status register
         status = rtl8211eReadPhyReg(interface, RTL8211E_PHYSR);

         //Check current speed
         switch(status & RTL8211E_PHYSR_SPEED)
         {
         //10BASE-T
         case RTL8211E_PHYSR_SPEED_10MBPS:
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
            break;
         //100BASE-TX
         case RTL8211E_PHYSR_SPEED_100MBPS:
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
            break;
         //1000BASE-T
         case RTL8211E_PHYSR_SPEED_1000MBPS:
            interface->linkSpeed = NIC_LINK_SPEED_1GBPS;
            break;
         //Unknown speed
         default:
            //Debug message
            TRACE_WARNING("Invalid speed\r\n");
            break;
         }

         //Check current duplex mode
         if((status & RTL8211E_PHYSR_DUPLEX) != 0)
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

void rtl8211eWritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t rtl8211eReadPhyReg(NetInterface *interface, uint8_t address)
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

void rtl8211eDumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         rtl8211eReadPhyReg(interface, i));
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

void rtl8211eWriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data)
{
   //Select register operation
   rtl8211eWritePhyReg(interface, RTL8211E_MMDACR,
      RTL8211E_MMDACR_FUNC_ADDR | (devAddr & RTL8211E_MMDACR_DEVAD));

   //Write MMD register address
   rtl8211eWritePhyReg(interface, RTL8211E_MMDAADR, regAddr);

   //Select data operation
   rtl8211eWritePhyReg(interface, RTL8211E_MMDACR,
      RTL8211E_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & RTL8211E_MMDACR_DEVAD));

   //Write the content of the MMD register
   rtl8211eWritePhyReg(interface, RTL8211E_MMDAADR, data);
}


/**
 * @brief Read MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return MMD register value
 **/

uint16_t rtl8211eReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr)
{
   //Select register operation
   rtl8211eWritePhyReg(interface, RTL8211E_MMDACR,
      RTL8211E_MMDACR_FUNC_ADDR | (devAddr & RTL8211E_MMDACR_DEVAD));

   //Write MMD register address
   rtl8211eWritePhyReg(interface, RTL8211E_MMDAADR, regAddr);

   //Select data operation
   rtl8211eWritePhyReg(interface, RTL8211E_MMDACR,
      RTL8211E_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & RTL8211E_MMDACR_DEVAD));

   //Read the content of the MMD register
   return rtl8211eReadPhyReg(interface, RTL8211E_MMDAADR);
}
