/**
 * @file nts_client_misc.c
 * @brief Helper functions for NTS client
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
#include "nts/nts_client.h"
#include "nts/nts_client_misc.h"
#include "nts/nts_debug.h"
#include "ntp/ntp_common.h"
#include "ntp/ntp_debug.h"
#include "cipher/cipher_algorithms.h"
#include "aead/aead_algorithms.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (NTS_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Update NTS client state
 * @param[in] context Pointer to the NTS client context
 * @param[in] newState New state to switch to
 **/

void ntsClientChangeState(NtsClientContext *context,
   NtsClientState newState)
{
   //Update HTTP connection state
   context->state = newState;

   //Save current time
   context->timestamp = osGetSystemTime();
}


/**
 * @brief Open NTS-KE connection
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientOpenNtsKeConnection(NtsClientContext *context)
{
   error_t error;

   //Invalid callback functions?
   if(context->tlsInitCallback == NULL || context->randCallback == NULL)
      return ERROR_OPEN_FAILED;

   //Open a TCP socket
   context->ntsKeSocket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
   //Failed to open socket?
   if(context->ntsKeSocket == NULL)
      return ERROR_OPEN_FAILED;

   //Associate the socket with the relevant interface
   error = socketBindToInterface(context->ntsKeSocket, context->interface);
   //Any error to report?
   if(error)
      return error;

   //Set timeout
   error = socketSetTimeout(context->ntsKeSocket, context->timeout);
   //Any error to report?
   if(error)
      return error;

   //Allocate TLS context
   context->tlsContext = tlsInit();
   //Failed to allocate TLS context?
   if(context->tlsContext == NULL)
      return ERROR_OPEN_FAILED;

   //Implementations must not negotiate TLS versions earlier than 1.3 (refer to
   //RFC 8915, section 3)
   error = tlsSetVersion(context->tlsContext, TLS_VERSION_1_3, TLS_VERSION_1_3);
   //Any error to report?
   if(error)
      return error;

   //Select client operation mode
   error = tlsSetConnectionEnd(context->tlsContext, TLS_CONNECTION_END_CLIENT);
   //Any error to report?
   if(error)
      return error;

   //Bind TLS to the relevant socket
   error = tlsSetSocket(context->tlsContext, context->ntsKeSocket);
   //Any error to report?
   if(error)
      return error;

   //Set TX and RX buffer size
   error = tlsSetBufferSize(context->tlsContext,
      NTS_CLIENT_TLS_TX_BUFFER_SIZE, NTS_CLIENT_TLS_RX_BUFFER_SIZE);
   //Any error to report?
   if(error)
      return error;

   //The two endpoints carry out a TLS handshake, with the client offering (via
   //an ALPN extension), and the server accepting, an application-layer protocol
   //of "ntske/1" (refer to RFC 8915, section 4)
   error = tlsSetAlpnProtocolList(context->tlsContext, "ntske/1");
   //Any error to report?
   if(error)
      return error;

   //Restore TLS session
   error = tlsRestoreSessionState(context->tlsContext, &context->tlsSession);
   //Any error to report?
   if(error)
      return error;

   //Perform TLS related initialization
   error = context->tlsInitCallback(context, context->tlsContext);
   //Any error to report?
   if(error)
      return error;

   //Successful processing
   return NO_ERROR;
}

/**
 * @brief Establish NTS-KE connection
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientEstablishNtsKeConnection(NtsClientContext *context)
{
   error_t error;

   //The client connects to an NTS-KE server on the NTS TCP port
   error = socketConnect(context->ntsKeSocket, &context->ntsKeServerIpAddr,
      context->ntsKeServerPort);

   //Check status code
   if(!error)
   {
      //Then, the two parties perform a TLS handshake
      error = tlsConnect(context->tlsContext);
   }

   //Return status code
   return error;
}


/**
 * @brief Format NTS-KE request
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientFormatNtsKeRequest(NtsClientContext *context)
{
   size_t length;
   NtsKeRecord *record;

   //Total length of the NTS-KE request
   length = 0;

   //The request each shall include exactly one NTS Next Protocol Negotiation
   //record (refer to RFC 8915, section 4)
   record = (NtsKeRecord *) (context->buffer + length);

   //Its body consists of a sequence of 16-bit unsigned integers in network
   //byte order
   record->type = HTONS(NTS_KE_CRITICAL | NTS_KE_RECORD_TYPE_NTS_NEXT_PROTO_NEGO);
   record->bodyLength = HTONS(sizeof(uint16_t));
   record->body[0] = MSB(NTS_PROTOCOL_ID_NTPV4);
   record->body[1] = LSB(NTS_PROTOCOL_ID_NTPV4);

   //Adjust the length of the request
   length += sizeof(NtsKeRecord) + ntohs(record->bodyLength);

   //If the NTS Next Protocol Negotiation record offers Protocol ID 0 (for
   //NTPv4), then the AEAD Algorithm Negotiation record must be included
   //exactly once
   record = (NtsKeRecord *) (context->buffer + length);

   //Its body consists of a sequence of 16-bit unsigned integers in network
   //byte order
   record->type = HTONS(NTS_KE_CRITICAL | NTS_KE_RECORD_TYPE_AEAD_ALGO_NEGO);
   record->bodyLength = HTONS(sizeof(uint16_t));
   record->body[0] = MSB(AEAD_AES_SIV_CMAC_256);
   record->body[1] = LSB(AEAD_AES_SIV_CMAC_256);

   //Adjust the length of the request
   length += sizeof(NtsKeRecord) + ntohs(record->bodyLength);

   //The sequence shall be terminated by a End of Message record
   record = (NtsKeRecord *) (context->buffer + length);

   //The End of Message record has a Record Type number of 0 and a zero-length
   //body (refer to RFC 8915, section 4.1.1)
   record->type = HTONS(NTS_KE_CRITICAL | NTS_KE_RECORD_TYPE_END_OF_MESSAGE);
   record->bodyLength = HTONS(0);

   //Adjust the length of the request
   length += sizeof(NtsKeRecord) + ntohs(record->bodyLength);

   //Save the length of the NTS-KE request
   context->bufferLen = length;
   context->bufferPos = 0;

   //Debug message
   TRACE_INFO("Sending NTS-KE request (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump NTS-KE records for debugging purpose
   ntsDumpNtsKeRecords(context->buffer, length);

   //Sucessful processing
   return NO_ERROR;
}


/**
 * @brief Send NTS-KE request
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientSendNtsKeRequest(NtsClientContext *context)
{
   error_t error;
   size_t n;

   //Any remaining data to be sent?
   if(context->bufferPos < context->bufferLen)
   {
      //Send more data
      error = tlsWrite(context->tlsContext, context->buffer + context->bufferPos,
         context->bufferLen - context->bufferPos, &n, 0);

      //Check status code
      if(error == NO_ERROR || error == ERROR_TIMEOUT)
      {
         //Advance data pointer
         context->bufferPos += n;
      }
   }
   else
   {
      //The request has been successfully transmitted
      error = NO_ERROR;

      //Flush the receive buffer
      context->bufferLen = 0;
      context->bufferPos = 0;

      //The client must discard all old cookies and parameters
      context->cookieLen = 0;
      context->ntsNextProtoNegoRecordReceived = FALSE;
      context->aeadAlgoNegoRecordReceived = FALSE;

      //If no NTPv4 Server Negotiation record is sent, the client shall interpret
      //this as a directive to associate with an NTPv4 server at the same IP
      //address as the NTS-KE server (refer to RFC 8915, section 4.1.7)
      ipAddrToString(&context->ntsKeServerIpAddr, context->ntpServerName);

      //If no NTPv4 Port Negotiation record is sent, the client shall assume a
      //default of 123 (refer to RFC 8915, section 4.1.8)
      context->ntpServerPort = NTP_PORT;

      //Debug message
      TRACE_INFO("Receiving NTS-KE response...\r\n");

      //Wait for server's response
      ntsClientChangeState(context, NTS_CLIENT_STATE_NTS_KE_RECEIVING);
   }

   //Return status code
   return error;
}


/**
 * @brief Receive NTS-KE response
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientReceiveNtsKeResponse(NtsClientContext *context)
{
   error_t error;
   size_t n;
   uint16_t type;
   NtsKeRecord *record;

   //The server's response each shall consist of a sequence of records
   if(context->bufferPos < sizeof(NtsKeRecord))
   {
      //Read record header
      error = tlsRead(context->tlsContext, context->buffer + context->bufferPos,
         sizeof(NtsKeRecord) - context->bufferPos, &n, 0);

      //Check status code
      if(!error)
      {
         //Advance data pointer
         context->bufferPos += n;

         //Valid record header?
         if(context->bufferPos >= sizeof(NtsKeRecord))
         {
            //Point to the received record header
            record = (NtsKeRecord *) context->buffer;

            //Get the length of the record body
            n = ntohs(record->bodyLength);

            //Clients may enforce length limits on responses
            if((sizeof(NtsKeRecord) + n) <= NTS_CLIENT_BUFFER_SIZE)
            {
               //Save the total length of the record
               context->bufferLen = sizeof(NtsKeRecord) + n;
            }
            else
            {
               //The record is malformed
               error = ERROR_INVALID_SYNTAX;
            }
         }
      }
   }
   else if(context->bufferPos < context->bufferLen)
   {
      //Read record body
      error = tlsRead(context->tlsContext, context->buffer + context->bufferPos,
         context->bufferLen - context->bufferPos, &n, 0);

      //Check status code
      if(!error)
      {
         //Advance data pointer
         context->bufferPos += n;
      }
   }
   else
   {
      //Debug message
      TRACE_DEBUG("NTS-KE record received (%" PRIuSIZE " bytes)...\r\n",
         context->bufferLen);

      //Point to the received record
      record = (NtsKeRecord *) context->buffer;

      //Get record type
      type = ntohs(record->type) & NTS_KE_RECORD_TYPE_MASK;
      //Get body length
      n = ntohs(record->bodyLength);

      //Dump NTS-KE record for debugging purpose
      ntsDumpNtsKeRecord(record, context->bufferLen);

      //Check record type
      if(type == NTS_KE_RECORD_TYPE_END_OF_MESSAGE)
      {
         //Process End of Message record
         error = ntsClientParseEndOfMessageRecord(context, record->body, n);
      }
      else if(type == NTS_KE_RECORD_TYPE_NTS_NEXT_PROTO_NEGO)
      {
         //Process NTS Next Protocol Negotiation record
         error = ntsClientParseNtsNextProtoNegoRecord(context, record->body, n);
      }
      else if(type == NTS_KE_RECORD_TYPE_ERROR)
      {
         //Process Error record
         error = ntsClientParseErrorRecord(context, record->body, n);
      }
      else if(type == NTS_KE_RECORD_TYPE_WARNING)
      {
         //Process Warning record
         error = ntsClientParseWarningRecord(context, record->body, n);
      }
      else if(type == NTS_KE_RECORD_TYPE_AEAD_ALGO_NEGO)
      {
         //Process AEAD Algorithm Negotiation record
         error = ntsClientParseAeadAlgoNegoRecord(context, record->body, n);
      }
      else if(type == NTS_KE_RECORD_TYPE_NEW_COOKIE_FOR_NTPV4)
      {
         //Process New Cookie for NTPv4 record
         error = ntsClientParseNewCookieForNtpv4Record(context, record->body, n);
      }
      else if(type == NTS_KE_RECORD_TYPE_NTPV4_SERVER_NEGO)
      {
         //Process NTPv4 Server Negotiation record
         error = ntsClientParseNtpv4ServerRecord(context, record->body, n);
      }
      else if(type == NTS_KE_RECORD_TYPE_NTPV4_PORT_NEGO)
      {
         //Process NTPv4 Port Negotiation record
         error = ntsClientParseNtpv4PortRecord(context, record->body, n);
      }
      else
      {
         //Discard unknown records
         error = NO_ERROR;
      }

      //Flush the receive buffer
      context->bufferLen = 0;
      context->bufferPos = 0;
   }

   //Return status code
   return error;
}


/**
 * @brief Parse End of Message record
 * @param[in] context Pointer to the NTS client context
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 * @return Error code
 **/

