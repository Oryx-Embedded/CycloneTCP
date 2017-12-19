/**
 * @file ipv4.c
 * @brief IPv4 (Internet Protocol Version 4)
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
 * @section Description
 *
 * The Internet Protocol (IP) provides the functions necessary to deliver a
 * datagram from a source to a destination over an interconnected system of
 * networks. Refer to RFC 791 for complete details
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.8.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL IPV4_TRACE_LEVEL

//Dependencies
#include <string.h>
#include <ctype.h>
#include "core/net.h"
#include "core/ethernet.h"
#include "core/ip.h"
#include "core/udp.h"
#include "core/tcp_fsm.h"
#include "core/raw_socket.h"
#include "ipv4/arp.h"
#include "ipv4/ipv4.h"
#include "ipv4/ipv4_routing.h"
#include "ipv4/icmp.h"
#include "ipv4/igmp.h"
#include "ipv4/auto_ip.h"
#include "dhcp/dhcp_client.h"
#include "mdns/mdns_responder.h"
#include "mibs/mib2_module.h"
#include "mibs/ip_mib_module.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED)


/**
 * @brief IPv4 related initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ipv4Init(NetInterface *interface)
{
   Ipv4Context *context;

   //Point to the IPv4 context
   context = &interface->ipv4Context;

   //Clear the IPv4 context
   memset(context, 0, sizeof(Ipv4Context));

   //Initialize interface specific variables
   context->linkMtu = interface->nicDriver->mtu;
   context->isRouter = FALSE;

   //Identification field is primarily used to identify
   //fragments of an original IP datagram
   context->identification = 0;

   //Initialize the list of DNS servers
   memset(context->dnsServerList, 0, sizeof(context->dnsServerList));
   //Initialize the multicast filter table
   memset(context->multicastFilter, 0, sizeof(context->multicastFilter));

#if (IPV4_FRAG_SUPPORT == ENABLED)
   //Initialize the reassembly queue
   memset(context->fragQueue, 0, sizeof(context->fragQueue));
#endif

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Assign host address
 * @param[in] interface Pointer to the desired network interface
 * @param[in] addr IPv4 host address
 * @return Error code
 **/

