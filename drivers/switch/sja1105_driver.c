/**
 * @file sja1105_driver.c
 * @brief SJA1105 5-port Ethernet switch driver
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
#include "core/ethernet_misc.h"
#include "drivers/switch/sja1105_driver.h"
#include "debug.h"


/**
 * @brief SJA1105 Ethernet switch driver
 **/

const SwitchDriver sja1105SwitchDriver =
{
   sja1105Init,
   sja1105Tick,
   sja1105EnableIrq,
   sja1105DisableIrq,
   sja1105EventHandler,
   sja1105TagFrame,
   sja1105UntagFrame,
   sja1105GetLinkState,
   sja1105GetLinkSpeed,
   sja1105GetDuplexMode,
   sja1105SetPortState,
   sja1105GetPortState,
   sja1105SetAgingTime,
   sja1105EnableIgmpSnooping,
   sja1105EnableMldSnooping,
   sja1105EnableRsvdMcastTable,
   sja1105AddStaticFdbEntry,
   sja1105DeleteStaticFdbEntry,
   sja1105GetStaticFdbEntry,
   sja1105FlushStaticFdbTable,
   sja1105GetDynamicFdbEntry,
   sja1105FlushDynamicFdbTable,
   sja1105SetUnknownMcastFwdPorts
};


/**
 * @brief SJA1105 Ethernet switch initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t sja1105Init(NetInterface *interface)
{
   error_t error;
   uint32_t temp;

   //Debug message
   TRACE_INFO("Initializing SJA1105...\r\n");

   //Initialize SPI interface
   interface->spiDriver->init();

   //Wait for the serial interface to be ready
   do
   {
      //Read CHIP_ID0 register
      temp = sja1105ReadSingleReg(interface, SJA1105_DEVICE_ID);

      //The returned data is invalid until the serial interface is ready
   } while(temp != SJA1105_DEVICE_ID_SJA1105PEL &&
      temp != SJA1105_DEVICE_ID_SJA1105QEL);

   //Dump switch registers for debugging purpose
   sja1105DumpReg(interface);

   //Perform custom configuration
   error = sja1105InitHook(interface);

   //Check status code
   if(!error)
   {
      //Force the TCP/IP stack to poll the link state at startup
      interface->phyEvent = TRUE;
      //Notify the TCP/IP stack of the event
      osSetEvent(&netEvent);
   }

   //Return status code
   return error;
}


/**
 * @brief SJA1105 custom configuration
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

__weak_func error_t sja1105InitHook(NetInterface *interface)
{
   //The static configuration must be loaded into the device
   return ERROR_FAILURE;
}


/**
 * @brief Load static configuration into the device
 * @param[in] interface Underlying network interface
 * @param[in] data Pointer to the HEX file to be loaded
 * @param[in] length Length of the HEX file
 * @return Error code
 **/

