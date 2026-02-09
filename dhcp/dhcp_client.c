/**
 * @file dhcp_client.c
 * @brief DHCP client (Dynamic Host Configuration Protocol)
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
 * The Dynamic Host Configuration Protocol is used to provide configuration
 * parameters to hosts. Refer to the following RFCs for complete details:
 * - RFC 2131: Dynamic Host Configuration Protocol
 * - RFC 2132: DHCP Options and BOOTP Vendor Extensions
 * - RFC 4039: Rapid Commit Option for the DHCP version 4
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.6.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL DHCP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "dhcp/dhcp_client.h"
#include "dhcp/dhcp_client_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED && DHCP_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains DHCP client settings
 **/

void dhcpClientGetDefaultSettings(DhcpClientSettings *settings)
{
   //Network interface to configure
   settings->interface = NULL;
   //Index of the IP address to be configured
   settings->ipAddrIndex = 0;

   //Support for quick configuration using rapid commit
   settings->rapidCommit = FALSE;
   //Use the DNS servers provided by the DHCP server
   settings->manualDnsConfig = FALSE;
   //DHCP configuration timeout
   settings->timeout = 0;

   //DHCP configuration timeout event
   settings->timeoutEvent = NULL;
   //Link state change event
   settings->linkChangeEvent = NULL;
   //FSM state change event
   settings->stateChangeEvent = NULL;

   //Add DHCP options callback
   settings->addOptionsCallback = NULL;
   //Parse DHCP options callback
   settings->parseOptionsCallback = NULL;
}


/**
 * @brief DHCP client initialization
 * @param[in] context Pointer to the DHCP client context
 * @param[in] settings DHCP client specific settings
 * @return Error code
 **/

error_t dhcpClientInit(DhcpClientContext *context,
   const DhcpClientSettings *settings)
{
   NetInterface *interface;

   //Debug message
   TRACE_INFO("Initializing DHCP client...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;

   //The DHCP client must be bound to a valid interface
   if(settings->interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Point to the underlying network interface
   interface = settings->interface;

   //Clear the DHCP client context
   osMemset(context, 0, sizeof(DhcpClientContext));

   //Attach TCP/IP stack context
   context->netContext = settings->interface->netContext;

   //Save user settings
   context->interface = settings->interface;
   context->ipAddrIndex = settings->ipAddrIndex;
   context->rapidCommit = settings->rapidCommit;
   context->manualDnsConfig = settings->manualDnsConfig;
   context->configTimeout = settings->timeout;
   context->timeoutEvent = settings->timeoutEvent;
   context->linkChangeEvent = settings->linkChangeEvent;
   context->stateChangeEvent = settings->stateChangeEvent;
   context->addOptionsCallback = settings->addOptionsCallback;
   context->parseOptionsCallback = settings->parseOptionsCallback;

   //DHCP client is currently suspended
   context->running = FALSE;
   //Initialize state machine
   context->state = DHCP_STATE_INIT;

   //Get exclusive access
   netLock(context->netContext);
   //Attach the DHCP client context to the network interface
   interface->dhcpClientContext = context;
   //Release exclusive access
   netUnlock(context->netContext);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Start DHCP client
 * @param[in] context Pointer to the DHCP client context
 * @return Error code
 **/

error_t dhcpClientStart(DhcpClientContext *context)
{
   error_t error;

   //Make sure the DHCP client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Starting DHCP client...\r\n");

   //Get exclusive access
   netLock(context->netContext);

   //Check the operational state of the DHCP client
   if(!context->running)
   {
      //Reset DHCP configuration
      dhcpClientResetConfig(context);

      //Initialize state machine
      context->state = DHCP_STATE_INIT;

      //Register the callback function to be called whenever a UDP datagram
      //is received on port 68
      error = udpRegisterRxCallback(context->interface, DHCP_CLIENT_PORT,
         dhcpClientProcessMessage, context);

      //Check status code
      if(!error)
      {
         //Start DHCP client
         context->running = TRUE;
      }
   }
   else
   {
      //The DHCP client is already running
      error = ERROR_ALREADY_RUNNING;
   }

   //Release exclusive access
   netUnlock(context->netContext);

   //Return status code
   return error;
}


/**
 * @brief Stop DHCP client
 * @param[in] context Pointer to the DHCP client context
 * @return Error code
 **/

error_t dhcpClientStop(DhcpClientContext *context)
{
   //Make sure the DHCP client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Stopping DHCP client...\r\n");

   //Get exclusive access
   netLock(context->netContext);

   //Check whether the DHCP client is running
   if(context->running)
   {
      //Unregister callback function
      udpUnregisterRxCallback(context->interface, DHCP_CLIENT_PORT);

      //Stop DHCP client
      context->running = FALSE;
      //Reinitialize state machine
      context->state = DHCP_STATE_INIT;
   }

   //Release exclusive access
   netUnlock(context->netContext);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Release DHCP lease
 * @param[in] context Pointer to the DHCP client context
 * @return Error code
 **/

error_t dhcpClientRelease(DhcpClientContext *context)
{
   //Check parameter
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Releasing DHCP lease...\r\n");

   //Get exclusive access
   netLock(context->netContext);

   //Check whether the DHCP client is running
   if(context->running)
   {
      //Check current state
      if(context->state == DHCP_STATE_BOUND ||
         context->state == DHCP_STATE_RENEWING ||
         context->state == DHCP_STATE_REBINDING)
      {
         //Select a new transaction identifier
         context->transactionId = netGenerateRand(context->netContext);

         //The client may choose to relinquish its lease on a network address
         //by sending a DHCPRELEASE message to the server
         dhcpClientSendRelease(context);

         //The host address is no longer valid
         dhcpClientResetConfig(context);
      }

      //Unregister callback function
      udpUnregisterRxCallback(context->interface, DHCP_CLIENT_PORT);

      //Stop DHCP client
      context->running = FALSE;
      //Reinitialize state machine
      context->state = DHCP_STATE_INIT;
   }

   //Release exclusive access
   netUnlock(context->netContext);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve current state
 * @param[in] context Pointer to the DHCP client context
 * @return Current DHCP client state
 **/

DhcpState dhcpClientGetState(DhcpClientContext *context)
{
   DhcpState state;

   //Get exclusive access
   netLock(context->netContext);
   //Get current state
   state = context->state;
   //Release exclusive access
   netUnlock(context->netContext);

   //Return current state
   return state;
}


/**
 * @brief Release DHCP client context
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpClientDeinit(DhcpClientContext *context)
{
   NetInterface *interface;

   //Make sure the DHCP client context is valid
   if(context != NULL)
   {
      //Get exclusive access
      netLock(context->netContext);

      //Point to the underlying network interface
      interface = context->interface;
      //Detach the DHCP client context from the network interface
      interface->dhcpClientContext = NULL;

      //Release exclusive access
      netUnlock(context->netContext);

      //Clear DHCP client context
      osMemset(context, 0, sizeof(DhcpClientContext));
   }
}

#endif
