/**
 * @file dp83tg720_driver.c
 * @brief DP83TG720 1000Base-T1 Ethernet PHY driver
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
#include "drivers/phy/dp83tg720_driver.h"
#include "debug.h"


/**
 * @brief DP83TG720 Ethernet PHY driver
 **/

const PhyDriver dp83tg720PhyDriver =
{
   dp83tg720Init,
   dp83tg720Tick,
   dp83tg720EnableIrq,
   dp83tg720DisableIrq,
   dp83tg720EventHandler
};


/**
 * @brief DP83TG720 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t dp83tg720Init(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing DP83TG720...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = DP83TG720_PHY_ADDR;
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
   dp83tg720WritePhyReg(interface, DP83TG720_BMCR, DP83TG720_BMCR_MII_RESET);

   //Wait for the reset to complete
   while(dp83tg720ReadPhyReg(interface, DP83TG720_BMCR) &
      DP83TG720_BMCR_MII_RESET)
   {
   }

   //Dump PHY registers for debugging purpose
   dp83tg720DumpPhyReg(interface);

   //Perform custom configuration
   dp83tg720InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief DP83TG720 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void dp83tg720InitHook(NetInterface *interface)
{
}


/**
 * @brief DP83TG720 timer handler
 * @param[in] interface Underlying network interface
 **/

void dp83tg720Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read PHY status register
      value = dp83tg720ReadPhyReg(interface, DP83TG720_BMSR);
      //Retrieve current link state
      linkState = (value & DP83TG720_BMSR_LINK_STATUS) ? TRUE : FALSE;

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

void dp83tg720EnableIrq(NetInterface *interface)
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

void dp83tg720DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief DP83TG720 event handler
 * @param[in] interface Underlying network interface
 **/

void dp83tg720EventHandler(NetInterface *interface)
{
   uint16_t value;

   //Read PHY status register
   value = dp83tg720ReadPhyReg(interface, DP83TG720_BMSR);

   //Link is up?
   if((value & DP83TG720_BMSR_LINK_STATUS) != 0)
   {
      //The PHY is only able to operate in 1 Gbps mode
      interface->linkSpeed = NIC_LINK_SPEED_1GBPS;
      interface->duplexMode = NIC_FULL_DUPLEX_MODE;

      //Adjust MAC configuration parameters for proper operation
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


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void dp83tg720WritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t dp83tg720ReadPhyReg(NetInterface *interface, uint8_t address)
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

void dp83tg720DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         dp83tg720ReadPhyReg(interface, i));
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

void dp83tg720WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data)
{
   //Select register operation
   dp83tg720WritePhyReg(interface, DP83TG720_REGCR,
      DP83TG720_REGCR_CMD_ADDR | (devAddr & DP83TG720_REGCR_DEVAD));

   //Write MMD register address
   dp83tg720WritePhyReg(interface, DP83TG720_ADDAR, regAddr);

   //Select data operation
   dp83tg720WritePhyReg(interface, DP83TG720_REGCR,
      DP83TG720_REGCR_CMD_DATA_NO_POST_INC | (devAddr & DP83TG720_REGCR_DEVAD));

   //Write the content of the MMD register
   dp83tg720WritePhyReg(interface, DP83TG720_ADDAR, data);
}


/**
 * @brief Read MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return MMD register value
 **/

uint16_t dp83tg720ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr)
{
   //Select register operation
   dp83tg720WritePhyReg(interface, DP83TG720_REGCR,
      DP83TG720_REGCR_CMD_ADDR | (devAddr & DP83TG720_REGCR_DEVAD));

   //Write MMD register address
   dp83tg720WritePhyReg(interface, DP83TG720_ADDAR, regAddr);

   //Select data operation
   dp83tg720WritePhyReg(interface, DP83TG720_REGCR,
      DP83TG720_REGCR_CMD_DATA_NO_POST_INC | (devAddr & DP83TG720_REGCR_DEVAD));

   //Read the content of the MMD register
   return dp83tg720ReadPhyReg(interface, DP83TG720_ADDAR);
}
