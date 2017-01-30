/**
 * @file snmp_agent.c
 * @brief SNMP agent (Simple Network Management Protocol)
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
 * @section Description
 *
 * SNMP is a simple protocol by which management information for a network
 * element may be inspected or altered by logically remote users. Refer
 * to the following RFCs for complete details:
 * - RFC 1157: A Simple Network Management Protocol (SNMP)
 * - RFC 1905: Protocol Operations for Version 2 of the Simple Network
 *     Management Protocol (SNMPv2)
 * - RFC 3410: Introduction and Applicability Statements for Internet
 *     Standard Management Framework
 * - RFC 3411: An Architecture for Describing SNMP Management Frameworks
 * - RFC 3412: Message Processing and Dispatching for the SNMP
 * - RFC 3413: Simple Network Management Protocol (SNMP) Applications
 * - RFC 3584: Coexistence between Version 1, Version 2, and Version 3 of
 *     SNMP Framework
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
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains SNMP agent settings
 **/

void snmpAgentGetDefaultSettings(SnmpAgentSettings *settings)
{
   //The SNMP agent is not bound to any interface
   settings->interface = NULL;

   //Minimum version accepted by the SNMP agent
   settings->versionMin = SNMP_VERSION_1;
   //Maximum version accepted by the SNMP agent
   settings->versionMax = SNMP_VERSION_3;

   //SNMP port number
   settings->port = SNMP_PORT;
   //SNMP trap port number
   settings->trapPort = SNMP_TRAP_PORT;

   //Random data generation callback function
   settings->randCallback = NULL;
}


/**
 * @brief SNMP agent initialization
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] settings SNMP agent specific settings
 * @return Error code
 **/

error_t snmpAgentInit(SnmpAgentContext *context, const SnmpAgentSettings *settings)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing SNMP agent...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;

   //Check minimum and maximum SNMP versions
   if(settings->versionMin > settings->versionMax)
      return ERROR_INVALID_PARAMETER;

   //Clear the SNMP agent context
   memset(context, 0, sizeof(SnmpAgentContext));

   //Save user settings
   context->settings = *settings;

#if (SNMP_V3_SUPPORT == ENABLED)
   //Get current time
   context->systemTime = osGetSystemTime();

   //Each SNMP engine maintains two values, snmpEngineBoots and snmpEngineTime,
   //which taken together provide an indication of time at that SNMP engine
   context->engineBoots = 1;
   context->engineTime = 0;

   //Check whether SNMPv3 is supported
   if(settings->versionMin <= SNMP_VERSION_3 &&
      settings->versionMax >= SNMP_VERSION_3)
   {
      //Make sure a random number generator has been registered
      if(settings->randCallback == NULL)
         return ERROR_INVALID_PARAMETER;

      //The salt integer is initialized to an arbitrary value at boot time
      error = settings->randCallback((uint8_t *) &context->salt, sizeof(context->salt));
      //Any error to report?
      if(error)
         return error;
   }
#endif

   //Create a mutex to prevent simultaneous access to SNMP agent context
   if(!osCreateMutex(&context->mutex))
   {
      //Failed to create mutex
      return ERROR_OUT_OF_RESOURCES;
   }

   //Open a UDP socket
   context->socket = socketOpen(SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);

   //Failed to open socket?
   if(!context->socket)
   {
      //Clean up side effects
      osDeleteMutex(&context->mutex);
      //Report an error
      return ERROR_OPEN_FAILED;
   }

   //Start of exception handling block
   do
   {
      //Explicitly associate the socket with the relevant interface
      error = socketBindToInterface(context->socket, settings->interface);
      //Unable to bind the socket to the desired interface?
      if(error)
         break;

      //The SNMP agent listens for messages on port 161
      error = socketBind(context->socket, &IP_ADDR_ANY, settings->port);
      //Unable to bind the socket to the desired port?
      if(error)
         break;

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Clean up side effects
      osDeleteMutex(&context->mutex);
      //Close underlying socket
      socketClose(context->socket);
   }

   //Return status code
   return error;
}


