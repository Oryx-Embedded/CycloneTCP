/**
 * @file ntp_debug.c
 * @brief Data logging functions for debugging purpose (NTP)
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
#define TRACE_LEVEL NTP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "ntp/ntp_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (NTP_TRACE_LEVEL >= TRACE_LEVEL_DEBUG)

//Leap indicators
const NtpParamName ntpLeapIndicatorList[] =
{
   {NTP_LI_NO_WARNING,           "no warning"},
   {NTP_LI_LAST_MIN_HAS_61_SECS, "last minute has 61 seconds"},
   {NTP_LI_LAST_MIN_HAS_59_SECS, "last minute has 59 seconds"},
   {NTP_LI_ALARM_CONDITION,      "alarm condition"}
};

//NTP version numbers
const NtpParamName ntpVersionList[] =
{
   {NTP_VERSION_3, "NTPv3"},
   {NTP_VERSION_4, "NTPv4"}
};

//Protocol modes
const NtpParamName ntpModeList[] =
{
   {NTP_MODE_SYMMETRIC_ACTIVE,  "symmetric active"},
   {NTP_MODE_SYMMETRIC_PASSIVE, "symmetric passive"},
   {NTP_MODE_CLIENT,            "client"},
   {NTP_MODE_SERVER,            "server"},
   {NTP_MODE_BROADCAST,         "broadcast"}
};

//Stratum
const NtpParamName ntpStratumList[] =
{
   {NTP_STRATUM_KISS_OF_DEATH, "kiss-of-death message"},
   {NTP_STRATUM_PRIMARY,       "primary"},
   {NTP_STRATUM_SECONDARY_2,   "secondary"},
   {NTP_STRATUM_SECONDARY_3,   "secondary"},
   {NTP_STRATUM_SECONDARY_4,   "secondary"},
   {NTP_STRATUM_SECONDARY_5,   "secondary"},
   {NTP_STRATUM_SECONDARY_6,   "secondary"},
   {NTP_STRATUM_SECONDARY_7,   "secondary"},
   {NTP_STRATUM_SECONDARY_8,   "secondary"},
   {NTP_STRATUM_SECONDARY_9,   "secondary"},
   {NTP_STRATUM_SECONDARY_10,  "secondary"},
   {NTP_STRATUM_SECONDARY_11,  "secondary"},
   {NTP_STRATUM_SECONDARY_12,  "secondary"},
   {NTP_STRATUM_SECONDARY_13,  "secondary"},
   {NTP_STRATUM_SECONDARY_14,  "secondary"},
   {NTP_STRATUM_SECONDARY_15,  "secondary"}
};

//Extensions field types
const NtpParamName ntpExtensionTypeList[] =
{
   {NTP_EXTENSION_TYPE_NO_OPERATION_REQ,        "No-Operation Request"},
   {NTP_EXTENSION_TYPE_UNIQUE_ID,               "Unique Identifier"},
   {NTP_EXTENSION_TYPE_NTS_COOKIE,              "NTS Cookie"},
   {NTP_EXTENSION_TYPE_NTS_COOKIE_PLACEHOLDER,  "NTS Cookie Placeholder"},
   {NTP_EXTENSION_TYPE_NTS_AEAD,                "NTS Authenticator and Encrypted Extension Fields"},
   {NTP_EXTENSION_TYPE_NO_OPERATION_RESP,       "No-Operation Response"},
   {NTP_EXTENSION_TYPE_NO_OPERATION_ERROR_RESP, "No-Operation Error Response"}
};


/**
 * @brief Dump NTP packet for debugging purpose
 * @param[in] packet Pointer to the NTP packet to dump
 * @param[in] length Length of the packet, in bytes
 **/

