/**
 * @file igmp_host_misc.c
 * @brief Helper functions for IGMP host
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
#include "ipv4/ipv4.h"
#include "ipv4/ipv4_multicast.h"
#include "ipv4/ipv4_misc.h"
#include "igmp/igmp_host.h"
#include "igmp/igmp_host_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED && IGMP_HOST_SUPPORT == ENABLED)


/**
 * @brief Change host compatibility mode
 * @param[in] context Pointer to the IGMP host context
 * @param[in] compatibilityMode New host compatibility mode
 **/

void igmpHostChangeCompatibilityMode(IgmpHostContext *context,
   IgmpVersion compatibilityMode)
{
   uint_t i;
   IgmpHostGroup *group;

   //Debug message
   TRACE_DEBUG("Changing host compatibility mode to IGMPv%u...\r\n",
      (uint_t) compatibilityMode);

   //Switch compatibility mode immediately
   context->compatibilityMode = compatibilityMode;

   //Whenever a host changes its compatibility mode, it cancels all its pending
   //response and retransmission timers (refer to RFC 3376, section 7.2.1)
   netStopTimer(&context->generalQueryTimer);
   netStopTimer(&context->stateChangeReportTimer);

   //Loop through multicast groups
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Valid group?
      if(group->state != IGMP_HOST_GROUP_STATE_NON_MEMBER)
      {
         //Reset retransmission counter
         group->retransmitCount = 0;

#if (IPV4_MAX_MULTICAST_SOURCES > 0)
         //Clear source lists
         group->allow.numSources = 0;
         group->block.numSources = 0;
         group->queriedSources.numSources = 0;
#endif
         //Cancel the pending response, if any
         netStopTimer(&group->timer);

         //Switch to the Idle Member state
         group->state = IGMP_HOST_GROUP_STATE_IDLE_MEMBER;
      }
   }

   //Delete groups in "non-existent" state
   igmpHostFlushUnusedGroups(context);
}


/**
 * @brief Send Membership Report message
 * @param[in] context Pointer to the IGMP host context
 * @param[in] groupAddr IPv4 address specifying the group address
 **/

void igmpHostSendMembershipReport(IgmpHostContext *context, Ipv4Addr groupAddr)
{
   size_t offset;
   NetBuffer *buffer;
   IgmpMessage *message;

   //Allocate a memory buffer to hold the IGMP message
   buffer = ipAllocBuffer(sizeof(IgmpMessage), &offset);
   //Failed to allocate memory?
   if(buffer == NULL)
      return;

   //Point to the beginning of the IGMP message
   message = netBufferAt(buffer, offset, 0);

   //The type of report is determined by the state of the interface
   if(context->compatibilityMode == IGMP_VERSION_1)
   {
      message->type = IGMP_TYPE_MEMBERSHIP_REPORT_V1;
   }
   else
   {
      message->type = IGMP_TYPE_MEMBERSHIP_REPORT_V2;
   }

   //Format the Membership Report message
   message->maxRespTime = 0;
   message->checksum = 0;
   message->groupAddr = groupAddr;

   //Message checksum calculation
   message->checksum = ipCalcChecksum(message, sizeof(IgmpMessage));

   //The Membership Report message is sent to the group being reported
   igmpSendMessage(context->interface, groupAddr, buffer, offset);

   //Free previously allocated memory
   netBufferFree(buffer);
}


/**
 * @brief Send Leave Group message
 * @param[in] context Pointer to the IGMP host context
 * @param[in] groupAddr IPv4 address specifying the group address being left
 **/

void igmpHostSendLeaveGroup(IgmpHostContext *context, Ipv4Addr groupAddr)
{
   size_t offset;
   NetBuffer *buffer;
   IgmpMessage *message;

   //If the interface state says the querier is running IGMPv1, this action
   //should be skipped
   if(context->compatibilityMode == IGMP_VERSION_1)
      return;

   //Allocate a memory buffer to hold the IGMP message
   buffer = ipAllocBuffer(sizeof(IgmpMessage), &offset);
   //Failed to allocate memory?
   if(buffer == NULL)
      return;

   //Point to the beginning of the IGMP message
   message = netBufferAt(buffer, offset, 0);

   //Format the Leave Group message
   message->type = IGMP_TYPE_LEAVE_GROUP;
   message->maxRespTime = 0;
   message->checksum = 0;
   message->groupAddr = groupAddr;

   //Message checksum calculation
   message->checksum = ipCalcChecksum(message, sizeof(IgmpMessage));

   //Leave Group messages are addressed to the all-routers group because other
   //group members have no need to know that a host has left the group, but it
   //does no harm to address the message to the group
   igmpSendMessage(context->interface, IGMP_ALL_ROUTERS_ADDR, buffer, offset);

   //Free previously allocated memory
   netBufferFree(buffer);
}


/**
 * @brief Send Current-State Report message
 * @param[in] context Pointer to the IGMP host context
 * @param[in] groupAddr IPv4 address specifying the group address
 **/

