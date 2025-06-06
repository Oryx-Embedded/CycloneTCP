/**
 * @file lan8841_driver.c
 * @brief LAN8841 Gigabit Ethernet PHY driver
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
#include "drivers/phy/lan8841_driver.h"
#include "debug.h"


/**
 * @brief LAN8841 Ethernet PHY driver
 **/

const PhyDriver lan8841PhyDriver =
{
   lan8841Init,
   lan8841Tick,
   lan8841EnableIrq,
   lan8841DisableIrq,
   lan8841EventHandler
};


/**
 * @brief LAN8841 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t lan8841Init(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing LAN8841...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = LAN8841_PHY_ADDR;
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
   lan8841WritePhyReg(interface, LAN8841_BMCR, LAN8841_BMCR_RESET);

   //Wait for the reset to complete
   while(lan8841ReadPhyReg(interface, LAN8841_BMCR) & LAN8841_BMCR_RESET)
   {
   }

   //Dump PHY registers for debugging purpose
   lan8841DumpPhyReg(interface);

   //Perform custom configuration
   lan8841InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief LAN8841 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void lan8841InitHook(NetInterface *interface)
{
   uint16_t value;

   //If MAC does not provide any delay for the TXC, the device may add a fixed
   //2ns delay to the TXC input
   value = lan8841ReadMmdReg(interface, LAN8841_TX_DLL_CTRL);
   value &= ~LAN8841_TX_DLL_CTRL_BYPASS_TXDLL;
   lan8841WriteMmdReg(interface, LAN8841_TX_DLL_CTRL, value);
}


/**
 * @brief LAN8841 timer handler
 * @param[in] interface Underlying network interface
 **/

void lan8841Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read basic status register
      value = lan8841ReadPhyReg(interface, LAN8841_BMSR);
      //Retrieve current link state
      linkState = (value & LAN8841_BMSR_LINK_STATUS) ? TRUE : FALSE;

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

void lan8841EnableIrq(NetInterface *interface)
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

void lan8841DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief LAN8841 event handler
 * @param[in] interface Underlying network interface
 **/

void lan8841EventHandler(NetInterface *interface)
{
   uint16_t value;

   //Read status register to acknowledge the interrupt
   value = lan8841ReadPhyReg(interface, LAN8841_ISR);

   //Link status change?
   if((value & (LAN8841_ISR_LINK_DOWN | LAN8841_ISR_LINK_UP)) != 0)
   {
      //Any link failure condition is latched in the BMSR register. Reading
      //the register twice will always return the actual link status
      value = lan8841ReadPhyReg(interface, LAN8841_BMSR);
      value = lan8841ReadPhyReg(interface, LAN8841_BMSR);

      //Link is up?
      if((value & LAN8841_BMSR_LINK_STATUS) != 0)
      {
         //Read PHY control register
         value = lan8841ReadPhyReg(interface, LAN8841_PHYCON);

         //Check current speed
         if((value & LAN8841_PHYCON_SPEED_1000BT) != 0)
         {
            //1000BASE-T
            interface->linkSpeed = NIC_LINK_SPEED_1GBPS;
         }
         else if((value & LAN8841_PHYCON_SPEED_100BTX) != 0)
         {
            //100BASE-TX
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
         }
         else if((value & LAN8841_PHYCON_SPEED_10BT) != 0)
         {
            //10BASE-T
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
         }
         else
         {
            //Debug message
            TRACE_WARNING("Invalid speed!\r\n");
         }

         //Check current duplex mode
         if((value & LAN8841_PHYCON_DUPLEX_STATUS) != 0)
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

void lan8841WritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t lan8841ReadPhyReg(NetInterface *interface, uint8_t address)
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

void lan8841DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         lan8841ReadPhyReg(interface, i));
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

void lan8841WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data)
{
   //Select register operation
   lan8841WritePhyReg(interface, LAN8841_MMDACR,
      LAN8841_MMDACR_FUNC_ADDR | (devAddr & LAN8841_MMDACR_DEVAD));

   //Write MMD register address
   lan8841WritePhyReg(interface, LAN8841_MMDAADR, regAddr);

   //Select data operation
   lan8841WritePhyReg(interface, LAN8841_MMDACR,
      LAN8841_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & LAN8841_MMDACR_DEVAD));

   //Write the content of the MMD register
   lan8841WritePhyReg(interface, LAN8841_MMDAADR, data);
}


/**
 * @brief Read MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return MMD register value
 **/

uint16_t lan8841ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr)
{
   //Select register operation
   lan8841WritePhyReg(interface, LAN8841_MMDACR,
      LAN8841_MMDACR_FUNC_ADDR | (devAddr & LAN8841_MMDACR_DEVAD));

   //Write MMD register address
   lan8841WritePhyReg(interface, LAN8841_MMDAADR, regAddr);

   //Select data operation
   lan8841WritePhyReg(interface, LAN8841_MMDACR,
      LAN8841_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & LAN8841_MMDACR_DEVAD));

   //Read the content of the MMD register
   return lan8841ReadPhyReg(interface, LAN8841_MMDAADR);
}
