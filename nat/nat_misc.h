/**
 * @file nat_misc.h
 * @brief Helper functions for NAT
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

#ifndef _NAT_MISC_H
#define _NAT_MISC_H

//Dependencies
#include "core/net.h"
#include "nat/nat.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//Tick counter to handle periodic operations
extern systime_t natTickCounter;

//NAT related functions
void natTick(NatContext *context);

bool_t natIsPublicInterface(NatContext *context, NetInterface *interface);
bool_t natIsPrivateInterface(NatContext *context, NetInterface *interface);

error_t natProcessPacket(NatContext *context, NetInterface *inInterface,
   const Ipv4PseudoHeader *inPseudoHeader, const NetBuffer *inBuffer,
   size_t inOffset, NetRxAncillary *ancillary);

error_t natTranslateInboundPacket(NatContext *context, NatIpPacket *packet);
error_t natTranslateOutboundPacket(NatContext *context, NatIpPacket *packet);

error_t natForwardPacket(NatContext *context, const NatIpPacket *packet);

NatPortFwdRule *natMatchPortFwdRule(NatContext *context,
   const NatIpPacket *packet);

NatSession *natMatchSession(NatContext *context, const NatIpPacket *packet);

NatSession *natCreateSession(NatContext *context);

uint16_t natAllocatePort(NatContext *context);
uint16_t natAllocateIcmpQueryId(NatContext *context);

error_t natParseTransportHeader(NatIpPacket *packet);

error_t natTranslateTransportHeader(const NatIpPacket *packet,
   const Ipv4PseudoHeader *pseudoHeader, const NetBuffer *buffer,
   size_t offset);

void natDumpPacket(const NatIpPacket *packet);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
