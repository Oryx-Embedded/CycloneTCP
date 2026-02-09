/**
 * @file net.h
 * @brief TCP/IP stack core
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2026 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.6.0
 **/

#ifndef _NET_H
#define _NET_H

//Forward declaration of NetContext structure
struct _NetContext;
#define NetContext struct _NetContext

//Forward declaration of NetInterface structure
struct _NetInterface;
#define NetInterface struct _NetInterface

//Dependencies
#include "os_port.h"
#include "net_config.h"
#include "core/net_legacy.h"
#include "core/net_mem.h"
#include "core/net_misc.h"
#include "core/nic.h"
#include "core/ethernet.h"
#include "core/ip.h"
#include "ipv4/ipv4.h"
#include "ipv4/ipv4_frag.h"
#include "ipv4/ipv4_routing.h"
#include "ipv4/icmp.h"
#include "ipv4/arp.h"
#include "ipv4/auto_ip.h"
#include "igmp/igmp_host.h"
#include "igmp/igmp_router.h"
#include "igmp/igmp_snooping.h"
#include "dhcp/dhcp_client.h"
#include "dhcp/dhcp_server.h"
#include "nat/nat.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_frag.h"
#include "ipv6/ipv6_routing.h"
#include "ipv6/icmpv6.h"
#include "ipv6/ndp.h"
#include "ipv6/ndp_router_adv.h"
#include "ipv6/slaac.h"
#include "mld/mld_node.h"
#include "dhcpv6/dhcpv6_client.h"
#include "dns/dns_client.h"
#include "mdns/mdns_responder.h"
#include "mdns/mdns_common.h"
#include "dns_sd/dns_sd_responder.h"
#include "ppp/ppp.h"
#include "cpu_endian.h"
#include "error.h"


/*
 * CycloneTCP Open is licensed under GPL version 2. In particular:
 *
 * - If you link your program to CycloneTCP Open, the result is a derivative
 *   work that can only be distributed under the same GPL license terms.
 *
 * - If additions or changes to CycloneTCP Open are made, the result is a
 *   derivative work that can only be distributed under the same license terms.
 *
 * - The GPL license requires that you make the source code available to
 *   whoever you make the binary available to.
 *
 * - If you sell or distribute a hardware product that runs CycloneTCP Open,
 *   the GPL license requires you to provide public and full access to all
 *   source code on a nondiscriminatory basis.
 *
 * If you fully understand and accept the terms of the GPL license, then edit
 * the os_port_config.h header and add the following directive:
 *
 * #define GPL_LICENSE_TERMS_ACCEPTED
 */

#ifndef GPL_LICENSE_TERMS_ACCEPTED
   #error Before compiling CycloneTCP Open, you must accept the terms of the GPL license
#endif

//Version string
#define CYCLONE_TCP_VERSION_STRING "2.6.0"
//Major version
#define CYCLONE_TCP_MAJOR_VERSION 2
//Minor version
#define CYCLONE_TCP_MINOR_VERSION 6
//Revision number
#define CYCLONE_TCP_REV_NUMBER 0

//RTOS support
#ifndef NET_RTOS_SUPPORT
   #define NET_RTOS_SUPPORT ENABLED
#elif (NET_RTOS_SUPPORT != ENABLED && NET_RTOS_SUPPORT != DISABLED)
   #error NET_RTOS_SUPPORT parameter is not valid
#endif

//Network interface statistics support
#ifndef NET_IF_STATS_SUPPORT
#if (MIB2_SUPPORT == ENABLED || IF_MIB_SUPPORT == ENABLED)
   #define NET_IF_STATS_SUPPORT ENABLED
#else
   #define NET_IF_STATS_SUPPORT DISABLED
#endif
#elif (NET_IF_STATS_SUPPORT != ENABLED && NET_IF_STATS_SUPPORT != DISABLED)
   #error NET_IF_STATS_SUPPORT parameter is not valid
#endif