error_t sja1105LoadStaticConfig(NetInterface *interface, const char_t *data,
   size_t length)
{
   size_t i;
   size_t j;
   size_t n;
   uint8_t type;
   uint32_t offset;
   uint32_t value;
   char_t *p;
   char_t buffer[9];

   //Debug message
   TRACE_INFO("Loading static configuration...\r\n");

   //Read the Initial Device Configuration Flag register
   value = sja1105ReadSingleReg(interface, SJA1105_INIT_DEV_CONFIG_FLAG);

   //if the CONFIGS flag set, the configuration is locked and cannot be
   //overridden without resetting the device
   if((value & SJA1105_INIT_DEV_CONFIG_FLAG_CONFIGS) != 0)
      return ERROR_ALREADY_CONFIGURED;

   //The load operation is initiated by writing the device ID to relative
   //address 0 (i.e. relative to the start address 0x20000)
   offset = 0;

   //Parse HEX file
   for(i = 0; i < length; i += n)
   {
      //The start of the record is marked by an ASCII colon
      if(data[i] == ':')
      {
         //Check the length of the record
         if((length - i) < 11)
            return ERROR_INVALID_LENGTH;

         //The start code is followed by a 1 byte (2 hex digits) of byte count
         buffer[0] = data[i + 1];
         buffer[1] = data[i + 2];
         buffer[2] = '\0';

         //Retrieve the number of bytes in the record
         n = osStrtoul(buffer, &p, 16);

         //Malformed record?
         if(*p != '\0')
            return ERROR_INVALID_SYNTAX;

         //Check the length of the record
         if((length - i) < (n + 11))
            return ERROR_INVALID_LENGTH;

         //The byte count is followed by a 2 byte (4 hex digits) address
         buffer[0] = data[i + 3];
         buffer[1] = data[i + 4];
         buffer[2] = data[i + 5];
         buffer[3] = data[i + 6];
         buffer[4] = '\0';

         //The address indicates the offset within the configuration file
         osStrtoul(buffer, &p, 16);

         //Malformed record?
         if(*p != '\0')
            return ERROR_INVALID_SYNTAX;

         //The record type is 1 byte (2 hex digits) in size
         buffer[0] = data[i + 7];
         buffer[1] = data[i + 8];
         buffer[2] = '\0';

         //Retrieve the number of bytes in the record
         type = (uint8_t) osStrtoul(buffer, &p, 16);

         //Malformed record?
         if(*p != '\0')
            return ERROR_INVALID_SYNTAX;

         //The record type represents the type of data
         if(type == 0x00)
         {
            //The static configuration is a stream of 32-bit data
            if((n % 4) != 0)
               return ERROR_INVALID_LENGTH;

            //Parse data
            for(j = 0; j < (2 * n); j += 8)
            {
               //A 32-bit word consists of 8 hex digits
               buffer[0] = data[i + j + 15];
               buffer[1] = data[i + j + 16];
               buffer[2] = data[i + j + 13];
               buffer[3] = data[i + j + 14];
               buffer[4] = data[i + j + 11];
               buffer[5] = data[i + j + 12];
               buffer[6] = data[i + j + 9];
               buffer[7] = data[i + j + 10];
               buffer[8] = '\0';

               //Retrieve the value of the 32-bit word
               value = osStrtoul(buffer, &p, 16);

               //Malformed record?
               if(*p != '\0')
                  return ERROR_INVALID_SYNTAX;

               //The load operation is relative to the start address 0x20000
               sja1105WriteSingleReg(interface, SJA1105_ETH_STATIC_BASE + offset, value);

               //Increment offset
               offset++;
            }
         }
         else if(type == 0x01)
         {
            //End of file
            break;
         }
         else
         {
            //Ignore unknown records
         }

         //Total length of the record
         n = (2 * n) + 11;
      }
      else
      {
         //All characters preceding the start code should be ignored
         n = 1;
      }
   }

   //Read the Initial Device Configuration Flag register
   value = sja1105ReadSingleReg(interface, SJA1105_INIT_DEV_CONFIG_FLAG);

   //The CONFIGS flag should be checked after loading the static configuration
   if((value & SJA1105_INIT_DEV_CONFIG_FLAG_CONFIGS) == 0)
      return ERROR_NOT_CONFIGURED;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief PLL1 setup for 50MHz
 * @param[in] interface Underlying network interface
 **/

void sja1105ConfigurePll1(NetInterface *interface)
{
   uint32_t config;

   //Debug message
   TRACE_INFO("Configuring PLL1...\r\n");

   //PLL1 setup for 50MHz
   config = SJA1105_PLL_x_C_PLLCLKSRC_XO66M_0 |
      SJA1105_PLL_x_C_MSEL_DIV2 | SJA1105_PLL_x_C_AUTOBLOCK |
      SJA1105_PLL_x_C_PSEL_DIV2 | SJA1105_PLL_x_C_FBSEL;

   //Configure PLL1
   sja1105WriteSingleReg(interface, SJA1105_PLL_1_C, config |
      SJA1105_PLL_x_C_PD);

   //Enable PLL1
   sja1105WriteSingleReg(interface, SJA1105_PLL_1_C, config);
}


/**
 * @brief Clock generation unit setup
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Error code
 **/

error_t sja1105ConfigureCgu(NetInterface *interface, uint8_t port)
{
   error_t error;
   uint_t n;
   uint32_t value;
   uint32_t mode;
   uint32_t speed;

   //Initialize status code
   error = NO_ERROR;

   //Check port number
   if(port >= SJA1105_PORT0 && port <= SJA1105_PORT4)
   {
      //Retrieve the zero-based index of the port
      n = port - SJA1105_PORT0;

      //Debug message
      TRACE_INFO("Configuring CGU (port %u)...\r\n", n);

      //Read port status register
      value = sja1105ReadSingleReg(interface, SJA1105_PORT_STATUS_MIIx(n));

      //Retrieve port mode
      mode = value & SJA1105_PORT_STATUS_MIIx_MODE;
      //Retrieve port speed
      speed = value & SJA1105_PORT_STATUS_MIIx_SPEED;

      //MII MAC mode?
      if(mode == SJA1105_PORT_STATUS_MIIx_MODE_MII_MAC)
      {
         //Disable IDIVx
         sja1105WriteSingleReg(interface, SJA1105_IDIV_x_C(n),
            SJA1105_IDIV_x_C_CLKSRC_XO66M_0 |
            SJA1105_IDIV_x_C_PD);

         //Set CLKSRC field of MII_TX_CLK_x to TX_CLK_x
         sja1105WriteSingleReg(interface, SJA1105_MII_TX_CLK_x(n),
            SJA1105_MIIx_CLK_CTRL_CLKSRC_TX_CLK_x(n) |
            SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);

         //Set CLKSRC field of MII_RX_CLK_x to RX_CLK_x
         sja1105WriteSingleReg(interface, SJA1105_MII_RX_CLK_x(n),
            SJA1105_MIIx_CLK_CTRL_CLKSRC_RX_CLK_x(n) |
            SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);
      }
      //MII PHY mode?
      else if(mode == SJA1105_PORT_STATUS_MIIx_MODE_MII_PHY)
      {
         //Check port speed
         if(speed == SJA1105_PORT_STATUS_MIIx_SPEED_10MBPS)
         {
            //Enable IDIVx and divide by 10
            sja1105WriteSingleReg(interface, SJA1105_IDIV_x_C(n),
               SJA1105_IDIV_x_C_CLKSRC_XO66M_0 | SJA1105_IDIV_x_C_AUTOBLOCK |
               SJA1105_IDIV_x_C_IDIV_DIV10);
         }
         else if(speed == SJA1105_PORT_STATUS_MIIx_SPEED_100MBPS)
         {
            //Enable IDIVx and divide by 1
            sja1105WriteSingleReg(interface, SJA1105_IDIV_x_C(n),
               SJA1105_IDIV_x_C_CLKSRC_XO66M_0 | SJA1105_IDIV_x_C_IDIV_DIV1);
         }
         else
         {
            //Report an error
            error = ERROR_FAILURE;
         }

         //Check status code
         if(!error)
         {
            //Set CLKSRC field of MII_TX_CLK_x to IDIVx
            sja1105WriteSingleReg(interface, SJA1105_MII_TX_CLK_x(n),
               SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIVx(n) |
               SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);

            //Set CLKSRC field of MII_RX_CLK_x to RX_CLK_x
            sja1105WriteSingleReg(interface, SJA1105_MII_RX_CLK_x(n),
               SJA1105_MIIx_CLK_CTRL_CLKSRC_RX_CLK_x(n) |
               SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);

            //Set CLKSRC field of EXT_TX_CLK_x to IDIVx
            sja1105WriteSingleReg(interface, SJA1105_EXT_TX_CLK_x(n),
               SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIVx(n) |
               SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);

            //Set CLKSRC field of EXT_RX_CLK_x to IDIVx
            sja1105WriteSingleReg(interface, SJA1105_EXT_RX_CLK_x(n),
               SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIVx(n) |
               SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);
         }
      }
      //RMII MAC mode?
      else if(mode == SJA1105_PORT_STATUS_MIIx_MODE_RMII_MAC)
      {
         //Disable IDIVx
         sja1105WriteSingleReg(interface, SJA1105_IDIV_x_C(n),
            SJA1105_IDIV_x_C_CLKSRC_XO66M_0 |
            SJA1105_IDIV_x_C_PD);

         //Set CLKSRC field of RMII_REF_CLK_x to TX_CLK_x
         sja1105WriteSingleReg(interface, SJA1105_RMII_REF_CLK_x(n),
            SJA1105_MIIx_CLK_CTRL_CLKSRC_TX_CLK_x(n) |
            SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);

         //Set CLKSRC field of EXT_TX_CLK_x to PLL1
         sja1105WriteSingleReg(interface, SJA1105_EXT_TX_CLK_x(n),
            SJA1105_MIIx_CLK_CTRL_CLKSRC_PLL1 |
            SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);
      }
      //RMII PHY mode?
      else if(mode == SJA1105_PORT_STATUS_MIIx_MODE_RMII_PHY)
      {
         //Disable IDIVx
         sja1105WriteSingleReg(interface, SJA1105_IDIV_x_C(n),
            SJA1105_IDIV_x_C_CLKSRC_XO66M_0 |
            SJA1105_IDIV_x_C_PD);

         //Set CLKSRC field of RMII_REF_CLK_x to TX_CLK_x
         sja1105WriteSingleReg(interface, SJA1105_RMII_REF_CLK_x(n),
            SJA1105_MIIx_CLK_CTRL_CLKSRC_TX_CLK_x(n) |
            SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);
      }
      //RGMII mode?
      else if(mode == SJA1105_PORT_STATUS_MIIx_MODE_RGMII)
      {
         //Check port speed
         if(speed == SJA1105_PORT_STATUS_MIIx_SPEED_10MBPS)
         {
            //Enable IDIVx and divide by 10
            sja1105WriteSingleReg(interface, SJA1105_IDIV_x_C(n),
               SJA1105_IDIV_x_C_CLKSRC_XO66M_0 | SJA1105_IDIV_x_C_AUTOBLOCK |
               SJA1105_IDIV_x_C_IDIV_DIV10);

            //Set CLKSRC field of RGMII_TXC_x to IDIVx
            sja1105WriteSingleReg(interface, SJA1105_RGMII_TX_CLK_x(n),
               SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIVx(n) |
               SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);
         }
         else if(speed == SJA1105_PORT_STATUS_MIIx_SPEED_100MBPS)
         {
            //Enable IDIVx and divide by 1
            sja1105WriteSingleReg(interface, SJA1105_IDIV_x_C(n),
               SJA1105_IDIV_x_C_CLKSRC_XO66M_0 | SJA1105_IDIV_x_C_IDIV_DIV1);

            //Set CLKSRC field of RGMII_TXC_x to IDIVx
            sja1105WriteSingleReg(interface, SJA1105_RGMII_TX_CLK_x(n),
               SJA1105_MIIx_CLK_CTRL_CLKSRC_IDIVx(n) |
               SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);
         }
         else
         {
            //Disable IDIVx
            sja1105WriteSingleReg(interface, SJA1105_IDIV_x_C(n),
               SJA1105_IDIV_x_C_CLKSRC_XO66M_0 |
               SJA1105_IDIV_x_C_PD);

            //Set CLKSRC field of RGMII_TXC_x to PLL0
            sja1105WriteSingleReg(interface, SJA1105_RGMII_TX_CLK_x(n),
               SJA1105_MIIx_CLK_CTRL_CLKSRC_PLL0 |
               SJA1105_MIIx_CLK_CTRL_AUTOBLOCK);
         }

         //Configure slew rate
         sja1105WriteSingleReg(interface, SJA1105_CFG_PAD_MIIx_TX(n),
            SJA1105_CFG_PAD_MIIx_TX_D32_OS_HIGH |
            SJA1105_CFG_PAD_MIIx_TX_D32_IPUD_PLAIN |
            SJA1105_CFG_PAD_MIIx_TX_D10_OS_HIGH |
            SJA1105_CFG_PAD_MIIx_TX_D10_IPUD_PLAIN |
            SJA1105_CFG_PAD_MIIx_TX_CTRL_OS_HIGH |
            SJA1105_CFG_PAD_MIIx_TX_CTRL_IPUD_PLAIN |
            SJA1105_CFG_PAD_MIIx_TX_CLK_OS_HIGH |
            SJA1105_CFG_PAD_MIIx_TX_CLK_IPUD_PLAIN);
      }
      //SGMII mode?
      else if(mode == SJA1105_PORT_STATUS_MIIx_MODE_SGMII)
      {
         //No special CGU setup is required as the digital clock is always
         //supplied automatically to the SGMII PHY
      }
      //Invalid mode?
      else
      {
         //Report an error
         error = ERROR_FAILURE;
      }
   }
   else
   {
      //The specified port number is not valid
      error = ERROR_INVALID_PARAMETER;
   }

   //Return status code
   return error;
}


/**
 * @brief SJA1105 timer handler
 * @param[in] interface Underlying network interface
 **/

void sja1105Tick(NetInterface *interface)
{
   uint_t port;
   bool_t linkState;

   //Initialize link state
   linkState = FALSE;

   //Loop through the ports
   for(port = SJA1105_PORT1; port <= SJA1105_PORT4; port++)
   {
      //Retrieve current link state
      if(sja1105GetLinkState(interface, port))
      {
         linkState = TRUE;
      }
   }

   //Link up or link down event?
   if(linkState != interface->linkState)
   {
      //Set event flag
      interface->phyEvent = TRUE;
      //Notify the TCP/IP stack of the event
      osSetEvent(&netEvent);
   }
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void sja1105EnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void sja1105DisableIrq(NetInterface *interface)
{
}


/**
 * @brief SJA1105 event handler
 * @param[in] interface Underlying network interface
 **/

void sja1105EventHandler(NetInterface *interface)
{
   uint_t port;
   bool_t linkState;

   //Initialize link state
   linkState = FALSE;

   //Loop through the ports
   for(port = SJA1105_PORT1; port <= SJA1105_PORT4; port++)
   {
      //Retrieve current link state
      if(sja1105GetLinkState(interface, port))
      {
         linkState = TRUE;
      }
   }

   //Link up event?
   if(linkState)
   {
      //Retrieve host interface speed
      interface->linkSpeed = sja1105GetLinkSpeed(interface, SJA1105_PORT0);
      //Retrieve host interface duplex mode
      interface->duplexMode = sja1105GetDuplexMode(interface, SJA1105_PORT0);

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
 * @brief Add tail tag to Ethernet frame
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the payload
 * @param[in,out] offset Offset to the first payload byte
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 * @return Error code
 **/

error_t sja1105TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, NetTxAncillary *ancillary)
{
   //Not implemented
   return NO_ERROR;
}


/**
 * @brief Decode tail tag from incoming Ethernet frame
 * @param[in] interface Underlying network interface
 * @param[in,out] frame Pointer to the received Ethernet frame
 * @param[in,out] length Length of the frame, in bytes
 * @param[in,out] ancillary Additional options passed to the stack along with
 *   the packet
 * @return Error code
 **/

error_t sja1105UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, NetRxAncillary *ancillary)
{
   //Not implemented
   return NO_ERROR;
}


/**
 * @brief Get link state
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Link state
 **/

__weak_func bool_t sja1105GetLinkState(NetInterface *interface, uint8_t port)
{
   //Return current link status
   return FALSE;
}


/**
 * @brief Get link speed
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Link speed
 **/

__weak_func uint32_t sja1105GetLinkSpeed(NetInterface *interface, uint8_t port)
{
   //Return current link speed
   return NIC_LINK_SPEED_UNKNOWN;
}


/**
 * @brief Get duplex mode
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Duplex mode
 **/

NicDuplexMode sja1105GetDuplexMode(NetInterface *interface,
   uint8_t port)
{
   //The xMII interfaces support full duplex mode only
   return NIC_FULL_DUPLEX_MODE;
}


/**
 * @brief Reconfigure port speed
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] speed Port speed
 **/

void sja1105SetPortSpeed(NetInterface *interface, uint8_t port, uint32_t speed)
{
   uint32_t temp;

   //Check port number
   if(port >= SJA1105_PORT0 && port <= SJA1105_PORT4)
   {
      //Debug message
      TRACE_INFO("Configuring port speed (port %u)...\r\n", port - 1);

      //Read the corresponding entry from the MAC configuration table
      sja1105ReadMacConfigEntry(interface, port);

      //Read the MAC Configuration Table Reconfiguration 4 register
      temp = sja1105ReadSingleReg(interface,
         SJA1105_MAC_CONFIG_TABLE_RECONFIG4);

      //Clear the SPEED field
      temp &= ~SJA1105_MAC_CONFIG_TABLE_RECONFIG4_SPEED;

      //Check port speed
      if(speed == NIC_LINK_SPEED_10MBPS)
      {
         //Force 10Mbps operation
         temp |= SJA1105_MAC_CONFIG_TABLE_RECONFIG4_SPEED_10MBPS;
      }
      else if(speed == NIC_LINK_SPEED_100MBPS)
      {
         //Force 100Mbps operation
         temp |= SJA1105_MAC_CONFIG_TABLE_RECONFIG4_SPEED_100MBPS;
      }
      else
      {
         //Force 1Gbps operation
         temp |= SJA1105_MAC_CONFIG_TABLE_RECONFIG4_SPEED_1GBPS;
      }

      //Update the entry
      sja1105WriteSingleReg(interface, SJA1105_MAC_CONFIG_TABLE_RECONFIG4,
         temp);

      //Reconfigure the MAC configuration table
      sja1105WriteMacConfigEntry(interface, port);
   }
}


/**
 * @brief Set port state
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] state Port state
 **/

void sja1105SetPortState(NetInterface *interface, uint8_t port,
   SwitchPortState state)
{
   //Not implemented
}


/**
 * @brief Get port state
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Port state
 **/

SwitchPortState sja1105GetPortState(NetInterface *interface, uint8_t port)
{
   //Not implemented
   return SWITCH_PORT_STATE_UNKNOWN;
}


/**
 * @brief Set aging time for dynamic filtering entries
 * @param[in] interface Underlying network interface
 * @param[in] agingTime Aging time, in seconds
 **/

void sja1105SetAgingTime(NetInterface *interface, uint32_t agingTime)
{
   //Not implemented
}


/**
 * @brief Enable IGMP snooping
 * @param[in] interface Underlying network interface
 * @param[in] enable Enable or disable IGMP snooping
 **/

void sja1105EnableIgmpSnooping(NetInterface *interface, bool_t enable)
{
   //Not implemented
}


/**
 * @brief Enable MLD snooping
 * @param[in] interface Underlying network interface
 * @param[in] enable Enable or disable MLD snooping
 **/

void sja1105EnableMldSnooping(NetInterface *interface, bool_t enable)
{
   //Not implemented
}


/**
 * @brief Enable reserved multicast table
 * @param[in] interface Underlying network interface
 * @param[in] enable Enable or disable reserved group addresses
 **/

void sja1105EnableRsvdMcastTable(NetInterface *interface, bool_t enable)
{
   //Not implemented
}


/**
 * @brief Add a new entry to the static MAC table
 * @param[in] interface Underlying network interface
 * @param[in] entry Pointer to the forwarding database entry
 * @return Error code
 **/

error_t sja1105AddStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry)
{
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
}


/**
 * @brief Remove an entry from the static MAC table
 * @param[in] interface Underlying network interface
 * @param[in] entry Forwarding database entry to remove from the table
 * @return Error code
 **/

error_t sja1105DeleteStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry)
{
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
}


