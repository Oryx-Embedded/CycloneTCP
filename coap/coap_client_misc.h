/**
 * @file coap_client_misc.h
 * @brief Helper functions for CoAP client
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

#ifndef _COAP_CLIENT_MISC_H
#define _COAP_CLIENT_MISC_H

//Dependencies
#include "core/net.h"
#include "coap/coap_client.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//CoAP client related functions
error_t coapClientProcessEvents(CoapClientContext *context, systime_t timeout);
error_t coapClientProcessRequestEvents(CoapClientRequest *request);

error_t coapClientChangeRequestState(CoapClientRequest *request,
   CoapRequestState newState);

error_t coapClientMatchResponse(const CoapClientRequest *request,
   const CoapMessage *response);

error_t coapClientProcessResponse(CoapClientRequest *request,
   const CoapMessage *response);

error_t coapClientRejectResponse(CoapClientContext *context,
   const CoapMessage *response);

error_t coapClientSendAck(CoapClientContext *context, uint16_t mid);
error_t coapClientSendReset(CoapClientContext *context, uint16_t mid);

void coapClientGenerateMessageId(CoapClientContext *context,
   CoapMessageHeader *header);

void coapClientGenerateToken(CoapClientContext *context,
   CoapMessageHeader *header);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
