/**
 * @file nic.c
 * @brief Network interface controller abstraction layer
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
 * @version 1.8.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/nic.h"
#include "core/socket.h"
#include "core/raw_socket.h"
#include "core/tcp_misc.h"
#include "core/udp.h"
#include "ipv4/ipv4.h"
#include "ipv6/ipv6.h"
#include "dns/dns_cache.h"
#include "dns/dns_client.h"
#include "mdns/mdns_client.h"
#include "mdns/mdns_responder.h"
#include "dns_sd/dns_sd.h"
#include "mibs/mib2_module.h"
#include "mibs/if_mib_module.h"
#include "debug.h"

//Tick counter to handle periodic operations
systime_t nicTickCounter;


/**
 * @brief Network controller timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void nicTick(NetInterface *interface)
{
   //Disable interrupts
   interface->nicDriver->disableIrq(interface);

   //Handle periodic operations
   interface->nicDriver->tick(interface);

   //Re-enable interrupts if necessary
   if(interface->configured)
      interface->nicDriver->enableIrq(interface);
}


/**
 * @brief Send a packet to the network controller
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t nicSendPacket(NetInterface *interface, const NetBuffer *buffer, size_t offset)
{
   error_t error;
   bool_t status;

#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   //Retrieve the length of the packet
   size_t length = netBufferGetLength(buffer) - offset;

   //Debug message
   TRACE_DEBUG("Sending packet (%" PRIuSIZE " bytes)...\r\n", length);
   TRACE_DEBUG_NET_BUFFER("  ", buffer, offset, length);
#endif

   //Wait for the transmitter to be ready to send
   status = osWaitForEvent(&interface->nicTxEvent, NIC_MAX_BLOCKING_TIME);

   //Check whether the specified event is in signaled state
   if(status)
   {
      //Disable interrupts
      interface->nicDriver->disableIrq(interface);

      //Send Ethernet frame
      error = interface->nicDriver->sendPacket(interface, buffer, offset);

      //Re-enable interrupts if necessary
      if(interface->configured)
         interface->nicDriver->enableIrq(interface);
   }
   else
   {
      //The transmitter is busy...
      return ERROR_TRANSMITTER_BUSY;
   }

   //Return status code
   return error;
}


/**
 * @brief Configure multicast MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t nicSetMulticastFilter(NetInterface *interface)
{
   error_t error;

   //Disable interrupts
   interface->nicDriver->disableIrq(interface);

   //Update MAC filter table
   error = interface->nicDriver->setMulticastFilter(interface);

   //Re-enable interrupts if necessary
   if(interface->configured)
      interface->nicDriver->enableIrq(interface);

   //Return status code
   return error;
}


/**
 * @brief Handle a packet received by the network controller
 * @param[in] interface Underlying network interface
 * @param[in] packet Incoming packet to process
 * @param[in] length Total packet length
 **/

void nicProcessPacket(NetInterface *interface, void *packet, size_t length)
{
   NicType type;

   //Re-enable interrupts if necessary
   if(interface->configured)
      interface->nicDriver->enableIrq(interface);

   //Debug message
   TRACE_DEBUG("Packet received (%" PRIuSIZE " bytes)...\r\n", length);
   TRACE_DEBUG_ARRAY("  ", packet, length);

   //Retrieve network interface type
   type = interface->nicDriver->type;

   //Ethernet interface?
   if(type == NIC_TYPE_ETHERNET)
   {
#if (ETH_SUPPORT == ENABLED)
      //Process incoming Ethernet frame
      ethProcessFrame(interface, packet, length);
#endif
   }
   //PPP interface?
   else if(type == NIC_TYPE_PPP)
   {
#if (PPP_SUPPORT == ENABLED)
      //Process incoming PPP frame
      pppProcessFrame(interface, packet, length);
#endif
   }
   //6LoWPAN interface?
   else if(type == NIC_TYPE_6LOWPAN)
   {
#if (IPV6_SUPPORT == ENABLED)
      NetBuffer1 buffer;

      //The incoming packet fits in a single chunk
      buffer.chunkCount = 1;
      buffer.maxChunkCount = 1;
      buffer.chunk[0].address = packet;
      buffer.chunk[0].length = (uint16_t) length;
      buffer.chunk[0].size = 0;

      //Process incoming IPv6 packet
      ipv6ProcessPacket(interface, (NetBuffer *) &buffer, 0);
#endif
   }

   //Disable interrupts
   interface->nicDriver->disableIrq(interface);
}


