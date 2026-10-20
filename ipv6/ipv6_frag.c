/**
 * @file ipv6_frag.c
 * @brief IPv6 fragmentation and reassembly
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2026 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.6.8
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL IPV6_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/ip.h"
#include "core/tcp.h"
#include "core/udp.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_frag.h"
#include "ipv6/icmpv6.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && IPV6_FRAG_SUPPORT == ENABLED)


/**
 * @brief Fragment IPv6 datagram into smaller packets
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader IPv6 pseudo header
 * @param[in] payload Multi-part buffer containing the payload
 * @param[in] payloadOffset Offset to the first payload byte
 * @param[in] pathMtu PMTU value
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 * @return Error code
 **/

error_t ipv6FragmentDatagram(NetInterface *interface,
   const Ipv6PseudoHeader *pseudoHeader, const NetBuffer *payload,
   size_t payloadOffset, size_t pathMtu, NetTxAncillary *ancillary)
{
   error_t error;
   uint32_t id;
   size_t offset;
   size_t length;
   size_t payloadLen;
   size_t fragmentOffset;
   size_t maxFragmentSize;
   NetBuffer *fragment;

   //Number of IP datagrams that would require fragmentation in order to be transmitted
   IPV6_SYSTEM_STATS_INC_COUNTER32(outFragReqds, 1);
   IPV6_IF_STATS_INC_COUNTER32(outFragReqds, 1);

   //Retrieve the length of the payload
   payloadLen = netBufferGetLength(payload) - payloadOffset;

   //Allocate a memory buffer to hold IP fragments
   fragment = ipAllocBuffer(0, &fragmentOffset);
   //Failed to allocate memory?
   if(fragment == NULL)
   {
      return ERROR_OUT_OF_MEMORY;
   }

   //Identification field is used to identify fragments of an original IP
   //datagram
   id = netGenerateRand(interface->netContext);

   //The node should never set its PMTU estimate below the IPv6 minimum link MTU
   pathMtu = MAX(pathMtu, IPV6_DEFAULT_MTU);

   //Determine the maximum payload size for fragmented packets
   maxFragmentSize = pathMtu - sizeof(Ipv6Header) - sizeof(Ipv6FragmentHeader);
   //The size shall be a multiple of 8-byte blocks
   maxFragmentSize -= (maxFragmentSize % 8);

   //Initialize error code
   error = NO_ERROR;

   //Split the payload into multiple IP fragments
   for(offset = 0; offset < payloadLen; offset += length)
   {
      //Flush the contents of the fragment
      error = netBufferSetLength(fragment, fragmentOffset);
      //Sanity check
      if(error)
      {
         break;
      }

      //Process the last fragment?
      if((payloadLen - offset) <= maxFragmentSize)
      {
         //Size of the current fragment
         length = payloadLen - offset;
         //Copy fragment data
         netBufferConcat(fragment, payload, payloadOffset + offset, length);

         //Do not set the MF flag for the last fragment
         error = ipv6SendPacket(interface, pseudoHeader, id, offset, fragment,
            fragmentOffset, ancillary);
      }
      else
      {
         //Size of the current fragment (must be a multiple of 8-byte blocks)
         length = maxFragmentSize;
         //Copy fragment data
         netBufferConcat(fragment, payload, payloadOffset + offset, length);

         //Fragmented packets must have the M flag set
         error = ipv6SendPacket(interface, pseudoHeader, id, IPV6_FLAG_M |
            offset, fragment, fragmentOffset, ancillary);
      }

      //Failed to send current IP fragment?
      if(error)
      {
         break;
      }

      //Number of IP datagram fragments that have been generated as a result
      //of fragmentation at this entity
      IPV6_SYSTEM_STATS_INC_COUNTER32(outFragCreates, 1);
      IPV6_IF_STATS_INC_COUNTER32(outFragCreates, 1);
   }

   //Check status code
   if(error)
   {
      //Number of IP datagrams that have been discarded because they needed
      //to be fragmented at this entity but could not be
      IPV6_SYSTEM_STATS_INC_COUNTER32(outFragFails, 1);
      IPV6_IF_STATS_INC_COUNTER32(outFragFails, 1);
   }
   else
   {
      //Number of IP datagrams that have been successfully fragmented at this
      //entity
      IPV6_SYSTEM_STATS_INC_COUNTER32(outFragOKs, 1);
      IPV6_IF_STATS_INC_COUNTER32(outFragOKs, 1);
   }

   //Free previously allocated memory
   netBufferFree(fragment);
   //Return status code
   return error;
}


/**
 * @brief Parse Fragment header and reassemble original datagram
 * @param[in] interface Underlying network interface
 * @param[in] ipPacket Multi-part buffer containing the incoming IPv6 packet
 * @param[in] ipPacketOffset Offset to the first byte of the IPv6 packet
 * @param[in] fragHeaderOffset Offset to the Fragment header
 * @param[in] nextHeaderOffset Offset to the Next Header field of the previous header
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 **/

