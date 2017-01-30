/**
 * @file snmp_usm.c
 * @brief User-based Security Model (USM) for SNMPv3
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
 * This module implements the User-based Security Model (USM) for Simple
 * Network Management Protocol (SNMP) version 3. Refer to the following
 * RFCs for complete details:
 * - RFC 3414: User-based Security Model (USM) for SNMPv3
 * - RFC 3826: AES Cipher Algorithm in the SNMP User-based Security Model
 * - RFC 7860: HMAC-SHA-2 Authentication Protocols in the User-based Security Model
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.7.6
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL SNMP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "snmp/snmp_common.h"
#include "snmp/snmp_usm.h"
#include "crypto.h"
#include "asn1.h"
#include "hmac.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SNMP_V3_SUPPORT == ENABLED)

//usmStatsUnsupportedSecLevels.0 object (1.3.6.1.6.3.15.1.1.1.0)
const uint8_t usmStatsUnsupportedSecLevelsObject[10] = {43, 6, 1, 6, 3, 15, 1, 1, 1, 0};
//usmStatsNotInTimeWindows.0 object (1.3.6.1.6.3.15.1.1.2.0)
const uint8_t usmStatsNotInTimeWindowsObject[10] = {43, 6, 1, 6, 3, 15, 1, 1, 2, 0};
//usmStatsUnknownUserNames.0 object (1.3.6.1.6.3.15.1.1.3.0)
const uint8_t usmStatsUnknownUserNamesObject[10] = {43, 6, 1, 6, 3, 15, 1, 1, 3, 0};
//usmStatsUnknownEngineIDs.0 object (1.3.6.1.6.3.15.1.1.4.0)
const uint8_t usmStatsUnknownEngineIdsObject[10] = {43, 6, 1, 6, 3, 15, 1, 1, 4, 0};
//usmStatsWrongDigests.0 object (1.3.6.1.6.3.15.1.1.5.0)
const uint8_t usmStatsWrongDigestsObject[10] = {43, 6, 1, 6, 3, 15, 1, 1, 5, 0};
//usmStatsDecryptionErrors.0 object (1.3.6.1.6.3.15.1.1.6.0)
const uint8_t usmStatsDecryptionErrorsObject[10] = {43, 6, 1, 6, 3, 15, 1, 1, 6, 0};


/**
 * @brief Password to key algorithm
 * @param[in] authProtocol Authentication protocol (MD5 or SHA-1)
 * @param[in] password NULL-terminated string that contains the password
 * @param[in] engineId Pointer to the engine ID
 * @param[in] engineIdLen Length of the engine ID
 * @param[out] key Pointer to the resulting key
 * @return Error code
 **/

