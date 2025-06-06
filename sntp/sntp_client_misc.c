/**
 * @file sntp_client_misc.c
 * @brief Helper functions for SNTP client
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
#define TRACE_LEVEL SNTP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "sntp/sntp_client.h"
#include "sntp/sntp_client_misc.h"
#include "ntp/ntp_debug.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SNTP_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Open UDP connection
 * @param[in] context Pointer to the SNTP client context
 * @return Error code
 **/

error_t sntpClientOpenConnection(SntpClientContext *context)
{
   error_t error;

   //Open a UDP socket
   context->socket = socketOpen(SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);

   //Valid socket?
   if(context->socket != NULL)
   {
      //Associate the socket with the relevant interface
      error = socketBindToInterface(context->socket, context->interface);
   }
   else
   {
      //Report an error
      error = ERROR_OPEN_FAILED;
   }

   //Return status code
   return error;
}


/**
 * @brief Close UDP connection
 * @param[in] context Pointer to the SNTP client context
 **/

void sntpClientCloseConnection(SntpClientContext *context)
{
   //Valid socket?
   if(context->socket != NULL)
   {
      //Close UDP socket
      socketClose(context->socket);
      context->socket = NULL;
   }
}


/**
 * @brief Send request to the NTP server
 * @param[in] context Pointer to the SNTP client context
 * @return Error code
 **/

error_t sntpClientSendRequest(SntpClientContext *context)
{
   error_t error;
   NtpHeader *header;

   //Point to the buffer where to format the NTP message
   header = (NtpHeader *) context->message;

   //The client initializes the NTP message header. For this purpose, all
   //the NTP header fields are set to 0, except the Mode, VN, and optional
   //Transmit Timestamp fields
   osMemset(header, 0, sizeof(NtpHeader));

   //Format NTP request
   header->vn = context->version;
   header->mode = NTP_MODE_CLIENT;

   //Time at which the NTP request was sent
   context->retransmitStartTime = osGetSystemTime();

   //The Transmit Timestamp allows a simple calculation to determine the
   //propagation delay between the server and client and to align the system
   //clock generally within a few tens of milliseconds relative to the server
   header->transmitTimestamp.seconds = 0;
   header->transmitTimestamp.fraction = htonl(context->retransmitStartTime);

   //Length of the NTP request
   context->messageLen = sizeof(NtpHeader);

   //Debug message
   TRACE_INFO("Sending NTP request message (%" PRIuSIZE " bytes)...\r\n",
      context->messageLen);

   //Dump the contents of the NTP packet for debugging purpose
   ntpDumpPacket(header, context->messageLen);

   //Send the request to the designated NTP server
   error = socketSendTo(context->socket, &context->serverIpAddr,
      context->serverPort, context->message, context->messageLen,
      NULL, 0);

   //Check status code
   if(!error)
   {
      //Wait for server's response
      context->state = SNTP_CLIENT_STATE_RECEIVING;
   }

   //Return status code
   return error;
}


/**
 * @brief Wait for NTP server's response
 * @param[in] context Pointer to the SNTP client context
 * @return Error code
 **/

error_t sntpClientReceiveResponse(SntpClientContext *context)
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
   error = socketSetTimeout(context->socket, MIN(t1, t2));

   //Check status code
   if(!error)
   {
      //Wait for server's response
      error = socketReceiveFrom(context->socket, &ipAddr, &port,
         context->message, NTP_MAX_MSG_SIZE, &context->messageLen, 0);
   }

   //Any datagram received?
   if(error == NO_ERROR)
   {
      //Check NTP response
      error = sntpClientCheckResponse(context, &ipAddr, port,
         context->message, context->messageLen);

      //Check status code
      if(!error)
      {
         //A valid NTP response has been received
         context->state = SNTP_CLIENT_STATE_COMPLETE;
      }
      else
      {
         //Silently discard invalid NTP packets
         error = sntpClientCheckTimeout(context);
      }
   }
   else if(error == ERROR_WOULD_BLOCK || error == ERROR_TIMEOUT)
   {
      //Check whether the timeout has elapsed
      error = sntpClientCheckTimeout(context);
   }
   else
   {
      //A communication error has occurred
   }

   //Return status code
   return error;
}


/**
 * @brief Check whether the NTP response is valid
 * @param[in] context Pointer to the SNTP client context
 * @param[in] ipAddr Remote IP address
 * @param[in] port Remote port number
 * @param[in] message Pointer to the NTP message
 * @param[in] length Length of the NTP message, in bytes
 * @return Error code
 **/

error_t sntpClientCheckResponse(SntpClientContext *context,
   const IpAddr *ipAddr, uint16_t port, const uint8_t *message,
   size_t length)
{
   NtpHeader *header;

   //Ensure the NTP packet is valid
   if(length < sizeof(NtpHeader))
      return ERROR_INVALID_MESSAGE;

   //Point to the NTP response
   header = (NtpHeader *) context->message;

   //Debug message
   TRACE_INFO("NTP response message received (%" PRIuSIZE " bytes)...\r\n",
      length);

   //Dump the contents of the NTP packet for debugging purpose
   ntpDumpPacket(header, length);

   //The server reply should be discarded if the VN field is 0
   if(header->vn == 0)
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
   if(header->originateTimestamp.seconds != 0)
      return ERROR_INVALID_MESSAGE;

   if(header->originateTimestamp.fraction != htonl(context->retransmitStartTime))
      return ERROR_INVALID_MESSAGE;

   //The NTP response message is acceptable
   return NO_ERROR;
}


/**
 * @brief Parse NTP server's response
 * @param[in] context Pointer to the SNTP client context
 * @param[out] timestamp Pointer to the NTP timestamp
 * @return Error code
 **/

error_t sntpClientParseResponse(SntpClientContext *context,
   NtpTimestamp *timestamp)
{
   NtpHeader *header;

   //Ensure the NTP packet is valid
   if(context->messageLen < sizeof(NtpHeader))
      return ERROR_INVALID_LENGTH;

   //Point to the NTP response
   header = (NtpHeader *) context->message;

   //Clear kiss code
   context->kissCode = 0;

   //Kiss-of-Death packet received?
   if(header->stratum == 0)
   {
      //The kiss code is encoded in four-character ASCII strings left
      //justified and zero filled
      context->kissCode = htonl(header->referenceId);

      //An SNTP client should stop sending to a particular server if that
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
 * @brief Determine whether a timeout error has occurred
 * @param[in] context Pointer to the SNTP client context
 * @return Error code
 **/

error_t sntpClientCheckTimeout(SntpClientContext *context)
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
         SNTP_CLIENT_MAX_RETRANSMIT_TIMEOUT);

      //Retransmit NTP request
      context->state = SNTP_CLIENT_STATE_SENDING;

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
