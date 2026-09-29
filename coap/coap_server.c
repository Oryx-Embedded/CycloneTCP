/**
 * @file coap_server.c
 * @brief CoAP server
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.6.6
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL COAP_TRACE_LEVEL

//Dependencies
#include <stdlib.h>
#include "coap/coap_server.h"
#include "coap/coap_server_observe.h"
#include "coap/coap_server_transport.h"
#include "coap/coap_server_misc.h"
#include "coap/coap_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (COAP_SERVER_SUPPORT == ENABLED)


/**
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains CoAP server settings
 **/

void coapServerGetDefaultSettings(CoapServerSettings *settings)
{
   //Default task parameters
   settings->task = OS_TASK_DEFAULT_PARAMS;
   settings->task.stackSize = COAP_SERVER_STACK_SIZE;
   settings->task.priority = COAP_SERVER_PRIORITY;

   //TCP/IP stack context
   settings->netContext = NULL;
   //The CoAP server is not bound to any interface
   settings->interface = NULL;

   //CoAP port number
   settings->port = COAP_PORT;

#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   //DTLS sessions
   settings->numSessions = 0;
   settings->sessions = NULL;
#endif

#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   //Observable resources
   settings->numResources = 0;
   settings->resources = NULL;

   //Observers
   settings->numObservers = 0;
   settings->observers = NULL;
#endif

   //UDP initialization callback
   settings->udpInitCallback = NULL;

#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   //DTLS initialization callback function
   settings->dtlsInitCallback = NULL;
#endif

   //CoAP request callback function
   settings->requestCallback = NULL;

#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   //Observe callback
   settings->observeCallback = NULL;
#endif
}


/**
 * @brief CoAP server initialization
 * @param[in] context Pointer to the CoAP server context
 * @param[in] settings CoAP server specific settings
 * @return Error code
 **/

