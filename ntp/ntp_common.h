/**
 * @file ntp_common.h
 * @brief Definitions common to NTP client and server
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

#ifndef _NTP_COMMON_H
#define _NTP_COMMON_H

//Dependencies
#include "core/net.h"

//NTP port number
#define NTP_PORT 123
//Maximum size of NTP messages
#define NTP_MAX_MSG_SIZE 68
//Difference between NTP and Unix time scales
#define NTP_UNIX_EPOCH 2208988800U

//Kiss code definition
#define NTP_KISS_CODE(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Leap indicator
 **/

typedef enum
{
   NTP_LI_NO_WARNING           = 0,
   NTP_LI_LAST_MIN_HAS_61_SECS = 1,
   NTP_LI_LAST_MIN_HAS_59_SECS = 2,
   NTP_LI_ALARM_CONDITION      = 3
} NtpLeapIndicator;


/**
 * @brief NTP version numbers
 **/

typedef enum
{
   NTP_VERSION_1 = 1,
   NTP_VERSION_2 = 2,
   NTP_VERSION_3 = 3,
   NTP_VERSION_4 = 4
} NtpVersion;


/**
 * @brief Protocol modes
 **/

typedef enum
{
   NTP_MODE_SYMMETRIC_ACTIVE  = 1,
   NTP_MODE_SYMMETRIC_PASSIVE = 2,
   NTP_MODE_CLIENT            = 3,
   NTP_MODE_SERVER            = 4,
   NTP_MODE_BROADCAST         = 5
} NtpMode;


/**
 * @brief Stratum
 **/

typedef enum
{
   NTP_STRATUM_KISS_OF_DEATH = 0,
   NTP_STRATUM_PRIMARY       = 1,
   NTP_STRATUM_SECONDARY_2   = 2,
   NTP_STRATUM_SECONDARY_3   = 3,
   NTP_STRATUM_SECONDARY_4   = 4,
   NTP_STRATUM_SECONDARY_5   = 5,
   NTP_STRATUM_SECONDARY_6   = 6,
   NTP_STRATUM_SECONDARY_7   = 7,
   NTP_STRATUM_SECONDARY_8   = 8,
   NTP_STRATUM_SECONDARY_9   = 9,
   NTP_STRATUM_SECONDARY_10  = 10,
   NTP_STRATUM_SECONDARY_11  = 11,
   NTP_STRATUM_SECONDARY_12  = 12,
   NTP_STRATUM_SECONDARY_13  = 13,
   NTP_STRATUM_SECONDARY_14  = 14,
   NTP_STRATUM_SECONDARY_15  = 15,
} NtpStratum;


/**
 * @brief Kiss codes
 *
 * The kiss codes can provide useful information for an intelligent client.
 * These codes are encoded in four-character ASCII strings left justified
 * and zero filled
 *
 **/

typedef enum
{
   NTP_KISS_CODE_ACST = NTP_KISS_CODE('A', 'C', 'S', 'T'), ///<The association belongs to a anycast server
   NTP_KISS_CODE_AUTH = NTP_KISS_CODE('A', 'U', 'T', 'H'), ///<Server authentication failed
   NTP_KISS_CODE_AUTO = NTP_KISS_CODE('A', 'U', 'T', 'O'), ///<Autokey sequence failed
   NTP_KISS_CODE_BCST = NTP_KISS_CODE('B', 'C', 'S', 'T'), ///<The association belongs to a broadcast server
   NTP_KISS_CODE_CRYP = NTP_KISS_CODE('C', 'R', 'Y', 'P'), ///<Cryptographic authentication or identification failed
   NTP_KISS_CODE_DENY = NTP_KISS_CODE('D', 'E', 'N', 'Y'), ///<Access denied by remote server
   NTP_KISS_CODE_DROP = NTP_KISS_CODE('D', 'R', 'O', 'P'), ///<Lost peer in symmetric mode
   NTP_KISS_CODE_RSTR = NTP_KISS_CODE('R', 'S', 'T', 'R'), ///<Access denied due to local policy
   NTP_KISS_CODE_INIT = NTP_KISS_CODE('I', 'N', 'I', 'T'), ///<The association has not yet synchronized for the first time
   NTP_KISS_CODE_MCST = NTP_KISS_CODE('M', 'C', 'S', 'T'), ///<The association belongs to a manycast server
   NTP_KISS_CODE_NKEY = NTP_KISS_CODE('N', 'K', 'E', 'Y'), ///<No key found
   NTP_KISS_CODE_RATE = NTP_KISS_CODE('R', 'A', 'T', 'E'), ///<Rate exceeded
   NTP_KISS_CODE_RMOT = NTP_KISS_CODE('R', 'M', 'O', 'T'), ///<Somebody is tinkering with the association from a remote host running ntpdc
   NTP_KISS_CODE_STEP = NTP_KISS_CODE('S', 'T', 'E', 'P'), ///<A step change in system time has occurred
   NTP_KISS_CODE_NTSN = NTP_KISS_CODE('N', 'T', 'S', 'N')  ///<NTS negative-acknowledgment (NAK)
} NtpKissCode;


