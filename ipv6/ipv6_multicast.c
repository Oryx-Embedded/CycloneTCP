/**
 * @file ipv6_multicast.c
 * @brief IPv6 multicast filtering
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
#define TRACE_LEVEL IPV6_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/socket_misc.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_multicast.h"
#include "mld/mld_node.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED)


/**
 * @brief Filter out incoming multicast traffic
 * @param[in] interface The interface on which the packet was received
 * @param[in] destAddr Destination IP address of the received packet
 * @param[in] srcAddr Source IP address of the received packet
 * @return Error code
 **/

error_t ipv6MulticastFilter(NetInterface *interface, const Ipv6Addr *destAddr,
   const Ipv6Addr *srcAddr)
{
   error_t error;
   uint_t i;
   Ipv6FilterEntry *entry;

   //Initialize status code
   error = ERROR_INVALID_ADDRESS;

   //Go through the multicast filter table
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE && error; i++)
   {
      //Point to the current entry
      entry = &interface->ipv6Context.multicastFilter[i];

      //Matching multicast address?
      if(ipv6CompAddr(&entry->addr, destAddr))
      {
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
         uint_t j;

         //Check filter mode
         if(entry->srcFilterMode == IP_FILTER_MODE_INCLUDE)
         {
            //In INCLUDE mode, reception of packets sent to the specified
            //multicast address is requested only from those IP source
            //addresses listed in the source list
            for(j = 0; j < entry->srcFilter.numSources && error; j++)
            {
               //Compare source addresses
               if(ipv6CompAddr(&entry->srcFilter.sources[j], srcAddr))
               {
                  error = NO_ERROR;
               }
            }
         }
         else
         {
            //In EXCLUDE mode, reception of packets sent to the given multicast
            //address is requested from all IP source addresses except those
            //listed in the source list
            error = NO_ERROR;

            //Loop through the list of excluded source addresses
            for(j = 0; j < entry->srcFilter.numSources && !error; j++)
            {
               //Compare source addresses
               if(ipv6CompAddr(&entry->srcFilter.sources[j], srcAddr))
               {
                  error = ERROR_INVALID_ADDRESS;
               }
            }
         }
#else
         //The multicast address is acceptable
         error = NO_ERROR;
#endif
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Join an IPv6 multicast group
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv6 Multicast address to join
 * @return Error code
 **/

error_t ipv6JoinMulticastGroup(NetInterface *interface,
   const Ipv6Addr *groupAddr)
{
   error_t error;
   Ipv6FilterEntry *entry;

   //Initialize status code
   error = NO_ERROR;

   //The IPv6 address must be a valid multicast address
   if(ipv6IsMulticastAddr(groupAddr))
   {
      //Search the IPv6 multicast filter table for the specified address
      entry = ipv6FindMulticastFilterEntry(interface, groupAddr);

      //No matching entry found?
      if(entry == NULL)
      {
         //Create a new entry
         entry = ipv6CreateMulticastFilterEntry(interface, groupAddr);
      }

      //Entry successfully created?
      if(entry != NULL)
      {
         //Increment the reference count
         entry->anySourceRefCount++;
      }
      else
      {
         //A new entry cannot be added to the multicast filter table
         error = ERROR_OUT_OF_RESOURCES;
      }
   }
   else
   {
      //The specified group address is not valid
      error = ERROR_INVALID_ADDRESS;
   }

   //Check status code
   if(!error)
   {
      //Update IPv6 multicast filter table
      ipv6UpdateMulticastFilter(interface, groupAddr);
   }

   //Return status code
   return error;
}


/**
 * @brief Leave an IPv6 multicast group
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv6 multicast address to drop
 * @return Error code
 **/

error_t ipv6LeaveMulticastGroup(NetInterface *interface,
   const Ipv6Addr *groupAddr)
{
   error_t error;
   Ipv6FilterEntry *entry;

   //Initialize status code
   error = NO_ERROR;

   //Search the IPv6 multicast filter table for the specified address
   entry = ipv6FindMulticastFilterEntry(interface, groupAddr);

   //Check whether a matching entry has been found
   if(entry != NULL)
   {
      //Decrement the reference count
      if(entry->anySourceRefCount > 0)
      {
         entry->anySourceRefCount--;
      }
   }
   else
   {
      //The specified IPv6 address does not exist
      error = ERROR_ADDRESS_NOT_FOUND;
   }

   //Check status code
   if(!error)
   {
      //Update IPv6 multicast filter table
      ipv6UpdateMulticastFilter(interface, groupAddr);
   }

   //Return status code
   return error;
}


/**
 * @brief Update IPv6 multicast filter table
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv6 multicast address
 **/

void ipv6UpdateMulticastFilter(NetInterface *interface,
   const Ipv6Addr *groupAddr)
{
   error_t error;
   uint_t i;
   Ipv6FilterEntry *entry;

   //First, reset the per-interface state
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->ipv6Context.multicastFilter[i];

      //Matching multicast address?
      if(ipv6CompAddr(groupAddr, &IPV6_UNSPECIFIED_ADDR) ||
         ipv6CompAddr(groupAddr, &entry->addr))
      {
         //Any-source multicast mode?
         if(entry->anySourceRefCount > 0)
         {
            entry->srcFilterMode = IP_FILTER_MODE_EXCLUDE;
            entry->srcFilter.numSources = 0;
         }
         else
         {
            entry->srcFilterMode = IP_FILTER_MODE_INCLUDE;
            entry->srcFilter.numSources = 0;
         }
      }
   }

#if (SOCKET_MAX_MULTICAST_GROUPS > 0)
   //The per-interface state is derived from the per-socket state, but may
   //differ from the per-socket state when different sockets have differing
   //filter modes and/or source lists for the same multicast address and
   //interface (refer to RFC 3376, section 3.2)
   for(i = 0; i < SOCKET_MAX_COUNT; i++)
   {
      uint_t j;
      Socket *socket;
      SocketMulticastGroup *group;

      //Point to the current socket
      socket = &socketTable[i];

      //Connectionless or raw socket?
      if(socket->type == SOCKET_TYPE_DGRAM ||
         socket->type == SOCKET_TYPE_RAW_IP)
      {
         //Loop through multicast groups
         for(j = 0; j < SOCKET_MAX_MULTICAST_GROUPS; j++)
         {
            //Point to the per-socket state
            group = &socket->multicastGroups[j];

            //IPv6 group address?
            if(group->addr.length == sizeof(Ipv6Addr))
            {
               //Matching multicast address?
               if(ipv6CompAddr(groupAddr, &IPV6_UNSPECIFIED_ADDR) ||
                  ipv6CompAddr(groupAddr, &group->addr.ipv6Addr))
               {
                  //Search the IPv6 multicast filter table for the specified
                  //address
                  entry = ipv6FindMulticastFilterEntry(interface,
                     &group->addr.ipv6Addr);

                  //No matching entry found?
                  if(entry == NULL)
                  {
                     //Create a new entry
                     entry = ipv6CreateMulticastFilterEntry(interface,
                        &group->addr.ipv6Addr);
                  }

                  //Entry successfully created?
                  if(entry != NULL)
                  {
                     //For each distinct (interface, multicast-address) pair
                     //that appears in any socket state, a per-interface record
                     //is created for that multicast address on that interface
                     ipv6DeriveInterfaceState(entry, group);
                  }
               }
            }
         }
      }
   }
#endif

   //Take the necessary actions when the per-interface state is changed
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->ipv6Context.multicastFilter[i];

      //Valid entry?
      if(!ipv6CompAddr(&entry->addr, &IPV6_UNSPECIFIED_ADDR))
      {
         //Check whether the interface has reception state for that group
         //address
         if(entry->srcFilterMode == IP_FILTER_MODE_EXCLUDE ||
            entry->srcFilter.numSources > 0)
         {
            //The MAC layer is reconfigured to accept the multicast traffic
            if(!entry->macFilterConfigured)
            {
               //Map the IPv6 multicast address to a MAC-layer address and add
               //the corresponding address to the MAC filter table
               error = ipv6AcceptMulticastAddr(interface, &entry->addr);

               //MAC filter table successfully updated?
               if(!error)
               {
                  entry->macFilterConfigured = TRUE;
               }
            }
         }
         else
         {
            //The MAC layer is reconfigured to drop the multicast traffic
            if(entry->macFilterConfigured)
            {
               ipv6DropMulticastAddr(interface, &entry->addr);
            }
         }

#if (MLD_NODE_SUPPORT == ENABLED)
         //Any change of interface state causes the system to immediately
         //transmit a State-Change Report from that interface
         mldNodeStateChangeEvent(&interface->mldNodeContext, &entry->addr,
            (IpFilterMode) entry->srcFilterMode, &entry->srcFilter);
#endif
         //If no state exists after the change, the "non-existent" state is
         //considered to have a filter mode of INCLUDE and an empty source list
         if(entry->srcFilterMode != IP_FILTER_MODE_EXCLUDE &&
            entry->srcFilter.numSources == 0)
         {
            //Remove the entry from the multicast filter
            ipv6DeleteMulticastFilterEntry(entry);
         }
      }
   }
}


