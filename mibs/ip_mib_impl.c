/**
 * @file ip_mib_impl.c
 * @brief IP MIB module implementation
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

//Switch to the appropriate trace level
#define TRACE_LEVEL SNMP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "ipv4/ipv4.h"
#include "ipv4/ipv4_misc.h"
#include "ipv4/arp_cache.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_misc.h"
#include "ipv6/ndp_cache.h"
#include "ipv6/ndp_router_adv.h"
#include "mibs/mib_common.h"
#include "mibs/ip_mib_module.h"
#include "mibs/ip_mib_impl.h"
#include "core/crypto.h"
#include "encoding/asn1.h"
#include "encoding/oid.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IP_MIB_SUPPORT == ENABLED)


/**
 * @brief IP MIB module initialization
 * @return Error code
 **/

error_t ipMibInit(void)
{
   NetContext *context;

   //Debug message
   TRACE_INFO("Initializing IP-MIB base...\r\n");

   //Clear IP MIB base
   osMemset(&ipMibBase, 0, sizeof(ipMibBase));

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //ipAddressSpinLock object
   if(context != NULL)
   {
      ipMibBase.ipAddressSpinLock = netGetRandRange(context, 1, INT32_MAX);
   }

#if (IPV4_SUPPORT == ENABLED)
   //ipForwarding object
   ipMibBase.ipForwarding = IP_MIB_IP_FORWARDING_DISABLED;
   //ipDefaultTTL object
   ipMibBase.ipDefaultTTL = IPV4_DEFAULT_TTL;
   //ipReasmTimeout object
   ipMibBase.ipReasmTimeout = IPV4_FRAG_TIME_TO_LIVE / 1000;
#endif

#if (IPV6_SUPPORT == ENABLED)
   //ipv6IpForwarding object
   ipMibBase.ipv6IpForwarding = IP_MIB_IP_FORWARDING_DISABLED;
   //ipv6IpDefaultHopLimit object
   ipMibBase.ipv6IpDefaultHopLimit = IPV6_DEFAULT_HOP_LIMIT;

   //ipv6RouterAdvertSpinLock object
   if(context != NULL)
   {
      ipMibBase.ipv6RouterAdvertSpinLock = netGetRandRange(context, 1,
         INT32_MAX);
   }
#endif

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set ipv4InterfaceEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in] value Object value
 * @param[in] valueLen Length of the object value, in bytes
 * @param[in] commit This flag tells whether the changes shall be committed
 *   to the MIB base
 * @return Error code
 **/

error_t ipMibSetIpv4InterfaceEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen, bool_t commit)
{
   //Not implemented
   return ERROR_WRITE_FAILED;
}


/**
 * @brief Get ipv4InterfaceEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpv4InterfaceEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   size_t n;
   uint_t index;
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipv4InterfaceIfIndex is used as instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &index);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check index range
   if(index < 1 || index > context->numInterfaces)
      return ERROR_INSTANCE_NOT_FOUND;

#if (IPV4_SUPPORT == ENABLED)
   //ipv4InterfaceReasmMaxSize object?
   if(osStrcmp(object->name, "ipv4InterfaceReasmMaxSize") == 0)
   {
      //Get object value
      value->integer = IPV4_MAX_FRAG_DATAGRAM_SIZE;
   }
   //ipv4InterfaceEnableStatus object?
   else if(osStrcmp(object->name, "ipv4InterfaceEnableStatus") == 0)
   {
      //Get object value
      value->integer = IP_MIB_IP_STATUS_UP;
   }
   //ipv4InterfaceRetransmitTime object?
   else if(osStrcmp(object->name, "ipv4InterfaceRetransmitTime") == 0)
   {
      //Get object value
      value->unsigned32 = ARP_REQUEST_TIMEOUT;
   }
   else
#endif
   //Unknown object?
   {
      //The specified object does not exist
      error = ERROR_OBJECT_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next ipv4InterfaceEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpv4InterfaceEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   size_t n;
   uint_t index;
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

   //Loop through network interfaces
   for(index = 1; index <= context->numInterfaces; index++)
   {
      //Append the instance identifier to the OID prefix
      n = object->oidLen;

      //ifIndex is used as instance identifier
      error = mibEncodeIndex(nextOid, *nextOidLen, &n, index);
      //Any error to report?
      if(error)
         return error;

      //Check whether the resulting object identifier lexicographically
      //follows the specified OID
      if(oidComp(nextOid, n, oid, oidLen) > 0)
      {
         //Save the length of the resulting object identifier
         *nextOidLen = n;
         //Next object found
         return NO_ERROR;
      }
   }

   //The specified OID does not lexicographically precede the name
   //of some object
   return ERROR_OBJECT_NOT_FOUND;
}


/**
 * @brief Set ipv6InterfaceEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in] value Object value
 * @param[in] valueLen Length of the object value, in bytes
 * @param[in] commit This flag tells whether the changes shall be committed
 *   to the MIB base
 * @return Error code
 **/

error_t ipMibSetIpv6InterfaceEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen, bool_t commit)
{
   //Not implemented
   return ERROR_WRITE_FAILED;
}


