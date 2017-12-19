/**
 * @file net_legacy.h
 * @brief Legacy definitions
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
 * @version 1.8.0
 **/

#ifndef _NET_LEGACY_H
#define _NET_LEGACY_H

//Check compiler
#if !defined(_WIN32)

//Deprecated properties
#ifdef TCP_IP_CALLBACK_TABLE_SIZE
   #warning TCP_IP_CALLBACK_TABLE_SIZE property is deprecated. NET_CALLBACK_TABLE_SIZE should be used instead.
   #define NET_CALLBACK_TABLE_SIZE TCP_IP_CALLBACK_TABLE_SIZE
#endif

#ifdef TCP_IP_MAX_IF_NAME_LEN
   #warning TCP_IP_MAX_IF_NAME_LEN property is deprecated. NET_MAX_IF_NAME_LEN should be used instead.
   #define NET_MAX_IF_NAME_LEN TCP_IP_MAX_IF_NAME_LEN
#endif

#ifdef TCP_IP_MAX_HOSTNAME_LEN
   #warning TCP_IP_MAX_HOSTNAME_LEN property is deprecated. NET_MAX_HOSTNAME_LEN should be used instead.
   #define NET_MAX_HOSTNAME_LEN TCP_IP_MAX_HOSTNAME_LEN
#endif

#ifdef TCP_IP_MAX_PROXY_NAME_LEN
   #warning TCP_IP_MAX_PROXY_NAME_LEN property is deprecated. NET_MAX_PROXY_NAME_LEN should be used instead.
   #define NET_MAX_PROXY_NAME_LEN TCP_IP_MAX_PROXY_NAME_LEN
#endif

#ifdef TCP_IP_STATIC_OS_RESOURCES
   #warning TCP_IP_STATIC_OS_RESOURCES property is deprecated. NET_STATIC_OS_RESOURCES should be used instead.
   #define NET_STATIC_OS_RESOURCES TCP_IP_STATIC_OS_RESOURCES
#endif

#ifdef TCP_IP_TICK_STACK_SIZE
   #warning TCP_IP_TICK_STACK_SIZE property is deprecated. NET_TICK_STACK_SIZE should be used instead.
   #define NET_TICK_STACK_SIZE TCP_IP_TICK_STACK_SIZE
#endif

#ifdef TCP_IP_TICK_PRIORITY
   #warning TCP_IP_TICK_PRIORITY property is deprecated. NET_TICK_PRIORITY should be used instead.
   #define NET_TICK_PRIORITY TCP_IP_TICK_PRIORITY
#endif

#ifdef TCP_IP_TICK_INTERVAL
   #warning TCP_IP_TICK_INTERVAL property is deprecated. NET_TICK_INTERVAL should be used instead.
   #define NET_TICK_INTERVAL TCP_IP_TICK_INTERVAL
#endif

#ifdef TCP_IP_RX_STACK_SIZE
   #warning TCP_IP_RX_STACK_SIZE property is deprecated. NET_RX_STACK_SIZE should be used instead.
   #define NET_RX_STACK_SIZE TCP_IP_RX_STACK_SIZE
#endif

#ifdef TCP_IP_RX_PRIORITY
   #warning TCP_IP_RX_PRIORITY property is deprecated. NET_RX_PRIORITY should be used instead.
   #define NET_RX_PRIORITY TCP_IP_RX_PRIORITY
#endif

#ifdef TCP_SYN_QUEUE_SIZE
   #warning TCP_SYN_QUEUE_SIZE property is deprecated. TCP_DEFAULT_SYN_QUEUE_SIZE should be used instead.
   #define TCP_DEFAULT_SYN_QUEUE_SIZE TCP_SYN_QUEUE_SIZE
#endif

#ifdef MAC_FILTER_MAX_SIZE
   //#warning MAC_FILTER_MAX_SIZE property is deprecated. MAC_MULTICAST_FILTER_SIZE should be used instead.
   #define MAC_MULTICAST_FILTER_SIZE MAC_FILTER_MAX_SIZE
#endif

#ifdef IPV4_FILTER_MAX_SIZE
   //#warning IPV4_FILTER_MAX_SIZE property is deprecated. IPV4_MULTICAST_FILTER_SIZE should be used instead.
   #define IPV4_MULTICAST_FILTER_SIZE IPV4_FILTER_MAX_SIZE
#endif

#ifdef IPV6_FILTER_MAX_SIZE
   //#warning IPV6_FILTER_MAX_SIZE property is deprecated. IPV6_MULTICAST_FILTER_SIZE should be used instead.
   #define IPV6_MULTICAST_FILTER_SIZE IPV6_FILTER_MAX_SIZE
#endif

#ifdef IPV4_MAX_DNS_SERVERS
   //#warning IPV4_MAX_DNS_SERVERS property is deprecated. IPV4_DNS_SERVER_LIST_SIZE should be used instead.
   #define IPV4_DNS_SERVER_LIST_SIZE IPV4_MAX_DNS_SERVERS
#endif

