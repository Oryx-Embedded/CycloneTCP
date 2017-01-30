/**
 * @file snmp_common.h
 * @brief Functions common to SNMP agent and SNMP manager
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

#ifndef _SNMP_COMMON_H
#define _SNMP_COMMON_H

//Dependencies
#include "core/net.h"

//SNMPv1 support
#ifndef SNMP_V1_SUPPORT
   #define SNMP_V1_SUPPORT ENABLED
#elif (SNMP_V1_SUPPORT != ENABLED && SNMP_V1_SUPPORT != DISABLED)
   #error SNMP_V1_SUPPORT parameter is not valid
#endif

//SNMPv2c support
#ifndef SNMP_V2C_SUPPORT
   #define SNMP_V2C_SUPPORT ENABLED
#elif (SNMP_V2C_SUPPORT != ENABLED && SNMP_V2C_SUPPORT != DISABLED)
   #error SNMP_V2C_SUPPORT parameter is not valid
#endif

//SNMPv3 support
#ifndef SNMP_V3_SUPPORT
   #define SNMP_V3_SUPPORT DISABLED
#elif (SNMP_V3_SUPPORT != ENABLED && SNMP_V3_SUPPORT != DISABLED)
   #error SNMP_V3_SUPPORT parameter is not valid
#endif

//Maximum size of SNMP messages
#ifndef SNMP_MAX_MSG_SIZE
   #define SNMP_MAX_MSG_SIZE 484
#elif (SNMP_MAX_MSG_SIZE < 484 || SNMP_MAX_MSG_SIZE > 65535)
   #error SNMP_MAX_MSG_SIZE parameter is not valid
#endif

//Maximum size for context engine identifier
#ifndef SNMP_MAX_CONTEXT_ENGINE_SIZE
   #define SNMP_MAX_CONTEXT_ENGINE_SIZE 32
#elif (SNMP_MAX_CONTEXT_ENGINE_SIZE < 8)
   #error SNMP_MAX_CONTEXT_ENGINE_SIZE parameter is not valid
#endif

//Maximum length for context name
#ifndef SNMP_MAX_CONTEXT_NAME_LEN
   #define SNMP_MAX_CONTEXT_NAME_LEN 16
#elif (SNMP_MAX_CONTEXT_NAME_LEN < 8)
   #error SNMP_MAX_CONTEXT_NAME_LEN parameter is not valid
#endif

//Maximum length for user names and community names
#ifndef SNMP_MAX_USER_NAME_LEN
   #define SNMP_MAX_USER_NAME_LEN 16
#elif (SNMP_MAX_USER_NAME_LEN < 8)
   #error SNMP_MAX_USER_NAME_LEN parameter is not valid
#endif

//Maximum size for object identifiers
#ifndef SNMP_MAX_OID_SIZE
   #define SNMP_MAX_OID_SIZE 16
#elif (SNMP_MAX_OID_SIZE < 1)
   #error SNMP_MAX_OID_SIZE parameter is not valid
#endif

//SNMP port number
#define SNMP_PORT 161
//SNMP trap port number
#define SNMP_TRAP_PORT 162

//SNMPv1 message header overhead
#define SNMP_V1_MSG_HEADER_OVERHEAD 48
//SNMPv2c message header overhead
#define SNMP_V2C_MSG_HEADER_OVERHEAD 37
//SNMPv3 message header overhead
#define SNMP_V3_MSG_HEADER_OVERHEAD 105


/**
 * @brief SNMP version identifiers
 **/

typedef enum
{
   SNMP_VERSION_1  = 0,
   SNMP_VERSION_2C = 1,
   SNMP_VERSION_3  = 3
} SnmpVersion;


/**
 * @brief SNMP PDU types
 **/