/**
 * @brief Get ipv6InterfaceEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpv6InterfaceEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   size_t n;
   uint_t index;
   NetContext *context;
   NetInterface *interface;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipv6InterfaceIfIndex is used as instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &index);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check index range
   if(index < 1 || index > context->numInterfaces)
      return ERROR_INSTANCE_NOT_FOUND;

   //Point to the underlying interface
   interface = &context->interfaces[index - 1];
   //Avoid warnings from the compiler
   (void) interface;

#if (IPV6_SUPPORT == ENABLED)
   //ipv6InterfaceReasmMaxSize object?
   if(osStrcmp(object->name, "ipv6InterfaceReasmMaxSize") == 0)
   {
      //Get object value
      value->unsigned32 = IPV6_MAX_FRAG_DATAGRAM_SIZE;
   }
   //ipv6InterfaceIdentifier object?
   else if(osStrcmp(object->name, "ipv6InterfaceIdentifier") == 0)
   {
      //Make sure the buffer is large enough to hold the entire object
      if(*valueLen >= sizeof(Eui64))
      {
         NetInterface *logicalInterface;

         //Point to the logical interface
         logicalInterface = nicGetLogicalInterface(interface);

         //Copy object value
         eui64CopyAddr(value->octetString, &logicalInterface->eui64);
         //Return object length
         *valueLen = sizeof(Eui64);
      }
      else
      {
         //Report an error
         error = ERROR_BUFFER_OVERFLOW;
      }
   }
   //ipv6InterfaceEnableStatus object?
   else if(osStrcmp(object->name, "ipv6InterfaceEnableStatus") == 0)
   {
      //Get object value
      value->integer = IP_MIB_IP_STATUS_UP;
   }
   //ipv6InterfaceReachableTime object?
   else if(osStrcmp(object->name, "ipv6InterfaceReachableTime") == 0)
   {
      //Get object value
      value->unsigned32 = interface->ndpContext.reachableTime;
   }
   //ipv6InterfaceRetransmitTime object?
   else if(osStrcmp(object->name, "ipv6InterfaceRetransmitTime") == 0)
   {
      //Get object value
      value->unsigned32 = interface->ndpContext.retransTimer;
   }
   //ipv6InterfaceForwarding object?
   else if(osStrcmp(object->name, "ipv6InterfaceForwarding") == 0)
   {
      //Get object value
      if(interface->ipv6Context.isRouter)
      {
         value->integer = IP_MIB_IP_FORWARDING_ENABLED;
      }
      else
      {
         value->integer = IP_MIB_IP_FORWARDING_DISABLED;
      }
   }
   else
#endif
   //Unknown object?
   {
      //The specified object does not exist
      error = ERROR_OBJECT_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next ipv6InterfaceEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpv6InterfaceEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   size_t n;
   uint_t index;
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

   //Loop through network interfaces
   for(index = 1; index <= context->numInterfaces; index++)
   {
      //Append the instance identifier to the OID prefix
      n = object->oidLen;

      //ifIndex is used as instance identifier
      error = mibEncodeIndex(nextOid, *nextOidLen, &n, index);
      //Any error to report?
      if(error)
         return error;

      //Check whether the resulting object identifier lexicographically
      //follows the specified OID
      if(oidComp(nextOid, n, oid, oidLen) > 0)
      {
         //Save the length of the resulting object identifier
         *nextOidLen = n;
         //Next object found
         return NO_ERROR;
      }
   }

   //The specified OID does not lexicographically precede the name
   //of some object
   return ERROR_OBJECT_NOT_FOUND;
}


/**
 * @brief Get ipSystemStatsEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpSystemStatsEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   size_t n;
   uint_t version;
   NetContext *context;
   IpSystemStats *ipSystemStats;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipSystemStatsIPVersion is used as instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &version);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

#if (IPV4_SUPPORT == ENABLED && IPV4_STATS_SUPPORT == ENABLED)
   //IPv4 version?
   if(version == INET_VERSION_IPV4)
   {
      //Point to the system-wide IPv4 statistics
      ipSystemStats = &context->ipv4SystemStats;
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED && IPV6_STATS_SUPPORT == ENABLED)
   //IPv6 version?
   if(version == INET_VERSION_IPV6)
   {
      //Point to the system-wide IPv6 statistics
      ipSystemStats = &context->ipv6SystemStats;
   }
   else
#endif
   //Invalid IP version?
   {
      //No statistics available
      ipSystemStats = NULL;
   }

   //Sanity check
   if(ipSystemStats != NULL)
   {
      //ipSystemStatsInReceives object?
      if(osStrcmp(object->name, "ipSystemStatsInReceives") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->inReceives;
      }
      //ipSystemStatsHCInReceives object?
      else if(osStrcmp(object->name, "ipSystemStatsHCInReceives") == 0)
      {
         value->counter64 = ipSystemStats->inReceives;
      }
      //ipSystemStatsInOctets object?
      else if(osStrcmp(object->name, "ipSystemStatsInOctets") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->inOctets;
      }
      //ipSystemStatsHCInOctets object?
      else if(osStrcmp(object->name, "ipSystemStatsHCInOctets") == 0)
      {
         value->counter64 = ipSystemStats->inOctets;
      }
      //ipSystemStatsInHdrErrors object?
      else if(osStrcmp(object->name, "ipSystemStatsInHdrErrors") == 0)
      {
         value->counter32 = ipSystemStats->inHdrErrors;
      }
      //ipSystemStatsInNoRoutes object?
      else if(osStrcmp(object->name, "ipSystemStatsInNoRoutes") == 0)
      {
         value->counter32 = ipSystemStats->inNoRoutes;
      }
      //ipSystemStatsInAddrErrors object?
      else if(osStrcmp(object->name, "ipSystemStatsInAddrErrors") == 0)
      {
         value->counter32 = ipSystemStats->inAddrErrors;
      }
      //ipSystemStatsInUnknownProtos object?
      else if(osStrcmp(object->name, "ipSystemStatsInUnknownProtos") == 0)
      {
         value->counter32 = ipSystemStats->inUnknownProtos;
      }
      //ipSystemStatsInTruncatedPkts object?
      else if(osStrcmp(object->name, "ipSystemStatsInTruncatedPkts") == 0)
      {
         value->counter32 = ipSystemStats->inTruncatedPkts;
      }
      //ipSystemStatsInForwDatagrams object?
      else if(osStrcmp(object->name, "ipSystemStatsInForwDatagrams") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->inForwDatagrams;
      }
      //ipSystemStatsHCInForwDatagrams object?
      else if(osStrcmp(object->name, "ipSystemStatsHCInForwDatagrams") == 0)
      {
         value->counter64 = ipSystemStats->inForwDatagrams;
      }
      //ipSystemStatsReasmReqds object?
      else if(osStrcmp(object->name, "ipSystemStatsReasmReqds") == 0)
      {
         value->counter32 = ipSystemStats->reasmReqds;
      }
      //ipSystemStatsReasmOKs object?
      else if(osStrcmp(object->name, "ipSystemStatsReasmOKs") == 0)
      {
         value->counter32 = ipSystemStats->reasmOKs;
      }
      //ipSystemStatsReasmFails object?
      else if(osStrcmp(object->name, "ipSystemStatsReasmFails") == 0)
      {
         value->counter32 = ipSystemStats->reasmFails;
      }
      //ipSystemStatsInDiscards object?
      else if(osStrcmp(object->name, "ipSystemStatsInDiscards") == 0)
      {
         value->counter32 = ipSystemStats->inDiscards;
      }
      //ipSystemStatsInDelivers object?
      else if(osStrcmp(object->name, "ipSystemStatsInDelivers") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->inDelivers;
      }
      //ipSystemStatsHCInDelivers object?
      else if(osStrcmp(object->name, "ipSystemStatsHCInDelivers") == 0)
      {
         value->counter64 = ipSystemStats->inDelivers;
      }
      //ipSystemStatsOutRequests object?
      else if(osStrcmp(object->name, "ipSystemStatsOutRequests") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->outRequests;
      }
      //ipSystemStatsHCOutRequests object?
      else if(osStrcmp(object->name, "ipSystemStatsHCOutRequests") == 0)
      {
         value->counter64 = ipSystemStats->outRequests;
      }
      //ipSystemStatsOutNoRoutes object?
      else if(osStrcmp(object->name, "ipSystemStatsOutNoRoutes") == 0)
      {
         value->counter32 = ipSystemStats->outNoRoutes;
      }
      //ipSystemStatsOutForwDatagrams object?
      else if(osStrcmp(object->name, "ipSystemStatsOutForwDatagrams") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->outForwDatagrams;
      }
      //ipSystemStatsHCOutForwDatagrams object?
      else if(osStrcmp(object->name, "ipSystemStatsHCOutForwDatagrams") == 0)
      {
         value->counter64 = ipSystemStats->outForwDatagrams;
      }
      //ipSystemStatsOutDiscards object?
      else if(osStrcmp(object->name, "ipSystemStatsOutDiscards") == 0)
      {
         value->counter32 = ipSystemStats->outDiscards;
      }
      //ipSystemStatsOutFragReqds object?
      else if(osStrcmp(object->name, "ipSystemStatsOutFragReqds") == 0)
      {
         value->counter32 = ipSystemStats->outFragReqds;
      }
      //ipSystemStatsOutFragOKs object?
      else if(osStrcmp(object->name, "ipSystemStatsOutFragOKs") == 0)
      {
         value->counter32 = ipSystemStats->outFragOKs;
      }
      //ipSystemStatsOutFragFails object?
      else if(osStrcmp(object->name, "ipSystemStatsOutFragFails") == 0)
      {
         value->counter32 = ipSystemStats->outFragFails;
      }
      //ipSystemStatsOutFragCreates object?
      else if(osStrcmp(object->name, "ipSystemStatsOutFragCreates") == 0)
      {
         value->counter32 = ipSystemStats->outFragCreates;
      }
      //ipSystemStatsOutTransmits object?
      else if(osStrcmp(object->name, "ipSystemStatsOutTransmits") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->outTransmits;
      }
      //ipSystemStatsHCOutTransmits object?
      else if(osStrcmp(object->name, "ipSystemStatsHCOutTransmits") == 0)
      {
         value->counter64 = ipSystemStats->outTransmits;
      }
      //ipSystemStatsOutOctets object?
      else if(osStrcmp(object->name, "ipSystemStatsOutOctets") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->outOctets;
      }
      //ipSystemStatsHCOutOctets object?
      else if(osStrcmp(object->name, "ipSystemStatsHCOutOctets") == 0)
      {
         value->counter64 = ipSystemStats->outOctets;
      }
      //ipSystemStatsInMcastPkts object?
      else if(osStrcmp(object->name, "ipSystemStatsInMcastPkts") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->inMcastPkts;
      }
      //ipSystemStatsHCInMcastPkts object?
      else if(osStrcmp(object->name, "ipSystemStatsHCInMcastPkts") == 0)
      {
         value->counter64 = ipSystemStats->inMcastPkts;
      }
      //ipSystemStatsInMcastOctets object?
      else if(osStrcmp(object->name, "ipSystemStatsInMcastOctets") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->inMcastOctets;
      }
      //ipSystemStatsHCInMcastOctets object?
      else if(osStrcmp(object->name, "ipSystemStatsHCInMcastOctets") == 0)
      {
         value->counter64 = ipSystemStats->inMcastOctets;
      }
      //ipSystemStatsOutMcastPkts object?
      else if(osStrcmp(object->name, "ipSystemStatsOutMcastPkts") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->outMcastPkts;
      }
      //ipSystemStatsHCOutMcastPkts object?
      else if(osStrcmp(object->name, "ipSystemStatsHCOutMcastPkts") == 0)
      {
         value->counter64 = ipSystemStats->outMcastPkts;
      }
      //ipSystemStatsOutMcastOctets object?
      else if(osStrcmp(object->name, "ipSystemStatsOutMcastOctets") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->outMcastOctets;
      }
      //ipSystemStatsHCOutMcastOctets object?
      else if(osStrcmp(object->name, "ipSystemStatsHCOutMcastOctets") == 0)
      {
         value->counter64 = ipSystemStats->outMcastOctets;
      }
      //ipSystemStatsInBcastPkts object?
      else if(osStrcmp(object->name, "ipSystemStatsInBcastPkts") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->inBcastPkts;
      }
      //ipSystemStatsHCInBcastPkts object?
      else if(osStrcmp(object->name, "ipSystemStatsHCInBcastPkts") == 0)
      {
         value->counter64 = ipSystemStats->inBcastPkts;
      }
      //ipSystemStatsOutBcastPkts object?
      else if(osStrcmp(object->name, "ipSystemStatsOutBcastPkts") == 0)
      {
         value->counter32 = (uint32_t) ipSystemStats->outBcastPkts;
      }
      //ipSystemStatsHCOutBcastPkts object?
      else if(osStrcmp(object->name, "ipSystemStatsHCOutBcastPkts") == 0)
      {
         value->counter64 = ipSystemStats->outBcastPkts;
      }
      //ipSystemStatsDiscontinuityTime object?
      else if(osStrcmp(object->name, "ipSystemStatsDiscontinuityTime") == 0)
      {
         value->timeTicks = ipSystemStats->discontinuityTime;
      }
      //ipSystemStatsRefreshRate object?
      else if(osStrcmp(object->name, "ipSystemStatsRefreshRate") == 0)
      {
         value->unsigned32 = ipSystemStats->refreshRate;
      }
      //Unknown object?
      else
      {
         error = ERROR_OBJECT_NOT_FOUND;
      }
   }
   else
   {
      //Report an error
      error = ERROR_INSTANCE_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next ipSystemStatsEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpSystemStatsEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   size_t n;
   uint_t version;

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

   //IP version-neutral table
   for(version = INET_VERSION_IPV4; version <= INET_VERSION_IPV6; version++)
   {
#if (IPV4_SUPPORT == DISABLED || IPV4_STATS_SUPPORT == DISABLED)
      //IPv4 version?
      if(version == INET_VERSION_IPV4)
      {
         //IPv4 is not implemented
         continue;
      }
#endif
#if (IPV6_SUPPORT == DISABLED || IPV6_STATS_SUPPORT == DISABLED)
      //IPv6 version?
      if(version == INET_VERSION_IPV6)
      {
         //IPv6 is not implemented
         continue;
      }
#endif

      //Append the instance identifier to the OID prefix
      n = object->oidLen;

      //ipSystemStatsIPVersion is used as instance identifier
      error = mibEncodeIndex(nextOid, *nextOidLen, &n, version);
      //Any error to report?
      if(error)
         return error;

      //Check whether the resulting object identifier lexicographically
      //follows the specified OID
      if(oidComp(nextOid, n, oid, oidLen) > 0)
      {
         //Save the length of the resulting object identifier
         *nextOidLen = n;
         //Next object found
         return NO_ERROR;
      }
   }

   //The specified OID does not lexicographically precede the name
   //of some object
   return ERROR_OBJECT_NOT_FOUND;
}


/**
 * @brief Get ipIfStatsEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpIfStatsEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   size_t n;
   uint_t version;
   uint_t index;
   NetContext *context;
   IpIfStats *ipIfStats;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipIfStatsIPVersion is used as 1st instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &version);
   //Invalid instance identifier?
   if(error)
      return error;

   //ipIfStatsIfIndex is used as 2nd instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &index);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check index range
   if(index < 1 || index > context->numInterfaces)
      return ERROR_INSTANCE_NOT_FOUND;

#if (IPV4_SUPPORT == ENABLED && IPV4_STATS_SUPPORT == ENABLED)
   //IPv4 version?
   if(version == INET_VERSION_IPV4)
   {
      //Point to the per-interface IPv4 statistics
      ipIfStats = &context->interfaces[index - 1].ipv4IfStats;
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED && IPV6_STATS_SUPPORT == ENABLED)
   //IPv6 version?
   if(version == INET_VERSION_IPV6)
   {
      //Point to the IPv6 statistics table entry
      ipIfStats = &context->interfaces[index - 1].ipv6IfStats;
   }
   else
#endif
   //Invalid IP version?
   {
      //No statistics available
      ipIfStats = NULL;
   }

   //Sanity check
   if(ipIfStats != NULL)
   {
      //ipIfStatsInReceives object?
      if(osStrcmp(object->name, "ipIfStatsInReceives") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->inReceives;
      }
      //ipIfStatsHCInReceives object?
      else if(osStrcmp(object->name, "ipIfStatsHCInReceives") == 0)
      {
         value->counter64 = ipIfStats->inReceives;
      }
      //ipIfStatsInOctets object?
      else if(osStrcmp(object->name, "ipIfStatsInOctets") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->inOctets;
      }
      //ipIfStatsHCInOctets object?
      else if(osStrcmp(object->name, "ipIfStatsHCInOctets") == 0)
      {
         value->counter64 = ipIfStats->inOctets;
      }
      //ipIfStatsInHdrErrors object?
      else if(osStrcmp(object->name, "ipIfStatsInHdrErrors") == 0)
      {
         value->counter32 = ipIfStats->inHdrErrors;
      }
      //ipIfStatsInNoRoutes object?
      else if(osStrcmp(object->name, "ipIfStatsInNoRoutes") == 0)
      {
         value->counter32 = ipIfStats->inNoRoutes;
      }
      //ipIfStatsInAddrErrors object?
      else if(osStrcmp(object->name, "ipIfStatsInAddrErrors") == 0)
      {
         value->counter32 = ipIfStats->inAddrErrors;
      }
      //ipIfStatsInUnknownProtos object?
      else if(osStrcmp(object->name, "ipIfStatsInUnknownProtos") == 0)
      {
         value->counter32 = ipIfStats->inUnknownProtos;
      }
      //ipIfStatsInTruncatedPkts object?
      else if(osStrcmp(object->name, "ipIfStatsInTruncatedPkts") == 0)
      {
         value->counter32 = ipIfStats->inTruncatedPkts;
      }
      //ipIfStatsInForwDatagrams object?
      else if(osStrcmp(object->name, "ipIfStatsInForwDatagrams") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->inForwDatagrams;
      }
      //ipIfStatsHCInForwDatagrams object?
      else if(osStrcmp(object->name, "ipIfStatsHCInForwDatagrams") == 0)
      {
         value->counter64 = ipIfStats->inForwDatagrams;
      }
      //ipIfStatsReasmReqds object?
      else if(osStrcmp(object->name, "ipIfStatsReasmReqds") == 0)
      {
         value->counter32 = ipIfStats->reasmReqds;
      }
      //ipIfStatsReasmOKs object?
      else if(osStrcmp(object->name, "ipIfStatsReasmOKs") == 0)
      {
         value->counter32 = ipIfStats->reasmOKs;
      }
      //ipIfStatsReasmFails object?
      else if(osStrcmp(object->name, "ipIfStatsReasmFails") == 0)
      {
         value->counter32 = ipIfStats->reasmFails;
      }
      //ipIfStatsInDiscards object?
      else if(osStrcmp(object->name, "ipIfStatsInDiscards") == 0)
      {
         value->counter32 = ipIfStats->inDiscards;
      }
      //ipIfStatsInDelivers object?
      else if(osStrcmp(object->name, "ipIfStatsInDelivers") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->inDelivers;
      }
      //ipIfStatsHCInDelivers object?
      else if(osStrcmp(object->name, "ipIfStatsHCInDelivers") == 0)
      {
         value->counter64 = ipIfStats->inDelivers;
      }
      //ipIfStatsOutRequests object?
      else if(osStrcmp(object->name, "ipIfStatsOutRequests") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->outRequests;
      }
      //ipIfStatsHCOutRequests object?
      else if(osStrcmp(object->name, "ipIfStatsHCOutRequests") == 0)
      {
         value->counter64 = ipIfStats->outRequests;
      }
      //ipIfStatsOutForwDatagrams object?
      else if(osStrcmp(object->name, "ipIfStatsOutForwDatagrams") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->outForwDatagrams;
      }
      //ipIfStatsHCOutForwDatagrams object?
      else if(osStrcmp(object->name, "ipIfStatsHCOutForwDatagrams") == 0)
      {
         value->counter64 = ipIfStats->outForwDatagrams;
      }
      //ipIfStatsOutDiscards object?
      else if(osStrcmp(object->name, "ipIfStatsOutDiscards") == 0)
      {
         value->counter32 = ipIfStats->outDiscards;
      }
      //ipIfStatsOutFragReqds object?
      else if(osStrcmp(object->name, "ipIfStatsOutFragReqds") == 0)
      {
         value->counter32 = ipIfStats->outFragReqds;
      }
      //ipIfStatsOutFragOKs object?
      else if(osStrcmp(object->name, "ipIfStatsOutFragOKs") == 0)
      {
         value->counter32 = ipIfStats->outFragOKs;
      }
      //ipIfStatsOutFragFails object?
      else if(osStrcmp(object->name, "ipIfStatsOutFragFails") == 0)
      {
         value->counter32 = ipIfStats->outFragFails;
      }
      //ipIfStatsOutFragCreates object?
      else if(osStrcmp(object->name, "ipIfStatsOutFragCreates") == 0)
      {
         value->counter32 = ipIfStats->outFragCreates;
      }
      //ipIfStatsOutTransmits object?
      else if(osStrcmp(object->name, "ipIfStatsOutTransmits") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->outTransmits;
      }
      //ipIfStatsHCOutTransmits object?
      else if(osStrcmp(object->name, "ipIfStatsHCOutTransmits") == 0)
      {
         value->counter64 = ipIfStats->outTransmits;
      }
      //ipIfStatsOutOctets object?
      else if(osStrcmp(object->name, "ipIfStatsOutOctets") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->outOctets;
      }
      //ipIfStatsHCOutOctets object?
      else if(osStrcmp(object->name, "ipIfStatsHCOutOctets") == 0)
      {
         value->counter64 = ipIfStats->outOctets;
      }
      //ipIfStatsInMcastPkts object?
      else if(osStrcmp(object->name, "ipIfStatsInMcastPkts") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->inMcastPkts;
      }
      //ipIfStatsHCInMcastPkts object?
      else if(osStrcmp(object->name, "ipIfStatsHCInMcastPkts") == 0)
      {
         value->counter64 = ipIfStats->inMcastPkts;
      }
      //ipIfStatsInMcastOctets object?
      else if(osStrcmp(object->name, "ipIfStatsInMcastOctets") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->inMcastOctets;
      }
      //ipIfStatsHCInMcastOctets object?
      else if(osStrcmp(object->name, "ipIfStatsHCInMcastOctets") == 0)
      {
         value->counter64 = ipIfStats->inMcastOctets;
      }
      //ipIfStatsOutMcastPkts object?
      else if(osStrcmp(object->name, "ipIfStatsOutMcastPkts") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->outMcastPkts;
      }
      //ipIfStatsHCOutMcastPkts object?
      else if(osStrcmp(object->name, "ipIfStatsHCOutMcastPkts") == 0)
      {
         value->counter64 = ipIfStats->outMcastPkts;
      }
      //ipIfStatsOutMcastOctets object?
      else if(osStrcmp(object->name, "ipIfStatsOutMcastOctets") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->outMcastOctets;
      }
      //ipIfStatsHCOutMcastOctets object?
      else if(osStrcmp(object->name, "ipIfStatsHCOutMcastOctets") == 0)
      {
         value->counter64 = ipIfStats->outMcastOctets;
      }
      //ipIfStatsInBcastPkts object?
      else if(osStrcmp(object->name, "ipIfStatsInBcastPkts") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->inBcastPkts;
      }
      //ipIfStatsHCInBcastPkts object?
      else if(osStrcmp(object->name, "ipIfStatsHCInBcastPkts") == 0)
      {
         value->counter64 = ipIfStats->inBcastPkts;
      }
      //ipIfStatsOutBcastPkts object?
      else if(osStrcmp(object->name, "ipIfStatsOutBcastPkts") == 0)
      {
         value->counter32 = (uint32_t) ipIfStats->outBcastPkts;
      }
      //ipIfStatsHCOutBcastPkts object?
      else if(osStrcmp(object->name, "ipIfStatsHCOutBcastPkts") == 0)
      {
         value->counter64 = ipIfStats->outBcastPkts;
      }
      //ipIfStatsDiscontinuityTime object?
      else if(osStrcmp(object->name, "ipIfStatsDiscontinuityTime") == 0)
      {
         value->timeTicks = ipIfStats->discontinuityTime;
      }
      //ipIfStatsRefreshRate object?
      else if(osStrcmp(object->name, "ipIfStatsRefreshRate") == 0)
      {
         value->unsigned32 = ipIfStats->refreshRate;
      }
      //Unknown object?
      else
      {
         error = ERROR_OBJECT_NOT_FOUND;
      }
   }
   else
   {
      //Report an error
      error = ERROR_INSTANCE_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next ipIfStatsEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpIfStatsEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   size_t n;
   uint_t version;
   uint_t index;
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

   //IP version-neutral table
   for(version = INET_VERSION_IPV4; version <= INET_VERSION_IPV6; version++)
   {
#if (IPV4_SUPPORT == DISABLED || IPV4_STATS_SUPPORT == DISABLED)
      //IPv4 version?
      if(version == INET_VERSION_IPV4)
      {
         //IPv4 is not implemented
         continue;
      }
#endif
#if (IPV6_SUPPORT == DISABLED || IPV6_STATS_SUPPORT == DISABLED)
      //IPv6 version?
      if(version == INET_VERSION_IPV6)
      {
         //IPv6 is not implemented
         continue;
      }
#endif

      //Loop through network interfaces
      for(index = 1; index <= context->numInterfaces; index++)
      {
         //Append the instance identifier to the OID prefix
         n = object->oidLen;

         //ipIfStatsIPVersion is used as 1st instance identifier
         error = mibEncodeIndex(nextOid, *nextOidLen, &n, version);
         //Any error to report?
         if(error)
            return error;

         //ipIfStatsIfIndex is used as 2nd instance identifier
         error = mibEncodeIndex(nextOid, *nextOidLen, &n, index);
         //Any error to report?
         if(error)
            return error;

         //Check whether the resulting object identifier lexicographically
         //follows the specified OID
         if(oidComp(nextOid, n, oid, oidLen) > 0)
         {
            //Save the length of the resulting object identifier
            *nextOidLen = n;
            //Next object found
            return NO_ERROR;
         }
      }
   }

   //The specified OID does not lexicographically precede the name
   //of some object
   return ERROR_OBJECT_NOT_FOUND;
}


/**
 * @brief Get ipAddressPrefixEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpAddressPrefixEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   uint_t i;
   size_t n;
   uint_t index;
   IpAddr prefix;
   uint32_t prefixLen;
   NetContext *context;
   NetInterface *interface;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipAddressPrefixIfIndex is used as 1st instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &index);
   //Invalid instance identifier?
   if(error)
      return error;

   //ipAddressPrefixType and ipAddressPrefixPrefix are used as
   //2nd and 3rd instance identifiers
   error = mibDecodeIpAddr(oid, oidLen, &n, &prefix);
   //Invalid instance identifier?
   if(error)
      return error;

   //ipAddressPrefixLength is used as 4th instance identifier
   error = mibDecodeUnsigned32(oid, oidLen, &n, &prefixLen);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check index range
   if(index < 1 || index > context->numInterfaces)
      return ERROR_INSTANCE_NOT_FOUND;

   //Point to the underlying interface
   interface = &context->interfaces[index - 1];

#if (IPV4_SUPPORT == ENABLED)
   //IPv4 prefix?
   if(prefix.length == sizeof(Ipv4Addr))
   {
      Ipv4AddrEntry *entry;

      //Loop through the list of IPv4 addresses assigned to the interface
      for(i = 0; i < IPV4_ADDR_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv4Context.addrList[i];

         //Valid IPv4 address?
         if(entry->state == IPV4_ADDR_STATE_VALID)
         {
            //Compare prefix length against the specified value
            if(ipv4GetPrefixLength(entry->subnetMask) == prefixLen)
            {
               //Check subnet mask
               if((entry->addr & entry->subnetMask) == prefix.ipv4Addr)
               {
                  break;
               }
            }
         }
      }

      //Any matching entry found?
      if(i < IPV4_ADDR_LIST_SIZE)
      {
         //ipAddressPrefixOrigin object?
         if(osStrcmp(object->name, "ipAddressPrefixOrigin") == 0)
         {
            //The origin of this prefix
            value->integer = IP_MIB_PREFIX_ORIGIN_MANUAL;
         }
         //ipAddressPrefixOnLinkFlag object?
         else if(osStrcmp(object->name, "ipAddressPrefixOnLinkFlag") == 0)
         {
            //This flag indicates whether this prefix can be used for on-link
            //determination
            value->integer = MIB_TRUTH_VALUE_TRUE;
         }
         //ipAddressPrefixAutonomousFlag object?
         else if(osStrcmp(object->name, "ipAddressPrefixAutonomousFlag") == 0)
         {
            //This flag indicates whether this prefix can be used for autonomous
            //address configuration
            value->integer = MIB_TRUTH_VALUE_FALSE;
         }
         //ipAddressPrefixAdvPreferredLifetime object?
         else if(osStrcmp(object->name, "ipAddressPrefixAdvPreferredLifetime") == 0)
         {
            //Remaining length of time, in seconds, that this prefix will
            //continue to be preferred
            value->unsigned32 = UINT32_MAX;
         }
         //ipAddressPrefixAdvValidLifetime object?
         else if(osStrcmp(object->name, "ipAddressPrefixAdvValidLifetime") == 0)
         {
            //Remaining length of time, in seconds, that this prefix will
            //continue to be valid
            value->unsigned32 = UINT32_MAX;
         }
         //Unknown object?
         else
         {
            //The specified object does not exist
            error = ERROR_OBJECT_NOT_FOUND;
         }
      }
      else
      {
         //Report an error
         error = ERROR_INSTANCE_NOT_FOUND;
      }
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 prefix?
   if(prefix.length == sizeof(Ipv6Addr))
   {
      Ipv6PrefixEntry *entry;

      //Loop through the IPv6 prefix list
      for(i = 0; i < IPV6_PREFIX_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv6Context.prefixList[i];

         //Check whether the prefix is valid
         if(entry->validLifetime > 0)
         {
            //Compare prefix length against the specified value
            if(entry->prefixLen == prefixLen)
            {
               //Check whether the current entry matches the specified prefix
               if(ipv6CompPrefix(&entry->prefix, &prefix.ipv6Addr, prefixLen))
               {
                  break;
               }
            }
         }
      }

      //Any matching entry found?
      if(i < IPV6_PREFIX_LIST_SIZE)
      {
         //ipAddressPrefixOrigin object?
         if(osStrcmp(object->name, "ipAddressPrefixOrigin") == 0)
         {
            //The origin of this prefix
            if(entry->permanent)
            {
               value->integer = IP_MIB_PREFIX_ORIGIN_MANUAL;
            }
            else
            {
               value->integer = IP_MIB_PREFIX_ORIGIN_ROUTER_ADV;
            }
         }
         //ipAddressPrefixOnLinkFlag object?
         else if(osStrcmp(object->name, "ipAddressPrefixOnLinkFlag") == 0)
         {
            //This flag indicates whether this prefix can be used for on-link
            //determination
            if(entry->onLinkFlag)
            {
               value->integer = MIB_TRUTH_VALUE_TRUE;
            }
            else
            {
               value->integer = MIB_TRUTH_VALUE_FALSE;
            }
         }
         //ipAddressPrefixAutonomousFlag object?
         else if(osStrcmp(object->name, "ipAddressPrefixAutonomousFlag") == 0)
         {
            //This flag indicates whether this prefix can be used for autonomous
            //address configuration
            if(entry->autonomousFlag)
            {
               value->integer = MIB_TRUTH_VALUE_TRUE;
            }
            else
            {
               value->integer = MIB_TRUTH_VALUE_FALSE;
            }
         }
         //ipAddressPrefixAdvPreferredLifetime object?
         else if(osStrcmp(object->name, "ipAddressPrefixAdvPreferredLifetime") == 0)
         {
            //Remaining length of time, in seconds, that this prefix will
            //continue to be preferred
            if(entry->preferredLifetime == INFINITE_DELAY)
            {
               value->unsigned32 = UINT32_MAX;
            }
            else
            {
               value->unsigned32 = entry->preferredLifetime / 1000;
            }
         }
         //ipAddressPrefixAdvValidLifetime object?
         else if(osStrcmp(object->name, "ipAddressPrefixAdvValidLifetime") == 0)
         {
            //Remaining length of time, in seconds, that this prefix will
            //continue to be valid
            if(entry->validLifetime == INFINITE_DELAY)
            {
               value->unsigned32 = UINT32_MAX;
            }
            else
            {
               value->unsigned32 = entry->validLifetime / 1000;
            }
         }
         //Unknown object?
         else
         {
            //The specified object does not exist
            error = ERROR_OBJECT_NOT_FOUND;
         }
      }
      else
      {
         //Report an error
         error = ERROR_INSTANCE_NOT_FOUND;
      }
   }
   else
#endif
   //Invalid prefix?
   {
      //Report an error
      error = ERROR_INSTANCE_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next ipAddressPrefixEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpAddressPrefixEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   uint_t i;
   size_t n;
   uint_t index;
   uint_t curIndex;
   uint_t length;
   uint_t curLength;
   bool_t acceptable;
   IpAddr prefix;
   IpAddr curPrefix;
   NetContext *context;
   NetInterface *interface;

   //Initialize variables
   index = 0;
   prefix = IP_ADDR_UNSPECIFIED;
   length = 0;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

#if (IPV4_SUPPORT == ENABLED)
   //Loop through network interfaces
   for(curIndex = 1; curIndex <= context->numInterfaces; curIndex++)
   {
      Ipv4AddrEntry *entry;

      //Point to the current interface
      interface = &context->interfaces[curIndex - 1];

      //Loop through the list of IPv4 addresses assigned to the interface
      for(i = 0; i < IPV4_ADDR_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv4Context.addrList[i];

         //Valid subnet mask?
         if(entry->state == IPV4_ADDR_STATE_VALID &&
            entry->subnetMask != IPV4_UNSPECIFIED_ADDR)
         {
            //Retrieve current prefix
            curPrefix.length = sizeof(Ipv4Addr);
            curPrefix.ipv4Addr = entry->addr & entry->subnetMask;
            curLength = ipv4GetPrefixLength(entry->subnetMask);

            //Append the instance identifier to the OID prefix
            n = object->oidLen;

            //ipAddressPrefixIfIndex is used as 1st instance identifier
            error = mibEncodeIndex(nextOid, *nextOidLen, &n, curIndex);
            //Any error to report?
            if(error)
               return error;

            //ipAddressPrefixType and ipAddressPrefixPrefix are used as
            //2nd and 3rd instance identifiers
            error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &curPrefix);
            //Invalid instance identifier?
            if(error)
               return error;

            //ipAddressPrefixLength is used as 4th instance identifier
            error = mibEncodeUnsigned32(nextOid, *nextOidLen, &n, curLength);
            //Any error to report?
            if(error)
               return error;

            //Check whether the resulting object identifier lexicographically
            //follows the specified OID
            if(oidComp(nextOid, n, oid, oidLen) > 0)
            {
               //Perform lexicographic comparison
               if(index == 0)
               {
                  acceptable = TRUE;
               }
               else if(curIndex < index)
               {
                  acceptable = TRUE;
               }
               else if(curIndex > index)
               {
                  acceptable = FALSE;
               }
               else if(mibCompIpAddr(&curPrefix, &prefix) < 0)
               {
                  acceptable = TRUE;
               }
               else if(mibCompIpAddr(&curPrefix, &prefix) > 0)
               {
                  acceptable = FALSE;
               }
               else if(curLength < length)
               {
                  acceptable = TRUE;
               }
               else
               {
                  acceptable = FALSE;
               }

               //Save the closest object identifier that follows the specified
               //OID in lexicographic order
               if(acceptable)
               {
                  index = curIndex;
                  prefix = curPrefix;
                  length = curLength;
               }
            }
         }
      }
   }
#endif

#if (IPV6_SUPPORT == ENABLED)
   //Loop through network interfaces
   for(curIndex = 1; curIndex <= context->numInterfaces; curIndex++)
   {
      Ipv6PrefixEntry *entry;

      //Point to the current interface
      interface = &context->interfaces[curIndex - 1];

      //Loop through the IPv6 prefix list
      for(i = 0; i < IPV6_PREFIX_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv6Context.prefixList[i];

         //Check whether the prefix is valid
         if(entry->validLifetime > 0)
         {
            //Retrieve current prefix
            curPrefix.length = sizeof(Ipv6Addr);
            curPrefix.ipv6Addr = entry->prefix;
            curLength = entry->prefixLen;

            //Append the instance identifier to the OID prefix
            n = object->oidLen;

            //ipAddressPrefixIfIndex is used as 1st instance identifier
            error = mibEncodeIndex(nextOid, *nextOidLen, &n, curIndex);
            //Any error to report?
            if(error)
               return error;

            //ipAddressPrefixType and ipAddressPrefixPrefix are used as
            //2nd and 3rd instance identifiers
            error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &curPrefix);
            //Invalid instance identifier?
            if(error)
               return error;

            //ipAddressPrefixLength is used as 4th instance identifier
            error = mibEncodeUnsigned32(nextOid, *nextOidLen, &n, curLength);
            //Any error to report?
            if(error)
               return error;

            //Check whether the resulting object identifier lexicographically
            //follows the specified OID
            if(oidComp(nextOid, n, oid, oidLen) > 0)
            {
               //Perform lexicographic comparison
               if(index == 0)
               {
                  acceptable = TRUE;
               }
               else if(curIndex < index)
               {
                  acceptable = TRUE;
               }
               else if(curIndex > index)
               {
                  acceptable = FALSE;
               }
               else if(mibCompIpAddr(&curPrefix, &prefix) < 0)
               {
                  acceptable = TRUE;
               }
               else if(mibCompIpAddr(&curPrefix, &prefix) > 0)
               {
                  acceptable = FALSE;
               }
               else if(curLength < length)
               {
                  acceptable = TRUE;
               }
               else
               {
                  acceptable = FALSE;
               }

               //Save the closest object identifier that follows the specified
               //OID in lexicographic order
               if(acceptable)
               {
                  index = curIndex;
                  prefix = curPrefix;
                  length = curLength;
               }
            }
         }
      }
   }
#endif

   //The specified OID does not lexicographically precede the name
   //of some object?
   if(index == 0)
      return ERROR_OBJECT_NOT_FOUND;

   //Append the instance identifier to the OID prefix
   n = object->oidLen;

   //ipAddressPrefixIfIndex is used as 1st instance identifier
   error = mibEncodeIndex(nextOid, *nextOidLen, &n, index);
   //Any error to report?
   if(error)
      return error;

   //ipAddressPrefixType and ipAddressPrefixPrefix are used as
   //2nd and 3rd instance identifiers
   error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &prefix);
   //Invalid instance identifier?
   if(error)
      return error;

   //ipAddressPrefixLength is used as 4th instance identifier
   error = mibEncodeUnsigned32(nextOid, *nextOidLen, &n, length);
   //Any error to report?
   if(error)
      return error;

   //Save the length of the resulting object identifier
   *nextOidLen = n;
   //Next object found
   return NO_ERROR;
}


/**
 * @brief Set ipAddressSpinLock object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in] value Object value
 * @param[in] valueLen Length of the object value, in bytes
 * @param[in] commit This flag tells whether the changes shall be committed
 *   to the MIB base
 * @return Error code
 **/

