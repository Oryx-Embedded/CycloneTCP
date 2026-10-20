/**
 * @file dhcp_server_misc.c
 * @brief Helper functions for DHCP server
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
#define TRACE_LEVEL DHCP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "dhcp/dhcp_server.h"
#include "dhcp/dhcp_server_misc.h"
#include "dhcp/dhcp_common.h"
#include "dhcp/dhcp_debug.h"
#include "date_time.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED && DHCP_SERVER_SUPPORT == ENABLED)


/**
 * @brief DHCP server timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * manage DHCP server operation
 *
 * @param[in] context Pointer to the DHCP server context
 **/

void dhcpServerTick(DhcpServerContext *context)
{
   uint_t i;
   systime_t time;
   systime_t leaseTime;
   DhcpServerBinding *binding;

   //Make sure the DHCP server has been properly instantiated
   if(context == NULL)
   {
      return;
   }

   //Get current time
   time = osGetSystemTime();
   //Convert the lease time to milliseconds
   leaseTime = context->leaseTime * 1000;

   //Loop through the list of bindings
   for(i = 0; i < DHCP_SERVER_MAX_CLIENTS; i++)
   {
      //Point to the current binding
      binding = &context->clientBinding[i];

      //Check the state of the binding
      if(binding->state == DHCP_SERVER_BINDING_STATE_COMMITTED)
      {
         //The lease has expired
         if((time - binding->timestamp) >= leaseTime)
         {
            //The address lease is not more valid
            binding->state = DHCP_SERVER_BINDING_STATE_RESERVED;
         }
      }
      else if(binding->state == DHCP_SERVER_BINDING_STATE_DECLINED)
      {
         //The declined network address can be reassigned once the hold time
         //has elapsed
         if((time - binding->timestamp) >= DHCP_SERVER_DECLINE_HOLD_TIME)
         {
            //Remove the binding from the list
            osMemset(binding, 0, sizeof(DhcpServerBinding));
         }
      }
      else
      {
         //Unused or reserved binding
      }
   }
}


/**
 * @brief Process incoming DHCP message
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader UDP pseudo header
 * @param[in] udpHeader UDP header
 * @param[in] buffer Multi-part buffer containing the incoming DHCP message
 * @param[in] offset Offset to the first byte of the DHCP message
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 * @param[in] param Pointer to the DHCP server context
 **/

