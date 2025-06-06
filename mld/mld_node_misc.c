/**
 * @file mld_node_misc.c
 * @brief Helper functions for MLD node
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
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_multicast.h"
#include "ipv6/icmpv6.h"
#include "mld/mld_node.h"
#include "mld/mld_node_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && MLD_NODE_SUPPORT == ENABLED)


/**
 * @brief Change host compatibility mode
 * @param[in] context Pointer to the MLD node context
 * @param[in] compatibilityMode New host compatibility mode
 **/

void mldNodeChangeCompatibilityMode(MldNodeContext *context,
   MldVersion compatibilityMode)
{
   uint_t i;
   MldNodeGroup *group;

   //Debug message
   TRACE_DEBUG("Changing host compatibility mode to MLDv%u...\r\n",
      (uint_t) compatibilityMode);

   //Switch compatibility mode immediately
   context->compatibilityMode = compatibilityMode;

   //Whenever a host changes its compatibility mode, it cancels all its pending
   //response and retransmission timers (refer to RFC 3810, section 8.2.1)
   netStopTimer(&context->generalQueryTimer);
   netStopTimer(&context->stateChangeReportTimer);

   //Loop through multicast groups
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Valid group?
      if(group->state != MLD_NODE_GROUP_STATE_NON_LISTENER)
      {
         //Reset retransmission counter
         group->retransmitCount = 0;

#if (IPV6_MAX_MULTICAST_SOURCES > 0)
         //Clear source lists
         group->allow.numSources = 0;
         group->block.numSources = 0;
         group->queriedSources.numSources = 0;
#endif
         //Cancel the pending response, if any
         netStopTimer(&group->timer);

         //Switch to the Idle Listener state
         group->state = MLD_NODE_GROUP_STATE_IDLE_LISTENER;
      }
   }

   //Delete groups in "non-existent" state
   mldNodeFlushUnusedGroups(context);
}


/**
 * @brief Send an unsolicited Report message
 * @param[in] context Pointer to the MLD node context
 * @param[in] groupAddr IPv6 address specifying the multicast address
 **/

void mldNodeSendUnsolicitedReport(MldNodeContext *context,
   const Ipv6Addr *groupAddr)
{
   MldNodeGroup *group;

   //Search the list of groups for the specified multicast address
   group = mldNodeFindGroup(context, groupAddr);

   //Any matching group found?
   if(group != NULL)
   {
      //Check group state
      if(group->state == MLD_NODE_GROUP_STATE_INIT_LISTENER)
      {
         //Check host compatibility mode
         if(context->compatibilityMode == MLD_VERSION_1)
         {
            //Send a Multicast Listener Report message
            mldNodeSendListenerReport(context, groupAddr);
         }
         else
         {
            //Send a Version 2 Multicast Listener Report message
            mldNodeSendStateChangeReport(context, groupAddr);
         }
      }
   }
}


/**
 * @brief Send Multicast Listener Report message
 * @param[in] context Pointer to the MLD node context
 * @param[in] groupAddr IPv6 address specifying the multicast address
 **/

void mldNodeSendListenerReport(MldNodeContext *context,
   const Ipv6Addr *groupAddr)
{
   size_t offset;
   NetBuffer *buffer;
   MldMessage *message;

   //Allocate a memory buffer to hold the MLD message
   buffer = ipAllocBuffer(sizeof(MldMessage), &offset);
   //Failed to allocate memory?
   if(buffer == NULL)
      return;

   //Point to the beginning of the MLD message
   message = netBufferAt(buffer, offset, 0);

   //Format the Multicast Listener Report message
   message->type = ICMPV6_TYPE_MCAST_LISTENER_REPORT_V1;
   message->code = 0;
   message->checksum = 0;
   message->maxRespDelay = 0;
   message->reserved = 0;
   message->multicastAddr = *groupAddr;

   //The Multicast Listener Report message is sent to the multicast address
   //being reported
   mldSendMessage(context->interface, groupAddr, buffer, offset);

   //Free previously allocated memory
   netBufferFree(buffer);
}


/**
 * @brief Send Multicast Listener Done message
 * @param[in] context Pointer to the MLD node context
 * @param[in] groupAddr IPv6 address specifying the multicast address being left
 **/

void mldNodeSendListenerDone(MldNodeContext *context,
   const Ipv6Addr *groupAddr)
{
   size_t offset;
   NetBuffer *buffer;
   MldMessage *message;

   //Allocate a memory buffer to hold the MLD message
   buffer = ipAllocBuffer(sizeof(MldMessage), &offset);
   //Failed to allocate memory?
   if(buffer == NULL)
      return;

   //Point to the beginning of the MLD message
   message = netBufferAt(buffer, offset, 0);

   //Format the Multicast Listener Done message
   message->type = ICMPV6_TYPE_MCAST_LISTENER_DONE_V1;
   message->code = 0;
   message->checksum = 0;
   message->maxRespDelay = 0;
   message->reserved = 0;
   message->multicastAddr = *groupAddr;

   //The Multicast Listener Done message is sent to the all-routers multicast
   //address
   mldSendMessage(context->interface, &IPV6_LINK_LOCAL_ALL_ROUTERS_ADDR,
      buffer, offset);

   //Free previously allocated memory
   netBufferFree(buffer);
}


