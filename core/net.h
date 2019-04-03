/**
 * @file net.h
 * @brief TCP/IP stack core
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.2
 **/

#ifndef _NET_H
#define _NET_H

//Forward declaration of NetInterface structure
struct _NetInterface;
#define NetInterface struct _NetInterface

//Dependencies
#include "os_port.h"
#include "net_config.h"
#include "core/net_legacy.h"
#include "core/net_mem.h"
#include "core/nic.h"
#include "core/ethernet.h"
#include "ipv4/ipv4.h"
#include "ipv4/ipv4_frag.h"
#include "ipv4/auto_ip.h"
#include "ipv6/ipv6.h"
#include "ipv4/arp.h"
#include "ipv6/ndp.h"
#include "ipv6/ndp_router_adv.h"
#include "ipv6/slaac.h"
#include "ppp/ppp.h"
#include "dhcp/dhcp_client.h"
#include "dhcp/dhcp_server.h"
#include "dhcpv6/dhcpv6_client.h"
#include "dns/dns_client.h"
#include "mdns/mdns_responder.h"
#include "mdns/mdns_common.h"
#include "dns_sd/dns_sd.h"
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
#define CYCLONE_TCP_VERSION_STRING "1.9.2"
//Major version
#define CYCLONE_TCP_MAJOR_VERSION 1
//Minor version
#define CYCLONE_TCP_MINOR_VERSION 9
//Revision number
#define CYCLONE_TCP_REV_NUMBER 2

//RTOS support
#ifndef NET_RTOS_SUPPORT
   #define NET_RTOS_SUPPORT ENABLED
#elif (NET_RTOS_SUPPORT != ENABLED && NET_RTOS_SUPPORT != DISABLED)
   #error NET_RTOS_SUPPORT parameter is not valid
#endif

//Number of network adapters
#ifndef NET_INTERFACE_COUNT
   #define NET_INTERFACE_COUNT 1
#elif (NET_INTERFACE_COUNT < 1)
   #error NET_INTERFACE_COUNT parameter is not valid
#endif

//Loopback interface support
#ifndef NET_LOOPBACK_IF_SUPPORT
   #define NET_LOOPBACK_IF_SUPPORT DISABLED
#elif (NET_LOOPBACK_IF_SUPPORT != ENABLED && NET_LOOPBACK_IF_SUPPORT != DISABLED)
   #error NET_LOOPBACK_IF_SUPPORT parameter is not valid
#endif

//Maximum number of callback functions that can be registered
//to monitor link changes
#ifndef NET_CALLBACK_TABLE_SIZE
   #define NET_CALLBACK_TABLE_SIZE 6
#elif (NET_CALLBACK_TABLE_SIZE < 1)
   #error NET_CALLBACK_TABLE_SIZE parameter is not valid
#endif

//Maximum length of interface name
#ifndef NET_MAX_IF_NAME_LEN
   #define NET_MAX_IF_NAME_LEN 8
#elif (NET_MAX_IF_NAME_LEN < 1)
   #error NET_MAX_IF_NAME_LEN parameter is not valid
#endif

//Maximum length of host name
#ifndef NET_MAX_HOSTNAME_LEN
   #define NET_MAX_HOSTNAME_LEN 16
#elif (NET_MAX_HOSTNAME_LEN < 1)
   #error NET_MAX_HOSTNAME_LEN parameter is not valid
#endif

//Maximum length of proxy server name
#ifndef NET_MAX_PROXY_NAME_LEN
   #define NET_MAX_PROXY_NAME_LEN 16
#elif (NET_MAX_PROXY_NAME_LEN < 1)
   #error NET_MAX_PROXY_NAME_LEN parameter is not valid
#endif

//OS resources are statically allocated at compile time
#ifndef NET_STATIC_OS_RESOURCES
   #define NET_STATIC_OS_RESOURCES DISABLED
#elif (NET_STATIC_OS_RESOURCES != ENABLED && NET_STATIC_OS_RESOURCES != DISABLED)
   #error NET_STATIC_OS_RESOURCES parameter is not valid
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

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief Structure describing a network interface
 **/

struct _NetInterface
{
   uint_t index;                                  ///<Zero-based index
   uint32_t id;                                   ///<A unique number identifying the interface
   Eui64 eui64;                                   ///<EUI-64 interface identifier
   char_t name[NET_MAX_IF_NAME_LEN + 1];          ///<A unique name identifying the interface
   char_t hostname[NET_MAX_HOSTNAME_LEN + 1];     ///<Host name
   char_t proxyName[NET_MAX_PROXY_NAME_LEN + 1];  ///<Proxy server name
   uint16_t proxyPort;                            ///<Proxy server port
   const NicDriver *nicDriver;                    ///<NIC driver
   const PhyDriver *phyDriver;                    ///<PHY driver
   uint8_t phyAddr;                               ///<PHY address
   const SpiDriver *spiDriver;                    ///<Underlying SPI driver
   const UartDriver *uartDriver;                  ///<Underlying UART driver
   const ExtIntDriver *extIntDriver;              ///<External interrupt line driver
   uint8_t nicContext[NIC_CONTEXT_SIZE];          ///<Driver specific context
   OsEvent nicTxEvent;                            ///<Network controller TX event
   bool_t nicEvent;                               ///<A NIC event is pending
   bool_t phyEvent;                               ///<A PHY event is pending
   bool_t linkState;                              ///<Link state
   uint32_t linkSpeed;                            ///<Link speed
   NicDuplexMode duplexMode;                      ///<Duplex mode
   bool_t configured;                             ///<Configuration done

#if (ETH_SUPPORT == ENABLED)
   MacAddr macAddr;                               ///<Link-layer address
   MacFilterEntry macAddrFilter[MAC_ADDR_FILTER_SIZE]; ///<MAC filter table
#endif
#if (ETH_VLAN_SUPPORT == ENABLED)
   uint16_t vlanId;                               ///<VLAN identifier (802.1q)
#endif
#if (ETH_VMAN_SUPPORT == ENABLED)
   uint16_t vmanId;                               ///<VMAN identifier (802.1ad)
#endif
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   uint8_t port;                                  ///<Switch port identifier
#endif
#if (ETH_VIRTUAL_IF_SUPPORT == ENABLED || ETH_VLAN_SUPPORT == ENABLED || \
   ETH_PORT_TAGGING_SUPPORT == ENABLED)
   NetInterface *parent;                          ///<Interface on top of which the virtual interface runs
