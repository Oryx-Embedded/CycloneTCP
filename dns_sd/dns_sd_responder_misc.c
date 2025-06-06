/**
 * @file dns_sd_responder_misc.c
 * @brief Helper functions for DNS-SD responder
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
#define TRACE_LEVEL DNS_SD_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "mdns/mdns_responder.h"
#include "dns_sd/dns_sd_responder.h"
#include "dns_sd/dns_sd_responder_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (DNS_SD_RESPONDER_SUPPORT == ENABLED)


/**
 * @brief Update FSM state
 * @param[in] service Pointer to a DNS-SD service
 * @param[in] newState New state to switch to
 * @param[in] delay Initial delay
 **/

void dnsSdResponderChangeState(DnsSdResponderService *service,
   MdnsState newState, systime_t delay)
{
   DnsSdResponderContext *context;

   //Point to the DNS-SD responder context
   context = service->context;

   //Set time stamp
   service->timestamp = osGetSystemTime();
   //Set initial delay
   service->timeout = delay;
   //Reset retransmission counter
   service->retransmitCount = 0;
   //Switch to the new state
   service->state = newState;

   //Any registered callback?
   if(context->stateChangeEvent != NULL)
   {
      //Release exclusive access
      osReleaseMutex(&netMutex);
      //Invoke user callback function
      context->stateChangeEvent(service, context->interface, newState);
      //Get exclusive access
      osAcquireMutex(&netMutex);
   }
}


/**
 * @brief Programmatically change the service instance name
 * @param[in] service Pointer to a DNS-SD service
 **/

void dnsSdResponderChangeInstanceName(DnsSdResponderService *service)
{
   size_t i;
   size_t m;
   size_t n;
   uint32_t index;
   char_t s[16];

   //Retrieve the length of the string
   n = osStrlen(service->instanceName);

   //Parse the string backwards
   for(i = n; i > 0; i--)
   {
      //Last character?
      if(i == n)
      {
         //Check whether the last character is a bracket
         if(service->instanceName[i - 1] != ')')
            break;
      }
      else
      {
         //Check whether the current character is a digit
         if(!osIsdigit(service->instanceName[i - 1]))
            break;
      }
   }

   //Any number following the service instance name?
   if(service->instanceName[i] != '\0')
   {
      //Retrieve the number at the end of the name
      index = osAtoi(service->instanceName + i);
      //Increment the value
      index++;

      //Check the length of the name
      if(i >= 2)
      {
         //Discard any space and bracket that may precede the number
         if(service->instanceName[i - 2] == ' ' &&
            service->instanceName[i - 1] == '(')
         {
            i -= 2;
         }
      }

      //Strip the digits
      service->instanceName[i] = '\0';
   }
   else
   {
      //Append the digit "2" to the name
      index = 2;
   }

   //Convert the number to a string of characters
   m = osSprintf(s, " (%" PRIu32 ")", index);

   //Sanity check
   if((i + m) <= DNS_SD_MAX_INSTANCE_NAME_LEN)
   {
      //Programmatically change the service instance name
      osStrcat(service->instanceName, s);
   }
}


/**
 * @brief Send probe packet
 * @param[in] service Pointer to a DNS-SD service
 * @return Error code
 **/

error_t dnsSdResponderSendProbe(DnsSdResponderService *service)
{
   error_t error;
   DnsSdResponderContext *context;
   NetInterface *interface;
   DnsQuestion *dnsQuestion;
   MdnsMessage message;

   //Point to the DNS-SD responder context
   context = service->context;
   //Point to the underlying network interface
   interface = context->interface;

   //For all those resource records that a mDNS responder desires to be unique
   //on the local link, it must send a mDNS query asking for those resource
   //records, to see if any of them are already in use
   error = mdnsCreateMessage(&message, FALSE);
   //Any error to report?
   if(error)
      return error;

   //Start of exception handling block
   do
   {
      //Encode the service name using DNS notation
      message.length += mdnsEncodeName(service->instanceName,
         service->serviceName, ".local",
         (uint8_t *) message.dnsHeader + message.length);

      //Point to the corresponding question structure
      dnsQuestion = DNS_GET_QUESTION(message.dnsHeader, message.length);

      //The probes should be sent as QU questions with the unicast-response
      //bit set, to allow a defending host to respond immediately via unicast
      dnsQuestion->qtype = HTONS(DNS_RR_TYPE_ANY);
      dnsQuestion->qclass = HTONS(MDNS_QCLASS_QU | DNS_RR_CLASS_IN);

      //Update the length of the mDNS query message
      message.length += sizeof(DnsQuestion);

      //Number of questions in the Question Section
      message.dnsHeader->qdcount++;

      //Format SRV resource record
      error = dnsSdResponderFormatSrvRecord(interface, &message, service,
         FALSE, DNS_SD_DEFAULT_RR_TTL);
      //Any error to report?
      if(error)
         break;

      //Format TXT resource record
      error = dnsSdResponderFormatTxtRecord(interface, &message, service,
         FALSE, DNS_SD_DEFAULT_RR_TTL);
      //Any error to report?
      if(error)
         break;

      //A probe query can be distinguished from a normal query by the fact that
      //a probe query contains a proposed record in the Authority Section that
      //answers the question in the Question Section
      message.dnsHeader->nscount = message.dnsHeader->ancount;
      message.dnsHeader->ancount = 0;

      //Send mDNS message
      error = mdnsSendMessage(interface, &message, NULL, MDNS_PORT);

      //End of exception handling block
   } while(0);

   //Free previously allocated memory
   mdnsDeleteMessage(&message);

   //Return status code
   return error;
}


