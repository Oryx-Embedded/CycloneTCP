/**
 * @file dp83tc811_driver.c
 * @brief DP83TC811 Ethernet PHY transceiver
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
#include "drivers/phy/dp83tc811_driver.h"
#include "debug.h"


/**
 * @brief DP83TC811 Ethernet PHY driver
 **/

const PhyDriver dp83tc811PhyDriver =
{
   dp83tc811Init,
   dp83tc811Tick,
   dp83tc811EnableIrq,
   dp83tc811DisableIrq,
   dp83tc811EventHandler,
   NULL,
   NULL
};


/**
 * @brief DP83TC811 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t dp83tc811Init(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing DP83TC811...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = DP83TC811_PHY_ADDR;
   }

   //Initialize external interrupt line driver
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->init();
   }

   //Reset PHY transceiver
   dp83tc811WritePhyReg(interface, DP83TC811_BMCR, DP83TC811_BMCR_RESET);

   //Wait for the reset to complete
   while(dp83tc811ReadPhyReg(interface, DP83TC811_BMCR) & DP83TC811_BMCR_RESET)
   {
   }

   //Dump PHY registers for debugging purpose
   dp83tc811DumpPhyReg(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief DP83TC811 timer handler
 * @param[in] interface Underlying network interface
 **/

void dp83tc811Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read status register
      value = dp83tc811ReadPhyReg(interface, DP83TC811_BMSR);
      //Retrieve current link state
      linkState = (value & DP83TC811_BMSR_LINK_STATUS) ? TRUE : FALSE;

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

void dp83tc811EnableIrq(NetInterface *interface)
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

void dp83tc811DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief DP83TC811 event handler
 * @param[in] interface Underlying network interface
 **/

void dp83tc811EventHandler(NetInterface *interface)
{
   uint16_t value;

   //Read status register
   value = dp83tc811ReadPhyReg(interface, DP83TC811_BMSR);

   //Link is up?
   if(value & DP83TC811_BMSR_LINK_STATUS)
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


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void dp83tc811WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data)
{
   //Write the specified PHY register
   interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE,
      interface->phyAddr, address, data);
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t dp83tc811ReadPhyReg(NetInterface *interface, uint8_t address)
{
   //Read the specified PHY register
   return interface->nicDriver->readPhyReg(SMI_OPCODE_READ,
      interface->phyAddr, address);
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void dp83tc811DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         dp83tc811ReadPhyReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Write MMD extended register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void dp83tc811WriteMmdReg(NetInterface *interface, uint16_t address,
   uint16_t data)
{
   //Write the value 0x001F to register REGCR
   dp83tc811WritePhyReg(interface, DP83TC811_REGCR,
      DP83TC811_REGCR_COMMAND_ADDR | DP83TC811_REGCR_DEVAD_31);

   //Write the desired register address to register ADDAR
   dp83tc811WritePhyReg(interface, DP83TC811_ADDAR, address);

   //Write the value 0x401F to register REGCR
   dp83tc811WritePhyReg(interface, DP83TC811_REGCR,
      DP83TC811_REGCR_COMMAND_DATA_NO_POST_INC | DP83TC811_REGCR_DEVAD_31);

   //Write the content of the desired extended register set to register ADDAR
   dp83tc811WritePhyReg(interface, DP83TC811_ADDAR, data);
}


/**
 * @brief Read MMD extended register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t dp83tc811ReadMmdReg(NetInterface *interface, uint16_t address)
{
   //Write the value 0x001F to register REGCR
   dp83tc811WritePhyReg(interface, DP83TC811_REGCR,
      DP83TC811_REGCR_COMMAND_ADDR | DP83TC811_REGCR_DEVAD_31);

   //Write the desired register address to register ADDAR
   dp83tc811WritePhyReg(interface, DP83TC811_ADDAR, address);

   //Write the value 0x401F to register REGCR
   dp83tc811WritePhyReg(interface, DP83TC811_REGCR,
      DP83TC811_REGCR_COMMAND_DATA_NO_POST_INC | DP83TC811_REGCR_DEVAD_31);

   //Read the content of the desired extended register set in register ADDAR
   return dp83tc811ReadPhyReg(interface, DP83TC811_ADDAR);
}


/**
 * @brief Write MMD1 extended register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void dp83tc811WriteMmd1Reg(NetInterface *interface, uint16_t address,
   uint16_t data)
{
   //Write the value 0x0001 to register REGCR
   dp83tc811WritePhyReg(interface, DP83TC811_REGCR,
      DP83TC811_REGCR_COMMAND_ADDR | DP83TC811_REGCR_DEVAD_1);

   //Write the desired register address to register ADDAR
   dp83tc811WritePhyReg(interface, DP83TC811_ADDAR, address);

   //Write the value 0x4001 to register REGCR
   dp83tc811WritePhyReg(interface, DP83TC811_REGCR,
      DP83TC811_REGCR_COMMAND_DATA_NO_POST_INC | DP83TC811_REGCR_DEVAD_1);

   //Write the content of the desired extended register set to register ADDAR
   dp83tc811WritePhyReg(interface, DP83TC811_ADDAR, data);
}


/**
 * @brief Read MMD1 extended register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t dp83tc811ReadMmd1Reg(NetInterface *interface, uint16_t address)
{
   //Write the value 0x0001 to register REGCR
   dp83tc811WritePhyReg(interface, DP83TC811_REGCR,
      DP83TC811_REGCR_COMMAND_ADDR | DP83TC811_REGCR_DEVAD_1);

   //Write the desired register address to register ADDAR
   dp83tc811WritePhyReg(interface, DP83TC811_ADDAR, address);

   //Write the value 0x4001 to register REGCR
   dp83tc811WritePhyReg(interface, DP83TC811_REGCR,
      DP83TC811_REGCR_COMMAND_DATA_NO_POST_INC | DP83TC811_REGCR_DEVAD_1);

   //Read the content of the desired extended register set in register ADDAR
   return dp83tc811ReadPhyReg(interface, DP83TC811_ADDAR);
}
