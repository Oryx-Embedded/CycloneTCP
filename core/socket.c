/**
 * @file socket.c
 * @brief Socket API
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
#include "dns/dns_client.h"
#include "mdns/mdns_client.h"
#include "netbios/nbns_client.h"
#include "llmnr/llmnr_client.h"
#include "debug.h"

//Socket table
Socket socketTable[SOCKET_MAX_COUNT];

//Default socket message
const SocketMsg SOCKET_DEFAULT_MSG =
{
   NULL,          //Pointer to the payload
   0,             //Size of the payload, in bytes
   0,             //Actual length of the payload, in bytes
   0,             //Time-to-live value
   0,             //Type-of-service value
   IP_DEFAULT_DF, //Do not fragment the IP packet
   NULL,          //Underlying network interface
   {0},           //Source IP address
   0,             //Source port
   {0},           //Destination IP address
   0,             //Destination port
#if (ETH_SUPPORT == ENABLED)
   {{{0}}},       //Source MAC address
   {{{0}}},       //Destination MAC address
   0,             //Ethernet type field
#endif
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   0,             //Switch port identifier
#endif
#if (ETH_TIMESTAMP_SUPPORT == ENABLED)
   -1,            //Unique identifier for hardware time stamping
   {0},           //Captured time stamp
#endif
};


/**
 * @brief Socket related initialization
 * @return Error code
 **/

