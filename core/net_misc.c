/**
 * @file net_misc.c
 * @brief Helper functions for TCP/IP stack
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
#include "ipv4/auto_ip_misc.h"
#include "igmp/igmp_host.h"
#include "dhcp/dhcp_client_misc.h"
#include "dhcp/dhcp_server_misc.h"
#include "nat/nat_misc.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_routing.h"
#include "ipv6/ndp.h"
#include "ipv6/ndp_router_adv_misc.h"
#include "mld/mld_node.h"
#include "dhcpv6/dhcpv6_client_misc.h"
#include "dns/dns_cache.h"
#include "dns/dns_client.h"
#include "mdns/mdns_client.h"
#include "mdns/mdns_responder.h"
#include "mdns/mdns_common.h"
#include "dns_sd/dns_sd_responder.h"
#include "netbios/nbns_client.h"
#include "netbios/nbns_responder.h"
#include "netbios/nbns_common.h"
#include "llmnr/llmnr_client.h"
#include "llmnr/llmnr_responder.h"
#include "mibs/mib2_module.h"
#include "mibs/if_mib_module.h"
#include "debug.h"

//Default options passed to the stack (TX path)
const NetTxAncillary NET_DEFAULT_TX_ANCILLARY =
{
#if (UDP_SUPPORT == ENABLED)
   FALSE,         //Disable UDP checksum generation
#endif
   0,             //Time-to-live value
   0,             //Type-of-service value
   IP_DEFAULT_DF, //Do not fragment the IP packet
   FALSE,         //Do not send the packet via a router
   FALSE,         //Do not add an IP Router Alert option
#if (ETH_SUPPORT == ENABLED)
   {{{0}}},       //Source MAC address
   {{{0}}},       //Destination MAC address
#endif
#if (ETH_VLAN_SUPPORT == ENABLED)
   -1,            //VLAN priority (802.1Q)
   -1,            //Drop eligible indicator
#endif
#if (ETH_VMAN_SUPPORT == ENABLED)
   -1,            //VMAN priority (802.1ad)
   -1,            //Drop eligible indicator
#endif
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   0,             //Egress port identifier
   0,             //Egress port map
   FALSE,         //Override port state
#endif
#if (ETH_TIMESTAMP_SUPPORT == ENABLED)
   -1,            //Unique identifier for hardware time stamping
#endif
};

//Default options passed to the stack (RX path)
const NetRxAncillary NET_DEFAULT_RX_ANCILLARY =
{
   0,       //Time-to-live value
   0,       //Type-of-service value
#if (ETH_SUPPORT == ENABLED)
   {{{0}}}, //Source MAC address
   {{{0}}}, //Destination MAC address
   0,       //Ethernet type field
#endif
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   0,       //Ingress port identifier
#endif
#if (ETH_TIMESTAMP_SUPPORT == ENABLED)
   {0},     //Captured time stamp
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

      //Check whether the entry is available
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
   MIB2_IF_SET_TIME_TICKS(ifTable[interface->index].ifLastChange,
      osGetSystemTime64() / 10);
   IF_MIB_SET_TIME_TICKS(ifTable[interface->index].ifLastChange,
      osGetSystemTime64() / 10);

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

#if (DNS_SD_RESPONDER_SUPPORT == ENABLED)
   //Perform probing and announcing
   dnsSdResponderLinkChangeEvent(interface->dnsSdResponderContext);
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
      socket = &socketTable[i];

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

      //Check whether the entry is available
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

#if (IPV4_SUPPORT == ENABLED && (IGMP_HOST_SUPPORT == ENABLED || \
   IGMP_ROUTER_SUPPORT == ENABLED || IGMP_SNOOPING_SUPPORT == ENABLED))
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
      {
         autoIpTick(netInterface[i].autoIpContext);
      }

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
      {
         dhcpClientTick(netInterface[i].dhcpClientContext);
      }

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
      {
         dhcpServerTick(netInterface[i].dhcpServerContext);
      }

      //Reset tick counter
      dhcpServerTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && NAT_SUPPORT == ENABLED)
   //Increment tick counter
   natTickCounter += NET_TICK_INTERVAL;

   //Manage NAT related timers
   if(natTickCounter >= NAT_TICK_INTERVAL)
   {
      //NAT timer handler
      natTick(netContext.natContext);
      //Reset tick counter
      natTickCounter = 0;
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

#if (IPV6_SUPPORT == ENABLED && MLD_NODE_SUPPORT == ENABLED)
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
      {
         ndpRouterAdvTick(netInterface[i].ndpRouterAdvContext);
      }

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
      {
         dhcpv6ClientTick(netInterface[i].dhcpv6ClientContext);
      }

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
   NBNS_CLIENT_SUPPORT == ENABLED || LLMNR_CLIENT_SUPPORT == ENABLED)
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
      {
         mdnsResponderTick(netInterface[i].mdnsResponderContext);
      }

      //Reset tick counter
      mdnsResponderTickCounter = 0;
   }
#endif

#if (DNS_SD_RESPONDER_SUPPORT == ENABLED)
   //Increment tick counter
   dnsSdResponderTickCounter += NET_TICK_INTERVAL;

   //Manage DNS-SD probing and announcing
   if(dnsSdResponderTickCounter >= DNS_SD_RESPONDER_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         dnsSdResponderTick(netInterface[i].dnsSdResponderContext);
      }

      //Reset tick counter
      dnsSdResponderTickCounter = 0;
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


/**
 * @brief Start timer
 * @param[in] timer Pointer to the timer structure
 * @param[in] interval Time interval
 **/

