/**
 * @file nic.c
 * @brief Network interface controller abstraction layer
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.4
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
#include "ipv4/ipv4_misc.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_misc.h"
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
 * @brief Retrieve logical interface
 * @param[in] interface Pointer to the network interface
 * @return Pointer to the physical interface
 **/

NetInterface *nicGetLogicalInterface(NetInterface *interface)
{
#if (ETH_VLAN_SUPPORT == ENABLED)
   uint_t i;

   //A virtual interface can inherit from multiple parent interfaces
   for(i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Check whether a valid MAC address has been assigned to the interface
      if(!macCompAddr(&interface->macAddr, &MAC_UNSPECIFIED_ADDR))
         break;

      //Last interface in the list?
      if(interface->parent == NULL)
         break;

      //Point to the interface on top of which the virtual interface runs
      interface = interface->parent;
   }
#endif

   //Return a pointer to the logical interface
   return interface;
}


/**
 * @brief Retrieve physical interface
 * @param[in] interface Pointer to the network interface
 * @return Pointer to the physical interface
 **/

NetInterface *nicGetPhysicalInterface(NetInterface *interface)
{
#if (ETH_VIRTUAL_IF_SUPPORT == ENABLED || ETH_VLAN_SUPPORT == ENABLED || \
   ETH_PORT_TAGGING_SUPPORT == ENABLED)
   uint_t i;

   //A virtual interface can inherit from multiple parent interfaces
   for(i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Physical interface?
      if(interface->nicDriver != NULL || interface->parent == NULL)
         break;

      //Point to the interface on top of which the virtual interface runs
      interface = interface->parent;
   }
#endif

   //Return a pointer to the physical interface
   return interface;
}


/**
 * @brief Retrieve switch port identifier
 * @param[in] interface Pointer to the network interface
 * @return Switch port identifier
 **/

uint8_t nicGetSwitchPort(NetInterface *interface)
{
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   uint_t i;

   //A virtual interface can inherit from multiple parent interfaces
   for(i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Valid switch port identifier?
      if(interface->port != 0 || interface->parent == NULL)
         break;

      //Point to the interface on top of which the virtual interface runs
      interface = interface->parent;
   }

   //Return switch port identifier
   return interface->port;
#else
   //Ethernet port multiplication (VLAN or tail tagging) is not supported
   return 0;
#endif
}


/**
 * @brief Retrieve VLAN identifier
 * @param[in] interface Pointer to the network interface
 * @return VLAN identifier
 **/

uint16_t nicGetVlanId(NetInterface *interface)
{
#if (ETH_VLAN_SUPPORT == ENABLED)
   uint_t i;

   //A virtual interface can inherit from multiple parent interfaces
   for(i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Valid VLAN identifier?
      if(interface->vlanId != 0 || interface->parent == NULL)
         break;

      //Point to the interface on top of which the virtual interface runs
      interface = interface->parent;
   }

   //Return VLAN identifier
   return interface->vlanId;
#else
   //VLAN is not supported
   return 0;
#endif
}


/**
 * @brief Retrieve VMAN identifier
 * @param[in] interface Pointer to the network interface
 * @return VMAN identifier
 **/

uint16_t nicGetVmanId(NetInterface *interface)
{
#if (ETH_VMAN_SUPPORT == ENABLED)
   uint_t i;

   //A virtual interface can inherit from multiple parent interfaces
   for(i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Valid VMAN identifier?
      if(interface->vmanId != 0 || interface->parent == NULL)
         break;

      //Point to the interface on top of which the virtual interface runs
      interface = interface->parent;
   }

   //Return VMAN identifier
   return interface->vmanId;
#else
   //VMAN is not supported
   return 0;
#endif
}


/**
 * @brief Test parent/child relationship between 2 interfaces
 * @param[in] interface Pointer to the child interface
 * @param[in] parent Pointer to the parent interface
 * @return TRUE is an existing parent/child relationship is found, else FALSE
 **/

bool_t nicIsParentInterface(NetInterface *interface, NetInterface *parent)
{
#if (ETH_VIRTUAL_IF_SUPPORT == ENABLED || ETH_VLAN_SUPPORT == ENABLED || \
   ETH_PORT_TAGGING_SUPPORT == ENABLED)
   uint_t i;
   bool_t flag;

   //Iterate through the parent interfaces
   for(flag = FALSE, i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Any parent/child relationship?
      if(interface == parent)
      {
         flag = TRUE;
         break;
      }

      //Last interface in the list?
      if(interface->parent == NULL)
         break;

      //Point to the interface on top of which the virtual interface runs
      interface = interface->parent;
   }

   //Return TRUE is an existing parent/child relationship is found
   return flag;
#else
   //Virtual interfaces are not supported
   return (interface == parent) ? TRUE : FALSE;
#endif
}


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
   //Valid NIC driver?
   if(interface->nicDriver != NULL)
   {
      //Disable interrupts
      interface->nicDriver->disableIrq(interface);

      //Handle periodic operations
      interface->nicDriver->tick(interface);

      //Re-enable interrupts if necessary
      if(interface->configured)
      {
         interface->nicDriver->enableIrq(interface);
      }
   }
}


