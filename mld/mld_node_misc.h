/**
 * @file mld_node_misc.h
 * @brief Helper functions for MLD node
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

#ifndef _MLD_NODE_MISC_H
#define _MLD_NODE_MISC_H

//Dependencies
#include "core/net.h"
#include "mld/mld_node.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//MLD node related functions
void mldNodeChangeCompatibilityMode(MldNodeContext *context,
   MldVersion compatibilityMode);

void mldNodeSendUnsolicitedReport(MldNodeContext *context,
   const Ipv6Addr *groupAddr);

void mldNodeSendListenerReport(MldNodeContext *context,
   const Ipv6Addr *groupAddr);

void mldNodeSendListenerDone(MldNodeContext *context,
   const Ipv6Addr *groupAddr);

void mldNodeSendCurrentStateReport(MldNodeContext *context,
   const Ipv6Addr *groupAddr);

void mldNodeSendStateChangeReport(MldNodeContext *context,
   const Ipv6Addr *groupAddr);

void mldNodeFlushReportRecords(MldNodeContext *context, NetBuffer *buffer,
   size_t offset, size_t *length);

void mldNodeProcessMessage(MldNodeContext *context,
   const Ipv6PseudoHeader *pseudoHeader, const MldMessage *message,
   size_t length);

void mldNodeProcessListenerQuery(MldNodeContext *context,
   const Ipv6PseudoHeader *pseudoHeader, const MldMessage *message,
   size_t length);

void mldNodeProcessListenerQueryV2(MldNodeContext *context,
   const Ipv6PseudoHeader *pseudoHeader, const MldListenerQueryV2 *message,
   size_t length);

void mldNodeProcessListenerReport(MldNodeContext *context,
   const Ipv6PseudoHeader *pseudoHeader, const MldMessage *message,
   size_t length);

void mldNodeMergeReports(MldNodeGroup *group,
   IpFilterMode newFilterMode, const Ipv6SrcAddrList *newFilter);

bool_t mldNodeGetRetransmitStatus(MldNodeContext *context);
bool_t mldNodeGetGroupRetransmitStatus(MldNodeGroup *group);
void mldNodeDecGroupRetransmitCounters(MldNodeGroup *group);

MldNodeGroup *mldNodeCreateGroup(MldNodeContext *context,
   const Ipv6Addr *groupAddr);

MldNodeGroup *mldNodeFindGroup(MldNodeContext *context,
   const Ipv6Addr *groupAddr);

bool_t mldNodeMatchGroup(MldNodeGroup *group, const Ipv6Addr *multicastAddr);
void mldNodeDeleteGroup(MldNodeGroup *group);
void mldNodeFlushUnusedGroups(MldNodeContext *context);

error_t mldNodeAddSrcAddr(MldNodeSrcAddrList *list, const Ipv6Addr *srcAddr);
void mldNodeRemoveSrcAddr(MldNodeSrcAddrList *list, const Ipv6Addr *srcAddr);

int_t mldNodeFindSrcAddr(const MldNodeSrcAddrList *list,
   const Ipv6Addr *srcAddr);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
