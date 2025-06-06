/**
 * @file arp_cache.h
 * @brief ARP cache management
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

#ifndef _ARP_CACHE_H
#define _ARP_CACHE_H

//Dependencies
#include "core/net.h"
#include "ipv4/arp.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//ARP related functions
void arpChangeState(ArpCacheEntry *entry, ArpState newState);

ArpCacheEntry *arpCreateEntry(NetInterface *interface);
ArpCacheEntry *arpFindEntry(NetInterface *interface, Ipv4Addr ipAddr);

void arpFlushCache(NetInterface *interface);

void arpSendQueuedPackets(NetInterface *interface, ArpCacheEntry *entry);
void arpFlushQueuedPackets(NetInterface *interface, ArpCacheEntry *entry);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