#ifdef IPV6_MAX_DNS_SERVERS
   //#warning IPV6_MAX_DNS_SERVERS property is deprecated. IPV6_DNS_SERVER_LIST_SIZE should be used instead.
   #define IPV6_DNS_SERVER_LIST_SIZE IPV6_MAX_DNS_SERVERS
#endif

#ifdef NET_TICK_STACK_SIZE
   #warning NET_TICK_STACK_SIZE property is deprecated and should be removed from net_config.h. The TCP/IP stack now uses a single task
#endif

#ifdef NET_TICK_PRIORITY
   #warning NET_TICK_PRIORITY property is deprecated and should be removed from net_config.h. TCP/IP stack now uses a single task.
#endif

#ifdef NET_RX_STACK_SIZE
   #warning NET_RX_STACK_SIZE property is deprecated since the TCP/IP stack now uses a single task. NET_TASK_STACK_SIZE should be used instead.
   #define NET_TASK_STACK_SIZE NET_RX_STACK_SIZE
#endif

#ifdef NET_RX_PRIORITY
   #warning NET_RX_PRIORITY property is deprecated since the TCP/IP stack now uses a single task. NET_TASK_PRIORITY should be used instead.
   #define NET_TASK_PRIORITY NET_RX_PRIORITY
#endif

#endif

//Legacy definitions
#define SOCKET_TYPE_RAW SOCKET_TYPE_RAW_IP
#define SOCKET_PROTOCOL_ICMP SOCKET_IP_PROTO_ICMP
#define SOCKET_PROTOCOL_IGMP SOCKET_IP_PROTO_IGMP
#define SOCKET_PROTOCOL_TCP SOCKET_IP_PROTO_TCP
#define SOCKET_PROTOCOL_UDP SOCKET_IP_PROTO_UDP
#define SOCKET_PROTOCOL_ICMPV6 SOCKET_IP_PROTO_ICMPV6

//Deprecated functions
#define tcpIpStackInit netInit
#define tcpIpStackSetInterfaceName netSetInterfaceName
#define tcpIpStackSetHostname netSetHostname
#define tcpIpStackSetDriver netSetDriver
#define tcpIpStackSetPhyDriver netSetPhyDriver
#define tcpIpStackSetSpiDriver netSetSpiDriver
#define tcpIpStackSetUartDriver netSetUartDriver
#define tcpIpStackSetExtIntDriver netSetExtIntDriver
#define tcpIpStackSetMacAddr netSetMacAddr
#define tcpIpStackSetProxy netSetProxy
#define tcpIpStackGetLinkState netGetLinkState
#define tcpIpStackConfigInterface netConfigInterface
#define tcpIpStackTickTask netTickTask
#define tcpIpStackRxTask netRxTask
#define tcpIpStackGetDefaultInterface netGetDefaultInterface
#define tcpIpStackInitRand netInitRand
#define tcpIpStackGetRand netGetRand
#define tcpIpStackGetRandRange netGetRandRange
#define tcpIpStackAttachLinkChangeCallback netAttachLinkChangeCallback
#define tcpIpStackDetachLinkChangeCallback netDetachLinkChangeCallback
#define tcpIpStackInvokeLinkChangeCallback netInvokeLinkChangeCallback

#define ChunkedBuffer NetBuffer
#define chunkedBufferGetLength netBufferGetLength
#define chunkedBufferRead netBufferRead

#define ipv4IsInLocalSubnet ipv4IsOnLocalSubnet

#ifdef SMTP_DEFAULT_TIMEOUT
   #define SMTP_CLIENT_DEFAULT_TIMEOUT SMTP_DEFAULT_TIMEOUT
#endif

#ifdef SMTP_MAX_LINE_LENGTH
   #define SMTP_CLIENT_MAX_LINE_LENGTH SMTP_MAX_LINE_LENGTH
#endif

#ifdef SMTP_TLS_SUPPORT
   #define SMTP_CLIENT_TLS_SUPPORT SMTP_TLS_SUPPORT
#endif

#ifdef SMTP_LOGIN_AUTH_SUPPORT
   #define SMTP_CLIENT_LOGIN_AUTH_SUPPORT SMTP_LOGIN_AUTH_SUPPORT
#endif

#ifdef SMTP_PLAIN_AUTH_SUPPORT
   #define SMTP_CLIENT_PLAIN_AUTH_SUPPORT SMTP_PLAIN_AUTH_SUPPORT
#endif

#ifdef SMTP_CRAM_MD5_AUTH_SUPPORT
   #define SMTP_CLIENT_CRAM_MD5_AUTH_SUPPORT SMTP_CRAM_MD5_AUTH_SUPPORT
#endif

#define DhcpClientCtx DhcpClientContext
#define Dhcpv6ClientCtx Dhcpv6ClientContext

#ifdef SNMP_AGENT_MAX_MIB_COUNT
   #define SNMP_AGENT_MAX_MIBS SNMP_AGENT_MAX_MIB_COUNT
#endif

#ifdef SNMP_AGENT_MAX_USER_COUNT
   #define SNMP_AGENT_MAX_USERS SNMP_AGENT_MAX_USER_COUNT
#endif

#endif
