/**
 * @file mib_common.h
 * @brief Common definitions for MIB modules
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

#ifndef _MIB_COMMON_H
#define _MIB_COMMON_H

//Dependencies
#include "core/net.h"

//Maximum OID size
#ifndef MIB_MAX_OID_SIZE
   #define MIB_MAX_OID_SIZE 16
#elif (MIB_MAX_OID_SIZE < 1)
   #error MIB_MAX_OID_SIZE parameter is not valid
#endif

//Forward declaration of MibObject structure
struct _MibObject;
#define MibObject struct _MibObject


/**
 * @brief MIB object types
 **/

typedef enum
{
   MIB_TYPE_IP_ADDRESS        = 0,
   MIB_TYPE_COUNTER32         = 1,
   MIB_TYPE_GAUGE32           = 2,
   MIB_TYPE_UNSIGNED32        = 2,
   MIB_TYPE_TIME_TICKS        = 3,
   MIB_TYPE_OPAQUE            = 4,
   MIB_TYPE_COUNTER64         = 6
} MibType;


/**
 * @brief Access modes
 **/

typedef enum
{
   MIB_ACCESS_NONE       = 0,
   MIB_ACCESS_READ_ONLY  = 1,
   MIB_ACCESS_WRITE_ONLY = 2,
   MIB_ACCESS_READ_WRITE = 3
} MibAccess;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief Variant data type
 **/

typedef __start_packed struct
{
   __start_packed union
   {
      int32_t integer;
      uint8_t octetString[1];
      uint8_t oid[1];
      uint8_t ipAddr[4];
      uint32_t counter32;
      uint32_t gauge32;
      uint32_t timeTicks;
      uint64_t counter64;
   };
} __end_packed MibVariant;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif


/**
 * @brief Set object value
 **/

typedef error_t (*MibSetValue)(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen);


/**
 * @brief Get object value
 **/

typedef error_t (*MibGetValue)(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen);


/**
 * @brief Get next object
 **/

typedef error_t (*MibGetNext)(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen);


/**
 * @brief MIB object descriptor
 **/

struct _MibObject
{
   const char_t *name;
   uint8_t oid[MIB_MAX_OID_SIZE];
   size_t oidLen;
   uint_t objClass;
   uint_t objType;
   MibAccess access;
   void *value;
   size_t *valueLen;
   size_t valueSize;
   MibSetValue setValue;
   MibGetValue getValue;
   MibGetNext getNext;
};


/**
 * @brief MIB initialization
 **/

typedef error_t (*MibInit)(void);


/**
 * @brief Lock MIB
 **/

typedef void (*MibLock)(void);


/**
 * @brief Unlock MIB
 **/

typedef void (*MibUnlock)(void);


/**
 * @brief MIB module
 **/

typedef struct
{
   const MibObject *objects;
   uint_t numObjects;
   MibInit init;
   MibLock lock;
   MibUnlock unlock;
} MibModule;


//MIB related functions
error_t mibEncodeIndex(uint8_t *oid, size_t maxOidLen, size_t *pos, uint_t index);
error_t mibDecodeIndex(const uint8_t *oid, size_t oidLen, size_t *pos, uint_t *index);

error_t mibEncodeIpv4Addr(uint8_t *oid, size_t maxOidLen, size_t *pos, Ipv4Addr ipAddr);
error_t mibDecodeIpv4Addr(const uint8_t *oid, size_t oidLen, size_t *pos, Ipv4Addr *ipAddr);

error_t mibEncodePort(uint8_t *oid, size_t maxOidLen, size_t *pos, uint16_t port);
error_t mibDecodePort(const uint8_t *oid, size_t oidLen, size_t *pos, uint16_t *port);

#endif
