/**
 * @file chargen.c
 * @brief Character generator protocol
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
 * The character generator service is a useful debugging and measurement
 * tool. The service simply sends data until the calling user terminates
 * the connection. Refer to RFC 864 for complete details
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.8.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL STD_SERVICES_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "std_services/chargen.h"
#include "debug.h"

//Character pattern (from RFC 864)
const char_t pattern[190] =
{
   '!', '"', '#',  '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3',
   '4', '5', '6',  '7', '8', '9', ':',  ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F',
   'G', 'H', 'I',  'J', 'K', 'L', 'M',  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
   'Z', '[', '\\', ']', '^', '_', '`',  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
   'm', 'n', 'o',  'p', 'q', 'r', 's',  't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', ' ',
   '!', '"', '#',  '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3',
   '4', '5', '6',  '7', '8', '9', ':',  ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F',
   'G', 'H', 'I',  'J', 'K', 'L', 'M',  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
   'Z', '[', '\\', ']', '^', '_', '`',  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
   'm', 'n', 'o',  'p', 'q', 'r', 's',  't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', ' '
};


/**
 * @brief Start TCP chargen service
 * @return Error code
 **/

error_t tcpChargenStart(void)
{
   error_t error;
   Socket *socket;
   OsTask *task;

   //Debug message
   TRACE_INFO("Starting TCP chargen service...\r\n");

   //Open a TCP socket
   socket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
   //Failed to open socket?
   if(socket == NULL)
      return ERROR_OPEN_FAILED;

   //Start of exception handling block
   do
   {
      //Bind the newly created socket to port 19
      error = socketBind(socket, &IP_ADDR_ANY, CHARGEN_PORT);
      //Failed to bind the socket to the desired port?
      if(error)
         break;

      //Place the socket into listening mode
      error = socketListen(socket, 0);
      //Any error to report?
      if(error)
         break;

      //Create a task to handle incoming connection requests
      task = osCreateTask("TCP Chargen Listener", tcpChargenListenerTask,
         socket, CHARGEN_SERVICE_STACK_SIZE, CHARGEN_SERVICE_PRIORITY);

      //Unable to create the task?
      if(task == OS_INVALID_HANDLE)
      {
         //Report an error to the calling function
         error = ERROR_OUT_OF_RESOURCES;
         break;
      }

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Clean up side effects...
      socketClose(socket);
   }

   //Return status code
   return error;
}


/**
 * @brief Task handling connection requests
 * @param[in] param Pointer to the chargen service context
 **/

void tcpChargenListenerTask(void *param)
{
   error_t error;
   uint16_t clientPort;
   IpAddr clientIpAddr;
   Socket *serverSocket;
   Socket *clientSocket;
   ChargenServiceContext *context;
   OsTask *task;

   //Point to the listening socket
   serverSocket = (Socket *) param;

   //Main loop
   while(1)
   {
      //Accept an incoming connection
      clientSocket = socketAccept(serverSocket, &clientIpAddr, &clientPort);
      //Check whether a valid connection request has been received
      if(!clientSocket)
         continue;

      //Debug message
      TRACE_INFO("Chargen service: connection established with client %s port %" PRIu16 "\r\n",
         ipAddrToString(&clientIpAddr, NULL), clientPort);

      //Adjust timeout
      error = socketSetTimeout(clientSocket, CHARGEN_TIMEOUT);

      //Any error to report?
      if(error)
      {
         //Close socket
         socketClose(clientSocket);
         //Wait for an incoming connection attempt
         continue;
      }

      //Allocate resources for the new connection
      context = osAllocMem(sizeof(ChargenServiceContext));

      //Failed to allocate memory?
      if(context == NULL)
      {
         //Close socket
         socketClose(clientSocket);
         //Wait for an incoming connection attempt
         continue;
      }

      //Record the handle of the newly created socket
      context->socket = clientSocket;

      //Create a task to service the current connection
      task = osCreateTask("TCP Chargen Connection", tcpChargenConnectionTask,
         context, CHARGEN_SERVICE_STACK_SIZE, CHARGEN_SERVICE_PRIORITY);

      //Did we encounter an error?
      if(task == OS_INVALID_HANDLE)
      {
         //Close socket
         socketClose(clientSocket);
         //Release resources
         osFreeMem(context);
      }
   }
}


/**
 * @brief TCP chargen service implementation
 * @param[in] param Pointer to the chargen service context
 **/

