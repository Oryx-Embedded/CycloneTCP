/**
 * @file nts_client_misc.h
 * @brief Helper functions for NTS client
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

#ifndef _NTS_CLIENT_MISC_H
#define _NTS_CLIENT_MISC_H

//Dependencies
#include "core/net.h"
#include "nts/nts_client.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//NTS client related functions
void ntsClientChangeState(NtsClientContext *context,
   NtsClientState newState);

error_t ntsClientOpenNtsKeConnection(NtsClientContext *context);
error_t ntsClientEstablishNtsKeConnection(NtsClientContext *context);
error_t ntsClientFormatNtsKeRequest(NtsClientContext *context);
error_t ntsClientSendNtsKeRequest(NtsClientContext *context);
error_t ntsClientReceiveNtsKeResponse(NtsClientContext *context);

error_t ntsClientParseEndOfMessageRecord(NtsClientContext *context,
   const uint8_t *body, size_t length);

error_t ntsClientParseNtsNextProtoNegoRecord(NtsClientContext *context,
   const uint8_t *body, size_t length);

error_t ntsClientParseErrorRecord(NtsClientContext *context,
   const uint8_t *body, size_t length);

error_t ntsClientParseWarningRecord(NtsClientContext *context,
   const uint8_t *body, size_t length);

error_t ntsClientParseAeadAlgoNegoRecord(NtsClientContext *context,
   const uint8_t *body, size_t length);

error_t ntsClientParseNewCookieForNtpv4Record(NtsClientContext *context,
   const uint8_t *body, size_t length);

error_t ntsClientParseNtpv4ServerRecord(NtsClientContext *context,
   const uint8_t *body, size_t length);

error_t ntsClientParseNtpv4PortRecord(NtsClientContext *context,
   const uint8_t *body, size_t length);

error_t ntsClientShutdownNtsKeConnection(NtsClientContext *context);
void ntsClientCloseNtsKeConnection(NtsClientContext *context);
error_t ntsClientCheckNtsKeTimeout(NtsClientContext *context);

error_t ntsClientOpenNtpConnection(NtsClientContext *context);
error_t ntsClientSendNtpRequest(NtsClientContext *context);
error_t ntsClientReceiveNtpResponse(NtsClientContext *context);

error_t ntsClientDecryptNtpResponse(NtsClientContext *context,
   const IpAddr *ipAddr, uint16_t port, const uint8_t *message,
   size_t length);

error_t ntsClientParseNtpResponse(NtsClientContext *context,
   NtpTimestamp *timestamp);

void ntsClientCloseNtpConnection(NtsClientContext *context);
error_t ntsClientCheckNtpTimeout(NtsClientContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
