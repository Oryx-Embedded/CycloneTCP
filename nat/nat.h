/**
 * @file nat.h
 * @brief NAT (IP Network Address Translator)
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

#ifndef _NAT_H
#define _NAT_H

//Dependencies
#include "core/net.h"

//NAT support
#ifndef NAT_SUPPORT
   #define NAT_SUPPORT DISABLED
#elif (NAT_SUPPORT != ENABLED && NAT_SUPPORT != DISABLED)
   #error NAT_SUPPORT parameter is not valid
#endif

//NAT support
#ifndef NAT_MAX_PRIVATE_INTERFACES
   #define NAT_MAX_PRIVATE_INTERFACES 4
#elif (NAT_MAX_PRIVATE_INTERFACES < 1)
   #error NAT_MAX_PRIVATE_INTERFACES parameter is not valid
#endif

//NAT tick interval
#ifndef NAT_TICK_INTERVAL
   #define NAT_TICK_INTERVAL 1000
#elif (NAT_TICK_INTERVAL < 10)
   #error NAT_TICK_INTERVAL parameter is not valid
#endif

//TCP session timeout
#ifndef NAT_TCP_SESSION_TIMEOUT
   #define NAT_TCP_SESSION_TIMEOUT 120000
#elif (NAT_TCP_SESSION_TIMEOUT < 1000)
   #error NAT_TCP_SESSION_TIMEOUT parameter is not valid
#endif

//UDP session timeout
#ifndef NAT_UDP_SESSION_TIMEOUT
   #define NAT_UDP_SESSION_TIMEOUT 120000
#elif (NAT_UDP_SESSION_TIMEOUT < 1000)
   #error NAT_UDP_SESSION_TIMEOUT parameter is not valid
#endif

//ICMP session timeout
#ifndef NAT_ICMP_SESSION_TIMEOUT
   #define NAT_ICMP_SESSION_TIMEOUT 10000
#elif (NAT_ICMP_SESSION_TIMEOUT < 1000)
   #error NAT_ICMP_SESSION_TIMEOUT parameter is not valid
#endif

//TCP/UDP port range (lower limit)
#ifndef NAT_TCP_UDP_PORT_MIN
   #define NAT_TCP_UDP_PORT_MIN 32768
#elif (NAT_TCP_UDP_PORT_MIN < 1024)
   #error NAT_TCP_UDP_PORT_MIN parameter is not valid
#endif

//TCP/UDP port range (upper limit)
#ifndef NAT_TCP_UDP_PORT_MAX
   #define NAT_TCP_UDP_PORT_MAX 49151
#elif (NAT_TCP_UDP_PORT_MAX <= NAT_TCP_UDP_PORT_MIN || NAT_TCP_UDP_PORT_MAX > 65535)
   #error NAT_TCP_UDP_PORT_MAX parameter is not valid
#endif

//ICMP query identifier range (lower limit)
#ifndef NAT_ICMP_QUERY_ID_MIN
   #define NAT_ICMP_QUERY_ID_MIN 32768
#elif (NAT_ICMP_QUERY_ID_MIN < 0)
   #error NAT_ICMP_QUERY_ID_MIN parameter is not valid
#endif

//ICMP query identifier range (upper limit)
#ifndef NAT_ICMP_QUERY_ID_MAX
   #define NAT_ICMP_QUERY_ID_MAX 65535
#elif (NAT_ICMP_QUERY_ID_MAX <= NAT_ICMP_QUERY_ID_MIN || NAT_ICMP_QUERY_ID_MAX > 65535)
   #error NAT_ICMP_QUERY_ID_MAX parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief IP packet
 **/

typedef struct
{
   NetInterface *interface;
   const NetBuffer *buffer;
   size_t offset;
   Ipv4Protocol protocol;
   Ipv4Addr srcIpAddr;
   uint16_t srcPort;
   Ipv4Addr destIpAddr;
   uint16_t destPort;
   uint16_t icmpType;
   uint16_t icmpQueryId;
   uint8_t ttl;
   uint8_t tos;
} NatIpPacket;