/**
 * @brief Derive the per-interface state from the per-socket state
 * @param[in] entry Pointer to the per-interface state
 * @param[in] group Pointer to the per-socket state
 **/

void ipv6DeriveInterfaceState(Ipv6FilterEntry *entry,
   SocketMulticastGroup *group)
{
#if (IPV6_MAX_MULTICAST_SOURCES > 0 && SOCKET_MAX_MULTICAST_SOURCES > 0)
   uint_t i;

   //For each distinct (interface, multicast-address) pair that appears in any
   //socket state, a per-interface record is created for that multicast address
   //on that interface
   if(entry->srcFilterMode == IP_FILTER_MODE_INCLUDE &&
      group->filterMode == IP_FILTER_MODE_INCLUDE)
   {
      //If all records have a filter mode of INCLUDE, then the filter mode of
      //the interface record is INCLUDE, and the source list of the interface
      //record is the union of the source lists of all the socket records
      for(i = 0; i < group->numSources; i++)
      {
         ipv6AddSrcAddr(&entry->srcFilter, &group->sources[i].ipv6Addr);
      }
   }
   else if(entry->srcFilterMode == IP_FILTER_MODE_EXCLUDE &&
      group->filterMode == IP_FILTER_MODE_EXCLUDE)
   {
      //The source list of the interface record is the intersection of the
      //source lists of all socket records in EXCLUDE mode
      for(i = 0; i < entry->srcFilter.numSources; )
      {
         IpAddr srcAddr;

         //Get current source address
         srcAddr.length = sizeof(Ipv6Addr);
         srcAddr.ipv6Addr = entry->srcFilter.sources[i];

         //Calculate the intersection of the records
         if(socketFindMulticastSrcAddr(group, &srcAddr) >= 0)
         {
            i++;
         }
         else
         {
            ipv6RemoveSrcAddr(&entry->srcFilter, &srcAddr.ipv6Addr);
         }
      }
   }
   else if(entry->srcFilterMode == IP_FILTER_MODE_EXCLUDE &&
      group->filterMode == IP_FILTER_MODE_INCLUDE)
   {
      //Remove the source addresses that appear in any socket record in INCLUDE
      //mode
      for(i = 0; i < group->numSources; i++)
      {
         ipv6RemoveSrcAddr(&entry->srcFilter, &group->sources[i].ipv6Addr);
      }
   }
   else if(entry->srcFilterMode == IP_FILTER_MODE_INCLUDE &&
      group->filterMode == IP_FILTER_MODE_EXCLUDE)
   {
      Ipv6SrcAddrList srcFilter;

      //If any record has a filter mode of EXCLUDE, then the filter mode of the
      //interface record is EXCLUDE
      entry->srcFilterMode = IP_FILTER_MODE_EXCLUDE;

      //Initialize record
      srcFilter.numSources = 0;

      //Copy the source addresses that appear in the per-socket record
      for(i = 0; i < group->numSources; i++)
      {
         ipv6AddSrcAddr(&srcFilter, &group->sources[i].ipv6Addr);
      }

      //Remove the source addresses that appear in any socket record in INCLUDE
      //mode
      for(i = 0; i < entry->srcFilter.numSources; i++)
      {
         ipv6RemoveSrcAddr(&srcFilter, &entry->srcFilter.sources[i]);
      }

      //Save the resulting per-interface record
      entry->srcFilter = srcFilter;
   }
   else
   {
      //Just for sanity
   }
#else
   //All sources are accepted
   entry->srcFilterMode = IP_FILTER_MODE_EXCLUDE;
   entry->srcFilter.numSources = 0;
#endif
}


