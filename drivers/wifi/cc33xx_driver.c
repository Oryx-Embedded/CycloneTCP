/**
 * @file cc33xx_driver.c
 * @brief CC3300/CC3301 Wi-Fi controller
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
#include <stdint.h>
#include "wlan_if.h"
#include "errors.h"
#include "core/net.h"
#include "drivers/wifi/cc33xx_driver.h"
#include "debug.h"

//Underlying network interface
NetInterface *cc33xxStaInterface = NULL;
NetInterface *cc33xxApInterface = NULL;

//Forward declaration of functions
void cc33xxEventCallback(WlanEvent_t *wlanEvent);
void cc33xxStaRxCallback(WlanRole_e role, uint8_t *data, uint32_t length);
void cc33xxApRxCallback(WlanRole_e role, uint8_t *data, uint32_t length);


/**
 * @brief CC33xx driver (STA mode)
 **/

const NicDriver cc33xxStaDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   cc33xxInit,
   cc33xxTick,
   cc33xxEnableIrq,
   cc33xxDisableIrq,
   cc33xxEventHandler,
   cc33xxSendPacket,
   cc33xxUpdateMacAddrFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   TRUE
};


/**
 * @brief CC33xx driver (AP mode)
 **/

const NicDriver cc33xxApDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   cc33xxInit,
   cc33xxTick,
   cc33xxEnableIrq,
   cc33xxDisableIrq,
   cc33xxEventHandler,
   cc33xxSendPacket,
   cc33xxUpdateMacAddrFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   TRUE
};


/**
 * @brief CC33xx initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t cc33xxInit(NetInterface *interface)
{
   int_t ret;
   WlanMacAddress_t macAddrParams;

   //Initialize status code
   ret = WLAN_RET_CODE_OK;

   //STA or AP mode?
   if(interface->nicDriver == &cc33xxStaDriver)
   {
      //Debug message
      TRACE_INFO("Initializing CC33xx (STA mode)...\r\n");
   }
   else
   {
      //Debug message
      TRACE_INFO("Initializing CC33xx (AP mode)...\r\n");
   }

   //Initialization sequence is performed once
   if(cc33xxStaInterface == NULL && cc33xxApInterface == NULL)
   {
      //Wi-Fi host driver initialization
      ret = Wlan_Start(cc33xxEventCallback);
   }

   //Check status code
   if(ret == WLAN_RET_CODE_OK)
   {
      //STA or AP mode?
      if(interface->nicDriver == &cc33xxStaDriver)
      {
         //Save underlying network interface (STA mode)
         cc33xxStaInterface = interface;

         //Optionally set the MAC address
         if(macCompAddr(&interface->macAddr, &MAC_UNSPECIFIED_ADDR))
         {
            //Use the factory preprogrammed station address
            macAddrParams.roleType = WLAN_ROLE_STA;
            osMemset(macAddrParams.pMacAddress, 0, 6);
            ret = Wlan_Get(WLAN_GET_MACADDRESS, &macAddrParams);

            //Check status code
            if(ret == WLAN_RET_CODE_OK)
            {
               //Generate the 64-bit interface identifier
               macAddrToEui64(&interface->macAddr, &interface->eui64);
            }
         }
         else
         {
            //Override the factory preprogrammed address
            macAddrParams.roleType = WLAN_ROLE_STA;
            osMemcpy(macAddrParams.pMacAddress, interface->macAddr.b, 6);
            ret = Wlan_Set(WLAN_SET_MACADDRESS, &macAddrParams);
         }

         //Check status code
         if(ret == WLAN_RET_CODE_OK)
         {
            //Register RX callback
            ret = Wlan_EtherPacketRecvRegisterCallback(WLAN_ROLE_STA,
               cc33xxStaRxCallback);
         }
      }
      else
      {
         //Save underlying network interface (AP mode)
         cc33xxApInterface = interface;

         //Optionally set the MAC address
         if(macCompAddr(&interface->macAddr, &MAC_UNSPECIFIED_ADDR))
         {
            //Use the factory preprogrammed station address
            macAddrParams.roleType = WLAN_ROLE_AP;
            osMemset(macAddrParams.pMacAddress, 0, 6);
            ret = Wlan_Get(WLAN_GET_MACADDRESS, &macAddrParams);

            //Check status code
            if(ret == WLAN_RET_CODE_OK)
            {
               //Generate the 64-bit interface identifier
               macAddrToEui64(&interface->macAddr, &interface->eui64);
            }
         }
         else
         {
            //Override the factory preprogrammed address
            macAddrParams.roleType = WLAN_ROLE_AP;
            osMemcpy(macAddrParams.pMacAddress, interface->macAddr.b, 6);
            ret = Wlan_Set(WLAN_SET_MACADDRESS, &macAddrParams);
         }

         //Check status code
         if(ret == WLAN_RET_CODE_OK)
         {
            //Register RX callback
            ret = Wlan_EtherPacketRecvRegisterCallback(WLAN_ROLE_AP,
               cc33xxApRxCallback);
         }
      }
   }

   //CC33xx is now ready to send
   osSetEvent(&interface->nicTxEvent);

   //Return status code
   if(ret == WLAN_RET_CODE_OK)
   {
      return NO_ERROR;
   }
   else
   {
      return ERROR_FAILURE;
   }
}


/**
 * @brief CC33xx timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void cc33xxTick(NetInterface *interface)
{
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void cc33xxEnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void cc33xxDisableIrq(NetInterface *interface)
{
}


/**
 * @brief CC33xx event handler
 * @param[in] interface Underlying network interface
 **/

