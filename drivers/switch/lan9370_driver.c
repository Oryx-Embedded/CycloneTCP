/**
 * @file lan9370_driver.c
 * @brief LAN9370 5-port 100Base-T1 Ethernet switch driver
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
#include "drivers/switch/lan9370_driver.h"
#include "debug.h"


/**
 * @brief LAN9370 Ethernet switch driver
 **/

const SwitchDriver lan9370SwitchDriver =
{
   lan9370Init,
   lan9370Tick,
   lan9370EnableIrq,
   lan9370DisableIrq,
   lan9370EventHandler,
   lan9370TagFrame,
   lan9370UntagFrame,
   lan9370GetLinkState,
   lan9370GetLinkSpeed,
   lan9370GetDuplexMode,
   lan9370SetPortState,
   lan9370GetPortState,
   lan9370SetAgingTime,
   lan9370EnableIgmpSnooping,
   lan9370EnableMldSnooping,
   lan9370EnableRsvdMcastTable,
   lan9370AddStaticFdbEntry,
   lan9370DeleteStaticFdbEntry,
   lan9370GetStaticFdbEntry,
   lan9370FlushStaticFdbTable,
   lan9370GetDynamicFdbEntry,
   lan9370FlushDynamicFdbTable,
   lan9370SetUnknownMcastFwdPorts
};


/**
 * @brief Tail tag rules (host to LAN9370)
 **/

const uint16_t lan9370IngressTailTag[5] =
{
   HTONS(LAN9370_TAIL_TAG_VALID | LAN9370_TAIL_TAG_NORMAL_ADDR_LOOKUP),
   HTONS(LAN9370_TAIL_TAG_VALID | LAN9370_TAIL_TAG_PORT_BLOCKING_OVERRIDE |
      LAN9370_TAIL_TAG_DEST_PORT1),
   HTONS(LAN9370_TAIL_TAG_VALID | LAN9370_TAIL_TAG_PORT_BLOCKING_OVERRIDE |
      LAN9370_TAIL_TAG_DEST_PORT2),
   HTONS(LAN9370_TAIL_TAG_VALID | LAN9370_TAIL_TAG_PORT_BLOCKING_OVERRIDE |
      LAN9370_TAIL_TAG_DEST_PORT3),
   HTONS(LAN9370_TAIL_TAG_VALID | LAN9370_TAIL_TAG_PORT_BLOCKING_OVERRIDE |
      LAN9370_TAIL_TAG_DEST_PORT4)
};


/**
 * @brief LAN9370 Ethernet switch initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t lan9370Init(NetInterface *interface)
{
   uint_t port;
   uint16_t temp;

   //Debug message
   TRACE_INFO("Initializing LAN9370...\r\n");

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Initialize SPI interface
      interface->spiDriver->init();

      //Wait for the serial interface to be ready
      do
      {
         //Read CHIP_ID1 register
         temp = lan9370ReadSwitchReg8(interface, LAN9370_CHIP_ID1);

         //The returned data is invalid until the serial interface is ready
      } while(temp != LAN9370_CHIP_ID1_DEFAULT);

      //Enable indirect access from SPI to the VPHY registers
      temp = lan9370ReadSwitchReg8(interface, LAN9370_GLOBAL_CTRL0);
      temp &= ~LAN9370_GLOBAL_CTRL0_APB_PHY_REG_BLK;
      lan9370WriteSwitchReg8(interface, LAN9370_GLOBAL_CTRL0, temp);

      temp = lan9370ReadSwitchReg16(interface, LAN9370_VPHY_SPECIAL_CTRL);
      temp |= LAN9370_VPHY_SPECIAL_CTRL_SPI_INDIRECT_EN;
      lan9370WriteSwitchReg16(interface, LAN9370_VPHY_SPECIAL_CTRL, temp);

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
      //Enable tail tag feature
      temp = lan9370ReadSwitchReg8(interface, LAN9370_PORT5_OP_CTRL0);
      temp |= LAN9370_PORTn_OP_CTRL0_TAIL_TAG_EN;
      lan9370WriteSwitchReg8(interface, LAN9370_PORT5_OP_CTRL0, temp);

      //Disable frame length check
      temp = lan9370ReadSwitchReg8(interface, LAN9370_PORT5_MAC_CTRL0);
      temp &= ~LAN9370_PORTn_MAC_CTRL0_FR_LEN_CHK;
      lan9370WriteSwitchReg8(interface, LAN9370_PORT5_MAC_CTRL0, temp);
#else
      //Disable tail tag feature
      temp = lan9370ReadSwitchReg8(interface, LAN9370_PORT5_OP_CTRL0);
      temp &= ~LAN9370_PORTn_OP_CTRL0_TAIL_TAG_EN;
      lan9370WriteSwitchReg8(interface, LAN9370_PORT5_OP_CTRL0, temp);

      //Enable frame length check
      temp = lan9370ReadSwitchReg8(interface, LAN9370_PORT5_MAC_CTRL0);
      temp |= LAN9370_PORTn_MAC_CTRL0_FRAME_LEN_CHECK_EN;
      lan9370WriteSwitchReg8(interface, LAN9370_PORT5_MAC_CTRL0, temp);
#endif

      //Loop through the ports
      for(port = LAN9370_PORT1; port <= LAN9370_PORT4; port++)
      {
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
         //Port separation mode?
         if(interface->port != 0)
         {
            //Disable packet transmission and address learning
            lan9370SetPortState(interface, port, SWITCH_PORT_STATE_LISTENING);
         }
         else
#endif
         {
            //Enable transmission, reception and address learning
            lan9370SetPortState(interface, port, SWITCH_PORT_STATE_FORWARDING);
         }
      }

      //Restore default age count
      lan9370WriteSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL0,
         LAN9370_SWITCH_LUE_CTRL0_DROP_INVALID_VID |
         LAN9370_SWITCH_LUE_CTRL0_AGE_COUNT_DEFAULT |
         LAN9370_SWITCH_LUE_CTRL0_HASH_OPTION_CRC);

      //Restore default age period
      lan9370WriteSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL3,
         LAN9370_SWITCH_LUE_CTRL3_AGE_PERIOD_7_0_DEFAULT);

      lan9370WriteSwitchReg16(interface, LAN9370_SWITCH_LUE_AGE,
         LAN9370_SWITCH_LUE_AGE_AGE_PERIOD_19_8_DEFAULT);

      //Add internal delay to ingress and egress RGMII clocks
      temp = lan9370ReadSwitchReg8(interface, LAN9370_PORT5_XMII_CTRL1);
      temp |= LAN9370_PORTn_XMII_CTRL1_RGMII_ID_IG;
      temp |= LAN9370_PORTn_XMII_CTRL1_RGMII_ID_EG;
      lan9370WriteSwitchReg8(interface, LAN9370_PORT5_XMII_CTRL1, temp);

      //Start switch operation
      lan9370WriteSwitchReg8(interface, LAN9370_SWITCH_OP,
         LAN9370_SWITCH_OP_START_SWITCH);
   }
   else if(interface->smiDriver != NULL)
   {
      //Initialize serial management interface
      interface->smiDriver->init();
   }
   else
   {
      //Just for sanity
   }

   //Loop through the ports
   for(port = LAN9370_PORT1; port <= LAN9370_PORT4; port++)
   {
      //Debug message
      TRACE_DEBUG("Port %u:\r\n", port);
      //Dump PHY registers for debugging purpose
      lan9370DumpPhyReg(interface, port);
   }

   //Perform custom configuration
   lan9370InitHook(interface);

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief LAN9370 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void lan9370InitHook(NetInterface *interface)
{
}


/**
 * @brief LAN9370 timer handler
 * @param[in] interface Underlying network interface
 **/