void dhcpServerProcessMessage(NetInterface *interface,
   const IpPseudoHeader *pseudoHeader, const UdpHeader *udpHeader,
   const NetBuffer *buffer, size_t offset, const NetRxAncillary *ancillary,
   void *param)
{
   error_t error;
   size_t length;
   DhcpServerContext *context;
   DhcpMessage *message;
   DhcpOption *option;
   DhcpMessageType type;

   //Point to the DHCP server context
   context = (DhcpServerContext *) param;

   //Retrieve the length of the DHCP message
   length = netBufferGetLength(buffer) - offset;

   //Make sure the DHCP message is valid
   if(length < sizeof(DhcpMessage) || length > DHCP_MAX_MSG_SIZE)
   {
      return;
   }

   //Point to the beginning of the DHCP message
   message = netBufferAt(buffer, offset, length);
   //Sanity check
   if(message == NULL)
   {
      return;
   }

   //Debug message
   TRACE_DEBUG("\r\n%s: DHCP message received (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), length);

   //Dump the contents of the message for debugging purpose
   dhcpDumpMessage(message, length);

   //Check opcode
   if(message->op != DHCP_OPCODE_BOOTREQUEST)
   {
      return;
   }

   //Enforce hardware type
   if(message->htype != DHCP_HARDWARE_TYPE_ETH)
   {
      return;
   }

   //Check the length of the hardware address
   if(message->hlen != sizeof(MacAddr))
   {
      return;
   }

   //The client hardware address must be a valid unicast MAC address
   if(macCompAddr(&message->chaddr, &MAC_UNSPECIFIED_ADDR) ||
      macIsMulticastAddr(&message->chaddr))
   {
      return;
   }

   //Check magic cookie
   if(message->magicCookie != HTONL(DHCP_MAGIC_COOKIE))
   {
      return;
   }

   //Search the DHCP message for the DHCP Message Type option
   option = dhcpGetOption(message, length, DHCP_OPT_DHCP_MESSAGE_TYPE);

   //Missing or malformed DHCP Message Type option?
   if(option == NULL || option->length != 1)
   {
      return;
   }

   //Retrieve message type
   type = (DhcpMessageType) option->value[0];

   //Any registered callback?
   if(context->parseOptionsCallback != NULL)
   {
      //Invoke user callback function
      error = context->parseOptionsCallback(context, message, length,
         type);
      //Check status code
      if(error)
      {
         return;
      }
   }

   //Check message type
   switch(type)
   {
   case DHCP_MSG_TYPE_DISCOVER:
      //Parse DHCPDISCOVER message
      dhcpServerParseDiscover(context, message, length);
      break;

   case DHCP_MSG_TYPE_REQUEST:
      //Parse DHCPREQUEST message
      dhcpServerParseRequest(context, message, length);
      break;

   case DHCP_MSG_TYPE_DECLINE:
      //Parse DHCPDECLINE message
      dhcpServerParseDecline(context, message, length);
      break;

   case DHCP_MSG_TYPE_RELEASE:
      //Parse DHCPRELEASE message
      dhcpServerParseRelease(context, message, length);
      break;

   case DHCP_MSG_TYPE_INFORM:
      //Parse DHCPINFORM message
      dhcpServerParseInform(context, message, length);
      break;

   default:
      //Silently drop incoming message
      break;
   }
}


/**
 * @brief Parse DHCPDISCOVER message
 * @param[in] context Pointer to the DHCP server context
 * @param[in] message Pointer to the incoming DHCP message
 * @param[in] length Length of the incoming message to parse
 **/

void dhcpServerParseDiscover(DhcpServerContext *context,
   const DhcpMessage *message, size_t length)
{
   error_t error;
   uint_t i;
   bool_t rapidCommit;
   NetInterface *interface;
   Ipv4Addr requestedIpAddr;
   DhcpOption *option;
   DhcpServerBinding *binding;

   //Point to the underlying network interface
   interface = context->interface;
   //Index of the IP address assigned to the DHCP server
   i = context->ipAddrIndex;

   //Search the DHCP message for the Server Identifier option
   option = dhcpGetOption(message, length, DHCP_OPT_SERVER_ID);

   //Valid option found?
   if(option != NULL && option->length == 4)
   {
      //Unexpected server identifier?
      if(!ipv4CompAddr(option->value, &interface->ipv4Context.addrList[i].addr))
      {
         return;
      }
   }

   //Search the DHCP message for the Requested IP Address option
   option = dhcpGetOption(message, length, DHCP_OPT_REQUESTED_IP_ADDR);

   //The client may include the 'requested IP address' option to suggest
   //that a particular IP address be assigned
   if(option != NULL && option->length == 4)
   {
      ipv4CopyAddr(&requestedIpAddr, option->value);
   }
   else
   {
      requestedIpAddr = IPV4_UNSPECIFIED_ADDR;
   }

   //Search the DHCP message for the Rapid Commit option
   option = dhcpGetOption(message, length, DHCP_OPT_RAPID_COMMIT);

   //The Rapid Commit option is used to indicate the use of the two-message
   //exchange for address assignment (refer to RFC 4039, section 4)
   if(option != NULL && option->length == 0)
   {
      rapidCommit = context->rapidCommit ? TRUE : FALSE;
   }
   else
   {
      rapidCommit = FALSE;
   }

   //Search the list for a matching binding
   binding = dhcpServerFindBindingByMacAddr(context, &message->chaddr);

   //Matching binding found?
   if(binding != NULL)
   {
      //Different IP address than cached?
      if(requestedIpAddr != binding->ipAddr)
      {
         //Ensure the IP address is in the server's pool of available addresses
         if(dhcpServerIsIpAddrInPool(context, requestedIpAddr))
         {
            //Make sure the IP address is not already allocated
            if(dhcpServerFindBindingByIpAddr(context, requestedIpAddr) == NULL)
            {
               //The new network address is only offered to the client. It is
               //not committed until the server receives a DHCPREQUEST message
               binding->state = DHCP_SERVER_BINDING_STATE_RESERVED;

               //Record IP address
               binding->ipAddr = requestedIpAddr;
               //Save current time
               binding->timestamp = osGetSystemTime();
            }
         }
      }

      //Address reserved for the client?
      if(binding->state == DHCP_SERVER_BINDING_STATE_RESERVED)
      {
         //Refresh the reservation so that the entry is not recycled while
         //the client is still active
         binding->timestamp = osGetSystemTime();
      }

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //Create a new binding
      binding = dhcpServerCreateBinding(context);

      //Binding successfully created?
      if(binding != NULL)
      {
         //Requested IP address in the pool and not already allocated?
         if(dhcpServerIsIpAddrInPool(context, requestedIpAddr) &&
            dhcpServerFindBindingByIpAddr(context, requestedIpAddr) == NULL)
         {
            //Record IP address
            binding->ipAddr = requestedIpAddr;
            //Successful processing
            error = NO_ERROR;
         }
         else
         {
            //Retrieve the next available IP address from the pool of addresses
            error = dhcpServerGetNextIpAddr(context, &binding->ipAddr);
         }

         //Check status code
         if(!error)
         {
            //The network address is reserved for the client
            binding->state = DHCP_SERVER_BINDING_STATE_RESERVED;
            //Record MAC address
            binding->macAddr = message->chaddr;
            //Save current time
            binding->timestamp = osGetSystemTime();
         }
      }
      else
      {
         //Failed to create a new binding
         error = ERROR_FAILURE;
      }
   }

   //Check status code
   if(!error)
   {
      //Two-message exchange?
      if(rapidCommit)
      {
         //Commit network address
         binding->state = DHCP_SERVER_BINDING_STATE_COMMITTED;
         //Save lease start time
         binding->timestamp = osGetSystemTime();

         //The server responds with a DHCPACK message
         dhcpServerSendReply(context, DHCP_MSG_TYPE_ACK, binding->ipAddr,
            message, length);
      }
      else
      {
         //The server responds with a DHCPOFFER message that includes an
         //available network address in the 'yiaddr' field (and other
         //configuration parameters in DHCP options)
         dhcpServerSendReply(context, DHCP_MSG_TYPE_OFFER, binding->ipAddr,
            message, length);
      }
   }
}


/**
 * @brief Parse DHCPREQUEST message
 * @param[in] context Pointer to the DHCP server context
 * @param[in] message Pointer to the incoming DHCP message
 * @param[in] length Length of the incoming message to parse
 **/

void dhcpServerParseRequest(DhcpServerContext *context,
   const DhcpMessage *message, size_t length)
{
   uint_t i;
   bool_t initReboot;
   bool_t knownClient;
   NetInterface *interface;
   Ipv4Addr clientIpAddr;
   DhcpOption *option;
   DhcpServerBinding *binding;

   //Initialize flags
   initReboot = FALSE;
   knownClient = FALSE;

   //Point to the underlying network interface
   interface = context->interface;
   //Index of the IP address assigned to the DHCP server
   i = context->ipAddrIndex;

   //Search the DHCP message for the Server Identifier option
   option = dhcpGetOption(message, length, DHCP_OPT_SERVER_ID);

   //Valid option found?
   if(option != NULL && option->length == 4)
   {
      //Unexpected server identifier?
      if(!ipv4CompAddr(option->value, &interface->ipv4Context.addrList[i].addr))
      {
         //Servers not selected by the DHCPREQUEST message use the message as
         //notification that the client has declined that server's offer
         //(refer to RFC 2131, section 3.1)
         binding = dhcpServerFindBindingByMacAddr(context, &message->chaddr);

         //Matching binding found?
         if(binding != NULL)
         {
            //Return the offered network address to the pool
            if(binding->state == DHCP_SERVER_BINDING_STATE_RESERVED)
            {
               osMemset(binding, 0, sizeof(DhcpServerBinding));
            }
         }

         //Exit immediately
         return;
      }
   }
   else
   {
      //Check whether the client is in the INIT-REBOOT state
      if(message->ciaddr == IPV4_UNSPECIFIED_ADDR)
      {
         initReboot = TRUE;
      }
   }

   //Check the 'ciaddr' field
   if(message->ciaddr != IPV4_UNSPECIFIED_ADDR)
   {
      //Save client's network address
      clientIpAddr = message->ciaddr;
   }
   else
   {
      //Search the DHCP message for the Requested IP Address option
      option = dhcpGetOption(message, length, DHCP_OPT_REQUESTED_IP_ADDR);

      //Valid option found?
      if(option != NULL && option->length == 4)
      {
         ipv4CopyAddr(&clientIpAddr, option->value);
      }
      else
      {
         clientIpAddr = IPV4_UNSPECIFIED_ADDR;
      }
   }

   //Valid client IP address?
   if(clientIpAddr != IPV4_UNSPECIFIED_ADDR)
   {
      //Search the list for a matching binding
      binding = dhcpServerFindBindingByMacAddr(context, &message->chaddr);

      //Matching binding found?
      if(binding != NULL)
      {
         //The server has a record of this client
         knownClient = TRUE;

         //Make sure the client's IP address is valid
         if(clientIpAddr == binding->ipAddr)
         {
            //Commit network address
            binding->state = DHCP_SERVER_BINDING_STATE_COMMITTED;
            //Save lease start time
            binding->timestamp = osGetSystemTime();

            //The server responds with a DHCPACK message containing the
            //configuration parameters for the requesting client
            dhcpServerSendReply(context, DHCP_MSG_TYPE_ACK, binding->ipAddr,
               message, length);

            //Exit immediately
            return;
         }
      }
      else
      {
         //Ensure the IP address is in the server's pool of available addresses
         if(dhcpServerIsIpAddrInPool(context, clientIpAddr))
         {
            //Make sure the IP address is not already allocated
            if(dhcpServerFindBindingByIpAddr(context, clientIpAddr) == NULL)
            {
               //Create a new binding
               binding = dhcpServerCreateBinding(context);

               //Binding successfully created?
               if(binding != NULL)
               {
                  //Commit network address
                  binding->state = DHCP_SERVER_BINDING_STATE_COMMITTED;
                  //Record MAC address
                  binding->macAddr = message->chaddr;
                  //Record IP address
                  binding->ipAddr = clientIpAddr;
                  //Save current time
                  binding->timestamp = osGetSystemTime();

                  //The server responds with a DHCPACK message containing the
                  //configuration parameters for the requesting client
                  dhcpServerSendReply(context, DHCP_MSG_TYPE_ACK,
                     binding->ipAddr, message, length);

                  //Exit immediately
                  return;
               }
            }
         }
      }
   }

   //Client in INIT-REBOOT state and unknown to the server?
   if(initReboot && !knownClient)
   {
      //Check whether the client is on the wrong network
      if(clientIpAddr != IPV4_UNSPECIFIED_ADDR &&
         !ipv4CompSubnet(clientIpAddr, context->ipAddrRangeMin,
         context->subnetMask))
      {
         //If the DHCP server detects that the client is on the wrong network,
         //then the server should send a DHCPNAK message to the client
         dhcpServerSendReply(context, DHCP_MSG_TYPE_NAK, IPV4_UNSPECIFIED_ADDR,
            message, length);
      }
      else
      {
         //If the DHCP server has no record of this client, then it must remain
         //silent. This is necessary for peaceful coexistence of
         //non-communicating DHCP servers on the same wire (refer to RFC 2131,
         //section 4.3.2)
      }
   }
   else
   {
      //If the server is unable to satisfy the DHCPREQUEST message, the
      //server should respond with a DHCPNAK message
      dhcpServerSendReply(context, DHCP_MSG_TYPE_NAK, IPV4_UNSPECIFIED_ADDR,
         message, length);
   }
}


/**
 * @brief Parse DHCPDECLINE message
 * @param[in] context Pointer to the DHCP server context
 * @param[in] message Pointer to the incoming DHCP message
 * @param[in] length Length of the incoming message to parse
 **/

void dhcpServerParseDecline(DhcpServerContext *context,
   const DhcpMessage *message, size_t length)
{
   uint_t i;
   NetInterface *interface;
   DhcpOption *option;
   DhcpServerBinding *binding;
   Ipv4Addr requestedIpAddr;

   //Point to the underlying network interface
   interface = context->interface;
   //Index of the IP address assigned to the DHCP server
   i = context->ipAddrIndex;

   //Search the DHCP message for the Server Identifier option
   option = dhcpGetOption(message, length, DHCP_OPT_SERVER_ID);

   //Valid option found?
   if(option != NULL && option->length == 4)
   {
      //Unexpected server identifier?
      if(!ipv4CompAddr(option->value, &interface->ipv4Context.addrList[i].addr))
      {
         return;
      }
   }
   else
   {
      //The DHCPDECLINE message must include the Server Identifier option
      return;
   }

   //Search the DHCP message for the Requested IP Address option
   option = dhcpGetOption(message, length, DHCP_OPT_REQUESTED_IP_ADDR);

   //Valid option found?
   if(option != NULL && option->length == 4)
   {
      //Copy the requested IP address
      ipv4CopyAddr(&requestedIpAddr, option->value);

      //Search the list for a matching binding
      binding = dhcpServerFindBindingByMacAddr(context, &message->chaddr);

      //Matching binding found?
      if(binding != NULL)
      {
         //Check the IP address against the requested IP address
         if(binding->ipAddr == requestedIpAddr)
         {
            //The server must mark the network address as not available
            binding->state = DHCP_SERVER_BINDING_STATE_DECLINED;
            //Save the time at which the network address was declined
            binding->timestamp = osGetSystemTime();
         }
      }
   }
}


/**
 * @brief Parse DHCPRELEASE message
 * @param[in] context Pointer to the DHCP server context
 * @param[in] message Pointer to the incoming DHCP message
 * @param[in] length Length of the incoming message to parse
 **/

void dhcpServerParseRelease(DhcpServerContext *context,
   const DhcpMessage *message, size_t length)
{
   uint_t i;
   NetInterface *interface;
   DhcpOption *option;
   DhcpServerBinding *binding;

   //Point to the underlying network interface
   interface = context->interface;
   //Index of the IP address assigned to the DHCP server
   i = context->ipAddrIndex;

   //Search the DHCP message for the Server Identifier option
   option = dhcpGetOption(message, length, DHCP_OPT_SERVER_ID);

   //Valid option found?
   if(option != NULL && option->length == 4)
   {
      //Unexpected server identifier?
      if(!ipv4CompAddr(option->value, &interface->ipv4Context.addrList[i].addr))
      {
         return;
      }
   }
   else
   {
      //The DHCPRELEASE message must include the Server Identifier option
      return;
   }

   //Search the list for a matching binding
   binding = dhcpServerFindBindingByMacAddr(context, &message->chaddr);

   //Matching binding found?
   if(binding != NULL)
   {
      //Check the IP address against the client IP address
      if(binding->ipAddr == message->ciaddr)
      {
         //Release the network address and cancel remaining lease
         binding->state = DHCP_SERVER_BINDING_STATE_RESERVED;
      }
   }
}


/**
 * @brief Parse DHCPINFORM message
 * @param[in] context Pointer to the DHCP server context
 * @param[in] message Pointer to the incoming DHCP message
 * @param[in] length Length of the incoming message to parse
 **/

void dhcpServerParseInform(DhcpServerContext *context,
   const DhcpMessage *message, size_t length)
{
   //Make sure the client IP address is valid
   if(message->ciaddr != IPV4_UNSPECIFIED_ADDR)
   {
      //Servers receiving a DHCPINFORM message construct a DHCPACK message
      //with any local configuration parameters appropriate for the client
      dhcpServerSendReply(context, DHCP_MSG_TYPE_ACK, IPV4_UNSPECIFIED_ADDR,
         message, length);
   }
}


/**
 * @brief Send DHCP reply message
 * @param[in] context Pointer to the DHCP server context
 * @param[in] type DHCP message type (DHCPOFFER, DHCPACK or DHCPNAK)
 * @param[in] yourIpAddr The IP address to be placed in the 'yiaddr' field
 * @param[in] request Pointer to DHCP message received from the client
 * @param[in] requestLen Length of the DHCP message received from the client
 * @return Error code
 **/

error_t dhcpServerSendReply(DhcpServerContext *context, uint8_t type,
   Ipv4Addr yourIpAddr, const DhcpMessage *request, size_t requestLen)
{
   error_t error;
   uint_t i;
   uint_t n;
   uint32_t value;
   size_t offset;
   size_t replyLen;
   NetBuffer *buffer;
   NetInterface *interface;
   DhcpMessage *reply;
   DhcpOption *option;
   IpAddr srcIpAddr;
   IpAddr destIpAddr;
   uint16_t destPort;
   NetTxAncillary ancillary;

   //Point to the underlying network interface
   interface = context->interface;
   //Index of the IP address assigned to the DHCP server
   i = context->ipAddrIndex;

   //Allocate a memory buffer to hold the DHCP message
   buffer = udpAllocBuffer(DHCP_MAX_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(buffer == NULL)
   {
      return ERROR_OUT_OF_MEMORY;
   }

   //Point to the beginning of the DHCP message
   reply = netBufferAt(buffer, offset, 0);

   //Clear memory buffer contents
   osMemset(reply, 0, DHCP_MAX_MSG_SIZE);

   //Format DHCP reply message
   reply->op = DHCP_OPCODE_BOOTREPLY;
   reply->htype = DHCP_HARDWARE_TYPE_ETH;
   reply->hlen = sizeof(MacAddr);
   reply->xid = request->xid;
   reply->secs = 0;
   reply->flags = request->flags;
   reply->ciaddr = IPV4_UNSPECIFIED_ADDR;
   reply->yiaddr = yourIpAddr;
   reply->siaddr = IPV4_UNSPECIFIED_ADDR;
   reply->giaddr = request->giaddr;
   reply->chaddr = request->chaddr;

   //DHCPACK message?
   if(type == DHCP_MSG_TYPE_ACK)
   {
      //The 'ciaddr' field of a DHCPACK message is copied from the request
      //(refer to RFC 2131, section 4.3.1)
      reply->ciaddr = request->ciaddr;
   }

   //DHCPNAK message?
   if(type == DHCP_MSG_TYPE_NAK)
   {
      //If 'giaddr' is set in the DHCPREQUEST message, the client is on a
      //different subnet. The server must set the broadcast bit in the
      //DHCPNAK, so that the relay agent will broadcast the DHCPNAK to the
      //client (refer to RFC 2131, section 4.3.2)
      if(request->giaddr != IPV4_UNSPECIFIED_ADDR)
      {
         reply->flags |= HTONS(DHCP_FLAG_BROADCAST);
      }
   }

   //Write magic cookie before setting any option
   reply->magicCookie = HTONL(DHCP_MAGIC_COOKIE);
   //Properly terminate options field
   reply->options[0] = DHCP_OPT_END;

   //Total length of the DHCP message
   replyLen = sizeof(DhcpMessage) + sizeof(uint8_t);

   //Add DHCP Message Type option
   dhcpAddOption(reply, &replyLen, DHCP_OPT_DHCP_MESSAGE_TYPE,
      &type, sizeof(type));

   //Add Server Identifier option
   dhcpAddOption(reply, &replyLen, DHCP_OPT_SERVER_ID,
      &interface->ipv4Context.addrList[i].addr, sizeof(Ipv4Addr));

   //DHCPACK message?
   if(type == DHCP_MSG_TYPE_ACK)
   {
      //Retrieve the type of the request
      option = dhcpGetOption(request, requestLen, DHCP_OPT_DHCP_MESSAGE_TYPE);

      //DHCPACK message sent in response to a DHCPDISCOVER message?
      if(option != NULL && option->length == 1 &&
         option->value[0] == DHCP_MSG_TYPE_DISCOVER)
      {
         //A server must include a Rapid Commit option in a DHCPACK message sent
         //in response to a DHCPDISCOVER message (refer to RFC 4039, section 4)
         dhcpAddOption(reply, &replyLen, DHCP_OPT_RAPID_COMMIT, NULL, 0);
      }
   }

   //DHCPOFFER or DHCPACK message?
   if(type == DHCP_MSG_TYPE_OFFER || type == DHCP_MSG_TYPE_ACK)
   {
      //Convert the lease time to network byte order
      value = htonl(context->leaseTime);

      //When responding to a DHCPINFORM message, the server must not
      //send a lease expiration time to the client
      if(yourIpAddr != IPV4_UNSPECIFIED_ADDR)
      {
         //Add IP Address Lease Time option
         dhcpAddOption(reply, &replyLen, DHCP_OPT_IP_ADDRESS_LEASE_TIME,
            &value, sizeof(value));
      }

      //Add Subnet Mask option
      if(context->subnetMask != IPV4_UNSPECIFIED_ADDR)
      {
         dhcpAddOption(reply, &replyLen, DHCP_OPT_SUBNET_MASK,
            &context->subnetMask, sizeof(Ipv4Addr));
      }

      //Add Router option
      if(context->defaultGateway != IPV4_UNSPECIFIED_ADDR)
      {
         dhcpAddOption(reply, &replyLen, DHCP_OPT_ROUTER,
            &context->defaultGateway, sizeof(Ipv4Addr));
      }

      //Retrieve the number of DNS servers
      for(n = 0; n < DHCP_SERVER_MAX_DNS_SERVERS; n++)
      {
         //Check whether the current DNS server is valid
         if(context->dnsServer[n] == IPV4_UNSPECIFIED_ADDR)
         {
            break;
         }
      }

      //Add DNS Server option
      if(n > 0)
      {
         dhcpAddOption(reply, &replyLen, DHCP_OPT_DNS_SERVER,
            context->dnsServer, n * sizeof(Ipv4Addr));
      }
   }

   //Search the request for the Client Identifier option
   option = dhcpGetOption(request, requestLen, DHCP_OPT_CLIENT_ID);

   //If the 'client identifier' option is present in a message received from a
   //client, the server must return the 'client identifier' option, unaltered,
   //in its response message (refer to RFC 6842, section 3)
   if(option != NULL)
   {
      dhcpAddOption(reply, &replyLen, DHCP_OPT_CLIENT_ID, option->value,
         option->length);
   }

   //Any registered callback?
   if(context->addOptionsCallback != NULL)
   {
      //Invoke user callback function
      context->addOptionsCallback(context, reply, &replyLen,
         (DhcpMessageType) type);
   }

   //The minimum length of BOOTP frames is 300 octets (refer to RFC 951,
   //section 3)
   replyLen = MAX(replyLen, DHCP_MIN_MSG_SIZE);

   //Adjust the length of the multi-part buffer
   netBufferSetLength(buffer, offset + replyLen);

   //Additional options can be passed to the stack along with the packet
   ancillary = NET_DEFAULT_TX_ANCILLARY;

   //A server with multiple network addresses (e.g. a multi-homed host) may
   //use any of its network addresses in outgoing DHCP messages (refer to
   //RFC 2131, section 4.1)
   srcIpAddr.length = sizeof(Ipv4Addr);
   srcIpAddr.ipv4Addr = interface->ipv4Context.addrList[i].addr;

   //Check whether the 'giaddr' field is non-zero
   if(request->giaddr != IPV4_UNSPECIFIED_ADDR)
   {
      //If the 'giaddr' field in a DHCP message from a client is non-zero,
      //the server sends any return messages to the 'DHCP server' port
      destPort = DHCP_SERVER_PORT;

      //The DHCP message is sent to the BOOTP relay agent whose address
      //appears in 'giaddr'
      destIpAddr.length = sizeof(Ipv4Addr);
      destIpAddr.ipv4Addr = request->giaddr;
   }
   else
   {
      //If the 'giaddr' field in a DHCP message from a client is zero,
      //the server sends any return messages to the 'DHCP client'
      destPort = DHCP_CLIENT_PORT;

      //DHCPOFFER or DHCPACK message?
      if(type == DHCP_MSG_TYPE_OFFER || type == DHCP_MSG_TYPE_ACK)
      {
         //Check whether the 'ciaddr' field is non-zero
         if(request->ciaddr != IPV4_UNSPECIFIED_ADDR)
         {
            //If the 'giaddr' field is zero and the 'ciaddr' field is nonzero,
            //then the server unicasts DHCPOFFER and DHCPACK messages to the
            //address in 'ciaddr'
            destIpAddr.length = sizeof(Ipv4Addr);
            destIpAddr.ipv4Addr = request->ciaddr;
         }
         else
         {
            //Check whether the broadcast bit is set
            if(ntohs(request->flags) & DHCP_FLAG_BROADCAST)
            {
               //If 'giaddr' is zero and 'ciaddr' is zero, and the broadcast
               //bit is set, then the server broadcasts DHCPOFFER and DHCPACK
               //messages
               destIpAddr.length = sizeof(Ipv4Addr);
               destIpAddr.ipv4Addr = IPV4_BROADCAST_ADDR;
            }
            else
            {
               //If 'giaddr' is zero and 'ciaddr' is zero, and the broadcast
               //bit is not set, then the server unicasts DHCPOFFER and DHCPACK
               //messages to the client's hardware address and 'yiaddr' address
               ancillary.destMacAddr = request->chaddr;
               destIpAddr.length = sizeof(Ipv4Addr);
               destIpAddr.ipv4Addr = yourIpAddr;
            }
         }
      }
      //DHCPNAK message?
      else
      {
         //In all cases, when 'giaddr' is zero, the server broadcasts any
         //DHCPNAK messages
         destIpAddr.length = sizeof(Ipv4Addr);
         destIpAddr.ipv4Addr = IPV4_BROADCAST_ADDR;
      }
   }

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCP message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), replyLen);

   //Dump the contents of the message for debugging purpose
   dhcpDumpMessage(reply, replyLen);

   //Send DHCP reply
   error = udpSendBuffer(context->netContext, interface, &srcIpAddr,
      DHCP_SERVER_PORT, &destIpAddr, destPort, buffer, offset, &ancillary);

   //Free previously allocated memory
   netBufferFree(buffer);

   //Return status code
   return error;
}