void ntpDumpPacket(const NtpHeader *packet, size_t length)
{
   uint32_t value;
   const char_t *name;
   const NtpHeader *header;

   //Malformed packet?
   if(length < sizeof(NtpHeader))
      return;

   //Point to the NTP packet header
   header = (NtpHeader *) packet;

   //Convert the Leap Indicator field to string representation
   name = ntpGetParamName(header->li, ntpLeapIndicatorList,
      arraysize(ntpLeapIndicatorList));

   //Dump Leap Indicator field
   TRACE_DEBUG("  Leap indicator = %" PRIu8 " (%s)\r\n", header->li, name);

   //Convert the Version Number field to string representation
   name = ntpGetParamName(header->vn, ntpVersionList,
      arraysize(ntpVersionList));

   //Dump Version Number field
   TRACE_DEBUG("  Version Number = %" PRIu8 " (%s)\r\n", header->vn, name);

   //Convert the Mode field to string representation
   name = ntpGetParamName(header->mode, ntpModeList, arraysize(ntpModeList));

   //Dump Mode field
   TRACE_DEBUG("  Mode = %" PRIu8 " (%s)\r\n", header->mode, name);

   //The Stratum field is significant only in server messages
   if(header->mode == NTP_MODE_SERVER)
   {
      //Convert the Stratum field to string representation
      name = ntpGetParamName(header->stratum, ntpStratumList,
         arraysize(ntpStratumList));

      //Dump Stratum field
      TRACE_DEBUG("  Stratum = %" PRIu8 " (%s)\r\n", header->stratum, name);
   }
   else
   {
      //Dump Stratum field
      TRACE_DEBUG("  Stratum = %" PRIu8 "\r\n", header->stratum);
   }

   //Dump other fields
   TRACE_DEBUG("  Poll = %" PRIu8 " (%us)\r\n", header->poll, 1U << header->poll);
   TRACE_DEBUG("  Precision = %" PRId8 "\r\n", header->precision);
   TRACE_DEBUG("  Root Delay = %" PRIu32 "\r\n", ntohl(header->rootDelay));
   TRACE_DEBUG("  Root Dispersion = %" PRIu32 "\r\n", ntohl(header->rootDispersion));

   //The Reference Identifier field is significant only in server messages,
   //where for stratum 0 (kiss-of-death message) and 1 (primary server)
   if(header->mode == NTP_MODE_SERVER &&
      header->stratum <= NTP_STRATUM_PRIMARY)
   {
      //The value is a four-character ASCII string
      value = htonl(header->referenceId);

      //Dump Reference Identifier field
      TRACE_DEBUG("  Reference Identifier = '%c%c%c%c'\r\n",
         (value >> 24) & 0xFF, (value >> 16) & 0xFF, (value >> 8) & 0xFF,
         value & 0xFF);
   }
   else
   {
      //Dump Reference Identifier field
      TRACE_DEBUG("  Reference Identifier = %" PRIu32 "\r\n",
         header->referenceId);
   }

   //Dump Reference Timestamp field
   TRACE_DEBUG("  ReferenceTimestamp\r\n");
   ntpDumpTimestamp(&header->referenceTimestamp);

   //Dump Originate Timestamp field
   TRACE_DEBUG("  Originate Timestamp\r\n");
   ntpDumpTimestamp(&header->originateTimestamp);

   //Dump Receive Timestamp field
   TRACE_DEBUG("  Receive Timestamp\r\n");
   ntpDumpTimestamp(&header->receiveTimestamp);

   //Dump Transmit Timestamp field
   TRACE_DEBUG("  Transmit Timestamp\r\n");
   ntpDumpTimestamp(&header->transmitTimestamp);

   //Get the length of the extension fields
   length -= sizeof(NtpHeader);
   //Parse extension fields
   ntpDumpExtensions(header->extensions, length);
}


/**
 * @brief Dump NTP extension fields
 * @param[in] extensions Pointer to the extension fields
 * @param[in] length Total length of the extension fields, in bytes
 **/

void ntpDumpExtensions(const uint8_t *extensions, size_t length)
{
   size_t i;
   size_t n;
   const NtpExtension *extension;

   //Parse extension fields
   for(i = 0; i < length; i += n)
   {
      //Malformed extension?
      if(length < sizeof(NtpExtension))
         break;

      //Point to the current extension
      extension = (NtpExtension *) (extensions + i);
      //Retrieve the length of the extension
      n = ntohs(extension->length);

      //Malformed extension?
      if(n < sizeof(NtpExtension) || (i + n) > length)
         break;

      //Dump extension
      ntpDumpExtension(extension, n);
   }
}


