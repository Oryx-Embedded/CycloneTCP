/**
 * @file mld_debug.c
 * @brief Data logging functions for debugging purpose (MLD)
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
#define TRACE_LEVEL MLD_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "ipv6/icmpv6.h"
#include "mld/mld_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && MLD_TRACE_LEVEL >= TRACE_LEVEL_DEBUG)

//MLD message types
const MldParamName mldMessageTypeList[] =
{
   {ICMPV6_TYPE_MCAST_LISTENER_QUERY,     "Multicast Listener Query"},
   {ICMPV6_TYPE_MCAST_LISTENER_REPORT_V1, "Version 1 Multicast Listener Report"},
   {ICMPV6_TYPE_MCAST_LISTENER_DONE_V1,   "Version 1 Multicast Listener Done"},
   {ICMPV6_TYPE_MCAST_LISTENER_REPORT_V2, "Version 2 Multicast Listener Report"}
};

//MLDv2 multicast address record types
const MldParamName mldMulticastAddrRecordTypeList[] =
{
   {MLD_MCAST_ADDR_RECORD_TYPE_IS_IN, "MODE_IS_INCLUDE"},
   {MLD_MCAST_ADDR_RECORD_TYPE_IS_EX, "MODE_IS_EXCLUDE"},
   {MLD_MCAST_ADDR_RECORD_TYPE_TO_IN, "CHANGE_TO_INCLUDE_MODE"},
   {MLD_MCAST_ADDR_RECORD_TYPE_TO_EX, "CHANGE_TO_EXCLUDE_MODE"},
   {MLD_MCAST_ADDR_RECORD_TYPE_ALLOW, "ALLOW_NEW_SOURCES"},
   {MLD_MCAST_ADDR_RECORD_TYPE_BLOCK, "BLOCK_OLD_SOURCES"}
};


/**
 * @brief Dump MLD message for debugging purpose
 * @param[in] message Pointer to the MLD message to dump
 * @param[in] length Length of the MLD message, in bytes
 **/

void mldDumpMessage(const MldMessage *message, size_t length)
{
   uint_t maxRespDelay;
   const char_t *name;

   //Malformed message?
   if(length < sizeof(MldMessage))
      return;

   //Convert the Type field to string representation
   name = mldGetParamName(message->type, mldMessageTypeList,
      arraysize(mldMessageTypeList));

   //Dump Type field
   TRACE_DEBUG("  Type = %" PRIu8 " (%s)\r\n", message->type, name);

   //Check message type
   if(message->type == ICMPV6_TYPE_MCAST_LISTENER_QUERY &&
      length == sizeof(MldMessage))
   {
      //Dump Code field
      TRACE_DEBUG("  Code = %" PRIu8 "\r\n", message->code);

      //The Maximum Response Delay field is meaningful only in Query messages,
      //and specifies the maximum allowed delay before sending a responding
      //report, in units of milliseconds
      maxRespDelay = ntohs(message->maxRespDelay);

      //Dump Maximum Response Delay field
      TRACE_DEBUG("  Maximum Response Delay = %" PRIu8 " (%u.%03us)\r\n",
         maxRespDelay, maxRespDelay / 1000, maxRespDelay % 1000);

      //Dump Checksum field
      TRACE_DEBUG("  Checksum = 0x%04" PRIX16 "\r\n", ntohs(message->checksum));

      //Dump Multicast Address field
      TRACE_DEBUG("  Multicast Address = %s\r\n",
         ipv6AddrToString(&message->multicastAddr, NULL));
   }
   else if(message->type == ICMPV6_TYPE_MCAST_LISTENER_QUERY &&
      length >= sizeof(MldListenerQueryV2))
   {
      //Dump Version 2 Multicast Listener Query message
      mldDumpQueryV2((MldListenerQueryV2 *) message, length);
   }
   else if(message->type == ICMPV6_TYPE_MCAST_LISTENER_REPORT_V2)
   {
      //Dump Version 2 Multicast Listener Report message
      mldDumpReportV2((MldListenerReportV2 *) message, length);
   }
   else
   {
      //Dump Code field
      TRACE_DEBUG("  Code = %" PRIu8 "\r\n", message->code);

      //Dump Checksum field
      TRACE_DEBUG("  Checksum = 0x%04" PRIX16 "\r\n", ntohs(message->checksum));

      //Dump Multicast Address field
      TRACE_DEBUG("  Multicast Address = %s\r\n",
         ipv6AddrToString(&message->multicastAddr, NULL));
   }
}


