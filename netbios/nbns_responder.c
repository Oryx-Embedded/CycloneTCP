/**
 * @file nbns_responder.c
 * @brief NBNS responder (NetBIOS Name Service)
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
 * @version 2.5.4
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NBNS_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "netbios/nbns_responder.h"
#include "netbios/nbns_common.h"
#include "dns/dns_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (NBNS_RESPONDER_SUPPORT == ENABLED && IPV4_SUPPORT == ENABLED)


/**
 * @brief Process NBNS query message
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader UDP pseudo header
 * @param[in] udpHeader UDP header
 * @param[in] message Pointer to the NBNS query message
 * @param[in] length Length of the message
 **/

void nbnsProcessQuery(NetInterface *interface,
   const Ipv4PseudoHeader *pseudoHeader, const UdpHeader *udpHeader,
   const NbnsHeader *message, size_t length)
{
   size_t pos;
   uint16_t destPort;
   IpAddr destIpAddr;
   DnsQuestion *question;

   //The NBNS query shall contain one question
   if(ntohs(message->qdcount) != 1)
      return;

   //Parse NetBIOS name
   pos = nbnsParseName(message, length, sizeof(DnsHeader), NULL);

   //Invalid name?
   if(!pos)
      return;
   //Malformed NBNS query message?
   if((pos + sizeof(DnsQuestion)) > length)
      return;

   //Point to the corresponding entry
   question = DNS_GET_QUESTION(message, pos);

   //Check the class of the request
   if(ntohs(question->qclass) != DNS_RR_CLASS_IN)
      return;

   //A response packet is always sent to the source UDP port and source IP
   //address of the request packet
   destIpAddr.length = sizeof(Ipv4Addr);
   destIpAddr.ipv4Addr = pseudoHeader->srcAddr;

   //Convert the port number to host byte order
   destPort = ntohs(udpHeader->srcPort);

   //Check the type of the request
   if(ntohs(question->qtype) == DNS_RR_TYPE_NB)
   {
      //Compare NetBIOS names
      if(nbnsCompareName(message, length, sizeof(DnsHeader),
         interface->hostname))
      {
         //Send positive name query response
         nbnsSendResponse(interface, &destIpAddr, destPort, message->id,
            DNS_RR_TYPE_NB);
      }
   }
   else if(ntohs(question->qtype) == DNS_RR_TYPE_NBSTAT)
   {
      //Send node status response
      nbnsSendResponse(interface, &destIpAddr, destPort, message->id,
         DNS_RR_TYPE_NBSTAT);
   }
   else
   {
      //Unknown request
   }
}


/**
 * @brief Send NBNS response message
 * @param[in] interface Underlying network interface
 * @param[in] destIpAddr Destination IP address
 * @param[in] destPort destination port
 * @param[in] id 16-bit identifier to be used when sending NBNS query
 * @param[in] qtype Resource record type
 **/