error_t ipv4SetHostAddr(NetInterface *interface, Ipv4Addr addr)
{
   //Check parameters
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //The IPv4 address must be a valid unicast address
   if(ipv4IsMulticastAddr(addr))
      return ERROR_INVALID_ADDRESS;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Set up host address
   interface->ipv4Context.addr = addr;
   //Clear conflict flag
   interface->ipv4Context.addrConflict = FALSE;

   //Check whether the new host address is valid
   if(addr != IPV4_UNSPECIFIED_ADDR)
   {
      //The use of the IPv4 address is now unrestricted
      interface->ipv4Context.addrState = IPV4_ADDR_STATE_VALID;
   }
   else
   {
      //The IPv4 address is no longer valid
      interface->ipv4Context.addrState = IPV4_ADDR_STATE_INVALID;
   }

#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   //Restart mDNS probing process
   mdnsResponderStartProbing(interface->mdnsResponderContext);
#endif

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve host address
 * @param[in] interface Pointer to the desired network interface
 * @param[out] addr IPv4 host address
 * @return Error code
 **/

error_t ipv4GetHostAddr(NetInterface *interface, Ipv4Addr *addr)
{
   //Check parameters
   if(interface == NULL || addr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Check whether the host address is valid
   if(interface->ipv4Context.addrState == IPV4_ADDR_STATE_VALID)
   {
      //Get IPv4 address
      *addr = interface->ipv4Context.addr;
   }
   else
   {
      //Return the unspecified address when no address has been assigned
      *addr = IPV4_UNSPECIFIED_ADDR;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Configure subnet mask
 * @param[in] interface Pointer to the desired network interface
 * @param[in] mask Subnet mask
 * @return Error code
 **/

error_t ipv4SetSubnetMask(NetInterface *interface, Ipv4Addr mask)
{
   //Check parameters
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set up subnet mask
   interface->ipv4Context.subnetMask = mask;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve subnet mask
 * @param[in] interface Pointer to the desired network interface
 * @param[out] mask Subnet mask
 * @return Error code
 **/

error_t ipv4GetSubnetMask(NetInterface *interface, Ipv4Addr *mask)
{
   //Check parameters
   if(interface == NULL || mask == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Get subnet mask
   *mask = interface->ipv4Context.subnetMask;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Configure default gateway
 * @param[in] interface Pointer to the desired network interface
 * @param[in] addr Default gateway address
 * @return Error code
 **/

error_t ipv4SetDefaultGateway(NetInterface *interface, Ipv4Addr addr)
{
   //Check parameters
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //The IPv4 address must be a valid unicast address
   if(ipv4IsMulticastAddr(addr))
      return ERROR_INVALID_ADDRESS;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set up default gateway address
   interface->ipv4Context.defaultGateway = addr;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve default gateway
 * @param[in] interface Pointer to the desired network interface
 * @param[out] addr Default gateway address
 * @return Error code
 **/

error_t ipv4GetDefaultGateway(NetInterface *interface, Ipv4Addr *addr)
{
   //Check parameters
   if(interface == NULL || addr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Get default gateway address
   *addr = interface->ipv4Context.defaultGateway;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Configure DNS server
 * @param[in] interface Pointer to the desired network interface
 * @param[in] index This parameter selects between the primary and secondary DNS server
 * @param[in] addr DNS server address
 * @return Error code
 **/

error_t ipv4SetDnsServer(NetInterface *interface, uint_t index, Ipv4Addr addr)
{
   //Check parameters
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Make sure that the index is valid
   if(index >= IPV4_DNS_SERVER_LIST_SIZE)
      return ERROR_OUT_OF_RANGE;

   //The IPv4 address must be a valid unicast address
   if(ipv4IsMulticastAddr(addr))
      return ERROR_INVALID_ADDRESS;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set up DNS server address
   interface->ipv4Context.dnsServerList[index] = addr;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve DNS server
 * @param[in] interface Pointer to the desired network interface
 * @param[in] index This parameter selects between the primary and secondary DNS server
 * @param[out] addr DNS server address
 * @return Error code
 **/

error_t ipv4GetDnsServer(NetInterface *interface, uint_t index, Ipv4Addr *addr)
{
   //Check parameters
   if(interface == NULL || addr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Make sure that the index is valid
   if(index >= IPV4_DNS_SERVER_LIST_SIZE)
   {
      //Return the unspecified address when the index is out of range
      *addr = IPV4_UNSPECIFIED_ADDR;
      //Report an error
      return ERROR_OUT_OF_RANGE;
   }

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Get DNS server address
   *addr = interface->ipv4Context.dnsServerList[index];
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Get IPv4 broadcast address
 * @param[in] interface Pointer to the desired network interface
 * @param[out] addr IPv4 broadcast address
 **/

error_t ipv4GetBroadcastAddr(NetInterface *interface, Ipv4Addr *addr)
{
   //Check parameters
   if(interface == NULL || addr == NULL)
      return ERROR_INVALID_PARAMETER;

   //The broadcast address is obtained by performing a bitwise OR operation
   //between the bit complement of the subnet mask and the host IP address
   *addr = interface->ipv4Context.addr;
   *addr |= ~interface->ipv4Context.subnetMask;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Callback function for link change event
 * @param[in] interface Underlying network interface
 **/

void ipv4LinkChangeEvent(NetInterface *interface)
{
   Ipv4Context *context;

   //Point to the IPv4 context
   context = &interface->ipv4Context;

   //Restore default MTU
   context->linkMtu = interface->nicDriver->mtu;

#if (ETH_SUPPORT == ENABLED)
   //Flush ARP cache contents
   arpFlushCache(interface);
#endif

#if (IPV4_FRAG_SUPPORT == ENABLED)
   //Flush the reassembly queue
   ipv4FlushFragQueue(interface);
#endif

#if (IGMP_SUPPORT == ENABLED)
   //Notify IGMP of link state changes
   igmpLinkChangeEvent(interface);
#endif

#if (AUTO_IP_SUPPORT == ENABLED)
   //Notify Auto-IP of link state changes
   autoIpLinkChangeEvent(interface->autoIpContext);
#endif

#if (DHCP_CLIENT_SUPPORT == ENABLED)
   //Notify the DHCP client of link state changes
   dhcpClientLinkChangeEvent(interface->dhcpClientContext);
#endif
}


/**
 * @brief Incoming IPv4 packet processing
 * @param[in] interface Underlying network interface
 * @param[in] packet Incoming IPv4 packet
 * @param[in] length Packet length including header and payload
 **/

void ipv4ProcessPacket(NetInterface *interface, Ipv4Header *packet, size_t length)
{
   //Total number of input datagrams received, including those received in error
   MIB2_INC_COUNTER32(ipGroup.ipInReceives, 1);
   IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInReceives, 1);
   IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCInReceives, 1);
   IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInReceives, 1);
   IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCInReceives, 1);

   //Total number of octets received in input IP datagrams
   IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInOctets, length);
   IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCInOctets, length);
   IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInOctets, length);
   IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCInOctets, length);

   //Ensure the packet length is greater than 20 bytes
   if(length < sizeof(Ipv4Header))
   {
      //Number of input IP datagrams discarded because the datagram frame
      //didn't carry enough data
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInTruncatedPkts, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInTruncatedPkts, 1);

      //Discard the received packet
      return;
   }

   //Debug message
   TRACE_INFO("IPv4 packet received (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump IP header contents for debugging purpose
   ipv4DumpHeader(packet);

   //A packet whose version number is not 4 must be silently discarded
   if(packet->version != IPV4_VERSION)
   {
      //Number of input datagrams discarded due to errors in their IP headers
      MIB2_INC_COUNTER32(ipGroup.ipInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInHdrErrors, 1);

      //Discard the received packet
      return;
   }

   //Valid IPv4 header shall contains more than five 32-bit words
   if(packet->headerLength < 5)
   {
      //Number of input datagrams discarded due to errors in their IP headers
      MIB2_INC_COUNTER32(ipGroup.ipInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInHdrErrors, 1);

      //Discard the received packet
      return;
   }

   //Ensure the total length is correct before processing the packet
   if(ntohs(packet->totalLength) < (packet->headerLength * 4))
   {
      //Number of input datagrams discarded due to errors in their IP headers
      MIB2_INC_COUNTER32(ipGroup.ipInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInHdrErrors, 1);

      //Discard the received packet
      return;
   }

   //Truncated packet?
   if(length < ntohs(packet->totalLength))
   {
      //Number of input IP datagrams discarded because the datagram frame
      //didn't carry enough data
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInTruncatedPkts, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInTruncatedPkts, 1);

      //Discard the received packet
      return;
   }

   //Source address filtering
   if(ipv4CheckSourceAddr(interface, packet->srcAddr))
   {
      //Number of input datagrams discarded due to errors in their IP headers
      MIB2_INC_COUNTER32(ipGroup.ipInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInHdrErrors, 1);

      //Discard the received packet
      return;
   }

#if defined(IPV4_PACKET_FORWARD_HOOK)
   IPV4_PACKET_FORWARD_HOOK(interface, packet, length);
#else
   //Destination address filtering
   if(ipv4CheckDestAddr(interface, packet->destAddr))
   {
#if (IPV4_ROUTING_SUPPORT == ENABLED)
      NetBuffer1 buffer;

      //Unfragmented datagrams fit in a single chunk
      buffer.chunkCount = 1;
      buffer.maxChunkCount = 1;
      buffer.chunk[0].address = packet;
      buffer.chunk[0].length = length;

      //Forward the packet according to the routing table
      ipv4ForwardPacket(interface, (NetBuffer *) &buffer, 0);
#else
      //Number of input datagrams discarded because the destination IP address
      //was not a valid address
      MIB2_INC_COUNTER32(ipGroup.ipInAddrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInAddrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInAddrErrors, 1);
#endif
      //We are done
      return;
   }
#endif

   //Packets addressed to a tentative address should be silently discarded
   if(ipv4IsTentativeAddr(interface, packet->destAddr))
   {
      //Number of input datagrams discarded because the destination IP address
      //was not a valid address
      MIB2_INC_COUNTER32(ipGroup.ipInAddrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInAddrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInAddrErrors, 1);

      //Discard the received packet
      return;
   }

   //The host must verify the IP header checksum on every received
   //datagram and silently discard every datagram that has a bad
   //checksum (see RFC 1122 3.2.1.2)
   if(ipCalcChecksum(packet, packet->headerLength * 4) != 0x0000)
   {
      //Debug message
      TRACE_WARNING("Wrong IP header checksum!\r\n");

      //Number of input datagrams discarded due to errors in their IP headers
      MIB2_INC_COUNTER32(ipGroup.ipInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInHdrErrors, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInHdrErrors, 1);

      //Discard incoming packet
      return;
   }

   //Update IP statistics
   ipv4UpdateInStats(interface, packet->destAddr, length);

   //Convert the total length from network byte order
   length = ntohs(packet->totalLength);

   //A fragmented packet was received?
   if(ntohs(packet->fragmentOffset) & (IPV4_FLAG_MF | IPV4_OFFSET_MASK))
   {
#if (IPV4_FRAG_SUPPORT == ENABLED)
      //Reassemble the original datagram
      ipv4ReassembleDatagram(interface, packet, length);
#endif
   }
   else
   {
      NetBuffer1 buffer;

      //Unfragmented datagrams fit in a single chunk
      buffer.chunkCount = 1;
      buffer.maxChunkCount = 1;
      buffer.chunk[0].address = packet;
      buffer.chunk[0].length = (uint16_t) length;

      //Pass the IPv4 datagram to the higher protocol layer
      ipv4ProcessDatagram(interface, (NetBuffer *) &buffer);
   }
}


/**
 * @brief Incoming IPv4 datagram processing
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer that holds the incoming IPv4 datagram
 **/

void ipv4ProcessDatagram(NetInterface *interface, const NetBuffer *buffer)
{
   error_t error;
   size_t offset;
   size_t length;
   Ipv4Header *header;
   IpPseudoHeader pseudoHeader;

   //Retrieve the length of the IPv4 datagram
   length = netBufferGetLength(buffer);

   //Point to the IPv4 header
   header = netBufferAt(buffer, 0);
   //Sanity check
   if(header == NULL)
      return;

   //Debug message
   TRACE_INFO("IPv4 datagram received (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump IP header contents for debugging purpose
   ipv4DumpHeader(header);

   //Get the offset to the payload
   offset = header->headerLength * 4;
   //Compute the length of the payload
   length -= header->headerLength * 4;

   //Form the IPv4 pseudo header
   pseudoHeader.length = sizeof(Ipv4PseudoHeader);
   pseudoHeader.ipv4Data.srcAddr = header->srcAddr;
   pseudoHeader.ipv4Data.destAddr = header->destAddr;
   pseudoHeader.ipv4Data.reserved = 0;
   pseudoHeader.ipv4Data.protocol = header->protocol;
   pseudoHeader.ipv4Data.length = htons(length);

#if defined(IPV4_DATAGRAM_FORWARD_HOOK)
   IPV4_DATAGRAM_FORWARD_HOOK(interface, &pseudoHeader, buffer, offset);
#endif

   //Check the protocol field
   switch(header->protocol)
   {
   //ICMP protocol?
   case IPV4_PROTOCOL_ICMP:
      //Process incoming ICMP message
      icmpProcessMessage(interface, header->srcAddr, buffer, offset);
#if (RAW_SOCKET_SUPPORT == ENABLED)
      //Allow raw sockets to process ICMP messages
      rawSocketProcessIpPacket(interface, &pseudoHeader, buffer, offset);
#endif
      //No error to report
      error = NO_ERROR;
      //Continue processing
      break;

#if (IGMP_SUPPORT == ENABLED)
   //IGMP protocol?
   case IPV4_PROTOCOL_IGMP:
      //Process incoming IGMP message
      igmpProcessMessage(interface, buffer, offset);
#if (RAW_SOCKET_SUPPORT == ENABLED)
      //Allow raw sockets to process IGMP messages
      rawSocketProcessIpPacket(interface, &pseudoHeader, buffer, offset);
#endif
      //No error to report
      error = NO_ERROR;
      //Continue processing
      break;
#endif

#if (TCP_SUPPORT == ENABLED)
   //TCP protocol?
   case IPV4_PROTOCOL_TCP:
      //Process incoming TCP segment
      tcpProcessSegment(interface, &pseudoHeader, buffer, offset);
      //No error to report
      error = NO_ERROR;
      //Continue processing
      break;
#endif

#if (UDP_SUPPORT == ENABLED)
   //UDP protocol?
   case IPV4_PROTOCOL_UDP:
      //Process incoming UDP datagram
      error = udpProcessDatagram(interface, &pseudoHeader, buffer, offset);
      //Continue processing
      break;
#endif

   //Unknown protocol?
   default:
#if (RAW_SOCKET_SUPPORT == ENABLED)
      //Allow raw sockets to process IPv4 packets
      error = rawSocketProcessIpPacket(interface, &pseudoHeader, buffer, offset);
#else
      //Report an error
      error = ERROR_PROTOCOL_UNREACHABLE;
#endif
      //Continue processing
      break;
   }

   //Unreachable protocol?
   if(error == ERROR_PROTOCOL_UNREACHABLE)
   {
      //Number of locally-addressed datagrams received successfully but discarded
      //because of an unknown or unsupported protocol
      MIB2_INC_COUNTER32(ipGroup.ipInUnknownProtos, 1);
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInUnknownProtos, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInUnknownProtos, 1);

      //Send a Destination Unreachable message
      icmpSendErrorMessage(interface, ICMP_TYPE_DEST_UNREACHABLE,
         ICMP_CODE_PROTOCOL_UNREACHABLE, 0, buffer, 0);
   }
   else
   {
      //Total number of input datagrams successfully delivered to IP
      //user-protocols
      MIB2_INC_COUNTER32(ipGroup.ipInDelivers, 1);
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInDelivers, 1);
      IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCInDelivers, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInDelivers, 1);
      IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCInDelivers, 1);
   }

   //Unreachable port?
   if(error == ERROR_PORT_UNREACHABLE)
   {
      //Send a Destination Unreachable message
      icmpSendErrorMessage(interface, ICMP_TYPE_DEST_UNREACHABLE,
         ICMP_CODE_PORT_UNREACHABLE, 0, buffer, 0);
   }
}


/**
 * @brief Send an IPv4 datagram
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader IPv4 pseudo header
 * @param[in] buffer Multi-part buffer containing the payload
 * @param[in] offset Offset to the first byte of the payload
 * @param[in] ttl TTL value. Default Time-To-Live is used when this parameter is zero
 * @return Error code
 **/

error_t ipv4SendDatagram(NetInterface *interface, Ipv4PseudoHeader *pseudoHeader,
   NetBuffer *buffer, size_t offset, uint8_t ttl)
{
   error_t error;
   size_t length;
   uint16_t id;

   //Total number of IP datagrams which local IP user-protocols supplied to IP
   //in requests for transmission
   MIB2_INC_COUNTER32(ipGroup.ipOutRequests, 1);
   IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsOutRequests, 1);
   IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCOutRequests, 1);
   IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsOutRequests, 1);
   IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCOutRequests, 1);

   //Retrieve the length of payload
   length = netBufferGetLength(buffer) - offset;

   //Check whether the TTL value is zero
   if(ttl == 0)
   {
      //Use default Time-To-Live value
      ttl = IPV4_DEFAULT_TTL;
   }

   //Identification field is primarily used to identify
   //fragments of an original IP datagram
   id = interface->ipv4Context.identification++;

   //If the payload length is smaller than the network
   //interface MTU then no fragmentation is needed
   if((length + sizeof(Ipv4Header)) <= interface->ipv4Context.linkMtu)
   {
      //Send data as is
      error = ipv4SendPacket(interface,
         pseudoHeader, id, 0, buffer, offset, ttl);
   }
   //If the payload length exceeds the network interface MTU
   //then the device must fragment the data
   else
   {
#if (IPV4_FRAG_SUPPORT == ENABLED)
      //Fragment IP datagram into smaller packets
      error = ipv4FragmentDatagram(interface,
         pseudoHeader, id, buffer, offset, ttl);
#else
      //Fragmentation is not supported
      error = ERROR_MESSAGE_TOO_LONG;
#endif
   }

   //Return status code
   return error;
}


