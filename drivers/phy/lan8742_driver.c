/**
 * @file lan8742_driver.c
 * @brief LAN8742 Ethernet PHY driver
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
#include "drivers/phy/lan8742_driver.h"
#include "debug.h"


/**
 * @brief LAN8742 Ethernet PHY driver
 **/

const PhyDriver lan8742PhyDriver =
{
   lan8742Init,
   lan8742Tick,
   lan8742EnableIrq,
   lan8742DisableIrq,
   lan8742EventHandler
};


/**
 * @brief LAN8742 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t lan8742Init(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing LAN8742...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = LAN8742_PHY_ADDR;
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

   //Reset PHY transceiver (soft reset)
   lan8742WritePhyReg(interface, LAN8742_BMCR, LAN8742_BMCR_RESET);

   //Wait for the reset to complete
   while(lan8742ReadPhyReg(interface, LAN8742_BMCR) & LAN8742_BMCR_RESET)
   {
   }

   //Restore default auto-negotiation advertisement parameters
   lan8742WritePhyReg(interface, LAN8742_ANAR, LAN8742_ANAR_100BTX_FD |
      LAN8742_ANAR_100BTX_HD | LAN8742_ANAR_10BT_FD | LAN8742_ANAR_10BT_HD |
      LAN8742_ANAR_SELECTOR_DEFAULT);

   //Enable auto-negotiation
   lan8742WritePhyReg(interface, LAN8742_BMCR, LAN8742_BMCR_AN_EN);

   //Dump PHY registers for debugging purpose
   lan8742DumpPhyReg(interface);

   //The PHY will generate interrupts when link status changes are detected
   lan8742WritePhyReg(interface, LAN8742_IMR, LAN8742_IMR_AN_COMPLETE |
      LAN8742_IMR_LINK_DOWN);

   //Perform custom configuration
   lan8742InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief LAN8742 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void lan8742InitHook(NetInterface *interface)
{
}


/**
 * @brief LAN8742 timer handler
 * @param[in] interface Underlying network interface
 **/

void lan8742Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read basic status register
      value = lan8742ReadPhyReg(interface, LAN8742_BMSR);
      //Retrieve current link state
      linkState = (value & LAN8742_BMSR_LINK_STATUS) ? TRUE : FALSE;

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

void lan8742EnableIrq(NetInterface *interface)
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

void lan8742DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief LAN8742 event handler
 * @param[in] interface Underlying network interface
 **/

void lan8742EventHandler(NetInterface *interface)
{
   uint16_t value;

   //Read status register to acknowledge the interrupt
   value = lan8742ReadPhyReg(interface, LAN8742_ISR);

   //Link status change?
   if((value & (LAN8742_IMR_AN_COMPLETE | LAN8742_IMR_LINK_DOWN)) != 0)
   {
      //Any link failure condition is latched in the BMSR register. Reading
      //the register twice will always return the actual link status
      value = lan8742ReadPhyReg(interface, LAN8742_BMSR);
      value = lan8742ReadPhyReg(interface, LAN8742_BMSR);

      //Link is up?
      if((value & LAN8742_BMSR_LINK_STATUS) != 0)
      {
         //Read PHY special control/status register
         value = lan8742ReadPhyReg(interface, LAN8742_PSCSR);

         //Check current operation mode
         switch(value & LAN8742_PSCSR_HCDSPEED)
         {
         //10BASE-T half-duplex
         case LAN8742_PSCSR_HCDSPEED_10BT_HD:
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;
            break;

         //10BASE-T full-duplex
         case LAN8742_PSCSR_HCDSPEED_10BT_FD:
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
            break;

         //100BASE-TX half-duplex
         case LAN8742_PSCSR_HCDSPEED_100BTX_HD:
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;
            break;

         //100BASE-TX full-duplex
         case LAN8742_PSCSR_HCDSPEED_100BTX_FD:
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

void lan8742WritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t lan8742ReadPhyReg(NetInterface *interface, uint8_t address)
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

void lan8742DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         lan8742ReadPhyReg(interface, i));
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

void lan8742WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data)
{
   //Select register operation
   lan8742WritePhyReg(interface, LAN8742_MMDACR,
      LAN8742_MMDACR_FUNC_ADDR | (devAddr & LAN8742_MMDACR_DEVAD));

   //Write MMD register address
   lan8742WritePhyReg(interface, LAN8742_MMDAADR, regAddr);

   //Select data operation
   lan8742WritePhyReg(interface, LAN8742_MMDACR,
      LAN8742_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & LAN8742_MMDACR_DEVAD));

   //Write the content of the MMD register
   lan8742WritePhyReg(interface, LAN8742_MMDAADR, data);
}


/**
 * @brief Read MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return MMD register value
 **/

uint16_t lan8742ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr)
{
   //Select register operation
   lan8742WritePhyReg(interface, LAN8742_MMDACR,
      LAN8742_MMDACR_FUNC_ADDR | (devAddr & LAN8742_MMDACR_DEVAD));

   //Write MMD register address
   lan8742WritePhyReg(interface, LAN8742_MMDAADR, regAddr);

   //Select data operation
   lan8742WritePhyReg(interface, LAN8742_MMDACR,
      LAN8742_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & LAN8742_MMDACR_DEVAD));

   //Read the content of the MMD register
   return lan8742ReadPhyReg(interface, LAN8742_MMDAADR);
}
