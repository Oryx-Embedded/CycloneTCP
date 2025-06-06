/**
 * @file igmp_common.c
 * @brief Definitions common to IGMP host, router and snooping switch
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
 * IGMP is used by IP hosts to report their multicast group memberships to any
 * immediately-neighboring multicast routers. Refer to the following RFCs for
 * complete details:
 * - RFC 1112: Host Extensions for IP Multicasting
 * - RFC 2236: Internet Group Management Protocol, Version 2
 * - RFC 3376: Internet Group Management Protocol, Version 3
 * - RFC 4541: Considerations for IGMP and MLD Snooping Switches
 * - RFC 9776: Internet Group Management Protocol, Version 3
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL IGMP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "ipv4/ipv4_multicast.h"
#include "ipv4/ipv4_misc.h"
#include "igmp/igmp_host.h"
#include "igmp/igmp_host_misc.h"
#include "igmp/igmp_router.h"
#include "igmp/igmp_router_misc.h"
#include "igmp/igmp_snooping.h"
#include "igmp/igmp_snooping_misc.h"
#include "igmp/igmp_common.h"
#include "igmp/igmp_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED && (IGMP_HOST_SUPPORT == ENABLED || \
   IGMP_ROUTER_SUPPORT == ENABLED || IGMP_SNOOPING_SUPPORT == ENABLED))

//Tick counter to handle periodic operations
systime_t igmpTickCounter;


/**
 * @brief IGMP initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t igmpInit(NetInterface *interface)
{
   error_t error;

   //The all-systems multicast address, 224.0.0.1, is handled as a special
   //case. On all systems (hosts and routers), reception of packets destined
   //to the all-systems multicast address is permanently enabled on all
   //interfaces on which multicast reception is supported
   error = ipv4JoinMulticastGroup(interface, IGMP_ALL_SYSTEMS_ADDR);
   //Any error to report?
   if(error)
      return error;

#if (IGMP_HOST_SUPPORT == ENABLED)
   //IGMP host initialization
   error = igmpHostInit(interface);
   //Any error to report?
   if(error)
      return error;
#endif

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief IGMP timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * handle IGMP related timers
 *
 * @param[in] interface Underlying network interface
 **/

void igmpTick(NetInterface *interface)
{
#if (IGMP_HOST_SUPPORT == ENABLED)
   //Manage IGMP host timers
   igmpHostTick(&interface->igmpHostContext);
#endif

#if (IGMP_ROUTER_SUPPORT == ENABLED)
   //Valid IGMP router context?
   if(interface->igmpRouterContext != NULL)
   {
      //Manage IGMP router timers
      igmpRouterTick(interface->igmpRouterContext);
   }
#endif

#if (IGMP_SNOOPING_SUPPORT == ENABLED)
   //Valid IGMP snooping switch context?
   if(interface->igmpSnoopingContext != NULL)
   {
      //Manage IGMP snooping switch timers
      igmpSnoopingTick(interface->igmpSnoopingContext);
   }
#endif
}


/**
 * @brief Callback function for link change event
 * @param[in] interface Underlying network interface
 **/

void igmpLinkChangeEvent(NetInterface *interface)
{
#if (IGMP_HOST_SUPPORT == ENABLED)
   //Notify the IGMP host of link state changes
   igmpHostLinkChangeEvent(&interface->igmpHostContext);
#endif
}


/**
 * @brief Send IGMP message
 * @param[in] interface Underlying network interface
 * @param[in] destAddr Destination IP address
 * @param[in] buffer Multi-part buffer containing the payload
 * @param[in] offset Offset to the first byte of the payload
 * @return Error code
 **/

