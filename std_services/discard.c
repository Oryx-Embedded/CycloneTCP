/**
 * @file discard.c
 * @brief Discard protocol
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
 * The discard service is a useful debugging and measurement tool. The service
 * simply throws away any data it receives. Refer to RFC 863 for complete details
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.8.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL STD_SERVICES_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "std_services/discard.h"
#include "debug.h"


/**
 * @brief Start TCP discard service
 * @return Error code
 **/

error_t tcpDiscardStart(void)
{
   error_t error;
   Socket *socket;
   OsTask *task;

   //Debug message
   TRACE_INFO("Starting TCP discard service...\r\n");

   //Open a TCP socket
   socket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
   //Failed to open socket?
   if(socket == NULL)
      return ERROR_OPEN_FAILED;

   //Start of exception handling block
   do
   {
      //Bind the newly created socket to port 9
      error = socketBind(socket, &IP_ADDR_ANY, DISCARD_PORT);
      //Failed to bind the socket to the desired port?
      if(error)
         break;

      //Place the socket into listening mode
      error = socketListen(socket, 0);
      //Any error to report?
      if(error)
         break;

      //Create a task to handle incoming connection requests
      task = osCreateTask("TCP Discard Listener", tcpDiscardListenerTask,
         socket, DISCARD_SERVICE_STACK_SIZE, DISCARD_SERVICE_PRIORITY);

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
 * @param[in] param Pointer to the discard service context
 **/

void tcpDiscardListenerTask(void *param)
{
   error_t error;
   uint16_t clientPort;
   IpAddr clientIpAddr;
   Socket *serverSocket;
   Socket *clientSocket;
   DiscardServiceContext *context;
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
      TRACE_INFO("Discard service: connection established with client %s port %" PRIu16 "\r\n",
         ipAddrToString(&clientIpAddr, NULL), clientPort);

      //Adjust timeout
      error = socketSetTimeout(clientSocket, DISCARD_TIMEOUT);

      //Any error to report?
      if(error)
      {
         //Close socket
         socketClose(clientSocket);
         //Wait for an incoming connection attempt
         continue;
      }

      //Allocate resources for the new connection
      context = osAllocMem(sizeof(DiscardServiceContext));

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
      task = osCreateTask("TCP Discard Connection", tcpDiscardConnectionTask,
         context, DISCARD_SERVICE_STACK_SIZE, DISCARD_SERVICE_PRIORITY);

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
 * @brief TCP discard service implementation
 * @param[in] param Pointer to the discard service context
 **/

void tcpDiscardConnectionTask(void *param)
{
   error_t error;
   size_t n;
   size_t byteCount;
   systime_t startTime;
   systime_t duration;
   DiscardServiceContext *context;

   //Get a pointer to the context
   context = (DiscardServiceContext *) param;
   //Get current time
   startTime = osGetSystemTime();

   //Total number of bytes received
   byteCount = 0;

   //Main loop
   while(1)
   {
      //Throw away any received datagram...
      error = socketReceive(context->socket, context->buffer, DISCARD_BUFFER_SIZE, &n, 0);
      //Any error to report?
      if(error)
         break;

      //Total number of bytes received
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
   TRACE_INFO("Discard service: %" PRIuSIZE " bytes "
      "received in %" PRIu32 " ms (%" PRIu32 " kBps, %" PRIu32 " kbps)\r\n",
      byteCount, duration, byteCount / duration, (byteCount * 8) / duration);

   //Close socket
   socketClose(context->socket);
   //Release previously allocated memory
   osFreeMem(context);

   //Kill ourselves
   osDeleteTask(NULL);
}


/**
 * @brief Start UDP discard service
 * @return Error code
 **/

error_t udpDiscardStart(void)
{
   error_t error;
   DiscardServiceContext *context;
   OsTask *task;

   //Debug message
   TRACE_INFO("Starting UDP discard service...\r\n");

   //Allocate a memory block to hold the context
   context = osAllocMem(sizeof(DiscardServiceContext));
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

      //The server listens for incoming datagrams on port 9
      error = socketBind(context->socket, &IP_ADDR_ANY, DISCARD_PORT);
      //Unable to bind the socket to the desired port?
      if(error)
         break;

      //Create a task to handle incoming datagrams
      task = osCreateTask("UDP Discard", udpDiscardTask,
         context, DISCARD_SERVICE_STACK_SIZE, DISCARD_SERVICE_PRIORITY);

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
 * @brief UDP discard service implementation
 * @param[in] param Pointer to the discard service context
 **/

void udpDiscardTask(void *param)
{
   error_t error;
   size_t length;
   uint16_t port;
   IpAddr ipAddr;
   DiscardServiceContext *context;

   //Get a pointer to the context
   context = (DiscardServiceContext *) param;

   //Main loop
   while(1)
   {
      //Wait for an incoming datagram
      error = socketReceiveFrom(context->socket, &ipAddr, &port,
         context->buffer, DISCARD_BUFFER_SIZE, &length, 0);

      //Any datagram received?
      if(!error)
      {
         //Debug message
         TRACE_INFO("Discard service: %" PRIuSIZE " bytes received from %s port %" PRIu16 "\r\n",
            length, ipAddrToString(&ipAddr, NULL), port);

         //Throw away any received datagram...
      }
   }
}