/**
 * @brief Read an entry from the static MAC table
 * @param[in] interface Underlying network interface
 * @param[in] index Zero-based index of the entry to read
 * @param[out] entry Pointer to the forwarding database entry
 * @return Error code
 **/

error_t sja1105GetStaticFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry)
{
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
}


/**
 * @brief Flush static MAC table
 * @param[in] interface Underlying network interface
 **/

void sja1105FlushStaticFdbTable(NetInterface *interface)
{
   //Not implemented
}


/**
 * @brief Set forward ports for unknown multicast packets
 * @param[in] interface Underlying network interface
 * @param[in] enable Enable or disable forwarding of unknown multicast packets
 * @param[in] forwardPorts Port map
 **/

void sja1105SetUnknownMcastFwdPorts(NetInterface *interface,
   bool_t enable, uint32_t forwardPorts)
{
   //Not implemented
}


/**
 * @brief Read an entry from the dynamic MAC table
 * @param[in] interface Underlying network interface
 * @param[in] index Zero-based index of the entry to read
 * @param[out] entry Pointer to the forwarding database entry
 * @return Error code
 **/

error_t sja1105GetDynamicFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry)
{
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
}


/**
 * @brief Flush dynamic MAC table
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 **/

void sja1105FlushDynamicFdbTable(NetInterface *interface, uint8_t port)
{
   //Not implemented
}


