/**
 * @file dns_sd_responder.c
 * @brief DNS-SD responder (DNS-Based Service Discovery)
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
 * @section Description
 *
 * DNS-SD allows clients to discover a list of named instances of that
 * desired service, using standard DNS queries. Refer to the following
 * RFCs for complete details:
 * - RFC 6763: DNS-Based Service Discovery
 * - RFC 2782: A DNS RR for specifying the location of services (DNS SRV)
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL DNS_SD_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "mdns/mdns_responder.h"
#include "dns_sd/dns_sd_responder.h"
#include "dns_sd/dns_sd_responder_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (DNS_SD_RESPONDER_SUPPORT == ENABLED)

//Tick counter to handle periodic operations
systime_t dnsSdResponderTickCounter;


/**
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains DNS-SD responder settings
 **/

void dnsSdResponderGetDefaultSettings(DnsSdResponderSettings *settings)
{
   //Use default interface
   settings->interface = netGetDefaultInterface();

   //DNS-SD services
   settings->numServices = 0;
   settings->services = NULL;

   //Number of announcement packets
   settings->numAnnouncements = MDNS_ANNOUNCE_NUM;
   //TTL resource record
   settings->ttl = DNS_SD_DEFAULT_RR_TTL;
   //FSM state change event
   settings->stateChangeEvent = NULL;
}


/**
 * @brief DNS-DS initialization
 * @param[in] context Pointer to the DNS-SD responder context
 * @param[in] settings DNS-SD specific settings
 * @return Error code
 **/

