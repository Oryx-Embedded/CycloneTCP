/**
 * @file igmp_common.h
 * @brief Definitions common to IGMP host, router and snooping switch
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

#ifndef _IGMP_COMMON_H
#define _IGMP_COMMON_H

//Dependencies
#include "core/net.h"

//IGMP tick interval
#ifndef IGMP_TICK_INTERVAL
   #define IGMP_TICK_INTERVAL 200
#elif (IGMP_TICK_INTERVAL < 10)
   #error IGMP_TICK_INTERVAL parameter is not valid
#endif

//Robustness Variable
#ifndef IGMP_ROBUSTNESS_VARIABLE
   #define IGMP_ROBUSTNESS_VARIABLE 2
#elif (IGMP_ROBUSTNESS_VARIABLE < 1)
   #error IGMP_ROBUSTNESS_VARIABLE parameter is not valid
#endif

//Query Interval
#ifndef IGMP_QUERY_INTERVAL
   #define IGMP_QUERY_INTERVAL 125000
#elif (IGMP_QUERY_INTERVAL < 1000)
   #error IGMP_QUERY_INTERVAL parameter is not valid
#endif

//Query Response Interval
#ifndef IGMP_QUERY_RESPONSE_INTERVAL
   #define IGMP_QUERY_RESPONSE_INTERVAL 10000
#elif (IGMP_QUERY_RESPONSE_INTERVAL < 1000 || IGMP_QUERY_RESPONSE_INTERVAL > IGMP_QUERY_INTERVAL)
   #error IGMP_QUERY_RESPONSE_INTERVAL parameter is not valid
#endif

//Group Membership Interval
#define IGMP_GROUP_MEMBERSHIP_INTERVAL ((IGMP_ROBUSTNESS_VARIABLE * \
   IGMP_QUERY_INTERVAL) + IGMP_QUERY_RESPONSE_INTERVAL)

//Other Querier Present Interval
#define IGMP_OTHER_QUERIER_PRESENT_INTERVAL ((IGMP_ROBUSTNESS_VARIABLE * \
   IGMP_QUERY_INTERVAL) + (IGMP_QUERY_RESPONSE_INTERVAL / 2))

//Startup Query Interval
#ifndef IGMP_STARTUP_QUERY_INTERVAL
   #define IGMP_STARTUP_QUERY_INTERVAL (IGMP_QUERY_INTERVAL / 4)
#elif (IGMP_STARTUP_QUERY_INTERVAL < 1000)
   #error IGMP_STARTUP_QUERY_INTERVAL parameter is not valid
#endif

//Startup Query Count
#ifndef IGMP_STARTUP_QUERY_COUNT
   #define IGMP_STARTUP_QUERY_COUNT IGMP_ROBUSTNESS_VARIABLE
#elif (IGMP_STARTUP_QUERY_COUNT < 1)
   #error IGMP_STARTUP_QUERY_COUNT parameter is not valid
#endif

//Last Member Query Interval
#ifndef IGMP_LAST_MEMBER_QUERY_INTERVAL
   #define IGMP_LAST_MEMBER_QUERY_INTERVAL 1000
#elif (IGMP_LAST_MEMBER_QUERY_INTERVAL < 100)
   #error IGMP_LAST_MEMBER_QUERY_INTERVAL parameter is not valid
#endif

//Last Member Query Count
#ifndef IGMP_LAST_MEMBER_QUERY_COUNT
   #define IGMP_LAST_MEMBER_QUERY_COUNT IGMP_ROBUSTNESS_VARIABLE
#elif (IGMP_LAST_MEMBER_QUERY_COUNT < 1)
   #error IGMP_LAST_MEMBER_QUERY_COUNT parameter is not valid
#endif

//Last Member Query Time
#define IGMP_LAST_MEMBER_QUERY_TIME (IGMP_LAST_MEMBER_QUERY_COUNT * \
   IGMP_LAST_MEMBER_QUERY_INTERVAL)

//Unsolicited Report Interval
#ifndef IGMP_UNSOLICITED_REPORT_INTERVAL
   #define IGMP_UNSOLICITED_REPORT_INTERVAL 10000
#elif (IGMP_UNSOLICITED_REPORT_INTERVAL < 1000)
   #error IGMP_UNSOLICITED_REPORT_INTERVAL parameter is not valid
#endif

//Older Version Querier Present Timeout
#ifndef IGMP_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT
   #define IGMP_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT 400000
#elif (IGMP_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT < 1000)
   #error IGMP_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT parameter is not valid
#endif

//Maximum response time for queries (IGMPv1)
#ifndef IGMP_V1_MAX_RESPONSE_TIME
   #define IGMP_V1_MAX_RESPONSE_TIME 10000
#elif (IGMP_V1_MAX_RESPONSE_TIME < 1000)
   #error IGMP_V1_MAX_RESPONSE_TIME parameter is not valid
#endif

//Unsolicited Report Interval (IGMPv3)
#ifndef IGMP_V3_UNSOLICITED_REPORT_INTERVAL
   #define IGMP_V3_UNSOLICITED_REPORT_INTERVAL 1000
#elif (IGMP_V3_UNSOLICITED_REPORT_INTERVAL < 1000)
   #error IGMP_V3_UNSOLICITED_REPORT_INTERVAL parameter is not valid
#endif

//Maximum size of IGMP messages
#ifndef IGMP_MAX_MSG_SIZE
   #define IGMP_MAX_MSG_SIZE 1024
#elif (IGMP_MAX_MSG_SIZE < 1)
   #error IGMP_MAX_MSG_SIZE parameter is not valid
#endif

//TTL used by IGMP messages
#define IGMP_TTL 1

//All-Systems address
#define IGMP_ALL_SYSTEMS_ADDR IPV4_ADDR(224, 0, 0, 1)
//All-Routers address
#define IGMP_ALL_ROUTERS_ADDR IPV4_ADDR(224, 0, 0, 2)
//IGMPv3 All-Routers address
#define IGMP_V3_ALL_ROUTERS_ADDR IPV4_ADDR(224, 0, 0, 22)

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief IGMP versions
 **/

