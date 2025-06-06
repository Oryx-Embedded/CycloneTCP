/**
 * @file socket_misc.c
 * @brief Helper functions for sockets
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
#define TRACE_LEVEL SOCKET_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/socket.h"
#include "core/socket_misc.h"
#include "core/raw_socket.h"
#include "core/udp.h"
#include "core/tcp.h"
#include "core/tcp_misc.h"
#include "debug.h"


/**
 * @brief Allocate a socket
 * @param[in] type Type specification for the new socket
 * @param[in] protocol Protocol to be used
 * @return Handle referencing the new socket
 **/

Socket *socketAllocate(uint_t type, uint_t protocol)
{
   error_t error;
   uint_t i;
   uint16_t port;
   Socket *socket;

   //Initialize socket handle
   socket = NULL;

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(type == SOCKET_TYPE_STREAM)
   {
      //Always use TCP as underlying transport protocol
      protocol = SOCKET_IP_PROTO_TCP;
      //Get an ephemeral port number
      port = tcpGetDynamicPort();
      //Continue processing
      error = NO_ERROR;
   }
   else
#endif
#if (UDP_SUPPORT == ENABLED)
   //Connectionless socket?
   if(type == SOCKET_TYPE_DGRAM)
   {
      //Always use UDP as underlying transport protocol
      protocol = SOCKET_IP_PROTO_UDP;
      //Get an ephemeral port number
      port = udpGetDynamicPort();
      //Continue processing
      error = NO_ERROR;
   }
   else
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
   //Raw socket?
   if(type == SOCKET_TYPE_RAW_IP || type == SOCKET_TYPE_RAW_ETH)
   {
      //Port numbers are not relevant for raw sockets
      port = 0;
      //Continue processing
      error = NO_ERROR;
   }
   else
#endif
   {
      //The socket type is not supported
      error = ERROR_INVALID_PARAMETER;
   }

   //Check status code
   if(!error)
   {
      //Loop through socket descriptors
      for(i = 0; i < SOCKET_MAX_COUNT; i++)
      {
         //Unused socket found?
         if(socketTable[i].type == SOCKET_TYPE_UNUSED)
         {
            //Save socket handle
            socket = &socketTable[i];
            //We are done
            break;
         }
      }

#if (TCP_SUPPORT == ENABLED)
      //No more sockets available?
      if(socket == NULL)
      {
         //Kill the oldest connection in the TIME-WAIT state whenever the
         //socket table runs out of space
         socket = tcpKillOldestConnection();
      }
#endif

      //Check whether the current entry is free
      if(socket != NULL)
      {
         //Save socket descriptor
         i = socket->descriptor;

         //Clear the structure keeping the event field untouched
         osMemset(socket, 0, offsetof(Socket, event));

         osMemset((uint8_t *) socket + offsetof(Socket, event) + sizeof(OsEvent),
            0, sizeof(Socket) - offsetof(Socket, event) - sizeof(OsEvent));

         //Save socket characteristics
         socket->descriptor = i;
         socket->type = type;
         socket->protocol = protocol;
         socket->localPort = port;
         socket->timeout = INFINITE_DELAY;

#if (ETH_VLAN_SUPPORT == ENABLED)
         //Default VLAN PCP and DEI fields
         socket->vlanPcp = -1;
         socket->vlanDei = -1;
#endif

#if (ETH_VMAN_SUPPORT == ENABLED)
         //Default VMAN PCP and DEI fields
         socket->vmanPcp = -1;
         socket->vmanDei = -1;
#endif

#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
         //TCP keep-alive mechanism must be disabled by default (refer to
         //RFC 1122, section 4.2.3.6)
         socket->keepAliveEnabled = FALSE;

         //Default TCP keep-alive parameters
         socket->keepAliveIdle = TCP_DEFAULT_KEEP_ALIVE_IDLE;
         socket->keepAliveInterval = TCP_DEFAULT_KEEP_ALIVE_INTERVAL;
         socket->keepAliveMaxProbes = TCP_DEFAULT_KEEP_ALIVE_PROBES;
#endif

#if (TCP_SUPPORT == ENABLED)
         //Default MSS value
         socket->mss = TCP_MAX_MSS;

         //Default TX and RX buffer size
         socket->txBufferSize = MIN(TCP_DEFAULT_TX_BUFFER_SIZE, TCP_MAX_TX_BUFFER_SIZE);
         socket->rxBufferSize = MIN(TCP_DEFAULT_RX_BUFFER_SIZE, TCP_MAX_RX_BUFFER_SIZE);

         //Compute the window scale factor to use for the receive window
         tcpComputeWindowScaleFactor(socket);
#endif
      }
   }

   //Return a handle to the freshly created socket
   return socket;
}


