/**
 * @file dhcpv6_relay.c
 * @brief DHCPv6 relay agent (Dynamic Host Configuration Protocol for IPv6)
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2026 Oryx Embedded SARL. All rights reserved.
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
 * @section Description
 *
 * DHCPv6 Relay-Agents are deployed to forward DHCPv6 messages between clients
 * and servers when they are not on the same IPv6 link and are often implemented
 * alongside a routing function in a common node. Refer to RFC 8415
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.6.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL DHCPV6_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "dhcpv6/dhcpv6_relay.h"
#include "dhcpv6/dhcpv6_relay_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && DHCPV6_RELAY_SUPPORT == ENABLED)


/**
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains DHCPv6 relay agent settings
 **/

void dhcpv6RelayGetDefaultSettings(Dhcpv6RelaySettings *settings)
{
   uint_t i;

   //Default task parameters
   settings->task = OS_TASK_DEFAULT_PARAMS;
   settings->task.stackSize = DHCPV6_RELAY_STACK_SIZE;
   settings->task.priority = DHCPV6_RELAY_PRIORITY;

   //Network-facing interface
   settings->serverInterface = NULL;

   //Number of client-facing interfaces
   settings->numClientInterfaces = 0;

   //Client-facing interfaces
   for(i = 0; i < DHCPV6_RELAY_MAX_CLIENT_INTERFACES; i++)
   {
      settings->clientInterfaces[i] = NULL;
   }

   //If the relay agent has not been explicitly configured, it must use the
   //All_DHCP_Servers multicast address as the default (refer to RFC 8415,
   //section 19)
   settings->serverIpAddr = DHCPV6_ALL_SERVERS_ADDR;
}


/**
 * @brief DHCPv6 relay agent initialization
 * @param[in] context Pointer to the DHCPv6 client context
 * @param[in] settings DHCPv6 relay agent specific settings
 * @return Error code
 **/