void lan9370Tick(NetInterface *interface)
{
   uint_t port;
   bool_t linkState;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Port separation mode?
   if(interface->port != 0)
   {
      uint_t i;
      NetInterface *virtualInterface;

      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Point to the current interface
         virtualInterface = &netInterface[i];

         //Check whether the current virtual interface is attached to the
         //physical interface
         if(virtualInterface == interface ||
            virtualInterface->parent == interface)
         {
            //Retrieve current link state
            linkState = lan9370GetLinkState(interface, virtualInterface->port);

            //Link up or link down event?
            if(linkState != virtualInterface->linkState)
            {
               //Set event flag
               interface->phyEvent = TRUE;
               //Notify the TCP/IP stack of the event
               osSetEvent(&netEvent);
            }
         }
      }
   }
   else
#endif
   {
      //Initialize link state
      linkState = FALSE;

      //Loop through the ports
      for(port = LAN9370_PORT1; port <= LAN9370_PORT4; port++)
      {
         //Retrieve current link state
         if(lan9370GetLinkState(interface, port))
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
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void lan9370EnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void lan9370DisableIrq(NetInterface *interface)
{
}


/**
 * @brief LAN9370 event handler
 * @param[in] interface Underlying network interface
 **/

void lan9370EventHandler(NetInterface *interface)
{
   uint_t port;
   bool_t linkState;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Port separation mode?
   if(interface->port != 0)
   {
      uint_t i;
      NetInterface *virtualInterface;

      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Point to the current interface
         virtualInterface = &netInterface[i];

         //Check whether the current virtual interface is attached to the
         //physical interface
         if(virtualInterface == interface ||
            virtualInterface->parent == interface)
         {
            //Get the port number associated with the current interface
            port = virtualInterface->port;

            //Valid port?
            if(port >= LAN9370_PORT1 && port <= LAN9370_PORT4)
            {
               //Retrieve current link state
               linkState = lan9370GetLinkState(interface, port);

               //Link up event?
               if(linkState && !virtualInterface->linkState)
               {
                  //Retrieve host interface speed
                  interface->linkSpeed = lan9370GetLinkSpeed(interface,
                     LAN9370_PORT5);

                  //Retrieve host interface duplex mode
                  interface->duplexMode = lan9370GetDuplexMode(interface,
                     LAN9370_PORT5);

                  //Adjust MAC configuration parameters for proper operation
                  interface->nicDriver->updateMacConfig(interface);

                  //Check current speed
                  virtualInterface->linkSpeed = lan9370GetLinkSpeed(interface,
                     port);

                  //Check current duplex mode
                  virtualInterface->duplexMode = lan9370GetDuplexMode(interface,
                     port);

                  //Update link state
                  virtualInterface->linkState = TRUE;

                  //Process link state change event
                  nicNotifyLinkChange(virtualInterface);
               }
               //Link down event
               else if(!linkState && virtualInterface->linkState)
               {
                  //Update link state
                  virtualInterface->linkState = FALSE;

                  //Process link state change event
                  nicNotifyLinkChange(virtualInterface);
               }
            }
         }
      }
   }
   else
#endif
   {
      //Initialize link state
      linkState = FALSE;

      //Loop through the ports
      for(port = LAN9370_PORT1; port <= LAN9370_PORT4; port++)
      {
         //Retrieve current link state
         if(lan9370GetLinkState(interface, port))
         {
            linkState = TRUE;
         }
      }

      //Link up event?
      if(linkState)
      {
         //Retrieve host interface speed
         interface->linkSpeed = lan9370GetLinkSpeed(interface, LAN9370_PORT5);
         //Retrieve host interface duplex mode
         interface->duplexMode = lan9370GetDuplexMode(interface, LAN9370_PORT5);

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

error_t lan9370TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, NetTxAncillary *ancillary)
{
   error_t error;

   //Initialize status code
   error = NO_ERROR;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Valid port?
      if(ancillary->port <= LAN9370_PORT5)
      {
         size_t length;
         const uint16_t *tailTag;

         //The two-byte tail tagging is used to indicate the destination port
         tailTag = &lan9370IngressTailTag[ancillary->port];

         //Retrieve the length of the Ethernet frame
         length = netBufferGetLength(buffer) - *offset;

         //The host controller should manually add padding to the packet before
         //inserting the tail tag
         error = ethPadFrame(buffer, &length);

         //Check status code
         if(!error)
         {
            //The tail tag is inserted at the end of the packet, just before
            //the CRC
            error = netBufferAppend(buffer, tailTag, sizeof(uint16_t));
         }
      }
      else
      {
         //The port number is not valid
         error = ERROR_INVALID_PORT;
      }
   }
#endif

   //Return status code
   return error;
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

error_t lan9370UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, NetRxAncillary *ancillary)
{
   error_t error;

   //Initialize status code
   error = NO_ERROR;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Valid Ethernet frame received?
      if(*length >= (sizeof(EthHeader) + sizeof(uint8_t)))
      {
         uint8_t *tailTag;

         //The tail tag is inserted at the end of the packet, just before
         //the CRC
         tailTag = *frame + *length - sizeof(uint8_t);

         //The one byte tail tagging is used to indicate the source port
         ancillary->port = (*tailTag & LAN9370_TAIL_TAG_SRC_PORT) + 1;

         //Strip tail tag from Ethernet frame
         *length -= sizeof(uint8_t);
      }
      else
      {
         //Drop the received frame
         error = ERROR_INVALID_LENGTH;
      }
   }
   else
   {
      //Tail tagging mode cannot be enabled through MDC/MDIO interface
      ancillary->port = 0;
   }
#endif

   //Return status code
   return error;
}


/**
 * @brief Get link state
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Link state
 **/

bool_t lan9370GetLinkState(NetInterface *interface, uint8_t port)
{
   uint16_t value;
   bool_t linkState;

   //Check port number
   if(port >= LAN9370_PORT1 && port <= LAN9370_PORT4)
   {
      //Any link failure condition is latched in the status register. Reading
      //the register twice will always return the actual link status
      value = lan9370ReadPhyReg(interface, port, LAN9370_BASIC_STATUS);
      value = lan9370ReadPhyReg(interface, port, LAN9370_BASIC_STATUS);

      //Retrieve current link state
      linkState = (value & LAN9370_BASIC_STATUS_LINK_STATUS) ? TRUE : FALSE;
   }
   else
   {
      //The specified port number is not valid
      linkState = FALSE;
   }

   //Return link status
   return linkState;
}


/**
 * @brief Get link speed
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Link speed
 **/

uint32_t lan9370GetLinkSpeed(NetInterface *interface, uint8_t port)
{
   uint8_t type;
   uint16_t value;
   uint32_t linkSpeed;

   //Check port number
   if(port >= LAN9370_PORT1 && port <= LAN9370_PORT4)
   {
      //The PHY is only able to operate in 100 Mbps mode
      linkSpeed = NIC_LINK_SPEED_100MBPS;
   }
   else if(port == LAN9370_PORT5)
   {
      //SPI slave mode?
      if(interface->spiDriver != NULL)
      {
         //Read port 5 XMII control 1 register
         value = lan9370ReadSwitchReg8(interface, LAN9370_PORT5_XMII_CTRL1);

         //Retrieve host interface type
         type = value & LAN9370_PORTn_XMII_CTRL1_MII_INTF_SEL;

         //Gigabit interface?
         if(type == LAN9370_PORTn_XMII_CTRL1_MII_INTF_SEL_RGMII &&
            (value & LAN9370_PORTn_XMII_CTRL1_RGMII_SPEED_1000) == 0)
         {
            //1000 Mb/s mode
            linkSpeed = NIC_LINK_SPEED_1GBPS;
         }
         else
         {
            //Read port 5 XMII control 0 register
            value = lan9370ReadSwitchReg8(interface, LAN9370_PORT5_XMII_CTRL0);

            //Retrieve host interface speed
            if((value & LAN9370_PORTn_XMII_CTRL0_SPEED_10_100) != 0)
            {
               //100 Mb/s mode
               linkSpeed = NIC_LINK_SPEED_100MBPS;
            }
            else
            {
               //10 Mb/s mode
               linkSpeed = NIC_LINK_SPEED_10MBPS;
            }
         }
      }
      else
      {
         //The MDC/MDIO interface does not have access to all the configuration
         //registers. It can only access the standard MIIM registers
         linkSpeed = NIC_LINK_SPEED_100MBPS;
      }
   }
   else
   {
      //The specified port number is not valid
      linkSpeed = NIC_LINK_SPEED_UNKNOWN;
   }

   //Return link status
   return linkSpeed;
}


/**
 * @brief Get duplex mode
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Duplex mode
 **/

NicDuplexMode lan9370GetDuplexMode(NetInterface *interface, uint8_t port)
{
   uint16_t value;
   NicDuplexMode duplexMode;

   //Check port number
   if(port >= LAN9370_PORT1 && port <= LAN9370_PORT4)
   {
      //The PHY is only able to operate in full duplex mode
      duplexMode = NIC_FULL_DUPLEX_MODE;
   }
   else if(port == LAN9370_PORT5)
   {
      //SPI slave mode?
      if(interface->spiDriver != NULL)
      {
         //Read port 5 XMII control 0 register
         value = lan9370ReadSwitchReg8(interface, LAN9370_PORT5_XMII_CTRL0);

         //Retrieve host interface duplex mode
         if((value & LAN9370_PORTn_XMII_CTRL0_DUPLEX) != 0)
         {
            duplexMode = NIC_FULL_DUPLEX_MODE;
         }
         else
         {
            duplexMode = NIC_HALF_DUPLEX_MODE;
         }
      }
      else
      {
         //The MDC/MDIO interface does not have access to all the configuration
         //registers. It can only access the standard MIIM registers
         duplexMode = NIC_FULL_DUPLEX_MODE;
      }
   }
   else
   {
      //The specified port number is not valid
      duplexMode = NIC_UNKNOWN_DUPLEX_MODE;
   }

   //Return duplex mode
   return duplexMode;
}


/**
 * @brief Set port state
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] state Port state
 **/

void lan9370SetPortState(NetInterface *interface, uint8_t port,
   SwitchPortState state)
{
   uint8_t temp;

   //Check port number
   if(port >= LAN9370_PORT1 && port <= LAN9370_PORT4)
   {
      //Read MSTP state register
      temp = lan9370ReadSwitchReg8(interface, LAN9370_PORTn_MSTP_STATE(port));

      //Update port state
      switch(state)
      {
      //Listening state
      case SWITCH_PORT_STATE_LISTENING:
         temp &= ~LAN9370_PORTn_MSTP_STATE_TRANSMIT_EN;
         temp |= LAN9370_PORTn_MSTP_STATE_RECEIVE_EN;
         temp |= LAN9370_PORTn_MSTP_STATE_LEARNING_DIS;
         break;

      //Learning state
      case SWITCH_PORT_STATE_LEARNING:
         temp &= ~LAN9370_PORTn_MSTP_STATE_TRANSMIT_EN;
         temp &= ~LAN9370_PORTn_MSTP_STATE_RECEIVE_EN;
         temp &= ~LAN9370_PORTn_MSTP_STATE_LEARNING_DIS;
         break;

      //Forwarding state
      case SWITCH_PORT_STATE_FORWARDING:
         temp |= LAN9370_PORTn_MSTP_STATE_TRANSMIT_EN;
         temp |= LAN9370_PORTn_MSTP_STATE_RECEIVE_EN;
         temp &= ~LAN9370_PORTn_MSTP_STATE_LEARNING_DIS;
         break;

      //Disabled state
      default:
         temp &= ~LAN9370_PORTn_MSTP_STATE_TRANSMIT_EN;
         temp &= ~LAN9370_PORTn_MSTP_STATE_RECEIVE_EN;
         temp |= LAN9370_PORTn_MSTP_STATE_LEARNING_DIS;
         break;
      }

      //Write the value back to MSTP state register
      lan9370WriteSwitchReg8(interface, LAN9370_PORTn_MSTP_STATE(port), temp);
   }
}


/**
 * @brief Get port state
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Port state
 **/

SwitchPortState lan9370GetPortState(NetInterface *interface, uint8_t port)
{
   uint8_t temp;
   SwitchPortState state;

   //Check port number
   if(port >= LAN9370_PORT1 && port <= LAN9370_PORT4)
   {
      //Read MSTP state register
      temp = lan9370ReadSwitchReg8(interface, LAN9370_PORTn_MSTP_STATE(port));

      //Check port state
      if((temp & LAN9370_PORTn_MSTP_STATE_TRANSMIT_EN) == 0 &&
         (temp & LAN9370_PORTn_MSTP_STATE_RECEIVE_EN) == 0 &&
         (temp & LAN9370_PORTn_MSTP_STATE_LEARNING_DIS) != 0)
      {
         //Disabled state
         state = SWITCH_PORT_STATE_DISABLED;
      }
      else if((temp & LAN9370_PORTn_MSTP_STATE_TRANSMIT_EN) == 0 &&
         (temp & LAN9370_PORTn_MSTP_STATE_RECEIVE_EN) != 0 &&
         (temp & LAN9370_PORTn_MSTP_STATE_LEARNING_DIS) != 0)
      {
         //Listening state
         state = SWITCH_PORT_STATE_LISTENING;
      }
      else if((temp & LAN9370_PORTn_MSTP_STATE_TRANSMIT_EN) == 0 &&
         (temp & LAN9370_PORTn_MSTP_STATE_RECEIVE_EN) == 0 &&
         (temp & LAN9370_PORTn_MSTP_STATE_LEARNING_DIS) == 0)
      {
         //Learning state
         state = SWITCH_PORT_STATE_LEARNING;
      }
      else if((temp & LAN9370_PORTn_MSTP_STATE_TRANSMIT_EN) != 0 &&
         (temp & LAN9370_PORTn_MSTP_STATE_RECEIVE_EN) != 0 &&
         (temp & LAN9370_PORTn_MSTP_STATE_LEARNING_DIS) == 0)
      {
         //Forwarding state
         state = SWITCH_PORT_STATE_FORWARDING;
      }
      else
      {
         //Unknown state
         state = SWITCH_PORT_STATE_UNKNOWN;
      }
   }
   else
   {
      //The specified port number is not valid
      state = SWITCH_PORT_STATE_DISABLED;
   }

   //Return port state
   return state;
}


/**
 * @brief Set aging time for dynamic filtering entries
 * @param[in] interface Underlying network interface
 * @param[in] agingTime Aging time, in seconds
 **/

void lan9370SetAgingTime(NetInterface *interface, uint32_t agingTime)
{
   //The Age Period in combination with the Age Count field determines the
   //aging time of dynamic entries in the address lookup table
   agingTime = (agingTime + 3) / 4;

   //The full Age Period has 20 bits
   agingTime = MIN(agingTime, 0xFFFFF);

   //Write the lower 8 bits
   lan9370WriteSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL3,
      agingTime & LAN9370_SWITCH_LUE_CTRL3_AGE_PERIOD_7_0);

   //Write the upper 12 bits
   lan9370WriteSwitchReg16(interface, LAN9370_SWITCH_LUE_AGE,
      (agingTime >> 8) & LAN9370_SWITCH_LUE_AGE_AGE_PERIOD_19_8);
}