/**
 * @brief Send an IPv4 packet
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader IPv4 pseudo header
 * @param[in] fragId Fragment identification field
 * @param[in] fragOffset Fragment offset field
 * @param[in] buffer Multi-part buffer containing the payload
 * @param[in] offset Offset to the first byte of the payload
 * @param[in] ttl Time-To-Live value
 * @return Error code
 **/

error_t ipv4SendPacket(NetInterface *interface, Ipv4PseudoHeader *pseudoHeader,
   uint16_t fragId, size_t fragOffset, NetBuffer *buffer, size_t offset, uint8_t ttl)
{
   error_t error;
   size_t length;
   Ipv4Header *packet;

   //Is there enough space for the IPv4 header?
   if(offset < sizeof(Ipv4Header))
      return ERROR_INVALID_PARAMETER;

   //Make room for the header
   offset -= sizeof(Ipv4Header);
   //Calculate the size of the entire packet, including header and data
   length = netBufferGetLength(buffer) - offset;

   //Point to the IPv4 header
   packet = netBufferAt(buffer, offset);

   //Format IPv4 header
   packet->version = IPV4_VERSION;
   packet->headerLength = 5;
   packet->typeOfService = 0;
   packet->totalLength = htons(length);
   packet->identification = htons(fragId);
   packet->fragmentOffset = htons(fragOffset);
   packet->timeToLive = ttl;
   packet->protocol = pseudoHeader->protocol;
   packet->headerChecksum = 0;
   packet->srcAddr = pseudoHeader->srcAddr;
   packet->destAddr = pseudoHeader->destAddr;

   //Calculate IP header checksum
   packet->headerChecksum = ipCalcChecksumEx(buffer, offset, packet->headerLength * 4);

   //Ensure the source address is valid
   error = ipv4CheckSourceAddr(interface, pseudoHeader->srcAddr);
   //Invalid source address?
   if(error)
      return error;

   //Destination address is the unspecified address?
   if(pseudoHeader->destAddr == IPV4_UNSPECIFIED_ADDR)
   {
      //Destination address is not acceptable
      return ERROR_INVALID_ADDRESS;
   }
   //Destination address is the loopback address?
   else if(pseudoHeader->destAddr == IPV4_LOOPBACK_ADDR)
   {
      //Check source address
      if(pseudoHeader->srcAddr != IPV4_LOOPBACK_ADDR)
      {
         //Destination address is not acceptable
         return ERROR_INVALID_ADDRESS;
      }
   }

#if (ETH_SUPPORT == ENABLED)
   //Ethernet interface?
   if(interface->nicDriver->type == NIC_TYPE_ETHERNET)
   {
      Ipv4Addr destIpAddr;
      MacAddr destMacAddr;

      //Get the destination IPv4 address
      destIpAddr = pseudoHeader->destAddr;

      //Destination address is a broadcast address?
      if(ipv4IsBroadcastAddr(interface, destIpAddr))
      {
         //Use of the broadcast MAC address to send the packet
         destMacAddr = MAC_BROADCAST_ADDR;
         //No error to report
         error = NO_ERROR;
      }
      //Destination address is a multicast address?
      else if(ipv4IsMulticastAddr(destIpAddr))
      {
         //Map IPv4 multicast address to MAC-layer multicast address
         error = ipv4MapMulticastAddrToMac(destIpAddr, &destMacAddr);
      }
      //Source or destination address is a link-local address?
      else if(ipv4IsLinkLocalAddr(pseudoHeader->srcAddr) ||
         ipv4IsLinkLocalAddr(destIpAddr))
      {
         //Packets with a link-local source or destination address are not
         //routable off the link
         error = arpResolve(interface, destIpAddr, &destMacAddr);
      }
      //Destination host is on the local subnet?
      else if(ipv4IsOnLocalSubnet(interface, destIpAddr))
      {
         //Resolve destination address before sending the packet
         error = arpResolve(interface, destIpAddr, &destMacAddr);
      }
      //Destination host is outside the local subnet?
      else
      {
         //Make sure the default gateway is properly set
         if(interface->ipv4Context.defaultGateway != IPV4_UNSPECIFIED_ADDR)
         {
            //Use the default gateway to forward the packet
            destIpAddr = interface->ipv4Context.defaultGateway;
            //Perform address resolution
            error = arpResolve(interface, destIpAddr, &destMacAddr);
         }
         else
         {
            //Number of IP datagrams discarded because no route could be found
            //to transmit them to their destination
            MIB2_INC_COUNTER32(ipGroup.ipOutNoRoutes, 1);
            IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsOutNoRoutes, 1);

            //Report an error
            error = ERROR_NO_ROUTE;
         }
      }

      //Successful address resolution?
      if(!error)
      {
         //Update IP statistics
         ipv4UpdateOutStats(interface, destIpAddr, length);

         //Debug message
         TRACE_INFO("Sending IPv4 packet (%" PRIuSIZE " bytes)...\r\n", length);
         //Dump IP header contents for debugging purpose
         ipv4DumpHeader(packet);

         //Send Ethernet frame
         error = ethSendFrame(interface, &destMacAddr, buffer, offset, ETH_TYPE_IPV4);
      }
      //Address resolution is in progress?
      else if(error == ERROR_IN_PROGRESS)
      {
         //Debug message
         TRACE_INFO("Enqueuing IPv4 packet (%" PRIuSIZE " bytes)...\r\n", length);
         //Dump IP header contents for debugging purpose
         ipv4DumpHeader(packet);

         //Enqueue packets waiting for address resolution
         error = arpEnqueuePacket(interface, destIpAddr, buffer, offset);
      }
      //Address resolution failed?
      else
      {
         //Debug message
         TRACE_WARNING("Cannot map IPv4 address to Ethernet address!\r\n");
      }
   }
   else
