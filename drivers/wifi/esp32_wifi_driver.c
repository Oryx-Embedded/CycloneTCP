/**
 * @file esp32_wifi_driver.c
 * @brief ESP32 Wi-Fi controller
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
#include "esp_private/wifi.h"
#include "core/net.h"
#include "drivers/wifi/esp32_wifi_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *esp32WifiStaInterface = NULL;
static NetInterface *esp32WifiApInterface = NULL;

//Forward declaration of functions
esp_err_t esp32WifiStaRxCallback(void *buffer, uint16_t length, void *eb);
esp_err_t esp32WifiApRxCallback(void *buffer, uint16_t length, void *eb);

void esp32WifiStaStartEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData);

void esp32WifiStaStopEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData);

void esp32WifiStaConnectedEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData);

void esp32WifiStaDisconnectedEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData);

void esp32WifiApStartEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData);

void esp32WifiApStopEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData);


/**
 * @brief ESP32 Wi-Fi driver (STA mode)
 **/

const NicDriver esp32WifiStaDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   esp32WifiInit,
   esp32WifiTick,
   esp32WifiEnableIrq,
   esp32WifiDisableIrq,
   esp32WifiEventHandler,
   esp32WifiSendPacket,
   esp32WifiUpdateMacAddrFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   TRUE
};


/**
 * @brief ESP32 Wi-Fi driver (AP mode)
 **/

const NicDriver esp32WifiApDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   esp32WifiInit,
   esp32WifiTick,
   esp32WifiEnableIrq,
   esp32WifiDisableIrq,
   esp32WifiEventHandler,
   esp32WifiSendPacket,
   esp32WifiUpdateMacAddrFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   TRUE
};


/**
 * @brief ESP32_WIFI initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t esp32WifiInit(NetInterface *interface)
{
   esp_err_t ret;

   //Initialize status code
   ret = ESP_OK;

   //STA or AP mode?
   if(interface->nicDriver == &esp32WifiStaDriver)
   {
      //Debug message
      TRACE_INFO("Initializing ESP32 Wi-Fi (STA mode)...\r\n");
   }
   else
   {
      //Debug message
      TRACE_INFO("Initializing ESP32 Wi-Fi (AP mode)...\r\n");
   }

   //Initialization sequence is performed once at startup
   if(esp32WifiStaInterface == NULL && esp32WifiApInterface == NULL)
   {
      //Set default configuration
      wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();

      //Initialize Wi-Fi driver
      ret = esp_wifi_init(&config);
   }

   //Check status code
   if(ret == ESP_OK)
   {
      //STA or AP mode?
      if(interface->nicDriver == &esp32WifiStaDriver)
      {
         //Save underlying network interface (STA mode)
         esp32WifiStaInterface = interface;

         //Register event handlers
         esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START,
            esp32WifiStaStartEvent, NULL);

         esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_STOP,
            esp32WifiStaStopEvent, NULL);

         esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED,
            esp32WifiStaConnectedEvent, NULL);

         esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
            esp32WifiStaDisconnectedEvent, NULL);

         //Optionally set the MAC address
         if(macCompAddr(&interface->macAddr, &MAC_UNSPECIFIED_ADDR))
         {
            //Use the factory preprogrammed station address
            ret = esp_wifi_get_mac(ESP_IF_WIFI_STA, interface->macAddr.b);

            //Check status code
            if(ret == ESP_OK)
            {
               //Generate the 64-bit interface identifier
               macAddrToEui64(&interface->macAddr, &interface->eui64);
            }
         }
         else
         {
            //Override the factory preprogrammed address
            ret = esp_wifi_set_mac(ESP_IF_WIFI_STA, interface->macAddr.b);
         }
      }
      else
      {
         //Save underlying network interface (AP mode)
         esp32WifiApInterface = interface;

         //Register event handlers
         esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START,
            esp32WifiApStartEvent, NULL);

         esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STOP,
            esp32WifiApStopEvent, NULL);

         //Optionally set the MAC address
         if(macCompAddr(&interface->macAddr, &MAC_UNSPECIFIED_ADDR))
         {
            //Use the factory preprogrammed station address
            ret = esp_wifi_get_mac(ESP_IF_WIFI_AP, interface->macAddr.b);

            //Check status code
            if(ret == ESP_OK)
            {
               //Generate the 64-bit interface identifier
               macAddrToEui64(&interface->macAddr, &interface->eui64);
            }
         }
         else
         {
            //Override the factory preprogrammed address
            ret = esp_wifi_set_mac(ESP_IF_WIFI_AP, interface->macAddr.b);
         }
      }
   }

   //ESP32 Wi-Fi is now ready to send
   osSetEvent(&interface->nicTxEvent);

   //Return status code
   if(ret == ESP_OK)
   {
      return NO_ERROR;
   }
   else
   {
      return ERROR_FAILURE;
   }
}


/**
 * @brief ESP32 Wi-Fi timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void esp32WifiTick(NetInterface *interface)
{
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void esp32WifiEnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void esp32WifiDisableIrq(NetInterface *interface)
{
}


/**
 * @brief ESP32 Wi-Fi event handler
 * @param[in] interface Underlying network interface
 **/

