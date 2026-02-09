/**
 * @file dhcpv6_relay.h
 * @brief DHCPv6 relay agent (Dynamic Host Configuration Protocol for IPv6)
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

#ifndef _DHCPV6_RELAY_H
#define _DHCPV6_RELAY_H

//Dependencies
#include "dhcpv6/dhcpv6_common.h"
#include "core/socket.h"

//DHCPv6 relay agent support
#ifndef DHCPV6_RELAY_SUPPORT
   #define DHCPV6_RELAY_SUPPORT DISABLED
#elif (DHCPV6_RELAY_SUPPORT != ENABLED && DHCPV6_RELAY_SUPPORT != DISABLED)
   #error DHCPV6_RELAY_SUPPORT parameter is not valid
#endif

//Stack size required to run the DHCPv6 relay agent
#ifndef DHCPV6_RELAY_STACK_SIZE
   #define DHCPV6_RELAY_STACK_SIZE 500
#elif (DHCPV6_RELAY_STACK_SIZE < 1)
   #error DHCPV6_RELAY_STACK_SIZE parameter is not valid
#endif

//Priority at which the DHCPv6 relay agent should run
#ifndef DHCPV6_RELAY_PRIORITY
   #define DHCPV6_RELAY_PRIORITY OS_TASK_PRIORITY_NORMAL
#endif

//Maximum number of client-facing interfaces
#ifndef DHCPV6_RELAY_MAX_CLIENT_INTERFACES
   #define DHCPV6_RELAY_MAX_CLIENT_INTERFACES 8
#elif (DHCPV6_RELAY_MAX_CLIENT_INTERFACES < 1)
   #error DHCPV6_RELAY_MAX_CLIENT_INTERFACES parameter is not valid
#endif

//The amount of overhead added by relay forwarding
#define DHCPV6_RELAY_FORWARDING_OVERHEAD (sizeof(Dhcpv6RelayMessage) + \
   2 * sizeof(Dhcpv6Option) + sizeof(uint32_t))

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief DHCPv6 relay agent settings
 **/

typedef struct
{
   OsTaskParameters task;                                              ///<Task parameters
   NetInterface *serverInterface;                                      ///<Network-facing interface
   uint_t numClientInterfaces;                                         ///<Number of client-facing interfaces
   NetInterface *clientInterfaces[DHCPV6_RELAY_MAX_CLIENT_INTERFACES]; ///<Client-facing interfaces
   Ipv6Addr serverIpAddr;                                              ///<Address to be used when relaying messages to the server
} Dhcpv6RelaySettings;


/**
 * @brief DHCPv6 relay agent context
 **/

typedef struct
{
   NetContext *netContext;                                             ///<TCP/IP stack context
   NetInterface *serverInterface;                                      ///<Network-facing interface
   uint_t numClientInterfaces;                                         ///<Number of client-facing interfaces
   NetInterface *clientInterfaces[DHCPV6_RELAY_MAX_CLIENT_INTERFACES]; ///<Client-facing interfaces
   Ipv6Addr serverIpAddr;                                              ///<Address to be used when relaying messages to the server
   Socket *serverSocket;                                               ///<Socket that handles the network-facing interface
   Socket *clientSockets[DHCPV6_RELAY_MAX_CLIENT_INTERFACES];          ///<Sockets that handle client-facing interfaces
   SocketEventDesc eventDesc[DHCPV6_RELAY_MAX_CLIENT_INTERFACES];      ///<The events the application is interested in
   bool_t running;                                                     ///<Operational state of the DHCPv6 relay agent
   bool_t stop;                                                        ///<Stop request
   OsEvent event;                                                      ///<Event object used to poll the sockets
   OsTaskParameters taskParams;                                        ///<Task parameters
   OsTaskId taskId;                                                    ///<Task identifier
   uint8_t buffer[DHCPV6_MAX_MSG_SIZE];                                ///<Scratch buffer to store DHCPv6 messages
} Dhcpv6RelayContext;


//DHCPv6 relay agent related functions
void dhcpv6RelayGetDefaultSettings(Dhcpv6RelaySettings *settings);

error_t dhcpv6RelayInit(Dhcpv6RelayContext *context,
   const Dhcpv6RelaySettings *settings);

error_t dhcpv6RelayStart(Dhcpv6RelayContext *context);
error_t dhcpv6RelayStop(Dhcpv6RelayContext *context);

void dhcpv6RelayTask(Dhcpv6RelayContext *context);

void dhcpv6RelayDeinit(Dhcpv6RelayContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
