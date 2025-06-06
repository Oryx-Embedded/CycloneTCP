/**
 * @file dns_sd_responder.h
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

#ifndef _DNS_SD_RESPONDER_H
#define _DNS_SD_RESPONDER_H

//Dependencies
#include "core/net.h"
#include "dns/dns_common.h"
#include "mdns/mdns_common.h"

//DNS-SD responder support
#ifndef DNS_SD_RESPONDER_SUPPORT
   #define DNS_SD_RESPONDER_SUPPORT DISABLED
#elif (DNS_SD_RESPONDER_SUPPORT != ENABLED && DNS_SD_RESPONDER_SUPPORT != DISABLED)
   #error DNS_SD_RESPONDER_SUPPORT parameter is not valid
#endif

//DNS-SD responder tick interval
#ifndef DNS_SD_RESPONDER_TICK_INTERVAL
   #define DNS_SD_RESPONDER_TICK_INTERVAL 250
#elif (DNS_SD_RESPONDER_TICK_INTERVAL < 10)
   #error DNS_SD_RESPONDER_TICK_INTERVAL parameter is not valid
#endif

//Maximum length of service name
#ifndef DNS_SD_MAX_SERVICE_NAME_LEN
   #define DNS_SD_MAX_SERVICE_NAME_LEN 32
#elif (DNS_SD_MAX_SERVICE_NAME_LEN < 1)
   #error DNS_SD_MAX_SERVICE_NAME_LEN parameter is not valid
#endif

//Maximum length of instance name
#ifndef DNS_SD_MAX_INSTANCE_NAME_LEN
   #define DNS_SD_MAX_INSTANCE_NAME_LEN 32
#elif (DNS_SD_MAX_INSTANCE_NAME_LEN < 1)
   #error DNS_SD_MAX_INSTANCE_NAME_LEN parameter is not valid
#endif

//Maximum length of the discovery-time metadata (TXT record)
#ifndef DNS_SD_MAX_METADATA_LEN
   #define DNS_SD_MAX_METADATA_LEN 128
#elif (DNS_SD_MAX_METADATA_LEN < 1)
   #error DNS_SD_MAX_METADATA_LEN parameter is not valid
#endif

//Default resource record TTL (cache lifetime)
#ifndef DNS_SD_DEFAULT_RR_TTL
   #define DNS_SD_DEFAULT_RR_TTL 120
#elif (DNS_SD_DEFAULT_RR_TTL < 1)
   #error DNS_SD_DEFAULT_RR_TTL parameter is not valid
#endif

//Forward declaration of DnsSdResponderContext structure
struct _DnsSdResponderContext;
#define DnsSdResponderContext struct _DnsSdResponderContext

//Forward declaration of DnsSdResponderService structure
struct _DnsSdResponderService;
#define DnsSdResponderService struct _DnsSdResponderService

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief FSM state change callback
 **/

typedef void (*DnsSdStateChangeCallback)(DnsSdResponderService *service,
   NetInterface *interface, MdnsState state);


/**
 * @brief DNS-SD responder settings
 **/

typedef struct
{
   NetInterface *interface;                   ///<Underlying network interface
   uint_t numServices;                        ///<Maximum number of DNS-SD services that can be registered
   DnsSdResponderService *services;           ///<DNS-SD services
   uint_t numAnnouncements;                   ///<Number of announcement packets
   uint32_t ttl;                              ///<TTL resource record
   DnsSdStateChangeCallback stateChangeEvent; ///<FSM state change event
} DnsSdResponderSettings;


/**
 * @brief DNS-SD service descriptor
 **/

struct _DnsSdResponderService
{
   MdnsState state;                                       ///<FSM state
   DnsSdResponderContext *context;                        ///<DNS-SD responder context
   char_t instanceName[DNS_SD_MAX_INSTANCE_NAME_LEN + 1]; ///<Instance name
   char_t serviceName[DNS_SD_MAX_SERVICE_NAME_LEN + 1];   ///<Service name
   uint16_t priority;                                     ///<Priority of the target host
   uint16_t weight;                                       ///<Server selection mechanism
   uint16_t port;                                         ///<Port on the target host of this service
   uint8_t metadata[DNS_SD_MAX_METADATA_LEN];             ///<Discovery-time metadata (TXT record)
   size_t metadataLen;                                    ///<Length of the metadata
   bool_t conflict;                                       ///<Conflict detected
   bool_t tieBreakLost;                                   ///<Tie-break lost
   systime_t timestamp;                                   ///<Timestamp to manage retransmissions
   systime_t timeout;                                     ///<Timeout value
   uint_t retransmitCount;                                ///<Retransmission counter
};


/**
 * @brief DNS-SD responder context
 **/

struct _DnsSdResponderContext
{
   NetInterface *interface;                   ///<Underlying network interface
   uint_t numServices;                        ///<Maximum number of DNS-SD services that can be registered
   DnsSdResponderService *services;           ///<DNS-SD services
   uint_t numAnnouncements;                   ///<Number of announcement packets
   uint32_t ttl;                              ///<TTL resource record
   DnsSdStateChangeCallback stateChangeEvent; ///<FSM state change event
   bool_t running;                            ///<DNS-SD responder is currently running
};


//Tick counter to handle periodic operations
extern systime_t dnsSdResponderTickCounter;

//DNS-SD responder related functions
void dnsSdResponderGetDefaultSettings(DnsSdResponderSettings *settings);

error_t dnsSdResponderInit(DnsSdResponderContext *context,
   const DnsSdResponderSettings *settings);

error_t dnsSdResponderStart(DnsSdResponderContext *context);
error_t dnsSdResponderStop(DnsSdResponderContext *context);

error_t dnsSdResponderRegisterService(DnsSdResponderContext *context,
   uint_t index, const char_t *instanceName, const char_t *serviceName,
   uint16_t priority, uint16_t weight, uint16_t port, const char_t *metadata);

error_t dnsSdResponderUnregisterService(DnsSdResponderContext *context,
   uint_t index);

error_t dnsSdResponderStartProbing(DnsSdResponderContext *context);

void dnsSdResponderTick(DnsSdResponderContext *interface);
void dnsSdResponderLinkChangeEvent(DnsSdResponderContext *interface);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
