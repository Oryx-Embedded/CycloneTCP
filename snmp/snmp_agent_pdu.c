/**
 * @file snmp_agent_pdu.c
 * @brief SNMP agent (PDU processing)
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
#include "snmp/snmp_agent_pdu.h"
#include "snmp/snmp_agent_misc.h"
#include "mibs/mib2_module.h"
#include "crypto.h"
#include "asn1.h"
#include "oid.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SNMP_AGENT_SUPPORT == ENABLED)

//sysUpTime.0 object (1.3.6.1.2.1.1.3.0)
static const uint8_t sysUpTimeObject[] = {43, 6, 1, 2, 1, 1, 3, 0};
//snmpTrapOID.0 object (1.3.6.1.6.3.1.1.4.1.0)
static const uint8_t snmpTrapOidObject[] = {43, 6, 1, 6, 3, 1, 1, 4, 1, 0};
//snmpTraps object (1.3.6.1.6.3.1.1.5)
static const uint8_t snmpTrapsObject[] = {43, 6, 1, 6, 3, 1, 1, 5};


/**
 * @brief Process PDU
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpProcessPdu(SnmpAgentContext *context)
{
   error_t error;

   //Parse PDU header
   error = snmpParsePduHeader(&context->request);
   //Any error to report?
   if(error)
      return error;

   //Check PDU type
   switch(context->request.pduType)
   {
   case SNMP_PDU_GET_REQUEST:
   case SNMP_PDU_GET_NEXT_REQUEST:
      //Process GetRequest-PDU or GetNextRequest-PDU
      error = snmpProcessGetRequestPdu(context);
      break;
   case SNMP_PDU_GET_BULK_REQUEST:
      //Process GetBulkRequest-PDU
      error = snmpProcessGetBulkRequestPdu(context);
      break;
   case SNMP_PDU_SET_REQUEST:
      //Process SetRequest-PDU
      error = snmpProcessSetRequestPdu(context);
      break;
   default:
      //Invalid PDU type
      error = ERROR_INVALID_TYPE;
      break;
   }

   //Check status code
   if(!error)
   {
      //Total number of SNMP Get-Response PDUs which have been generated
      //by the SNMP protocol entity
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpOutGetResponses, 1);

      //Format PDU header
      error = snmpWritePduHeader(&context->response);
   }

   //Return status code
   return error;
}


/**
 * @brief Process GetRequest-PDU or GetNextRequest-PDU
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpProcessGetRequestPdu(SnmpAgentContext *context)
{
   error_t error;
   int_t index;
   size_t n;
   size_t length;
   const uint8_t *p;
   SnmpVarBind var;

   //Check PDU type
   if(context->request.pduType == SNMP_PDU_GET_REQUEST)
   {
      //Debug message
      TRACE_INFO("Parsing GetRequest-PDU...\r\n");

      //Total number of SNMP Get-Request PDUs which have been accepted and
      //processed by the SNMP protocol entity
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInGetRequests, 1);
   }
   else if(context->request.pduType == SNMP_PDU_GET_NEXT_REQUEST)
   {
      //Debug message
      TRACE_INFO("Parsing GetNextRequest-PDU...\r\n");

      //Total number of SNMP Get-NextRequest PDUs which have been accepted
      //and processed by the SNMP protocol entity
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInGetNexts, 1);
   }

   //Enforce access policy
   if(context->user->mode != SNMP_ACCESS_READ_ONLY &&
      context->user->mode != SNMP_ACCESS_READ_WRITE)
   {
      //Total number of SNMP messages delivered to the SNMP protocol entity
      //which represented an SNMP operation which was not allowed by the SNMP
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInBadCommunityUses, 1);

      //Report an error
      return ERROR_ACCESS_DENIED;
   }

   //Initialize response message
   error = snmpInitResponse(context);
   //Any error to report?
   if(error)
      return error;

   //Point to the first variable binding of the request
   p = context->request.varBindList;
   length = context->request.varBindListLen;

   //Lock access to MIB bases
   snmpLockMib(context);

   //Loop through the list
   for(index = 1; length > 0; index++)
   {
      //Parse variable binding
      error = snmpParseVarBinding(p, length, &var, &n);
      //Failed to parse variable binding?
      if(error)
         break;

      //Make sure that the object identifier is valid
      error = oidCheck(var.oid, var.oidLen);
      //Invalid object identifier?
      if(error)
         break;

      //GetRequest-PDU?
      if(context->request.pduType == SNMP_PDU_GET_REQUEST)
      {
         //Retrieve object value
         error = snmpGetObjectValue(context, &var);
      }
      //GetNextRequest-PDU?
      else
      {
         //Search the MIB for the next object
         error = snmpGetNextObject(context, &var);

         //SNMPv1 version?
         if(context->request.version == SNMP_VERSION_1)
         {
            //Check status code
            if(error == NO_ERROR)
            {
               //Retrieve object value
               error = snmpGetObjectValue(context, &var);
            }
            else
            {
               //Stop immediately
               break;
            }
         }
         //SNMPv2c or SNMPv3 version?
         else
         {
            //Check status code
            if(error == NO_ERROR)
            {
               //Retrieve object value
               error = snmpGetObjectValue(context, &var);
            }
            else if(error == ERROR_OBJECT_NOT_FOUND)
            {
               //The variable binding's value field is set to endOfMibView
               var.objClass = ASN1_CLASS_CONTEXT_SPECIFIC;
               var.objType = SNMP_EXCEPTION_END_OF_MIB_VIEW;
               var.valueLen = 0;

               //Catch exception
               error = NO_ERROR;
            }
            else
            {
               //Stop immediately
               break;
            }
         }
      }

      //Failed to retrieve object value?
      if(error)
      {
         //SNMPv1 version?
         if(context->request.version == SNMP_VERSION_1)
         {
            //Stop immediately
            break;
         }
         //SNMPv2c or SNMPv3 version?
         else
         {
            //Catch exception
            if(error == ERROR_ACCESS_DENIED ||
               error == ERROR_OBJECT_NOT_FOUND)
            {
               //The variable binding's value field is set to noSuchObject
               var.objClass = ASN1_CLASS_CONTEXT_SPECIFIC;
               var.objType = SNMP_EXCEPTION_NO_SUCH_OBJECT;
               var.valueLen = 0;
            }
            else if(error == ERROR_INSTANCE_NOT_FOUND)
            {
               //The variable binding's value field is set to noSuchInstance
               var.objClass = ASN1_CLASS_CONTEXT_SPECIFIC;
               var.objType = SNMP_EXCEPTION_NO_SUCH_INSTANCE;
               var.valueLen = 0;
            }
            else
            {
               //Stop immediately
               break;
            }
         }
      }
      else
      {
         //Total number of MIB objects which have been retrieved successfully
         //by the SNMP protocol entity as the result of receiving valid SNMP
         //Get-Request and Get-NextRequest PDUs
         MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInTotalReqVars, 1);
      }

      //Append variable binding to the list
      error = snmpWriteVarBinding(context, &var);
      //Any error to report?
      if(error)
         break;

      //Advance data pointer
      p += n;
      length -= n;
   }

   //Unlock access to MIB bases
   snmpUnlockMib(context);

   //Check status code
   if(error)
   {
      //Set error-status and error-index fields
      error = snmpTranslateStatusCode(&context->response, error, index);
      //If the parsing of the request fails, the SNMP agent discards the message
      if(error)
         return error;

      //Check whether an alternate Response-PDU should be sent
      if(context->response.version != SNMP_VERSION_1 &&
         context->response.errorStatus == SNMP_ERROR_TOO_BIG)
      {
         //The alternate Response-PDU is formatted with the same value in its
         //request-id field as the received GetRequest-PDU and an empty
         //variable-bindings field
         context->response.varBindListLen = 0;
      }
      else
      {
         //The Response-PDU is re-formatted with the same values in its request-id
         //and variable-bindings fields as the received GetRequest-PDU
         error = snmpCopyVarBindingList(context);
         //Any error to report?
         if(error)
            return error;
      }
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Process GetBulkRequest-PDU
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpProcessGetBulkRequestPdu(SnmpAgentContext *context)
{
#if (SNMP_V2C_SUPPORT == ENABLED || SNMP_V3_SUPPORT == ENABLED)
   error_t error;
   int_t index;
   size_t n;
   size_t m;
   size_t length;
   bool_t endOfMibView;
   const uint8_t *p;
   const uint8_t *next;
   SnmpVarBind var;

   //Debug message
   TRACE_INFO("Parsing GetBulkRequest-PDU...\r\n");

   //Make sure the SNMP version identifier is valid
   if(context->request.version == SNMP_VERSION_1)
   {
      //The SNMP version is not acceptable
      return ERROR_INVALID_TYPE;
   }

   //Enforce access policy
   if(context->user->mode != SNMP_ACCESS_READ_ONLY &&
      context->user->mode != SNMP_ACCESS_READ_WRITE)
   {
      //Total number of SNMP messages delivered to the SNMP protocol entity
      //which represented an SNMP operation which was not allowed by the SNMP
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInBadCommunityUses, 1);

      //Report an error
      return ERROR_ACCESS_DENIED;
   }

   //Initialize response message
   error = snmpInitResponse(context);
   //Any error to report?
   if(error)
      return error;

   //Point to the first variable binding of the request
   p = context->request.varBindList;
   length = context->request.varBindListLen;

   //Lock access to MIB bases
   snmpLockMib(context);

   //Loop through the list
   for(index = 1; length > 0; index++)
   {
      //The non-repeaters field specifies the number of non-repeating objects
      //at the start of the variable binding list
      if((index - 1) == context->request.nonRepeaters)
      {
         //Pointer to the first variable binding that will be processed during
         //the next iteration
         next = context->response.varBindList + context->response.varBindListLen;

         //Actual size of the variable binding list
         m = context->response.varBindListLen;

         //This flag tells whether all variable bindings have the value field
         //set to endOfMibView for a given iteration
         endOfMibView = TRUE;

         //If the max-repetitions field is zero, the list is trimmed to the
         //first non-repeating variable bindings
         if(context->request.maxRepetitions == 0)
            break;
      }

      //Parse variable binding
      error = snmpParseVarBinding(p, length, &var, &n);
      //Failed to parse variable binding?
      if(error)
         break;

      //Make sure that the object identifier is valid
      error = oidCheck(var.oid, var.oidLen);
      //Invalid object identifier?
      if(error)
         break;

      //Search the MIB for the next object
      error = snmpGetNextObject(context, &var);

      //Check status code
      if(error == NO_ERROR)
      {
         //Next object found
         endOfMibView = FALSE;
         //Retrieve object value
         error = snmpGetObjectValue(context, &var);
      }
      else if(error == ERROR_OBJECT_NOT_FOUND)
      {
         //The variable binding's value field is set to endOfMibView
         var.objClass = ASN1_CLASS_CONTEXT_SPECIFIC;
         var.objType = SNMP_EXCEPTION_END_OF_MIB_VIEW;
         var.valueLen = 0;

         //Catch exception
         error = NO_ERROR;
      }
      else
      {
         //Stop immediately
         break;
      }

      //Failed to retrieve object value?
      if(error)
      {
         //Catch exception
         if(error == ERROR_ACCESS_DENIED ||
            error == ERROR_OBJECT_NOT_FOUND)
         {
            //The variable binding's value field is set to noSuchObject
            var.objClass = ASN1_CLASS_CONTEXT_SPECIFIC;
            var.objType = SNMP_EXCEPTION_NO_SUCH_OBJECT;
            var.valueLen = 0;
         }
         else if(error == ERROR_INSTANCE_NOT_FOUND)
         {
            //The variable binding's value field is set to noSuchInstance
            var.objClass = ASN1_CLASS_CONTEXT_SPECIFIC;
            var.objType = SNMP_EXCEPTION_NO_SUCH_INSTANCE;
            var.valueLen = 0;
         }
         else
         {
            //Stop immediately
            break;
         }
      }
      else
      {
         //Total number of MIB objects which have been retrieved successfully
         //by the SNMP protocol entity as the result of receiving valid SNMP
         //Get-Request and Get-NextRequest PDUs
         MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInTotalReqVars, 1);
      }

      //Append variable binding to the list
      error = snmpWriteVarBinding(context, &var);
      //Any error to report?
      if(error)
         break;

      //Advance data pointer
      p += n;
      length -= n;

      //Next iteration?
      if(length == 0 && index > context->request.nonRepeaters)
      {
         //Decrement repeat counter
         context->request.maxRepetitions--;

         //Last iteration?
         if(!context->request.maxRepetitions)
            break;
         //All variable bindings have the value field set to endOfMibView?
         if(endOfMibView)
            break;

         //Point to the first variable binding to be processed
         p = next;
         //Number of bytes to be processed
         length = context->response.varBindListLen - m;
         //Rewind index
         index = context->request.nonRepeaters;
      }
   }

   //Unlock access to MIB bases
   snmpUnlockMib(context);

   //Check status code
   if(error == ERROR_BUFFER_OVERFLOW)
   {
      //If the size of the message containing the requested number of variable
      //bindings would be greater than the maximum message size, then the
      //response is generated with a lesser number of variable bindings
   }
   else if(error)
   {
      //Set error-status and error-index fields
      error = snmpTranslateStatusCode(&context->response, error, index);
      //If the parsing of the request fails, the SNMP agent discards the message
      if(error)
         return error;

      //The Response-PDU is re-formatted with the same values in its request-id
      //and variable-bindings fields as the received GetRequest-PDU
      error = snmpCopyVarBindingList(context);
      //Any error to report?
      if(error)
         return error;
   }

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Process SetRequest-PDU
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpProcessSetRequestPdu(SnmpAgentContext *context)
{
   error_t error;
   int_t index;
   size_t n;
   size_t length;
   const uint8_t *p;
   SnmpVarBind var;

   //Debug message
   TRACE_INFO("Parsing SetRequest-PDU...\r\n");

   //Total number of SNMP Set-Request PDUs which have been accepted and
   //processed by the SNMP protocol entity
   MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInSetRequests, 1);

   //Enforce access policy
   if(context->user->mode != SNMP_ACCESS_WRITE_ONLY &&
      context->user->mode != SNMP_ACCESS_READ_WRITE)
   {
      //Total number of SNMP messages delivered to the SNMP protocol entity
      //which represented an SNMP operation which was not allowed by the SNMP
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInBadCommunityUses, 1);

      //Report an error
      return ERROR_ACCESS_DENIED;
   }

   //Initialize response message
   error = snmpInitResponse(context);
   //Any error to report?
   if(error)
      return error;

   //The variable bindings are processed as a two phase operation. In the
   //first phase, each variable binding is validated
   p = context->request.varBindList;
   length = context->request.varBindListLen;

   //Loop through the list
   for(index = 1; length > 0; index++)
   {
      //Parse variable binding
      error = snmpParseVarBinding(p, length, &var, &n);
      //Failed to parse variable binding?
      if(error)
         break;

      //Assign object value
      error = snmpSetObjectValue(context, &var, FALSE);
      //Any error to report?
      if(error)
         break;

      //Advance data pointer
      p += n;
      length -= n;
   }

   //If all validations are successful, then each variable is altered in
   //the second phase
   if(!error)
   {
      //The changes are committed to the MIB base during the second phase
      p = context->request.varBindList;
      length = context->request.varBindListLen;

      //Lock access to MIB bases
      snmpLockMib(context);

      //Loop through the list
      for(index = 1; length > 0; index++)
      {
         //Parse variable binding
         error = snmpParseVarBinding(p, length, &var, &n);
         //Failed to parse variable binding?
         if(error)
            break;

         //Assign object value
         error = snmpSetObjectValue(context, &var, TRUE);
         //Any error to report?
         if(error)
            break;

         //Total number of MIB objects which have been altered successfully
         //by the SNMP protocol entity as the result of receiving valid
         //SNMP Set-Request PDUs
         MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpInTotalSetVars, 1);

         //Advance data pointer
         p += n;
         length -= n;
      }

      //Unlock access to MIB bases
      snmpUnlockMib(context);
   }

   //Any error to report?
   if(error)
   {
      //Set error-status and error-index fields
      error = snmpTranslateStatusCode(&context->response, error, index);
      //If the parsing of the request fails, the SNMP agent discards the message
      if(error)
         return error;
   }

   //The SNMP agent sends back a GetResponse-PDU of identical form
   error = snmpCopyVarBindingList(context);
   //Return status code
   return error;
}


/**
 * @brief Format Trap-PDU or SNMPv2-Trap-PDU
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] version SNMP version identifier
 * @param[in] username User name or community name
 * @param[in] genericTrapType Generic trap type
 * @param[in] specificTrapCode Specific code
 * @param[in] objectList List of object names
 * @param[in] objectListSize Number of entries in the list
 * @return Error code
 **/