#endif

#if (IPV4_SUPPORT == ENABLED)
   Ipv4Context ipv4Context;                       ///<IPv4 context
   ArpCacheEntry arpCache[ARP_CACHE_SIZE];        ///<ARP cache
#if (IGMP_SUPPORT == ENABLED)
   systime_t igmpv1RouterPresentTimer;            ///<IGMPv1 router present timer
   bool_t igmpv1RouterPresent;                    ///<An IGMPv1 query has been recently heard
#endif
#if (AUTO_IP_SUPPORT == ENABLED)
   AutoIpContext *autoIpContext;                  ///<Auto-IP context
#endif
#if (DHCP_CLIENT_SUPPORT == ENABLED)
   DhcpClientContext *dhcpClientContext;          ///<DHCP client context
#endif
#if (DHCP_SERVER_SUPPORT == ENABLED)
   DhcpServerContext *dhcpServerContext;          ///<DHCP server context
#endif
#endif

#if (IPV6_SUPPORT == ENABLED)
   Ipv6Context ipv6Context;                       ///<IPv6 context
#if (NDP_SUPPORT == ENABLED)
   NdpContext ndpContext;                         ///<NDP context
#endif
#if (NDP_ROUTER_ADV_SUPPORT == ENABLED)
   NdpRouterAdvContext *ndpRouterAdvContext;      ///<RA service context
#endif
#if (SLAAC_SUPPORT == ENABLED)
   SlaacContext *slaacContext;                    ///<SLAAC context
#endif
#if (DHCPV6_CLIENT_SUPPORT == ENABLED)
   Dhcpv6ClientContext *dhcpv6ClientContext;      ///<DHCPv6 client context
#endif
#endif

#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   MdnsResponderContext *mdnsResponderContext;    ///<mDNS responder context
#endif

#if (DNS_SD_SUPPORT == ENABLED)
   DnsSdContext *dnsSdContext;                    ///DNS-SD context
#endif

#if (PPP_SUPPORT == ENABLED)
   PppContext *pppContext;                        ///<PPP context
#endif
};


/**
 * @brief Link change callback
 **/

typedef void (*LinkChangeCallback)(NetInterface *interface, bool_t linkState, void *param);


/**
 * @brief Entry describing a user callback
 **/

typedef struct
{
   NetInterface *interface;
   LinkChangeCallback callback;
   void *param;
} LinkChangeCallbackDesc;


//Global variables
extern OsTask *netTaskHandle;
extern OsMutex netMutex;
extern OsEvent netEvent;
extern NetInterface netInterface[NET_INTERFACE_COUNT];

//TCP/IP stack related functions
error_t netInit(void);

error_t netSetMacAddr(NetInterface *interface, const MacAddr *macAddr);
error_t netGetMacAddr(NetInterface *interface, MacAddr *macAddr);

error_t netSetEui64(NetInterface *interface, const Eui64 *eui64);
error_t netGetEui64(NetInterface *interface, Eui64 *eui64);

error_t netSetInterfaceId(NetInterface *interface, uint32_t id);
error_t netSetInterfaceName(NetInterface *interface, const char_t *name);
error_t netSetHostname(NetInterface *interface, const char_t *name);
error_t netSetProxy(NetInterface *interface, const char_t *name, uint16_t port);

error_t netSetVlanId(NetInterface *interface, uint16_t vlanId);
error_t netSetVmanId(NetInterface *interface, uint16_t vmanId);
error_t netSetSwitchPort(NetInterface *interface, uint8_t port);

error_t netSetParentInterface(NetInterface *interface,
   NetInterface *physicalInterface);

error_t netSetDriver(NetInterface *interface, const NicDriver *driver);

error_t netSetPhyDriver(NetInterface *interface, const PhyDriver *driver);
error_t netSetPhyAddr(NetInterface *interface, uint8_t phyAddr);

error_t netSetSpiDriver(NetInterface *interface, const SpiDriver *driver);
error_t netSetUartDriver(NetInterface *interface, const UartDriver *driver);
error_t netSetExtIntDriver(NetInterface *interface, const ExtIntDriver *driver);

error_t netSetLinkState(NetInterface *interface, NicLinkState linkState);
bool_t netGetLinkState(NetInterface *interface);

error_t netConfigInterface(NetInterface *interface);

void netTask(void);
void netTick(void);

NetInterface *netGetDefaultInterface(void);

error_t netInitRand(uint32_t seed);
uint32_t netGetRand(void);
int32_t netGetRandRange(int32_t min, int32_t max);

error_t netAttachLinkChangeCallback(NetInterface *interface,
   LinkChangeCallback callback, void *param, uint_t *cookie);

error_t netDetachLinkChangeCallback(uint_t cookie);

void netInvokeLinkChangeCallback(NetInterface *interface, bool_t linkState);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