void esp32WifiEventHandler(NetInterface *interface)
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

error_t esp32WifiSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   static uint8_t temp[2048];
   size_t length;
   int_t ret;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Copy user data
   netBufferRead(temp, buffer, offset, length);

   //STA or AP mode?
   if(interface == esp32WifiStaInterface)
   {
      //Send packet
      ret = esp_wifi_internal_tx(ESP_IF_WIFI_STA, temp, length);
   }
   else
   {
      //Send packet
      ret = esp_wifi_internal_tx(ESP_IF_WIFI_AP, temp, length);
   }

   //The transmitter can accept another packet
   osSetEvent(&interface->nicTxEvent);

   //Return status code
   if(ret == ESP_OK)
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

error_t esp32WifiUpdateMacAddrFilter(NetInterface *interface)
{
   //Not implemented
   return NO_ERROR;
}


/**
 * @brief Process incoming packets (STA interface)
 * @param[in] buffer Incoming packet
 * @param[in] length Length of the packet, in bytes
 * @param[in] eb Pointer to the buffer allocated by the Wi-Fi driver
 * @return Error code
 **/

esp_err_t esp32WifiStaRxCallback(void *buffer, uint16_t length, void *eb)
{
   NetRxAncillary ancillary;

   //Valid STA interface?
   if(esp32WifiStaInterface != NULL)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_RX_ANCILLARY;

      //Pass the packet to the upper layer
      nicProcessPacket(esp32WifiStaInterface, buffer, length, &ancillary);

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }

   //Valid buffer?
   if(eb != NULL)
   {
      //Release buffer
      esp_wifi_internal_free_rx_buffer(eb);
   }

   //Successful processing
   return ESP_OK;
}


/**
 * @brief Process incoming packets (AP interface)
 * @param[in] buffer Incoming packet
 * @param[in] length Length of the packet, in bytes
 * @param[in] eb Pointer to the buffer allocated by the Wi-Fi driver
 * @return Error code
 **/

esp_err_t esp32WifiApRxCallback(void *buffer, uint16_t length, void *eb)
{
   NetRxAncillary ancillary;

   //Valid AP interface?
   if(esp32WifiApInterface != NULL)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_RX_ANCILLARY;

      //Pass the packet to the upper layer
      nicProcessPacket(esp32WifiApInterface, buffer, length, &ancillary);

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }

   //Valid buffer?
   if(eb != NULL)
   {
      //Release buffer
      esp_wifi_internal_free_rx_buffer(eb);
   }

   //Successful processing
   return ESP_OK;
}


/**
 * @brief Station start (event handler)
 * @param[in] arg User-specific parameter
 * @param[in] eventBase Event base
 * @param[in] eventId Event identifier
 * @param[in] eventData Event-specific data
 **/

void esp32WifiStaStartEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData)
{
   //Debug message
   TRACE_INFO("ESP32: STA start event\r\n");
}


/**
 * @brief Station stop (event handler)
 * @param[in] arg User-specific parameter
 * @param[in] eventBase Event base
 * @param[in] eventId Event identifier
 * @param[in] eventData Event-specific data
 **/