//Loopback interface support
#ifndef NET_LOOPBACK_IF_SUPPORT
   #define NET_LOOPBACK_IF_SUPPORT DISABLED
#elif (NET_LOOPBACK_IF_SUPPORT != ENABLED && NET_LOOPBACK_IF_SUPPORT != DISABLED)
   #error NET_LOOPBACK_IF_SUPPORT parameter is not valid
#endif

//Maximum number of link change callback functions that can be registered
#ifndef NET_MAX_LINK_CHANGE_CALLBACKS
   #define NET_MAX_LINK_CHANGE_CALLBACKS 6
#elif (NET_MAX_LINK_CHANGE_CALLBACKS < 1)
   #error NET_MAX_LINK_CHANGE_CALLBACKS parameter is not valid
#endif

//Maximum number of timer callback functions that can be registered
#ifndef NET_MAX_TIMER_CALLBACKS
   #define NET_MAX_TIMER_CALLBACKS 6
#elif (NET_MAX_TIMER_CALLBACKS < 1)
   #error NET_MAX_TIMER_CALLBACKS parameter is not valid
#endif

//Maximum length of interface name
#ifndef NET_MAX_IF_NAME_LEN
   #define NET_MAX_IF_NAME_LEN 8
#elif (NET_MAX_IF_NAME_LEN < 1)
   #error NET_MAX_IF_NAME_LEN parameter is not valid
#endif

//Maximum length of host name
#ifndef NET_MAX_HOSTNAME_LEN
   #define NET_MAX_HOSTNAME_LEN 24
#elif (NET_MAX_HOSTNAME_LEN < 1)
   #error NET_MAX_HOSTNAME_LEN parameter is not valid
#endif

//Size of the seed
#ifndef NET_RAND_SEED_SIZE
   #define NET_RAND_SEED_SIZE 16
#elif (NET_RAND_SEED_SIZE < 10)
   #error NET_RAND_SEED_SIZE parameter is not valid
#endif

//Stack size required to run the TCP/IP task
#ifndef NET_TASK_STACK_SIZE
   #define NET_TASK_STACK_SIZE 650
#elif (NET_TASK_STACK_SIZE < 1)
   #error NET_TASK_STACK_SIZE parameter is not valid
#endif

//Priority at which the TCP/IP task should run
#ifndef NET_TASK_PRIORITY
   #define NET_TASK_PRIORITY OS_TASK_PRIORITY_HIGH
#endif

//TCP/IP stack tick interval
#ifndef NET_TICK_INTERVAL
   #define NET_TICK_INTERVAL 100
#elif (NET_TICK_INTERVAL < 10)
   #error NET_TICK_INTERVAL parameter is not valid
#endif

//Get system tick count
#ifndef netGetSystemTickCount
   #define netGetSystemTickCount() osGetSystemTime()
#endif

//Network interface statistics
#if (NET_IF_STATS_SUPPORT == ENABLED)
   #define NET_IF_STATS_SET_TIME_TICKS(name, value) interface->ifStats.name = value
   #define NET_IF_STATS_INC_COUNTER32(name, value) interface->ifStats.name += value
   #define NET_IF_STATS_INC_COUNTER64(name, value) interface->ifStats.name += value
#else
   #define NET_IF_STATS_SET_TIME_TICKS(name, value)
   #define NET_IF_STATS_INC_COUNTER32(name, value)
   #define NET_IF_STATS_INC_COUNTER64(name, value)
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Network interface statistics
 **/

typedef struct
{
   uint32_t lastChange;
   uint64_t inOctets;
   uint64_t inUcastPkts;
   uint32_t inNUcastPkts;
   uint64_t inMulticastPkts;
   uint64_t inBroadcastPkts;
   uint32_t inUnknownProtos;
   uint32_t inDiscards;
   uint32_t inErrors;
   uint64_t outOctets;
   uint64_t outUcastPkts;
   uint32_t outNUcastPkts;
   uint64_t outMulticastPkts;
   uint64_t outBroadcastPkts;
   uint32_t outDiscards;
   uint32_t outErrors;
} NetIfStats;