/**
 * @brief Enable IGMP snooping
 * @param[in] interface Underlying network interface
 * @param[in] enable Enable or disable IGMP snooping
 **/

void lan9370EnableIgmpSnooping(NetInterface *interface, bool_t enable)
{
   uint8_t temp;

   //Read the Global Port Mirroring and Snooping Control register
   temp = lan9370ReadSwitchReg8(interface,
      LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL);

   //Enable or disable IGMP snooping
   if(enable)
   {
      temp |= LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL_IGMP_SNOOP_EN;
   }
   else
   {
      temp &= ~LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL_IGMP_SNOOP_EN;
   }

   //Write the value back to Global Port Mirroring and Snooping Control register
   lan9370WriteSwitchReg8(interface, LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL,
      temp);
}


/**
 * @brief Enable MLD snooping
 * @param[in] interface Underlying network interface
 * @param[in] enable Enable or disable MLD snooping
 **/

void lan9370EnableMldSnooping(NetInterface *interface, bool_t enable)
{
   uint8_t temp;

   //Read the Global Port Mirroring and Snooping Control register
   temp = lan9370ReadSwitchReg8(interface,
      LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL);

   //Enable or disable MLD snooping
   if(enable)
   {
      temp |= LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL_MLD_SNOOP_EN;
   }
   else
   {
      temp &= ~LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL_MLD_SNOOP_EN;
   }

   //Write the value back to Global Port Mirroring and Snooping Control register
   lan9370WriteSwitchReg8(interface, LAN9370_GLOBAL_PORT_MIRROR_SNOOP_CTRL,
      temp);
}


