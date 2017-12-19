/**
 * @file ftp_server_events.c
 * @brief FTP server (event handlers)
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

//Abbreviated months
static const char months[13][4] =
{
   "   ",
   "Jan",
   "Feb",
   "Mar",
   "Apr",
   "May",
   "Jun",
   "Jul",
   "Aug",
   "Sep",
   "Oct",
   "Nov",
   "Dec"
};


/**
 * @brief Control connection event handler
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] eventFlags Event to be processed
 **/

void ftpServerControlEventHandler(FtpServerContext *context,
   FtpClientConnection *connection, uint_t eventFlags)
{
   error_t error;
   size_t n;

   //Send buffer is available for writing?
   if(eventFlags == SOCKET_EVENT_TX_READY)
   {
      //Send data back to the client
      error = socketSend(connection->controlSocket, connection->response +
         connection->responsePos, connection->responseLength, &n, 0);

      //Failed to send data?
      if(error != NO_ERROR && error != ERROR_TIMEOUT)
      {
         //Close connection with the client
         ftpServerCloseConnection(context, connection);
         //Exit immediately
         return;
      }

      //Advance data pointer
      connection->responsePos += n;
      //Number of bytes still available in the response buffer
      connection->responseLength -= n;
   }
   //Data is pending in the receive buffer?
   else if(eventFlags == SOCKET_EVENT_RX_READY)
   {
      //Read data from the client
      error = socketReceive(connection->controlSocket,
         connection->command + connection->commandLength,
         FTP_SERVER_MAX_LINE_LEN - connection->commandLength, &n, 0);

      //Failed to receive data?
      if(error == ERROR_END_OF_STREAM)
      {
         //Gracefully disconnect from the remote host
         connection->controlState = FTP_CONTROL_STATE_WAIT_ACK;
         //Exit immediately
         return;
      }
      else if(error)
      {
         //Close connection with the client
         ftpServerCloseConnection(context, connection);
         //Exit immediately
         return;
      }

      //Number of bytes available in the command buffer
      connection->commandLength += n;
      //Process incoming command
      ftpServerProcessCmd(context, connection);
   }
   //Data are transmitted and acknowledged?
   else if(eventFlags == SOCKET_EVENT_TX_ACKED)
   {
      //Disable transmission
      socketShutdown(connection->controlSocket, SOCKET_SD_SEND);
      //Next state
      connection->controlState = FTP_CONTROL_STATE_SHUTDOWN_TX;
   }
   //Transmission is shut down?
   else if(eventFlags == SOCKET_EVENT_TX_SHUTDOWN)
   {
      //Disable reception
      socketShutdown(connection->controlSocket, SOCKET_SD_RECEIVE);
      //Next state
      connection->controlState = FTP_CONTROL_STATE_SHUTDOWN_RX;
   }
   //Reception is shut down?
   else if(eventFlags == SOCKET_EVENT_RX_SHUTDOWN)
   {
      //Properly close connection
      ftpServerCloseConnection(context, connection);
   }
}


/**
 * @brief Data connection event handler
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] eventFlags Event to be processed
 **/

void ftpServerDataEventHandler(FtpServerContext *context,
   FtpClientConnection *connection, uint_t eventFlags)
{
   //Any connection attempt?
   if(connection->dataState == FTP_DATA_STATE_LISTEN)
   {
      //Accept data connection
      ftpServerAcceptDataConnection(connection);
   }
   //Ready to send data?
   else if(connection->dataState == FTP_DATA_STATE_SEND)
   {
      //Send more data to the remote host
      ftpServerSendData(context, connection);
   }
   //Any data pending in the receive buffer?
   else if(connection->dataState == FTP_DATA_STATE_RECEIVE)
   {
      //Process incoming data
      ftpServerReceiveData(context, connection);
   }
   //Data are transmitted and acknowledged?
   else if(connection->dataState == FTP_DATA_STATE_WAIT_ACK)
   {
      //Disable transmission
      socketShutdown(connection->dataSocket, SOCKET_SD_SEND);
      //Next state
      connection->dataState = FTP_DATA_STATE_SHUTDOWN_TX;
   }
   //Transmission is shut down?
   else if(connection->dataState == FTP_DATA_STATE_SHUTDOWN_TX)
   {
      //Disable reception
      socketShutdown(connection->dataSocket, SOCKET_SD_RECEIVE);
      //Next state
      connection->dataState = FTP_DATA_STATE_SHUTDOWN_RX;
   }
   //Reception is shut down?
   else if(connection->dataState == FTP_DATA_STATE_SHUTDOWN_RX)
   {
      //Close the data connection
      ftpServerCloseDataConnection(connection);

      //Back to idle state
      connection->controlState = FTP_CONTROL_STATE_IDLE;

      //Transfer status
      strcpy(connection->response, "226 Transfer complete\r\n");
      //Debug message
      TRACE_DEBUG("FTP server: %s", connection->response);

      //Number of bytes in the response buffer
      connection->responseLength = strlen(connection->response);
      connection->responsePos = 0;
   }
}