/**
 * @brief Reconfigure an entry in the MAC configuration table
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Error code
 **/

error_t sja1105WriteMacConfigEntry(NetInterface *interface, uint8_t port)
{
   error_t error;
   uint_t n;
   uint32_t temp;

   //Check port number
   if(port >= SJA1105_PORT0 && port <= SJA1105_PORT4)
   {
      //Retrieve the zero-based index of the entry
      n = port - SJA1105_PORT0;

      //Set up a write operation
      temp = SJA1105_MAC_CONFIG_TABLE_RECONFIG0_VALID |
         SJA1105_MAC_CONFIG_TABLE_RECONFIG0_RDWRSET;

      //The PORTIDX field specifies the port number which is affected by this
      //dynamic reconfiguration
      temp |= n & SJA1105_MAC_CONFIG_TABLE_RECONFIG0_PORTIDX;

      //Start the write operation
      sja1105WriteSingleReg(interface, SJA1105_MAC_CONFIG_TABLE_RECONFIG0,
         temp);

      //The access completes when the VALID flags is cleared
      do
      {
         //Read the MAC Configuration Table Reconfiguration register 0
         temp = sja1105ReadSingleReg(interface,
            SJA1105_MAC_CONFIG_TABLE_RECONFIG0);

         //Check the value of the VALID flag
      } while((temp & SJA1105_MAC_CONFIG_TABLE_RECONFIG0_VALID) != 0);

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //The specified port number is not valid
      error = ERROR_INVALID_PARAMETER;
   }

   //Return status code
   return error;
}


