/**
 * @file ftp_server_commands.c
 * @brief FTP server (command processing)
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
#include <stdlib.h>
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
 * @brief FTP command processing
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 **/

void ftpServerProcessCmd(FtpServerContext *context,
   FtpClientConnection *connection)
{
   size_t n;
   char_t *p;

   //The <CRLF> sequence should be used to terminate the command line
   for(n = 0; n < connection->commandLength; n++)
   {
      if(connection->command[n] == '\n')
         break;
   }

   //Any command to process?
   if(n < connection->commandLength)
   {
      //Properly terminate the string with a NULL character
      connection->command[n] = '\0';
      //Remove trailing whitespace from the command line
      strRemoveTrailingSpace(connection->command);

      //Debug message
      TRACE_DEBUG("FTP client: %s\r\n", connection->command);

      //Command line too long?
      if(connection->controlState == FTP_CONTROL_STATE_DISCARD)
      {
         //Switch to idle state
         connection->controlState = FTP_CONTROL_STATE_IDLE;
         //Format response message
         strcpy(connection->response, "500 Command line too long\r\n");
      }
      else
      {
         //The command name and the arguments are separated by one or more spaces
         for(p = connection->command; *p != '\0' && *p != ' '; p++);

         //Space character found?
         if(*p == ' ')
         {
            //Split the string at the first occurrence of the space character
            *(p++) = '\0';
            //Skip extra whitespace
            while(*p == ' ') p++;
         }

         //NOOP command received
         if(!strcasecmp(connection->command, "NOOP"))
            ftpServerProcessNoop(context, connection, p);
         //SYST command received
         else if(!strcasecmp(connection->command, "SYST"))
            ftpServerProcessSyst(context, connection, p);
         //FEAT command received?
         else if(!strcasecmp(connection->command, "FEAT"))
            ftpServerProcessFeat(context, connection, p);
         //TYPE command received?
         else if(!strcasecmp(connection->command, "TYPE"))
            ftpServerProcessType(context, connection, p);
         //STRU command received?
         else if(!strcasecmp(connection->command, "STRU"))
            ftpServerProcessStru(context, connection, p);
         //MODE command received?
         else if(!strcasecmp(connection->command, "MODE"))
            ftpServerProcessMode(context, connection, p);
         //USER command received?
         else if(!strcasecmp(connection->command, "USER"))
            ftpServerProcessUser(context, connection, p);
         //PASS command received?
         else if(!strcasecmp(connection->command, "PASS"))
            ftpServerProcessPass(context, connection, p);
         //REIN command received?
         else if(!strcasecmp(connection->command, "REIN"))
            ftpServerProcessRein(context, connection, p);
         //QUIT command received?
         else if(!strcasecmp(connection->command, "QUIT"))
            ftpServerProcessQuit(context, connection, p);
         //PORT command received?
         else if(!strcasecmp(connection->command, "PORT"))
            ftpServerProcessPort(context, connection, p);
         //EPRT command received?
         else if(!strcasecmp(connection->command, "EPRT"))
            ftpServerProcessEprt(context, connection, p);
         //PASV command received?
         else if(!strcasecmp(connection->command, "PASV"))
            ftpServerProcessPasv(context, connection, p);
         //EPSV command received?
         else if(!strcasecmp(connection->command, "EPSV"))
            ftpServerProcessEpsv(context, connection, p);
         //ABOR command received?
         else if(!strcasecmp(connection->command, "ABOR"))
            ftpServerProcessAbor(context, connection, p);
         //PWD command received?
         else if(!strcasecmp(connection->command, "PWD"))
            ftpServerProcessPwd(context, connection, p);
         //LIST command received?
         else if(!strcasecmp(connection->command, "LIST"))
            ftpServerProcessList(context, connection, p);
         //CWD command received?
         else if(!strcasecmp(connection->command, "CWD"))
            ftpServerProcessCwd(context, connection, p);
         //CDUP command received?
         else if(!strcasecmp(connection->command, "CDUP"))
            ftpServerProcessCdup(context, connection, p);
         //MKD command received?
         else if(!strcasecmp(connection->command, "MKD"))
            ftpServerProcessMkd(context, connection, p);
         //RMD command received?
         else if(!strcasecmp(connection->command, "RMD"))
            ftpServerProcessRmd(context, connection, p);
         //SIZE command received?
         else if(!strcasecmp(connection->command, "SIZE"))
            ftpServerProcessSize(context, connection, p);
         //RETR command received?
         else if(!strcasecmp(connection->command, "RETR"))
            ftpServerProcessRetr(context, connection, p);
         //STOR command received?
         else if(!strcasecmp(connection->command, "STOR"))
            ftpServerProcessStor(context, connection, p);
         //APPE command received?
         else if(!strcasecmp(connection->command, "APPE"))
            ftpServerProcessAppe(context, connection, p);
         //RNFR command received?
         else if(!strcasecmp(connection->command, "RNFR"))
            ftpServerProcessRnfr(context, connection, p);
         //RNTO command received?
         else if(!strcasecmp(connection->command, "RNTO"))
            ftpServerProcessRnto(context, connection, p);
         //DELE command received?
         else if(!strcasecmp(connection->command, "DELE"))
            ftpServerProcessDele(context, connection, p);
         //Unknown command received?
         else
            ftpServerProcessUnknownCmd(context, connection, p);
      }

      //Debug message
      TRACE_DEBUG("FTP server: %s", connection->response);

      //Number of bytes in the response buffer
      connection->responseLength = strlen(connection->response);
      connection->responsePos = 0;

      //Clear command line
      connection->commandLength = 0;
   }
   else if(connection->commandLength >= FTP_SERVER_MAX_LINE_LEN)
   {
      //The command line is too long...
      connection->controlState = FTP_CONTROL_STATE_DISCARD;
      //Drop incoming data
      connection->commandLength = 0;
   }
}