/**
 * @brief Enable reserved multicast table
 * @param[in] interface Underlying network interface
 * @param[in] enable Enable or disable reserved group addresses
 **/

void lan9370EnableRsvdMcastTable(NetInterface *interface, bool_t enable)
{
   uint8_t temp;

   //Read the Switch Lookup Engine Control 0 register
   temp = lan9370ReadSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL0);

   //Enable or disable the reserved multicast table
   if(enable)
   {
      temp |= LAN9370_SWITCH_LUE_CTRL0_RESERVED_LUE_EN;
   }
   else
   {
      temp &= ~LAN9370_SWITCH_LUE_CTRL0_RESERVED_LUE_EN;
   }

   //Write the value back to Switch Lookup Engine Control 0 register
   lan9370WriteSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL0, temp);
}


/**
 * @brief Add a new entry to the static MAC table
 * @param[in] interface Underlying network interface
 * @param[in] entry Pointer to the forwarding database entry
 * @return Error code
 **/

error_t lan9370AddStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry)
{
   error_t error;
   uint_t i;
   uint_t j;
   uint32_t value;
   SwitchFdbEntry currentEntry;

   //Keep track of the first free entry
   j = LAN9370_STATIC_MAC_TABLE_SIZE;

   //Loop through the static MAC table
   for(i = 0; i < LAN9370_STATIC_MAC_TABLE_SIZE; i++)
   {
      //Read current entry
      error = lan9370GetStaticFdbEntry(interface, i, &currentEntry);

      //Valid entry?
      if(!error)
      {
         //Check whether the table already contains the specified MAC address
         if(macCompAddr(&currentEntry.macAddr, &entry->macAddr))
         {
            j = i;
            break;
         }
      }
      else
      {
         //Keep track of the first free entry
         if(j == LAN9370_STATIC_MAC_TABLE_SIZE)
         {
            j = i;
         }
      }
   }

   //Any entry available?
   if(j < LAN9370_STATIC_MAC_TABLE_SIZE)
   {
      //Write the Static Address Table Entry 1 register
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY1,
         LAN9370_STATIC_TABLE_ENTRY1_VALID);

      //Set the relevant forward ports
      if(entry->destPorts == SWITCH_CPU_PORT_MASK)
      {
         value = LAN9370_PORT5_MASK;
      }
      else
      {
         value = entry->destPorts & LAN9370_PORT_MASK;
      }

      //Enable overriding of port state
      if(entry->override)
      {
         value |= LAN9370_STATIC_TABLE_ENTRY2_OVERRIDE;
      }

      //Write the Static Address Table Entry 2 register
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY2, value);

      //Copy MAC address (first 16 bits)
      value = (entry->macAddr.b[0] << 8) | entry->macAddr.b[1];

      //Write the Static Address Table Entry 3 register
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY3, value);

      //Copy MAC address (last 32 bits)
      value = (entry->macAddr.b[2] << 24) | (entry->macAddr.b[3] << 16) |
         (entry->macAddr.b[4] << 8) | entry->macAddr.b[5];

      //Write the Static Address Table Entry 4 register
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY4, value);

      //Setup a write operation
      value = LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_SELECT_STATIC |
         LAN9370_STATIC_MCAST_TABLE_CTRL_ACTION_WRITE |
         LAN9370_STATIC_MCAST_TABLE_CTRL_START_FINISH;

      //Write the TABLE_INDEX field with the 8-bit index value
      value |= (j << 8) & LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_INDEX;

      //Start the write operation
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_MCAST_TABLE_CTRL,
         value);

      //When the operation is complete, the START_FINISH bit will be cleared
      //automatically
      do
      {
         //Read the Static Address and Reserved Multicast Table Control register
         value = lan9370ReadSwitchReg32(interface,
            LAN9370_STATIC_MCAST_TABLE_CTRL);

         //Poll the START_FINISH bit
      } while((value & LAN9370_STATIC_MCAST_TABLE_CTRL_START_FINISH) != 0);

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //The static MAC table is full
      error = ERROR_TABLE_FULL;
   }

   //Return status code
   return error;
}