void igmpHostSendCurrentStateReport(IgmpHostContext *context,
   Ipv4Addr groupAddr)
{
   uint_t i;
   size_t n;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   IgmpHostGroup *group;
   IgmpMembershipReportV3 *message;
   IgmpGroupRecord *record;

   //Allocate a memory buffer to hold the IGMP message
   buffer = ipAllocBuffer(IGMP_MAX_MSG_SIZE, &offset);
   //Failed to allocate memory?
   if(buffer == NULL)
      return;

   //Point to the beginning of the IGMP message
   message = netBufferAt(buffer, offset, 0);

   //Format the Version 3 Membership Report message
   message->type = IGMP_TYPE_MEMBERSHIP_REPORT_V3;
   message->reserved = 0;
   message->checksum = 0;
   message->flags = 0;
   message->numOfGroupRecords = 0;

   //Total length of the message
   length = sizeof(IgmpMembershipReportV3);

   //Loop through multicast groups
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Matching group?
      if(igmpHostMatchGroup(group, groupAddr))
      {
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
         //Check whether the interface has reception state for that group
         //address
         if(group->filterMode == IP_FILTER_MODE_EXCLUDE ||
            group->filter.numSources > 0)
         {
            uint_t j;

            //General Query or Group-Specific Query?
            if(groupAddr == IPV4_UNSPECIFIED_ADDR ||
               group->queriedSources.numSources == 0)
            {
               //Calculate the length of the group record
               n = sizeof(IgmpGroupRecord) + group->filter.numSources *
                  sizeof(Ipv4Addr);

               //If the set of Group Records required in a Report does not fit
               //within the size limit of a single Report message, the Group
               //Records are sent in as many Report messages as needed to report
               //the entire set (refer to RFC 3376, section 4.2.16)
               if((length + n) > IGMP_MAX_MSG_SIZE)
               {
                  //Send report message
                  igmpHostFlushReportRecords(context, buffer, offset, &length);
               }

               //Point to the buffer where to format the record
               record = (IgmpGroupRecord *) ((uint8_t *) message + length);

               //The Current-State Record carries the associated filter mode
               //(MODE_IS_INCLUDE or MODE_IS_EXCLUDE)
               if(group->filterMode == IP_FILTER_MODE_INCLUDE)
               {
                  record->recordType = IGMP_GROUP_RECORD_TYPE_IS_IN;
               }
               else
               {
                  record->recordType = IGMP_GROUP_RECORD_TYPE_IS_EX;
               }

               //Format group record
               record->auxDataLen = 0;
               record->numOfSources = htons(group->filter.numSources);
               record->multicastAddr = group->addr;

               //Format the list of source addresses
               for(j = 0; j < group->filter.numSources; j++)
               {
                  record->srcAddr[j] = group->filter.sources[j];
               }

               //Increment the number of group records
               message->numOfGroupRecords++;
               //Update the length of the message
               length += n;
            }
            else
            {
               //If the list of recorded sources B for that group is non-empty,
               //then the contents of the responding Current-State Record is
               //determined from the interface state and the pending response
               //record
               if(group->filterMode == IP_FILTER_MODE_INCLUDE)
               {
                  //If the interface state is INCLUDE (A), then the contents of
                  //the responding Current-State Record is IS_IN (A*B)
                  for(j = 0; j < group->queriedSources.numSources; )
                  {
                     if(ipv4FindSrcAddr(&group->filter,
                        group->queriedSources.sources[j]) >= 0)
                     {
                        j++;
                     }
                     else
                     {
                        ipv4RemoveSrcAddr(&group->queriedSources,
                           group->queriedSources.sources[j]);
                     }
                  }
               }
               else
               {
                  //If the interface state is EXCLUDE (A), then the contents of
                  //the responding Current-State Record is IS_IN (B-A)
                  for(j = 0; j < group->filter.numSources; j++)
                  {
                     ipv4RemoveSrcAddr(&group->queriedSources,
                        group->filter.sources[j]);
                  }
               }

               //If the resulting Current-State Record has an empty set of
               //source addresses, then no response is sent
               if(group->queriedSources.numSources > 0)
               {
                  //Calculate the length of the group record
                  n = sizeof(IgmpGroupRecord) +
                     group->queriedSources.numSources * sizeof(Ipv4Addr);

                  //If the set of Group Records required in a Report does not
                  //fit within the size limit of a single Report message, the
                  //Group Records are sent in as many Report messages as needed
                  //to report the entire set (refer to RFC 3376, section 4.2.16)
                  if((length + n) > IGMP_MAX_MSG_SIZE)
                  {
                     //Send report message
                     igmpHostFlushReportRecords(context, buffer, offset, &length);
                  }

                  //Point to the buffer where to format the record
                  record = (IgmpGroupRecord *) ((uint8_t *) message + length);

                  //Format group record
                  record->recordType = IGMP_GROUP_RECORD_TYPE_IS_IN;
                  record->auxDataLen = 0;
                  record->numOfSources = htons(group->queriedSources.numSources);
                  record->multicastAddr = group->addr;

                  //Format the list of source addresses
                  for(j = 0; j < group->queriedSources.numSources; j++)
                  {
                     record->srcAddr[j] = group->queriedSources.sources[j];
                  }

                  //Increment the number of group records
                  message->numOfGroupRecords++;
                  //Update the length of the message
                  length += n;
               }
            }
         }

         //Finally, after any required Report messages have been generated,
         //the source lists associated with any reported groups are cleared
         group->queriedSources.numSources = 0;
#else
         //Check whether the interface has reception state for that group
         //address
         if(group->filterMode == IP_FILTER_MODE_EXCLUDE)
         {
            //Calculate the length of the group record
            n = sizeof(IgmpGroupRecord);

            //If the set of Group Records required in a Report does not fit
            //within the size limit of a single Report message, the Group
            //Records are sent in as many Report messages as needed to report
            //the entire set (refer to RFC 3376, section 4.2.16)
            if((length + n) > IGMP_MAX_MSG_SIZE)
            {
               //Send report message
               igmpHostFlushReportRecords(context, buffer, offset, &length);
            }

            //Point to the buffer where to format the record
            record = (IgmpGroupRecord *) ((uint8_t *) message + length);

            //The Current-State Record carries the associated filter mode
            record->recordType = IGMP_GROUP_RECORD_TYPE_IS_EX;
            record->auxDataLen = 0;
            record->numOfSources = HTONS(0);
            record->multicastAddr = group->addr;

            //Increment the number of group records
            message->numOfGroupRecords++;
            //Update the length of the message
            length += n;
         }
#endif
      }
   }

   //Version 3 Reports are sent with an IP destination address of 224.0.0.22,
   //to which all IGMPv3-capable multicast routers listen (refer to RFC 3376,
   //section 4.2.14)
   igmpHostFlushReportRecords(context, buffer, offset, &length);

   //Free previously allocated memory
   netBufferFree(buffer);
}