error_t ntsClientParseEndOfMessageRecord(NtsClientContext *context,
   const uint8_t *body, size_t length)
{
   error_t error;
   uint8_t contextValue[5];

   //The NTS Next Protocol Negotiation record must be included exactly once
   if(!context->ntsNextProtoNegoRecordReceived)
      return ERROR_INVALID_SYNTAX;

   //If the NTS Next Protocol Negotiation record offers Protocol ID 0 (for
   //NTPv4), then the AEAD Algorithm Negotiation record record must be included
   //exactly once
   if(!context->aeadAlgoNegoRecordReceived)
      return ERROR_INVALID_SYNTAX;

   //Servers must send at least one New Cookie for NTPv4 record (refer to
   //RFC 8915, section 4.1.6)
   if(context->cookieLen == 0)
      return ERROR_WRONG_COOKIE;

   //The per-association context value shall consist of the five octets
   contextValue[0] = MSB(NTS_PROTOCOL_ID_NTPV4);
   contextValue[1] = LSB(NTS_PROTOCOL_ID_NTPV4);
   contextValue[2] = MSB(AEAD_AES_SIV_CMAC_256);
   contextValue[3] = LSB(AEAD_AES_SIV_CMAC_256);

   //The final octet shall be 0x00 for the C2S key
   contextValue[4] = 0x00;

   //Extract the client-to-server (C2S) key
   error = tlsExportKeyingMaterial(context->tlsContext,
      "EXPORTER-network-time-security", TRUE, contextValue,
      sizeof(contextValue), context->c2sKey, 32);
   //Failed to extract C2S key?
   if(error)
      return error;

   //The final octet shall be 0x01 for the S2C key
   contextValue[4] = 0x01;

   //Extract the server-to-client (S2C) key
   error = tlsExportKeyingMaterial(context->tlsContext,
      "EXPORTER-network-time-security", TRUE, contextValue,
      sizeof(contextValue), context->s2cKey, 32);
   //Failed to extract S2C key?
   if(error)
      return error;

   //Save TLS session
   error = tlsSaveSessionState(context->tlsContext, &context->tlsSession);
   //Any error to report?
   if(error)
      return error;

   //After sending their respective request and response, the client and server
   //shall send TLS "close_notify" alerts (refer to RFC 8915, section 4)
   ntsClientChangeState(context, NTS_CLIENT_STATE_NTS_KE_DISCONNECTING);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Parse NTS Next Protocol Negotiation record
 * @param[in] context Pointer to the NTS client context
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 * @return Error code
 **/

error_t ntsClientParseNtsNextProtoNegoRecord(NtsClientContext *context,
   const uint8_t *body, size_t length)
{
   error_t error;

   //The NTS Next Protocol Negotiation record must be included exactly once
   if(context->ntsNextProtoNegoRecordReceived)
   {
      //The response includes a duplicate record
      error = ERROR_INVALID_SYNTAX;
   }
   else
   {
      //The response includes a NTS Next Protocol Negotiation record
      context->ntsNextProtoNegoRecordReceived = TRUE;

      //Protocol IDs listed in the server's response must comprise a subset of
      //those listed in the request (refer to RFC 8915, section 4.1.2)
      if(length == sizeof(uint16_t) &&
         body[0] == MSB(NTS_PROTOCOL_ID_NTPV4) &&
         body[1] == LSB(NTS_PROTOCOL_ID_NTPV4))
      {
         error = NO_ERROR;
      }
      else
      {
         error = ERROR_INVALID_PROTOCOL;
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Parse Error record
 * @param[in] context Pointer to the NTS client context
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 * @return Error code
 **/

error_t ntsClientParseErrorRecord(NtsClientContext *context,
   const uint8_t *body, size_t length)
{
   //If clients receive a server response that includes an Error record, they
   //must discard any key material negotiated during the initial TLS exchange
   //and must not proceed to the Next Protocol (refer to RFC 8915, section 4.1.3)
   return ERROR_UNEXPECTED_RESPONSE;
}


/**
 * @brief Parse Warning record
 * @param[in] context Pointer to the NTS client context
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 * @return Error code
 **/

error_t ntsClientParseWarningRecord(NtsClientContext *context,
   const uint8_t *body, size_t length)
{
   //If clients receive a server response that includes a Warning record, they
   //may discard any negotiated key material and abort without proceeding to
   //the Next Protocol
   return NO_ERROR;
}


/**
 * @brief Parse AEAD Algorithm Negotiation record
 * @param[in] context Pointer to the NTS client context
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 * @return Error code
 **/

error_t ntsClientParseAeadAlgoNegoRecord(NtsClientContext *context,
   const uint8_t *body, size_t length)
{
   error_t error;

   //If the NTS Next Protocol Negotiation record offers Protocol ID 0 (for
   //NTPv4), then this record must be included exactly once
   if(context->aeadAlgoNegoRecordReceived)
   {
      //The response includes a duplicate record
      error = ERROR_INVALID_SYNTAX;
   }
   else
   {
      //The response includes an AEAD Algorithm Negotiation record
      context->aeadAlgoNegoRecordReceived = TRUE;

      //When included in a response, the AEAD Algorithm Negotiation record
      //denotes which algorithm the server chooses to use. It is empty if the
      //server supports none of the algorithms offered (refer to RFC 8915,
      //section 4.1.5)
      if(length == sizeof(uint16_t) &&
         body[0] == MSB(AEAD_AES_SIV_CMAC_256) &&
         body[1] == LSB(AEAD_AES_SIV_CMAC_256))
      {
         error = NO_ERROR;
      }
      else
      {
         error = ERROR_UNSUPPORTED_ALGO;
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Parse New Cookie for NTPv4 record
 * @param[in] context Pointer to the NTS client context
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 * @return Error code
 **/

error_t ntsClientParseNewCookieForNtpv4Record(NtsClientContext *context,
   const uint8_t *body, size_t length)
{
   //Malformed record?
   if(length == 0)
      return ERROR_INVALID_SYNTAX;

   //The implementation limits the size of NTS cookies
   if(length > NTS_CLIENT_MAX_COOKIE_SIZE)
      return ERROR_BUFFER_OVERFLOW;

   //The server may send multiple cookies
   if(context->cookieLen == 0)
   {
      //The contents of its body shall be implementation-defined, and clients
      //must not attempt to interpret them (refer to RFC 8915, section 4.1.6)
      osMemcpy(context->cookie, body, length);

      //Save the size of the NTS cookie
      context->cookieLen = length;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Parse NTPv4 Server Negotiation record
 * @param[in] context Pointer to the NTS client context
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 * @return Error code
 **/

error_t ntsClientParseNtpv4ServerRecord(NtsClientContext *context,
   const uint8_t *body, size_t length)
{
   //Malformed record?
   if(length == 0)
      return ERROR_INVALID_SYNTAX;

   //The implementation limits the length of NTP server names
   if(length > NTS_CLIENT_MAX_NTP_SERVER_NAME_LEN)
      return ERROR_BUFFER_OVERFLOW;

   //The body consists of an ASCII-encoded string. The contents of the string
   //shall be either an IPv4 address, an IPv6 address, or a fully qualified
   //domain name (refer to RFC 8915, section 4.1.7)
   osMemcpy(context->ntpServerName, body, length);

   //Properly terminate the string with a NULL character
   context->ntpServerName[length] = '\0';

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Parse NTPv4 Port Negotiation record
 * @param[in] context Pointer to the NTS client context
 * @param[in] body Pointer to the record body
 * @param[in] length Length of the record body, in bytes
 * @return Error code
 **/

error_t ntsClientParseNtpv4PortRecord(NtsClientContext *context,
   const uint8_t *body, size_t length)
{
   //Malformed record?
   if(length != sizeof(uint16_t))
      return ERROR_INVALID_SYNTAX;

   //The body consists of a 16-bit unsigned integer in network byte order,
   //denoting a UDP port number (refer to RFC 8915, section 4.1.8)
   context->ntpServerPort = LOAD16BE(body);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Shutdown NTS-KE connection
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientShutdownNtsKeConnection(NtsClientContext *context)
{
   error_t error;

   //After sending their respective request and response, the client and server
   //shall send TLS "close_notify" alerts
   error = tlsShutdown(context->tlsContext);

   //Check status code
   if(!error)
   {
      //Valid TCP socket?
      if(context->ntsKeSocket != NULL)
      {
         //Shutdown TCP connection
         error = socketShutdown(context->ntsKeSocket, SOCKET_SD_BOTH);
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Close NTS-KE connection
 * @param[in] context Pointer to the NTS client context
 **/

void ntsClientCloseNtsKeConnection(NtsClientContext *context)
{
   //Valid TLS context?
   if(context->tlsContext != NULL)
   {
      //Release TLS context
      tlsFree(context->tlsContext);
      context->tlsContext = NULL;
   }

   //Valid socket?
   if(context->ntsKeSocket != NULL)
   {
      //Close NTS-KE socket
      socketClose(context->ntsKeSocket);
      context->ntsKeSocket = NULL;
   }
}


/**
 * @brief Determine whether a timeout error has occurred (NTS-KE phase)
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientCheckNtsKeTimeout(NtsClientContext *context)
{
#if (NET_RTOS_SUPPORT == DISABLED)
   error_t error;
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //Check whether the timeout has elapsed
   if(timeCompare(time, context->timestamp + context->timeout) >= 0)
   {
      //Report a timeout error
      error = ERROR_TIMEOUT;
   }
   else
   {
      //The operation would block
      error = ERROR_WOULD_BLOCK;
   }

   //Return status code
   return error;
#else
   //Report a timeout error
   return ERROR_TIMEOUT;
#endif
}


/**
 * @brief Open NTP connection
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientOpenNtpConnection(NtsClientContext *context)
{
   error_t error;

   //Generate a unique identifier
   error = context->randCallback(context->uniqueId, NTS_CLIENT_UNIQUE_ID_SIZE);
   //Any error to report?
   if(error)
      return error;

   //Generate a nonce
   error = context->randCallback(context->nonce, NTS_CLIENT_NONCE_SIZE);
   //Any error to report?
   if(error)
      return error;

   //Open a UDP socket
   context->ntpSocket = socketOpen(SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);
   //Failed to open socket?
   if(context->ntpSocket == NULL)
      return ERROR_OPEN_FAILED;

   //Associate the socket with the relevant interface
   error = socketBindToInterface(context->ntpSocket, context->interface);
   //Any error to report?
   if(error)
      return error;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Send NTP request to the server
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientSendNtpRequest(NtsClientContext *context)
{
   error_t error;
   size_t length;
   NtpHeader *header;
   NtpExtension *extension;
   NtpNtsAeadExtension *ntsAeadExtension;
   DataChunk ad[2];

   //Point to the buffer where to format the NTP message
   header = (NtpHeader *) context->buffer;

   //The client initializes the NTP message header. For this purpose, all
   //the NTP header fields are set to 0, except the Mode, VN, and optional
   //Transmit Timestamp fields
   osMemset(header, 0, sizeof(NtpHeader));

   //Format NTP request
   header->vn = NTP_VERSION_4;
   header->mode = NTP_MODE_CLIENT;

   //Time at which the NTP request was sent
   context->retransmitStartTime = osGetSystemTime();

   //The Transmit Timestamp allows a simple calculation to determine the
   //propagation delay between the server and client and to align the system
   //clock generally within a few tens of milliseconds relative to the server
   header->transmitTimestamp.seconds = 0;
   header->transmitTimestamp.fraction = 0;

   //The NTP header is 48 octets long
   length = sizeof(NtpHeader);

   //The Unique Identifier extension field provides the client with a
   //cryptographically strong means of detecting replayed packets (refer to
   //RFC 8915, section 5.3)
   extension = (NtpExtension *) (context->buffer + length);
   extension->fieldType = HTONS(NTP_EXTENSION_TYPE_UNIQUE_ID);

   //The string must be at least 32 octets long
   osMemcpy(extension->value, context->uniqueId, NTS_CLIENT_UNIQUE_ID_SIZE);

   //Calculate the length of the extension
   extension->length = HTONS(sizeof(NtpExtension) + NTS_CLIENT_UNIQUE_ID_SIZE);
   //Adjust the length of the packet
   length += ntohs(extension->length);

   //The purpose of the NTS Cookie extension is to carry information that
   //enables the server to recompute keys and other session state without
   //having to store any per-client state (refer to RFC 8915, section 5.4)
   extension = (NtpExtension *) (context->buffer + length);
   extension->fieldType = HTONS(NTP_EXTENSION_TYPE_NTS_COOKIE);

   //The cookie must be one which has been previously provided to the client,
   //either from the key establishment server during the NTS-KE handshake or
   //from the NTP server in response to a previous NTS-protected NTP request
   //(refer to RFC 8915, section 5.7)
   osMemcpy(extension->value, context->cookie, context->cookieLen);

   //Calculate the length of the extension
   extension->length = htons(sizeof(NtpExtension) + context->cookieLen);
   //Adjust the length of the packet
   length += ntohs(extension->length);

   //The NTS Authenticator and Encrypted Extension Fields extension is the
   //central cryptographic element of an NTS-protected NTP packet (refer to
   //RFC 8915, section 5.6)
   ntsAeadExtension = (NtpNtsAeadExtension *) (context->buffer + length);
   ntsAeadExtension->fieldType = HTONS(NTP_EXTENSION_TYPE_NTS_AEAD);
   ntsAeadExtension->nonceLength = HTONS(NTS_CLIENT_NONCE_SIZE);
   ntsAeadExtension->ciphertextLength = HTONS(SIV_IV_LEN);

   //Copy the nonce
   osMemcpy(ntsAeadExtension->nonce, context->nonce, NTS_CLIENT_NONCE_SIZE);

   //The associated data shall consist of the portion of the NTP packet
   //beginning from the start of the NTP header and ending at the end of the
   //last extension field that precedes the NTS Authenticator and Encrypted
   //Extension Fields extension field (refer to RFC 8915, section 5.6)
   ad[0].buffer = context->buffer;
   ad[0].length = length;
   ad[1].buffer = context->nonce;
   ad[1].length = NTS_CLIENT_NONCE_SIZE;

   //The Ciphertext field is the output of the negotiated AEAD algorithm
   error = sivEncrypt(AES_CIPHER_ALGO, context->c2sKey, 32, ad, arraysize(ad),
      NULL, NULL, 0, ntsAeadExtension->nonce + NTS_CLIENT_NONCE_SIZE);
   //AEAD encryption failed?
   if(error)
      return error;

   //Calculate the length of the extension
   ntsAeadExtension->length = htons(sizeof(NtpNtsAeadExtension) +
      NTS_CLIENT_NONCE_SIZE + SIV_IV_LEN);

   //Adjust the length of the packet
   length += ntohs(ntsAeadExtension->length);
   //Save the length of the resulting NTP request
   context->bufferLen = length;

   //Debug message
   TRACE_INFO("Sending NTP request message (%" PRIuSIZE " bytes)...\r\n",
      context->bufferLen);

   //Dump the contents of the NTP packet for debugging purpose
   ntpDumpPacket(header, context->bufferLen);

   //Send the NTP request to the designated server
   error = socketSendTo(context->ntpSocket, &context->ntpServerIpAddr,
      context->ntpServerPort, context->buffer, context->bufferLen,
      NULL, 0);

   //Check status code
   if(!error)
   {
      //Wait for server's response
      ntsClientChangeState(context, NTS_CLIENT_STATE_NTP_RECEIVING);
   }

   //Return status code
   return error;
}


/**
 * @brief Wait for NTP response
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientReceiveNtpResponse(NtsClientContext *context)
{
   error_t error;
   systime_t t1;
   systime_t t2;
   systime_t time;
   IpAddr ipAddr;
   uint16_t port;

   //Get current time
   time = osGetSystemTime();

   //Compute request timeout
   if(timeCompare(context->startTime + context->timeout, time) > 0)
   {
      t1 = context->startTime + context->timeout - time;
   }
   else
   {
      t1 = 0;
   }

   //Compute retransmission timeout
   if(timeCompare(context->retransmitStartTime + context->retransmitTimeout, time) > 0)
   {
      t2 = context->retransmitStartTime + context->retransmitTimeout - time;
   }
   else
   {
      t2 = 0;
   }

   //Adjust receive timeout
   error = socketSetTimeout(context->ntpSocket, MIN(t1, t2));

   //Check status code
   if(!error)
   {
      //Wait for server's response
      error = socketReceiveFrom(context->ntpSocket, &ipAddr, &port,
         context->buffer, NTS_CLIENT_BUFFER_SIZE, &context->bufferLen, 0);
   }

   //Any datagram received?
   if(error == NO_ERROR)
   {
      //Decrypt NTP response
      error = ntsClientDecryptNtpResponse(context, &ipAddr, port,
         context->buffer, context->bufferLen);

      //Check status code
      if(!error)
      {
         //A valid NTP response has been received
         ntsClientChangeState(context, NTS_CLIENT_STATE_COMPLETE);
      }
      else
      {
         //Silently discard invalid NTP packets
         error = ntsClientCheckNtpTimeout(context);
      }
   }
   else if(error == ERROR_WOULD_BLOCK || error == ERROR_TIMEOUT)
   {
      //Check whether the timeout has elapsed
      error = ntsClientCheckNtpTimeout(context);
   }
   else
   {
      //A communication error has occurred
   }

   //Return status code
   return error;
}


/**
 * @brief Decrypt NTP response
 * @param[in] context Pointer to the NTS client context
 * @param[in] ipAddr Remote IP address
 * @param[in] port Remote port number
 * @param[in] message Pointer to the NTP message
 * @param[in] length Length of the NTP message, in bytes
 * @return Error code
 **/

error_t ntsClientDecryptNtpResponse(NtsClientContext *context,
   const IpAddr *ipAddr, uint16_t port, const uint8_t *message,
   size_t length)
{
   error_t error;
   size_t n;
   size_t nonceLen;
   const NtpHeader *header;
   const NtpExtension *uniqueIdExtension;
   const NtpNtsAeadExtension *ntsAeadExtension;
   const NtpExtension *ntsCookieExtension;
   const uint8_t *iv;
   const uint8_t *ciphertext;
   uint8_t *plaintext;
   DataChunk ad[2];

   //Ensure the NTP packet is valid
   if(length < sizeof(NtpHeader))
      return ERROR_INVALID_MESSAGE;

   //Point to the NTP response
   header = (NtpHeader *) context->buffer;

   //Debug message
   TRACE_INFO("NTP response message received (%" PRIuSIZE " bytes)...\r\n",
      length);

   //Dump the contents of the NTP packet for debugging purpose
   ntpDumpPacket(header, length);

   //Check NTP version
   if(header->vn != NTP_VERSION_4)
      return ERROR_INVALID_MESSAGE;

   //The server reply should be discarded if the Transmit Timestamp fields is 0
   if(header->transmitTimestamp.seconds == 0 &&
      header->transmitTimestamp.fraction == 0)
   {
      return ERROR_INVALID_MESSAGE;
   }

   //The server reply should be discarded if the Mode field is not 4 (unicast)
   //or 5 (broadcast)
   if(header->mode != NTP_MODE_SERVER && header->mode != NTP_MODE_BROADCAST)
      return ERROR_INVALID_MESSAGE;

   //The Originate Timestamp in the server reply should match the Transmit
   //Timestamp used in the client request
   if(header->originateTimestamp.seconds != 0 ||
      header->originateTimestamp.fraction != 0)
   {
      return ERROR_INVALID_MESSAGE;
   }

   //Determine the total length occupied by the NTP extensions
   length -= sizeof(NtpHeader);

   //All server packets generated by NTS-implementing servers in response to
   //client packets containing the Unique Identifier extension field must also
   //contain this field with the same content as in the client's request (refer
   //to RFC 8915, section 5.3)
   uniqueIdExtension = ntpGetExtension(header->extensions, length,
      NTP_EXTENSION_TYPE_UNIQUE_ID, 0);

   //Exactly one Unique Identifier extension must be present
   if(ntpGetExtension(header->extensions, length, NTP_EXTENSION_TYPE_UNIQUE_ID,
      1) != NULL || uniqueIdExtension == NULL)
   {
      return ERROR_INVALID_MESSAGE;
   }

   //Retrieve the length of the octet string
   n = ntohs(uniqueIdExtension->length) - sizeof(NtpExtension);

   //Upon receiving an NTS-protected response, the client must verify that the
   //Unique Identifier matches that of an outstanding request (refer to
   //RFC 8915, section 5.7)
   if(n != NTS_CLIENT_UNIQUE_ID_SIZE ||
      osMemcmp(uniqueIdExtension->value, context->uniqueId, n) != 0)
   {
      return ERROR_INVALID_MESSAGE;
   }

   //Search for the NTS Authenticator and Encrypted Extension Fields extension
   ntsAeadExtension = (NtpNtsAeadExtension *) ntpGetExtension(header->extensions,
      length, NTP_EXTENSION_TYPE_NTS_AEAD, 0);

   //Exactly one NTS Authenticator and Encrypted Extension Fields extension
   //must be present
   if(ntpGetExtension(header->extensions, length, NTP_EXTENSION_TYPE_NTS_AEAD,
      1) != NULL || ntsAeadExtension == NULL)
   {
      return ERROR_INVALID_MESSAGE;
   }

   //Malformed NTS Authenticator and Encrypted Extension Fields extension?
   if(ntohs(ntsAeadExtension->length) < sizeof(NtpNtsAeadExtension))
   {
      return ERROR_INVALID_MESSAGE;
   }

   //Retrieve the length of the nonce and ciphertext
   nonceLen = ntohs(ntsAeadExtension->nonceLength);
   n = ntohs(ntsAeadExtension->ciphertextLength);

   //Malformed NTS Authenticator and Encrypted Extension Fields extension?
   if(ntohs(ntsAeadExtension->length) < (sizeof(NtpNtsAeadExtension) +
      nonceLen + n))
   {
      return ERROR_INVALID_MESSAGE;
   }

   //Check the length of the ciphertext
   if(n < SIV_IV_LEN)
      return ERROR_INVALID_MESSAGE;

   //Point to the synthetic IV
   iv = ntsAeadExtension->nonce + nonceLen;

   //Point to the ciphertext
   ciphertext = iv + SIV_IV_LEN;
   n -= SIV_IV_LEN;

   //Point to the buffer where to store the resulting plaintext
   plaintext = (uint8_t *) ciphertext;

   //The associated data shall consist of the portion of the NTP packet
   //beginning from the start of the NTP header and ending at the end of the
   //last extension field that precedes the NTS Authenticator and Encrypted
   //Extension Fields extension field (refer to RFC 8915, section 5.6)
   ad[0].buffer = context->buffer;
   ad[0].length = (uint8_t *) ntsAeadExtension - context->buffer;
   ad[1].buffer = ntsAeadExtension->nonce;
   ad[1].length = nonceLen;

   //The Unique Identifier extension field must be authenticated but must not
   //be encrypted
   if((uint8_t *) uniqueIdExtension >= (uint8_t *) ntsAeadExtension)
      return ERROR_INVALID_MESSAGE;

   //The client must verify that the packet is authentic under the S2C key
   //associated with that request (refer to RFC 8915, section 5.7)
   error = sivDecrypt(AES_CIPHER_ALGO, context->s2cKey, 32, ad, arraysize(ad),
      ciphertext, plaintext, n, iv);
   //AEAD decryption failed?
   if(error)
      return ERROR_INVALID_MESSAGE;

   //Debug message
   TRACE_DEBUG("Plaintext (%" PRIuSIZE " bytes):\r\n", n);
   //Dump plaintext for debugging purpose
   ntpDumpExtensions(plaintext, n);

   //One or more NTS Cookie extension fields must be authenticated and
   //encrypted (refer to RFC 8915, section 5.7)
   ntsCookieExtension = ntpGetExtension(plaintext, n,
      NTP_EXTENSION_TYPE_NTS_COOKIE, 0);

   //NTS Cookie extension not found?
   if(ntsCookieExtension == NULL)
      return ERROR_MISSING_EXTENSION;

   //Retrieve the length of the cookie
   n = ntohs(ntsCookieExtension->length) - sizeof(NtpExtension);

   //The implementation limits the size of NTS cookies
   if(n > NTS_CLIENT_MAX_COOKIE_SIZE)
      return ERROR_BUFFER_OVERFLOW;

   //The contents of its body shall be implementation-defined, and clients
   //must not attempt to interpret them (refer to RFC 8915, section 4.1.6)
   osMemcpy(context->cookie, ntsCookieExtension->value, n);

   //Save the size of the NTS cookie
   context->cookieLen = n;

   //The NTP response message is acceptable
   return NO_ERROR;
}


/**
 * @brief Parse NTP response
 * @param[in] context Pointer to the NTS client context
 * @param[out] timestamp Pointer to the NTP timestamp
 * @return Error code
 **/

error_t ntsClientParseNtpResponse(NtsClientContext *context,
   NtpTimestamp *timestamp)
{
   NtpHeader *header;

   //Ensure the NTP packet is valid
   if(context->bufferLen < sizeof(NtpHeader))
      return ERROR_INVALID_LENGTH;

   //Point to the NTP response
   header = (NtpHeader *) context->buffer;

   //Clear kiss code
   context->kissCode = 0;

   //Kiss-of-Death packet received?
   if(header->stratum == 0)
   {
      //The kiss code is encoded in four-character ASCII strings left
      //justified and zero filled
      context->kissCode = htonl(header->referenceId);

      //An NTS client should stop sending to a particular server if that
      //server returns a reply with a Stratum field of 0
      return ERROR_REQUEST_REJECTED;
   }

   //Extract NTP timestamp from server's response
   timestamp->seconds = ntohl(header->transmitTimestamp.seconds);
   timestamp->fraction = ntohl(header->transmitTimestamp.fraction);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Close NTP connection
 * @param[in] context Pointer to the NTS client context
 **/

void ntsClientCloseNtpConnection(NtsClientContext *context)
{
   //Valid socket?
   if(context->ntpSocket != NULL)
   {
      //Close UDP socket
      socketClose(context->ntpSocket);
      context->ntpSocket = NULL;
   }
}


/**
 * @brief Determine whether a timeout error has occurred (NTP phase)
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientCheckNtpTimeout(NtsClientContext *context)
{
   error_t error;
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //Check whether the timeout has elapsed
   if(timeCompare(time, context->startTime + context->timeout) >= 0)
   {
      //Report a timeout error
      error = ERROR_TIMEOUT;
   }
   else if(timeCompare(time, context->retransmitStartTime + context->retransmitTimeout) >= 0)
   {
      //The timeout value is doubled for each subsequent retransmission
      context->retransmitTimeout = MIN(context->retransmitTimeout * 2,
         NTS_CLIENT_MAX_NTP_RETRANSMIT_TIMEOUT);

      //Retransmit NTP request
      ntsClientChangeState(context, NTS_CLIENT_STATE_NTP_SENDING);

      //Continue processing
      error = NO_ERROR;
   }
   else
   {
#if (NET_RTOS_SUPPORT == ENABLED)
      //Report a timeout error
      error = ERROR_TIMEOUT;
#else
      //The operation would block
      error = ERROR_WOULD_BLOCK;
#endif
   }

   //Return status code
   return error;
}

#endif