/**
 * @brief Remove an entry from the static MAC table
 * @param[in] interface Underlying network interface
 * @param[in] entry Forwarding database entry to remove from the table
 * @return Error code
 **/

error_t lan9370DeleteStaticFdbEntry(NetInterface *interface,
   const SwitchFdbEntry *entry)
{
   error_t error;
   uint_t j;
   uint32_t value;
   SwitchFdbEntry currentEntry;

   //Loop through the static MAC table
   for(j = 0; j < LAN9370_STATIC_MAC_TABLE_SIZE; j++)
   {
      //Read current entry
      error = lan9370GetStaticFdbEntry(interface, j, &currentEntry);

      //Valid entry?
      if(!error)
      {
         //Check whether the table contains the specified MAC address
         if(macCompAddr(&currentEntry.macAddr, &entry->macAddr))
         {
            break;
         }
      }
   }

   //Any matching entry?
   if(j < LAN9370_STATIC_MAC_TABLE_SIZE)
   {
      //Clear Static Address Table Entry registers
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY1, 0);
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY2, 0);
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY3, 0);
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY4, 0);

      //Setup a write operation
      value = LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_SELECT_STATIC |
         LAN9370_STATIC_MCAST_TABLE_CTRL_ACTION_WRITE |
         LAN9370_STATIC_MCAST_TABLE_CTRL_START_FINISH;

      //Write the TABLE_INDEX field with the 8-bit index value
      value |= (j << 8) & LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_INDEX;

      //Start the write operation
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_MCAST_TABLE_CTRL,
         value);

      //When the operation is complete, the START_FINISH bit will be cleared
      //automatically
      do
      {
         //Read the Static Address and Reserved Multicast Table Control register
         value = lan9370ReadSwitchReg32(interface,
            LAN9370_STATIC_MCAST_TABLE_CTRL);

         //Poll the START_FINISH bit
      } while((value & LAN9370_STATIC_MCAST_TABLE_CTRL_START_FINISH) != 0);

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //The static MAC table does not contain the specified address
      error = ERROR_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Read an entry from the static MAC table
 * @param[in] interface Underlying network interface
 * @param[in] index Zero-based index of the entry to read
 * @param[out] entry Pointer to the forwarding database entry
 * @return Error code
 **/

