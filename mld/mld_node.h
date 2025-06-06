/**
 * @file mld_node.h
 * @brief MLD node (Multicast Listener Discovery for IPv6)
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

#ifndef _MLD_NODE_H
#define _MLD_NODE_H

//Dependencies
#include "core/net.h"
#include "mld/mld_common.h"

//MLD node support
#ifndef MLD_NODE_SUPPORT
   #define MLD_NODE_SUPPORT DISABLED
#elif (MLD_NODE_SUPPORT != ENABLED && MLD_NODE_SUPPORT != DISABLED)
   #error MLD_NODE_SUPPORT parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief MLD node group states
 **/

typedef enum
{
   MLD_NODE_GROUP_STATE_NON_LISTENER      = 0,
   MLD_NODE_GROUP_STATE_INIT_LISTENER     = 1,
   MLD_NODE_GROUP_STATE_DELAYING_LISTENER = 2,
   MLD_NODE_GROUP_STATE_IDLE_LISTENER     = 3
} MldNodeGroupState;


/**
 * @brief Source address
 **/

typedef struct
{
   Ipv6Addr addr;          ///<Source address
   uint_t retransmitCount; ///<Retransmission counter
} MldNodeSrcAddr;


/**
 * @brief Source address list
 **/

typedef struct
{
   uint_t numSources;                                  ///<Number of source address
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   MldNodeSrcAddr sources[IPV6_MAX_MULTICAST_SOURCES]; ///<List of source addresses
#endif
} MldNodeSrcAddrList;


/**
 * @brief Multicast group
 **/

typedef struct
{
   MldNodeGroupState state;        ///<Multicast group state
   Ipv6Addr addr;                  ///<Multicast group address
   bool_t flag;                    ///<We are the last host to send a report for this group
   uint_t retransmitCount;         ///<Filter mode retransmission counter
   NetTimer timer;                 ///<Report delay timer
   IpFilterMode filterMode;        ///<Filter mode
   Ipv6SrcAddrList filter;         ///<Current-state record
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   MldNodeSrcAddrList allow;       ///<ALLOW group record
   MldNodeSrcAddrList block;       ///<BLOCK group record
   Ipv6SrcAddrList queriedSources; ///<List of sources to be reported
#endif
} MldNodeGroup;


/**
 * @brief MLD node context
 **/

typedef struct
{
   NetInterface *interface;                         ///<Underlying network interface
   MldVersion compatibilityMode;                    ///<Host compatibility mode
   NetTimer olderVersionQuerierPresentTimer;        ///<Older version querier present timer
   NetTimer generalQueryTimer;                      ///<Timer for scheduling responses to general queries
   NetTimer stateChangeReportTimer;                 ///<Retransmission timer for state-change reports
   MldNodeGroup groups[IPV6_MULTICAST_FILTER_SIZE]; ///<Multicast groups
} MldNodeContext;


//MLD node related functions
error_t mldNodeInit(NetInterface *interface);
void mldNodeTick(MldNodeContext *context);

void mldNodeStateChangeEvent(MldNodeContext *context, const Ipv6Addr *groupAddr,
   IpFilterMode newFilterMode, const Ipv6SrcAddrList *newFilter);

void mldNodeLinkChangeEvent(MldNodeContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