#endif
#if (PPP_SUPPORT == ENABLED)
   //PPP interface?
   if(interface->nicDriver->type == NIC_TYPE_PPP)
   {
      //Update IP statistics
      ipv4UpdateOutStats(interface, pseudoHeader->destAddr, length);

      //Debug message
      TRACE_INFO("Sending IPv4 packet (%" PRIuSIZE " bytes)...\r\n", length);
      //Dump IP header contents for debugging purpose
      ipv4DumpHeader(packet);

      //Send PPP frame
      error = pppSendFrame(interface, buffer, offset, PPP_PROTOCOL_IP);
   }
   else
#endif
   //Unknown interface type?
   {
      //Report an error
      error = ERROR_INVALID_INTERFACE;
   }

   //Return status code
   return error;
}


/**
 * @brief Source IPv4 address filtering
 * @param[in] interface Underlying network interface
 * @param[in] ipAddr Source IPv4 address to be checked
 * @return Error code
 **/

error_t ipv4CheckSourceAddr(NetInterface *interface, Ipv4Addr ipAddr)
{
   //Broadcast and multicast addresses must not be used as source
   //address (see RFC 1122 3.2.1.3)
   if(ipv4IsBroadcastAddr(interface, ipAddr) || ipv4IsMulticastAddr(ipAddr))
   {
      //Debug message
      TRACE_WARNING("Wrong source IPv4 address!\r\n");
      //The source address not is acceptable
      return ERROR_INVALID_ADDRESS;
   }

   //The source address is acceptable
   return NO_ERROR;
}