/**
 * @brief Process link state change event
 * @param[in] interface Underlying network interface
 **/

void nicNotifyLinkChange(NetInterface *interface)
{
   uint_t i;
   Socket *socket;

   //Re-enable interrupts if necessary
   if(interface->configured)
      interface->nicDriver->enableIrq(interface);

   //Check link state
   if(interface->linkState)
   {
      //Display link state
      TRACE_INFO("Link is up (%s)...\r\n", interface->name);

      //Display link speed
      if(interface->linkSpeed == NIC_LINK_SPEED_1GBPS)
      {
         //1000BASE-T
         TRACE_INFO("  Link speed = 1000 Mbps\r\n");
      }
      else if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
      {
         //100BASE-TX
         TRACE_INFO("  Link speed = 100 Mbps\r\n");
      }
      else if(interface->linkSpeed == NIC_LINK_SPEED_10MBPS)
      {
         //10BASE-T
         TRACE_INFO("  Link speed = 10 Mbps\r\n");
      }
      else if(interface->linkSpeed != NIC_LINK_SPEED_UNKNOWN)
      {
         //10BASE-T
         TRACE_INFO("  Link speed = %" PRIu32 " bps\r\n", interface->linkSpeed);
      }

      //Display duplex mode
      if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
      {
         //1000BASE-T
         TRACE_INFO("  Duplex mode = Full-Duplex\r\n");
      }
      else if(interface->duplexMode == NIC_HALF_DUPLEX_MODE)
      {
         //100BASE-TX
         TRACE_INFO("  Duplex mode = Half-Duplex\r\n");
      }
   }
   else
   {
      //Display link state
      TRACE_INFO("Link is down (%s)...\r\n", interface->name);
   }

   //The time at which the interface entered its current operational state
   MIB2_SET_TIME_TICKS(ifGroup.ifTable[interface->index].ifLastChange, osGetSystemTime() / 10);
   IF_MIB_SET_TIME_TICKS(ifTable[interface->index].ifLastChange, osGetSystemTime() / 10);

#if (IPV4_SUPPORT == ENABLED)
   //Notify IPv4 of link state changes
   ipv4LinkChangeEvent(interface);
#endif

#if (IPV6_SUPPORT == ENABLED)
   //Notify IPv6 of link state changes
   ipv6LinkChangeEvent(interface);
#endif

#if (DNS_CLIENT_SUPPORT == ENABLED || MDNS_CLIENT_SUPPORT == ENABLED || \
   NBNS_CLIENT_SUPPORT == ENABLED)
   //Flush DNS cache
   dnsFlushCache(interface);
#endif

#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   //Perform probing and announcing
   mdnsResponderLinkChangeEvent(interface->mdnsResponderContext);
#endif

#if (DNS_SD_SUPPORT == ENABLED)
   //Perform probing and announcing
   dnsSdLinkChangeEvent(interface->dnsSdContext);
#endif

   //Notify registered users of link state changes
   netInvokeLinkChangeCallback(interface, interface->linkState);

   //Loop through opened sockets
   for(i = 0; i < SOCKET_MAX_COUNT; i++)
   {
      //Point to the current socket
      socket = socketTable + i;

#if (TCP_SUPPORT == ENABLED)
      //Connection-oriented socket?
      if(socket->type == SOCKET_TYPE_STREAM)
      {
         tcpUpdateEvents(socket);
      }
#endif
#if (UDP_SUPPORT == ENABLED)
      //Connectionless socket?
      if(socket->type == SOCKET_TYPE_DGRAM)
      {
         udpUpdateEvents(socket);
      }
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
      //Raw socket?
      if(socket->type == SOCKET_TYPE_RAW_IP ||
         socket->type == SOCKET_TYPE_RAW_ETH)
      {
         rawSocketUpdateEvents(socket);
      }
#endif
   }

   //Disable interrupts
   interface->nicDriver->disableIrq(interface);
}
