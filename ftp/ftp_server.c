/**
 * @file ftp_server.c
 * @brief FTP server (File Transfer Protocol)
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
 * File Transfer Protocol (FTP) is a standard network protocol used to
 * transfer files from one host to another host over a TCP-based network.
 * Refer to the following RFCs for complete details:
 * - RFC 959: File Transfer Protocol (FTP)
 * - RFC 3659: Extensions to FTP
 * - RFC 2428: FTP Extensions for IPv6 and NATs
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
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains FTP server settings
 **/

void ftpServerGetDefaultSettings(FtpServerSettings *settings)
{
   //The FTP server is not bound to any interface
   settings->interface = NULL;

   //FTP command port number
   settings->port = FTP_PORT;
   //FTP data port number
   settings->dataPort = FTP_DATA_PORT;
   //Passive port range
   settings->passivePortMin = FTP_SERVER_PASSIVE_PORT_MIN;
   settings->passivePortMax = FTP_SERVER_PASSIVE_PORT_MAX;
   //Set root directory
   strcpy(settings->rootDir, "/");
   //User verification callback function
   settings->checkUserCallback = NULL;
   //Password verification callback function
   settings->checkPasswordCallback = NULL;
   //Callback used to retrieve file permissions
   settings->getFilePermCallback = NULL;
   //Unknown command callback function
   settings->unknownCommandCallback = NULL;
}


/**
 * @brief FTP server initialization
 * @param[in] context Pointer to the FTP server context
 * @param[in] settings FTP server specific settings
 * @return Error code
 **/

error_t ftpServerInit(FtpServerContext *context, const FtpServerSettings *settings)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing FTP server...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;

   //Check passive port range
   if(settings->passivePortMax <= settings->passivePortMin)
      return ERROR_INVALID_PARAMETER;

   //Clear the FTP server context
   memset(context, 0, sizeof(FtpServerContext));

   //Save user settings
   context->settings = *settings;

   //Clean the root directory path
   pathCanonicalize(context->settings.rootDir);
   pathRemoveSlash(context->settings.rootDir);

   //Create an event object to poll the state of sockets
   if(!osCreateEvent(&context->event))
   {
      //Failed to create event
      return ERROR_OUT_OF_RESOURCES;
   }

   //Start of exception handling block
   do
   {
      //Open a TCP socket
      context->socket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
      //Failed to open socket?
      if(!context->socket)
      {
         //Report an error
         error = ERROR_OPEN_FAILED;
         //Exit immediately
         break;
      }

      //Set timeout for blocking functions
      error = socketSetTimeout(context->socket, INFINITE_DELAY);
      //Any error to report?
      if(error)
         break;

      //Change the size of the TX buffer
      error = socketSetTxBufferSize(context->socket,
         FTP_SERVER_CTRL_SOCKET_BUFFER_SIZE);
      //Any error to report?
      if(error)
         break;

      //Change the size of the RX buffer
      error = socketSetRxBufferSize(context->socket,
         FTP_SERVER_CTRL_SOCKET_BUFFER_SIZE);
      //Any error to report?
      if(error)
         break;

      //Associate the socket with the relevant interface
      error = socketBindToInterface(context->socket, settings->interface);
      //Unable to bind the socket to the desired interface?
      if(error)
         break;

      //Bind newly created socket to port 21
      error = socketBind(context->socket, &IP_ADDR_ANY, settings->port);
      //Failed to bind socket to port 21?
      if(error)
         break;

      //Place socket in listening state
      error = socketListen(context->socket, FTP_SERVER_BACKLOG);
      //Any failure to report?
      if(error)
         break;

      //End of exception handling block
   } while(0);

   //Did we encounter an error?
   if(error)
   {
      //Free previously allocated resources
      osDeleteEvent(&context->event);
      //Close socket
      socketClose(context->socket);
   }

   //Return status code
   return error;
}


/**
 * @brief Start FTP server
 * @param[in] context Pointer to the FTP server context
 * @return Error code
 **/

