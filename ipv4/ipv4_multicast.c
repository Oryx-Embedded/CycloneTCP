/**
 * @file ipv4_multicast.c
 * @brief IPv4 multicast filtering
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
#define TRACE_LEVEL IPV4_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/socket_misc.h"
#include "ipv4/ipv4.h"
#include "ipv4/ipv4_multicast.h"
#include "igmp/igmp_host.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED)


/**
 * @brief Filter out incoming multicast traffic
 * @param[in] interface The interface on which the packet was received
 * @param[in] destAddr Destination IP address of the received packet
 * @param[in] srcAddr Source IP address of the received packet
 * @return Error code
 **/

error_t ipv4MulticastFilter(NetInterface *interface, Ipv4Addr destAddr,
   Ipv4Addr srcAddr)
{
   error_t error;
   uint_t i;
   Ipv4FilterEntry *entry;

   //Initialize status code
   error = ERROR_INVALID_ADDRESS;

   //Go through the multicast filter table
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE && error; i++)
   {
      //Point to the current entry
      entry = &interface->ipv4Context.multicastFilter[i];

      //Matching multicast address?
      if(entry->addr == destAddr)
      {
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
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
               if(entry->srcFilter.sources[j] == srcAddr)
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
               if(entry->srcFilter.sources[j] == srcAddr)
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
 * @brief Join the specified host group
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv4 address identifying the host group to join
 * @return Error code
 **/

error_t ipv4JoinMulticastGroup(NetInterface *interface, Ipv4Addr groupAddr)
{
   error_t error;
   Ipv4FilterEntry *entry;

   //Initialize status code
   error = NO_ERROR;

   //The IPv4 address must be a valid multicast address
   if(ipv4IsMulticastAddr(groupAddr))
   {
      //Search the IPv4 multicast filter table for the specified address
      entry = ipv4FindMulticastFilterEntry(interface, groupAddr);

      //No matching entry found?
      if(entry == NULL)
      {
         //Create a new entry
         entry = ipv4CreateMulticastFilterEntry(interface, groupAddr);
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
      //Update IPv4 multicast filter table
      ipv4UpdateMulticastFilter(interface, groupAddr);
   }

   //Return status code
   return error;
}


/**
 * @brief Leave the specified host group
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv4 address identifying the host group to leave
 * @return Error code
 **/

error_t ipv4LeaveMulticastGroup(NetInterface *interface, Ipv4Addr groupAddr)
{
   error_t error;
   Ipv4FilterEntry *entry;

   //Initialize status code
   error = NO_ERROR;

   //Search the IPv4 multicast filter table for the specified address
   entry = ipv4FindMulticastFilterEntry(interface, groupAddr);

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
      //The specified IPv4 address does not exist
      error = ERROR_ADDRESS_NOT_FOUND;
   }

   //Check status code
   if(!error)
   {
      //Update IPv4 multicast filter table
      ipv4UpdateMulticastFilter(interface, groupAddr);
   }

   //Return status code
   return error;
}


/**
 * @brief Update IPv4 multicast filter table
 * @param[in] interface Underlying network interface
 * @param[in] groupAddr IPv4 multicast address
 **/

void ipv4UpdateMulticastFilter(NetInterface *interface, Ipv4Addr groupAddr)
{
   error_t error;
   uint_t i;
   Ipv4FilterEntry *entry;

   //First, reset the per-interface state
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->ipv4Context.multicastFilter[i];

      //Matching multicast address?
      if(groupAddr == IPV4_UNSPECIFIED_ADDR || groupAddr == entry->addr)
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

            //IPv4 group address?
            if(group->addr.length == sizeof(Ipv4Addr))
            {
               //Matching multicast address?
               if(groupAddr == IPV4_UNSPECIFIED_ADDR ||
                  groupAddr == group->addr.ipv4Addr)
               {
                  //Search the IPv4 multicast filter table for the specified
                  //address
                  entry = ipv4FindMulticastFilterEntry(interface,
                     group->addr.ipv4Addr);

                  //No matching entry found?
                  if(entry == NULL)
                  {
                     //Create a new entry
                     entry = ipv4CreateMulticastFilterEntry(interface,
                        group->addr.ipv4Addr);
                  }

                  //Entry successfully created?
                  if(entry != NULL)
                  {
                     //For each distinct (interface, multicast-address) pair
                     //that appears in any socket state, a per-interface record
                     //is created for that multicast address on that interface
                     ipv4DeriveInterfaceState(entry, group);
                  }
               }
            }
         }
      }
   }