void cc33xxEventHandler(NetInterface *interface)
{
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 * @return Error code
 **/

error_t cc33xxSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   static uint8_t temp[2048];
   int_t ret;
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Debug message
   //TRACE_INFO("CC33xx TX packet (%u bytes)\r\n", length);

   //Make sure the link is up before transmitting the frame
   if(!interface->linkState)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Drop current packet
      return NO_ERROR;
   }

   //Copy user data to the transmit buffer
   netBufferRead(temp, buffer, offset, length);

   //STA or AP mode?
   if(interface == cc33xxStaInterface)
   {
      //Send packet
      ret = Wlan_EtherPacketSend(WLAN_ROLE_STA, temp, length, 0);
   }
   else
   {
      //Send packet
      ret = Wlan_EtherPacketSend(WLAN_ROLE_AP, temp, length, 0);
   }

   //The transmitter can accept another packet
   osSetEvent(&interface->nicTxEvent);

   //Return status code
   if(ret >= 0)
   {
      return NO_ERROR;
   }
   else
   {
      return ERROR_FAILURE;
   }
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t cc33xxUpdateMacAddrFilter(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Updating CC33xx multicast filter...\r\n");

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief WLAN event callback function
 * @param[in] event Pointer to the event object
 **/

void __weak_func cc33xxEventCallback(WlanEvent_t *event)
{
   //Debug message
   TRACE_INFO("CC33xx WLAN event callback\r\n");

   if(event->Id == WLAN_EVENT_CONNECT)
   {
      //Debug message
      TRACE_INFO("  WLAN_EVENT_CONNECT\r\n");
      TRACE_INFO("    Channel = %u\r\n", event->Data.Connect.Channel);
   }
   else if(event->Id == WLAN_EVENT_DISCONNECT)
   {
      //Debug message
      TRACE_INFO("  WLAN_EVENT_DISCONNECT\r\n");
      TRACE_INFO("    Reason Code = %u\r\n", event->Data.Disconnect.ReasonCode);
   }
   else if(event->Id == WLAN_EVENT_SCAN_RESULT)
   {
      //Debug message
      TRACE_INFO("  WLAN_EVENT_SCAN_RESULT\r\n");
   }
   else if(event->Id == WLAN_EVENT_ADD_PEER)
   {
      //Debug message
      TRACE_INFO("  WLAN_EVENT_ADD_PEER\r\n");
      TRACE_INFO_ARRAY("    MAC = ", event->Data.AddPeer.Mac, 6);
   }
   else if(event->Id == WLAN_EVENT_REMOVE_PEER)
   {
      //Debug message
      TRACE_INFO("  WLAN_EVENT_REMOVE_PEER\r\n");
      TRACE_INFO_ARRAY("    MAC = ", event->Data.RemovePeer.Mac, 6);
   }
   else
   {
      //Debug message
      TRACE_INFO("  Unknown event ID (%u)\r\n", event->Id);
   }
}


/**
 * @brief Process incoming packets (STA interface)
 * @param[in] role WLAN role
 * @param[in] data Pointer to the received packet
 * @param[in] length Length of the packet, in bytes
 **/

void cc33xxStaRxCallback(WlanRole_e role, uint8_t *data, uint32_t length)
{
   NetRxAncillary ancillary;

   //Debug message
   //TRACE_INFO("CC33xx STA RX received callback (%u bytes)\r\n", length);

   //Valid STA interface?
   if(cc33xxStaInterface != NULL)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_RX_ANCILLARY;

      //Pass the packet to the upper layer
      nicProcessPacket(cc33xxStaInterface, data, length, &ancillary);

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }
}


/**
 * @brief Process incoming packets (AP interface)
 * @param[in] role WLAN role
 * @param[in] data Pointer to the received packet
 * @param[in] length Length of the packet, in bytes
 **/

void cc33xxApRxCallback(WlanRole_e role, uint8_t *data, uint32_t length)
{
   NetRxAncillary ancillary;

   //Debug message
   //TRACE_INFO("CC33xx AP RX received callback (%u bytes)\r\n", length);

   //Valid AP interface?
   if(cc33xxApInterface != NULL)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_RX_ANCILLARY;

      //Pass the packet to the upper layer
      nicProcessPacket(cc33xxApInterface, data, length, &ancillary);

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }
}
