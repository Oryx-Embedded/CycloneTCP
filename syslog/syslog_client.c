/**
 * @file syslog_client.c
 * @brief Syslog client
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
 * @section Description
 *
 * The Syslog protocol is used to convey event notification messages. It
 * provides a message format that allows vendor-specific extensions to be
 * provided in a structured way. Refer to RFC 3164 for more details
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL SYSLOG_TRACE_LEVEL

//Dependencies
#include <stdarg.h>
#include "syslog/syslog_client.h"
#include "syslog/syslog_client_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SYSLOG_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Syslog client initialization
 * @param[in] context Pointer to the Syslog client context
 * @return Error code
 **/

error_t syslogClientInit(SyslogClientContext *context)
{
   //Make sure the Syslog client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Initializing Syslog client...\r\n");

   //Initialize context
   osMemset(context, 0, sizeof(SyslogClientContext));
   //Use default interface
   context->interface = netGetDefaultInterface();

   //Create a mutex to prevent simultaneous access to the context
   if(!osCreateMutex(&context->mutex))
      return ERROR_OUT_OF_RESOURCES;

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Bind the Syslog client to a particular network interface
 * @param[in] context Pointer to the Syslog client context
 * @param[in] interface Network interface to be used
 * @return Error code
 **/

error_t syslogClientBindToInterface(SyslogClientContext *context,
   NetInterface *interface)
{
   //Make sure the Syslog client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Explicitly associate the Syslog client with the specified interface
   context->interface = interface;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Specify the address of the Syslog server
 * @param[in] context Pointer to the Syslog client context
 * @param[in] serverIpAddr IP address of the Syslog server to connect to
 * @param[in] serverPort UDP port number
 * @return Error code
 **/

error_t syslogClientConnect(SyslogClientContext *context,
   const IpAddr *serverIpAddr, uint16_t serverPort)
{
   error_t error;

   //Check parameters
   if(context == NULL || serverIpAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Start of exception handling block
   do
   {
      //Open a UDP socket
      context->socket = socketOpen(SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);
      //Failed to open socket?
      if(context->socket == NULL)
      {
         //Report an error
         error = ERROR_OPEN_FAILED;
         break;
      }

      //Associate the socket with the relevant interface
      error = socketBindToInterface(context->socket, context->interface);
      //Any error to report?
      if(error)
         break;

      //Connect the socket to the remote Syslog server
      error = socketConnect(context->socket, serverIpAddr, serverPort);
      //Any error to report?
      if(error)
         break;

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Clean up side effects
      socketClose(context->socket);
      context->socket = NULL;
   }

   //Return status code
   return error;
}


/**
 * @brief Send Syslog message
 * @param[in] context Pointer to the Syslog client context
 * @param[in] facility Facility value
 * @param[in] severity Severity value
 * @param[in] message NULL-terminated string that holds the message
 * @return Error code
 **/

error_t syslogClientSendMessage(SyslogClientContext *context, uint_t facility,
   uint_t severity, const char_t *message)
{
   error_t error;
   size_t n;
   size_t messageLen;
   uint_t priority;
   time_t time;

   //Check parameters
   if(context == NULL || message == NULL)
      return ERROR_INVALID_PARAMETER;

   //Make sure the UDP socket is valid
   if(context->socket == NULL)
      return ERROR_NOT_CONNECTED;

   //Acquire exclusive access to the Syslog client context
   osAcquireMutex(&context->mutex);

   //The Priority value is calculated by first multiplying the Facility
   //number by 8 and then adding the numerical value of the Severity
   priority = (facility * 8) + severity;

   //Format the PRI part of the Syslog packet
   n = osSprintf(context->buffer, "<%u> ", priority);

   //Retrieve current time
   time = getCurrentUnixTime();
   //Format TIMESTAMP field
   n += syslogClientFormatTimestamp(time, context->buffer + n);

   //Format HOSTNAME field
   n += osSprintf(context->buffer + n, " %s ", context->interface->hostname);

   //Retrieve the length of the message
   messageLen = osStrlen(message);
   //The MSG part will fill the remainder of the syslog message
   messageLen = MIN(messageLen, SYSLOG_CLIENT_BUFFER_SIZE - 1 - n);

   //Format the MSG part of the Syslog packet
   osStrncpy(context->buffer + n, message, messageLen);
   //Total length of the Syslog packet
   n += messageLen;

   //Properly terminate the string with a NULL character
   context->buffer[n] = '\0';

   //Debug message
   TRACE_DEBUG("Sending Syslog message (%" PRIuSIZE " bytes)...\r\n", n);
   TRACE_DEBUG("  %s\r\n", context->buffer);

   //Send Syslog packet
   error = socketSend(context->socket, context->buffer, n, NULL, 0);

   //Release exclusive access to the Syslog client context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
}


/**
 * @brief Format Syslog message
 * @param[in] context Pointer to the Syslog client context
 * @param[in] facility Facility value
 * @param[in] severity Severity value
 * @param[in] format NULL-terminated string that that contains a format string
 * @param[in] ... Optional arguments
 * @return Error code
 **/

error_t syslogClientFormatMessage(SyslogClientContext *context, uint_t facility,
   uint_t severity, const char_t *format, ...)
{
   error_t error;
   size_t n;
   size_t maxMessageLen;
   uint_t priority;
   time_t time;
   va_list args;

   //Check parameters
   if(context == NULL || format == NULL)
      return ERROR_INVALID_PARAMETER;

   //Make sure the UDP socket is valid
   if(context->socket == NULL)
      return ERROR_NOT_CONNECTED;

   //Acquire exclusive access to the Syslog client context
   osAcquireMutex(&context->mutex);

   //The Priority value is calculated by first multiplying the Facility
   //number by 8 and then adding the numerical value of the Severity
   priority = (facility * 8) + severity;

   //Format the PRI part of the Syslog packet
   n = osSprintf(context->buffer, "<%u> ", priority);

   //Retrieve current time
   time = getCurrentUnixTime();
   //Format TIMESTAMP field
   n += syslogClientFormatTimestamp(time, context->buffer + n);

   //Format HOSTNAME field
   n += osSprintf(context->buffer + n, " %s ", context->interface->hostname);

   //The MSG part will fill the remainder of the syslog message
   maxMessageLen = SYSLOG_CLIENT_BUFFER_SIZE - 1 - n;

   //Initialize processing of a varying-length argument list
   va_start(args, format);
   //Format the MSG part of the Syslog packet
   n += osVsnprintf(context->buffer + n, maxMessageLen, format, args);
   //End varying-length argument list processing
   va_end(args);

   //Debug message
   TRACE_DEBUG("Sending Syslog message (%" PRIuSIZE " bytes)...\r\n", n);
   TRACE_DEBUG("  %s\r\n", context->buffer);

   //Send Syslog packet
   error = socketSend(context->socket, context->buffer, n, NULL, 0);

   //Release exclusive access to the Syslog client context
   osReleaseMutex(&context->mutex);

   //Return status code
   return error;
}


/**
 * @brief Close the connection with the Syslog server
 * @param[in] context Pointer to the Syslog client context
 * @return Error code
 **/

error_t syslogClientClose(SyslogClientContext *context)
{
   //Make sure the Syslog client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Close UDP socket
   if(context->socket != NULL)
   {
      socketClose(context->socket);
      context->socket = NULL;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Release Syslog client context
 * @param[in] context Pointer to the Syslog client context
 **/

void syslogClientDeinit(SyslogClientContext *context)
{
   //Make sure the Syslog client context is valid
   if(context != NULL)
   {
      //Close UDP socket
      if(context->socket != NULL)
      {
         socketClose(context->socket);
      }

      //Release previously allocated resources
      osDeleteMutex(&context->mutex);

      //Clear Syslog client context
      osMemset(context, 0, sizeof(SyslogClientContext));
   }
}

#endif