/**
 * @brief Network interface
 **/

struct _NetInterface
{
   NetContext *netContext;                        ///<TCP/IP stack context
   uint_t index;                                  ///<Zero-based index
   uint32_t id;                                   ///<A unique number identifying the interface
   Eui64 eui64;                                   ///<EUI-64 interface identifier
   char_t name[NET_MAX_IF_NAME_LEN + 1];          ///<A unique name identifying the interface
   char_t hostname[NET_MAX_HOSTNAME_LEN + 1];     ///<Host name
   const NicDriver *nicDriver;                    ///<NIC driver
   const SpiDriver *spiDriver;                    ///<Underlying SPI driver
   const UartDriver *uartDriver;                  ///<Underlying UART driver
   const ExtIntDriver *extIntDriver;              ///<External interrupt line driver
   uint8_t nicContext[NIC_CONTEXT_SIZE];          ///<Driver specific context
   OsEvent nicTxEvent;                            ///<Network controller TX event
   bool_t nicEvent;                               ///<A NIC event is pending
   NicLinkState adminLinkState;                   ///<Administrative link state
   bool_t linkState;                              ///<Link state
   uint32_t linkSpeed;                            ///<Link speed
   NicDuplexMode duplexMode;                      ///<Duplex mode
   bool_t configured;                             ///<Configuration done
   systime_t initialRto;                          ///<TCP initial retransmission timeout
#if (ETH_SUPPORT == ENABLED)
   const PhyDriver *phyDriver;                    ///<Ethernet PHY driver
   uint8_t phyAddr;                               ///<PHY address
   bool_t phyEvent;                               ///<A PHY event is pending
   const SwitchDriver *switchDriver;              ///<Ethernet switch driver
   const SmiDriver *smiDriver;                    ///<SMI driver
   MacAddr macAddr;                               ///<Link-layer address
   MacFilterEntry macAddrFilter[MAC_ADDR_FILTER_SIZE]; ///<MAC filter table
   bool_t promiscuous;                            ///<Promiscuous mode
   bool_t acceptAllMulticast;                     ///<Accept all frames with a multicast destination address
#endif
#if (ETH_VLAN_SUPPORT == ENABLED)
   uint16_t vlanId;                               ///<VLAN identifier (802.1Q)
#endif
#if (ETH_VMAN_SUPPORT == ENABLED)
   uint16_t vmanId;                               ///<VMAN identifier (802.1ad)
#endif
#if (ETH_LLC_SUPPORT == ENABLED)
   LlcRxCallback llcRxCallback;                   ///<LLC frame received callback (802.2)
   void *llcRxParam;                              ///<Callback parameter
#endif
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   uint8_t port;                                  ///<Switch port identifier
#endif
#if (ETH_VIRTUAL_IF_SUPPORT == ENABLED || ETH_VLAN_SUPPORT == ENABLED || \
   ETH_PORT_TAGGING_SUPPORT == ENABLED)
   NetInterface *parent;                          ///<Interface on top of which the virtual interface runs
#endif
#if (PPP_SUPPORT == ENABLED)
   PppContext *pppContext;                        ///<PPP context
#endif
#if (IPV4_SUPPORT == ENABLED)
   Ipv4Context ipv4Context;                       ///<IPv4 context
#endif
#if (IPV4_SUPPORT == ENABLED && IPV4_STATS_SUPPORT == ENABLED)
   IpIfStats ipv4IfStats;                         ///<Per-interface IPv4 statistics
#endif
#if (IPV4_SUPPORT == ENABLED && ETH_SUPPORT == ENABLED)
   bool_t enableArp;                              ///<Enable address resolution using ARP
   systime_t arpReachableTime;                    ///<ARP reachable time
   systime_t arpProbeTimeout;                     ///<ARP probe timeout
   ArpCacheEntry arpCache[ARP_CACHE_SIZE];        ///<ARP cache