/**
 * @brief Port redirection rule
 **/

typedef struct
{
   Ipv4Protocol protocol;          ///<Transport protocol (TCP or UDP)
   uint16_t publicPortMin;         ///<Public port range to be redirected (lower value)
   uint16_t publicPortMax;         ///<Public port range to be redirected (upper value)
   NetInterface *privateInterface; ///<Destination interface
   Ipv4Addr privateIpAddr;         ///<Destination IP address
   uint16_t privatePortMin;        ///<Destination port (lower value)
   uint16_t privatePortMax;        ///<Destination port (upper value)
} NatPortFwdRule;


/**
 * @brief NAT session
 **/

typedef struct
{
   Ipv4Protocol protocol;          ///<IP protocol (TCP, UDP or ICMP)
   NetInterface *privateInterface; ///<Private interface
   Ipv4Addr privateIpAddr;         ///<Internal IP address
   uint16_t privatePort;           ///<Internal TCP or UDP port number
   uint16_t privateIcmpQueryId;    ///<Internal ICMP query identifier
   Ipv4Addr publicIpAddr;          ///<External IP address
   uint16_t publicPort;            ///<External TCP or UDP port number
   uint16_t publicIcmpQueryId;     ///<External ICMP query identifier
   Ipv4Addr remoteIpAddr;          ///<Remote IP address
   uint16_t remotePort;            ///<Remote TCP or UDP port number
   systime_t timestamp;            ///<Timestamp to manage session timeout
} NatSession;


/**
 * @brief NAT settings
 **/

typedef struct
{
   NetInterface *publicInterface;                               ///<Public interface
   uint_t publicIpAddrIndex;                                    ///<Index of the public IP address to use
   NetInterface *privateInterfaces[NAT_MAX_PRIVATE_INTERFACES]; ///<Private interfaces
   uint_t numPrivateInterfaces;                                 ///<Number of private interfaces
   NatPortFwdRule *portFwdRules;                                ///<Port redirection rules
   uint_t numPortFwdRules;                                      ///<Number of port redirection rules
   NatSession *sessions;                                        ///<NAT sessions (initiated from a private host)
   uint_t numSessions;                                          ///<Number of NAT sessions
} NatSettings;


/**
 * @brief NAT context
 **/

typedef struct
{
   bool_t running;                                              ///<This flag tells whether the NAT is running or not
   NetInterface *publicInterface;                               ///<Public interface
   uint_t publicIpAddrIndex;                                    ///<Index of the public IP address to use
   NetInterface *privateInterfaces[NAT_MAX_PRIVATE_INTERFACES]; ///<Private interfaces
   uint_t numPrivateInterfaces;                                 ///<Number of private interfaces
   NatPortFwdRule *portFwdRules;                                ///<Port redirection rules
   uint_t numPortFwdRules;                                      ///<Number of port redirection rules
   NatSession *sessions;                                        ///<NAT sessions (initiated from a private host)
   uint_t numSessions;                                          ///<Number of NAT sessions
} NatContext;


//NAT related functions
void natGetDefaultSettings(NatSettings *settings);
error_t natInit(NatContext *context, const NatSettings *settings);

error_t natSetPublicInterface(NatContext *context,
   NetInterface *publicInterface);

error_t natSetPortFwdRule(NatContext *context, uint_t index,
   Ipv4Protocol protocol, uint16_t publicPort, NetInterface *privateInterface,
   Ipv4Addr privateIpAddr, uint16_t privatePort);

error_t natSetPortRangeFwdRule(NatContext *context, uint_t index,
   Ipv4Protocol protocol, uint16_t publicPortMin, uint16_t publicPortMax,
   NetInterface *privateInterface, Ipv4Addr privateIpAddr,
   uint16_t privatePortMin);

error_t natClearPortFwdRule(NatContext *context, uint_t index);

error_t natStart(NatContext *context);
error_t natStop(NatContext *context);

void natDeinit(NatContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