/**
 * @brief Send State-Change Report message
 * @param[in] context Pointer to the IGMP host context
 **/

void igmpHostSendStateChangeReport(IgmpHostContext *context)
{
   uint_t i;
   size_t n;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   IgmpHostGroup *group;
   IgmpMembershipReportV3 *message;
   IgmpGroupRecord *record;
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   uint_t j;
#endif

   //Allocate a memory buffer to hold the IGMP message
   buffer = ipAllocBuffer(IGMP_MAX_MSG_SIZE, &offset);
   //Failed to allocate memory?
   if(buffer == NULL)
      return;

   //Point to the beginning of the IGMP message
   message = netBufferAt(buffer, offset, 0);

   //Format the Version 3 Membership Report message
   message->type = IGMP_TYPE_MEMBERSHIP_REPORT_V3;
   message->reserved = 0;
   message->checksum = 0;
   message->flags = 0;
   message->numOfGroupRecords = 0;

   //Total length of the message
   length = sizeof(IgmpMembershipReportV3);

   //Loop through multicast groups
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Valid group?
      if(group->state != IGMP_HOST_GROUP_STATE_NON_MEMBER)
      {
         //The report should contain a Filter Mode Change Record if the Filter
         //Mode Retransmission Counter has a value higher than zero
         if(group->retransmitCount > 0)
         {
            //Calculate the length of the group record
            n = sizeof(IgmpGroupRecord) + group->filter.numSources *
               sizeof(Ipv4Addr);

            //If the set of Group Records required in a Report does not fit
            //within the size limit of a single Report message, the Group
            //Records are sent in as many Report messages as needed to report
            //the entire set (refer to RFC 3376, section 4.2.16)
            if((length + n) > IGMP_MAX_MSG_SIZE)
            {
               //Send report message
               igmpHostFlushReportRecords(context, buffer, offset, &length);
            }

            //Point to the buffer where to format the record
            record = (IgmpGroupRecord *) ((uint8_t *) message + length);

            //If the current filter-mode of the interface is INCLUDE, a TO_IN
            //record is included in the report, otherwise a TO_EX record is
            //included
            if(group->filterMode == IP_FILTER_MODE_INCLUDE)
            {
               record->recordType = IGMP_GROUP_RECORD_TYPE_TO_IN;
            }
            else
            {
               record->recordType = IGMP_GROUP_RECORD_TYPE_TO_EX;
            }

            //Format group record
            record->auxDataLen = 0;
            record->numOfSources = htons(group->filter.numSources);
            record->multicastAddr = group->addr;

#if (IPV4_MAX_MULTICAST_SOURCES > 0)
            //Format the list of source addresses
            for(j = 0; j < group->filter.numSources; j++)
            {
               record->srcAddr[j] = group->filter.sources[j];
            }
#endif
            //Increment the number of group records
            message->numOfGroupRecords++;
            //Update the length of the message
            length += n;
         }
         else
         {
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
            //If the computed source list for a ALLOW record is empty, that
            //record is omitted from the State-Change report
            if(group->allow.numSources > 0)
            {
               //Calculate the length of the ALLOW record
               n = sizeof(IgmpGroupRecord) + group->allow.numSources *
                  sizeof(Ipv4Addr);

               //If the set of Group Records required in a Report does not fit
               //within the size limit of a single Report message, the Group
               //Records are sent in as many Report messages as needed to report
               //the entire set (refer to RFC 3376, section 4.2.16)
               if((length + n) > IGMP_MAX_MSG_SIZE)
               {
                  igmpHostFlushReportRecords(context, buffer, offset, &length);
               }

               //Point to the buffer where to format the record
               record = (IgmpGroupRecord *) ((uint8_t *) message + length);

               //The ALLOW record contains the list of the additional sources
               //that the system wishes to hear from
               record->recordType = IGMP_GROUP_RECORD_TYPE_ALLOW;
               record->auxDataLen = 0;
               record->numOfSources = htons(group->allow.numSources);
               record->multicastAddr = group->addr;

               //Format the list of source addresses
               for(j = 0; j < group->allow.numSources; j++)
               {
                  record->srcAddr[j] = group->allow.sources[j].addr;
               }

               //Increment the number of group records
               message->numOfGroupRecords++;
               //Update the length of the message
               length += n;
            }

            //If the computed source list for a BLOCK record is empty, that
            //record is omitted from the State-Change report
            if(group->block.numSources > 0)
            {
               //Calculate the length of the BLOCK record
               n = sizeof(IgmpGroupRecord) + group->block.numSources *
                  sizeof(Ipv4Addr);

               //If the set of Group Records required in a Report does not fit
               //within the size limit of a single Report message, the Group
               //Records are sent in as many Report messages as needed to report
               //the entire set (refer to RFC 3376, section 4.2.16)
               if((length + n) > IGMP_MAX_MSG_SIZE)
               {
                  igmpHostFlushReportRecords(context, buffer, offset, &length);
               }

               //Point to the buffer where to format the record
               record = (IgmpGroupRecord *) ((uint8_t *) message + length);

               //The BLOCK record contains the list of the sources that the
               //system no longer wishes to hear from
               record->recordType = IGMP_GROUP_RECORD_TYPE_BLOCK;
               record->auxDataLen = 0;
               record->numOfSources = htons(group->block.numSources);
               record->multicastAddr = group->addr;

               //Format the list of source addresses
               for(j = 0; j < group->block.numSources; j++)
               {
                  record->srcAddr[j] = group->block.sources[j].addr;
               }

               //Increment the number of group records
               message->numOfGroupRecords++;
               //Update the length of the message
               length += n;
            }
#endif
         }

         //Retransmission state needs to be maintained until [Robustness
         //Variable] State-Change reports have been sent by the host
         igmpHostDecGroupRetransmitCounters(group);
      }
   }

   //Version 3 Reports are sent with an IP destination address of 224.0.0.22,
   //to which all IGMPv3-capable multicast routers listen (refer to RFC 3376,
   //section 4.2.14)
   igmpHostFlushReportRecords(context, buffer, offset, &length);

   //Free previously allocated memory
   netBufferFree(buffer);
}


