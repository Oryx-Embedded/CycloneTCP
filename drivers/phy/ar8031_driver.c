/**
 * @file ar8031_driver.c
 * @brief AR8031 Gigabit Ethernet PHY driver
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
#include "drivers/phy/ar8031_driver.h"
#include "debug.h"


/**
 * @brief AR8031 Ethernet PHY driver
 **/

const PhyDriver ar8031PhyDriver =
{
   ar8031Init,
   ar8031Tick,
   ar8031EnableIrq,
   ar8031DisableIrq,
   ar8031EventHandler
};


/**
 * @brief AR8031 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ar8031Init(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing AR8031...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = AR8031_PHY_ADDR;
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
   ar8031WritePhyReg(interface, AR8031_BMCR, AR8031_BMCR_RESET);

   //Wait for the reset to complete
   while(ar8031ReadPhyReg(interface, AR8031_BMCR) & AR8031_BMCR_RESET)
   {
   }

   //Dump PHY registers for debugging purpose
   ar8031DumpPhyReg(interface);

   //Chip configuration register
   ar8031WritePhyReg(interface, AR8031_CHIP_CONF,
      AR8031_CHIP_CONF_BT_BX_REG_SEL | AR8031_CHIP_CONF_PRIORITY_SEL);

   //Basic mode control register
   ar8031WritePhyReg(interface, AR8031_BMCR, AR8031_BMCR_SPEED_SEL_LSB |
      AR8031_BMCR_AN_EN | AR8031_BMCR_DUPLEX_MODE);

   //Auto-negotiation advertisement register
   ar8031WritePhyReg(interface, AR8031_ANAR, AR8031_ANAR_XNP_ABLE |
      AR8031_ANAR_ASYM_PAUSE | AR8031_ANAR_PAUSE | AR8031_ANAR_100BTX_FD |
      AR8031_ANAR_100BTX_HD | AR8031_ANAR_10BT_FD | AR8031_ANAR_10BT_HD |
      AR8031_ANAR_SELECTOR_DEFAULT);

   //1000 BASE-T control register
   ar8031WritePhyReg(interface, AR8031_GBCR, AR8031_GBCR_1000BT_FD);

   //Function control register
   ar8031WritePhyReg(interface, AR8031_FUNC_CTRL,
      AR8031_FUNC_CTRL_ASSERT_CRS_ON_TX | AR8031_FUNC_CTRL_MDIX_MODE_AUTO |
      AR8031_FUNC_CTRL_POLARITY_REVERSAL);

   //The PHY will generate interrupts when link status changes are detected
   ar8031WritePhyReg(interface, AR8031_INT_EN, AR8031_INT_STATUS_LINK_FAIL |
      AR8031_INT_STATUS_LINK_SUCCESS);

   //Perform custom configuration
   ar8031InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief AR8031 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void ar8031InitHook(NetInterface *interface)
{
}


/**
 * @brief AR8031 timer handler
 * @param[in] interface Underlying network interface
 **/

void ar8031Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read basic status register
      value = ar8031ReadPhyReg(interface, AR8031_BMSR);
      //Retrieve current link state
      linkState = (value & AR8031_BMSR_LINK_STATUS) ? TRUE : FALSE;

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

void ar8031EnableIrq(NetInterface *interface)
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

void ar8031DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief AR8031 event handler
 * @param[in] interface Underlying network interface
 **/

void ar8031EventHandler(NetInterface *interface)
{
   uint16_t status;

   //Read status register to acknowledge the interrupt
   status = ar8031ReadPhyReg(interface, AR8031_INT_STATUS);

   //Link status change?
   if((status & (AR8031_INT_STATUS_LINK_FAIL | AR8031_INT_STATUS_LINK_SUCCESS)) != 0)
   {
      //Read PHY status register
      status = ar8031ReadPhyReg(interface, AR8031_PHY_STATUS);

      //Link is up?
      if((status & AR8031_PHY_STATUS_LINK) != 0)
      {
         //Check current speed
         switch(status & AR8031_PHY_STATUS_SPEED)
         {
         //10BASE-T
         case AR8031_PHY_STATUS_SPEED_10MBPS:
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
            break;
         //100BASE-TX
         case AR8031_PHY_STATUS_SPEED_100MBPS:
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
            break;
         //1000BASE-T
         case AR8031_PHY_STATUS_SPEED_1000MBPS:
            interface->linkSpeed = NIC_LINK_SPEED_1GBPS;
            break;
         //Unknown speed
         default:
            //Debug message
            TRACE_WARNING("Invalid speed\r\n");
            break;
         }

         //Check current duplex mode
         if((status & AR8031_PHY_STATUS_DUPLEX) != 0)
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

void ar8031WritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t ar8031ReadPhyReg(NetInterface *interface, uint8_t address)
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

void ar8031DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         ar8031ReadPhyReg(interface, i));
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

void ar8031WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data)
{
   //Select register operation
   ar8031WritePhyReg(interface, AR8031_MMDACR,
      AR8031_MMDACR_FUNC_ADDR | (devAddr & AR8031_MMDACR_DEVAD));

   //Write MMD register address
   ar8031WritePhyReg(interface, AR8031_MMDAADR, regAddr);

   //Select data operation
   ar8031WritePhyReg(interface, AR8031_MMDACR,
      AR8031_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & AR8031_MMDACR_DEVAD));

   //Write the content of the MMD register
   ar8031WritePhyReg(interface, AR8031_MMDAADR, data);
}


/**
 * @brief Read MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return MMD register value
 **/

uint16_t ar8031ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr)
{
   //Select register operation
   ar8031WritePhyReg(interface, AR8031_MMDACR,
      AR8031_MMDACR_FUNC_ADDR | (devAddr & AR8031_MMDACR_DEVAD));

   //Write MMD register address
   ar8031WritePhyReg(interface, AR8031_MMDAADR, regAddr);

   //Select data operation
   ar8031WritePhyReg(interface, AR8031_MMDACR,
      AR8031_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & AR8031_MMDACR_DEVAD));

   //Read the content of the MMD register
   return ar8031ReadPhyReg(interface, AR8031_MMDAADR);
}
