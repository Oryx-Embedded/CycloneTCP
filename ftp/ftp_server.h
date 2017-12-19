/**
 * @file ftp_server.h
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.8.0
 **/

#ifndef _FTP_SERVER_H
#define _FTP_SERVER_H

//Dependencies
#include "core/net.h"
#include "core/socket.h"
#include "fs_port.h"

//FTP server support
#ifndef FTP_SERVER_SUPPORT
   #define FTP_SERVER_SUPPORT ENABLED
#elif (FTP_SERVER_SUPPORT != ENABLED && FTP_SERVER_SUPPORT != DISABLED)
   #error FTP_SERVER_SUPPORT parameter is not valid
#endif

//Stack size required to run the FTP server
#ifndef FTP_SERVER_STACK_SIZE
   #define FTP_SERVER_STACK_SIZE 650
#elif (FTP_SERVER_STACK_SIZE < 1)
   #error FTP_SERVER_STACK_SIZE parameter is not valid
#endif

//Priority at which the FTP server should run
#ifndef FTP_SERVER_PRIORITY
   #define FTP_SERVER_PRIORITY OS_TASK_PRIORITY_NORMAL
#endif

//Maximum number of simultaneous connections
#ifndef FTP_SERVER_MAX_CONNECTIONS
   #define FTP_SERVER_MAX_CONNECTIONS 4
#elif (FTP_SERVER_MAX_CONNECTIONS < 1)
   #error FTP_SERVER_MAX_CONNECTIONS parameter is not valid
#endif

//Maximum time the server will wait before closing the connection
#ifndef FTP_SERVER_TIMEOUT
   #define FTP_SERVER_TIMEOUT 60000
#elif (FTP_SERVER_TIMEOUT < 1000)
   #error FTP_SERVER_TIMEOUT parameter is not valid
#endif

//Socket polling timeout
#ifndef FTP_SERVER_SOCKET_POLLING_TIMEOUT
   #define FTP_SERVER_SOCKET_POLLING_TIMEOUT 2000
#elif (FTP_SERVER_SOCKET_POLLING_TIMEOUT < 1000)
   #error FTP_SERVER_SOCKET_POLLING_TIMEOUT parameter is not valid
#endif

//Maximum length of the pending connection queue
#ifndef FTP_SERVER_BACKLOG
   #define FTP_SERVER_BACKLOG 4
#elif (FTP_SERVER_BACKLOG < 1)
   #error FTP_SERVER_BACKLOG parameter is not valid
#endif

//Maximum line length
#ifndef FTP_SERVER_MAX_LINE_LEN
   #define FTP_SERVER_MAX_LINE_LEN 255
#elif (FTP_SERVER_MAX_LINE_LEN < 64)
   #error FTP_SERVER_MAX_LINE_LEN parameter is not valid
#endif

//Size of buffer used for input/output operations
#ifndef FTP_SERVER_BUFFER_SIZE
   #define FTP_SERVER_BUFFER_SIZE 1536
#elif (FTP_SERVER_BUFFER_SIZE < 128)
   #error FTP_SERVER_BUFFER_SIZE parameter is not valid
#endif

//Maximum size of root directory
#ifndef FTP_SERVER_MAX_ROOT_DIR_LEN
   #define FTP_SERVER_MAX_ROOT_DIR_LEN 63
#elif (FTP_SERVER_MAX_ROOT_DIR_LEN < 7)
   #error FTP_SERVER_MAX_ROOT_DIR_LEN parameter is not valid
#endif

//Maximum size of home directory
#ifndef FTP_SERVER_MAX_HOME_DIR_LEN
   #define FTP_SERVER_MAX_HOME_DIR_LEN 63
#elif (FTP_SERVER_MAX_HOME_DIR_LEN < 7)
   #error FTP_SERVER_MAX_HOME_DIR_LEN parameter is not valid
#endif

//Maximum user name length
#ifndef FTP_SERVER_MAX_USERNAME_LEN
   #define FTP_SERVER_MAX_USERNAME_LEN 63
#elif (FTP_SERVER_MAX_USERNAME_LEN < 7)
   #error FTP_SERVER_MAX_USERNAME_LEN parameter is not valid
#endif

//Maximum path length
#ifndef FTP_SERVER_MAX_PATH_LEN
   #define FTP_SERVER_MAX_PATH_LEN 255