void esp32WifiStaStopEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData)
{
   //Debug message
   TRACE_INFO("ESP32: STA stop event\r\n");
}


/**
 * @brief Station connected to AP (event handler)
 * @param[in] arg User-specific parameter
 * @param[in] eventBase Event base
 * @param[in] eventId Event identifier
 * @param[in] eventData Event-specific data
 **/

void esp32WifiStaConnectedEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData)
{
   esp_err_t ret;

   //Debug message
   TRACE_INFO("ESP32: STA connected event\r\n");

   //Register RX callback
   ret = esp_wifi_internal_reg_rxcb(ESP_IF_WIFI_STA, esp32WifiStaRxCallback);

   //Check status code
   if(ret == ESP_OK)
   {
      //Valid STA interface?
      if(esp32WifiStaInterface != NULL)
      {
         //The link is up
         esp32WifiStaInterface->linkState = TRUE;

         //Get exclusive access
         osAcquireMutex(&netMutex);
         //Process link state change event
         nicNotifyLinkChange(esp32WifiStaInterface);
         //Release exclusive access
         osReleaseMutex(&netMutex);
      }
   }
}


/**
 * @brief Station disconnected from AP (event handler)
 * @param[in] arg User-specific parameter
 * @param[in] eventBase Event base
 * @param[in] eventId Event identifier
 * @param[in] eventData Event-specific data
 **/

void esp32WifiStaDisconnectedEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData)
{
   esp_err_t ret;

   //Debug message
   TRACE_INFO("ESP32: STA disconnected event\r\n");

   //Unregister RX callback
   ret = esp_wifi_internal_reg_rxcb(ESP_IF_WIFI_STA, NULL);

   //Check status code
   if(ret == ESP_OK)
   {
      //Valid STA interface?
      if(esp32WifiStaInterface != NULL)
      {
         //The link is down
         esp32WifiStaInterface->linkState = FALSE;

         //Get exclusive access
         osAcquireMutex(&netMutex);
         //Process link state change event
         nicNotifyLinkChange(esp32WifiStaInterface);
         //Release exclusive access
         osReleaseMutex(&netMutex);
      }
   }
}


/**
 * @brief Soft-AP start (event handler)
 * @param[in] arg User-specific parameter
 * @param[in] eventBase Event base
 * @param[in] eventId Event identifier
 * @param[in] eventData Event-specific data
 **/

void esp32WifiApStartEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData)
{
   esp_err_t ret;

   //Debug message
   TRACE_INFO("ESP32: AP start event\r\n");

   //Register RX callback
   ret = esp_wifi_internal_reg_rxcb(ESP_IF_WIFI_AP, esp32WifiApRxCallback);

   //Check status code
   if(ret == ESP_OK)
   {
      //Valid AP interface?
      if(esp32WifiApInterface != NULL)
      {
         //The link is up
         esp32WifiApInterface->linkState = TRUE;

         //Get exclusive access
         osAcquireMutex(&netMutex);
         //Process link state change event
         nicNotifyLinkChange(esp32WifiApInterface);
         //Release exclusive access
         osReleaseMutex(&netMutex);
      }
   }
}


/**
 * @brief Soft-AP stop (event handler)
 * @param[in] arg User-specific parameter
 * @param[in] eventBase Event base
 * @param[in] eventId Event identifier
 * @param[in] eventData Event-specific data
 **/

void esp32WifiApStopEvent(void *arg, esp_event_base_t eventBase,
   int32_t eventId, void *eventData)
{
   esp_err_t ret;

   //Debug message
   TRACE_INFO("ESP32: AP stop event\r\n");

   //Unregister RX callback
   ret = esp_wifi_internal_reg_rxcb(ESP_IF_WIFI_AP, NULL);

   //Check status code
   if(ret == ESP_OK)
   {
      //Valid AP interface?
      if(esp32WifiApInterface != NULL)
      {
         //The link is down
         esp32WifiApInterface->linkState = FALSE;

         //Get exclusive access
         osAcquireMutex(&netMutex);
         //Process link state change event
         nicNotifyLinkChange(esp32WifiApInterface);
         //Release exclusive access
         osReleaseMutex(&netMutex);
      }
   }
}