/**
 * @brief Flush report records
 * @param[in] context Pointer to the IGMP host context
 * @param[in] buffer Multi-part buffer containing the report message
 * @param[in] offset Offset to the first byte of the report message
 * @param[in,out] length Length of the report message, in bytes
 **/

void igmpHostFlushReportRecords(IgmpHostContext *context, NetBuffer *buffer,
   size_t offset, size_t *length)
{
   IgmpMembershipReportV3 *message;

   //Any group records included in the message?
   if(*length > 0)
   {
      //Point to the beginning of the report message
      message = netBufferAt(buffer, offset, 0);

      //The Number of Group Records field specifies how many Group Records are
      //present in this Report
      message->numOfGroupRecords = htons(message->numOfGroupRecords);

      //Message checksum calculation
      message->checksum = ipCalcChecksum(message, *length);

      //Adjust the length of the multi-part buffer
      netBufferSetLength(buffer, offset + *length);

      //Version 3 Reports are sent with an IP destination address of 224.0.0.22,
      //to which all IGMPv3-capable multicast routers listen (refer to RFC 3376,
      //section 4.2.14)
      igmpSendMessage(context->interface, IGMP_V3_ALL_ROUTERS_ADDR, buffer,
         offset);

      //Reset the Checksum field
      message->checksum = 0;
      //Reset the Number of Group Records field
      message->numOfGroupRecords = 0;

      //Update the length of the message
      *length = sizeof(IgmpMembershipReportV3);
   }
}


/**
 * @brief Process incoming IGMP message
 * @param[in] context Pointer to the IGMP host context
 * @param[in] pseudoHeader IPv4 pseudo header
 * @param[in] message Pointer to the incoming IGMP message
 * @param[in] length Length of the IGMP message, in bytes
 **/

void igmpHostProcessMessage(IgmpHostContext *context,
   const Ipv4PseudoHeader *pseudoHeader, const IgmpMessage *message,
   size_t length)
{
   //Check IGMP message type
   if(message->type == IGMP_TYPE_MEMBERSHIP_QUERY)
   {
      //The IGMP version of a Membership Query message is determined as follows
      if(length == sizeof(IgmpMessage))
      {
         //Process Membership Query message
         igmpHostProcessMembershipQuery(context, pseudoHeader, message, length);
      }
      else if(length >= sizeof(IgmpMembershipQueryV3))
      {
         //Process Version 3 Membership Query message
         igmpHostProcessMembershipQueryV3(context, pseudoHeader,
            (IgmpMembershipQueryV3 *) message, length);
      }
      else
      {
         //Query messages that do not match any of the above conditions must be
         //silently ignored (refer to RFC 3376, section 7.1)
      }
   }
   else if(message->type == IGMP_TYPE_MEMBERSHIP_REPORT_V1 ||
      message->type == IGMP_TYPE_MEMBERSHIP_REPORT_V2)
   {
      //Process Membership Report message
      igmpHostProcessMembershipReport(context, pseudoHeader, message, length);
   }
   else
   {
      //Discard other messages
   }
}


/**
 * @brief Process incoming Membership Query message
 * @param[in] context Pointer to the IGMP host context
 * @param[in] pseudoHeader IPv4 pseudo header
 * @param[in] message Pointer to the incoming IGMP message
 * @param[in] length Length of the IGMP message, in bytes
 **/

void igmpHostProcessMembershipQuery(IgmpHostContext *context,
   const Ipv4PseudoHeader *pseudoHeader, const IgmpMessage *message,
   size_t length)
{
   uint_t i;
   systime_t delay;
   systime_t maxRespTime;
   IgmpHostGroup *group;

   //The group address in the IGMP header must either be zero or a valid
   //multicast group address (refer to RFC 2236, section 6)
   if(message->groupAddr != IPV4_UNSPECIFIED_ADDR &&
      !ipv4IsMulticastAddr(message->groupAddr))
   {
      return;
   }

   //Hosts should ignore v2 or v3 General Queries sent to a multicast address
   //other than 224.0.0.1 (refer to RFC 3376, section 9.1)
   if(message->groupAddr == IPV4_UNSPECIFIED_ADDR &&
      pseudoHeader->destAddr != IGMP_ALL_SYSTEMS_ADDR)
   {
      return;
   }

   //When in IGMPv1 mode, routers send Periodic Queries with a Max Response
   //Time of 0
   if(message->maxRespTime == 0)
   {
      //IGMPv1 Querier Present timer is set to Older Version Querier Present
      //Timeout seconds whenever an IGMPv1 Membership Query is received
      netStartTimer(&context->igmpv1QuerierPresentTimer,
         IGMP_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT);

      //The Host Compatibility Mode of an interface changes whenever an older
      //version query (than the current compatibility mode) is heard
      if(context->compatibilityMode > IGMP_VERSION_1)
      {
         //The host should switch compatibility mode immediately
         igmpHostChangeCompatibilityMode(context, IGMP_VERSION_1);
      }

      //IGMPv1 routers send General Queries with the Max Response Time set to
      //0. This must be interpreted as a value of 100 (10 seconds)
      maxRespTime = IGMP_V1_MAX_RESPONSE_TIME;
   }
   else
   {
      //IGMPv2 Querier Present timer is set to Older Version Querier Present
      //Timeout seconds whenever an IGMPv2 Membership Query is received
      netStartTimer(&context->igmpv2QuerierPresentTimer,
         IGMP_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT);

      //The Host Compatibility Mode of an interface changes whenever an older
      //version query (than the current compatibility mode) is heard
      if(context->compatibilityMode > IGMP_VERSION_2)
      {
         //The host should switch compatibility mode immediately
         igmpHostChangeCompatibilityMode(context, IGMP_VERSION_2);
      }

      //The Max Resp Time field specifies the maximum time allowed before
      //sending a responding report (in units of 1/10 second)
      maxRespTime = message->maxRespTime * 100;
   }

   //Loop through multicast groups
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Matching group?
      if(igmpHostMatchGroup(group, message->groupAddr))
      {
         //Check group state
         if(group->state == IGMP_HOST_GROUP_STATE_DELAYING_MEMBER)
         {
            //If a timer for the group is already running, it is reset to
            //the random value only if the requested Max Response Time is
            //less than the remaining value of the running timer
            if(maxRespTime < netGetRemainingTime(&group->timer))
            {
               //Select a random value in the range 0 - Max Response Time
               delay = igmpGetRandomDelay(maxRespTime);
               //Restart delay timer
               netStartTimer(&group->timer, delay);
            }
         }
         else if(group->state == IGMP_HOST_GROUP_STATE_IDLE_MEMBER)
         {
            //Select a random value in the range 0 - Max Response Time
            delay = igmpGetRandomDelay(maxRespTime);
            //Start delay timer
            netStartTimer(&group->timer, delay);

            //Switch to the Delaying Member state
            group->state = IGMP_HOST_GROUP_STATE_DELAYING_MEMBER;
         }
         else
         {
            //Just for sanity
         }
      }
   }
}