/**
 * @brief Dump MLDv2 Query message for debugging purpose
 * @param[in] message Pointer to the MLD message to dump
 * @param[in] length Length of the MLD message, in bytes
 **/

void mldDumpQueryV2(const MldListenerQueryV2 *message, size_t length)
{
   uint_t i;
   uint_t n;
   uint_t qqic;
   uint16_t maxRespCode;
   uint_t maxRespDelay;

   //Malformed message?
   if(length < sizeof(MldListenerQueryV2))
      return;

   //Dump Code field
   TRACE_DEBUG("  Code = %" PRIu8 "\r\n", message->code);

   //The Maximum Response Code field specifies the maximum time allowed before
   //sending a responding report
   maxRespCode = ntohs(message->maxRespCode);

   //The Maximum Response Delay is is derived from the Maximum Response Code as
   //follows
   if(maxRespCode < 32768)
   {
      //The time is represented in units of milliseconds
      maxRespDelay = maxRespCode;
   }
   else
   {
      //Maximum Response Code represents a floating-point value
      maxRespDelay = mldDecodeFloatingPointValue16(maxRespCode);
   }

   //Dump Maximum Response Code field
   TRACE_DEBUG("  Maximum Response Code = %" PRIu16 " (%u.%03us)\r\n",
      message->maxRespCode, maxRespDelay / 1000, maxRespDelay % 1000);

   //Dump Checksum field
   TRACE_DEBUG("  Checksum = 0x%04" PRIX16 "\r\n", ntohs(message->checksum));

   //Dump Multicast Address field
   TRACE_DEBUG("  Multicast Address = %s\r\n",
      ipv6AddrToString(&message->multicastAddr, NULL));

   //Dump Flags field
   TRACE_DEBUG("  Flags = 0x%" PRIX8 "\r\n", message->flags);
   //Dump S field
   TRACE_DEBUG("  S = %" PRIu8 "\r\n", message->s);
   //Dump QRV field
   TRACE_DEBUG("  QRV = %" PRIu8 "\r\n", message->qrv);

   //The Querier's Query Interval Code field specifies the [Query Interval]
   //used by the querier
   if(message->qqic < 128)
   {
      //The time is represented in units of seconds
      qqic = message->qqic;
   }
   else
   {
      //Max Resp Code represents a floating-point value
      qqic = mldDecodeFloatingPointValue8(message->maxRespCode);
   }

   //Dump QQIC field
   TRACE_DEBUG("  QQIC = %" PRIu8 " (%us)\r\n", message->qqic, qqic);

   //The Number of Sources field specifies how many source addresses are
   //present in the Query
   n = ntohs(message->numOfSources);

   //Malformed message?
   if(length < (sizeof(MldListenerQueryV2) + n * sizeof(Ipv6Addr)))
      return;

   //Dump Number of Sources field
   TRACE_DEBUG("  Number of Sources = %u\r\n", n);

   //Dump Source Address field
   for(i = 0; i < n; i++)
   {
      TRACE_DEBUG("  Source Address %u = %s\r\n", i + 1,
         ipv6AddrToString(&message->srcAddr[i], NULL));
   }
}


/**
 * @brief Dump MLDv2 Report message for debugging purpose
 * @param[in] message Pointer to the MLD message to dump
 * @param[in] length Length of the MLD message, in bytes
 **/