#elif (FTP_SERVER_MAX_PATH_LEN < 7)
   #error FTP_SERVER_MAX_PATH_LEN parameter is not valid
#endif

//Socket buffer size (control connection)
#ifndef FTP_SERVER_CTRL_SOCKET_BUFFER_SIZE
   #define FTP_SERVER_CTRL_SOCKET_BUFFER_SIZE 1430
#elif (FTP_SERVER_CTRL_SOCKET_BUFFER_SIZE < 1)
   #error FTP_SERVER_CTRL_SOCKET_BUFFER_SIZE parameter is not valid
#endif

//Socket buffer size (data connection)
#ifndef FTP_SERVER_DATA_SOCKET_BUFFER_SIZE
   #define FTP_SERVER_DATA_SOCKET_BUFFER_SIZE 2860
#elif (FTP_SERVER_DATA_SOCKET_BUFFER_SIZE < 1)
   #error FTP_SERVER_DATA_SOCKET_BUFFER_SIZE parameter is not valid
#endif

//Passive port range (lower limit)
#ifndef FTP_SERVER_PASSIVE_PORT_MIN
   #define FTP_SERVER_PASSIVE_PORT_MIN 48128
#elif (FTP_SERVER_PASSIVE_PORT_MIN < 1024)
   #error FTP_SERVER_PASSIVE_PORT_MIN parameter is not valid
#endif

//Passive port range (upper limit)
#ifndef FTP_SERVER_PASSIVE_PORT_MAX
   #define FTP_SERVER_PASSIVE_PORT_MAX 49151
#elif (FTP_SERVER_PASSIVE_PORT_MAX <= FTP_SERVER_PASSIVE_PORT_MIN || FTP_SERVER_PASSIVE_PORT_MAX > 65535)
   #error FTP_SERVER_PASSIVE_PORT_MAX parameter is not valid
#endif

//FTP port number
#define FTP_PORT 21
//FTP data port number
#define FTP_DATA_PORT 20

//FTPS port number (implicit mode)
#define FTPS_PORT 990
//FTPS data port number (implicit mode)
#define FTPS_DATA_PORT 989

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief Control connection state
 **/

typedef enum
{
   FTP_CONTROL_STATE_CLOSED      = 0,
   FTP_CONTROL_STATE_IDLE        = 1,
   FTP_CONTROL_STATE_DISCARD     = 2,
   FTP_CONTROL_STATE_USER        = 3,
   FTP_CONTROL_STATE_LIST        = 4,
   FTP_CONTROL_STATE_RETR        = 5,
   FTP_CONTROL_STATE_STOR        = 6,
   FTP_CONTROL_STATE_APPE        = 7,
   FTP_CONTROL_STATE_RNFR        = 8,
   FTP_CONTROL_STATE_WAIT_ACK    = 9,
   FTP_CONTROL_STATE_SHUTDOWN_TX = 10,
   FTP_CONTROL_STATE_SHUTDOWN_RX = 11
} FtpControlConnState;


/**
 * @brief Data connection state
 **/

typedef enum
{
   FTP_DATA_STATE_CLOSED      = 0,
   FTP_DATA_STATE_LISTEN      = 1,
   FTP_DATA_STATE_IDLE        = 2,
   FTP_DATA_STATE_SEND        = 3,
   FTP_DATA_STATE_RECEIVE     = 4,
   FTP_DATA_STATE_WAIT_ACK    = 5,
   FTP_DATA_STATE_SHUTDOWN_TX = 6,
   FTP_DATA_STATE_SHUTDOWN_RX = 7
} FtpDataConnState;


/**
 * @brief FTP server access status
 **/

typedef enum
{
   FTP_ACCESS_DENIED     = 0,
   FTP_ACCESS_ALLOWED    = 1,
   FTP_PASSWORD_REQUIRED = 2
} FtpAccessStatus;


/**
 * @brief File permissions
 **/

typedef enum
{
   FTP_FILE_PERM_LIST  = 0x01,
   FTP_FILE_PERM_READ  = 0x02,
   FTP_FILE_PERM_WRITE = 0x04
} FtpFilePerm;


/**
 * @brief FTP client connection
 **/