/**
 * @brief Create a new binding
 * @param[in] context Pointer to the DHCP server context
 * @return Pointer to the newly created binding
 **/

DhcpServerBinding *dhcpServerCreateBinding(DhcpServerContext *context)
{
   uint_t i;
   systime_t time;
   DhcpServerBinding *binding;
   DhcpServerBinding *oldestBinding;

   //Get current time
   time = osGetSystemTime();

   //Keep track of the oldest binding
   oldestBinding = NULL;

   //Loop through the list of bindings
   for(i = 0; i < DHCP_SERVER_MAX_CLIENTS; i++)
   {
      //Point to the current binding
      binding = &context->clientBinding[i];

      //Check the state of the binding
      if(binding->state == DHCP_SERVER_BINDING_STATE_NONE)
      {
         //Clear the unused entry
         osMemset(binding, 0, sizeof(DhcpServerBinding));

         //Return a pointer to the newly created binding
         return binding;
      }
      else if(binding->state == DHCP_SERVER_BINDING_STATE_RESERVED)
      {
         //Keep track of the oldest binding in the list
         if(oldestBinding == NULL)
         {
            oldestBinding = binding;
         }
         else if((time - binding->timestamp) > (time - oldestBinding->timestamp))
         {
            oldestBinding = binding;
         }
      }
      else
      {
         //Bindings that have been committed or declined cannot be removed
      }
   }

   //Any reserved binding that can be recycled?
   if(oldestBinding != NULL)
   {
      //Discard the previous reservation so that the entry can be reused
      osMemset(oldestBinding, 0, sizeof(DhcpServerBinding));
   }

   //Return a pointer to the oldest binding
   return oldestBinding;
}