void netStartTimer(NetTimer *timer, systime_t interval)
{
   //Start timer
   timer->startTime = osGetSystemTime();
   timer->interval = interval;
   timer->running = TRUE;
}


/**
 * @brief Stop timer
 * @param[in] timer Pointer to the timer structure
 **/

void netStopTimer(NetTimer *timer)
{
   //Stop timer
   timer->running = FALSE;
}


/**
 * @brief Check whether the timer is running
 * @param[in] timer Pointer to the timer structure
 * @return TRUE if the timer is running, else FALSE
 **/

bool_t netTimerRunning(NetTimer *timer)
{
   //Return TRUE if the timer is running
   return timer->running;
}


/**
 * @brief Check whether the timer has expired
 * @param[in] timer Pointer to the timer structure
 * @return TRUE if the timer has expired, else FALSE
 **/

bool_t netTimerExpired(NetTimer *timer)
{
   bool_t expired;
   systime_t time;

   //Initialize flag
   expired = FALSE;
   //Get current time
   time = osGetSystemTime();

   //Check whether the timer is running
   if(timer->running)
   {
      //Check whether the specified time interval has elapsed
      if((time - timer->startTime) >= timer->interval)
      {
         expired = TRUE;
      }
   }

   //Return TRUE if the timer has expired
   return expired;
}


/**
 * @brief Get the remaining value of the running timer
 * @param[in] timer Pointer to the timer structure
 * @return Remaining time
 **/

systime_t netGetRemainingTime(NetTimer *timer)
{
   systime_t time;
   systime_t remaining;

   //Initialize variable
   remaining = 0;
   //Get current time
   time = osGetSystemTime();

   //Check whether the timer is running
   if(timer->running)
   {
      //Calculate remaining time
      if((time - timer->startTime) < timer->interval)
      {
         remaining = timer->startTime + timer->interval - time;
      }
   }

   //Return remaining time
   return remaining;
}


/**
 * @brief Initialize random number generator
 **/

void netInitRand(void)
{
   uint_t i;
   NetRandState *state;
   uint8_t iv[10];

   //Point to the PRNG state
   state = &netContext.randState;

   //Increment invocation counter
   state->counter++;

   //Copy the EUI-64 identifier of the default interface
   eui64CopyAddr(iv, &netInterface[0].eui64);
   //Append the invocation counter
   STORE16BE(state->counter, iv + sizeof(Eui64));

   //Clear the 288-bit internal state
   osMemset(state->s, 0, 36);

   //Let (s1, s2, ..., s93) = (K1, ..., K80, 0, ..., 0)
   for(i = 0; i < 10; i++)
   {
      state->s[i] = netContext.randSeed[i];
   }

   //Load the 80-bit initialization vector
   for(i = 0; i < 10; i++)
   {
      state->s[12 + i] = iv[i];
   }

   //Let (s94, s95, ..., s177) = (IV1, ..., IV80, 0, ..., 0)
   for(i = 11; i < 22; i++)
   {
      state->s[i] = (state->s[i + 1] << 5) | (state->s[i] >> 3);
   }

   //Let (s178, s279, ..., s288) = (0, ..., 0, 1, 1, 1)
   NET_RAND_STATE_SET_BIT(state->s, 286, 1);
   NET_RAND_STATE_SET_BIT(state->s, 287, 1);
   NET_RAND_STATE_SET_BIT(state->s, 288, 1);

   //The state is rotated over 4 full cycles, without generating key stream bit
   for(i = 0; i < (4 * 288); i++)
   {
      netGenerateRandBit(state);
   }
}