/**
 * @brief Send Current-State Report message
 * @param[in] context Pointer to the MLD node context
 * @param[in] groupAddr IPv6 address specifying the group address
 **/

void mldNodeSendCurrentStateReport(MldNodeContext *context,
   const Ipv6Addr *groupAddr)
{
   uint_t i;
   size_t n;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   MldNodeGroup *group;
   MldListenerReportV2 *message;
   MldMcastAddrRecord *record;

   //Allocate a memory buffer to hold the MLD message
   buffer = ipAllocBuffer(MLD_MAX_MSG_SIZE, &offset);
   //Failed to allocate memory?
   if(buffer == NULL)
      return;

   //Point to the beginning of the MLD message
   message = netBufferAt(buffer, offset, 0);

   //Format the Version 2 Multicast Listener Report message
   message->type = ICMPV6_TYPE_MCAST_LISTENER_REPORT_V2;
   message->reserved = 0;
   message->checksum = 0;
   message->flags = 0;
   message->numOfMcastAddrRecords = 0;

   //Total length of the message
   length = sizeof(MldListenerReportV2);

   //Loop through multicast groups
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Matching group?
      if(mldNodeMatchGroup(group, groupAddr))
      {
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
         //Check whether the interface has reception state for that group
         //address
         if(group->filterMode == IP_FILTER_MODE_EXCLUDE ||
            group->filter.numSources > 0)
         {
            uint_t j;

            //General Query or Group-Specific Query?
            if(ipv6CompAddr(groupAddr, &IPV6_UNSPECIFIED_ADDR) ||
               group->queriedSources.numSources == 0)
            {
               //Calculate the length of the multicast address record
               n = sizeof(MldMcastAddrRecord) + group->filter.numSources *
                  sizeof(Ipv6Addr);

               //If the set of Multicast Address Records required in a Report
               //does not fit within the size limit of a single Report message,
               //the Multicast Address Records are sent in as many Report
               //messages as needed to report the entire set (refer to RFC 3810,
               //section 5.2.15)
               if((length + n) > MLD_MAX_MSG_SIZE)
               {
                  //Send report message
                  mldNodeFlushReportRecords(context, buffer, offset, &length);
               }

               //Point to the buffer where to format the record
               record = (MldMcastAddrRecord *) ((uint8_t *) message + length);

               //The Current-State Record carries the associated filter mode
               //(MODE_IS_INCLUDE or MODE_IS_EXCLUDE)
               if(group->filterMode == IP_FILTER_MODE_INCLUDE)
               {
                  record->recordType = MLD_MCAST_ADDR_RECORD_TYPE_IS_IN;
               }
               else
               {
                  record->recordType = MLD_MCAST_ADDR_RECORD_TYPE_IS_EX;
               }

               //Format multicast address record
               record->auxDataLen = 0;
               record->numOfSources = htons(group->filter.numSources);
               record->multicastAddr = group->addr;

               //Format the list of source addresses
               for(j = 0; j < group->filter.numSources; j++)
               {
                  record->srcAddr[j] = group->filter.sources[j];
               }

               //Increment the number of multicast address records
               message->numOfMcastAddrRecords++;
               //Update the length of the message
               length += n;
            }
            else
            {
               //If the list of recorded sources B for that multicast address
               //is non-empty, then the contents of the responding Current-State
               //Record is determined from the interface state and the pending
               //response record
               if(group->filterMode == IP_FILTER_MODE_INCLUDE)
               {
                  //If the interface state is INCLUDE (A), then the contents of
                  //the responding Current-State Record is IS_IN (A*B)
                  for(j = 0; j < group->queriedSources.numSources; )
                  {
                     if(ipv6FindSrcAddr(&group->filter,
                        &group->queriedSources.sources[j]) >= 0)
                     {
                        j++;
                     }
                     else
                     {
                        ipv6RemoveSrcAddr(&group->queriedSources,
                           &group->queriedSources.sources[j]);
                     }
                  }
               }
               else
               {
                  //If the interface state is EXCLUDE (A), then the contents of
                  //the responding Current-State Record is IS_IN (B-A)
                  for(j = 0; j < group->filter.numSources; j++)
                  {
                     ipv6RemoveSrcAddr(&group->queriedSources,
                        &group->filter.sources[j]);
                  }
               }

               //If the resulting Current-State Record has an empty set of
               //source addresses, then no response is sent
               if(group->queriedSources.numSources > 0)
               {
                  //Calculate the length of the multicast address record
                  n = sizeof(MldMcastAddrRecord) +
                     group->queriedSources.numSources * sizeof(Ipv6Addr);

                  //If the set of Multicast Address Records required in a
                  //Report does not fit within the size limit of a single
                  //Report message, the Multicast Address Records are sent in
                  //as many Report messages as needed to report the entire set
                  //(refer to RFC 3810, section 5.2.15)
                  if((length + n) > MLD_MAX_MSG_SIZE)
                  {
                     //Send report message
                     mldNodeFlushReportRecords(context, buffer, offset, &length);
                  }

                  //Point to the buffer where to format the record
                  record = (MldMcastAddrRecord *) ((uint8_t *) message + length);

                  //Format multicast address record
                  record->recordType = MLD_MCAST_ADDR_RECORD_TYPE_IS_IN;
                  record->auxDataLen = 0;
                  record->numOfSources = htons(group->queriedSources.numSources);
                  record->multicastAddr = group->addr;

                  //Format the list of source addresses
                  for(j = 0; j < group->queriedSources.numSources; j++)
                  {
                     record->srcAddr[j] = group->queriedSources.sources[j];
                  }

                  //Increment the number of multicast address records
                  message->numOfMcastAddrRecords++;
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
            //Calculate the length of the multicast address record
            n = sizeof(MldMcastAddrRecord);

            //If the set of Multicast Address Records required in a Report
            //does not fit within the size limit of a single Report message,
            //the Multicast Address Records are sent in as many Report
            //messages as needed to report the entire set (refer to RFC 3810,
            //section 5.2.15)
            if((length + n) > MLD_MAX_MSG_SIZE)
            {
               //Send report message
               mldNodeFlushReportRecords(context, buffer, offset, &length);
            }

            //Point to the buffer where to format the record
            record = (MldMcastAddrRecord *) ((uint8_t *) message + length);

            //The Current-State Record carries the associated filter mode
            record->recordType = MLD_MCAST_ADDR_RECORD_TYPE_IS_EX;
            record->auxDataLen = 0;
            record->numOfSources = HTONS(0);
            record->multicastAddr = group->addr;

            //Increment the number of multicast address records
            message->numOfMcastAddrRecords++;
            //Update the length of the message
            length += n;
         }
#endif
      }
   }

   //Version 2 Multicast Listener Reports are sent with an IP destination
   //address of ff02:0:0:0:0:0:0:16, to which all MLDv2-capable multicast
   //routers listen (refer to RFC 3810, section 5.2.14)
   mldNodeFlushReportRecords(context, buffer, offset, &length);

   //Free previously allocated memory
   netBufferFree(buffer);
}


/**
 * @brief Send State-Change Report message
 * @param[in] context Pointer to the MLD node context
 * @param[in] groupAddr IPv6 address specifying the group address
 **/

void mldNodeSendStateChangeReport(MldNodeContext *context,
   const Ipv6Addr *groupAddr)
{
   uint_t i;
   size_t n;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   MldNodeGroup *group;
   MldListenerReportV2 *message;
   MldMcastAddrRecord *record;
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   uint_t j;
#endif

   //Allocate a memory buffer to hold the MLD message
   buffer = ipAllocBuffer(MLD_MAX_MSG_SIZE, &offset);
   //Failed to allocate memory?
   if(buffer == NULL)
      return;

   //Point to the beginning of the MLD message
   message = netBufferAt(buffer, offset, 0);

   //Format the Version 2 Multicast Listener Report message
   message->type = ICMPV6_TYPE_MCAST_LISTENER_REPORT_V2;
   message->reserved = 0;
   message->checksum = 0;
   message->flags = 0;
   message->numOfMcastAddrRecords = 0;

   //Total length of the message
   length = sizeof(MldListenerReportV2);

   //Loop through multicast groups
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Matching group?
      if(mldNodeMatchGroup(group, groupAddr))
      {
         //The report should contain a Filter Mode Change Record if the Filter
         //Mode Retransmission Counter has a value higher than zero
         if(group->retransmitCount > 0)
         {
            //Calculate the length of the multicast address record
            n = sizeof(MldMcastAddrRecord) + group->filter.numSources *
               sizeof(Ipv6Addr);

            //If the set of Multicast Address Records required in a Report
            //does not fit within the size limit of a single Report message,
            //the Multicast Address Records are sent in as many Report
            //messages as needed to report the entire set (refer to RFC 3810,
            //section 5.2.15)
            if((length + n) > MLD_MAX_MSG_SIZE)
            {
               //Send report message
               mldNodeFlushReportRecords(context, buffer, offset, &length);
            }

            //Point to the buffer where to format the record
            record = (MldMcastAddrRecord *) ((uint8_t *) message + length);

            //If the current filter-mode of the interface is INCLUDE, a TO_IN
            //record is included in the report, otherwise a TO_EX record is
            //included
            if(group->filterMode == IP_FILTER_MODE_INCLUDE)
            {
               record->recordType = MLD_MCAST_ADDR_RECORD_TYPE_TO_IN;
            }
            else
            {
               record->recordType = MLD_MCAST_ADDR_RECORD_TYPE_TO_EX;
            }

            //Format multicast address record
            record->auxDataLen = 0;
            record->numOfSources = htons(group->filter.numSources);
            record->multicastAddr = group->addr;

#if (IPV6_MAX_MULTICAST_SOURCES > 0)
            //Format the list of source addresses
            for(j = 0; j < group->filter.numSources; j++)
            {
               record->srcAddr[j] = group->filter.sources[j];
            }
#endif
            //Increment the number of multicast address records
            message->numOfMcastAddrRecords++;
            //Update the length of the message
            length += n;
         }
         else
         {
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
            //If the computed source list for a ALLOW record is empty, that
            //record is omitted from the State-Change report
            if(group->allow.numSources > 0)
            {
               //Calculate the length of the ALLOW record
               n = sizeof(MldMcastAddrRecord) + group->allow.numSources *
                  sizeof(Ipv6Addr);

               //If the set of Multicast Address Records required in a Report
               //does not fit within the size limit of a single Report message,
               //the Multicast Address Records are sent in as many Report
               //messages as needed to report the entire set (refer to RFC 3810,
               //section 5.2.15)
               if((length + n) > MLD_MAX_MSG_SIZE)
               {
                  //Send report message
                  mldNodeFlushReportRecords(context, buffer, offset, &length);
               }

               //Point to the buffer where to format the record
               record = (MldMcastAddrRecord *) ((uint8_t *) message + length);

               //The ALLOW record contains the list of the additional sources
               //that the system wishes to hear from
               record->recordType = MLD_MCAST_ADDR_RECORD_TYPE_ALLOW;
               record->auxDataLen = 0;
               record->numOfSources = htons(group->allow.numSources);
               record->multicastAddr = group->addr;

               //Format the list of source addresses
               for(j = 0; j < group->allow.numSources; j++)
               {
                  record->srcAddr[j] = group->allow.sources[j].addr;
               }

               //Increment the number of multicast address records
               message->numOfMcastAddrRecords++;
               //Update the length of the message
               length += n;
            }

            //If the computed source list for a BLOCK record is empty, that
            //record is omitted from the State-Change report
            if(group->block.numSources > 0)
            {
               //Calculate the length of the BLOCK record
               n = sizeof(MldMcastAddrRecord) + group->block.numSources *
                  sizeof(Ipv6Addr);

               //If the set of Multicast Address Records required in a Report
               //does not fit within the size limit of a single Report message,
               //the Multicast Address Records are sent in as many Report
               //messages as needed to report the entire set (refer to RFC 3810,
               //section 5.2.15)
               if((length + n) > MLD_MAX_MSG_SIZE)
               {
                  //Send report message
                  mldNodeFlushReportRecords(context, buffer, offset, &length);
               }

               //Point to the buffer where to format the record
               record = (MldMcastAddrRecord *) ((uint8_t *) message + length);

               //The BLOCK record contains the list of the sources that the
               //system no longer wishes to hear from
               record->recordType = MLD_MCAST_ADDR_RECORD_TYPE_BLOCK;
               record->auxDataLen = 0;
               record->numOfSources = htons(group->block.numSources);
               record->multicastAddr = group->addr;

               //Format the list of source addresses
               for(j = 0; j < group->block.numSources; j++)
               {
                  record->srcAddr[j] = group->block.sources[j].addr;
               }

               //Increment the number of multicast address records
               message->numOfMcastAddrRecords++;
               //Update the length of the message
               length += n;
            }
#endif
         }

         //Retransmission state needs to be maintained until [Robustness
         //Variable] State-Change reports have been sent by the host
         mldNodeDecGroupRetransmitCounters(group);
      }
   }

   //Version 2 Multicast Listener Reports are sent with an IP destination
   //address of ff02:0:0:0:0:0:0:16, to which all MLDv2-capable multicast
   //routers listen (refer to RFC 3810, section 5.2.14)
   mldNodeFlushReportRecords(context, buffer, offset, &length);

   //Free previously allocated memory
   netBufferFree(buffer);
}