#endif
#if (IPV4_SUPPORT == ENABLED && IGMP_HOST_SUPPORT == ENABLED)
   IgmpHostContext igmpHostContext;               ///<IGMP host context
#endif
#if (IPV4_SUPPORT == ENABLED && IGMP_ROUTER_SUPPORT == ENABLED)
   IgmpRouterContext *igmpRouterContext;          ///<IGMP router context
#endif
#if (IPV4_SUPPORT == ENABLED && IGMP_SNOOPING_SUPPORT == ENABLED)
   IgmpSnoopingContext *igmpSnoopingContext;      ///<IGMP snooping switch context
#endif
#if (IPV4_SUPPORT == ENABLED && AUTO_IP_SUPPORT == ENABLED)
   AutoIpContext *autoIpContext;                  ///<Auto-IP context
#endif
#if (IPV4_SUPPORT == ENABLED && DHCP_CLIENT_SUPPORT == ENABLED)
   DhcpClientContext *dhcpClientContext;          ///<DHCP client context
#endif
#if (IPV4_SUPPORT == ENABLED && DHCP_SERVER_SUPPORT == ENABLED)
   DhcpServerContext *dhcpServerContext;          ///<DHCP server context
#endif
#if (IPV6_SUPPORT == ENABLED)
   Ipv6Context ipv6Context;                       ///<IPv6 context
#endif
#if (IPV6_SUPPORT == ENABLED && IPV6_STATS_SUPPORT == ENABLED)
   IpIfStats ipv6IfStats;                         ///<Per-interface IPv6 statistics
#endif
#if (IPV6_SUPPORT == ENABLED && NDP_SUPPORT == ENABLED)
   NdpContext ndpContext;                         ///<NDP context
#endif
#if (IPV6_SUPPORT == ENABLED && NDP_ROUTER_ADV_SUPPORT == ENABLED)
   NdpRouterAdvContext *ndpRouterAdvContext;      ///<RA service context
#endif
#if (IPV6_SUPPORT == ENABLED && MLD_NODE_SUPPORT == ENABLED)
   MldNodeContext mldNodeContext;                 ///<MLD node context
#endif
#if (IPV6_SUPPORT == ENABLED && SLAAC_SUPPORT == ENABLED)
   SlaacContext *slaacContext;                    ///<SLAAC context
#endif
#if (IPV6_SUPPORT == ENABLED && DHCPV6_CLIENT_SUPPORT == ENABLED)
   Dhcpv6ClientContext *dhcpv6ClientContext;      ///<DHCPv6 client context
#endif
#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   MdnsResponderContext *mdnsResponderContext;    ///<mDNS responder context
#endif
#if (DNS_SD_RESPONDER_SUPPORT == ENABLED)
   DnsSdResponderContext *dnsSdResponderContext;  ///<DNS-SD responder context
#endif
#if (NET_IF_STATS_SUPPORT == ENABLED)
   NetIfStats ifStats;                            ///<Network interface statistics
#endif
};


/**
 * @brief TCP/IP stack settings
 **/

typedef struct
{
   OsTaskParameters task;    ///<Task parameters
   NetInterface *interfaces; ///<Network interfaces
   uint_t numInterfaces;     ///<Number of network interfaces
} NetSettings;


/**
 * @brief TCP/IP stack context
 **/

