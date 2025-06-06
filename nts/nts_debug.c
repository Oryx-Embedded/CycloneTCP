/**
 * @file nts_debug.c
 * @brief Data logging functions for debugging purpose (NTS)
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
#define TRACE_LEVEL NTS_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "nts/nts_debug.h"
#include "aead/aead_algorithms.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (NTS_TRACE_LEVEL >= TRACE_LEVEL_DEBUG)

//NTS-KE record types
const NtsParamName ntsKeRecordTypeList[] =
{
   {NTS_KE_RECORD_TYPE_END_OF_MESSAGE,       "End of Message"},
   {NTS_KE_RECORD_TYPE_NTS_NEXT_PROTO_NEGO,  "NTS Next Protocol Negotiation"},
   {NTS_KE_RECORD_TYPE_ERROR,                "Error"},
   {NTS_KE_RECORD_TYPE_WARNING,              "Warning"},
   {NTS_KE_RECORD_TYPE_AEAD_ALGO_NEGO,       "AEAD Algorithm Negotiation"},
   {NTS_KE_RECORD_TYPE_NEW_COOKIE_FOR_NTPV4, "New Cookie for NTPv4"},
   {NTS_KE_RECORD_TYPE_NTPV4_SERVER_NEGO,    "NTPv4 Server Negotiation"},
   {NTS_KE_RECORD_TYPE_NTPV4_PORT_NEGO,      "NTPv4 Port Negotiation"}
};

//Protocol IDs
const NtsParamName ntsProtocolIdList[] =
{
   {NTS_PROTOCOL_ID_NTPV4, "NTPv4"}
};

//AEAD algorithms
const NtsParamName ntsAeadAlgoList[] =
{
   {AEAD_AES_SIV_CMAC_256, "AEAD_AES_SIV_CMAC_256"}
};

//Error codes
const NtsParamName ntsErrorCodeList[] =
{
   {NTS_ERROR_CODE_UNRECOGNIZED_CRITICAL_RECORD, "Unrecognized Critical Record"},
   {NTS_ERROR_CODE_BAD_REQUEST,                  "Bad Request"},
   {NTS_ERROR_CODE_INTERNAL_SERVER_ERROR,        "Internal Server Error"}
};


/**
 * @brief Dump NTS-KE records
 * @param[in] records Pointer to the records
 * @param[in] length Total length of the records, in bytes
 **/

void ntsDumpNtsKeRecords(const uint8_t *records, size_t length)
{
   size_t i;
   size_t n;
   const NtsKeRecord *record;

   //Parse NTS-KE records
   for(i = 0; i < length; i += sizeof(NtsKeRecord) + n)
   {
      //Malformed record?
      if(length < sizeof(NtsKeRecord))
         break;

      //Point to the current record
      record = (NtsKeRecord *) (records + i);
      //Retrieve the length of the record
      n = ntohs(record->bodyLength);

      //Malformed record?
      if((i + sizeof(NtsKeRecord) + n) > length)
         break;

      //Debug message
      TRACE_DEBUG("  NTS-KE Record (%" PRIu16 " bytes)\r\n",
         sizeof(NtsKeRecord) + n);

      //Dump NTS-KE record
      ntsDumpNtsKeRecord(record, sizeof(NtsKeRecord) + n);
   }
}


/**
 * @brief Dump NTS-KE record
 * @param[in] record Pointer to the record
 * @param[in] length Length of the record, in bytes
 **/

void ntsDumpNtsKeRecord(const NtsKeRecord *record, size_t length)
{
   bool_t critical;
   uint16_t recordType;
   uint16_t bodyLen;
   const char_t *name;

   //Malformed record?
   if(length < sizeof(NtsKeRecord))
      return;

   //Get critical flag
   critical = (ntohs(record->type) & NTS_KE_CRITICAL) ? TRUE : FALSE;
   //Get record type
   recordType = ntohs(record->type) & NTS_KE_RECORD_TYPE_MASK;
   //Get body length
   bodyLen = ntohs(record->bodyLength);

   //Malformed record?
   if(length < (sizeof(NtsKeRecord) + bodyLen))
      return;

   //Convert the Record Type field to string representation
   name = ntsGetParamName(recordType, ntsKeRecordTypeList,
      arraysize(ntsKeRecordTypeList));

   //Dump record
   TRACE_DEBUG("    Critical = %u\r\n", critical);
   TRACE_DEBUG("    Record Type = %" PRIu16 " (%s)\r\n", recordType, name);
   TRACE_DEBUG("    Body Length = %" PRIu16 "\r\n", bodyLen);

   //Check the length of the record body
   if(bodyLen > 0)
   {
      //Debug message
      TRACE_DEBUG("    Body (%" PRIu16 " bytes)\r\n", bodyLen);
   }

   //Check record type
   if(recordType == NTS_KE_RECORD_TYPE_NTS_NEXT_PROTO_NEGO)
   {
      //Dump NTS Next Protocol Negotiation record
      ntsDumpNtsNextProtoNegoRecord(record->body, bodyLen);
   }
   else if(recordType == NTS_KE_RECORD_TYPE_ERROR)
   {
      //Dump Error record
      ntsDumpErrorRecord(record->body, bodyLen);
   }
   else if(recordType == NTS_KE_RECORD_TYPE_WARNING)
   {
      //Dump Warning record
      ntsDumpWarningRecord(record->body, bodyLen);
   }
   else if(recordType == NTS_KE_RECORD_TYPE_AEAD_ALGO_NEGO)
   {
      //Dump AEAD Algorithm Negotiation record
      ntsDumpAeadAlgoNegoRecord(record->body, bodyLen);
   }
   else if(recordType == NTS_KE_RECORD_TYPE_NTPV4_SERVER_NEGO)
   {
      //Dump NTPv4 Server Negotiation record
      ntsDumpNtpv4ServerNegoRecord(record->body, bodyLen);
   }
   else if(recordType == NTS_KE_RECORD_TYPE_NTPV4_PORT_NEGO)
   {
      //Dump NTPv4 Port Negotiation record
      ntsDumpNtpv4PortNegoRecord(record->body, bodyLen);
   }
   else
   {
      //Dump record body
      TRACE_DEBUG_ARRAY("      ", record->body, bodyLen);
   }
}


