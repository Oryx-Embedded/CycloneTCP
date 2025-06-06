/**
 * @file mld_common.h
 * @brief Definitions common to MLD node, router and snooping switch
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

#ifndef _MLD_COMMON_H
#define _MLD_COMMON_H

//Dependencies
#include "core/net.h"

//MLD tick interval
#ifndef MLD_TICK_INTERVAL
   #define MLD_TICK_INTERVAL 200
#elif (MLD_TICK_INTERVAL < 10)
   #error MLD_TICK_INTERVAL parameter is not valid
#endif

//Robustness Variable
#ifndef MLD_ROBUSTNESS_VARIABLE
   #define MLD_ROBUSTNESS_VARIABLE 2
#elif (MLD_ROBUSTNESS_VARIABLE < 1)
   #error MLD_ROBUSTNESS_VARIABLE parameter is not valid
#endif

//Unsolicited report interval
#ifndef MLD_UNSOLICITED_REPORT_INTERVAL
   #define MLD_UNSOLICITED_REPORT_INTERVAL 10000
#elif (MLD_UNSOLICITED_REPORT_INTERVAL < 1000)
   #error MLD_UNSOLICITED_REPORT_INTERVAL parameter is not valid
#endif

//Older Version Querier Present Timeout
#ifndef MLD_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT
   #define MLD_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT 400000
#elif (MLD_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT < 1000)
   #error MLD_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT parameter is not valid
#endif

//Unsolicited Report Interval (MLDv2)
#ifndef MLD_V2_UNSOLICITED_REPORT_INTERVAL
   #define MLD_V2_UNSOLICITED_REPORT_INTERVAL 1000
#elif (MLD_V2_UNSOLICITED_REPORT_INTERVAL < 1000)
   #error MLD_V2_UNSOLICITED_REPORT_INTERVAL parameter is not valid
#endif

//Maximum size of MLD messages
#ifndef MLD_MAX_MSG_SIZE
   #define MLD_MAX_MSG_SIZE 1024
#elif (MLD_MAX_MSG_SIZE < 1)
   #error MLD_MAX_MSG_SIZE parameter is not valid
#endif

//Hop Limit used by MLD messages
#define MLD_HOP_LIMIT 1

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief MLD versions
 **/

typedef enum
{
   MLD_VERSION_1 = 1,
   MLD_VERSION_2 = 2
} MldVersion;


/**
 * @brief MLDv2 multicast address record types
 **/

typedef enum
{
   MLD_MCAST_ADDR_RECORD_TYPE_IS_IN = 1,
   MLD_MCAST_ADDR_RECORD_TYPE_IS_EX = 2,
   MLD_MCAST_ADDR_RECORD_TYPE_TO_IN = 3,
   MLD_MCAST_ADDR_RECORD_TYPE_TO_EX = 4,
   MLD_MCAST_ADDR_RECORD_TYPE_ALLOW = 5,
   MLD_MCAST_ADDR_RECORD_TYPE_BLOCK = 6
} MldMcastAddrRecordType;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma pack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief MLD message
 **/

typedef __packed_struct
{
   uint8_t type;           //0
   uint8_t code;           //1
   uint16_t checksum;      //2-3
   uint16_t maxRespDelay;  //4-5
   uint16_t reserved;      //6-7
   Ipv6Addr multicastAddr; //8-23
} MldMessage;


/**
 * @brief MLDv2 Query message
 **/

typedef __packed_struct
{
   uint8_t type;           //0
   uint8_t code;           //1
   uint16_t checksum;      //2-3
   uint16_t maxRespCode;   //4-5
   uint16_t reserved;     //6-7
   Ipv6Addr multicastAddr; //8-23
#if defined(_CPU_BIG_ENDIAN) && !defined(__ICCRX__)
   uint8_t flags : 4;      //24
   uint8_t s : 1;
   uint8_t qrv : 3;
#else
   uint8_t qrv : 3;        //24
   uint8_t s : 1;
   uint8_t flags : 4;
#endif
   uint8_t qqic;           //25
   uint16_t numOfSources;  //26-27
   Ipv6Addr srcAddr[];     //28
} MldListenerQueryV2;


/**
 * @brief MLDv2 Report message
 **/

typedef __packed_struct
{
   uint8_t type;                   //0
   uint8_t reserved;              //1
   uint16_t checksum;              //2-3
   uint16_t flags;                 //4-5
   uint16_t numOfMcastAddrRecords; //6-7
   uint8_t mcastAddrRecords[];     //8
} MldListenerReportV2;


/**
 * @brief MLDv2 multicast address record
 **/

typedef __packed_struct
{
   uint8_t recordType;     //0
   uint8_t auxDataLen;     //1
   uint16_t numOfSources;  //2-3
   Ipv6Addr multicastAddr; //4-19
   Ipv6Addr srcAddr[];     //20
} MldMcastAddrRecord;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma unpack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif

//MLD related constants
extern const Ipv6Addr MLD_V2_ALL_ROUTERS_ADDR;

//Tick counter to handle periodic operations
extern systime_t mldTickCounter;

//MLD related functions
error_t mldInit(NetInterface *interface);
void mldTick(NetInterface *interface);
void mldLinkChangeEvent(NetInterface *interface);

error_t mldSendMessage(NetInterface *interface, const Ipv6Addr *destAddr,
   NetBuffer *buffer, size_t offset);

void mldProcessMessage(NetInterface *interface,
   const Ipv6PseudoHeader *pseudoHeader, const NetBuffer *buffer,
   size_t offset, const NetRxAncillary *ancillary);

systime_t mldGetRandomDelay(systime_t maxDelay);

uint32_t mldDecodeFloatingPointValue8(uint8_t code);
uint32_t mldDecodeFloatingPointValue16(uint16_t code);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
