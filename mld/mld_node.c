/**
 * @file mld_node.c
 * @brief MLD node (Multicast Listener Discovery for IPv6)
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
 * MLD is used by an IPv6 router to discover the presence of multicast
 * listeners on its directly attached links, and to discover specifically
 * which multicast addresses are of interest to those neighboring nodes.
 * Refer to the following RFCs for complete details:
 * - RFC 2710: Multicast Listener Discovery (MLD) for IPv6
 * - RFC 3590: Source Address Selection for MLD Protocol
 * - RFC 3810: Multicast Listener Discovery Version 2 (MLDv2) for IPv6
 * - RFC 9777: Multicast Listener Discovery Version 2 (MLDv2) for IPv6
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
#include "ipv6/ipv6_misc.h"
#include "mld/mld_node.h"
#include "mld/mld_node_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && MLD_NODE_SUPPORT == ENABLED)


/**
 * @brief MLD node initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mldNodeInit(NetInterface *interface)
{
   MldNodeContext *context;

   //Point to the MLD node context
   context = &interface->mldNodeContext;

   //Clear the MLD node context
   osMemset(context, 0, sizeof(MldNodeContext));

   //Underlying network interface
   context->interface = interface;
   //The default host compatibility mode is MLDv2
   context->compatibilityMode = MLD_VERSION_2;

   //In order to ensure interoperability, hosts maintain an Older Version
   //Querier Present timer per interface
   netStopTimer(&context->olderVersionQuerierPresentTimer);

   //A timer per interface is used for scheduling responses to General Queries
   netStopTimer(&context->generalQueryTimer);

   //A timer is used to retransmit State-Change reports
   netStopTimer(&context->stateChangeReportTimer);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief MLD node timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * handle MLD related timers
 *
 * @param[in] context Pointer to the MLD node context
 **/

