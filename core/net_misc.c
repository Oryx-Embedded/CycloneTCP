/**
 * @file net_misc.c
 * @brief Helper functions for TCP/IP stack
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2020 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.8
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/net_misc.h"
#include "core/socket.h"
#include "core/raw_socket.h"
#include "core/tcp_timer.h"
#include "core/tcp_misc.h"
#include "core/ethernet.h"
#include "ipv4/arp.h"
#include "ipv4/ipv4.h"
#include "ipv4/ipv4_routing.h"
#include "ipv4/igmp.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_routing.h"
#include "ipv6/mld.h"
#include "ipv6/ndp.h"
#include "ipv6/ndp_router_adv.h"
#include "dhcp/dhcp_client.h"
#include "dhcp/dhcp_server.h"
#include "dns/dns_cache.h"
#include "dns/dns_client.h"
#include "mdns/mdns_client.h"
#include "mdns/mdns_responder.h"
#include "mdns/mdns_common.h"
#include "dns_sd/dns_sd.h"
#include "netbios/nbns_client.h"
#include "netbios/nbns_responder.h"
#include "netbios/nbns_common.h"
#include "llmnr/llmnr_responder.h"
#include "mibs/mib2_module.h"
#include "mibs/if_mib_module.h"
#include "debug.h"

//Default options passed to the stack (TX path)
const NetTxAncillary NET_DEFAULT_TX_ANCILLARY =
{
   0,     //Time-to-live value
   FALSE, //Do not send the packet via a router
#if (IP_DIFF_SERV_SUPPORT == ENABLED)
   0,     //Differentiated services codepoint
#endif
#if (ETH_SUPPORT == ENABLED)
   {0},  //Source MAC address
   {0},  //Destination MAC address
#endif
#if (ETH_VLAN_SUPPORT == ENABLED)
   -1,    //VLAN priority (802.1Q)
   -1,    //Drop eligible indicator
#endif
#if (ETH_VMAN_SUPPORT == ENABLED)
   -1,    //VMAN priority (802.1ad)
   -1,    //Drop eligible indicator
#endif
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   0,     //Switch port identifier
#endif
#if (ETH_TIMESTAMP_SUPPORT == ENABLED)
   -1,    //Unique identifier for hardware time stamping
#endif
};

//Default options passed to the stack (RX path)
const NetRxAncillary NET_DEFAULT_RX_ANCILLARY =
{
   0,   //Time-to-live value
#if (ETH_SUPPORT == ENABLED)
   {0},  ///<Source MAC address
   {0},  ///<Destination MAC address
#endif
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   0,   //Switch port identifier
#endif
#if (ETH_TIMESTAMP_SUPPORT == ENABLED)
   {0}, //Captured time stamp
#endif
};


/**
 * @brief Register link change callback
 * @param[in] interface Underlying network interface
 * @param[in] callback Callback function to be called when the link state changed
 * @param[in] param Callback function parameter
 * @return Error code
 **/

error_t netAttachLinkChangeCallback(NetInterface *interface,
   NetLinkChangeCallback callback, void *param)
{
   uint_t i;
   NetLinkChangeCallbackEntry *entry;

   //Loop through the table
   for(i = 0; i < NET_MAX_LINK_CHANGE_CALLBACKS; i++)
   {
      //Point to the current entry
      entry = &netContext.linkChangeCallbacks[i];

      //Check whether the entry is currently in available
      if(entry->callback == NULL)
      {
         //Create a new entry
         entry->interface = interface;
         entry->callback = callback;
         entry->param = param;

         //Successful processing
         return NO_ERROR;
      }
   }

   //The table runs out of space
   return ERROR_OUT_OF_RESOURCES;
}


/**
 * @brief Unregister link change callback
 * @param[in] interface Underlying network interface
 * @param[in] callback Callback function to be unregistered
 * @param[in] param Callback function parameter
 * @return Error code
 **/