typedef enum
{
   SNMP_PDU_GET_REQUEST      = 0,
   SNMP_PDU_GET_NEXT_REQUEST = 1,
   SNMP_PDU_GET_RESPONSE     = 2,
   SNMP_PDU_SET_REQUEST      = 3,
   SNMP_PDU_TRAP             = 4,
   SNMP_PDU_GET_BULK_REQUEST = 5,
   SNMP_PDU_INFORM_REQUEST   = 6,
   SNMP_PDU_TRAP_V2          = 7,
   SNMP_PDU_REPORT           = 8
} SnmpPduType;


/**
 * @brief SNMP generic trap types
 **/

typedef enum
{
   SNMP_TRAP_COLD_START          = 0,
   SNMP_TRAP_WARM_START          = 1,
   SNMP_TRAP_LINK_DOWN           = 2,
   SNMP_TRAP_LINK_UP             = 3,
   SNMP_TRAP_AUTH_FAILURE        = 4,
   SNMP_TRAP_EGP_NEIGHBOR_LOSS   = 5,
   SNMP_TRAP_ENTERPRISE_SPECIFIC = 6
} SnmpGenericTrapType;


/**
 * @brief SNMP error status
 **/

typedef enum
{
   SNMP_ERROR_NONE                 = 0,
   SNMP_ERROR_TOO_BIG              = 1,
   SNMP_ERROR_NO_SUCH_NAME         = 2,
   SNMP_ERROR_BAD_VALUE            = 3,
   SNMP_ERROR_READ_ONLY            = 4,
   SNMP_ERROR_GENERIC              = 5,
   SNMP_ERROR_NO_ACCESS            = 6,
   SNMP_ERROR_WRONG_TYPE           = 7,
   SNMP_ERROR_WRONG_LENGTH         = 8,
   SNMP_ERROR_WRONG_ENCODING       = 9,
   SNMP_ERROR_WRONG_VALUE          = 10,
   SNMP_ERROR_NO_CREATION          = 11,
   SNMP_ERROR_INCONSISTENT_VALUE   = 12,
   SNMP_ERROR_RESOURCE_UNAVAILABLE = 13,
   SNMP_ERROR_COMMIT_FAILED        = 14,
   SNMP_ERROR_UNDO_FAILED          = 15,
   SNMP_ERROR_AUTHORIZATION        = 16,
   SNMP_ERROR_NOT_WRITABLE         = 17,
   SNMP_ERROR_INCONSISTENT_NAME    = 18
} SnmpErrorStatus;


/**
 * @brief SNMP exceptions
 **/

typedef enum
{
   SNMP_EXCEPTION_NO_SUCH_OBJECT   = 0,
   SNMP_EXCEPTION_NO_SUCH_INSTANCE = 1,
   SNMP_EXCEPTION_END_OF_MIB_VIEW  = 2
} SnmpException;


/**
 * @brief SNMP engine ID format
 **/

typedef enum
{
   SNMP_ENGINE_ID_FORMAT_IPV4   = 1,
   SNMP_ENGINE_ID_FORMAT_IPV6   = 2,
   SNMP_ENGINE_ID_FORMAT_MAC    = 3,
   SNMP_ENGINE_ID_FORMAT_TEXT   = 4,
   SNMP_ENGINE_ID_FORMAT_OCTETS = 5,
} SnmpEngineIdFormat;


/**
 * @brief SNMP message
 **/

