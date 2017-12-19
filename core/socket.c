/**
 * @file socket.c
 * @brief Socket API
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.8.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL SOCKET_TRACE_LEVEL

//Dependencies
#include <string.h>
#include "core/net.h"
#include "core/socket.h"
#include "core/raw_socket.h"
#include "core/udp.h"
#include "core/tcp.h"
#include "core/tcp_misc.h"
#include "dns/dns_client.h"
#include "mdns/mdns_client.h"
#include "netbios/nbns_client.h"
#include "debug.h"

//Socket table
Socket socketTable[SOCKET_MAX_COUNT];


/**
 * @brief Socket related initialization
 * @return Error code
 **/

error_t socketInit(void)
{
   uint_t i;
   uint_t j;

   //Initialize socket descriptors
   memset(socketTable, 0, sizeof(socketTable));

   //Loop through socket descriptors
   for(i = 0; i < SOCKET_MAX_COUNT; i++)
   {
      //Set socket identifier
      socketTable[i].descriptor = i;

      //Create an event object to track socket events
      if(!osCreateEvent(&socketTable[i].event))
      {
         //Clean up side effects
         for(j = 0; j < i; j++)
            osDeleteEvent(&socketTable[j].event);

         //Report an error
         return ERROR_OUT_OF_RESOURCES;
      }
   }

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Create a socket (UDP or TCP)
 * @param[in] type Type specification for the new socket
 * @param[in] protocol Protocol to be used
 * @return Handle referencing the new socket
 **/

Socket *socketOpen(uint_t type, uint_t protocol)
{
   error_t error;
   uint_t i;
   uint16_t port;
   Socket *socket;
   OsEvent event;

   //Initialize socket handle
   socket = NULL;

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(type == SOCKET_TYPE_STREAM)
   {
      //Always use TCP as underlying transport protocol
      protocol = SOCKET_IP_PROTO_TCP;
      //Get an ephemeral port number
      port = tcpGetDynamicPort();
      //Continue processing
      error = NO_ERROR;
   }
   else
#endif
#if (UDP_SUPPORT == ENABLED)
   //Connectionless socket?
   if(type == SOCKET_TYPE_DGRAM)
   {
      //Always use UDP as underlying transport protocol
      protocol = SOCKET_IP_PROTO_UDP;
      //Get an ephemeral port number
      port = udpGetDynamicPort();
      //Continue processing
      error = NO_ERROR;
   }
   else
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
   //Raw socket?
   if(type == SOCKET_TYPE_RAW_IP || type == SOCKET_TYPE_RAW_ETH)
   {
      //Port numbers are not relevant for raw sockets
      port = 0;
      //Continue processing
      error = NO_ERROR;
   }
   else
#endif
   {
      //The socket type is not supported
      error = ERROR_INVALID_PARAMETER;
   }

   //Check status code
   if(!error)
   {
      //Loop through socket descriptors
      for(i = 0; i < SOCKET_MAX_COUNT; i++)
      {
         //Unused socket found?
         if(socketTable[i].type == SOCKET_TYPE_UNUSED)
         {
            //Save socket handle
            socket = &socketTable[i];
            //We are done
            break;
         }
      }

#if (TCP_SUPPORT == ENABLED)
      //No more sockets available?
      if(socket == NULL)
      {
         //Kill the oldest connection in the TIME-WAIT state
         //whenever the socket table runs out of space
         socket = tcpKillOldestConnection();
      }
#endif

      //Check whether the current entry is free
      if(socket != NULL)
      {
         //Save socket descriptor
         i = socket->descriptor;
         //Save event object instance
         memcpy(&event, &socket->event, sizeof(OsEvent));

         //Clear associated structure
         memset(socket, 0, sizeof(Socket));
         //Reuse event objects and avoid recreating them whenever possible
         memcpy(&socket->event, &event, sizeof(OsEvent));

         //Save socket characteristics
         socket->descriptor = i;
         socket->type = type;
         socket->protocol = protocol;
         socket->localPort = port;
         socket->timeout = INFINITE_DELAY;

#if (TCP_SUPPORT == ENABLED)
         socket->txBufferSize = MIN(TCP_DEFAULT_TX_BUFFER_SIZE, TCP_MAX_TX_BUFFER_SIZE);
         socket->rxBufferSize = MIN(TCP_DEFAULT_RX_BUFFER_SIZE, TCP_MAX_RX_BUFFER_SIZE);
#endif
      }
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return a handle to the freshly created socket
   return socket;
}


/**
 * @brief Set timeout value for blocking operations
 * @param[in] socket Handle to a socket
 * @param[in] timeout Maximum time to wait
 * @return Error code
 **/

error_t socketSetTimeout(Socket *socket, systime_t timeout)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Record timeout value
   socket->timeout = timeout;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Specify the size of the send buffer
 * @param[in] socket Handle to a socket
 * @param[in] size Desired buffer size in bytes
 * @return Error code
 **/

error_t socketSetTxBufferSize(Socket *socket, size_t size)
{
#if (TCP_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;
   //Check parameter value
   if(size < 1 || size > TCP_MAX_TX_BUFFER_SIZE)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connection-oriented socket types
   if(socket->type != SOCKET_TYPE_STREAM)
      return ERROR_INVALID_SOCKET;
   //The buffer size cannot be changed when the connection is established
   if(tcpGetState(socket) != TCP_STATE_CLOSED)
      return ERROR_INVALID_SOCKET;

   //Use the specified buffer size
   socket->txBufferSize = size;
   //No error to report
   return NO_ERROR;
#else
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Specify the size of the receive buffer
 * @param[in] socket Handle to a socket
 * @param[in] size Desired buffer size in bytes
 * @return Error code
 **/

error_t socketSetRxBufferSize(Socket *socket, size_t size)
{
#if (TCP_SUPPORT == ENABLED)
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;
   //Check parameter value
   if(size < 1 || size > TCP_MAX_RX_BUFFER_SIZE)
      return ERROR_INVALID_PARAMETER;

   //This function shall be used with connection-oriented socket types
   if(socket->type != SOCKET_TYPE_STREAM)
      return ERROR_INVALID_SOCKET;
   //The buffer size cannot be changed when the connection is established
   if(tcpGetState(socket) != TCP_STATE_CLOSED)
      return ERROR_INVALID_SOCKET;

   //Use the specified buffer size
   socket->rxBufferSize = size;
   //No error to report
   return NO_ERROR;
#else
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Bind a socket to a particular network interface
 * @param[in] socket Handle to a socket
 * @param[in] interface Network interface to be used
 * @return Error code
 **/

error_t socketBindToInterface(Socket *socket, NetInterface *interface)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Explicitly associate the socket with the specified interface
   socket->interface = interface;

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Associate a local address with a socket
 * @param[in] socket Handle to a socket
 * @param[in] localIpAddr Local address to assign to the bound socket
 * @param[in] localPort Local port number to assign to the bound socket
 * @return Error code
 **/

error_t socketBind(Socket *socket, const IpAddr *localIpAddr, uint16_t localPort)
{
   //Check input parameters
   if(!socket || !localIpAddr)
      return ERROR_INVALID_PARAMETER;
   //Make sure the socket type is correct
   if(socket->type != SOCKET_TYPE_STREAM && socket->type != SOCKET_TYPE_DGRAM)
      return ERROR_INVALID_SOCKET;

   //Associate the specified IP address and port number
   socket->localIpAddr = *localIpAddr;
   socket->localPort = localPort;

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Establish a connection to a specified socket
 * @param[in] socket Handle to an unconnected socket
 * @param[in] remoteIpAddr IP address of the remote host
 * @param[in] remotePort Remote port number that will be used to establish the connection
 * @return Error code
 **/

error_t socketConnect(Socket *socket, const IpAddr *remoteIpAddr, uint16_t remotePort)
{
   error_t error;

   //Check input parameters
   if(!socket || !remoteIpAddr)
      return ERROR_INVALID_PARAMETER;

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(socket->type == SOCKET_TYPE_STREAM)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Establish TCP connection
      error = tcpConnect(socket, remoteIpAddr, remotePort);

      //Release exclusive access
      osReleaseMutex(&netMutex);
   }
   else
#endif
   //Connectionless socket?
   if(socket->type == SOCKET_TYPE_DGRAM)
   {
      //Save port number and IP address of the remote host
      socket->remoteIpAddr = *remoteIpAddr;
      socket->remotePort = remotePort;
      //No error to report
      error = NO_ERROR;
   }
   //Raw socket?
   else if(socket->type == SOCKET_TYPE_RAW_IP)
   {
      //Save the IP address of the remote host
      socket->remoteIpAddr = *remoteIpAddr;
      //No error to report
      error = NO_ERROR;
   }
   //Socket type not supported...
   else
   {
      //Invalid socket type
      error = ERROR_INVALID_SOCKET;
   }

   //Return status code
   return error;
}


/**
 * @brief Place a socket in the listening state
 *
 * Place a socket in a state in which it is listening for an incoming connection
 *
 * @param[in] socket Socket to place in the listening state
 * @param[in] backlog backlog The maximum length of the pending connection queue.
 *   If this parameter is zero, then the default backlog value is used instead
 * @return Error code
 **/

error_t socketListen(Socket *socket, uint_t backlog)
{
#if (TCP_SUPPORT == ENABLED)
   error_t error;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;
   //This function shall be used with connection-oriented socket types
   if(socket->type != SOCKET_TYPE_STREAM)
      return ERROR_INVALID_SOCKET;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Start listening for an incoming connection
   error = tcpListen(socket, backlog);

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Permit an incoming connection attempt on a socket
 * @param[in] socket Handle to a socket previously placed in a listening state
 * @param[out] clientIpAddr IP address of the client
 * @param[out] clientPort Port number used by the client
 * @return Handle to the socket in which the actual connection is made
 **/

Socket *socketAccept(Socket *socket, IpAddr *clientIpAddr, uint16_t *clientPort)
{
#if (TCP_SUPPORT == ENABLED)
   Socket *newSocket;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return NULL;
   //This function shall be used with connection-oriented socket types
   if(socket->type != SOCKET_TYPE_STREAM)
      return NULL;

   //Accept an incoming connection attempt
   newSocket = tcpAccept(socket, clientIpAddr, clientPort);

   //Return a handle to the newly created socket
   return newSocket;
#else
   return NULL;
#endif
}


/**
 * @brief Send data to a connected socket
 * @param[in] socket Handle that identifies a connected socket
 * @param[in] data Pointer to a buffer containing the data to be transmitted
 * @param[in] length Number of data bytes to send
 * @param[out] written Actual number of bytes written (optional parameter)
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketSend(Socket *socket, const void *data,
   size_t length, size_t *written, uint_t flags)
{
   //Use default remote IP address for connectionless or raw sockets
   return socketSendTo(socket, &socket->remoteIpAddr,
      socket->remotePort, data, length, written, flags);
}


/**
 * @brief Send a datagram to a specific destination
 * @param[in] socket Handle that identifies a socket
 * @param[in] destIpAddr IP address of the target host
 * @param[in] destPort Target port number
 * @param[in] data Pointer to a buffer containing the data to be transmitted
 * @param[in] length Number of data bytes to send
 * @param[out] written Actual number of bytes written (optional parameter)
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketSendTo(Socket *socket, const IpAddr *destIpAddr, uint16_t destPort,
   const void *data, size_t length, size_t *written, uint_t flags)
{
   error_t error;

   //No data has been transmitted yet
   if(written)
      *written = 0;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(socket->type == SOCKET_TYPE_STREAM)
   {
      //For connection-oriented sockets, target address is ignored
      error = tcpSend(socket, data, length, written, flags);
   }
   else
#endif
#if (UDP_SUPPORT == ENABLED)
   //Connectionless socket?
   if(socket->type == SOCKET_TYPE_DGRAM)
   {
      //Send UDP datagram
      error = udpSendDatagram(socket, destIpAddr,
         destPort, data, length, written);
   }
   else
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
   //Raw socket?
   if(socket->type == SOCKET_TYPE_RAW_IP)
   {
      //Send a raw IP packet
      error = rawSocketSendIpPacket(socket, destIpAddr, data, length, written);
   }
   else if(socket->type == SOCKET_TYPE_RAW_ETH)
   {
      //Send a raw Ethernet packet
      error = rawSocketSendEthPacket(socket, data, length, written);
   }
   else
#endif
   //Socket type not supported...
   {
      //Invalid socket type
      error = ERROR_INVALID_SOCKET;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Receive data from a connected socket
 * @param[in] socket Handle that identifies a connected socket
 * @param[out] data Buffer where to store the incoming data
 * @param[in] size Maximum number of bytes that can be received
 * @param[out] received Number of bytes that have been received
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketReceive(Socket *socket, void *data,
   size_t size, size_t *received, uint_t flags)
{
   //For connection-oriented sockets, source and destination addresses are no use
   return socketReceiveEx(socket, NULL, NULL, NULL, data, size, received, flags);
}


/**
 * @brief Receive a datagram from a connectionless socket
 * @param[in] socket Handle that identifies a socket
 * @param[out] srcIpAddr Source IP address (optional)
 * @param[out] srcPort Source port number (optional)
 * @param[out] data Buffer where to store the incoming data
 * @param[in] size Maximum number of bytes that can be received
 * @param[out] received Number of bytes that have been received
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketReceiveFrom(Socket *socket, IpAddr *srcIpAddr, uint16_t *srcPort,
   void *data, size_t size, size_t *received, uint_t flags)
{
   //Destination address is no use
   return socketReceiveEx(socket, srcIpAddr, srcPort, NULL, data, size, received, flags);
}


/**
 * @brief Receive a datagram
 * @param[in] socket Handle that identifies a socket
 * @param[out] srcIpAddr Source IP address (optional)
 * @param[out] srcPort Source port number (optional)
 * @param[out] destIpAddr Destination IP address (optional)
 * @param[out] data Buffer where to store the incoming data
 * @param[in] size Maximum number of bytes that can be received
 * @param[out] received Number of bytes that have been received
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t socketReceiveEx(Socket *socket, IpAddr *srcIpAddr, uint16_t *srcPort,
   IpAddr *destIpAddr, void *data, size_t size, size_t *received, uint_t flags)
{
   error_t error;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(socket->type == SOCKET_TYPE_STREAM)
   {
      //Receive data
      error = tcpReceive(socket, data, size, received, flags);

      //Output parameters
      if(srcIpAddr)
         *srcIpAddr = socket->remoteIpAddr;
      if(srcPort)
         *srcPort = socket->remotePort;
      if(destIpAddr)
         *destIpAddr = socket->localIpAddr;
   }
   else
#endif
#if (UDP_SUPPORT == ENABLED)
   //Connectionless socket?
   if(socket->type == SOCKET_TYPE_DGRAM)
   {
      //Receive UDP datagram
      error = udpReceiveDatagram(socket, srcIpAddr,
         srcPort, destIpAddr, data, size, received, flags);
   }
   else
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
   //Raw socket?
   if(socket->type == SOCKET_TYPE_RAW_IP)
   {
      //Receive a raw IP packet
      error = rawSocketReceiveIpPacket(socket,
         srcIpAddr, destIpAddr, data, size, received, flags);
   }
   else if(socket->type == SOCKET_TYPE_RAW_ETH)
   {
      //Receive a raw Ethernet packet
      error = rawSocketReceiveEthPacket(socket, data, size, received, flags);
   }
   else
#endif
   //Socket type not supported...
   {
      //No data can be read
      *received = 0;
      //Invalid socket type
      error = ERROR_INVALID_SOCKET;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Retrieve the local address for a given socket
 * @param[in] socket Handle that identifies a socket
 * @param[out] localIpAddr Local IP address (optional)
 * @param[out] localPort Local port number (optional)
 * @return Error code
 **/

error_t socketGetLocalAddr(Socket *socket, IpAddr *localIpAddr, uint16_t *localPort)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Retrieve local IP address
   if(localIpAddr != NULL)
      *localIpAddr = socket->localIpAddr;

   //Retrieve local port number
   if(localPort != NULL)
      *localPort = socket->localPort;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve the address of the peer to which a socket is connected
 * @param[in] socket Handle that identifies a socket
 * @param[out] remoteIpAddr IP address of the remote host (optional)
 * @param[out] remotePort Remote port number (optional)
 * @return Error code
 **/

error_t socketGetRemoteAddr(Socket *socket, IpAddr *remoteIpAddr, uint16_t *remotePort)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Retrieve local IP address
   if(remoteIpAddr != NULL)
      *remoteIpAddr = socket->remoteIpAddr;

   //Retrieve local port number
   if(remotePort != NULL)
      *remotePort = socket->remotePort;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Disable reception, transmission, or both
 *
 * Note that socketShutdown() does not close the socket, and resources attached
 * to the socket will not be freed until socketClose() is invoked
 *
 * @param[in] socket Handle to a socket
 * @param[in] how Flag that describes what types of operation will no longer be allowed
 * @return Error code
 **/

error_t socketShutdown(Socket *socket, uint_t how)
{
#if (TCP_SUPPORT == ENABLED)
   error_t error;

   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;
   //Make sure the socket type is correct
   if(socket->type != SOCKET_TYPE_STREAM)
      return ERROR_INVALID_SOCKET;
   //Check flags
   if((how != SOCKET_SD_SEND) && (how != SOCKET_SD_RECEIVE) && (how != SOCKET_SD_BOTH))
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Graceful shutdown
   error = tcpShutdown(socket, how);

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
#else
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Close an existing socket
 * @param[in] socket Handle identifying the socket to close
 **/

void socketClose(Socket *socket)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return;

   //Get exclusive access
   osAcquireMutex(&netMutex);

#if (TCP_SUPPORT == ENABLED)
   //Connection-oriented socket?
   if(socket->type == SOCKET_TYPE_STREAM)
   {
      //Abort the current TCP connection
      tcpAbort(socket);
   }
#endif
#if (UDP_SUPPORT == ENABLED || RAW_SOCKET_SUPPORT == ENABLED)
   //Connectionless socket or raw socket?
   if(socket->type == SOCKET_TYPE_DGRAM ||
      socket->type == SOCKET_TYPE_RAW_IP ||
      socket->type == SOCKET_TYPE_RAW_ETH)
   {
      //Point to the first item in the receive queue
      SocketQueueItem *queueItem = socket->receiveQueue;

      //Purge the receive queue
      while(queueItem)
      {
         //Keep track of the next item in the queue
         SocketQueueItem *nextQueueItem = queueItem->next;
         //Free previously allocated memory
         memPoolFree(queueItem->buffer);
         //Point to the next item
         queueItem = nextQueueItem;
      }

      //Mark the socket as closed
      socket->type = SOCKET_TYPE_UNUSED;
   }
#endif

   //Release exclusive access
   osReleaseMutex(&netMutex);
}


/**
 * @brief Wait for one of a set of sockets to become ready to perform I/O
 *
 * The socketPoll function determines the status of one or more sockets,
 * waiting if necessary, to perform synchronous I/O
 *
 * @param[in,out] eventDesc Set of entries specifying the events the user is interested in
 * @param[in] size Number of entries in the descriptor set
 * @param[in] extEvent External event that can abort the wait if necessary (optional)
 * @param[in] timeout Maximum time to wait before returning
 * @return Error code
 **/

error_t socketPoll(SocketEventDesc *eventDesc, uint_t size, OsEvent *extEvent, systime_t timeout)
{
   uint_t i;
   bool_t status;
   OsEvent *event;
   OsEvent eventObject;

   //Check parameters
   if(!eventDesc || !size)
      return ERROR_INVALID_PARAMETER;

   //Try to use the supplied event object to receive notifications
   if(!extEvent)
   {
      //Create an event object only if necessary
      if(!osCreateEvent(&eventObject))
      {
         //Report an error
         return ERROR_OUT_OF_RESOURCES;
      }

      //Reference to the newly created event
      event = &eventObject;
   }
   else
   {
      //Reference to the external event
      event = extEvent;
   }

   //Loop through descriptors
   for(i = 0; i < size; i++)
   {
      //Clear event flags
      eventDesc[i].eventFlags = 0;
      //Subscribe to the requested events
      socketRegisterEvents(eventDesc[i].socket, event, eventDesc[i].eventMask);
   }

   //Block the current task until an event occurs
   status = osWaitForEvent(event, timeout);

   //Any socket event is in the signaled state?
   if(status)
   {
      //Loop through descriptors
      for(i = 0; i < size; i++)
      {
         //Retrieve event flags for the current socket
         socketGetEvents(eventDesc[i].socket, &eventDesc[i].eventFlags);
         //Clear unnecessary flags
         eventDesc[i].eventFlags &= eventDesc[i].eventMask;
      }
   }

   //Unsubscribe previously registered events
   for(i = 0; i < size; i++)
      socketUnregisterEvents(eventDesc[i].socket);

   //Reset event object before exiting...
   osResetEvent(event);

   //Release previously allocated resources
   if(!extEvent)
      osDeleteEvent(&eventObject);

   //Return status code
   return status ? NO_ERROR : ERROR_TIMEOUT;
}


/**
 * @brief Subscribe to the specified socket events
 * @param[in] socket Handle that identifies a socket
 * @param[in] event Event object used to receive notifications
 * @param[in] eventMask Logic OR of the requested socket events
 * @return Error code
 **/

error_t socketRegisterEvents(Socket *socket, OsEvent *event, uint_t eventMask)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //An user event may have been previously registered...
   if(socket->userEvent != NULL)
      socket->eventMask |= eventMask;
   else
      socket->eventMask = eventMask;

   //Suscribe to get notified of events
   socket->userEvent = event;

#if (TCP_SUPPORT == ENABLED)
   //Handle TCP specific events
   if(socket->type == SOCKET_TYPE_STREAM)
   {
      tcpUpdateEvents(socket);
   }
#endif
#if (UDP_SUPPORT == ENABLED)
   //Handle UDP specific events
   if(socket->type == SOCKET_TYPE_DGRAM)
   {
      udpUpdateEvents(socket);
   }
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
   //Handle events that are specific to raw sockets
   if(socket->type == SOCKET_TYPE_RAW_IP ||
      socket->type == SOCKET_TYPE_RAW_ETH)
   {
      rawSocketUpdateEvents(socket);
   }
#endif

   //Release exclusive access
   osReleaseMutex(&netMutex);
   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Unsubscribe previously registered events
 * @param[in] socket Handle that identifies a socket
 * @return Error code
 **/

error_t socketUnregisterEvents(Socket *socket)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Unsuscribe socket events
   socket->userEvent = NULL;

   //Release exclusive access
   osReleaseMutex(&netMutex);
   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve event flags for a specified socket
 * @param[in] socket Handle that identifies a socket
 * @param[out] eventFlags Logic OR of events in the signaled state
 * @return Error code
 **/

error_t socketGetEvents(Socket *socket, uint_t *eventFlags)
{
   //Make sure the socket handle is valid
   if(socket == NULL)
   {
      //Always return a valid value
      *eventFlags = 0;
      //Report an error
      return ERROR_INVALID_PARAMETER;
   }

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Read event flags for the specified socket
   *eventFlags = socket->eventFlags;

   //Release exclusive access
   osReleaseMutex(&netMutex);
   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Resolve a host name into an IP address
 * @param[in] interface Underlying network interface (optional parameter)
 * @param[in] name Name of the host to be resolved
 * @param[out] ipAddr IP address corresponding to the specified host name
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t getHostByName(NetInterface *interface,
   const char_t *name, IpAddr *ipAddr, uint_t flags)
{
   error_t error;

   //Default address type depends on TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED)
   HostType type = HOST_TYPE_IPV4;
#elif (IPV6_SUPPORT == ENABLED)
   HostType type = HOST_TYPE_IPV6;
#else
   HostType type = HOST_TYPE_ANY;
#endif

   //Default name resolution protocol depends on TCP/IP stack configuration
#if (DNS_CLIENT_SUPPORT == ENABLED)
   HostnameResolver protocol = HOST_NAME_RESOLVER_DNS;
#elif (MDNS_CLIENT_SUPPORT == ENABLED)
   HostnameResolver protocol = HOST_NAME_RESOLVER_MDNS;
#elif (NBNS_CLIENT_SUPPORT == ENABLED)
   HostnameResolver protocol = HOST_NAME_RESOLVER_NBNS;
#else
   HostnameResolver protocol = HOST_NAME_RESOLVER_ANY;
#endif

   //Check parameters
   if(name == NULL || ipAddr == NULL)
      return ERROR_INVALID_PARAMETER;

   //Use default network interface?
   if(interface == NULL)
      interface = netGetDefaultInterface();

   //The specified name can be either an IP or a host name
   error = ipStringToAddr(name, ipAddr);

   //Perform name resolution if necessary
   if(error)
   {
      //The user may provide a hint to choose between IPv4 and IPv6
      if(flags & HOST_TYPE_IPV4)
         type = HOST_TYPE_IPV4;
      else if(flags & HOST_TYPE_IPV6)
         type = HOST_TYPE_IPV6;

      //The user may provide a hint to to select the desired protocol to be used
      if(flags & HOST_NAME_RESOLVER_DNS)
      {
         //Use DNS to resolve the specified host name
         protocol = HOST_NAME_RESOLVER_DNS;
      }
      else if(flags & HOST_NAME_RESOLVER_MDNS)
      {
         //Use mDNS to resolve the specified host name
         protocol = HOST_NAME_RESOLVER_MDNS;
      }
      else if(flags & HOST_NAME_RESOLVER_NBNS)
      {
         //Use NBNS to resolve the specified host name
         protocol = HOST_NAME_RESOLVER_NBNS;
      }
      else
      {
         //Retrieve the length of the host name to be resolved
         size_t n = strlen(name);

         //Select the most suitable protocol
         if(n >= 6 && !strcasecmp(name + n - 6, ".local"))
         {
#if (MDNS_CLIENT_SUPPORT == ENABLED)
            //Use mDNS to resolve the specified host name
            protocol = HOST_NAME_RESOLVER_MDNS;
#endif
         }
         else if(n <= 15 && !strchr(name, '.') && type == HOST_TYPE_IPV4)
         {
#if (NBNS_CLIENT_SUPPORT == ENABLED)
            //Use NetBIOS Name Service to resolve the specified host name
            protocol = HOST_NAME_RESOLVER_NBNS;
#endif
         }
      }

#if (DNS_CLIENT_SUPPORT == ENABLED)
      //Use DNS protocol?
      if(protocol == HOST_NAME_RESOLVER_DNS)
      {
         //Perform host name resolution
         error = dnsResolve(interface, name, type, ipAddr);
      }
      else
#endif
#if (MDNS_CLIENT_SUPPORT == ENABLED)
      //Use mDNS protocol?
      if(protocol == HOST_NAME_RESOLVER_MDNS)
      {
         //Perform host name resolution
         error = mdnsClientResolve(interface, name, type, ipAddr);
      }
      else
#endif
#if (NBNS_CLIENT_SUPPORT == ENABLED && IPV4_SUPPORT == ENABLED)
      //Use NetBIOS Name Service protocol?
      if(protocol == HOST_NAME_RESOLVER_NBNS)
      {
         //Perform host name resolution
         error = nbnsResolve(interface, name, ipAddr);
      }
      else
#endif
      //Invalid protocol?
      {
         //Report an error
         error = ERROR_INVALID_PARAMETER;
      }
   }

   //Return status code
   return error;
}
