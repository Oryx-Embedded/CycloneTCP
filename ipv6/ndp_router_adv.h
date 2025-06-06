/**
 * @file ndp_router_adv.h
 * @brief Router advertisement service
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

#ifndef _NDP_ROUTER_ADV_H
#define _NDP_ROUTER_ADV_H

//Dependencies
#include "core/net.h"
#include "ipv6/ipv6.h"

//RA service support
#ifndef NDP_ROUTER_ADV_SUPPORT
   #define NDP_ROUTER_ADV_SUPPORT DISABLED
#elif (NDP_ROUTER_ADV_SUPPORT != ENABLED && NDP_ROUTER_ADV_SUPPORT != DISABLED)
   #error NDP_ROUTER_ADV_SUPPORT parameter is not valid
#endif

//RA service tick interval
#ifndef NDP_ROUTER_ADV_TICK_INTERVAL
   #define NDP_ROUTER_ADV_TICK_INTERVAL 100
#elif (NDP_ROUTER_ADV_TICK_INTERVAL < 10)
   #error NDP_ROUTER_ADV_TICK_INTERVAL parameter is not valid
#endif

//Application specific context
#ifndef NDP_ROUTER_ADV_PRIVATE_CONTEXT
   #define NDP_ROUTER_ADV_PRIVATE_CONTEXT
#endif

//Forward declaration of NdpRouterAdvContext structure
struct _NdpRouterAdvContext;
#define NdpRouterAdvContext struct _NdpRouterAdvContext

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Add Router Advertisement options callback
 **/

typedef void (*NdpRouterAddOptionsCallback)(NdpRouterAdvContext *context,
   NdpRouterAdvMessage *message, size_t *length);


/**
 * @brief IPv6 prefix information
 **/

typedef struct
{
   Ipv6Addr prefix;
   uint8_t length;
   bool_t onLinkFlag;
   bool_t autonomousFlag;
   uint32_t validLifetime;
   uint32_t preferredLifetime;
} NdpRouterAdvPrefixInfo;


/**
 * @brief Route information
 **/

typedef struct
{
   Ipv6Addr prefix;
   uint8_t length;
   uint8_t preference;
   uint32_t routeLifetime;
} NdpRouterAdvRouteInfo;


/**
 * @brief Context information for 6LoWPAN header compression
 **/

typedef struct
{
   uint8_t cid;
   Ipv6Addr prefix;
   uint8_t length;
   bool_t compression;
   uint16_t validLifetime;
} NdpRouterAdvContextInfo;


/**
 * @brief RA service settings
 **/

typedef struct
{
   NetInterface *interface;                        ///<Underlying network interface
   systime_t maxRtrAdvInterval;                    ///<Minimum time between unsolicited Router Advertisements
   systime_t minRtrAdvInterval;                    ///<Maximum time between unsolicited Router Advertisements
   uint8_t curHopLimit;                            ///<Value of the Cur Hop Limit field
   bool_t managedFlag;                             ///<Managed Address Configuration flag
   bool_t otherConfigFlag;                         ///<Other Configuration flag
   bool_t homeAgentFlag;                           ///<Mobile IPv6 Home Agent flag
   uint8_t preference;                             ///<Value of the Router Selection Preferences field
   bool_t proxyFlag;                               ///<Value of the Neighbor Discovery Proxy flag
   uint16_t defaultLifetime;                       ///<Value of the Router Lifetime field
   uint32_t reachableTime;                         ///<Value of the Reachable Time field
   uint32_t retransTimer;                          ///<Value of the Retrans Timer field
   uint32_t linkMtu;                               ///<Recommended MTU for the link (MTU option)
   NdpRouterAdvPrefixInfo *prefixList;             ///<List of prefixes (PIO option)
   uint_t prefixListLength;                        ///<Number of prefixes in the list
   NdpRouterAdvRouteInfo *routeList;               ///<List of routes (RIO option)
   uint_t routeListLength;                         ///<Number of routes in the list
   NdpRouterAdvContextInfo *contextList;           ///<List of compression contexts (6CO option)
   uint_t contextListLength;                       ///<Number of compression contexts in the list
   NdpRouterAddOptionsCallback addOptionsCallback; ///<Add Router Advertisement options callback
} NdpRouterAdvSettings;


/**
 * @brief RA service context
 **/

struct _NdpRouterAdvContext
{
   NdpRouterAdvSettings settings; ///<RA service settings
   bool_t running;                ///<This flag tells whether the RA service is running
   systime_t timestamp;           ///<Timestamp to manage retransmissions
   systime_t timeout;             ///<Timeout value
   uint_t routerAdvCount;         ///<Router Advertisement message counter
   NDP_ROUTER_ADV_PRIVATE_CONTEXT ///<Application specific context
};


//Tick counter to handle periodic operations
extern systime_t ndpRouterAdvTickCounter;

//RA service related functions
void ndpRouterAdvGetDefaultSettings(NdpRouterAdvSettings *settings);

error_t ndpRouterAdvInit(NdpRouterAdvContext *context,
   const NdpRouterAdvSettings *settings);

error_t ndpRouterAdvStart(NdpRouterAdvContext *context);
error_t ndpRouterAdvStop(NdpRouterAdvContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