/**
 * @brief Send data on the data connection
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 **/

void ftpServerSendData(FtpServerContext *context, FtpClientConnection *connection)
{
   error_t error;
   size_t n;

   //Any data waiting for transmission?
   if(connection->bufferLength > 0)
   {
      //Send more data
      error = socketSend(connection->dataSocket, connection->buffer +
         connection->bufferPos, connection->bufferLength, &n, 0);

      //Failed to send data?
      if(error != NO_ERROR && error != ERROR_TIMEOUT)
      {
         //Close the data connection
         ftpServerCloseDataConnection(connection);

         //Release previously allocated resources
         if(connection->file != NULL)
         {
            fsCloseFile(connection->file);
            connection->file = NULL;
         }
         if(connection->dir != NULL)
         {
            fsCloseDir(connection->dir);
            connection->dir = NULL;
         }

         //Back to idle state
         connection->controlState = FTP_CONTROL_STATE_IDLE;

         //Transfer status
         strcpy(connection->response, "451 Transfer aborted\r\n");
         //Debug message
         TRACE_DEBUG("FTP server: %s", connection->response);

         //Number of bytes in the response buffer
         connection->responseLength = strlen(connection->response);
         connection->responsePos = 0;

         //Exit immediately
         return;
      }

      //Advance data pointer
      connection->bufferPos += n;
      //Number of bytes still available in the buffer
      connection->bufferLength -= n;
   }

   //Empty transmission buffer?
   if(connection->bufferLength == 0)
   {
      //File transfer in progress?
      if(connection->controlState == FTP_CONTROL_STATE_RETR)
      {
         //Read more data
         error = fsReadFile(connection->file,
            connection->buffer, FTP_SERVER_BUFFER_SIZE, &n);

         //End of stream?
         if(error)
         {
            //Close file
            fsCloseFile(connection->file);
            connection->file = NULL;

            //Wait for all the data to be transmitted and acknowledged
            connection->dataState = FTP_DATA_STATE_WAIT_ACK;

            //Exit immediately
            return;
         }
      }
      //Directory listing in progress?
      else if(connection->controlState == FTP_CONTROL_STATE_LIST)
      {
         uint_t perm;
         time_t currentTime;
         time_t modified;
         char_t *path;
         FsDirEntry dirEntry;

         //Read a new entry in the directory
         error = fsReadDir(connection->dir, &dirEntry);

         //End of stream?
         if(error)
         {
            //Close directory
            fsCloseDir(connection->dir);
            connection->dir = NULL;

            //Wait for all the data to be transmitted and acknowledged
            connection->dataState = FTP_DATA_STATE_WAIT_ACK;

            //Exit immediately
            return;
         }

         //Point to the scratch buffer
         path = connection->buffer;

         //Get the pathname of the directory being listed
         strcpy(path, connection->path);
         //Retrieve the full pathname
         pathCombine(path, dirEntry.name, FTP_SERVER_MAX_PATH_LEN);
         pathCanonicalize(path);

         //Get permissions for the specified file
         perm = ftpServerGetFilePermissions(context, connection, path);

         //Enforce access rights
         if(perm & FTP_FILE_PERM_LIST)
         {
            //Format links, owner, group and size fields
            n = sprintf(connection->buffer, "----------   1 owner    group    %10" PRIu32,
               dirEntry.size);

            //Check whether the current entry is a directory
            if(dirEntry.attributes & FS_FILE_ATTR_DIRECTORY)
               connection->buffer[0] = 'd';

            //Read access permitted?
            if(perm & FTP_FILE_PERM_READ)
            {
               connection->buffer[1] = 'r';
               connection->buffer[4] = 'r';
               connection->buffer[7] = 'r';
            }

            //Write access permitted?
            if(perm & FTP_FILE_PERM_WRITE)
            {
               //Make sure the file is not marked as read-only
               if(!(dirEntry.attributes & FS_FILE_ATTR_READ_ONLY))
               {
                  connection->buffer[2] = 'w';
                  connection->buffer[5] = 'w';
                  connection->buffer[8] = 'w';
               }
            }

            //Get current time
            currentTime = getCurrentUnixTime();
            //Get modification time
            modified = convertDateToUnixTime(&dirEntry.modified);

            //Check whether the modification time is within the previous 180 days
            if(currentTime > modified && currentTime < (modified + FTP_SERVER_180_DAYS))
            {
               //The format of the date/time field is Mmm dd hh:mm
               n += sprintf(connection->buffer + n, " %s %02" PRIu8 " %02" PRIu8 ":%02" PRIu8,
                  months[MIN(dirEntry.modified.month, 12)], dirEntry.modified.day,
                  dirEntry.modified.hours, dirEntry.modified.minutes);
            }
            else
            {
               //The format of the date/time field is Mmm dd  yyyy
               n += sprintf(connection->buffer + n, " %s %02" PRIu8 "  %04" PRIu16,
                  months[MIN(dirEntry.modified.month, 12)], dirEntry.modified.day,
                  dirEntry.modified.year);
            }

            //Append filename
            n += sprintf(connection->buffer + n, " %s\r\n", dirEntry.name);
            //Debug message
            TRACE_DEBUG("FTP server: %s", connection->buffer);
         }
         else
         {
            //Insufficient access rights
            n = 0;
         }
      }
      //Invalid state?
      else
      {
         //The FTP server has encountered a critical error
         ftpServerCloseConnection(context, connection);
         //Exit immediately
         return;
      }

      //Number of bytes in the buffer
      connection->bufferPos = 0;
      connection->bufferLength = n;
   }
}


