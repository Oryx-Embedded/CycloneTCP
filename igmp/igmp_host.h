/**
 * @file igmp_host.h
 * @brief IGMP host
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

#ifndef _IGMP_HOST_H
#define _IGMP_HOST_H

//Dependencies
#include "core/net.h"
#include "igmp/igmp_common.h"

//IGMP host support
#ifndef IGMP_HOST_SUPPORT
   #define IGMP_HOST_SUPPORT ENABLED
#elif (IGMP_HOST_SUPPORT != ENABLED && IGMP_HOST_SUPPORT != DISABLED)
   #error IGMP_HOST_SUPPORT parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Multicast group states
 **/

typedef enum
{
   IGMP_HOST_GROUP_STATE_NON_MEMBER      = 0,
   IGMP_HOST_GROUP_STATE_INIT_MEMBER     = 1,
   IGMP_HOST_GROUP_STATE_DELAYING_MEMBER = 2,
   IGMP_HOST_GROUP_STATE_IDLE_MEMBER     = 3
} IgmpHostGroupState;


/**
 * @brief Source address
 **/

typedef struct
{
   Ipv4Addr addr;          ///<Source address
   uint_t retransmitCount; ///<Retransmission counter
} IgmpHostSrcAddr;


/**
 * @brief Source address list
 **/

typedef struct
{
   uint_t numSources;                                   ///<Number of source address
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   IgmpHostSrcAddr sources[IPV4_MAX_MULTICAST_SOURCES]; ///<List of source addresses
#endif
} IgmpHostSrcAddrList;


/**
 * @brief Multicast group
 **/

typedef struct
{
   IgmpHostGroupState state;       ///<Multicast group state
   Ipv4Addr addr;                  ///<Multicast group address
   bool_t flag;                    ///<We are the last host to send a report for this group
   uint_t retransmitCount;         ///<Filter mode retransmission counter
   NetTimer timer;                 ///<Report delay timer
   IpFilterMode filterMode;        ///<Filter mode
   Ipv4SrcAddrList filter;         ///<Current-state record
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   IgmpHostSrcAddrList allow;      ///<ALLOW group record
   IgmpHostSrcAddrList block;      ///<BLOCK group record
   Ipv4SrcAddrList queriedSources; ///<List of sources to be reported
#endif
} IgmpHostGroup;


/**
 * @brief IGMP host context
 **/

typedef struct
{
   NetInterface *interface;                          ///<Underlying network interface
   IgmpVersion compatibilityMode;                    ///<Host compatibility mode
   NetTimer igmpv1QuerierPresentTimer;               ///<IGMPv1 querier present timer
   NetTimer igmpv2QuerierPresentTimer;               ///<IGMPv2 querier present timer
   NetTimer generalQueryTimer;                       ///<Timer for scheduling responses to general queries
   NetTimer stateChangeReportTimer;                  ///<Retransmission timer for state-change reports
   IgmpHostGroup groups[IPV4_MULTICAST_FILTER_SIZE]; ///<Multicast groups
} IgmpHostContext;


//IGMP host related functions
error_t igmpHostInit(NetInterface *interface);
void igmpHostTick(IgmpHostContext *context);

void igmpHostStateChangeEvent(IgmpHostContext *context, Ipv4Addr groupAddr,
   IpFilterMode newFilterMode, const Ipv4SrcAddrList *newFilter);

void igmpHostLinkChangeEvent(IgmpHostContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