error_t snmpFormatTrapPdu(SnmpAgentContext *context, SnmpVersion version,
   const char_t *username, uint_t genericTrapType, uint_t specificTrapCode,
   const SnmpTrapObject *objectList, uint_t objectListSize)
{
   error_t error;
   uint_t i;
   size_t n;
   systime_t time;
   SnmpMessage *message;
   SnmpVarBind var;

   //Point to the SNMP message
   message = &context->response;
   //Initialize SNMP message
   snmpInitMessage(message);

   //SNMP version identifier
   message->version = version;

#if (SNMP_V1_SUPPORT == ENABLED)
   //SNMPv1 version?
   if(version == SNMP_VERSION_1)
   {
#if (IPV4_SUPPORT == ENABLED)
      NetInterface *interface;

      //Point to the underlying network interface
      interface = context->settings.interface;
#endif

      //Community name
      message->community = username;
      message->communityLen = strlen(username);

      //Prepare to send a Trap-PDU
      message->pduType = SNMP_PDU_TRAP;
      //Type of object generating trap
      message->enterpriseOid = context->enterpriseOid;
      message->enterpriseOidLen = context->enterpriseOidLen;

#if (IPV4_SUPPORT == ENABLED)
      //Address of object generating trap
      if(interface != NULL)
         message->agentAddr = interface->ipv4Context.addr;
#endif

      //Generic trap type
      message->genericTrapType = genericTrapType;
      //Specific trap code
      message->specificTrapCode = specificTrapCode;
      //Timestamp
      message->timestamp = osGetSystemTime() / 10;
   }
   else
#endif
#if (SNMP_V2C_SUPPORT == ENABLED)
   //SNMPv2c version?
   if(version == SNMP_VERSION_2C)
   {
      //Community name
      message->community = username;
      message->communityLen = strlen(username);

      //Prepare to send a SNMPv2-Trap-PDU
      message->pduType = SNMP_PDU_TRAP_V2;
   }
   else
#endif
#if (SNMP_V3_SUPPORT == ENABLED)
   //SNMPv3 version?
   if(version == SNMP_VERSION_3)
   {
      //Maximum message size supported by the sender
      message->msgMaxSize = SNMP_MAX_MSG_SIZE;

      //Bit fields which control processing of the message
      if(context->user->authProtocol != SNMP_AUTH_PROTOCOL_NONE)
         message->msgFlags |= SNMP_MSG_FLAG_AUTH;
      if(context->user->privProtocol != SNMP_PRIV_PROTOCOL_NONE)
         message->msgFlags |= SNMP_MSG_FLAG_PRIV;

      //Security model used by the sender
      message->msgSecurityModel = SNMP_SECURITY_MODEL_USM;

      //Authoritative engine identifier
      message->msgAuthEngineId = context->contextEngine;
      message->msgAuthEngineIdLen = context->contextEngineLen;
      //Number of times the SNMP engine has rebooted
      message->msgAuthEngineBoots = context->engineBoots;
      //Number of seconds since last reboot
      message->msgAuthEngineTime = context->engineTime;
      //User name
      message->msgUserName = username;
      message->msgUserNameLen = strlen(username);
      //Authentication parameters
      message->msgAuthParameters = NULL;

      //Length of the authentication parameters
      if(context->user->authProtocol == SNMP_AUTH_PROTOCOL_MD5)
         message->msgAuthParametersLen = 12;
      else if(context->user->authProtocol == SNMP_AUTH_PROTOCOL_SHA1)
         message->msgAuthParametersLen = 12;
      else if(context->user->authProtocol == SNMP_AUTH_PROTOCOL_SHA224)
         message->msgAuthParametersLen = 16;
      else if(context->user->authProtocol == SNMP_AUTH_PROTOCOL_SHA256)
         message->msgAuthParametersLen = 24;
      else if(context->user->authProtocol == SNMP_AUTH_PROTOCOL_SHA384)
         message->msgAuthParametersLen = 32;
      else if(context->user->authProtocol == SNMP_AUTH_PROTOCOL_SHA512)
         message->msgAuthParametersLen = 48;
      else
         message->msgAuthParametersLen = 0;

      //Privacy parameters
      message->msgPrivParameters = context->privParameters;

      //Length of the privacy parameters
      if(context->user->privProtocol == SNMP_PRIV_PROTOCOL_DES)
         message->msgPrivParametersLen = 8;
      else if(context->user->privProtocol == SNMP_PRIV_PROTOCOL_AES)
         message->msgPrivParametersLen = 8;
      else
         message->msgPrivParametersLen = 0;

      //Context engine identifier
      message->contextEngineId = context->contextEngine;
      message->contextEngineIdLen = context->contextEngineLen;
      //Context name
      message->contextName = (uint8_t *) context->contextName;
      message->contextNameLen = strlen(context->contextName);

      //Prepare to send a SNMPv2-Trap-PDU
      message->pduType = SNMP_PDU_TRAP_V2;
   }
   else
#endif
   //Invalid SNMP version?
   {
      //Report an error
      return ERROR_INVALID_VERSION;
   }

   //Make room for the message header at the beginning of the buffer
   error = snmpComputeMessageOverhead(&context->response);
   //Any error to report?
   if(error)
      return error;

#if (SNMP_V2C_SUPPORT == ENABLED || SNMP_V3_SUPPORT == ENABLED)
   //SNMPv2c or SNMPv3 version?
   if(version == SNMP_VERSION_2C || version == SNMP_VERSION_3)
   {
      //Get current time
      time = osGetSystemTime() / 10;

      //Encode the object value using ASN.1 rules
      error = snmpEncodeUnsignedInt32(time, message->buffer, &n);
      //Any error to report?
      if(error)
         return error;

      //The first two variable bindings in the variable binding list of an
      //SNMPv2-Trap-PDU are sysUpTime.0 and snmpTrapOID.0 respectively
      var.oid = sysUpTimeObject;
      var.oidLen = sizeof(sysUpTimeObject);
      var.objClass = ASN1_CLASS_APPLICATION;
      var.objType = MIB_TYPE_TIME_TICKS;
      var.value = message->buffer;
      var.valueLen = n;

      //Append sysUpTime.0 to the variable binding list
      error = snmpWriteVarBinding(context, &var);
      //Any error to report?
      if(error)
         return error;

      //Generic or enterprise-specific trap?
      if(genericTrapType < SNMP_TRAP_ENTERPRISE_SPECIFIC)
      {
         //Retrieve the length of the snmpTraps OID
         n = sizeof(snmpTrapsObject);
         //Copy the OID
         memcpy(message->buffer, snmpTrapsObject, n);

         //For generic traps, the SNMPv2 snmpTrapOID parameter shall be
         //the corresponding trap as defined in section 2 of 3418
         message->buffer[n] = genericTrapType + 1;

         //Update the length of the snmpTrapOID parameter
         n++;
      }
      else
      {
         //Retrieve the length of the enterprise OID
         n = context->enterpriseOidLen;

         //For enterprise specific traps, the SNMPv2 snmpTrapOID parameter shall
         //be the concatenation of the SNMPv1 enterprise OID and two additional
         //sub-identifiers: '0' and the SNMPv1 specific trap parameter
         memcpy(message->buffer, context->enterpriseOid, n);

         //Concatenate the '0' sub-identifier
         message->buffer[n++] = 0;

         //Concatenate the specific trap parameter
         message->buffer[n] = specificTrapCode % 128;

         //Loop as long as necessary
         for(i = 1; specificTrapCode > 128; i++)
         {
            //Split the binary representation into 7 bit chunks
            specificTrapCode /= 128;
            //Make room for the new chunk
            memmove(message->buffer + n + 1, message->buffer + n, i);
            //Set the most significant bit in the current chunk
            message->buffer[n] = OID_MORE_FLAG | (specificTrapCode % 128);
         }

         //Update the length of the snmpTrapOID parameter
         n += i;
      }

      //The snmpTrapOID.0 variable occurs as the second variable
      //binding in every SNMPv2-Trap-PDU
      var.oid = snmpTrapOidObject;
      var.oidLen = sizeof(snmpTrapOidObject);
      var.objClass = ASN1_CLASS_UNIVERSAL;
      var.objType = ASN1_TYPE_OBJECT_IDENTIFIER;
      var.value = message->buffer;
      var.valueLen = n;

      //Append snmpTrapOID.0 to the variable binding list
      error = snmpWriteVarBinding(context, &var);
      //Any error to report?
      if(error)
         return error;
   }
#endif

   //Loop through the list of objects
   for(i = 0; i < objectListSize; i++)
   {
      //Get object identifier
      var.oid = objectList[i].oid;
      var.oidLen = objectList[i].oidLen;

      //Retrieve object value
      error = snmpGetObjectValue(context, &var);
      //Any error to report?
      if(error)
         return error;

      //Append variable binding to the list
      error = snmpWriteVarBinding(context, &var);
      //Any error to report?
      if(error)
         return error;
   }

   //Total number of SNMP Trap PDUs which have been generated by
   //the SNMP protocol entity
   MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpOutTraps, 1);

   //Format PDU header
   error = snmpWritePduHeader(&context->response);
   //Return status code
   return error;
}


