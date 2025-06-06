/**
 * @file igmp_host.c
 * @brief IGMP host
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
 * @section Description
 *
 * IGMP is used by IP hosts to report their multicast group memberships to any
 * immediately-neighboring multicast routers. Refer to the following RFCs for
 * complete details:
 * - RFC 1112: Host Extensions for IP Multicasting
 * - RFC 2236: Internet Group Management Protocol, Version 2
 * - RFC 3376: Internet Group Management Protocol, Version 3
 * - RFC 9776: Internet Group Management Protocol, Version 3
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
 * @brief IGMP host initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t igmpHostInit(NetInterface *interface)
{
   IgmpHostContext *context;

   //Point to the IGMP host context
   context = &interface->igmpHostContext;

   //Clear the IGMP host context
   osMemset(context, 0, sizeof(IgmpHostContext));

   //Underlying network interface
   context->interface = interface;
   //The default host compatibility mode is IGMPv3
   context->compatibilityMode = IGMP_VERSION_3;

   //In order to switch gracefully between versions of IGMP, hosts keep both
   //an IGMPv1 Querier Present timer and an IGMPv2 Querier Present timer per
   //interface (refer to RFC 3376, section 7.2.1)
   netStopTimer(&context->igmpv1QuerierPresentTimer);
   netStopTimer(&context->igmpv2QuerierPresentTimer);

   //A timer per interface is used for scheduling responses to General Queries
   netStopTimer(&context->generalQueryTimer);

   //A timer is used to retransmit State-Change reports
   netStopTimer(&context->stateChangeReportTimer);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief IGMP host timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * handle IGMP related timers
 *
 * @param[in] context Pointer to the IGMP host context
 **/