void ipv6ParseFragmentHeader(NetInterface *interface, const NetBuffer *ipPacket,
   size_t ipPacketOffset, size_t fragHeaderOffset, size_t nextHeaderOffset,
   NetRxAncillary *ancillary)
{
   error_t error;
   size_t n;
   size_t length;
   uint16_t offset;
   uint16_t dataFirst;
   uint16_t dataLast;
   Ipv6FragDesc *frag;
   Ipv6HoleDesc *hole;
   Ipv6HoleDesc *prevHole;
   Ipv6Header *ipHeader;
   Ipv6FragmentHeader *fragHeader;

   //Number of IP fragments received which needed to be reassembled
   IPV6_SYSTEM_STATS_INC_COUNTER32(reasmReqds, 1);
   IPV6_IF_STATS_INC_COUNTER32(reasmReqds, 1);

   //Drop nested fragments
   if(ancillary->reassembled)
   {
      //Number of failures detected by the IP reassembly algorithm
      IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
      IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);

      //Drop the incoming fragment
      return;
   }

   //Remaining bytes to process in the payload
   length = netBufferGetLength(ipPacket) - fragHeaderOffset;

   //Ensure the fragment header is valid
   if(length < sizeof(Ipv6FragmentHeader))
   {
      //Number of failures detected by the IP reassembly algorithm
      IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
      IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);

      //Drop the incoming fragment
      return;
   }

   //Point to the IPv6 header
   ipHeader = netBufferAt(ipPacket, ipPacketOffset, sizeof(Ipv6Header));
   //Malformed IPv6 packet?
   if(ipHeader == NULL)
   {
      return;
   }

   //Point to the Fragment header
   fragHeader = netBufferAt(ipPacket, fragHeaderOffset,
      sizeof(Ipv6FragmentHeader));
   //Malformed IPv6 packet?
   if(fragHeader == NULL)
   {
      return;
   }

   //Calculate the length of the fragment
   length -= sizeof(Ipv6FragmentHeader);
   //Convert the fragment offset from network byte order
   offset = ntohs(fragHeader->fragmentOffset);

   //Every fragment except the last must contain a multiple of 8 bytes of data
   if((offset & IPV6_FLAG_M) != 0 && (length % 8) != 0)
   {
      //Number of failures detected by the IP reassembly algorithm
      IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
      IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);

      //Compute the offset of the Payload Length field within the packet
      n = (uint8_t *) &ipHeader->payloadLen - (uint8_t *) ipHeader;

      //The fragment must be discarded and an ICMP Parameter Problem
      //message should be sent to the source of the fragment, pointing
      //to the Payload Length field of the fragment packet
      icmpv6SendErrorMessage(interface, ICMPV6_TYPE_PARAM_PROBLEM,
         ICMPV6_CODE_INVALID_HEADER_FIELD, n, ipPacket, ipPacketOffset);

      //Drop the incoming fragment
      return;
   }

   //Calculate the index of the first byte
   dataFirst = offset & IPV6_OFFSET_MASK;
   //Calculate the index immediately following the last byte
   dataLast = dataFirst + (uint16_t) length;

   //Check for potential integer overflow
   if(dataLast < dataFirst)
   {
      //Number of failures detected by the IP reassembly algorithm
      IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
      IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);

      //Drop the incoming fragment
      return;
   }

   //A Fragment header with the Fragment Offset field and the M flag both
   //set to zero denotes an atomic fragment
   if((offset & (IPV6_OFFSET_MASK | IPV6_FLAG_M)) == 0)
   {
      //Atomic fragments must be processed in isolation from any other packets
      //or fragments (refer to RFC 6946, section 4)
      ipv6ProcessAtomicFragment(interface, ipPacket, ipPacketOffset,
         fragHeaderOffset, nextHeaderOffset, length, ancillary);

      //Exit immediately
      return;
   }

   //Search for a matching IP datagram being reassembled
   frag = ipv6SearchFragQueue(interface, ipHeader, fragHeader);

   //No matching entry in the reassembly queue?
   if(frag == NULL)
   {
      //Number of failures detected by the IP reassembly algorithm
      IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
      IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);

      //Drop the incoming fragment
      return;
   }

   //An empty fragment can never close a hole. Such a fragment causes the whole
   //datagram being reassembled to be discarded
   if(dataFirst == dataLast)
   {
      //Drop the reconstructed datagram
      ipv6DropFragDatagram(interface, frag);
      //Exit immediately
      return;
   }

   //The end of the datagram is known as soon as the last fragment has been
   //received. A last fragment that ends before data already received, or a
   //fragment located beyond the end of the datagram, causes the whole datagram
   //to be discarded
   if(((offset & IPV6_FLAG_M) == 0 && dataLast < frag->fragPartLen) ||
      (frag->lastFragReceived && dataLast > frag->fragPartLen))
   {
      //Drop the reconstructed datagram
      ipv6DropFragDatagram(interface, frag);
      //Exit immediately
      return;
   }

   //Check whether the fragment overlaps data already received
   error = ipv6CheckFragOverlap(interface, frag, dataFirst, dataLast);
   //Any error to report?
   if(error)
   {
      return;
   }

   //The very first fragment requires special handling
   if((offset & IPV6_OFFSET_MASK) == 0)
   {
      uint8_t *p;

      //The first fragment must include the entire IPv6 header chain
      error = ipv6CheckFragHeaderChain(interface, ipPacket, ipPacketOffset,
         fragHeaderOffset);

      //Invalid header chain?
      if(error)
      {
         //Drop the reconstructed datagram
         ipv6DropFragDatagram(interface, frag);
         //Exit immediately
         return;
      }

      //Calculate the length of the unfragmentable part
      frag->unfragPartLen = fragHeaderOffset - ipPacketOffset;

      //The size of the reconstructed datagram exceeds the maximum value?
      if((frag->unfragPartLen + frag->fragPartLen) > IPV6_MAX_FRAG_DATAGRAM_SIZE)
      {
         //Retrieve the offset of the Fragment header within the packet
         n = fragHeaderOffset - ipPacketOffset;
         //Compute the exact offset of the Fragment Offset field
         n += (uint8_t *) &fragHeader->fragmentOffset - (uint8_t *) fragHeader;

         //The fragment must be discarded and an ICMP Parameter Problem
         //message should be sent to the source of the fragment, pointing
         //to the Fragment Offset field of the fragment packet
         icmpv6SendErrorMessage(interface, ICMPV6_TYPE_PARAM_PROBLEM,
            ICMPV6_CODE_INVALID_HEADER_FIELD, n, ipPacket, ipPacketOffset);

         //Drop the reconstructed datagram
         ipv6DropFragDatagram(interface, frag);
         //Exit immediately
         return;
      }

      //Make sure the unfragmentable part entirely fits in the first chunk
      if(frag->unfragPartLen > frag->buffer.chunk[0].size)
      {
         //Drop the reconstructed datagram
         ipv6DropFragDatagram(interface, frag);
         //Exit immediately
         return;
      }

      //Fix the length of the first chunk
      frag->buffer.chunk[0].length = (uint16_t) frag->unfragPartLen;

      //The unfragmentable part of the reassembled packet consists of all
      //headers up to, but not including, the Fragment header of the first
      //fragment packet
      netBufferCopy((NetBuffer *) &frag->buffer, 0, ipPacket,
         ipPacketOffset, frag->unfragPartLen);

      //Point to the Next Header field of the last header
      p = netBufferAt((NetBuffer *) &frag->buffer,
         nextHeaderOffset - ipPacketOffset, 0);

      //The Next Header field of the last header of the unfragmentable part is
      //obtained from the Next Header field of the first fragment's Fragment
      //header
      *p = fragHeader->nextHeader;
   }

   //It may be necessary to increase the size of the buffer
   if(dataLast > frag->fragPartLen)
   {
      //The size of the reconstructed datagram exceeds the maximum value?
      if((frag->unfragPartLen + dataLast) > IPV6_MAX_FRAG_DATAGRAM_SIZE)
      {
         //Retrieve the offset of the Fragment header within the packet
         n = fragHeaderOffset - ipPacketOffset;
         //Compute the exact offset of the Fragment Offset field
         n += (uint8_t *) &fragHeader->fragmentOffset - (uint8_t *) fragHeader;

         //The fragment must be discarded and an ICMP Parameter Problem
         //message should be sent to the source of the fragment, pointing
         //to the Fragment Offset field of the fragment packet
         icmpv6SendErrorMessage(interface, ICMPV6_TYPE_PARAM_PROBLEM,
            ICMPV6_CODE_INVALID_HEADER_FIELD, n, ipPacket, ipPacketOffset);

         //Drop the reconstructed datagram
         ipv6DropFragDatagram(interface, frag);
         //Exit immediately
         return;
      }

      //Adjust the size of the reconstructed datagram
      error = netBufferSetLength((NetBuffer *) &frag->buffer,
         frag->unfragPartLen + dataLast + sizeof(Ipv6HoleDesc));

      //Any error to report?
      if(error)
      {
         //Drop the reconstructed datagram
         ipv6DropFragDatagram(interface, frag);
         //Exit immediately
         return;
      }

      //Actual length of the fragmentable part
      frag->fragPartLen = dataLast;
   }

   //Select the first hole descriptor from the list
   hole = ipv6FindHole(frag, frag->firstHole);
   //Keep track of the previous hole in the list
   prevHole = NULL;

   //Iterate through the hole descriptors
   while(hole != NULL)
   {
      //Save lower and upper boundaries for later use
      uint16_t holeFirst = hole->first;
      uint16_t holeLast = hole->last;

      //Check whether the newly arrived fragment interacts with this hole
      //in some way
      if(dataFirst < holeLast && dataLast > holeFirst)
      {
         //The current descriptor is no longer valid. We will destroy it, and in
         //the next two steps, we will determine whether or not it is necessary
         //to create any new hole descriptors
         if(prevHole != NULL)
         {
            prevHole->next = hole->next;
         }
         else
         {
            frag->firstHole = hole->next;
         }

         //Is there still a hole at the beginning of the segment?
         if(dataFirst > holeFirst)
         {
            //Create a new entry that describes this hole
            hole = ipv6InsertHole(frag, prevHole, holeFirst, dataFirst);

            //Make sure the hole descriptor is valid
            if(hole == NULL)
            {
               //Drop the reconstructed datagram
               ipv6DropFragDatagram(interface, frag);
               //Exit immediately
               return;
            }

            //Always keep track of the previous hole
            prevHole = hole;
         }

         //Is there still a hole at the end of the segment?
         if(dataLast < holeLast && (offset & IPV6_FLAG_M) != 0)
         {
            //Create a new entry that describes this hole
            hole = ipv6InsertHole(frag, prevHole, dataLast, holeLast);

            //Make sure the hole descriptor is valid
            if(hole == NULL)
            {
               //Drop the reconstructed datagram
               ipv6DropFragDatagram(interface, frag);
               //Exit immediately
               return;
            }

            //Always keep track of the previous hole
            prevHole = hole;
         }
      }
      else
      {
         //The newly arrived fragment does not interact with the current hole
         prevHole = hole;
      }

      //Select the next hole descriptor from the list
      hole = ipv6FindHole(frag, prevHole ? prevHole->next : frag->firstHole);
   }

   //Last fragment?
   if((offset & IPV6_FLAG_M) == 0)
   {
      frag->lastFragReceived = TRUE;
   }

   //Copy data from the fragment to the reassembly buffer
   netBufferCopy((NetBuffer *) &frag->buffer, frag->unfragPartLen + dataFirst,
      ipPacket, fragHeaderOffset + sizeof(Ipv6FragmentHeader), length);

   //Dump hole descriptor list
   ipv6DumpHoleList(frag);

   //If the hole descriptor list is empty, the reassembly process is now
   //complete
   if(ipv6FindHole(frag, frag->firstHole) == NULL)
   {
      //Discard the extra hole descriptor that follows the reconstructed
      //datagram
      error = netBufferSetLength((NetBuffer *) &frag->buffer,
         frag->unfragPartLen + frag->fragPartLen);

      //Check status code
      if(error)
      {
         //Number of failures detected by the IP reassembly algorithm
         IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
         IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);
      }
      else
      {
         //Point to the IPv6 header
         Ipv6Header *datagram = netBufferAt((NetBuffer *) &frag->buffer, 0, 0);

         //Fix the Payload Length field
         datagram->payloadLen = htons(frag->unfragPartLen +
            frag->fragPartLen - sizeof(Ipv6Header));

         //Number of IP datagrams successfully reassembled
         IPV6_SYSTEM_STATS_INC_COUNTER32(reasmOKs, 1);
         IPV6_IF_STATS_INC_COUNTER32(reasmOKs, 1);

         //The IPv6 datagram has been reassembled
         ancillary->reassembled = TRUE;

         //The Router Alert option is reported only if present in the
         //Hop-by-Hop Options header of the reassembled datagram
         ancillary->routerAlert = FALSE;

         //Pass the original IPv6 datagram to the higher protocol layer
         ipv6ProcessPacket(interface, (NetBuffer *) &frag->buffer, 0,
            ancillary);
      }

      //Release previously allocated memory
      netBufferSetLength((NetBuffer *) &frag->buffer, 0);
   }
}