/**
 * @brief Send announcement packet
 * @param[in] service Pointer to a DNS-SD service
 * @return Error code
 **/

error_t dnsSdResponderSendAnnouncement(DnsSdResponderService *service)
{
   error_t error;
   DnsSdResponderContext *context;
   NetInterface *interface;
   MdnsMessage message;

   //Point to the DNS-SD responder context
   context = service->context;
   //Point to the underlying network interface
   interface = context->interface;

   //Send an unsolicited mDNS response containing, in the Answer Section, all
   //of its newly registered resource records
   error = mdnsCreateMessage(&message, TRUE);
   //Any error to report?
   if(error)
      return error;

   //Start of exception handling block
   do
   {
      //Format PTR resource record (service type enumeration)
      error = dnsSdResponderFormatServiceEnumPtrRecord(interface, &message,
         service, DNS_SD_DEFAULT_RR_TTL);
      //Any error to report?
      if(error)
         break;

      //Format PTR resource record
      error = dnsSdResponderFormatPtrRecord(interface, &message, service,
         DNS_SD_DEFAULT_RR_TTL);
      //Any error to report?
      if(error)
         break;

      //Format SRV resource record
      error = dnsSdResponderFormatSrvRecord(interface, &message, service, TRUE,
         DNS_SD_DEFAULT_RR_TTL);
      //Any error to report?
      if(error)
         break;

      //Format TXT resource record
      error = dnsSdResponderFormatTxtRecord(interface, &message, service, TRUE,
         DNS_SD_DEFAULT_RR_TTL);
      //Any error to report?
      if(error)
         break;

      //Send mDNS message
      error = mdnsSendMessage(interface, &message, NULL, MDNS_PORT);

      //End of exception handling block
   } while(0);

   //Free previously allocated memory
   mdnsDeleteMessage(&message);

   //Return status code
   return error;
}


/**
 * @brief Send goodbye packet
 * @param[in] service Pointer to a DNS-SD service
 * @return Error code
 **/

error_t dnsSdResponderSendGoodbye(DnsSdResponderService *service)
{
   error_t error;
   DnsSdResponderContext *context;
   NetInterface *interface;
   MdnsMessage message;

   //Point to the DNS-SD responder context
   context = service->context;
   //Point to the underlying network interface
   interface = context->interface;

   //Create an empty mDNS response message
   error = mdnsCreateMessage(&message, TRUE);
   //Any error to report?
   if(error)
      return error;

   //Start of exception handling block
   do
   {
      //Format PTR resource record (service type enumeration)
      error = dnsSdResponderFormatServiceEnumPtrRecord(interface, &message,
         service, 0);
      //Any error to report?
      if(error)
         break;

      //Format PTR resource record
      error = dnsSdResponderFormatPtrRecord(interface, &message, service, 0);
      //Any error to report?
      if(error)
         break;

      //Format SRV resource record
      error = dnsSdResponderFormatSrvRecord(interface, &message, service,
         TRUE, 0);
      //Any error to report?
      if(error)
         break;

      //Format TXT resource record
      error = dnsSdResponderFormatTxtRecord(interface, &message, service,
         TRUE, 0);
      //Any error to report?
      if(error)
         break;

      //Send mDNS message
      error = mdnsSendMessage(interface, &message, NULL, MDNS_PORT);

      //End of exception handling block
   } while(0);

   //Free previously allocated memory
   mdnsDeleteMessage(&message);

   //Return status code
   return error;
}


/**
 * @brief Parse a question
 * @param[in] interface Underlying network interface
 * @param[in] query Incoming mDNS query message
 * @param[in] offset Offset to first byte of the question
 * @param[in] question Pointer to the question
 * @param[in,out] response mDNS response message
 * @return Error code
 **/