/**
 * @brief Start SNMP agent
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpAgentStart(SnmpAgentContext *context)
{
   OsTask *task;

   //Debug message
   TRACE_INFO("Starting SNMP agent...\r\n");

   //Make sure the SNMP agent context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Start the SNMP agent service
   task = osCreateTask("SNMP Agent", (OsTaskCode) snmpAgentTask,
      context, SNMP_AGENT_STACK_SIZE, SNMP_AGENT_PRIORITY);

   //Unable to create the task?
   if(task == OS_INVALID_HANDLE)
      return ERROR_OUT_OF_RESOURCES;

   //The SNMP agent has successfully started
   return NO_ERROR;
}


/**
 * @brief Load a MIB module
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] module Pointer the MIB module to be loaded
 * @return Error code
 **/

error_t snmpAgentLoadMib(SnmpAgentContext *context, const MibModule *module)
{
   error_t error;
   uint_t i;
   uint_t j;

   //Check parameters
   if(context == NULL || module == NULL)
      return ERROR_INVALID_PARAMETER;
   if(module->numObjects < 1)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);

   //Loop through existing MIBs
   for(i = 0; i < context->mibModuleCount; i++)
   {
      //Check whether the specified MIB module is already loaded
      if(context->mibModule[i] == module)
         break;
   }

   //MIB module found?
   if(i < context->mibModuleCount)
   {
      //Prevent the SNMP agent from loading the specified MIB multiple times
      error = NO_ERROR;
   }
   else
   {
      //Make sure there is enough room to add the specified MIB
      if(context->mibModuleCount < SNMP_AGENT_MAX_MIB_COUNT)
      {
         //Loop through existing MIBs
         for(i = 0; i < context->mibModuleCount; i++)
         {
            //Compare object identifiers
            if(oidComp(module->objects[0].oid, module->objects[0].oidLen,
               context->mibModule[i]->objects[0].oid, context->mibModule[i]->objects[0].oidLen) < 0)
            {
               //Make room for the new MIB
               for(j = context->mibModuleCount; j > i; j--)
                  context->mibModule[j] = context->mibModule[j - 1];

               //We are done
               break;
            }
         }

         //Insert the new MIB to the list
         context->mibModule[i] = module;
         //Update the number of MIBs
         context->mibModuleCount++;

         //Successful processing
         error = NO_ERROR;
      }
      else
      {
         //Failed to load the specified MIB
         error = ERROR_OUT_OF_RESOURCES;
      }
   }

   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
}


/**
 * @brief Unload a MIB module
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] module Pointer the MIB module to be unloaded
 * @return Error code
 **/

error_t snmpAgentUnloadMib(SnmpAgentContext *context, const MibModule *module)
{
   error_t error;
   uint_t i;
   uint_t j;

   //Check parameters
   if(context == NULL || module == NULL)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);

   //Loop through existing MIBs
   for(i = 0; i < context->mibModuleCount; i++)
   {
      //Check whether the specified MIB module is already loaded
      if(context->mibModule[i] == module)
         break;
   }

   //MIB module found?
   if(i < context->mibModuleCount)
   {
      //Update the number of MIBs
      context->mibModuleCount--;

      //Remove the specified MIB from the list
      for(j = i; j < context->mibModuleCount; j++)
         context->mibModule[j] = context->mibModule[j + 1];

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //Failed to unload the specified MIB
      error = ERROR_NOT_FOUND;
   }

   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
}


/**
 * @brief Set the value of the snmpEngineBoots variable
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] engineBoots Number of times the SNMP engine has re-booted
 * @return Error code
 **/

error_t snmpAgentSetEngineBoots(SnmpAgentContext *context, int32_t engineBoots)
{
#if (SNMP_V3_SUPPORT == ENABLED)
   //Check parameters
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;
   if(engineBoots < 0)
      return ERROR_OUT_OF_RANGE;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);

   //Get current time
   context->systemTime = osGetSystemTime();

   //Set the value of the snmpEngineBoots
   context->engineBoots = engineBoots;
   //The snmpEngineTime is reset to zero
   context->engineTime = 0;

   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Get the value of the snmpEngineBoots variable
 * @param[in] context Pointer to the SNMP agent context
 * @param[out] engineBoots Number of times the SNMP engine has re-booted
 * @return Error code
 **/

