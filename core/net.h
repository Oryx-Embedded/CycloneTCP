/**
 * @file net.h
 * @brief TCP/IP stack core
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.7.6
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
#include "mibs/mib2_module.h"
#include "cpu_endian.h"
#include "error.h"

//Version string
#define NET_VERSION_STRING "1.7.6"
//Major version
#define NET_MAJOR_VERSION 1
//Minor version
#define NET_MINOR_VERSION 7
//Revision number
#define NET_REV_NUMBER 6

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


/**
 * @brief Structure describing a network interface
 **/

struct _NetInterface
{
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
   MacFilterEntry macMulticastFilter[MAC_MULTICAST_FILTER_SIZE]; ///<Multicast MAC filter
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

#if (MIB2_SUPPORT == ENABLED)
   Mib2IfEntry *mibIfEntry;
#endif
};


/**
 * @brief Link change callback
 **/

typedef void (*LinkChangeCallback)(NetInterface *interface, bool_t linkState, void *params);


/**
 * @brief Entry describing a user callback
 **/

typedef struct
{
   NetInterface *interface;
   LinkChangeCallback callback;
   void *params;
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
   LinkChangeCallback callback, void *params, uint_t *cookie);

error_t netDetachLinkChangeCallback(uint_t cookie);

void netInvokeLinkChangeCallback(NetInterface *interface, bool_t linkState);

#endif