void mldNodeTick(MldNodeContext *context)
{
   uint_t i;
   systime_t delay;
   MldNodeGroup *group;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->interface;

   //In order to be compatible with MLDv1 routers, MLDv2 hosts must operate in
   //version 1 compatibility mode (refer to RFC 3810, section 8.2.1)
   if(netTimerExpired(&context->olderVersionQuerierPresentTimer))
   {
      //Stop Older Version Querier Present timer
      netStopTimer(&context->olderVersionQuerierPresentTimer);

      //If the Older Version Querier Present timer expires, the host switches
      //back to Host Compatibility Mode of MLDv2
      mldNodeChangeCompatibilityMode(context, MLD_VERSION_2);
   }

   //Check host compatibility mode
   if(context->compatibilityMode == MLD_VERSION_1)
   {
      //Loop through multicast groups
      for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
      {
         //Point to the current group
         group = &context->groups[i];

         //Check group state
         if(group->state == MLD_NODE_GROUP_STATE_INIT_LISTENER)
         {
            //Valid link-local address assigned to the interface?
            if(interface->linkState &&
               ipv6GetLinkLocalAddrState(interface) == IPV6_ADDR_STATE_PREFERRED)
            {
               //When a node starts listening to a multicast address on an
               //interface, it should immediately transmit an unsolicited Report
               //for that address on that interface
               mldNodeSendListenerReport(context, &group->addr);

               //Start delay timer
               netStartTimer(&group->timer, MLD_UNSOLICITED_REPORT_INTERVAL);

               //Set flag
               group->flag = TRUE;
               //Enter the Delaying Listener state
               group->state = MLD_NODE_GROUP_STATE_DELAYING_LISTENER;
            }
         }
         else if(group->state == MLD_NODE_GROUP_STATE_DELAYING_LISTENER)
         {
            //Delay timer expired?
            if(netTimerExpired(&group->timer))
            {
               //Send a Multicast Listener Report message for the group on the
               //interface
               mldNodeSendListenerReport(context, &group->addr);

               //Stop delay timer
               netStopTimer(&group->timer);

               //Set flag
               group->flag = TRUE;
               //Switch to the Idle Listener state
               group->state = MLD_NODE_GROUP_STATE_IDLE_LISTENER;
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
         mldNodeSendCurrentStateReport(context, &IPV6_UNSPECIFIED_ADDR);

         //Stop interface timer
         netStopTimer(&context->generalQueryTimer);
      }

      //If the expired timer is a group timer, then a single Current-State
      //Record is sent for the corresponding group address
      for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
      {
         //Point to the current group
         group = &context->groups[i];

         //Check group state
         if(group->state == MLD_NODE_GROUP_STATE_INIT_LISTENER)
         {
            //Valid link-local address assigned to the interface?
            if(interface->linkState &&
               ipv6GetLinkLocalAddrState(interface) == IPV6_ADDR_STATE_PREFERRED)
            {
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
               //Once a valid link-local address is available, a node should
               //generate new MLD Report messages for all multicast addresses
               //joined on the interface (refer to RFC 3590, section 4)
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
                     group->allow.sources[j].retransmitCount = MLD_ROBUSTNESS_VARIABLE;
                  }

                  //Send a State-Change report immediately
                  netStartTimer(&context->stateChangeReportTimer, 0);
               }
               else if(group->filterMode == IP_FILTER_MODE_EXCLUDE)
               {
                  //The State-Change report will include a TO_EX record
                  group->retransmitCount = MLD_ROBUSTNESS_VARIABLE;
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
               //Once a valid link-local address is available, a node should
               //generate new MLD Report messages for all multicast addresses
               //joined on the interface (refer to RFC 3590, section 4)
               if(group->filterMode == IP_FILTER_MODE_EXCLUDE)
               {
                  //The State-Change report will include a TO_EX record
                  group->retransmitCount = MLD_ROBUSTNESS_VARIABLE;
                  //Send a State-Change report immediately
                  netStartTimer(&context->stateChangeReportTimer, 0);
               }
#endif
               //Enter the Idle Listener state
               group->state = MLD_NODE_GROUP_STATE_IDLE_LISTENER;
            }
         }
         else if(group->state == MLD_NODE_GROUP_STATE_IDLE_LISTENER)
         {
            //Check whether the group timer has expired
            if(netTimerExpired(&group->timer))
            {
               //Send Current-State report message
               mldNodeSendCurrentStateReport(context, &group->addr);

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
         mldNodeSendStateChangeReport(context, &IPV6_UNSPECIFIED_ADDR);

         //Retransmission state needs to be maintained until [Robustness
         //Variable] State-Change reports have been sent by the host
         if(mldNodeGetRetransmitStatus(context))
         {
            //Select a value in the range 0 - Unsolicited Report Interval
            delay = mldGetRandomDelay(MLD_V2_UNSOLICITED_REPORT_INTERVAL);
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
         mldNodeFlushUnusedGroups(context);
      }
   }
}


/**
 * @brief Process multicast reception state change
 * @param[in] context Pointer to the MLD node context
 * @param[in] groupAddr Multicast group address
 * @param[in] newFilterMode New filter mode for the affected group
 * @param[in] newFilter New interface state for the affected group
 **/

void mldNodeStateChangeEvent(MldNodeContext *context, const Ipv6Addr *groupAddr,
   IpFilterMode newFilterMode, const Ipv6SrcAddrList *newFilter)
{
   systime_t delay;
   MldNodeGroup *group;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->interface;

   //Search the list of groups for the specified multicast address
   group = mldNodeFindGroup(context, groupAddr);

   //Check whether the interface has reception state for that group address
   if(newFilterMode == IP_FILTER_MODE_EXCLUDE || newFilter->numSources > 0)
   {
      //No matching group found?
      if(group == NULL)
      {
         //Create a new group
         group = mldNodeCreateGroup(context, groupAddr);

         //Entry successfully created?
         if(group != NULL)
         {
            //Valid link-local address assigned to the interface?
            if(interface->linkState &&
               ipv6GetLinkLocalAddrState(interface) == IPV6_ADDR_STATE_PREFERRED)
            {
               //Check host compatibility mode
               if(context->compatibilityMode == MLD_VERSION_1)
               {
                  //When a node starts listening to a multicast address on an
                  //interface, it should immediately transmit an unsolicited
                  //Report for that address on that interface
                  mldNodeSendListenerReport(context, &group->addr);

                  //Start delay timer
                  netStartTimer(&group->timer, MLD_UNSOLICITED_REPORT_INTERVAL);

                  //Set flag
                  group->flag = TRUE;
                  //Enter the Delaying Listener state
                  group->state = MLD_NODE_GROUP_STATE_DELAYING_LISTENER;
               }
               else
               {
                  //Enter the Idle Listener state
                  group->state = MLD_NODE_GROUP_STATE_IDLE_LISTENER;
               }
            }
            else
            {
               //Clear flag
               group->flag = FALSE;
               //Enter the Init Listener state
               group->state = MLD_NODE_GROUP_STATE_INIT_LISTENER;
            }
         }
      }
   }

   //Valid group?
   if(group != NULL)
   {
      //Any state change detected?
      if(group->filterMode != newFilterMode ||
         !ipv6CompareSrcAddrLists(&group->filter, newFilter))
      {
         //Merge the difference report resulting from the state change and the
         //pending report
         mldNodeMergeReports(group, newFilterMode, newFilter);

         //Save the new state
         group->filterMode = newFilterMode;
         group->filter = *newFilter;

         //Check host compatibility mode
         if(context->compatibilityMode == MLD_VERSION_1)
         {
            //The "non-existent" state is considered to have a filter mode of
            //INCLUDE and an empty source list
            if(group->filterMode == IP_FILTER_MODE_INCLUDE &&
               group->filter.numSources == 0)
            {
               //Send a Multicast Listener Done message if the flag is set
               if(group->flag)
               {
                  mldNodeSendListenerDone(context, &group->addr);
               }

               //Delete the group
               mldNodeDeleteGroup(group);
            }
         }
         else
         {
            //Check group state
            if(group->state == MLD_NODE_GROUP_STATE_INIT_LISTENER)
            {
               //The "non-existent" state is considered to have a filter mode
               //of INCLUDE and an empty source list
               if(group->filterMode == IP_FILTER_MODE_INCLUDE &&
                  group->filter.numSources == 0)
               {
                  //Delete the group
                  mldNodeDeleteGroup(group);
               }
            }
            else
            {
               //Send a State-Change report message
               mldNodeSendStateChangeReport(context, &IPV6_UNSPECIFIED_ADDR);

               //To cover the possibility of the State-Change report being
               //missed by one or more multicast routers, it is retransmitted
               //[Robustness Variable] - 1 more times
               if(mldNodeGetRetransmitStatus(context))
               {
                  //Select a value in the range 0 - Unsolicited Report Interval
                  delay = mldGetRandomDelay(MLD_V2_UNSOLICITED_REPORT_INTERVAL);
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
               mldNodeFlushUnusedGroups(context);
            }
         }
      }
   }
}


/**
 * @brief Callback function for link change event
 * @param[in] context Pointer to the MLD node context
 **/

void mldNodeLinkChangeEvent(MldNodeContext *context)
{
   uint_t i;
   MldNodeGroup *group;

   //The default host compatibility mode is MLDv2
   context->compatibilityMode = MLD_VERSION_2;

   //Stop timers
   netStopTimer(&context->olderVersionQuerierPresentTimer);
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
         //Reset parameters
         group->flag = FALSE;
         group->retransmitCount = 0;

#if (IPV6_MAX_MULTICAST_SOURCES > 0)
         //Clear source lists
         group->allow.numSources = 0;
         group->block.numSources = 0;
         group->queriedSources.numSources = 0;
#endif
         //Stop delay timer
         netStopTimer(&group->timer);

         //Enter the Init Listener state
         group->state = MLD_NODE_GROUP_STATE_INIT_LISTENER;
      }
   }

   //Delete groups in "non-existent" state
   mldNodeFlushUnusedGroups(context);
}

#endif