/**
 * @brief Generate a random 32-bit value
 * @return Random value
 **/

uint32_t netGenerateRand(void)
{
   uint_t i;
   uint32_t value;

   //Initialize value
   value = 0;

   //Generate a random 32-bit value
   for(i = 0; i < 32; i++)
   {
      value |= netGenerateRandBit(&netContext.randState) << i;
   }

   //Return the random value
   return value + netContext.entropy;
}


/**
 * @brief Generate a random value in the specified range
 * @param[in] min Lower bound
 * @param[in] max Upper bound
 * @return Random value in the specified range
 **/

uint32_t netGenerateRandRange(uint32_t min, uint32_t max)
{
   uint32_t value;

   //Valid parameters?
   if(max > min)
   {
      //Pick up a random value in the given range
      value = min + (netGenerateRand() % (max - min + 1));
   }
   else
   {
      //Use default value
      value = min;
   }

   //Return the random value
   return value;
}


/**
 * @brief Get a string of random data
 * @param[out] data Buffer where to store random data
 * @param[in] length Number of random bytes to generate
 **/

void netGenerateRandData(uint8_t *data, size_t length)
{
   size_t i;
   size_t j;

   //Generate a string of random data
   for(i = 0; i < length; i++)
   {
      //Initialize value
      data[i] = 0;

      //Generate a random 8-bit value
      for(j = 0; j < 8; j++)
      {
         data[i] |= netGenerateRandBit(&netContext.randState) << j;
      }

      data[i] += netContext.entropy;
   }
}


/**
 * @brief Generate one random bit
 * @param[in] state Pointer to the PRNG state
 * @return Key stream bit
 **/

uint32_t netGenerateRandBit(NetRandState *state)
{
   uint_t i;
   uint8_t t1;
   uint8_t t2;
   uint8_t t3;
   uint8_t z;

   //Let t1 = s66 + s93
   t1 = NET_RAND_GET_BIT(state->s, 66);
   t1 ^= NET_RAND_GET_BIT(state->s, 93);

   //Let t2 = s162 + s177
   t2 = NET_RAND_GET_BIT(state->s, 162);
   t2 ^= NET_RAND_GET_BIT(state->s, 177);

   //Let t3 = s243 + s288
   t3 = NET_RAND_GET_BIT(state->s, 243);
   t3 ^= NET_RAND_GET_BIT(state->s, 288);

   //Generate a key stream bit z
   z = t1 ^ t2 ^ t3;

   //Let t1 = t1 + s91.s92 + s171
   t1 ^= NET_RAND_GET_BIT(state->s, 91) & NET_RAND_GET_BIT(state->s, 92);
   t1 ^= NET_RAND_GET_BIT(state->s, 171);

   //Let t2 = t2 + s175.s176 + s264
   t2 ^= NET_RAND_GET_BIT(state->s, 175) & NET_RAND_GET_BIT(state->s, 176);
   t2 ^= NET_RAND_GET_BIT(state->s, 264);

   //Let t3 = t3 + s286.s287 + s69
   t3 ^= NET_RAND_GET_BIT(state->s, 286) & NET_RAND_GET_BIT(state->s, 287);
   t3 ^= NET_RAND_GET_BIT(state->s, 69);

   //Rotate the internal state
   for(i = 35; i > 0; i--)
   {
      state->s[i] = (state->s[i] << 1) | (state->s[i - 1] >> 7);
   }

   state->s[0] = state->s[0] << 1;

   //Let s1 = t3
   NET_RAND_STATE_SET_BIT(state->s, 1, t3);
   //Let s94 = t1
   NET_RAND_STATE_SET_BIT(state->s, 94, t1);
   //Let s178 = t2
   NET_RAND_STATE_SET_BIT(state->s, 178, t2);

   //Return one bit of key stream
   return z;
}