error_t dnsSdResponderInit(DnsSdResponderContext *context,
   const DnsSdResponderSettings *settings)
{
   uint_t i;
   NetInterface *interface;
   DnsSdResponderService *service;

   //Debug message
   TRACE_INFO("Initializing DNS-SD...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;

   //Check settings
   if(settings->interface == NULL || settings->services == NULL ||
      settings->numServices < 1)
   {
      return ERROR_INVALID_PARAMETER;
   }

   //Point to the underlying network interface
   interface = settings->interface;

   //Clear the DNS-SD responder context
   osMemset(context, 0, sizeof(DnsSdResponderContext));

   //Initialize DNS-SD responder context
   context->interface = settings->interface;
   context->numServices = settings->numServices;
   context->services = settings->services;
   context->numAnnouncements = settings->numAnnouncements;
   context->ttl = settings->ttl;
   context->stateChangeEvent = settings->stateChangeEvent;

   //DNS-SD responder is currently suspended
   context->running = FALSE;

   //Loop through the list of registered services
   for(i = 0; i < context->numServices; i++)
   {
      //Point to the current entry
      service = &context->services[i];

      //Clear entry
      osMemset(service, 0, sizeof(DnsSdResponderService));

      //Attach DNS-SD responder context
      service->context = context;
      //Initialize state machine
      service->state = MDNS_STATE_INIT;
   }

   //Attach the DNS-SD responder context to the network interface
   interface->dnsSdResponderContext = context;

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Start DNS-SD responder
 * @param[in] context Pointer to the DNS-SD responder context
 * @return Error code
 **/

error_t dnsSdResponderStart(DnsSdResponderContext *context)
{
   uint_t i;

   //Make sure the DNS-SD responder context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Starting DNS-SD...\r\n");

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Start DNS-SD responder
   context->running = TRUE;

   //Loop through the list of registered services
   for(i = 0; i < context->numServices; i++)
   {
      //Initialize state machine
      context->services[i].state = MDNS_STATE_INIT;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Stop DNS-SD responder
 * @param[in] context Pointer to the DNS-SD responder context
 * @return Error code
 **/

error_t dnsSdResponderStop(DnsSdResponderContext *context)
{
   uint_t i;

   //Make sure the DNS-SD responder context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Stopping DNS-SD...\r\n");

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Suspend DNS-SD responder
   context->running = FALSE;

   //Loop through the list of registered services
   for(i = 0; i < context->numServices; i++)
   {
      //Initialize state machine
      context->services[i].state = MDNS_STATE_INIT;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Register a DNS-SD service
 * @param[in] context Pointer to the DNS-SD responder context
 * @param[in] index Zero-based index identifying a slot
 * @param[in] instanceName NULL-terminated string that contains the service
 *   instance name
 * @param[in] serviceName NULL-terminated string that contains the name of the
 *   service to be registered
 * @param[in] priority Priority field
 * @param[in] weight Weight field
 * @param[in] port Port number
 * @param[in] metadata NULL-terminated string that contains the discovery-time
 *   metadata (TXT record)
 * @return Error code
 **/

error_t dnsSdResponderRegisterService(DnsSdResponderContext *context,
   uint_t index, const char_t *instanceName, const char_t *serviceName,
   uint16_t priority, uint16_t weight, uint16_t port, const char_t *metadata)
{
   size_t i;
   size_t j;
   size_t k;
   size_t n;
   DnsSdResponderService *service;

   //Check parameters
   if(context == NULL || instanceName == NULL || serviceName == NULL ||
      metadata == NULL)
   {
      return ERROR_INVALID_PARAMETER;
   }

   //The implementation limits the number of services that can be advertised
   if(index >= context->numServices)
      return ERROR_INVALID_PARAMETER;

   //Make sure the length of the instance name is acceptable
   if(osStrlen(instanceName) > DNS_SD_MAX_INSTANCE_NAME_LEN)
      return ERROR_INVALID_LENGTH;

   //Make sure the length of the service name is acceptable
   if(osStrlen(serviceName) > DNS_SD_MAX_SERVICE_NAME_LEN)
      return ERROR_INVALID_LENGTH;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Point to the specified entry
   service = &context->services[index];

   //Valid service?
   if(service->instanceName[0] != '\0' &&
      service->serviceName[0] != '\0')
   {
      //Send a goodbye packet
      dnsSdResponderSendGoodbye(service);
   }

   //Instance name
   osStrcpy(service->instanceName, instanceName);
   //Service name
   osStrcpy(service->serviceName, serviceName);

   //Priority field
   service->priority = priority;
   //Weight field
   service->weight = weight;
   //Port number
   service->port = port;

   //Clear TXT record
   service->metadataLen = 0;

   //Point to the beginning of the information string
   i = 0;
   j = 0;

   //Point to the beginning of the resulting TXT record data
   k = 0;

   //Format TXT record
   while(1)
   {
      //End of text data?
      if(metadata[i] == '\0' || metadata[i] == ';')
      {
         //Calculate the length of the text data
         n = MIN(i - j, UINT8_MAX);

         //Check the length of the resulting TXT record
         if((service->metadataLen + n + 1) > DNS_SD_MAX_METADATA_LEN)
            break;

         //Write length field
         service->metadata[k] = n;
         //Write text data
         osMemcpy(service->metadata + k + 1, metadata + j, n);

         //Jump to the next text data
         j = i + 1;
         //Advance write index
         k += n + 1;

         //Update the length of the TXT record
         service->metadataLen += n + 1;

         //End of string detected?
         if(metadata[i] == '\0')
            break;
      }

      //Advance read index
      i++;
   }

   //Empty TXT record?
   if(service->metadataLen == 0)
   {
      //An empty TXT record shall contain a single zero byte
      service->metadata[0] = 0;
      service->metadataLen = 1;
   }

   //Restart probing process
   dnsSdResponderStartProbing(context);

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Unregister a DNS-SD service
 * @param[in] context Pointer to the DNS-SD responder context
 * @param[in] index Zero-based index identifying the service to be unregistered
 * @return Error code
 **/

error_t dnsSdResponderUnregisterService(DnsSdResponderContext *context,
   uint_t index)
{
   DnsSdResponderService *service;

   //Check parameters
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //The implementation limits the number of services that can be advertised
   if(index >= context->numServices)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Point to the specified entry
   service = &context->services[index];

   //Valid service?
   if(service->instanceName[0] != '\0' &&
      service->serviceName[0] != '\0')
   {
      //Send a goodbye packet
      dnsSdResponderSendGoodbye(service);
   }

   //Remove the service from the list
   service->instanceName[0] = '\0';
   service->serviceName[0] = '\0';

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Restart probing process
 * @param[in] context Pointer to the DNS-SD responder context
 * @return Error code
 **/

error_t dnsSdResponderStartProbing(DnsSdResponderContext *context)
{
   uint_t i;

   //Check parameter
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Loop through the list of registered services
   for(i = 0; i < context->numServices; i++)
   {
      //Force DNS-SD to start probing again
      context->services[i].state = MDNS_STATE_INIT;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief DNS-SD responder timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * manage DNS-SD operation
 *
 * @param[in] context Pointer to the DNS-SD responder context
 **/

void dnsSdResponderTick(DnsSdResponderContext *context)
{
   uint_t i;
   systime_t time;
   systime_t delay;
   NetInterface *interface;
   DnsSdResponderService *service;

   //Make sure the DNS-SD responder has been properly instantiated
   if(context == NULL)
      return;

   //Point to the underlying network interface
   interface = context->interface;

   //Get current time
   time = osGetSystemTime();

   //Loop through the list of registered services
   for(i = 0; i < context->numServices; i++)
   {
      //Point to the current entry
      service = &context->services[i];

      //Valid service?
      if(service->instanceName[0] != '\0' &&
         service->serviceName[0] != '\0')
      {
         //Check current state
         if(service->state == MDNS_STATE_INIT)
         {
            //Ensure the mDNS and DNS-SD services are running
            if(context->running && interface->mdnsResponderContext != NULL)
            {
               //Wait for mDNS probing to complete
               if(interface->mdnsResponderContext->state == MDNS_STATE_IDLE)
               {
                  //Initial random delay
                  delay = netGenerateRandRange(MDNS_RAND_DELAY_MIN,
                     MDNS_RAND_DELAY_MAX);

                  //Perform probing
                  dnsSdResponderChangeState(service, MDNS_STATE_PROBING, delay);
               }
            }
         }
         else if(service->state == MDNS_STATE_PROBING)
         {
            //Probing failed?
            if(service->conflict && service->retransmitCount > 0)
            {
               //Programmatically change the service instance name
               dnsSdResponderChangeInstanceName(service);

               //Probe again, and repeat as necessary until a unique name is found
               dnsSdResponderChangeState(service, MDNS_STATE_PROBING,
                  MDNS_PROBE_CONFLICT_DELAY);
            }
            //Tie-break lost?
            else if(service->tieBreakLost && service->retransmitCount > 0)
            {
               //The host defers to the winning host by waiting one second, and
               //then begins probing for this record again
               dnsSdResponderChangeState(service, MDNS_STATE_PROBING,
                  MDNS_PROBE_DEFER_DELAY);
            }
            else
            {
               //Check current time
               if(timeCompare(time, service->timestamp + service->timeout) >= 0)
               {
                  //Probing is on-going?
                  if(service->retransmitCount < MDNS_PROBE_NUM)
                  {
                     //First probe?
                     if(service->retransmitCount == 0)
                     {
                        //Apparently conflicting mDNS responses received before
                        //the first probe packet is sent must be silently ignored
                        service->conflict = FALSE;
                        service->tieBreakLost = FALSE;
                     }

                     //Send probe packet
                     dnsSdResponderSendProbe(service);

                     //Save the time at which the packet was sent
                     service->timestamp = time;
                     //Time interval between subsequent probe packets
                     service->timeout = MDNS_PROBE_DELAY;
                     //Increment retransmission counter
                     service->retransmitCount++;
                  }
                  //Probing is complete?
                  else
                  {
                     //The mDNS responder must send unsolicited mDNS responses
                     //containing all of its newly registered resource records
                     if(context->numAnnouncements > 0)
                     {
                        dnsSdResponderChangeState(service, MDNS_STATE_ANNOUNCING, 0);
                     }
                     else
                     {
                        dnsSdResponderChangeState(service, MDNS_STATE_IDLE, 0);
                     }
                  }
               }
            }
         }
         else if(service->state == MDNS_STATE_ANNOUNCING)
         {
            //Whenever a mDNS responder receives any mDNS response (solicited or
            //otherwise) containing a conflicting resource record, the conflict
            //must be resolved
            if(service->conflict)
            {
               //Probe again, and repeat as necessary until a unique name is
               //found
               dnsSdResponderChangeState(service, MDNS_STATE_PROBING, 0);
            }
            else
            {
               //Check current time
               if(timeCompare(time, service->timestamp + service->timeout) >= 0)
               {
                  //Send announcement packet
                  dnsSdResponderSendAnnouncement(service);

                  //Save the time at which the packet was sent
                  service->timestamp = time;
                  //Increment retransmission counter
                  service->retransmitCount++;

                  //First announcement packet?
                  if(service->retransmitCount == 1)
                  {
                     //The mDNS responder must send at least two unsolicited
                     //responses, one second apart
                     service->timeout = MDNS_ANNOUNCE_DELAY;
                  }
                  else
                  {
                     //To provide increased robustness against packet loss, a
                     //mDNS responder may send up to eight unsolicited responses,
                     //provided that the interval between unsolicited responses
                     //increases by at least a factor of two with every response
                     //sent
                     service->timeout *= 2;
                  }

                  //Last announcement packet?
                  if(service->retransmitCount >= context->numAnnouncements)
                  {
                     //A mDNS responder must not send regular periodic
                     //announcements
                     dnsSdResponderChangeState(service, MDNS_STATE_IDLE, 0);
                  }
               }
            }
         }
         else if(service->state == MDNS_STATE_IDLE)
         {
            //Whenever a mDNS responder receives any mDNS response (solicited or
            //otherwise) containing a conflicting resource record, the conflict
            //must be resolved
            if(service->conflict)
            {
               //Probe again, and repeat as necessary until a unique name is
               //found
               dnsSdResponderChangeState(service, MDNS_STATE_PROBING, 0);
            }
         }
      }
   }
}


/**
 * @brief Callback function for link change event
 * @param[in] context Pointer to the DNS-SD responder context
 **/

void dnsSdResponderLinkChangeEvent(DnsSdResponderContext *context)
{
   uint_t i;

   //Make sure the DNS-SD responder has been properly instantiated
   if(context == NULL)
      return;

   //Loop through the list of registered services
   for(i = 0; i < context->numServices; i++)
   {
      //Whenever a mDNS responder receives an indication of a link change
      //event, it must perform probing and announcing
      context->services[i].state = MDNS_STATE_INIT;
   }
}

#endif