error_t netDetachLinkChangeCallback(NetInterface *interface,
   NetLinkChangeCallback callback, void *param)
{
   uint_t i;
   NetLinkChangeCallbackEntry *entry;

   //Loop through the table
   for(i = 0; i < NET_MAX_LINK_CHANGE_CALLBACKS; i++)
   {
      //Point to the current entry
      entry = &netContext.linkChangeCallbacks[i];

      //Check whether the current entry matches the specified callback function
      if(entry->interface == interface && entry->callback == callback &&
         entry->param == param)
      {
         //Unregister callback function
         entry->interface = NULL;
         entry->callback = NULL;
         entry->param = NULL;
      }
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Process link state change event
 * @param[in] interface Underlying network interface
 **/

void netProcessLinkChange(NetInterface *interface)
{
   uint_t i;
   Socket *socket;

   //Check link state
   if(interface->linkState)
   {
      //Display link state
      TRACE_INFO("Link is up (%s)...\r\n", interface->name);

      //Display link speed
      if(interface->linkSpeed == NIC_LINK_SPEED_1GBPS)
      {
         //1000BASE-T
         TRACE_INFO("  Link speed = 1000 Mbps\r\n");
      }
      else if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
      {
         //100BASE-TX
         TRACE_INFO("  Link speed = 100 Mbps\r\n");
      }
      else if(interface->linkSpeed == NIC_LINK_SPEED_10MBPS)
      {
         //10BASE-T
         TRACE_INFO("  Link speed = 10 Mbps\r\n");
      }
      else if(interface->linkSpeed != NIC_LINK_SPEED_UNKNOWN)
      {
         //10BASE-T
         TRACE_INFO("  Link speed = %" PRIu32 " bps\r\n",
            interface->linkSpeed);
      }

      //Display duplex mode
      if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
      {
         //1000BASE-T
         TRACE_INFO("  Duplex mode = Full-Duplex\r\n");
      }
      else if(interface->duplexMode == NIC_HALF_DUPLEX_MODE)
      {
         //100BASE-TX
         TRACE_INFO("  Duplex mode = Half-Duplex\r\n");
      }
   }
   else
   {
      //Display link state
      TRACE_INFO("Link is down (%s)...\r\n", interface->name);
   }

   //The time at which the interface entered its current operational state
   MIB2_SET_TIME_TICKS(ifGroup.ifTable[interface->index].ifLastChange,
      osGetSystemTime() / 10);
   IF_MIB_SET_TIME_TICKS(ifTable[interface->index].ifLastChange,
      osGetSystemTime() / 10);

#if (IPV4_SUPPORT == ENABLED)
   //Notify IPv4 of link state changes
   ipv4LinkChangeEvent(interface);
#endif

#if (IPV6_SUPPORT == ENABLED)
   //Notify IPv6 of link state changes
   ipv6LinkChangeEvent(interface);
#endif

#if (DNS_CLIENT_SUPPORT == ENABLED || MDNS_CLIENT_SUPPORT == ENABLED || \
   NBNS_CLIENT_SUPPORT == ENABLED)
   //Flush DNS cache
   dnsFlushCache(interface);
#endif

#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   //Perform probing and announcing
   mdnsResponderLinkChangeEvent(interface->mdnsResponderContext);
#endif

#if (DNS_SD_SUPPORT == ENABLED)
   //Perform probing and announcing
   dnsSdLinkChangeEvent(interface->dnsSdContext);
#endif

   //Loop through the link change callback table
   for(i = 0; i < NET_MAX_LINK_CHANGE_CALLBACKS; i++)
   {
      NetLinkChangeCallbackEntry *entry;

      //Point to the current entry
      entry = &netContext.linkChangeCallbacks[i];

      //Any registered callback?
      if(entry->callback != NULL)
      {
         //Check whether the network interface matches the current entry
         if(entry->interface == NULL || entry->interface == interface)
         {
            //Invoke user callback function
            entry->callback(interface, interface->linkState, entry->param);
         }
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
}


/**
 * @brief Register timer callback
 * @param[in] period Timer reload value, in milliseconds
 * @param[in] callback Callback function to be called when the timer expires
 * @param[in] param Callback function parameter
 * @return Error code
 **/

error_t netAttachTimerCallback(systime_t period, NetTimerCallback callback,
   void *param)
{
   uint_t i;
   NetTimerCallbackEntry *entry;

   //Loop through the table
   for(i = 0; i < NET_MAX_TIMER_CALLBACKS; i++)
   {
      //Point to the current entry
      entry = &netContext.timerCallbacks[i];

      //Check whether the entry is currently in available
      if(entry->callback == NULL)
      {
         //Create a new entry
         entry->timerValue = 0;
         entry->timerPeriod = period;
         entry->callback = callback;
         entry->param = param;

         //Successful processing
         return NO_ERROR;
      }
   }

   //The table runs out of space
   return ERROR_OUT_OF_RESOURCES;
}


/**
 * @brief Unregister timer callback
 * @param[in] callback Callback function to be unregistered
 * @param[in] param Callback function parameter
 * @return Error code
 **/

error_t netDetachTimerCallback(NetTimerCallback callback, void *param)
{
   uint_t i;
   NetTimerCallbackEntry *entry;

   //Loop through the table
   for(i = 0; i < NET_MAX_TIMER_CALLBACKS; i++)
   {
      //Point to the current entry
      entry = &netContext.timerCallbacks[i];

      //Check whether the current entry matches the specified callback function
      if(entry->callback == callback && entry->param == param)
      {
         //Unregister callback function
         entry->timerValue = 0;
         entry->timerPeriod = 0;
         entry->callback = NULL;
         entry->param = NULL;
      }
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Manage TCP/IP timers
 **/

void netTick(void)
{
   uint_t i;
   NetTimerCallbackEntry *entry;

   //Increment tick counter
   nicTickCounter += NET_TICK_INTERVAL;

   //Handle periodic operations such as polling the link state
   if(nicTickCounter >= NIC_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            nicTick(&netInterface[i]);
      }

      //Reset tick counter
      nicTickCounter = 0;
   }

#if (PPP_SUPPORT == ENABLED)
   //Increment tick counter
   pppTickCounter += NET_TICK_INTERVAL;

   //Manage PPP related timers
   if(pppTickCounter >= PPP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            pppTick(&netInterface[i]);
      }

      //Reset tick counter
      pppTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && ETH_SUPPORT == ENABLED)
   //Increment tick counter
   arpTickCounter += NET_TICK_INTERVAL;

   //Manage ARP cache
   if(arpTickCounter >= ARP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            arpTick(&netInterface[i]);
      }

      //Reset tick counter
      arpTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && IPV4_FRAG_SUPPORT == ENABLED)
   //Increment tick counter
   ipv4FragTickCounter += NET_TICK_INTERVAL;

   //Handle IPv4 fragment reassembly timeout
   if(ipv4FragTickCounter >= IPV4_FRAG_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            ipv4FragTick(&netInterface[i]);
      }

      //Reset tick counter
      ipv4FragTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && IGMP_SUPPORT == ENABLED)
   //Increment tick counter
   igmpTickCounter += NET_TICK_INTERVAL;

   //Handle IGMP related timers
   if(igmpTickCounter >= IGMP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            igmpTick(&netInterface[i]);
      }

      //Reset tick counter
      igmpTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && AUTO_IP_SUPPORT == ENABLED)
   //Increment tick counter
   autoIpTickCounter += NET_TICK_INTERVAL;

   //Handle Auto-IP related timers
   if(autoIpTickCounter >= AUTO_IP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         autoIpTick(netInterface[i].autoIpContext);

      //Reset tick counter
      autoIpTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && DHCP_CLIENT_SUPPORT == ENABLED)
   //Increment tick counter
   dhcpClientTickCounter += NET_TICK_INTERVAL;

   //Handle DHCP client related timers
   if(dhcpClientTickCounter >= DHCP_CLIENT_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         dhcpClientTick(netInterface[i].dhcpClientContext);

      //Reset tick counter
      dhcpClientTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && DHCP_SERVER_SUPPORT == ENABLED)
   //Increment tick counter
   dhcpServerTickCounter += NET_TICK_INTERVAL;

   //Handle DHCP server related timers
   if(dhcpServerTickCounter >= DHCP_SERVER_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         dhcpServerTick(netInterface[i].dhcpServerContext);

      //Reset tick counter
      dhcpServerTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && IPV6_FRAG_SUPPORT == ENABLED)
   //Increment tick counter
   ipv6FragTickCounter += NET_TICK_INTERVAL;

   //Handle IPv6 fragment reassembly timeout
   if(ipv6FragTickCounter >= IPV6_FRAG_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            ipv6FragTick(&netInterface[i]);
      }

      //Reset tick counter
      ipv6FragTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && MLD_SUPPORT == ENABLED)
   //Increment tick counter
   mldTickCounter += NET_TICK_INTERVAL;

   //Handle MLD related timers
   if(mldTickCounter >= MLD_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            mldTick(&netInterface[i]);
      }

      //Reset tick counter
      mldTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && NDP_SUPPORT == ENABLED)
   //Increment tick counter
   ndpTickCounter += NET_TICK_INTERVAL;

   //Handle NDP related timers
   if(ndpTickCounter >= NDP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            ndpTick(&netInterface[i]);
      }

      //Reset tick counter
      ndpTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && NDP_ROUTER_ADV_SUPPORT == ENABLED)
   //Increment tick counter
   ndpRouterAdvTickCounter += NET_TICK_INTERVAL;

   //Handle RA service related timers
   if(ndpRouterAdvTickCounter >= NDP_ROUTER_ADV_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         ndpRouterAdvTick(netInterface[i].ndpRouterAdvContext);

      //Reset tick counter
      ndpRouterAdvTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && DHCPV6_CLIENT_SUPPORT == ENABLED)
   //Increment tick counter
   dhcpv6ClientTickCounter += NET_TICK_INTERVAL;

   //Handle DHCPv6 client related timers
   if(dhcpv6ClientTickCounter >= DHCPV6_CLIENT_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         dhcpv6ClientTick(netInterface[i].dhcpv6ClientContext);

      //Reset tick counter
      dhcpv6ClientTickCounter = 0;
   }