/**
 * @brief Unknown command processing
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessUnknownCmd(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;

   //Invoke user-defined callback, if any
   if(context->settings.unknownCommandCallback != NULL)
   {
      //Custom command processing
      error = context->settings.unknownCommandCallback(connection,
         connection->command, param);
   }
   else
   {
      //Report an error
      error = ERROR_INVALID_COMMAND;
   }

   //Invalid command received?
   if(error == ERROR_INVALID_COMMAND)
   {
      //Format response message
      strcpy(connection->response, "500 Command unrecognized\r\n");
   }
}


/**
 * @brief NOOP command processing
 *
 * The NOOP command does not affect any parameters or previously entered
 * commands. It specifies no action other than that the server send an OK reply
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessNoop(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //Send an OK reply
   strcpy(connection->response, "200 Command okay\r\n");
}


/**
 * @brief SYST command processing
 *
 * The SYST command is used to find out the type of operating system
 * at the server side
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessSyst(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //Format the response to the SYST command
   strcpy(connection->response, "215 UNIX Type: L8\r\n");
}


/**
 * @brief FEAT command processing
 *
 * The FEAT command allows a client to discover which optional
 * commands a server supports
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessFeat(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //Format the response to the FEAT command
   strcpy(connection->response, "211-Features supported:\r\n");
   strcat(connection->response, " SIZE\r\n");
   strcat(connection->response, " EPRT\r\n");
   strcat(connection->response, " EPSV\r\n");
   strcat(connection->response, "211 End\r\n");
}


/**
 * @brief TYPE command processing
 *
 * The TYPE command specifies the representation type
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessType(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //The argument specifies the representation type
   if(*param != '\0')
   {
      //ASCII type?
      if(!strcasecmp(param, "A"))
      {
         //Format the response to the TYPE command
         strcpy(connection->response, "200 Type set to A\r\n");
      }
      //Image type?
      else if(!strcasecmp(param, "I"))
      {
         //Format the response to the TYPE command
         strcpy(connection->response, "200 Type set to I\r\n");
      }
      //Unknown type?
      else
      {
         //Report an error
         strcpy(connection->response, "504 Unknown type\r\n");
      }
   }
   else
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
   }
}


/**
 * @brief STRU command processing
 *
 * The STRU command specifies the file structure
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessStru(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //The argument specifies the file structure
   if(*param != '\0')
   {
      //No record structure?
      if(!strcasecmp(param, "F"))
      {
         //Format the response to the STRU command
         strcpy(connection->response, "200 Structure set to F\r\n");
      }
      //Unknown file structure?
      else
      {
         //Report an error
         strcpy(connection->response, "504 Unknown structure\r\n");
      }
   }
   else
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
   }
}


/**
 * @brief MODE command processing
 *
 * The MODE command specifies the data transfer mode
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessMode(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //The argument specifies the data transfer mode
   if(*param != '\0')
   {
      //Stream mode?
      if(!strcasecmp(param, "S"))
      {
         //Format the response to the MODE command
         strcpy(connection->response, "200 Mode set to S\r\n");
      }
      //Unknown data transfer mode?
      else
      {
         //Report an error
         strcpy(connection->response, "504 Unknown mode\r\n");
      }
   }
   else
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
   }
}


/**
 * @brief USER command processing
 *
 * The USER command is used to identify the user
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessUser(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   uint_t status;

   //The argument specifies the user name
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Check the length of the user name
   if(strlen(param) > FTP_SERVER_MAX_USERNAME_LEN)
   {
      //The specified user name is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Save user name
   strcpy(connection->user, param);
   //Log out the user
   connection->userLoggedIn = FALSE;
   //Set home directory
   strcpy(connection->homeDir, context->settings.rootDir);
   //Set current directory
   strcpy(connection->currentDir, context->settings.rootDir);

   //Invoke user-defined callback, if any
   if(context->settings.checkUserCallback != NULL)
      status = context->settings.checkUserCallback(connection, param);
   else
      status = FTP_ACCESS_ALLOWED;

   //Access allowed?
   if(status == FTP_ACCESS_ALLOWED)
   {
      //The user is now logged in
      connection->userLoggedIn = TRUE;
      //Format response message
      strcpy(connection->response, "230 User logged in, proceed\r\n");
   }
   //Password required?
   else if(status == FTP_PASSWORD_REQUIRED)
   {
      //This command must be immediately followed by a PASS command
      connection->controlState = FTP_CONTROL_STATE_USER;
      //Format response message
      strcpy(connection->response, "331 User name okay, need password\r\n");
   }
   //Access denied?
   else
   {
      //Format response message
      strcpy(connection->response, "530 Login authentication failed\r\n");
   }
}


/**
 * @brief PASS command processing
 *
 * The USER command specifies the user's password
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessPass(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   uint_t status;

   //This command must immediately follow a USER command
   if(connection->controlState != FTP_CONTROL_STATE_USER)
   {
      //Switch to idle state
      connection->controlState = FTP_CONTROL_STATE_IDLE;
      //Report an error
      strcpy(connection->response, "503 Bad sequence of commands\r\n");
      //Exit immediately
      return;
   }

   //Switch to idle state
   connection->controlState = FTP_CONTROL_STATE_IDLE;

   //The argument specifies the password
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Invoke user-defined callback, if any
   if(context->settings.checkPasswordCallback != NULL)
      status = context->settings.checkPasswordCallback(connection, connection->user, param);
   else
      status = FTP_ACCESS_ALLOWED;

   //Access allowed?
   if(status == FTP_ACCESS_ALLOWED)
   {
      //The user is now logged in
      connection->userLoggedIn = TRUE;
      //Format response message
      strcpy(connection->response, "230 User logged in, proceed\r\n");
   }
   //Access denied?
   else
   {
      //Format response message
      strcpy(connection->response, "530 Login authentication failed\r\n");
   }
}


/**
 * @brief REIN command processing
 *
 * The REIN command is used to reinitialize a user session
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessRein(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //Close data connection
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

   //Clear account information
   connection->userLoggedIn = FALSE;

   //Format response message
   strcpy(connection->response, "220 Service ready for new user\r\n");
}


/**
 * @brief QUIT command processing
 *
 * The QUIT command is used to terminate a user session
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessQuit(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //There are two cases to consider upon receipt of this command
   if(connection->dataState == FTP_DATA_STATE_CLOSED)
   {
      //If the FTP service command was already completed, the server closes
      //the data connection (if it is open)...
      ftpServerCloseDataConnection(connection);

      //...and responds with a 221 reply
      strcpy(connection->response, "221 Service closing control connection\r\n");
   }
   else
   {
      //If the FTP service command is still in progress, the server aborts
      //the FTP service in progress and closes the data connection...
      ftpServerCloseDataConnection(connection);

      //...returning a 426 reply to indicate that the service request
      //terminated abnormally
      strcpy(connection->response, "426 Connection closed; transfer aborted\r\n");

      //The server then sends a 221 reply
      strcat(connection->response, "221 Service closing control connection\r\n");
   }

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

   //Clear account information
   connection->userLoggedIn = FALSE;
   //Gracefully disconnect from the remote host
   connection->controlState = FTP_CONTROL_STATE_WAIT_ACK;
}


/**
 * @brief PORT command processing
 *
 * The PORT command specifies the data port to be used for the data connection
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessPort(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   size_t i;
   size_t j;
   char_t *p;
   char_t *token;
   char_t *end;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument is the concatenation of the IP address and the 16-bit port number
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Close the data connection, if any
   ftpServerCloseDataConnection(connection);

   //Start of exception handling block
   do
   {
      //Assume an error condition...
      error = ERROR_INVALID_SYNTAX;

      //Parse the string
      for(i = 0, j = 1; param[i] != '\0'; i++)
      {
         //Change commas to dots
         if(param[i] == ',' && j < sizeof(Ipv4Addr))
         {
            param[i] = '.';
            j++;
         }
      }

      //Get the IP address to be used
      token = strtok_r(param, ",", &p);
      //Syntax error?
      if(token == NULL)
         break;

      //Convert the dot-decimal string to a binary IP address
      error = ipStringToAddr(token, &connection->remoteIpAddr);
      //Invalid IP address?
      if(error)
         break;

      //Assume an error condition...
      error = ERROR_INVALID_SYNTAX;

      //Get the most significant byte of the port number
      token = strtok_r(NULL, ",", &p);
      //Syntax error?
      if(token == NULL)
         break;

      //Convert the string representation to integer
      connection->remotePort = strtoul(token, &end, 10) << 8;
      //Syntax error?
      if(*end != '\0')
         break;

      //Get the least significant byte of the port number
      token = strtok_r(NULL, ",", &p);
      //Syntax error?
      if(token == NULL)
         break;

      //Convert the string representation to integer
      connection->remotePort |= strtoul(token, &end, 10) & 0xFF;
      //Syntax error?
      if(*end != '\0')
         break;

      //Successful processing
      error = NO_ERROR;

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Re initialize data connection
      connection->passiveMode = FALSE;
      connection->remotePort = 0;

      //Format response message
      strcpy(connection->response, "501 Syntax error in parameters or arguments\r\n");
      //Exit immediately
      return;
   }

   //Successful processing
   strcpy(connection->response, "200 Command okay\r\n");
}


/**
 * @brief EPRT command processing
 *
 * The EPRT command allows for the specification of an extended address
 * for the data connection
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessEprt(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t protocol;
   char_t *p;
   char_t *token;
   char_t *end;
   char_t delimiter[2];

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The extended address must consist of the network protocol
   //as well as the IP address and the 16-bit port number
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Close the data connection, if any
   ftpServerCloseDataConnection(connection);

   //Start of exception handling block
   do
   {
      //A delimiter character must be specified
      delimiter[0] = param[0];
      delimiter[1] = '\0';
      //Skip delimiter character
      param++;

      //Assume an error condition...
      error = ERROR_INVALID_SYNTAX;

      //Retrieve the network protocol to be used
      token = strtok_r(param, delimiter, &p);
      //Syntax error?
      if(token == NULL)
         break;

      //Convert the string representation to integer
      protocol = strtoul(token, &end, 10);
      //Syntax error?
      if(*end != '\0')
         break;

      //Get the IP address to be used
      token = strtok_r(NULL, delimiter, &p);
      //Syntax error?
      if(token == NULL)
         break;

#if (IPV4_SUPPORT == ENABLED)
      //IPv4 address family?
      if(protocol == 1)
      {
         //IPv4 addresses are 4-byte long
         connection->remoteIpAddr.length = sizeof(Ipv4Addr);
         //Convert the string to IPv4 address
         error = ipv4StringToAddr(token, &connection->remoteIpAddr.ipv4Addr);
         //Invalid IP address?
         if(error)
            break;
      }
      else
#endif
#if (IPV6_SUPPORT == ENABLED)
      //IPv6 address family?
      if(protocol == 2)
      {
         //IPv6 addresses are 16-byte long
         connection->remoteIpAddr.length = sizeof(Ipv6Addr);
         //Convert the string to IPv6 address
         error = ipv6StringToAddr(token, &connection->remoteIpAddr.ipv6Addr);
         //Invalid IP address?
         if(error)
            break;
      }
      else
#endif
      //Unknown address family?
      {
         //Report an error
         error = ERROR_INVALID_ADDRESS;
         //Exit immediately
         break;
      }

      //Assume an error condition...
      error = ERROR_INVALID_SYNTAX;

      //Get the port number to be used
      token = strtok_r(NULL, delimiter, &p);
      //Syntax error?
      if(token == NULL)
         break;

      //Convert the string representation to integer
      connection->remotePort = strtoul(token, &end, 10);
      //Syntax error?
      if(*end != '\0')
         break;

      //Successful processing
      error = NO_ERROR;

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Re initialize data connection
      connection->passiveMode = FALSE;
      connection->remotePort = 0;

      //Format response message
      strcpy(connection->response, "501 Syntax error in parameters or arguments\r\n");
      //Exit immediately
      return;
   }

   //Successful processing
   strcpy(connection->response, "200 Command okay\r\n");
}


/**
 * @brief PASV command processing
 *
 * The PASV command requests the server to listen on a data port and
 * to wait for a connection rather than initiate one upon receipt of
 * a transfer command
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessPasv(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   size_t n;
   IpAddr ipAddr;
   uint16_t port;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //Close the data connection, if any
   ftpServerCloseDataConnection(connection);

   //Get the next passive port number to be used
   port = ftpServerGetPassivePort(context);

   //Start of exception handling block
   do
   {
      //Open data socket
      connection->dataSocket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
      //Failed to open socket?
      if(!connection->dataSocket)
      {
         //Report an error
         error = ERROR_OPEN_FAILED;
         //Exit immediately
         break;
      }

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

      //Bind the socket to the passive port number
      error = socketBind(connection->dataSocket, &IP_ADDR_ANY, port);
      //Failed to bind the socket to the desired port?
      if(error)
         break;

      //Place the data socket in the listening state
      error = socketListen(connection->dataSocket, 1);
      //Any error to report?
      if(error)
         break;

      //Retrieve local IP address
      error = socketGetLocalAddr(connection->controlSocket, &ipAddr, NULL);
      //Any error to report?
      if(error)
         break;

      //The local IP address must be a valid IPv4 address
      if(ipAddr.length != sizeof(Ipv4Addr))
      {
         //PASV command cannot be used on IPv6 connections
         error = ERROR_INVALID_ADDRESS;
         //Exit immediately
         break;
      }

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Clean up side effects
      ftpServerCloseDataConnection(connection);
      //Format response message
      strcpy(connection->response, "425 Can't enter passive mode\r\n");
      //Exit immediately
      return;
   }

   //Use passive data transfer
   connection->passiveMode = TRUE;
   //Update data connection state
   connection->dataState = FTP_DATA_STATE_LISTEN;

#if defined(FTP_SERVER_PASV_HOOK)
   FTP_SERVER_PASV_HOOK(connection, ipAddr);
#endif

   //Format response message
   n = sprintf(connection->response, "227 Entering passive mode (");
   //Append host address
   ipAddrToString(&ipAddr, connection->response + n);

   //Parse the resulting string
   for(n = 0; connection->response[n] != '\0'; n++)
   {
      //Change dots to commas
      if(connection->response[n] == '.')
         connection->response[n] = ',';
   }

   //Append port number
   sprintf(connection->response + n, ",%" PRIu8 ",%" PRIu8 ")\r\n", MSB(port), LSB(port));
}


/**
 * @brief EPSV command processing
 *
 * The EPSV command requests that a server listen on a data port and
 * wait for a connection
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessEpsv(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint16_t port;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //Close the data connection, if any
   ftpServerCloseDataConnection(connection);

   //Get the next passive port number to be used
   port = ftpServerGetPassivePort(context);

   //Start of exception handling block
   do
   {
      //Open data socket
      connection->dataSocket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
      //Failed to open socket?
      if(!connection->dataSocket)
      {
         //Report an error
         error = ERROR_OPEN_FAILED;
         //Exit immediately
         break;
      }

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

      //Bind the socket to the passive port number
      error = socketBind(connection->dataSocket, &IP_ADDR_ANY, port);
      //Failed to bind the socket to the desired port?
      if(error)
         break;

      //Place the data socket in the listening state
      error = socketListen(connection->dataSocket, 1);
      //Any error to report?
      if(error)
         break;

      //End of exception handling block
   } while(0);

   //Any error to report?
   if(error)
   {
      //Clean up side effects
      ftpServerCloseDataConnection(connection);
      //Format response message
      strcpy(connection->response, "425 Can't enter passive mode\r\n");
      //Exit immediately
      return;
   }

   //Use passive data transfer
   connection->passiveMode = TRUE;
   //Update data connection state
   connection->dataState = FTP_DATA_STATE_LISTEN;

   //The response code for entering passive mode using an extended address must be 229
   sprintf(connection->response, "229 Entering extended passive mode (|||%" PRIu16 "|)\r\n",
      port);
}


/**
 * @brief ABOR command processing
 *
 * The ABOR command tells the server to abort the previous FTP
 * service command and any associated transfer of data
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessAbor(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //There are two cases to consider upon receipt of this command
   if(connection->dataState == FTP_DATA_STATE_CLOSED)
   {
      //If the FTP service command was already completed, the server closes
      //the data connection (if it is open)...
      ftpServerCloseDataConnection(connection);

      //...and responds with a 226 reply, indicating that the abort command
      //was successfully processed
      strcpy(connection->response, "226 Abort command successful\r\n");
   }
   else
   {
      //If the FTP service command is still in progress, the server aborts
      //the FTP service in progress and closes the data connection...
      ftpServerCloseDataConnection(connection);

      //...returning a 426 reply to indicate that the service request
      //terminated abnormally
      strcpy(connection->response, "426 Connection closed; transfer aborted\r\n");

      //The server then sends a 226 reply, indicating that the abort command
      //was successfully processed
      strcat(connection->response, "226 Abort command successful\r\n");
   }

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
}


/**
 * @brief PWD command processing
 *
 * The PWD command causes the name of the current working
 * directory to be returned in the reply
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessPwd(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //A successful PWD command uses the 257 reply code
   sprintf(connection->response, "257 \"%s\" is current directory\r\n",
      ftpServerStripHomeDir(connection, connection->currentDir));
}


/**
 * @brief CWD command processing
 *
 * The CWD command allows the user to work with a different
 * directory
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessCwd(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument specifies the pathname
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname
   error = ftpServerGetPath(connection, param,
      connection->path, FTP_SERVER_MAX_PATH_LEN);

   //Make sure the pathname is valid
   if(error)
   {
      //Report an error
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_READ))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Make sure the specified directory exists
   if(!fsDirExists(connection->path))
   {
      //Report an error
      strcpy(connection->response, "550 Directory not found\r\n");
      //Exit immediately
      return;
   }

   //Change current working directory
   strcpy(connection->currentDir, connection->path);

   //A successful PWD command uses the 250 reply code
   sprintf(connection->response, "250 Directory changed to %s\r\n",
      ftpServerStripHomeDir(connection, connection->currentDir));
}


/**
 * @brief CDUP command processing
 *
 * The CDUP command allows the user to change to the parent directory
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessCdup(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //Get current directory
   strcpy(connection->path, connection->currentDir);

   //Change to the parent directory
   pathCombine(connection->path, "..", FTP_SERVER_MAX_PATH_LEN);
   pathCanonicalize(connection->path);

   //Retrieve permissions for the directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Check access rights
   if(perm & FTP_FILE_PERM_READ)
   {
      //Update current directory
      strcpy(connection->currentDir, connection->path);
   }

   //A successful PWD command uses the 250 reply code
   sprintf(connection->response, "250 Directory changed to %s\r\n",
      ftpServerStripHomeDir(connection, connection->currentDir));
}


/**
 * @brief LIST command processing
 *
 * The LIST command is used to list the content of a directory
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessList(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //Any option flags
   while(*param == '-')
   {
      //Skip option flags
      while(*param != ' ' && *param != '\0')
         param++;
      //Skip whitespace characters
      while(*param == ' ')
         param++;
   }

   //The pathname is optional
   if(*param == '\0')
   {
      //Use current directory if no pathname is specified
      strcpy(connection->path, connection->currentDir);
   }
   else
   {
      //Retrieve the full pathname
      error = ftpServerGetPath(connection, param,
         connection->path, FTP_SERVER_MAX_PATH_LEN);

      //Any error to report?
      if(error)
      {
         //The specified pathname is not valid...
         strcpy(connection->response, "501 Invalid parameter\r\n");
         //Exit immediately
         return;
      }
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_READ))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Open specified directory for reading
   connection->dir = fsOpenDir(connection->path);

   //Failed to open the directory?
   if(!connection->dir)
   {
      //Report an error
      strcpy(connection->response, "550 Directory not found\r\n");
      //Exit immediately
      return;
   }

   //Check current data transfer mode
   if(connection->passiveMode)
   {
      //Check whether the data connection is already opened
      if(connection->dataState == FTP_DATA_STATE_IDLE)
         connection->dataState = FTP_DATA_STATE_SEND;
   }
   else
   {
      //Open the data connection
      error = ftpServerOpenDataConnection(context, connection);

      //Any error to report?
      if(error)
      {
         //Clean up side effects
         fsCloseDir(connection->dir);
         //Format response
         strcpy(connection->response, "450 Can't open data connection\r\n");
         //Exit immediately
         return;
      }

      //The data connection is ready to send data
      connection->dataState = FTP_DATA_STATE_SEND;
   }

   //Flush transmission buffer
   connection->bufferLength = 0;
   connection->bufferPos = 0;

   //LIST command is being processed
   connection->controlState = FTP_CONTROL_STATE_LIST;

   //Format response message
   strcpy(connection->response, "150 Opening data connection\r\n");
}


/**
 * @brief MKD command processing
 *
 * The MKD command causes the directory specified in the pathname
 * to be created as a directory
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessMkd(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument specifies the pathname
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname
   error = ftpServerGetPath(connection, param,
      connection->path, FTP_SERVER_MAX_PATH_LEN);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_WRITE))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Create the specified directory
   error = fsCreateDir(connection->path);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "550 Can't create directory\r\n");
      //Exit immediately
      return;
   }

   //The specified directory was successfully created
   sprintf(connection->response, "257 \"%s\" created\r\n",
      ftpServerStripHomeDir(connection, connection->path));
}


/**
 * @brief RMD command processing
 *
 * The RMD command causes the directory specified in the pathname
 * to be removed
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessRmd(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument specifies the directory to be removed
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname of the directory
   error = ftpServerGetPath(connection, param,
      connection->path, FTP_SERVER_MAX_PATH_LEN);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_WRITE))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Remove the specified directory
   error = fsRemoveDir(connection->path);

   //Any error to report?
   if(error)
   {
      //The specified directory cannot be deleted...
      strcpy(connection->response, "550 Can't remove directory\r\n");
      //Exit immediately
      return;
   }

   //The specified directory was successfully removed
   strcpy(connection->response, "250 Directory removed\r\n");
}


/**
 * @brief SIZE command processing
 *
 * The SIZE command is used to obtain the transfer size of the specified file
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessSize(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;
   uint32_t size;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument specifies the pathname of the file
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname
   error = ftpServerGetPath(connection, param,
      connection->path, FTP_SERVER_MAX_PATH_LEN);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_LIST) && !(perm & FTP_FILE_PERM_READ))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the size of the specified file
   error = fsGetFileSize(connection->path, &size);

   //Any error to report?
   if(error)
   {
      //Report an error
      strcpy(connection->response, "550 File not found\r\n");
      //Exit immediately
      return;
   }

   //Format response message
   sprintf(connection->response, "213 %" PRIu32 "\r\n", size);
}


/**
 * @brief RETR command processing
 *
 * The RETR command is used to retrieve the content of the specified file
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessRetr(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument specifies the pathname of the file to read
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname
   error = ftpServerGetPath(connection, param,
      connection->path, FTP_SERVER_MAX_PATH_LEN);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_READ))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Open specified file for reading
   connection->file = fsOpenFile(connection->path, FS_FILE_MODE_READ);

   //Failed to open the file?
   if(!connection->file)
   {
      //Report an error
      strcpy(connection->response, "550 File not found\r\n");
      //Exit immediately
      return;
   }

   //Check current data transfer mode
   if(connection->passiveMode)
   {
      //Check whether the data connection is already opened
      if(connection->dataState == FTP_DATA_STATE_IDLE)
         connection->dataState = FTP_DATA_STATE_SEND;
   }
   else
   {
      //Open the data connection
      error = ftpServerOpenDataConnection(context, connection);

      //Any error to report?
      if(error)
      {
         //Clean up side effects
         fsCloseFile(connection->file);
         //Format response
         strcpy(connection->response, "450 Can't open data connection\r\n");
         //Exit immediately
         return;
      }

      //The data connection is ready to send data
      connection->dataState = FTP_DATA_STATE_SEND;
   }

   //Flush transmission buffer
   connection->bufferLength = 0;
   connection->bufferPos = 0;

   //RETR command is being processed
   connection->controlState = FTP_CONTROL_STATE_RETR;

   //Format response message
   strcpy(connection->response, "150 Opening data connection\r\n");
}


/**
 * @brief STOR command processing
 *
 * The STOR command is used to store data to the specified file
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessStor(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument specifies the pathname of the file to written
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname
   error = ftpServerGetPath(connection, param,
      connection->path, FTP_SERVER_MAX_PATH_LEN);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_WRITE))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Open specified file for writing
   connection->file = fsOpenFile(connection->path,
      FS_FILE_MODE_WRITE | FS_FILE_MODE_CREATE | FS_FILE_MODE_TRUNC);

   //Failed to open the file?
   if(!connection->file)
   {
      //Report an error
      strcpy(connection->response, "550 File not found\r\n");
      //Exit immediately
      return;
   }

   //Check current data transfer mode
   if(connection->passiveMode)
   {
      //Check whether the data connection is already opened
      if(connection->dataState == FTP_DATA_STATE_IDLE)
         connection->dataState = FTP_DATA_STATE_RECEIVE;
   }
   else
   {
      //Open the data connection
      error = ftpServerOpenDataConnection(context, connection);

      //Any error to report?
      if(error)
      {
         //Clean up side effects
         fsCloseFile(connection->file);
         //Format response
         strcpy(connection->response, "450 Can't open data connection\r\n");
         //Exit immediately
         return;
      }

      //The data connection is ready to receive data
      connection->dataState = FTP_DATA_STATE_RECEIVE;
   }

   //Flush reception buffer
   connection->bufferLength = 0;
   connection->bufferPos = 0;

   //STOR command is being processed
   connection->controlState = FTP_CONTROL_STATE_STOR;

   //Format response message
   strcpy(connection->response, "150 Opening data connection\r\n");
}


/**
 * @brief APPE command processing
 *
 * The APPE command is used to append data to the specified file
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessAppe(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument specifies the pathname of the file to written
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname
   error = ftpServerGetPath(connection, param,
      connection->path, FTP_SERVER_MAX_PATH_LEN);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_WRITE))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Open specified file for writing
   connection->file = fsOpenFile(connection->path,
      FS_FILE_MODE_WRITE | FS_FILE_MODE_CREATE);

   //Failed to open the file?
   if(!connection->file)
   {
      //Report an error
      strcpy(connection->response, "550 File not found\r\n");
      //Exit immediately
      return;
   }

   //Move to the end of the file
   error = fsSeekFile(connection->file, 0, FS_SEEK_END);

   //Any error to report?
   if(error)
   {
      //Clean up side effects
      fsCloseFile(connection->file);
      //Format response
      strcpy(connection->response, "550 File unavailable\r\n");
   }

   //Check current data transfer mode
   if(connection->passiveMode)
   {
      //Check whether the data connection is already opened
      if(connection->dataState == FTP_DATA_STATE_IDLE)
         connection->dataState = FTP_DATA_STATE_RECEIVE;
   }
   else
   {
      //Open the data connection
      error = ftpServerOpenDataConnection(context, connection);

      //Any error to report?
      if(error)
      {
         //Clean up side effects
         fsCloseFile(connection->file);
         //Format response
         strcpy(connection->response, "450 Can't open data connection\r\n");
         //Exit immediately
         return;
      }

      //The data connection is ready to receive data
      connection->dataState = FTP_DATA_STATE_RECEIVE;
   }

   //Flush reception buffer
   connection->bufferLength = 0;
   connection->bufferPos = 0;

   //APPE command is being processed
   connection->controlState = FTP_CONTROL_STATE_APPE;

   //Format response message
   strcpy(connection->response, "150 Opening data connection\r\n");
}


/**
 * @brief RNFR command processing
 *
 * The RNFR command specifies the old pathname of the file which is
 * to be renamed
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessRnfr(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument specifies the file to be renamed
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname
   error = ftpServerGetPath(connection, param,
      connection->path, FTP_SERVER_MAX_PATH_LEN);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_WRITE))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Make sure the file exists
   if(!fsFileExists(connection->path) && !fsDirExists(connection->path))
   {
      //No such file or directory...
      strcpy(connection->response, "550 File not found\r\n");
      //Exit immediately
      return;
   }

   //This command must be immediately followed by a RNTO command
   connection->controlState = FTP_CONTROL_STATE_RNFR;
   //Format the response message
   strcpy(connection->response, "350 File exists, ready for destination name\r\n");
}


/**
 * @brief RNTO command processing
 *
 * The RNTO command specifies the new pathname of the file specified
 * in the immediately preceding RNFR command
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessRnto(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;
   char_t newPath[FTP_SERVER_MAX_PATH_LEN];

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //This command must immediately follow a RNFR command
   if(connection->controlState != FTP_CONTROL_STATE_RNFR)
   {
      //Switch to idle state
      connection->controlState = FTP_CONTROL_STATE_IDLE;
      //Report an error
      strcpy(connection->response, "503 Bad sequence of commands\r\n");
      //Exit immediately
      return;
   }

   //Switch to idle state
   connection->controlState = FTP_CONTROL_STATE_IDLE;

   //The argument specifies the new pathname
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname
   error = ftpServerGetPath(connection, param,
      newPath, FTP_SERVER_MAX_PATH_LEN);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, newPath);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_WRITE))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Check whether the file name already exists
   if(fsFileExists(newPath) || fsDirExists(newPath))
   {
      //Report an error
      strcpy(connection->response, "550 File already exists\r\n");
      //Exit immediately
      return;
   }

   //Rename the specified file
   error = fsRenameFile(connection->path, newPath);

   //Any error to report?
   if(error)
   {
      //The specified file cannot be renamed
      strcpy(connection->response, "550 Can't rename file\r\n");
      //Exit immediately
      return;
   }

   //The specified file was successfully deleted
   strcpy(connection->response, "250 File renamed\r\n");
}


/**
 * @brief DELE command processing
 *
 * The DELE command causes the file specified in the pathname to be
 * deleted at the server site
 *
 * @param[in] context Pointer to the FTP server context
 * @param[in] connection Pointer to the client connection
 * @param[in] param Command line parameters
 **/