#endif

   //Take the necessary actions when the per-interface state is changed
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->ipv4Context.multicastFilter[i];

      //Valid entry?
      if(entry->addr != IPV4_UNSPECIFIED_ADDR)
      {
         //Check whether the interface has reception state for that group
         //address
         if(entry->srcFilterMode == IP_FILTER_MODE_EXCLUDE ||
            entry->srcFilter.numSources > 0)
         {
            //The MAC layer is reconfigured to accept the multicast traffic
            if(!entry->macFilterConfigured)
            {
               //Map the IPv4 multicast address to a MAC-layer address and add
               //the corresponding address to the MAC filter table
               error = ipv4AcceptMulticastAddr(interface, entry->addr);

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
               ipv4DropMulticastAddr(interface, entry->addr);
            }
         }

#if (IGMP_HOST_SUPPORT == ENABLED)
         //Any change of interface state causes the system to immediately
         //transmit a State-Change Report from that interface
         igmpHostStateChangeEvent(&interface->igmpHostContext, entry->addr,
            (IpFilterMode) entry->srcFilterMode, &entry->srcFilter);
#endif
         //If no state exists after the change, the "non-existent" state is
         //considered to have a filter mode of INCLUDE and an empty source list
         if(entry->srcFilterMode != IP_FILTER_MODE_EXCLUDE &&
            entry->srcFilter.numSources == 0)
         {
            //Remove the entry from the multicast filter
            ipv4DeleteMulticastFilterEntry(entry);
         }
      }
   }
}


/**
 * @brief Derive the per-interface state from the per-socket state
 * @param[in] entry Pointer to the per-interface state
 * @param[in] group Pointer to the per-socket state
 **/