/**
 * @brief Process an atomic fragment
 * @param[in] interface Underlying network interface
 * @param[in] ipPacket Multi-part buffer that holds the IPv6 packet
 * @param[in] ipPacketOffset Offset to the first byte of the IPv6 packet
 * @param[in] fragHeaderOffset Offset to the Fragment header
 * @param[in] nextHeaderOffset Offset to the Next Header field of the previous
 *   header
 * @param[in] length Length of the data that follows the Fragment header
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 **/

void ipv6ProcessAtomicFragment(NetInterface *interface,
   const NetBuffer *ipPacket, size_t ipPacketOffset, size_t fragHeaderOffset,
   size_t nextHeaderOffset, size_t length, NetRxAncillary *ancillary)
{
   error_t error;
   size_t unfragPartLen;
   uint8_t *p;
   NetBuffer *buffer;
   Ipv6Header *ipHeader;
   Ipv6FragmentHeader *fragHeader;

   //Point to the Fragment header
   fragHeader = netBufferAt(ipPacket, fragHeaderOffset,
      sizeof(Ipv6FragmentHeader));
   //Malformed IPv6 packet?
   if(fragHeader == NULL)
   {
      return;
   }

   //An atomic fragment is also a first fragment, so it must include the
   //entire IPv6 header chain
   error = ipv6CheckFragHeaderChain(interface, ipPacket, ipPacketOffset,
      fragHeaderOffset);

   //Invalid header chain?
   if(error)
   {
      //Number of failures detected by the IP reassembly algorithm
      IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
      IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);

      //Drop the incoming fragment
      return;
   }

   //Calculate the length of the unfragmentable part
   unfragPartLen = fragHeaderOffset - ipPacketOffset;

   //Allocate a buffer to hold the unfragmentable part and the data that
   //follows the Fragment header
   buffer = netBufferAlloc(unfragPartLen + length);

   //Failed to allocate memory?
   if(buffer == NULL)
   {
      //Number of failures detected by the IP reassembly algorithm
      IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
      IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);

      //Exit immediately
      return;
   }

   //Copy the unfragmentable part
   error = netBufferCopy(buffer, 0, ipPacket, ipPacketOffset, unfragPartLen);

   //Check status code
   if(!error)
   {
      //Copy the data that follows the Fragment header
      error = netBufferCopy(buffer, unfragPartLen, ipPacket,
         fragHeaderOffset + sizeof(Ipv6FragmentHeader), length);
   }

   //Check status code
   if(!error)
   {
      //Point to the Next Header field of the last header of the unfragmentable
      //part
      p = netBufferAt(buffer, nextHeaderOffset - ipPacketOffset,
         sizeof(uint8_t));

      //Point to the IPv6 header
      ipHeader = netBufferAt(buffer, 0, sizeof(Ipv6Header));

      //Sanity check
      if(p != NULL && ipHeader != NULL)
      {
         //The Fragment header is removed from the header chain
         *p = fragHeader->nextHeader;

         //Fix the Payload Length field
         ipHeader->payloadLen = htons(unfragPartLen + length -
            sizeof(Ipv6Header));

         //Number of IP datagrams successfully reassembled
         IPV6_SYSTEM_STATS_INC_COUNTER32(reasmOKs, 1);
         IPV6_IF_STATS_INC_COUNTER32(reasmOKs, 1);

         //The Fragment header has been processed
         ancillary->reassembled = TRUE;

         //The Router Alert option is reported only if present in the Hop-by-Hop
         //Options header of the resulting packet
         ancillary->routerAlert = FALSE;

         //Pass the packet to the higher protocol layer
         ipv6ProcessPacket(interface, buffer, 0, ancillary);
      }
      else
      {
         //Report an error
         error = ERROR_INVALID_PACKET;
      }
   }

   //Any error to report?
   if(error)
   {
      //Number of failures detected by the IP reassembly algorithm
      IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
      IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);
   }

   //Release previously allocated memory
   netBufferFree(buffer);
}