/**
 * @brief Flush report records
 * @param[in] context Pointer to the MLD node context
 * @param[in] buffer Multi-part buffer containing the report message
 * @param[in] offset Offset to the first byte of the report message
 * @param[in,out] length Length of the report message, in bytes
 **/

void mldNodeFlushReportRecords(MldNodeContext *context, NetBuffer *buffer,
   size_t offset, size_t *length)
{
   MldListenerReportV2 *message;

   //Any multicast address records included in the message?
   if(*length > 0)
   {
      //Point to the beginning of the report message
      message = netBufferAt(buffer, offset, 0);

      //The Nr of Mcast Address Records field specifies how many Multicast
      //Address Records are present in this Report
      message->numOfMcastAddrRecords = htons(message->numOfMcastAddrRecords);

      //Adjust the length of the multi-part buffer
      netBufferSetLength(buffer, offset + *length);

      //Version 2 Multicast Listener Reports are sent with an IP destination
      //address of ff02:0:0:0:0:0:0:16, to which all MLDv2-capable multicast
      //routers listen (refer to RFC 3810, section 5.2.14)
      mldSendMessage(context->interface, &MLD_V2_ALL_ROUTERS_ADDR, buffer,
         offset);

      //Reset the Checksum field
      message->checksum = 0;
      //Reset the Nr of Mcast Address Records field
      message->numOfMcastAddrRecords = 0;

      //Update the length of the message
      *length = sizeof(MldListenerReportV2);
   }
}


