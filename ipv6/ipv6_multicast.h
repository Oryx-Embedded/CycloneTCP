/**
 * @file ipv6_multicast.h
 * @brief IPv6 multicast filtering
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

#ifndef _IPV6_MULTICAST_H
#define _IPV6_MULTICAST_H

//Dependencies
#include "core/net.h"
#include "ipv6/ipv6.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//IPv6 multicast related functions
error_t ipv6MulticastFilter(NetInterface *interface, const Ipv6Addr *destAddr,
   const Ipv6Addr *srcAddr);

error_t ipv6JoinMulticastGroup(NetInterface *interface,
   const Ipv6Addr *groupAddr);

error_t ipv6LeaveMulticastGroup(NetInterface *interface,
   const Ipv6Addr *groupAddr);

void ipv6UpdateMulticastFilter(NetInterface *interface,
   const Ipv6Addr *groupAddr);

void ipv6DeriveInterfaceState(Ipv6FilterEntry *entry,
   SocketMulticastGroup *group);

error_t ipv6AcceptMulticastAddr(NetInterface *interface,
   const Ipv6Addr *groupAddr);

void ipv6DropMulticastAddr(NetInterface *interface,
   const Ipv6Addr *groupAddr);

error_t ipv6MapMulticastAddrToMac(const Ipv6Addr *ipAddr, MacAddr *macAddr);

Ipv6FilterEntry *ipv6CreateMulticastFilterEntry(NetInterface *interface,
   const Ipv6Addr *multicastAddr);

Ipv6FilterEntry *ipv6FindMulticastFilterEntry(NetInterface *interface,
   const Ipv6Addr *multicastAddr);

void ipv6DeleteMulticastFilterEntry(Ipv6FilterEntry *entry);

error_t ipv6AddSrcAddr(Ipv6SrcAddrList *list, const Ipv6Addr *srcAddr);
void ipv6RemoveSrcAddr(Ipv6SrcAddrList *list, const Ipv6Addr *srcAddr);
int_t ipv6FindSrcAddr(const Ipv6SrcAddrList *list, const Ipv6Addr *srcAddr);

bool_t ipv6CompareSrcAddrLists(const Ipv6SrcAddrList *list1,
   const Ipv6SrcAddrList *list2);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
