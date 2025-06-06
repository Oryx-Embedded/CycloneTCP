/**
 * @file nat_misc.c
 * @brief Helper functions for NAT
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NAT_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "ipv4/ipv4_misc.h"
#include "ipv4/icmp.h"
#include "nat/nat.h"
#include "nat/nat_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (NAT_SUPPORT == ENABLED)

//Tick counter to handle periodic operations
systime_t natTickCounter;


/**
 * @brief NAT timer handler
 * @param[in] context Pointer to the NAT context
 **/

void natTick(NatContext *context)
{
   uint_t i;
   systime_t time;
   NatSession *session;

   //Make sure the NAT context has been properly instantiated
   if(context == NULL)
      return;

   //Get current time
   time = osGetSystemTime();

   //Loop through the NAT sessions
   for(i = 0; i < context->numSessions; i++)
   {
      //Point to the current session
      session = &context->sessions[i];

      //Valid session?
      if(session->protocol != 0)
      {
         //TCP, UDP or ICMP session?
         if(session->protocol == IPV4_PROTOCOL_TCP)
         {
            //Check whether the TCP session timer has expired
            if((time - session->timestamp) >= NAT_TCP_SESSION_TIMEOUT)
            {
               session->protocol = IPV4_PROTOCOL_NONE;
            }
         }
         else if(session->protocol == IPV4_PROTOCOL_UDP)
         {
            //Check whether the UDP session timer has expired
            if((time - session->timestamp) >= NAT_UDP_SESSION_TIMEOUT)
            {
               session->protocol = IPV4_PROTOCOL_NONE;
            }
         }
         else
         {
            //Check whether the ICMP session timer has expired
            if((time - session->timestamp) >= NAT_ICMP_SESSION_TIMEOUT)
            {
               session->protocol = IPV4_PROTOCOL_NONE;
            }
         }
      }
   }
}


/**
 * @brief Check whether a network interface is the WAN interface
 * @param[in] context Pointer to the NAT context
 * @param[in] interface Pointer to a network interface
 * @return TRUE if the specified interface is the WAN interface, else FALSE
 **/

bool_t natIsPublicInterface(NatContext *context, NetInterface *interface)
{
   bool_t res;

   //Initialize flag
   res = FALSE;

   //Check the operational state of the NAT
   if(context != NULL && context->running)
   {
      //Matching interface?
      if(interface == context->publicInterface)
      {
         res = TRUE;
      }
   }

   //Return TRUE if the specified interface is the WAN interface
   return res;
}


/**
 * @brief Check whether a network interface is a LAN interface
 * @param[in] context Pointer to the NAT context
 * @param[in] interface Pointer to a network interface
 * @return TRUE if the specified interface is a LAN interface, else FALSE
 **/

bool_t natIsPrivateInterface(NatContext *context, NetInterface *interface)
{
   uint_t i;
   bool_t res;

   //Initialize flag
   res = FALSE;

   //Check the operational state of the NAT
   if(context != NULL && context->running)
   {
      //Loop through private interfaces
      for(i = 0; i < context->numPrivateInterfaces; i++)
      {
         //Matching interface?
         if(context->privateInterfaces[i] == interface)
         {
            res = TRUE;
            break;
         }
      }
   }

   //Return TRUE if the specified interface is a LAN interface
   return res;
}


/**
 * @brief Process IP packet
 * @param[in] context Pointer to the NAT context
 * @param[in] inInterface Pointer to the interface where the packet was received
 * @param[in] inPseudoHeader Pointer to the pseudo header
 * @param[in] inBuffer Multi-part buffer that holds the incoming IP packet
 * @param[in] inOffset Packet Offset to the payload of the IP packet
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 * @return Error code
 **/