/**
 * @brief Process incoming MLD message
 * @param[in] context Pointer to the MLD node context
 * @param[in] pseudoHeader IPv6 pseudo header
 * @param[in] message Pointer to the incoming MLD message
 * @param[in] length Length of the MLD message, in bytes
 **/

void mldNodeProcessMessage(MldNodeContext *context,
   const Ipv6PseudoHeader *pseudoHeader, const MldMessage *message,
   size_t length)
{
   //Check MLD message type
   if(message->type == ICMPV6_TYPE_MCAST_LISTENER_QUERY)
   {
      //The MLD version of a Multicast Listener Query message is determined as
      //follows
      if(length == sizeof(MldMessage))
      {
         //Process Multicast Listener Query message
         mldNodeProcessListenerQuery(context, pseudoHeader, message, length);
      }
      else if(length >= sizeof(MldListenerQueryV2))
      {
         //Process Version 2 Multicast Listener Query message
         mldNodeProcessListenerQueryV2(context, pseudoHeader,
            (MldListenerQueryV2 *) message, length);
      }
      else
      {
         //Query messages that do not match any of the above conditions must be
         //silently ignored (refer to RFC 3810, section 8.1)
      }
   }
   else if(message->type == ICMPV6_TYPE_MCAST_LISTENER_REPORT_V1)
   {
      //Process Multicast Listener Report message
      mldNodeProcessListenerReport(context, pseudoHeader, message, length);
   }
   else
   {
      //Discard other messages
   }
}