struct _NetContext
{
   bool_t running;                       ///<Operational state of the TCP/IP stack
   bool_t stop;                          ///<Stop request
   OsMutex mutex;                        ///<Mutex preventing simultaneous access to the TCP/IP stack
   OsEvent event;                        ///<Event object to receive notifications from drivers
   OsTaskParameters taskParams;          ///<Task parameters
   OsTaskId taskId;                      ///<Task identifier
   uint32_t entropy;
   systime_t timestamp;
   uint8_t randSeed[NET_RAND_SEED_SIZE]; ///<Random seed
   NetRandState randState;               ///<Pseudo-random number generator state
   NetInterface *interfaces;             ///<Network interfaces
   uint_t numInterfaces;                 ///<Number of network interfaces
   NetLinkChangeCallbackEntry linkChangeCallbacks[NET_MAX_LINK_CHANGE_CALLBACKS];
   NetTimerCallbackEntry timerCallbacks[NET_MAX_TIMER_CALLBACKS];
   systime_t nicTickCounter;             ///<Tick counter to handle periodic operations
#if (PPP_SUPPORT == ENABLED)
   systime_t pppTickCounter;
#endif
#if (IPV4_SUPPORT == ENABLED && ETH_SUPPORT == ENABLED)
   systime_t arpTickCounter;
#endif
#if (IPV4_SUPPORT == ENABLED && IPV4_FRAG_SUPPORT == ENABLED)
   systime_t ipv4FragTickCounter;
#endif
#if (IPV4_SUPPORT == ENABLED && IPV4_ROUTING_SUPPORT == ENABLED)
   Ipv4RoutingTableEntry ipv4RoutingTable[IPV4_ROUTING_TABLE_SIZE];
#endif
#if (IPV4_SUPPORT == ENABLED && (IGMP_HOST_SUPPORT == ENABLED || \
   IGMP_ROUTER_SUPPORT == ENABLED || IGMP_SNOOPING_SUPPORT == ENABLED))
   systime_t igmpTickCounter;
#endif
#if (IPV4_SUPPORT == ENABLED && AUTO_IP_SUPPORT == ENABLED)
   systime_t autoIpTickCounter;
#endif
#if (IPV4_SUPPORT == ENABLED && DHCP_CLIENT_SUPPORT == ENABLED)
   systime_t dhcpClientTickCounter;
#endif
#if (IPV4_SUPPORT == ENABLED && DHCP_SERVER_SUPPORT == ENABLED)
   systime_t dhcpServerTickCounter;
#endif
#if (IPV4_SUPPORT == ENABLED && NAT_SUPPORT == ENABLED)
   NatContext *natContext;               ///<NAT context
   systime_t natTickCounter;
#endif
#if (IPV4_SUPPORT == ENABLED && IPV4_IPSEC_SUPPORT == ENABLED)
   void *ipsecContext;                   ///<IPsec context
   void *ikeContext;                     ///<IKE context
#endif
#if (IPV6_SUPPORT == ENABLED && IPV6_FRAG_SUPPORT == ENABLED)
   systime_t ipv6FragTickCounter;
#endif
#if (IPV6_SUPPORT == ENABLED && IPV6_ROUTING_SUPPORT == ENABLED)
   Ipv6RoutingTableEntry ipv6RoutingTable[IPV6_ROUTING_TABLE_SIZE];
#endif
#if (IPV6_SUPPORT == ENABLED && MLD_NODE_SUPPORT == ENABLED)
   systime_t mldTickCounter;
#endif
#if (IPV6_SUPPORT == ENABLED && NDP_SUPPORT == ENABLED)
   systime_t ndpTickCounter;
#endif
#if (IPV6_SUPPORT == ENABLED && NDP_ROUTER_ADV_SUPPORT == ENABLED)
   systime_t ndpRouterAdvTickCounter;
#endif
#if (IPV6_SUPPORT == ENABLED && DHCPV6_CLIENT_SUPPORT == ENABLED)
   systime_t dhcpv6ClientTickCounter;
#endif
#if (TCP_SUPPORT == ENABLED)
   uint16_t tcpDynamicPort;              ///<TCP ephemeral port number
   systime_t tcpTickCounter;
#endif
#if (UDP_SUPPORT == ENABLED)
   uint16_t udpDynamicPort;              ///<UDP ephemeral port number
