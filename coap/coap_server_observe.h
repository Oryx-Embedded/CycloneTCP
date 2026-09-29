/**
 * @file coap_server_observe.h
 * @brief CoAP observe
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
 * @version 2.6.6
 **/

#ifndef _COAP_SERVER_OBSERVE_H
#define _COAP_SERVER_OBSERVE_H

//Dependencies
#include "core/net.h"
#include "coap/coap_server.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//CoAP server related functions
void coapServerProcessObserveEvents(CoapServerContext *context);
error_t coapServerProcessRegistrationRequest(CoapServerContext *context);

void coapServerRestoreRegistrationRequest(CoapServerContext *context,
   CoapObserver *observer);

void coapServerInitNotificationResponse(CoapServerContext *context,
   CoapObserver *observer);

error_t coapServerSendNotificationResponse(CoapServerContext *context,
   CoapObserver *observer);

error_t coapServerProcessAck(CoapServerContext *context);
error_t coapServerProcessReset(CoapServerContext *context);

CoapResource *coapServerFindResource(CoapServerContext *context,
   const char_t *uri);

CoapObserver *coapServerCreateObserver(CoapServerContext *context,
   const uint8_t *token, size_t tokenLen, CoapResource *resource);

CoapObserver *coapServerFindObserver(CoapServerContext *context,
   const uint8_t *token, size_t tokenLen);

void coapServerDeleteObserver(CoapObserver *observer);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