error_t ipMibSetIpAddressSpinLock(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen, bool_t commit)
{
   //Test and increment spin lock
   return mibTestAndIncSpinLock(&ipMibBase.ipAddressSpinLock,
      value->integer, commit);
}


/**
 * @brief Get ipAddressSpinLock object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpAddressSpinLock(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   //Get the current value of the spin lock
   value->integer = ipMibBase.ipAddressSpinLock;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set ipAddressEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in] value Object value
 * @param[in] valueLen Length of the object value, in bytes
 * @param[in] commit This flag tells whether the changes shall be committed
 *   to the MIB base
 * @return Error code
 **/

error_t ipMibSetIpAddressEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen, bool_t commit)
{
   //Not implemented
   return ERROR_WRITE_FAILED;
}


/**
 * @brief Get ipAddressEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpAddressEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   uint_t i;
   size_t n;
   uint_t index;
   IpAddr ipAddr;
   NetContext *context;
   NetInterface *interface;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipAddressAddrType and ipAddressAddr are used as instance identifiers
   error = mibDecodeIpAddr(oid, oidLen, &n, &ipAddr);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

#if (IPV4_SUPPORT == ENABLED)
   //IPv4 address?
   if(ipAddr.length == sizeof(Ipv4Addr))
   {
      Ipv4AddrEntry *entry;

      //Loop through network interfaces
      for(index = 1; index <= context->numInterfaces; index++)
      {
         //Point to the current interface
         interface = &context->interfaces[index - 1];

         //Loop through the list of IPv4 addresses assigned to the interface
         for(i = 0; i < IPV4_ADDR_LIST_SIZE; i++)
         {
            //Point to the current entry
            entry = &interface->ipv4Context.addrList[i];

            //Compare the current address against the IP address used as
            //instance identifier
            if(entry->state == IPV4_ADDR_STATE_VALID &&
               entry->addr == ipAddr.ipv4Addr)
            {
               break;
            }
         }

         //IPv4 address found?
         if(i < IPV4_ADDR_LIST_SIZE)
            break;
      }

      //IPv4 address found?
      if(index <= context->numInterfaces)
      {
         //ipAddressIfIndex object?
         if(osStrcmp(object->name, "ipAddressIfIndex") == 0)
         {
            //Index value that uniquely identifies the interface to which
            //this entry is applicable
            value->integer = index;
         }
         //ipAddressType object?
         else if(osStrcmp(object->name, "ipAddressType") == 0)
         {
            //Type of IP address
            value->integer = IP_MIB_ADDR_TYPE_UNICAST;
         }
         //ipAddressPrefix object?
         else if(osStrcmp(object->name, "ipAddressPrefix") == 0)
         {
            IpAddr prefix;
            uint_t length;

            //OID of the ipAddressPrefixOrigin object
            const uint8_t ipAddressPrefixOriginOid[] = {43, 6, 1, 2, 1, 4, 32, 1, 5};

            //Retrieve current prefix
            prefix.length = sizeof(Ipv4Addr);
            prefix.ipv4Addr = entry->addr & entry->subnetMask;
            length = ipv4GetPrefixLength(entry->subnetMask);

            //Build a pointer to the row in the prefix table to which this
            //address belongs
            n = sizeof(ipAddressPrefixOriginOid);

            //Make sure the buffer is large enough to hold the OID prefix
            if(*valueLen < n)
               return ERROR_BUFFER_OVERFLOW;

            //Copy OID prefix
            osMemcpy(value->oid, ipAddressPrefixOriginOid, n);

            //ipAddressPrefixIfIndex is used as 1st instance identifier
            error = mibEncodeIndex(value->oid, *valueLen, &n, index);
            //Any error to report?
            if(error)
               return error;

            //ipAddressPrefixType and ipAddressPrefixPrefix are used as
            //2nd and 3rd instance identifiers
            error = mibEncodeIpAddr(value->oid, *valueLen, &n, &prefix);
            //Invalid instance identifier?
            if(error)
               return error;

            //ipAddressPrefixLength is used as 4th instance identifier
            error = mibEncodeUnsigned32(value->oid, *valueLen, &n, length);
            //Any error to report?
            if(error)
               return error;

            //Return object length
            *valueLen = n;
         }
         //ipAddressOrigin object?
         else if(osStrcmp(object->name, "ipAddressOrigin") == 0)
         {
#if (AUTO_IP_SUPPORT == ENABLED)
            //Address chosen by the system at random?
            if(interface->autoIpContext != NULL &&
               interface->autoIpContext->running)
            {
               //Origin of the address
               value->integer = IP_MIB_ADDR_ORIGIN_RANDOM;
            }
            else
#endif
#if (DHCP_CLIENT_SUPPORT == ENABLED)
            //Address assigned to this system by a DHCP server?
            if(interface->dhcpClientContext != NULL &&
               interface->dhcpClientContext->running)
            {
               //Origin of the address
               value->integer = IP_MIB_ADDR_ORIGIN_DHCP;
            }
            else
#endif
            //Manually configured address?
            {
               //Origin of the address
               value->integer = IP_MIB_ADDR_ORIGIN_MANUAL;
            }
         }
         //ipAddressStatus object?
         else if(osStrcmp(object->name, "ipAddressStatus") == 0)
         {
            //Status of the IP address
            if(entry->state == IPV4_ADDR_STATE_VALID)
            {
               value->integer = IP_MIB_ADDR_STATUS_PREFERRED;
            }
            else if(entry->state == IPV4_ADDR_STATE_TENTATIVE)
            {
               value->integer = IP_MIB_ADDR_STATUS_TENTATIVE;
            }
            else
            {
               value->integer = IP_MIB_ADDR_STATUS_UNKNOWN;
            }
         }
         //ipAddressCreated object?
         else if(osStrcmp(object->name, "ipAddressCreated") == 0)
         {
            //Get object value
            value->timeTicks = 0;
         }
         //ipAddressLastChanged object?
         else if(osStrcmp(object->name, "ipAddressLastChanged") == 0)
         {
            //Get object value
            value->timeTicks = 0;
         }
         //ipAddressRowStatus object?
         else if(osStrcmp(object->name, "ipAddressRowStatus") == 0)
         {
            //Get object value
            value->integer = MIB_ROW_STATUS_ACTIVE;
         }
         //ipAddressStorageType object?
         else if(osStrcmp(object->name, "ipAddressStorageType") == 0)
         {
            //Get object value
            value->integer = MIB_STORAGE_TYPE_VOLATILE;
         }
         //Unknown object?
         else
         {
            //The specified object does not exist
            error = ERROR_OBJECT_NOT_FOUND;
         }
      }
      else
      {
         //Report an error
         error = ERROR_INSTANCE_NOT_FOUND;
      }
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 address?
   if(ipAddr.length == sizeof(Ipv6Addr))
   {
      Ipv6AddrEntry *entry;

      //Loop through network interfaces
      for(index = 1; index <= context->numInterfaces; index++)
      {
         //Point to the current interface
         interface = &context->interfaces[index - 1];

         //Loop through the list of IPv6 addresses assigned to the interface
         for(i = 0; i < IPV6_ADDR_LIST_SIZE; i++)
         {
            //Point to the current entry
            entry = &interface->ipv6Context.addrList[i];

            //Compare the current address against the IP address used as
            //instance identifier
            if(entry->state != IPV6_ADDR_STATE_INVALID &&
               ipv6CompAddr(&entry->addr, &ipAddr.ipv6Addr))
            {
               break;
            }
         }

         //IPv6 address found?
         if(i < IPV6_ADDR_LIST_SIZE)
            break;
      }

      //IPv6 address found?
      if(index <= context->numInterfaces)
      {
         //ipAddressIfIndex object?
         if(osStrcmp(object->name, "ipAddressIfIndex") == 0)
         {
            //Index value that uniquely identifies the interface to which
            //this entry is applicable
            value->integer = index;
         }
         //ipAddressType object?
         else if(osStrcmp(object->name, "ipAddressType") == 0)
         {
            //Type of IP address
            value->integer = IP_MIB_ADDR_TYPE_UNICAST;
         }
         //ipAddressPrefix object?
         else if(osStrcmp(object->name, "ipAddressPrefix") == 0)
         {
            //Unknown OID
            const uint8_t unknownOid[] = {0};

            //Make sure the buffer is large enough to hold the OID prefix
            if(*valueLen < sizeof(unknownOid))
               return ERROR_BUFFER_OVERFLOW;

            //Copy OID prefix
            osMemcpy(value->oid, unknownOid, sizeof(unknownOid));
            //Return object length
            *valueLen = sizeof(unknownOid);
         }
         //ipAddressOrigin object?
         else if(osStrcmp(object->name, "ipAddressOrigin") == 0)
         {
            //Origin of the address
            value->integer = IP_MIB_ADDR_ORIGIN_MANUAL;
         }
         //ipAddressStatus object?
         else if(osStrcmp(object->name, "ipAddressStatus") == 0)
         {
            //Status of the IP address
            if(entry->state == IPV6_ADDR_STATE_PREFERRED)
            {
               value->integer = IP_MIB_ADDR_STATUS_PREFERRED;
            }
            else if(entry->state == IPV6_ADDR_STATE_DEPRECATED)
            {
               value->integer = IP_MIB_ADDR_STATUS_DEPRECATED;
            }
            else if(entry->state == IPV6_ADDR_STATE_TENTATIVE)
            {
               value->integer = IP_MIB_ADDR_STATUS_TENTATIVE;
            }
            else
            {
               value->integer = IP_MIB_ADDR_STATUS_UNKNOWN;
            }
         }
         //ipAddressCreated object?
         else if(osStrcmp(object->name, "ipAddressCreated") == 0)
         {
            //Get object value
            value->timeTicks = 0;
         }
         //ipAddressLastChanged object?
         else if(osStrcmp(object->name, "ipAddressLastChanged") == 0)
         {
            //Get object value
            value->timeTicks = 0;
         }
         //ipAddressRowStatus object?
         else if(osStrcmp(object->name, "ipAddressRowStatus") == 0)
         {
            //Get object value
            value->integer = MIB_ROW_STATUS_ACTIVE;
         }
         //ipAddressStorageType object?
         else if(osStrcmp(object->name, "ipAddressStorageType") == 0)
         {
            //Get object value
            value->integer = MIB_STORAGE_TYPE_VOLATILE;
         }
         //Unknown object?
         else
         {
            //The specified object does not exist
            error = ERROR_OBJECT_NOT_FOUND;
         }
      }
      else
      {
         //Report an error
         error = ERROR_INSTANCE_NOT_FOUND;
      }
   }
   else
#endif
   //Invalid IP address?
   {
      //Report an error
      error = ERROR_INSTANCE_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next ipAddressEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpAddressEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   uint_t i;
   size_t n;
   uint_t index;
   bool_t acceptable;
   IpAddr ipAddr;
   IpAddr curIpAddr;
   NetContext *context;
   NetInterface *interface;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Initialize variable
   ipAddr = IP_ADDR_UNSPECIFIED;

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

#if (IPV4_SUPPORT == ENABLED)
   //Loop through network interfaces
   for(index = 1; index <= context->numInterfaces; index++)
   {
      Ipv4AddrEntry *entry;

      //Point to the current interface
      interface = &context->interfaces[index - 1];

      //Loop through the list of IPv4 addresses assigned to the interface
      for(i = 0; i < IPV4_ADDR_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv4Context.addrList[i];

         //Valid IPv4 address?
         if(entry->state == IPV4_ADDR_STATE_VALID)
         {
            //Get current address
            curIpAddr.length = sizeof(Ipv4Addr);
            curIpAddr.ipv4Addr = entry->addr;

            //Append the instance identifier to the OID prefix
            n = object->oidLen;

            //ipAddressAddrType and ipAddressAddr are used as instance identifiers
            error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &curIpAddr);
            //Any error to report?
            if(error)
               return error;

            //Check whether the resulting object identifier lexicographically
            //follows the specified OID
            if(oidComp(nextOid, n, oid, oidLen) > 0)
            {
               //Perform lexicographic comparison
               if(ipAddr.length == 0)
               {
                  acceptable = TRUE;
               }
               else if(mibCompIpAddr(&curIpAddr, &ipAddr) < 0)
               {
                  acceptable = TRUE;
               }
               else
               {
                  acceptable = FALSE;
               }

               //Save the closest object identifier that follows the specified
               //OID in lexicographic order
               if(acceptable)
                  ipAddr = curIpAddr;
            }
         }
      }
   }
#endif

#if (IPV6_SUPPORT == ENABLED)
   //Loop through network interfaces
   for(index = 1; index <= context->numInterfaces; index++)
   {
      Ipv6AddrEntry *entry;

      //Point to the current interface
      interface = &context->interfaces[index - 1];

      //Loop through the list of IPv6 addresses assigned to the interface
      for(i = 0; i < IPV6_ADDR_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv6Context.addrList[i];

         //Valid IPv6 address?
         if(entry->state != IPV6_ADDR_STATE_INVALID)
         {
            //Get current address
            curIpAddr.length = sizeof(Ipv6Addr);
            curIpAddr.ipv6Addr = entry->addr;

            //Append the instance identifier to the OID prefix
            n = object->oidLen;

            //ipAddressAddrType and ipAddressAddr are used as instance identifiers
            error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &curIpAddr);
            //Any error to report?
            if(error)
               return error;

            //Check whether the resulting object identifier lexicographically
            //follows the specified OID
            if(oidComp(nextOid, n, oid, oidLen) > 0)
            {
               //Perform lexicographic comparison
               if(ipAddr.length == 0)
               {
                  acceptable = TRUE;
               }
               else if(mibCompIpAddr(&curIpAddr, &ipAddr) < 0)
               {
                  acceptable = TRUE;
               }
               else
               {
                  acceptable = FALSE;
               }

               //Save the closest object identifier that follows the specified
               //OID in lexicographic order
               if(acceptable)
                  ipAddr = curIpAddr;
            }
         }
      }
   }
#endif

   //The specified OID does not lexicographically precede the name
   //of some object?
   if(ipAddr.length == 0)
      return ERROR_OBJECT_NOT_FOUND;

   //Append the instance identifier to the OID prefix
   n = object->oidLen;

   //ipAddressAddrType and ipAddressAddr are used as instance identifiers
   error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &ipAddr);
   //Any error to report?
   if(error)
      return error;

   //Save the length of the resulting object identifier
   *nextOidLen = n;
   //Next object found
   return NO_ERROR;
}


/**
 * @brief Set ipNetToPhysicalEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in] value Object value
 * @param[in] valueLen Length of the object value, in bytes
 * @param[in] commit This flag tells whether the changes shall be committed
 *   to the MIB base
 * @return Error code
 **/

error_t ipMibSetIpNetToPhysicalEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen, bool_t commit)
{
   //Not implemented
   return ERROR_WRITE_FAILED;
}


/**
 * @brief Get ipNetToPhysicalEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpNetToPhysicalEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   size_t n;
   uint_t index;
   IpAddr ipAddr;
   NetContext *context;
   NetInterface *interface;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipNetToPhysicalIfIndex is used as 1st instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &index);
   //Invalid instance identifier?
   if(error)
      return error;

   //ipNetToPhysicalNetAddressType and ipNetToPhysicalNetAddress are
   //used as 2nd and 3rd instance identifiers
   error = mibDecodeIpAddr(oid, oidLen, &n, &ipAddr);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check index range
   if(index < 1 || index > context->numInterfaces)
      return ERROR_INSTANCE_NOT_FOUND;

   //Point to the network interface
   interface = &context->interfaces[index - 1];
   //Avoid warnings from the compiler
   (void) interface;

#if (IPV4_SUPPORT == ENABLED && ETH_SUPPORT == ENABLED)
   //IPv4 address?
   if(ipAddr.length == sizeof(Ipv4Addr))
   {
      ArpCacheEntry *entry;

      //Search the ARP cache for the specified IPv4 address
      entry = arpFindEntry(interface, ipAddr.ipv4Addr);

      //Check whether a matching entry has been found
      if(entry != NULL)
      {
         //ipNetToPhysicalPhysAddress object?
         if(osStrcmp(object->name, "ipNetToPhysicalPhysAddress") == 0)
         {
            //Make sure the buffer is large enough to hold the entire object
            if(*valueLen >= sizeof(MacAddr))
            {
               //Copy object value
               macCopyAddr(value->octetString, &entry->macAddr);
               //Return object length
               *valueLen = sizeof(MacAddr);
            }
            else
            {
               //Report an error
               error = ERROR_BUFFER_OVERFLOW;
            }
         }
         //ipNetToPhysicalLastUpdated object?
         else if(osStrcmp(object->name, "ipNetToPhysicalLastUpdated") == 0)
         {
            //Get object value
            value->timeTicks = entry->timestamp / 10;
         }
         //ipNetToPhysicalType object?
         else if(osStrcmp(object->name, "ipNetToPhysicalType") == 0)
         {
            //Get object value
            value->integer = IP_MIB_NET_TO_PHYS_TYPE_DYNAMIC;
         }
         //ipNetToPhysicalState object?
         else if(osStrcmp(object->name, "ipNetToPhysicalState") == 0)
         {
            //Get object value
            value->integer = IP_MIB_NET_TO_PHYS_STATE_UNKNOWN;
         }
         //ipNetToPhysicalRowStatus object?
         else if(osStrcmp(object->name, "ipNetToPhysicalRowStatus") == 0)
         {
            //Get object value
            value->integer = MIB_ROW_STATUS_ACTIVE;
         }
         //Unknown object?
         else
         {
            //The specified object does not exist
            error = ERROR_OBJECT_NOT_FOUND;
         }
      }
      else
      {
         //Report an error
         error = ERROR_INSTANCE_NOT_FOUND;
      }
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 address?
   if(ipAddr.length == sizeof(Ipv6Addr))
   {
      NdpNeighborCacheEntry *entry;

      //Search the Neighbor cache for the specified IPv6 address
      entry = ndpFindNeighborCacheEntry(interface, &ipAddr.ipv6Addr);

      //Check whether a matching entry has been found
      if(entry != NULL)
      {
         //ipNetToPhysicalPhysAddress object?
         if(osStrcmp(object->name, "ipNetToPhysicalPhysAddress") == 0)
         {
            //Make sure the buffer is large enough to hold the entire object
            if(*valueLen >= sizeof(MacAddr))
            {
               //Copy object value
               macCopyAddr(value->octetString, &entry->macAddr);
               //Return object length
               *valueLen = sizeof(MacAddr);
            }
            else
            {
               //Report an error
               error = ERROR_BUFFER_OVERFLOW;
            }
         }
         //ipNetToPhysicalLastUpdated object?
         else if(osStrcmp(object->name, "ipNetToPhysicalLastUpdated") == 0)
         {
            //Get object value
            value->timeTicks = entry->timestamp / 10;
         }
         //ipNetToPhysicalType object?
         else if(osStrcmp(object->name, "ipNetToPhysicalType") == 0)
         {
            //Get object value
            value->integer = IP_MIB_NET_TO_PHYS_TYPE_DYNAMIC;
         }
         //ipNetToPhysicalState object?
         else if(osStrcmp(object->name, "ipNetToPhysicalState") == 0)
         {
            //Get object value
            if(entry->state == NDP_STATE_INCOMPLETE)
            {
               value->integer = IP_MIB_NET_TO_PHYS_STATE_INCOMPLETE;
            }
            else if(entry->state == NDP_STATE_REACHABLE)
            {
               value->integer = IP_MIB_NET_TO_PHYS_STATE_REACHABLE;
            }
            else if(entry->state == NDP_STATE_STALE)
            {
               value->integer = IP_MIB_NET_TO_PHYS_STATE_STALE;
            }
            else if(entry->state == NDP_STATE_DELAY)
            {
               value->integer = IP_MIB_NET_TO_PHYS_STATE_DELAY;
            }
            else if(entry->state == NDP_STATE_PROBE)
            {
               value->integer = IP_MIB_NET_TO_PHYS_STATE_PROBE;
            }
            else
            {
               value->integer = IP_MIB_NET_TO_PHYS_STATE_UNKNOWN;
            }
         }
         //ipNetToPhysicalRowStatus object?
         else if(osStrcmp(object->name, "ipNetToPhysicalRowStatus") == 0)
         {
            //Get object value
            value->integer = MIB_ROW_STATUS_ACTIVE;
         }
         //Unknown object?
         else
         {
            //The specified object does not exist
            error = ERROR_OBJECT_NOT_FOUND;
         }
      }
      else
      {
         //Report an error
         error = ERROR_INSTANCE_NOT_FOUND;
      }
   }
   else
#endif
   //Invalid IP address?
   {
      //Report an error
      error = ERROR_INSTANCE_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next ipNetToPhysicalEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpNetToPhysicalEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   uint_t i;
   size_t n;
   uint_t index;
   uint_t curIndex;
   bool_t acceptable;
   IpAddr ipAddr;
   IpAddr curIpAddr;
   NetContext *context;
   NetInterface *interface;

   //Initialize variables
   index = 0;
   ipAddr = IP_ADDR_UNSPECIFIED;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

   //Loop through network interfaces
   for(curIndex = 1; curIndex <= context->numInterfaces; curIndex++)
   {
      //Point to the current interface
      interface = &context->interfaces[curIndex - 1];

      //Avoid warnings from the compiler
      (void) i;
      (void) acceptable;
      (void) curIpAddr;
      (void) interface;

#if (IPV4_SUPPORT == ENABLED && ETH_SUPPORT == ENABLED)
      //Loop through ARP cache entries
      for(i = 0; i < ARP_CACHE_SIZE; i++)
      {
         ArpCacheEntry *entry;

         //Point to the current entry
         entry = &interface->arpCache[i];

         //Valid entry?
         if(entry->state != ARP_STATE_NONE)
         {
            //Get current IP address
            curIpAddr.length = sizeof(Ipv4Addr);
            curIpAddr.ipv4Addr = entry->ipAddr;

            //Append the instance identifier to the OID prefix
            n = object->oidLen;

            //ipNetToPhysicalIfIndex is used as 1st instance identifier
            error = mibEncodeIndex(nextOid, *nextOidLen, &n, curIndex);
            //Any error to report?
            if(error)
               return error;

            //ipNetToPhysicalNetAddressType and ipNetToPhysicalNetAddress are
            //used as 2nd and 3rd instance identifiers
            error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &curIpAddr);
            //Any error to report?
            if(error)
               return error;

            //Check whether the resulting object identifier lexicographically
            //follows the specified OID
            if(oidComp(nextOid, n, oid, oidLen) > 0)
            {
               //Perform lexicographic comparison
               if(index == 0)
               {
                  acceptable = TRUE;
               }
               else if(curIndex < index)
               {
                  acceptable = TRUE;
               }
               else if(curIndex > index)
               {
                  acceptable = FALSE;
               }
               else if(mibCompIpAddr(&curIpAddr, &ipAddr) < 0)
               {
                  acceptable = TRUE;
               }
               else
               {
                  acceptable = FALSE;
               }

               //Save the closest object identifier that follows the specified
               //OID in lexicographic order
               if(acceptable)
               {
                  index = curIndex;
                  ipAddr = curIpAddr;
               }
            }
         }
      }
#endif

#if (IPV6_SUPPORT == ENABLED)
      //Loop through Neighbor cache entries
      for(i = 0; i < NDP_NEIGHBOR_CACHE_SIZE; i++)
      {
         NdpNeighborCacheEntry *entry;

         //Point to the current entry
         entry = &interface->ndpContext.neighborCache[i];

         //Valid entry?
         if(entry->state != NDP_STATE_NONE)
         {
            //Get current IP address
            curIpAddr.length = sizeof(Ipv6Addr);
            curIpAddr.ipv6Addr = entry->ipAddr;

            //Append the instance identifier to the OID prefix
            n = object->oidLen;

            //ipNetToPhysicalIfIndex is used as 1st instance identifier
            error = mibEncodeIndex(nextOid, *nextOidLen, &n, curIndex);
            //Any error to report?
            if(error)
               return error;

            //ipNetToPhysicalNetAddressType and ipNetToPhysicalNetAddress are
            //used as 2nd and 3rd instance identifiers
            error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &curIpAddr);
            //Any error to report?
            if(error)
               return error;

            //Check whether the resulting object identifier lexicographically
            //follows the specified OID
            if(oidComp(nextOid, n, oid, oidLen) > 0)
            {
               //Perform lexicographic comparison
               if(index == 0)
               {
                  acceptable = TRUE;
               }
               else if(curIndex < index)
               {
                  acceptable = TRUE;
               }
               else if(curIndex > index)
               {
                  acceptable = FALSE;
               }
               else if(mibCompIpAddr(&curIpAddr, &ipAddr) < 0)
               {
                  acceptable = TRUE;
               }
               else
               {
                  acceptable = FALSE;
               }

               //Save the closest object identifier that follows the specified
               //OID in lexicographic order
               if(acceptable)
               {
                  index = curIndex;
                  ipAddr = curIpAddr;
               }
            }
         }
      }
#endif
   }

   //The specified OID does not lexicographically precede the name
   //of some object?
   if(index == 0)
      return ERROR_OBJECT_NOT_FOUND;

   //Append the instance identifier to the OID prefix
   n = object->oidLen;

   //ipNetToPhysicalIfIndex is used as 1st instance identifier
   error = mibEncodeIndex(nextOid, *nextOidLen, &n, index);
   //Any error to report?
   if(error)
      return error;

   //ipNetToPhysicalNetAddressType and ipNetToPhysicalNetAddress are
   //used as 2nd and 3rd instance identifiers
   error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &ipAddr);
   //Any error to report?
   if(error)
      return error;

   //Save the length of the resulting object identifier
   *nextOidLen = n;
   //Next object found
   return NO_ERROR;
}


/**
 * @brief Get ipv6ScopeZoneIndexEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpv6ScopeZoneIndexEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   size_t n;
   uint_t index;
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipv6ScopeZoneIndexIfIndex is used as instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &index);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check index range
   if(index < 1 || index > context->numInterfaces)
      return ERROR_INSTANCE_NOT_FOUND;

   //ipv6ScopeZoneIndexLinkLocal object?
   if(osStrcmp(object->name, "ipv6ScopeZoneIndexLinkLocal") == 0)
   {
      value->unsigned32 = index;
   }
   //ipv6ScopeZoneIndex3 object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndex3") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndexAdminLocal object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndexAdminLocal") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndexSiteLocal object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndexSiteLocal") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndex6 object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndex6") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndex7 object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndex7") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndexOrganizationLocal object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndexOrganizationLocal") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndex9 object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndex9") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndexA object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndexA") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndexB object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndexB") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndexC object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndexC") == 0)
   {
      value->unsigned32 = 0;
   }
   //ipv6ScopeZoneIndexD object?
   else if(osStrcmp(object->name, "ipv6ScopeZoneIndexD") == 0)
   {
      value->unsigned32 = 0;
   }
   //Unknown object?
   else
   {
      error = ERROR_OBJECT_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next ipv6ScopeZoneIndexEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpv6ScopeZoneIndexEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   size_t n;
   uint_t index;
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

   //Loop through network interfaces
   for(index = 1; index <= context->numInterfaces; index++)
   {
      //Append the instance identifier to the OID prefix
      n = object->oidLen;

      //ipv6ScopeZoneIndexIfIndex is used as instance identifier
      error = mibEncodeIndex(nextOid, *nextOidLen, &n, index);
      //Any error to report?
      if(error)
         return error;

      //Check whether the resulting object identifier lexicographically
      //follows the specified OID
      if(oidComp(nextOid, n, oid, oidLen) > 0)
      {
         //Save the length of the resulting object identifier
         *nextOidLen = n;
         //Next object found
         return NO_ERROR;
      }
   }

   //The specified OID does not lexicographically precede the name
   //of some object
   return ERROR_OBJECT_NOT_FOUND;
}


/**
 * @brief Get ipDefaultRouterEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpDefaultRouterEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   uint_t i;
   size_t n;
   uint_t index;
   IpAddr ipAddr;
   NetContext *context;
   NetInterface *interface;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipDefaultRouterAddress and ipDefaultRouterAddressType are used
   //as 1st and 2nd instance identifiers
   error = mibDecodeIpAddr(oid, oidLen, &n, &ipAddr);
   //Invalid instance identifier?
   if(error)
      return error;

   //ipDefaultRouterIfIndex is used as 3rd instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &index);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check index range
   if(index < 1 || index > context->numInterfaces)
      return ERROR_INSTANCE_NOT_FOUND;

   //Point to the network interface
   interface = &context->interfaces[index - 1];

#if (IPV4_SUPPORT == ENABLED)
   //IPv4 address?
   if(ipAddr.length == sizeof(Ipv4Addr))
   {
      Ipv4AddrEntry *entry;

      //Loop through the list of default gateways
      for(i = 0; i < IPV4_ADDR_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv4Context.addrList[i];

         //Check whether the specified IPv4 address matches any default gateway
         if(entry->state == IPV4_ADDR_STATE_VALID &&
            entry->defaultGateway != ipAddr.ipv4Addr)
         {
            break;
         }
      }

      //Valid gateway address?
      if(i <= IPV4_ADDR_LIST_SIZE)
      {
         //ipDefaultRouterLifetime object?
         if(osStrcmp(object->name, "ipDefaultRouterLifetime") == 0)
         {
            //Get object value
            value->unsigned32 = UINT16_MAX;
         }
         //ipDefaultRouterPreference object?
         else if(osStrcmp(object->name, "ipDefaultRouterPreference") == 0)
         {
            //Get object value
            value->integer = 0;
         }
         //Unknown object?
         else
         {
            //The specified object does not exist
            error = ERROR_OBJECT_NOT_FOUND;
         }
      }
      else
      {
         //Report an error
         error = ERROR_INSTANCE_NOT_FOUND;
      }
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED)
   //IPv6 address?
   if(ipAddr.length == sizeof(Ipv6Addr))
   {
      Ipv6RouterEntry *entry;

      //Loop through the Default Router List
      for(i = 0; i < IPV6_ROUTER_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv6Context.routerList[i];

         //Check the lifetime associated with the default router
         if(entry->lifetime)
         {
            //Check whether the specified IPv6 address matches any default router
            if(ipv6CompAddr(&entry->addr, &ipAddr.ipv6Addr))
            {
               break;
            }
         }
      }

      //Valid router address?
      if(i <= IPV6_ROUTER_LIST_SIZE)
      {
         //ipDefaultRouterLifetime object?
         if(osStrcmp(object->name, "ipDefaultRouterLifetime") == 0)
         {
            //Get object value
            if(entry->lifetime == INFINITE_DELAY)
            {
               value->unsigned32 = UINT16_MAX;
            }
            else
            {
               value->unsigned32 = entry->lifetime / 1000;
            }
         }
         //ipDefaultRouterPreference object?
         else if(osStrcmp(object->name, "ipDefaultRouterPreference") == 0)
         {
            //Get object value
            if(entry->preference == NDP_ROUTER_SEL_PREFERENCE_LOW)
            {
               value->integer = IP_MIB_ROUTER_PREFERENCE_LOW;
            }
            else if(entry->preference == NDP_ROUTER_SEL_PREFERENCE_MEDIUM)
            {
               value->integer = IP_MIB_ROUTER_PREFERENCE_MEDIUM;
            }
            else if(entry->preference == NDP_ROUTER_SEL_PREFERENCE_HIGH)
            {
               value->integer = IP_MIB_ROUTER_PREFERENCE_HIGH;
            }
            else
            {
               value->integer = IP_MIB_ROUTER_PREFERENCE_RESERVED;
            }
         }
         //Unknown object?
         else
         {
            //The specified object does not exist
            error = ERROR_OBJECT_NOT_FOUND;
         }
      }
      else
      {
         //Report an error
         error = ERROR_INSTANCE_NOT_FOUND;
      }
   }
   else
#endif
   //Invalid IP address?
   {
      //Report an error
      error = ERROR_INSTANCE_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next ipDefaultRouterEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpDefaultRouterEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   uint_t i;
   size_t n;
   uint_t index;
   uint_t curIndex;
   bool_t acceptable;
   IpAddr ipAddr;
   IpAddr curIpAddr;
   NetContext *context;
   NetInterface *interface;

   //Initialize variables
   index = 0;
   ipAddr = IP_ADDR_UNSPECIFIED;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

#if (IPV4_SUPPORT == ENABLED)
   //Loop through network interfaces
   for(curIndex = 1; curIndex <= context->numInterfaces; curIndex++)
   {
      Ipv4AddrEntry *entry;

      //Point to the current interface
      interface = &context->interfaces[curIndex - 1];

      //Loop through the list of default gateways
      for(i = 0; i < IPV4_ADDR_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv4Context.addrList[i];

         //Check whether the gateway address is valid
         if(entry->state == IPV4_ADDR_STATE_VALID &&
            entry->defaultGateway != IPV4_UNSPECIFIED_ADDR)
         {
            //Get the IP address of the default gateway
            curIpAddr.length = sizeof(Ipv4Addr);
            curIpAddr.ipv4Addr = entry->defaultGateway;

            //Append the instance identifier to the OID prefix
            n = object->oidLen;

            //ipDefaultRouterAddress and ipDefaultRouterAddressType are
            //used as 1st and 2nd instance identifiers
            error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &curIpAddr);
            //Any error to report?
            if(error)
               return error;

            //ipDefaultRouterIfIndex is used as 3rd instance identifier
            error = mibEncodeIndex(nextOid, *nextOidLen, &n, curIndex);
            //Any error to report?
            if(error)
               return error;

            //Check whether the resulting object identifier lexicographically
            //follows the specified OID
            if(oidComp(nextOid, n, oid, oidLen) > 0)
            {
               //Perform lexicographic comparison
               if(index == 0)
               {
                  acceptable = TRUE;
               }
               else if(mibCompIpAddr(&curIpAddr, &ipAddr) < 0)
               {
                  acceptable = TRUE;
               }
               else if(mibCompIpAddr(&curIpAddr, &ipAddr) > 0)
               {
                  acceptable = FALSE;
               }
               else if(curIndex < index)
               {
                  acceptable = TRUE;
               }
               else
               {
                  acceptable = FALSE;
               }

               //Save the closest object identifier that follows the specified
               //OID in lexicographic order
               if(acceptable)
               {
                  index = curIndex;
                  ipAddr = curIpAddr;
               }
            }
         }
      }
   }
#endif

#if (IPV6_SUPPORT == ENABLED)
   //Loop through network interfaces
   for(curIndex = 1; curIndex <= context->numInterfaces; curIndex++)
   {
      Ipv6RouterEntry *entry;

      //Point to the current interface
      interface = &context->interfaces[curIndex - 1];

      //Loop through the Default Router List
      for(i = 0; i < IPV6_ROUTER_LIST_SIZE; i++)
      {
         //Point to the current entry
         entry = &interface->ipv6Context.routerList[i];

         //Check the lifetime associated with the default router
         if(entry->lifetime)
         {
            //Get the IP address of the default gateway
            curIpAddr.length = sizeof(Ipv6Addr);
            curIpAddr.ipv6Addr = entry->addr;

            //Append the instance identifier to the OID prefix
            n = object->oidLen;

            //ipDefaultRouterAddress and ipDefaultRouterAddressType are
            //used as 1st and 2nd instance identifiers
            error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &curIpAddr);
            //Any error to report?
            if(error)
               return error;

            //ipDefaultRouterIfIndex is used as 3rd instance identifier
            error = mibEncodeIndex(nextOid, *nextOidLen, &n, curIndex);
            //Any error to report?
            if(error)
               return error;

            //Check whether the resulting object identifier lexicographically
            //follows the specified OID
            if(oidComp(nextOid, n, oid, oidLen) > 0)
            {
               //Perform lexicographic comparison
               if(index == 0)
               {
                  acceptable = TRUE;
               }
               else if(mibCompIpAddr(&curIpAddr, &ipAddr) < 0)
               {
                  acceptable = TRUE;
               }
               else if(mibCompIpAddr(&curIpAddr, &ipAddr) > 0)
               {
                  acceptable = FALSE;
               }
               else if(curIndex < index)
               {
                  acceptable = TRUE;
               }
               else
               {
                  acceptable = FALSE;
               }

               //Save the closest object identifier that follows the specified
               //OID in lexicographic order
               if(acceptable)
               {
                  index = curIndex;
                  ipAddr = curIpAddr;
               }
            }
         }
      }
   }
#endif

   //The specified OID does not lexicographically precede the name
   //of some object?
   if(index == 0)
      return ERROR_OBJECT_NOT_FOUND;

   //Append the instance identifier to the OID prefix
   n = object->oidLen;

   //ipDefaultRouterAddress and ipDefaultRouterAddressType are
   //used as 1st and 2nd instance identifiers
   error = mibEncodeIpAddr(nextOid, *nextOidLen, &n, &ipAddr);
   //Any error to report?
   if(error)
      return error;

   //ipDefaultRouterIfIndex is used as 3rd instance identifier
   error = mibEncodeIndex(nextOid, *nextOidLen, &n, index);
   //Any error to report?
   if(error)
      return error;

   //Save the length of the resulting object identifier
   *nextOidLen = n;
   //Next object found
   return NO_ERROR;
}


/**
 * @brief Set ipv6RouterAdvertSpinLock object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in] value Object value
 * @param[in] valueLen Length of the object value, in bytes
 * @param[in] commit This flag tells whether the changes shall be committed
 *   to the MIB base
 * @return Error code
 **/

