/**
 * @file ftp_server_misc.c
 * @brief FTP server (miscellaneous functions)
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
#define TRACE_LEVEL FTP_TRACE_LEVEL

//Dependencies
#include "ftp/ftp_server.h"
#include "ftp/ftp_server_events.h"
#include "ftp/ftp_server_commands.h"
#include "ftp/ftp_server_misc.h"
#include "str.h"
#include "path.h"
#include "error.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (FTP_SERVER_SUPPORT == ENABLED)


/**
 * @brief Get a passive port number
 * @param[in] context Pointer to the FTP server context
 * @return Passive port number
 **/

uint16_t ftpServerGetPassivePort(FtpServerContext *context)
{
   uint_t port;

   //Retrieve current passive port number
   port = context->passivePort;

   //Invalid port number?
   if(port < context->settings.passivePortMin ||
      port > context->settings.passivePortMax)
   {
      //Generate a random port number
      port = context->settings.passivePortMin + netGetRand() %
         (context->settings.passivePortMax - context->settings.passivePortMin + 1);
   }

   //Next passive port to use
   if(port < context->settings.passivePortMax)
   {
      //Increment port number
      context->passivePort = port + 1;
   }
   else
   {
      //Wrap around if necessary
      context->passivePort = context->settings.passivePortMin;
   }

   //Return the passive port number
   return port;
}


/**
 * @brief Close client connection properly
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection to be closed
 **/

void ftpServerCloseConnection(FtpServerContext *context,
   FtpClientConnection *connection)
{
   uint_t i;

   //Make sure the connection is active
   if(connection != NULL)
   {
      //Loop through client connection table
      for(i = 0; i < FTP_SERVER_MAX_CONNECTIONS; i++)
      {
         //Search the table for the specified connection
         if(context->connection[i] == connection)
         {
            //Close data connection
            ftpServerCloseDataConnection(connection);
            //Close control connection
            ftpServerCloseControlConnection(connection);

            //Release previously allocated resources
            if(connection->file != NULL)
               fsCloseFile(connection->file);
            if(connection->dir != NULL)
               fsCloseDir(connection->dir);

            //Free memory
            memPoolFree(connection->buffer);
            memPoolFree(connection);

            //Mark the entry as free
            context->connection[i] = NULL;
            //We are done
            break;
         }
      }
   }
}


/**
 * @brief Accept control connection
 * @param[in] context Pointer to the FTP server context
 * @return Pointer to the connection
 **/

FtpClientConnection *ftpServerAcceptControlConnection(FtpServerContext *context)
{
   error_t error;
   uint_t i;
   Socket *socket;
   IpAddr clientIpAddr;
   uint16_t clientPort;
   FtpClientConnection *connection;

   //Accept incoming connection
   socket = socketAccept(context->socket, &clientIpAddr, &clientPort);
   //Failure detected?
   if(socket == NULL)
      return NULL;

   //Force the socket to operate in non-blocking mode
   error = socketSetTimeout(socket, 0);
   //Any error to report?
   if(error)
   {
      //Close socket
      socketClose(socket);
      //Exit immediately
      return NULL;
   }

   //Loop through client connection table
   for(i = 0; i < FTP_SERVER_MAX_CONNECTIONS; i++)
   {
      //Check whether the entry is currently in used or not
      if(context->connection[i] == NULL)
      {
         //Allocate resources for the new connection
         connection = memPoolAlloc(sizeof(FtpClientConnection));
         //Failed to allocate memory?
         if(connection == NULL)
         {
            //Debug message
            TRACE_ERROR("FTP server: Failed to allocate memory!\r\n");
            //Exit immediately
            break;
         }

         //Clear the structure
         memset(connection, 0, sizeof(FtpClientConnection));

         //Allocate a memory buffer for I/O operations
         connection->buffer = memPoolAlloc(FTP_SERVER_BUFFER_SIZE);
         //Failed to allocate memory
         if(connection->buffer == NULL)
         {
            //Clean up side effects
            memPoolFree(connection);
            //Debug message
            TRACE_ERROR("FTP server: Failed to allocate memory!\r\n");
            //Exit immediately
            break;
         }

         //Debug message
         TRACE_INFO("TCP server: Control connection established with client %s port %" PRIu16 "...\r\n",
            ipAddrToString(&clientIpAddr, NULL), clientPort);

         //Underlying network interface
         connection->interface = socket->interface;
         //Save socket handle
         connection->controlSocket = socket;
         //Set home directory
         strcpy(connection->homeDir, context->settings.rootDir);
         //Set current directory
         strcpy(connection->currentDir, context->settings.rootDir);

         //Format greeting message
         strcpy(connection->response, "220 Service ready for new user\r\n");
         //Debug message
         TRACE_DEBUG("FTP server: %s", connection->response);

         //Number of bytes in the response buffer
         connection->responseLength = strlen(connection->response);
         connection->responsePos = 0;

         //The client connection is ready for use
         context->connection[i] = connection;
         //Successful processing
         return connection;
      }
   }

   //Debug message
   TRACE_INFO("TCP server: Connection refused with client %s port %" PRIu16 "...\r\n",
      ipAddrToString(&clientIpAddr, NULL), clientPort);

   //Close socket
   socketClose(socket);
   //The FTP server cannot accept the incoming connection request
   return NULL;
}


