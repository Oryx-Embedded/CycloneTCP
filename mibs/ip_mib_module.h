/**
 * @file ip_mib_module.h
 * @brief IP MIB module
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
 * @version 2.6.0
 **/

#ifndef _IP_MIB_MODULE_H
#define _IP_MIB_MODULE_H

//Dependencies
#include "mibs/mib_common.h"

//IP MIB module support
#ifndef IP_MIB_SUPPORT
   #define IP_MIB_SUPPORT DISABLED
#elif (IP_MIB_SUPPORT != ENABLED && IP_MIB_SUPPORT != DISABLED)
   #error IP_MIB_SUPPORT parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief IP forwarding state
 **/

typedef enum
{
   IP_MIB_IP_FORWARDING_ENABLED  = 1,
   IP_MIB_IP_FORWARDING_DISABLED = 2
} IpMibIpForwarding;


/**
 * @brief IP status
 **/

typedef enum
{
   IP_MIB_IP_STATUS_UP   = 1,
   IP_MIB_IP_STATUS_DOWN = 2
} IpMibIpStatus;


/**
 * @brief IP address type
 **/

typedef enum
{
   IP_MIB_ADDR_TYPE_UNICAST   = 1,
   IP_MIB_ADDR_TYPE_ANYCAST   = 2,
   IP_MIB_ADDR_TYPE_BROADCAST = 3
} IpMibAddrType;


/**
 * @brief IP address origin
 **/

typedef enum
{
   IP_MIB_ADDR_ORIGIN_MANUAL     = 2,
   IP_MIB_ADDR_ORIGIN_DHCP       = 4,
   IP_MIB_ADDR_ORIGIN_LINK_LAYER = 5,
   IP_MIB_ADDR_ORIGIN_RANDOM     = 6,
} IpMibAddrOrigin;

/**
 * @brief IP address status
 **/

typedef enum
{
   IP_MIB_ADDR_STATUS_PREFERRED    = 1,
   IP_MIB_ADDR_STATUS_DEPRECATED   = 2,
   IP_MIB_ADDR_STATUS_INVALID      = 3,
   IP_MIB_ADDR_STATUS_INACCESSIBLE = 4,
   IP_MIB_ADDR_STATUS_UNKNOWN      = 5,
   IP_MIB_ADDR_STATUS_TENTATIVE    = 6,
   IP_MIB_ADDR_STATUS_DUPLICATE    = 7,
   IP_MIB_ADDR_STATUS_OPTIMISTIC   = 8
} IpMibAddrStatus;


/**
 * @brief Prefix origin
 **/

typedef enum
{
   IP_MIB_PREFIX_ORIGIN_MANUAL     = 2,
   IP_MIB_PREFIX_ORIGIN_WELL_KNOWN = 3,
   IP_MIB_PREFIX_ORIGIN_DHCP       = 4,
   IP_MIB_PREFIX_ORIGIN_ROUTER_ADV = 5,
} IpMibPrefixOrigin;


/**
 * @brief Type of mapping
 **/

typedef enum
{
   IP_MIB_NET_TO_PHYS_TYPE_OTHER   = 1,
   IP_MIB_NET_TO_PHYS_TYPE_INVALID = 2,
   IP_MIB_NET_TO_PHYS_TYPE_DYNAMIC = 3,
   IP_MIB_NET_TO_PHYS_TYPE_STATIC  = 4,
   IP_MIB_NET_TO_PHYS_TYPE_LOCAL   = 5
} IpMibNetToPhysType;


/**
 * @brief Entry state
 **/

typedef enum
{
   IP_MIB_NET_TO_PHYS_STATE_REACHABLE  = 1,
   IP_MIB_NET_TO_PHYS_STATE_STALE      = 2,
   IP_MIB_NET_TO_PHYS_STATE_DELAY      = 3,
   IP_MIB_NET_TO_PHYS_STATE_PROBE      = 4,
   IP_MIB_NET_TO_PHYS_STATE_INVALID    = 5,
   IP_MIB_NET_TO_PHYS_STATE_UNKNOWN    = 6,
   IP_MIB_NET_TO_PHYS_STATE_INCOMPLETE = 7
} IpMibNetToPhysState;


/**
 * @brief Router preferences
 **/

typedef enum
{
   IP_MIB_ROUTER_PREFERENCE_RESERVED = -2,
   IP_MIB_ROUTER_PREFERENCE_LOW      = -1,
   IP_MIB_ROUTER_PREFERENCE_MEDIUM   = 0,
   IP_MIB_ROUTER_PREFERENCE_HIGH     = 1
} IpMibRouterPreference;


/**
 * @brief IP MIB base
 **/

typedef struct
{
   uint32_t ipIfStatsTableLastChange;
   int32_t ipAddressSpinLock;
#if (IPV4_SUPPORT == ENABLED)
   int32_t ipForwarding;
   int32_t ipDefaultTTL;
   int32_t ipReasmTimeout;
   uint32_t ipv4InterfaceTableLastChange;
#endif
#if (IPV6_SUPPORT == ENABLED)
   int32_t ipv6IpForwarding;
   int32_t ipv6IpDefaultHopLimit;
   uint32_t ipv6InterfaceTableLastChange;
   int32_t ipv6RouterAdvertSpinLock;
#endif
} IpMibBase;


//IP MIB related constants
extern IpMibBase ipMibBase;
extern const MibObject ipMibObjects[];
extern const MibModule ipMibModule;

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