error_t ipMibSetIpv6RouterAdvertSpinLock(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen, bool_t commit)
{
#if (IPV6_SUPPORT == ENABLED)
   //Test and increment spin lock
   return mibTestAndIncSpinLock(&ipMibBase.ipv6RouterAdvertSpinLock,
      value->integer, commit);
#else
   //Not implemented
   return ERROR_WRITE_FAILED;
#endif
}


/**
 * @brief Get ipv6RouterAdvertSpinLock object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpv6RouterAdvertSpinLock(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
#if (IPV6_SUPPORT == ENABLED)
   //Get the current value of the spin lock
   value->integer = ipMibBase.ipv6RouterAdvertSpinLock;

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_OBJECT_NOT_FOUND;
#endif
}


/**
 * @brief Set ipv6RouterAdvertEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in] value Object value
 * @param[in] valueLen Length of the object value, in bytes
 * @param[in] commit This flag tells whether the changes shall be committed
 *   to the MIB base
 * @return Error code
 **/

error_t ipMibSetIpv6RouterAdvertEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen, bool_t commit)
{
   //Not implemented
   return ERROR_WRITE_FAILED;
}


/**
 * @brief Get ipv6RouterAdvertEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIpv6RouterAdvertEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;

#if (IPV6_SUPPORT == ENABLED && NDP_ROUTER_ADV_SUPPORT == ENABLED)
   size_t n;
   uint_t index;
   NetContext *context;
   NdpRouterAdvContext *routerAdvContext;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //ipv6RouterAdvertIfIndex is used as instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &index);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check index range
   if(index < 1 || index > context->numInterfaces)
      return ERROR_INSTANCE_NOT_FOUND;

   //Point to the RA service context
   routerAdvContext = context->interfaces[index - 1].ndpRouterAdvContext;

   //Any RA service instantiated?
   if(routerAdvContext != NULL)
   {
      //ipv6RouterAdvertSendAdverts object?
      if(osStrcmp(object->name, "ipv6RouterAdvertSendAdverts") == 0)
      {
         //This flag indicates whether the router sends periodic router
         //advertisements and responds to router solicitations on this
         //interface
         if(routerAdvContext->running)
         {
            value->integer = MIB_TRUTH_VALUE_TRUE;
         }
         else
         {
            value->integer = MIB_TRUTH_VALUE_FALSE;
         }
      }
      //ipv6RouterAdvertMaxInterval object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertMaxInterval") == 0)
      {
         //Maximum time allowed between sending unsolicited router
         //advertisements from this interface
         value->unsigned32 = routerAdvContext->maxRtrAdvInterval;
      }
      //ipv6RouterAdvertMinInterval object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertMinInterval") == 0)
      {
         //Minimum time allowed between sending unsolicited router
         //advertisements from this interface
         value->unsigned32 = routerAdvContext->minRtrAdvInterval;
      }
      //ipv6RouterAdvertManagedFlag object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertManagedFlag") == 0)
      {
         //Value to be placed into the Managed Address Configuration flag
         //field in router advertisements sent from this interface
         if(routerAdvContext->managedFlag)
         {
            value->integer = MIB_TRUTH_VALUE_TRUE;
         }
         else
         {
            value->integer = MIB_TRUTH_VALUE_FALSE;
         }
      }
      //ipv6RouterAdvertOtherConfigFlag object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertOtherConfigFlag") == 0)
      {
         //Value to be placed into the Other Configuration flag field in
         //router advertisements sent from this interface
         if(routerAdvContext->otherConfigFlag)
         {
            value->integer = MIB_TRUTH_VALUE_TRUE;
         }
         else
         {
            value->integer = MIB_TRUTH_VALUE_FALSE;
         }
      }
      //ipv6RouterAdvertLinkMTU object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertLinkMTU") == 0)
      {
         //Value to be placed in the MTU option sent by the router on this
         //interface
         value->unsigned32 = routerAdvContext->linkMtu;
      }
      //ipv6RouterAdvertReachableTime object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertReachableTime") == 0)
      {
         //Value to be placed in the Reachable Time field in router
         //advertisement messages sent from this interface
         value->unsigned32 = routerAdvContext->reachableTime;
      }
      //ipv6RouterAdvertRetransmitTime object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertRetransmitTime") == 0)
      {
         //Value to be placed in the Retrans Timer field in router
         //advertisements sent from this interface
         value->unsigned32 = routerAdvContext->retransTimer;
      }
      //ipv6RouterAdvertCurHopLimit object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertCurHopLimit") == 0)
      {
         //Value to be placed in the Cur Hop Limit field in router
         //advertisements sent from this interface
         value->unsigned32 = routerAdvContext->curHopLimit;
      }
      //ipv6RouterAdvertDefaultLifetime object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertDefaultLifetime") == 0)
      {
         //Value to be placed in the Router Lifetime field of router
         //advertisements sent from this interface
         value->unsigned32 = routerAdvContext->defaultLifetime;
      }
      //ipv6RouterAdvertRowStatus object?
      else if(osStrcmp(object->name, "ipv6RouterAdvertRowStatus") == 0)
      {
         //Status of this conceptual row
         value->integer = MIB_ROW_STATUS_ACTIVE;
      }
      //Unknown object?
      else
      {
         //The specified object does not exist
         error = ERROR_OBJECT_NOT_FOUND;
      }
   }
   else
   {
      //Report an error
      error = ERROR_INSTANCE_NOT_FOUND;
   }
#else
   //Not implemented
   error = ERROR_OBJECT_NOT_FOUND;
#endif

   //Return status code
   return error;
}


/**
 * @brief Get next ipv6RouterAdvertEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIpv6RouterAdvertEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
#if (IPV6_SUPPORT == ENABLED && NDP_ROUTER_ADV_SUPPORT == ENABLED)
   error_t error;
   size_t n;
   uint_t index;
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

   //Loop through network interfaces
   for(index = 1; index <= context->numInterfaces; index++)
   {
      //Any RA service instantiated?
      if(context->interfaces[index - 1].ndpRouterAdvContext != NULL)
      {
         //Append the instance identifier to the OID prefix
         n = object->oidLen;

         //ipv6RouterAdvertIfIndex is used as instance identifier
         error = mibEncodeIndex(nextOid, *nextOidLen, &n, index);
         //Any error to report?
         if(error)
            return error;

         //Check whether the resulting object identifier lexicographically
         //follows the specified OID
         if(oidComp(nextOid, n, oid, oidLen) > 0)
         {
            //Save the length of the resulting object identifier
            *nextOidLen = n;
            //Next object found
            return NO_ERROR;
         }
      }
   }
#endif

   //The specified OID does not lexicographically precede the name
   //of some object
   return ERROR_OBJECT_NOT_FOUND;
}


/**
 * @brief Get icmpStatsEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIcmpStatsEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   size_t n;
   uint_t version;
   NetContext *context;
   IcmpStats *icmpStats;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //icmpStatsIPVersion is used as instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &version);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

#if (IPV4_SUPPORT == ENABLED && ICMP_STATS_SUPPORT == ENABLED)
   //IPv4 version?
   if(version == INET_VERSION_IPV4)
   {
      //Point to the ICMP statistics
      icmpStats = &context->icmpStats;
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED && ICMPV6_STATS_SUPPORT == ENABLED)
   //IPv6 version?
   if(version == INET_VERSION_IPV6)
   {
      //Point to the ICMPv6 statistics
      icmpStats = &context->icmpv6Stats;
   }
   else
#endif
   //Invalid IP version?
   {
      //No statistics available
      icmpStats = NULL;
   }

   //Sanity check
   if(icmpStats != NULL)
   {
      //icmpStatsInMsgs object?
      if(osStrcmp(object->name, "icmpStatsInMsgs") == 0)
      {
         value->counter32 = icmpStats->inMsgs;
      }
      //icmpStatsInErrors object?
      else if(osStrcmp(object->name, "icmpStatsInErrors") == 0)
      {
         value->counter32 = icmpStats->inErrors;
      }
      //icmpStatsOutMsgs object?
      else if(osStrcmp(object->name, "icmpStatsOutMsgs") == 0)
      {
         value->counter32 = icmpStats->outMsgs;
      }
      //icmpStatsOutErrors object?
      else if(osStrcmp(object->name, "icmpStatsOutErrors") == 0)
      {
         value->counter32 = icmpStats->outErrors;
      }
      //Unknown object?
      else
      {
         error = ERROR_OBJECT_NOT_FOUND;
      }
   }
   else
   {
      //Report an error
      error = ERROR_INSTANCE_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next icmpStatsEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIcmpStatsEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   size_t n;
   uint_t version;

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

   //IP version-neutral table
   for(version = INET_VERSION_IPV4; version <= INET_VERSION_IPV6; version++)
   {
#if (IPV4_SUPPORT == DISABLED || ICMP_STATS_SUPPORT == DISABLED)
      //IPv4 version?
      if(version == INET_VERSION_IPV4)
      {
         //IPv4 is not implemented
         continue;
      }
#endif
#if (IPV6_SUPPORT == DISABLED || ICMPV6_STATS_SUPPORT == DISABLED)
      //IPv6 version?
      if(version == INET_VERSION_IPV6)
      {
         //IPv6 is not implemented
         continue;
      }
#endif

      //Append the instance identifier to the OID prefix
      n = object->oidLen;

      //ipSystemStatsIPVersion is used as instance identifier
      error = mibEncodeIndex(nextOid, *nextOidLen, &n, version);
      //Any error to report?
      if(error)
         return error;

      //Check whether the resulting object identifier lexicographically
      //follows the specified OID
      if(oidComp(nextOid, n, oid, oidLen) > 0)
      {
         //Save the length of the resulting object identifier
         *nextOidLen = n;
         //Next object found
         return NO_ERROR;
      }
   }

   //The specified OID does not lexicographically precede the name
   //of some object
   return ERROR_OBJECT_NOT_FOUND;
}


/**
 * @brief Get icmpMsgStatsEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t ipMibGetIcmpMsgStatsEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   size_t n;
   uint_t version;
   uint_t type;
   NetContext *context;
   IcmpStats *icmpStats;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Point to the instance identifier
   n = object->oidLen;

   //icmpMsgStatsIPVersion is used as 1st instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &version);
   //Invalid instance identifier?
   if(error)
      return error;

   //icmpMsgStatsType is used as 2nd instance identifier
   error = mibDecodeIndex(oid, oidLen, &n, &type);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check icmpMsgStatsType value
   if(type >= 256)
      return ERROR_INSTANCE_NOT_FOUND;

#if (IPV4_SUPPORT == ENABLED && ICMP_STATS_SUPPORT == ENABLED)
   //IPv4 version?
   if(version == INET_VERSION_IPV4)
   {
      //Point to the ICMP statistics
      icmpStats = &context->icmpStats;
   }
   else
#endif
#if (IPV6_SUPPORT == ENABLED && ICMPV6_STATS_SUPPORT == ENABLED)
   //IPv6 version?
   if(version == INET_VERSION_IPV6)
   {
      //Point to the ICMPv6 statistics
      icmpStats = &context->icmpv6Stats;
   }
   else
#endif
   //Invalid IP version?
   {
      //No statistics available
      icmpStats = NULL;
   }

   //Sanity check
   if(icmpStats != NULL)
   {
      //icmpMsgStatsInPkts object?
      if(osStrcmp(object->name, "icmpMsgStatsInPkts") == 0)
      {
         value->counter32 = icmpStats->inPkts[type];
      }
      //icmpMsgStatsOutPkts object?
      else if(osStrcmp(object->name, "icmpMsgStatsOutPkts") == 0)
      {
         value->counter32 = icmpStats->outPkts[type];
      }
      //Unknown object?
      else
      {
         error = ERROR_OBJECT_NOT_FOUND;
      }
   }
   else
   {
      //Report an error
      error = ERROR_INSTANCE_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get next icmpMsgStatsEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t ipMibGetNextIcmpMsgStatsEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   size_t n;
   uint_t version;
   uint_t type;
   NetContext *context;
   IcmpStats *icmpStats;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   osMemcpy(nextOid, object->oid, object->oidLen);

   //IP version-neutral table
   for(version = INET_VERSION_IPV4; version <= INET_VERSION_IPV6; version++)
   {
#if (IPV4_SUPPORT == ENABLED && ICMP_STATS_SUPPORT == ENABLED)
      //IPv4 version?
      if(version == INET_VERSION_IPV4)
      {
         //Point to the ICMP statistics
         icmpStats = &context->icmpStats;
      }
      else
#endif
#if (IPV6_SUPPORT == ENABLED && ICMPV6_STATS_SUPPORT == ENABLED)
      //IPv6 version?
      if(version == INET_VERSION_IPV6)
      {
         //Point to the ICMPv6 statistics
         icmpStats = &context->icmpv6Stats;
      }
      else
#endif
      //Invalid IP version?
      {
         //No statistics available
         icmpStats = NULL;
      }

      //Sanity check
      if(icmpStats != NULL)
      {
         //The system should track each ICMP type value
         for(type = 0; type < 256; type++)
         {
            //a given row need not be instantiated unless an ICMP message of
            //that type has been processed
            if(icmpStats->inPkts[type] != 0 || icmpStats->outPkts[type] != 0)
            {
               //Append the instance identifier to the OID prefix
               n = object->oidLen;

               //icmpMsgStatsIPVersion is used as 1st instance identifier
               error = mibEncodeIndex(nextOid, *nextOidLen, &n, version);
               //Any error to report?
               if(error)
                  return error;

               //icmpMsgStatsType is used as 2nd instance identifier
               error = mibEncodeIndex(nextOid, *nextOidLen, &n, type);
               //Any error to report?
               if(error)
                  return error;

               //Check whether the resulting object identifier lexicographically
               //follows the specified OID
               if(oidComp(nextOid, n, oid, oidLen) > 0)
               {
                  //Save the length of the resulting object identifier
                  *nextOidLen = n;
                  //Next object found
                  return NO_ERROR;
               }
            }
         }
      }
   }

   //The specified OID does not lexicographically precede the name
   //of some object
   return ERROR_OBJECT_NOT_FOUND;
}

#endif