error_t coapServerInit(CoapServerContext *context,
   const CoapServerSettings *settings)
{
   error_t error;
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED || COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   uint_t i;
#endif

   //Debug message
   TRACE_INFO("Initializing CoAP server...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;

#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   if(settings->sessions == NULL && settings->numSessions != 0)
      return ERROR_INVALID_PARAMETER;
#endif

#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   if(settings->resources == NULL && settings->numResources != 0)
      return ERROR_INVALID_PARAMETER;

   if(settings->observers == NULL && settings->numObservers != 0)
      return ERROR_INVALID_PARAMETER;
#endif

   //Initialize status code
   error = NO_ERROR;

   //Clear the CoAP server context
   osMemset(context, 0, sizeof(CoapServerContext));

   //Initialize task parameters
   context->taskParams = settings->task;
   context->taskId = OS_INVALID_TASK_ID;

   //Attach TCP/IP stack context
   if(settings->netContext != NULL)
   {
      context->netContext = settings->netContext;
   }
   else if(settings->interface != NULL)
   {
      context->netContext = settings->interface->netContext;
   }
   else
   {
      context->netContext = netGetDefaultContext();
   }

   //Save user settings
   context->interface = settings->interface;
   context->port = settings->port;
   context->udpInitCallback = settings->udpInitCallback;
   context->requestCallback = settings->requestCallback;

#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   //DTLS sessions
   context->numSessions = settings->numSessions;
   context->sessions = settings->sessions;

   //Initialize entries
   for(i = 0; i < context->numSessions; i++)
   {
      osMemset(&context->sessions[i], 0, sizeof(CoapDtlsSession));
   }

   //DTLS initialization callback
   context->dtlsInitCallback = settings->dtlsInitCallback;
#endif

#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   //Observable resources
   context->numResources = settings->numResources;
   context->resources = settings->resources;

   //Initialize entries
   for(i = 0; i < context->numResources; i++)
   {
      osMemset(&context->resources[i], 0, sizeof(CoapResource));
   }

   //Observers
   context->numObservers = settings->numObservers;
   context->observers = settings->observers;

   //Initialize entries
   for(i = 0; i < context->numObservers; i++)
   {
      osMemset(&context->observers[i], 0, sizeof(CoapObserver));
   }

   //Observe callback
   context->observeCallback = settings->observeCallback;

   //It is strongly recommended that the initial value of the message ID be
   //randomized (refer to RFC 7252, section 4.4)
   context->mid = (uint16_t) netGetRand(context->netContext);
#endif

   //Start of exception handling block
   do
   {
      //Create a mutex to prevent simultaneous access to the context
      if(!osCreateMutex(&context->mutex))
      {
         //Failed to create mutex
         error = ERROR_OUT_OF_RESOURCES;
         break;
      }

      //Create an event object to poll the state of the UDP socket
      if(!osCreateEvent(&context->event))
      {
         //Failed to create event
         error = ERROR_OUT_OF_RESOURCES;
         break;
      }

#if (COAP_SERVER_DTLS_SUPPORT == ENABLED && TLS_TICKET_SUPPORT == ENABLED)
      //Initialize ticket encryption context
      error = tlsInitTicketContext(&context->dtlsTicketContext);
      //Any error to report?
      if(error)
         break;
#endif

      //End of exception handling block
   } while(0);

   //Check status code
   if(error)
   {
      //Clean up side effects
      coapServerDeinit(context);
   }

   //Return status code
   return error;
}


/**
 * @brief Set cookie secret
 *
 * This function specifies the cookie secret used while generating and
 * verifying a cookie during the DTLS handshake
 *
 * @param[in] context Pointer to the CoAP server context
 * @param[in] cookieSecret Pointer to the secret key
 * @param[in] cookieSecretLen Length of the secret key, in bytes
 * @return Error code
 **/

error_t coapServerSetCookieSecret(CoapServerContext *context,
   const uint8_t *cookieSecret, size_t cookieSecretLen)
{
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   //Ensure the parameters are valid
   if(context == NULL || cookieSecret == NULL)
      return ERROR_INVALID_PARAMETER;
   if(cookieSecretLen > COAP_SERVER_MAX_COOKIE_SECRET_SIZE)
      return ERROR_INVALID_PARAMETER;

   //Save the secret key
   osMemcpy(context->cookieSecret, cookieSecret, cookieSecretLen);
   //Save the length of the secret key
   context->cookieSecretLen = cookieSecretLen;

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Start CoAP server
 * @param[in] context Pointer to the CoAP server context
 * @return Error code
 **/

error_t coapServerStart(CoapServerContext *context)
{
   error_t error;

   //Make sure the CoAP server context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Starting CoAP server...\r\n");

   //Make sure the CoAP server is not already running
   if(context->running)
      return ERROR_ALREADY_RUNNING;

   //Start of exception handling block
   do
   {
      //Open a UDP socket
      context->socket = socketOpenEx(context->netContext, SOCKET_TYPE_DGRAM,
         SOCKET_IP_PROTO_UDP);
      //Failed to open socket?
      if(context->socket == NULL)
      {
         //Report an error
         error = ERROR_OPEN_FAILED;
         break;
      }

      //Force the socket to operate in non-blocking mode
      error = socketSetTimeout(context->socket, 0);
      //Any error to report?
      if(error)
         break;

      //Associate the socket with the relevant interface
      error = socketBindToInterface(context->socket, context->interface);
      //Unable to bind the socket to the desired interface?
      if(error)
         break;

      //The CoAP server listens for datagrams on port 5683
      error = socketBind(context->socket, &IP_ADDR_ANY, context->port);
      //Unable to bind the socket to the desired port?
      if(error)
         break;

      //Any registered callback?
      if(context->udpInitCallback != NULL)
      {
         //Invoke user callback function
         error = context->udpInitCallback(context, context->socket);
         //Any error to report?
         if(error)
            break;
      }

      //Start the CoAP server
      context->stop = FALSE;
      context->running = TRUE;

      //Create a task
      context->taskId = osCreateTask("CoAP Server", (OsTaskCode) coapServerTask,
         context, &context->taskParams);

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

      //Close the UDP socket
      socketClose(context->socket);
      context->socket = NULL;
   }

   //Return status code
   return error;
}


/**
 * @brief Stop CoAP server
 * @param[in] context Pointer to the CoAP server context
 * @return Error code
 **/

error_t coapServerStop(CoapServerContext *context)
{
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   uint_t i;
#endif

   //Make sure the CoAP server context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Stopping CoAP server...\r\n");

   //Check whether the CoAP server is running
   if(context->running)
   {
#if (NET_RTOS_SUPPORT == ENABLED)
      //Stop the CoAP server
      context->stop = TRUE;
      //Send a signal to the task to abort any blocking operation
      osSetEvent(&context->event);

      //Wait for the task to terminate
      while(context->running)
      {
         osDelayTask(1);
      }
#endif

#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
      //Loop through DTLS sessions
      for(i = 0; i < context->numSessions; i++)
      {
         //Release DTLS session
         coapServerDeleteSession(&context->sessions[i]);
      }
#endif

      //Close the UDP socket
      socketClose(context->socket);
      context->socket = NULL;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Create a new observable resource
 * @param[in] context Pointer to the CoAP server context
 * @param[in] uri NULL-terminated string containing the path to the resource
 * @return Error code
 **/

error_t coapServerCreateResource(CoapServerContext *context, const char_t *uri)
{
#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   error_t error;
   uint_t i;
   CoapResource *resource;

   //Check parameters
   if(context == NULL || uri == NULL)
      return ERROR_INVALID_PARAMETER;

   //Check the length of the URI
   if(osStrlen(uri) > COAP_SERVER_MAX_URI_LEN)
      return ERROR_INVALID_LENGTH;

   //Initialize status code
   error = NO_ERROR;

   //Acquire exclusive access to the CoAP server context
   osAcquireMutex(&context->mutex);

   //Search the list of resources for a matching URI
   resource = coapServerFindResource(context, uri);

   //If the resource does not exist, then create a new entry
   if(resource == NULL)
   {
      //Loop through the list of resources
      for(i = 0; i < context->numResources; i++)
      {
         //Check whether the entry is available
         if(context->resources[i].uri[0] == '\0')
         {
            resource = &context->resources[i];
            break;
         }
      }
   }

   //Valid entry
   if(resource != NULL)
   {
      //Save the full request URI
      osStrcpy(resource->uri, uri);
      //Initialize resource state
      resource->dataLen = 0;

      //The sequence number may start at any value (refer to RFC 7641,
      //section 4.4)
      resource->seqNum = netGetRand(context->netContext) & 0x00FFFFFF;
   }
   else
   {
      //A new entry cannot be created
      error = ERROR_OUT_OF_RESOURCES;
   }

   //Release exclusive access to the CoAP server context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Update the state of an observable resource
 * @param[in] context Pointer to the CoAP server context
 * @param[in] uri NULL-terminated string containing the path to the resource
 * @param[in] data New resource state
 * @param[in] length Length of the resource state
 * @return Error code
 **/

error_t coapServerUpdateResource(CoapServerContext *context, const char_t *uri,
   const void *data, size_t length)
{
#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   error_t error;
   uint_t i;
   CoapResource *resource;
   CoapObserver *observer;

   //Check parameters
   if(context == NULL || uri == NULL)
      return ERROR_INVALID_PARAMETER;

   //Check the length of the resource state
   if(length > COAP_SERVER_MAX_OBS_RESOURCE_SIZE)
      return ERROR_INVALID_LENGTH;

   //Initialize status code
   error = NO_ERROR;

   //Acquire exclusive access to the CoAP server context
   osAcquireMutex(&context->mutex);

   //Search the list of resources for a matching URI
   resource = coapServerFindResource(context, uri);

   //Matching resource found?
   if(resource != NULL)
   {
      //Save the new resource state
      osMemcpy(resource->data, data, length);
      //Update the length of the resource state
      resource->dataLen = length;

      //An implementation can store a 24-bit unsigned integer variable per
      //resource and increment this variable each time the resource undergoes
      //a change of state (refer to RFC 7641, section 4.4)
      resource->seqNum = (resource->seqNum + 1) & 0x00FFFFFF;

      //Loop through the list of registered observers
      for(i = 0; i < context->numObservers; i++)
      {
         //Point to the current entry
         observer = &context->observers[i];

         //Matching entry?
         if(observer->resource == resource)
         {
            //The resource state has changed
            observer->changed = TRUE;
         }
      }

      //Whenever the state of a resource changes, the server notifies each
      //client in the list of observers of the resource
      osSetEvent(&context->event);
   }
   else
   {
      //The target resource does not exist
      error = ERROR_NOT_FOUND;
   }

   //Release exclusive access to the CoAP server context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Delete an observable resource
 * @param[in] context Pointer to the CoAP server context
 * @param[in] uri NULL-terminated string containing the path to the resource
 * @return Error code
 **/

error_t coapServerDeleteResource(CoapServerContext *context, const char_t *uri)
{
#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   error_t error;
   uint_t i;
   CoapResource *resource;
   CoapObserver *observer;

   //Check parameters
   if(context == NULL || uri == NULL)
      return ERROR_INVALID_PARAMETER;

   //Initialize status code
   error = NO_ERROR;

   //Acquire exclusive access to the CoAP server context
   osAcquireMutex(&context->mutex);

   //Search the list of resources for a matching URI
   resource = coapServerFindResource(context, uri);

   //Matching resource found?
   if(resource != NULL)
   {
      //Loop through the list of registered observers
      for(i = 0; i < context->numObservers; i++)
      {
         //Point to the current entry
         observer = &context->observers[i];

         //Matching entry?
         if(observer->resource == resource)
         {
            //Remove the client's entry from the list of observers of the
            //resource
            observer->resource = NULL;

            //The resource state has changed
            observer->changed = TRUE;
         }
      }

      //The resource is deleted
      osMemset(resource, 0, sizeof(CoapResource));
   }
   else
   {
      //The target resource does not exist
      error = ERROR_NOT_FOUND;
   }

   //Release exclusive access to the CoAP server context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief CoAP server task
 * @param[in] context Pointer to the CoAP server context
 **/

void coapServerTask(CoapServerContext *context)
{
   error_t error;
   SocketMsg msg;
   SocketEventDesc eventDesc;

#if (NET_RTOS_SUPPORT == ENABLED)
   //Task prologue
   osEnterTask();

   //Process events
   while(1)
   {
#endif
      //Specify the events the application is interested in
      eventDesc.socket = context->socket;
      eventDesc.eventMask = SOCKET_EVENT_RX_READY;
      eventDesc.eventFlags = 0;

      //Wait for an event
      socketPoll(&eventDesc, 1, &context->event, COAP_SERVER_TICK_INTERVAL);

      //Stop request?
      if(context->stop)
      {
         //Stop CoAP server operation
         context->running = FALSE;
         //Task epilogue
         osExitTask();
         //Kill ourselves
         osDeleteTask(OS_SELF_TASK_ID);
      }

      //Any datagram received?
      if(eventDesc.eventFlags != 0)
      {
         //Point to the receive buffer
         msg = SOCKET_DEFAULT_MSG;
         msg.data = context->buffer;
         msg.size = COAP_SERVER_BUFFER_SIZE;

         //Receive incoming datagram
         error = socketReceiveMsg(context->socket, &msg, 0);

         //Check status code
         if(!error)
         {
            //An endpoint must be prepared to receive multicast messages but may
            //ignore them if multicast service discovery is not desired (refer
            //to RFC 7252, section 8)
            if(!ipIsMulticastAddr(&msg.destIpAddr))
            {
               //Retrieve the length of the datagram
               context->bufferLen = msg.length;

               //Get the source and destination IP addresses
               context->localInterface = msg.interface;
               context->localIpAddr = msg.destIpAddr;
               context->remoteIpAddr = msg.srcIpAddr;
               context->remotePort = msg.srcPort;

               //Acquire exclusive access to the CoAP server context
               osAcquireMutex(&context->mutex);

#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
               //DTLS-secured communication?
               if(context->dtlsInitCallback != NULL)
               {
                  //Demultiplexing of incoming datagrams into separate DTLS sessions
                  error = coapServerDemultiplexSession(context);
               }
               else
#endif
               {
                  //Process the received CoAP message
                  error = coapServerProcessMessage(context, context->buffer,
                     context->bufferLen);
               }

               //Release exclusive access to the CoAP server context
               osReleaseMutex(&context->mutex);
            }
         }
      }

      //Acquire exclusive access to the CoAP server context
      osAcquireMutex(&context->mutex);
      //Handle periodic operations
      coapServerTick(context);
      //Release exclusive access to the CoAP server context
      osReleaseMutex(&context->mutex);
#if (NET_RTOS_SUPPORT == ENABLED)
   }
#endif
}


/**
 * @brief Release CoAP server context
 * @param[in] context Pointer to the CoAP server context
 **/

void coapServerDeinit(CoapServerContext *context)
{
   //Make sure the CoAP server context is valid
   if(context != NULL)
   {
      //Release previously allocated resources
      osDeleteMutex(&context->mutex);
      osDeleteEvent(&context->event);

#if (COAP_SERVER_DTLS_SUPPORT == ENABLED && TLS_TICKET_SUPPORT == ENABLED)
      //Release ticket encryption context
      tlsFreeTicketContext(&context->dtlsTicketContext);
#endif

      //Clear CoAP server context
      osMemset(context, 0, sizeof(CoapServerContext));
   }
}

#endif
