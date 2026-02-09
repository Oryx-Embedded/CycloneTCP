/**
 * @file mib2_impl_icmp.c
 * @brief MIB-II module implementation (ICMP group)
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
#include "ipv4/icmp.h"
#include "mibs/mib_common.h"
#include "mibs/mib2_module.h"
#include "mibs/mib2_impl.h"
#include "mibs/mib2_impl_icmp.h"
#include "core/crypto.h"
#include "encoding/asn1.h"
#include "encoding/oid.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (MIB2_SUPPORT == ENABLED && MIB2_ICMP_GROUP_SUPPORT == ENABLED)


/**
 * @brief Get icmpInMsgs object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInMsgs(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Total number of ICMP messages which the entity received
   value->counter32 = context->icmpStats.inMsgs;

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInErrors object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInErrors(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP messages which the entity received but determined as having
   //ICMP-specific errors
   value->counter32 = context->icmpStats.inErrors;

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInDestUnreachs object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInDestUnreachs(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Destination Unreachable messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_DEST_UNREACHABLE];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInTimeExcds object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInTimeExcds(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Time Exceeded messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_TIME_EXCEEDED];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInParmProbs object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInParmProbs(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Parameter Problem messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_PARAM_PROBLEM];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInSrcQuenchs object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInSrcQuenchs(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Source Quench messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_SOURCE_QUENCH];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInRedirects object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInRedirects(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Redirect messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_REDIRECT];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInEchos object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInEchos(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Echo Request messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_ECHO_REQUEST];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInEchoReps object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInEchoReps(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Echo Reply messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_ECHO_REPLY];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInTimestamps object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInTimestamps(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Timestamp Request messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_TIMESTAMP_REQUEST];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInTimestampReps object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInTimestampReps(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Timestamp Reply messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_TIMESTAMP_REPLY];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInAddrMasks object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInAddrMasks(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Address Mask Request messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_ADDR_MASK_REQUEST];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpInAddrMaskReps object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpInAddrMaskReps(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Address Mask Reply messages received
   value->counter32 = context->icmpStats.inPkts[ICMP_TYPE_ADDR_MASK_REPLY];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutMsgs object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutMsgs(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Total number of ICMP messages which this entity attempted to send
   value->counter32 = context->icmpStats.outMsgs;

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutErrors object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutErrors(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP messages which this entity did not send due to problems
   //discovered within ICMP such as a lack of buffers
   value->counter32 = context->icmpStats.outErrors;

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutDestUnreachs object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutDestUnreachs(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Destination Unreachable messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_DEST_UNREACHABLE];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutTimeExcds object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutTimeExcds(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Time Exceeded messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_TIME_EXCEEDED];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutParmProbs object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutParmProbs(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Parameter Problem messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_PARAM_PROBLEM];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutSrcQuenchs object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutSrcQuenchs(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Source Quench messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_SOURCE_QUENCH];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutRedirects object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutRedirects(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Redirect messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_REDIRECT];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutEchos object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutEchos(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Echo Request messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_ECHO_REQUEST];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutEchoReps object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutEchoReps(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Echo Reply messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_ECHO_REPLY];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutTimestamps object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutTimestamps(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Timestamp Request messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_TIMESTAMP_REQUEST];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutTimestampReps object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutTimestampReps(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Timestamp Reply messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_TIMESTAMP_REPLY];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutAddrMasks object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutAddrMasks(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Address Mask Request messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_ADDR_MASK_REQUEST];

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Get icmpOutAddrMaskReps object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t mib2GetIcmpOutAddrMaskReps(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   NetContext *context;

   //Point to the TCP/IP stack context
   context = netGetDefaultContext();

   //Number of ICMP Address Mask Reply messages sent
   value->counter32 = context->icmpStats.outPkts[ICMP_TYPE_ADDR_MASK_REPLY];

   //Return status code
   return NO_ERROR;
}

#endif