/**
 * @brief Send a packet to the network controller
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t nicSendPacket(NetInterface *interface, const NetBuffer *buffer,
   size_t offset)
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

   //Valid NIC driver?
   if(interface->nicDriver != NULL)
   {
      //Loopback interface?
      if(interface->nicDriver->type == NIC_TYPE_LOOPBACK)
      {
         //The loopback interface is always available
         status = TRUE;
      }
      else
      {
         //Wait for the transmitter to be ready to send
         status = osWaitForEvent(&interface->nicTxEvent, NIC_MAX_BLOCKING_TIME);
      }

      //Check whether the specified event is in signaled state
      if(status)
      {
         //Disable interrupts
         interface->nicDriver->disableIrq(interface);

         //Send Ethernet frame
         error = interface->nicDriver->sendPacket(interface, buffer, offset);

         //Re-enable interrupts if necessary
         if(interface->configured)
         {
            interface->nicDriver->enableIrq(interface);
         }
      }
      else
      {
         //The transmitter is busy
         error = ERROR_TRANSMITTER_BUSY;
      }
   }
   else
   {
      //Report an error
      error = ERROR_INVALID_INTERFACE;
   }

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t nicUpdateMacAddrFilter(NetInterface *interface)
{
   error_t error;

   //Valid NIC driver?
   if(interface->nicDriver != NULL)
   {
      //Disable interrupts
      interface->nicDriver->disableIrq(interface);

      //Update MAC filter table
      error = interface->nicDriver->updateMacAddrFilter(interface);

      //Re-enable interrupts if necessary
      if(interface->configured)
      {
         interface->nicDriver->enableIrq(interface);
      }
   }
   else
   {
      //Report an error
      error = ERROR_INVALID_INTERFACE;
   }

   //Return status code
   return error;
}


/**
 * @brief Handle a packet received by the network controller
 * @param[in] interface Underlying network interface
 * @param[in] packet Incoming packet to process
 * @param[in] length Total packet length
 **/

