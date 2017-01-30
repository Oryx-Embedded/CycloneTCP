/**
 * @file ip.c
 * @brief IPv4 and IPv6 common routines
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

//Switch to the appropriate trace level
#define TRACE_LEVEL IP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/ethernet.h"
#include "core/ip.h"
#include "ipv4/ipv4.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_misc.h"
#include "debug.h"

//Special IP address
const IpAddr IP_ADDR_ANY = {0};
const IpAddr IP_ADDR_UNSPECIFIED = {0};


/**
 * @brief Send an IP datagram
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader IP pseudo header
 * @param[in] buffer Multi-part buffer containing the payload
 * @param[in] offset Offset to the first payload byte
 * @param[in] ttl TTL value. Default Time-To-Live is used when this parameter is zero
 * @return Error code
 **/

error_t ipSendDatagram(NetInterface *interface, IpPseudoHeader *pseudoHeader,
   NetBuffer *buffer, size_t offset, uint8_t ttl)
{
   error_t error;

#if (IPV4_SUPPORT == ENABLED)
   //Destination address is an IPv4 address?
   if(pseudoHeader->length == sizeof(Ipv4PseudoHeader))
   {
      //Form an IPv4 packet and send it
      error = ipv4SendDatagram(interface, &pseudoHeader->ipv4Data,
         buffer, offset, ttl);
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //Destination address is an IPv6 address?
   if(pseudoHeader->length == sizeof(Ipv6PseudoHeader))
   {
      //Form an IPv6 packet and send it
      error = ipv6SendDatagram(interface, &pseudoHeader->ipv6Data,
         buffer, offset, ttl);
   }
   else
#endif
   //Destination address is invalid
   {
      //Report an error
      error = ERROR_INVALID_ADDRESS;
   }

   //Return status code
   return error;
}


/**
 * @brief IP source address selection
 *
 * This function selects the source address and the relevant network interface
 * to be used in order to join the specified destination address
 *
 * @param[in,out] interface A pointer to a valid network interface may be provided as
 *   a hint. The function returns a pointer identifying the interface to be used
 * @param[in] destAddr Destination IP address
 * @param[out] srcAddr Local IP address to be used
 * @return Error code
 **/

error_t ipSelectSourceAddr(NetInterface **interface,
   const IpAddr *destAddr, IpAddr *srcAddr)
{
#if (IPV4_SUPPORT == ENABLED)
   //The destination address is an IPv4 address?
   if(destAddr->length == sizeof(Ipv4Addr))
   {
      //An IPv4 address is expected
      srcAddr->length = sizeof(Ipv4Addr);
      //Get the most appropriate source address to use
      return ipv4SelectSourceAddr(interface, destAddr->ipv4Addr, &srcAddr->ipv4Addr);
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //The destination address is an IPv6 address?
   if(destAddr->length == sizeof(Ipv6Addr))
   {
      //An IPv6 address is expected
      srcAddr->length = sizeof(Ipv6Addr);
      //Get the most appropriate source address to use
      return ipv6SelectSourceAddr(interface, &destAddr->ipv6Addr, &srcAddr->ipv6Addr);
   }
   else
#endif
   //The destination address is not valid?
   {
      //Report an error
      return ERROR_INVALID_ADDRESS;
   }
}


/**
 * @brief IP checksum calculation
 * @param[in] data Pointer to the data over which to calculate the IP checksum
 * @param[in] length Number of bytes to process
 * @return Checksum value
 **/

uint16_t ipCalcChecksum(const void *data, size_t length)
{
   //Checksum preset value
   uint32_t checksum = 0x0000;

   //Process all the data
   while(length > 1)
   {
      //Update checksum value
      checksum += *((uint16_t *) data);
      //Point to the next 16-bit word
      data = (uint16_t *) data + 1;
      //Adjust the number of remaining words to process
      length -= 2;
   }

   //Add left-over byte, if any
   if(length > 0)
      checksum += *((uint8_t *) data);

   //Fold 32-bit sum to 16 bits
   while(checksum >> 16)
      checksum = (checksum & 0xFFFF) + (checksum >> 16);

   //Return 1's complement value
   return checksum ^ 0xFFFF;
}


/**
 * @brief Calculate IP checksum over a multi-part buffer
 * @param[in] buffer Pointer to the multi-part buffer
 * @param[in] offset Offset from the beginning of the buffer
 * @param[in] length Number of bytes to process
 * @return Checksum value
 **/

uint16_t ipCalcChecksumEx(const NetBuffer *buffer, size_t offset, size_t length)
{
   uint_t i;
   uint_t m;
   uint_t n;
   bool_t odd;
   uint8_t *data;
   uint32_t checksum;

   //Checksum preset value
   checksum = 0x0000;
   //Total number of bytes processed
   n = 0;

   //Loop through data chunks
   for(i = 0; i < buffer->chunkCount && n < length; i++)
   {
      //Is there any data to process in the current chunk?
      if(offset < buffer->chunk[i].length)
      {
         //Check whether the total number of bytes already processed is odd
         odd = (n & 1) ? TRUE : FALSE;

         //Point to the first data byte
         data = (uint8_t *) buffer->chunk[i].address + offset;

         //Number of bytes available in the current chunk
         m = buffer->chunk[i].length - offset;
         //Limit the number of byte to process
         m = MIN(m, length - n);

         //Now adjust the total length
         n += m;

         //Data buffer is not aligned on 16-bit boundaries?
         if((uint_t) data & 1)
         {
            //The total number of bytes is even?
            if(!odd)
            {
               //Fold 32-bit sum to 16 bits
               while(checksum >> 16)
                  checksum = (checksum & 0xFFFF) + (checksum >> 16);
               //Swap checksum value
               checksum = ((checksum >> 8) | (checksum << 8)) & 0xFFFF;
            }

            //Restore the alignment on 16-bit boundaries
            if(m > 0)
            {
#ifdef _CPU_BIG_ENDIAN
               //Update checksum value
               checksum += *data;
#else
               //Update checksum value
               checksum += *data << 8;
#endif
               //Point to the next byte
               data += 1;
               m -= 1;
            }

            //Process the data 2 bytes at a time
            while(m > 1)
            {
               //Update checksum value
               checksum += *((uint16_t *) data);
               //Point to the next 16-bit word
               data += 2;
               m -= 2;
            }

            //Add left-over byte, if any
            if(m > 0)
            {
#ifdef _CPU_BIG_ENDIAN
               //Update checksum value
               checksum += *data << 8;
#else
               //Update checksum value
               checksum += *data;
#endif
            }

            //Restore checksum endianness
            if(!odd)
            {
               //Fold 32-bit sum to 16 bits
               while(checksum >> 16)
                  checksum = (checksum & 0xFFFF) + (checksum >> 16);
               //Swap checksum value
               checksum = ((checksum >> 8) | (checksum << 8)) & 0xFFFF;
            }
         }
         //Data buffer is aligned on 16-bit boundaries?
         else
         {
            //The total number of bytes is odd?
            if(odd)
            {
               //Fold 32-bit sum to 16 bits
               while(checksum >> 16)
                  checksum = (checksum & 0xFFFF) + (checksum >> 16);
               //Swap checksum value
               checksum = ((checksum >> 8) | (checksum << 8)) & 0xFFFF;
            }

            //Process the data 2 bytes at a time
            while(m > 1)
            {
               //Update checksum value
               checksum += *((uint16_t *) data);
               //Point to the next 16-bit word
               data += 2;
               m -= 2;
            }

            //Add left-over byte, if any
            if(m > 0)
            {
#ifdef _CPU_BIG_ENDIAN
               //Update checksum value
               checksum += *data << 8;
#else
               //Update checksum value
               checksum += *data;
#endif
            }

            //Restore checksum endianness
            if(odd)
            {
               //Fold 32-bit sum to 16 bits
               while(checksum >> 16)
                  checksum = (checksum & 0xFFFF) + (checksum >> 16);
               //Swap checksum value
               checksum = ((checksum >> 8) | (checksum << 8)) & 0xFFFF;
            }
         }

         //Process the next block from the start
         offset = 0;
      }
      else
      {
         //Skip the current chunk
         offset -= buffer->chunk[i].length;
      }
   }

   //Fold 32-bit sum to 16 bits
   while(checksum >> 16)
      checksum = (checksum & 0xFFFF) + (checksum >> 16);

   //Return 1's complement value
   return checksum ^ 0xFFFF;
}


/**
 * @brief Calculate IP upper-layer checksum
 * @param[in] pseudoHeader Pointer to the pseudo header
 * @param[in] pseudoHeaderLength Pseudo header length
 * @param[in] data Pointer to the upper-layer data
 * @param[in] dataLength Upper-layer data length
 * @return Checksum value
 **/

uint16_t ipCalcUpperLayerChecksum(const void *pseudoHeader,
   size_t pseudoHeaderLength, const void *data, size_t dataLength)
{
   //Checksum preset value
   uint32_t checksum = 0x0000;

   //Process pseudo header
   while(pseudoHeaderLength > 1)
   {
      //Update checksum value
      checksum += *((uint16_t *) pseudoHeader);
      //Point to the next 16-bit word
      pseudoHeader = (uint16_t *) pseudoHeader + 1;
      //Adjust the number of remaining words to process
      pseudoHeaderLength -= 2;
   }

   //Process upper-layer data
   while(dataLength > 1)
   {
      //Update checksum value
      checksum += *((uint16_t *) data);
      //Point to the next 16-bit word
      data = (uint16_t *) data + 1;
      //Adjust the number of remaining words to process
      dataLength -= 2;
   }

   //Add left-over byte, if any
   if(dataLength > 0)
   {
#ifdef _CPU_BIG_ENDIAN
      //Update checksum value
      checksum += *((uint8_t *) data) << 8;
#else
      //Update checksum value
      checksum += *((uint8_t *) data);
#endif
   }

   //Fold 32-bit sum to 16 bits
   while(checksum >> 16)
      checksum = (checksum & 0xFFFF) + (checksum >> 16);

   //Return 1's complement value
   return checksum ^ 0xFFFF;
}


/**
 * @brief Calculate IP upper-layer checksum over a multi-part buffer
 * @param[in] pseudoHeader Pointer to the pseudo header
 * @param[in] pseudoHeaderLength Pseudo header length
 * @param[in] buffer Multi-part buffer containing the upper-layer data
 * @param[in] offset Offset from the first data byte to process
 * @param[in] length Number of data bytes to process
 * @return Checksum value
 **/

uint16_t ipCalcUpperLayerChecksumEx(const void *pseudoHeader,
   size_t pseudoHeaderLength, const NetBuffer *buffer, size_t offset, size_t length)
{
   uint32_t checksum;

   //Process upper-layer data
   checksum = ipCalcChecksumEx(buffer, offset, length);
   //Calculate 1's complement value
   checksum = checksum ^ 0xFFFF;

   //Process pseudo header
   while(pseudoHeaderLength > 1)
   {
      //Update checksum value
      checksum += *((uint16_t *) pseudoHeader);
      //Point to the next 16-bit word
      pseudoHeader = (uint16_t *) pseudoHeader + 1;
      //Adjust the number of remaining words to process
      pseudoHeaderLength -= 2;
   }

   //Fold 32-bit sum to 16 bits
   while(checksum >> 16)
      checksum = (checksum & 0xFFFF) + (checksum >> 16);

   //Return 1's complement value
   return checksum ^ 0xFFFF;
}


/**
 * @brief Allocate a buffer to hold an IP packet
 * @param[in] length Desired payload length
 * @param[out] offset Offset to the first byte of the payload
 * @return The function returns a pointer to the newly allocated
 *   buffer. If the system is out of resources, NULL is returned
 **/

NetBuffer *ipAllocBuffer(size_t length, size_t *offset)
{
   size_t headerLength;
   NetBuffer *buffer;

#if (IPV6_SUPPORT == ENABLED)
   //Maximum overhead when using IPv6
   headerLength = sizeof(Ipv6Header) + sizeof(Ipv6FragmentHeader);
#else
   //Maximum overhead when using IPv4
   headerLength = sizeof(Ipv4Header);
#endif

#if (ETH_SUPPORT == ENABLED)
   //Allocate a buffer to hold the Ethernet header and the IP packet
   buffer = ethAllocBuffer(length + headerLength, offset);
#elif (PPP_SUPPORT == ENABLED)
   //Allocate a buffer to hold the PPP header and the IP packet
   buffer = pppAllocBuffer(length + headerLength, offset);
#else
   //Allocate a buffer to hold the IP packet
   buffer = netBufferAlloc(length + headerLength);
   //Clear offset value
   *offset = 0;
#endif

   //Successful memory allocation?
   if(buffer != NULL)
   {
      //Offset to the first byte of the payload
      *offset += headerLength;
   }

   //Return a pointer to the freshly allocated buffer
   return buffer;
}


/**
 * @brief Join the specified host group
 * @param[in] interface Underlying network interface (optional parameter)
 * @param[in] groupAddr IP address identifying the host group to join
 * @return Error code
 **/

error_t ipJoinMulticastGroup(NetInterface *interface, const IpAddr *groupAddr)
{
   error_t error;

   //Use default network interface?
   if(interface == NULL)
      interface = netGetDefaultInterface();

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (IPV4_SUPPORT == ENABLED)
   //IPv4 multicast address?
   if(groupAddr->length == sizeof(Ipv4Addr))
   {
      //Join the specified host group
      error = ipv4JoinMulticastGroup(interface, groupAddr->ipv4Addr);
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 multicast address?
   if(groupAddr->length == sizeof(Ipv6Addr))
   {
      //Join the specified host group
      error = ipv6JoinMulticastGroup(interface, &groupAddr->ipv6Addr);
   }
   else
#endif
   //Invalid IP address?
   {
      //Report an error
      error = ERROR_INVALID_ADDRESS;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Leave the specified host group
 * @param[in] interface Underlying network interface (optional parameter)
 * @param[in] groupAddr IP address identifying the host group to leave
 * @return Error code
 **/

error_t ipLeaveMulticastGroup(NetInterface *interface, const IpAddr *groupAddr)
{
   //Use default network interface?
   if(interface == NULL)
      interface = netGetDefaultInterface();

#if (IPV4_SUPPORT == ENABLED)
   //IPv4 multicast address?
   if(groupAddr->length == sizeof(Ipv4Addr))
   {
      //Drop membership
      return ipv4LeaveMulticastGroup(interface, groupAddr->ipv4Addr);
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 multicast address?
   if(groupAddr->length == sizeof(Ipv6Addr))
   {
      //Drop membership
      return ipv6LeaveMulticastGroup(interface, &groupAddr->ipv6Addr);
   }
   else
#endif
   //Invalid IP address?
   {
      return ERROR_INVALID_ADDRESS;
   }
}


/**
 * @brief Compare an IP address against the unspecified address
 * @param[in] ipAddr IP address
 * @return TRUE if the IP address is unspecified, else FALSE
 **/

bool_t ipIsUnspecifiedAddr(const IpAddr *ipAddr)
{
#if (IPV4_SUPPORT == ENABLED)
   //IPv4 address?
   if(ipAddr->length == sizeof(Ipv4Addr))
   {
      //Compare IPv4 address
      return (ipAddr->ipv4Addr == IPV4_UNSPECIFIED_ADDR) ? TRUE : FALSE;
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 address?
   if(ipAddr->length == sizeof(Ipv6Addr))
   {
      //Compare IPv6 address
      return ipv6CompAddr(&ipAddr->ipv6Addr, &IPV6_UNSPECIFIED_ADDR);
   }
   else
#endif
   //Invalid IP address?
   {
      return FALSE;
   }
}


/**
 * @brief Convert a string representation of an IP address to a binary IP address
 * @param[in] str NULL-terminated string representing the IP address
 * @param[out] ipAddr Binary representation of the IP address
 * @return Error code
 **/

error_t ipStringToAddr(const char_t *str, IpAddr *ipAddr)
{
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 address?
   if(strchr(str, ':'))
   {
      //IPv6 addresses are 16-byte long
      ipAddr->length = sizeof(Ipv6Addr);
      //Convert the string to IPv6 address
      return ipv6StringToAddr(str, &ipAddr->ipv6Addr);
   }
   else
#endif
#if (IPV4_SUPPORT == ENABLED)
   //IPv4 address?
   if(strchr(str, '.'))
   {
      //IPv4 addresses are 4-byte long
      ipAddr->length = sizeof(Ipv4Addr);
      //Convert the string to IPv4 address
      return ipv4StringToAddr(str, &ipAddr->ipv4Addr);
   }
   else
#endif
   //Invalid IP address?
   {
      //Report an error
      return ERROR_FAILURE;
   }
}


/**
 * @brief Convert a binary IP address to a string representation
 * @param[in] ipAddr Binary representation of the IP address
 * @param[out] str NULL-terminated string representing the IP address
 * @return Pointer to the formatted string
 **/

char_t *ipAddrToString(const IpAddr *ipAddr, char_t *str)
{
#if (IPV4_SUPPORT == ENABLED)
   //IPv4 address?
   if(ipAddr->length == sizeof(Ipv4Addr))
   {
      //Convert IPv4 address to string representation
      return ipv4AddrToString(ipAddr->ipv4Addr, str);
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 address?
   if(ipAddr->length == sizeof(Ipv6Addr))
   {
      //Convert IPv6 address to string representation
      return ipv6AddrToString(&ipAddr->ipv6Addr, str);
   }
   else
#endif
   //Invalid IP address?
   {
      static char_t c;
      //The str parameter is optional
      if(!str) str = &c;
      //Properly terminate the string
      str[0] = '\0';
      //Return an empty string
      return str;
   }
}