/**
 * @brief Reconfigure the MAC layer to accept multicast traffic
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv6 multicast address to accept
 * @return Error code
 **/

error_t ipv6AcceptMulticastAddr(NetInterface *interface,
   const Ipv6Addr *groupAddr)
{
   error_t error;
#if (ETH_SUPPORT == ENABLED)
   NetInterface *physicalInterface;
   MacAddr macAddr;
#endif

   //Initialize status code
   error = NO_ERROR;

#if (ETH_SUPPORT == ENABLED)
   //Point to the physical interface
   physicalInterface = nicGetPhysicalInterface(interface);

   //Map the IPv6 multicast address to a MAC-layer address
   ipv6MapMulticastAddrToMac(groupAddr, &macAddr);

   //Add the corresponding address to the MAC filter table
   error = ethAcceptMacAddr(interface, &macAddr);

   //Check status code
   if(!error)
   {
      //Virtual interface?
      if(interface != physicalInterface)
      {
         //Configure the physical interface to accept the MAC address
         error = ethAcceptMacAddr(physicalInterface, &macAddr);

         //Any error to report?
         if(error)
         {
            //Clean up side effects
            ethDropMacAddr(interface, &macAddr);
         }
      }
   }
#endif

   //Return status code
   return error;
}


/**
 * @brief Reconfigure the MAC layer to reject multicast traffic
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv6 multicast address to reject
 **/