error_t snmpAgentGetEngineBoots(SnmpAgentContext *context, int32_t *engineBoots)
{
#if (SNMP_V3_SUPPORT == ENABLED)
   //Check parameters
   if(context == NULL || engineBoots == NULL)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);
   //Get the current value of the snmpEngineBoots
   *engineBoots = context->engineBoots;
   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Set enterprise OID
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] enterpriseOid Pointer to the enterprise OID
 * @param[in] enterpriseOidLen Length of the enterprise OID
 * @return Error code
 **/

error_t snmpAgentSetEnterpriseOid(SnmpAgentContext *context,
   const uint8_t *enterpriseOid, size_t enterpriseOidLen)
{
   //Check parameters
   if(context == NULL || enterpriseOid == NULL)
      return ERROR_INVALID_PARAMETER;
   if(enterpriseOidLen > SNMP_MAX_OID_SIZE)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);

   //Set enterprise OID
   memcpy(context->enterpriseOid, enterpriseOid, enterpriseOidLen);
   //Save the length of the enterprise OID
   context->enterpriseOidLen = enterpriseOidLen;

   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set context engine identifier
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] contextEngine Pointer to the context engine identifier
 * @param[in] contextEngineLen Length of the context engine identifier
 * @return Error code
 **/

error_t snmpAgentSetContextEngine(SnmpAgentContext *context,
   const void *contextEngine, size_t contextEngineLen)
{
#if (SNMP_V3_SUPPORT == ENABLED)
   //Check parameters
   if(context == NULL || contextEngine == NULL)
      return ERROR_INVALID_PARAMETER;
   if(contextEngineLen > SNMP_MAX_CONTEXT_ENGINE_SIZE)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);

   //Set context engine identifier
   memcpy(context->contextEngine, contextEngine, contextEngineLen);
   //Save the length of the context engine identifier
   context->contextEngineLen = contextEngineLen;

   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Set context name
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] contextName NULL-terminated string that contains the context name
 * @return Error code
 **/

error_t snmpAgentSetContextName(SnmpAgentContext *context,
   const char_t *contextName)
{
#if (SNMP_V3_SUPPORT == ENABLED)
   size_t n;

   //Check parameters
   if(context == NULL || contextName == NULL)
      return ERROR_INVALID_PARAMETER;

   //Retrieve the length of the context name
   n = strlen(contextName);

   //Make sure the context name is valid
   if(n > SNMP_MAX_CONTEXT_NAME_LEN)
      return ERROR_INVALID_LENGTH;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);
   //Set context name
   strcpy(context->contextName, contextName);
   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Create a new community string
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] community NULL-terminated string that contains the community name
 * @param[in] mode Access rights
 * @return Error code
 **/

error_t snmpAgentCreateCommunity(SnmpAgentContext *context,
   const char_t *community, SnmpAccess mode)
{
#if (SNMP_V1_SUPPORT == ENABLED || SNMP_V2C_SUPPORT == ENABLED)
   //Add the community string to the local configuration datastore
   return snmpAgentCreateUser(context, community, mode, SNMP_KEY_FORMAT_NONE,
      SNMP_AUTH_PROTOCOL_NONE, NULL, SNMP_PRIV_PROTOCOL_NONE, NULL);
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Remove a community string
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] community NULL-terminated string that contains the community name
 * @return Error code
 **/

error_t snmpAgentDeleteCommunity(SnmpAgentContext *context, const char_t *community)
{
#if (SNMP_V1_SUPPORT == ENABLED || SNMP_V2C_SUPPORT == ENABLED)
   //Remove the community string from the local configuration datastore
   return snmpAgentDeleteUser(context, community);
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Create a new user
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] username NULL-terminated string that contains the user name
 * @param[in] mode Access rights
 * @param[in] keyFormat Key format (ASCII password or raw key)
 * @param[in] authProtocol Authentication type
 * @param[in] authKey Key to be used for data authentication
 * @param[in] privProtocol Privacy type
 * @param[in] privKey Key to be used for data encryption
 * @return Error code
 **/