/**
 * @brief Dump NTP extension field
 * @param[in] extension Pointer to the extension field
 * @param[in] length Length of the extension field, in bytes
 **/

void ntpDumpExtension(const NtpExtension *extension, size_t length)
{
   uint16_t type;
   const char_t *name;

   //Malformed extension?
   if(length < sizeof(NtpExtension))
      return;

   //Get extension type
   type = ntohs(extension->fieldType);

   //Convert the Type field to string representation
   name = ntpGetParamName(type, ntpExtensionTypeList,
      arraysize(ntpExtensionTypeList));

   //Display the name of the current extension
   if(osStrcmp(name, "Unknown") != 0)
   {
      TRACE_DEBUG("  %s Extension (%" PRIu8 " bytes)\r\n", name, length);
   }
   else
   {
      TRACE_DEBUG("  Extension %" PRIu16 " (%" PRIu8 " bytes)\r\n", type,
         length);
   }

   //Check extension type
   if(type == NTP_EXTENSION_TYPE_NTS_AEAD)
   {
      //Dump NTS Authenticator and Encrypted Extension Fields extension
      ntpDumpNtsAeadExtension((NtpNtsAeadExtension *) extension, length);
   }
   else
   {
      //Retrieve the length of the extension value
      length -= sizeof(NtpExtension);
      //Dump extension value
      TRACE_DEBUG_ARRAY("    ", extension->value, length);
   }
}


/**
 * @brief Dump NTS Authenticator and Encrypted Extension Fields extension
 * @param[in] extension Pointer to the extension field
 * @param[in] length Length of the extension field, in bytes
 **/

void ntpDumpNtsAeadExtension(const NtpNtsAeadExtension *extension, size_t length)
{
   uint16_t nonceLen;
   uint16_t ciphertextLen;

   //Malformed extension?
   if(length < sizeof(NtpNtsAeadExtension))
      return;

   //Retrieve the length of the nonce and ciphertext
   nonceLen = ntohs(extension->nonceLength);
   ciphertextLen = ntohs(extension->ciphertextLength);

   //Malformed extension?
   if(length < (sizeof(NtpNtsAeadExtension) + nonceLen + ciphertextLen))
      return;

   //Dump extension
   TRACE_DEBUG("    Nonce Length = %" PRIu16 "\r\n", nonceLen);
   TRACE_DEBUG("    Ciphertext Length = %" PRIu16 "\r\n", ciphertextLen);

   //Dump nonce
   TRACE_DEBUG("    Nonce (%" PRIu16 " bytes)\r\n", nonceLen);
   TRACE_DEBUG_ARRAY("      ", extension->nonce, nonceLen);

   //Dump ciphertext
   TRACE_DEBUG("    Ciphertext (%" PRIu16 " bytes)\r\n", ciphertextLen);
   TRACE_DEBUG_ARRAY("      ", extension->nonce + nonceLen, ciphertextLen);
}


/**
 * @brief Dump NTP timestamp
 * @param[in] timestamp Pointer to the NTP timestamp
 **/

void ntpDumpTimestamp(const NtpTimestamp *timestamp)
{
   //Dump seconds
   TRACE_DEBUG("    Seconds = %" PRIu32 "\r\n", ntohl(timestamp->seconds));
   //Dump fraction field
   TRACE_DEBUG("    Fraction = %" PRIu32 "\r\n", ntohl(timestamp->fraction));
}


/**
 * @brief Convert a parameter to string representation
 * @param[in] value Parameter value
 * @param[in] paramList List of acceptable parameters
 * @param[in] paramListLen Number of entries in the list
 * @return NULL-terminated string describing the parameter
 **/

const char_t *ntpGetParamName(uint_t value, const NtpParamName *paramList,
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