/**
 * @brief Search the list of bindings for a given MAC address
 * @param[in] context Pointer to the DHCP server context
 * @param[in] macAddr MAC address
 * @return Pointer to the corresponding DHCP binding
 **/

DhcpServerBinding *dhcpServerFindBindingByMacAddr(DhcpServerContext *context,
   const MacAddr *macAddr)
{
   uint_t i;
   DhcpServerBinding *binding;

   //Loop through the list of bindings
   for(i = 0; i < DHCP_SERVER_MAX_CLIENTS; i++)
   {
      //Point to the current binding
      binding = &context->clientBinding[i];

      //Binding associated with a client?
      if(binding->state == DHCP_SERVER_BINDING_STATE_RESERVED ||
         binding->state == DHCP_SERVER_BINDING_STATE_COMMITTED)
      {
         //Check whether the current binding matches the specified MAC address
         if(macCompAddr(&binding->macAddr, macAddr))
         {
            //Return the pointer to the corresponding binding
            return binding;
         }
      }
   }

   //No matching binding
   return NULL;
}


/**
 * @brief Search the list of bindings for a given IP address
 * @param[in] context Pointer to the DHCP server context
 * @param[in] ipAddr IP address
 * @return Pointer to the corresponding DHCP binding
 **/

DhcpServerBinding *dhcpServerFindBindingByIpAddr(DhcpServerContext *context,
   Ipv4Addr ipAddr)
{
   uint_t i;
   DhcpServerBinding *binding;

   //Loop through the list of bindings
   for(i = 0; i < DHCP_SERVER_MAX_CLIENTS; i++)
   {
      //Point to the current binding
      binding = &context->clientBinding[i];

      //Binding in use?
      if(binding->state != DHCP_SERVER_BINDING_STATE_NONE)
      {
         //Check whether the current binding matches the specified IP address
         if(binding->ipAddr == ipAddr)
         {
            //Return the pointer to the corresponding binding
            return binding;
         }
      }
   }

   //No matching binding
   return NULL;
}


