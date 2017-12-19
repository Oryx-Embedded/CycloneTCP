/**
 * @file sntp_client.c
 * @brief SNTP client (Simple Network Time Protocol)
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
 * The Simple Network Time Protocol is used to synchronize computer clocks
 * in the Internet. Refer to RFC 4330 for more details
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.8.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL SNTP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "sntp/sntp_client.h"
#include "error.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SNTP_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Retrieve current time from NTP server using SNTP protocol
 * @param[in] interface Underlying network interface (optional parameter)
 * @param[in] serverIpAddr IP address of the NTP server
 * @param[out] timestamp Current time
 * @return Error code
 **/

error_t sntpClientGetTimestamp(NetInterface *interface,
   const IpAddr *serverIpAddr, NtpTimestamp *timestamp)
{
   error_t error;
   uint_t i;
   systime_t timeout;
   SntpClientContext context;

   //Check parameters
   if(serverIpAddr == NULL || timestamp == NULL)
      return ERROR_INVALID_PARAMETER;

   //Use default network interface?
   if(interface == NULL)
      interface = netGetDefaultInterface();

   //Open a UDP socket
   context.socket = socketOpen(SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);
   //Failed to open socket?
   if(!context.socket)
      return ERROR_OPEN_FAILED;

   //Associate the socket with the relevant interface
   error = socketBindToInterface(context.socket, interface);
   //Any error to report?
   if(error)
   {
      //Close socket
      socketClose(context.socket);
      //Return status code
      return error;
   }

   //Only accept datagrams from the specified NTP server
   error = socketConnect(context.socket, serverIpAddr, NTP_PORT);
   //Any error to report?
   if(error)
   {
      //Close socket
      socketClose(context.socket);
      //Return status code
      return error;
   }

   //Initial timeout value
   timeout = SNTP_CLIENT_INIT_TIMEOUT;

   //Retransmission loop
   for(i = 0; i < SNTP_CLIENT_MAX_RETRIES; i++)
   {
      //Send NTP request message
      error = sntpSendRequest(&context);
      //Failed to send message ?
      if(error)
         break;

      //Wait for a valid NTP response message
      error = sntpWaitForResponse(&context, timeout);
      //Valid NTP response received?
      if(!error)
         break;

      //The timeout value is doubled for each subsequent retransmission
      timeout = MIN(timeout * 2, SNTP_CLIENT_MAX_TIMEOUT);
   }

   //Successful processing?
   if(!error)
   {
      //Save server timestamp
      timestamp->seconds = ntohl(context.message.transmitTimestamp.seconds);
      timestamp->fraction = ntohl(context.message.transmitTimestamp.fraction);
   }

   //Close socket
   socketClose(context.socket);
   //Return status code
   return error;
}


/**
 * @brief Send NTP request using SNTP protocol
 * @param[in] context SNTP client context
 * @return Error code
 **/

error_t sntpSendRequest(SntpClientContext *context)
{
   size_t length;
   NtpHeader *message;

   //Point to the buffer where to format the NTP message
   message = &context->message;

   //Clear NTP message
   memset(message, 0, sizeof(NtpHeader));

   //Format NTP request
   message->vn = NTP_VERSION_3;
   message->mode = NTP_MODE_CLIENT;

   //Time at which the NTP request was sent
   context->t1 = osGetSystemTime();

   //The Transmit Timestamp allows a simple calculation to determine the
   //propagation delay between the server and client and to align the system
   //clock generally within a few tens of milliseconds relative to the server
   message->transmitTimestamp.seconds = 0;
   message->transmitTimestamp.fraction = htonl(context->t1);

   //Length of the NTP request
   length = sizeof(NtpHeader);

   //Debug message
   TRACE_INFO("Sending NTP request message (%" PRIuSIZE " bytes)...\r\n", sizeof(NtpHeader));
   //Dump NTP message
   sntpDumpMessage(message, length);

   //Send NTP request
   return socketSend(context->socket, message, length, NULL, 0);
}


/**
 * @brief Wait for a valid response from the NTP server
 * @param[in] context Pointer to the SNTP client context
 * @param[in] timeout Maximum time period to wait
 * @return Error code
 **/

error_t sntpWaitForResponse(SntpClientContext *context, systime_t timeout)
{
   error_t error;
   size_t length;
   systime_t elapsedTime;

   //Time elapsed since the NTP request was sent
   elapsedTime = 0;

   //Keep listening as long as the retransmission timeout has not been reached
   while(elapsedTime < timeout)
   {
      //Adjust receive timeout
      error = socketSetTimeout(context->socket, timeout - elapsedTime);
      //Any error to report?
      if(error)
         break;

      //Wait for a response from the NTP server
      error = socketReceive(context->socket, &context->message,
         sizeof(NtpHeader), &length, 0);

      //Any datagram received?
      if(!error)
      {
         //Time at which the response was received
         context->t4 = osGetSystemTime();

         //Parse incoming datagram
         error = sntpParseResponse(context, &context->message, length);
         //Valid NTP response message?
         if(!error)
            return NO_ERROR;
      }

      //Compute the time elapsed since the NTP request was sent
      elapsedTime = osGetSystemTime() - context->t1;
   }

   //The timeout period elapsed
   return ERROR_TIMEOUT;
}