/**
 * @brief Destination IPv4 address filtering
 * @param[in] interface Underlying network interface
 * @param[in] ipAddr Destination IPv4 address to be checked
 * @return Error code
 **/

error_t ipv4CheckDestAddr(NetInterface *interface, Ipv4Addr ipAddr)
{
   error_t error;
   uint_t i;
   Ipv4FilterEntry *entry;

   //Filter out any invalid addresses
   error = ERROR_INVALID_ADDRESS;

   //Broadcast address?
   if(ipv4IsBroadcastAddr(interface, ipAddr))
   {
      //Always accept broadcast address
      error = NO_ERROR;
   }
   //Multicast address?
   else if(ipv4IsMulticastAddr(ipAddr))
   {
      //Go through the multicast filter table
      for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv4Context.multicastFilter[i];

         //Valid entry?
         if(entry->refCount > 0)
         {
            //Check whether the destination IPv4 address matches
            //a relevant multicast address
            if(entry->addr == ipAddr)
            {
               //The multicast address is acceptable
               error = NO_ERROR;
               //Stop immediately
               break;
            }
         }
      }
   }
   //Unicast address?
   else
   {
      //Valid host address?
      if(interface->ipv4Context.addrState != IPV4_ADDR_STATE_INVALID)
      {
         //Check whether the destination address matches the host address
         if(interface->ipv4Context.addr == ipAddr)
         {
            //The destination address is acceptable
            error = NO_ERROR;
         }
      }
   }

   //Return status code
   return error;
}


/**
 * @brief IPv4 source address selection
 *
 * This function selects the source address and the relevant network interface
 * to be used in order to join the specified destination address
 *
 * @param[in,out] interface A pointer to a valid network interface may be provided as
 *   a hint. The function returns a pointer identifying the interface to be used
 * @param[in] destAddr Destination IPv4 address
 * @param[out] srcAddr Local IPv4 address to be used
 * @return Error code
 **/

