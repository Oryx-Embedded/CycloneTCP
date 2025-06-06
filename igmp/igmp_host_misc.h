/**
 * @file igmp_host_misc.h
 * @brief Helper functions for IGMP host
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

#ifndef _IGMP_HOST_MISC_H
#define _IGMP_HOST_MISC_H

//Dependencies
#include "core/net.h"
#include "igmp/igmp_host.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//IGMP host related functions
void igmpHostChangeCompatibilityMode(IgmpHostContext *context,
   IgmpVersion compatibilityMode);

void igmpHostSendMembershipReport(IgmpHostContext *context, Ipv4Addr groupAddr);
void igmpHostSendLeaveGroup(IgmpHostContext *context, Ipv4Addr groupAddr);

void igmpHostSendCurrentStateReport(IgmpHostContext *context,
   Ipv4Addr groupAddr);

void igmpHostSendStateChangeReport(IgmpHostContext *context);

void igmpHostFlushReportRecords(IgmpHostContext *context, NetBuffer *buffer,
   size_t offset, size_t *length);

void igmpHostProcessMessage(IgmpHostContext *context,
   const Ipv4PseudoHeader *pseudoHeader, const IgmpMessage *message,
   size_t length);

void igmpHostProcessMembershipQuery(IgmpHostContext *context,
   const Ipv4PseudoHeader *pseudoHeader, const IgmpMessage *message,
   size_t length);

void igmpHostProcessMembershipQueryV3(IgmpHostContext *context,
   const Ipv4PseudoHeader *pseudoHeader, const IgmpMembershipQueryV3 *message,
   size_t length);

void igmpHostProcessMembershipReport(IgmpHostContext *context,
   const Ipv4PseudoHeader *pseudoHeader, const IgmpMessage *message,
   size_t length);

void igmpHostMergeReports(IgmpHostGroup *group,
   IpFilterMode newFilterMode, const Ipv4SrcAddrList *newFilter);

bool_t igmpHostGetRetransmitStatus(IgmpHostContext *context);
bool_t igmpHostGetGroupRetransmitStatus(IgmpHostGroup *group);
void igmpHostDecGroupRetransmitCounters(IgmpHostGroup *group);

IgmpHostGroup *igmpHostCreateGroup(IgmpHostContext *context,
   Ipv4Addr groupAddr);

IgmpHostGroup *igmpHostFindGroup(IgmpHostContext *context, Ipv4Addr groupAddr);
bool_t igmpHostMatchGroup(IgmpHostGroup *group, Ipv4Addr multicastAddr);
void igmpHostDeleteGroup(IgmpHostGroup *group);
void igmpHostFlushUnusedGroups(IgmpHostContext *context);

error_t igmpHostAddSrcAddr(IgmpHostSrcAddrList *list, Ipv4Addr srcAddr);
void igmpHostRemoveSrcAddr(IgmpHostSrcAddrList *list, Ipv4Addr srcAddr);
int_t igmpHostFindSrcAddr(const IgmpHostSrcAddrList *list, Ipv4Addr srcAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
