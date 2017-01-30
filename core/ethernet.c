/**
 * @file ethernet.c
 * @brief Ethernet
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
#define TRACE_LEVEL ETH_TRACE_LEVEL

//Dependencies
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "core/net.h"
#include "core/nic.h"
#include "core/ethernet.h"
#include "core/socket.h"
#include "core/raw_socket.h"
#include "core/tcp_timer.h"
#include "ipv4/arp.h"
#include "ipv4/ipv4.h"
#include "ipv6/ipv6.h"
#include "mibs/mib2_module.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (ETH_SUPPORT == ENABLED)

//Unspecified MAC address
const MacAddr MAC_UNSPECIFIED_ADDR = {{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}};
//Broadcast MAC address
const MacAddr MAC_BROADCAST_ADDR = {{{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}};
//Unspecified EUI-64 address
const Eui64 EUI64_UNSPECIFIED_ADDR = {{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}};

//Padding bytes
const uint8_t ethPadding[64] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//A lookup table can be used to speed up CRC calculation
#if (ETH_FAST_CRC_SUPPORT == ENABLED)

static const uint32_t crc32Table[256] =
{
   0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
   0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
   0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
   0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
   0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
   0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
   0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
   0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
   0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
   0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
   0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
   0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
   0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
   0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
   0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
   0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
   0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
   0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
   0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
   0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
   0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
   0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
   0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
   0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
   0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
   0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
   0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
   0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
   0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
   0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
   0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
   0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
   0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
   0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
   0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
   0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
   0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
   0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
   0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
   0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
   0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
   0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
   0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
   0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
   0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
   0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
   0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
   0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
   0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
   0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
   0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
   0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
   0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
   0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
   0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
   0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
   0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
   0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
   0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
   0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
   0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
   0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
   0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
   0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

#endif


/**
 * @brief Ethernet related initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ethInit(NetInterface *interface)
{
   //Clear the MAC filter table contents
   memset(interface->macMulticastFilter, 0,
      sizeof(interface->macMulticastFilter));

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Process an incoming Ethernet frame
 * @param[in] interface Underlying network interface
 * @param[in] ethFrame Incoming Ethernet frame to process
 * @param[in] length Total frame length
 **/