error_t snmpAgentCreateUser(SnmpAgentContext *context,
   const char_t *username, SnmpAccess mode, SnmpKeyFormat keyFormat,
   SnmpAuthProtocol authProtocol, const void *authKey,
   SnmpPrivProtocol privProtocol, const void *privKey)
{
   error_t error;
   uint_t i;
   size_t n;
   SnmpUserInfo *entry;
   SnmpUserInfo *firstFreeEntry;

   //Check parameters
   if(context == NULL || username == NULL)
      return ERROR_INVALID_PARAMETER;

   //Data authentication?
   if(authProtocol != SNMP_AUTH_PROTOCOL_NONE)
   {
      //Check key format
      if(keyFormat != SNMP_KEY_FORMAT_TEXT && keyFormat != SNMP_KEY_FORMAT_RAW)
         return ERROR_INVALID_PARAMETER;

      //Data authentication requires a key
      if(authKey == NULL)
         return ERROR_INVALID_PARAMETER;
   }

   //Data confidentiality?
   if(privProtocol != SNMP_PRIV_PROTOCOL_NONE)
   {
      //Check key format
      if(keyFormat != SNMP_KEY_FORMAT_TEXT && keyFormat != SNMP_KEY_FORMAT_RAW)
         return ERROR_INVALID_PARAMETER;

      //Data confidentiality requires a key
      if(privKey == NULL)
         return ERROR_INVALID_PARAMETER;

      //There is no provision for data confidentiality without data authentication
      if(authProtocol == SNMP_AUTH_PROTOCOL_NONE)
         return ERROR_INVALID_PARAMETER;
   }

   //Retrieve the length of the user name
   n = strlen(username);

   //Make sure the user name is valid
   if(n == 0 || n > SNMP_MAX_USER_NAME_LEN)
      return ERROR_INVALID_LENGTH;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);

   //Keep track of the first free entry
   firstFreeEntry = NULL;

   //Loop through the list of users
   for(i = 0; i < SNMP_AGENT_MAX_USER_COUNT; i++)
   {
      //Point to the current entry
      entry = &context->userTable[i];

      //Check if the entry is currently in use
      if(entry->name[0] != '\0')
      {
         //Check whether the user name already exists
         if(!strcmp(entry->name, username))
            break;
      }
      else
      {
         //Keep track of the first free entry
         if(firstFreeEntry == NULL)
            firstFreeEntry = entry;
      }
   }

   //If the specified user name does not exist, then a new
   //entry should be created
   if(i >= SNMP_AGENT_MAX_USER_COUNT)
      entry = firstFreeEntry;

   //Check whether the service list runs out of space
   if(entry != NULL)
   {
      //Save user name
      strcpy(entry->name, username);
      //Access rights
      entry->mode = mode;

      //Successful processing
      error = NO_ERROR;

#if (SNMP_V3_SUPPORT == ENABLED)
      //Authentication protocol
      entry->authProtocol = authProtocol;
      //Privacy protocol
      entry->privProtocol = privProtocol;

      //Data authentication?
      if(authProtocol != SNMP_AUTH_PROTOCOL_NONE)
      {
         //ASCII password or raw key?
         if(keyFormat == SNMP_KEY_FORMAT_TEXT)
         {
            //Generate the authentication key from the provided password
            error = snmpGenerateKey(authProtocol, authKey, context->contextEngine,
               context->contextEngineLen, &entry->authKey);
         }
         else
         {
            //Save the authentication key
            memcpy(&entry->authKey, authKey, sizeof(SnmpKey));
         }
      }

      //Check status code
      if(!error)
      {
         //Data confidentiality?
         if(privProtocol != SNMP_PRIV_PROTOCOL_NONE)
         {
            //ASCII password or raw key?
            if(keyFormat == SNMP_KEY_FORMAT_TEXT)
            {
               //Generate the privacy key from the provided password
               error = snmpGenerateKey(authProtocol, privKey, context->contextEngine,
                  context->contextEngineLen, &entry->privKey);
            }
            else
            {
               //Save the privacy key
               memcpy(&entry->privKey, privKey, sizeof(SnmpKey));
            }
         }
      }

      //Check status code
      if(error)
      {
         //Clean up side effects
         memset(entry, 0, sizeof(SnmpUserInfo));
      }
#endif
   }
   else
   {
      //Unable to add new user
      error = ERROR_OUT_OF_RESOURCES;
   }

   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Return error code
   return error;
}