error_t ipv4SelectSourceAddr(NetInterface **interface,
   Ipv4Addr destAddr, Ipv4Addr *srcAddr)
{
   uint_t i;
   NetInterface *currentInterface;
   NetInterface *bestInterface;

   //Initialize variables
   bestInterface = NULL;

   //Loop through network interfaces
   for(i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Point to the current interface
      currentInterface = &netInterface[i];

      //A network interface may be provided as a hint...
      if(*interface != currentInterface && *interface != NULL)
      {
         //Select the next interface in the list
         continue;
      }

      //Check the state of the address
      if(currentInterface->ipv4Context.addrState != IPV4_ADDR_STATE_VALID)
      {
         //Select the next interface in the list
         continue;
      }

      //Select the first interface as default
      if(bestInterface == NULL)
      {
         //Give the current interface the higher precedence
         bestInterface = currentInterface;
         //Select the next interface in the list
         continue;
      }

      //Prefer same address
      if(bestInterface->ipv4Context.addr == destAddr)
      {
         //Select the next interface in the list
         continue;
      }
      else if(currentInterface->ipv4Context.addr == destAddr)
      {
         //Give the current interface the higher precedence
         bestInterface = currentInterface;
         //Select the next interface in the list
         continue;
      }

      //Check whether the destination address matches the default gateway
      if(bestInterface->ipv4Context.defaultGateway == destAddr)
      {
         //Select the next interface in the list
         continue;
      }
      else if(currentInterface->ipv4Context.defaultGateway == destAddr)
      {
         //Give the current interface the higher precedence
         bestInterface = currentInterface;
         //Select the next interface in the list
         continue;
      }

      //Prefer appropriate scope
      if(ipv4GetAddrScope(currentInterface->ipv4Context.addr) <
         ipv4GetAddrScope(bestInterface->ipv4Context.addr))
      {
         if(ipv4GetAddrScope(currentInterface->ipv4Context.addr) >=
            ipv4GetAddrScope(destAddr))
         {
            //Give the current interface the higher precedence
            bestInterface = currentInterface;
         }

         //Select the next interface in the list
         continue;
      }
      else if(ipv4GetAddrScope(bestInterface->ipv4Context.addr) <
         ipv4GetAddrScope(currentInterface->ipv4Context.addr))
      {
         if(ipv4GetAddrScope(bestInterface->ipv4Context.addr) <
            ipv4GetAddrScope(destAddr))
         {
            //Give the current interface the higher precedence
            bestInterface = currentInterface;
         }

         //Select the next interface in the list
         continue;
      }

      //Prefer appropriate subnet mask
      if(ipv4IsOnLocalSubnet(bestInterface, destAddr))
      {
         //Select the next interface in the list
         continue;
      }
      else if(ipv4IsOnLocalSubnet(currentInterface, destAddr))
      {
         //Give the current interface the higher precedence
         bestInterface = currentInterface;
         //Select the next interface in the list
         continue;
      }

      //Use longest subnet mask
      if(ntohl(currentInterface->ipv4Context.subnetMask) >
         ntohl(bestInterface->ipv4Context.subnetMask))
      {
         //Give the current interface the higher precedence
         bestInterface = currentInterface;
      }
   }

   //Source address selection failed?
   if(bestInterface == NULL)
   {
      //Report an error
      return ERROR_NO_ADDRESS;
   }

   //Return the out-going interface and the source address to be used
   *interface = bestInterface;
   *srcAddr = bestInterface->ipv4Context.addr;

   //Successful source address selection
   return NO_ERROR;
}


/**
 * @brief Check whether an IPv4 address is a broadcast address
 * @param[in] interface Underlying network interface
 * @param[in] ipAddr IPv4 address to be checked
 * @return TRUE if the IPv4 address is a broadcast address, else FALSE
 **/

bool_t ipv4IsBroadcastAddr(NetInterface *interface, Ipv4Addr ipAddr)
{
   //Check whether the specified IPv4 address is the broadcast address
   if(ipAddr == IPV4_BROADCAST_ADDR)
      return TRUE;

   //Check whether the specified IPv4 address belongs to the local network
   if(ipv4IsOnLocalSubnet(interface, ipAddr))
   {
      //Make sure the subnet mask is not 255.255.255.255
      if(interface->ipv4Context.subnetMask != IPV4_BROADCAST_ADDR)
      {
         //Directed broadcast address?
         if((ipAddr | interface->ipv4Context.subnetMask) == IPV4_BROADCAST_ADDR)
            return TRUE;
      }
   }

   //The specified IPv4 address is not a broadcast address
   return FALSE;
}


/**
 * @brief Retrieve the scope of an IPv4 address
 * @param[in] ipAddr IPv4 address
 * @return IPv4 address scope
 **/

uint_t ipv4GetAddrScope(Ipv4Addr ipAddr)
{
   uint_t scope;

   //Broadcast address?
   if(ipAddr == IPV4_BROADCAST_ADDR)
   {
      //The broadcast address is never forwarded by the routers connecting
      //the local network to other networks
      scope = IPV4_ADDR_SCOPE_LINK_LOCAL;
   }
   //Multicast address?
   else if(ipv4IsMulticastAddr(ipAddr))
   {
      //Local Network Control Block?
      if((ipAddr & IPV4_MULTICAST_LNCB_MASK) == IPV4_MULTICAST_LNCB_PREFIX)
      {
         //Addresses in the Local Network Control Block are used for protocol
         //control traffic that is not forwarded off link
         scope = IPV4_ADDR_SCOPE_LINK_LOCAL;
      }
      //Any other multicast address?
      else
      {
         //Other addresses are assigned global scope
         scope = IPV4_ADDR_SCOPE_GLOBAL;
      }
   }
   //Unicast address?
   else
   {
      //Loopback address?
      if((ipAddr & IPV4_LOOPBACK_ADDR_MASK) == IPV4_LOOPBACK_ADDR_PREFIX)
      {
         //IPv4 loopback addresses, which have the prefix 127.0.0.0/8,
         //are assigned interface-local scope
         scope = IPV4_ADDR_SCOPE_INTERFACE_LOCAL;
      }
      //Link-local address?
      else if((ipAddr & IPV4_LINK_LOCAL_MASK) == IPV4_LINK_LOCAL_PREFIX)
      {
         //IPv4 auto-configuration addresses, which have the prefix
         //169.254.0.0/16, are assigned link-local scope
         scope = IPV4_ADDR_SCOPE_LINK_LOCAL;
      }
      //Any other unicast address?
      else
      {
         //Other addresses are assigned global scope
         scope = IPV4_ADDR_SCOPE_GLOBAL;
      }
   }

   //Return the scope of the specified IPv4 address
   return scope;
}