/**
 * @brief Subscribe to the specified socket events
 * @param[in] socket Handle that identifies a socket
 * @param[in] event Event object used to receive notifications
 * @param[in] eventMask Logic OR of the requested socket events
 **/

void socketRegisterEvents(Socket *socket, OsEvent *event, uint_t eventMask)
{
   //Valid socket handle?
   if(socket != NULL)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //An user event may have been previously registered...
      if(socket->userEvent != NULL)
      {
         socket->eventMask |= eventMask;
      }
      else
      {
         socket->eventMask = eventMask;
      }

      //Suscribe to get notified of events
      socket->userEvent = event;

#if (TCP_SUPPORT == ENABLED)
      //Handle TCP specific events
      if(socket->type == SOCKET_TYPE_STREAM)
      {
         tcpUpdateEvents(socket);
      }
#endif
#if (UDP_SUPPORT == ENABLED)
      //Handle UDP specific events
      if(socket->type == SOCKET_TYPE_DGRAM)
      {
         udpUpdateEvents(socket);
      }
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
      //Handle events that are specific to raw sockets
      if(socket->type == SOCKET_TYPE_RAW_IP ||
         socket->type == SOCKET_TYPE_RAW_ETH)
      {
         rawSocketUpdateEvents(socket);
      }
#endif

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }
}


/**
 * @brief Unsubscribe previously registered events
 * @param[in] socket Handle that identifies a socket
 **/

void socketUnregisterEvents(Socket *socket)
{
   //Valid socket handle?
   if(socket != NULL)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Unsuscribe socket events
      socket->userEvent = NULL;

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }
}


/**
 * @brief Retrieve event flags for a specified socket
 * @param[in] socket Handle that identifies a socket
 * @return Logic OR of events in the signaled state
 **/

uint_t socketGetEvents(Socket *socket)
{
   uint_t eventFlags;

   //Valid socket handle?
   if(socket != NULL)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Read event flags for the specified socket
      eventFlags = socket->eventFlags;

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }
   else
   {
      //The socket handle is not valid
      eventFlags = 0;
   }

   //Return the events in the signaled state
   return eventFlags;
}


/**
 * @brief Filter out incoming multicast traffic
 * @param[in] socket Handle that identifies a socket
 * @param[in] destAddr Destination IP address of the received packet
 * @param[in] srcAddr Source IP address of the received packet
 * @return Return TRUE if the multicast packet should be accepted, else FALSE
 **/

bool_t socketMulticastFilter(Socket *socket, const IpAddr *destAddr,
   const IpAddr *srcAddr)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0)
   uint_t i;
   bool_t acceptable;
   SocketMulticastGroup *group;

   //Initialize flag
   acceptable = FALSE;

   //Loop through multicast groups
   for(i = 0; i < SOCKET_MAX_MULTICAST_GROUPS; i++)
   {
      //Point to the current multicast group
      group = &socket->multicastGroups[i];

      //Matching multicast address?
      if(ipCompAddr(&group->addr, destAddr))
      {
#if (SOCKET_MAX_MULTICAST_SOURCES > 0)
         uint_t j;

         //Check filter mode
         if(group->filterMode == IP_FILTER_MODE_INCLUDE)
         {
            //In INCLUDE mode, reception of packets sent to the specified
            //multicast address is requested only from those IP source
            //addresses listed in the source list
            for(j = 0; j < SOCKET_MAX_MULTICAST_SOURCES && !acceptable; j++)
            {
               //Compare source addresses
               if(ipCompAddr(&group->sources[j], srcAddr))
               {
                  acceptable = TRUE;
               }
            }
         }
         else
         {
            //In EXCLUDE mode, reception of packets sent to the given multicast
            //address is requested from all IP source addresses except those
            //listed in the source list
            acceptable = TRUE;

            //Loop through the list of excluded source addresses
            for(j = 0; j < group->numSources && acceptable; j++)
            {
               //Compare source addresses
               if(ipCompAddr(&group->sources[j], srcAddr))
               {
                  acceptable = FALSE;
               }
            }
         }
#else
         //The multicast address is acceptable
         acceptable = TRUE;
#endif
      }
   }

   //Return TRUE if the multicast packet should be accepted
   return acceptable;
#else
   //Not implemented
   return FALSE;
#endif
}


/**
 * @brief Create a new multicast group
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying a multicast group
 * @return Pointer to the newly created multicast group
 **/