void ipv6DropMulticastAddr(NetInterface *interface,
   const Ipv6Addr *groupAddr)
{
#if (ETH_SUPPORT == ENABLED)
   NetInterface *physicalInterface;
   MacAddr macAddr;

   //Point to the physical interface
   physicalInterface = nicGetPhysicalInterface(interface);

   //Map the IPv6 multicast address to a MAC-layer address
   ipv6MapMulticastAddrToMac(groupAddr, &macAddr);
   //Drop the corresponding address from the MAC filter table
   ethDropMacAddr(interface, &macAddr);

   //Virtual interface?
   if(interface != physicalInterface)
   {
      //Drop the corresponding address from the MAC filter table of the
      //physical interface
      ethDropMacAddr(physicalInterface, &macAddr);
   }
#endif
}


/**
 * @brief Map an IPv6 multicast address to a MAC-layer multicast address
 * @param[in] ipAddr IPv6 multicast address
 * @param[out] macAddr Corresponding MAC-layer multicast address
 * @return Error code
 **/

error_t ipv6MapMulticastAddrToMac(const Ipv6Addr *ipAddr, MacAddr *macAddr)
{
   error_t error;

   //Ensure the specified IPv6 address is a multicast address
   if(ipv6IsMulticastAddr(ipAddr))
   {
      //To support IPv6 multicasting, MAC address range of 33-33-00-00-00-00
      //to 33-33-FF-FF-FF-FF is reserved (refer to RFC 2464)
      macAddr->b[0] = 0x33;
      macAddr->b[1] = 0x33;

      //The low-order 32 bits of the IPv6 multicast address are mapped directly
      //to the low-order 32 bits in the MAC-layer multicast address
      macAddr->b[2] = ipAddr->b[12];
      macAddr->b[3] = ipAddr->b[13];
      macAddr->b[4] = ipAddr->b[14];
      macAddr->b[5] = ipAddr->b[15];

      //The specified IPv6 multicast address was successfully mapped to a
      //MAC-layer address
      error = NO_ERROR;
   }
   else
   {
      //Report an error
      error = ERROR_INVALID_ADDRESS;
   }

   //Return status code
   return error;
}


/**
 * @brief Create a new multicast filter entry
 * @param[in] interface Underlying network interface
 * @param[in] multicastAddr IPv6 multicast address
 * @return Pointer to the newly created multicast filter entry
 **/

Ipv6FilterEntry *ipv6CreateMulticastFilterEntry(NetInterface *interface,
   const Ipv6Addr *multicastAddr)
{
   uint_t i;
   Ipv6FilterEntry *entry;

   //Initialize pointer
   entry = NULL;

   //Go through the multicast filter table
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Check whether the current entry is available for use
      if(ipv6CompAddr(&interface->ipv6Context.multicastFilter[i].addr,
         &IPV6_UNSPECIFIED_ADDR))
      {
         //Point to the current entry
         entry = &interface->ipv6Context.multicastFilter[i];

         //Initialize entry
         entry->addr = *multicastAddr;
         entry->anySourceRefCount = 0;
         entry->macFilterConfigured = FALSE;
         entry->srcFilterMode = IP_FILTER_MODE_INCLUDE;
         entry->srcFilter.numSources = 0;

         //We are done
         break;
      }
   }

   //Return a pointer to the newly created multicast filter entry
   return entry;
}


/**
 * @brief Search the multicast filter for a given address
 * @param[in] interface Underlying network interface
 * @param[in] multicastAddr IPv6 multicast address
 * @return A pointer to the matching multicast filter entry is returned. NULL
 *   is returned if the specified group address cannot be found
 **/