#endif
#if (DNS_CLIENT_SUPPORT == ENABLED || MDNS_CLIENT_SUPPORT == ENABLED || \
   NBNS_CLIENT_SUPPORT == ENABLED)
   systime_t dnsTickCounter;
#endif
#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   systime_t mdnsResponderTickCounter;
#endif
#if (DNS_SD_RESPONDER_SUPPORT == ENABLED)
   systime_t dnsSdResponderTickCounter;
#endif
#if (IPV4_SUPPORT == ENABLED && IPV4_STATS_SUPPORT == ENABLED)
   IpSystemStats ipv4SystemStats;        ///<System-wide IPv4 statistics
#endif
#if (IPV4_SUPPORT == ENABLED && ICMP_STATS_SUPPORT == ENABLED)
   IcmpStats icmpStats;                  ///<ICMP statistics
#endif
#if (IPV6_SUPPORT == ENABLED && IPV6_STATS_SUPPORT == ENABLED)
   IpSystemStats ipv6SystemStats;        ///<System-wide IPv6 statistics
#endif
#if (IPV6_SUPPORT == ENABLED && ICMPV6_STATS_SUPPORT == ENABLED)
   IcmpStats icmpv6Stats;                ///<ICMPv6 statistics
#endif
};


//TCP/IP stack related functions
void netGetDefaultSettings(NetSettings *settings);
error_t netInit(NetContext *context, const NetSettings *settings);

void netLock(NetContext *context);
void netUnlock(NetContext *context);

error_t netStart(NetContext *context);

error_t netSeedRand(NetContext *context, const uint8_t *seed, size_t length);
uint32_t netGetRand(NetContext *context);
uint32_t netGetRandRange(NetContext *context, uint32_t min, uint32_t max);
void netGetRandData(NetContext *context, uint8_t *data, size_t length);

NetContext *netGetDefaultContext(void);
NetInterface *netGetDefaultInterface(NetContext *context);

error_t netSetMacAddr(NetInterface *interface, const MacAddr *macAddr);
error_t netGetMacAddr(NetInterface *interface, MacAddr *macAddr);

error_t netSetEui64(NetInterface *interface, const Eui64 *eui64);
error_t netGetEui64(NetInterface *interface, Eui64 *eui64);

error_t netSetInterfaceId(NetInterface *interface, uint32_t id);
error_t netSetInterfaceName(NetInterface *interface, const char_t *name);
error_t netSetHostname(NetInterface *interface, const char_t *name);

error_t netSetVlanId(NetInterface *interface, uint16_t vlanId);
error_t netSetVmanId(NetInterface *interface, uint16_t vmanId);

error_t netSetParentInterface(NetInterface *interface,
   NetInterface *physicalInterface);

error_t netSetDriver(NetInterface *interface, const NicDriver *driver);

error_t netSetPhyDriver(NetInterface *interface, const PhyDriver *driver);
error_t netSetPhyAddr(NetInterface *interface, uint8_t phyAddr);

error_t netSetSwitchDriver(NetInterface *interface, const SwitchDriver *driver);
error_t netSetSwitchPort(NetInterface *interface, uint8_t port);

error_t netSetSmiDriver(NetInterface *interface, const SmiDriver *driver);
error_t netSetSpiDriver(NetInterface *interface, const SpiDriver *driver);
error_t netSetUartDriver(NetInterface *interface, const UartDriver *driver);
error_t netSetExtIntDriver(NetInterface *interface, const ExtIntDriver *driver);

error_t netSetLinkState(NetInterface *interface, bool_t linkState);
bool_t netGetLinkState(NetInterface *interface);

uint_t netGetLinkSpeed(NetInterface *interface);
NicDuplexMode netGetDuplexMode(NetInterface *interface);

error_t netEnablePromiscuousMode(NetInterface *interface, bool_t enable);

error_t netConfigInterface(NetInterface *interface);
error_t netStartInterface(NetInterface *interface);
error_t netStopInterface(NetInterface *interface);

void netTask(NetContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
