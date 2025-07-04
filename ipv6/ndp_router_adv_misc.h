/**
 * @file ndp_router_adv_misc.h
 * @brief Helper functions for router advertisement service
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

#ifndef _NDP_ROUTER_ADV_MISC_H
#define _NDP_ROUTER_ADV_MISC_H

//Dependencies
#include "core/net.h"
#include "ipv6/ndp_router_adv.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//Tick counter to handle periodic operations
extern systime_t ndpRouterAdvTickCounter;

//RA service related functions
void ndpRouterAdvTick(NdpRouterAdvContext *context);
void ndpRouterAdvLinkChangeEvent(NdpRouterAdvContext *context);

void ndpProcessRouterSol(NetInterface *interface,
   const Ipv6PseudoHeader *pseudoHeader, const NetBuffer *buffer,
   size_t offset, const NetRxAncillary *ancillary);

error_t ndpSendRouterAdv(NdpRouterAdvContext *context, uint16_t routerLifetime);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