/**
 * @brief Format Report-PDU
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] errorIndication Error indication
 * @return Error code
 **/

error_t snmpFormatReportPdu(SnmpAgentContext *context, error_t errorIndication)
{
   error_t error;

#if (SNMP_V3_SUPPORT == ENABLED)
   size_t n;
   SnmpVarBind var;

   //Initialize SNMP message
   snmpInitMessage(&context->response);

   //SNMP version identifier
   context->response.version = context->request.version;

   //Message identifier
   context->response.msgId = context->request.msgId;
   //Maximum message size supported by the sender
   context->response.msgMaxSize = SNMP_MAX_MSG_SIZE;
   //Bit fields which control processing of the message
   context->response.msgFlags = 0;
   //Security model used by the sender
   context->response.msgSecurityModel = SNMP_SECURITY_MODEL_USM;

   //Authoritative engine identifier
   context->response.msgAuthEngineId = context->contextEngine;
   context->response.msgAuthEngineIdLen = context->contextEngineLen;
   //Number of times the SNMP engine has rebooted
   context->response.msgAuthEngineBoots = context->engineBoots;
   //Number of seconds since last reboot
   context->response.msgAuthEngineTime = context->engineTime;

   //Context engine identifier
   context->response.contextEngineId = context->contextEngine;
   context->response.contextEngineIdLen = context->contextEngineLen;
   //Context name
   context->response.contextName = (uint8_t *) context->contextName;
   context->response.contextNameLen = strlen(context->contextName);

   //PDU type
   context->response.pduType = SNMP_PDU_REPORT;
   //Request identifier
   context->response.requestId = context->request.requestId;

   //Make room for the message header at the beginning of the buffer
   error = snmpComputeMessageOverhead(&context->response);
   //Any error to report?
   if(error)
      return error;

   //Encode the object value using ASN.1 rules
   error = snmpEncodeUnsignedInt32(1, context->response.buffer, &n);
   //Any error to report?
   if(error)
      return error;

   //Check error indication
   switch(errorIndication)
   {
   case ERROR_UNSUPPORTED_SECURITY_LEVEL:
      //Add the usmStatsUnsupportedSecLevels counter in the varBindList
      var.oid = usmStatsUnsupportedSecLevelsObject;
      var.oidLen = sizeof(usmStatsUnsupportedSecLevelsObject);
      break;
   case ERROR_NOT_IN_TIME_WINDOW:
      //Add the usmStatsNotInTimeWindows counter in the varBindList
      var.oid = usmStatsNotInTimeWindowsObject;
      var.oidLen = sizeof(usmStatsNotInTimeWindowsObject);
      break;
   case ERROR_UNKNOWN_USER_NAME:
      //Add the usmStatsUnknownUserNames counter in the varBindList
      var.oid = usmStatsUnknownUserNamesObject;
      var.oidLen = sizeof(usmStatsUnknownUserNamesObject);
      break;
   case ERROR_UNKNOWN_ENGINE_ID:
      //Add the usmStatsUnknownEngineIDs counter in the varBindList
      var.oid = usmStatsUnknownEngineIdsObject;
      var.oidLen = sizeof(usmStatsUnknownEngineIdsObject);
      break;
   case ERROR_AUTHENTICATION_FAILED:
      //Add the usmStatsWrongDigests counter in the varBindList
      var.oid = usmStatsWrongDigestsObject;
      var.oidLen = sizeof(usmStatsWrongDigestsObject);
      break;
   case ERROR_DECRYPTION_FAILED:
      //Add the usmStatsDecryptionErrors counter in the varBindList
      var.oid = usmStatsDecryptionErrorsObject;
      var.oidLen = sizeof(usmStatsDecryptionErrorsObject);
      break;
   default:
      //Just for sanity's sake...
      var.oid = NULL;
      var.oidLen = 0;
      break;
   }

   //The counter is encoded in ASN.1 format
   var.objClass = ASN1_CLASS_APPLICATION;
   var.objType = MIB_TYPE_COUNTER32;
   var.value = context->response.buffer;
   var.valueLen = n;

   //Append the variable binding list to the varBindList
   error = snmpWriteVarBinding(context, &var);
   //Any error to report?
   if(error)
      return error;

   //Format PDU header
   error = snmpWritePduHeader(&context->response);
#else
   //SNMPv3 is not supported
   error = ERROR_NOT_IMPLEMENTED;
#endif

   //Return status code
   return error;
}

#endif
