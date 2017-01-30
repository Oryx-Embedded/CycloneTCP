/**
 * @file snmp_usm.h
 * @brief User-based Security Model (USM) for SNMPv3
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.7.6
 **/

#ifndef _SNMP_USM_H
#define _SNMP_USM_H

//Dependencies
#include "core/net.h"
#include "snmp/snmp_common.h"
#include "crypto.h"

//Time window for replay protection
#ifndef SNMP_TIME_WINDOW
   #define SNMP_TIME_WINDOW 150
#elif (SNMP_TIME_WINDOW < 1)
   #error SNMP_TIME_WINDOW parameter is not valid
#endif

//MD5 authentication support
#ifndef SNMP_MD5_SUPPORT
   #define SNMP_MD5_SUPPORT ENABLED
#elif (SNMP_MD5_SUPPORT != ENABLED && SNMP_MD5_SUPPORT != DISABLED)
   #error SNMP_MD5_SUPPORT parameter is not valid
#endif

//SHA-1 authentication support
#ifndef SNMP_SHA1_SUPPORT
   #define SNMP_SHA1_SUPPORT ENABLED
#elif (SNMP_SHA1_SUPPORT != ENABLED && SNMP_SHA1_SUPPORT != DISABLED)
   #error SNMP_SHA1_SUPPORT parameter is not valid
#endif

//SHA-224 authentication support
#ifndef SNMP_SHA224_SUPPORT
   #define SNMP_SHA224_SUPPORT DISABLED
#elif (SNMP_SHA224_SUPPORT != ENABLED && SNMP_SHA224_SUPPORT != DISABLED)
   #error SNMP_SHA224_SUPPORT parameter is not valid
#endif

//SHA-256 authentication support
#ifndef SNMP_SHA256_SUPPORT
   #define SNMP_SHA256_SUPPORT DISABLED
#elif (SNMP_SHA256_SUPPORT != ENABLED && SNMP_SHA256_SUPPORT != DISABLED)
   #error SNMP_SHA256_SUPPORT parameter is not valid
#endif

//SHA-384 authentication support
#ifndef SNMP_SHA384_SUPPORT
   #define SNMP_SHA384_SUPPORT DISABLED
#elif (SNMP_SHA384_SUPPORT != ENABLED && SNMP_SHA384_SUPPORT != DISABLED)
   #error SNMP_SHA384_SUPPORT parameter is not valid
#endif

//SHA-512 authentication support
#ifndef SNMP_SHA512_SUPPORT
   #define SNMP_SHA512_SUPPORT DISABLED
#elif (SNMP_SHA512_SUPPORT != ENABLED && SNMP_SHA512_SUPPORT != DISABLED)
   #error SNMP_SHA512_SUPPORT parameter is not valid
#endif

//DES encryption support
#ifndef SNMP_DES_SUPPORT
   #define SNMP_DES_SUPPORT ENABLED
#elif (SNMP_DES_SUPPORT != ENABLED && SNMP_DES_SUPPORT != DISABLED)
   #error SNMP_DES_SUPPORT parameter is not valid
#endif

//AES encryption support
#ifndef SNMP_AES_SUPPORT
   #define SNMP_AES_SUPPORT ENABLED
#elif (SNMP_AES_SUPPORT != ENABLED && SNMP_AES_SUPPORT != DISABLED)
   #error SNMP_AES_SUPPORT parameter is not valid
#endif

//Support for MD5 authentication?
#if (SNMP_MD5_SUPPORT == ENABLED)
   #include "md5.h"
#endif

//Support for SHA-1 authentication?
#if (SNMP_SHA1_SUPPORT == ENABLED)
   #include "sha1.h"
#endif

//Support for SHA-224 authentication?
#if (SNMP_SHA224_SUPPORT == ENABLED)
   #include "sha224.h"
#endif

//Support for SHA-256 authentication?
#if (SNMP_SHA256_SUPPORT == ENABLED)
   #include "sha256.h"
#endif

//Support for SHA-384 authentication?
#if (SNMP_SHA384_SUPPORT == ENABLED)
   #include "sha384.h"
#endif

//Support for SHA-512 authentication?
#if (SNMP_SHA512_SUPPORT == ENABLED)
   #include "sha512.h"
#endif

//Support for DES encryption?
#if (SNMP_DES_SUPPORT == ENABLED)
   #include "des.h"
   #include "cipher_mode_cbc.h"
#endif

//Support for AES encryption ?
#if (SNMP_AES_SUPPORT == ENABLED)
   #include "aes.h"
   #include "cipher_mode_cfb.h"
#endif

//Maximum size for authentication and privacy keys
#if (SNMP_SHA512_SUPPORT == ENABLED)
   #define SNMP_MAX_KEY_SIZE 64
#elif (SNMP_SHA384_SUPPORT == ENABLED)
   #define SNMP_MAX_KEY_SIZE 48
#elif (SNMP_SHA256_SUPPORT == ENABLED)
   #define SNMP_MAX_KEY_SIZE 32
#elif (SNMP_SHA224_SUPPORT == ENABLED)
   #define SNMP_MAX_KEY_SIZE 28
#elif (SNMP_SHA1_SUPPORT == ENABLED)
   #define SNMP_MAX_KEY_SIZE 20
#else
   #define SNMP_MAX_KEY_SIZE 16
#endif

//Maximum size for HMAC digests
#if (SNMP_SHA512_SUPPORT == ENABLED)
   #define SNMP_MAX_HMAC_DIGEST_SIZE 48