Ipv6FilterEntry *ipv6FindMulticastFilterEntry(NetInterface *interface,
   const Ipv6Addr *multicastAddr)
{
   uint_t i;
   Ipv6FilterEntry *entry;

   //Initialize pointer
   entry = NULL;

   //Go through the multicast filter table
   for(i = 0; i < IPV6_MULTICAST_FILTER_SIZE; i++)
   {
      //Compare multicast addresses
      if(ipv6CompAddr(&interface->ipv6Context.multicastFilter[i].addr,
         multicastAddr))
      {
         //Point to the current entry
         entry = &interface->ipv6Context.multicastFilter[i];
         break;
      }
   }

   //Return a pointer to the matching multicast filter entry
   return entry;
}


/**
 * @brief Delete a multicast filter entry
 * @param[in] entry Pointer to the multicast filter entry
 **/

void ipv6DeleteMulticastFilterEntry(Ipv6FilterEntry *entry)
{
   //Delete the specified entry
   entry->addr = IPV6_UNSPECIFIED_ADDR;
}


/**
 * @brief Append a source address to a given list
 * @param[in] list Pointer to the list of source addresses
 * @param[in] srcAddr Source IP address to be added
 **/

error_t ipv6AddSrcAddr(Ipv6SrcAddrList *list, const Ipv6Addr *srcAddr)
{
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   error_t error;

   //Initialize status code
   error = NO_ERROR;

   //Make sure that the source address is not a duplicate
   if(ipv6FindSrcAddr(list, srcAddr) < 0)
   {
      //Check the length of the list
      if(list->numSources < IPV6_MAX_MULTICAST_SOURCES)
      {
         //Append the source address to the list
         list->sources[list->numSources] = *srcAddr;
         list->numSources++;
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
 * @brief Remove a source address from a given list
 * @param[in] list Pointer to the list of source addresses
 * @param[in] srcAddr Source IP address to be removed
 **/

void ipv6RemoveSrcAddr(Ipv6SrcAddrList *list, const Ipv6Addr *srcAddr)
{
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   uint_t i;
   uint_t j;

   //Loop through the list of source addresses
   for(i = 0; i < list->numSources; i++)
   {
      //Matching IP address?
      if(ipv6CompAddr(&list->sources[i], srcAddr))
      {
         //Remove the source address from the list
         for(j = i + 1; j < list->numSources; j++)
         {
            list->sources[j - 1] = list->sources[j];
         }

         //Update the length of the list
         list->numSources--;

         //We are done
         break;
      }
   }
#endif
}


/**
 * @brief Search the list of sources for a given IP address
 * @param[in] list Pointer to the list of source addresses
 * @param[in] srcAddr Source IP address
 * @return Index of the matching IP address is returned. -1 is
 *   returned if the specified IP address cannot be found
 **/

int_t ipv6FindSrcAddr(const Ipv6SrcAddrList *list, const Ipv6Addr *srcAddr)
{
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   int_t i;
   int_t index;

   //Initialize index
   index = -1;

   //Loop through the list of source addresses
   for(i = 0; i < list->numSources; i++)
   {
      //Matching IP address?
      if(ipv6CompAddr(&list->sources[i], srcAddr))
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


/**
 * @brief Compare lists of sources
 * @param[in] list1 Pointer to the first list of source addresses
 * @param[in] list2 Pointer to the second list of source addresses
 * @return TRUE if the lists contain the same set of addresses, else FALSE
 **/

bool_t ipv6CompareSrcAddrLists(const Ipv6SrcAddrList *list1,
   const Ipv6SrcAddrList *list2)
{
#if (IPV6_MAX_MULTICAST_SOURCES > 0)
   uint_t i;
   bool_t res;

   //Same number of elements?
   if(list1->numSources == list2->numSources)
   {
      //Initialize flag
      res = TRUE;

      //Loop through the first list of source addresses
      for(i = 0; i < list1->numSources && res; i++)
      {
         //Check whether the current address is present in the second list
         if(ipv6FindSrcAddr(list2, &list1->sources[i]) < 0)
         {
            res = FALSE;
         }
      }
   }
   else
   {
      //The lists do not have the same number of elements
      res = FALSE;
   }

   //Return TRUE if the lists contain the same set of addresses
   return res;
#else
   //Not implemented
   return TRUE;
#endif
}

#endif
