/**
 * @file mib_common.c
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

//Dependencies
#include "core/net.h"
#include "mibs/mib_common.h"
#include "oid.h"
#include "debug.h"


/**
 * @brief Encode instance identifier (index)
 * @param[in] oid Pointer to the object identifier
 * @param[in] maxOidLen Maximum number of bytes the OID can hold
 * @param[in,out] pos Offset where to write the instance identifier
 * @param[in] index Index value
 * @return Error code
 **/

error_t mibEncodeIndex(uint8_t *oid, size_t maxOidLen, size_t *pos, uint_t index)
{
   //Encode instance identifier
   return oidEncodeSubIdentifier(oid, maxOidLen, pos, index);
}


/**
 * @brief Decode instance identifier (index)
 * @param[in] oid Pointer to the object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in,out] pos Offset where to read the instance identifier
 * @param[out] index Index value
 * @return Error code
 **/

error_t mibDecodeIndex(const uint8_t *oid, size_t oidLen, size_t *pos, uint_t *index)
{
   error_t error;
   uint32_t value;

   //Decode instance identifier
   error = oidDecodeSubIdentifier(oid, oidLen, pos, &value);
   //Invalid sub-identifier?
   if(error)
      return error;

   //Save index value
   *index = value;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Encode instance identifier (IPv4 address)
 * @param[in] oid Pointer to the object identifier
 * @param[in] maxOidLen Maximum number of bytes the OID can hold
 * @param[in,out] pos Offset where to write the instance identifier
 * @param[in] ipAddr IPv4 address
 * @return Error code
 **/

error_t mibEncodeIpv4Addr(uint8_t *oid, size_t maxOidLen, size_t *pos, Ipv4Addr ipAddr)
{
   error_t error;
   uint_t i;
   uint8_t *p;

   //Cast the IPv4 address as a byte array
   p = (uint8_t *) &ipAddr;

   //The address is encoded as 4 subsequent sub-identifiers
   for(i = 0; i < 4; i++)
   {
      //Encode the current byte
      error = oidEncodeSubIdentifier(oid, maxOidLen, pos, p[i]);
      //Any error to report?
      if(error)
         return error;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Decode instance identifier (IPv4 address)
 * @param[in] oid Pointer to the object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in,out] pos Offset where to read the instance identifier
 * @param[out] ipAddr IPv4 address
 * @return Error code
 **/

error_t mibDecodeIpv4Addr(const uint8_t *oid, size_t oidLen, size_t *pos, Ipv4Addr *ipAddr)
{
   error_t error;
   uint_t i;
   uint32_t value;
   uint8_t *p;

   //Cast the IPv4 address as a byte array
   p = (uint8_t *) ipAddr;

   //The address is encoded as 4 subsequent sub-identifiers
   for(i = 0; i < 4; i++)
   {
      //Decode the current sub-identifier
      error = oidDecodeSubIdentifier(oid, oidLen, pos, &value);
      //Invalid sub-identifier?
      if(error)
         return error;

      //Each byte of the IPv4 address must be in the range 0-255
      if(value > 255)
         return ERROR_INSTANCE_NOT_FOUND;

      //Save the current byte
      p[i] = value & 0xFF;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Encode instance identifier (port number)
 * @param[in] oid Pointer to the object identifier
 * @param[in] maxOidLen Maximum number of bytes the OID can hold
 * @param[in,out] pos Offset where to write the instance identifier
 * @param[in] port Port number
 * @return Error code
 **/

error_t mibEncodePort(uint8_t *oid, size_t maxOidLen, size_t *pos, uint16_t port)
{
   //Encode instance identifier
   return oidEncodeSubIdentifier(oid, maxOidLen, pos, port);
}


/**
 * @brief Decode instance identifier (port number)
 * @param[in] oid Pointer to the object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in,out] pos Offset where to read the instance identifier
 * @param[out] port Port number
 * @return Error code
 **/

error_t mibDecodePort(const uint8_t *oid, size_t oidLen, size_t *pos, uint16_t *port)
{
   error_t error;
   uint32_t value;

   //Decode instance identifier
   error = oidDecodeSubIdentifier(oid, oidLen, pos, &value);
   //Invalid sub-identifier?
   if(error)
      return error;

   //Port number must be in range 0-65535
   if(value > 65535)
      return ERROR_INSTANCE_NOT_FOUND;

   //Save port number
   *port = value;

   //Successful processing
   return NO_ERROR;
}