error_t dnsSdResponderParseQuestion(NetInterface *interface,
   const MdnsMessage *query, size_t offset, const DnsQuestion *question,
   MdnsMessage *response)
{
   error_t error;
   uint_t i;
   uint16_t qclass;
   uint16_t qtype;
   uint32_t ttl;
   bool_t cacheFlush;
   DnsSdResponderContext *context;
   DnsSdResponderService *service;

   //Point to the DNS-SD responder context
   context = interface->dnsSdResponderContext;
   //Make sure the DNS-SD responder has been properly instantiated
   if(context == NULL)
      return NO_ERROR;

   //Convert the query class to host byte order
   qclass = ntohs(question->qclass);
   //Discard QU flag
   qclass &= ~MDNS_QCLASS_QU;

   //Convert the query type to host byte order
   qtype = ntohs(question->qtype);

   //Get the TTL resource record
   ttl = context->ttl;

   //Check whether the querier originating the query is a simple resolver
   if(ntohs(query->udpHeader->srcPort) != MDNS_PORT)
   {
      //The resource record TTL given in a legacy unicast response should not
      //be greater than ten seconds, even if the true TTL of the mDNS resource
      //record is higher
      ttl = MIN(ttl, MDNS_LEGACY_UNICAST_RR_TTL);

      //The cache-flush bit must not be set in legacy unicast responses
      cacheFlush = FALSE;
   }
   else
   {
      //The cache-bit should be set for unique resource records
      cacheFlush = TRUE;
   }

   //Loop through the list of registered services
   for(i = 0; i < context->numServices; i++)
   {
      //Point to the current entry
      service = &context->services[i];

      //Valid service?
      if(service->instanceName[0] != '\0' &&
         service->serviceName[0] != '\0' &&
         service->state != MDNS_STATE_INIT &&
         service->state != MDNS_STATE_WAITING &&
         service->state != MDNS_STATE_PROBING)
      {
         //Check the class of the query
         if(qclass == DNS_RR_CLASS_IN || qclass == DNS_RR_CLASS_ANY)
         {
            //Compare service name
            if(!mdnsCompareName(query->dnsHeader, query->length,
               offset, "", "_services._dns-sd._udp", ".local", 0))
            {
               //PTR query?
               if(qtype == DNS_RR_TYPE_PTR || qtype == DNS_RR_TYPE_ANY)
               {
                  //Format PTR resource record (service type enumeration)
                  error = dnsSdResponderFormatServiceEnumPtrRecord(interface,
                     response, service, ttl);
                  //Any error to report?
                  if(error)
                     return error;

                  //Update the number of shared resource records
                  response->sharedRecordCount++;
               }
            }
            else if(!mdnsCompareName(query->dnsHeader, query->length,
               offset, "", service->serviceName, ".local", 0))
            {
               //PTR query?
               if(qtype == DNS_RR_TYPE_PTR || qtype == DNS_RR_TYPE_ANY)
               {
                  //Format PTR resource record
                  error = dnsSdResponderFormatPtrRecord(interface, response,
                     service, ttl);
                  //Any error to report?
                  if(error)
                     return error;

                  //Update the number of shared resource records
                  response->sharedRecordCount++;
               }
            }
            else if(!mdnsCompareName(query->dnsHeader, query->length, offset,
               service->instanceName, service->serviceName, ".local", 0))
            {
               //SRV query?
               if(qtype == DNS_RR_TYPE_SRV || qtype == DNS_RR_TYPE_ANY)
               {
                  //Format SRV resource record
                  error = dnsSdResponderFormatSrvRecord(interface, response,
                     service, cacheFlush, ttl);
                  //Any error to report?
                  if(error)
                     return error;
               }

               //TXT query?
               if(qtype == DNS_RR_TYPE_TXT || qtype == DNS_RR_TYPE_ANY)
               {
                  //Format TXT resource record
                  error = dnsSdResponderFormatTxtRecord(interface, response,
                     service, cacheFlush, ttl);
                  //Any error to report?
                  if(error)
                     return error;
               }

               //NSEC query?
               if(qtype != DNS_RR_TYPE_SRV && qtype != DNS_RR_TYPE_TXT)
               {
                  //Format NSEC resource record
                  error = dnsSdResponderFormatNsecRecord(interface, response,
                     service, cacheFlush, ttl);
                  //Any error to report?
                  if(error)
                     return error;
               }
            }
         }
      }
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Parse the Authority Section
 * @param[in] interface Underlying network interface
 * @param[in] query Incoming mDNS query message
 * @param[in] offset Offset to first byte of the resource record
 **/

void dnsSdResponderParseNsRecords(NetInterface *interface,
   const MdnsMessage *query, size_t offset)
{
   uint_t i;
   uint_t j;
   int_t res;
   DnsSdResponderContext *context;
   DnsSdResponderService *service;
   DnsResourceRecord *record;

   //Point to the DNS-SD responder context
   context = interface->dnsSdResponderContext;
   //Make sure the DNS-SD responder has been properly instantiated
   if(context == NULL)
      return;

   //Loop through the list of registered services
   for(i = 0; i < context->numServices; i++)
   {
      //Point to the current entry
      service = &context->services[i];

      //Valid service?
      if(service->instanceName[0] != '\0' &&
         service->serviceName[0] != '\0')
      {
         //Get the first tiebreaker record in lexicographical order
         record = dnsSdResponderGetNextTiebreakerRecord(service, query, offset,
            NULL);

         //When a host is probing for a set of records with the same name, or a
         //message is received containing multiple tiebreaker records answering
         //a given probe question in the Question Section, the host's records
         //and the tiebreaker records from the message are each sorted into order
         for(j = 1; ; j++)
         {
            //The records are compared pairwise
            if(record == NULL && j >= 3)
            {
               //If both lists run out of records at the same time without any
               //difference being found, then this indicates that two devices
               //are advertising identical sets of records, as is sometimes done
               //for fault tolerance, and there is, in fact, no conflict
               break;
            }
            else if(record != NULL && j >= 3)
            {
               //If either list of records runs out of records before any
               //difference is found, then the list with records remaining is
               //deemed to have won the tiebreak
               service->tieBreakLost = TRUE;
               break;
            }
            else if(record == NULL && j < 3)
            {
               //The host has won the tiebreak
               break;
            }
            else
            {
               //The two records are compared and the lexicographically later data
               //wins
               if(j == 1)
               {
                  res = dnsSdResponderCompareTxtRecord(service, query, record);
               }
               else
               {
                  res = dnsSdResponderCompareSrvRecord(interface, service, query, record);
               }

               //Check comparison result
               if(res > 0)
               {
                  //If the host finds that its own data is lexicographically earlier,
                  //then it defers to the winning host by waiting one second, and
                  //then begins probing for this record again
                  service->tieBreakLost = TRUE;
                  break;
               }
               else if(res < 0)
               {
                  //If the host finds that its own data is lexicographically later,
                  //it simply ignores the other host's probe
                  break;
               }
               else
               {
                  //When comparing the records, if the first records match perfectly,
                  //then the second records are compared, and so on
               }
            }

            //Get the next tiebreaker record in lexicographical order
            record = dnsSdResponderGetNextTiebreakerRecord(service, query, offset,
               record);
         }
      }
   }
}


/**
 * @brief Parse a resource record from the Answer Section
 * @param[in] interface Underlying network interface
 * @param[in] response Incoming mDNS response message
 * @param[in] offset Offset to first byte of the resource record to be checked
 * @param[in] record Pointer to the resource record
 **/

void dnsSdResponderParseAnRecord(NetInterface *interface,
   const MdnsMessage *response, size_t offset, const DnsResourceRecord *record)
{
   uint_t i;
   uint16_t rclass;
   DnsSdResponderContext *context;
   DnsSdResponderService *service;

   //Point to the DNS-SD responder context
   context = interface->dnsSdResponderContext;
   //Make sure the DNS-SD responder has been properly instantiated
   if(context == NULL)
      return;

   //Loop through the list of registered services
   for(i = 0; i < context->numServices; i++)
   {
      //Point to the current entry
      service = &context->services[i];

      //Valid service?
      if(service->instanceName[0] != '\0' &&
         service->serviceName[0] != '\0')
      {
         //Check for conflicts
         if(!mdnsCompareName(response->dnsHeader, response->length, offset,
            service->instanceName, service->serviceName, ".local", 0))
         {
            //Convert the class to host byte order
            rclass = ntohs(record->rclass);
            //Discard Cache Flush flag
            rclass &= ~MDNS_RCLASS_CACHE_FLUSH;

            //Check the class of the resource record
            if(rclass == DNS_RR_CLASS_IN)
            {
               //A conflict occurs when a mDNS responder has a unique record
               //for which it is currently authoritative, and it receives a
               //mDNS response message containing a record with the same name,
               //rrtype and rrclass, but inconsistent rdata
               if(ntohs(record->rtype) == DNS_RR_TYPE_SRV)
               {
                  //Inconsistent rdata?
                  if(dnsSdResponderCompareSrvRecord(interface, service,
                     response, record) != 0)
                  {
                     //The service instance name is already in use by some
                     //other host
                     service->conflict = TRUE;
                  }
               }
               else if(ntohs(record->rtype) == DNS_RR_TYPE_TXT)
               {
                  //Inconsistent rdata?
                  if(dnsSdResponderCompareTxtRecord(service, response,
                     record) != 0)
                  {
                     //The service instance name is already in use by some
                     //other host
                     service->conflict = TRUE;
                  }
               }
               else
               {
                  //Just for sanity
               }
            }
         }
      }
   }
}


/**
 * @brief Additional record generation
 * @param[in] interface Underlying network interface
 * @param[in,out] response mDNS response message
 * @param[in] legacyUnicast This flag is set for legacy unicast responses
 **/

void dnsSdResponderGenerateAdditionalRecords(NetInterface *interface,
   MdnsMessage *response, bool_t legacyUnicast)
{
#if (DNS_SD_ADDITIONAL_RECORDS_SUPPORT == ENABLED)
   uint_t i;
   uint_t j;
   size_t n;
   size_t offset;
   uint_t ancount;
   uint16_t rclass;
   uint32_t ttl;
   bool_t cacheFlush;
   DnsSdResponderContext *context;
   DnsSdResponderService *service;
   DnsResourceRecord *record;

   //Point to the DNS-SD responder context
   context = interface->dnsSdResponderContext;
   //Make sure the DNS-SD responder has been properly instantiated
   if(context == NULL)
      return;

   //mDNS responses must not contain any questions in the Question Section
   if(response->dnsHeader->qdcount != 0)
      return;

   //Get the TTL resource record
   ttl = context->ttl;

   //Check whether the querier originating the query is a simple resolver
   if(legacyUnicast)
   {
      //The resource record TTL given in a legacy unicast response should
      //not be greater than ten seconds, even if the true TTL of the mDNS
      //resource record is higher
      ttl = MIN(ttl, MDNS_LEGACY_UNICAST_RR_TTL);

      //The cache-flush bit must not be set in legacy unicast responses
      cacheFlush = FALSE;
   }
   else
   {
      //The cache-bit should be set for unique resource records
      cacheFlush = TRUE;
   }

   //Point to the first resource record
   offset = sizeof(DnsHeader);

   //Save the number of resource records in the Answer Section
   ancount = response->dnsHeader->ancount;

   //Parse the Answer Section
   for(i = 0; i < ancount; i++)
   {
      //Parse resource record name
      n = dnsParseName(response->dnsHeader, response->length, offset, NULL, 0);
      //Invalid name?
      if(!n)
         break;

      //Point to the associated resource record
      record = DNS_GET_RESOURCE_RECORD(response->dnsHeader, n);
      //Point to the resource data
      n += sizeof(DnsResourceRecord);

      //Make sure the resource record is valid
      if(n > response->length)
         break;
      if((n + ntohs(record->rdlength)) > response->length)
         break;

      //Convert the record class to host byte order
      rclass = ntohs(record->rclass);
      //Discard the cache-flush bit
      rclass &= ~MDNS_RCLASS_CACHE_FLUSH;

      //Loop through the list of registered services
      for(j = 0; j < context->numServices; j++)
      {
         //Point to the current entry
         service = &context->services[j];

         //Valid service?
         if(service->instanceName[0] != '\0' &&
            service->serviceName[0] != '\0')
         {
            //Check the class of the resource record
            if(rclass == DNS_RR_CLASS_IN)
            {
               //PTR record?
               if(ntohs(record->rtype) == DNS_RR_TYPE_PTR)
               {
                  //Compare service name
                  if(!mdnsCompareName(response->dnsHeader, response->length,
                     offset, "", service->serviceName, ".local", 0))
                  {
                     //Format SRV resource record
                     dnsSdResponderFormatSrvRecord(interface, response, service,
                        cacheFlush, ttl);

                     //Format TXT resource record
                     dnsSdResponderFormatTxtRecord(interface, response, service,
                        cacheFlush, ttl);
                  }
               }
               //SRV record?
               else if(ntohs(record->rtype) == DNS_RR_TYPE_SRV)
               {
                  //Compare service name
                  if(!mdnsCompareName(response->dnsHeader, response->length,
                     offset, service->instanceName, service->serviceName,
                     ".local", 0))
                  {
                     //Format TXT resource record
                     dnsSdResponderFormatTxtRecord(interface, response, service,
                        cacheFlush, ttl);
                  }
               }
            }
         }
      }

      //Point to the next resource record
      offset = n + ntohs(record->rdlength);
   }

   //Number of resource records in the Additional Section
   response->dnsHeader->arcount += response->dnsHeader->ancount - ancount;
   //Number of resource records in the Answer Section
   response->dnsHeader->ancount = ancount;
#endif
}


/**
 * @brief Format PTR resource record (in response to a meta-query)
 * @param[in] interface Underlying network interface
 * @param[in,out] message Pointer to the mDNS message
 * @param[in] service Pointer to a DNS-SD service
 * @param[in] ttl Resource record TTL (cache lifetime)
 * @return Error code
 **/

error_t dnsSdResponderFormatServiceEnumPtrRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service, uint32_t ttl)
{
   size_t n;
   size_t offset;
   bool_t duplicate;
   DnsResourceRecord *record;

   //Check whether the resource record is already present in the Answer
   //Section of the message
   duplicate = mdnsCheckDuplicateRecord(message, "", "_services._dns-sd._udp",
      ".local", DNS_RR_TYPE_PTR, NULL, 0);

   //The duplicates should be suppressed and the resource record should
   //appear only once in the list
   if(!duplicate)
   {
      //Set the position to the end of the buffer
      offset = message->length;

      //The first pass calculates the length of the DNS encoded service name
      n = mdnsEncodeName("", "_services._dns-sd._udp", ".local", NULL);

      //Check the length of the resulting mDNS message
      if((offset + n) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //The second pass encodes the service name using the DNS name notation
      offset += mdnsEncodeName("", "_services._dns-sd._udp", ".local",
         (uint8_t *) message->dnsHeader + offset);

      //Consider the length of the resource record itself
      if((offset + sizeof(DnsResourceRecord)) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //Point to the corresponding resource record
      record = DNS_GET_RESOURCE_RECORD(message->dnsHeader, offset);

      //Fill in resource record
      record->rtype = HTONS(DNS_RR_TYPE_PTR);
      record->rclass = HTONS(DNS_RR_CLASS_IN);
      record->ttl = htonl(ttl);

      //Advance write index
      offset += sizeof(DnsResourceRecord);

      //The first pass calculates the length of the DNS encoded service name
      n = mdnsEncodeName("", service->serviceName, ".local", NULL);

      //Check the length of the resulting mDNS message
      if((offset + n) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //The second pass encodes the service name using DNS notation
      n = mdnsEncodeName("", service->serviceName, ".local", record->rdata);

      //Convert length field to network byte order
      record->rdlength = htons(n);

      //Number of resource records in the answer section
      message->dnsHeader->ancount++;
      //Update the length of the DNS message
      message->length = offset + n;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Format PTR resource record
 * @param[in] interface Underlying network interface
 * @param[in,out] message Pointer to the mDNS message
 * @param[in] service Pointer to a DNS-SD service
 * @param[in] ttl Resource record TTL (cache lifetime)
 * @return Error code
 **/

error_t dnsSdResponderFormatPtrRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service, uint32_t ttl)
{
   size_t n;
   size_t offset;
   bool_t duplicate;
   uint8_t *p;
   DnsResourceRecord *record;

   //Set the position to the end of the buffer
   p = (uint8_t *) message->dnsHeader + message->length;
   offset = message->length;

   //The first pass calculates the length of the DNS encoded instance name
   n = mdnsEncodeName(service->instanceName, service->serviceName, ".local",
      NULL);

   //Sanity check
   if((message->length + n) > MDNS_MESSAGE_MAX_SIZE)
      return ERROR_MESSAGE_TOO_LONG;

   //The second pass encodes the instance name using DNS notation
   n = mdnsEncodeName(service->instanceName, service->serviceName, ".local", p);

   //Check whether the resource record is already present in the Answer
   //Section of the message
   duplicate = mdnsCheckDuplicateRecord(message, "", service->serviceName,
      ".local", DNS_RR_TYPE_PTR, p, n);

   //The duplicates should be suppressed and the resource record should
   //appear only once in the list
   if(!duplicate)
   {
      //The first pass calculates the length of the DNS encoded service name
      n = mdnsEncodeName("", service->serviceName, ".local", NULL);

      //Check the length of the resulting mDNS message
      if((offset + n) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //The second pass encodes the service name using the DNS name notation
      offset += mdnsEncodeName("", service->serviceName, ".local", p);

      //Consider the length of the resource record itself
      if((offset + sizeof(DnsResourceRecord)) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //Point to the corresponding resource record
      record = DNS_GET_RESOURCE_RECORD(message->dnsHeader, offset);

      //Fill in resource record
      record->rtype = HTONS(DNS_RR_TYPE_PTR);
      record->rclass = HTONS(DNS_RR_CLASS_IN);
      record->ttl = htonl(ttl);

      //Advance write index
      offset += sizeof(DnsResourceRecord);

      //The first pass calculates the length of the DNS encoded instance name
      n = mdnsEncodeName(service->instanceName, service->serviceName, ".local",
         NULL);

      //Check the length of the resulting mDNS message
      if((offset + n) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //The second pass encodes the instance name using DNS notation
      n = mdnsEncodeName(service->instanceName, service->serviceName, ".local",
         record->rdata);

      //Convert length field to network byte order
      record->rdlength = htons(n);

      //Number of resource records in the answer section
      message->dnsHeader->ancount++;
      //Update the length of the DNS message
      message->length = offset + n;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Format SRV resource record
 * @param[in] interface Underlying network interface
 * @param[in,out] message Pointer to the mDNS message
 * @param[in] service Pointer to a DNS-SD service
 * @param[in] cacheFlush Cache-flush bit
 * @param[in] ttl Resource record TTL (cache lifetime)
 * @return Error code
 **/

error_t dnsSdResponderFormatSrvRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service,
   bool_t cacheFlush, uint32_t ttl)
{
   size_t n;
   size_t offset;
   bool_t duplicate;
   MdnsResponderContext *mdnsResponderContext;
   DnsSrvResourceRecord *record;

   //Point to the mDNS responder context
   mdnsResponderContext = interface->mdnsResponderContext;

   //Check whether the resource record is already present in the Answer
   //Section of the message
   duplicate = mdnsCheckDuplicateRecord(message, service->instanceName,
      service->serviceName, ".local", DNS_RR_TYPE_SRV, NULL, 0);

   //The duplicates should be suppressed and the resource record should
   //appear only once in the list
   if(!duplicate)
   {
      //Set the position to the end of the buffer
      offset = message->length;

      //The first pass calculates the length of the DNS encoded instance name
      n = mdnsEncodeName(service->instanceName, service->serviceName,
         ".local", NULL);

      //Check the length of the resulting mDNS message
      if((offset + n) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //The second pass encodes the instance name using DNS notation
      offset += mdnsEncodeName(service->instanceName, service->serviceName,
         ".local", (uint8_t *) message->dnsHeader + offset);

      //Consider the length of the resource record itself
      if((offset + sizeof(DnsSrvResourceRecord)) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //Point to the corresponding resource record
      record = (DnsSrvResourceRecord *) DNS_GET_RESOURCE_RECORD(message->dnsHeader,
         offset);

      //Fill in resource record
      record->rtype = HTONS(DNS_RR_TYPE_SRV);
      record->rclass = HTONS(DNS_RR_CLASS_IN);
      record->ttl = htonl(ttl);
      record->priority = htons(service->priority);
      record->weight = htons(service->weight);
      record->port = htons(service->port);

      //Check whether the cache-flush bit should be set
      if(cacheFlush)
      {
         record->rclass |= HTONS(MDNS_RCLASS_CACHE_FLUSH);
      }

      //Advance write index
      offset += sizeof(DnsSrvResourceRecord);

      //The first pass calculates the length of the DNS encoded target name
      n = mdnsEncodeName("", mdnsResponderContext->hostname, ".local", NULL);

      //Check the length of the resulting mDNS message
      if((offset + n) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //The second pass encodes the target name using DNS notation
      n = mdnsEncodeName("", mdnsResponderContext->hostname, ".local",
         record->target);

      //Calculate data length
      record->rdlength = htons(sizeof(DnsSrvResourceRecord) -
         sizeof(DnsResourceRecord) + n);

      //Number of resource records in the answer section
      message->dnsHeader->ancount++;
      //Update the length of the DNS message
      message->length = offset + n;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Format TXT resource record
 * @param[in] interface Underlying network interface
 * @param[in,out] message Pointer to the mDNS message
 * @param[in] service Pointer to a DNS-SD service
 * @param[in] cacheFlush Cache-flush bit
 * @param[in] ttl Resource record TTL (cache lifetime)
 * @return Error code
 **/

error_t dnsSdResponderFormatTxtRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service,
   bool_t cacheFlush, uint32_t ttl)
{
   size_t n;
   size_t offset;
   bool_t duplicate;
   DnsResourceRecord *record;

   //Check whether the resource record is already present in the Answer
   //Section of the message
   duplicate = mdnsCheckDuplicateRecord(message, service->instanceName,
      service->serviceName, ".local", DNS_RR_TYPE_TXT, NULL, 0);

   //The duplicates should be suppressed and the resource record should
   //appear only once in the list
   if(!duplicate)
   {
      //Set the position to the end of the buffer
      offset = message->length;

      //The first pass calculates the length of the DNS encoded instance name
      n = mdnsEncodeName(service->instanceName, service->serviceName, ".local",
         NULL);

      //Check the length of the resulting mDNS message
      if((offset + n) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //The second pass encodes the instance name using DNS notation
      offset += mdnsEncodeName(service->instanceName, service->serviceName,
         ".local", (uint8_t *) message->dnsHeader + offset);

      //Consider the length of the resource record itself
      if((offset + sizeof(DnsResourceRecord)) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //Point to the corresponding resource record
      record = DNS_GET_RESOURCE_RECORD(message->dnsHeader, offset);

      //Fill in resource record
      record->rtype = HTONS(DNS_RR_TYPE_TXT);
      record->rclass = HTONS(DNS_RR_CLASS_IN);
      record->ttl = htonl(ttl);
      record->rdlength = htons(service->metadataLen);

      //Check whether the cache-flush bit should be set
      if(cacheFlush)
      {
         record->rclass |= HTONS(MDNS_RCLASS_CACHE_FLUSH);
      }

      //Advance write index
      offset += sizeof(DnsResourceRecord);

      //Check the length of the resulting mDNS message
      if((offset + service->metadataLen) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //Copy metadata
      osMemcpy(record->rdata, service->metadata, service->metadataLen);

      //Update the length of the DNS message
      message->length = offset + service->metadataLen;
      //Number of resource records in the answer section
      message->dnsHeader->ancount++;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Format NSEC resource record
 * @param[in] interface Underlying network interface
 * @param[in,out] message Pointer to the mDNS message
 * @param[in] service Pointer to a DNS-SD service
 * @param[in] cacheFlush Cache-flush bit
 * @param[in] ttl Resource record TTL (cache lifetime)
 * @return Error code
 **/

error_t dnsSdResponderFormatNsecRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdResponderService *service,
   bool_t cacheFlush, uint32_t ttl)
{
   size_t n;
   size_t offset;
   bool_t duplicate;
   size_t bitmapLen;
   uint8_t bitmap[8];
   DnsResourceRecord *record;

   //Check whether the resource record is already present in the Answer
   //Section of the message
   duplicate = mdnsCheckDuplicateRecord(message, service->instanceName,
      service->serviceName, ".local", DNS_RR_TYPE_NSEC, NULL, 0);

   //The duplicates should be suppressed and the resource record should
   //appear only once in the list
   if(!duplicate)
   {
      //The bitmap identifies the resource record types that exist
      osMemset(bitmap, 0, sizeof(bitmap));

      //TXT resource record is supported
      DNS_SET_NSEC_BITMAP(bitmap, DNS_RR_TYPE_TXT);
      //SRV resource record is supported
      DNS_SET_NSEC_BITMAP(bitmap, DNS_RR_TYPE_SRV);

      //Compute the length of the bitmap
      for(bitmapLen = sizeof(bitmap); bitmapLen > 0; bitmapLen--)
      {
         //Trailing zero octets in the bitmap must be omitted
         if(bitmap[bitmapLen - 1] != 0)
         {
            break;
         }
      }

      //Set the position to the end of the buffer
      offset = message->length;

      //The first pass calculates the length of the DNS encoded instance name
      n = mdnsEncodeName(service->instanceName, service->serviceName, ".local",
         NULL);

      //Check the length of the resulting mDNS message
      if((offset + n) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //The second pass encodes the instance name using the DNS name notation
      offset += mdnsEncodeName(service->instanceName, service->serviceName,
         ".local", (uint8_t *) message->dnsHeader + offset);

      //Consider the length of the resource record itself
      if((offset + sizeof(DnsResourceRecord)) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //Point to the corresponding resource record
      record = DNS_GET_RESOURCE_RECORD(message->dnsHeader, offset);

      //Fill in resource record
      record->rtype = HTONS(DNS_RR_TYPE_NSEC);
      record->rclass = HTONS(DNS_RR_CLASS_IN);
      record->ttl = htonl(ttl);

      //Check whether the cache-flush bit should be set
      if(cacheFlush)
      {
         record->rclass |= HTONS(MDNS_RCLASS_CACHE_FLUSH);
      }

      //Advance write index
      offset += sizeof(DnsResourceRecord);

      //Check the length of the resulting mDNS message
      if((offset + n + 2) > MDNS_MESSAGE_MAX_SIZE)
         return ERROR_MESSAGE_TOO_LONG;

      //The Next Domain Name field contains the record's own name
      mdnsEncodeName(service->instanceName, service->serviceName,
         ".local", record->rdata);

      //DNS NSEC record is limited to Window Block number zero
      record->rdata[n++] = 0;
      //The Bitmap Length is a value in the range 1-32
      record->rdata[n++] = bitmapLen;

      //The Bitmap data identifies the resource record types that exist
      osMemcpy(record->rdata + n, bitmap, bitmapLen);

      //Convert length field to network byte order
      record->rdlength = htons(n + bitmapLen);

      //Number of resource records in the answer section
      message->dnsHeader->ancount++;
      //Update the length of the DNS message
      message->length = offset + n + bitmapLen;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Sort the tiebreaker records in lexicographical order
 * @param[in] service Pointer to a DNS-SD service
 * @param[in] query Incoming mDNS query message
 * @param[in] offset Offset to first byte of the Authority Section
 * @param[in] record Pointer to the current record
 * @return Pointer to the next record, if any
 **/

DnsResourceRecord *dnsSdResponderGetNextTiebreakerRecord(
   DnsSdResponderService *service, const MdnsMessage *query, size_t offset,
   DnsResourceRecord *record)
{
   uint_t i;
   size_t n;
   int_t res;
   DnsResourceRecord *curRecord;
   DnsResourceRecord *nextRecord;

   //Initialize record pointer
   nextRecord = NULL;

   //Parse Authority Section
   for(i = 0; i < ntohs(query->dnsHeader->nscount); i++)
   {
      //Parse resource record name
      n = dnsParseName(query->dnsHeader, query->length, offset, NULL, 0);
      //Invalid name?
      if(!n)
         break;

      //Point to the associated resource record
      curRecord = DNS_GET_RESOURCE_RECORD(query->dnsHeader, n);
      //Point to the resource data
      n += sizeof(DnsResourceRecord);

      //Make sure the resource record is valid
      if(n > query->length)
         break;
      if((n + ntohs(curRecord->rdlength)) > query->length)
         break;

      //Matching host name?
      if(!mdnsCompareName(query->dnsHeader, query->length, offset,
         service->instanceName, service->serviceName, ".local", 0))
      {
         //Perform lexicographical comparison
         if(record != NULL)
         {
            res = mdnsCompareRecord(query, curRecord, query, record);
         }
         else
         {
            res = 1;
         }

         //Check whether the record is lexicographically later
         if(res > 0)
         {
            if(nextRecord == NULL)
            {
               nextRecord = curRecord;
            }
            else if(mdnsCompareRecord(query, curRecord, query, nextRecord) < 0)
            {
               nextRecord = curRecord;
            }
         }
      }

      //Point to the next resource record
      offset = n + ntohs(curRecord->rdlength);
   }

   //Return the pointer to the next record
   return nextRecord;
}


/**
 * @brief Compare SRV resource records
 * @param[in] interface Underlying network interface
 * @param[in] service Pointer to a DNS-SD service
 * @param[in] message Pointer to mDNS message
 * @param[in] record Pointer the resource record
 * @return The function returns 0 if the resource record match the SRV resource
 *   record of the host, -1 if the resource record lexicographically precedes
 *   it, or 1 if the resource record lexicographically precedes it
 **/

int_t dnsSdResponderCompareSrvRecord(NetInterface *interface,
   DnsSdResponderService *service, const MdnsMessage *message,
   const DnsResourceRecord *record)
{
   int_t res;
   size_t offset;
   uint16_t value;
   MdnsResponderContext *mdnsResponderContext;
   DnsSrvResourceRecord *srvRecord;

   //Point to the mDNS responder context
   mdnsResponderContext = interface->mdnsResponderContext;

   //Convert the record class to host byte order
   value = ntohs(record->rclass);
   //Discard cache-flush bit
   value &= ~MDNS_RCLASS_CACHE_FLUSH;

   //The determination of lexicographically later record is performed by
   //first comparing the record class (excluding the cache-flush bit)
   if(value < DNS_RR_CLASS_IN)
   {
      return -1;
   }
   else if(value > DNS_RR_CLASS_IN)
   {
      return 1;
   }
   else
   {
   }

   //Convert the record type to host byte order
   value = ntohs(record->rtype);

   //Then compare the record type
   if(value < DNS_RR_TYPE_SRV)
   {
      return -1;
   }
   else if(value > DNS_RR_TYPE_SRV)
   {
      return 1;
   }
   else
   {
   }

   //If the rrtype and rrclass both match, then the rdata is compared
   srvRecord = (DnsSrvResourceRecord *) record;
   //Convert the Priority field to host byte order
   value = ntohs(srvRecord->priority);

   //Compare Priority fields
   if(value < service->priority)
   {
      return -1;
   }
   else if(value > service->priority)
   {
      return 1;
   }
   else
   {
   }

   //Convert the Weight field to host byte order
   value = ntohs(srvRecord->weight);

   //Compare Weight fields
   if(value < service->weight)
   {
      return -1;
   }
   else if(value > service->weight)
   {
      return 1;
   }
   else
   {
   }

   //Convert the Port field to host byte order
   value = ntohs(srvRecord->port);

   //Compare Port fields
   if(value < service->port)
   {
      return -1;
   }
   else if(value > service->port)
   {
      return 1;
   }
   else
   {
   }

   //Compute the offset of the first byte of the Target field
   offset = srvRecord->target - (uint8_t *) message->dnsHeader;

   //Compare Target fields
   res = mdnsCompareName(message->dnsHeader, message->length, offset,
      "", mdnsResponderContext->hostname, ".local", 0);

   //Return comparison result
   return res;
}


/**
 * @brief Compare TXT resource records
 * @param[in] service Pointer to a DNS-SD service
 * @param[in] message Pointer to mDNS message
 * @param[in] record Pointer the resource record
 * @return The function returns 0 if the resource record match the TXT resource
 *   record of the host, -1 if the resource record lexicographically precedes
 *   it, or 1 if the resource record lexicographically precedes it
 **/

int_t dnsSdResponderCompareTxtRecord(DnsSdResponderService *service,
   const MdnsMessage *message, const DnsResourceRecord *record)
{
   int_t res;
   size_t n;
   uint16_t value;

   //Convert the record class to host byte order
   value = ntohs(record->rclass);
   //Discard cache-flush bit
   value &= ~MDNS_RCLASS_CACHE_FLUSH;

   //The determination of lexicographically later record is performed by
   //first comparing the record class (excluding the cache-flush bit)
   if(value < DNS_RR_CLASS_IN)
   {
      return -1;
   }
   else if(value > DNS_RR_CLASS_IN)
   {
      return 1;
   }
   else
   {
   }

   //Convert the record type to host byte order
   value = ntohs(record->rtype);

   //Then compare the record type
   if(value < DNS_RR_TYPE_TXT)
   {
      return -1;
   }
   else if(value > DNS_RR_TYPE_TXT)
   {
      return 1;
   }
   else
   {
   }

   //Retrieve the length of the rdata fields
   n = htons(record->rdlength);

   //The bytes of the raw uncompressed rdata are compared in turn, interpreting
   //the bytes as eight-bit unsigned values, until a byte is found whose value
   //is greater than that of its counterpart (in which case, the rdata whose
   //byte has the greater value is deemed lexicographically later) or one of the
   //resource records runs out of rdata (in which case, the resource record which
   //still has remaining data first is deemed lexicographically later)
   if(n < service->metadataLen)
   {
      //Raw comparison of the binary content of the rdata
      res = osMemcmp(record->rdata, service->metadata, n);

      //Check comparison result
      if(!res)
      {
         //The first resource records runs out of rdata
         res = -1;
      }
   }
   else if(n > service->metadataLen)
   {
      //Raw comparison of the binary content of the rdata
      res = osMemcmp(record->rdata, service->metadata, service->metadataLen);

      //Check comparison result
      if(!res)
      {
         //The second resource records runs out of rdata
         res = 1;
      }
   }
   else
   {
      //Raw comparison of the binary content of the rdata
      res = osMemcmp(record->rdata, service->metadata, n);
   }

   //Return comparison result
   return res;
}

#endif
