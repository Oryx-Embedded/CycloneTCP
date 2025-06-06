/**
 * @file mld_common.c
 * @brief Definitions common to MLD node, router and snooping switch
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
 * MLD is used by an IPv6 router to discover the presence of multicast
 * listeners on its directly attached links, and to discover specifically
 * which multicast addresses are of interest to those neighboring nodes.
 * Refer to the following RFCs for complete details:
 * - RFC 2710: Multicast Listener Discovery (MLD) for IPv6
 * - RFC 3590: Source Address Selection for MLD Protocol
 * - RFC 3810: Multicast Listener Discovery Version 2 (MLDv2) for IPv6
 * - RFC 9777: Multicast Listener Discovery Version 2 (MLDv2) for IPv6
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL MLD_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "ipv6/ipv6_multicast.h"
#include "ipv6/icmpv6.h"
#include "mld/mld_node.h"
#include "mld/mld_node_misc.h"
#include "mld/mld_common.h"
#include "mld/mld_debug.h"
#include "mibs/ip_mib_module.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && MLD_NODE_SUPPORT == ENABLED)

//Link-local All-Routers IPv6 address
const Ipv6Addr MLD_V2_ALL_ROUTERS_ADDR =
   IPV6_ADDR(0xFF02, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0016);

//Tick counter to handle periodic operations
systime_t mldTickCounter;


/**
 * @brief MLD initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mldInit(NetInterface *interface)
{
   error_t error;

   //Join the All-Nodes multicast address
   error = ipv6JoinMulticastGroup(interface, &IPV6_LINK_LOCAL_ALL_NODES_ADDR);
   //Any error to report?
   if(error)
      return error;

#if (MLD_NODE_SUPPORT == ENABLED)
   //MLD node initialization
   error = mldNodeInit(interface);
   //Any error to report?
   if(error)
      return error;
#endif

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief MLD node timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * handle MLD related timers
 *
 * @param[in] interface Underlying network interface
 **/

void mldTick(NetInterface *interface)
{
#if (MLD_NODE_SUPPORT == ENABLED)
   //Manage MLD node timers
   mldNodeTick(&interface->mldNodeContext);
#endif
}


/**
 * @brief Callback function for link change event
 * @param[in] interface Underlying network interface
 **/

void mldLinkChangeEvent(NetInterface *interface)
{
#if (MLD_NODE_SUPPORT == ENABLED)
   //Notify the MLD node of link state changes
   mldNodeLinkChangeEvent(&interface->mldNodeContext);
#endif
}


/**
 * @brief Send MLD message
 * @param[in] interface Underlying network interface
 * @param[in] destAddr Destination IP address
 * @param[in] buffer Multi-part buffer containing the payload
 * @param[in] offset Offset to the first byte of the payload
 * @return Error code
 **/