error_t socketInit(void)
{
   uint_t i;
   uint_t j;

   //Initialize socket descriptors
   osMemset(socketTable, 0, sizeof(socketTable));

   //Loop through socket descriptors
   for(i = 0; i < SOCKET_MAX_COUNT; i++)
   {
      //Set socket identifier
      socketTable[i].descriptor = i;

      //Create an event object to track socket events
      if(!osCreateEvent(&socketTable[i].event))
      {
         //Clean up side effects
         for(j = 0; j < i; j++)
         {
            osDeleteEvent(&socketTable[j].event);
         }

         //Report an error
         return ERROR_OUT_OF_RESOURCES;
      }
   }

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Create a socket (UDP or TCP)
 * @param[in] type Type specification for the new socket
 * @param[in] protocol Protocol to be used
 * @return Handle referencing the new socket
 **/

Socket *socketOpen(uint_t type, uint_t protocol)
{
   Socket *socket;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Allocate a new socket
   socket = socketAllocate(type, protocol);
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return a handle to the freshly created socket
   return socket;
}


/**
 * @brief Set timeout value for blocking operations
 * @param[in] socket Handle to a socket
 * @param[in] timeout Maximum time to wait
 * @return Error code
 **/

error_t socketSetTimeout(Socket *socket, systime_t timeout)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Record timeout value
   socket->timeout = timeout;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Set TTL value for unicast datagrams
 * @param[in] socket Handle to a socket
 * @param[in] ttl Time-to-live value
 * @return Error code
 **/

error_t socketSetTtl(Socket *socket, uint8_t ttl)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set TTL value
   socket->ttl = ttl;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Set TTL value for multicast datagrams
 * @param[in] socket Handle to a socket
 * @param[in] ttl Time-to-live value
 * @return Error code
 **/

error_t socketSetMulticastTtl(Socket *socket, uint8_t ttl)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set TTL value
   socket->multicastTtl = ttl;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Set DSCP field
 * @param[in] socket Handle to a socket
 * @param[in] dscp Differentiated services codepoint
 * @return Error code
 **/

error_t socketSetDscp(Socket *socket, uint8_t dscp)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //The DSCP field is 6 bits wide
   if(dscp >= 64)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set differentiated services codepoint
   socket->tos = (dscp << 2) & 0xFF;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Set VLAN priority
 * @param[in] socket Handle to a socket
 * @param[in] pcp VLAN priority value
 * @return Error code
 **/

error_t socketSetVlanPcp(Socket *socket, uint8_t pcp)
{
#if (ETH_VLAN_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //The PCP field is 3 bits wide
   if(pcp >= 8)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //The PCP field specifies the frame priority level. Different PCP values
   //can be used to prioritize different classes of traffic
   socket->vlanPcp = pcp;

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Set VLAN DEI flag
 * @param[in] socket Handle to a socket
 * @param[in] dei Drop eligible indicator
 * @return Error code
 **/

error_t socketSetVlanDei(Socket *socket, bool_t dei)
{
#if (ETH_VLAN_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //The DEI flag may be used to indicate frames eligible to be dropped in
   //the presence of congestion
   socket->vlanDei = dei;

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Set VMAN priority
 * @param[in] socket Handle to a socket
 * @param[in] pcp VLAN priority value
 * @return Error code
 **/

error_t socketSetVmanPcp(Socket *socket, uint8_t pcp)
{
#if (ETH_VMAN_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //The PCP field is 3 bits wide
   if(pcp >= 8)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //The PCP field specifies the frame priority level. Different PCP values
   //can be used to prioritize different classes of traffic
   socket->vmanPcp = pcp;

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Set VMAN DEI flag
 * @param[in] socket Handle to a socket
 * @param[in] dei Drop eligible indicator
 * @return Error code
 **/

error_t socketSetVmanDei(Socket *socket, bool_t dei)
{
#if (ETH_VMAN_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //The DEI flag may be used to indicate frames eligible to be dropped in
   //the presence of congestion
   socket->vmanDei = dei;

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Enable reception of broadcast messages
 * @param[in] socket Handle to a socket
 * @param[in] enabled Specifies whether broadcast messages should be accepted
 * @return Error code
 **/

error_t socketEnableBroadcast(Socket *socket, bool_t enabled)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Check whether broadcast messages should be accepted
   if(enabled)
   {
      socket->options |= SOCKET_OPTION_BROADCAST;
   }
   else
   {
      socket->options &= ~SOCKET_OPTION_BROADCAST;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Join the specified host group
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying the host group to join
 * @return Error code
 **/

error_t socketJoinMulticastGroup(Socket *socket, const IpAddr *groupAddr)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0)
   error_t error;
   SocketMulticastGroup *group;

   //Check parameters
   if(socket == NULL || groupAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connectionless or raw sockets
   if(socket->type != SOCKET_TYPE_DGRAM &&
      socket->type != SOCKET_TYPE_RAW_IP)
   {
      return ERROR_INVALID_SOCKET;
   }

   //The group address must be a valid multicast address
   if(!ipIsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Initialize status code
   error = NO_ERROR;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the list of multicast groups for en existing entry
   group = socketFindMulticastGroupEntry(socket, groupAddr);

   //No matching entry found?
   if(group == NULL)
   {
      //Create a new entry
      group = socketCreateMulticastGroupEntry(socket, groupAddr);
   }

   //Entry successfully created?
   if(group != NULL)
   {
      //Update the multicast reception state of the interface
      ipUpdateMulticastFilter(socket->interface, groupAddr);
   }
   else
   {
      //Report an error
      error = ERROR_OUT_OF_RESOURCES;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Leave the specified host group
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying the host group to leave
 * @return Error code
 **/

error_t socketLeaveMulticastGroup(Socket *socket, const IpAddr *groupAddr)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0)
   error_t error;
   SocketMulticastGroup *group;

   //Check parameters
   if(socket == NULL || groupAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connectionless or raw sockets
   if(socket->type != SOCKET_TYPE_DGRAM &&
      socket->type != SOCKET_TYPE_RAW_IP)
   {
      return ERROR_INVALID_SOCKET;
   }

   //The group address must be a valid multicast address
   if(!ipIsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Initialize status code
   error = NO_ERROR;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the list of multicast groups for the specified address
   group = socketFindMulticastGroupEntry(socket, groupAddr);

   //Any matching entry found?
   if(group != NULL)
   {
      //Delete entry
      socketDeleteMulticastGroupEntry(group);

      //Update the multicast reception state of the interface
      ipUpdateMulticastFilter(socket->interface, groupAddr);
   }
   else
   {
      //Report an error
      error = ERROR_ADDRESS_NOT_FOUND;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Set multicast source filter (full-state API)
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying a multicast group
 * @param[in] filterMode Multicast filter mode (include or exclude)
 * @param[in] sources IP addresses of sources to include or exclude depending
 *   on the filter mode
 * @param[in] numSources Number of source addresses in the list
 * @return Error code
 **/

error_t socketSetMulticastSourceFilter(Socket *socket, const IpAddr *groupAddr,
   IpFilterMode filterMode, const IpAddr *sources, uint_t numSources)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0 && SOCKET_MAX_MULTICAST_SOURCES > 0)
   error_t error;
   uint_t i;
   SocketMulticastGroup *group;

   //Check parameters
   if(socket == NULL || groupAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connectionless or raw sockets
   if(socket->type != SOCKET_TYPE_DGRAM &&
      socket->type != SOCKET_TYPE_RAW_IP)
   {
      return ERROR_INVALID_SOCKET;
   }

   //The group address must be a valid multicast address
   if(!ipIsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Invalid filter mode?
   if(filterMode != IP_FILTER_MODE_INCLUDE &&
      filterMode != IP_FILTER_MODE_EXCLUDE)
   {
      return ERROR_INVALID_PARAMETER;
   }

   //Invalid set of source addresses?
   if(numSources != 0 && sources == NULL)
      return ERROR_INVALID_PARAMETER;

   //The implementation limits the number of source addresses
   if(numSources > SOCKET_MAX_MULTICAST_SOURCES)
      return ERROR_INVALID_PARAMETER;

   //Initialize status code
   error = NO_ERROR;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the list of multicast groups for the specified address
   group = socketFindMulticastGroupEntry(socket, groupAddr);

   //The "non-existent" state is considered to have a filter mode of INCLUDE
   //and an empty source list
   if(filterMode == IP_FILTER_MODE_INCLUDE && numSources == 0)
   {
      //Any matching entry found?
      if(group != NULL)
      {
         //Delete entry
         socketDeleteMulticastGroupEntry(group);
      }
   }
   else
   {
      //No matching entry found?
      if(group == NULL)
      {
         //Create a new entry
         group = socketCreateMulticastGroupEntry(socket, groupAddr);
      }

      //Entry successfully created?
      if(group != NULL)
      {
         //Replace the previous filter with a new one
         group->filterMode = filterMode;
         group->numSources = numSources;

         //Save the list of sources to include or exclude
         for(i = 0; i < numSources; i++)
         {
            group->sources[i] = sources[i];
         }
      }
      else
      {
         //Report an error
         error = ERROR_OUT_OF_RESOURCES;
      }
   }

   //Check status code
   if(!error)
   {
      //Update the multicast reception state of the interface
      ipUpdateMulticastFilter(socket->interface, groupAddr);
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Get multicast source filter
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying a multicast group
 * @param[out] filterMode Multicast filter mode (include or exclude)
 * @param[out] sources IP addresses of sources to include or exclude depending
 *   on the filter mode
 * @param[out] numSources Number of source addresses in the list
 * @return Error code
 **/

error_t socketGetMulticastSourceFilter(Socket *socket, const IpAddr *groupAddr,
   IpFilterMode *filterMode, IpAddr *sources, uint_t *numSources)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0 && SOCKET_MAX_MULTICAST_SOURCES > 0)
   uint_t i;
   SocketMulticastGroup *group;

   //Check parameters
   if(socket == NULL || groupAddr == NULL || numSources == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connectionless or raw sockets
   if(socket->type != SOCKET_TYPE_DGRAM &&
      socket->type != SOCKET_TYPE_RAW_IP)
   {
      return ERROR_INVALID_SOCKET;
   }

   //The group address must be a valid multicast address
   if(!ipIsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the list of multicast groups for the specified address
   group = socketFindMulticastGroupEntry(socket, groupAddr);

   //Any matching entry found?
   if(group != NULL)
   {
      //Filter mode
      *filterMode = group->filterMode;
      //Number of source addresses in the list
      *numSources = group->numSources;

      //This parameter is optional
      if(sources != NULL)
      {
         //Copy the list of source addresses
         for(i = 0; i < group->numSources; i++)
         {
            sources[i] = group->sources[i];
         }
      }
   }
   else
   {
      //The "non-existent" state is considered to have a filter mode of INCLUDE
      //and an empty source list
      *filterMode = IP_FILTER_MODE_EXCLUDE;
      *numSources = 0;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Accept specific source for specific group (delta-based API)
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying a multicast group
 * @param[in] srcAddr Source IP address to accept
 * @return Error code
 **/

error_t socketAddMulticastSource(Socket *socket, const IpAddr *groupAddr,
   const IpAddr *srcAddr)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0 && SOCKET_MAX_MULTICAST_SOURCES > 0)
   error_t error;
   SocketMulticastGroup *group;

   //Check parameters
   if(socket == NULL || groupAddr == NULL || srcAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connectionless or raw sockets
   if(socket->type != SOCKET_TYPE_DGRAM &&
      socket->type != SOCKET_TYPE_RAW_IP)
   {
      return ERROR_INVALID_SOCKET;
   }

   //The group address must be a valid multicast address
   if(!ipIsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Initialize status code
   error = NO_ERROR;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the list of multicast groups for en existing entry
   group = socketFindMulticastGroupEntry(socket, groupAddr);

   //No matching entry found?
   if(group == NULL)
   {
      //Create a new entry
      group = socketCreateMulticastGroupEntry(socket, groupAddr);

      //Failed to create entry?
      if(group == NULL)
      {
         //Report an error
         error = ERROR_OUT_OF_RESOURCES;
      }
   }

   //Check status code
   if(!error)
   {
      //Check multicast filter mode
      if(group->filterMode == IP_FILTER_MODE_EXCLUDE)
      {
         //Switch to source-specific multicast mode
         group->filterMode = IP_FILTER_MODE_INCLUDE;
         group->numSources = 0;
      }

      //Add the address to the list of allowed sources
      error = socketAddMulticastSrcAddr(group, srcAddr);
   }

   //Check status code
   if(!error)
   {
      //Update the multicast reception state of the interface
      ipUpdateMulticastFilter(socket->interface, groupAddr);
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Drop specific source for specific group (delta-based API)
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying a multicast group
 * @param[in] srcAddr Source IP address to drop
 * @return Error code
 **/

error_t socketDropMulticastSource(Socket *socket, const IpAddr *groupAddr,
   const IpAddr *srcAddr)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0 && SOCKET_MAX_MULTICAST_SOURCES > 0)
   error_t error;
   SocketMulticastGroup *group;

   //Check parameters
   if(socket == NULL || groupAddr == NULL || srcAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connectionless or raw sockets
   if(socket->type != SOCKET_TYPE_DGRAM &&
      socket->type != SOCKET_TYPE_RAW_IP)
   {
      return ERROR_INVALID_SOCKET;
   }

   //The group address must be a valid multicast address
   if(!ipIsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Initialize status code
   error = NO_ERROR;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the list of multicast groups for en existing entry
   group = socketFindMulticastGroupEntry(socket, groupAddr);

   //Any matching entry found?
   if(group != NULL)
   {
      //Check multicast filter mode
      if(group->filterMode == IP_FILTER_MODE_INCLUDE)
      {
         //Remove the address to the list of allowed sources
         socketRemoveMulticastSrcAddr(group, srcAddr);

         //Empty set of source addresses?
         if(group->numSources == 0)
         {
            //Delete entry
            socketDeleteMulticastGroupEntry(group);
         }

         //Update the multicast reception state of the interface
         ipUpdateMulticastFilter(socket->interface, groupAddr);
      }
   }
   else
   {
      //Report an error
      error = ERROR_ADDRESS_NOT_FOUND;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Block specific source for specific group (delta-based API)
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying a multicast group
 * @param[in] srcAddr Source IP address to block
 * @return Error code
 **/

error_t socketBlockMulticastSource(Socket *socket, const IpAddr *groupAddr,
   const IpAddr *srcAddr)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0 && SOCKET_MAX_MULTICAST_SOURCES > 0)
   error_t error;
   SocketMulticastGroup *group;

   //Check parameters
   if(socket == NULL || groupAddr == NULL || srcAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connectionless or raw sockets
   if(socket->type != SOCKET_TYPE_DGRAM &&
      socket->type != SOCKET_TYPE_RAW_IP)
   {
      return ERROR_INVALID_SOCKET;
   }

   //The group address must be a valid multicast address
   if(!ipIsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Initialize status code
   error = NO_ERROR;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the list of multicast groups for en existing entry
   group = socketFindMulticastGroupEntry(socket, groupAddr);

   //No matching entry found?
   if(group == NULL)
   {
      //Create a new entry
      group = socketCreateMulticastGroupEntry(socket, groupAddr);

      //Failed to create entry?
      if(group == NULL)
      {
         //Report an error
         error = ERROR_OUT_OF_RESOURCES;
      }
   }

   //Check status code
   if(!error)
   {
      //Check multicast filter mode
      if(group->filterMode == IP_FILTER_MODE_INCLUDE)
      {
         //Switch to any-source multicast mode
         group->filterMode = IP_FILTER_MODE_EXCLUDE;
         group->numSources = 0;
      }

      //Add the address to the list of blocked sources
      error = socketAddMulticastSrcAddr(group, srcAddr);
   }

   //Check status code
   if(!error)
   {
      //Update the multicast reception state of the interface
      ipUpdateMulticastFilter(socket->interface, groupAddr);
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Unblock specific source for specific group (delta-based API)
 * @param[in] socket Handle to a socket
 * @param[in] groupAddr IP address identifying a multicast group
 * @param[in] srcAddr Source IP address to unblock
 * @return Error code
 **/

error_t socketUnblockMulticastSource(Socket *socket, const IpAddr *groupAddr,
   const IpAddr *srcAddr)
{
#if (SOCKET_MAX_MULTICAST_GROUPS > 0 && SOCKET_MAX_MULTICAST_SOURCES > 0)
   error_t error;
   SocketMulticastGroup *group;

   //Check parameters
   if(socket == NULL || groupAddr == NULL || srcAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connectionless or raw sockets
   if(socket->type != SOCKET_TYPE_DGRAM &&
      socket->type != SOCKET_TYPE_RAW_IP)
   {
      return ERROR_INVALID_SOCKET;
   }

   //The group address must be a valid multicast address
   if(!ipIsMulticastAddr(groupAddr))
      return ERROR_INVALID_ADDRESS;

   //Initialize status code
   error = NO_ERROR;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Search the list of multicast groups for en existing entry
   group = socketFindMulticastGroupEntry(socket, groupAddr);

   //Any matching entry found?
   if(group != NULL)
   {
      //Check multicast filter mode
      if(group->filterMode == IP_FILTER_MODE_EXCLUDE)
      {
         //Remove the address from the list of blocked sources
         socketRemoveMulticastSrcAddr(group, srcAddr);

         //Update the multicast reception state of the interface
         ipUpdateMulticastFilter(socket->interface, groupAddr);
      }
   }
   else
   {
      //Report an error
      error = ERROR_ADDRESS_NOT_FOUND;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Enable TCP keep-alive
 * @param[in] socket Handle to a socket
 * @param[in] enabled Specifies whether TCP keep-alive is enabled
 * @return Error code
 **/

error_t socketEnableKeepAlive(Socket *socket, bool_t enabled)
{
#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Check whether TCP keep-alive mechanism should be enabled
   if(enabled)
   {
      //Enable TCP keep-alive mechanism
      socket->keepAliveEnabled = TRUE;
      //Reset keep-alive probe counter
      socket->keepAliveProbeCount = 0;
      //Start keep-alive timer
      socket->keepAliveTimestamp = osGetSystemTime();
   }
   else
   {
      //Disable TCP keep-alive mechanism
      socket->keepAliveEnabled = FALSE;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Set TCP keep-alive parameters
 * @param[in] socket Handle to a socket
 * @param[in] idle Time interval between last data packet sent and first
 *   keep-alive probe
 * @param[in] interval Time interval between subsequent keep-alive probes
 * @param[in] maxProbes Number of unacknowledged keep-alive probes to send before
 *   considering the connection is dead
 * @return Error code
 **/

error_t socketSetKeepAliveParams(Socket *socket, systime_t idle,
   systime_t interval, uint_t maxProbes)
{
#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Check parameters
   if(socket == NULL || idle == 0 || interval == 0 || maxProbes == 0)
   {
      return ERROR_INVALID_PARAMETER;
   }

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Time interval between last data packet sent and first keep-alive probe
   socket->keepAliveIdle = idle;

   //Time interval between subsequent keep-alive probes
   socket->keepAliveInterval = interval;

   //Number of unacknowledged keep-alive probes to send before considering
   //the connection is dead
   socket->keepAliveMaxProbes = maxProbes;

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Specify the maximum segment size for outgoing TCP packets
 * @param[in] socket Handle to a socket
 * @param[in] mss Maximum segment size, in bytes
 * @return Error code
 **/

error_t socketSetMaxSegmentSize(Socket *socket, size_t mss)
{
#if (TCP_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //TCP imposes its minimum and maximum bounds over the value provided
   mss = MIN(mss, TCP_MAX_MSS);
   mss = MAX(mss, TCP_MIN_MSS);

   //Set the maximum segment size for outgoing TCP packets. If this option
   //is set before connection establishment, it also change the MSS value
   //announced to the other end in the initial SYN packet
   socket->mss = mss;

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
#else
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Specify the size of the TCP send buffer
 * @param[in] socket Handle to a socket
 * @param[in] size Desired buffer size, in bytes
 * @return Error code
 **/

error_t socketSetTxBufferSize(Socket *socket, size_t size)
{
#if (TCP_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connection-oriented sockets
   if(socket->type != SOCKET_TYPE_STREAM)
      return ERROR_INVALID_SOCKET;

   //The buffer size cannot be changed when the connection is established
   if(tcpGetState(socket) != TCP_STATE_CLOSED)
      return ERROR_INVALID_SOCKET;

   //Check parameter value
   if(size < 1 || size > TCP_MAX_TX_BUFFER_SIZE)
      return ERROR_INVALID_PARAMETER;

   //Use the specified buffer size
   socket->txBufferSize = size;

   //No error to report
   return NO_ERROR;
#else
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Specify the size of the TCP receive buffer
 * @param[in] socket Handle to a socket
 * @param[in] size Desired buffer size, in bytes
 * @return Error code
 **/

error_t socketSetRxBufferSize(Socket *socket, size_t size)
{
#if (TCP_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connection-oriented sockets
   if(socket->type != SOCKET_TYPE_STREAM)
      return ERROR_INVALID_SOCKET;

   //The buffer size cannot be changed when the connection is established
   if(tcpGetState(socket) != TCP_STATE_CLOSED)
      return ERROR_INVALID_SOCKET;

   //Check parameter value
   if(size < 1 || size > TCP_MAX_RX_BUFFER_SIZE)
      return ERROR_INVALID_PARAMETER;

   //Use the specified buffer size
   socket->rxBufferSize = size;

   //Compute the window scale factor to use for the receive window
   tcpComputeWindowScaleFactor(socket);

   //No error to report
   return NO_ERROR;
#else
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Bind a socket to a particular network interface
 * @param[in] socket Handle to a socket
 * @param[in] interface Network interface to be used
 * @return Error code
 **/

error_t socketSetInterface(Socket *socket, NetInterface *interface)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Explicitly associate the socket with the specified interface
   socket->interface = interface;

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Retrieve the underlying interface
 * @param[in] socket Handle to a socket
 * @return Pointer to the underlying network interface
 **/

NetInterface *socketGetInterface(Socket *socket)
{
   NetInterface *interface = NULL;

   //Make sure the socket handle is valid
   if(socket != NULL)
   {
      interface = socket->interface;
   }

   //Return a pointer to the underlying network interface
   return interface;
}


/**
 * @brief Associate a local address with a socket
 * @param[in] socket Handle to a socket
 * @param[in] localIpAddr Local address to assign to the bound socket
 * @param[in] localPort Local port number to assign to the bound socket
 * @return Error code
 **/

error_t socketBind(Socket *socket, const IpAddr *localIpAddr,
   uint16_t localPort)
{
   //Check input parameters
   if(socket == NULL || localIpAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Make sure the socket type is correct
   if(socket->type != SOCKET_TYPE_STREAM &&
      socket->type != SOCKET_TYPE_DGRAM &&
      socket->type != SOCKET_TYPE_RAW_IP)
   {
      return ERROR_INVALID_SOCKET;
   }

   //Associate the specified IP address and port number
   socket->localIpAddr = *localIpAddr;
   socket->localPort = localPort;

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Establish a connection to a specified socket
 * @param[in] socket Handle to an unconnected socket
 * @param[in] remoteIpAddr IP address of the remote host
 * @param[in] remotePort Remote port number that will be used to establish
 *   the connection
 * @return Error code
 **/

error_t socketConnect(Socket *socket, const IpAddr *remoteIpAddr,
   uint16_t remotePort)
{
   error_t error;

   //Check input parameters
   if(socket == NULL || remoteIpAddr == NULL)
      return ERROR_INVALID_PARAMETER;

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(socket->type == SOCKET_TYPE_STREAM)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Establish TCP connection
      error = tcpConnect(socket, remoteIpAddr, remotePort);

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }
   else
#endif
   //Connectionless socket?
   if(socket->type == SOCKET_TYPE_DGRAM)
   {
      //Save port number and IP address of the remote host
      socket->remoteIpAddr = *remoteIpAddr;
      socket->remotePort = remotePort;
      //No error to report
      error = NO_ERROR;
   }
   //Raw socket?
   else if(socket->type == SOCKET_TYPE_RAW_IP)
   {
      //Save the IP address of the remote host
      socket->remoteIpAddr = *remoteIpAddr;
      //No error to report
      error = NO_ERROR;
   }
   //Invalid socket type?
   else
   {
      //Report an error
      error = ERROR_INVALID_SOCKET;
   }

   //Return status code
   return error;
}


/**
 * @brief Place a socket in the listening state
 *
 * Place a socket in a state in which it is listening for an incoming connection
 *
 * @param[in] socket Socket to place in the listening state
 * @param[in] backlog backlog The maximum length of the pending connection queue.
 *   If this parameter is zero, then the default backlog value is used instead
 * @return Error code
 **/

error_t socketListen(Socket *socket, uint_t backlog)
{
#if (TCP_SUPPORT == ENABLED)
   error_t error;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connection-oriented sockets
   if(socket->type != SOCKET_TYPE_STREAM)
      return ERROR_INVALID_SOCKET;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Start listening for an incoming connection
   error = tcpListen(socket, backlog);

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Permit an incoming connection attempt on a socket
 * @param[in] socket Handle to a socket previously placed in a listening state
 * @param[out] clientIpAddr IP address of the client
 * @param[out] clientPort Port number used by the client
 * @return Handle to the socket in which the actual connection is made
 **/

Socket *socketAccept(Socket *socket, IpAddr *clientIpAddr,
   uint16_t *clientPort)
{
#if (TCP_SUPPORT == ENABLED)
   Socket *newSocket;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return NULL;

   //This function shall be used with connection-oriented sockets
   if(socket->type != SOCKET_TYPE_STREAM)
      return NULL;

   //Accept an incoming connection attempt
   newSocket = tcpAccept(socket, clientIpAddr, clientPort);

   //Return a handle to the newly created socket
   return newSocket;
#else
   return NULL;
#endif
}


/**
 * @brief Send data to a connected socket
 * @param[in] socket Handle that identifies a connected socket
 * @param[in] data Pointer to a buffer containing the data to be transmitted
 * @param[in] length Number of data bytes to send
 * @param[out] written Actual number of bytes written (optional parameter)
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketSend(Socket *socket, const void *data, size_t length,
   size_t *written, uint_t flags)
{
   //Use default remote IP address for connectionless or raw sockets
   return socketSendTo(socket, &socket->remoteIpAddr, socket->remotePort,
      data, length, written, flags);
}


/**
 * @brief Send a datagram to a specific destination
 * @param[in] socket Handle that identifies a socket
 * @param[in] destIpAddr IP address of the target host
 * @param[in] destPort Target port number
 * @param[in] data Pointer to a buffer containing the data to be transmitted
 * @param[in] length Number of data bytes to send
 * @param[out] written Actual number of bytes written (optional parameter)
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketSendTo(Socket *socket, const IpAddr *destIpAddr, uint16_t destPort,
   const void *data, size_t length, size_t *written, uint_t flags)
{
   error_t error;

   //No data has been transmitted yet
   if(written != NULL)
   {
      *written = 0;
   }

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(socket->type == SOCKET_TYPE_STREAM)
   {
      //For connection-oriented sockets, target address is ignored
      error = tcpSend(socket, data, length, written, flags);
   }
   else
#endif
   {
      SocketMsg message;

      //Initialize structure
      message = SOCKET_DEFAULT_MSG;

      //Set destination IP address
      if(destIpAddr != NULL)
      {
         message.destIpAddr = *destIpAddr;
      }

      //Set destination port
      message.destPort = destPort;

#if (UDP_SUPPORT == ENABLED)
      //Connectionless socket?
      if(socket->type == SOCKET_TYPE_DGRAM)
      {
         //Set data payload
         message.data = (uint8_t *) data;
         message.length = length;

         //Send UDP datagram
         error = udpSendDatagram(socket, &message, flags);
      }
      else
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
      //Raw socket?
      if(socket->type == SOCKET_TYPE_RAW_IP)
      {
         //Set data payload
         message.data = (uint8_t *) data;
         message.length = length;

         //Send a raw IP packet
         error = rawSocketSendIpPacket(socket, &message, flags);
      }
      else if(socket->type == SOCKET_TYPE_RAW_ETH)
      {
         //Sanity check
         if(length >= sizeof(EthHeader))
         {
            EthHeader *header;

            //Point to the Ethernet header
            header = (EthHeader *) data;

            //Set source and destination MAC addresses
            message.srcMacAddr = header->srcAddr;
            message.destMacAddr = header->destAddr;

            //Set the value of the EtherType field
            message.ethType = ntohs(header->type);

            //Set data payload
            message.data = (uint8_t *) data + sizeof(EthHeader);
            message.length = length - sizeof(EthHeader);

            //Send a raw Ethernet packet
            error = rawSocketSendEthPacket(socket, &message, flags);
         }
         else
         {
            //Report an error
            error = ERROR_INVALID_LENGTH;
         }
      }
      else
#endif
      //Invalid socket type?
      {
         //Report an error
         error = ERROR_INVALID_SOCKET;
      }

      //Check status code
      if(!error)
      {
         //Total number of data bytes successfully transmitted
         if(written != NULL)
         {
            *written = message.length;
         }
      }
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Send a message to a connectionless socket
 * @param[in] socket Handle that identifies a socket
 * @param[in] message Pointer to the structure describing the message
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketSendMsg(Socket *socket, const SocketMsg *message, uint_t flags)
{
   error_t error;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (UDP_SUPPORT == ENABLED)
   //Connectionless socket?
   if(socket->type == SOCKET_TYPE_DGRAM)
   {
      //Send UDP datagram
      error = udpSendDatagram(socket, message, flags);
   }
   else
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
   //Raw socket?
   if(socket->type == SOCKET_TYPE_RAW_IP)
   {
      //Send a raw IP packet
      error = rawSocketSendIpPacket(socket, message, flags);
   }
   else if(socket->type == SOCKET_TYPE_RAW_ETH)
   {
      //Send a raw Ethernet packet
      error = rawSocketSendEthPacket(socket, message, flags);
   }
   else
#endif
   //Invalid socket type?
   {
      //Report an error
      error = ERROR_INVALID_SOCKET;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Receive data from a connected socket
 * @param[in] socket Handle that identifies a connected socket
 * @param[out] data Buffer where to store the incoming data
 * @param[in] size Maximum number of bytes that can be received
 * @param[out] received Number of bytes that have been received
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketReceive(Socket *socket, void *data,
   size_t size, size_t *received, uint_t flags)
{
   //For connection-oriented sockets, source and destination addresses are
   //no use
   return socketReceiveEx(socket, NULL, NULL, NULL, data, size, received,
      flags);
}


/**
 * @brief Receive a datagram from a connectionless socket
 * @param[in] socket Handle that identifies a socket
 * @param[out] srcIpAddr Source IP address (optional)
 * @param[out] srcPort Source port number (optional)
 * @param[out] data Buffer where to store the incoming data
 * @param[in] size Maximum number of bytes that can be received
 * @param[out] received Number of bytes that have been received
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketReceiveFrom(Socket *socket, IpAddr *srcIpAddr, uint16_t *srcPort,
   void *data, size_t size, size_t *received, uint_t flags)
{
   //Destination address is no use
   return socketReceiveEx(socket, srcIpAddr, srcPort, NULL, data, size,
      received, flags);
}


/**
 * @brief Receive a datagram
 * @param[in] socket Handle that identifies a socket
 * @param[out] srcIpAddr Source IP address (optional)
 * @param[out] srcPort Source port number (optional)
 * @param[out] destIpAddr Destination IP address (optional)
 * @param[out] data Buffer where to store the incoming data
 * @param[in] size Maximum number of bytes that can be received
 * @param[out] received Number of bytes that have been received
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketReceiveEx(Socket *socket, IpAddr *srcIpAddr, uint16_t *srcPort,
   IpAddr *destIpAddr, void *data, size_t size, size_t *received, uint_t flags)
{
   error_t error;

   //No data has been received yet
   if(received != NULL)
      *received = 0;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(socket->type == SOCKET_TYPE_STREAM)
   {
      //Receive data
      error = tcpReceive(socket, data, size, received, flags);

      //Save the source IP address
      if(srcIpAddr != NULL)
      {
         *srcIpAddr = socket->remoteIpAddr;
      }

      //Save the source port number
      if(srcPort != NULL)
      {
         *srcPort = socket->remotePort;
      }

      //Save the destination IP address
      if(destIpAddr != NULL)
      {
         *destIpAddr = socket->localIpAddr;
      }
   }
   else
#endif
   {
      SocketMsg message;

      //Initialize structure
      message = SOCKET_DEFAULT_MSG;

#if (UDP_SUPPORT == ENABLED)
      //Connectionless socket?
      if(socket->type == SOCKET_TYPE_DGRAM)
      {
         //Set data buffer
         message.data = data;
         message.size = size;

         //Receive UDP datagram
         error = udpReceiveDatagram(socket, &message, flags);
      }
      else
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
      //Raw socket?
      if(socket->type == SOCKET_TYPE_RAW_IP)
      {
         //Set data buffer
         message.data = data;
         message.size = size;

         //Receive a raw IP packet
         error = rawSocketReceiveIpPacket(socket, &message, flags);
      }
      else if(socket->type == SOCKET_TYPE_RAW_ETH)
      {
         //Sanity check
         if(size >= sizeof(EthHeader))
         {
            //Set data buffer
            message.data = (uint8_t *) data + sizeof(EthHeader);
            message.size = size - sizeof(EthHeader);

            //Receive a raw Ethernet packet
            error = rawSocketReceiveEthPacket(socket, &message, flags);

            //Check status code
            if(!error)
            {
               EthHeader *header;

               //Point to the Ethernet header
               header = (EthHeader *) data;

               //Reconstruct Ethernet header
               header->destAddr = message.destMacAddr;
               header->srcAddr = message.srcMacAddr;
               header->type = htons(message.ethType);

               //Ajust the length of the frame
               message.length += sizeof(EthHeader);
            }
         }
         else
         {
            //Report an error
            error = ERROR_BUFFER_OVERFLOW;
         }
      }
      else
#endif
      //Invalid socket type?
      {
         //Report an error
         error = ERROR_INVALID_SOCKET;
      }

      //Check status code
      if(!error)
      {
         //Save the source IP address
         if(srcIpAddr != NULL)
         {
            *srcIpAddr = message.srcIpAddr;
         }

         //Save the source port number
         if(srcPort != NULL)
         {
            *srcPort = message.srcPort;
         }

         //Save the destination IP address
         if(destIpAddr != NULL)
         {
            *destIpAddr = message.destIpAddr;
         }

         //Total number of data that have been received
         *received = message.length;
      }
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Receive a message from a connectionless socket
 * @param[in] socket Handle that identifies a socket
 * @param[in,out] message Pointer to the structure describing the message
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketReceiveMsg(Socket *socket, SocketMsg *message, uint_t flags)
{
   error_t error;

   //No data has been received yet
   message->length = 0;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (UDP_SUPPORT == ENABLED)
   //Connectionless socket?
   if(socket->type == SOCKET_TYPE_DGRAM)
   {
      //Receive UDP datagram
      error = udpReceiveDatagram(socket, message, flags);
   }
   else
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
   //Raw socket?
   if(socket->type == SOCKET_TYPE_RAW_IP)
   {
      //Receive a raw IP packet
      error = rawSocketReceiveIpPacket(socket, message, flags);
   }
   else if(socket->type == SOCKET_TYPE_RAW_ETH)
   {
      //Receive a raw Ethernet packet
      error = rawSocketReceiveEthPacket(socket, message, flags);
   }
   else
#endif
   //Invalid socket type?
   {
      //Report an error
      error = ERROR_INVALID_SOCKET;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Retrieve the local address for a given socket
 * @param[in] socket Handle that identifies a socket
 * @param[out] localIpAddr Local IP address (optional)
 * @param[out] localPort Local port number (optional)
 * @return Error code
 **/

error_t socketGetLocalAddr(Socket *socket, IpAddr *localIpAddr,
   uint16_t *localPort)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Retrieve local IP address
   if(localIpAddr != NULL)
   {
      *localIpAddr = socket->localIpAddr;
   }

   //Retrieve local port number
   if(localPort != NULL)
   {
      *localPort = socket->localPort;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve the address of the peer to which a socket is connected
 * @param[in] socket Handle that identifies a socket
 * @param[out] remoteIpAddr IP address of the remote host (optional)
 * @param[out] remotePort Remote port number (optional)
 * @return Error code
 **/

error_t socketGetRemoteAddr(Socket *socket, IpAddr *remoteIpAddr,
   uint16_t *remotePort)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Retrieve local IP address
   if(remoteIpAddr != NULL)
   {
      *remoteIpAddr = socket->remoteIpAddr;
   }

   //Retrieve local port number
   if(remotePort != NULL)
   {
      *remotePort = socket->remotePort;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Disable reception, transmission, or both
 *
 * Note that socketShutdown() does not close the socket, and resources attached
 * to the socket will not be freed until socketClose() is invoked
 *
 * @param[in] socket Handle to a socket
 * @param[in] how Flag that describes what types of operation will no longer be allowed
 * @return Error code
 **/

error_t socketShutdown(Socket *socket, uint_t how)
{
#if (TCP_SUPPORT == ENABLED)
   error_t error;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Make sure the socket type is correct
   if(socket->type != SOCKET_TYPE_STREAM)
      return ERROR_INVALID_SOCKET;

   //Check flags
   if(how != SOCKET_SD_SEND && how != SOCKET_SD_RECEIVE &&
      how != SOCKET_SD_BOTH)
   {
      return ERROR_INVALID_PARAMETER;
   }

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Graceful shutdown
   error = tcpShutdown(socket, how);

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Close an existing socket
 * @param[in] socket Handle identifying the socket to close
 **/

void socketClose(Socket *socket)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return;

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (SOCKET_MAX_MULTICAST_GROUPS > 0)
   //Connectionless or raw socket?
   if(socket->type == SOCKET_TYPE_DGRAM ||
      socket->type == SOCKET_TYPE_RAW_IP)
   {
      uint_t i;
      IpAddr groupAddr;

      //Loop through multicast groups
      for(i = 0; i < SOCKET_MAX_MULTICAST_GROUPS; i++)
      {
         //Get current group address
         groupAddr = socket->multicastGroups[i].addr;

         //Valid multicast address?
         if(groupAddr.length != 0)
         {
            //Delete entry
            socket->multicastGroups[i].addr = IP_ADDR_UNSPECIFIED;

            //Update the multicast reception state of the interface
            ipUpdateMulticastFilter(socket->interface, &groupAddr);
         }
      }
   }
#endif

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(socket->type == SOCKET_TYPE_STREAM)
   {
      //Abort the current TCP connection
      tcpAbort(socket);
   }
#endif
#if (UDP_SUPPORT == ENABLED || RAW_SOCKET_SUPPORT == ENABLED)
   //Connectionless socket or raw socket?
   if(socket->type == SOCKET_TYPE_DGRAM ||
      socket->type == SOCKET_TYPE_RAW_IP ||
      socket->type == SOCKET_TYPE_RAW_ETH)
   {
      //Point to the first item in the receive queue
      SocketQueueItem *queueItem = socket->receiveQueue;

      //Purge the receive queue
      while(queueItem != NULL)
      {
         //Keep track of the next item in the queue
         SocketQueueItem *nextQueueItem = queueItem->next;
         //Free previously allocated memory
         netBufferFree(queueItem->buffer);
         //Point to the next item
         queueItem = nextQueueItem;
      }

      //Mark the socket as closed
      socket->type = SOCKET_TYPE_UNUSED;
   }
#endif

   //Release exclusive access
   osReleaseMutex(&netMutex);
}


/**
 * @brief Wait for one of a set of sockets to become ready to perform I/O
 *
 * This function determines the status of one or more sockets, waiting if
 *   necessary, to perform synchronous I/O
 *
 * @param[in,out] eventDesc Set of entries specifying the events the user is interested in
 * @param[in] size Number of entries in the descriptor set
 * @param[in] extEvent External event that can abort the wait if necessary (optional)
 * @param[in] timeout Maximum time to wait before returning
 * @return Error code
 **/

error_t socketPoll(SocketEventDesc *eventDesc, uint_t size, OsEvent *extEvent,
   systime_t timeout)
{
   error_t error;
   uint_t i;
   bool_t status;
   OsEvent *event;
   OsEvent eventObject;

   //Check parameters
   if(eventDesc == NULL || size == 0)
      return ERROR_INVALID_PARAMETER;

   //Try to use the supplied event object to receive notifications
   if(extEvent == NULL)
   {
      //Create an event object only if necessary
      if(!osCreateEvent(&eventObject))
      {
         //Report an error
         return ERROR_OUT_OF_RESOURCES;
      }

      //Reference to the newly created event
      event = &eventObject;
   }
   else
   {
      //Reference to the external event
      event = extEvent;
   }

   //Loop through descriptors
   for(i = 0; i < size; i++)
   {
      //Valid socket handle?
      if(eventDesc[i].socket != NULL)
      {
         //Clear event flags
         eventDesc[i].eventFlags = 0;

         //Subscribe to the requested events
         socketRegisterEvents(eventDesc[i].socket, event,
            eventDesc[i].eventMask);
      }
   }

   //Block the current task until an event occurs
   status = osWaitForEvent(event, timeout);

   //Any event?
   if(status)
   {
      //Clear flag
      status = FALSE;

      //Loop through descriptors
      for(i = 0; i < size; i++)
      {
         //Valid socket handle?
         if(eventDesc[i].socket != NULL)
         {
            //Retrieve event flags for the current socket
            eventDesc[i].eventFlags = socketGetEvents(eventDesc[i].socket);
            //Clear unnecessary flags
            eventDesc[i].eventFlags &= eventDesc[i].eventMask;

            //Check whether the socket is ready to perform I/O
            if(eventDesc[i].eventFlags != 0)
            {
               status = TRUE;
            }
         }
      }

      //Any socket event in the signaled state?
      if(status)
      {
         error = NO_ERROR;
      }
      else
      {
         error = ERROR_WAIT_CANCELED;
      }
   }
   else
   {
      //Report a timeout error
      error = ERROR_TIMEOUT;
   }

   //Loop through descriptors
   for(i = 0; i < size; i++)
   {
      //Valid socket handle?
      if(eventDesc[i].socket != NULL)
      {
         //Unsubscribe previously registered events
         socketUnregisterEvents(eventDesc[i].socket);
      }
   }

   //Reset event object
   osResetEvent(event);

   //Release previously allocated resources
   if(extEvent == NULL)
   {
      osDeleteEvent(&eventObject);
   }

   //Return status code
   return error;
}


/**
 * @brief Resolve a host name into an IP address
 * @param[in] interface Underlying network interface (optional parameter)
 * @param[in] name Name of the host to be resolved
 * @param[out] ipAddr IP address corresponding to the specified host name
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t getHostByName(NetInterface *interface, const char_t *name,
   IpAddr *ipAddr, uint_t flags)
{
   error_t error;
   HostType type;
   HostnameResolver protocol;

   //Default address type depends on TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED)
   type = HOST_TYPE_IPV4;
#elif (IPV6_SUPPORT == ENABLED)
   type = HOST_TYPE_IPV6;
#else
   type = HOST_TYPE_ANY;
#endif

   //Default name resolution protocol depends on TCP/IP stack configuration
#if (DNS_CLIENT_SUPPORT == ENABLED)
   protocol = HOST_NAME_RESOLVER_DNS;
#elif (MDNS_CLIENT_SUPPORT == ENABLED)
   protocol = HOST_NAME_RESOLVER_MDNS;
#elif (NBNS_CLIENT_SUPPORT == ENABLED)
   protocol = HOST_NAME_RESOLVER_NBNS;
#elif (LLMNR_CLIENT_SUPPORT == ENABLED)
   protocol = HOST_NAME_RESOLVER_LLMNR;
#else
   protocol = HOST_NAME_RESOLVER_ANY;
#endif

   //Check parameters
   if(name == NULL || ipAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Use default network interface?
   if(interface == NULL)
   {
      interface = netGetDefaultInterface();
   }

   //The specified name can be either an IP or a host name
   error = ipStringToAddr(name, ipAddr);

   //Perform name resolution if necessary
   if(error)
   {
      //The user may provide a hint to choose between IPv4 and IPv6
      if((flags & HOST_TYPE_IPV4) != 0)
      {
         type = HOST_TYPE_IPV4;
      }
      else if((flags & HOST_TYPE_IPV6) != 0)
      {
         type = HOST_TYPE_IPV6;
      }
      else
      {
         //Just for sanity
      }

      //The user may provide a hint to to select the desired protocol to be used
      if((flags & HOST_NAME_RESOLVER_DNS) != 0)
      {
         //Use DNS to resolve the specified host name
         protocol = HOST_NAME_RESOLVER_DNS;
      }
      else if((flags & HOST_NAME_RESOLVER_MDNS) != 0)
      {
         //Use mDNS to resolve the specified host name
         protocol = HOST_NAME_RESOLVER_MDNS;
      }
      else if((flags & HOST_NAME_RESOLVER_NBNS) != 0)
      {
         //Use NBNS to resolve the specified host name
         protocol = HOST_NAME_RESOLVER_NBNS;
      }
      else if((flags & HOST_NAME_RESOLVER_LLMNR) != 0)
      {
         //Use LLMNR to resolve the specified host name
         protocol = HOST_NAME_RESOLVER_LLMNR;
      }
      else
      {
         //Retrieve the length of the host name to be resolved
         size_t n = osStrlen(name);

         //Select the most suitable protocol
         if(n >= 6 && osStrcasecmp(name + n - 6, ".local") == 0)
         {
#if (MDNS_CLIENT_SUPPORT == ENABLED)
            //Use mDNS to resolve the specified host name
            protocol = HOST_NAME_RESOLVER_MDNS;
#endif
         }
         else if(n <= 15 && osStrchr(name, '.') == NULL &&
            type == HOST_TYPE_IPV4)
         {
#if (NBNS_CLIENT_SUPPORT == ENABLED)
            //Use NetBIOS Name Service to resolve the specified host name
            protocol = HOST_NAME_RESOLVER_NBNS;
#elif (LLMNR_CLIENT_SUPPORT == ENABLED)
            //Use LLMNR to resolve the specified host name
            protocol = HOST_NAME_RESOLVER_LLMNR;
#endif
         }
         else if(osStrchr(name, '.') == NULL)
         {
#if (LLMNR_CLIENT_SUPPORT == ENABLED)
            //Use LLMNR to resolve the specified host name
            protocol = HOST_NAME_RESOLVER_LLMNR;
#endif
         }
      }

#if (DNS_CLIENT_SUPPORT == ENABLED)
      //Use DNS protocol?
      if(protocol == HOST_NAME_RESOLVER_DNS)
      {
         //Perform host name resolution
         error = dnsResolve(interface, name, type, ipAddr);
      }
      else
#endif
#if (MDNS_CLIENT_SUPPORT == ENABLED)
      //Use mDNS protocol?
      if(protocol == HOST_NAME_RESOLVER_MDNS)
      {
         //Perform host name resolution
         error = mdnsClientResolve(interface, name, type, ipAddr);
      }
      else
#endif
#if (NBNS_CLIENT_SUPPORT == ENABLED && IPV4_SUPPORT == ENABLED)
      //Use NetBIOS Name Service protocol?
      if(protocol == HOST_NAME_RESOLVER_NBNS)
      {
         //Perform host name resolution
         error = nbnsResolve(interface, name, ipAddr);
      }
      else
#endif
#if (LLMNR_CLIENT_SUPPORT == ENABLED)
      //Use LLMNR protocol?
      if(protocol == HOST_NAME_RESOLVER_LLMNR)
      {
         //Perform host name resolution
         error = llmnrResolve(interface, name, type, ipAddr);
      }
      else
#endif
      //Invalid protocol?
      {
         //Just for sanity
         (void) protocol;
         //Report an error
         error = ERROR_INVALID_PARAMETER;
      }
   }

   //Return status code
   return error;
}