typedef struct
{
   uint8_t buffer[SNMP_MAX_MSG_SIZE]; ///<Buffer that holds the message
   size_t bufferLen;                  ///<Original length of the message
   uint8_t *pos;                      ///<Current position
   size_t length;                     ///<Length of the message
   int32_t version;                   ///<SNMP version identifier
#if (SNMP_V1_SUPPORT == ENABLED || SNMP_V2C_SUPPORT == ENABLED)
   const char_t *community;           ///<Community name
   size_t communityLen;               ///<Length of the community name
#endif
#if (SNMP_V3_SUPPORT == ENABLED)
   int32_t msgId;                     ///<Message identifier
   int32_t msgMaxSize;                ///<Maximum message size supported by the sender
   uint8_t msgFlags;                  ///<Bit fields which control processing of the message
   int32_t msgSecurityModel;          ///<Security model used by the sender
   const uint8_t *msgAuthEngineId;    ///<Authoritative engine identifier
   size_t msgAuthEngineIdLen;         ///<Length of the authoritative engine identifier
   int32_t msgAuthEngineBoots;        ///<Number of times the SNMP engine has rebooted
   int32_t msgAuthEngineTime;         ///<Number of seconds since last reboot
   const char_t *msgUserName;         ///<User name
   size_t msgUserNameLen;             ///<Length of the user name
   uint8_t *msgAuthParameters;        ///<Authentication parameters
   size_t msgAuthParametersLen;       ///<Length of the authentication parameters
   const uint8_t *msgPrivParameters;  ///<Privacy parameters
   size_t msgPrivParametersLen;       ///<Length of the privacy parameters
   const uint8_t *contextEngineId;    ///<Context engine identifier
   size_t contextEngineIdLen;         ///<Length of the context engine identifier
   const uint8_t *contextName;        ///<Context name
   size_t contextNameLen;             ///<Length of the context name
#endif
   SnmpPduType pduType;               ///<PDU type
   int32_t requestId;                 ///<Request identifier
   int32_t errorStatus;               ///<Error status
   int32_t errorIndex;                ///<Error index
#if (SNMP_V1_SUPPORT == ENABLED)
   const uint8_t *enterpriseOid;      ///<Type of object generating trap
   size_t enterpriseOidLen;           ///<Length of the enterprise OID
   Ipv4Addr agentAddr;                ///<Address of object generating trap
   int32_t genericTrapType;           ///<Generic trap type
   int32_t specificTrapCode;          ///<Specific trap code
   uint32_t timestamp;                ///<Timestamp
#endif
#if (SNMP_V2C_SUPPORT == ENABLED || SNMP_V3_SUPPORT == ENABLED)
   int32_t nonRepeaters;              ///<GetBulkRequest-PDU specific parameter
   int32_t maxRepetitions;            ///<GetBulkRequest-PDU specific parameter
#endif
   uint8_t *varBindList;              ///<List of variable bindings
   size_t varBindListLen;             ///<Length of the list in bytes
   size_t varBindListMaxLen;          ///<Maximum length of the list in bytes
   size_t oidLen;                     ///<Length of the object identifier
} SnmpMessage;


/**
 * @brief Variable binding
 **/

typedef struct
{
   const uint8_t *oid;
   size_t oidLen;
   uint_t objClass;
   uint_t objType;
   const uint8_t *value;
   size_t valueLen;
} SnmpVarBind;


//SNMP related functions
void snmpInitMessage(SnmpMessage *message);
error_t snmpComputeMessageOverhead(SnmpMessage *message);

error_t snmpParseMessageHeader(SnmpMessage *message);
error_t snmpWriteMessageHeader(SnmpMessage *message);

error_t snmpParseCommunity(SnmpMessage *message);
error_t snmpWriteCommunity(SnmpMessage *message);

error_t snmpParseGlobalData(SnmpMessage *message);
error_t snmpWriteGlobalData(SnmpMessage *message);

error_t snmpParseSecurityParameters(SnmpMessage *message);
error_t snmpWriteSecurityParameters(SnmpMessage *message);

error_t snmpParseScopedPdu(SnmpMessage *message);
error_t snmpWriteScopedPdu(SnmpMessage *message);

error_t snmpParsePduHeader(SnmpMessage *message);
error_t snmpWritePduHeader(SnmpMessage *message);

error_t snmpEncodeInt32(int32_t value, uint8_t *dest, size_t *length);
error_t snmpEncodeUnsignedInt32(uint32_t value, uint8_t *dest, size_t *length);
error_t snmpEncodeUnsignedInt64(uint64_t value, uint8_t *dest, size_t *length);

error_t snmpDecodeInt32(const uint8_t *src, size_t length, int32_t *value);
error_t snmpDecodeUnsignedInt32(const uint8_t *src, size_t length, uint32_t *value);
error_t snmpDecodeUnsignedInt64(const uint8_t *src, size_t length, uint64_t *value);

#endif