/**
 * @brief Calculate prefix length for a given subnet mask
 * @param[in] mask Subnet mask
 * @return Prefix length
 **/

uint_t ipv4GetPrefixLength(Ipv4Addr mask)
{
   uint_t i;

   //Convert from network byte order to host byte order
   mask = ntohl(mask);

   //Count of the number of leading 1 bits in the network mask
   for(i = 0; i < 32; i++)
   {
      //Check the value of the current bit
      if(!(mask & (1 << (31 - i))))
         break;
   }

   //Return prefix length
   return i;
}


/**
 * @brief Join the specified host group
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv4 address identifying the host group to join
 * @return Error code
 **/

error_t ipv4JoinMulticastGroup(NetInterface *interface, Ipv4Addr groupAddr)
{
   error_t error;
   uint_t i;
   Ipv4FilterEntry *entry;
   Ipv4FilterEntry *firstFreeEntry;
#if (ETH_SUPPORT == ENABLED)
   MacAddr macAddr;
#endif

   //The IPv4 address must be a valid multicast address
   if(!ipv4IsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Initialize error code
   error = NO_ERROR;
   //Keep track of the first free entry
   firstFreeEntry = NULL;

   //Go through the multicast filter table
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->ipv4Context.multicastFilter[i];

      //Valid entry?
      if(entry->refCount > 0)
      {
         //Check whether the table already contains the specified IPv4 address
         if(entry->addr == groupAddr)
         {
            //Increment the reference count
            entry->refCount++;
            //Successful processing
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

#if (ETH_SUPPORT == ENABLED)
   //Map the IPv4 multicast address to a MAC-layer address
   ipv4MapMulticastAddrToMac(groupAddr, &macAddr);
   //Add the corresponding address to the MAC filter table
   error = ethAcceptMulticastAddr(interface, &macAddr);
#endif

   //MAC filter table successfully updated?
   if(!error)
   {
      //Now we can safely add a new entry to the table
      firstFreeEntry->addr = groupAddr;
      //Initialize the reference count
      firstFreeEntry->refCount = 1;

#if (IGMP_SUPPORT == ENABLED)
      //Report multicast group membership to the router
      igmpJoinGroup(interface, firstFreeEntry);
#endif
   }

   //Return status code
   return error;
}


/**
 * @brief Leave the specified host group
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv4 address identifying the host group to leave
 * @return Error code
 **/

error_t ipv4LeaveMulticastGroup(NetInterface *interface, Ipv4Addr groupAddr)
{
   uint_t i;
   Ipv4FilterEntry *entry;
#if (ETH_SUPPORT == ENABLED)
   MacAddr macAddr;
#endif

   //The IPv4 address must be a valid multicast address
   if(!ipv4IsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Go through the multicast filter table
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->ipv4Context.multicastFilter[i];

      //Valid entry?
      if(entry->refCount > 0)
      {
         //Specified IPv4 address found?
         if(entry->addr == groupAddr)
         {
            //Decrement the reference count
            entry->refCount--;

            //Remove the entry if the reference count drops to zero
            if(entry->refCount == 0)
            {
#if (IGMP_SUPPORT == ENABLED)
               //Report group membership termination
               igmpLeaveGroup(interface, entry);
#endif
#if (ETH_SUPPORT == ENABLED)
               //Map the IPv4 multicast address to a MAC-layer address
               ipv4MapMulticastAddrToMac(groupAddr, &macAddr);
               //Drop the corresponding address from the MAC filter table
               ethDropMulticastAddr(interface, &macAddr);
#endif
               //Remove the multicast address from the list
               entry->addr = IPV4_UNSPECIFIED_ADDR;
            }

            //Successful processing
            return NO_ERROR;
         }
      }
   }

   //The specified IPv4 address does not exist
   return ERROR_ADDRESS_NOT_FOUND;
}


/**
 * @brief Map an host group address to a MAC-layer multicast address
 * @param[in] ipAddr IPv4 host group address
 * @param[out] macAddr Corresponding MAC-layer multicast address
 * @return Error code
 **/

error_t ipv4MapMulticastAddrToMac(Ipv4Addr ipAddr, MacAddr *macAddr)
{
   uint8_t *p;

   //Ensure the specified IPv4 address is a valid host group address
   if(!ipv4IsMulticastAddr(ipAddr))
      return ERROR_INVALID_ADDRESS;

   //Cast the address to byte array
   p = (uint8_t *) &ipAddr;

   //An IP host group address is mapped to an Ethernet multicast address
   //by placing the low-order 23-bits of the IP address into the low-order
   //23 bits of the Ethernet multicast address 01-00-5E-00-00-00
   macAddr->b[0] = 0x01;
   macAddr->b[1] = 0x00;
   macAddr->b[2] = 0x5E;
   macAddr->b[3] = p[1] & 0x7F;
   macAddr->b[4] = p[2];
   macAddr->b[5] = p[3];

   //The specified host group address was successfully
   //mapped to a MAC-layer address
   return NO_ERROR;
}


/**
 * @brief Update IPv4 input statistics
 * @param[in] interface Underlying network interface
 * @param[in] destIpAddr Destination IP address
 * @param[in] length Length of the incoming IP packet
 **/

void ipv4UpdateInStats(NetInterface *interface, Ipv4Addr destIpAddr, size_t length)
{
   //Check whether the destination address is a unicast, broadcast or multicast address
   if(ipv4IsBroadcastAddr(interface, destIpAddr))
   {
      //Number of IP broadcast datagrams transmitted
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInBcastPkts, 1);
      IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCInBcastPkts, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInBcastPkts, 1);
      IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCInBcastPkts, 1);
   }
   else if(ipv4IsMulticastAddr(destIpAddr))
   {
      //Number of IP multicast datagrams transmitted
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInMcastPkts, 1);
      IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCInMcastPkts, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInMcastPkts, 1);
      IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCInMcastPkts, 1);

      //Total number of octets transmitted in IP multicast datagrams
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsInMcastOctets, length);
      IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCInMcastOctets, length);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsInMcastOctets, length);
      IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCInMcastOctets, length);
   }
}