typedef enum
{
   IGMP_VERSION_1 = 1,
   IGMP_VERSION_2 = 2,
   IGMP_VERSION_3 = 3
} IgmpVersion;


/**
 * @brief IGMP message types
 **/

typedef enum
{
   IGMP_TYPE_MEMBERSHIP_QUERY     = 0x11,
   IGMP_TYPE_MEMBERSHIP_REPORT_V1 = 0x12,
   IGMP_TYPE_MEMBERSHIP_REPORT_V2 = 0x16,
   IGMP_TYPE_LEAVE_GROUP          = 0x17,
   IGMP_TYPE_MEMBERSHIP_REPORT_V3 = 0x22
} IgmpType;


/**
 * @brief IGMPv3 group record types
 **/

typedef enum
{
   IGMP_GROUP_RECORD_TYPE_IS_IN = 1,
   IGMP_GROUP_RECORD_TYPE_IS_EX = 2,
   IGMP_GROUP_RECORD_TYPE_TO_IN = 3,
   IGMP_GROUP_RECORD_TYPE_TO_EX = 4,
   IGMP_GROUP_RECORD_TYPE_ALLOW = 5,
   IGMP_GROUP_RECORD_TYPE_BLOCK = 6
} IgmpGroupRecordType;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma pack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief IGMP message
 **/

typedef __packed_struct
{
   uint8_t type;        //0
   uint8_t maxRespTime; //1
   uint16_t checksum;   //2-3
   Ipv4Addr groupAddr;  //4-7
} IgmpMessage;


/**
 * @brief IGMPv3 Membership Query message
 **/

typedef __packed_struct
{
   uint8_t type;          //0
   uint8_t maxRespCode;   //1
   uint16_t checksum;     //2-3
   Ipv4Addr groupAddr;    //4-7
#if defined(_CPU_BIG_ENDIAN) && !defined(__ICCRX__)
   uint8_t flags : 4;     //8
   uint8_t s : 1;
   uint8_t qrv : 3;
#else
   uint8_t qrv : 3;       //8
   uint8_t s : 1;
   uint8_t flags : 4;
#endif
   uint8_t qqic;          //9
   uint16_t numOfSources; //10-11
   Ipv4Addr srcAddr[];    //12
} IgmpMembershipQueryV3;


/**
 * @brief IGMPv3 Membership Report message
 **/

typedef __packed_struct
{
   uint8_t type;               //0
   uint8_t reserved;          //1
   uint16_t checksum;          //2-3
   uint16_t flags;             //4-5
   uint16_t numOfGroupRecords; //6-7
   uint8_t groupRecords[];     //8
} IgmpMembershipReportV3;


/**
 * @brief IGMPv3 group record
 **/

typedef __packed_struct
{
   uint8_t recordType;     //0
   uint8_t auxDataLen;     //1
   uint16_t numOfSources;  //2-3
   Ipv4Addr multicastAddr; //4-7
   Ipv4Addr srcAddr[];     //8
} IgmpGroupRecord;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma unpack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif

//Tick counter to handle periodic operations
extern systime_t igmpTickCounter;

//IGMP related functions
error_t igmpInit(NetInterface *interface);
void igmpTick(NetInterface *interface);
void igmpLinkChangeEvent(NetInterface *interface);

error_t igmpSendMessage(NetInterface *interface, Ipv4Addr destAddr,
   NetBuffer *buffer, size_t offset);

void igmpProcessMessage(NetInterface *interface,
   const Ipv4PseudoHeader *pseudoHeader, const NetBuffer *buffer,
   size_t offset, const NetRxAncillary *ancillary);

systime_t igmpGetRandomDelay(systime_t maxDelay);

uint32_t igmpDecodeFloatingPointValue(uint8_t code);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