void nicProcessPacket(NetInterface *interface, uint8_t *packet, size_t length)
{
   NicType type;

   //Re-enable interrupts if necessary
   if(interface->configured)
   {
      interface->nicDriver->enableIrq(interface);
   }

   //Debug message
   TRACE_DEBUG("Packet received (%" PRIuSIZE " bytes)...\r\n", length);
   TRACE_DEBUG_ARRAY("  ", packet, length);

   //Retrieve network interface type
   type = interface->nicDriver->type;

#if (ETH_SUPPORT == ENABLED)
   //Ethernet interface?
   if(type == NIC_TYPE_ETHERNET)
   {
      //Process incoming Ethernet frame
      ethProcessFrame(interface, packet, length);
   }
   else
#endif
#if (PPP_SUPPORT == ENABLED)
   //PPP interface?
   if(type == NIC_TYPE_PPP)
   {
      //Process incoming PPP frame
      pppProcessFrame(interface, packet, length);
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //6LoWPAN interface?
   if(type == NIC_TYPE_6LOWPAN)
   {
      NetBuffer1 buffer;

      //The incoming packet fits in a single chunk
      buffer.chunkCount = 1;
      buffer.maxChunkCount = 1;
      buffer.chunk[0].address = packet;
      buffer.chunk[0].length = (uint16_t) length;
      buffer.chunk[0].size = 0;

      //Process incoming IPv6 packet
      ipv6ProcessPacket(interface, (NetBuffer *) &buffer, 0);
   }
   else
#endif
#if (NET_LOOPBACK_IF_SUPPORT == ENABLED)
   //Loopback interface?
   if(type == NIC_TYPE_LOOPBACK)
   {
#if (IPV4_SUPPORT == ENABLED)
      //IPv4 packet received?
      if(length >= sizeof(Ipv4Header) && (packet[0] >> 4) == 4)
      {
         error_t error;
         uint_t i;
         Ipv4Header *header;

         //Point to the IPv4 header
         header = (Ipv4Header *) packet;

         //Loop through network interfaces
         for(i = 0; i < NET_INTERFACE_COUNT; i++)
         {
            //Check destination address
            error = ipv4CheckDestAddr(&netInterface[i], header->destAddr);

            //Valid destination address?
            if(!error)
            {
               //Process incoming IPv4 packet
               ipv4ProcessPacket(&netInterface[i], (Ipv4Header *) packet,
                  length);
            }
         }
      }
      else
#endif
#if (IPV6_SUPPORT == ENABLED)
      //IPv6 packet received?
      if(length >= sizeof(Ipv6Header) && (packet[0] >> 4) == 6)
      {
         error_t error;
         uint_t i;
         NetBuffer1 buffer;
         Ipv6Header *header;

         //Point to the IPv6 header
         header = (Ipv6Header *) packet;

         //Loop through network interfaces
         for(i = 0; i < NET_INTERFACE_COUNT; i++)
         {
            //Check destination address
            error = ipv6CheckDestAddr(&netInterface[i], &header->destAddr);

            //Valid destination address?
            if(!error)
            {
               //The incoming packet fits in a single chunk
               buffer.chunkCount = 1;
               buffer.maxChunkCount = 1;
               buffer.chunk[0].address = packet;
               buffer.chunk[0].length = (uint16_t) length;
               buffer.chunk[0].size = 0;

               //Process incoming IPv6 packet
               ipv6ProcessPacket(&netInterface[i], (NetBuffer *) &buffer, 0);
            }
         }
      }
      else
#endif
      {
         //Invalid version number
      }
   }
   else
#endif
   //Unknown interface type?
   {
      //Silently discard the received packet
   }

   //Disable interrupts
   interface->nicDriver->disableIrq(interface);
}


/**
 * @brief Process link state change notification
 * @param[in] interface Underlying network interface
 **/

void nicNotifyLinkChange(NetInterface *interface)
{
   uint_t i;
   Socket *socket;
   NetInterface *virtualInterface;
   NetInterface *physicalInterface;

   //Point to the physical interface
   physicalInterface = nicGetPhysicalInterface(interface);

   //Re-enable interrupts if necessary
   if(physicalInterface->configured)
   {
      physicalInterface->nicDriver->enableIrq(physicalInterface);
   }

   //Loop through network interfaces
   for(i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Point to the current interface
      virtualInterface = &netInterface[i];

      //Check whether the current virtual interface is attached to the physical
      //interface
      if(nicIsParentInterface(virtualInterface, interface) &&
         nicGetSwitchPort(virtualInterface) == nicGetSwitchPort(interface))
      {
         //Set operation mode
         virtualInterface->linkSpeed = interface->linkSpeed;
         virtualInterface->duplexMode = interface->duplexMode;

         //Update link state
         virtualInterface->linkState = interface->linkState;

         //Check link state
         if(virtualInterface->linkState)
         {
            //Display link state
            TRACE_INFO("Link is up (%s)...\r\n", virtualInterface->name);

            //Display link speed
            if(virtualInterface->linkSpeed == NIC_LINK_SPEED_1GBPS)
            {
               //1000BASE-T
               TRACE_INFO("  Link speed = 1000 Mbps\r\n");
            }
            else if(virtualInterface->linkSpeed == NIC_LINK_SPEED_100MBPS)
            {
               //100BASE-TX
               TRACE_INFO("  Link speed = 100 Mbps\r\n");
            }
            else if(virtualInterface->linkSpeed == NIC_LINK_SPEED_10MBPS)
            {
               //10BASE-T
               TRACE_INFO("  Link speed = 10 Mbps\r\n");
            }
            else if(virtualInterface->linkSpeed != NIC_LINK_SPEED_UNKNOWN)
            {
               //10BASE-T
               TRACE_INFO("  Link speed = %" PRIu32 " bps\r\n",
                  virtualInterface->linkSpeed);
            }

            //Display duplex mode
            if(virtualInterface->duplexMode == NIC_FULL_DUPLEX_MODE)
            {
               //1000BASE-T
               TRACE_INFO("  Duplex mode = Full-Duplex\r\n");
            }
            else if(virtualInterface->duplexMode == NIC_HALF_DUPLEX_MODE)
            {
               //100BASE-TX
               TRACE_INFO("  Duplex mode = Half-Duplex\r\n");
            }
         }
         else
         {
            //Display link state
            TRACE_INFO("Link is down (%s)...\r\n", virtualInterface->name);
         }

         //The time at which the interface entered its current operational state
         MIB2_SET_TIME_TICKS(ifGroup.ifTable[virtualInterface->index].ifLastChange,
            osGetSystemTime() / 10);
         IF_MIB_SET_TIME_TICKS(ifTable[virtualInterface->index].ifLastChange,
            osGetSystemTime() / 10);

#if (IPV4_SUPPORT == ENABLED)
         //Notify IPv4 of link state changes
         ipv4LinkChangeEvent(virtualInterface);
#endif

#if (IPV6_SUPPORT == ENABLED)
         //Notify IPv6 of link state changes
         ipv6LinkChangeEvent(virtualInterface);
#endif

#if (DNS_CLIENT_SUPPORT == ENABLED || MDNS_CLIENT_SUPPORT == ENABLED || \
         NBNS_CLIENT_SUPPORT == ENABLED)
         //Flush DNS cache
         dnsFlushCache(virtualInterface);
#endif

#if (MDNS_RESPONDER_SUPPORT == ENABLED)
         //Perform probing and announcing
         mdnsResponderLinkChangeEvent(virtualInterface->mdnsResponderContext);
#endif

#if (DNS_SD_SUPPORT == ENABLED)
         //Perform probing and announcing
         dnsSdLinkChangeEvent(virtualInterface->dnsSdContext);
#endif
         //Notify registered users of link state changes
         netInvokeLinkChangeCallback(virtualInterface,
            virtualInterface->linkState);
      }
   }

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
   physicalInterface->nicDriver->disableIrq(physicalInterface);
}
