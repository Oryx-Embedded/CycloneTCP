/**
 * @file snmp_usm_mib_impl.c
 * @brief SNMP USM MIB module implementation
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
 * @version 1.7.8
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL SNMP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "mibs/mib_common.h"
#include "mibs/snmp_usm_mib_module.h"
#include "mibs/snmp_usm_mib_impl.h"
#include "snmp/snmp_agent.h"
#include "snmp/snmp_agent_misc.h"
#include "crypto.h"
#include "asn1.h"
#include "oid.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SNMP_USM_MIB_SUPPORT == ENABLED)

//usmNoAuthProtocol OID (1.3.6.1.6.3.10.1.1.1)
const uint8_t usmNoAuthProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 1, 1};
//usmHMACMD5AuthProtocol OID (1.3.6.1.6.3.10.1.1.2)
const uint8_t usmHMACMD5AuthProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 1, 2};
//usmHMACSHAAuthProtocol OID (1.3.6.1.6.3.10.1.1.3)
const uint8_t usmHMACSHAAuthProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 1, 3};
//usmHMAC128SHA224AuthProtocol OID (1.3.6.1.6.3.10.1.1.4)
const uint8_t usmHMAC128SHA224AuthProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 1, 4};
//usmHMAC192SHA256AuthProtocol OID (1.3.6.1.6.3.10.1.1.5)
const uint8_t usmHMAC192SHA256AuthProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 1, 5};
//usmHMAC256SHA384AuthProtocol OID (1.3.6.1.6.3.10.1.1.6)
const uint8_t usmHMAC256SHA384AuthProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 1, 6};
//usmHMAC384SHA512AuthProtocol OID (1.3.6.1.6.3.10.1.1.7)
const uint8_t usmHMAC384SHA512AuthProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 1, 7};

//usmNoPrivProtocol OID (1.3.6.1.6.3.10.1.2.1)
const uint8_t usmNoPrivProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 2, 1};
//usmDESPrivProtocol OID (1.3.6.1.6.3.10.1.2.2)
const uint8_t usmDESPrivProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 2, 2};
//usmAesCfb128Protocol OID (1.3.6.1.6.3.10.1.2.4)
const uint8_t usmAesCfb128ProtocolOid[9] = {43, 6, 1, 6, 3, 10, 1, 2, 4};


/**
 * @brief SNMP USM MIB module initialization
 * @return Error code
 **/