/**
 * @brief Retrieve the next available IP address from the pool
 * @param[in] context Pointer to the DHCP server context
 * @param[out] ipAddr Next available IP address
 * @return Error code
 **/

error_t dhcpServerGetNextIpAddr(DhcpServerContext *context, Ipv4Addr *ipAddr)
{
   error_t error;
   uint_t i;
   DhcpServerBinding *binding;

   //Initialize status code
   error = ERROR_NO_ADDRESS;

   //Search the pool for any available IP address
   for(i = 0; i < (DHCP_SERVER_MAX_CLIENTS + IPV4_ADDR_LIST_SIZE); i++)
   {
      //Check whether the current IP address is already in use
      binding = dhcpServerFindBindingByIpAddr(context, context->nextIpAddr);

      //IP address available for use?
      if(binding == NULL)
      {
         //Check whether the IP address can be assigned to a new client
         if(dhcpServerIsIpAddrInPool(context, context->nextIpAddr))
         {
            //Return the next available IP address
            *ipAddr = context->nextIpAddr;

            //We are done
            error = NO_ERROR;
            break;
         }
      }

      //Compute the next IP address that will be assigned by the DHCP server
      if(ntohl(context->nextIpAddr) >= ntohl(context->ipAddrRangeMax))
      {
         //Wrap around to the beginning of the pool
         context->nextIpAddr = context->ipAddrRangeMin;
      }
      else
      {
         //Increment IP address
         context->nextIpAddr = htonl(ntohl(context->nextIpAddr) + 1);
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Check whether an IP address belongs to the pool of addresses
 * @param[in] context Pointer to the DHCP server context
 * @param[in] ipAddr IP address to be checked
 * @return TRUE if the IP address is in the pool, else FALSE
 **/

bool_t dhcpServerIsIpAddrInPool(DhcpServerContext *context, Ipv4Addr ipAddr)
{
   bool_t res;
   uint_t i;
   NetInterface *interface;
   Ipv4AddrEntry *entry;

   //Point to the underlying network interface
   interface = context->interface;

   //Check whether the IP address is within the range of addresses managed by
   //the server
   if(ntohl(ipAddr) >= ntohl(context->ipAddrRangeMin) &&
      ntohl(ipAddr) <= ntohl(context->ipAddrRangeMax))
   {
      res = TRUE;
   }
   else
   {
      res = FALSE;
   }

   //Loop through the list of IPv4 addresses assigned to the interface
   for(i = 0; i < IPV4_ADDR_LIST_SIZE && res; i++)
   {
      //Point to the current entry
      entry = &interface->ipv4Context.addrList[i];

      //Valid entry?
      if(entry->state != IPV4_ADDR_STATE_INVALID)
      {
         //An address assigned to the host cannot be assigned to a client
         if(entry->addr == ipAddr)
         {
            res = FALSE;
         }
      }
   }

   //Return TRUE if the IP address can be assigned to a client
   return res;
}

#endif
