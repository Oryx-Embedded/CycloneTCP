/**
 * @file ar8035_driver.c
 * @brief AR8035 Gigabit Ethernet PHY transceiver
 *
 * @section License
 *
 * Copyright (C) 2010-2018 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.8.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "drivers/phy/ar8035_driver.h"
#include "debug.h"


/**
 * @brief AR8035 Ethernet PHY driver
 **/

const PhyDriver ar8035PhyDriver =
{
   ar8035Init,
   ar8035Tick,
   ar8035EnableIrq,
   ar8035DisableIrq,
   ar8035EventHandler,
};


/**
 * @brief AR8035 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ar8035Init(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing AR8035...\r\n");

   //Initialize external interrupt line driver
   if(interface->extIntDriver != NULL)
      interface->extIntDriver->init();

   //Reset PHY transceiver
   ar8035WritePhyReg(interface, AR8035_PHY_REG_BMCR, BMCR_RESET);
   //Wait for the reset to complete
   while(ar8035ReadPhyReg(interface, AR8035_PHY_REG_BMCR) & BMCR_RESET);

   //Basic mode control register
   ar8035WritePhyReg(interface, AR8035_PHY_REG_BMCR,
      BMCR_SPEED_SEL_LSB | BMCR_AN_EN | BMCR_DUPLEX_MODE);

   //Auto-negotiation advertisement register
   ar8035WritePhyReg(interface, AR8035_PHY_REG_ANAR,
      ANAR_XNP_ABLE | ANAR_ASYMMETRIC_PAUSE | ANAR_PAUSE | ANAR_100BTX_FD |
      ANAR_100BTX_HD | ANAR_10BT_FD | ANAR_10BT_HD | ANAR_SELECTOR0);

   //1000 BASE-T control register
   ar8035WritePhyReg(interface, AR8035_PHY_REG_1000BT_CTRL,
      _1000BT_CTRL_1000BT_FD);

   //Function control register
   ar8035WritePhyReg(interface, AR8035_PHY_REG_FUNCTION_CTRL,
      FUNCTION_ASSERT_CRS_ON_TX | FUNCTION_MDI_CROSSOVER_MODE1 |
      FUNCTION_MDI_CROSSOVER_MODE0 | FUNCTION_POLARITY_REVERSAL);

   //Dump PHY registers for debugging purpose
   ar8035DumpPhyReg(interface);

   //The PHY will generate interrupts when link status changes are detected
   ar8035WritePhyReg(interface, AR8035_PHY_REG_INT_EN,
      INT_STATUS_LINK_FAIL | INT_STATUS_LINK_SUCCESS);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief AR8035 timer handler
 * @param[in] interface Underlying network interface
 **/

void ar8035Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
      //Read basic status register
      value = ar8035ReadPhyReg(interface, AR8035_PHY_REG_BMSR);
      //Retrieve current link state
      linkState = (value & BMSR_LINK_STATUS) ? TRUE : FALSE;

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

void ar8035EnableIrq(NetInterface *interface)
{
   //Enable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
      interface->extIntDriver->enableIrq();
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void ar8035DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
      interface->extIntDriver->disableIrq();
}


/**
 * @brief AR8035 event handler
 * @param[in] interface Underlying network interface
 **/

void ar8035EventHandler(NetInterface *interface)
{
   uint16_t status;

   //Read status register to acknowledge the interrupt
   status = ar8035ReadPhyReg(interface, AR8035_PHY_REG_INT_STATUS);

   //Link status change?
   if(status & (INT_STATUS_LINK_FAIL | INT_STATUS_LINK_SUCCESS))
   {
      //Read PHY status register
      status = ar8035ReadPhyReg(interface, AR8035_PHY_REG_PHY_STATUS);

      //Link is up?
      if(status & PHY_STATUS_LINK)
      {
         //Check current speed
         switch(status & PHY_STATUS_SPEED_MASK)
         {
         //10BASE-T
         case PHY_STATUS_SPEED_10:
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
            break;
         //100BASE-TX
         case PHY_STATUS_SPEED_100:
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
            break;
         //1000BASE-T
         case PHY_STATUS_SPEED_1000:
            interface->linkSpeed = NIC_LINK_SPEED_1GBPS;
            break;
         //Unknown speed
         default:
            //Debug message
            TRACE_WARNING("Invalid speed\r\n");
            break;
         }

         //Check current duplex mode
         if(status & PHY_STATUS_DUPLEX)
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
         else
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;

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

void ar8035WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data)
{
   uint8_t phyAddr;

   //Get the address of the PHY transceiver
   if(interface->phyAddr < 32)
      phyAddr = interface->phyAddr;
   else
      phyAddr = AR8035_PHY_ADDR;

   //Write the specified PHY register
   interface->nicDriver->writePhyReg(phyAddr, address, data);
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t ar8035ReadPhyReg(NetInterface *interface, uint8_t address)
{
   uint8_t phyAddr;

   //Get the address of the PHY transceiver
   if(interface->phyAddr < 32)
      phyAddr = interface->phyAddr;
   else
      phyAddr = AR8035_PHY_ADDR;

   //Read the specified PHY register
   return interface->nicDriver->readPhyReg(phyAddr, address);
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void ar8035DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i, ar8035ReadPhyReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