void igmpHostTick(IgmpHostContext *context)
{
   uint_t i;
   systime_t delay;
   IgmpHostGroup *group;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->interface;

   //In order to be compatible with older version routers, IGMPv3 hosts must
   //operate in version 1 and version 2 compatibility modes (refer to RFC 3376,
   //section 7.2.1)
   if(netTimerExpired(&context->igmpv1QuerierPresentTimer))
   {
      //Stop IGMPv1 Querier Present timer
      netStopTimer(&context->igmpv1QuerierPresentTimer);

      //Check whether IGMPv2 Querier Present timer is running
      if(netTimerRunning(&context->igmpv2QuerierPresentTimer))
      {
         //When the IGMPv1 Querier Present timer expires, a host switches to
         //Host Compatibility mode of IGMPv2 if it has a running IGMPv2
         //Querier Present timer
         igmpHostChangeCompatibilityMode(context, IGMP_VERSION_2);
      }
      else
      {
         //If it does not have a running IGMPv2 Querier Present timer then it
         //switches to Host Compatibility of IGMPv3
         igmpHostChangeCompatibilityMode(context, IGMP_VERSION_3);
      }
   }
   else if(netTimerExpired(&context->igmpv2QuerierPresentTimer))
   {
      //Stop IGMPv2 Querier Present timer
      netStopTimer(&context->igmpv2QuerierPresentTimer);

      //Check whether IGMPv1 Querier Present timer is running
      if(netTimerRunning(&context->igmpv1QuerierPresentTimer))
      {
         //The Host Compatibility Mode is set IGMPv1 when the IGMPv1 Querier
         //Present timer is running
      }
      else
      {
         //When the IGMPv2 Querier Present timer expires, a host switches to
         //Host Compatibility mode of IGMPv3
         igmpHostChangeCompatibilityMode(context, IGMP_VERSION_3);
      }
   }
   else
   {
      //Just for sanity
   }

   //Check host compatibility mode
   if(context->compatibilityMode <= IGMP_VERSION_2)
   {
      //Loop through multicast groups
      for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
      {
         //Point to the current group
         group = &context->groups[i];

         //Check group state
         if(group->state == IGMP_HOST_GROUP_STATE_INIT_MEMBER)
         {
            //Valid IPv4 address assigned to the interface?
            if(interface->linkState && ipv4IsHostAddrValid(interface))
            {
               //When a host joins a multicast group, it should immediately
               //transmit an unsolicited Membership Report for that group
               igmpHostSendMembershipReport(context, group->addr);

               //Start delay timer
               netStartTimer(&group->timer, IGMP_UNSOLICITED_REPORT_INTERVAL);

               //Set flag
               group->flag = TRUE;
               //Enter the Delaying Member state
               group->state = IGMP_HOST_GROUP_STATE_DELAYING_MEMBER;
            }
         }
         else if(group->state == IGMP_HOST_GROUP_STATE_DELAYING_MEMBER)
         {
            //Delay timer expired?
            if(netTimerExpired(&group->timer))
            {
               //Send a Membership Report message for the group on the interface
               igmpHostSendMembershipReport(context, group->addr);

               //Stop delay timer
               netStopTimer(&group->timer);

               //Set flag
               group->flag = TRUE;
               //Switch to the Idle Member state
               group->state = IGMP_HOST_GROUP_STATE_IDLE_MEMBER;
            }
         }
         else
         {
            //Just for sanity
         }
      }
   }
   else
   {
      //If the expired timer is the interface timer, then one Current-State
      //Record is sent for each multicast address for which the specified
      //interface has reception state
      if(netTimerExpired(&context->generalQueryTimer))
      {
         //Send Current-State report message
         igmpHostSendCurrentStateReport(context, IPV4_UNSPECIFIED_ADDR);

         //Stop interface timer
         netStopTimer(&context->generalQueryTimer);
      }

      //If the expired timer is a group timer, then a single Current-State
      //Record is sent for the corresponding group address
      for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
      {
         //Point to the current group
         group = &context->groups[i];

         //Check group state
         if(group->state == IGMP_HOST_GROUP_STATE_INIT_MEMBER)
         {
            //Valid IPv4 address assigned to the interface?
            if(interface->linkState && ipv4IsHostAddrValid(interface))
            {
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
               //Once a valid address is available, a node should generate new
               //IGMP Report messages for all multicast addresses joined on the
               //interface
               if(group->filterMode == IP_FILTER_MODE_INCLUDE &&
                  group->filter.numSources > 0)
               {
                  uint_t j;

                  //The State-Change report will include an ALLOW record
                  group->retransmitCount = 0;
                  group->allow.numSources = group->filter.numSources;
                  group->block.numSources = 0;

                  //List of the sources that the system wishes to hear from
                  for(j = 0; j < group->filter.numSources; j++)
                  {
                     group->allow.sources[j].addr = group->filter.sources[j];
                     group->allow.sources[j].retransmitCount = IGMP_ROBUSTNESS_VARIABLE;
                  }

                  //Send a State-Change report immediately
                  netStartTimer(&context->stateChangeReportTimer, 0);
               }
               else if(group->filterMode == IP_FILTER_MODE_EXCLUDE)
               {
                  //The State-Change report will include a TO_EX record
                  group->retransmitCount = IGMP_ROBUSTNESS_VARIABLE;
                  group->allow.numSources = 0;
                  group->block.numSources = 0;

                  //Send a State-Change report immediately
                  netStartTimer(&context->stateChangeReportTimer, 0);
               }
               else
               {
                  //Just for sanity
               }
#else
               //Once a valid address is available, a node should generate new
               //IGMP Report messages for all multicast addresses joined on the
               //interface
               if(group->filterMode == IP_FILTER_MODE_EXCLUDE)
               {
                  //The State-Change report will include a TO_EX record
                  group->retransmitCount = IGMP_ROBUSTNESS_VARIABLE;
                  //Send a State-Change report immediately
                  netStartTimer(&context->stateChangeReportTimer, 0);
               }
#endif
               //Enter the Idle Member state
               group->state = IGMP_HOST_GROUP_STATE_IDLE_MEMBER;
            }
         }
         else if(group->state == IGMP_HOST_GROUP_STATE_IDLE_MEMBER)
         {
            //Check whether the group timer has expired
            if(netTimerExpired(&group->timer))
            {
               //Send Current-State report message
               igmpHostSendCurrentStateReport(context, group->addr);

               //Stop group timer
               netStopTimer(&group->timer);
            }
         }
         else
         {
            //Just for sanity
         }
      }

      //If the expired timer is the retransmission timer, then the State-Change
      //report is retransmitted
      if(netTimerExpired(&context->stateChangeReportTimer))
      {
         //Retransmit the State-Change report message
         igmpHostSendStateChangeReport(context);

         //Retransmission state needs to be maintained until [Robustness
         //Variable] State-Change reports have been sent by the host
         if(igmpHostGetRetransmitStatus(context))
         {
            //Select a value in the range 0 - Unsolicited Report Interval
            delay = igmpGetRandomDelay(IGMP_V3_UNSOLICITED_REPORT_INTERVAL);
            //Restart retransmission timer
            netStartTimer(&context->stateChangeReportTimer, delay);
         }
         else
         {
            //[Robustness Variable] State-Change reports have been sent by the
            //host
            netStopTimer(&context->stateChangeReportTimer);
         }

         //Delete groups in "non-existent" state
         igmpHostFlushUnusedGroups(context);
      }
   }
}


/**
 * @brief Process multicast reception state change
 * @param[in] context Pointer to the IGMP host context
 * @param[in] groupAddr Multicast group address
 * @param[in] newFilterMode New filter mode for the affected group
 * @param[in] newFilter New interface state for the affected group
 **/

