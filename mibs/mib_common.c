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
 * @brief Encode instance identifier (unsigned 32-bit integer)
 * @param[in] oid Pointer to the object identifier
 * @param[in] maxOidLen Maximum number of bytes the OID can hold
 * @param[in,out] pos Offset where to write the instance identifier
 * @param[in] value Unsigned 32-bit integer
 * @return Error code
 **/

error_t mibEncodeUnsigned32(uint8_t *oid, size_t maxOidLen, size_t *pos, uint32_t value)
{
   //Encode instance identifier
   return oidEncodeSubIdentifier(oid, maxOidLen, pos, value);
}


/**
 * @brief Decode instance identifier (unsigned 32-bit integer)
 * @param[in] oid Pointer to the object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in,out] pos Offset where to read the instance identifier
 * @param[out] value Unsigned 32-bit integer
 * @return Error code
 **/

error_t mibDecodeUnsigned32(const uint8_t *oid, size_t oidLen, size_t *pos, uint32_t *value)
{
   //Decode instance identifier
   return oidDecodeSubIdentifier(oid, oidLen, pos, value);
}


/**
 * @brief Encode instance identifier (octet string)
 * @param[in] oid Pointer to the object identifier
 * @param[in] maxOidLen Maximum number of bytes the OID can hold
 * @param[in,out] pos Offset where to write the instance identifier
 * @param[in] value Unsigned 32-bit integer
 * @param[in] data Pointer to the octet string
 * @param[in] dataLen Length of the octet string, in bytes
 * @return Error code
 **/