error_t igmpSendMessage(NetInterface *interface, Ipv4Addr destAddr,
   NetBuffer *buffer, size_t offset)
{
   error_t error;
   size_t length;
   IgmpMessage *message;
   Ipv4Addr srcIpAddr;
   Ipv4PseudoHeader pseudoHeader;

   //Retrieve the length of payload
   length = netBufferGetLength(buffer) - offset;

   //Point to the beginning of the IGMP message
   message = netBufferAt(buffer, offset, length);
   //Sanity check
   if(message == NULL)
      return ERROR_FAILURE;

   //Select the source IPv4 address to use
   error = ipv4SelectSourceAddr(&interface, destAddr, &srcIpAddr);

   //Check status code
   if(!error)
   {
      pseudoHeader.srcAddr = srcIpAddr;
   }
   else
   {
      pseudoHeader.srcAddr = IPV4_UNSPECIFIED_ADDR;
   }

   //Format IPv4 pseudo header
   pseudoHeader.destAddr = destAddr;
   pseudoHeader.reserved = 0;
   pseudoHeader.protocol = IPV4_PROTOCOL_IGMP;
   pseudoHeader.length = htons(length);

   //Debug message
   TRACE_INFO("Sending IGMP message (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump message contents for debugging purpose
   igmpDumpMessage(message, length);

#if (IGMP_SNOOPING_SUPPORT == ENABLED)
   //Valid IGMP snooping switch context?
   if(interface->igmpSnoopingContext != NULL)
   {
      NetRxAncillary ancillary;

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_RX_ANCILLARY;
      //Specify ingress port
      ancillary.port = SWITCH_CPU_PORT;

      //Forward the message to the IGMP snooping switch
      igmpSnoopingProcessMessage(interface->igmpSnoopingContext, &pseudoHeader,
         message, length, &ancillary);

      //Sucessful processing
      error = NO_ERROR;
   }
   else
#endif
   {
      NetTxAncillary ancillary;

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_TX_ANCILLARY;

      //All IGMP messages are sent with an IP TTL of 1 and contain an IP Router
      //Alert option in their IP header (refer to RFC 2236, section 2)
      ancillary.ttl = IGMP_TTL;
      ancillary.routerAlert = TRUE;

      //Every IGMPv3 message is sent with an IP Precedence of Internetwork
      //Control (refer to RFC 3376, section 4)
      if(message->type == IGMP_TYPE_MEMBERSHIP_QUERY &&
         length >= sizeof(IgmpMembershipQueryV3))
      {
         ancillary.tos = IPV4_TOS_PRECEDENCE_INTERNETWORK_CTRL;
      }
      else if(message->type == IGMP_TYPE_MEMBERSHIP_REPORT_V3)
      {
         ancillary.tos = IPV4_TOS_PRECEDENCE_INTERNETWORK_CTRL;
      }
      else
      {
         ancillary.tos = 0;
      }

      //Send the IGMP message
      error = ipv4SendDatagram(interface, &pseudoHeader, buffer, offset,
         &ancillary);
   }

#if (IGMP_HOST_SUPPORT == ENABLED && IGMP_ROUTER_SUPPORT == ENABLED)
   //Check IGMP message type
   if(message->type == IGMP_TYPE_MEMBERSHIP_QUERY)
   {
      //Forward Membership Query messages to the IGMP host
      igmpHostProcessMessage(&interface->igmpHostContext, &pseudoHeader,
         message, length);
   }
   else if(message->type == IGMP_TYPE_MEMBERSHIP_REPORT_V1 ||
      message->type == IGMP_TYPE_MEMBERSHIP_REPORT_V2 ||
      message->type == IGMP_TYPE_LEAVE_GROUP)
   {
      //Valid IGMP router context?
      if(interface->igmpRouterContext != NULL)
      {
         //Forward Membership Report and Leave Group messages to the IGMP router
         igmpRouterProcessMessage(interface->igmpRouterContext, &pseudoHeader,
            message, length);
      }
   }
   else
   {
      //Just for sanity
   }
#endif

   //Return status code
   return error;
}


/**
 * @brief Process incoming IGMP message
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader IPv4 pseudo header
 * @param[in] buffer Multi-part buffer containing the incoming IGMP message
 * @param[in] offset Offset to the first byte of the IGMP message
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 **/

void igmpProcessMessage(NetInterface *interface,
   const Ipv4PseudoHeader *pseudoHeader, const NetBuffer *buffer,
   size_t offset, const NetRxAncillary *ancillary)
{
   size_t length;
   const IgmpMessage *message;

   //Retrieve the length of the IGMP message
   length = netBufferGetLength(buffer) - offset;

   //To be valid, an IGMP message must be at least 8 octets long
   if(length < sizeof(IgmpMessage))
      return;

   //Point to the beginning of the IGMP message
   message = netBufferAt(buffer, offset, length);
   //Sanity check
   if(message == NULL)
      return;

   //Debug message
   TRACE_INFO("IGMP message received (%" PRIuSIZE " bytes)...\r\n", length);

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Dump switch port identifier
   if(ancillary->port != 0)
   {
      TRACE_INFO("  Switch Port = %" PRIu8 "\r\n", ancillary->port);
   }
#endif

   //Dump message contents for debugging purpose
   igmpDumpMessage(message, length);

   //Verify checksum value
   if(ipCalcChecksumEx(buffer, offset, length) != 0x0000)
   {
      //Debug message
      TRACE_WARNING("Wrong IGMP header checksum!\r\n");
      //Drop incoming message
      return;
   }

   //All IGMP messages are sent with an IP TTL of 1
   if(ancillary->ttl != IGMP_TTL)
      return;

#if (IGMP_HOST_SUPPORT == ENABLED)
   //Pass the message to the IGMP host
   igmpHostProcessMessage(&interface->igmpHostContext, pseudoHeader, message,
      length);
#endif

#if (IGMP_ROUTER_SUPPORT == ENABLED)
   //Valid IGMP router context?
   if(interface->igmpRouterContext != NULL)
   {
      //Pass the message to the IGMP router
      igmpRouterProcessMessage(interface->igmpRouterContext, pseudoHeader,
         message, length);
   }
#endif

#if (IGMP_SNOOPING_SUPPORT == ENABLED)
   //Valid IGMP snooping switch context?
   if(interface->igmpSnoopingContext != NULL)
   {
      //Pass the message to the IGMP snooping switch
      igmpSnoopingProcessMessage(interface->igmpSnoopingContext, pseudoHeader,
         message, length, ancillary);
   }
#endif
}


/**
 * @brief Generate a random delay
 * @param[in] maxDelay maximum delay
 * @return Random amount of time
 **/

systime_t igmpGetRandomDelay(systime_t maxDelay)
{
   systime_t delay;

   //Generate a random delay in the specified range
   if(maxDelay > IGMP_TICK_INTERVAL)
   {
      delay = netGenerateRandRange(0, maxDelay - IGMP_TICK_INTERVAL);
   }
   else
   {
      delay = 0;
   }

   //Return the random value
   return delay;
}


/**
 * @brief Decode a floating-point value
 * @param[in] code Floating-point representation
 * @return Decoded value
 **/

uint32_t igmpDecodeFloatingPointValue(uint8_t code)
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

#endif