/**
 * @brief Remove existing user
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] username NULL-terminated string that contains the user name
 * @return Error code
 **/

error_t snmpAgentDeleteUser(SnmpAgentContext *context, const char_t *username)
{
   error_t error;
   uint_t i;
   SnmpUserInfo *entry;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);

   //Loop through the list of users
   for(i = 0; i < SNMP_AGENT_MAX_USER_COUNT; i++)
   {
      //Point to the current entry
      entry = &context->userTable[i];

      //Compare user names
      if(!strcmp(entry->name, username))
         break;
   }

   //User name found?
   if(i < SNMP_AGENT_MAX_USER_COUNT)
   {
      //Clear the security profile of the user
      memset(entry, 0, sizeof(SnmpUserInfo));
      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //The specified user name does not exist
      error = ERROR_NOT_FOUND;
   }

   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
}


/**
 * @brief Send SNMP trap message
 * @param[in] context Pointer to the SNMP agent context
 * @param[in] destIpAddr Destination IP address
 * @param[in] version SNMP version identifier
 * @param[in] username User name or community name
 * @param[in] genericTrapType Generic trap type
 * @param[in] specificTrapCode Specific code
 * @param[in] objectList List of object names
 * @param[in] objectListSize Number of entries in the list
 * @return Error code
 **/

error_t snmpAgentSendTrap(SnmpAgentContext *context, const IpAddr *destIpAddr,
   SnmpVersion version, const char_t *username, uint_t genericTrapType,
   uint_t specificTrapCode, const SnmpTrapObject *objectList, uint_t objectListSize)
{
   error_t error;

   //Check parameters
   if(context == NULL || destIpAddr == NULL || username == NULL)
      return ERROR_INVALID_PARAMETER;

   //Make sure the list of objects is valid
   if(objectListSize > 0 && objectList == NULL)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the SNMP agent context
   osAcquireMutex(&context->mutex);

   //Refresh SNMP engine time
   snmpRefreshEngineTime(context);

   //Start of exception handling block
   do
   {
#if (SNMP_V1_SUPPORT == ENABLED)
      //SNMPv1 version?
      if(version == SNMP_VERSION_1)
      {
         //Format Trap-PDU
         error = snmpFormatTrapPdu(context, version, username,
            genericTrapType, specificTrapCode, objectList, objectListSize);
         //Any error to report?
         if(error)
            break;

         //Format SMNP message header
         error = snmpWriteMessageHeader(&context->response);
         //Any error to report?
         if(error)
            break;
      }
      else
#endif
#if (SNMP_V2C_SUPPORT == ENABLED)
      //SNMPv2c version?
      if(version == SNMP_VERSION_2C)
      {
         //Format SNMPv2-Trap-PDU
         error = snmpFormatTrapPdu(context, version, username,
            genericTrapType, specificTrapCode, objectList, objectListSize);
         //Any error to report?
         if(error)
            break;

         //Format SMNP message header
         error = snmpWriteMessageHeader(&context->response);
         //Any error to report?
         if(error)
            break;
      }
      else
#endif
#if (SNMP_V3_SUPPORT == ENABLED)
      //SNMPv3 version?
      if(version == SNMP_VERSION_3)
      {
         //Information about the user name is extracted from the local
         //configuration datastore
         context->user = snmpFindUser(context, username, strlen(username));

         //Invalid user name?
         if(context->user == NULL)
         {
            //Report an error
            error = ERROR_UNKNOWN_USER_NAME;
            //Exit immediately
            break;
         }

         //Format SNMPv2-Trap-PDU
         error = snmpFormatTrapPdu(context, version, username,
            genericTrapType, specificTrapCode, objectList, objectListSize);
         //Any error to report?
         if(error)
            break;

         //Format scopedPDU
         error = snmpWriteScopedPdu(&context->response);
         //Any error to report?
         if(error)
            break;

         //Check whether the privFlag is set
         if(context->response.msgFlags & SNMP_MSG_FLAG_PRIV)
         {
            //Encrypt data
            error = snmpEncryptData(context->user, &context->response, &context->salt);
            //Any error to report?
            if(error)
               break;
         }

         //Format SMNP message header
         error = snmpWriteMessageHeader(&context->response);
         //Any error to report?
         if(error)
            break;

         //Check whether the authFlag is set
         if(context->response.msgFlags & SNMP_MSG_FLAG_AUTH)
         {
            //Authenticate outgoing SNMP message
            error = snmpAuthOutgoingMessage(context->user, &context->response);
            //Any error to report?
            if(error)
               break;
         }
      }
      else
#endif
      //Invalid SNMP version?
      {
         //Debug message
         TRACE_WARNING("  Invalid SNMP version!\r\n");
         //Report an error
         error = ERROR_INVALID_VERSION;
         //Exit immediately
         break;
      }

      //Total number of messages which were passed from the SNMP protocol
      //entity to the transport service
      MIB2_INC_COUNTER32(mib2Base.snmpGroup.snmpOutPkts, 1);

      //Debug message
      TRACE_INFO("Sending SNMP message to %s port %" PRIu16
         " (%" PRIuSIZE " bytes)...\r\n",
         ipAddrToString(destIpAddr, NULL),
         context->settings.trapPort, context->response.length);

      //Display the contents of the SNMP message
      TRACE_DEBUG_ARRAY("  ", context->response.pos, context->response.length);
      //Display ASN.1 structure
      asn1DumpObject(context->response.pos, context->response.length, 0);

      //Send SNMP trap message
      error = socketSendTo(context->socket, destIpAddr, context->settings.trapPort,
         context->response.pos, context->response.length, NULL, 0);

      //End of exception handling block
   } while(0);

   //Release exclusive access to the SNMP agent context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
}