error_t lan9370GetStaticFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry)
{
   error_t error;
   uint32_t value;

   //Check index parameter
   if(index < LAN9370_STATIC_MAC_TABLE_SIZE)
   {
      //Setup a write operation
      value = LAN9370_STATIC_MCAST_TABLE_CTRL_DIRECT_ACCESS |
         LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_SELECT_STATIC |
         LAN9370_STATIC_MCAST_TABLE_CTRL_ACTION_READ |
         LAN9370_STATIC_MCAST_TABLE_CTRL_START_FINISH;

      //Write the TABLE_INDEX field with the 8-bit index value
      value |= (index << 8) & LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_INDEX;

      //Start the read operation
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_MCAST_TABLE_CTRL,
         value);

      //When the operation is complete, the START_FINISH bit will be cleared
      //automatically
      do
      {
         //Read the Static Address and Reserved Multicast Table Control register
         value = lan9370ReadSwitchReg32(interface,
            LAN9370_STATIC_MCAST_TABLE_CTRL);

         //Poll the START_FINISH bit
      } while((value & LAN9370_STATIC_MCAST_TABLE_CTRL_START_FINISH) != 0);

      //Read the Static Address Table Entry 1 register
      value = lan9370ReadSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY1);

      //Valid entry?
      if((value & LAN9370_STATIC_TABLE_ENTRY1_VALID) != 0)
      {
         //Read the Static Address Table Entry 2 register
         value = lan9370ReadSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY2);

         //Retrieve the ports associated with this MAC address
         entry->srcPort = 0;
         entry->destPorts = value & LAN9370_STATIC_TABLE_ENTRY2_PORT_FORWARD;

         //Check the value of the OVERRIDE bit
         if((value & LAN9370_STATIC_TABLE_ENTRY2_OVERRIDE) != 0)
         {
            entry->override = TRUE;
         }
         else
         {
            entry->override = FALSE;
         }

         //Read the Static Address Table Entry 3 register
         value = lan9370ReadSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY3);

         //Copy MAC address (first 16 bits)
         entry->macAddr.b[0] = (value >> 8) & 0xFF;
         entry->macAddr.b[1] = value & 0xFF;

         //Read the Static Address Table Entry 4 register
         value = lan9370ReadSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY4);

         //Copy MAC address (last 32 bits)
         entry->macAddr.b[2] = (value >> 24) & 0xFF;
         entry->macAddr.b[3] = (value >> 16) & 0xFF;
         entry->macAddr.b[4] = (value >> 8) & 0xFF;
         entry->macAddr.b[5] = value & 0xFF;

         //Successful processing
         error = NO_ERROR;
      }
      else
      {
         //The entry is not valid
         error = ERROR_INVALID_ENTRY;
      }
   }
   else
   {
      //The end of the table has been reached
      error = ERROR_END_OF_TABLE;
   }

   //Return status code
   return error;
}


/**
 * @brief Flush static MAC table
 * @param[in] interface Underlying network interface
 **/

void lan9370FlushStaticFdbTable(NetInterface *interface)
{
   uint_t i;
   uint32_t value;

   //Loop through the static MAC table
   for(i = 0; i < LAN9370_STATIC_MAC_TABLE_SIZE; i++)
   {
      //Clear Static Address Table Entry registers
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY1, 0);
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY2, 0);
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY3, 0);
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_TABLE_ENTRY4, 0);

      //Setup a write operation
      value = LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_SELECT_STATIC |
         LAN9370_STATIC_MCAST_TABLE_CTRL_ACTION_WRITE |
         LAN9370_STATIC_MCAST_TABLE_CTRL_START_FINISH;

      //Write the TABLE_INDEX field with the 8-bit index value
      value |= (i << 8) & LAN9370_STATIC_MCAST_TABLE_CTRL_TABLE_INDEX;

      //Start the write operation
      lan9370WriteSwitchReg32(interface, LAN9370_STATIC_MCAST_TABLE_CTRL,
         value);

      //When the operation is complete, the START_FINISH bit will be cleared
      //automatically
      do
      {
         //Read the Static Address and Reserved Multicast Table Control register
         value = lan9370ReadSwitchReg32(interface,
            LAN9370_STATIC_MCAST_TABLE_CTRL);

         //Poll the START_FINISH bit
      } while((value & LAN9370_STATIC_MCAST_TABLE_CTRL_START_FINISH) != 0);
   }
}


/**
 * @brief Read an entry from the dynamic MAC table
 * @param[in] interface Underlying network interface
 * @param[in] index Zero-based index of the entry to read
 * @param[out] entry Pointer to the forwarding database entry
 * @return Error code
 **/

error_t lan9370GetDynamicFdbEntry(NetInterface *interface, uint_t index,
   SwitchFdbEntry *entry)
{
   error_t error;
   uint32_t value;

   //First entry?
   if(index == 0)
   {
      //Clear the ALU Table Access Control register to stop any operation
      lan9370WriteSwitchReg32(interface, LAN9370_ALU_TABLE_CTRL, 0);

      //Start the search operation
      lan9370WriteSwitchReg32(interface, LAN9370_ALU_TABLE_CTRL,
         LAN9370_ALU_TABLE_CTRL_START_FINISH |
         LAN9370_ALU_TABLE_CTRL_ACTION_SEARCH);
   }

   //Poll the VALID_ENTRY_OR_SEARCH_END bit until it is set
   do
   {
      //Read the ALU Table Access Control register
      value = lan9370ReadSwitchReg32(interface, LAN9370_ALU_TABLE_CTRL);

      //This bit goes high to indicate either a new valid entry is returned or
      //the search is complete
   } while((value & LAN9370_ALU_TABLE_CTRL_VALID_ENTRY_OR_SEARCH_END) == 0);

   //Check whether the next valid entry is ready
   if((value & LAN9370_ALU_TABLE_CTRL_VALID) != 0)
   {
      //Store the data from the ALU table entry
      entry->destPorts = 0;
      entry->override = FALSE;

      //Read the ALU Table Entry 1 and 2 registers
      value = lan9370ReadSwitchReg32(interface, LAN9370_ALU_TABLE_ENTRY1);
      value = lan9370ReadSwitchReg32(interface, LAN9370_ALU_TABLE_ENTRY2);

      //Retrieve the port associated with this MAC address
      switch(value & LAN9370_ALU_TABLE_ENTRY2_PORT_FORWARD)
      {
      case LAN9370_ALU_TABLE_ENTRY2_PORT1_FORWARD:
         entry->srcPort = LAN9370_PORT1;
         break;
      case LAN9370_ALU_TABLE_ENTRY2_PORT2_FORWARD:
         entry->srcPort = LAN9370_PORT2;
         break;
      case LAN9370_ALU_TABLE_ENTRY2_PORT3_FORWARD:
         entry->srcPort = LAN9370_PORT3;
         break;
      case LAN9370_ALU_TABLE_ENTRY2_PORT4_FORWARD:
         entry->srcPort = LAN9370_PORT4;
         break;
      case LAN9370_ALU_TABLE_ENTRY2_PORT5_FORWARD:
         entry->srcPort = LAN9370_PORT5;
         break;
      default:
         entry->srcPort = 0;
         break;
      }

      //Read the ALU Table Entry 3 register
      value = lan9370ReadSwitchReg32(interface, LAN9370_ALU_TABLE_ENTRY3);

      //Copy MAC address (first 16 bits)
      entry->macAddr.b[0] = (value >> 8) & 0xFF;
      entry->macAddr.b[1] = value & 0xFF;

      //Read the ALU Table Entry 4 register
      value = lan9370ReadSwitchReg32(interface, LAN9370_ALU_TABLE_ENTRY4);

      //Copy MAC address (last 32 bits)
      entry->macAddr.b[2] = (value >> 24) & 0xFF;
      entry->macAddr.b[3] = (value >> 16) & 0xFF;
      entry->macAddr.b[4] = (value >> 8) & 0xFF;
      entry->macAddr.b[5] = value & 0xFF;

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //The search can be stopped any time by setting the START_FINISH bit to 0
      lan9370WriteSwitchReg32(interface, LAN9370_ALU_TABLE_CTRL, 0);

      //The end of the table has been reached
      error = ERROR_END_OF_TABLE;
   }

   //Return status code
   return error;
}


