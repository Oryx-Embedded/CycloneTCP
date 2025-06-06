/**
 * @file nts_client.c
 * @brief NTS client (Network Time Security)
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
 * Network Time Security (NTS) is a mechanism for using TLS and AEAD to provide
 * cryptographic security for the client-server mode of the NTP. Refer to
 * RFC 8915 for more details
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
#include "debug.h"

//Check TCP/IP stack configuration
#if (NTS_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Initialize NTS client context
 * @param[in] context Pointer to the NTS client context
 * @return Error code
 **/

error_t ntsClientInit(NtsClientContext *context)
{
   error_t error;

   //Make sure the NTS client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Clear NTS client context
   osMemset(context, 0, sizeof(NtsClientContext));

   //Initialize TLS session state
   error = tlsInitSessionState(&context->tlsSession);
   //Any error to report?
   if(error)
      return error;

   //Initialize NTS client state
   context->state = NTS_CLIENT_STATE_INIT;
   //Default timeout
   context->timeout = NTS_CLIENT_DEFAULT_TIMEOUT;

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Set communication timeout
 * @param[in] context Pointer to the NTS client context
 * @param[in] timeout Timeout value, in milliseconds
 * @return Error code
 **/

error_t ntsClientSetTimeout(NtsClientContext *context, systime_t timeout)
{
   //Make sure the NTS client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Save timeout value
   context->timeout = timeout;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Register TLS initialization callback function
 * @param[in] context Pointer to the NTS client context
 * @param[in] callback TLS initialization callback function
 * @return Error code
 **/

error_t ntsClientRegisterTlsInitCallback(NtsClientContext *context,
   NtsClientTlsInitCallback callback)
{
   //Make sure the NTS client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Save callback function
   context->tlsInitCallback = callback;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Register random data generation callback function
 * @param[in] context Pointer to the NTS client context
 * @param[in] callback Random data generation callback function
 * @return Error code
 **/

error_t ntsClientRegisterRandCallback(NtsClientContext *context,
   NtsClientRandCallback callback)
{
   //Make sure the NTS client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Save callback function
   context->randCallback = callback;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Bind the NTS client to a particular network interface
 * @param[in] context Pointer to the NTS client context
 * @param[in] interface Network interface to be used
 * @return Error code
 **/

error_t ntsClientBindToInterface(NtsClientContext *context,
   NetInterface *interface)
{
   //Make sure the NTS client context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Explicitly associate the NTS client with the specified interface
   context->interface = interface;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Specify the IP address of the NTS server
 * @param[in] context Pointer to the NTS client context
 * @param[in] serverIpAddr IP address of the NTS server
 * @param[in] serverPort Port number
 * @return Error code
 **/

error_t ntsClientSetServerAddr(NtsClientContext *context,
   const IpAddr *serverIpAddr, uint16_t serverPort)
{
   //Check parameters
   if(context == NULL || serverIpAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Save the IP address and the port number of the NTS server
   context->ntsKeServerIpAddr = *serverIpAddr;
   context->ntsKeServerPort = serverPort;

   //Close NTS-KE socket
   ntsClientCloseNtsKeConnection(context);
   //Close NTP socket
   ntsClientCloseNtpConnection(context);

   //Revert to default state
   ntsClientChangeState(context, NTS_CLIENT_STATE_INIT);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve current time from NTS server
 * @param[in] context Pointer to the NTS client context
 * @param[out] timestamp Pointer to the NTP timestamp
 * @return Error code
 **/

error_t ntsClientGetTimestamp(NtsClientContext *context,
   NtpTimestamp *timestamp)
{
   error_t error;

   //Check parameters
   if(context == NULL || timestamp == NULL)
      return ERROR_INVALID_PARAMETER;

   //Initialize status code
   error = NO_ERROR;

   //Send NTP request and wait for server's response
   while(!error)
   {
      //Check current state
      if(context->state == NTS_CLIENT_STATE_INIT)
      {
         //Initialize NTS key establishment
         ntsClientChangeState(context, NTS_CLIENT_STATE_NTS_KE_INIT);
      }
      else if(context->state == NTS_CLIENT_STATE_NTS_KE_INIT)
      {
         //Open NTS-KE connection
         error = ntsClientOpenNtsKeConnection(context);

         //Check status code
         if(!error)
         {
            //Perform TLS handshake
            ntsClientChangeState(context, NTS_CLIENT_STATE_NTS_KE_CONNECTING);
         }
      }
      else if(context->state == NTS_CLIENT_STATE_NTS_KE_CONNECTING)
      {
         //Establish NTS-KE connection
         error = ntsClientEstablishNtsKeConnection(context);

         //Check status code
         if(error == NO_ERROR)
         {
            //Immediately following a successful handshake, the client shall
            //send a single request as Application Data encapsulated in the
            //TLS-protected channel (refer to RFC 8915, section 4)
            error = ntsClientFormatNtsKeRequest(context);

            //Send the request
            ntsClientChangeState(context, NTS_CLIENT_STATE_NTS_KE_SENDING);
         }
         else if(error == ERROR_WOULD_BLOCK || error == ERROR_TIMEOUT)
         {
            //Check whether the timeout has elapsed
            error = ntsClientCheckNtsKeTimeout(context);
         }
         else
         {
            //A communication error has occurred
         }
      }
      else if(context->state == NTS_CLIENT_STATE_NTS_KE_SENDING)
      {
         //The client shall send a single request
         error = ntsClientSendNtsKeRequest(context);

         //Check status code
         if(error == ERROR_WOULD_BLOCK || error == ERROR_TIMEOUT)
         {
            //Check whether the timeout has elapsed
            error = ntsClientCheckNtsKeTimeout(context);
         }
      }
      else if(context->state == NTS_CLIENT_STATE_NTS_KE_RECEIVING)
      {
         //Then, the server shall send a single response
         error = ntsClientReceiveNtsKeResponse(context);

         //Check status code
         if(error == ERROR_WOULD_BLOCK || error == ERROR_TIMEOUT)
         {
            //Check whether the timeout has elapsed
            error = ntsClientCheckNtsKeTimeout(context);
         }
      }
      else if(context->state == NTS_CLIENT_STATE_NTS_KE_DISCONNECTING)
      {
         //Close the connection
         error = ntsClientShutdownNtsKeConnection(context);

         //Check status code
         if(error == NO_ERROR)
         {
            //At this point, the NTS-KE phase of the protocol is complete
            ntsClientCloseNtsKeConnection(context);

            //Time synchronization proceeds with the indicated NTP server
            ntsClientChangeState(context, NTS_CLIENT_STATE_NTP_RESOLVING);
         }
         else if(error == ERROR_WOULD_BLOCK || error == ERROR_TIMEOUT)
         {
            //Check whether the timeout has elapsed
            error = ntsClientCheckNtsKeTimeout(context);
         }
         else
         {
            //A communication error has occurred
         }
      }
      else if(context->state == NTS_CLIENT_STATE_NTP_RESOLVING)
      {
         //The NTP server name shall be either an IPv4 address, an IPv6 address,
         //or a fully qualified domain name (FQDN)
         error = getHostByName(context->interface, context->ntpServerName,
            &context->ntpServerIpAddr, 0);

         //Check status code
         if(error == NO_ERROR)
         {
            //Successful host name resolution
            ntsClientChangeState(context, NTS_CLIENT_STATE_NTP_INIT);
         }
         else if(error == ERROR_IN_PROGRESS)
         {
            //Check whether the timeout has elapsed
            error = ntsClientCheckNtsKeTimeout(context);
         }
         else
         {
            //Host name resolution failed
         }
      }
      else if(context->state == NTS_CLIENT_STATE_NTP_INIT)
      {
         //Valid cookie?
         if(context->cookieLen > 0)
         {
            //Open NTP connection
            error = ntsClientOpenNtpConnection(context);

            //Check status code
            if(!error)
            {
               //Save current time
               context->startTime = osGetSystemTime();
               //Initialize retransmission timeout
               context->retransmitTimeout = NTS_CLIENT_INIT_NTP_RETRANSMIT_TIMEOUT;

               //Send the NTP request to the designated server
               ntsClientChangeState(context, NTS_CLIENT_STATE_NTP_SENDING);
            }
         }
         else
         {
            //If the client does not have any cookies that it has not already
            //sent, it should initiate a rerun of the NTS-KE protocol (refer to
            //RFC 5915, section 5.7)
            ntsClientChangeState(context, NTS_CLIENT_STATE_NTS_KE_INIT);
         }
      }
      else if(context->state == NTS_CLIENT_STATE_NTP_SENDING)
      {
         //Send the NTP request to the designated server
         error = ntsClientSendNtpRequest(context);
      }
      else if(context->state == NTS_CLIENT_STATE_NTP_RECEIVING)
      {
         //Wait for server's response
         error = ntsClientReceiveNtpResponse(context);
      }
      else if(context->state == NTS_CLIENT_STATE_COMPLETE)
      {
         //Extract NTP timestamp from server's response
         error = ntsClientParseNtpResponse(context, timestamp);
         //We are done
         break;
      }
      else
      {
         //Invalid state
         error = ERROR_WRONG_STATE;
      }
   }

   //Check status code
   if(error == NO_ERROR)
   {
      //Close NTP connection
      ntsClientCloseNtpConnection(context);

      //Ideally, the client never needs to connect to the NTS-KE server again
      ntsClientChangeState(context, NTS_CLIENT_STATE_NTP_INIT);
   }
   else if(error == ERROR_WOULD_BLOCK)
   {
      //The NTS-KE or NTP phase of the protocol is in progress
   }
   else
   {
      //Close NTS-KE connection
      ntsClientCloseNtsKeConnection(context);
      //Close NTP connection
      ntsClientCloseNtpConnection(context);

      //Revert to default state
      ntsClientChangeState(context, NTS_CLIENT_STATE_INIT);
   }

   //Return status code
   return error;
}


/**
 * @brief Retrieve the kiss code from a Kiss-of-Death message
 * @param[in] context Pointer to the NTS client context
 * @return Kiss code
 **/

uint32_t ntsClientGetKissCode(NtsClientContext *context)
{
   uint32_t kissCode;

   //Make sure the NTS client context is valid
   if(context != NULL)
   {
      //Get kiss code
      kissCode = context->kissCode;
   }
   else
   {
      //The NTS client context is not valid
      kissCode = 0;
   }

   //Return kiss code
   return kissCode;
}


/**
 * @brief Release NTS client context
 * @param[in] context Pointer to the NTS client context
 **/

void ntsClientDeinit(NtsClientContext *context)
{
   //Make sure the NTS client context is valid
   if(context != NULL)
   {
      //Close NTS-KE connection
      ntsClientCloseNtsKeConnection(context);
      //Close NTP connection
      ntsClientCloseNtpConnection(context);

      //Release TLS session state
      tlsFreeSessionState(&context->tlsSession);

      //Clear NTS client context
      osMemset(context, 0, sizeof(NtsClientContext));
   }
}

#endif