error_t snmpUsmMibInit(void)
{
   //Debug message
   TRACE_INFO("Initializing SNMP-USM-MIB base...\r\n");

   //Clear SNMP USM MIB base
   memset(&snmpUsmMibBase, 0, sizeof(snmpUsmMibBase));

   //usmUserSpinLock object
   snmpUsmMibBase.usmUserSpinLock = netGetRandRange(1, INT32_MAX);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Load SNMP USM MIB module
 * @param[in] context Pointer to the SNMP agent context
 * @return Error code
 **/

error_t snmpUsmMibLoad(void *context)
{
   //Register SNMP agent context
   snmpUsmMibBase.context = context;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Unload SNMP USM MIB module
 * @param[in] context Pointer to the SNMP agent context
 **/

void snmpUsmMibUnload(void *context)
{
   //Unregister SNMP agent context
   snmpUsmMibBase.context = NULL;
}


/**
 * @brief Lock SNMP USM MIB base
 **/

void snmpUsmMibLock(void)
{
}


/**
 * @brief Unlock SNMP USM MIB base
 **/

void snmpUsmMibUnlock(void)
{
}


/**
 * @brief Set usmUserSpinLock object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in] value Object value
 * @param[in] valueLen Length of the object value, in bytes
 * @param[in] commit This flag tells whether the changes shall be committed
 *   to the MIB base
 * @return Error code
 **/

error_t snmpUsmMibSetUsmUserSpinLock(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen, bool_t commit)
{
   error_t error;

   //The new value supplied via the management protocol must precisely match
   //the value presently held by the instance
   if(value->integer == snmpUsmMibBase.usmUserSpinLock)
   {
      //Check whether the changes shall be committed to the MIB base
      if(commit)
      {
         //The value held by the instance is incremented by one
         snmpUsmMibBase.usmUserSpinLock++;

         //if the current value is the maximum value of 2^31-1, then the value
         //held by the instance is wrapped to zero
         if(snmpUsmMibBase.usmUserSpinLock < 0)
            snmpUsmMibBase.usmUserSpinLock = 0;
      }

      //Successful operation
      error = NO_ERROR;
   }
   else
   {
      //The management protocol set operation fails with an error of
      //inconsistentValue
      error = ERROR_INCONSISTENT_VALUE;
   }

   //Return status code
   return error;
}


/**
 * @brief Get usmUserSpinLock object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t snmpUsmMibGetUsmUserSpinLock(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   //Get the current value of the spin lock
   value->integer = snmpUsmMibBase.usmUserSpinLock;

   //Return status code
   return NO_ERROR;
}


/**
 * @brief Set usmUserEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[in] value Object value
 * @param[in] valueLen Length of the object value, in bytes
 * @param[in] commit This flag tells whether the changes shall be committed
 *   to the MIB base
 * @return Error code
 **/

error_t snmpUsmMibSetUsmUserEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, const MibVariant *value, size_t valueLen, bool_t commit)
{
   error_t error;
   size_t n;
   uint8_t userEngineId[SNMP_MAX_CONTEXT_ENGINE_SIZE];
   size_t userEngineIdLen;
   char_t userName[SNMP_MAX_USER_NAME_LEN];
   size_t userNameLen;
   SnmpAgentContext *context;

   //Point to the instance identifier
   n = object->oidLen;

   //usmUserEngineID is used as 1st instance identifier
   error = mibDecodeOctetString(oid, oidLen, &n, userEngineId,
      SNMP_MAX_CONTEXT_ENGINE_SIZE, &userEngineIdLen);
   //Invalid instance identifier?
   if(error)
      return error;

   //usmUserName is used as 2nd instance identifier
   error = mibDecodeOctetString(oid, oidLen, &n, (uint8_t *) userName,
      SNMP_MAX_USER_NAME_LEN, &userNameLen);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Point to the SNMP agent context
   context = (SnmpAgentContext *) snmpUsmMibBase.context;

   //Sanity check
   if(context == NULL)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check the length of the SNMP engine ID
   if(userEngineIdLen != context->contextEngineLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check SNMP engine ID
   if(memcmp(userEngineId, context->contextEngine, userEngineIdLen))
      return ERROR_INSTANCE_NOT_FOUND;

   //usmUserCloneFrom object?
   else if(!strcmp(object->name, "usmUserCloneFrom"))
   {
      //Copy object value
      //memcpy(temp, value->oid, valueLen);
   }
   //usmUserAuthProtocol object?
   else if(!strcmp(object->name, "usmUserAuthProtocol"))
   {
      //Copy object value
      //memcpy(temp, value->oid, valueLen);
   }
   //usmUserAuthKeyChange object?
   else if(!strcmp(object->name, "usmUserAuthKeyChange"))
   {
      //Copy object value
      //memcpy(temp, value->octetString, valueLen);
   }
   //usmUserOwnAuthKeyChange object?
   else if(!strcmp(object->name, "usmUserOwnAuthKeyChange"))
   {
      //Copy object value
      //memcpy(temp, value->octetString, valueLen);
   }
   //usmUserPrivProtocol object?
   else if(!strcmp(object->name, "usmUserPrivProtocol"))
   {
      //Copy object value
      //memcpy(temp, value->oid, valueLen);
   }
   //usmUserPrivKeyChange object?
   else if(!strcmp(object->name, "usmUserPrivKeyChange"))
   {
      //Copy object value
      //memcpy(temp, value->octetString, valueLen);
   }
   //usmUserOwnPrivKeyChange object?
   else if(!strcmp(object->name, "usmUserOwnPrivKeyChange"))
   {
      //Copy object value
      //memcpy(temp, value->octetString, valueLen);
   }
   //usmUserPublic object?
   else if(!strcmp(object->name, "usmUserPublic"))
   {
      //Copy object value
      //memcpy(temp, value->octetString, valueLen);
   }
   //usmUserStorageType object?
   else if(!strcmp(object->name, "usmUserStorageType"))
   {
      //Set object value
      //int32_t temp = value->integer;
   }
   //usmUserStatus object?
   else if(!strcmp(object->name, "usmUserStatus"))
   {
      //Set object value
      //int32_t temp = value->integer;
   }
   //Unknown object?
   else
   {
      //The specified object does not exist
      error = ERROR_OBJECT_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Get usmUserEntry object value
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier (object name and instance identifier)
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] value Object value
 * @param[in,out] valueLen Length of the object value, in bytes
 * @return Error code
 **/

error_t snmpUsmMibGetUsmUserEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, MibVariant *value, size_t *valueLen)
{
   error_t error;
   size_t n;
   uint8_t userEngineId[SNMP_MAX_CONTEXT_ENGINE_SIZE];
   size_t userEngineIdLen;
   char_t userName[SNMP_MAX_USER_NAME_LEN];
   size_t userNameLen;
   SnmpAgentContext *context;
   SnmpUserInfo *user;

   //Point to the instance identifier
   n = object->oidLen;

   //usmUserEngineID is used as 1st instance identifier
   error = mibDecodeOctetString(oid, oidLen, &n, userEngineId,
      SNMP_MAX_CONTEXT_ENGINE_SIZE, &userEngineIdLen);
   //Invalid instance identifier?
   if(error)
      return error;

   //usmUserName is used as 2nd instance identifier
   error = mibDecodeOctetString(oid, oidLen, &n, (uint8_t *) userName,
      SNMP_MAX_USER_NAME_LEN, &userNameLen);
   //Invalid instance identifier?
   if(error)
      return error;

   //Sanity check
   if(n != oidLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Point to the SNMP agent context
   context = (SnmpAgentContext *) snmpUsmMibBase.context;

   //Sanity check
   if(context == NULL)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check the length of the SNMP engine ID
   if(userEngineIdLen != context->contextEngineLen)
      return ERROR_INSTANCE_NOT_FOUND;

   //Check SNMP engine ID
   if(memcmp(userEngineId, context->contextEngine, userEngineIdLen))
      return ERROR_INSTANCE_NOT_FOUND;

   //Get security profile corresponding to the specified user name
   user = snmpFindUser(context, userName, userNameLen);

   //Valid user?
   if(user != NULL)
   {
      //usmUserSecurityName object?
      if(!strcmp(object->name, "usmUserSecurityName"))
      {
         //The security name is the same as the user name
         n = strlen(user->name);

         //Make sure the buffer is large enough to hold the entire object
         if(*valueLen >= n)
         {
            //Copy object value
            memcpy(value->octetString, user->name, n);
            //Return object length
            *valueLen = n;
         }
         else
         {
            //Report an error
            error = ERROR_BUFFER_OVERFLOW;
         }
      }
      //usmUserCloneFrom object?
      else if(!strcmp(object->name, "usmUserCloneFrom"))
      {
         //When this object is read, the ZeroDotZero OID is returned
         uint8_t zeroDotZeroOid[] = {0};

         //Make sure the buffer is large enough to hold the entire object
         if(*valueLen >= sizeof(zeroDotZeroOid))
         {
            //Copy object value
            memcpy(value->octetString, zeroDotZeroOid, sizeof(zeroDotZeroOid));
            //Return object length
            *valueLen = sizeof(zeroDotZeroOid);
         }
         else
         {
            //Report an error
            error = ERROR_BUFFER_OVERFLOW;
         }
      }
      //usmUserAuthProtocol object?
      else if(!strcmp(object->name, "usmUserAuthProtocol"))
      {
         size_t protocolLen;
         const uint8_t *protocol;

         //Check the type of authentication protocol which is used
         switch(user->authProtocol)
         {
         //HMAC-MD5-96 authentication protocol?
         case SNMP_AUTH_PROTOCOL_MD5:
            protocol = usmHMACMD5AuthProtocolOid;
            protocolLen = sizeof(usmHMACMD5AuthProtocolOid);
            break;
         //HMAC-SHA-1-96 authentication protocol?
         case SNMP_AUTH_PROTOCOL_SHA1:
            protocol = usmHMACSHAAuthProtocolOid;
            protocolLen = sizeof(usmHMACSHAAuthProtocolOid);
            break;
         //HMAC-SHA-224-128 authentication protocol?
         case SNMP_AUTH_PROTOCOL_SHA224:
            protocol = usmHMAC128SHA224AuthProtocolOid;
            protocolLen = sizeof(usmHMAC128SHA224AuthProtocolOid);
            break;
         //HMAC-SHA-256-192 authentication protocol?
         case SNMP_AUTH_PROTOCOL_SHA256:
            protocol = usmHMAC192SHA256AuthProtocolOid;
            protocolLen = sizeof(usmHMAC192SHA256AuthProtocolOid);
            break;
         //HMAC-SHA-384-256 authentication protocol?
         case SNMP_AUTH_PROTOCOL_SHA384:
            protocol = usmHMAC256SHA384AuthProtocolOid;
            protocolLen = sizeof(usmHMAC256SHA384AuthProtocolOid);
            break;
         //HMAC-SHA-512-384 authentication protocol?
         case SNMP_AUTH_PROTOCOL_SHA512:
            protocol = usmHMAC384SHA512AuthProtocolOid;
            protocolLen = sizeof(usmHMAC384SHA512AuthProtocolOid);
            break;
         //No authentication?
         default:
            protocol = usmNoAuthProtocolOid;
            protocolLen = sizeof(usmNoAuthProtocolOid);
            break;
         }

         //Make sure the buffer is large enough to hold the entire object
         if(*valueLen >= protocolLen)
         {
            //Copy object value
            memcpy(value->octetString, protocol, protocolLen);
            //Return object length
            *valueLen = protocolLen;
         }
         else
         {
            //Report an error
            error = ERROR_BUFFER_OVERFLOW;
         }
      }
      //usmUserAuthKeyChange object?
      else if(!strcmp(object->name, "usmUserAuthKeyChange"))
      {
         //When this object is read, the zero-length (empty) string is returned
         *valueLen = 0;
      }
      //usmUserOwnAuthKeyChange object?
      else if(!strcmp(object->name, "usmUserOwnAuthKeyChange"))
      {
         //When this object is read, the zero-length (empty) string is returned
         *valueLen = 0;
      }
      //usmUserPrivProtocol object?
      else if(!strcmp(object->name, "usmUserPrivProtocol"))
      {
         size_t protocolLen;
         const uint8_t *protocol;

         //Check the type of privacy protocol which is used
         switch(user->privProtocol)
         {
         //DES-CBC privacy protocol?
         case SNMP_PRIV_PROTOCOL_DES:
            protocol = usmDESPrivProtocolOid;
            protocolLen = sizeof(usmDESPrivProtocolOid);
            break;
         //AES-128-CFB privacy protocol?
         case SNMP_PRIV_PROTOCOL_AES:
            protocol = usmAesCfb128ProtocolOid;
            protocolLen = sizeof(usmAesCfb128ProtocolOid);
            break;
         //No privacy?
         default:
            protocol = usmNoPrivProtocolOid;
            protocolLen = sizeof(usmNoPrivProtocolOid);
            break;
         }

         //Make sure the buffer is large enough to hold the entire object
         if(*valueLen >= protocolLen)
         {
            //Copy object value
            memcpy(value->octetString, protocol, protocolLen);
            //Return object length
            *valueLen = protocolLen;
         }
         else
         {
            //Report an error
            error = ERROR_BUFFER_OVERFLOW;
         }
      }
      //usmUserPrivKeyChange object?
      else if(!strcmp(object->name, "usmUserPrivKeyChange"))
      {
         //When this object is read, the zero-length (empty) string is returned
         *valueLen = 0;
      }
      //usmUserOwnPrivKeyChange object?
      else if(!strcmp(object->name, "usmUserOwnPrivKeyChange"))
      {
         //When this object is read, the zero-length (empty) string is returned
         *valueLen = 0;
      }
      //usmUserPublic object?
      else if(!strcmp(object->name, "usmUserPublic"))
      {
         //This publicly-readable value which can be written as part of the
         //procedure for changing a user's secret authentication and/or privacy
         //key, and later read to determine whether the change of the secret
         //was effected
         *valueLen = 0;
      }
      //usmUserStorageType object?
      else if(!strcmp(object->name, "usmUserStorageType"))
      {
         //Get the storage type for this conceptual row
         value->integer = MIB_STORAGE_TYPE_VOLATILE;
      }
      //usmUserStatus object?
      else if(!strcmp(object->name, "usmUserStatus"))
      {
         //Get the status of this conceptual row
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

   //Return status code
   return error;
}


/**
 * @brief Get next usmUserEntry object
 * @param[in] object Pointer to the MIB object descriptor
 * @param[in] oid Object identifier
 * @param[in] oidLen Length of the OID, in bytes
 * @param[out] nextOid OID of the next object in the MIB
 * @param[out] nextOidLen Length of the next object identifier, in bytes
 * @return Error code
 **/

error_t snmpUsmMibGetNextUsmUserEntry(const MibObject *object, const uint8_t *oid,
   size_t oidLen, uint8_t *nextOid, size_t *nextOidLen)
{
   error_t error;
   uint_t i;
   size_t n;
   bool_t acceptable;
   SnmpAgentContext *context;
   SnmpUserInfo *user;
   SnmpUserInfo *curUser;

   //Initialize variables
   user = NULL;

   //Point to the SNMP agent context
   context = (SnmpAgentContext *) snmpUsmMibBase.context;

   //Sanity check
   if(context == NULL)
      return ERROR_OBJECT_NOT_FOUND;

   //Make sure the buffer is large enough to hold the OID prefix
   if(*nextOidLen < object->oidLen)
      return ERROR_BUFFER_OVERFLOW;

   //Copy OID prefix
   memcpy(nextOid, object->oid, object->oidLen);

   //Loop through the list of users
   for(i = 0; i < SNMP_AGENT_MAX_USER_COUNT; i++)
   {
      //Point to the current entry
      curUser = &context->userTable[i];

      //Check if the entry is currently in use
      if(curUser->name[0] != '\0')
      {
         //Append the instance identifier to the OID prefix
         n = object->oidLen;

         //usmUserEngineID is used as 1st instance identifier
         error = mibEncodeOctetString(nextOid, *nextOidLen, &n,
            context->contextEngine, context->contextEngineLen);
         //Any error to report?
         if(error)
            return error;

         //usmUserName is used as 2nd instance identifier
         error = mibEncodeOctetString(nextOid, *nextOidLen, &n,
            (uint8_t *) curUser->name, strlen(curUser->name));
         //Any error to report?
         if(error)
            return error;

         //Check whether the resulting object identifier lexicographically
         //follows the specified OID
         if(oidComp(nextOid, n, oid, oidLen) > 0)
         {
            //Perform lexicographic comparison
            if(user == NULL)
               acceptable = TRUE;
            else if(strlen(curUser->name) < strlen(user->name))
               acceptable = TRUE;
            else if(strlen(curUser->name) > strlen(user->name))
               acceptable = FALSE;
            else if(strcmp(curUser->name, user->name) < 0)
               acceptable = TRUE;
            else
               acceptable = FALSE;

            //Save the closest object identifier that follows the specified
            //OID in lexicographic order
            if(acceptable)
               user = curUser;
         }
      }
   }

   //The specified OID does not lexicographically precede the name
   //of some object?
   if(user == NULL)
      return ERROR_OBJECT_NOT_FOUND;

   //Append the instance identifier to the OID prefix
   n = object->oidLen;

   //usmUserEngineID is used as 1st instance identifier
   error = mibEncodeOctetString(nextOid, *nextOidLen, &n,
      context->contextEngine, context->contextEngineLen);
   //Any error to report?
   if(error)
      return error;

   //usmUserName is used as 2nd instance identifier
   error = mibEncodeOctetString(nextOid, *nextOidLen, &n,
      (uint8_t *) user->name, strlen(user->name));
   //Any error to report?
   if(error)
      return error;

   //Save the length of the resulting object identifier
   *nextOidLen = n;
   //Next object found
   return NO_ERROR;
}

#endif