/**
 * @brief Update IPv4 output statistics
 * @param[in] interface Underlying network interface
 * @param[in] destIpAddr Destination IP address
 * @param[in] length Length of the outgoing IP packet
 **/

void ipv4UpdateOutStats(NetInterface *interface, Ipv4Addr destIpAddr, size_t length)
{
   //Check whether the destination address is a unicast, broadcast or multicast address
   if(ipv4IsBroadcastAddr(interface, destIpAddr))
   {
      //Number of IP broadcast datagrams transmitted
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsOutBcastPkts, 1);
      IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCOutBcastPkts, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsOutBcastPkts, 1);
      IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCOutBcastPkts, 1);
   }
   else if(ipv4IsMulticastAddr(destIpAddr))
   {
      //Number of IP multicast datagrams transmitted
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsOutMcastPkts, 1);
      IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCOutMcastPkts, 1);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsOutMcastPkts, 1);
      IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCOutMcastPkts, 1);

      //Total number of octets transmitted in IP multicast datagrams
      IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsOutMcastOctets, length);
      IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCOutMcastOctets, length);
      IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsOutMcastOctets, length);
      IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCOutMcastOctets, length);
   }

   //Total number of IP datagrams that this entity supplied to the lower
   //layers for transmission
   IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsOutTransmits, 1);
   IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCOutTransmits, 1);
   IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsOutTransmits, 1);
   IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCOutTransmits, 1);

   //Total number of octets in IP datagrams delivered to the lower layers
   //for transmission
   IP_MIB_INC_COUNTER32(ipv4SystemStats.ipSystemStatsOutOctets, length);
   IP_MIB_INC_COUNTER64(ipv4SystemStats.ipSystemStatsHCOutOctets, length);
   IP_MIB_INC_COUNTER32(ipv4IfStatsTable[interface->index].ipIfStatsOutOctets, length);
   IP_MIB_INC_COUNTER64(ipv4IfStatsTable[interface->index].ipIfStatsHCOutOctets, length);
}


/**
 * @brief Convert a dot-decimal string to a binary IPv4 address
 * @param[in] str NULL-terminated string representing the IPv4 address
 * @param[out] ipAddr Binary representation of the IPv4 address
 * @return Error code
 **/

error_t ipv4StringToAddr(const char_t *str, Ipv4Addr *ipAddr)
{
   error_t error;
   int_t i = 0;
   int_t value = -1;

   //Parse input string
   while(1)
   {
      //Decimal digit found?
      if(isdigit((uint8_t) *str))
      {
         //First digit to be decoded?
         if(value < 0)
            value = 0;

         //Update the value of the current byte
         value = (value * 10) + (*str - '0');

         //The resulting value shall be in range 0 to 255
         if(value > 255)
         {
            //The conversion failed
            error = ERROR_INVALID_SYNTAX;
            break;
         }
      }
      //Dot separator found?
      else if(*str == '.' && i < 4)
      {
         //Each dot must be preceded by a valid number
         if(value < 0)
         {
            //The conversion failed
            error = ERROR_INVALID_SYNTAX;
            break;
         }

         //Save the current byte
         ((uint8_t *) ipAddr)[i++] = value;
         //Prepare to decode the next byte
         value = -1;
      }
      //End of string detected?
      else if(*str == '\0' && i == 3)
      {
         //The NULL character must be preceded by a valid number
         if(value < 0)
         {
            //The conversion failed
            error = ERROR_INVALID_SYNTAX;
         }
         else
         {
            //Save the last byte of the IPv4 address
            ((uint8_t *) ipAddr)[i] = value;
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
 * @brief Convert a binary IPv4 address to dot-decimal notation
 * @param[in] ipAddr Binary representation of the IPv4 address
 * @param[out] str NULL-terminated string representing the IPv4 address
 * @return Pointer to the formatted string
 **/

char_t *ipv4AddrToString(Ipv4Addr ipAddr, char_t *str)
{
   uint8_t *p;
   static char_t buffer[16];

   //If the NULL pointer is given as parameter, then the internal buffer is used
   if(str == NULL)
      str = buffer;

   //Cast the address to byte array
   p = (uint8_t *) &ipAddr;
   //Format IPv4 address
   sprintf(str, "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "", p[0], p[1], p[2], p[3]);

   //Return a pointer to the formatted string
   return str;
}


/**
 * @brief Dump IPv4 header for debugging purpose
 * @param[in] ipHeader Pointer to the IPv4 header
 **/

void ipv4DumpHeader(const Ipv4Header *ipHeader)
{
   //Dump IP header contents
   TRACE_DEBUG("  Version = %" PRIu8 "\r\n", ipHeader->version);
   TRACE_DEBUG("  Header Length = %" PRIu8 "\r\n", ipHeader->headerLength);
   TRACE_DEBUG("  Type Of Service = %" PRIu8 "\r\n", ipHeader->typeOfService);
   TRACE_DEBUG("  Total Length = %" PRIu16 "\r\n", ntohs(ipHeader->totalLength));
   TRACE_DEBUG("  Identification = %" PRIu16 "\r\n", ntohs(ipHeader->identification));
   TRACE_DEBUG("  Flags = 0x%01X\r\n", ntohs(ipHeader->fragmentOffset) >> 13);
   TRACE_DEBUG("  Fragment Offset = %" PRIu16 "\r\n", ntohs(ipHeader->fragmentOffset) & 0x1FFF);
   TRACE_DEBUG("  Time To Live = %" PRIu8 "\r\n", ipHeader->timeToLive);
   TRACE_DEBUG("  Protocol = %" PRIu8 "\r\n", ipHeader->protocol);
   TRACE_DEBUG("  Header Checksum = 0x%04" PRIX16 "\r\n", ntohs(ipHeader->headerChecksum));
   TRACE_DEBUG("  Src Addr = %s\r\n", ipv4AddrToString(ipHeader->srcAddr, NULL));
   TRACE_DEBUG("  Dest Addr = %s\r\n", ipv4AddrToString(ipHeader->destAddr, NULL));
}

#endif