void ethProcessFrame(NetInterface *interface, EthHeader *ethFrame, size_t length)
{
#if (IPV6_SUPPORT == ENABLED)
   NetBuffer1 buffer;
#endif
   uint32_t crc;

   //Total number of octets received on the interface
   MIB2_INC_COUNTER32(interface->mibIfEntry->ifInOctets, length);

   //Ensure the length of the incoming frame is valid
   if(length < sizeof(EthHeader))
   {
      //Number of inbound packets that contained errors
      MIB2_INC_COUNTER32(interface->mibIfEntry->ifInErrors, 1);
      //Discard the received frame
      return;
   }

   //Debug message
   TRACE_DEBUG("Ethernet frame received (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump Ethernet header contents for debugging purpose
   ethDumpHeader(ethFrame);

   //Check whether the CRC is included in the received frame
   if(!interface->nicDriver->autoCrcStrip)
   {
      //Ensure the length of the incoming frame is valid
      if(length < (sizeof(EthHeader) + ETH_CRC_SIZE))
      {
         //Number of inbound packets that contained errors
         MIB2_INC_COUNTER32(interface->mibIfEntry->ifInErrors, 1);
         //Discard the received frame
         return;
      }

      //CRC verification not supported by hardware?
      if(!interface->nicDriver->autoCrcVerif)
      {
         //The value of the residue is 0x2144DF1C when no CRC errors are detected
         if(ethCalcCrc(ethFrame, length) != 0x2144DF1C)
         {
            //Debug message
            TRACE_WARNING("Wrong CRC detected!\r\n");
            //Number of inbound packets that contained errors
            MIB2_INC_COUNTER32(interface->mibIfEntry->ifInErrors, 1);
            //Discard the received frame
            return;
         }
      }

      //Retrieve CRC value
      memcpy(&crc, (uint8_t *) ethFrame + length - ETH_CRC_SIZE, ETH_CRC_SIZE);
      //Reseed the pseudo-random number generator
      netInitRand(crc);
   }

   //Frame filtering based on destination MAC address
   if(ethCheckDestAddr(interface, &ethFrame->destAddr))
   {
      //Number of inbound packets which were chosen to be discarded
      //even though no errors had been detected
      MIB2_INC_COUNTER32(interface->mibIfEntry->ifInDiscards, 1);
      //Discard the received frame
      return;
   }

   //Check whether the destination address is a group address
   if(macIsMulticastAddr(&ethFrame->destAddr))
   {
      //Number of non-unicast packets delivered to a higher-layer protocol
      MIB2_INC_COUNTER32(interface->mibIfEntry->ifInNUcastPkts, 1);
   }
   else
   {
      //Number of unicast packets delivered to a higher-layer protocol
      MIB2_INC_COUNTER32(interface->mibIfEntry->ifInUcastPkts, 1);
   }

#if (RAW_SOCKET_SUPPORT == ENABLED)
   //Allow raw sockets to process Ethernet packets
   rawSocketProcessEthPacket(interface, ethFrame, length - ETH_CRC_SIZE);
#endif

   //Calculate the length of the data payload
   length -= sizeof(EthHeader);

   //Check whether the CRC is included in the received frame
   if(!interface->nicDriver->autoCrcStrip)
      length -= ETH_CRC_SIZE;

   //Check Ethernet type field
   switch(ntohs(ethFrame->type))
   {
#if (IPV4_SUPPORT == ENABLED)
   //ARP packet received?
   case ETH_TYPE_ARP:
      //Process incoming ARP packet
      arpProcessPacket(interface, (ArpPacket *) ethFrame->data, length);
      break;
   //IPv4 packet received?
   case ETH_TYPE_IPV4:
      //Process incoming IPv4 packet
      ipv4ProcessPacket(interface, (Ipv4Header *) ethFrame->data, length);
      break;
#endif

#if (IPV6_SUPPORT == ENABLED)
   //IPv6 packet received?
   case ETH_TYPE_IPV6:
      //The incoming Ethernet frame fits in a single chunk
      buffer.chunkCount = 1;
      buffer.maxChunkCount = 1;
      buffer.chunk[0].address = ethFrame->data;
      buffer.chunk[0].length = length;
      buffer.chunk[0].size = 0;

      //Process incoming IPv6 packet
      ipv6ProcessPacket(interface, (NetBuffer *) &buffer, 0);
      break;
#endif

   //Unknown packet received?
   default:
      //Debug message
      TRACE_WARNING("Unknown Ethernet type!\r\n");

      //Number of packets received via the interface which were
      //discarded because of an unknown or unsupported protocol
      MIB2_INC_COUNTER32(interface->mibIfEntry->ifInUnknownProtos , 1);
      break;
   }
}


/**
 * @brief Send an Ethernet frame
 * @param[in] interface Underlying network interface
 * @param[in] destAddr MAC address of the destination host
 * @param[in] buffer Multi-part buffer containing the payload
 * @param[in] offset Offset to the first payload byte
 * @param[in] type Ethernet type
 * @return Error code
 **/

error_t ethSendFrame(NetInterface *interface, const MacAddr *destAddr,
   NetBuffer *buffer, size_t offset, uint16_t type)
{
   error_t error;
   size_t length;
   EthHeader *header;

   //Is there enough space for the Ethernet header?
   if(offset < sizeof(EthHeader))
      return ERROR_INVALID_PARAMETER;

   //Make room for the Ethernet header
   offset -= sizeof(EthHeader);
   //Retrieve the length of the frame
   length = netBufferGetLength(buffer) - offset;

   //Position to the beginning of the frame
   header = netBufferAt(buffer, offset);

   //Format Ethernet header
   header->destAddr = *destAddr;
   header->srcAddr = interface->macAddr;
   header->type = htons(type);

   //Automatic padding not supported by hardware?
   if(!interface->nicDriver->autoPadding)
   {
      //The host controller should manually add padding
      //to the packet before transmitting it
      if(length < (ETH_MIN_FRAME_SIZE - ETH_CRC_SIZE))
      {
         size_t n;

         //Add padding as necessary
         n = (ETH_MIN_FRAME_SIZE - ETH_CRC_SIZE) - length;

         //Append padding bytes
         error = netBufferAppend(buffer, ethPadding, n);
         //Any error to report?
         if(error)
            return error;

         //Adjust frame length
         length += n;
      }
   }

   //CRC calculation not supported by hardware?
   if(!interface->nicDriver->autoCrcCalc)
   {
      uint32_t crc;

      //Compute CRC over the header and payload
      crc = ethCalcCrcEx(buffer, offset, length);
      //Convert from host byte order to little-endian byte order
      crc = htole32(crc);

      //Append the calculated CRC value
      error = netBufferAppend(buffer, &crc, sizeof(crc));
      //Any error to report?
      if(error)
         return error;

      //Adjust frame length
      length += sizeof(crc);
   }

   //Total number of octets transmitted out of the interface
   MIB2_INC_COUNTER32(interface->mibIfEntry->ifOutOctets, length);

   //Check whether the destination address is a group address
   if(macIsMulticastAddr(&header->destAddr))
   {
      //Total number of non-unicast packets that higher-level protocols
      //requested be transmitted
      MIB2_INC_COUNTER32(interface->mibIfEntry->ifOutNUcastPkts, 1);
   }
   else
   {
      //Total number of unicast packets that higher-level protocols
      //requested be transmitted
      MIB2_INC_COUNTER32(interface->mibIfEntry->ifOutUcastPkts, 1);
   }

   //Debug message
   TRACE_DEBUG("Sending Ethernet frame (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump Ethernet header contents for debugging purpose
   ethDumpHeader(header);

   //Send the resulting packet over the specified link
   error = nicSendPacket(interface, buffer, offset);
   //Return status code
   return error;
}


/**
 * @brief Destination MAC address filtering
 * @param[in] interface Underlying network interface
 * @param[in] macAddr Destination MAC address to be checked
 * @return Error code
 **/

error_t ethCheckDestAddr(NetInterface *interface, const MacAddr *macAddr)
{
   uint_t i;
   MacFilterEntry *entry;

   //Filter out any invalid addresses
   error_t error = ERROR_INVALID_ADDRESS;

   //Host address?
   if(macCompAddr(macAddr, &interface->macAddr))
   {
      error = NO_ERROR;
   }
   //Broadcast address?
   else if(macCompAddr(macAddr, &MAC_BROADCAST_ADDR))
   {
      error = NO_ERROR;
   }
   //Multicast address?
   else if(macIsMulticastAddr(macAddr))
   {
      //Go through the multicast filter table
      for(i = 0; i < MAC_MULTICAST_FILTER_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->macMulticastFilter[i];

         //Valid entry?
         if(entry->refCount > 0)
         {
            //Check whether the destination MAC address matches
            //a relevant multicast address
            if(macCompAddr(&entry->addr, macAddr))
            {
               //The MAC address is acceptable
               error = NO_ERROR;
               //Stop immediately
               break;
            }
         }
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Add a multicast address to the MAC filter table
 * @param[in] interface Underlying network interface
 * @param[in] macAddr Multicast MAC address to accept
 * @return Error code
 **/

error_t ethAcceptMulticastAddr(NetInterface *interface, const MacAddr *macAddr)
{
   uint_t i;
   MacFilterEntry *entry;
   MacFilterEntry *firstFreeEntry;

   //Keep track of the first free entry
   firstFreeEntry = NULL;

   //Go through the multicast filter table
   for(i = 0; i < MAC_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->macMulticastFilter[i];

      //Valid entry?
      if(entry->refCount > 0)
      {
         //Check whether the table already contains the specified MAC address
         if(macCompAddr(&entry->addr, macAddr))
         {
            //Increment the reference count
            entry->refCount++;
            //No error to report
            return NO_ERROR;
         }
      }
      else
      {
         //Keep track of the first free entry
         if(firstFreeEntry == NULL)
            firstFreeEntry = entry;
      }
   }

   //Check whether the multicast filter table is full
   if(firstFreeEntry == NULL)
   {
      //A new entry cannot be added
      return ERROR_FAILURE;
   }

   //Add a new entry to the table
   firstFreeEntry->addr = *macAddr;
   //Initialize the reference count
   firstFreeEntry->refCount = 1;

   //Force the network interface controller to add the current
   //entry to its MAC filter table
   firstFreeEntry->addFlag = TRUE;
   firstFreeEntry->deleteFlag = FALSE;

   //Update the MAC filter table
   nicSetMulticastFilter(interface);

   //Clear the flag
   firstFreeEntry->addFlag = FALSE;

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Remove a multicast address from the MAC filter table
 * @param[in] interface Underlying network interface
 * @param[in] macAddr Multicast MAC address to drop
 * @return Error code
 **/

error_t ethDropMulticastAddr(NetInterface *interface, const MacAddr *macAddr)
{
   uint_t i;
   MacFilterEntry *entry;

   //Go through the multicast filter table
   for(i = 0; i < MAC_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->macMulticastFilter[i];

      //Valid entry?
      if(entry->refCount > 0)
      {
         //Specified MAC address found?
         if(macCompAddr(&entry->addr, macAddr))
         {
            //Decrement the reference count
            entry->refCount--;

            //Remove the entry if the reference count drops to zero
            if(entry->refCount == 0)
            {
               //Force the network interface controller to remove the current
               //entry from its MAC filter table
               entry->deleteFlag = TRUE;

               //Update the MAC filter table
               nicSetMulticastFilter(interface);

               //Clear the flag
               entry->deleteFlag = FALSE;
               //Remove the multicast address from the list
               entry->addr = MAC_UNSPECIFIED_ADDR;
            }

            //No error to report
            return NO_ERROR;
         }
      }
   }

   //The specified MAC address does not exist
   return ERROR_ADDRESS_NOT_FOUND;
}


/**
 * @brief Ethernet CRC calculation
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t ethCalcCrc(const void *data, size_t length)
{
//A lookup table can be used to speed up CRC calculation
#if (ETH_FAST_CRC_SUPPORT == ENABLED)
   uint_t i;

   //Point to the data over which to calculate the CRC
   const uint8_t *p = (uint8_t *) data;
   //CRC preset value
   uint32_t crc = 0xFFFFFFFF;

   //Loop through data
   for(i = 0; i < length; i++)
   {
      //The message is processed byte by byte
      crc = (crc >> 8) ^ crc32Table[(crc & 0xFF) ^ p[i]];
   }

   //Return 1's complement value
   return ~crc;

//Bit by bit CRC calculation
#else
   uint_t i;
   uint_t j;

   //Point to the data over which to calculate the CRC
   const uint8_t *p = (uint8_t *) data;
   //CRC preset value
   uint32_t crc = 0xFFFFFFFF;

   //Loop through data
   for(i = 0; i < length; i++)
   {
      //Update CRC value
      crc ^= p[i];
      //The message is processed bit by bit
      for(j = 0; j < 8; j++)
      {
         if(crc & 0x00000001)
            crc = (crc >> 1) ^ 0xEDB88320;
         else
            crc = crc >> 1;
      }
   }

   //Return 1's complement value
   return ~crc;
#endif
}


/**
 * @brief Calculate CRC over a multi-part buffer
 * @param[in] buffer Pointer to the multi-part buffer
 * @param[in] offset Offset from the beginning of the buffer
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t ethCalcCrcEx(const NetBuffer *buffer, size_t offset, size_t length)
{
   uint_t i;
   uint_t n;
   uint32_t crc;
   uint8_t *p;
#if (ETH_FAST_CRC_SUPPORT == DISABLED)
   uint_t k;
#endif

   //CRC preset value
   crc = 0xFFFFFFFF;

   //Loop through data chunks
   for(i = 0; i < buffer->chunkCount && length > 0; i++)
   {
      //Is there any data to process in the current chunk?
      if(offset < buffer->chunk[i].length)
      {
         //Point to the first data byte
         p = (uint8_t *) buffer->chunk[i].address + offset;
         //Compute the number of bytes to process
         n = MIN(buffer->chunk[i].length - offset, length);
         //Adjust byte counter
         length -= n;

         //Process current chunk
         while(n > 0)
         {
#if (ETH_FAST_CRC_SUPPORT == ENABLED)
            //The message is processed byte by byte
            crc = (crc >> 8) ^ crc32Table[(crc & 0xFF) ^ *p];
#else
            //Update CRC value
            crc ^= *p;

            //The message is processed bit by bit
            for(k = 0; k < 8; k++)
            {
               if(crc & 0x00000001)
                  crc = (crc >> 1) ^ 0xEDB88320;
               else
                  crc = crc >> 1;
            }
#endif
            //Next byte
            p++;
            n--;
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

   //Return 1's complement value
   return ~crc;
}


/**
 * @brief Allocate a buffer to hold an Ethernet frame
 * @param[in] length Desired payload length
 * @param[out] offset Offset to the first byte of the payload
 * @return The function returns a pointer to the newly allocated
 *   buffer. If the system is out of resources, NULL is returned
 **/

NetBuffer *ethAllocBuffer(size_t length, size_t *offset)
{
   NetBuffer *buffer;

   //Allocate a buffer to hold the Ethernet header and the payload
   buffer = netBufferAlloc(length + sizeof(EthHeader));
   //Failed to allocate buffer?
   if(buffer == NULL)
      return NULL;

   //Offset to the first byte of the payload
   *offset = sizeof(EthHeader);

   //Return a pointer to the freshly allocated buffer
   return buffer;
}


/**
 * @brief Convert a string representation of a MAC address to a binary MAC address
 * @param[in] str NULL-terminated string representing the MAC address
 * @param[out] macAddr Binary representation of the MAC address
 * @return Error code
 **/

error_t macStringToAddr(const char_t *str, MacAddr *macAddr)
{
   error_t error;
   int_t i = 0;
   int_t value = -1;

   //Parse input string
   while(1)
   {
      //Hexadecimal digit found?
      if(isxdigit((uint8_t) *str))
      {
         //First digit to be decoded?
         if(value < 0) value = 0;
         //Update the value of the current byte
         if(isdigit((uint8_t) *str))
            value = (value * 16) + (*str - '0');
         else if(isupper((uint8_t) *str))
            value = (value * 16) + (*str - 'A' + 10);
         else
            value = (value * 16) + (*str - 'a' + 10);
         //Check resulting value
         if(value > 0xFF)
         {
            //The conversion failed
            error = ERROR_INVALID_SYNTAX;
            break;
         }
      }
      //Dash or colon separator found?
      else if((*str == '-' || *str == ':') && i < 6)
      {
         //Each separator must be preceded by a valid number
         if(value < 0)
         {
            //The conversion failed
            error = ERROR_INVALID_SYNTAX;
            break;
         }

         //Save the current byte
         macAddr->b[i++] = value;
         //Prepare to decode the next byte
         value = -1;
      }
      //End of string detected?
      else if(*str == '\0' && i == 5)
      {
         //The NULL character must be preceded by a valid number
         if(value < 0)
         {
            //The conversion failed
            error = ERROR_INVALID_SYNTAX;
         }
         else
         {
            //Save the last byte of the MAC address
            macAddr->b[i] = value;
            //The conversion succeeded
            error = NO_ERROR;
         }

         //We are done
         break;
      }
      //Invalid character...
      else
      {
         //The conversion failed
         error = ERROR_INVALID_SYNTAX;
         break;
      }

      //Point to the next character
      str++;
   }

   //Return status code
   return error;
}


/**
 * @brief Convert a MAC address to a dash delimited string
 * @param[in] macAddr Pointer to the MAC address
 * @param[out] str NULL-terminated string representing the MAC address
 * @return Pointer to the formatted string
 **/

char_t *macAddrToString(const MacAddr *macAddr, char_t *str)
{
   static char_t buffer[18];

   //The str parameter is optional
   if(str == NULL)
      str = buffer;

   //Format MAC address
   sprintf(str, "%02" PRIX8 "-%02" PRIX8 "-%02" PRIX8 "-%02" PRIX8 "-%02" PRIX8 "-%02" PRIX8,
      macAddr->b[0], macAddr->b[1], macAddr->b[2], macAddr->b[3], macAddr->b[4], macAddr->b[5]);

   //Return a pointer to the formatted string
   return str;
}


/**
 * @brief Convert a string representation of an EUI-64 address to a binary EUI-64 address
 * @param[in] str NULL-terminated string representing the EUI-64 address
 * @param[out] eui64 Binary representation of the EUI-64 address
 * @return Error code
 **/

error_t eui64StringToAddr(const char_t *str, Eui64 *eui64)
{
   error_t error;
   int_t i = 0;
   int_t value = -1;

   //Parse input string
   while(1)
   {
      //Hexadecimal digit found?
      if(isxdigit((uint8_t) *str))
      {
         //First digit to be decoded?
         if(value < 0) value = 0;
         //Update the value of the current byte
         if(isdigit((uint8_t) *str))
            value = (value * 16) + (*str - '0');
         else if(isupper((uint8_t) *str))
            value = (value * 16) + (*str - 'A' + 10);
         else
            value = (value * 16) + (*str - 'a' + 10);
         //Check resulting value
         if(value > 0xFF)
         {
            //The conversion failed
            error = ERROR_INVALID_SYNTAX;
            break;
         }
      }
      //Dash or colon separator found?
      else if((*str == '-' || *str == ':') && i < 8)
      {
         //Each separator must be preceded by a valid number
         if(value < 0)
         {
            //The conversion failed
            error = ERROR_INVALID_SYNTAX;
            break;
         }

         //Save the current byte
         eui64->b[i++] = value;
         //Prepare to decode the next byte
         value = -1;
      }
      //End of string detected?
      else if(*str == '\0' && i == 7)
      {
         //The NULL character must be preceded by a valid number
         if(value < 0)
         {
            //The conversion failed
            error = ERROR_INVALID_SYNTAX;
         }
         else
         {
            //Save the last byte of the EUI-64 address
            eui64->b[i] = value;
            //The conversion succeeded
            error = NO_ERROR;
         }

         //We are done
         break;
      }
      //Invalid character...
      else
      {
         //The conversion failed
         error = ERROR_INVALID_SYNTAX;
         break;
      }

      //Point to the next character
      str++;
   }

   //Return status code
   return error;
}


/**
 * @brief Convert an EUI-64 address to a dash delimited string
 * @param[in] eui64 Pointer to the EUI-64 address
 * @param[out] str NULL-terminated string representing the EUI-64 address
 * @return Pointer to the formatted string
 **/

char_t *eui64AddrToString(const Eui64 *eui64, char_t *str)
{
   static char_t buffer[24];

   //The str parameter is optional
   if(str == NULL)
      str = buffer;

   //Format EUI-64 identifier
   sprintf(str, "%02" PRIX8 "-%02" PRIX8 "-%02" PRIX8 "-%02" PRIX8
      "-%02" PRIX8 "-%02" PRIX8 "-%02" PRIX8 "-%02" PRIX8,
      eui64->b[0], eui64->b[1], eui64->b[2], eui64->b[3],
      eui64->b[4], eui64->b[5], eui64->b[6], eui64->b[7]);

   //Return a pointer to the formatted string
   return str;
}


/**
 * @brief Map a MAC address to the IPv6 modified EUI-64 identifier
 * @param[in] macAddr Host MAC address
 * @param[out] interfaceId IPv6 modified EUI-64 identifier
 **/

void macAddrToEui64(const MacAddr *macAddr, Eui64 *interfaceId)
{
   //Copy the Organization Unique Identifier (OUI)
   interfaceId->b[0] = macAddr->b[0];
   interfaceId->b[1] = macAddr->b[1];
   interfaceId->b[2] = macAddr->b[2];

   //The middle 16 bits are given the value 0xFFFE
   interfaceId->b[3] = 0xFF;
   interfaceId->b[4] = 0xFE;

   //Copy the right-most 24 bits of the MAC address
   interfaceId->b[5] = macAddr->b[3];
   interfaceId->b[6] = macAddr->b[4];
   interfaceId->b[7] = macAddr->b[5];

   //Modified EUI-64 format interface identifiers are
   //formed by inverting the Universal/Local bit
   interfaceId->b[0] ^= MAC_ADDR_FLAG_LOCAL;
}


/**
 * @brief Dump Ethernet header for debugging purpose
 * @param[in] ethHeader Pointer to the Ethernet header
 **/

void ethDumpHeader(const EthHeader *ethHeader)
{
   //Dump Ethernet header contents
   TRACE_DEBUG("  Dest Addr = %s\r\n", macAddrToString(&ethHeader->destAddr, NULL));
   TRACE_DEBUG("  Src Addr = %s\r\n", macAddrToString(&ethHeader->srcAddr, NULL));
   TRACE_DEBUG("  Type = 0x%04" PRIX16 "\r\n", ntohs(ethHeader->type));
}

#endif