/**
 * @brief Check the IPv6 header chain of a first fragment
 * @param[in] interface Underlying network interface
 * @param[in] ipPacket Multi-part buffer that holds the IPv6 packet
 * @param[in] ipPacketOffset Offset to the first byte of the IPv6 packet
 * @param[in] fragHeaderOffset Offset to the Fragment header
 * @return Error code (ERROR_INVALID_LENGTH if the header chain is incomplete,
 *   ERROR_INVALID_HEADER if a Hop-by-Hop Options header is found in the
 *   fragmentable part, ERROR_INVALID_PACKET if the packet is malformed)
 **/

error_t ipv6CheckFragHeaderChain(NetInterface *interface,
   const NetBuffer *ipPacket, size_t ipPacketOffset, size_t fragHeaderOffset)
{
   error_t error;
   size_t i;
   size_t n;
   size_t length;
   size_t nextHeaderOffset;
   uint8_t type;
   Ipv6ExtHeader *extHeader;

   //Total length of the packet
   length = netBufferGetLength(ipPacket);

   //Point to the Fragment header
   extHeader = netBufferAt(ipPacket, fragHeaderOffset,
      sizeof(Ipv6FragmentHeader));
   //Malformed IPv6 packet?
   if(extHeader == NULL)
   {
      return ERROR_INVALID_PACKET;
   }

   //The Next Header field of the Fragment header identifies the first header
   //of the fragmentable part
   type = extHeader->nextHeader;

   //Keep track of the Next Header field that identifies the current header
   nextHeaderOffset = fragHeaderOffset;
   //Point to the first header of the fragmentable part
   i = fragHeaderOffset + sizeof(Ipv6FragmentHeader);

   //Loop through the headers of the fragmentable part
   while(1)
   {
      //Hop-by-Hop Options header?
      if(type == IPV6_HOP_BY_HOP_OPT_HEADER)
      {
         //The Hop-by-Hop Options header, when present, must immediately follow
         //the IPv6 header
         error = ERROR_INVALID_HEADER;
         break;
      }
      else if(type == IPV6_DEST_OPT_HEADER ||
         type == IPV6_ROUTING_HEADER ||
         type == IPV6_AH_HEADER ||
         type == IPV6_FRAGMENT_HEADER)
      {
         //The Next Header and length fields must be present
         if((i + sizeof(Ipv6ExtHeader)) > length)
         {
            error = ERROR_INVALID_LENGTH;
            break;
         }

         //Point to the extension header
         extHeader = netBufferAt(ipPacket, i, sizeof(Ipv6ExtHeader));
         //Malformed IPv6 packet?
         if(extHeader == NULL)
         {
            error = ERROR_INVALID_PACKET;
            break;
         }

         //Calculate the length of the extension header
         if(type == IPV6_FRAGMENT_HEADER)
         {
            n = sizeof(Ipv6FragmentHeader);
         }
         else if(type == IPV6_AH_HEADER)
         {
            n = (extHeader->hdrExtLen + 2) * 4;
         }
         else
         {
            n = (extHeader->hdrExtLen + 1) * 8;
         }

         //Truncated extension header?
         if((i + n) > length)
         {
            error = ERROR_INVALID_LENGTH;
            break;
         }

         //Each extension header is identified by the Next Header field of the
         //preceding header
         type = extHeader->nextHeader;

         //Keep track of the Next Header field that identifies the next header
         nextHeaderOffset = i;
         //Point to the next header
         i += n;
      }
      else
      {
         //Minimum length of the upper-layer header
         if(type == IPV6_ICMPV6_HEADER)
         {
            n = sizeof(Icmpv6Header);
         }
         else if(type == IPV6_TCP_HEADER)
         {
            n = sizeof(TcpHeader);
         }
         else if(type == IPV6_UDP_HEADER)
         {
            n = sizeof(UdpHeader);
         }
         else if(type == IPV6_NO_NEXT_HEADER)
         {
            n = 0;
         }
         else
         {
            n = 1;
         }

         //The header chain is complete if the upper-layer header lies within
         //the fragment
         if((i + n) <= length)
         {
            error = NO_ERROR;
         }
         else
         {
            error = ERROR_INVALID_LENGTH;
         }

         //End of the header chain
         break;
      }
   }

   //Check status code
   if(error == ERROR_INVALID_HEADER)
   {
      //Compute the offset of the unrecognized Next Header field within the
      //packet
      n = nextHeaderOffset - ipPacketOffset;

      //Send an ICMPv6 Parameter Problem message to the source of the fragment
      icmpv6SendErrorMessage(interface, ICMPV6_TYPE_PARAM_PROBLEM,
         ICMPV6_CODE_UNKNOWN_NEXT_HEADER, n, ipPacket, ipPacketOffset);
   }
   else if(error == ERROR_INVALID_LENGTH)
   {
      //A first fragment that does not include the entire IPv6 header chain
      //must be discarded, and an ICMPv6 Parameter Problem message with code 3
      //should be sent to the source of the fragment (refer to RFC 7112,
      //section 5)
      icmpv6SendErrorMessage(interface, ICMPV6_TYPE_PARAM_PROBLEM,
         ICMPV6_CODE_INCOMPLETE_HEADER_CHAIN, 0, ipPacket, ipPacketOffset);
   }
   else
   {
      //Just for sanity
   }

   //Return status code
   return error;
}


