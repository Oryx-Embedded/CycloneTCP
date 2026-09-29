/**
 * @file coap_server_observe.c
 * @brief CoAP observe
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
#include "core/net.h"
#include "coap/coap_server.h"
#include "coap/coap_server_observe.h"
#include "coap/coap_server_misc.h"
#include "coap/coap_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (COAP_SERVER_SUPPORT == ENABLED && COAP_SERVER_OBSERVE_SUPPORT == ENABLED)


/**
 * @brief Process observe-related events
 * @param[in] context Pointer to the CoAP server context
 **/

void coapServerProcessObserveEvents(CoapServerContext *context)
{
   uint_t i;
   systime_t time;
   CoapObserver *observer;

   //Get current time
   time = osGetSystemTime();

   //Loop through the list of registered observers
   for(i = 0; i < context->numObservers; i++)
   {
      //Point to the current entry
      observer = &context->observers[i];

      //Check the state of the observer
      if(observer->state == COAP_OBSERVER_STATE_REGISTERED)
      {
         //Whenever the state of a resource changes, the server notifies each
         //client in the list of observers of the resource
         if(observer->changed)
         {
            //Confirmable notification?
            if(observer->type == COAP_TYPE_CON)
            {
               //Start to transmit a confirmable notification
               coapServerSendNotificationResponse(context, observer);

               //Check whether the target resource has been deleted
               if(observer->state == COAP_OBSERVER_STATE_REGISTERED &&
                  observer->resource == NULL)
               {
                  //Removes the client's entry from the list of observers
                  coapServerDeleteObserver(observer);
               }
            }
            else
            {
               //The server should not send more than one non-confirmable
               //notification per round-trip time (RTT) to a client on average.
               //If the server cannot maintain an RTT estimate for a client, it
               //should not send more than one non-confirmable notification
               //every 3 seconds and should use an even less aggressive rate
               //when possible (refer to RFC 7641, section 4.5.1)
               if((time - observer->retransmitTimestamp) >=
                  COAP_SERVER_MIN_NON_CONFIRMABLE_NOTIF_INTERVAL)
               {
                  //Start to transmit a non-confirmable notification
                  coapServerSendNotificationResponse(context, observer);

                  //Check whether the target resource has been deleted
                  if(observer->state == COAP_OBSERVER_STATE_REGISTERED &&
                     observer->resource == NULL)
                  {
                     //Removes the client's entry from the list of observers
                     coapServerDeleteObserver(observer);
                  }
               }
            }
         }
      }
      else if(observer->state == COAP_OBSERVER_STATE_UPDATING)
      {
         //Check whether the retransmission timer has expired
         if((time - observer->retransmitTimestamp) >=
            observer->retransmitTimeout)
         {
            //The sender retransmits the Confirmable message at exponentially
            //increasing intervals, until it receives an acknowledgement (or
            //Reset message) or runs out of attempts
            if(observer->retransmitCount < COAP_SERVER_MAX_RETRANSMIT)
            {
               //Retransmit the confirmable notification
               coapServerSendNotificationResponse(context, observer);
            }
            else
            {
               //If the server does not receive an acknowledgement in reply to
               //a confirmable notification, it will assume that the client is
               //no longer interested and will eventually remove the associated
               //entry from the list of observers (refer to RFC 7641, section 3.5)
               coapServerDeleteObserver(observer);
            }
         }
      }
      else
      {
         //Just for sanity
      }
   }
}


/**
 * @brief Process registration request
 * @param[in] context Pointer to the CoAP server context
 * @return Error code
 **/