/**
 * @brief SNMP agent task
 * @param[in] context Pointer to the SNMP agent context
 **/

void snmpAgentTask(SnmpAgentContext *context)
{
   error_t error;

#if (NET_RTOS_SUPPORT == ENABLED)
   //Main loop
   while(1)
   {
#endif
      //Wait for an incoming datagram
      error = socketReceiveFrom(context->socket, &context->remoteIpAddr,
         &context->remotePort, context->request.buffer,
         SNMP_MAX_MSG_SIZE, &context->request.bufferLen, 0);

      //Any datagram received?
      if(!error)
      {
         //Acquire exclusive access to the SNMP agent context
         osAcquireMutex(&context->mutex);

         //Debug message
         TRACE_INFO("\r\nSNMP message received from %s port %" PRIu16
            " (%" PRIuSIZE " bytes)...\r\n",
            ipAddrToString(&context->remoteIpAddr, NULL),
            context->remotePort, context->request.bufferLen);

         //Display the contents of the SNMP message
         TRACE_DEBUG_ARRAY("  ", context->request.buffer, context->request.bufferLen);
         //Dump ASN.1 structure
         asn1DumpObject(context->request.buffer, context->request.bufferLen, 0);

         //Process incoming SNMP message
         error = snmpProcessMessage(context);

         //Check status code
         if(!error)
         {
            //Debug message
            TRACE_INFO("Sending SNMP message to %s port %" PRIu16
               " (%" PRIuSIZE " bytes)...\r\n",
               ipAddrToString(&context->remoteIpAddr, NULL),
               context->remotePort, context->response.length);

            //Display the contents of the SNMP message
            TRACE_DEBUG_ARRAY("  ", context->response.pos, context->response.length);
            //Display ASN.1 structure
            asn1DumpObject(context->response.pos, context->response.length, 0);

            //Send SNMP response message
            socketSendTo(context->socket, &context->remoteIpAddr, context->remotePort,
               context->response.pos, context->response.length, NULL, 0);
         }

         //Release exclusive access to the SNMP agent context
         osReleaseMutex(&context->mutex);
      }
#if (NET_RTOS_SUPPORT == ENABLED)
   }
#endif
}

#endif