/**
 * @brief Process incoming Version 3 Membership Query message
 * @param[in] context Pointer to the IGMP host context
 * @param[in] pseudoHeader IPv4 pseudo header
 * @param[in] message Pointer to the incoming IGMP message
 * @param[in] length Length of the IGMP message, in bytes
 **/

void igmpHostProcessMembershipQueryV3(IgmpHostContext *context,
   const Ipv4PseudoHeader *pseudoHeader, const IgmpMembershipQueryV3 *message,
   size_t length)
{
   uint_t i;
   uint_t n;
   systime_t delay;
   systime_t maxRespTime;
   IgmpHostGroup *group;

   //When Host Compatibility Mode is IGMPv2, a host acts in IGMPv2
   //compatibility mode, using only the IGMPv2 protocol, on that interface
   if(context->compatibilityMode < IGMP_VERSION_3)
      return;

   //Check the length of the Version 3 Membership Query message
   if(length < sizeof(IgmpMembershipQueryV3))
      return;

   //The Group Address field is set to zero when sending a General Query,
   //and set to the IP multicast address being queried when sending a
   //Group-Specific Query or Group-and-Source-Specific Query (refer to
   //RFC 3376, section 4.1.3)
   if(message->groupAddr != IPV4_UNSPECIFIED_ADDR &&
      !ipv4IsMulticastAddr(message->groupAddr))
   {
      return;
   }

   //Hosts should ignore v3 General Queries sent to a multicast address other
   //than 224.0.0.1 (refer to RFC 3376, section 9.1)
   if(message->groupAddr == IPV4_UNSPECIFIED_ADDR &&
      pseudoHeader->destAddr != IGMP_ALL_SYSTEMS_ADDR)
   {
      return;
   }

   //The Number of Sources field specifies how many source addresses are
   //present in the Query
   n = ntohs(message->numOfSources);

   //Malformed message?
   if(length < (sizeof(IgmpMembershipQueryV3) + n * sizeof(Ipv4Addr)))
      return;

   //The Max Resp Code field specifies the maximum time allowed before sending
   //a responding report
   if(message->maxRespCode < 128)
   {
      //The time is represented in units of 1/10 second
      maxRespTime = message->maxRespCode * 100;
   }
   else
   {
      //Max Resp Code represents a floating-point value
      maxRespTime = igmpDecodeFloatingPointValue(message->maxRespCode) * 100;
   }

   //When a new Query arrives on an interface, provided the system has state
   //to report, a delay for a response is randomly selected in the range
   //0 - Max Resp Time (refer to RFC 3376, section 5.2)
   delay = igmpGetRandomDelay(maxRespTime);

   //The following rules are then used to determine if a Report needs to be
   //scheduled and the type of Report to schedule
   if(netTimerRunning(&context->generalQueryTimer) &&
      netGetRemainingTime(&context->generalQueryTimer) < delay)
   {
      //If there is a pending response to a previous General Query scheduled
      //sooner than the selected delay, no additional response needs to be
      //scheduled
   }
   else if(message->groupAddr == IPV4_UNSPECIFIED_ADDR)
   {
      //If the received Query is a General Query, the interface timer is used
      //to schedule a response to the General Query after the selected delay.
      //Any previously pending response to a General Query is canceled
      netStartTimer(&context->generalQueryTimer, delay);
   }
   else
   {
      //Loop through multicast groups
      for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
      {
         //Point to the current group
         group = &context->groups[i];

         //Matching group?
         if(igmpHostMatchGroup(group, message->groupAddr))
         {
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
            error_t error;
            uint_t j;

            //Initialize status code
            error = NO_ERROR;

            //No pending response to a previous Query for this group?
            if(!netTimerRunning(&group->timer))
            {
               //If the received Query is a Group-and-Source-Specific Query, the
               //list of queried sources is recorded to be used when generating
               //a response
               group->queriedSources.numSources = 0;

               //Save the list of queried sources
               for(j = 0; j < n; j++)
               {
                  error = ipv4AddSrcAddr(&group->queriedSources,
                     message->srcAddr[j]);
               }

               //The implementation limits the number of source addresses that
               //can be recorded
               if(error)
               {
                  group->queriedSources.numSources = 0;
               }

               //If the received Query is a Group-Specific Query or a Group-and-
               //Source-Specific Query and there is no pending response to a
               //previous Query for this group, then the group timer is used to
               //schedule a report
               netStartTimer(&group->timer, delay);
            }
            else
            {
               //Check whether the new Query is a Group-Specific Query or the
               //recorded source-list associated with the group is empty
               if(n == 0 || group->queriedSources.numSources == 0)
               {
                  //If there already is a pending response to a previous Query
                  //scheduled for this group, and either the new Query is a
                  //Group-Specific Query or the recorded source-list associated
                  //with the group is empty, then the group source-list is
                  //cleared and a single response is scheduled using
                  //the group timer
                  group->queriedSources.numSources = 0;
               }
               else
               {
                  //If the received Query is a Group-and-Source-Specific Query
                  //and there is a pending response for this group with a non-
                  //empty source-list, then the group source list is augmented
                  //to contain the list of sources in the new Query and a single
                  //response is scheduled using the group timer
                  for(j = 0; j < n; j++)
                  {
                     error = ipv4AddSrcAddr(&group->queriedSources,
                        message->srcAddr[j]);
                  }

                  //The implementation limits the number of source addresses
                  //that can be recorded
                  if(error)
                  {
                     group->queriedSources.numSources = 0;
                  }
               }

               //The new response is scheduled to be sent at the earliest of the
               //remaining time for the pending report and the selected delay
               if(delay < netGetRemainingTime(&group->timer))
               {
                  netStartTimer(&group->timer, delay);
               }
            }
#else
            //No pending response to a previous Query for this group?
            if(!netTimerRunning(&group->timer))
            {
               //The group timer is used to schedule a report
               netStartTimer(&group->timer, delay);
            }
            else
            {
               //The new response is scheduled to be sent at the earliest of the
               //remaining time for the pending report and the selected delay
               if(delay < netGetRemainingTime(&group->timer))
               {
                  netStartTimer(&group->timer, delay);
               }
            }
#endif
         }
      }
   }
}