/**
 * @brief Read an entry from the MAC configuration table
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Error code
 **/

error_t sja1105ReadMacConfigEntry(NetInterface *interface, uint8_t port)
{
   error_t error;
   uint_t n;
   uint32_t temp;

   //Check port number
   if(port >= SJA1105_PORT0 && port <= SJA1105_PORT4)
   {
      //Retrieve the zero-based index of the entry
      n = port - SJA1105_PORT0;

      //Set up a read operation
      temp = SJA1105_MAC_CONFIG_TABLE_RECONFIG0_VALID;
      //The PORTIDX field specifies the port number
      temp |= n & SJA1105_MAC_CONFIG_TABLE_RECONFIG0_PORTIDX;

      //Start the read operation
      sja1105WriteSingleReg(interface, SJA1105_MAC_CONFIG_TABLE_RECONFIG0,
         temp);

      //The access completes when the VALID flags is cleared
      do
      {
         //Read the MAC Configuration Table Reconfiguration register 0
         temp = sja1105ReadSingleReg(interface,
            SJA1105_MAC_CONFIG_TABLE_RECONFIG0);

         //Check the value of the VALID flag
      } while((temp & SJA1105_MAC_CONFIG_TABLE_RECONFIG0_VALID) != 0);

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //The specified port number is not valid
      error = ERROR_INVALID_PARAMETER;
   }

   //Return status code
   return error;
}