error_t nbnsSendResponse(NetInterface *interface, const IpAddr *destIpAddr,
   uint16_t destPort, uint16_t id, uint16_t qtype)
{
   error_t error;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   NbnsHeader *message;
   DnsResourceRecord *record;
   NetTxAncillary ancillary;

   //Initialize status code
   error = NO_ERROR;

   //Allocate a memory buffer to hold the NBNS response message
   buffer = udpAllocBuffer(DNS_MESSAGE_MAX_SIZE, &offset);
   //Failed to allocate buffer?
   if(buffer == NULL)
      return ERROR_OUT_OF_MEMORY;

   //Point to the NBNS header
   message = netBufferAt(buffer, offset, 0);

   //Take the identifier from the query message
   message->id = id;

   //Format NBNS response header
   message->qr = 1;
   message->opcode = DNS_OPCODE_QUERY;
   message->aa = 1;
   message->tc = 0;
   message->rd = 0;
   message->ra = 0;
   message->z = 0;
   message->b = 0;
   message->rcode = DNS_RCODE_NOERROR;
   message->qdcount = 0;
   message->ancount = 0;
   message->nscount = 0;
   message->arcount = 0;

   //NBNS response message length
   length = sizeof(DnsHeader);

   //Check the type of the requested resource record
   if(qtype == DNS_RR_TYPE_NB)
   {
      uint_t i;
      Ipv4AddrEntry *entry;
      NbnsAddrEntry *addrEntry;

      //Set RD and RA flags
      message->rd = 1;
      message->ra = 1;

      //Loop through the list of IPv4 addresses assigned to the interface
      for(i = 0; i < IPV4_ADDR_LIST_SIZE && message->ancount == 0; i++)
      {
         //Point to the current entry
         entry = &interface->ipv4Context.addrList[i];

         //Check the state of the address
         if(entry->state == IPV4_ADDR_STATE_VALID)
         {
            //Check whether the address belongs to the same subnet as the source
            //address of the query
            if(ipv4IsOnSubnet(entry, destIpAddr->ipv4Addr))
            {
               //Encode the host name using the NBNS name notation
               length += nbnsEncodeName(interface->hostname,
                  (uint8_t *) message + length);

               //Point to the corresponding resource record
               record = DNS_GET_RESOURCE_RECORD(message, length);

               //Fill in resource record
               record->rtype = HTONS(DNS_RR_TYPE_NB);
               record->rclass = HTONS(DNS_RR_CLASS_IN);
               record->ttl = HTONL(NBNS_DEFAULT_RESOURCE_RECORD_TTL);
               record->rdlength = HTONS(sizeof(NbnsAddrEntry));

               //The ADDR_ENTRY ARRAY a sequence of zero or more ADDR_ENTRY
               //records (refer to RFC 1002, section 4.2.13)
               addrEntry = (NbnsAddrEntry *) record->rdata;

               //Each ADDR_ENTRY record represents an owner of a name
               addrEntry->flags = HTONS(NBNS_FLAG_ONT_BNODE);
               addrEntry->addr = entry->addr;

               //Update the length of the NBNS response message
               length += sizeof(DnsResourceRecord) + sizeof(NbnsAddrEntry);

               //Number of resource records in the answer section
               message->ancount++;
            }
         }
      }
   }
   else if(qtype == DNS_RR_TYPE_NBSTAT)
   {
      size_t i;
      size_t n;
      NbnsNodeNameArray *nodeNameArray;
      NbnsStatistics *statistics;

      //Determine the length of the host name
      n = osStrlen(interface->hostname);

      //Valid host name assigned to the interface?
      if(n >= 1 && n <= 15)
      {
         //RR_NAME is the requesting name
         length += nbnsEncodeName("*", (uint8_t *) message + length);

         //Point to the corresponding resource record
         record = DNS_GET_RESOURCE_RECORD(message, length);

         //Fill in resource record
         record->rtype = HTONS(DNS_RR_TYPE_NBSTAT);
         record->rclass = HTONS(DNS_RR_CLASS_IN);
         record->ttl = HTONL(0);

         //Calculate the length of the resource record
         record->rdlength = HTONS(sizeof(NbnsNodeNameArray) +
            sizeof(NbnsNodeNameEntry) + sizeof(NbnsStatistics));

         //The NODE_NAME ARRAY is an array of zero or more NUM_NAMES entries
         //of NODE_NAME records (refer to RFC 1002, section 4.2.18)
         nodeNameArray = (NbnsNodeNameArray *) record->rdata;

         //Set NUM_NAMES field
         nodeNameArray->numNames = 1;

         //Each NODE_NAME entry represents an active name in the same NetBIOS
         //scope as the requesting name in the local name table of the responder
         for(i = 0; i < n; i++)
         {
            nodeNameArray->names[0].name[i] = osToupper(interface->hostname[i]);
         }

         //Pad NetBIOS name with space characters
         for(; i < 15; i++)
         {
            nodeNameArray->names[0].name[i] = ' ';
         }

         //The 16th character is the NetBIOS suffix
         nodeNameArray->names[0].name[15] = 0;

         //Set NAME_FLAGS field
         nodeNameArray->names[0].flags = HTONS(NBNS_NAME_FLAG_ONT_BNODE |
            NBNS_NAME_FLAG_ACT);

         //Point to the STATISTICS field
         statistics = (NbnsStatistics *) (record->rdata +
            sizeof(NbnsNodeNameArray) + sizeof(NbnsNodeNameEntry));

         //Clear statistics
         osMemset(statistics, 0, sizeof(NbnsStatistics));
         //The UNIT_ID field specifies the unique unit ID
         statistics->unitId = interface->macAddr;

         //Update the length of the NBNS response message
         length += sizeof(DnsResourceRecord) + sizeof(NbnsNodeNameArray) +
            sizeof(NbnsNodeNameEntry) + sizeof(NbnsStatistics);

         //Number of resource records in the answer section
         message->ancount++;
      }
   }
   else
   {
      //Just for sanity
   }

   //Valid NBNS response?
   if(message->ancount > 0)
   {
      //The ANCOUNT field specifies the number of resource records in the
      //answer section
      message->ancount = htons(message->ancount);

      //Adjust the length of the multi-part buffer
      netBufferSetLength(buffer, offset + length);

      //Debug message
      TRACE_INFO("Sending NBNS message (%" PRIuSIZE " bytes)...\r\n", length);
      //Dump message
      dnsDumpMessage((DnsHeader *) message, length);

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_TX_ANCILLARY;

      //This flag tells the stack that the destination is on a locally attached
      //network and not to perform a lookup of the routing table
      ancillary.dontRoute = TRUE;

      //A response packet is always sent to the source UDP port and source IP
      //address of the request packet
      error = udpSendBuffer(interface, NULL, NBNS_PORT, destIpAddr, destPort,
         buffer, offset, &ancillary);
   }

   //Free previously allocated memory
   netBufferFree(buffer);

   //Return status code
   return error;
}

#endif