error_t coapServerProcessRegistrationRequest(CoapServerContext *context)
{
   error_t error;
   uint32_t value;
   CoapResource *resource;
   CoapObserver *observer;
   CoapMessageHeader *header;

   //CoAP uses a short fixed-length binary header (4 bytes)
   header = (CoapMessageHeader *) context->request.buffer;

   //When included in a GET request, the Observe option extends the GET method
   //so it does not only retrieve a current representation of the target
   //resource, but also requests the server to add or remove an entry in the
   //list of observers of the resource depending on the option value
   error = coapGetUintOption(&context->request, COAP_OPT_OBSERVE, 0, &value);

   //The Observe Option is not critical for processing the request
   if(!error)
   {
      //Search the list of resources for a matching URI
      resource = coapServerFindResource(context, context->uri);

      //Matching resource found?
      if(resource != NULL)
      {
         //The entry in the list of observers is keyed by the client endpoint
         //and the token specified by the client in the request
         observer = coapServerFindObserver(context, header->token,
            header->tokenLen);

         //If an entry with a matching endpoint/token pair is already present
         //in the list, the server must not add a new entry but must replace or
         //update the existing one
         if(observer == NULL)
         {
            //Add an entry with the client endpoint and request token to the
            //list of observers of the target resource
            observer = coapServerCreateObserver(context, header->token,
               header->tokenLen, resource);
         }

         //Valid entry?
         if(observer != NULL)
         {
            //When included in a response, the Observe Option identifies the
            //message as a notification. The option value is a sequence number
            //for reordering detection (refer to RFC 7641, section 2)
            coapSetUintOption(&context->response, COAP_OPT_OBSERVE, 0,
               resource->seqNum);

            //Any registered callback?
            if(context->observeCallback != NULL)
            {
               //Return the current representation of the target resource
               error = context->observeCallback(context, observer, resource);
            }
            else
            {
               //No callback function registered
               error = ERROR_NOT_FOUND;
            }
         }
         else
         {
            //If the server is unwilling or unable to add a new entry to the
            //list of observers, then the request falls back to a normal GET
            //request and the response does not include the Observe Option
            error = ERROR_NOT_FOUND;
         }
      }
      else
      {
         //The target resource does not exist
         error = ERROR_NOT_FOUND;
      }
   }
   else
   {
      //Process the GET request as usual
      error = ERROR_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Restore original registration request
 * @param[in] context Pointer to the CoAP server context
 * @param[in] observer Pointer to the registered observer
 **/

void coapServerRestoreRegistrationRequest(CoapServerContext *context,
   CoapObserver *observer)
{
   CoapMessageHeader *header;

   //Point to the CoAP request header
   header = (CoapMessageHeader *) context->request.buffer;

   //Format message header
   header->version = COAP_VERSION_1;
   header->type = observer->type;
   header->tokenLen = observer->tokenLen;
   header->code = COAP_CODE_GET;
   header->mid = htons(observer->mid);

   //Copy the token specified by the client
   osMemcpy(header->token, observer->token, observer->tokenLen);

   //Set the length of the CoAP message
   context->request.length = sizeof(CoapMessageHeader) + header->tokenLen;
   context->request.pos = 0;

   //Check whether the target resource exists
   if(observer->resource != NULL)
   {
      //Encode the path component into multiple Uri-Path options
      coapSplitRepeatableOption(&context->request, COAP_OPT_URI_PATH,
         observer->resource->uri, '/');
   }

   //Restore the source and destination IP addresses
   context->localInterface = observer->interface;
   context->localIpAddr = observer->serverIpAddr;
   context->remoteIpAddr = observer->clientIpAddr;
   context->remotePort = observer->clientPort;
}


/**
 * @brief Initialize notification response
 * @param[in] context Pointer to the CoAP server context
 * @param[in] observer Pointer to the registered observer
 **/

void coapServerInitNotificationResponse(CoapServerContext *context,
   CoapObserver *observer)
{
   CoapMessageHeader *header;

   //Point to the CoAP response header
   header = (CoapMessageHeader *) context->response.buffer;

   //Format message header
   header->version = COAP_VERSION_1;
   header->type = observer->type;
   header->tokenLen = observer->tokenLen;
   header->code = COAP_CODE_INTERNAL_SERVER;

   //A new notification is transmitted with a new message ID
   if(observer->changed)
   {
      //To properly process a Reset message that rejects a non-confirmable
      //notification, a server needs to remember the message IDs of the
      //non-confirmable notifications it sends (refer to RFC 7641, section 4.5)
      observer->mid = context->mid;

      //Increment message identifier
      context->mid++;
   }

   //The message ID is a 16-bit unsigned integer that is generated by
   //the sender of a Confirmable or Non-confirmable message
   header->mid = htons(observer->mid);

   //Each notification includes the token specified by the client in the
   //request (refer to RFC 7641, section 3.2)
   osMemcpy(header->token, observer->token, observer->tokenLen);

   //Set the length of the CoAP message
   context->response.length = sizeof(CoapMessageHeader) + header->tokenLen;
   context->response.pos = 0;
}


/**
 * @brief Send notification response
 * @param[in] context Pointer to the CoAP server context
 * @param[in] observer Pointer to the registered observer
 * @return Error code
 **/

error_t coapServerSendNotificationResponse(CoapServerContext *context,
   CoapObserver *observer)
{
   error_t error;
   systime_t time;
   CoapMessageType type;
   CoapResource *resource;

   //Initialize status code
   error = NO_ERROR;

   //Get current time
   time = osGetSystemTime();

   //Restore original registration request
   coapServerRestoreRegistrationRequest(context, observer);
   //Initialize notification response
   coapServerInitNotificationResponse(context, observer);

   //Point to the resource observed by the client
   resource = observer->resource;

   //Check whether the target resource exists
   if(resource != NULL)
   {
      //To provide an order among notifications for the client, the server sets
      //the value of the Observe Option in each notification to the 24 least
      //significant bits of a strictly increasing sequence number (refer to
      //RFC 7641, section 3.4)
      coapSetUintOption(&context->response, COAP_OPT_OBSERVE, 0,
         resource->seqNum);

      //Any registered callback?
      if(context->observeCallback != NULL)
      {
         //Invoke user callback function
         error = context->observeCallback(context, observer, resource);
      }
      else
      {
         //No callback function registered
         error = ERROR_FAILURE;
      }

      //Check status code
      if(!error)
      {
         //A notification can be sent in a confirmable or a non-confirmable
         //message. The message type used is typically application dependent
         //and may be determined by the server for each notification
         //individually
         if(observer->type == COAP_TYPE_CON)
         {
            //The server may send a notification in a confirmable CoAP message
            //to request an acknowledgement from the client
            coapSetType(&context->response, COAP_TYPE_CON);
         }
         else
         {
            //A server that transmits notifications mostly in non-confirmable
            //messages must send a notification in a confirmable message instead
            //of a non-confirmable message at least every 24 hours. This
            //prevents a client that went away or is no longer interested from
            //remaining in the list of observers indefinitely (refer to RFC 7641,
            //section 3.5)
            if((time - observer->ackTimestamp) >=
               COAP_SERVER_MAX_CONFIRMABLE_NOTIF_INTERVAL)
            {
               coapSetType(&context->response, COAP_TYPE_CON);
            }
            else
            {
               coapSetType(&context->response, COAP_TYPE_NON);
            }
         }
      }
   }
   else
   {
      //When the resource is deleted, the server sends a notification with an
      //appropriate response code (such as 4.04 Not Found)
      coapSetCode(&context->response, COAP_CODE_NOT_FOUND);

      //Send the notification in a confirmable CoAP message to request an
      //acknowledgement from the client
      coapSetType(&context->response, COAP_TYPE_CON);
   }

   //Check status code
   if(!error)
   {
      //Debug message
      TRACE_INFO("CoAP Server: Sending CoAP message (%" PRIuSIZE " bytes)...\r\n",
         context->response.length);

      //Dump the contents of the message for debugging purpose
      coapDumpMessage(context->response.buffer, context->response.length);

      //Send notification response
      coapServerSendResponse(context, context->response.buffer,
         context->response.length);

      //Save the time at which the message was sent
      observer->retransmitTimestamp = osGetSystemTime();

      //Retrieve message type
      coapGetType(&context->response, &type);

      //Confirmable notification?
      if(type == COAP_TYPE_CON)
      {
         //Initial transmission?
         if(observer->state == COAP_OBSERVER_STATE_REGISTERED)
         {
            //Initialize the transmission parameters
            observer->retransmitCount = 0;

            //The initial timeout is set to a random duration
            observer->retransmitTimeout = netGetRandRange(context->netContext,
               COAP_SERVER_ACK_TIMEOUT_MIN, COAP_SERVER_ACK_TIMEOUT_MAX);
         }
         else
         {
            //The timeout is doubled
            observer->retransmitTimeout *= 2;
         }

         //Increment retransmission counter
         observer->retransmitCount++;

         //Update the state of the observer
         observer->state = COAP_OBSERVER_STATE_UPDATING;
      }
   }

   //Clear flag
   observer->changed = FALSE;

   //Return status code
   return error;
}


/**
 * @brief Process Acknowledgement message
 * @param[in] context Pointer to the CoAP server context
 * @return Error code
 **/

error_t coapServerProcessAck(CoapServerContext *context)
{
   error_t error;
   CoapMessageHeader *header;

   //Initialize status code
   error = NO_ERROR;

   //CoAP uses a short fixed-length binary header (4 bytes)
   header = (CoapMessageHeader *) context->request.buffer;

   //Empty message?
   if(header->code == COAP_CODE_EMPTY)
   {
      uint_t i;
      CoapObserver *observer;

      //Loop through the list of registered observers
      for(i = 0; i < context->numObservers; i++)
      {
         //Point to the current entry
         observer = &context->observers[i];

         //Check the state of the observer
         if(observer->state == COAP_OBSERVER_STATE_UPDATING)
         {
            //The source endpoint of the response must be the same as the
            //destination endpoint of the original request
            if(observer->interface == context->localInterface &&
               ipCompAddr(&observer->serverIpAddr, &context->localIpAddr) &&
               ipCompAddr(&observer->clientIpAddr, &context->remoteIpAddr) &&
               observer->clientPort == context->remotePort)
            {
               //The message ID of the Acknowledgment must match the message ID
               //of the Confirmable message
               if(observer->mid == ntohs(header->mid))
               {
                  //An acknowledgement message signals to the server that the
                  //client is alive and interested in receiving further
                  //notifications (refer to RFC 7641, section 3.5)
                  observer->ackTimestamp = osGetSystemTime();

                  //The server must stop retransmitting its notification on any
                  //matching Acknowledgement
                  observer->state = COAP_OBSERVER_STATE_REGISTERED;

                  //The server must keep the resource state that is observed by
                  //the client as closely in sync with the actual resource state
                  //as possible
                  if(observer->changed)
                  {
                     //Start to transmit a new notification with a representation
                     //of the current resource state. Should the resource have
                     //changed its state more than once in the meantime, the
                     //notifications for the intermediate states are silently
                     //skipped (refer to RFC 7641, section 4.5.2)
                     coapServerSendNotificationResponse(context, observer);
                  }

                  //Check whether the target resource has been deleted
                  if(observer->state == COAP_OBSERVER_STATE_REGISTERED &&
                     observer->resource == NULL)
                  {
                     //Removes the client's entry from the list of observers
                     coapServerDeleteObserver(observer);
                  }
               }
            }
         }
      }
   }
   else
   {
      //Rejecting an Acknowledgement message is effected by silently ignoring
      //it (refer to RFC 7252, section 4.2)
      error = ERROR_INVALID_MESSAGE;
   }

   //More generally, recipients of Acknowledgement and Reset messages must not
   //respond with either Acknowledgement or Reset messages
   context->response.length = 0;

   //Return status code
   return error;
}


/**
 * @brief Process Reset message
 * @param[in] context Pointer to the CoAP server context
 * @return Error code
 **/

error_t coapServerProcessReset(CoapServerContext *context)
{
   error_t error;
   CoapMessageHeader *header;

   //Initialize status code
   error = NO_ERROR;

   //CoAP uses a short fixed-length binary header (4 bytes)
   header = (CoapMessageHeader *) context->request.buffer;

   //Empty message?
   if(header->code == COAP_CODE_EMPTY)
   {
      uint_t i;
      CoapObserver *observer;

      //Loop through the list of registered observers
      for(i = 0; i < context->numObservers; i++)
      {
         //Point to the current entry
         observer = &context->observers[i];

         //Check the state of the observer
         if(observer->state != COAP_OBSERVER_STATE_UNREGISTERED)
         {
            //The source endpoint of the response must be the same as the
            //destination endpoint of the original request
            if(observer->interface == context->localInterface &&
               ipCompAddr(&observer->serverIpAddr, &context->localIpAddr) &&
               ipCompAddr(&observer->clientIpAddr, &context->remoteIpAddr) &&
               observer->clientPort == context->remotePort)
            {
               //The message ID of the Reset must match the message ID of the
               //Confirmable message
               if(observer->mid == ntohs(header->mid))
               {
                  //If a client rejects a notification with a Reset message,
                  //then the client is considered no longer interested and the
                  //server must remove the associated entry from the list of
                  //observers (refer to RFC 7641, section 4.5)
                  coapServerDeleteObserver(observer);
               }
            }
         }
      }
   }
   else
   {
      //Rejecting an Reset message is effected by silently ignoring it (refer
      //to RFC 7252, section 4.2)
      error = ERROR_INVALID_MESSAGE;
   }

   //More generally, recipients of Acknowledgement and Reset messages must not
   //respond with either Acknowledgement or Reset messages
   context->response.length = 0;

   //Return status code
   return error;
}


/**
 * @brief Search the list of resources for a given URI
 * @param[in] context Pointer to the CoAP server context
 * @param[in] uri NULL-terminated string containing the path to the resource
 * @return Pointer to the matching resource
 **/

CoapResource *coapServerFindResource(CoapServerContext *context,
   const char_t *uri)
{
   uint_t i;
   CoapResource *resource;

   //Initialize pointer
   resource = NULL;

   //Loop through the list of resources
   for(i = 0; i < context->numResources; i++)
   {
      //Valid entry?
      if(context->resources[i].uri[0] != '\0')
      {
         //Matching URI?
         if(osStrcmp(context->resources[i].uri, uri) == 0)
         {
            resource = &context->resources[i];
            break;
         }
      }
   }

   //Return a pointer to the matching entry
   return resource;
}


/**
 * @brief Create a new entry in the list of observers
 * @param[in] context Pointer to the CoAP server context
 * @param[in] token Pointer to the token specified by the client
 * @param[in] tokenLen Length of the token, in bytes
 * @param[in] resource Pointer to the target resource
 * @return Pointer to the newly created entry
 **/

CoapObserver *coapServerCreateObserver(CoapServerContext *context,
   const uint8_t *token, size_t tokenLen, CoapResource *resource)
{
   uint_t i;
   systime_t time;
   CoapObserver *observer;

   //Get current time
   time = osGetSystemTime();

   //Initialize pointer
   observer = NULL;

   //Loop through the list of registered observers
   for(i = 0; i < context->numObservers; i++)
   {
      //Check whether the entry is available
      if(context->observers[i].state == COAP_OBSERVER_STATE_UNREGISTERED)
      {
         //Point to the current entry
         observer = &context->observers[i];

         //Clear entry
         osMemset(observer, 0, sizeof(CoapObserver));

         //Initialize entry
         observer->resource = resource;
         observer->interface = context->localInterface;
         observer->serverIpAddr = context->localIpAddr;
         observer->clientIpAddr = context->remoteIpAddr;
         observer->clientPort = context->remotePort;
         observer->retransmitTimestamp = time;
         observer->ackTimestamp = time;

         //All notifications carry the token specified by the client, so the
         //client can easily correlate them to the request
         osMemcpy(observer->token, token, tokenLen);
         observer->tokenLen = tokenLen;

         //A notification can be confirmable or non-confirmable. The message
         //type used for a notification is independent of the type used for
         //the request and of any previous notification (refer to RFC 7641,
         //section 3.5)
         observer->type = COAP_TYPE_CON;

         //Update the state of the observer
         observer->state = COAP_OBSERVER_STATE_REGISTERED;

         //We are done
         break;
      }
   }

   //Return a pointer to the newly created entry
   return observer;
}


/**
 * @brief Search the list of observers for a given entry
 * @param[in] context Pointer to the CoAP server context
 * @param[in] token Pointer to the token specified by the client
 * @param[in] tokenLen Length of the token, in bytes
 * @return Pointer to the matching entry
 **/

CoapObserver *coapServerFindObserver(CoapServerContext *context,
   const uint8_t *token, size_t tokenLen)
{
   uint_t i;
   CoapObserver *observer;

   //Loop through the list of registered observers
   for(i = 0; i < context->numObservers; i++)
   {
      //Point to the current entry
      observer = &context->observers[i];

      //Valid entry?
      if(observer->state != COAP_OBSERVER_STATE_UNREGISTERED)
      {
         //Matching client endpoint?
         if(observer->interface == context->localInterface &&
            ipCompAddr(&observer->serverIpAddr, &context->localIpAddr) &&
            ipCompAddr(&observer->clientIpAddr, &context->remoteIpAddr) &&
            observer->clientPort == context->remotePort)
         {
            //Matching token?
            if(observer->tokenLen == tokenLen &&
               osMemcmp(observer->token, token, tokenLen) == 0)
            {
               break;
            }
         }
      }
   }

   //Return a pointer to the matching entry
   return (i < context->numObservers) ? observer : NULL;
}


/**
 * @brief Remove an entry from the list of observers
 * @param[in] observer Pointer to the entry to be removed
 **/

void coapServerDeleteObserver(CoapObserver *observer)
{
   //Check the state of the observer
   if(observer->state != COAP_OBSERVER_STATE_UNREGISTERED)
   {
      //Debug message
      TRACE_INFO("CoAP Server: Deleting observer...\r\n");

      //Delete the entry
      osMemset(observer, 0, sizeof(CoapObserver));
   }
}

#endif