/**
 * @brief Process incoming Membership Report message
 * @param[in] context Pointer to the IGMP host context
 * @param[in] pseudoHeader IPv4 pseudo header
 * @param[in] message Pointer to the incoming IGMP message
 * @param[in] length Length of the IGMP message, in bytes
 **/

void igmpHostProcessMembershipReport(IgmpHostContext *context,
   const Ipv4PseudoHeader *pseudoHeader, const IgmpMessage *message,
   size_t length)
{
   IgmpHostGroup *group;

   //When Host Compatibility Mode is IGMPv3, a host acts using the IGMPv3
   //protocol on that interface
   if(context->compatibilityMode > IGMP_VERSION_2)
      return;

   //The group address in the IGMP header must be a valid multicast group
   //address
   if(!ipv4IsMulticastAddr(message->groupAddr))
      return;

   //Search the list of groups for the specified multicast address
   group = igmpHostFindGroup(context, message->groupAddr);

   //Any matching group found?
   if(group != NULL)
   {
      //Report messages are ignored for memberships in the Non-Member or Idle
      //Member state
      if(group->state == IGMP_HOST_GROUP_STATE_DELAYING_MEMBER)
      {
         //Clear flag
         group->flag = FALSE;
         //Switch to the Idle Member state
         group->state = IGMP_HOST_GROUP_STATE_IDLE_MEMBER;
      }
   }
}


/**
 * @brief Merge the difference report and the pending report
 * @param[in] group Pointer to the multicast group
 * @param[in] newFilterMode New filter mode for the affected group
 * @param[in] newFilter New interface state for the affected group
 **/

void igmpHostMergeReports(IgmpHostGroup *group,
   IpFilterMode newFilterMode, const Ipv4SrcAddrList *newFilter)
{
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   uint_t i;

   //The interface state for the affected group before and after the latest
   //change is compared
   if(newFilterMode == IP_FILTER_MODE_INCLUDE &&
      group->filterMode == IP_FILTER_MODE_EXCLUDE)
   {
      //The interface has changed to INCLUDE filter mode for the specified
      //multicast address
      group->allow.numSources = 0;
      group->block.numSources = 0;
   }
   else if(newFilterMode == IP_FILTER_MODE_EXCLUDE &&
      group->filterMode == IP_FILTER_MODE_INCLUDE)
   {
      //The interface has changed to EXCLUDE filter mode for the specified
      //multicast address
      group->allow.numSources = 0;
      group->block.numSources = 0;
   }
   else if(newFilterMode == IP_FILTER_MODE_INCLUDE &&
      group->filterMode == IP_FILTER_MODE_INCLUDE)
   {
      //The ALLOW record contains the list of the additional sources that the
      //system wishes to hear from
      for(i = 0; i < newFilter->numSources; i++)
      {
         if(ipv4FindSrcAddr(&group->filter, newFilter->sources[i]) < 0)
         {
            igmpHostAddSrcAddr(&group->allow, newFilter->sources[i]);
            igmpHostRemoveSrcAddr(&group->block, newFilter->sources[i]);
         }
      }

      //The BLOCK record contains the list of the sources that the system no
      //longer wishes to hear from
      for(i = 0; i < group->filter.numSources; i++)
      {
         if(ipv4FindSrcAddr(newFilter, group->filter.sources[i]) < 0)
         {
            igmpHostAddSrcAddr(&group->block, group->filter.sources[i]);
            igmpHostRemoveSrcAddr(&group->allow, group->filter.sources[i]);
         }
      }
   }
   else if(newFilterMode == IP_FILTER_MODE_EXCLUDE &&
      group->filterMode == IP_FILTER_MODE_EXCLUDE)
   {
      //The BLOCK record contains the list of the sources that the system no
      //longer wishes to hear from
      for(i = 0; i < newFilter->numSources; i++)
      {
         if(ipv4FindSrcAddr(&group->filter, newFilter->sources[i]) < 0)
         {
            igmpHostAddSrcAddr(&group->block, newFilter->sources[i]);
            igmpHostRemoveSrcAddr(&group->allow, newFilter->sources[i]);
         }
      }

      //The ALLOW record contains the list of the additional sources that the
      //system wishes to hear from
      for(i = 0; i < group->filter.numSources; i++)
      {
         if(ipv4FindSrcAddr(newFilter, group->filter.sources[i]) < 0)
         {
            igmpHostAddSrcAddr(&group->allow, group->filter.sources[i]);
            igmpHostRemoveSrcAddr(&group->block, group->filter.sources[i]);
         }
      }
   }
   else
   {
      //Just for sanity
   }
#endif

   //When the filter mode changes, the Filter Mode Retransmission Counter is
   //set to [Robustness Variable]
   if(newFilterMode != group->filterMode)
   {
      group->retransmitCount = IGMP_ROBUSTNESS_VARIABLE;
   }
}


