/**
 * @file ipv4_multicast.h
 * @brief IPv4 multicast filtering
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

#ifndef _IPV4_MULTICAST_H
#define _IPV4_MULTICAST_H

//Dependencies
#include "core/net.h"
#include "ipv4/ipv4.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//IPv4 multicast related functions
error_t ipv4MulticastFilter(NetInterface *interface, Ipv4Addr destAddr,
   Ipv4Addr srcAddr);

error_t ipv4JoinMulticastGroup(NetInterface *interface, Ipv4Addr groupAddr);
error_t ipv4LeaveMulticastGroup(NetInterface *interface, Ipv4Addr groupAddr);

void ipv4UpdateMulticastFilter(NetInterface *interface, Ipv4Addr groupAddr);

void ipv4DeriveInterfaceState(Ipv4FilterEntry *entry,
   SocketMulticastGroup *group);

error_t ipv4AcceptMulticastAddr(NetInterface *interface, Ipv4Addr groupAddr);
void ipv4DropMulticastAddr(NetInterface *interface, Ipv4Addr groupAddr);

error_t ipv4MapMulticastAddrToMac(Ipv4Addr ipAddr, MacAddr *macAddr);

Ipv4FilterEntry *ipv4CreateMulticastFilterEntry(NetInterface *interface,
   Ipv4Addr multicastAddr);

Ipv4FilterEntry *ipv4FindMulticastFilterEntry(NetInterface *interface,
   Ipv4Addr multicastAddr);

void ipv4DeleteMulticastFilterEntry(Ipv4FilterEntry *entry);

error_t ipv4AddSrcAddr(Ipv4SrcAddrList *list, Ipv4Addr srcAddr);
void ipv4RemoveSrcAddr(Ipv4SrcAddrList *list, Ipv4Addr srcAddr);
int_t ipv4FindSrcAddr(const Ipv4SrcAddrList *list, Ipv4Addr srcAddr);

bool_t ipv4CompareSrcAddrLists(const Ipv4SrcAddrList *list1,
   const Ipv4SrcAddrList *list2);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
