/**
 * @file igmp_debug.c
 * @brief Data logging functions for debugging purpose (IGMP)
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
#define TRACE_LEVEL IGMP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "igmp/igmp_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED && IGMP_TRACE_LEVEL >= TRACE_LEVEL_DEBUG)

//IGMP message types
const IgmpParamName igmpMessageTypeList[] =
{
   {IGMP_TYPE_MEMBERSHIP_QUERY,     "Membership Query"},
   {IGMP_TYPE_MEMBERSHIP_REPORT_V1, "Version 1 Membership Report"},
   {IGMP_TYPE_MEMBERSHIP_REPORT_V2, "Version 2 Membership Report"},
   {IGMP_TYPE_LEAVE_GROUP,          "Leave Group"},
   {IGMP_TYPE_MEMBERSHIP_REPORT_V3, "Version 3 Membership Report"}
};

//IGMPv3 group record types
const IgmpParamName igmpGroupRecordTypeList[] =
{
   {IGMP_GROUP_RECORD_TYPE_IS_IN, "MODE_IS_INCLUDE"},
   {IGMP_GROUP_RECORD_TYPE_IS_EX, "MODE_IS_EXCLUDE"},
   {IGMP_GROUP_RECORD_TYPE_TO_IN, "CHANGE_TO_INCLUDE_MODE"},
   {IGMP_GROUP_RECORD_TYPE_TO_EX, "CHANGE_TO_EXCLUDE_MODE"},
   {IGMP_GROUP_RECORD_TYPE_ALLOW, "ALLOW_NEW_SOURCES"},
   {IGMP_GROUP_RECORD_TYPE_BLOCK, "BLOCK_OLD_SOURCES"}
};


/**
 * @brief Dump IGMP message for debugging purpose
 * @param[in] message Pointer to the IGMP message to dump
 * @param[in] length Length of the IGMP message, in bytes
 **/

void igmpDumpMessage(const IgmpMessage *message, size_t length)
{
   uint_t maxRespTime;
   const char_t *name;

   //Malformed message?
   if(length < sizeof(IgmpMessage))
      return;

   //Convert the Type field to string representation
   name = igmpGetParamName(message->type, igmpMessageTypeList,
      arraysize(igmpMessageTypeList));

   //Dump Type field
   TRACE_DEBUG("  Type = 0x%02" PRIX8 " (%s)\r\n", message->type, name);

   //Check message type
   if(message->type == IGMP_TYPE_MEMBERSHIP_QUERY &&
      length == sizeof(IgmpMessage))
   {
      //The Max Response Time field is meaningful only in Membership Query
      //messages, and specifies the maximum allowed time before sending a
      //responding report in units of 1/10 second
      maxRespTime = message->maxRespTime;

      //Dump Max Response Time field
      TRACE_DEBUG("  Max Resp Time = %" PRIu8 " (%u.%us)\r\n",
         maxRespTime, maxRespTime / 10, maxRespTime % 10);

      //Dump Checksum field
      TRACE_DEBUG("  Checksum = 0x%04" PRIX16 "\r\n", ntohs(message->checksum));

      //Dump Group Address field
      TRACE_DEBUG("  Group Address = %s\r\n",
         ipv4AddrToString(message->groupAddr, NULL));
   }
   else if(message->type == IGMP_TYPE_MEMBERSHIP_QUERY &&
      length >= sizeof(IgmpMembershipQueryV3))
   {
      //Dump Version 3 Membership Query message
      igmpDumpMembershipQueryV3((IgmpMembershipQueryV3 *) message, length);
   }
   else if(message->type == IGMP_TYPE_MEMBERSHIP_REPORT_V3)
   {
      //Dump Version 3 Membership Report message
      igmpDumpMessageMembershipReportV3((IgmpMembershipReportV3 *) message,
         length);
   }
   else
   {
      //Dump Checksum field
      TRACE_DEBUG("  Checksum = 0x%04" PRIX16 "\r\n", ntohs(message->checksum));

      //Dump Group Address field
      TRACE_DEBUG("  Group Address = %s\r\n",
         ipv4AddrToString(message->groupAddr, NULL));
   }
}


/**
 * @brief Dump Version 3 Membership Query message
 * @param[in] message Pointer to the IGMP message to dump
 * @param[in] length Length of the IGMP message, in bytes
 **/

