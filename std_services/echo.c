/**
 * @file echo.c
 * @brief Echo protocol
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
 * The echo service simply sends back to the originating source
 * any data it receives. Refer to RFC 862 for complete details
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.8.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL STD_SERVICES_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "std_services/echo.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (NET_STATIC_OS_RESOURCES == ENABLED)

//UDP Echo service
static OsTask udpEchoTaskStruct;
static uint_t udpEchoTaskStack[ECHO_SERVICE_STACK_SIZE];

#endif


/**
 * @brief Start TCP echo service
 * @return Error code
 **/

error_t tcpEchoStart(void)
{
   error_t error;
   Socket *socket;
   OsTask *task;

   //Debug message
   TRACE_INFO("Starting TCP echo service...\r\n");

   //Open a TCP socket
   socket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
   //Failed to open socket?
   if(socket == NULL)
      return ERROR_OPEN_FAILED;

   //Start of exception handling block
   do
   {
      //Bind the newly created socket to port 7
      error = socketBind(socket, &IP_ADDR_ANY, ECHO_PORT);
      //Failed to bind the socket to the desired port?
      if(error)
         break;

      //Place the socket into listening mode
      error = socketListen(socket, 0);
      //Any error to report?
      if(error)
         break;

      //Create a task to handle incoming connection requests
      task = osCreateTask("TCP Echo Listener", tcpEchoListenerTask,
         socket, ECHO_SERVICE_STACK_SIZE, ECHO_SERVICE_PRIORITY);

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
 * @param[in] param Pointer to the echo service context
 **/

void tcpEchoListenerTask(void *param)
{
   error_t error;
   uint16_t clientPort;
   IpAddr clientIpAddr;
   Socket *serverSocket;
   Socket *clientSocket;
   EchoServiceContext *context;
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
      TRACE_INFO("Echo service: connection established with client %s port %" PRIu16 "\r\n",
         ipAddrToString(&clientIpAddr, NULL), clientPort);

      //The socket operates in non-blocking mode
      error = socketSetTimeout(clientSocket, 0);

      //Any error to report?
      if(error)
      {
         //Close socket
         socketClose(clientSocket);
         //Wait for an incoming connection attempt
         continue;
      }

      //Allocate resources for the new connection
      context = osAllocMem(sizeof(EchoServiceContext));

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
      task = osCreateTask("TCP Echo Connection", tcpEchoConnectionTask,
         context, ECHO_SERVICE_STACK_SIZE, ECHO_SERVICE_PRIORITY);

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
 * @brief TCP echo service implementation
 * @param[in] param Pointer to the echo service context
 **/

void tcpEchoConnectionTask(void *param)
{
   error_t error;
   size_t n;
   size_t writeIndex;
   size_t readIndex;
   size_t bufferLength;
   size_t rxByteCount;
   size_t txByteCount;
   systime_t startTime;
   systime_t duration;
   SocketEventDesc eventDesc;
   EchoServiceContext *context;

   //Get a pointer to the context
   context = (EchoServiceContext *) param;
   //Get current time
   startTime = osGetSystemTime();

   //Initialize variables
   writeIndex = 0;
   readIndex = 0;
   bufferLength = 0;
   rxByteCount = 0;
   txByteCount = 0;

   //Main loop
   while(1)
   {
      //Buffer is empty?
      if(!bufferLength)
      {
         //Get notified when the socket is readable
         eventDesc.socket = context->socket;
         eventDesc.eventMask = SOCKET_EVENT_RX_READY;
      }
      //Buffer is not empty of full?
      else if(bufferLength < ECHO_BUFFER_SIZE)
      {
         //Get notified when the socket is readable or writable
         eventDesc.socket = context->socket;
         eventDesc.eventMask = SOCKET_EVENT_RX_READY | SOCKET_EVENT_TX_READY;
      }
      //Buffer is full?
      else
      {
         //Get notified when the socket is writable
         eventDesc.socket = context->socket;
         eventDesc.eventMask = SOCKET_EVENT_TX_READY;
      }

      //Wait for an event to be fired
      error = socketPoll(&eventDesc, 1, NULL, ECHO_TIMEOUT);
      //Timeout error or any other exception to report?
      if(error)
         break;

      //The socket is available for reading
      if(eventDesc.eventFlags & SOCKET_EVENT_RX_READY)
      {
         //Read as much data as possible
         n = MIN(ECHO_BUFFER_SIZE - writeIndex, ECHO_BUFFER_SIZE - bufferLength);

         //Read incoming data
         error = socketReceive(context->socket, context->buffer + writeIndex, n, &n, 0);
         //Any error to report?
         if(error)
            break;

         //Increment write index
         writeIndex += n;
         //Wrap around if necessary
         if(writeIndex >= ECHO_BUFFER_SIZE)
            writeIndex = 0;

         //Increment buffer length
         bufferLength += n;
         //Total number of bytes received
         rxByteCount += n;
      }

      //The socket is available for writing?
      if(eventDesc.eventFlags & SOCKET_EVENT_TX_READY)
      {
         //Write as much data as possible
         n = MIN(ECHO_BUFFER_SIZE - readIndex, bufferLength);

         //Send data back to the client
         error = socketSend(context->socket, context->buffer + readIndex, n, &n, 0);
         //Any error to report?
         if(error && error != ERROR_TIMEOUT)
            break;

         //Increment read index
         readIndex += n;
         //Wrap around if necessary
         if(readIndex >= ECHO_BUFFER_SIZE)
            readIndex = 0;

         //Update buffer length
         bufferLength -= n;
         //Total number of bytes sent
         txByteCount += n;
      }
   }

   //Adjust timeout value
   socketSetTimeout(context->socket, ECHO_TIMEOUT);
   //Graceful shutdown
   socketShutdown(context->socket, SOCKET_SD_BOTH);
   //Compute total duration
   duration = osGetSystemTime() - startTime;

   //Debug message
   TRACE_INFO("Echo service: %" PRIuSIZE " bytes received, %" PRIuSIZE " bytes sent in %" PRIu32 " ms\r\n",
      rxByteCount, txByteCount, duration);

   //Close socket
   socketClose(context->socket);
   //Release previously allocated memory
   osFreeMem(context);

   //Kill ourselves
   osDeleteTask(NULL);
}


/**
 * @brief Start UDP echo service
 * @return Error code
 **/

error_t udpEchoStart(void)
{
   error_t error;
   EchoServiceContext *context;

#if (NET_STATIC_OS_RESOURCES == DISABLED)
   OsTask *task;
#endif

   //Debug message
   TRACE_INFO("Starting UDP echo service...\r\n");

   //Allocate a memory block to hold the context
   context = osAllocMem(sizeof(EchoServiceContext));
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

      //The server listens for incoming datagrams on port 7
      error = socketBind(context->socket, &IP_ADDR_ANY, ECHO_PORT);
      //Unable to bind the socket to the desired port?
      if(error)
         break;

#if (NET_STATIC_OS_RESOURCES == ENABLED)
      //Create a task to handle incoming datagrams
      osCreateStaticTask(&udpEchoTaskStruct, "UDP Echo", udpEchoTask, context,
         udpEchoTaskStack, ECHO_SERVICE_STACK_SIZE, ECHO_SERVICE_PRIORITY);
#else
      //Create a task to handle incoming datagrams
      task = osCreateTask("UDP Echo", udpEchoTask,
         context, ECHO_SERVICE_STACK_SIZE, ECHO_SERVICE_PRIORITY);

      //Unable to create the task?
      if(task == OS_INVALID_HANDLE)
      {
         //Report an error to the calling function
         error = ERROR_OUT_OF_RESOURCES;
         break;
      }
#endif

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
 * @brief UDP echo service implementation
 * @param[in] param Pointer to the echo service context
 **/

void udpEchoTask(void *param)
{
   error_t error;
   size_t length;
   uint16_t port;
   IpAddr ipAddr;
   EchoServiceContext *context;

   //Get a pointer to the context
   context = (EchoServiceContext *) param;

   //Main loop
   while(1)
   {
      //Wait for an incoming datagram
      error = socketReceiveFrom(context->socket, &ipAddr, &port,
         context->buffer, ECHO_BUFFER_SIZE, &length, 0);

      //Any datagram received?
      if(!error)
      {
         //Debug message
         TRACE_INFO("Echo service: %" PRIuSIZE " bytes received from %s port %" PRIu16 "\r\n",
            length, ipAddrToString(&ipAddr, NULL), port);

         //Send the data back to the source
         error = socketSendTo(context->socket, &ipAddr, port,
            context->buffer, length, NULL, 0);
      }
   }
}