void mldDumpReportV2(const MldListenerReportV2 *message, size_t length)
{
   size_t i;
   size_t n;
   uint_t k;
   uint_t numRecords;
   const MldMcastAddrRecord *record;

   //Malformed message?
   if(length < sizeof(MldListenerReportV2))
      return;

   //Get the length occupied by the multicast address records
   length -= sizeof(MldListenerReportV2);

   //Dump Checksum field
   TRACE_DEBUG("  Checksum = 0x%04" PRIX16 "\r\n", ntohs(message->checksum));
   //Dump Flags field
   TRACE_DEBUG("  Flags = 0x%04" PRIX16 "\r\n", ntohs(message->flags));

   //The Nr of Mcast Address Records field specifies how many Group Records are
   //present in this Report
   numRecords = ntohs(message->numOfMcastAddrRecords);

   //Dump Nr of Mcast Address Records field
   TRACE_DEBUG("  Nr of Mcast Address Records = %u\r\n", numRecords);

   //Loop through the multicast address records
   for(i = 0, k = 0; i < length && k < numRecords; i += n, k++)
   {
      //Malformed message?
      if((i + sizeof(MldMcastAddrRecord)) > length)
         break;

      //Point to the current multicast address record
      record = (MldMcastAddrRecord *) (message->mcastAddrRecords + i);

      //Determine the length of the multicast address record
      n = sizeof(MldMcastAddrRecord) + record->auxDataLen +
         ntohs(record->numOfSources) * sizeof(Ipv6Addr);

      //Malformed message?
      if((i + n) > length)
         break;

      //Debug message
      TRACE_DEBUG("  Multicast Address Record %u\r\n", k + 1);

      //Dump current multicast address record
      mldDumpMulticastAddrRecord(record, n);
   }
}


/**
 * @brief Dump multicast address record for debugging purpose
 * @param[in] record Pointer to the multicast address record to dump
 * @param[in] length Length of the multicast address record, in bytes
 **/

void mldDumpMulticastAddrRecord(const MldMcastAddrRecord *record,
   size_t length)
{
   uint_t i;
   uint_t n;
   const char_t *name;

   //Malformed multicast address record?
   if(length < sizeof(MldMcastAddrRecord))
      return;

   //Convert the Record Type field to string representation
   name = mldGetParamName(record->recordType, mldMulticastAddrRecordTypeList,
      arraysize(mldMulticastAddrRecordTypeList));

   //Dump Record Type field
   TRACE_DEBUG("    Record Type = 0x%02" PRIX8 " (%s)\r\n", record->recordType,
      name);

   //Dump Aux Data Len field
   TRACE_DEBUG("    Aux Data Len = %" PRIu8 "\r\n", record->auxDataLen);

   //The Number of Sources field specifies how many source addresses are
   //present in this Group Record
   n = htons(record->numOfSources);

   //Malformed group record?
   if(length < (sizeof(MldMcastAddrRecord) + n * sizeof(Ipv6Addr)))
      return;

   //Dump Number of Sources field
   TRACE_DEBUG("    Number of Sources = %u\r\n", n);

   //Dump Multicast Address field
   TRACE_DEBUG("    Multicast Address = %s\r\n",
      ipv6AddrToString(&record->multicastAddr, NULL));

   //Dump Source Address field
   for(i = 0; i < n; i++)
   {
      TRACE_DEBUG("    Source Address %u = %s\r\n", i + 1,
         ipv6AddrToString(&record->srcAddr[i], NULL));
   }
}


/**
 * @brief Convert a parameter to string representation
 * @param[in] value Parameter value
 * @param[in] paramList List of acceptable parameters
 * @param[in] paramListLen Number of entries in the list
 * @return NULL-terminated string describing the parameter
 **/

const char_t *mldGetParamName(uint_t value, const MldParamName *paramList,
   size_t paramListLen)
{
   uint_t i;

   //Default name for unknown values
   static const char_t defaultName[] = "Unknown";

   //Loop through the list of acceptable parameters
   for(i = 0; i < paramListLen; i++)
   {
      if(paramList[i].value == value)
         return paramList[i].name;
   }

   //Unknown value
   return defaultName;
}

#endif