/**
 * @brief Get the retransmission status of the State-Change report
 * @param[in] context Pointer to the IGMP host context
 * @return TRUE if additional retransmissions are needed, else FALSE
 **/

bool_t igmpHostGetRetransmitStatus(IgmpHostContext *context)
{
   uint_t i;
   bool_t status;
   IgmpHostGroup *group;

   //Clear status flag
   status = FALSE;

   //Loop through multicast groups
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Valid group?
      if(group->state != IGMP_HOST_GROUP_STATE_NON_MEMBER)
      {
         //Retransmission in progress?
         if(igmpHostGetGroupRetransmitStatus(group))
         {
            status = TRUE;
            break;
         }
      }
   }

   //Return TRUE if additional retransmissions are needed
   return status;
}


/**
 * @brief Get the retransmission status for a given group
 * @param[in] group Pointer to the multicast group
 * @return TRUE if additional retransmissions are needed, else FALSE
 **/

bool_t igmpHostGetGroupRetransmitStatus(IgmpHostGroup *group)
{
   bool_t status;

   //Clear status flag
   status = FALSE;

#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   //Retransmission in progress?
   if(group->retransmitCount > 0 || group->allow.numSources > 0 ||
      group->block.numSources > 0)
   {
      status = TRUE;
   }
#else
   //Retransmission in progress?
   if(group->retransmitCount > 0)
   {
      status = TRUE;
   }
#endif

   //Return TRUE if additional retransmissions are needed
   return status;
}


/**
 * @brief Decrement retransmission counters for a given group
 * @param[in] group Pointer to the multicast group
 **/

void igmpHostDecGroupRetransmitCounters(IgmpHostGroup *group)
{
   //Filter mode change?
   if(group->retransmitCount > 0)
   {
      //The Filter Mode Retransmission Counter is decremented by one unit after
      //the transmission of the report
      group->retransmitCount--;
   }
   else
   {
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
      int_t i;

      //The ALLOW record contains the list of the additional sources that the
      //system wishes to hear from
      for(i = group->allow.numSources - 1; i >= 0; i--)
      {
         //For each included source, its Source Retransmission Counter is
         //decreased with one unit after the transmission of the report
         if(group->allow.sources[i].retransmitCount > 0)
         {
            group->allow.sources[i].retransmitCount--;
         }

         //If the counter reaches zero, the source is deleted from the list
         if(group->allow.sources[i].retransmitCount == 0)
         {
            igmpHostRemoveSrcAddr(&group->allow, group->allow.sources[i].addr);
         }
      }

      //The BLOCK record contains the list of the sources that the system no
      //longer wishes to hear from
      for(i = group->block.numSources - 1; i >= 0; i--)
      {
         //For each included source, its Source Retransmission Counter is
         //decreased with one unit after the transmission of the report
         if(group->block.sources[i].retransmitCount > 0)
         {
            group->block.sources[i].retransmitCount--;
         }

         //If the counter reaches zero, the source is deleted from the list
         if(group->block.sources[i].retransmitCount == 0)
         {
            igmpHostRemoveSrcAddr(&group->block, group->block.sources[i].addr);
         }
      }
#endif
   }
}


/**
 * @brief Create a new multicast group
 * @param[in] context Pointer to the IGMP host context
 * @param[in] groupAddr Multicast group address
 * @return Pointer to the newly created multicast group
 **/

IgmpHostGroup *igmpHostCreateGroup(IgmpHostContext *context,
   Ipv4Addr groupAddr)
{
   uint_t i;
   IgmpHostGroup *group;

   //Initialize pointer
   group = NULL;

   //Valid multicast address?
   if(ipv4IsMulticastAddr(groupAddr) && groupAddr != IGMP_ALL_SYSTEMS_ADDR)
   {
      //Loop through multicast groups
      for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
      {
         //Check whether the entry is available
         if(context->groups[i].state == IGMP_HOST_GROUP_STATE_NON_MEMBER)
         {
            //Debug message
            TRACE_DEBUG("Creating IGMP group (%s)...\r\n",
               ipv4AddrToString(groupAddr, NULL));

            //Point to the current group
            group = &context->groups[i];

            //Initialize group
            osMemset(group, 0, sizeof(IgmpHostGroup));

            //Switch to the Init Member state
            group->state = IGMP_HOST_GROUP_STATE_INIT_MEMBER;
            //Save the multicast group address
            group->addr = groupAddr;

            //A per-group and interface timer is used for scheduling responses to
            //Group-Specific and Group-and-Source-Specific Queries
            netStopTimer(&group->timer);

            //The "non-existent" state is considered to have a filter mode of
            //INCLUDE and an empty source list
            group->filterMode = IP_FILTER_MODE_INCLUDE;
            group->filter.numSources = 0;

            //We are done
            break;
         }
      }
   }

   //Return a pointer to the newly created multicast group
   return group;
}


