/**
 * @file arp.c
 * @brief ARP (Address Resolution Protocol)
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
 * @section Description
 *
 * Address Resolution Protocol is used to determine the hardware address of
 * a specific host when only its IPv4 address is known. Refer to RFC 826
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL ARP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/ethernet.h"
#include "ipv4/arp.h"
#include "ipv4/arp_cache.h"
#include "ipv4/ipv4_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED && ETH_SUPPORT == ENABLED)

//Tick counter to handle periodic operations
systime_t arpTickCounter;


/**
 * @brief ARP cache initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t arpInit(NetInterface *interface)
{
   //Enable ARP protocol
   interface->enableArp = TRUE;
   //Set ARP reachable time
   interface->arpReachableTime = ARP_REACHABLE_TIME;
   //Set ARP probe timeout
   interface->arpProbeTimeout = ARP_PROBE_TIMEOUT;

   //Initialize the ARP cache
   osMemset(interface->arpCache, 0, sizeof(interface->arpCache));

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Enable address resolution using ARP
 * @param[in] interface Underlying network interface
 * @param[in] enable This flag specifies whether the host is allowed to send
 *   ARP requests and respond to incoming ARP requests. When the flag is set
 *   to FALSE, the host relies exclusively on static ARP entries to map IPv4
 *   addresses into MAC addresses and silently drop incoming ARP requests
 * @return Error code
 **/

