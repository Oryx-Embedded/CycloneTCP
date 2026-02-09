/**
 * @file ip.h
 * @brief IPv4 and IPv6 common routines
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
 * @version 2.6.0
 **/

#ifndef _IP_H
#define _IP_H

//Dependencies
#include "ipv4/ipv4.h"
#include "ipv6/ipv6.h"

//Default value for DF flag
#ifndef IP_DEFAULT_DF
   #define IP_DEFAULT_DF FALSE
#elif (IP_DEFAULT_DF != FALSE && IP_DEFAULT_DF != TRUE)
   #error IP_DEFAULT_DF parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief IP supported protocols
 **/

typedef enum
{
   IP_PROTOCOL_TCP  = 6,
   IP_PROTOCOL_UDP  = 17
} IpProtocol;


/**
 * @brief Multicast filter mode
 **/

typedef enum
{
   IP_FILTER_MODE_EXCLUDE = 0,
   IP_FILTER_MODE_INCLUDE = 1
} IpFilterMode;


/**
 * @brief Flags used by I/O functions
 **/

typedef enum
{
   IP_FLAG_DONT_ROUTE = 0x0400,
   IP_FLAG_TTL        = 0x00FF,
   IP_FLAG_HOP_LIMIT  = 0x00FF
} IpFlags;


/**
 * @brief IP network address
 **/

typedef struct
{
   size_t length;
   union
   {
#if (IPV4_SUPPORT == ENABLED)
      Ipv4Addr ipv4Addr;
#endif
#if (IPV6_SUPPORT == ENABLED)
      Ipv6Addr ipv6Addr;
#endif
      uint8_t addr[4];
   };
} IpAddr;


/**
 * @brief IP pseudo header
 **/

typedef struct
{
   size_t length;
   union
   {
#if (IPV4_SUPPORT == ENABLED)
      Ipv4PseudoHeader ipv4Data;
#endif
#if (IPV6_SUPPORT == ENABLED)
      Ipv6PseudoHeader ipv6Data;
#endif
      uint8_t data[4];
   };
} IpPseudoHeader;


/**
 * @brief System-wide IP statistics
 **/

typedef struct
{
   uint64_t inReceives;
   uint64_t inOctets;
   uint32_t inHdrErrors;
   uint32_t inNoRoutes;
   uint32_t inAddrErrors;
   uint32_t inUnknownProtos;
   uint32_t inTruncatedPkts;
   uint64_t inForwDatagrams;
   uint32_t reasmReqds;
   uint32_t reasmOKs;
   uint32_t reasmFails;
   uint32_t inDiscards;
   uint64_t inDelivers;
   uint64_t outRequests;
   uint32_t outNoRoutes;
   uint64_t outForwDatagrams;
   uint32_t outDiscards;
   uint32_t outFragReqds;
   uint32_t outFragOKs;
   uint32_t outFragFails;
   uint32_t outFragCreates;
   uint64_t outTransmits;
   uint64_t outOctets;
   uint64_t inMcastPkts;
   uint64_t inMcastOctets;
   uint64_t outMcastPkts;
   uint64_t outMcastOctets;
   uint64_t inBcastPkts;
   uint64_t outBcastPkts;
   uint32_t discontinuityTime;
   uint32_t refreshRate;
} IpSystemStats;


/**
 * @brief Per-interface IP statistics
 **/

typedef struct
{
   uint64_t inReceives;
   uint64_t inOctets;
   uint32_t inHdrErrors;
   uint32_t inNoRoutes;
   uint32_t inAddrErrors;
   uint32_t inUnknownProtos;
   uint32_t inTruncatedPkts;
   uint64_t inForwDatagrams;
   uint32_t reasmReqds;
   uint32_t reasmOKs;
   uint32_t reasmFails;
   uint32_t inDiscards;
   uint64_t inDelivers;
   uint64_t outRequests;
   uint64_t outForwDatagrams;
   uint32_t outDiscards;
   uint32_t outFragReqds;
   uint32_t outFragOKs;
   uint32_t outFragFails;
   uint32_t outFragCreates;
   uint64_t outTransmits;
   uint64_t outOctets;
   uint64_t inMcastPkts;
   uint64_t inMcastOctets;
   uint64_t outMcastPkts;
   uint64_t outMcastOctets;
   uint64_t inBcastPkts;
   uint64_t outBcastPkts;
   uint32_t discontinuityTime;
   uint32_t refreshRate;
} IpIfStats;


/**
 * @brief ICMP statistics
 **/

typedef struct
{
   uint32_t inMsgs;
   uint32_t inErrors;
   uint32_t outMsgs;
   uint32_t outErrors;
   uint32_t inPkts[256];
   uint32_t outPkts[256];
} IcmpStats;


//IP related constants
extern const IpAddr IP_ADDR_ANY;
extern const IpAddr IP_ADDR_UNSPECIFIED;

//IP related functions
error_t ipSendDatagram(NetInterface *interface,
   const IpPseudoHeader *pseudoHeader, NetBuffer *buffer, size_t offset,
   NetTxAncillary *ancillary);

error_t ipSelectSourceAddr(NetContext *context, NetInterface **interface,
   const IpAddr *destAddr, IpAddr *srcAddr);

bool_t ipIsUnspecifiedAddr(const IpAddr *ipAddr);
bool_t ipIsLinkLocalAddr(const IpAddr *ipAddr);
bool_t ipIsMulticastAddr(const IpAddr *ipAddr);
bool_t ipIsBroadcastAddr(const IpAddr *ipAddr);

bool_t ipCompAddr(const IpAddr *ipAddr1, const IpAddr *ipAddr2);

bool_t ipCompPrefix(const IpAddr *ipAddr1, const IpAddr *ipAddr2,
   size_t length);

void ipUpdateMulticastFilter(NetContext *context, NetInterface *interface,
   const IpAddr *groupAddr);

uint16_t ipCalcChecksum(const void *data, size_t length);
uint16_t ipCalcChecksumEx(const NetBuffer *buffer, size_t offset, size_t length);

uint16_t ipCalcUpperLayerChecksum(const void *pseudoHeader,
   size_t pseudoHeaderLen, const void *data, size_t dataLen);

uint16_t ipCalcUpperLayerChecksumEx(const void *pseudoHeader,
   size_t pseudoHeaderLen, const NetBuffer *buffer, size_t offset, size_t length);

NetBuffer *ipAllocBuffer(size_t length, size_t *offset);

error_t ipStringToAddr(const char_t *str, IpAddr *ipAddr);
char_t *ipAddrToString(const IpAddr *ipAddr, char_t *str);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