error_t mldSendMessage(NetInterface *interface, const Ipv6Addr *destAddr,
   NetBuffer *buffer, size_t offset)
{
   size_t length;
   MldMessage *message;
   Ipv6PseudoHeader pseudoHeader;
   NetTxAncillary ancillary;

   //Retrieve the length of payload
   length = netBufferGetLength(buffer) - offset;

   //Point to the beginning of the MLD message
   message = netBufferAt(buffer, offset, length);
   //Sanity check
   if(message == NULL)
      return ERROR_FAILURE;

   //Check if a valid link-local address is available on the interface
   if(ipv6GetLinkLocalAddrState(interface) == IPV6_ADDR_STATE_PREFERRED)
   {
      //The message is sent with a link-local address as the IPv6 source address
      pseudoHeader.srcAddr = interface->ipv6Context.addrList[0].addr;
   }
   else
   {
      //Check MLD message type
      if(message->type == ICMPV6_TYPE_MCAST_LISTENER_QUERY)
      {
         //MLD Query messages must be sent with a valid link-local address as
         //the IPv6 source address (refer to RFC 3590, section 4)
         return ERROR_NO_ADDRESS;
      }
      else
      {
         //MLD Report and Done messages are sent with a link-local address as
         //the IPv6 source address, if a valid address is available on the
         //interface. If a valid link-local address is not available, the
         //message is sent with the unspecified address as the IPv6 source
         //address
         pseudoHeader.srcAddr = IPV6_UNSPECIFIED_ADDR;
      }
   }

   //Format IPv6 pseudo header
   pseudoHeader.destAddr = *destAddr;
   pseudoHeader.length = htons(length);
   pseudoHeader.reserved[0] = 0;
   pseudoHeader.reserved[1] = 0;
   pseudoHeader.reserved[2] = 0;
   pseudoHeader.nextHeader = IPV6_ICMPV6_HEADER;

   //Message checksum calculation
   message->checksum = ipCalcUpperLayerChecksumEx(&pseudoHeader,
      sizeof(Ipv6PseudoHeader), buffer, offset, length);

   //Total number of ICMP messages which this entity attempted to send
   IP_MIB_INC_COUNTER32(icmpv6Stats.icmpStatsOutMsgs, 1);
   //Increment per-message type ICMP counter
   IP_MIB_INC_COUNTER32(icmpv6MsgStatsTable.icmpMsgStatsOutPkts[message->type], 1);

   //Debug message
   TRACE_INFO("Sending MLD message (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump message contents for debugging purpose
   mldDumpMessage(message, length);

   //Additional options can be passed to the stack along with the packet
   ancillary = NET_DEFAULT_TX_ANCILLARY;

   //All MLD messages must be sent with an IPv6 Hop Limit of 1, and an IPv6
   //Router Alert option in a Hop-by-Hop Options header (refer to RFC 2710,
   //section 3)
   ancillary.ttl = MLD_HOP_LIMIT;
   ancillary.routerAlert = TRUE;

   //Send the MLD message
   return ipv6SendDatagram(interface, &pseudoHeader, buffer, offset,
      &ancillary);
}


/**
 * @brief Process incoming MLD message
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader IPv6 pseudo header
 * @param[in] buffer Multi-part buffer containing the incoming MLD message
 * @param[in] offset Offset to the first byte of the MLD message
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 **/

void mldProcessMessage(NetInterface *interface,
   const Ipv6PseudoHeader *pseudoHeader, const NetBuffer *buffer,
   size_t offset, const NetRxAncillary *ancillary)
{
   size_t length;
   const MldMessage *message;

   //Retrieve the length of the MLD message
   length = netBufferGetLength(buffer) - offset;

   //To be valid, an MLD message must be at least 24 octets long
   if(length < sizeof(MldMessage))
      return;

   //Point to the beginning of the MLD message
   message = netBufferAt(buffer, offset, length);
   //Sanity check
   if(message == NULL)
      return;

   //Debug message
   TRACE_INFO("MLD message received (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump message contents for debugging purpose
   mldDumpMessage(message, length);

   //All MLD messages are sent with an IPv6 Hop Limit of 1
   if(ancillary->ttl != MLD_HOP_LIMIT)
      return;

#if (MLD_NODE_SUPPORT == ENABLED)
   //Pass the message to the MLD node
   mldNodeProcessMessage(&interface->mldNodeContext, pseudoHeader, message,
      length);
#endif
}


/**
 * @brief Generate a random delay
 * @param[in] maxDelay maximum delay
 * @return Random amount of time
 **/

systime_t mldGetRandomDelay(systime_t maxDelay)
{
   systime_t delay;

   //Generate a random delay in the specified range
   if(maxDelay > MLD_TICK_INTERVAL)
   {
      delay = netGenerateRandRange(0, maxDelay - MLD_TICK_INTERVAL);
   }
   else
   {
      delay = 0;
   }

   //Return the random value
   return delay;
}


/**
 * @brief Decode a floating-point value (8-bit code)
 * @param[in] code Floating-point representation
 * @return Decoded value
 **/

uint32_t mldDecodeFloatingPointValue8(uint8_t code)
{
   uint8_t exp;
   uint8_t mant;

   //Retrieve the value of the exponent
   exp = (code >> 4) & 0x07;
   //Retrieve the value of the mantissa
   mant = code & 0x0F;

   //The code represents a floating-point value
   return (mant | 0x10) << (exp + 3);
}


/**
 * @brief Decode a floating-point value (16-bit code)
 * @param[in] code Floating-point representation
 * @return Decoded value
 **/

uint32_t mldDecodeFloatingPointValue16(uint16_t code)
{
   uint16_t exp;
   uint16_t mant;

   //Retrieve the value of the exponent
   exp = (code >> 12) & 0x07;
   //Retrieve the value of the mantissa
   mant = code & 0x0FFF;

   //The code represents a floating-point value
   return (mant | 0x1000) << (exp + 3);
}

#endif