SocketMulticastGroup *socketCreateMulticastGroupEntry(Socket *socket,
   const IpAddr *groupAddr)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0)
   uint_t i;
   SocketMulticastGroup *group;

   //Initialize pointer
   group = NULL;

   //Loop through multicast groups
   for(i = 0; i < SOCKET_MAX_MULTICAST_GROUPS; i++)
   {
      //Check whether the current entry is available for use
      if(socket->multicastGroups[i].addr.length == 0)
      {
         //Point to the current group
         group = &socket->multicastGroups[i];

         //Save multicast group address
         group->addr = *groupAddr;

#if (SOCKET_MAX_MULTICAST_SOURCES > 0)
         //By default, all sources are accepted
         group->filterMode = IP_FILTER_MODE_EXCLUDE;
         group->numSources = 0;
#endif
         //We are done
         break;
      }
   }

   //Return a pointer to the newly created multicast group
   return group;
#else
   //Not implemented
   return NULL;
#endif
}


/**
 * @brief Search the list of multicast groups for a given group address
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying a multicast group
 * @return A pointer to the matching multicast group is returned. NULL is
 *   returned if the specified group address cannot be found
 **/

SocketMulticastGroup *socketFindMulticastGroupEntry(Socket *socket,
   const IpAddr *groupAddr)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0)
   uint_t i;
   SocketMulticastGroup *group;

   //Initialize pointer
   group = NULL;

   //Loop through multicast groups
   for(i = 0; i < SOCKET_MAX_MULTICAST_GROUPS; i++)
   {
      //Compare group addresses
      if(ipCompAddr(&socket->multicastGroups[i].addr, groupAddr))
      {
         //Point to the current group
         group = &socket->multicastGroups[i];
         break;
      }
   }

   //Return a pointer to the matching multicast group
   return group;
#else
   //Not implemented
   return NULL;
#endif
}


/**
 * @brief Delete a multicast group
 * @param[in] group Pointer to the multicast group
 **/

void socketDeleteMulticastGroupEntry(SocketMulticastGroup *group)
{
   //Delete the specified entry
   group->addr = IP_ADDR_UNSPECIFIED;
}


/**
 * @brief Add an address to the multicast source filter
 * @param[in] group Pointer to the multicast group
 * @param[in] srcAddr IP address to be added to the list
 * @return Error code
 **/

error_t socketAddMulticastSrcAddr(SocketMulticastGroup *group,
   const IpAddr *srcAddr)
{
#if (SOCKET_MAX_MULTICAST_SOURCES > 0)
   error_t error;

   //Initialize status code
   error = NO_ERROR;

   //Make sure that the source address is not a duplicate
   if(socketFindMulticastSrcAddr(group, srcAddr) < 0)
   {
      //Check the length of the list
      if(group->numSources < SOCKET_MAX_MULTICAST_SOURCES)
      {
         //Append the source address to the list
         group->sources[group->numSources] = *srcAddr;
         group->numSources++;
      }
      else
      {
         //The implementation limits the number of source addresses
         error = ERROR_OUT_OF_RESOURCES;
      }
   }

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Remove an address from the multicast source filter
 * @param[in] group Pointer to the multicast group
 * @param[in] srcAddr IP address to be removed from the list
 **/

void socketRemoveMulticastSrcAddr(SocketMulticastGroup *group,
   const IpAddr *srcAddr)
{
#if (SOCKET_MAX_MULTICAST_SOURCES > 0)
   uint_t i;
   uint_t j;

   //Loop through the list of source addresses
   for(i = 0; i < group->numSources; i++)
   {
      //Matching IP address?
      if(ipCompAddr(&group->sources[i], srcAddr))
      {
         //Remove the source address from the list
         for(j = i + 1; j < group->numSources; j++)
         {
            group->sources[j - 1] = group->sources[j];
         }

         //Update the length of the list
         group->numSources--;

         //We are done
         break;
      }
   }
#endif
}


/**
 * @brief Search the list of multicast sources for a given IP address
 * @param[in] group Pointer to the multicast group
 * @param[in] srcAddr Source IP address
 * @return Index of the matching IP address is returned. -1 is
 *   returned if the specified IP address cannot be found
 **/

int_t socketFindMulticastSrcAddr(SocketMulticastGroup *group,
   const IpAddr *srcAddr)
{
#if (SOCKET_MAX_MULTICAST_SOURCES > 0)
   int_t i;
   int_t index;

   //Initialize index
   index = -1;

   //Loop through the list of source addresses
   for(i = 0; i < group->numSources; i++)
   {
      //Matching IP address?
      if(ipCompAddr(&group->sources[i], srcAddr))
      {
         index = i;
         break;
      }
   }

   //Return the index of the matching IP address, if any
   return index;
#else
   //Not implemented
   return -1;
#endif
}
