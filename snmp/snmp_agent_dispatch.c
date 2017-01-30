/**
 * @file snmp_agent_dispatch.c
 * @brief SNMP message dispatching
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

//Switch to the appropriate trace level
#define TRACE_LEVEL SNMP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "snmp/snmp_agent.h"
#include "snmp/snmp_agent_dispatch.h"
#include "snmp/snmp_agent_pdu.h"
#include "snmp/snmp_agent_misc.h"
#include "mibs/mib2_module.h"
#include "crypto.h"
#include "asn1.h"
#include "oid.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SNMP_AGENT_SUPPORT == ENABLED)


/**
 * @brief Process incoming SNMP message
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpProcessMessage(SnmpAgentContext *context)
{
   error_t error;

   //Total number of messages delivered to the SNMP entity from the
   //transport service
   MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInPkts, 1);

   //Refresh SNMP engine time
   snmpRefreshEngineTime(context);

   //Message parsing initialization
   snmpInitMessage(&context->request);

   //Parse SNMP message header
   error = snmpParseMessageHeader(&context->request);
   //Any error to report?
   if(error)
      return error;

   //The SNMP agent verifies the version number. If there is a mismatch,
   //it discards the datagram and performs no further actions
   if(context->request.version < context->settings.versionMin ||
      context->request.version > context->settings.versionMax)
   {
      //Debug message
      TRACE_WARNING("  Invalid SNMP version!\r\n");
      //Discard incoming SNMP message
      return ERROR_INVALID_VERSION;
   }

#if (SNMP_V1_SUPPORT == ENABLED)
   //SNMPv1 version?
   if(context->request.version == SNMP_VERSION_1)
   {
      //Process incoming SNMPv1 message
      error = snmpv1ProcessMessage(context);
   }
   else
#endif
#if (SNMP_V2C_SUPPORT == ENABLED)
   //SNMPv2c version?
   if(context->request.version == SNMP_VERSION_2C)
   {
      //Process incoming SNMPv2c message
      error = snmpv2cProcessMessage(context);
   }
   else
#endif
#if (SNMP_V3_SUPPORT == ENABLED)
   //SNMPv3 version?
   if(context->request.version == SNMP_VERSION_3)
   {
      //Process incoming SNMPv3 message
      error = snmpv3ProcessMessage(context);
   }
   else
#endif
   //Invalid SNMP version?
   {
      //Debug message
      TRACE_WARNING("  Invalid SNMP version!\r\n");

      //Total number of SNMP messages which were delivered to the SNMP
      //protocol entity and were for an unsupported SNMP version
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInBadVersions, 1);

      //Discard incoming SNMP message
      error = ERROR_INVALID_VERSION;
   }

   //Check status code
   if(error == NO_ERROR)
   {
      //Total number of messages which were passed from the SNMP protocol
      //entity to the transport service
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpOutPkts, 1);
   }
   else if(error == ERROR_INVALID_TAG)
   {
      //Total number of ASN.1 or BER errors encountered by the SNMP protocol
      //entity when decoding received SNMP messages
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInASNParseErrs, 1);
   }

   //Return status code
   return error;
}


/**
 * @brief Process incoming SNMPv1 message
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpv1ProcessMessage(SnmpAgentContext *context)
{
   error_t error;

#if (SNMP_V1_SUPPORT == ENABLED)
   //Parse community name
   error = snmpParseCommunity(&context->request);
   //Any error to report?
   if(error)
      return error;

   //Information about the community name is extracted from the local
   //configuration datastore
   context->user = snmpFindUser(context, context->request.community,
      context->request.communityLen);

   //Invalid community name?
   if(context->user == NULL)
   {
      //Debug message
      TRACE_WARNING("  Invalid community name!\r\n");

      //Total number of SNMP messages delivered to the SNMP protocol entity
      //which used a SNMP community name not known to said entity
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInBadCommunityNames, 1);

      //Report an error
      return ERROR_UNKNOWN_USER_NAME;
   }

   //Process PDU
   error = snmpProcessPdu(context);
   //Any error to report?
   if(error)
      return error;

   //Format SNMP message header
   error = snmpWriteMessageHeader(&context->response);
#else
   //Report an error
   error = ERROR_INVALID_VERSION;
#endif

   //Return status code
   return error;
}


/**
 * @brief Process incoming SNMPv2c message
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpv2cProcessMessage(SnmpAgentContext *context)
{
   error_t error;

#if (SNMP_V2C_SUPPORT == ENABLED)
   //Parse community name
   error = snmpParseCommunity(&context->request);
   //Any error to report?
   if(error)
      return error;

   //Information about the community name is extracted from the local
   //configuration datastore
   context->user = snmpFindUser(context, context->request.community,
      context->request.communityLen);

   //Invalid community name?
   if(context->user == NULL)
   {
      //Debug message
      TRACE_WARNING("  Invalid community name!\r\n");

      //Total number of SNMP messages delivered to the SNMP protocol entity
      //which used a SNMP community name not known to said entity
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInBadCommunityNames, 1);

      //Report an error
      return ERROR_UNKNOWN_USER_NAME;
   }

   //Process PDU
   error = snmpProcessPdu(context);
   //Any error to report?
   if(error)
      return error;

   //Format SNMP message header
   error = snmpWriteMessageHeader(&context->response);
#else
   //Report an error
   error = ERROR_INVALID_VERSION;
#endif

   //Return status code
   return error;
}


/**
 * @brief Process incoming SNMPv3 message
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpv3ProcessMessage(SnmpAgentContext *context)
{
   error_t error;

#if (SNMP_V3_SUPPORT == ENABLED)
   //Parse msgGlobalData field
   error = snmpParseGlobalData(&context->request);
   //Any error to report?
   if(error)
      return error;

   //Parse msgSecurityParameters field
   error = snmpParseSecurityParameters(&context->request);
   //Any error to report?
   if(error)
      return error;

   //Start of exception handling block
   do
   {
      //Information about the value of the msgUserName field is extracted
      //from the local configuration datastore
      context->user = snmpFindUser(context, context->request.msgUserName,
         context->request.msgUserNameLen);

      //Check security parameters
      error = snmpCheckSecurityParameters(context->user, &context->request,
         context->contextEngine, context->contextEngineLen);
      //Invalid security parameters?
      if(error)
         break;

      //Check whether the authFlag is set
      if(context->request.msgFlags & SNMP_MSG_FLAG_AUTH)
      {
         //Authenticate incoming SNMP message
         error = snmpAuthIncomingMessage(context->user, &context->request);
         //Data authentication failed?
         if(error)
            break;

         //Replay protection
         error = snmpCheckEngineTime(context, &context->request);
         //Message outside of the time window?
         if(error)
            break;
      }

      //Check whether the privFlag is set
      if(context->request.msgFlags & SNMP_MSG_FLAG_PRIV)
      {
         //Decrypt data
         error = snmpDecryptData(context->user, &context->request);
         //Data decryption failed?
         if(error)
            break;
      }

      //End of exception handling block
   } while(0);

   //Check error indication
   if(error == ERROR_UNSUPPORTED_SECURITY_LEVEL ||
      error == ERROR_NOT_IN_TIME_WINDOW ||
      error == ERROR_UNKNOWN_USER_NAME ||
      error == ERROR_UNKNOWN_ENGINE_ID ||
      error == ERROR_AUTHENTICATION_FAILED ||
      error == ERROR_DECRYPTION_FAILED)
   {
      //Report the error indication to the remote SNMP engine using a Report-PDU
      error = snmpFormatReportPdu(context, error);
      //Any error to report?
      if(error)
         return error;
   }
   else if(error == NO_ERROR)
   {
      //Parse scopedPDU
      error = snmpParseScopedPdu(&context->request);
      //Any error to report?
      if(error)
         return error;

      //Process PDU
      error = snmpProcessPdu(context);
      //Any error to report?
      if(error)
         return error;
   }
   else
   {
      //Stop processing
      return error;
   }

   //Format scopedPDU
   error = snmpWriteScopedPdu(&context->response);
   //Any error to report?
   if(error)
      return error;

   //Check whether the privFlag is set
   if(context->response.msgFlags & SNMP_MSG_FLAG_PRIV)
   {
      //Encrypt data
      error = snmpEncryptData(context->user, &context->response, &context->salt);
      //Any error to report?
      if(error)
         return error;
   }

   //Format SNMP message header
   error = snmpWriteMessageHeader(&context->response);
   //Any error to report?
   if(error)
      return error;

   //Check whether the authFlag is set
   if(context->response.msgFlags & SNMP_MSG_FLAG_AUTH)
   {
      //Authenticate outgoing SNMP message
      error = snmpAuthOutgoingMessage(context->user, &context->response);
      //Any error to report?
      if(error)
         return error;
   }
#else
   //Report an error
   error = ERROR_INVALID_VERSION;
#endif

   //Return status code
   return error;
}

#endif