/**
 * @brief Process incoming Multicast Listener Query message
 * @param[in] context Pointer to the MLD node context
 * @param[in] pseudoHeader IPv6 pseudo header
 * @param[in] message Pointer to the incoming MLD message
 * @param[in] length Length of the MLD message, in bytes
 **/

void mldNodeProcessListenerQuery(MldNodeContext *context,
   const Ipv6PseudoHeader *pseudoHeader, const MldMessage *message,
   size_t length)
{
   uint_t i;
   systime_t delay;
   systime_t maxRespDelay;
   MldNodeGroup *group;

   //MLD Query messages must be sent with a valid link-local address as the
   //IPv6 source address. If a node receives a query message with an IPv6
   //source address set to the unspecified address, it must silently discard
   //the message (refer to RFC 3590, section 4)
   if(!ipv6IsLinkLocalUnicastAddr(&pseudoHeader->srcAddr))
      return;

   //A General Query is used to learn which multicast addresses have listeners
   //on an attached link. A Multicast-Address-Specific Query is used to learn
   //if a particular multicast address has any listeners on an attached link
   if(!ipv6CompAddr(&message->multicastAddr, &IPV6_UNSPECIFIED_ADDR) &&
      !ipv6IsMulticastAddr(&message->multicastAddr))
   {
      return;
   }

   //The Older Version Querier Present timer for the interface is set to Older
   //Version Querier Present Timeout seconds
   netStartTimer(&context->olderVersionQuerierPresentTimer,
      MLD_OLDER_VERSION_QUERIER_PRESENT_TIMEOUT);

   //The Host Compatibility Mode of an interface is set to MLDv1 whenever an
   //MLDv1 Multicast Address Listener Query is received on that interface
   if(context->compatibilityMode > MLD_VERSION_1)
   {
      //The host should switch compatibility mode immediately
      mldNodeChangeCompatibilityMode(context, MLD_VERSION_1);
   }

   //The Maximum Response Delay field specifies the maximum allowed delay
   //before sending a responding report, in units of milliseconds
   maxRespDelay = ntohs(message->maxRespDelay);

   //Loop through multicast groups
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Matching group?
      if(mldNodeMatchGroup(group, &message->multicastAddr))
      {
         //Check group state
         if(group->state == MLD_NODE_GROUP_STATE_DELAYING_LISTENER)
         {
            //If a timer for the address is already running, it is reset to
            //the new random value only if the requested Max Response Delay
            //is less than the remaining value of the running timer
            if(maxRespDelay < netGetRemainingTime(&group->timer))
            {
               //Select a random value in the range 0 - Max Response Delay
               delay = mldGetRandomDelay(maxRespDelay);
               //Restart delay timer
               netStartTimer(&group->timer, delay);
            }
         }
         else if(group->state == MLD_NODE_GROUP_STATE_IDLE_LISTENER)
         {
            //Select a random value in the range 0 - Max Response Delay
            delay = mldGetRandomDelay(maxRespDelay);
            //Start delay timer
            netStartTimer(&group->timer, delay);

            //Switch to the Delaying Listener state
            group->state = MLD_NODE_GROUP_STATE_DELAYING_LISTENER;
         }
         else
         {
            //Just for sanity
         }
      }
   }
}


