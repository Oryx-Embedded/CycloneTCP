/**
 * @file dhcpv6_relay_misc.
 * @brief Helper functions for DHCPv6 relay agent
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
 * @section Description
 *
 * DHCPv6 Relay-Agents are deployed to forward DHCPv6 messages between clients
 * and servers when they are not on the same IPv6 link and are often implemented
 * alongside a routing function in a common node. Refer to RFC 3315
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.6.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL DHCPV6_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "ipv6/ipv6_multicast.h"
#include "dhcpv6/dhcpv6_relay.h"
#include "dhcpv6/dhcpv6_relay_misc.h"
#include "dhcpv6/dhcpv6_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && DHCPV6_RELAY_SUPPORT == ENABLED)


/**
 * @brief Open client-facing socket
 * @param[in] context Pointer to the DHCPv6 relay agent context
 * @param[in] index Zero-based index
 * @return Error code
 * @return 
 **/

error_t dhcpv6RelayOpenClientSocket(Dhcpv6RelayContext *context, uint_t index)
{
   error_t error;

   //Open a UDP socket
   context->clientSockets[index] = socketOpenEx(context->netContext,
      SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);

   //Valid socket handle?
   if(context->serverSocket != NULL)
   {
      //Explicitly associate the socket with the relevant interface
      error = socketBindToInterface(context->clientSockets[index],
         context->clientInterfaces[index]);

      //Check status code
      if(!error)
      {
         //Relay agents listen for DHCPv6 messages on UDP port 547
         error = socketBind(context->clientSockets[index], &IP_ADDR_ANY,
            DHCPV6_SERVER_PORT);
      }

      //Check status code
      if(!error)
      {
         IpAddr multicastAddr;

         //The All_DHCP_Relay_Agents_and_Servers address (ff02::1:2) is a
         //link-scoped multicast address used by a client to communicate
         //with neighboring relay agents and servers
         multicastAddr.length = sizeof(Ipv6Addr);
         multicastAddr.ipv6Addr = DHCPV6_ALL_RELAY_AGENTS_AND_SERVERS_ADDR;

         //All servers and relay agents are members of this multicast group
         //(refer to RFC 8415, section 7.1)
         error = socketJoinMulticastGroup(context->clientSockets[index],
            &multicastAddr);
      }
   }
   else
   {
      //Report an error
      error = ERROR_OPEN_FAILED;
   }

   //Return status code
   return error;
}


/**
 * @brief Open server-facing socket
 * @param[in] context Pointer to the DHCPv6 relay agent context
 * @return Error code
 **/

error_t dhcpv6RelayOpenServerSocket(Dhcpv6RelayContext *context)
{
   error_t error;

   //Open a UDP socket
   context->serverSocket = socketOpenEx(context->netContext, SOCKET_TYPE_DGRAM,
      SOCKET_IP_PROTO_UDP);

   //Valid socket handle?
   if(context->serverSocket != NULL)
   {
      //Explicitly associate the socket with the relevant interface
      error = socketBindToInterface(context->serverSocket,
         context->serverInterface);

      //Check status code
      if(!error)
      {
         //Relay agents listen for DHCPv6 messages on UDP port 547
         error = socketBind(context->serverSocket, &IP_ADDR_ANY,
            DHCPV6_SERVER_PORT);
      }

      //Check status code
      if(!error)
      {
         //Only accept datagrams with source port number 547
         error = socketConnect(context->serverSocket, &IP_ADDR_ANY,
            DHCPV6_SERVER_PORT);
      }

      //Check status code
      if(!error)
      {
         IpAddr multicastAddr;

         //The All_DHCP_Relay_Agents_and_Servers address (ff02::1:2) is a
         //link-scoped multicast address used by a client to communicate
         //with neighboring relay agents and servers
         multicastAddr.length = sizeof(Ipv6Addr);
         multicastAddr.ipv6Addr = DHCPV6_ALL_RELAY_AGENTS_AND_SERVERS_ADDR;

         //All servers and relay agents are members of this multicast group
         //(refer to RFC 8415, section 7.1)
         error = socketJoinMulticastGroup(context->serverSocket,
            &multicastAddr);
      }

      //Check status code
      if(!error)
      {
         //If the relay agent relays messages to the All_DHCP_Servers address
         //or other multicast addresses, it sets the Hop Limit field to 8
         //(refer to RFC 8415, section 19)
         error = socketSetTtl(context->serverSocket,
            DHCPV6_RELAY_HOP_COUNT_LIMIT);
      }
   }
   else
   {
      //Report an error
      error = ERROR_OPEN_FAILED;
   }

   //Return status code
   return error;
}


/**
 * @brief Forward client message
 * @param[in] context Pointer to the DHCPv6 relay agent context
 * @param[in] index Index identifying the interface on which the message was received
 * @return Error code
 **/