typedef struct
{
   NetInterface *interface;                         ///<Underlying network interface
   bool_t userLoggedIn;                             ///<This flag tells whether the user is logged in
   systime_t timestamp;                             ///<Time stamp to manage timeout
   FtpControlConnState controlState;                ///<Control connection state
   Socket *controlSocket;                           ///<Control connection socket
   FtpDataConnState dataState;                      ///<Data connection state
   Socket *dataSocket;                              ///<Data connection socket
   FsFile *file;                                    ///<File pointer
   FsDir *dir;                                      ///<Directory pointer
   bool_t passiveMode;                              ///<Passive data transfer
   IpAddr remoteIpAddr;                             ///<Remote IP address
   uint16_t remotePort;                             ///<Remote port number
   char_t user[FTP_SERVER_MAX_USERNAME_LEN + 1];    ///<User name
   char_t homeDir[FTP_SERVER_MAX_HOME_DIR_LEN + 1]; ///<Home directory
   char_t currentDir[FTP_SERVER_MAX_PATH_LEN + 1];  ///<Current directory
   char_t path[FTP_SERVER_MAX_PATH_LEN + 1];        ///<Pathname
   char_t command[FTP_SERVER_MAX_LINE_LEN + 1];     ///<Incoming command
   size_t commandLength;                            ///<Number of bytes available in the command buffer
   char_t response[FTP_SERVER_MAX_LINE_LEN + 1];    ///<Response buffer
   size_t responseLength;                           ///<Number of bytes available in the response buffer
   size_t responsePos;                              ///<Current position in the response buffer
   char_t *buffer;                                  ///<Memory buffer for I/O operations
   size_t bufferLength;                             ///<Number of bytes available in the I/O buffer
   size_t bufferPos;                                ///<Current position in the I/O buffer
} FtpClientConnection;


/**
 * @brief User verification callback function
 **/

typedef uint_t (*FtpCheckUserCallback)(FtpClientConnection *connection,
   const char_t *user);


/**
 * @brief Password verification callback function
 **/

typedef uint_t (*FtpCheckPasswordCallback)(FtpClientConnection *connection,
   const char_t *user, const char_t *password);


/**
 * @brief Callback used to retrieve file permissions
 **/

typedef uint_t (*FtpGetFilePermCallback)(FtpClientConnection *connection,
   const char_t *user, const char_t *path);


/**
 * @brief Unknown command callback function
 **/

typedef error_t (*FtpUnknownCommandCallback)(FtpClientConnection *connection,
   const char_t *command, const char_t *param);


/**
 * @brief FTP server settings
 **/

typedef struct
{
   NetInterface *interface;                          ///<Underlying network interface
   uint16_t port;                                    ///<FTP command port number
   uint16_t dataPort;                                ///<FTP data port number
   uint16_t passivePortMin;                          ///<Passive port range (lower value)
   uint16_t passivePortMax;                          ///<Passive port range (upper value)
   char_t rootDir[FTP_SERVER_MAX_ROOT_DIR_LEN + 1];  ///<Root directory
   FtpCheckUserCallback checkUserCallback;           ///<User verification callback function
   FtpCheckPasswordCallback checkPasswordCallback;   ///<Password verification callback function
   FtpGetFilePermCallback getFilePermCallback;       ///<Callback used to retrieve file permissions
   FtpUnknownCommandCallback unknownCommandCallback; ///<Unknown command callback function
} FtpServerSettings;


/**
 * @brief FTP server context
 **/

typedef struct
{
   FtpServerSettings settings;                                    ///<User settings
   OsEvent event;                                                 ///<Event object used to poll the sockets
   Socket *socket;                                                ///<Listening socket
   uint16_t passivePort;                                          ///<Current passive port number
   FtpClientConnection *connection[FTP_SERVER_MAX_CONNECTIONS];   ///<Client connections
   SocketEventDesc eventDesc[2 * FTP_SERVER_MAX_CONNECTIONS + 1]; ///<The events the application is interested in
} FtpServerContext;


//FTP server related functions
void ftpServerGetDefaultSettings(FtpServerSettings *settings);
error_t ftpServerInit(FtpServerContext *context, const FtpServerSettings *settings);
error_t ftpServerStart(FtpServerContext *context);
error_t ftpServerSetHomeDir(FtpClientConnection *connection, const char_t *homeDir);

void ftpServerTask(FtpServerContext *context);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