/**
 * @brief Flush dynamic MAC table
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 **/

void lan9370FlushDynamicFdbTable(NetInterface *interface, uint8_t port)
{
   uint_t temp;
   uint8_t state;

   //Flush only dynamic table entries
   temp = lan9370ReadSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL2);
   temp &= ~LAN9370_SWITCH_LUE_CTRL2_FLUSH_OPTION;
   temp |= LAN9370_SWITCH_LUE_CTRL2_FLUSH_OPTION_DYNAMIC;
   lan9370WriteSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL2, temp);

   //Valid port number?
   if(port >= LAN9370_PORT1 && port <= LAN9370_PORT5)
   {
      //Save the current state of the port
      state = lan9370ReadSwitchReg8(interface, LAN9370_PORTn_MSTP_STATE(port));

      //Turn off learning capability
      lan9370WriteSwitchReg8(interface, LAN9370_PORTn_MSTP_STATE(port),
         state | LAN9370_PORTn_MSTP_STATE_LEARNING_DIS);

      //All the entries associated with a port that has its learning capability
      //being turned off will be flushed
      temp = lan9370ReadSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL1);
      temp |= LAN9370_SWITCH_LUE_CTRL1_FLUSH_MSTP_ENTRIES;
      lan9370WriteSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL1, temp);

      //Restore the original state of the port
      lan9370WriteSwitchReg8(interface, LAN9370_PORTn_MSTP_STATE(port), state);
   }
   else
   {
      //Trigger a flush of the entire address lookup table
      temp = lan9370ReadSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL1);
      temp |= LAN9370_SWITCH_LUE_CTRL1_FLUSH_ALU_TABLE;
      lan9370WriteSwitchReg8(interface, LAN9370_SWITCH_LUE_CTRL1, temp);
   }
}


/**
 * @brief Set forward ports for unknown multicast packets
 * @param[in] interface Underlying network interface
 * @param[in] enable Enable or disable forwarding of unknown multicast packets
 * @param[in] forwardPorts Port map
 **/

void lan9370SetUnknownMcastFwdPorts(NetInterface *interface,
   bool_t enable, uint32_t forwardPorts)
{
   uint32_t temp;

   //Read Unknown Multicast Control register
   temp = lan9370ReadSwitchReg32(interface, LAN9370_UNKONWN_MULTICAST_CTRL);

   //Clear port map
   temp &= ~LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP;

   //Enable or disable forwarding of unknown multicast packets
   if(enable)
   {
      //Enable forwarding
      temp |= LAN9370_UNKONWN_MULTICAST_CTRL_FWD;

      //Check whether unknown multicast packets should be forwarded to the CPU port
      if((forwardPorts & SWITCH_CPU_PORT_MASK) != 0)
      {
         temp |= LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP_PORT5;
      }

      //Select the desired forward ports
      temp |= forwardPorts & LAN9370_UNKONWN_MULTICAST_CTRL_FWD_MAP_ALL;
   }
   else
   {
      //Disable forwarding
      temp &= ~LAN9370_UNKONWN_MULTICAST_CTRL_FWD;
   }

   //Write the value back to Unknown Multicast Control register
   lan9370WriteSwitchReg32(interface, LAN9370_UNKONWN_MULTICAST_CTRL, temp);
}


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void lan9370WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data)
{
   uint16_t n;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //The SPI interface provides access to all PHY registers
      n = LAN9370_PORTn_T1_PHY_REG(port, address);

      //Set register address
      lan9370WriteSwitchReg16(interface, LAN9370_IND_ADDR, n);
      //Set register value
      lan9370WriteSwitchReg16(interface, LAN9370_IND_DATA, data);

      //Start a write operation
      lan9370WriteSwitchReg16(interface, LAN9370_IND_CTRL,
         LAN9370_IND_CTRL_OP_WRITE | LAN9370_IND_CTRL_BUSY);

      //Wait for the write to complete
      while((lan9370ReadSwitchReg16(interface, LAN9370_IND_CTRL) &
         LAN9370_IND_CTRL_BUSY) != 0)
      {
      }
   }
   else if(interface->smiDriver != NULL)
   {
      //Write the specified PHY register
      interface->smiDriver->writePhyReg(SMI_OPCODE_WRITE, port, address, data);
   }
   else
   {
      //Write the specified PHY register
      interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE, port, address, data);
   }
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t lan9370ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address)
{
   uint16_t n;
   uint16_t data;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //The SPI interface provides access to all PHY registers
      n = LAN9370_PORTn_T1_PHY_REG(port, address);

      //Set register address
      lan9370WriteSwitchReg16(interface, LAN9370_IND_ADDR, n);

      //Start a read operation
      lan9370WriteSwitchReg16(interface, LAN9370_IND_CTRL,
         LAN9370_IND_CTRL_OP_READ | LAN9370_IND_CTRL_BUSY);

      //Wait for the read to complete
      while((lan9370ReadSwitchReg16(interface, LAN9370_IND_CTRL) &
         LAN9370_IND_CTRL_BUSY) != 0)
      {
      }

      //Read the 16-bit value
      data = lan9370ReadSwitchReg16(interface, LAN9370_IND_DATA);
   }
   else if(interface->smiDriver != NULL)
   {
      //Read the specified PHY register
      data = interface->smiDriver->readPhyReg(SMI_OPCODE_READ, port, address);
   }
   else
   {
      //Read the specified PHY register
      data = interface->nicDriver->readPhyReg(SMI_OPCODE_READ, port, address);
   }

   //Return register value
   return data;
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 **/