void igmpDumpMembershipQueryV3(const IgmpMembershipQueryV3 *message,
   size_t length)
{
   uint_t i;
   uint_t n;
   uint_t qqic;
   uint_t maxRespTime;

   //Malformed message?
   if(length < sizeof(IgmpMembershipQueryV3))
      return;

   //The Max Resp Code field specifies the maximum time allowed before
   //sending a responding report
   if(message->maxRespCode < 128)
   {
      //The time is represented in units of 1/10 second
      maxRespTime = message->maxRespCode;
   }
   else
   {
      //Max Resp Code represents a floating-point value
      maxRespTime = igmpDecodeFloatingPointValue(message->maxRespCode);
   }

   //Dump Max Response Code field
   TRACE_DEBUG("  Max Resp Code = %" PRIu8 " (%u.%us)\r\n",
      message->maxRespCode, maxRespTime / 10, maxRespTime % 10);

   //Dump Checksum field
   TRACE_DEBUG("  Checksum = 0x%04" PRIX16 "\r\n", ntohs(message->checksum));

   //Dump Group Address field
   TRACE_DEBUG("  Group Address = %s\r\n",
      ipv4AddrToString(message->groupAddr, NULL));

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
      qqic = igmpDecodeFloatingPointValue(message->maxRespCode);
   }

   //Dump QQIC field
   TRACE_DEBUG("  QQIC = %" PRIu8 " (%us)\r\n", message->qqic, qqic);

   //The Number of Sources field specifies how many source addresses are
   //present in the Query
   n = ntohs(message->numOfSources);

   //Malformed message?
   if(length < (sizeof(IgmpMembershipQueryV3) + n * sizeof(Ipv4Addr)))
      return;

   //Dump Number of Sources field
   TRACE_DEBUG("  Number of Sources = %u\r\n", n);

   //Dump Source Address field
   for(i = 0; i < n; i++)
   {
      TRACE_DEBUG("  Source Address %u = %s\r\n", i + 1,
         ipv4AddrToString(message->srcAddr[i], NULL));
   }
}


/**
 * @brief Dump Version 3 Membership Report message
 * @param[in] message Pointer to the IGMP message to dump
 * @param[in] length Length of the IGMP message, in bytes
 **/

void igmpDumpMessageMembershipReportV3(const IgmpMembershipReportV3 *message,
   size_t length)
{
   size_t i;
   size_t n;
   uint_t k;
   uint_t numRecords;
   const IgmpGroupRecord *record;

   //Malformed message?
   if(length < sizeof(IgmpMembershipReportV3))
      return;

   //Get the length occupied by the group records
   length -= sizeof(IgmpMembershipReportV3);

   //Dump Checksum field
   TRACE_DEBUG("  Checksum = 0x%04" PRIX16 "\r\n", ntohs(message->checksum));
   //Dump Flags field
   TRACE_DEBUG("  Flags = 0x%04" PRIX16 "\r\n", ntohs(message->flags));

   //The Number of Group Records field specifies how many Group Records are
   //present in this Report
   numRecords = ntohs(message->numOfGroupRecords);

   //Dump Number of Sources field
   TRACE_DEBUG("  Number of Group Records = %u\r\n", numRecords);

   //Loop through the group records
   for(i = 0, k = 0; i < length && k < numRecords; i += n, k++)
   {
      //Malformed message?
      if((i + sizeof(IgmpGroupRecord)) > length)
         break;

      //Point to the current group record
      record = (IgmpGroupRecord *) (message->groupRecords + i);

      //Determine the length of the group record
      n = sizeof(IgmpGroupRecord) + record->auxDataLen +
         ntohs(record->numOfSources) * sizeof(Ipv4Addr);

      //Malformed message?
      if((i + n) > length)
         break;

      //Debug message
      TRACE_DEBUG("  Group Record %u\r\n", k + 1);

      //Dump current group record
      igmpDumpGroupRecord(record, n);
   }
}


/**
 * @brief Dump group record for debugging purpose
 * @param[in] record Pointer to the group record to dump
 * @param[in] length Length of the group record, in bytes
 **/

void igmpDumpGroupRecord(const IgmpGroupRecord *record,
   size_t length)
{
   uint_t i;
   uint_t n;
   const char_t *name;

   //Malformed group record?
   if(length < sizeof(IgmpGroupRecord))
      return;

   //Convert the Record Type field to string representation
   name = igmpGetParamName(record->recordType, igmpGroupRecordTypeList,
      arraysize(igmpGroupRecordTypeList));

   //Dump Record Type field
   TRACE_DEBUG("    Record Type = 0x%02" PRIX8 " (%s)\r\n", record->recordType,
      name);

   //Dump Aux Data Len field
   TRACE_DEBUG("    Aux Data Len = %" PRIu8 "\r\n", record->auxDataLen);

   //The Number of Sources field specifies how many source addresses are
   //present in this Group Record
   n = htons(record->numOfSources);

   //Malformed group record?
   if(length < (sizeof(IgmpGroupRecord) + n * sizeof(Ipv4Addr)))
      return;

   //Dump Number of Sources field
   TRACE_DEBUG("    Number of Sources = %u\r\n", n);

   //Dump Multicast Address field
   TRACE_DEBUG("    Multicast Address = %s\r\n",
      ipv4AddrToString(record->multicastAddr, NULL));

   //Dump Source Address field
   for(i = 0; i < n; i++)
   {
      TRACE_DEBUG("    Source Address %u = %s\r\n", i + 1,
         ipv4AddrToString(record->srcAddr[i], NULL));
   }
}


/**
 * @brief Convert a parameter to string representation
 * @param[in] value Parameter value
 * @param[in] paramList List of acceptable parameters
 * @param[in] paramListLen Number of entries in the list
 * @return NULL-terminated string describing the parameter
 **/

const char_t *igmpGetParamName(uint_t value, const IgmpParamName *paramList,
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