/**
 * @brief Write a single register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @param[in] data Register value
 **/

void sja1105WriteSingleReg(NetInterface *interface, uint32_t address,
   uint32_t data)
{
   //Perform write operation
   sja1105WriteMultipleRegs(interface, address, &data, 1);
}


/**
 * @brief Read a single register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @return Register value
 **/

uint32_t sja1105ReadSingleReg(NetInterface *interface, uint32_t address)
{
   uint32_t data;

   //Perform read operation
   sja1105ReadMultipleRegs(interface, address, &data, 1);

   //Return register value
   return data;
}


/**
 * @brief Write multiple registers
 * @param[in] interface Underlying network interface
 * @param[in] address Address of the first register to be written
 * @param[in] data Values of the registers
 * @param[in] count Number of registers to write
 **/


void sja1105WriteMultipleRegs(NetInterface *interface, uint32_t address,
   const uint32_t *data, uint_t count)
{
   uint_t i;
   uint32_t control;

   //Set up a write operation
   control = SJA1105_SPI_CTRL_WRITE;
   //Specify the address
   control |= (address << 4) & SJA1105_SPI_CTRL_ADDR;

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Set up a write operation
   control = SJA1105_SPI_CTRL_WRITE;
   //Specify the address
   control |= (address << 4) & SJA1105_SPI_CTRL_ADDR;

   //Control phase
   interface->spiDriver->transfer((control >> 24) & 0xFF);
   interface->spiDriver->transfer((control >> 16) & 0xFF);
   interface->spiDriver->transfer((control >> 8) & 0xFF);
   interface->spiDriver->transfer(control & 0xFF);

   //Data phase
   for(i = 0; i < count; i++)
   {
      //Write current 32-bit data word
      interface->spiDriver->transfer((data[i] >> 24) & 0xFF);
      interface->spiDriver->transfer((data[i] >> 16) & 0xFF);
      interface->spiDriver->transfer((data[i] >> 8) & 0xFF);
      interface->spiDriver->transfer(data[i] & 0xFF);
   }

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Read multiple registers
 * @param[in] interface Underlying network interface
 * @param[in] address Address of the first register to be read
 * @param[out] data Values of the registers
 * @param[in] count Number of registers to read
 **/

void sja1105ReadMultipleRegs(NetInterface *interface, uint32_t address,
   uint32_t *data, uint_t count)
{
   uint_t i;
   uint32_t control;

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Set up a read operation
   control = SJA1105_SPI_CTRL_READ;
   //Specify the number of words to be read
   control |= (count << 25) & SJA1105_SPI_CTRL_RC;
   //Specify the address
   control |= (address << 4) & SJA1105_SPI_CTRL_ADDR;

   //Control phase
   interface->spiDriver->transfer((control >> 24) & 0xFF);
   interface->spiDriver->transfer((control >> 16) & 0xFF);
   interface->spiDriver->transfer((control >> 8) & 0xFF);
   interface->spiDriver->transfer(control & 0xFF);

   //Data phase
   for(i = 0; i < count; i++)
   {
      data[i] = interface->spiDriver->transfer(0xFF) << 24;
      data[i] |= interface->spiDriver->transfer(0xFF) << 16;
      data[i] |= interface->spiDriver->transfer(0xFF) << 8;
      data[i] |= interface->spiDriver->transfer(0xFF);
   }

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Dump registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void sja1105DumpReg(NetInterface *interface)
{
   uint32_t i;

   //Loop through switch registers
   for(i = 0; i < 16; i++)
   {
      //Display current switch register
      TRACE_DEBUG("0x%02" PRIX32 " : 0x%08" PRIX32 "\r\n",
         i, sja1105ReadSingleReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @param[in] data Register value
 **/

void sja1105WritePhyReg(NetInterface *interface, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   //Write the specified PHY register
   if(interface->smiDriver != NULL)
   {
      interface->smiDriver->writePhyReg(SMI_OPCODE_WRITE, phyAddr, regAddr,
         data);
   }
   else
   {
      interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE, phyAddr, regAddr,
         data);
   }
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t sja1105ReadPhyReg(NetInterface *interface, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;

   //Read the specified PHY register
   if(interface->smiDriver != NULL)
   {
      data = interface->smiDriver->readPhyReg(SMI_OPCODE_READ, phyAddr,
         regAddr);
   }
   else
   {
      data = interface->nicDriver->readPhyReg(SMI_OPCODE_READ, phyAddr,
         regAddr);
   }

   //Return the value of the PHY register
   return data;
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 * @param[in] phyAddr PHY address
 **/

void sja1105DumpPhyReg(NetInterface *interface, uint8_t phyAddr)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         sja1105ReadPhyReg(interface, phyAddr, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