error_t dhcpv6ForwardClientMessage(Dhcpv6RelayContext *context, uint_t index)
{
   error_t error;
   uint32_t interfaceId;
   size_t inputMessageLen;
   size_t outputMessageLen;
   Dhcpv6RelayMessage *inputMessage;
   Dhcpv6RelayMessage *outputMessage;
   Dhcpv6Option *option;
   IpAddr ipAddr;
   uint16_t port;

   //Point to the buffer where to store the incoming DHCPv6 message
   inputMessage = (Dhcpv6RelayMessage *) (context->buffer +
      DHCPV6_RELAY_FORWARDING_OVERHEAD);

   //Message that will be forwarded by the DHCPv6 relay agent
   outputMessage = (Dhcpv6RelayMessage *) context->buffer;

   //Read incoming message
   error = socketReceiveFrom(context->clientSockets[index], &ipAddr, &port,
      inputMessage, DHCPV6_MAX_MSG_SIZE - DHCPV6_RELAY_FORWARDING_OVERHEAD,
      &inputMessageLen, 0);
   //Any error to report?
   if(error)
      return error;

   //Debug message
   TRACE_INFO("\r\nDHCPv6 message received on client-facing interface %s (%" PRIuSIZE " bytes)...\r\n",
      context->clientInterfaces[index]->name, inputMessageLen);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(inputMessage, inputMessageLen);

   //The source address must be a valid IPv6 address
   if(ipAddr.length != sizeof(Ipv6Addr))
      return ERROR_INVALID_ADDRESS;

   //Check the length of the DHCPv6 message
   if(inputMessageLen < sizeof(Dhcpv6Message))
      return ERROR_INVALID_MESSAGE;

   //When the relay agent receives a valid message to be relayed, it constructs
   //a new Relay-Forward message
   outputMessage->msgType = DHCPV6_MSG_TYPE_RELAY_FORW;

   //Inspect message type
   if(inputMessage->msgType == DHCPV6_MSG_TYPE_SOLICIT ||
      inputMessage->msgType == DHCPV6_MSG_TYPE_REQUEST ||
      inputMessage->msgType == DHCPV6_MSG_TYPE_CONFIRM ||
      inputMessage->msgType == DHCPV6_MSG_TYPE_RENEW ||
      inputMessage->msgType == DHCPV6_MSG_TYPE_REBIND ||
      inputMessage->msgType == DHCPV6_MSG_TYPE_RELEASE ||
      inputMessage->msgType == DHCPV6_MSG_TYPE_DECLINE ||
      inputMessage->msgType == DHCPV6_MSG_TYPE_INFO_REQUEST)
   {
      //Clients use UDP source port 546
      if(port != DHCPV6_CLIENT_PORT)
         return ERROR_INVALID_PORT;

      //If the relay agent received the message to be relayed from a client,
      //the hop-count in the Relay-Forward message is set to 0
      outputMessage->hopCount = 0;
   }
   else if(inputMessage->msgType == DHCPV6_MSG_TYPE_RELAY_FORW)
   {
      //Relay agents use UDP source port 547
      if(port != DHCPV6_SERVER_PORT)
         return ERROR_INVALID_PORT;

      //If the message received by the relay agent is a Relay-Forward message
      //and the hop-count in the message is greater than or equal to
      //HOP_COUNT_LIMIT, the relay agent discards the received message
      if(inputMessage->hopCount >= DHCPV6_RELAY_HOP_COUNT_LIMIT)
         return ERROR_INVALID_MESSAGE;

      //Set the hop-count field to the value of the hop-count field in the
      //received message incremented by 1
      outputMessage->hopCount = inputMessage->hopCount + 1;
   }
   else
   {
      //Discard ADVERTISE, REPLY, RECONFIGURE and RELAY-REPL messages
      return ERROR_INVALID_MESSAGE;
   }

   //Set the link-address field to the unspecified address
   outputMessage->linkAddress = IPV6_UNSPECIFIED_ADDR;

   //Copy the source address from the header of the IP datagram in which the
   //message was received to the peer-address field
   outputMessage->peerAddress = ipAddr.ipv6Addr;

   //Size of the Relay-Forward message
   outputMessageLen = sizeof(Dhcpv6RelayMessage);

   //Get the interface identifier
   interfaceId = context->clientInterfaces[index]->id;
   //Convert the 32-bit integer to network byte order
   interfaceId = htonl(interfaceId);

   //If the relay agent cannot use the address in the link-address field
   //to identify the interface through which the response to the client
   //will be relayed, the relay agent must include an Interface ID option
   dhcpv6AddOption(outputMessage, &outputMessageLen, DHCPV6_OPT_INTERFACE_ID,
      &interfaceId, sizeof(interfaceId));

   //The relay agent copies the received DHCPv6 message into a Relay Message
   //option in the new message (refer to RFC 8415, section 19.1)
   option = dhcpv6AddOption(outputMessage, &outputMessageLen,
      DHCPV6_OPT_RELAY_MSG, NULL, 0);

   //Set the appropriate length of the option
   option->length = htons(inputMessageLen);
   //Adjust the length of the Relay-Forward message
   outputMessageLen += inputMessageLen;

   //Debug message
   TRACE_INFO("Forwarding DHCPv6 message on network-facing interface %s (%" PRIuSIZE " bytes)...\r\n",
      context->serverInterface->name, outputMessageLen);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(outputMessage, outputMessageLen);

   //The destination address is selected by the network administrator
   ipAddr.length = sizeof(Ipv6Addr);
   ipAddr.ipv6Addr = context->serverIpAddr;

   //Relay the client message to the server
   return socketSendTo(context->serverSocket, &ipAddr, DHCPV6_SERVER_PORT,
      outputMessage, outputMessageLen, NULL, 0);
}