error_t ftpServerStart(FtpServerContext *context)
{
   OsTask *task;

   //Debug message
   TRACE_INFO("Starting FTP server...\r\n");

   //Make sure the FTP server context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Create the FTP server task
   task = osCreateTask("FTP Server", (OsTaskCode) ftpServerTask,
      context, FTP_SERVER_STACK_SIZE, FTP_SERVER_PRIORITY);

   //Unable to create the task?
   if(task == OS_INVALID_HANDLE)
      return ERROR_OUT_OF_RESOURCES;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set home directory
 * @param[in] connection Pointer to the client connection
 * @param[in] homeDir NULL-terminated string specifying the home directory
 * @return Error code
 **/

error_t ftpServerSetHomeDir(FtpClientConnection *connection, const char_t *homeDir)
{
   //Ensure the parameters are valid
   if(connection == NULL || homeDir == NULL)
      return ERROR_INVALID_PARAMETER;

   //Set home directory
   pathCombine(connection->homeDir, homeDir, FTP_SERVER_MAX_HOME_DIR_LEN);
   //Clean the resulting path
   pathCanonicalize(connection->homeDir);
   pathRemoveSlash(connection->homeDir);

   //Set current directory
   strcpy(connection->currentDir, connection->homeDir);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief FTP server task
 * @param[in] context Pointer to the FTP server context
 **/

void ftpServerTask(FtpServerContext *context)
{
   error_t error;
   uint_t i;
   systime_t time;
   FtpClientConnection *connection;

#if (NET_RTOS_SUPPORT == ENABLED)
   //Process events
   while(1)
   {
#endif
      //Clear event descriptor set
      memset(context->eventDesc, 0, sizeof(context->eventDesc));

      //Specify the events the application is interested in
      for(i = 0; i < FTP_SERVER_MAX_CONNECTIONS; i++)
      {
         //Point to the structure describing the current connection
         connection = context->connection[i];

         //Loop through active connections only
         if(connection != NULL)
         {
            //Ensure the control connection is opened
            if(connection->controlSocket != NULL)
            {
               //Check the state of the control connection
               if(connection->responseLength > 0)
               {
                  //Wait until there is more room in the send buffer
                  context->eventDesc[2 * i].socket = connection->controlSocket;
                  context->eventDesc[2 * i].eventMask = SOCKET_EVENT_TX_READY;
               }
               else if(connection->controlState == FTP_CONTROL_STATE_WAIT_ACK)
               {
                  //Wait for all the data to be transmitted and acknowledged
                  context->eventDesc[2 * i].socket = connection->controlSocket;
                  context->eventDesc[2 * i].eventMask = SOCKET_EVENT_TX_ACKED;
               }
               else if(connection->controlState == FTP_CONTROL_STATE_SHUTDOWN_TX)
               {
                  //Wait for the FIN to be acknowledged
                  context->eventDesc[2 * i].socket = connection->controlSocket;
                  context->eventDesc[2 * i].eventMask = SOCKET_EVENT_TX_SHUTDOWN;
               }
               else if(connection->controlState == FTP_CONTROL_STATE_SHUTDOWN_RX)
               {
                  //Wait for a FIN to be received
                  context->eventDesc[2 * i].socket = connection->controlSocket;
                  context->eventDesc[2 * i].eventMask = SOCKET_EVENT_RX_SHUTDOWN;
               }
               else
               {
                  //Wait for data to be available for reading
                  context->eventDesc[2 * i].socket = connection->controlSocket;
                  context->eventDesc[2 * i].eventMask = SOCKET_EVENT_RX_READY;
               }
            }

            //Ensure the data connection is opened
            if(connection->dataSocket != NULL)
            {
               //Check the state of the data connection
               if(connection->dataState == FTP_DATA_STATE_LISTEN ||
                  connection->dataState == FTP_DATA_STATE_RECEIVE)
               {
                  //Wait for data to be available for reading
                  context->eventDesc[2 * i + 1].socket = connection->dataSocket;
                  context->eventDesc[2 * i + 1].eventMask = SOCKET_EVENT_RX_READY;
               }
               else if(connection->dataState == FTP_DATA_STATE_SEND)
               {
                  //Wait until there is more room in the send buffer
                  context->eventDesc[2 * i + 1].socket = connection->dataSocket;
                  context->eventDesc[2 * i + 1].eventMask = SOCKET_EVENT_TX_READY;
               }

               else if(connection->dataState == FTP_DATA_STATE_WAIT_ACK)
               {
                  //Wait for all the data to be transmitted and acknowledged
                  context->eventDesc[2 * i + 1].socket = connection->dataSocket;
                  context->eventDesc[2 * i + 1].eventMask = SOCKET_EVENT_TX_ACKED;
               }
               else if(connection->dataState == FTP_DATA_STATE_SHUTDOWN_TX)
               {
                  //Wait for the FIN to be acknowledged
                  context->eventDesc[2 * i + 1].socket = connection->dataSocket;
                  context->eventDesc[2 * i + 1].eventMask = SOCKET_EVENT_TX_SHUTDOWN;
               }
               else if(connection->dataState == FTP_DATA_STATE_SHUTDOWN_RX)
               {
                  //Wait for a FIN to be received
                  context->eventDesc[2 * i + 1].socket = connection->dataSocket;
                  context->eventDesc[2 * i + 1].eventMask = SOCKET_EVENT_RX_SHUTDOWN;
               }
            }
         }
      }

      //Accept connection request events
      context->eventDesc[2 * i].socket = context->socket;
      context->eventDesc[2 * i].eventMask = SOCKET_EVENT_RX_READY;

      //Wait for one of the set of sockets to become ready to perform I/O
      error = socketPoll(context->eventDesc, 2 * FTP_SERVER_MAX_CONNECTIONS + 1,
         &context->event, FTP_SERVER_SOCKET_POLLING_TIMEOUT);

      //Get current time
      time = osGetSystemTime();

      //Verify status code
      if(!error)
      {
         //Event-driven processing
         for(i = 0; i < FTP_SERVER_MAX_CONNECTIONS; i++)
         {
            //Point to the structure describing the current connection
            connection = context->connection[i];

            //Make sure the control connection is still active
            if(connection != NULL && connection->controlSocket != NULL)
            {
               //Check whether the control socket is to ready to perform I/O
               if(context->eventDesc[2 * i].eventFlags)
               {
                  //Update time stamp
                  connection->timestamp = time;
                  //Control connection event handler
                  ftpServerControlEventHandler(context, connection,
                     context->eventDesc[2 * i].eventFlags);
               }
            }

            //The connection may have been closed...
            connection = context->connection[i];

            //Make sure the data connection is still active
            if(connection != NULL && connection->dataSocket != NULL)
            {
               //Check whether the data socket to is ready to perform I/O
               if(context->eventDesc[2 * i + 1].eventFlags)
               {
                  //Update time stamp
                  connection->timestamp = time;
                  //Data connection event handler
                  ftpServerDataEventHandler(context, connection,
                     context->eventDesc[2 * i + 1].eventFlags);
               }
            }
         }

         //Check the state of the listening socket
         if(context->eventDesc[2 * i].eventFlags & SOCKET_EVENT_RX_READY)
         {
            //Process incoming connection request
            connection = ftpServerAcceptControlConnection(context);
            //Initialize time stamp
            if(connection != NULL)
               connection->timestamp = time;
         }
      }

      //Manage timeouts
      for(i = 0; i < FTP_SERVER_MAX_CONNECTIONS; i++)
      {
         //Point to the structure describing the current connection
         connection = context->connection[i];

         //Any client connected?
         if(connection != NULL)
         {
            //Disconnect inactive client after idle timeout
            if((time - connection->timestamp) >= FTP_SERVER_TIMEOUT)
            {
               //Debug message
               TRACE_INFO("FTP server: Closing inactive connection...\r\n");
               //Close connection with the client
               ftpServerCloseConnection(context, connection);
            }
         }
      }
#if (NET_RTOS_SUPPORT == ENABLED)
   }
#endif
}

#endif
