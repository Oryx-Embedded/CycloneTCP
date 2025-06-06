/**
 * @file ncn26000_driver.c
 * @brief NCN26000 10Base-T1S Ethernet PHY driver
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
#include "drivers/phy/ncn26000_driver.h"
#include "debug.h"


/**
 * @brief NCN26000 Ethernet PHY driver
 **/

const PhyDriver ncn26000PhyDriver =
{
   ncn26000Init,
   ncn26000Tick,
   ncn26000EnableIrq,
   ncn26000DisableIrq,
   ncn26000EventHandler
};


/**
 * @brief NCN26000 PHY transceiver initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ncn26000Init(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing NCN26000...\r\n");

   //Undefined PHY address?
   if(interface->phyAddr >= 32)
   {
      //Use the default address
      interface->phyAddr = NCN26000_PHY_ADDR;
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

   //Dump PHY registers for debugging purpose
   ncn26000DumpPhyReg(interface);

   //Perform custom configuration
   ncn26000InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief NCN26000 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void ncn26000InitHook(NetInterface *interface)
{
#if (NCN26000_PLCA_SUPPORT == ENABLED)
   //Set PLCA burst
   ncn26000WriteMmdReg(interface, NCN26000_PLCA_BURST_MODE,
      NCN26000_PLCA_BURST_MODE_MAX_BURST_COUNT_DEFAULT |
      NCN26000_PLCA_BURST_MODE_IFG_COMP_TMR_DEFAULT);

   //Set PLCA node count and local ID
   ncn26000WriteMmdReg(interface, NCN26000_PLCA_CTRL1,
      ((NCN26000_NODE_COUNT << 8) & NCN26000_PLCA_CTRL1_NCNT) |
      (NCN26000_LOCAL_ID & NCN26000_PLCA_CTRL1_ID));

   //Enable PLCA
   ncn26000WriteMmdReg(interface, NCN26000_PLCA_CTRL0, NCN26000_PLCA_CTRL0_EN);
#else
   //Disable PLCA
   ncn26000WriteMmdReg(interface, NCN26000_PLCA_CTRL0, 0);
#endif
}


/**
 * @brief NCN26000 timer handler
 * @param[in] interface Underlying network interface
 **/

void ncn26000Tick(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

   //No external interrupt line driver?
   if(interface->extIntDriver == NULL)
   {
#if (NCN26000_PLCA_SUPPORT == ENABLED)
      //Read PLCA status register
      value = ncn26000ReadMmdReg(interface, NCN26000_PLCA_STATUS);

      //The PST field indicates that the PLCA reconciliation sublayer is active
      //and a BEACON is being regularly transmitted or received
      linkState = (value & NCN26000_PLCA_STATUS_PST) ? TRUE : FALSE;
#else
      //Link status indication is not supported
      linkState = TRUE;
#endif

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

void ncn26000EnableIrq(NetInterface *interface)
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

void ncn26000DisableIrq(NetInterface *interface)
{
   //Disable PHY transceiver interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief NCN26000 event handler
 * @param[in] interface Underlying network interface
 **/

void ncn26000EventHandler(NetInterface *interface)
{
   uint16_t value;
   bool_t linkState;

#if (NCN26000_PLCA_SUPPORT == ENABLED)
   //Read PLCA status register
   value = ncn26000ReadMmdReg(interface, NCN26000_PLCA_STATUS);

   //The PST field indicates that the PLCA reconciliation sublayer is active
   //and a BEACON is being regularly transmitted or received
   linkState = (value & NCN26000_PLCA_STATUS_PST) ? TRUE : FALSE;
#else
   //Link status indication is not supported
   linkState = TRUE;
#endif

   //Link is up?
   if(linkState)
   {
      //The PHY is only able to operate in 10 Mbps mode
      interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
      interface->duplexMode = NIC_HALF_DUPLEX_MODE;

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

void ncn26000WritePhyReg(NetInterface *interface, uint8_t address,
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

uint16_t ncn26000ReadPhyReg(NetInterface *interface, uint8_t address)
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

void ncn26000DumpPhyReg(NetInterface *interface)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         ncn26000ReadPhyReg(interface, i));
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

void ncn26000WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data)
{
   //Select register operation
   ncn26000WritePhyReg(interface, NCN26000_MACR,
      NCN26000_MACR_FUNC_ADDR | (devAddr & NCN26000_MACR_DEVADD));

   //Write MMD register address
   ncn26000WritePhyReg(interface, NCN26000_MAADR, regAddr);

   //Select data operation
   ncn26000WritePhyReg(interface, NCN26000_MACR,
      NCN26000_MACR_FUNC_DATA_NO_POST_INC | (devAddr & NCN26000_MACR_DEVADD));

   //Write the content of the MMD register
   ncn26000WritePhyReg(interface, NCN26000_MAADR, data);
}


/**
 * @brief Read MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return MMD register value
 **/

uint16_t ncn26000ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr)
{
   //Select register operation
   ncn26000WritePhyReg(interface, NCN26000_MACR,
      NCN26000_MACR_FUNC_ADDR | (devAddr & NCN26000_MACR_DEVADD));

   //Write MMD register address
   ncn26000WritePhyReg(interface, NCN26000_MAADR, regAddr);

   //Select data operation
   ncn26000WritePhyReg(interface, NCN26000_MACR,
      NCN26000_MACR_FUNC_DATA_NO_POST_INC | (devAddr & NCN26000_MACR_DEVADD));

   //Read the content of the MMD register
   return ncn26000ReadPhyReg(interface, NCN26000_MAADR);
}


/**
 * @brief Modify MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @param[in] mask 16-bit mask
 * @param[in] data 16-bit value
 **/

void ncn26000ModifyMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t mask, uint16_t data)
{
   uint16_t value;

   //Read the current value of the MMD register
   value = ncn26000ReadMmdReg(interface, devAddr, regAddr);
   //Modify register value
   value = (value & ~mask) | data;
   //Write the modified value back to the MMD register
   ncn26000WriteMmdReg(interface, devAddr, regAddr, value);
}