/**
 * @brief Fragment reassembly timeout handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * handle IPv6 fragment reassembly timeout
 *
 * @param[in] interface Underlying network interface
 **/

void ipv6FragTick(NetInterface *interface)
{
   error_t error;
   uint_t i;
   systime_t time;
   Ipv6HoleDesc *hole;

   //Get current time
   time = osGetSystemTime();

   //Loop through the reassembly queue
   for(i = 0; i < IPV6_MAX_FRAG_DATAGRAMS; i++)
   {
      //Point to the current entry in the reassembly queue
      Ipv6FragDesc *frag = &interface->ipv6Context.fragQueue[i];

      //Make sure the entry is currently in use
      if(frag->buffer.chunkCount > 0)
      {
         //If the timer runs out, the partially-reassembled datagram must be
         //discarded and ICMPv6 Time Exceeded message sent to the source host
         if((time - frag->timestamp) >= IPV6_FRAG_TIME_TO_LIVE)
         {
            //Debug message
            TRACE_INFO("IPv6 fragment reassembly timeout...\r\n");
            //Dump IP header contents for debugging purpose
            ipv6DumpHeader(frag->buffer.chunk[0].address);

            //Number of failures detected by the IP reassembly algorithm
            IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
            IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);

            //Point to the first hole descriptor
            hole = ipv6FindHole(frag, frag->firstHole);

            //Make sure the fragment zero has been received before sending an
            //ICMPv6 message
            if(hole != NULL && hole->first > 0)
            {
               //Fix the size of the reconstructed datagram
               error = netBufferSetLength((NetBuffer *) &frag->buffer,
                  frag->unfragPartLen + hole->first);

               //Check status code
               if(!error)
               {
                  //Send an ICMPv6 Time Exceeded message
                  icmpv6SendErrorMessage(interface, ICMPV6_TYPE_TIME_EXCEEDED,
                     ICMPV6_CODE_REASSEMBLY_TIME_EXCEEDED, 0,
                     (NetBuffer *) &frag->buffer, 0);
               }
            }

            //Drop the partially reconstructed datagram
            netBufferSetLength((NetBuffer *) &frag->buffer, 0);
         }
      }
   }
}