/**
 * @brief Close control connection
 * @param[in] connection Pointer to the client connection
 **/

void ftpServerCloseControlConnection(FtpClientConnection *connection)
{
   IpAddr clientIpAddr;
   uint16_t clientPort;

   //Any running control connection?
   if(connection->controlSocket != NULL)
   {
      //Retrieve the address of the peer to which a socket is connected
      socketGetRemoteAddr(connection->controlSocket, &clientIpAddr, &clientPort);

      //Debug message
      TRACE_INFO("FTP server: Closing control connection with client %s port %" PRIu16 "...\r\n",
         ipAddrToString(&clientIpAddr, NULL), clientPort);

      //Close control connection
      socketClose(connection->controlSocket);
      connection->controlSocket = NULL;

      //Back to idle state
      connection->controlState = FTP_CONTROL_STATE_IDLE;
   }
}


/**
 * @brief Open data connection
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @return Error code
 **/

error_t ftpServerOpenDataConnection(FtpServerContext *context,
   FtpClientConnection *connection)
{
   error_t error;

   //Release previously allocated resources
   ftpServerCloseDataConnection(connection);

   //No port specified?
   if(!connection->remotePort)
      return ERROR_FAILURE;

   //Debug message
   TRACE_INFO("FTP server: Opening data connection with client %s port %" PRIu16 "...\r\n",
      ipAddrToString(&connection->remoteIpAddr, NULL), connection->remotePort);

   //Open data socket
   connection->dataSocket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
   //Failed to open socket?
   if(!connection->dataSocket)
      return ERROR_OPEN_FAILED;

   //Start of exception handling block
   do
   {
      //Force the socket to operate in non-blocking mode
      error = socketSetTimeout(connection->dataSocket, 0);
      //Any error to report?
      if(error)
         break;

      //Change the size of the TX buffer
      error = socketSetTxBufferSize(connection->dataSocket,
         FTP_SERVER_DATA_SOCKET_BUFFER_SIZE);
      //Any error to report?
      if(error)
         break;

      //Change the size of the RX buffer
      error = socketSetRxBufferSize(connection->dataSocket,
         FTP_SERVER_DATA_SOCKET_BUFFER_SIZE);
      //Any error to report?
      if(error)
         break;

      //Associate the socket with the relevant interface
      error = socketBindToInterface(connection->dataSocket, connection->interface);
      //Unable to bind the socket to the desired interface?
      if(error)
         break;

      //The server initiates the data connection from port 20
      error = socketBind(connection->dataSocket, &IP_ADDR_ANY,
         context->settings.dataPort);
      //Any error to report?
      if(error)
         break;

      //Establish data connection
      error = socketConnect(connection->dataSocket,
         &connection->remoteIpAddr, connection->remotePort);
      //Any error to report?
      if(error != NO_ERROR && error != ERROR_TIMEOUT)
         break;

      //Connection is being established
      error = NO_ERROR;

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Clean up side effects
      ftpServerCloseDataConnection(connection);
      //Exit immediately
      return error;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Accept data connection
 * @param[in] connection Pointer to the client connection
 **/

void ftpServerAcceptDataConnection(FtpClientConnection *connection)
{
   error_t error;
   Socket *socket;
   IpAddr clientIpAddr;
   uint16_t clientPort;

   //Accept incoming connection
   socket = socketAccept(connection->dataSocket, &clientIpAddr, &clientPort);
   //Failure detected?
   if(socket == NULL)
      return;

   //Debug message
   TRACE_INFO("FTP server: Data connection established with client %s port %" PRIu16 "...\r\n",
      ipAddrToString(&clientIpAddr, NULL), clientPort);

   //Close the listening socket
   socketClose(connection->dataSocket);
   //Save socket handle
   connection->dataSocket = socket;

   //Force the socket to operate in non-blocking mode
   error = socketSetTimeout(connection->dataSocket, 0);
   //Any error to report?
   if(error)
   {
      //Clean up side effects
      socketClose(connection->dataSocket);
      //Exit immediately
      return;
   }

   //Check current state
   if(connection->controlState == FTP_CONTROL_STATE_LIST ||
      connection->controlState == FTP_CONTROL_STATE_RETR)
   {
      //Prepare to send data
      connection->dataState = FTP_DATA_STATE_SEND;
   }
   else if(connection->controlState == FTP_CONTROL_STATE_STOR ||
      connection->controlState == FTP_CONTROL_STATE_APPE)
   {
      //Prepare to receive data
      connection->dataState = FTP_DATA_STATE_RECEIVE;
   }
   else
   {
      //Data transfer direction is unknown...
      connection->dataState = FTP_DATA_STATE_IDLE;
   }
}


/**
 * @brief Close data connection
 * @param[in] connection Pointer to the client connection
 **/

void ftpServerCloseDataConnection(FtpClientConnection *connection)
{
   IpAddr clientIpAddr;
   uint16_t clientPort;

   //Any running data connection?
   if(connection->dataSocket != NULL)
   {
      //Retrieve the address of the peer to which a socket is connected
      socketGetRemoteAddr(connection->dataSocket, &clientIpAddr, &clientPort);

      //Check whether the data connection is established
      if(clientPort != 0)
      {
         //Debug message
         TRACE_INFO("FTP server: Closing data connection with client %s port %" PRIu16 "...\r\n",
            ipAddrToString(&clientIpAddr, NULL), clientPort);
      }

      //Close data connection
      socketClose(connection->dataSocket);
      connection->dataSocket = NULL;

      //Re initialize data connection
      connection->passiveMode = FALSE;
      connection->remotePort = 0;

      //Back to default state
      connection->dataState = FTP_DATA_STATE_CLOSED;
   }
}


/**
 * @brief Retrieve the full pathname
 * @param[in] connection Pointer to the client connection
 * @param[in] inputPath Relative or absolute path
 * @param[out] outputPath Resulting full path
 * @param[in] maxLen Maximum acceptable path length
 * @return Error code
 **/

error_t ftpServerGetPath(FtpClientConnection *connection,
   const char_t *inputPath, char_t *outputPath, size_t maxLen)
{
   size_t n;

   //Relative or absolute path?
   if(pathIsRelative(inputPath))
   {
      //Sanity check
      if(strlen(connection->currentDir) > maxLen)
         return ERROR_FAILURE;

      //Copy current directory
      strcpy(outputPath, connection->currentDir);
      //Append the specified path
      pathCombine(outputPath, inputPath, maxLen);
   }
   else
   {
      //Sanity check
      if(strlen(connection->homeDir) > maxLen)
         return ERROR_FAILURE;

      //Copy home directory
      strcpy(outputPath, connection->homeDir);
      //Append the specified path
      pathCombine(outputPath, inputPath, maxLen);
   }

   //Clean the resulting path
   pathCanonicalize(outputPath);
   pathRemoveSlash(outputPath);

   //Calculate the length of the home directory
   n = strlen(connection->homeDir);

   //Make sure the pathname is valid
   if(strncmp(outputPath, connection->homeDir, n))
      return ERROR_INVALID_PATH;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Get permissions for the specified file or directory
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] path Canonical path of the file
 * @return Access rights for the specified file
 **/

uint_t ftpServerGetFilePermissions(FtpServerContext *context,
   FtpClientConnection *connection, const char_t *path)
{
   size_t n;
   uint_t perm;

   //Calculate the length of the home directory
   n = strlen(connection->homeDir);

   //Make sure the pathname is valid
   if(!strncmp(path, connection->homeDir, n))
   {
      //Strip root directory from the pathname
      path = ftpServerStripRootDir(context, path);

      //Invoke user-defined callback, if any
      if(context->settings.getFilePermCallback != NULL)
      {
         //Retrieve access rights for the specified file
         perm = context->settings.getFilePermCallback(connection, connection->user, path);
      }
      else
      {
         //Use default access rights
         perm = FTP_FILE_PERM_LIST | FTP_FILE_PERM_READ | FTP_FILE_PERM_WRITE;
      }
   }
   else
   {
      //The specified pathname is not valid
      perm = 0;
   }

   //Return access rights
   return perm;
}


/**
 * @brief Strip root dir from specified pathname
 * @param[in] context Pointer to the FTP server context
 * @param[in] path input pathname
 * @return Resulting pathname with root dir stripped
 **/

const char_t *ftpServerStripRootDir(FtpServerContext *context, const char_t *path)
{
   //Default directory
   static const char_t defaultDir[] = "/";

   //Local variables
   size_t m;
   size_t n;

   //Retrieve the length of the root directory
   n = strlen(context->settings.rootDir);
   //Retrieve the length of the specified pathname
   m = strlen(path);

   //Strip the root dir from the specified pathname
   if(n <= 1)
      return path;
   else if(n < m)
      return path + n;
   else
      return defaultDir;
}


/**
 * @brief Strip home directory from specified pathname
 * @param[in] connection Pointer to the client connection
 * @param[in] path input pathname
 * @return Resulting pathname with home directory stripped
 **/

const char_t *ftpServerStripHomeDir(FtpClientConnection *connection, const char_t *path)
{
   //Default directory
   static const char_t defaultDir[] = "/";

   //Local variables
   size_t m;
   size_t n;

   //Retrieve the length of the home directory
   n = strlen(connection->homeDir);
   //Retrieve the length of the specified pathname
   m = strlen(path);

   //Strip the home directory from the specified pathname
   if(n <= 1)
      return path;
   else if(n < m)
      return path + n;
   else
      return defaultDir;
}

#endif