/**
 * @brief Process incoming Version 2 Multicast Listener Query message
 * @param[in] context Pointer to the MLD node context
 * @param[in] pseudoHeader IPv6 pseudo header
 * @param[in] message Pointer to the incoming MLD message
 * @param[in] length Length of the MLD message, in bytes
 **/

void mldNodeProcessListenerQueryV2(MldNodeContext *context,
   const Ipv6PseudoHeader *pseudoHeader, const MldListenerQueryV2 *message,
   size_t length)
{
   uint_t i;
   uint_t n;
   systime_t delay;
   systime_t maxRespDelay;
   uint16_t maxRespCode;
   MldNodeGroup *group;

   //When Host Compatibility Mode is MLDv1, a host acts in MLDv1 compatibility
   //mode, using only the MLDv1 protocol, on that interface
   if(context->compatibilityMode < MLD_VERSION_2)
      return;

   //Check the length of the Version 2 Membership Query message
   if(length < sizeof(MldListenerQueryV2))
      return;

   //For a General Query, the Multicast Address field is set to zero. For a
   //Multicast Address Specific Query or Multicast Address and Source Specific
   //Query, it is set to the multicast address being queried (refer to
   //RFC 3810, section 5.1.5)
   if(!ipv6CompAddr(&message->multicastAddr, &IPV6_UNSPECIFIED_ADDR) &&
      !ipv6IsMulticastAddr(&message->multicastAddr))
   {
      return;
   }

   //The Number of Sources field specifies how many source addresses are
   //present in the Query
   n = ntohs(message->numOfSources);

   //Malformed message?
   if(length < (sizeof(MldListenerQueryV2) + n * sizeof(Ipv6Addr)))
      return;

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

   //If the node has a per-interface listening state record that corresponds to
   //the queried multicast address, a delay for a response is randomly selected
   //in the range 0 - Maximum Response Delay (refer to RFC 3810, section 6.2)
   delay = mldGetRandomDelay(maxRespDelay);

   //The following rules are then used to determine if a Report needs to be
   //scheduled and the type of Report to schedule
   if(netTimerRunning(&context->generalQueryTimer) &&
      netGetRemainingTime(&context->generalQueryTimer) < delay)
   {
      //If there is a pending response to a previous General Query scheduled
      //sooner than the selected delay, no additional response needs to be
      //scheduled
   }
   else if(ipv6CompAddr(&message->multicastAddr, &IPV6_UNSPECIFIED_ADDR))
   {
      //If the received Query is a General Query, the interface timer is used
      //to schedule a response to the General Query after the selected delay.
      //Any previously pending response to a General Query is canceled
      netStartTimer(&context->generalQueryTimer, delay);
   }
   else
   {
      //Loop through multicast groups
      for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
      {
         //Point to the current group
         group = &context->groups[i];

         //Matching group?
         if(mldNodeMatchGroup(group, &message->multicastAddr))
         {
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
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
                  error = ipv6AddSrcAddr(&group->queriedSources,
                     &message->srcAddr[j]);
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
                     error = ipv6AddSrcAddr(&group->queriedSources,
                        &message->srcAddr[j]);
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
 * @brief Process incoming Multicast Listener Report message
 * @param[in] context Pointer to the MLD node context
 * @param[in] pseudoHeader IPv6 pseudo header
 * @param[in] message Pointer to the incoming MLD message
 * @param[in] length Length of the MLD message, in bytes
 **/

void mldNodeProcessListenerReport(MldNodeContext *context,
   const Ipv6PseudoHeader *pseudoHeader, const MldMessage *message,
   size_t length)
{
   MldNodeGroup *group;

   //When Host Compatibility Mode is MLDv2, a host acts using the MLDv2
   //protocol on that interface
   if(context->compatibilityMode > MLD_VERSION_1)
      return;

   //In a Report message, the Multicast Address field holds a specific IPv6
   //multicast address to which the message sender is listening
   if(!ipv6IsMulticastAddr(&message->multicastAddr))
      return;

   //Search the list of groups for the specified multicast address
   group = mldNodeFindGroup(context, &message->multicastAddr);

   //Any matching group found?
   if(group != NULL)
   {
      //Report messages are ignored for multicast addresses in the Non-Listener
      //or Idle Listener state
      if(group->state == MLD_NODE_GROUP_STATE_DELAYING_LISTENER)
      {
         //Clear flag
         group->flag = FALSE;
         //Switch to the Idle Listener state
         group->state = MLD_NODE_GROUP_STATE_IDLE_LISTENER;
      }
   }
}


/**
 * @brief Merge the difference report and the pending report
 * @param[in] group Pointer to the multicast group
 * @param[in] newFilterMode New filter mode for the affected group
 * @param[in] newFilter New interface state for the affected group
 **/

void mldNodeMergeReports(MldNodeGroup *group,
   IpFilterMode newFilterMode, const Ipv6SrcAddrList *newFilter)
{
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
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
         if(ipv6FindSrcAddr(&group->filter, &newFilter->sources[i]) < 0)
         {
            mldNodeAddSrcAddr(&group->allow, &newFilter->sources[i]);
            mldNodeRemoveSrcAddr(&group->block, &newFilter->sources[i]);
         }
      }

      //The BLOCK record contains the list of the sources that the system no
      //longer wishes to hear from
      for(i = 0; i < group->filter.numSources; i++)
      {
         if(ipv6FindSrcAddr(newFilter, &group->filter.sources[i]) < 0)
         {
            mldNodeAddSrcAddr(&group->block, &group->filter.sources[i]);
            mldNodeRemoveSrcAddr(&group->allow, &group->filter.sources[i]);
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
         if(ipv6FindSrcAddr(&group->filter, &newFilter->sources[i]) < 0)
         {
            mldNodeAddSrcAddr(&group->block, &newFilter->sources[i]);
            mldNodeRemoveSrcAddr(&group->allow, &newFilter->sources[i]);
         }
      }

      //The ALLOW record contains the list of the additional sources that the
      //system wishes to hear from
      for(i = 0; i < group->filter.numSources; i++)
      {
         if(ipv6FindSrcAddr(newFilter, &group->filter.sources[i]) < 0)
         {
            mldNodeAddSrcAddr(&group->allow, &group->filter.sources[i]);
            mldNodeRemoveSrcAddr(&group->block, &group->filter.sources[i]);
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
      group->retransmitCount = MLD_ROBUSTNESS_VARIABLE;
   }
}


/**
 * @brief Get the retransmission status of the State-Change report
 * @param[in] context Pointer to the MLD node context
 * @return TRUE if additional retransmissions are needed, else FALSE
 **/

bool_t mldNodeGetRetransmitStatus(MldNodeContext *context)
{
   uint_t i;
   bool_t status;
   MldNodeGroup *group;

   //Clear status flag
   status = FALSE;

   //Loop through multicast groups
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Valid group?
      if(group->state != MLD_NODE_GROUP_STATE_NON_LISTENER)
      {
         //Retransmission in progress?
         if(mldNodeGetGroupRetransmitStatus(group))
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

bool_t mldNodeGetGroupRetransmitStatus(MldNodeGroup *group)
{
   bool_t status;

   //Clear status flag
   status = FALSE;

#if (IPV6_MAX_MULTICAST_SOURCES > 0)
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

void mldNodeDecGroupRetransmitCounters(MldNodeGroup *group)
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
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
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
            mldNodeRemoveSrcAddr(&group->allow, &group->allow.sources[i].addr);
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
            mldNodeRemoveSrcAddr(&group->block, &group->block.sources[i].addr);
         }
      }
#endif
   }
}


/**
 * @brief Create a new multicast group
 * @param[in] context Pointer to the MLD node context
 * @param[in] groupAddr Multicast group address
 * @return Pointer to the newly created multicast group
 **/

MldNodeGroup *mldNodeCreateGroup(MldNodeContext *context,
   const Ipv6Addr *groupAddr)
{
   uint_t i;
   MldNodeGroup *group;

   //Initialize pointer
   group = NULL;

   //Valid multicast address?
   if(ipv6IsMulticastAddr(groupAddr) &&
      !ipv6CompAddr(groupAddr, &IPV6_LINK_LOCAL_ALL_NODES_ADDR))
   {
      //Loop through multicast groups
      for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
      {
         //Check whether the entry is available
         if(context->groups[i].state == MLD_NODE_GROUP_STATE_NON_LISTENER)
         {
            //Debug message
            TRACE_DEBUG("Creating MLD group (%s)...\r\n",
               ipv6AddrToString(groupAddr, NULL));

            //Point to the current group
            group = &context->groups[i];

            //Initialize group
            osMemset(group, 0, sizeof(MldNodeGroup));

            //Switch to the Init Listener state
            group->state = MLD_NODE_GROUP_STATE_INIT_LISTENER;
            //Save the multicast group address
            group->addr = *groupAddr;

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
 * @param[in] context Pointer to the MLD node context
 * @param[in] groupAddr Multicast group address
 * @return Pointer to the matching multicast group, if any
 **/

MldNodeGroup *mldNodeFindGroup(MldNodeContext *context,
   const Ipv6Addr *groupAddr)
{
   uint_t i;
   MldNodeGroup *group;

   //Initialize pointer
   group = NULL;

   //Loop through multicast groups
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Matching group?
      if(context->groups[i].state != MLD_NODE_GROUP_STATE_NON_LISTENER &&
         ipv6CompAddr(&context->groups[i].addr, groupAddr))
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
 * @param[in] multicastAddr IPv6 multicast address
 * @return TRUE if the group matches the specified multicast address, else FALSE
 **/

bool_t mldNodeMatchGroup(MldNodeGroup *group, const Ipv6Addr *multicastAddr)
{
   bool_t match;

   //Initialize flag
   match = FALSE;

   //Valid group?
   if(group->state != MLD_NODE_GROUP_STATE_NON_LISTENER)
   {
      //Matching multicast address?
      if(ipv6CompAddr(multicastAddr, &IPV6_UNSPECIFIED_ADDR) ||
         ipv6CompAddr(multicastAddr, &group->addr))
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

void mldNodeDeleteGroup(MldNodeGroup *group)
{
   //Debug message
   TRACE_DEBUG("Deleting MLD group (%s)...\r\n",
      ipv6AddrToString(&group->addr, NULL));

   //Groups in Non-Listener state require no storage in the host
   group->state = MLD_NODE_GROUP_STATE_NON_LISTENER;
}


/**
 * @brief Delete groups in "non-existent" state
 * @param[in] context Pointer to the MLD node context
 **/

void mldNodeFlushUnusedGroups(MldNodeContext *context)
{
   uint_t i;
   MldNodeGroup *group;

   //Loop through multicast groups
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current group
      group = &context->groups[i];

      //Valid group?
      if(group->state != MLD_NODE_GROUP_STATE_NON_LISTENER)
      {
         //Retransmission state needs to be maintained until [Robustness
         //Variable] State-Change reports have been sent by the host
         if(!mldNodeGetGroupRetransmitStatus(group))
         {
            //The "non-existent" state is considered to have a filter mode of
            //INCLUDE and an empty source list
            if(group->filterMode == IP_FILTER_MODE_INCLUDE &&
               group->filter.numSources == 0)
            {
               //Delete the group
               mldNodeDeleteGroup(group);
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

error_t mldNodeAddSrcAddr(MldNodeSrcAddrList *list, const Ipv6Addr *srcAddr)
{
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   error_t error;
   size_t n;

   //Initialize status code
   error = NO_ERROR;

   //Make sure that the source address is not a duplicate
   if(mldNodeFindSrcAddr(list, srcAddr) < 0)
   {
      //Make sure there is enough room to add the source address
      if(list->numSources < IPV6_MAX_MULTICAST_SOURCES)
      {
         //Get the index of the new element
         n = list->numSources;

         //When a source is included in the list, its counter is set to
         //[Robustness Variable]
         list->sources[n].addr = *srcAddr;
         list->sources[n].retransmitCount = MLD_ROBUSTNESS_VARIABLE;

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

void mldNodeRemoveSrcAddr(MldNodeSrcAddrList *list, const Ipv6Addr *srcAddr)
{
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   uint_t i;
   uint_t j;

   //Loop through the list of source addresses
   for(i = 0; i < list->numSources; i++)
   {
      //Matching IP address?
      if(ipv6CompAddr(&list->sources[i].addr, srcAddr))
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

int_t mldNodeFindSrcAddr(const MldNodeSrcAddrList *list,
   const Ipv6Addr *srcAddr)
{
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   int_t i;
   int_t index;

   //Initialize index
   index = -1;

   //Loop through the list of source addresses
   for(i = 0; i < list->numSources; i++)
   {
      //Matching IP address?
      if(ipv6CompAddr(&list->sources[i].addr, srcAddr))
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