#endif

#if (TCP_SUPPORT == ENABLED)
   //Increment tick counter
   tcpTickCounter += NET_TICK_INTERVAL;

   //Manage TCP related timers
   if(tcpTickCounter >= TCP_TICK_INTERVAL)
   {
      //TCP timer handler
      tcpTick();
      //Reset tick counter
      tcpTickCounter = 0;
   }
#endif

#if (DNS_CLIENT_SUPPORT == ENABLED || MDNS_CLIENT_SUPPORT == ENABLED || \
   NBNS_CLIENT_SUPPORT == ENABLED)
   //Increment tick counter
   dnsTickCounter += NET_TICK_INTERVAL;

   //Manage DNS cache
   if(dnsTickCounter >= DNS_TICK_INTERVAL)
   {
      //DNS timer handler
      dnsTick();
      //Reset tick counter
      dnsTickCounter = 0;
   }
#endif

#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   //Increment tick counter
   mdnsResponderTickCounter += NET_TICK_INTERVAL;

   //Manage mDNS probing and announcing
   if(mdnsResponderTickCounter >= MDNS_RESPONDER_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         mdnsResponderTick(netInterface[i].mdnsResponderContext);

      //Reset tick counter
      mdnsResponderTickCounter = 0;
   }
#endif

#if (DNS_SD_SUPPORT == ENABLED)
   //Increment tick counter
   dnsSdTickCounter += NET_TICK_INTERVAL;

   //Manage DNS-SD probing and announcing
   if(dnsSdTickCounter >= DNS_SD_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         dnsSdTick(netInterface[i].dnsSdContext);

      //Reset tick counter
      dnsSdTickCounter = 0;
   }
#endif

   //Loop through the timer callback table
   for(i = 0; i < NET_MAX_TIMER_CALLBACKS; i++)
   {
      //Point to the current entry
      entry = &netContext.timerCallbacks[i];

      //Any registered callback?
      if(entry->callback != NULL)
      {
         //Increment timer value
         entry->timerValue += NET_TICK_INTERVAL;

         //Timer period elapsed?
         if(entry->timerValue >= entry->timerPeriod)
         {
            //Invoke user callback function
            entry->callback(entry->param);
            //Reload timer
            entry->timerValue = 0;
         }
      }
   }
}
