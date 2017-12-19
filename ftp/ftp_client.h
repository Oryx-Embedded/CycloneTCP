/**
 * @file ftp_client.h
 * @brief FTP client (File Transfer Protocol)
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

#ifndef _FTP_CLIENT_H
#define _FTP_CLIENT_H

//Dependencies
#include "core/net.h"
#include "core/socket.h"

//FTP client support
#ifndef FTP_CLIENT_SUPPORT
   #define FTP_CLIENT_SUPPORT ENABLED
#elif (FTP_CLIENT_SUPPORT != ENABLED && FTP_CLIENT_SUPPORT != DISABLED)
   #error FTP_CLIENT_SUPPORT parameter is not valid
#endif

//FTP over SSL/TLS
#ifndef FTP_CLIENT_TLS_SUPPORT
   #define FTP_CLIENT_TLS_SUPPORT DISABLED
#elif (FTP_CLIENT_TLS_SUPPORT != ENABLED && FTP_CLIENT_TLS_SUPPORT != DISABLED)
   #error FTP_CLIENT_TLS_SUPPORT parameter is not valid
#endif

//Default timeout
#ifndef FTP_CLIENT_DEFAULT_TIMEOUT
   #define FTP_CLIENT_DEFAULT_TIMEOUT 20000
#elif (FTP_CLIENT_DEFAULT_TIMEOUT < 1000)
   #error FTP_CLIENT_DEFAULT_TIMEOUT parameter is not valid
#endif

//Size of the buffer for input/output operations
#ifndef FTP_CLIENT_BUFFER_SIZE
   #define FTP_CLIENT_BUFFER_SIZE 512
#elif (FTP_CLIENT_BUFFER_SIZE < 64)
   #error FTP_CLIENT_BUFFER_SIZE parameter is not valid
#endif

//Minimum TX buffer size for FTP sockets
#ifndef FTP_CLIENT_SOCKET_MIN_TX_BUFFER_SIZE
   #define FTP_CLIENT_SOCKET_MIN_TX_BUFFER_SIZE 1430
#elif (FTP_CLIENT_SOCKET_MIN_TX_BUFFER_SIZE < 1)
   #error FTP_CLIENT_SOCKET_MIN_TX_BUFFER_SIZE parameter is not valid
#endif

//Minimum RX buffer size for FTP sockets
#ifndef FTP_CLIENT_SOCKET_MIN_RX_BUFFER_SIZE
   #define FTP_CLIENT_SOCKET_MIN_RX_BUFFER_SIZE 1430
#elif (FTP_CLIENT_SOCKET_MIN_RX_BUFFER_SIZE < 1)
   #error FTP_CLIENT_SOCKET_MIN_RX_BUFFER_SIZE parameter is not valid
#endif

//Maximum TX buffer size for FTP sockets
#ifndef FTP_CLIENT_SOCKET_MAX_TX_BUFFER_SIZE
   #define FTP_CLIENT_SOCKET_MAX_TX_BUFFER_SIZE 2860
#elif (FTP_CLIENT_SOCKET_MAX_TX_BUFFER_SIZE < 1)
   #error FTP_CLIENT_SOCKET_MAX_TX_BUFFER_SIZE parameter is not valid
#endif

//Maximum RX buffer size for FTP sockets
#ifndef FTP_CLIENT_SOCKET_MAX_RX_BUFFER_SIZE
   #define FTP_CLIENT_SOCKET_MAX_RX_BUFFER_SIZE 2860
#elif (FTP_CLIENT_SOCKET_MAX_RX_BUFFER_SIZE < 1)
   #error FTP_CLIENT_SOCKET_MAX_RX_BUFFER_SIZE parameter is not valid
#endif

//SSL/TLS supported?
#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)
   #include "core/crypto.h"
   #include "tls.h"
#endif

//Test macros for FTP response codes
#define FTP_REPLY_CODE_1YZ(code) ((code) >= 100 && (code) < 200)
#define FTP_REPLY_CODE_2YZ(code) ((code) >= 200 && (code) < 300)
#define FTP_REPLY_CODE_3YZ(code) ((code) >= 300 && (code) < 400)
#define FTP_REPLY_CODE_4YZ(code) ((code) >= 400 && (code) < 500)
#define FTP_REPLY_CODE_5YZ(code) ((code) >= 500 && (code) < 600)

//Forward declaration of FtpClientContext structure
struct _FtpClientContext;
#define FtpClientContext struct _FtpClientContext

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief Connection options
 **/