void lan9370DumpPhyReg(NetInterface *interface, uint8_t port)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         lan9370ReadPhyReg(interface, port, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Write extended register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] bank Register bank
 * @param[in] addr Register address
 * @param[in] data Extended register value
 **/

void lan9370WriteExtReg(NetInterface *interface, uint8_t port, uint8_t bank,
   uint8_t addr, uint16_t data)
{
   uint16_t temp;

   //Set up a write operation
   temp = LAN9370_EXT_REG_CTL_WRITE_CONTROL;
   //Select register bank
   temp |= (bank << 8) & LAN9370_EXT_REG_CTL_REGISTER_BANK;
   //Select register address
   temp |= addr & LAN9370_EXT_REG_CTL_REGISTER_ADDR;

   //Write the EXT_REG_WR_DATA register with the desired 16-bit data
   lan9370WritePhyReg(interface, port, LAN9370_EXT_REG_WR_DATA, data);

   //Write the EXT_REG_CTL register
   lan9370WritePhyReg(interface, port, LAN9370_EXT_REG_CTL, temp);
}


/**
 * @brief Read extended register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] bank Register bank
 * @param[in] addr Register address
 * @return Extended register value
 **/

uint16_t lan9370ReadExtReg(NetInterface *interface, uint8_t port, uint8_t bank,
   uint8_t addr)
{
   uint16_t temp;

   //Set up a read operation
   temp = LAN9370_EXT_REG_CTL_READ_CONTROL;
   //Select register bank
   temp |= (bank << 8) & LAN9370_EXT_REG_CTL_REGISTER_BANK;
   //Select register address
   temp |= addr & LAN9370_EXT_REG_CTL_REGISTER_ADDR;

   //Write the EXT_REG_CTL register
   lan9370WritePhyReg(interface, port, LAN9370_EXT_REG_CTL, temp);

   //Read the EXT_REG_RD_DATA register
   return lan9370ReadPhyReg(interface, port, LAN9370_EXT_REG_RD_DATA);
}


/**
 * @brief Write switch register (8 bits)
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @param[in] data Register value
 **/

void lan9370WriteSwitchReg8(NetInterface *interface, uint16_t address,
   uint8_t data)
{
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a write operation
      command = LAN9370_SPI_CMD_WRITE;
      //Set register address
      command |= (address << 5) & LAN9370_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Write 8-bit data
      interface->spiDriver->transfer(data);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //The MDC/MDIO interface does not have access to all the configuration
      //registers. It can only access the standard MIIM registers
   }
}


/**
 * @brief Read switch register (8 bits)
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @return Register value
 **/

uint8_t lan9370ReadSwitchReg8(NetInterface *interface, uint16_t address)
{
   uint8_t data;
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a read operation
      command = LAN9370_SPI_CMD_READ;
      //Set register address
      command |= (address << 5) & LAN9370_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Read 8-bit data
      data = interface->spiDriver->transfer(0xFF);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //The MDC/MDIO interface does not have access to all the configuration
      //registers. It can only access the standard MIIM registers
      data = 0;
   }

   //Return register value
   return data;
}


/**
 * @brief Write switch register (16 bits)
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @param[in] data Register value
 **/

void lan9370WriteSwitchReg16(NetInterface *interface, uint16_t address,
   uint16_t data)
{
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a write operation
      command = LAN9370_SPI_CMD_WRITE;
      //Set register address
      command |= (address << 5) & LAN9370_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Write 16-bit data
      interface->spiDriver->transfer((data >> 8) & 0xFF);
      interface->spiDriver->transfer(data & 0xFF);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //The MDC/MDIO interface does not have access to all the configuration
      //registers. It can only access the standard MIIM registers
   }
}


/**
 * @brief Read switch register (16 bits)
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @return Register value
 **/

uint16_t lan9370ReadSwitchReg16(NetInterface *interface, uint16_t address)
{
   uint16_t data;
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a read operation
      command = LAN9370_SPI_CMD_READ;
      //Set register address
      command |= (address << 5) & LAN9370_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Read 16-bit data
      data = interface->spiDriver->transfer(0xFF) << 8;
      data |= interface->spiDriver->transfer(0xFF);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //The MDC/MDIO interface does not have access to all the configuration
      //registers. It can only access the standard MIIM registers
      data = 0;
   }

   //Return register value
   return data;
}


/**
 * @brief Write switch register (32 bits)
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @param[in] data Register value
 **/

void lan9370WriteSwitchReg32(NetInterface *interface, uint16_t address,
   uint32_t data)
{
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a write operation
      command = LAN9370_SPI_CMD_WRITE;
      //Set register address
      command |= (address << 5) & LAN9370_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Write 32-bit data
      interface->spiDriver->transfer((data >> 24) & 0xFF);
      interface->spiDriver->transfer((data >> 16) & 0xFF);
      interface->spiDriver->transfer((data >> 8) & 0xFF);
      interface->spiDriver->transfer(data & 0xFF);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //The MDC/MDIO interface does not have access to all the configuration
      //registers. It can only access the standard MIIM registers
   }
}


/**
 * @brief Read switch register (32 bits)
 * @param[in] interface Underlying network interface
 * @param[in] address Switch register address
 * @return Register value
 **/

uint32_t lan9370ReadSwitchReg32(NetInterface *interface, uint16_t address)
{
   uint32_t data;
   uint32_t command;

   //SPI slave mode?
   if(interface->spiDriver != NULL)
   {
      //Set up a read operation
      command = LAN9370_SPI_CMD_READ;
      //Set register address
      command |= (address << 5) & LAN9370_SPI_CMD_ADDR;

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Write 32-bit command
      interface->spiDriver->transfer((command >> 24) & 0xFF);
      interface->spiDriver->transfer((command >> 16) & 0xFF);
      interface->spiDriver->transfer((command >> 8) & 0xFF);
      interface->spiDriver->transfer(command & 0xFF);

      //Read 32-bit data
      data = interface->spiDriver->transfer(0xFF) << 24;
      data |= interface->spiDriver->transfer(0xFF) << 16;
      data |= interface->spiDriver->transfer(0xFF) << 8;
      data |= interface->spiDriver->transfer(0xFF);

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();
   }
   else
   {
      //The MDC/MDIO interface does not have access to all the configuration
      //registers. It can only access the standard MIIM registers
      data = 0;
   }

   //Return register value
   return data;
}