/**
 * @brief Search for a matching datagram in the reassembly queue
 * @param[in] interface Underlying network interface
 * @param[in] packet Incoming IPv6 packet
 * @param[in] header Pointer to the Fragment header
 * @return Matching fragment descriptor
 **/

Ipv6FragDesc *ipv6SearchFragQueue(NetInterface *interface,
   const Ipv6Header *packet, const Ipv6FragmentHeader *header)
{
   error_t error;
   uint_t i;
   systime_t time;
   Ipv6Header *datagram;
   Ipv6FragDesc *frag;
   Ipv6FragDesc *oldestFrag;
   Ipv6HoleDesc *hole;

   //Search for a matching IP datagram being reassembled
   for(i = 0; i < IPV6_MAX_FRAG_DATAGRAMS; i++)
   {
      //Point to the current entry in the reassembly queue
      frag = &interface->ipv6Context.fragQueue[i];

      //Check whether the current entry is used?
      if(frag->buffer.chunkCount > 0)
      {
         //Point to the corresponding datagram
         datagram = netBufferAt((NetBuffer *) &frag->buffer, 0, 0);

         //Check source and destination addresses
         if(!ipv6CompAddr(&datagram->srcAddr, &packet->srcAddr))
         {
            continue;
         }

         if(!ipv6CompAddr(&datagram->destAddr, &packet->destAddr))
         {
            continue;
         }

         //Compare fragment identification fields
         if(frag->identification != header->identification)
         {
            continue;
         }

         //A matching entry has been found in the reassembly queue
         return frag;
      }
   }

   //Get current time
   time = osGetSystemTime();
   //Keep track of the oldest entry
   oldestFrag = NULL;

   //Check whether the reassembly queue is full
   for(i = 0; i < IPV6_MAX_FRAG_DATAGRAMS; i++)
   {
      //Point to the current entry in the reassembly queue
      frag = &interface->ipv6Context.fragQueue[i];

      //Check whether the current entry is free
      if(frag->buffer.chunkCount == 0)
      {
         break;
      }

      //Keep track of the oldest entry
      if(oldestFrag == NULL)
      {
         oldestFrag = frag;
      }
      else if((time - frag->timestamp) > (time - oldestFrag->timestamp))
      {
         oldestFrag = frag;
      }
      else
      {
      }
   }

   //Reassembly queue full?
   if(i >= IPV6_MAX_FRAG_DATAGRAMS)
   {
      //Evict the oldest datagram being reassembled to make room for the new
      //one, so that incomplete datagrams cannot block the reassembly process
      //until they time out
      if(oldestFrag != NULL)
      {
         ipv6DropFragDatagram(interface, oldestFrag);
      }
   }

   //If the current packet does not match an existing entry in the reassembly
   //queue, then create a new entry
   for(i = 0; i < IPV6_MAX_FRAG_DATAGRAMS; i++)
   {
      //Point to the current entry in the reassembly queue
      frag = &interface->ipv6Context.fragQueue[i];

      //Check whether the current entry is free
      if(frag->buffer.chunkCount == 0)
      {
         //Number of chunks that comprise the reassembly buffer
         frag->buffer.maxChunkCount = arraysize(frag->buffer.chunk);

         //Allocate sufficient memory to hold the IPv6 header and the first
         //hole descriptor
         error = netBufferSetLength((NetBuffer *) &frag->buffer,
            NET_MEM_POOL_BUFFER_SIZE + sizeof(Ipv6HoleDesc));

         //Failed to allocate memory?
         if(error)
         {
            //Clean up side effects
            netBufferSetLength((NetBuffer *) &frag->buffer, 0);
            //Exit immediately
            return NULL;
         }

         //Initial length of the reconstructed datagram
         frag->unfragPartLen = sizeof(Ipv6Header);
         frag->fragPartLen = 0;
         frag->lastFragReceived = FALSE;

         //Fix the length of the first chunk
         frag->buffer.chunk[0].length = (uint16_t) frag->unfragPartLen;
         //Copy IPv6 header from the incoming fragment
         netBufferWrite((NetBuffer *) &frag->buffer, 0, packet, frag->unfragPartLen);

         //Save current time
         frag->timestamp = osGetSystemTime();
         //Record fragment identification field
         frag->identification = header->identification;
         //Create a new entry in the hole descriptor list
         frag->firstHole = 0;

         //Point to first hole descriptor
         hole = ipv6FindHole(frag, frag->firstHole);

         //Make sure the hole descriptor is valid
         if(hole == NULL)
         {
            //Clean up side effects
            netBufferSetLength((NetBuffer *) &frag->buffer, 0);
            //Exit immediately
            return NULL;
         }

         //The entry describes the datagram as being completely missing
         hole->first = 0;
         hole->last = IPV6_INFINITY;
         hole->next = IPV6_INFINITY;

         //Dump hole descriptor list
         ipv6DumpHoleList(frag);

         //Return the matching fragment descriptor
         return frag;
      }
   }

   //Failed to create a new entry
   return NULL;
}


