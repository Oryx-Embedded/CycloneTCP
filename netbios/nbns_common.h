/**
 * @file nbns_common.h
 * @brief Definitions common to NBNS client and NBNS responder
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
 * @version 2.5.4
 **/

#ifndef _NBNS_COMMON_H
#define _NBNS_COMMON_H

//Dependencies
#include "core/net.h"
#include "dns/dns_common.h"

//Default resource record TTL (cache lifetime)
#ifndef NBNS_DEFAULT_RESOURCE_RECORD_TTL
   #define NBNS_DEFAULT_RESOURCE_RECORD_TTL 120
#elif (NBNS_DEFAULT_RESOURCE_RECORD_TTL < 1)
   #error NBNS_DEFAULT_RESOURCE_RECORD_TTL parameter is not valid
#endif

//NBNS port number
#define NBNS_PORT 137

//Macro definition
#define NBNS_ENCODE_H(c) ('A' + (((c) >> 4) & 0x0F))
#define NBNS_ENCODE_L(c) ('A' + ((c) & 0x0F))

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief NBNS flags
 **/

typedef enum
{
   NBNS_FLAG_ONT       = 0x6000, ///<Owner node type
   NBNS_FLAG_ONT_BNODE = 0x0000, ///<Owner node type (B node)
   NBNS_FLAG_ONT_PNODE = 0x2000, ///<Owner node type (P node)
   NBNS_FLAG_ONT_MNODE = 0x4000, ///<Owner node type (M node)
   NBNS_FLAG_G         = 0x8000  ///<Group name flag
} NbnsFlags;


/**
 * @brief NBNS name flags
 **/

typedef enum
{
   NBNS_NAME_FLAG_PRM       = 0x0200, ///<Permanent name flag
   NBNS_NAME_FLAG_ACT       = 0x0400, ///<Active name flag
   NBNS_NAME_FLAG_CNF       = 0x0800, ///<Conflict flag
   NBNS_NAME_FLAG_DRG       = 0x1000, ///<Deregister flag
   NBNS_NAME_FLAG_ONT       = 0x6000, ///<Owner node type
   NBNS_NAME_FLAG_ONT_BNODE = 0x0000, ///<Owner node type (B node)
   NBNS_NAME_FLAG_ONT_PNODE = 0x2000, ///<Owner node type (P node)
   NBNS_NAME_FLAG_ONT_MNODE = 0x4000, ///<Owner node type (M node)
   NBNS_NAME_FLAG_G         = 0x8000  ///<Group name flag
} NbnsNameFlags;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma pack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief NBNS message header
 **/

typedef __packed_struct
{
   uint16_t id;         //0-1
#if defined(_CPU_BIG_ENDIAN) && !defined(__ICCRX__)
   uint8_t qr : 1;      //2
   uint8_t opcode : 4;
   uint8_t aa : 1;
   uint8_t tc : 1;
   uint8_t rd : 1;
   uint8_t ra : 1;      //3
   uint8_t z : 2;
   uint8_t b : 1;
   uint8_t rcode : 4;
#else
   uint8_t rd : 1;      //2
   uint8_t tc : 1;
   uint8_t aa : 1;
   uint8_t opcode : 4;
   uint8_t qr : 1;
   uint8_t rcode : 4;   //3
   uint8_t b : 1;
   uint8_t z : 2;
   uint8_t ra : 1;
#endif
   uint16_t qdcount;    //4-5
   uint16_t ancount;    //6-7
   uint16_t nscount;    //8-9
   uint16_t arcount;    //10-11
   uint8_t questions[]; //12
} NbnsHeader;


/**
 * @brief Address entry
 **/

typedef __packed_struct
{
   uint16_t flags;
   Ipv4Addr addr;
} NbnsAddrEntry;


/**
 * @brief Node name entry
 **/

typedef __packed_struct
{
   uint8_t name[16];
   uint16_t flags;
} NbnsNodeNameEntry;


/**
 * @brief Node name array
 **/

typedef __packed_struct
{
   uint8_t numNames;
   NbnsNodeNameEntry names[];
} NbnsNodeNameArray;


/**
 * @brief Statistics
 **/

typedef __packed_struct
{
   MacAddr unitId;
   uint8_t jumpers;
   uint8_t testResult;
   uint16_t versionNumber;
   uint16_t periodOfStatistics;
   uint16_t numberOfCrcs;
   uint16_t numberAlignmentErrors;
   uint16_t numberOfCollisions;
   uint16_t numberSendAborts;
   uint32_t numberGoodSends;
   uint32_t numberGoodReceives;
   uint16_t numberRetransmits;
   uint16_t numberNoResourceConditions;
   uint16_t numberFreeCommandBlocks;
   uint16_t totalNumberCommandBlocks;
   uint16_t maxTotalNumberCommandBlocks;
   uint16_t numberPendingSessions;
   uint16_t maxNumberPendingSessions;
   uint16_t maxTotalSessionsPossible;
   uint16_t sessionDataPacketSize;
} NbnsStatistics;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma unpack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif

//NBNS related functions
error_t nbnsInit(NetInterface *interface);

void nbnsProcessMessage(NetInterface *interface,
   const IpPseudoHeader *pseudoHeader, const UdpHeader *udpHeader,
   const NetBuffer *buffer, size_t offset, const NetRxAncillary *ancillary,
   void *param);

size_t nbnsEncodeName(const char_t *src, uint8_t *dest);

size_t nbnsParseName(const NbnsHeader *message, size_t length, size_t pos,
   char_t *dest);

bool_t nbnsCompareName(const NbnsHeader *message, size_t length, size_t pos,
   const char_t *name);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
