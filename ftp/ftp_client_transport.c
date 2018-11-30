/**
 * @file ftp_client_transport.c
 * @brief Transport protocol abstraction layer
 *
 * @section License
 *
 * Copyright (C) 2010-2018 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL FTP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "ftp/ftp_client.h"
#include "ftp/ftp_client_transport.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (FTP_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Open network connection
 * @param[in] context Pointer to the FTP client context
 * @param[in] connection Control or data connection
 * @param[in] txBufferSize TX buffer size
 * @param[in] rxBufferSize RX buffer size
 * @return Error code
 **/

error_t ftpClientOpenConnection(FtpClientContext *context,
   FtpClientSocket *connection, size_t txBufferSize, size_t rxBufferSize)
{
   error_t error;

   //Open a TCP socket
   connection->socket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
   //Failed to open socket?
   if(connection->socket == NULL)
      return ERROR_OPEN_FAILED;

   //Associate the socket with the relevant interface
   error = socketBindToInterface(connection->socket, context->interface);
   //Any error to report?
   if(error)
      return error;

   //Set timeout
   error = socketSetTimeout(connection->socket, context->timeout);
   //Any error to report?
   if(error)
      return error;

   //Specify the size of the send buffer
   error = socketSetTxBufferSize(connection->socket, txBufferSize);
   //Any error to report?
   if(error)
      return error;

   //Specify the size of the receive buffer
   error = socketSetRxBufferSize(connection->socket, rxBufferSize);
   //Any error to report?
   if(error)
      return error;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Open secure connection
 * @param[in] context Pointer to the FTP client context
 * @param[in] connection Control or data connection
 * @param[in] txBufferSize TX buffer size
 * @param[in] rxBufferSize RX buffer size
 * @return Error code
 **/

error_t ftpClientOpenSecureConnection(FtpClientContext *context,
   FtpClientSocket *connection, size_t txBufferSize, size_t rxBufferSize)
{
#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)
   error_t error;

   //Allocate TLS context
   connection->tlsContext = tlsInit();
   //Failed to allocate TLS context?
   if(connection->tlsContext == NULL)
      return ERROR_OPEN_FAILED;

   //Select client operation mode
   error = tlsSetConnectionEnd(connection->tlsContext,
      TLS_CONNECTION_END_CLIENT);
   //Any error to report?
   if(error)
      return error;

   //Bind TLS to the relevant socket
   error = tlsSetSocket(connection->tlsContext, connection->socket);
   //Any error to report?
   if(error)
      return error;

   //Set TX and RX buffer size
   error = tlsSetBufferSize(connection->tlsContext, txBufferSize, rxBufferSize);
   //Any error to report?
   if(error)
      return error;

   //Data connection?
   if(connection == &context->dataConnection)
   {
      //Save TLS session from control connection
      error = tlsSaveSessionState(context->controlConnection.tlsContext,
         &context->tlsSession);
      //Any error to report?
      if(error)
         return error;
   }

   //Restore TLS session
   error = tlsRestoreSessionState(connection->tlsContext,
      &context->tlsSession);
   //Any error to report?
   if(error)
      return error;

   //Invoke user-defined callback, if any
   if(context->tlsInitCallback != NULL)
   {
      //Perform TLS related initialization
      error = context->tlsInitCallback(context, connection->tlsContext);
      //Any error to report?
      if(error)
         return error;
   }

   //Successful processing
   return NO_ERROR;
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Establish secure connection
 * @param[in] connection Control or data connection
 * @return Error code
 **/

error_t ftpClientEstablishSecureConnection(FtpClientSocket *connection)
{
#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)
   //Establish a TLS connection
   return tlsConnect(connection->tlsContext);
#else
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
#endif
}


/**
 * @brief Shutdown network connection
 * @param[in] connection Control or data connection
 * @return Error code
 **/

error_t ftpClientShutdownConnection(FtpClientSocket *connection)
{
   error_t error;

   //Initialize status code
   error = NO_ERROR;

#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)
   //Valid TLS context?
   if(connection->tlsContext != NULL)
   {
      //Shutdown TLS session
      error = tlsShutdown(connection->tlsContext);
   }
#endif

   //Check status code
   if(!error)
   {
      //Valid TCP socket?
      if(connection->socket != NULL)
      {
         //Shutdown TCP connection
         error = socketShutdown(connection->socket, SOCKET_SD_BOTH);
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Close network connection
 * @param[in] connection Control or data connection
 **/

void ftpClientCloseConnection(FtpClientSocket *connection)
{
#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)
   //Release TLS context
   if(connection->tlsContext != NULL)
   {
      tlsFree(connection->tlsContext);
      connection->tlsContext = NULL;
   }
#endif

   //Close TCP connection
   if(connection->socket != NULL)
   {
      socketClose(connection->socket);
      connection->socket = NULL;
   }
}


/**
 * @brief Send data using the relevant transport protocol
 * @param[in] connection Control or data connection
 * @param[in] data Pointer to a buffer containing the data to be transmitted
 * @param[in] length Number of bytes to be transmitted
 * @param[out] written Actual number of bytes written (optional parameter)
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t ftpClientSendData(FtpClientSocket *connection, const void *data,
   size_t length, size_t *written, uint_t flags)
{
   error_t error;

#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)
   //TLS-secured connection?
   if(connection->tlsContext != NULL)
   {
      //Send TLS-encrypted data
      error = tlsWrite(connection->tlsContext, data, length, written, flags);
   }
   else
#endif
   {
      //Transmit data
      error = socketSend(connection->socket, data, length, written, flags);
   }

   //Return status code
   return error;
}


/**
 * @brief Receive data using the relevant transport protocol
 * @param[in] connection Control or data connection
 * @param[out] data Buffer into which received data will be placed
 * @param[in] size Maximum number of bytes that can be received
 * @param[out] received Number of bytes that have been received
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t ftpClientReceiveData(FtpClientSocket *connection, void *data,
   size_t size, size_t *received, uint_t flags)
{
   error_t error;

#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)
   //TLS-secured connection?
   if(connection->tlsContext != NULL)
   {
      //Receive TLS-encrypted data
      error = tlsRead(connection->tlsContext, data, size, received, flags);
   }
   else
#endif
   {
      //Receive data
      error = socketReceive(connection->socket, data, size, received, flags);
   }

   //Return status code
   return error;
}

#endif