/**
 * @brief NTP extensions field types
 **/

typedef enum
{
   NTP_EXTENSION_TYPE_NO_OPERATION_REQ        = 0x0002, ///<No-Operation Request
   NTP_EXTENSION_TYPE_UNIQUE_ID               = 0x0104, ///<Unique Identifier
   NTP_EXTENSION_TYPE_NTS_COOKIE              = 0x0204, ///<NTS Cookie
   NTP_EXTENSION_TYPE_NTS_COOKIE_PLACEHOLDER  = 0x0304, ///<NTS Cookie Placeholder
   NTP_EXTENSION_TYPE_NTS_AEAD                = 0x0404, ///<NTS Authenticator and Encrypted Extension Fields
   NTP_EXTENSION_TYPE_NO_OPERATION_RESP       = 0x8002, ///<No-Operation Response
   NTP_EXTENSION_TYPE_NO_OPERATION_ERROR_RESP = 0xC002  ///<No-Operation Error Response
} NtpExtensionType;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma pack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief NTP timestamp representation
 **/

typedef __packed_struct
{
   uint32_t seconds;
   uint32_t fraction;
} NtpTimestamp;


/**
 * @brief NTP packet header
 **/

typedef __packed_struct
{
#if defined(_CPU_BIG_ENDIAN) && !defined(__ICCRX__)
   uint8_t li : 2;                  //0
   uint8_t vn : 3;
   uint8_t mode : 3;
#else
   uint8_t mode : 3;                //0
   uint8_t vn : 3;
   uint8_t li : 2;
#endif
   uint8_t stratum;                 //1
   uint8_t poll;                    //2
   int8_t precision;                //3
   uint32_t rootDelay;              //4-7
   uint32_t rootDispersion;         //8-11
   uint32_t referenceId;            //12-15
   NtpTimestamp referenceTimestamp; //16-23
   NtpTimestamp originateTimestamp; //24-31
   NtpTimestamp receiveTimestamp;   //32-39
   NtpTimestamp transmitTimestamp;  //40-47
   uint8_t extensions[];            //48
} NtpHeader;


/**
 * @brief NTP extension field
 **/

typedef __packed_struct
{
   uint16_t fieldType; //0-1
   uint16_t length;    //2-3
   uint8_t value[];    //4
} NtpExtension;


/**
 * @brief NTS Authenticator and Encrypted Extension Fields extension
 **/

typedef __packed_struct
{
   uint16_t fieldType;        //0-1
   uint16_t length;           //2-3
   uint16_t nonceLength;      //4-5
   uint16_t ciphertextLength; //6-7
   uint8_t nonce[];           //8
} NtpNtsAeadExtension;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma unpack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif

//NTP related functions
const NtpExtension *ntpGetExtension(const uint8_t *extensions, size_t length,
   uint16_t type, uint_t index);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