/**
 * @brief Flush IPv6 reassembly queue
 * @param[in] interface Underlying network interface
 **/

void ipv6FlushFragQueue(NetInterface *interface)
{
   uint_t i;

   //Loop through the reassembly queue
   for(i = 0; i < IPV6_MAX_FRAG_DATAGRAMS; i++)
   {
      //Drop any partially reconstructed datagram
      netBufferSetLength((NetBuffer *) &interface->ipv6Context.fragQueue[i].buffer, 0);
   }
}


/**
 * @brief Drop the datagram being reassembled
 * @param[in] interface Underlying network interface
 * @param[in] frag IPv6 fragment descriptor
 **/

void ipv6DropFragDatagram(NetInterface *interface, Ipv6FragDesc *frag)
{
   //Number of failures detected by the IP reassembly algorithm
   IPV6_SYSTEM_STATS_INC_COUNTER32(reasmFails, 1);
   IPV6_IF_STATS_INC_COUNTER32(reasmFails, 1);

   //Release the reassembly buffer
   netBufferSetLength((NetBuffer *) &frag->buffer, 0);
}


/**
 * @brief Check whether a fragment overlaps data already received
 * @param[in] interface Underlying network interface
 * @param[in] frag IPv6 fragment descriptor
 * @param[in] dataFirst Index of the first byte of the fragment
 * @param[in] dataLast Index immediately following the last byte of the fragment
 * @return Error code
 **/