/**
 * @brief Dump NTS Next Protocol Negotiation record
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 **/

void ntsDumpNtsNextProtoNegoRecord(const uint8_t *body, size_t length)
{
   size_t i;
   uint16_t protocolId;
   const char_t *name;

   //The body consists of a sequence of 16-bit unsigned integers in network
   //byte order (refer to RFC 8915, section 4.1.2)
   for(i = 0; (i + 1) < length; i += sizeof(uint16_t))
   {
      //Each integer represents a protocol ID
      protocolId = LOAD16BE(body + i);

      //Convert the protocol ID to string representation
      name = ntsGetParamName(protocolId, ntsProtocolIdList,
         arraysize(ntsProtocolIdList));

      //Dump protocol ID
      TRACE_DEBUG("      Protocol ID = %" PRIu16 " (%s)\r\n", protocolId, name);
   }
}


/**
 * @brief Dump Error record
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 **/

void ntsDumpErrorRecord(const uint8_t *body, size_t length)
{
   uint16_t errorCode;
   const char_t *name;

   //Malformed record?
   if(length < sizeof(uint16_t))
      return;

   //The body is exactly two octets long, consisting of an unsigned 16-bit
   //integer in network byte order, denoting an error code (refer to RFC 8915,
   //section 4.1.3)
   errorCode = LOAD16BE(body);

   //Convert the error code to string representation
   name = ntsGetParamName(errorCode, ntsErrorCodeList,
      arraysize(ntsErrorCodeList));

   //Dump error code
   TRACE_DEBUG("      Error Code = %" PRIu16 " (%s)\r\n", errorCode, name);
}


/**
 * @brief Dump Warning record
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 **/

void ntsDumpWarningRecord(const uint8_t *body, size_t length)
{
   uint16_t warningCode;

   //Malformed record?
   if(length < sizeof(uint16_t))
      return;

   //The body is exactly two octets long, consisting of an unsigned 16-bit
   //integer in network byte order, denoting an warning code (refer to RFC 8915,
   //section 4.1.4)
   warningCode = LOAD16BE(body);

   //Dump warning code
   TRACE_DEBUG("      Warning Code = %" PRIu16 "\r\n", warningCode);
}


/**
 * @brief Dump AEAD Algorithm Negotiation record
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 **/

void ntsDumpAeadAlgoNegoRecord(const uint8_t *body, size_t length)
{
   size_t i;
   uint16_t aeadAlgo;
   const char_t *name;

   //The body consists of a sequence of 16-bit unsigned integers in network
   //byte order (refer to RFC 8915, section 4.1.5)
   for(i = 0; (i + 1) < length; i += sizeof(uint16_t))
   {
      //Each integer represents an AEAD algorithm
      aeadAlgo = LOAD16BE(body + i);

      //Convert the AEAD algorithm to string representation
      name = ntsGetParamName(aeadAlgo, ntsAeadAlgoList,
         arraysize(ntsAeadAlgoList));

      //Dump AEAD algorithm
      TRACE_DEBUG("      AEAD Algorithm = %" PRIu16 " (%s)\r\n", aeadAlgo, name);
   }
}


/**
 * @brief Dump NTPv4 Server Negotiation record
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 **/

void ntsDumpNtpv4ServerNegoRecord(const uint8_t *body, size_t length)
{
   size_t i;

   //Dump NTPv4 server
   TRACE_DEBUG("      NTPv4 Server = ");

   //The body consists of an ASCII-encoded string. The contents of the string
   //shall be either an IPv4 address, an IPv6 address, or a fully qualified
   //domain name (refer to RFC 8915, section 4.1.7)
   for(i = 0; i < length; i++)
   {
      TRACE_DEBUG("%c", body[i]);
   }

   //Add a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Dump NTPv4 Port Negotiation record
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 **/

void ntsDumpNtpv4PortNegoRecord(const uint8_t *body, size_t length)
{
   uint16_t port;

   //Malformed record?
   if(length < sizeof(uint16_t))
      return;

   //The body consists of a 16-bit unsigned integer in network byte order,
   //denoting a UDP port number (refer to RFC 8915, section 4.1.8)
   port = LOAD16BE(body);

   //Dump NTPv4 port
   TRACE_DEBUG("      NTPv4 Port = %" PRIu16 "\r\n", port);
}


/**
 * @brief Convert a parameter to string representation
 * @param[in] value Parameter value
 * @param[in] paramList List of acceptable parameters
 * @param[in] paramListLen Number of entries in the list
 * @return NULL-terminated string describing the parameter
 **/

const char_t *ntsGetParamName(uint_t value, const NtsParamName *paramList,
   size_t paramListLen)
{
   uint_t i;

   //Default name for unknown values
   static const char_t defaultName[] = "Unknown";

   //Loop through the list of acceptable parameters
   for(i = 0; i < paramListLen; i++)
   {
      if(paramList[i].value == value)
         return paramList[i].name;
   }

   //Unknown value
   return defaultName;
}

#endif