error_t natProcessPacket(NatContext *context, NetInterface *inInterface,
   const Ipv4PseudoHeader *inPseudoHeader, const NetBuffer *inBuffer,
   size_t inOffset, NetRxAncillary *ancillary)
{
   error_t error;
   NatIpPacket packet;

   //Incoming IP packet
   packet.interface = inInterface;
   packet.buffer = inBuffer;
   packet.offset = inOffset;
   packet.protocol = (Ipv4Protocol) inPseudoHeader->protocol;
   packet.srcIpAddr = inPseudoHeader->srcAddr;
   packet.srcPort = 0;
   packet.destIpAddr = inPseudoHeader->destAddr;
   packet.destPort = 0;
   packet.icmpQueryId = 0;
   packet.ttl = ancillary->ttl;
   packet.tos = ancillary->tos;

   //Make sure the NAT context has been properly instantiated
   if(context == NULL)
      return ERROR_FAILURE;

   //Broadcast and multicast packets are not forwarded by the NAT
   if(ipv4IsBroadcastAddr(packet.interface, packet.destIpAddr) ||
      ipv4IsMulticastAddr(packet.destIpAddr))
   {
      return ERROR_INVALID_ADDRESS;
   }

   //Packets with a link-local source or destination address are not routable
   //off the link
   if(ipv4IsLinkLocalAddr(packet.srcIpAddr) ||
      ipv4IsLinkLocalAddr(packet.destIpAddr))
   {
      return ERROR_INVALID_ADDRESS;
   }

   //Inbound or outbound traffic?
   if(natIsPublicInterface(context, packet.interface))
   {
      //Extract transport identifiers (TCP/UDP ports or ICMP query ID)
      error = natParseTransportHeader(&packet);
      //Unrecognized packet?
      if(error)
         return error;

      //Debug message
      TRACE_DEBUG("NAT: Packet received on interface %s...\r\n",
         packet.interface->name);
      //Dump IP packet for debugging purpose
      natDumpPacket(&packet);

      //Perform address translation
      error = natTranslateInboundPacket(context, &packet);
      //Any error to report?
      if(error)
         return error;
   }
   else if(natIsPrivateInterface(context, packet.interface))
   {
      //Check destination IP address
      error = ipv4CheckDestAddr(packet.interface, packet.destIpAddr);
      //Do not forward packets destined to the host
      if(!error)
         return ERROR_INVALID_ADDRESS;

      //Extract transport identifiers (TCP/UDP ports or ICMP query ID)
      error = natParseTransportHeader(&packet);
      //Unrecognized packet?
      if(error)
         return NO_ERROR;

      //Debug message
      TRACE_DEBUG("NAT: Packet received on interface %s...\r\n",
         packet.interface->name);
      //Dump IP packet for debugging purpose
      natDumpPacket(&packet);

      //Perform address translation
      error = natTranslateOutboundPacket(context, &packet);
      //Any error to report?
      if(error)
         return NO_ERROR;

      //Check destination IP address
      error = ipv4CheckDestAddr(packet.interface, packet.destIpAddr);

      //NATs that forward packets originating from an internal address,
      //destined for an external address that matches the active mapping for
      //an internal address, back to that internal address are defined in
      //as supporting "hairpinning"
      if(!error)
      {
         //Perform address translation
         natTranslateInboundPacket(context, &packet);
      }
   }
   else
   {
      //Do not forward the received packet
      return ERROR_FAILURE;
   }

   //TTL exceeded in transit?
   if(packet.ttl <= 1)
   {
      //A NAT device must generate a Time Exceeded ICMP Error message when
      //it discards a packet due to an expired Time to Live field (refer to
      //RFC 5508, section 7.2)
      icmpSendErrorMessage(packet.interface, ICMP_TYPE_TIME_EXCEEDED, 0, 0,
         packet.buffer, 0);
   }
   else
   {
      //NAT devices decrement the TTL on packets that they forward
      packet.ttl--;
      //Forward the packet to the specified interface
      natForwardPacket(context, &packet);
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Perform address translation (inbound packet)
 * @param[in] context Pointer to the NAT context
 * @param[in] packet IP packet
 * @return Error code
 **/

error_t natTranslateInboundPacket(NatContext *context, NatIpPacket *packet)
{
   error_t error;
   NatPortFwdRule *rule;
   NatSession *session;

   //Initialize status code
   error = NO_ERROR;

   //Check whether the packet matches any port forwarding rule
   rule = natMatchPortFwdRule(context, packet);

   //Matching port forwarding rule found?
   if(rule != NULL)
   {
      //Translate destination IP address
      packet->destIpAddr = rule->privateIpAddr;

      //Translate destination port
      packet->destPort = rule->privatePortMin + packet->destPort -
         rule->publicPortMin;

      //Interface where to forward the received packet
      packet->interface = rule->privateInterface;
   }
   else
   {
      //Check whether the packet matches any existing session
      session = natMatchSession(context, packet);

      //Matching session found?
      if(session != NULL)
      {
         //Translate destination IP address and port
         packet->destIpAddr = session->privateIpAddr;
         packet->destPort = session->privatePort;

         //Translate ICMP query identifier
         packet->icmpQueryId = session->privateIcmpQueryId;
         //Interface where to forward the received packet
         packet->interface = session->privateInterface;

         //Keep the mapping active when a packet goes from the external
         //side of the NAT to the internal side of the NAT
         session->timestamp = osGetSystemTime();
      }
      else
      {
         //Report an error
         error = ERROR_INVALID_SESSION;
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Perform address translation (outbound packet)
 * @param[in] context Pointer to the NAT context
 * @param[in] packet IP packet
 * @return Error code
 **/

error_t natTranslateOutboundPacket(NatContext *context, NatIpPacket *packet)
{
   error_t error;
   Ipv4Addr publicIpAddr;
   Ipv4Context *ipv4Context;
   NatPortFwdRule *rule;
   NatSession *session;

   //Initialize status code
   error = NO_ERROR;

   //Point to the IPv4 context
   ipv4Context = &context->publicInterface->ipv4Context;
   //Get the external address
   publicIpAddr = ipv4Context->addrList[context->publicIpAddrIndex].addr;

   //Check whether the packet matches any port forwarding rule
   rule = natMatchPortFwdRule(context, packet);

   //Matching port forwarding rule found?
   if(rule != NULL)
   {
      //Translate source IP address
      packet->srcIpAddr = publicIpAddr;

      //Translate source port
      packet->srcPort = rule->publicPortMin + packet->srcPort -
         rule->privatePortMin;

      //Interface where to forward the received packet
      packet->interface = context->publicInterface;
   }
   else
   {
      //Check whether the packet matches any existing session
      session = natMatchSession(context, packet);

      //Matching session found?
      if(session == NULL)
      {
         //a NAT device must permit ICMP Queries and their associated
         //responses, when the Query is initiated from a private host to
         //the external hosts (refer to RFC 5508, section 3.1)
         if(packet->protocol != IPV4_PROTOCOL_ICMP ||
            packet->icmpType == ICMP_TYPE_ECHO_REQUEST ||
            packet->icmpType == ICMP_TYPE_TIMESTAMP_REQUEST ||
            packet->icmpType == ICMP_TYPE_ADDR_MASK_REQUEST)
         {
            //Create a new session
            session = natCreateSession(context);
         }

         //Valid session?
         if(session != NULL)
         {
            //A NAT session is an association between a session as seen in the
            //private realm and a session as seen in the public realm
            session->protocol = packet->protocol;
            session->privateInterface = packet->interface;
            session->privateIpAddr = packet->srcIpAddr;
            session->privatePort = 0;
            session->privateIcmpQueryId = 0;
            session->publicPort = 0;
            session->publicIcmpQueryId = 0;
            session->remoteIpAddr = packet->destIpAddr;
            session->remotePort = 0;

            //NAT sessions are restricted to sessions based on TCP, UDP, and
            //ICMP
            if(session->protocol == IPV4_PROTOCOL_TCP ||
               session->protocol == IPV4_PROTOCOL_UDP)
            {
               //The NAPT assigns the session a public port number, so that
               //subsequent response packets from the external endpoint can
               //be received by the NAPT, translated, and forwarded to the
               //internal host
               session->privatePort = packet->srcPort;
               session->publicPort = natAllocatePort(context);
               session->remotePort = packet->destPort;
            }
            else
            {
               //The identifier field in ICMP message header is uniquely mapped
               //to a query identifier of the registered IP address (refer to
               //RFC 3022, section 2.2)
               session->privateIcmpQueryId = packet->icmpQueryId;
               session->publicIcmpQueryId = natAllocateIcmpQueryId(context);
            }

            //A private address is bound to an external address, when the first
            //outgoing session is initiated from the private host (refer to
            //RFC 3022, section 3.1)
            error = ipv4SelectSourceAddr(&context->publicInterface,
               packet->destIpAddr, &session->publicIpAddr);

            //Check status code
            if(error)
            {
               //Send an ICMP destination unreachable messages
               icmpSendErrorMessage(packet->interface, ICMP_TYPE_DEST_UNREACHABLE,
                  ICMP_CODE_NET_UNREACHABLE, 0, packet->buffer, 0);

               //Terminate session
               session->protocol = IPV4_PROTOCOL_NONE;
            }
         }
         else
         {
            //Report an error
            error = ERROR_INVALID_SESSION;
         }
      }

      //Check status code
      if(!error)
      {
         //Translate source IP address and port
         packet->srcIpAddr = publicIpAddr;
         packet->srcPort = session->publicPort;

         //Translate ICMP query identifier
         packet->icmpQueryId = session->publicIcmpQueryId;
         //Interface where to forward the received packet
         packet->interface = context->publicInterface;

         //Keep the mapping active when a packet goes from the internal side of
         //the NAT to the external side of the NAT
         session->timestamp = osGetSystemTime();
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Forward an IP packet to the specified interface
 * @param[in] context Pointer to the NAT context
 * @param[in] packet IP packet
 * @return Error code
 **/

error_t natForwardPacket(NatContext *context, const NatIpPacket *packet)
{
   error_t error;
   size_t length;
   NetBuffer *outBuffer;
   size_t outOffset;
   Ipv4PseudoHeader pseudoHeader;
   NetTxAncillary ancillary;

   //Retrieve the length of the incoming IP packet
   length = netBufferGetLength(packet->buffer) - packet->offset;

   //Allocate a buffer to hold the IP packet
   outBuffer = ipAllocBuffer(length, &outOffset);

   //Successful memory allocation?
   if(outBuffer != NULL)
   {
      //Copy the payload
      error = netBufferCopy(outBuffer, outOffset, packet->buffer,
         packet->offset, length);

      //Check status code
      if(!error)
      {
         //Format pseudo header
         pseudoHeader.srcAddr = packet->srcIpAddr;
         pseudoHeader.destAddr = packet->destIpAddr;
         pseudoHeader.reserved = 0;
         pseudoHeader.protocol = packet->protocol;
         pseudoHeader.length = htons(length);

         //Modify transport identifiers (TCP/UDP ports or ICMP query ID)
         error = natTranslateTransportHeader(packet, &pseudoHeader, outBuffer,
            outOffset);
      }

      //Check status code
      if(!error)
      {
         //Additional options can be passed to the stack along with the packet
         ancillary = NET_DEFAULT_TX_ANCILLARY;

         //Specify TTL value
         ancillary.ttl = packet->ttl;
         //Specify ToS value
         ancillary.tos = packet->tos;

         //Debug message
         TRACE_DEBUG("NAT: Sending packet on interface %s...\r\n",
            packet->interface->name);
         //Dump IP packet for debugging purpose
         natDumpPacket(packet);

         //Forward IP packet
         error = ipv4SendDatagram(packet->interface, &pseudoHeader, outBuffer,
            outOffset, &ancillary);
      }

      //Free previously allocated memory
      netBufferFree(outBuffer);
   }
   else
   {
      //Report an error
      error = ERROR_OUT_OF_MEMORY;
   }

   //Return status code
   return error;
}


/**
 * @brief Search the port forwarding rules for a matching entry
 * @param[in] context Pointer to the NAT context
 * @param[in] packet IP packet
 * @return Pointer to the matching port forwarding rule, if any
 **/

NatPortFwdRule *natMatchPortFwdRule(NatContext *context,
   const NatIpPacket *packet)
{
   uint_t i;
   NatPortFwdRule *rule;

   //Inbound or outbound traffic?
   if(packet->interface == context->publicInterface)
   {
      //Loop through the list of port redirection rules
      for(i = 0; i < context->numPortFwdRules; i++)
      {
         //Point to the current rule
         rule = &context->portFwdRules[i];

         //Check protocol field
         if(rule->protocol == packet->protocol)
         {
            //Check destination port number
            if(packet->destPort >= rule->publicPortMin &&
               packet->destPort <= rule->publicPortMax)
            {
               return rule;
            }
         }
      }
   }
   else
   {
      //Loop through the list of port redirection rules
      for(i = 0; i < context->numPortFwdRules; i++)
      {
         //Point to the current rule
         rule = &context->portFwdRules[i];

         //Check protocol field and source IP address
         if(rule->privateInterface == packet->interface &&
            rule->protocol == packet->protocol &&
            rule->privateIpAddr == packet->srcIpAddr)
         {
            //Check source port number
            if(packet->srcPort >= rule->privatePortMin &&
               packet->srcPort <= rule->privatePortMax)
            {
               return rule;
            }
         }
      }
   }

   //No matching port forwarding rule
   return NULL;
}


/**
 * @brief Search the NAT sessions for a matching entry
 * @param[in] context Pointer to the NAT context
 * @param[in] packet IP packet
 * @return Pointer to the matching session, if any
 **/

NatSession *natMatchSession(NatContext *context, const NatIpPacket *packet)
{
   uint_t i;
   NatSession *session;

   //Inbound or outbound traffic?
   if(packet->interface == context->publicInterface)
   {
      //Loop through the NAT sessions
      for(i = 0; i < context->numSessions; i++)
      {
         //Point to the current session
         session = &context->sessions[i];

         //Matching session?
         if(session->protocol == packet->protocol &&
            session->remoteIpAddr == packet->srcIpAddr &&
            session->publicIpAddr == packet->destIpAddr)
         {
            //Matching transport identifiers?
            if(session->protocol == IPV4_PROTOCOL_TCP &&
               session->remotePort == packet->srcPort &&
               session->publicPort == packet->destPort)
            {
               return session;
            }
            else if(session->protocol == IPV4_PROTOCOL_UDP &&
               session->remotePort == packet->srcPort &&
               session->publicPort == packet->destPort)
            {
               return session;
            }
            else if(session->protocol == IPV4_PROTOCOL_ICMP &&
               session->publicIcmpQueryId == packet->icmpQueryId)
            {
               //Check the type of ICMP message
               if(packet->icmpType == ICMP_TYPE_ECHO_REPLY ||
                  packet->icmpType == ICMP_TYPE_TIMESTAMP_REPLY ||
                  packet->icmpType == ICMP_TYPE_ADDR_MASK_REPLY)
               {
                  return session;
               }
            }
            else
            {
            }
         }
      }
   }
   else
   {
      //Loop through the NAT sessions
      for(i = 0; i < context->numSessions; i++)
      {
         //Point to the current session
         session = &context->sessions[i];

         //Matching session?
         if(session->privateInterface == packet->interface &&
            session->protocol == packet->protocol &&
            session->privateIpAddr == packet->srcIpAddr &&
            session->remoteIpAddr == packet->destIpAddr)
         {
            //Matching transport identifiers?
            if(session->protocol == IPV4_PROTOCOL_TCP &&
               session->privatePort == packet->srcPort &&
               session->remotePort == packet->destPort)
            {
               return session;
            }
            else if(session->protocol == IPV4_PROTOCOL_UDP &&
               session->privatePort == packet->srcPort &&
               session->remotePort == packet->destPort)
            {
               return session;
            }
            else if(session->protocol == IPV4_PROTOCOL_ICMP &&
               session->privateIcmpQueryId == packet->icmpQueryId)
            {
               //Check the type of ICMP message
               if(packet->icmpType == ICMP_TYPE_ECHO_REQUEST ||
                  packet->icmpType == ICMP_TYPE_TIMESTAMP_REQUEST ||
                  packet->icmpType == ICMP_TYPE_ADDR_MASK_REQUEST)
               {
                  return session;
               }
            }
            else
            {
            }
         }
      }
   }

   //No matching session
   return NULL;
}


/**
 * @brief Create a new NAT session
 * @param[in] context Pointer to the NAT context
 * @return Pointer to the newly created session
 **/

NatSession *natCreateSession(NatContext *context)
{
   uint_t i;
   systime_t time;
   NatSession *session;
   NatSession *oldestSession;

   //Loop through the NAT sessions
   for(i = 0; i < context->numSessions; i++)
   {
      //Point to the current session
      session = &context->sessions[i];

      //Check whether the session is a available for use
      if(session->protocol == IPV4_PROTOCOL_NONE)
      {
         return session;
      }
   }

   //Get current time
   time = osGetSystemTime();
   //Keep track of the oldest session
   oldestSession = NULL;

   //Loop through the NAT sessions
   for(i = 0; i < context->numSessions; i++)
   {
      //Point to the current session
      session = &context->sessions[i];

      //Keep track of the oldest session
      if(oldestSession == NULL)
      {
         oldestSession = session;
      }
      else if((time - session->timestamp) > (time - oldestSession->timestamp))
      {
         oldestSession = session;
      }
      else
      {
      }
   }

   //Return a pointer to the NAT session
   return oldestSession;
}


/**
 * @brief Allocate a new port number
 * @param[in] context Pointer to the NAT context
 * @return Port number
 **/

uint16_t natAllocatePort(NatContext *context)
{
   uint_t i;
   uint16_t port;
   bool_t valid;
   NatSession *session;

   //Assign a new port number
   do
   {
      //Generate a random port number
      port = netGenerateRandRange(NAT_TCP_UDP_PORT_MIN, NAT_TCP_UDP_PORT_MAX);

      //Loop through the NAT sessions
      for(valid = TRUE, i = 0; i < context->numSessions; i++)
      {
         //Point to the current session
         session = &context->sessions[i];

         //TCP or UDP session?
         if(session->protocol == IPV4_PROTOCOL_TCP ||
            session->protocol == IPV4_PROTOCOL_UDP)
         {
            //Test whether the port number is a duplicate
            if(session->publicPort == port)
            {
               valid = FALSE;
            }
         }
      }

      //Repeat as necessary until a unique port number is generated
   } while(!valid);

   //Return the port number
   return port;
}


/**
 * @brief Allocate a new ICMP query identifier
 * @param[in] context Pointer to the NAT context
 * @return ICMP query identifier
 **/

uint16_t natAllocateIcmpQueryId(NatContext *context)
{
   uint_t i;
   uint16_t id;
   bool_t valid;
   NatSession *session;

   //Assign a new ICMP query identifier
   do
   {
      //Generate a random identifier
      id = netGenerateRandRange(NAT_ICMP_QUERY_ID_MIN, NAT_ICMP_QUERY_ID_MAX);

      //Loop through the NAT sessions
      for(valid = TRUE, i = 0; i < context->numSessions; i++)
      {
         //Point to the current session
         session = &context->sessions[i];

         //ICMP session
         if(session->protocol == IPV4_PROTOCOL_ICMP)
         {
            //Test whether the ICMP query identifier is a duplicate
            if(session->publicIcmpQueryId == id)
            {
               valid = FALSE;
            }
         }
      }

      //Repeat as necessary until a unique identifier is generated
   } while(!valid);

   //Return the ICMP query identifier
   return id;
}


/**
 * @brief Parse transport header (TCP, UDP or ICMP)
 * @param[in,out] packet IP packet
 * @return Error code
 **/

error_t natParseTransportHeader(NatIpPacket *packet)
{
   error_t error;
   size_t length;

   //Initialize status code
   error = NO_ERROR;

   //Check packet type
   if(packet->protocol == IPV4_PROTOCOL_TCP)
   {
      TcpHeader *header;

      //Point to the TCP header
      header = netBufferAt(packet->buffer, packet->offset, sizeof(TcpHeader));

      //Valid TCP header?
      if(header != NULL)
      {
         //Retrieve source and destination ports
         packet->srcPort = ntohs(header->srcPort);
         packet->destPort = ntohs(header->destPort);
      }
      else
      {
         //Report an error
         error = ERROR_INVALID_PACKET;
      }
   }
   else if(packet->protocol == IPV4_PROTOCOL_UDP)
   {
      UdpHeader *header;

      //Point to the UDP header
      header = netBufferAt(packet->buffer, packet->offset, sizeof(UdpHeader));

      //Valid UDP header?
      if(header != NULL)
      {
         //Retrieve source and destination ports
         packet->srcPort = ntohs(header->srcPort);
         packet->destPort = ntohs(header->destPort);
      }
      else
      {
         //Report an error
         error = ERROR_INVALID_PACKET;
      }
   }
   else if(packet->protocol == IPV4_PROTOCOL_ICMP)
   {
      IcmpQueryMessage *header;

      //Point to the ICMP header
      header = netBufferAt(packet->buffer, packet->offset,
         sizeof(IcmpQueryMessage));

      //Valid ICMP header?
      if(header != NULL)
      {
         //Check the type of ICMP message
         if(header->type == ICMP_TYPE_ECHO_REQUEST ||
            header->type == ICMP_TYPE_ECHO_REPLY ||
            header->type == ICMP_TYPE_TIMESTAMP_REQUEST ||
            header->type == ICMP_TYPE_TIMESTAMP_REPLY ||
            header->type == ICMP_TYPE_ADDR_MASK_REQUEST ||
            header->type == ICMP_TYPE_ADDR_MASK_REPLY)
         {
            //Save ICMP message type
            packet->icmpType = header->type;
            //Retrieve the value of the identifier
            packet->icmpQueryId = ntohs(header->identifier);
         }
         else
         {
            //Report an error
            error = ERROR_UNKNOWN_TYPE;
         }
      }
      else
      {
         //Report an error
         error = ERROR_INVALID_PACKET;
      }
   }
   else
   {
      //Unknown protocol
      error = ERROR_INVALID_PROTOCOL;
   }

   //Return status code
   return error;
}


/**
 * @brief Translate transport header (TCP, UDP or ICMP)
 * @param[in] packet IP packet
 * @param[in] pseudoHeader Pointer to the pseudo header
 * @param[in] buffer Multi-part buffer that holds the IP packet
 * @param[in] offset Packet Offset to the payload of the IP packet
 * @return Error code
 **/

error_t natTranslateTransportHeader(const NatIpPacket *packet,
   const Ipv4PseudoHeader *pseudoHeader, const NetBuffer *buffer,
   size_t offset)
{
   error_t error;

   //Initialize status code
   error = NO_ERROR;

   //Check packet type
   if(packet->protocol == IPV4_PROTOCOL_TCP)
   {
      TcpHeader *header;

      //Point to the TCP header
      header = netBufferAt(buffer, offset, sizeof(TcpHeader));

      //Valid TCP header?
      if(header != NULL)
      {
         //Replace source and destination ports
         header->srcPort = htons(packet->srcPort);
         header->destPort = htons(packet->destPort);
         header->checksum = 0;

         //Recompute message checksum
         header->checksum = ipCalcUpperLayerChecksumEx(pseudoHeader,
            sizeof(Ipv4PseudoHeader), buffer, offset,
            ntohs(pseudoHeader->length));
      }
   }
   else if(packet->protocol == IPV4_PROTOCOL_UDP)
   {
      UdpHeader *header;

      //Point to the UDP header
      header = netBufferAt(buffer, offset, sizeof(UdpHeader));

      //Valid UDP header?
      if(header != NULL)
      {
         //Replace source and destination ports
         header->srcPort = htons(packet->srcPort);
         header->destPort = htons(packet->destPort);
         header->checksum = 0;

         //Recompute message checksum
         header->checksum = ipCalcUpperLayerChecksumEx(pseudoHeader,
            sizeof(Ipv4PseudoHeader), buffer, offset,
            ntohs(pseudoHeader->length));
      }
   }
   else if(packet->protocol == IPV4_PROTOCOL_ICMP)
   {
      IcmpQueryMessage *header;

      //Point to the ICMP header
      header = netBufferAt(buffer, offset, sizeof(IcmpQueryMessage));

      //Valid ICMP header?
      if(header != NULL)
      {
         //A NAPT device translates the ICMP Query Id and the associated
         //checksum in the ICMP header prior to forwarding (refer to
         //RFC 5508, section 3.1)
         header->identifier = htons(packet->icmpQueryId);
         header->checksum = 0;

         //Recompute message checksum
         header->checksum = ipCalcChecksumEx(buffer, offset,
            ntohs(pseudoHeader->length));
      }
   }
   else
   {
      //Unknown protocol
      error = ERROR_INVALID_PROTOCOL;
   }

   //Return status code
   return error;
}


/**
 * @brief Dump IP packet for debugging purpose
 * @param[in] packet IP packet
 **/

void natDumpPacket(const NatIpPacket *packet)
{
#if (NAT_TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   size_t length;
   const char_t *name;

   //Retrieve the length of the IP packet
   length = netBufferGetLength(packet->buffer);

   //Convert the protocol to string representation
   if(packet->protocol == IPV4_PROTOCOL_TCP)
   {
      name = "TCP";
   }
   else if(packet->protocol == IPV4_PROTOCOL_UDP)
   {
      name = "UDP";
   }
   else if(packet->protocol == IPV4_PROTOCOL_ICMP)
   {
      name = "ICMP";
   }
   else
   {
      name = "Unknown";
   }

   //Dump IP packet
   TRACE_DEBUG("  Length = %" PRIuSIZE "\r\n", length);
   TRACE_DEBUG("  Protocol = %d (%s)\r\n", packet->protocol, name);

   //Check packet type
   if(packet->protocol == IPV4_PROTOCOL_TCP ||
      packet->protocol == IPV4_PROTOCOL_UDP)
   {
      TRACE_DEBUG("  Src IP Addr = %s\r\n", ipv4AddrToString(packet->srcIpAddr, NULL));
      TRACE_DEBUG("  Src Port = %" PRIu16 "\r\n", packet->srcPort);
      TRACE_DEBUG("  Dest IP Addr = %s\r\n", ipv4AddrToString(packet->destIpAddr, NULL));
      TRACE_DEBUG("  Dest Port = %" PRIu16 "\r\n", packet->destPort);
   }
   else
   {
      TRACE_DEBUG("  Src IP Addr = %s\r\n", ipv4AddrToString(packet->srcIpAddr, NULL));
      TRACE_DEBUG("  Dest IP Addr = %s\r\n", ipv4AddrToString(packet->destIpAddr, NULL));
      TRACE_DEBUG("  Type = %" PRIu8 "\r\n", packet->icmpType);
      TRACE_DEBUG("  Identifier = %" PRIu16 "\r\n", packet->icmpQueryId);
   }
#endif
}

#endif
