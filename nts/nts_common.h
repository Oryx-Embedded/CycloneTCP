/**
 * @file nts_common.h
 * @brief Definitions common to NTS client and server
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

#ifndef _NTS_COMMON_H
#define _NTS_COMMON_H

//Dependencies
#include "core/net.h"

//NTS port number
#define NTS_PORT 4460

//Critical flag
#define NTS_KE_CRITICAL 0x8000
//Record type mask
#define NTS_KE_RECORD_TYPE_MASK 0x7FFF

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief NTS-KE record types
 **/

typedef enum
{
   NTS_KE_RECORD_TYPE_END_OF_MESSAGE       = 0, ///<End of Message
   NTS_KE_RECORD_TYPE_NTS_NEXT_PROTO_NEGO  = 1, ///<NTS Next Protocol Negotiation
   NTS_KE_RECORD_TYPE_ERROR                = 2, ///<Error
   NTS_KE_RECORD_TYPE_WARNING              = 3, ///<Warning
   NTS_KE_RECORD_TYPE_AEAD_ALGO_NEGO       = 4, ///<AEAD Algorithm Negotiation
   NTS_KE_RECORD_TYPE_NEW_COOKIE_FOR_NTPV4 = 5, ///<New Cookie for NTPv4
   NTS_KE_RECORD_TYPE_NTPV4_SERVER_NEGO    = 6, ///<NTPv4 Server Negotiation
   NTS_KE_RECORD_TYPE_NTPV4_PORT_NEGO      = 7  ///<NTPv4 Port Negotiation
} NtsKeRecordType;


/**
 * @brief Protocol IDs
 **/

typedef enum
{
   NTS_PROTOCOL_ID_NTPV4 = 0 ///<Network Time Protocol version 4 (NTPv4)
} NtsProtocolId;


/**
 * @brief Error codes
 **/

typedef enum
{
   NTS_ERROR_CODE_UNRECOGNIZED_CRITICAL_RECORD = 0, ///<Unrecognized Critical Record
   NTS_ERROR_CODE_BAD_REQUEST                  = 1, ///<Bad Request
   NTS_ERROR_CODE_INTERNAL_SERVER_ERROR        = 2  ///<Internal Server Error
} NtsErrorCode;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma pack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief NTS-KE record
 **/

typedef __packed_struct
{
   uint16_t type;       //0-1
   uint16_t bodyLength; //2-3
   uint8_t body[];      //4
} NtsKeRecord;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma unpack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
