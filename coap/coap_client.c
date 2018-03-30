/**
 * @file coap_client.c
 * @brief CoAP client
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
#define TRACE_LEVEL COAP_TRACE_LEVEL

//Dependencies
#include <stdlib.h>
#include "core/net.h"
#include "coap/coap_client.h"
#include "coap/coap_client_transport.h"
#include "coap/coap_client_misc.h"
#include "coap/coap_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (COAP_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Initialize COAP client context
 * @param[in] context Pointer to the COAP client context
 * @return Error code
 **/

error_t coapClientInit(CoapClientContext *context)
{
   //Make sure the COAP client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Clear COAP client context
   memset(context, 0, sizeof(CoapClientContext));

   //Create a mutex to prevent simultaneous access to the context
   if(!osCreateMutex(&context->mutex))
   {
      //Report an error
      return ERROR_OUT_OF_RESOURCES;
   }

   //Create a event object to receive notifications
   if(!osCreateEvent(&context->event))
   {
      //Clean up side effects
      coapClientDeinit(context);
      //Report an error
      return ERROR_OUT_OF_RESOURCES;
   }

   //Initialize CoAP client state
   context->state = COAP_CLIENT_STATE_DISCONNECTED;

   //Default transport protocol
   context->transportProtocol = COAP_TRANSPORT_PROTOCOL_UDP;
   //Default timeout
   context->timeout = COAP_CLIENT_DEFAULT_TIMEOUT;
   //Default token length
   context->tokenLen = COAP_CLIENT_DEFAULT_TOKEN_LEN;

   //It is strongly recommended that the initial value of the message ID
   //be randomized (refer to RFC 7252, section 4.4)
   context->mid = (uint16_t) netGetRand();

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Set the transport protocol to be used
 * @param[in] context Pointer to the COAP client context
 * @param[in] transportProtocol Transport protocol to be used (UDP or DTLS)
 * @return Error code
 **/

error_t coapClientSetTransportProtocol(CoapClientContext *context,
   CoapTransportProtocol transportProtocol)
{
   //Make sure the COAP client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the CoAP client context
   osAcquireMutex(&context->mutex);
   //Save the transport protocol to be used
   context->transportProtocol = transportProtocol;
   //Release exclusive access to the CoAP client context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


#if (COAP_CLIENT_DTLS_SUPPORT == ENABLED)

/**
 * @brief Register DTLS initialization callback function
 * @param[in] context Pointer to the COAP client context
 * @param[in] callback DTLS initialization callback function
 * @return Error code
 **/

error_t coapClientRegisterDtlsInitCallback(CoapClientContext *context,
   CoapClientDtlsInitCallback callback)
{
   //Check parameters
   if(context == NULL || callback == NULL)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the CoAP client context
   osAcquireMutex(&context->mutex);
   //Save callback function
   context->dtlsInitCallback = callback;
   //Release exclusive access to the CoAP client context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}

#endif


/**
 * @brief Set default request timeout
 * @param[in] context Pointer to the COAP client context
 * @param[in] timeout Timeout value, in milliseconds
 * @return Error code
 **/

error_t coapClientSetTimeout(CoapClientContext *context, systime_t timeout)
{
   //Make sure the COAP client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the CoAP client context
   osAcquireMutex(&context->mutex);
   //Save timeout value
   context->timeout = timeout;
   //Release exclusive access to the CoAP client context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set the length of the token
 * @param[in] context Pointer to the COAP client context
 * @param[in] length Token length
 * @return Error code
 **/

error_t coapClientSetTokenLength(CoapClientContext *context, size_t length)
{
   //Make sure the COAP client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Make sure the token length is acceptable
   if(length > COAP_MAX_TOKEN_LEN)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the CoAP client context
   osAcquireMutex(&context->mutex);
   //Save token length
   context->tokenLen = length;
   //Release exclusive access to the CoAP client context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Bind the CoAP client to a particular network interface
 * @param[in] context Pointer to the CoAP client context
 * @param[in] interface Network interface to be used
 * @return Error code
 **/

error_t coapClientBindToInterface(CoapClientContext *context,
   NetInterface *interface)
{
   //Make sure the CoAP client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the CoAP client context
   osAcquireMutex(&context->mutex);
   //Explicitly associate the CoAP client with the specified interface
   context->interface = interface;
   //Release exclusive access to the CoAP client context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Establish connection with the CoAP server
 * @param[in] context Pointer to the CoAP client context
 * @param[in] serverIpAddr IP address of the CoAP server to connect to
 * @param[in] serverPort TCP port number that will be used
 * @return Error code
 **/

error_t coapClientConnect(CoapClientContext *context,
   const IpAddr *serverIpAddr, uint16_t serverPort)
{
   error_t error;
   systime_t time;

   //Check parameters
   if(context == NULL || serverIpAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the CoAP client context
   osAcquireMutex(&context->mutex);

   //Initialize status code
   error = NO_ERROR;

   //Establish connection with the CoAP server
   while(!error)
   {
      //Get current time
      time = osGetSystemTime();

      //Check current state
      if(context->state == COAP_CLIENT_STATE_DISCONNECTED)
      {
         //Open network connection
         error = coapClientOpenConnection(context);

         //Check status code
         if(!error)
         {
            //Save current time
            context->startTime = time;
            //Update CoAP client state
            context->state = COAP_CLIENT_STATE_CONNECTING;
         }
      }
      else if(context->state == COAP_CLIENT_STATE_CONNECTING)
      {
         //Establish network connection
         error = coapClientEstablishConnection(context, serverIpAddr,
            serverPort);

         //Check status code
         if(error == NO_ERROR)
         {
            //Update CoAP client state
            context->state = COAP_CLIENT_STATE_CONNECTED;
         }
         else if(error == ERROR_WOULD_BLOCK || error == ERROR_TIMEOUT)
         {
            //Check whether the timeout has elapsed
            if(timeCompare(time, context->startTime + context->timeout) < 0)
            {
               //Wait for an incoming datagram
               coapClientWaitForDatagram(context, COAP_CLIENT_TICK_INTERVAL);
               //Continue processing
               error = NO_ERROR;
            }
            else
            {
               //Report an error
               error = ERROR_TIMEOUT;
            }
         }
         else
         {
            //Just for sanity
         }
      }
      else if(context->state == COAP_CLIENT_STATE_CONNECTED)
      {
         //The CoAP client is connected
         break;
      }
      else
      {
         //Invalid state
         error = ERROR_WRONG_STATE;
      }
   }

   //Failed to establish connection with the CoAP server?
   if(error)
   {
      //Clean up side effects
      coapClientCloseConnection(context);
      //Update CoAP client state
      context->state = COAP_CLIENT_STATE_DISCONNECTED;
   }

   //Release exclusive access to the CoAP client context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
}


/**
 * @brief Process CoAP client events
 * @param[in] context Pointer to the CoAP client context
 * @param[in] timeout Maximum time to wait before returning
 * @return Error code
 **/

error_t coapClientTask(CoapClientContext *context, systime_t timeout)
{
   error_t error;

   //Acquire exclusive access to the CoAP client context
   osAcquireMutex(&context->mutex);
   //Process CoAP client events
   error = coapClientProcessEvents(context, timeout);
   //Release exclusive access to the CoAP client context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
}


/**
 * @brief Disconnect from the CoAP server
 * @param[in] context Pointer to the CoAP client context
 * @return Error code
 **/

error_t coapClientDisconnect(CoapClientContext *context)
{
   error_t error;

   //Make sure the CoAP client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Initialize status code
   error = NO_ERROR;

   //Acquire exclusive access to the CoAP client context
   osAcquireMutex(&context->mutex);

   //Check current state
   if(context->state == COAP_CLIENT_STATE_CONNECTED)
   {
      //Properly dispose the network connection
      error = coapClientShutdownConnection(context);
   }

   //Close connection
   coapClientCloseConnection(context);
   //Update CoAP client state
   context->state = COAP_CLIENT_STATE_DISCONNECTED;

   //Release exclusive access to the CoAP client context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
}


/**
 * @brief Release COAP client context
 * @param[in] context Pointer to the COAP client context
 **/

void coapClientDeinit(CoapClientContext *context)
{
   //Make sure the CoAP client context is valid
   if(context != NULL)
   {
      //Close connection
      coapClientCloseConnection(context);

      //Release previously allocated resources
      osDeleteMutex(&context->mutex);
      osDeleteEvent(&context->event);

      //Clear COAP client context
      memset(context, 0, sizeof(CoapClientContext));
   }
}

#endif