error_t snmpGenerateKey(SnmpAuthProtocol authProtocol, const char_t *password,
   const uint8_t *engineId, size_t engineIdLen, SnmpKey *key)
{
   size_t i;
   size_t n;
   size_t passwordLen;
   const HashAlgo *hash;
   uint8_t context[MAX_HASH_CONTEXT_SIZE];

   //Clear SNMP key
   memset(key, 0, sizeof(SnmpKey));

#if (SNMP_MD5_SUPPORT == ENABLED)
   //HMAC-MD5-96 authentication protocol?
   if(authProtocol == SNMP_AUTH_PROTOCOL_MD5)
   {
      //Use MD5 to generate the key
      hash = MD5_HASH_ALGO;
   }
   else
#endif
#if (SNMP_SHA1_SUPPORT == ENABLED)
   //HMAC-SHA-1-96 authentication protocol?
   if(authProtocol == SNMP_AUTH_PROTOCOL_SHA1)
   {
      //Use SHA-1 to generate the key
      hash = SHA1_HASH_ALGO;
   }
   else
#endif
#if (SNMP_SHA224_SUPPORT == ENABLED)
   //HMAC-SHA-224-128 authentication protocol?
   if(authProtocol == SNMP_AUTH_PROTOCOL_SHA224)
   {
      //Use SHA-224 to generate the key
      hash = SHA224_HASH_ALGO;
   }
   else
#endif
#if (SNMP_SHA256_SUPPORT == ENABLED)
   //HMAC-SHA-256-192 authentication protocol?
   if(authProtocol == SNMP_AUTH_PROTOCOL_SHA256)
   {
      //Use SHA-256 to generate the key
      hash = SHA256_HASH_ALGO;
   }
   else
#endif
#if (SNMP_SHA384_SUPPORT == ENABLED)
   //HMAC-SHA-384-256 authentication protocol?
   if(authProtocol == SNMP_AUTH_PROTOCOL_SHA384)
   {
      //Use SHA-384 to generate the key
      hash = SHA384_HASH_ALGO;
   }
   else
#endif
#if (SNMP_SHA512_SUPPORT == ENABLED)
   //HMAC-SHA-512-384 authentication protocol?
   if(authProtocol == SNMP_AUTH_PROTOCOL_SHA512)
   {
      //Use SHA-512 to generate the key
      hash = SHA512_HASH_ALGO;
   }
   else
#endif
   {
      //Invalid authentication protocol
      return ERROR_INVALID_PARAMETER;
   }

   //Retrieve the length of the password
   passwordLen = strlen(password);

   //SNMP implementations must ensure that passwords are at
   //least 8 characters in length (see RFC 3414 11.2)
   if(passwordLen < 8)
      return ERROR_INVALID_LENGTH;

   //Initialize hash context
   hash->init(context);

   //Loop until we have done 1 megabyte
   for(i = 0; i < 1048576; i += n)
   {
      n = MIN(passwordLen, 1048576 - i);
      hash->update(context, password, n);
   }

   //Finalize hash computation
   hash->final(context, key->b);

   //Key localization
   hash->init(context);
   hash->update(context, key, hash->digestSize);
   hash->update(context, engineId, engineIdLen);
   hash->update(context, key, hash->digestSize);
   hash->final(context, key->b);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Check security parameters
 * @param[in] user Security profile of the user
 * @param[in,out] message Pointer to the incoming SNMP message
 * @param[in] engineId Pointer to the authoritative engine ID
 * @param[in] engineIdLen Length of the authoritative engine ID
 * @return Error code
 **/

error_t snmpCheckSecurityParameters(const SnmpUserInfo *user,
   SnmpMessage *message, const uint8_t *engineId, size_t engineIdLen)
{
   //Check the length of the authoritative engine ID
   if(message->msgAuthEngineIdLen != engineIdLen)
      return ERROR_UNKNOWN_ENGINE_ID;

   //If the value of the msgAuthoritativeEngineID field is unknown, then an
   //error indication (unknownEngineID) is returned to the calling module
   if(memcmp(message->msgAuthEngineId, engineId, engineIdLen))
      return ERROR_UNKNOWN_ENGINE_ID;

   //If no information is available for the user, then an error indication
   //(unknownSecurityName) is returned to the calling module
   if(user == NULL)
      return ERROR_UNKNOWN_USER_NAME;

   //Check whether the securityLevel specifies that the message should
   //be authenticated
   if(user->authProtocol != SNMP_AUTH_PROTOCOL_NONE)
   {
      //Make sure the authFlag is set
      if(!(message->msgFlags & SNMP_MSG_FLAG_AUTH))
         return ERROR_UNSUPPORTED_SECURITY_LEVEL;
   }

   //Check whether the securityLevel specifies that the message should
   //be encrypted
   if(user->privProtocol != SNMP_PRIV_PROTOCOL_NONE)
   {
      //Make sure the privFlag is set
      if(!(message->msgFlags & SNMP_MSG_FLAG_PRIV))
         return ERROR_UNSUPPORTED_SECURITY_LEVEL;
   }

   //Security parameters are valid
   return NO_ERROR;
}


/**
 * @brief Authenticate outgoing SNMP message
 * @param[in] user Security profile of the user
 * @param[in,out] message Pointer to the outgoing SNMP message
 * @return Error code
 **/

error_t snmpAuthOutgoingMessage(const SnmpUserInfo *user, SnmpMessage *message)
{
   const HashAlgo *hash;
   size_t hmacDigestSize;
   HmacContext hmacContext;

#if (SNMP_MD5_SUPPORT == ENABLED)
   //HMAC-MD5-96 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_MD5)
   {
      //Use MD5 hash algorithm
      hash = MD5_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 12;
   }
   else
#endif
#if (SNMP_SHA1_SUPPORT == ENABLED)
   //HMAC-SHA-1-96 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA1)
   {
      //Use SHA-1 hash algorithm
      hash = SHA1_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 12;
   }
   else
#endif
#if (SNMP_SHA224_SUPPORT == ENABLED)
   //HMAC-SHA-224-128 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA224)
   {
      //Use SHA-224 hash algorithm
      hash = SHA224_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 16;
   }
   else
#endif
#if (SNMP_SHA256_SUPPORT == ENABLED)
   //HMAC-SHA-256-192 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA256)
   {
      //Use SHA-256 hash algorithm
      hash = SHA256_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 24;
   }
   else
#endif
#if (SNMP_SHA384_SUPPORT == ENABLED)
   //HMAC-SHA-384-256 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA384)
   {
      //Use SHA-384 hash algorithm
      hash = SHA384_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 32;
   }
   else
#endif
#if (SNMP_SHA512_SUPPORT == ENABLED)
   //HMAC-SHA-512-384 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA512)
   {
      //Use SHA-512 hash algorithm
      hash = SHA512_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 48;
   }
   else
#endif
   //Invalid authentication protocol?
   {
      //Report en error
      return ERROR_FAILURE;
   }

   //Check the length of the msgAuthenticationParameters field
   if(message->msgAuthParametersLen != hmacDigestSize)
      return ERROR_FAILURE;

   //The MAC is calculated over the whole message
   hmacInit(&hmacContext, hash, user->authKey.b, hash->digestSize);
   hmacUpdate(&hmacContext, message->pos, message->length);
   hmacFinal(&hmacContext, NULL);

   //Replace the msgAuthenticationParameters field with the calculated MAC
   memcpy(message->msgAuthParameters, hmacContext.digest, hmacDigestSize);

   //Successful message authentication
   return NO_ERROR;
}


/**
 * @brief Authenticate incoming SNMP message
 * @param[in] user Security profile of the user
 * @param[in] message Pointer to the incoming SNMP message
 * @return Error code
 **/

error_t snmpAuthIncomingMessage(const SnmpUserInfo *user, SnmpMessage *message)
{
   const HashAlgo *hash;
   size_t hmacDigestSize;
   uint8_t hmacDigest[SNMP_MAX_HMAC_DIGEST_SIZE];
   HmacContext hmacContext;

#if (SNMP_MD5_SUPPORT == ENABLED)
   //HMAC-MD5-96 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_MD5)
   {
      //Use MD5 hash algorithm
      hash = MD5_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 12;
   }
   else
#endif
#if (SNMP_SHA1_SUPPORT == ENABLED)
   //HMAC-SHA-1-96 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA1)
   {
      //Use SHA-1 hash algorithm
      hash = SHA1_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 12;
   }
   else
#endif
#if (SNMP_SHA224_SUPPORT == ENABLED)
   //HMAC-SHA-224-128 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA224)
   {
      //Use SHA-224 hash algorithm
      hash = SHA224_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 16;
   }
   else
#endif
#if (SNMP_SHA256_SUPPORT == ENABLED)
   //HMAC-SHA-256-192 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA256)
   {
      //Use SHA-256 hash algorithm
      hash = SHA256_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 24;
   }
   else
#endif
#if (SNMP_SHA384_SUPPORT == ENABLED)
   //HMAC-SHA-384-256 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA384)
   {
      //Use SHA-384 hash algorithm
      hash = SHA384_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 32;
   }
   else
#endif
#if (SNMP_SHA512_SUPPORT == ENABLED)
   //HMAC-SHA-512-384 authentication protocol?
   if(user->authProtocol == SNMP_AUTH_PROTOCOL_SHA512)
   {
      //Use SHA-512 hash algorithm
      hash = SHA512_HASH_ALGO;
      //Length of the message digest
      hmacDigestSize = 48;
   }
   else
#endif
   //Invalid authentication protocol?
   {
      //Report an error
      return ERROR_AUTHENTICATION_FAILED;
   }

   //Check the length of the msgAuthenticationParameters field
   if(message->msgAuthParametersLen != hmacDigestSize)
      return ERROR_AUTHENTICATION_FAILED;

   //The MAC received in the msgAuthenticationParameters field is saved
   memcpy(hmacDigest, message->msgAuthParameters, hmacDigestSize);

   //The digest in the msgAuthenticationParameters field is replaced by
   //a null octet string
   memset(message->msgAuthParameters, 0, hmacDigestSize);

   //The MAC is calculated over the whole message
   hmacInit(&hmacContext, hash, user->authKey.b, hash->digestSize);
   hmacUpdate(&hmacContext, message->buffer, message->bufferLen);
   hmacFinal(&hmacContext, NULL);

   //Restore the value of the msgAuthenticationParameters field
   memcpy(message->msgAuthParameters, hmacDigest, hmacDigestSize);

   //The newly calculated MAC is compared with the MAC value that was
   //saved in the first step
   if(memcmp(hmacContext.digest, hmacDigest, hmacDigestSize))
      return ERROR_AUTHENTICATION_FAILED;

   //Successful message authentication
   return NO_ERROR;
}


/**
 * @brief Data encryption
 * @param[in] user Security profile of the user
 * @param[in,out] message Pointer to the outgoing SNMP message
 * @param[in,out] salt Pointer to the salt integer
 * @return Error code
 **/

error_t snmpEncryptData(const SnmpUserInfo *user, SnmpMessage *message, uint64_t *salt)
{
   error_t error;
   uint_t i;
   size_t n;
   Asn1Tag tag;

   //Debug message
   TRACE_DEBUG("Scoped PDU (%" PRIuSIZE " bytes):\r\n", message->length);
   //Display the contents of the scopedPDU
   TRACE_DEBUG_ARRAY("  ", message->pos, message->length);
   //Display ASN.1 structure
   asn1DumpObject(message->pos, message->length, 0);

#if (SNMP_DES_SUPPORT == ENABLED)
   //DES-CBC privacy protocol?
   if(user->privProtocol == SNMP_PRIV_PROTOCOL_DES)
   {
      DesContext desContext;
      uint8_t iv[DES_BLOCK_SIZE];

      //The data to be encrypted is treated as sequence of octets. Its length
      //should be an integral multiple of 8
      if(message->length % 8)
      {
         //If it is not, the data is padded at the end as necessary
         n = 8 - (message->length % 8);
         //The actual pad value is irrelevant
         memset(message->pos + message->length, n, n);
         //Update the length of the data
         message->length += n;
      }

      //The 32-bit snmpEngineBoots is converted to the first 4 octets of our salt
      STORE32BE(message->msgAuthEngineBoots, message->msgPrivParameters);
      //The 32-bit integer is then converted to the last 4 octet of our salt
      STORE32BE(*salt, message->msgPrivParameters + 4);

      //The resulting salt is then put into the msgPrivacyParameters field
      message->msgPrivParametersLen = 8;

      //Initialize DES context
      error = desInit(&desContext, user->privKey.b, 8);
      //Initialization failed?
      if(error)
         return error;

      //The last 8 octets of the 16-octet secret (private privacy key) are
      //used as pre-IV
      memcpy(iv, user->privKey.b + DES_BLOCK_SIZE, DES_BLOCK_SIZE);

      //The msgPrivacyParameters field is XOR-ed with the pre-IV to obtain the IV
      for(i = 0; i < DES_BLOCK_SIZE; i++)
         iv[i] ^= message->msgPrivParameters[i];

      //Perform CBC encryption
      error = cbcEncrypt(DES_CIPHER_ALGO, &desContext, iv,
         message->pos, message->pos, message->length);
      //Any error to report?
      if(error)
         return error;
   }
   else
#endif
#if (SNMP_AES_SUPPORT == ENABLED)
   //AES-128-CFB privacy protocol?
   if(user->privProtocol == SNMP_PRIV_PROTOCOL_AES)
   {
      AesContext aesContext;
      uint8_t iv[AES_BLOCK_SIZE];

      //The 32-bit snmpEngineBoots is converted to the first 4 octets of the IV
      STORE32BE(message->msgAuthEngineBoots, iv);
      //The 32-bit snmpEngineTime is converted to the subsequent 4 octets
      STORE32BE(message->msgAuthEngineTime, iv + 4);
      //The 64-bit integer is then converted to the last 8 octets
      STORE64BE(*salt, iv + 8);

      //The 64-bit integer must be placed in the msgPrivacyParameters field to
      //enable the receiving entity to compute the correct IV and to decrypt
      //the message
      STORE64BE(*salt, message->msgPrivParameters);
      message->msgPrivParametersLen = 8;

      //Initialize AES context
      error = aesInit(&aesContext, user->privKey.b, 16);
      //Initialization failed?
      if(error)
         return error;

      //Perform CFB-128 encryption
      error = cfbEncrypt(AES_CIPHER_ALGO, &aesContext, 128, iv,
         message->pos, message->pos, message->length);
      //Any error to report?
      if(error)
         return error;
   }
   else
#endif
   //Invalid privacy protocol?
   {
      //Report an error
      return ERROR_FAILURE;
   }

   //The encryptedPDU is encapsulated within an octet string
   tag.constructed = FALSE;
   tag.objClass = ASN1_CLASS_UNIVERSAL;
   tag.objType = ASN1_TYPE_OCTET_STRING;
   tag.length = message->length;
   tag.value = NULL;

   //Write the corresponding ASN.1 tag
   error = asn1WriteTag(&tag, TRUE, message->pos, &n);
   //Any error to report?
   if(error)
      return error;

   //Move backward
   message->pos -= n;
   //Total length of the encryptedPDU
   message->length += n;

   //The salt integer is then modified. It is incremented by one and wrap
   //when it reaches its maximum value
   *salt += 1;

   //Successful encryption
   return NO_ERROR;
}


/**
 * @brief Data decryption
 * @param[in] user Security profile of the user
 * @param[in,out] message Pointer to the incoming SNMP message
 * @return Error code
 **/

error_t snmpDecryptData(const SnmpUserInfo *user, SnmpMessage *message)
{
   error_t error;
   uint_t i;
   Asn1Tag tag;

   //The encryptedPDU is encapsulated within an octet string
   error = asn1ReadTag(message->pos, message->length, &tag);
   //Failed to decode ASN.1 tag?
   if(error)
      return error;

   //Enforce encoding, class and type
   error = asn1CheckTag(&tag, FALSE, ASN1_CLASS_UNIVERSAL, ASN1_TYPE_OCTET_STRING);
   //The tag does not match the criteria?
   if(error)
      return error;

   //Point to the encryptedPDU
   message->pos = (uint8_t *) tag.value;
   //Length of the encryptedPDU
   message->length = tag.length;

#if (SNMP_DES_SUPPORT == ENABLED)
   //DES-CBC privacy protocol?
   if(user->privProtocol == SNMP_PRIV_PROTOCOL_DES)
   {
      DesContext desContext;
      uint8_t iv[DES_BLOCK_SIZE];

      //Before decryption, the encrypted data length is verified. The length
      //of the encrypted data must be a multiple of 8 octets
      if(message->length % 8)
         return ERROR_DECRYPTION_FAILED;

      //Check the length of the msgPrivacyParameters field
      if(message->msgPrivParametersLen != 8)
         return ERROR_DECRYPTION_FAILED;

      //Initialize DES context
      error = desInit(&desContext, user->privKey.b, 8);
      //Initialization failed?
      if(error)
         return error;

      //The last 8 octets of the 16-octet secret (private privacy key) are
      //used as pre-IV
      memcpy(iv, user->privKey.b + DES_BLOCK_SIZE, DES_BLOCK_SIZE);

      //The msgPrivacyParameters field is XOR-ed with the pre-IV to obtain the IV
      for(i = 0; i < DES_BLOCK_SIZE; i++)
         iv[i] ^= message->msgPrivParameters[i];

      //Perform CBC decryption
      error = cbcDecrypt(DES_CIPHER_ALGO, &desContext, iv,
         message->pos, message->pos, message->length);
      //Any error to report?
      if(error)
         return error;
   }
   else
#endif
#if (SNMP_AES_SUPPORT == ENABLED)
   //AES-128-CFB privacy protocol?
   if(user->privProtocol == SNMP_PRIV_PROTOCOL_AES)
   {
      AesContext aesContext;
      uint8_t iv[AES_BLOCK_SIZE];

      //Check the length of the msgPrivacyParameters field
      if(message->msgPrivParametersLen != 8)
         return ERROR_DECRYPTION_FAILED;

      //The 32-bit snmpEngineBoots is converted to the first 4 octets of the IV
      STORE32BE(message->msgAuthEngineBoots, iv);
      //The 32-bit snmpEngineTime is converted to the subsequent 4 octets
      STORE32BE(message->msgAuthEngineTime, iv + 4);
      //The 64-bit integer is then converted to the last 8 octets
      memcpy(iv + 8, message->msgPrivParameters, 8);

      //Initialize AES context
      error = aesInit(&aesContext, user->privKey.b, 16);
      //Initialization failed?
      if(error)
         return error;

      //Perform CFB-128 encryption
      error = cfbDecrypt(AES_CIPHER_ALGO, &aesContext, 128, iv,
         message->pos, message->pos, message->length);
      //Any error to report?
      if(error)
         return error;
   }
   else
#endif
   //Invalid privacy protocol?
   {
      //Report an error
      return ERROR_DECRYPTION_FAILED;
   }

   //Debug message
   TRACE_DEBUG("Scoped PDU (%" PRIuSIZE " bytes):\r\n", message->length);
   //Display the contents of the scopedPDU
   TRACE_DEBUG_ARRAY("  ", message->pos, message->length);
   //Display ASN.1 structure
   asn1DumpObject(message->pos, message->length, 0);

   //Successful decryption
   return NO_ERROR;
}

#endif