void tcpChargenConnectionTask(void *param)
{
   error_t error;
   //size_t i;
   size_t n;
   //size_t offset;
   size_t byteCount;
   systime_t startTime;
   systime_t duration;
   ChargenServiceContext *context;

   //Get a pointer to the context
   context = (ChargenServiceContext *) param;
   //Get current time
   startTime = osGetSystemTime();

   //Initialize counters
   byteCount = 0;
   //offset = 0;

   //Once a connection is established a stream of data is sent out
   //the connection (and any data received is thrown away). This
   //continues until the calling user terminates the connection
   while(1)
   {
      //Format output data
      /*for(i = 0; i < CHARGEN_BUFFER_SIZE; i += 95)
      {
         //Calculate the length of the current line
         n = MIN(CHARGEN_BUFFER_SIZE - i, 95);
         //Copy character pattern
         memcpy(context->buffer + i, pattern + offset, n);
      }

      //Update offset
      offset += CHARGEN_BUFFER_SIZE + 95 - i;
      //Wrap around if necessary
      if(offset >= 95) offset = 0;*/

      //Send data
      error = socketSend(context->socket, context->buffer, CHARGEN_BUFFER_SIZE, &n, 0);
      //Any error to report?
      if(error)
         break;

      //Total number of bytes sent
      byteCount += n;
   }

   //Graceful shutdown
   socketShutdown(context->socket, SOCKET_SD_BOTH);
   //Compute total duration
   duration = osGetSystemTime() - startTime;
   //Avoid division by zero...
   if(!duration)
      duration = 1;

   //Debug message
   TRACE_INFO("Chargen service: %" PRIuSIZE " bytes "
      "sent in %" PRIu32 " ms (%" PRIu32 " kBps, %" PRIu32 " kbps)\r\n",
      byteCount, duration, byteCount / duration, (byteCount * 8) / duration);

   //Close socket
   socketClose(context->socket);
   //Release previously allocated memory
   osFreeMem(context);

   //Kill ourselves
   osDeleteTask(NULL);
}


/**
 * @brief Start UDP chargen service
 * @return Error code
 **/

error_t udpChargenStart(void)
{
   error_t error;
   ChargenServiceContext *context;
   OsTask *task;

   //Debug message
   TRACE_INFO("Starting UDP chargen service...\r\n");

   //Allocate a memory block to hold the context
   context = osAllocMem(sizeof(ChargenServiceContext));
   //Failed to allocate memory?
   if(context == NULL)
      return ERROR_OUT_OF_MEMORY;

   //Start of exception handling block
   do
   {
      //Open a UDP socket
      context->socket = socketOpen(SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);

      //Failed to open socket?
      if(!context->socket)
      {
         //Report an error
         error = ERROR_OPEN_FAILED;
         //Exit immediately
         break;
      }

      //The server listens for incoming datagrams on port 19
      error = socketBind(context->socket, &IP_ADDR_ANY, CHARGEN_PORT);
      //Unable to bind the socket to the desired port?
      if(error)
         break;

      //Create a task to handle incoming datagrams
      task = osCreateTask("UDP Chargen", udpChargenTask,
         context, CHARGEN_SERVICE_STACK_SIZE, CHARGEN_SERVICE_PRIORITY);

      //Unable to create the task?
      if(task == OS_INVALID_HANDLE)
      {
         //Report an error to the calling function
         error = ERROR_OUT_OF_RESOURCES;
         break;
      }

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Clean up side effects...
      socketClose(context->socket);
      osFreeMem(context);
   }

   //Return status code
   return error;
}


/**
 * @brief UDP chargen service implementation
 * @param[in] param Pointer to the chargen service context
 **/

void udpChargenTask(void *param)
{
   error_t error;
   size_t i;
   size_t k;
   size_t n;
   size_t length;
   uint16_t port;
   IpAddr ipAddr;
   ChargenServiceContext *context;

   //Get a pointer to the context
   context = (ChargenServiceContext *) param;

   //Main loop
   while(1)
   {
      //Wait for an incoming datagram
      error = socketReceiveFrom(context->socket, &ipAddr, &port,
         context->buffer, CHARGEN_BUFFER_SIZE, &n, 0);

      //Any datagram received?
      if(!error)
      {
         //When a datagram is received, an answering datagram is sent
         //containing a random number (between 0 and 512) of characters
         length = netGetRand() % 513;

         //Reset line counter
         n = 0;

         //Format output data
         for(i = 0; i < length; i += 74)
         {
            //Calculate the length of the current line
            k = MIN(length - i, 74);
            //Copy character pattern
            memcpy(context->buffer + i, pattern + n, k);

            //End each line with carriage return and line feed
            if(k == 74)
            {
               context->buffer[i + 72] = '\r';
               context->buffer[i + 73] = '\n';
            }

            //Increment line counter
            if(++n >= 95)
               n = 0;
         }

         //Send data to the remote host
         error = socketSendTo(context->socket, &ipAddr, port,
            context->buffer, length, &n, 0);

         //Debug message
         TRACE_INFO("Chargen service: %" PRIuSIZE " bytes sent to %s port %" PRIu16 "\r\n",
            n, ipAddrToString(&ipAddr, NULL), port);
      }
   }
}