void ipv4DeriveInterfaceState(Ipv4FilterEntry *entry,
   SocketMulticastGroup *group)
{
#if (IPV4_MAX_MULTICAST_SOURCES > 0 && SOCKET_MAX_MULTICAST_SOURCES > 0)
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
         ipv4AddSrcAddr(&entry->srcFilter, group->sources[i].ipv4Addr);
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
         srcAddr.length = sizeof(Ipv4Addr);
         srcAddr.ipv4Addr = entry->srcFilter.sources[i];

         //Calculate the intersection of the records
         if(socketFindMulticastSrcAddr(group, &srcAddr) >= 0)
         {
            i++;
         }
         else
         {
            ipv4RemoveSrcAddr(&entry->srcFilter, srcAddr.ipv4Addr);
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
         ipv4RemoveSrcAddr(&entry->srcFilter, group->sources[i].ipv4Addr);
      }
   }
   else if(entry->srcFilterMode == IP_FILTER_MODE_INCLUDE &&
      group->filterMode == IP_FILTER_MODE_EXCLUDE)
   {
      Ipv4SrcAddrList srcFilter;

      //If any record has a filter mode of EXCLUDE, then the filter mode of the
      //interface record is EXCLUDE
      entry->srcFilterMode = IP_FILTER_MODE_EXCLUDE;

      //Initialize record
      srcFilter.numSources = 0;

      //Copy the source addresses that appear in the per-socket record
      for(i = 0; i < group->numSources; i++)
      {
         ipv4AddSrcAddr(&srcFilter, group->sources[i].ipv4Addr);
      }

      //Remove the source addresses that appear in any socket record in INCLUDE
      //mode
      for(i = 0; i < entry->srcFilter.numSources; i++)
      {
         ipv4RemoveSrcAddr(&srcFilter, entry->srcFilter.sources[i]);
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
 * @param[in] groupAddr IPv4 multicast address to accept
 * @return Error code
 **/

error_t ipv4AcceptMulticastAddr(NetInterface *interface, Ipv4Addr groupAddr)
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

   //Map the IPv4 multicast address to a MAC-layer address
   ipv4MapMulticastAddrToMac(groupAddr, &macAddr);

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
 * @param[in] groupAddr IPv4 multicast address to reject
 **/

void ipv4DropMulticastAddr(NetInterface *interface, Ipv4Addr groupAddr)
{
#if (ETH_SUPPORT == ENABLED)
   NetInterface *physicalInterface;
   MacAddr macAddr;

   //Point to the physical interface
   physicalInterface = nicGetPhysicalInterface(interface);

   //Map the IPv4 multicast address to a MAC-layer address
   ipv4MapMulticastAddrToMac(groupAddr, &macAddr);
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
 * @brief Map an host group address to a MAC-layer multicast address
 * @param[in] ipAddr IPv4 host group address
 * @param[out] macAddr Corresponding MAC-layer multicast address
 * @return Error code
 **/

error_t ipv4MapMulticastAddrToMac(Ipv4Addr ipAddr, MacAddr *macAddr)
{
   error_t error;
   uint8_t *p;

   //Initialize status code
   error = NO_ERROR;

   //Check parameters
   if(macAddr != NULL)
   {
      //Ensure the specified IPv4 address is a valid host group address
      if(ipv4IsMulticastAddr(ipAddr))
      {
         //Cast the address to byte array
         p = (uint8_t *) &ipAddr;

         //An IP host group address is mapped to an Ethernet multicast address
         //by placing the low-order 23-bits of the IP address into the low-order
         //23 bits of the Ethernet multicast address 01-00-5E-00-00-00
         macAddr->b[0] = 0x01;
         macAddr->b[1] = 0x00;
         macAddr->b[2] = 0x5E;
         macAddr->b[3] = p[1] & 0x7F;
         macAddr->b[4] = p[2];
         macAddr->b[5] = p[3];
      }
      else
      {
         //The specified IPv4 address is not a valid host group address
         error = ERROR_INVALID_ADDRESS;
      }
   }
   else
   {
      //Report an error
      error = ERROR_INVALID_PARAMETER;
   }

   //Return status code
   return error;
}


/**
 * @brief Create a new multicast filter entry
 * @param[in] interface Underlying network interface
 * @param[in] multicastAddr IPv4 multicast address
 * @return Pointer to the newly created multicast filter entry
 **/

Ipv4FilterEntry *ipv4CreateMulticastFilterEntry(NetInterface *interface,
   Ipv4Addr multicastAddr)
{
   uint_t i;
   Ipv4FilterEntry *entry;

   //Initialize pointer
   entry = NULL;

   //Go through the multicast filter table
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Check whether the current entry is available for use
      if(interface->ipv4Context.multicastFilter[i].addr == IPV4_UNSPECIFIED_ADDR)
      {
         //Point to the current entry
         entry = &interface->ipv4Context.multicastFilter[i];

         //Initialize entry
         entry->addr = multicastAddr;
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
 * @param[in] multicastAddr IPv4 multicast address
 * @return A pointer to the matching multicast filter entry is returned. NULL
 *   is returned if the specified group address cannot be found
 **/

Ipv4FilterEntry *ipv4FindMulticastFilterEntry(NetInterface *interface,
   Ipv4Addr multicastAddr)
{
   uint_t i;
   Ipv4FilterEntry *entry;

   //Initialize pointer
   entry = NULL;

   //Go through the multicast filter table
   for(i = 0; i < IPV4_MULTICAST_FILTER_SIZE; i++)
   {
      //Compare multicast addresses
      if(interface->ipv4Context.multicastFilter[i].addr == multicastAddr)
      {
         //Point to the current entry
         entry = &interface->ipv4Context.multicastFilter[i];
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

void ipv4DeleteMulticastFilterEntry(Ipv4FilterEntry *entry)
{
   //Delete the specified entry
   entry->addr = IPV4_UNSPECIFIED_ADDR;
}


/**
 * @brief Append a source address to a given list
 * @param[in] list Pointer to the list of source addresses
 * @param[in] srcAddr Source IP address to be added
 **/

error_t ipv4AddSrcAddr(Ipv4SrcAddrList *list, Ipv4Addr srcAddr)
{
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   error_t error;

   //Initialize status code
   error = NO_ERROR;

   //Make sure that the source address is not a duplicate
   if(ipv4FindSrcAddr(list, srcAddr) < 0)
   {
      //Check the length of the list
      if(list->numSources < IPV4_MAX_MULTICAST_SOURCES)
      {
         //Append the source address to the list
         list->sources[list->numSources] = srcAddr;
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

void ipv4RemoveSrcAddr(Ipv4SrcAddrList *list, Ipv4Addr srcAddr)
{
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   uint_t i;
   uint_t j;

   //Loop through the list of source addresses
   for(i = 0; i < list->numSources; i++)
   {
      //Matching IP address?
      if(list->sources[i] == srcAddr)
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

int_t ipv4FindSrcAddr(const Ipv4SrcAddrList *list, Ipv4Addr srcAddr)
{
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
   int_t i;
   int_t index;

   //Initialize index
   index = -1;

   //Loop through the list of source addresses
   for(i = 0; i < list->numSources; i++)
   {
      //Matching IP address?
      if(list->sources[i] == srcAddr)
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

bool_t ipv4CompareSrcAddrLists(const Ipv4SrcAddrList *list1,
   const Ipv4SrcAddrList *list2)
{
#if (IPV4_MAX_MULTICAST_SOURCES > 0)
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
         if(ipv4FindSrcAddr(list2, list1->sources[i]) < 0)
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