void ftpServerProcessDele(FtpServerContext *context,
   FtpClientConnection *connection, char_t *param)
{
   error_t error;
   uint_t perm;

   //Ensure the user is logged in
   if(!connection->userLoggedIn)
   {
      //Format response message
      strcpy(connection->response, "530 Not logged in\r\n");
      //Exit immediately
      return;
   }

   //The argument specifies the file to be deleted
   if(*param == '\0')
   {
      //The argument is missing...
      strcpy(connection->response, "501 Missing parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve the full pathname of the file
   error = ftpServerGetPath(connection, param,
      connection->path, FTP_SERVER_MAX_PATH_LEN);

   //Any error to report?
   if(error)
   {
      //The specified pathname is not valid...
      strcpy(connection->response, "501 Invalid parameter\r\n");
      //Exit immediately
      return;
   }

   //Retrieve permissions for the specified directory
   perm = ftpServerGetFilePermissions(context, connection, connection->path);

   //Insufficient access rights?
   if(!(perm & FTP_FILE_PERM_WRITE))
   {
      //Report an error
      strcpy(connection->response, "550 Access denied\r\n");
      //Exit immediately
      return;
   }

   //Delete the specified file
   error = fsDeleteFile(connection->path);

   //Any error to report?
   if(error)
   {
      //The specified file cannot be deleted...
      strcpy(connection->response, "550 Can't delete file\r\n");
      //Exit immediately
      return;
   }

   //The specified file was successfully deleted
   strcpy(connection->response, "250 File deleted\r\n");
}

#endif