void igmpHostStateChangeEvent(IgmpHostContext *context, Ipv4Addr groupAddr,
   IpFilterMode newFilterMode, const Ipv4SrcAddrList *newFilter)
{
   systime_t delay;
   IgmpHostGroup *group;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->interface;

   //Search the list of groups for the specified multicast address
   group = igmpHostFindGroup(context, groupAddr);

   //Check whether the interface has reception state for that group address
   if(newFilterMode == IP_FILTER_MODE_EXCLUDE || newFilter->numSources > 0)
   {
      //No matching group found?
      if(group == NULL)
      {
         //Create a new group
         group = igmpHostCreateGroup(context, groupAddr);

         //Entry successfully created?
         if(group != NULL)
         {
            //Valid IPv4 address assigned to the interface?
            if(interface->linkState && ipv4IsHostAddrValid(interface))
            {
               //Check host compatibility mode
               if(context->compatibilityMode <= IGMP_VERSION_2)
               {
                  //When a host joins a multicast group, it should immediately
                  //transmit an unsolicited Membership Report for that group
                  igmpHostSendMembershipReport(context, group->addr);

                  //Start delay timer
                  netStartTimer(&group->timer, IGMP_UNSOLICITED_REPORT_INTERVAL);

                  //Set flag
                  group->flag = TRUE;
                  //Enter the Delaying Member state
                  group->state = IGMP_HOST_GROUP_STATE_DELAYING_MEMBER;
               }
               else
               {
                  //Enter the Idle Member state
                  group->state = IGMP_HOST_GROUP_STATE_IDLE_MEMBER;
               }
            }
            else
            {
               //Clear flag
               group->flag = FALSE;
               //Enter the Init Member state
               group->state = IGMP_HOST_GROUP_STATE_INIT_MEMBER;
            }
         }
      }
   }

   //Valid group?
   if(group != NULL)
   {
      //Any state change detected?
      if(group->filterMode != newFilterMode ||
         !ipv4CompareSrcAddrLists(&group->filter, newFilter))
      {
         //Merge the difference report resulting from the state change and the
         //pending report
         igmpHostMergeReports(group, newFilterMode, newFilter);

         //Save the new state
         group->filterMode = newFilterMode;
         group->filter = *newFilter;

         //Check host compatibility mode
         if(context->compatibilityMode <= IGMP_VERSION_2)
         {
            //The "non-existent" state is considered to have a filter mode of
            //INCLUDE and an empty source list
            if(group->filterMode == IP_FILTER_MODE_INCLUDE &&
               group->filter.numSources == 0)
            {
               //Send a Leave Group message if the flag is set
               if(group->flag)
               {
                  igmpHostSendLeaveGroup(context, group->addr);
               }

               //Delete the group
               igmpHostDeleteGroup(group);
            }
         }
         else
         {
            //Check group state
            if(group->state == IGMP_HOST_GROUP_STATE_INIT_MEMBER)
            {
               //The "non-existent" state is considered to have a filter mode
               //of INCLUDE and an empty source list
               if(group->filterMode == IP_FILTER_MODE_INCLUDE &&
                  group->filter.numSources == 0)
               {
                  //Delete the group
                  igmpHostDeleteGroup(group);
               }
            }
            else
            {
               //Send a State-Change report message
               igmpHostSendStateChangeReport(context);

               //To cover the possibility of the State-Change report being
               //missed by one or more multicast routers, it is retransmitted
               //[Robustness Variable] - 1 more times
               if(igmpHostGetRetransmitStatus(context))
               {
                  //Select a value in the range 0 - Unsolicited Report Interval
                  delay = igmpGetRandomDelay(IGMP_V3_UNSOLICITED_REPORT_INTERVAL);
                  //Start retransmission timer
                  netStartTimer(&context->stateChangeReportTimer, delay);
               }
               else
               {
                  //[Robustness Variable] State-Change reports have been sent
                  //by the host
                  netStopTimer(&context->stateChangeReportTimer);
               }

               //Delete groups in "non-existent" state
               igmpHostFlushUnusedGroups(context);
            }
         }
      }
   }
}


/**
 * @brief Process link state change
 * @param[in] context Pointer to the IGMP host context
 **/

void igmpHostLinkChangeEvent(IgmpHostContext *context)
{
   uint_t i;
   IgmpHostGroup *group;

   //The default host compatibility mode is IGMPv3
   context->compatibilityMode = IGMP_VERSION_3;

   //Stop timers
   netStopTimer(&context->igmpv1QuerierPresentTimer);
   netStopTimer(&context->igmpv2QuerierPresentTimer);
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
         //Reset parameters
         group->flag = FALSE;
         group->retransmitCount = 0;

#if (IPV4_MAX_MULTICAST_SOURCES > 0)
         //Clear source lists
         group->allow.numSources = 0;
         group->block.numSources = 0;
         group->queriedSources.numSources = 0;
#endif
         //Stop delay timer
         netStopTimer(&group->timer);

         //Enter the Init Member state
         group->state = IGMP_HOST_GROUP_STATE_INIT_MEMBER;
      }
   }

   //Delete groups in "non-existent" state
   igmpHostFlushUnusedGroups(context);
}

#endif