typedef enum
{
   FTP_NO_SECURITY       = 0,
   FTP_IMPLICIT_SECURITY = 1,
   FTP_EXPLICIT_SECURITY = 2,
   FTP_ACTIVE_MODE       = 0,
   FTP_PASSIVE_MODE      = 4
} FtpConnectionFlags;


/**
 * @brief File opening options
 **/

typedef enum
{
   FTP_FOR_READING   = 0,
   FTP_FOR_WRITING   = 1,
   FTP_FOR_APPENDING = 2,
   FTP_BINARY_TYPE   = 0,
   FTP_TEXT_TYPE     = 4
} FtpFileOpeningFlags;


/**
 * @brief Flags used by I/O functions
 **/

typedef enum
{
   FTP_FLAG_PEEK       = 0x0200,
   FTP_FLAG_WAIT_ALL   = 0x0800,
   FTP_FLAG_BREAK_CHAR = 0x1000,
   FTP_FLAG_BREAK_CRLF = 0x100A,
   FTP_FLAG_WAIT_ACK   = 0x2000
} FtpFlags;


//SSL/TLS supported?
#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)

/**
 * @brief SSL initialization callback function
 **/

typedef error_t (*FtpClientTlsInitCallback)(FtpClientContext *context,
   TlsContext *tlsContext);

#endif


/**
 * @brief FTP client context
 **/

struct _FtpClientContext
{
   NetInterface *interface;                  ///<Underlying network interface
   IpAddr serverIpAddr;                      ///<IP address of the FTP server
   bool_t passiveMode;                       ///<Passive mode
   Socket *controlSocket;                    ///<Control connection socket
   Socket *dataSocket;                       ///<Data connection socket
   char_t buffer[FTP_CLIENT_BUFFER_SIZE];    ///<Memory buffer for input/output operations
#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)
   TlsContext *controlTlsContext;            ///<SSL context (control connection)
   TlsContext *dataTlsContext;               ///<SSL context (data connection)
   TlsSession tlsSession;                    ///<SSL session
   FtpClientTlsInitCallback tlsInitCallback; ///<SSL initialization callback function
#endif
};


//FTP client related functions
error_t ftpConnect(FtpClientContext *context, NetInterface *interface,
   const IpAddr *serverIpAddr, uint16_t serverPort, uint_t flags);

error_t ftpAuth(FtpClientContext *context);

error_t ftpLogin(FtpClientContext *context, const char_t *username,
   const char_t *password, const char_t *account);

error_t ftpGetWorkingDir(FtpClientContext *context, char_t *path, size_t size);
error_t ftpChangeWorkingDir(FtpClientContext *context, const char_t *path);
error_t ftpChangeToParentDir(FtpClientContext *context);

error_t ftpMakeDir(FtpClientContext *context, const char_t *path);
error_t ftpRemoveDir(FtpClientContext *context, const char_t *path);

error_t ftpOpenFile(FtpClientContext *context, const char_t *path, uint_t flags);

error_t ftpWriteFile(FtpClientContext *context,
   const void *data, size_t length, uint_t flags);

error_t ftpReadFile(FtpClientContext *context,
   void *data, size_t size, size_t *length, uint_t flags);

error_t ftpCloseFile(FtpClientContext *context);

error_t ftpRenameFile(FtpClientContext *context,
   const char_t *oldName, const char_t *newName);

error_t ftpDeleteFile(FtpClientContext *context, const char_t *path);

error_t ftpClose(FtpClientContext *context);

error_t ftpSendCommand(FtpClientContext *context,
   const char_t *command, uint_t *replyCode);

#if (FTP_CLIENT_TLS_SUPPORT == ENABLED)

error_t ftpRegisterTlsInitCallback(FtpClientContext *context,
   FtpClientTlsInitCallback callback);

error_t ftpInitControlTlsContext(FtpClientContext *context);
error_t ftpInitDataTlsContext(FtpClientContext *context);

#endif

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