/**
 * @brief Parse NTP server response
 * @param[in] context Pointer to the SNTP client context
 * @param[in] message NTP response message to parse
 * @param[in] length Length of the incoming NTP message
 * @return Error code
 **/

error_t sntpParseResponse(SntpClientContext *context,
   const NtpHeader *message, size_t length)
{
   //Ensure the NTP message is valid
   if(length < sizeof(NtpHeader))
      return ERROR_INVALID_MESSAGE;

   //Debug message
   TRACE_INFO("NTP response message received (%" PRIuSIZE " bytes)...\r\n", length);
   //Dump NTP message
   sntpDumpMessage(message, length);

   //The server reply should be discarded if any of the VN, Stratum,
   //or Transmit Timestamp fields is 0
   if(!message->vn || !message->stratum)
      return ERROR_INVALID_MESSAGE;
   if(!message->transmitTimestamp.seconds || !message->transmitTimestamp.fraction)
      return ERROR_INVALID_MESSAGE;

   //The server reply should be discarded if the Mode field is
   //not 4 (unicast) or 5 (broadcast)
   if(message->mode != NTP_MODE_SERVER && message->mode != NTP_MODE_BROADCAST)
      return ERROR_INVALID_MESSAGE;

   //The Originate Timestamp in the server reply should match the
   //Transmit Timestamp used in the client request
   if(message->originateTimestamp.seconds != 0)
      return ERROR_INVALID_TIMESTAMP;
   if(message->originateTimestamp.fraction != htonl(context->t1))
      return ERROR_INVALID_TIMESTAMP;

   //The NTP response message is acceptable
   return NO_ERROR;
}


/**
 * @brief Dump NTP message for debugging purpose
 * @param[in] message Pointer to the NTP message
 * @param[in] length Length of the NTP message
 **/

void sntpDumpMessage(const NtpHeader *message, size_t length)
{
#if (SNTP_TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   uint8_t *p;
   NtpAuthData *authData;

   //Point to the beginning of the message
   p = (uint8_t *) message;

   //Check message length
   if(length >= sizeof(NtpHeader))
   {
      //Dump NTP message
      TRACE_DEBUG("  Mode = %" PRIu8 "\r\n", message->mode);
      TRACE_DEBUG("  Version = %" PRIu8 "\r\n", message->vn);
      TRACE_DEBUG("  Leap indicator = %" PRIu8 "\r\n", message->li);
      TRACE_DEBUG("  Stratum = %" PRIu8 "\r\n", message->stratum);
      TRACE_DEBUG("  Poll = %" PRIu8 "\r\n", message->poll);
      TRACE_DEBUG("  Precision = %" PRId8 "\r\n", message->precision);
      TRACE_DEBUG("  Root Delay = %" PRIu32 "\r\n", ntohl(message->rootDelay));
      TRACE_DEBUG("  Root Dispersion = %" PRIu32 "\r\n", ntohl(message->rootDispersion));
      TRACE_DEBUG("  Reference Identifier = %" PRIu32 "\r\n", ntohl(message->referenceIdentifier));

      //Dump reference timestamp
      TRACE_DEBUG("  ReferenceTimestamp\r\n");
      sntpDumpTimestamp(&message->referenceTimestamp);

      //Dump originate timestamp
      TRACE_DEBUG("  Originate Timestamp\r\n");
      sntpDumpTimestamp(&message->originateTimestamp);

      //Dump receive timestamp
      TRACE_DEBUG("  Receive Timestamp\r\n");
      sntpDumpTimestamp(&message->receiveTimestamp);

      //Dump transmit timestamp
      TRACE_DEBUG("  Transmit Timestamp\r\n");
      sntpDumpTimestamp(&message->transmitTimestamp);

      //Advance data pointer
      p += sizeof(NtpHeader);
      length -= sizeof(NtpHeader);

      //Any authentication data?
      if(length >= sizeof(NtpAuthData))
      {
         //Point to the beginning of the authentication data
         authData = (NtpAuthData *) p;

         //Dump transmit timestamp
         TRACE_DEBUG("  Key Identifier = %" PRIu32 "\r\n", ntohl(authData->keyIdentifier));
         //Dump message digest
         TRACE_DEBUG("  Message Digest\r\n");
         TRACE_DEBUG_ARRAY("    ", authData->messageDigest, 16);
      }
   }
#endif
}


/**
 * @brief Dump NTP timestamp
 * @param[in] timestamp Pointer to the NTP timestamp
 **/

void sntpDumpTimestamp(const NtpTimestamp *timestamp)
{
   //Dump seconds
   TRACE_DEBUG("    Seconds = %" PRIu32 "\r\n", ntohl(timestamp->seconds));
   //Dump fraction field
   TRACE_DEBUG("    Fraction = %" PRIu32 "\r\n", ntohl(timestamp->fraction));
}

#endif