/**
 * @brief Forward Relay-Reply message
 * @param[in] context Pointer to the DHCPv6 relay agent context
 * @return Error code
 **/

error_t dhcpv6ForwardRelayReplyMessage(Dhcpv6RelayContext *context)
{
   error_t error;
   uint_t i;
   uint32_t interfaceId;
   size_t inputMessageLen;
   size_t outputMessageLen;
   Dhcpv6RelayMessage *inputMessage;
   Dhcpv6Message *outputMessage;
   Dhcpv6Option *option;
   IpAddr ipAddr;
   uint16_t port;

   //Point to the buffer where to store the incoming DHCPv6 message
   inputMessage = (Dhcpv6RelayMessage *) context->buffer;

   //Read incoming message
   error = socketReceiveFrom(context->serverSocket, &ipAddr, &port,
      inputMessage, DHCPV6_MAX_MSG_SIZE, &inputMessageLen, 0);
   //Any error to report?
   if(error)
      return error;

   //Debug message
   TRACE_INFO("\r\nDHCPv6 message received on network-facing interface %s (%" PRIuSIZE " bytes)...\r\n",
      context->serverInterface->name, inputMessageLen);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(inputMessage, inputMessageLen);

   //Check the length of the DHCPv6 message
   if(inputMessageLen < sizeof(Dhcpv6RelayMessage))
      return ERROR_INVALID_MESSAGE;

   //Inspect the message type and only forward Relay-Reply messages. Other
   //DHCPv6 message types must be silently discarded
   if(inputMessage->msgType != DHCPV6_MSG_TYPE_RELAY_REPL)
      return ERROR_INVALID_MESSAGE;

   //Get the length of the Options field
   inputMessageLen -= sizeof(Dhcpv6Message);

   //The Relay-Reply message must include a Relay Message option
   option = dhcpv6GetOption(inputMessage->options, inputMessageLen,
      DHCPV6_OPT_RELAY_MSG);
   //Failed to retrieve specified option?
   if(option == NULL || ntohs(option->length) < sizeof(Dhcpv6Message))
      return ERROR_INVALID_MESSAGE;

   //The relay agent extracts the message from the Relay Message option. Relay
   //agents must not modify the message (refer to RFC 8415, section 19.2)
   outputMessage = (Dhcpv6Message *) option->value;

   //Save the length of the message
   outputMessageLen = ntohs(option->length);

   //Check whether an Interface ID option is included in the Relay-Reply
   option = dhcpv6GetOption(inputMessage->options, inputMessageLen,
      DHCPV6_OPT_INTERFACE_ID);
   //Failed to retrieve specified option?
   if(option == NULL || ntohs(option->length) != sizeof(interfaceId))
      return ERROR_INVALID_MESSAGE;

   //Read the Interface ID option contents
   osMemcpy(&interfaceId, option->value, sizeof(interfaceId));
   //Convert the 32-bit integer from network byte order
   interfaceId = ntohl(interfaceId);

   //Loop through client-facing interfaces
   for(i = 0; i < context->numClientInterfaces; i++)
   {
      //Check whether the current interface matches the Interface ID option
      if(context->clientInterfaces[i]->id == interfaceId)
      {
         break;
      }
   }

   //Unknown interface identifier?
   if(i >= context->numClientInterfaces)
      return ERROR_WRONG_IDENTIFIER;

   //Debug message
   TRACE_INFO("Forwarding DHCPv6 message on client-facing interface %s (%" PRIuSIZE " bytes)...\r\n",
      context->clientInterfaces[i]->name, outputMessageLen);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(outputMessage, outputMessageLen);

   //Relay the message to the address contained in the peer-address field of
   //the Relay-reply message
   ipAddr.length = sizeof(Ipv6Addr);
   ipAddr.ipv6Addr = inputMessage->peerAddress;

   //Select the destination port number to use
   if(outputMessage->msgType == DHCPV6_MSG_TYPE_RELAY_REPL)
   {
      //The destination port number is set to 547 if the Relay-reply message is
      //sent to other relay agents
      port = DHCPV6_SERVER_PORT;
   }
   else
   {
      //The destination port number is set to 546 if the message extracted from
      //the Relay-reply message is sent to the client
      port = DHCPV6_CLIENT_PORT;
   }

   //Relay the DHCPv6 message from the server to the client on the link
   //identified by the Interface ID option
   return socketSendTo(context->clientSockets[i], &ipAddr, port, outputMessage,
      outputMessageLen, NULL, 0);
}

#endif