/**
 * @brief Receive data on the data connection
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 **/

void ftpServerReceiveData(FtpServerContext *context, FtpClientConnection *connection)
{
   error_t error;
   bool_t eof;
   size_t n;

   //File transfer in progress?
   if(connection->controlState == FTP_CONTROL_STATE_STOR ||
      connection->controlState == FTP_CONTROL_STATE_APPE)
   {
      //Read incoming data
      error = socketReceive(connection->dataSocket,
         connection->buffer + connection->bufferPos,
         FTP_SERVER_BUFFER_SIZE - connection->bufferLength, &n, 0);

      //Any error to report?
      if(error)
      {
         //Cannot read more data
         eof = TRUE;
      }
      else
      {
         //Successful read operation
         eof = FALSE;

         //Advance data pointer
         connection->bufferPos += n;
         connection->bufferLength += n;
      }

      //Read data until the buffer is full or the end of the file is reached
      if(eof || connection->bufferLength >= FTP_SERVER_BUFFER_SIZE)
      {
         //Any data to be written?
         if(connection->bufferLength > 0)
         {
            //Write data to the specified file
            error = fsWriteFile(connection->file,
               connection->buffer, connection->bufferLength);

            //Any error to report?
            if(error)
            {
               //Close the data connection
               ftpServerCloseDataConnection(connection);

               //Release previously allocated resources
               fsCloseFile(connection->file);
               connection->file = NULL;

               //Back to idle state
               connection->controlState = FTP_CONTROL_STATE_IDLE;

               //Transfer status
               strcpy(connection->response, "451 Transfer aborted\r\n");
               //Debug message
               TRACE_DEBUG("FTP server: %s", connection->response);

               //Number of bytes in the response buffer
               connection->responseLength = strlen(connection->response);
               connection->responsePos = 0;

               //Exit immediately
               return;
            }
         }

         //Flush reception buffer
         connection->bufferLength = 0;
         connection->bufferPos = 0;
      }

      //End of stream?
      if(eof)
      {
         //Close file
         fsCloseFile(connection->file);
         connection->file = NULL;

         //Graceful shutdown sequence
         connection->dataState = FTP_DATA_STATE_WAIT_ACK;
      }
   }
   //Invalid state?
   else
   {
      //The FTP server has encountered a critical error
      ftpServerCloseConnection(context, connection);
   }
}

#endif