/**
 * @brief Search the list of multicast groups for a given group address
 * @param[in] context Pointer to the IGMP host context
 * @param[in] groupAddr Multicast group address
 * @return Pointer to the matching multicast group, if any
 **/

IgmpHostGroup *igmpHostFindGroup(IgmpHostContext *context, Ipv4Addr groupAddr)
{
   uint_t i;
   IgmpHostGroup *group;

   //Initialize pointer
   group = NULL;

   //Loop through multicast groups
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Matching group?
      if(context->groups[i].state != IGMP_HOST_GROUP_STATE_NON_MEMBER &&
         context->groups[i].addr == groupAddr)
      {
         //Point to the current group
         group = &context->groups[i];
         break;
      }
   }

   //Return a pointer to the matching multicast group
   return group;
}


/**
 * @brief Check whether a group matches a given multicast address
 * @param[in] group Pointer to the multicast group
 * @param[in] multicastAddr IPv4 multicast address
 * @return TRUE if the group matches the specified multicast address, else FALSE
 **/

bool_t igmpHostMatchGroup(IgmpHostGroup *group, Ipv4Addr multicastAddr)
{
   bool_t match;

   //Initialize flag
   match = FALSE;

   //Valid group?
   if(group->state != IGMP_HOST_GROUP_STATE_NON_MEMBER)
   {
      //Matching multicast address?
      if(multicastAddr == IPV4_UNSPECIFIED_ADDR ||
         multicastAddr == group->addr)
      {
         match = TRUE;
      }
   }

   //Return TRUE if the group matches the specified multicast address
   return match;
}


/**
 * @brief Delete a multicast group
 * @param[in] group Pointer to the multicast group
 **/

void igmpHostDeleteGroup(IgmpHostGroup *group)
{
   //Debug message
   TRACE_DEBUG("Deleting IGMP group (%s)...\r\n",
      ipv4AddrToString(group->addr, NULL));

   //Groups in Non-Member state require no storage in the host
   group->state = IGMP_HOST_GROUP_STATE_NON_MEMBER;
}


/**
 * @brief Delete groups in "non-existent" state
 * @param[in] context Pointer to the IGMP host context
 **/

void igmpHostFlushUnusedGroups(IgmpHostContext *context)
{
   uint_t i;
   IgmpHostGroup *group;

   //Loop through multicast groups
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Valid group?
      if(group->state != IGMP_HOST_GROUP_STATE_NON_MEMBER)
      {
         //Retransmission state needs to be maintained until [Robustness
         //Variable] State-Change reports have been sent by the host
         if(!igmpHostGetGroupRetransmitStatus(group))
         {
            //The "non-existent" state is considered to have a filter mode of
            //INCLUDE and an empty source list
            if(group->filterMode == IP_FILTER_MODE_INCLUDE &&
               group->filter.numSources == 0)
            {
               //Delete the group
               igmpHostDeleteGroup(group);
            }
         }
      }
   }
}


/**
 * @brief Append a source address to a given list
 * @param[in] list Pointer to the list of source addresses
 * @param[in] srcAddr Source address to be added
 **/

error_t igmpHostAddSrcAddr(IgmpHostSrcAddrList *list, Ipv4Addr srcAddr)
{
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   error_t error;
   size_t n;

   //Initialize status code
   error = NO_ERROR;

   //Make sure that the source address is not a duplicate
   if(igmpHostFindSrcAddr(list, srcAddr) < 0)
   {
      //Make sure there is enough room to add the source address
      if(list->numSources < IPV4_MAX_MULTICAST_SOURCES)
      {
         //Get the index of the new element
         n = list->numSources;

         //When a source is included in the list, its counter is set to
         //[Robustness Variable]
         list->sources[n].addr = srcAddr;
         list->sources[n].retransmitCount = IGMP_ROBUSTNESS_VARIABLE;

         //Adjust the number of elements
         list->numSources++;
      }
      else
      {
         //The implementation limits the number of source addresses
         error = ERROR_OUT_OF_RESOURCES;
      }
   }

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Remove a source address from a given list
 * @param[in] list Pointer to the list of source addresses
 * @param[in] srcAddr Source address to be removed
 **/

void igmpHostRemoveSrcAddr(IgmpHostSrcAddrList *list, Ipv4Addr srcAddr)
{
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   uint_t i;
   uint_t j;

   //Loop through the list of source addresses
   for(i = 0; i < list->numSources; i++)
   {
      //Matching IP address?
      if(list->sources[i].addr == srcAddr)
      {
         //Remove the source address from the list
         for(j = i + 1; j < list->numSources; j++)
         {
            list->sources[j - 1] = list->sources[j];
         }

         //Update the length of the list
         list->numSources--;

         //We are done
         break;
      }
   }
#endif
}


/**
 * @brief Search the list of sources for a given IP address
 * @param[in] list Pointer to the list of source addresses
 * @param[in] srcAddr Source IP address
 * @return Index of the matching IP address is returned. -1 is
 *   returned if the specified IP address cannot be found
 **/

int_t igmpHostFindSrcAddr(const IgmpHostSrcAddrList *list, Ipv4Addr srcAddr)
{
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   int_t i;
   int_t index;

   //Initialize index
   index = -1;

   //Loop through the list of source addresses
   for(i = 0; i < list->numSources; i++)
   {
      //Matching IP address?
      if(list->sources[i].addr == srcAddr)
      {
         index = i;
         break;
      }
   }

   //Return the index of the matching IP address, if any
   return index;
#else
   //Not implemented
   return -1;
#endif
}

#endif
