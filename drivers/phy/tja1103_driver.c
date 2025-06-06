/**
 * @file tja1103_driver.c
 * @brief TJA1103 100Base-T1 Ethernet PHY driver
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
#include "drivers/phy/tja1103_driver.h"
#include "debug.h"


/**
 * @brief TJA1103 Ethernet PHY driver
 **/

const PhyDriver tja1103PhyDriver =
{
   tja1103Init,
   tja1103Tick,
   tja1103EnableIrq,
   tja1103DisableIrq,
   tja1103EventHandler
};


/**
 * @brief TJA1103 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t tja1103Init(NetInterface *interface)
{
   uint16_t value;

   //Debug message
   TRACE_INFO("Initializing TJA1103...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = TJA1103_PHY_ADDR;
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

   //Wait for the MII interface to be ready
   do
   {
      //Read PHY identification register 1
      value = tja1103ReadPhyReg(interface, TJA1103_PHY_ID1);

      //Check identifier value
   } while (value != TJA1103_PHY_ID1_OUI_MSB_DEFAULT);

   //Dump PHY registers for debugging purpose
   tja1103DumpPhyReg(interface);

   //Enable configuration register access
   tja1103WriteMmdReg(interface, TJA1103_DEVICE_CTRL,
      TJA1103_DEVICE_CTRL_CONFIG_GLOBAL_EN | TJA1103_DEVICE_CTRL_CONFIG_ALL_EN);

   tja1103WriteMmdReg(interface, TJA1103_PORT_CTRL, TJA1103_PORT_CTRL_EN);
   tja1103WriteMmdReg(interface, TJA1103_PHY_CTRL, TJA1103_PHY_CTRL_CONFIG_EN);
   tja1103WriteMmdReg(interface, TJA1103_INFRA_CTRL, TJA1103_INFRA_CTRL_EN);

   //Perform custom configuration
   tja1103InitHook(interface);

   //The PHY is configured for autonomous operation
   value = tja1103ReadMmdReg(interface, TJA1103_PHY_CONFIG);
   value |= TJA1103_PHY_CONFIG_AUTO;
   tja1103WriteMmdReg(interface, TJA1103_PHY_CONFIG, value);

   //Clear FUSA_PASS interrupt flag
   tja1103WriteMmdReg(interface, TJA1103_ALWAYS_ACCESSIBLE,
      TJA1103_ALWAYS_ACCESSIBLE_FUSA_PASS);

   //Start operation
   value = tja1103ReadMmdReg(interface, TJA1103_PHY_CTRL);
   value |= TJA1103_PHY_CTRL_START_OP;
   tja1103WriteMmdReg(interface, TJA1103_PHY_CTRL, value);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief TJA1103 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void tja1103InitHook(NetInterface *interface)
{
}


/**
 * @brief TJA1103 timer handler
 * @param[in] interface Underlying network interface
 **/

void tja1103Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read status register
      value = tja1103ReadMmdReg(interface, TJA1103_PHY_STAT);
      //Retrieve current link state
      linkState = (value & TJA1103_PHY_STAT_LINK_STATUS) ? TRUE : FALSE;

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

void tja1103EnableIrq(NetInterface *interface)
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

void tja1103DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief TJA1103 event handler
 * @param[in] interface Underlying network interface
 **/

void tja1103EventHandler(NetInterface *interface)
{
   uint16_t value;

   //Read status register
   value = tja1103ReadMmdReg(interface, TJA1103_PHY_STAT);

   //Link is up?
   if((value & TJA1103_PHY_STAT_LINK_STATUS) != 0)
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

void tja1103WritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t tja1103ReadPhyReg(NetInterface *interface, uint8_t address)
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

void tja1103DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         tja1103ReadPhyReg(interface, i));
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

void tja1103WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data)
{
   //Select register operation
   tja1103WritePhyReg(interface, TJA1103_MMDCTRL,
      TJA1103_MMDCTRL_FNCTN_ADDR | (devAddr & TJA1103_MMDCTRL_DEVAD));

   //Write MMD register address
   tja1103WritePhyReg(interface, TJA1103_MMDAD, regAddr);

   //Select data operation
   tja1103WritePhyReg(interface, TJA1103_MMDCTRL,
      TJA1103_MMDCTRL_FNCTN_DATA_NO_POST_INC | (devAddr & TJA1103_MMDCTRL_DEVAD));

   //Write the content of the MMD register
   tja1103WritePhyReg(interface, TJA1103_MMDAD, data);
}


/**
 * @brief Read MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return MMD register value
 **/

uint16_t tja1103ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr)
{
   //Select register operation
   tja1103WritePhyReg(interface, TJA1103_MMDCTRL,
      TJA1103_MMDCTRL_FNCTN_ADDR | (devAddr & TJA1103_MMDCTRL_DEVAD));

   //Write MMD register address
   tja1103WritePhyReg(interface, TJA1103_MMDAD, regAddr);

   //Select data operation
   tja1103WritePhyReg(interface, TJA1103_MMDCTRL,
      TJA1103_MMDCTRL_FNCTN_DATA_NO_POST_INC | (devAddr & TJA1103_MMDCTRL_DEVAD));

   //Read the content of the MMD register
   return tja1103ReadPhyReg(interface, TJA1103_MMDAD);
}