error_t ipv6CheckFragOverlap(NetInterface *interface, Ipv6FragDesc *frag,
   uint16_t dataFirst, uint16_t dataLast)
{
#if (IPV6_OVERLAPPING_FRAG_SUPPORT == DISABLED)
   error_t error;
   Ipv6HoleDesc *hole;

   //Select the first hole descriptor from the list
   hole = ipv6FindHole(frag, frag->firstHole);

   //Search for a hole the fragment interacts with
   while(hole != NULL && (dataFirst >= hole->last ||
      dataLast <= hole->first))
   {
      hole = ipv6FindHole(frag, hole->next);
   }

   //Check whether the fragment interacts with any hole
   if(hole == NULL)
   {
      //A fragment that lies entirely within data already received is silently
      //dropped so that it cannot overwrite previously received data
      error = ERROR_INVALID_PACKET;
   }
   else if(dataFirst < hole->first || dataLast > hole->last)
   {
      //When reassembling an IPv6 datagram, if one or more its constituent
      //fragments is determined to be an overlapping fragment, the entire
      //datagram must be silently discarded (refer to RFC 5722, section 4)
      ipv6DropFragDatagram(interface, frag);

      //Report an error
      error = ERROR_INVALID_PACKET;
   }
   else
   {
      //The fragment lies entirely within the hole
      error = NO_ERROR;
   }

   //Return status code
   return error;
#else
   //Overlapping fragments are accepted
   return NO_ERROR;
#endif
}


/**
 * @brief Insert a new hole descriptor into the list
 * @param[in] frag IPv6 fragment descriptor
 * @param[in] prevHole Hole descriptor after which the new entry is inserted
 *   (NULL to insert the new entry at the head of the list)
 * @param[in] first Index of the first byte of the hole
 * @param[in] last Index immediately following the last byte of the hole
 * @return Pointer to the newly created hole descriptor
 **/

Ipv6HoleDesc *ipv6InsertHole(Ipv6FragDesc *frag, Ipv6HoleDesc *prevHole,
   uint16_t first, uint16_t last)
{
   Ipv6HoleDesc *hole;

   //Point to the location of the new hole descriptor
   hole = ipv6FindHole(frag, first);

   //Make sure the hole descriptor is valid
   if(hole != NULL)
   {
      //Save the boundaries of the hole
      hole->first = first;
      hole->last = last;

      //Insert the newly created entry into the hole descriptor list
      if(prevHole != NULL)
      {
         hole->next = prevHole->next;
         prevHole->next = hole->first;
      }
      else
      {
         hole->next = frag->firstHole;
         frag->firstHole = hole->first;
      }
   }

   //Return a pointer to the newly created hole descriptor
   return hole;
}


/**
 * @brief Retrieve hole descriptor
 * @param[in] frag IPv6 fragment descriptor
 * @param[in] offset Offset of the hole
 * @return A pointer to the hole descriptor is returned if the
 *   specified offset is valid. Otherwise NULL is returned
 **/

Ipv6HoleDesc *ipv6FindHole(Ipv6FragDesc *frag, uint16_t offset)
{
   //Return a pointer to the hole descriptor
   return netBufferAt((NetBuffer *) &frag->buffer, frag->unfragPartLen + offset,
      sizeof(Ipv6HoleDesc));
}


/**
 * @brief Dump hole descriptor list
 * @param[in] frag IPv6 fragment descriptor
 **/

void ipv6DumpHoleList(Ipv6FragDesc *frag)
{
//Check debugging level
#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   Ipv6HoleDesc *hole;

   //Debug message
   TRACE_DEBUG("Hole descriptor list:\r\n");
   //Select the first hole descriptor from the list
   hole = ipv6FindHole(frag, frag->firstHole);

   //Loop through the hole descriptor list
   while(hole != NULL)
   {
      //Display current hole
      TRACE_DEBUG("  %" PRIu16 " - %" PRIu16 "\r\n", hole->first, hole->last);
      //Select the next hole descriptor from the list
      hole = ipv6FindHole(frag, hole->next);
   }
#endif
}

#endif