error_t arpEnable(NetInterface *interface, bool_t enable)
{
   //Check parameters
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Enable or disable ARP protocol
   interface->enableArp = enable;

   //If ARP is disabled then flush dynamic entries from the ARP cache
   if(!enable)
   {
      arpFlushCache(interface);
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Configure the ARP reachable time
 * @param[in] interface Underlying network interface
 * @param[in] reachableTime The time, in milliseconds, a node assumes a
 *   neighbor is reachable
 * @return Error code
 **/

error_t arpSetReachableTime(NetInterface *interface, systime_t reachableTime)
{
   uint_t i;
   ArpCacheEntry *entry;

   //Check parameters
   if(interface == NULL || reachableTime == 0)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Save ARP reachable time
   interface->arpReachableTime = reachableTime;

   //Go through ARP cache
   for(i = 0; i < ARP_CACHE_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->arpCache[i];

      //Check the state of the ARP entry
      if(entry->state == ARP_STATE_REACHABLE)
      {
         //Adjust the reachable time, if necessary
         if(entry->timeout > reachableTime)
         {
            entry->timeout = reachableTime;
         }
      }
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Configure the time interval between subsequent ARP probes
 * @param[in] interface Underlying network interface
 * @param[in] probeTimeout The time, in milliseconds, between subsequent
 *   retransmissions of probes
 * @return Error code
 **/

error_t arpSetProbeTimout(NetInterface *interface, systime_t probeTimeout)
{
   uint_t i;
   ArpCacheEntry *entry;

   //Check parameters
   if(interface == NULL || probeTimeout == 0)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Save ARP probe timeout
   interface->arpProbeTimeout = probeTimeout;

   //Go through ARP cache
   for(i = 0; i < ARP_CACHE_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->arpCache[i];

      //Check the state of the ARP entry
      if(entry->state == ARP_STATE_PROBE)
      {
         //Adjust the probe timeout, if necessary
         if(entry->timeout > probeTimeout)
         {
            entry->timeout = probeTimeout;
         }
      }
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Add a static entry in the ARP cache
 * @param[in] interface Underlying network interface
 * @param[in] ipAddr IPv4 address
 * @param[in] macAddr MAC address
 * @return Error code
 **/

error_t arpAddStaticEntry(NetInterface *interface, Ipv4Addr ipAddr,
   const MacAddr *macAddr)
{
   error_t error;
   ArpCacheEntry *entry;

   //Check parameters
   if(interface == NULL || macAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the ARP cache for the specified IPv4 address
   entry = arpFindEntry(interface, ipAddr);

   //Check whether a matching entry exists
   if(entry != NULL)
   {
      //Check the state of the ARP entry
      if(entry->state == ARP_STATE_INCOMPLETE)
      {
         //Record the corresponding MAC address
         entry->macAddr = *macAddr;
         //Send all the packets that are pending for transmission
         arpSendQueuedPackets(interface, entry);
      }
   }
   else
   {
      //Create a new entry in the ARP cache
      entry = arpCreateEntry(interface);
   }

   //ARP cache entry successfully created?
   if(entry != NULL)
   {
      //Record the IPv4 address and the corresponding MAC address
      entry->ipAddr = ipAddr;
      entry->macAddr = *macAddr;

      //Unused parameters
      entry->timeout = 0;
      entry->retransmitCount = 0;
      entry->queueSize = 0;

      //Update entry state
      arpChangeState(entry, ARP_STATE_PERMANENT);

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //Failed to create ARP cache entry
      error = ERROR_OUT_OF_RESOURCES;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Remove a static entry from the ARP cache
 * @param[in] interface Underlying network interface
 * @param[in] ipAddr IPv4 address
 * @return Error code
 **/

error_t arpRemoveStaticEntry(NetInterface *interface, Ipv4Addr ipAddr)
{
   error_t error;
   ArpCacheEntry *entry;

   //Check parameters
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the ARP cache for the specified IPv4 address
   entry = arpFindEntry(interface, ipAddr);

   //Check whether a matching entry has been found
   if(entry != NULL && entry->state == ARP_STATE_PERMANENT)
   {
      //Delete ARP entry
      arpChangeState(entry, ARP_STATE_NONE);
      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //No matching entry in ARP cache
      error = ERROR_NOT_FOUND;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Address resolution using ARP protocol
 * @param[in] interface Underlying network interface
 * @param[in] ipAddr IPv4 address
 * @param[in] macAddr Physical address matching the specified IPv4 address
 * @return Error code
 **/

error_t arpResolve(NetInterface *interface, Ipv4Addr ipAddr, MacAddr *macAddr)
{
   error_t error;
   ArpCacheEntry *entry;

   //Search the ARP cache for the specified IPv4 address
   entry = arpFindEntry(interface, ipAddr);

   //Check whether a matching entry has been found
   if(entry != NULL)
   {
      //Check the state of the ARP entry
      if(entry->state == ARP_STATE_INCOMPLETE)
      {
         //The address resolution is already in progress
         error = ERROR_IN_PROGRESS;
      }
      else if(entry->state == ARP_STATE_STALE)
      {
         //Copy the MAC address associated with the specified IPv4 address
         *macAddr = entry->macAddr;

         //Delay before sending the first probe
         entry->timeout = ARP_DELAY_FIRST_PROBE_TIME;
         //Switch to the DELAY state
         arpChangeState(entry, ARP_STATE_DELAY);

         //Successful address resolution
         error = NO_ERROR;
      }
      else
      {
         //Copy the MAC address associated with the specified IPv4 address
         *macAddr = entry->macAddr;

         //Successful address resolution
         error = NO_ERROR;
      }
   }
   else
   {
      //Check whether ARP is enabled
      if(interface->enableArp)
      {
         //If no entry exists, then create a new one
         entry = arpCreateEntry(interface);

         //ARP cache entry successfully created?
         if(entry != NULL)
         {
            //Record the IPv4 address whose MAC address is unknown
            entry->ipAddr = ipAddr;

            //Reset retransmission counter
            entry->retransmitCount = 0;
            //No packet are pending in the transmit queue
            entry->queueSize = 0;

            //Send an ARP request
            arpSendRequest(interface, entry->ipAddr, &MAC_BROADCAST_ADDR);

            //Set timeout value
            entry->timeout = ARP_REQUEST_TIMEOUT;
            //Enter INCOMPLETE state
            arpChangeState(entry, ARP_STATE_INCOMPLETE);

            //The address resolution is in progress
            error = ERROR_IN_PROGRESS;
         }
         else
         {
            //Failed to create ARP cache entry
            error = ERROR_OUT_OF_RESOURCES;
         }
      }
      else
      {
         //ARP is disabled
         error = ERROR_INVALID_ADDRESS;
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Enqueue an IPv4 packet waiting for address resolution
 * @param[in] interface Underlying network interface
 * @param[in] ipAddr IPv4 address of the destination host
 * @param[in] buffer Multi-part buffer containing the packet to be enqueued
 * @param[in] offset Offset to the first byte of the packet
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 * @return Error code
 **/

error_t arpEnqueuePacket(NetInterface *interface, Ipv4Addr ipAddr,
   NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   error_t error;
   uint_t i;
   size_t length;
   ArpCacheEntry *entry;

   //Retrieve the length of the multi-part buffer
   length = netBufferGetLength(buffer);

   //Search the ARP cache for the specified IPv4 address
   entry = arpFindEntry(interface, ipAddr);

   //Check whether a matching entry exists
   if(entry != NULL)
   {
      //Check the state of the ARP entry
      if(entry->state == ARP_STATE_INCOMPLETE)
      {
         //Check whether the packet queue is full
         if(entry->queueSize >= ARP_MAX_PENDING_PACKETS)
         {
            //When the queue overflows, the new arrival should replace the oldest entry
            netBufferFree(entry->queue[0].buffer);

            //Make room for the new packet
            for(i = 1; i < ARP_MAX_PENDING_PACKETS; i++)
            {
               entry->queue[i - 1] = entry->queue[i];
            }

            //Adjust the number of pending packets
            entry->queueSize--;
         }

         //Index of the entry to be filled in
         i = entry->queueSize;
         //Allocate a memory buffer to store the packet
         entry->queue[i].buffer = netBufferAlloc(length);

         //Successful memory allocation?
         if(entry->queue[i].buffer != NULL)
         {
            //Copy the contents of the IPv4 packet
            netBufferCopy(entry->queue[i].buffer, 0, buffer, 0, length);
            //Offset to the first byte of the IPv4 header
            entry->queue[i].offset = offset;
            //Additional options passed to the stack along with the packet
            entry->queue[i].ancillary = *ancillary;

            //Increment the number of queued packets
            entry->queueSize++;
            //The packet was successfully enqueued
            error = NO_ERROR;
         }
         else
         {
            //Failed to allocate memory
            error = ERROR_OUT_OF_MEMORY;
         }
      }
      else
      {
         //The address is already resolved
         error = ERROR_UNEXPECTED_STATE;
      }
   }
   else
   {
      //No matching entry in ARP cache
      error = ERROR_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief ARP timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * manage ARP cache
 *
 * @param[in] interface Underlying network interface
 **/

void arpTick(NetInterface *interface)
{
   uint_t i;
   systime_t time;
   ArpCacheEntry *entry;

   //Get current time
   time = osGetSystemTime();

   //Go through ARP cache
   for(i = 0; i < ARP_CACHE_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->arpCache[i];

      //Check the state of the ARP entry
      if(entry->state == ARP_STATE_PERMANENT)
      {
         //Static ARP entries are never updated
      }
      else if(entry->state == ARP_STATE_INCOMPLETE)
      {
         //The request timed out?
         if(timeCompare(time, entry->timestamp + entry->timeout) >= 0)
         {
            //Increment retransmission counter
            entry->retransmitCount++;

            //Check whether the maximum number of retransmissions has been exceeded
            if(entry->retransmitCount < ARP_MAX_REQUESTS)
            {
               //Retransmit ARP request
               arpSendRequest(interface, entry->ipAddr, &MAC_BROADCAST_ADDR);

               //Save the time at which the packet was sent
               entry->timestamp = time;
               //Set timeout value
               entry->timeout = ARP_REQUEST_TIMEOUT;
            }
            else
            {
               //Drop packets that are waiting for address resolution
               arpFlushQueuedPackets(interface, entry);

               //The entry should be deleted since address resolution has failed
               arpChangeState(entry, ARP_STATE_NONE);
            }
         }
      }
      else if(entry->state == ARP_STATE_REACHABLE)
      {
         //Periodically time out ARP cache entries
         if(timeCompare(time, entry->timestamp + entry->timeout) >= 0)
         {
            //Enter STALE state
            arpChangeState(entry, ARP_STATE_STALE);
         }
      }
      else if(entry->state == ARP_STATE_STALE)
      {
         //The neighbor is no longer known to be reachable but until traffic
         //is sent to the neighbor, no attempt should be made to verify its
         //reachability
      }
      else if(entry->state == ARP_STATE_DELAY)
      {
         //Wait for the specified delay before sending the first probe
         if(timeCompare(time, entry->timestamp + entry->timeout) >= 0)
         {
            //Reset retransmission counter
            entry->retransmitCount = 0;

            //Send a point-to-point ARP request to the host
            arpSendRequest(interface, entry->ipAddr, &entry->macAddr);

            //Set timeout value
            entry->timeout = interface->arpProbeTimeout;
            //Switch to the PROBE state
            arpChangeState(entry, ARP_STATE_PROBE);
         }
      }
      else if(entry->state == ARP_STATE_PROBE)
      {
         //The request timed out?
         if(timeCompare(time, entry->timestamp + entry->timeout) >= 0)
         {
            //Increment retransmission counter
            entry->retransmitCount++;

            //Check whether the maximum number of retransmissions has been
            //exceeded
            if(entry->retransmitCount < ARP_MAX_PROBES)
            {
               //Send a point-to-point ARP request to the host
               arpSendRequest(interface, entry->ipAddr, &entry->macAddr);

               //Save the time at which the packet was sent
               entry->timestamp = time;
               //Set timeout value
               entry->timeout = interface->arpProbeTimeout;
            }
            else
            {
               //The entry should be deleted since the host is not reachable
               //anymore
               arpChangeState(entry, ARP_STATE_NONE);
            }
         }
      }
      else
      {
         //Just for sanity
         arpChangeState(entry, ARP_STATE_NONE);
      }
   }
}


/**
 * @brief Incoming ARP packet processing
 * @param[in] interface Underlying network interface
 * @param[in] arpPacket Incoming ARP packet
 * @param[in] length Packet length
 **/

void arpProcessPacket(NetInterface *interface, ArpPacket *arpPacket,
   size_t length)
{
   uint_t i;
   bool_t validTarget;
   Ipv4AddrEntry *addrEntry;
   NetInterface *logicalInterface;

   //Discard invalid ARP packets
   if(length < sizeof(ArpPacket))
      return;

   //Debug message
   TRACE_INFO("ARP packet received (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump ARP packet contents for debugging purpose
   arpDumpPacket(arpPacket);

   //Make sure the hardware type is valid
   if(arpPacket->hrd != HTONS(ARP_HARDWARE_TYPE_ETH))
      return;

   //Make sure the protocol type is valid
   if(arpPacket->pro != HTONS(ARP_PROTOCOL_TYPE_IPV4))
      return;

   //Check the length of the hardware address
   if(arpPacket->hln != sizeof(MacAddr))
      return;

   //Check the length of the protocol address
   if(arpPacket->pln != sizeof(Ipv4Addr))
      return;

   //The target protocol address must a valid address assigned to the interface
   //or a tentative address whose uniqueness on a link is being verified
   validTarget = FALSE;

   //Loop through the list of IPv4 addresses assigned to the interface
   for(i = 0; i < IPV4_ADDR_LIST_SIZE; i++)
   {
      //Point to the current entry
      addrEntry = &interface->ipv4Context.addrList[i];

      //Valid entry?
      if(addrEntry->state != IPV4_ADDR_STATE_INVALID)
      {
         //Check whether the sender protocol address matches the IP address
         //assigned to the interface
         if(addrEntry->addr == arpPacket->spa)
         {
            //Tentative address?
            if(addrEntry->state == IPV4_ADDR_STATE_TENTATIVE)
            {
               //If the host receives any ARP packet where the sender IP
               //address is the address being probed for, then this is a
               //conflicting ARP packet
               addrEntry->conflict = TRUE;
               //Exit immediately
               return;
            }
            else
            {
               //Point to the logical interface
               logicalInterface = nicGetLogicalInterface(interface);

               //If the sender hardware address does not match the hardware
               //address of that interface, then this is a conflicting ARP
               //packet
               if(!macCompAddr(&arpPacket->sha, &logicalInterface->macAddr))
               {
                  //An address conflict has been detected...
                  addrEntry->conflict = TRUE;
                  //Exit immediately
                  return;
               }
            }
         }

         //Check whether the target protocol address matches an IP address
         //assigned to the interface
         if(addrEntry->addr == arpPacket->tpa)
         {
            validTarget = TRUE;
         }
      }
   }

   //Valid target protocol address?
   if(validTarget)
   {
      //Check operation code
      switch(ntohs(arpPacket->op))
      {
      //ARP request?
      case ARP_OPCODE_ARP_REQUEST:
         //Process incoming ARP request
         arpProcessRequest(interface, arpPacket);
         break;

      //ARP reply?
      case ARP_OPCODE_ARP_REPLY:
         //Process incoming ARP reply
         arpProcessReply(interface, arpPacket);
         break;

      //Unknown operation code?
      default:
         //Debug message
         TRACE_INFO("Unknown operation code!\r\n");
         //Discard incoming packet
         break;
      }
   }
}


/**
 * @brief Incoming ARP request processing
 * @param[in] interface Underlying network interface
 * @param[in] arpRequest Incoming ARP request
 **/

void arpProcessRequest(NetInterface *interface, ArpPacket *arpRequest)
{
   uint_t i;
   bool_t validTarget;
   Ipv4AddrEntry *addrEntry;
   NetInterface *logicalInterface;

   //Debug message
   TRACE_INFO("ARP Request received...\r\n");

   //Check sender protocol address
   if(ipv4IsBroadcastAddr(interface, arpRequest->spa) ||
      ipv4IsMulticastAddr(arpRequest->spa))
   {
      return;
   }

   //Initialize flag
   validTarget = TRUE;

   //Loop through the list of IPv4 addresses assigned to the interface
   for(i = 0; i < IPV4_ADDR_LIST_SIZE; i++)
   {
      //Point to the current entry
      addrEntry = &interface->ipv4Context.addrList[i];

      //Tentative address?
      if(addrEntry->state == IPV4_ADDR_STATE_TENTATIVE)
      {
         //Check whether the target IP address is an address being probed for
         if(addrEntry->addr == arpRequest->tpa)
         {
            //The target protocol address is a tentative address
            validTarget = FALSE;

            //ARP probe received?
            if(arpRequest->spa == IPV4_UNSPECIFIED_ADDR)
            {
               //Point to the logical interface
               logicalInterface = nicGetLogicalInterface(interface);

               //If the sender hardware address does not match the hardware
               //address of that interface, then this is a conflicting ARP
               //packet
               if(!macCompAddr(&arpRequest->sha, &logicalInterface->macAddr))
               {
                  //An address conflict has been detected...
                  addrEntry->conflict = TRUE;
               }
            }
         }
      }
   }

   //In all cases, the host must not respond to an ARP request for an address
   //being probed for
   if(validTarget && interface->enableArp)
   {
      //Send ARP reply
      arpSendReply(interface, arpRequest->tpa, arpRequest->spa,
         &arpRequest->sha);
   }
}


/**
 * @brief Incoming ARP reply processing
 * @param[in] interface Underlying network interface
 * @param[in] arpReply Incoming ARP reply
 **/

void arpProcessReply(NetInterface *interface, ArpPacket *arpReply)
{
   ArpCacheEntry *entry;

   //Debug message
   TRACE_INFO("ARP Reply received...\r\n");

   //Check sender protocol address
   if(arpReply->spa == IPV4_UNSPECIFIED_ADDR ||
      ipv4IsBroadcastAddr(interface, arpReply->spa) ||
      ipv4IsMulticastAddr(arpReply->spa))
   {
      return;
   }

   //Check sender hardware address
   if(macCompAddr(&arpReply->sha, &MAC_UNSPECIFIED_ADDR) ||
      macCompAddr(&arpReply->sha, &MAC_BROADCAST_ADDR) ||
      macIsMulticastAddr(&arpReply->sha))
   {
      return;
   }

   //Check whether the target IP address is an address being probed for
   if(ipv4IsTentativeAddr(interface, arpReply->tpa))
      return;

   //Search the ARP cache for the specified IPv4 address
   entry = arpFindEntry(interface, arpReply->spa);

   //Check whether a matching entry has been found
   if(entry != NULL)
   {
      //Check the state of the ARP entry
      if(entry->state == ARP_STATE_INCOMPLETE)
      {
         //Record the corresponding MAC address
         entry->macAddr = arpReply->sha;

         //Send all the packets that are pending for transmission
         arpSendQueuedPackets(interface, entry);

         //The validity of the ARP entry is limited in time
         entry->timeout = interface->arpReachableTime;
         //Switch to the REACHABLE state
         arpChangeState(entry, ARP_STATE_REACHABLE);
      }
      else if(entry->state == ARP_STATE_REACHABLE)
      {
         //Different link-layer address than cached?
         if(!macCompAddr(&arpReply->sha, &entry->macAddr))
         {
            //Enter STALE state
            arpChangeState(entry, ARP_STATE_STALE);
         }
      }
      else if(entry->state == ARP_STATE_PROBE)
      {
         //Record IPv4/MAC address pair
         entry->ipAddr = arpReply->spa;
         entry->macAddr = arpReply->sha;

         //The validity of the ARP entry is limited in time
         entry->timeout = interface->arpReachableTime;
         //Switch to the REACHABLE state
         arpChangeState(entry, ARP_STATE_REACHABLE);
      }
      else
      {
         //Static ARP entries are never updated
      }
   }
}


/**
 * @brief Send ARP probe
 * @param[in] interface Underlying network interface
 * @param[in] targetIpAddr Target IPv4 address
 * @return Error code
 **/

error_t arpSendProbe(NetInterface *interface, Ipv4Addr targetIpAddr)
{
   error_t error;
   size_t offset;
   NetBuffer *buffer;
   ArpPacket *arpRequest;
   NetInterface *logicalInterface;
   NetTxAncillary ancillary;

   //Point to the logical interface
   logicalInterface = nicGetLogicalInterface(interface);

   //Allocate a memory buffer to hold an ARP packet
   buffer = ethAllocBuffer(sizeof(ArpPacket), &offset);
   //Failed to allocate buffer?
   if(buffer == NULL)
      return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the ARP packet
   arpRequest = netBufferAt(buffer, offset, 0);

   //Format ARP request
   arpRequest->hrd = htons(ARP_HARDWARE_TYPE_ETH);
   arpRequest->pro = htons(ARP_PROTOCOL_TYPE_IPV4);
   arpRequest->hln = sizeof(MacAddr);
   arpRequest->pln = sizeof(Ipv4Addr);
   arpRequest->op = htons(ARP_OPCODE_ARP_REQUEST);
   arpRequest->sha = logicalInterface->macAddr;
   arpRequest->spa = IPV4_UNSPECIFIED_ADDR;
   arpRequest->tha = MAC_UNSPECIFIED_ADDR;
   arpRequest->tpa = targetIpAddr;

   //Debug message
   TRACE_INFO("Sending ARP Probe (%" PRIuSIZE " bytes)...\r\n", sizeof(ArpPacket));
   //Dump ARP packet contents for debugging purpose
   arpDumpPacket(arpRequest);

   //Additional options can be passed to the stack along with the packet
   ancillary = NET_DEFAULT_TX_ANCILLARY;

   //Send ARP request
   error = ethSendFrame(interface, &MAC_BROADCAST_ADDR, ETH_TYPE_ARP, buffer,
      offset, &ancillary);

   //Free previously allocated memory
   netBufferFree(buffer);

   //Return status code
   return error;
}


/**
 * @brief Send ARP request
 * @param[in] interface Underlying network interface
 * @param[in] targetIpAddr Target IPv4 address
 * @param[in] destMacAddr Destination MAC address
 * @return Error code
 **/

error_t arpSendRequest(NetInterface *interface, Ipv4Addr targetIpAddr,
   const MacAddr *destMacAddr)
{
   error_t error;
   size_t offset;
   NetBuffer *buffer;
   ArpPacket *arpRequest;
   Ipv4Addr senderIpAddr;
   NetInterface *logicalInterface;
   NetTxAncillary ancillary;

   //Point to the logical interface
   logicalInterface = nicGetLogicalInterface(interface);

   //Select the most appropriate sender IP address to be used
   error = ipv4SelectSourceAddr(&interface, targetIpAddr, &senderIpAddr);
   //No address assigned to the interface?
   if(error)
      return error;

   //Allocate a memory buffer to hold an ARP packet
   buffer = ethAllocBuffer(sizeof(ArpPacket), &offset);
   //Failed to allocate buffer?
   if(buffer == NULL)
      return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the ARP packet
   arpRequest = netBufferAt(buffer, offset, 0);

   //Format ARP request
   arpRequest->hrd = htons(ARP_HARDWARE_TYPE_ETH);
   arpRequest->pro = htons(ARP_PROTOCOL_TYPE_IPV4);
   arpRequest->hln = sizeof(MacAddr);
   arpRequest->pln = sizeof(Ipv4Addr);
   arpRequest->op = htons(ARP_OPCODE_ARP_REQUEST);
   arpRequest->sha = logicalInterface->macAddr;
   arpRequest->spa = senderIpAddr;
   arpRequest->tha = MAC_UNSPECIFIED_ADDR;
   arpRequest->tpa = targetIpAddr;

   //Debug message
   TRACE_INFO("Sending ARP Request (%" PRIuSIZE " bytes)...\r\n", sizeof(ArpPacket));
   //Dump ARP packet contents for debugging purpose
   arpDumpPacket(arpRequest);

   //Additional options can be passed to the stack along with the packet
   ancillary = NET_DEFAULT_TX_ANCILLARY;

   //Send ARP request
   error = ethSendFrame(interface, destMacAddr, ETH_TYPE_ARP, buffer, offset,
      &ancillary);

   //Free previously allocated memory
   netBufferFree(buffer);

   //Return status code
   return error;
}


/**
 * @brief Send ARP reply
 * @param[in] interface Underlying network interface
 * @param[in] senderIpAddr Sender Ipv4 address
 * @param[in] targetIpAddr Target IPv4 address
 * @param[in] targetMacAddr Target MAC address
 * @return Error code
 **/

error_t arpSendReply(NetInterface *interface, Ipv4Addr senderIpAddr,
   Ipv4Addr targetIpAddr, const MacAddr *targetMacAddr)
{
   error_t error;
   size_t offset;
   NetBuffer *buffer;
   ArpPacket *arpReply;
   NetInterface *logicalInterface;
   NetTxAncillary ancillary;

   //Point to the logical interface
   logicalInterface = nicGetLogicalInterface(interface);

   //Allocate a memory buffer to hold an ARP packet
   buffer = ethAllocBuffer(sizeof(ArpPacket), &offset);
   //Failed to allocate buffer?
   if(buffer == NULL)
      return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the ARP packet
   arpReply = netBufferAt(buffer, offset, 0);

   //Format ARP reply
   arpReply->hrd = htons(ARP_HARDWARE_TYPE_ETH);
   arpReply->pro = htons(ETH_TYPE_IPV4);
   arpReply->hln = sizeof(MacAddr);
   arpReply->pln = sizeof(Ipv4Addr);
   arpReply->op = htons(ARP_OPCODE_ARP_REPLY);
   arpReply->sha = logicalInterface->macAddr;
   arpReply->spa = senderIpAddr;
   arpReply->tha = *targetMacAddr;
   arpReply->tpa = targetIpAddr;

   //Debug message
   TRACE_INFO("Sending ARP Reply (%" PRIuSIZE " bytes)...\r\n", sizeof(ArpPacket));
   //Dump ARP packet contents for debugging purpose
   arpDumpPacket(arpReply);

   //Additional options can be passed to the stack along with the packet
   ancillary = NET_DEFAULT_TX_ANCILLARY;

   //Send ARP reply
   error = ethSendFrame(interface, targetMacAddr, ETH_TYPE_ARP, buffer, offset,
      &ancillary);

   //Free previously allocated memory
   netBufferFree(buffer);

   //Return status code
   return error;
}


/**
 * @brief Dump ARP packet for debugging purpose
 * @param[in] arpPacket ARP header
 **/

void arpDumpPacket(const ArpPacket *arpPacket)
{
   //Dump ARP packet contents
   TRACE_DEBUG("  Hardware Type (hrd) = 0x%04" PRIX16 "\r\n", ntohs(arpPacket->hrd));
   TRACE_DEBUG("  Protocol Type (pro) = 0x%04" PRIX16 "\r\n", ntohs(arpPacket->pro));
   TRACE_DEBUG("  Hardware Address Length (hln) = %" PRIu8 "\r\n", arpPacket->hln);
   TRACE_DEBUG("  Protocol Address Length (pln) = %" PRIu8 "\r\n", arpPacket->pln);
   TRACE_DEBUG("  Opcode (op) = %" PRIu16 "\r\n", ntohs(arpPacket->op));
   TRACE_DEBUG("  Sender Hardware Address (sha) = %s\r\n", macAddrToString(&arpPacket->sha, NULL));
   TRACE_DEBUG("  Sender Protocol Address (spa) = %s\r\n", ipv4AddrToString(arpPacket->spa, NULL));
   TRACE_DEBUG("  Target Hardware Address (tha)= %s\r\n", macAddrToString(&arpPacket->tha, NULL));
   TRACE_DEBUG("  Target Protocol Address (tpa) = %s\r\n", ipv4AddrToString(arpPacket->tpa, NULL));
}

#endif