#elif (SNMP_SHA384_SUPPORT == ENABLED)
   #define SNMP_MAX_HMAC_DIGEST_SIZE 32
#elif (SNMP_SHA256_SUPPORT == ENABLED)
   #define SNMP_MAX_HMAC_DIGEST_SIZE 24
#elif (SNMP_SHA224_SUPPORT == ENABLED)
   #define SNMP_MAX_HMAC_DIGEST_SIZE 16
#elif (SNMP_SHA1_SUPPORT == ENABLED)
   #define SNMP_MAX_HMAC_DIGEST_SIZE 12
#else
   #define SNMP_MAX_HMAC_DIGEST_SIZE 12
#endif

//SNMP message encryption overhead
#if (SNMP_DES_SUPPORT == ENABLED)
   #define SNMP_MSG_ENCRYPTION_OVERHEAD 8
#else
   #define SNMP_MSG_ENCRYPTION_OVERHEAD 0
#endif


/**
 * SNMP message flags
 **/

typedef enum
{
   SNMP_MSG_FLAG_AUTH   = 1,
   SNMP_MSG_FLAG_PRIV   = 2,
   SNMP_MSG_FLAG_REPORT = 4
} SnmpMessageFlags;


/**
 * SNMP security models
 **/

typedef enum
{
   SNMP_SECURITY_MODEL_USM = 3, ///<User-based security model
   SNMP_SECURITY_MODEL_TSM = 4  ///<Transport security model
} SnmpSecurityModel;


/**
 * @brief Access modes
 **/

typedef enum
{
   SNMP_ACCESS_NONE       = 0,
   SNMP_ACCESS_READ_ONLY  = 1,
   SNMP_ACCESS_WRITE_ONLY = 2,
   SNMP_ACCESS_READ_WRITE = 3
} SnmpAccess;


/**
 * SNMP authentication protocols
 **/

typedef enum
{
   SNMP_AUTH_PROTOCOL_NONE   = 0, ///<No authentication
   SNMP_AUTH_PROTOCOL_MD5    = 1, ///<HMAC-MD5-96
   SNMP_AUTH_PROTOCOL_SHA1   = 2, ///<HMAC-SHA-1-96
   SNMP_AUTH_PROTOCOL_SHA224 = 3, ///<HMAC-SHA-224-128
   SNMP_AUTH_PROTOCOL_SHA256 = 4, ///<HMAC-SHA-256-192
   SNMP_AUTH_PROTOCOL_SHA384 = 5, ///<HMAC-SHA-384-256
   SNMP_AUTH_PROTOCOL_SHA512 = 6  ///<HMAC-SHA-512-384
} SnmpAuthProtocol;


/**
 * SNMP privacy protocols
 **/

typedef enum
{
   SNMP_PRIV_PROTOCOL_NONE = 0, ///<No privacy
   SNMP_PRIV_PROTOCOL_DES  = 1, ///<DES-CBC
   SNMP_PRIV_PROTOCOL_AES  = 2  ///<AES-128-CFB
} SnmpPrivProtocol;


/**
 * @brief SNMP key format
 **/

typedef enum
{
   SNMP_KEY_FORMAT_NONE = 0, ///<Unspecified key format
   SNMP_KEY_FORMAT_TEXT = 1, ///<ASCII password
   SNMP_KEY_FORMAT_RAW  = 2  ///<Raw key
} SnmpKeyFormat;


/**
 * @brief SNMP secret key
 **/

typedef struct
{
   uint8_t b[SNMP_MAX_KEY_SIZE];
} SnmpKey;


/**
 * @brief SNMP user information
 **/

typedef struct
{
   char_t name[SNMP_MAX_USER_NAME_LEN + 1]; ///<User name
   SnmpAccess mode;                         ///<Access mode
#if (SNMP_V3_SUPPORT == ENABLED)
   SnmpAuthProtocol authProtocol;           ///<Authentication protocol
   SnmpKey authKey;                         ///<Authentication key
   SnmpPrivProtocol privProtocol;           ///<Privacy protocol
   SnmpKey privKey;                         ///<Privacy key
#endif
} SnmpUserInfo;


//USM related constants
extern const uint8_t usmStatsUnsupportedSecLevelsObject[10];
extern const uint8_t usmStatsNotInTimeWindowsObject[10];
extern const uint8_t usmStatsUnknownUserNamesObject[10];
extern const uint8_t usmStatsUnknownEngineIdsObject[10];
extern const uint8_t usmStatsWrongDigestsObject[10];
extern const uint8_t usmStatsDecryptionErrorsObject[10];

//USM related functions
error_t snmpGenerateKey(SnmpAuthProtocol authProtocol, const char_t *password,
   const uint8_t *engineId, size_t engineIdLen, SnmpKey *key);

error_t snmpCheckSecurityParameters(const SnmpUserInfo *user,
   SnmpMessage *message, const uint8_t *engineId, size_t engineIdLen);

error_t snmpAuthOutgoingMessage(const SnmpUserInfo *user, SnmpMessage *message);
error_t snmpAuthIncomingMessage(const SnmpUserInfo *user, SnmpMessage *message);

error_t snmpEncryptData(const SnmpUserInfo *user, SnmpMessage *message, uint64_t *salt);
error_t snmpDecryptData(const SnmpUserInfo *user, SnmpMessage *message);

#endif