error_t dhcpv6RelayInit(Dhcpv6RelayContext *context,
   const Dhcpv6RelaySettings *settings)
{
   error_t error;
   uint_t i;

   //Debug message
   TRACE_INFO("Initializing DHCPv6 relay agent...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;
   
   //Invalid network-facing interface?
   if(settings->serverInterface == NULL)
      return ERROR_INVALID_INTERFACE;

   //Invalid number of client-facing interfaces
   if(settings->numClientInterfaces < 1 ||
      settings->numClientInterfaces >= DHCPV6_RELAY_MAX_CLIENT_INTERFACES)
   {
      return ERROR_INVALID_PARAMETER;
   }

   //Loop through the client-facing interfaces
   for(i = 0; i < settings->numClientInterfaces; i++)
   {
      //Invalid client-facing interface?
      if(settings->clientInterfaces[i] == NULL)
         return ERROR_INVALID_INTERFACE;
   }

   //Check the address to be used when forwarding messages to the server
   if(ipv6CompAddr(&settings->serverIpAddr, &IPV6_UNSPECIFIED_ADDR))
      return ERROR_INVALID_ADDRESS;

   //Initialize status code
   error = NO_ERROR;

   //Clear the DHCPv6 relay agent context
   osMemset(context, 0, sizeof(Dhcpv6RelayContext));

   //Initialize task parameters
   context->taskParams = settings->task;
   context->taskId = OS_INVALID_TASK_ID;

   //Attach TCP/IP stack context
   context->netContext = settings->serverInterface->netContext;

   //Save user settings
   context->serverInterface = settings->serverInterface;
   context->numClientInterfaces = settings->numClientInterfaces;
   context->serverIpAddr = settings->serverIpAddr;

   //Save client-facing interfaces
   for(i = 0; i < context->numClientInterfaces; i++)
   {
      context->clientInterfaces[i] = settings->clientInterfaces[i];
   }

   //Create an event object to poll the state of sockets
   if(!osCreateEvent(&context->event))
   {
      //Failed to create event
      error = ERROR_OUT_OF_RESOURCES;
   }

   //Any error to report?
   if(error)
   {
      //Clean up side effects
      dhcpv6RelayDeinit(context);
   }

   //Return status code
   return error;
}


/**
 * @brief Start DHCPv6 relay agent
 * @param[in] context Pointer to the DHCPv6 relay agent context
 * @return Error code
 **/

error_t dhcpv6RelayStart(Dhcpv6RelayContext *context)
{
   error_t error;
   uint_t i;

   //Make sure the DHCPv6 relay agent context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Starting DHCPv6 relay agent...\r\n");

   //Make sure the DHCPv6 relay agent is not already running
   if(context->running)
      return ERROR_ALREADY_RUNNING;

   //Start of exception handling block
   do
   {
      //Open the socket that carries traffic towards the DHCPv6 server
      error = dhcpv6RelayOpenServerSocket(context);
      //Any error to report?
      if(error)
         break;

      //Open the sockets that carry traffic towards the DHCPv6 clients
      for(i = 0; i < context->numClientInterfaces && !error; i++)
      {
         error = dhcpv6RelayOpenClientSocket(context, i);
      }

      //Propagate exception if necessary
      if(error)
         break;

      //Start the DHCPv6 relay agent
      context->stop = FALSE;
      context->running = TRUE;

      //Create a task
      context->taskId = osCreateTask("DHCPv6 Relay",
         (OsTaskCode) dhcpv6RelayTask, context, &context->taskParams);

      //Failed to create task?
      if(context->taskId == OS_INVALID_TASK_ID)
      {
         //Report an error
         error = ERROR_OUT_OF_RESOURCES;
         break;
      }

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Clean up side effects
      context->running = FALSE;

      //Close the socket associated with the network-facing interface
      socketClose(context->serverSocket);
      context->serverSocket = NULL;

      //Close the socket associated with client-facing interfaces
      for(i = 0; i < context->numClientInterfaces; i++)
      {
         socketClose(context->clientSockets[i]);
         context->clientSockets[i] = NULL;
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Stop DHCPv6 relay agent
 * @param[in] context Pointer to the DHCPv6 relay agent context
 * @return Error code
 **/

error_t dhcpv6RelayStop(Dhcpv6RelayContext *context)
{
   uint_t i;

   //Make sure the DHCPv6 relay agent context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Stopping DHCPv6 relay agent...\r\n");

   //Check whether the DHCPv6 relay agent is running
   if(context->running)
   {
#if (NET_RTOS_SUPPORT == ENABLED)
      //Stop the DHCPv6 relay agent
      context->stop = TRUE;
      //Send a signal to the task to abort any blocking operation
      osSetEvent(&context->event);

      //Wait for the task to terminate
      while(context->running)
      {
         osDelayTask(1);
      }
#endif

      //Close the socket that carries traffic towards the DHCPv6 server
      socketClose(context->serverSocket);
      context->serverSocket = NULL;

      //Close the sockets that carry traffic towards the DHCPv6 clients
      for(i = 0; i < context->numClientInterfaces; i++)
      {
         socketClose(context->clientSockets[i]);
         context->clientSockets[i] = NULL;
      }
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief DHCPv6 relay agent task
 * @param[in] context Pointer to the DHCPv6 relay agent context
 **/

void dhcpv6RelayTask(Dhcpv6RelayContext *context)
{
   error_t error;
   uint_t i;

   //Task prologue
   osEnterTask();

   //Specify the events the application is interested in for each client-facing
   //sockets
   for(i = 0; i < context->numClientInterfaces; i++)
   {
      context->eventDesc[i].socket = context->clientSockets[i];
      context->eventDesc[i].eventMask = SOCKET_EVENT_RX_READY;
   }

   //Specify the events the application is interested in for the network-facing
   //socket
   context->eventDesc[i].socket = context->serverSocket;
   context->eventDesc[i].eventMask = SOCKET_EVENT_RX_READY;

   //Main loop
   while(1)
   {
      //Wait for incoming packets on network-facing or client-facing interfaces
      error = socketPoll(context->eventDesc, context->numClientInterfaces + 1,
         &context->event, INFINITE_DELAY);

      //Check status code
      if(error == NO_ERROR || error == ERROR_TIMEOUT ||
         error == ERROR_WAIT_CANCELED)
      {
         //Stop request?
         if(context->stop)
         {
            //Stop DHCPv6 relay agent operation
            context->running = FALSE;
            //Task epilogue
            osExitTask();
            //Kill ourselves
            osDeleteTask(OS_SELF_TASK_ID);
         }

         //Loop through client-facing interfaces
         for(i = 0; i < context->numClientInterfaces; i++)
         {
            //Check the state of each client-facing socket
            if(context->eventDesc[i].eventFlags & SOCKET_EVENT_RX_READY)
            {
               //Relay messages from the clients
               dhcpv6ForwardClientMessage(context, i);
            }
         }

         //Check the state of the network-facing socket
         if(context->eventDesc[i].eventFlags & SOCKET_EVENT_RX_READY)
         {
            //Forward Relay-Reply messages from the network
            dhcpv6ForwardRelayReplyMessage(context);
         }
      }
   }
}


/**
 * @brief Release DHCP relay agent context
 * @param[in] context Pointer to the DHCP relay agent context
 **/

void dhcpv6RelayDeinit(Dhcpv6RelayContext *context)
{
   //Make sure the DHCP relay agent context is valid
   if(context != NULL)
   {
      //Free previously allocated resources
      osDeleteEvent(&context->event);

      //Clear DHCP relay agent context
      osMemset(context, 0, sizeof(Dhcpv6RelayContext));
   }
}

#endif
