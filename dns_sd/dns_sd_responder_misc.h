/**
 * @file dns_sd_responder_misc.h
 * @brief Helper functions for DNS-SD responder
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

#ifndef _DNS_SD_RESPONDER_MISC_H
#define _DNS_SD_RESPONDER_MISC_H

//Dependencies
#include "core/net.h"
#include "dns_sd/dns_sd_responder.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//DNS-SD responder related functions
void dnsSdResponderChangeState(DnsSdResponderService *service,
   MdnsState newState, systime_t delay);

void dnsSdResponderChangeInstanceName(DnsSdResponderService *service);

error_t dnsSdResponderSendProbe(DnsSdResponderService *service);
error_t dnsSdResponderSendAnnouncement(DnsSdResponderService *service);
error_t dnsSdResponderSendGoodbye(DnsSdResponderService *service);

error_t dnsSdResponderParseQuestion(NetInterface *interface,
   const MdnsMessage *query, size_t offset, const DnsQuestion *question,
   MdnsMessage *response);

void dnsSdResponderParseNsRecords(NetInterface *interface,
   const MdnsMessage *query, size_t offset);

void dnsSdResponderParseAnRecord(NetInterface *interface,
   const MdnsMessage *response, size_t offset, const DnsResourceRecord *record);

void dnsSdResponderGenerateAdditionalRecords(NetInterface *interface,
   MdnsMessage *response, bool_t legacyUnicast);

error_t dnsSdResponderFormatServiceEnumPtrRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service, uint32_t ttl);

error_t dnsSdResponderFormatPtrRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service, uint32_t ttl);

error_t dnsSdResponderFormatSrvRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service,
   bool_t cacheFlush, uint32_t ttl);

error_t dnsSdResponderFormatTxtRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service,
   bool_t cacheFlush, uint32_t ttl);

error_t dnsSdResponderFormatNsecRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service,
   bool_t cacheFlush, uint32_t ttl);

DnsResourceRecord *dnsSdResponderGetNextTiebreakerRecord(
   DnsSdResponderService *service, const MdnsMessage *query, size_t offset,
   DnsResourceRecord *record);

int_t dnsSdResponderCompareSrvRecord(NetInterface *interface,
   DnsSdResponderService *service, const MdnsMessage *message,
   const DnsResourceRecord *record);

int_t dnsSdResponderCompareTxtRecord(DnsSdResponderService *service,
   const MdnsMessage *message, const DnsResourceRecord *record);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