error_t mibEncodeOctetString(uint8_t *oid, size_t maxOidLen, size_t *pos,
   const uint8_t *data, size_t dataLen)
{
   error_t error;
   uint_t i;

   //Encode the length of the octet string
   error = oidEncodeSubIdentifier(oid, maxOidLen, pos, dataLen);
   //Any error to report?
   if(error)
      return error;

   //Encode the octet string
   for(i = 0; i < dataLen; i++)
   {
      //Encode the current byte
      error = oidEncodeSubIdentifier(oid, maxOidLen, pos, data[i]);
      //Any error to report?
      if(error)
         return error;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Decode instance identifier (octet string)
 * @param[in] oid Pointer to the object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in,out] pos Offset where to read the instance identifier
 * @param[out] data Buffer where to store the octet string
 * @param[in] maxDataLen Maximum number of bytes the buffer can hold
 * @param[out] dataLen Length of the octet string, in bytes
 * @return Error code
 **/

error_t mibDecodeOctetString(const uint8_t *oid, size_t oidLen, size_t *pos,
   uint8_t *data, size_t maxDataLen, size_t *dataLen)
{
   error_t error;
   uint_t i;
   uint32_t length;
   uint32_t value;

   //Decode the length of the octet string
   error = oidDecodeSubIdentifier(oid, oidLen, pos, &length);
   //Any error to report?
   if(error)
      return error;

   //Make sure the length of the octet string is valid
   if(length > maxDataLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Decode the octet string
   for(i = 0; i < length; i++)
   {
      //Decode the current sub-identifier
      error = oidDecodeSubIdentifier(oid, oidLen, pos, &value);
      //Invalid sub-identifier?
      if(error)
         return error;

      //Each byte of the octet string must be in the range 0-255
      if(value > 255)
         return ERROR_INSTANCE_NOT_FOUND;

      //Save the current byte
      data[i] = value & 0xFF;
   }

   //Return the length of the octet string
   *dataLen = length;

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


/**
 * @brief Encode instance identifier (MAC address)
 * @param[in] oid Pointer to the object identifier
 * @param[in] maxOidLen Maximum number of bytes the OID can hold
 * @param[in,out] pos Offset where to write the instance identifier
 * @param[in] macAddr MAC address
 * @return Error code
 **/

error_t mibEncodeMacAddr(uint8_t *oid, size_t maxOidLen, size_t *pos, const MacAddr *macAddr)
{
   error_t error;
   uint_t i;

   //Encode the length of the octet string
   error = oidEncodeSubIdentifier(oid, maxOidLen, pos, sizeof(MacAddr));
   //Any error to report?
   if(error)
      return error;

   //The address is encoded as 6 subsequent sub-identifiers
   for(i = 0; i < sizeof(MacAddr); i++)
   {
      //Encode the current byte
      error = oidEncodeSubIdentifier(oid, maxOidLen, pos, macAddr->b[i]);
      //Any error to report?
      if(error)
         return error;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Decode instance identifier (MAC address)
 * @param[in] oid Pointer to the object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in,out] pos Offset where to read the instance identifier
 * @param[out] macAddr MAC address
 * @return Error code
 **/

error_t mibDecodeMacAddr(const uint8_t *oid, size_t oidLen, size_t *pos, MacAddr *macAddr)
{
   error_t error;
   uint_t i;
   uint32_t length;
   uint32_t value;

   //Decode the length of the octet string
   error = oidDecodeSubIdentifier(oid, oidLen, pos, &length);
   //Any error to report?
   if(error)
      return error;

   //Make sure the length of the octet string is valid
   if(length != sizeof(MacAddr))
      return ERROR_INSTANCE_NOT_FOUND;

   //The address is encoded as 6 subsequent sub-identifiers
   for(i = 0; i < sizeof(MacAddr); i++)
   {
      //Decode the current sub-identifier
      error = oidDecodeSubIdentifier(oid, oidLen, pos, &value);
      //Invalid sub-identifier?
      if(error)
         return error;

      //Each byte of the MAC address must be in the range 0-255
      if(value > 255)
         return ERROR_INSTANCE_NOT_FOUND;

      //Save the current byte
      macAddr->b[i] = value & 0xFF;
   }

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
   for(i = 0; i < sizeof(Ipv4Addr); i++)
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
   for(i = 0; i < sizeof(Ipv4Addr); i++)
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
 * @brief Encode instance identifier (IPv6 address)
 * @param[in] oid Pointer to the object identifier
 * @param[in] maxOidLen Maximum number of bytes the OID can hold
 * @param[in,out] pos Offset where to write the instance identifier
 * @param[in] ipAddr IPv6 address
 * @return Error code
 **/

error_t mibEncodeIpv6Addr(uint8_t *oid, size_t maxOidLen, size_t *pos, const Ipv6Addr *ipAddr)
{
   error_t error;
   uint_t i;

   //The address is encoded as 16 subsequent sub-identifiers
   for(i = 0; i < sizeof(Ipv6Addr); i++)
   {
      //Encode the current byte
      error = oidEncodeSubIdentifier(oid, maxOidLen, pos, ipAddr->b[i]);
      //Any error to report?
      if(error)
         return error;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Decode instance identifier (IPv6 address)
 * @param[in] oid Pointer to the object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in,out] pos Offset where to read the instance identifier
 * @param[out] ipAddr IPv6 address
 * @return Error code
 **/

error_t mibDecodeIpv6Addr(const uint8_t *oid, size_t oidLen, size_t *pos, Ipv6Addr *ipAddr)
{
   error_t error;
   uint_t i;
   uint32_t value;

   //The address is encoded as 16 subsequent sub-identifiers
   for(i = 0; i < sizeof(Ipv6Addr); i++)
   {
      //Decode the current sub-identifier
      error = oidDecodeSubIdentifier(oid, oidLen, pos, &value);
      //Invalid sub-identifier?
      if(error)
         return error;

      //Each byte of the IPv6 address must be in the range 0-255
      if(value > 255)
         return ERROR_INSTANCE_NOT_FOUND;

      //Save the current byte
      ipAddr->b[i] = value & 0xFF;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Encode instance identifier (IP address)
 * @param[in] oid Pointer to the object identifier
 * @param[in] maxOidLen Maximum number of bytes the OID can hold
 * @param[in,out] pos Offset where to write the instance identifier
 * @param[in] ipAddr IP address
 * @return Error code
 **/

error_t mibEncodeIpAddr(uint8_t *oid, size_t maxOidLen, size_t *pos, const IpAddr *ipAddr)
{
   error_t error;

#if (IPV4_SUPPORT == ENABLED)
   //IPv4 address?
   if(ipAddr->length == sizeof(Ipv4Addr))
   {
      //Encode address type (IPv4)
      error = oidEncodeSubIdentifier(oid, maxOidLen, pos, INET_ADDR_TYPE_IPV4);

      //Check status code
      if(!error)
      {
         //Encode the length of the octet string
         error = oidEncodeSubIdentifier(oid, maxOidLen, pos, sizeof(Ipv4Addr));
      }

      //Check status code
      if(!error)
      {
         //Encode IPv4 address
         error = mibEncodeIpv4Addr(oid, maxOidLen, pos, ipAddr->ipv4Addr);
      }
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 address?
   if(ipAddr->length == sizeof(Ipv6Addr))
   {
      //Encode address type (IPv6)
      error = oidEncodeSubIdentifier(oid, maxOidLen, pos, INET_ADDR_TYPE_IPV6);

      //Check status code
      if(!error)
      {
         //Encode the length of the octet string
         error = oidEncodeSubIdentifier(oid, maxOidLen, pos, sizeof(Ipv6Addr));
      }

      //Check status code
      if(!error)
      {
         //Encode IPv6 address
         error = mibEncodeIpv6Addr(oid, maxOidLen, pos, &ipAddr->ipv6Addr);
      }
   }
   else
#endif
   //Unknown address?
   {
      //Encode address type (unknown)
      error = oidEncodeSubIdentifier(oid, maxOidLen, pos, INET_ADDR_TYPE_UNKNOWN);

      //Check status code
      if(!error)
      {
         //The unknown address is a zero-length octet string
         error = oidEncodeSubIdentifier(oid, maxOidLen, pos, 0);
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Decode instance identifier (IP address)
 * @param[in] oid Pointer to the object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in,out] pos Offset where to read the instance identifier
 * @param[out] ipAddr IP address
 * @return Error code
 **/

error_t mibDecodeIpAddr(const uint8_t *oid, size_t oidLen, size_t *pos, IpAddr *ipAddr)
{
   error_t error;
   uint32_t type;
   uint32_t length;

   //Decode address type
   error = oidDecodeSubIdentifier(oid, oidLen, pos, &type);

   //Check status code
   if(!error)
   {
      //Decode the length of the octet string
      error = oidDecodeSubIdentifier(oid, oidLen, pos, &length);
   }

   //Check status code
   if(!error)
   {
      //Unknown address?
      if(type == INET_ADDR_TYPE_UNKNOWN && length == 0)
      {
         //The unknown address is a zero-length octet string
         *ipAddr = IP_ADDR_ANY;
      }
#if (IPV4_SUPPORT == ENABLED)
      //IPv4 address?
      else if(type == INET_ADDR_TYPE_IPV4 && length == sizeof(Ipv4Addr))
      {
         //Save the length of the address
         ipAddr->length = sizeof(Ipv4Addr);

         //Decode IPv4 address
         error = mibDecodeIpv4Addr(oid, oidLen, pos, &ipAddr->ipv4Addr);
      }
#endif
#if (IPV6_SUPPORT == ENABLED)
      //IPv6 address?
      else if(type == INET_ADDR_TYPE_IPV6 && length == sizeof(Ipv6Addr))
      {
         //Save the length of the address
         ipAddr->length = sizeof(Ipv6Addr);

         //Decode IPv6 address
         error = mibDecodeIpv6Addr(oid, oidLen, pos, &ipAddr->ipv6Addr);
      }
#endif
      //Invalid address?
      else
      {
         //Report an error
         error = ERROR_INSTANCE_NOT_FOUND;
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Compare IP addresses
 * @param[in] ipAddr1 First IP address
 * @param[in] ipAddr2 Second IP address
 * @return Comparaison result
 **/

int_t mibCompIpAddr(const IpAddr *ipAddr1, const IpAddr *ipAddr2)
{
   int_t res;

   //Compare length fields
   if(ipAddr1->length < ipAddr2->length)
      return -1;
   else if(ipAddr1->length > ipAddr2->length)
      return 1;
   else if(ipAddr1->length == 0)
      return 0;

   //Compare IP addresses
   res = memcmp((uint8_t *) ipAddr1 + sizeof(size_t),
      (uint8_t *) ipAddr2 + sizeof(size_t), ipAddr1->length);

   //Return comparison result
   return res;
}
