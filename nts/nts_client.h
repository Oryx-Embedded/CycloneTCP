/**
 * @file nts_client.h
 * @brief NTS client (Network Time Security)
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2025 Oryx Embedded SARL. All rights reserved.
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
 * The Simple Network Time Protocol is used to synchronize computer clocks
 * in the Internet. Refer to RFC 4330 for more details
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

#ifndef _NTS_CLIENT_H
#define _NTS_CLIENT_H

//Dependencies
#include "core/net.h"
#include "ntp/ntp_common.h"
#include "nts/nts_common.h"
#include "core/crypto.h"
#include "tls.h"

//NTS client support
#ifndef NTS_CLIENT_SUPPORT
   #define NTS_CLIENT_SUPPORT ENABLED
#elif (NTS_CLIENT_SUPPORT != ENABLED && NTS_CLIENT_SUPPORT != DISABLED)
   #error NTS_CLIENT_SUPPORT parameter is not valid
#endif

//Default timeout
#ifndef NTS_CLIENT_DEFAULT_TIMEOUT
   #define NTS_CLIENT_DEFAULT_TIMEOUT 30000
#elif (NTS_CLIENT_DEFAULT_TIMEOUT < 1000)
   #error NTS_CLIENT_DEFAULT_TIMEOUT parameter is not valid
#endif

//Initial NTP retransmission timeout
#ifndef NTS_CLIENT_INIT_NTP_RETRANSMIT_TIMEOUT
   #define NTS_CLIENT_INIT_NTP_RETRANSMIT_TIMEOUT 2000
#elif (NTS_CLIENT_INIT_NTP_RETRANSMIT_TIMEOUT < 1000)
   #error NTS_CLIENT_INIT_NTP_RETRANSMIT_TIMEOUT parameter is not valid
#endif

//Maximum NTP retransmission timeout
#ifndef NTS_CLIENT_MAX_NTP_RETRANSMIT_TIMEOUT
   #define NTS_CLIENT_MAX_NTP_RETRANSMIT_TIMEOUT 15000
#elif (NTS_CLIENT_MAX_NTP_RETRANSMIT_TIMEOUT < 1000)
   #error NTS_CLIENT_MAX_NTP_RETRANSMIT_TIMEOUT parameter is not valid
#endif

//Size of the buffer for input/output operations
#ifndef NTS_CLIENT_BUFFER_SIZE
   #define NTS_CLIENT_BUFFER_SIZE 512
#elif (NTS_CLIENT_BUFFER_SIZE < 128)
   #error NTS_CLIENT_BUFFER_SIZE parameter is not valid
#endif

//TX buffer size for TLS connections
#ifndef NTS_CLIENT_TLS_TX_BUFFER_SIZE
   #define NTS_CLIENT_TLS_TX_BUFFER_SIZE 2048
#elif (NTS_CLIENT_TLS_TX_BUFFER_SIZE < 512)
   #error NTS_CLIENT_TLS_TX_BUFFER_SIZE parameter is not valid
#endif

//RX buffer size for TLS connections
#ifndef NTS_CLIENT_TLS_RX_BUFFER_SIZE
   #define NTS_CLIENT_TLS_RX_BUFFER_SIZE 4096
#elif (NTS_CLIENT_TLS_RX_BUFFER_SIZE < 512)
   #error NTS_CLIENT_TLS_RX_BUFFER_SIZE parameter is not valid
#endif

//Maximum size for NTS cookies
#ifndef NTS_CLIENT_MAX_COOKIE_SIZE
   #define NTS_CLIENT_MAX_COOKIE_SIZE 128
#elif (NTS_CLIENT_MAX_COOKIE_SIZE < 1)
   #error NTS_CLIENT_MAX_COOKIE_SIZE parameter is not valid
#endif

//Maximum length of NTP server names
#ifndef NTS_CLIENT_MAX_NTP_SERVER_NAME_LEN
   #define NTS_CLIENT_MAX_NTP_SERVER_NAME_LEN 64
#elif (NTS_CLIENT_MAX_NTP_SERVER_NAME_LEN < 1)
   #error NTS_CLIENT_MAX_NTP_SERVER_NAME_LEN parameter is not valid
#endif

//Size of the unique identifier
#ifndef NTS_CLIENT_UNIQUE_ID_SIZE
   #define NTS_CLIENT_UNIQUE_ID_SIZE 32
#elif (NTS_CLIENT_UNIQUE_ID_SIZE < 32)
   #error NTS_CLIENT_UNIQUE_ID_SIZE parameter is not valid
#endif

//Size of the nonce
#ifndef NTS_CLIENT_NONCE_SIZE
   #define NTS_CLIENT_NONCE_SIZE 16
#elif (NTS_CLIENT_NONCE_SIZE < 16)
   #error NTS_CLIENT_NONCE_SIZE parameter is not valid
#endif

//Application specific context
#ifndef NTS_CLIENT_PRIVATE_CONTEXT
   #define NTS_CLIENT_PRIVATE_CONTEXT
#endif

//Forward declaration of NtsClientContext structure
struct _NtsClientContext;
#define NtsClientContext struct _NtsClientContext

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief NTS client states
 **/

typedef enum
{
   NTS_CLIENT_STATE_INIT                 = 0,
   NTS_CLIENT_STATE_NTS_KE_INIT          = 1,
   NTS_CLIENT_STATE_NTS_KE_CONNECTING    = 2,
   NTS_CLIENT_STATE_NTS_KE_SENDING       = 3,
   NTS_CLIENT_STATE_NTS_KE_RECEIVING     = 4,
   NTS_CLIENT_STATE_NTS_KE_DISCONNECTING = 5,
   NTS_CLIENT_STATE_NTP_RESOLVING        = 6,
   NTS_CLIENT_STATE_NTP_INIT             = 7,
   NTS_CLIENT_STATE_NTP_SENDING          = 8,
   NTS_CLIENT_STATE_NTP_RECEIVING        = 9,
   NTS_CLIENT_STATE_COMPLETE             = 10
} NtsClientState;


/**
 * @brief TLS initialization callback function
 **/

typedef error_t (*NtsClientTlsInitCallback)(NtsClientContext *context,
   TlsContext *tlsContext);


/**
 * @brief Random data generation callback function
 **/

typedef error_t (*NtsClientRandCallback)(uint8_t *data, size_t length);


/**
 * @brief NTS client context
 **/

struct _NtsClientContext
{
   NtsClientState state;                        ///<NTS client state
   NetInterface *interface;                     ///<Underlying network interface
   IpAddr ntsKeServerIpAddr;                    ///<NTS-KE server address
   uint16_t ntsKeServerPort;                    ///<NTS-KE server port
   char_t ntpServerName[NTS_CLIENT_MAX_NTP_SERVER_NAME_LEN + 1]; ///<NTP server name
   IpAddr ntpServerIpAddr;                      ///<NTP server address
   uint16_t ntpServerPort;                      ///<NTP server port
   systime_t timeout;                           ///<Timeout value
   systime_t timestamp;                         ///<Timestamp to manage timeout
   Socket *ntsKeSocket;                         ///<NTS-KE socket
   Socket *ntpSocket;                           ///<NTP socket
   TlsContext *tlsContext;                      ///<TLS context
   TlsSessionState tlsSession;                  ///<TLS session state
   NtsClientTlsInitCallback tlsInitCallback;    ///<TLS initialization callback function
   NtsClientRandCallback randCallback;          ///<Random data generation callback function
   systime_t startTime;                         ///<Request start time
   systime_t retransmitStartTime;               ///<Time at which the last request was sent
   systime_t retransmitTimeout;                 ///<Retransmission timeout
   uint8_t buffer[NTS_CLIENT_BUFFER_SIZE];      ///<Memory buffer for input/output operations
   size_t bufferLen;                            ///<Length of the buffer, in bytes
   size_t bufferPos;                            ///<Current position in the buffer
   bool_t ntsNextProtoNegoRecordReceived;       ///<The NTS Next Protocol Negotiation record has been received
   bool_t aeadAlgoNegoRecordReceived;           ///<The AEAD Algorithm Negotiation record has been received
   uint8_t c2sKey[32];                          ///<Client-to-server (C2S) key
   uint8_t s2cKey[32];                          ///<Server-to-client (S2C) key
   uint8_t cookie[NTS_CLIENT_MAX_COOKIE_SIZE];  ///<NTS cookie
   size_t cookieLen;                            ///<Length of the NTS cookie, in bytes
   uint8_t uniqueId[NTS_CLIENT_UNIQUE_ID_SIZE]; ///<Unique identifier
   uint8_t nonce[NTS_CLIENT_NONCE_SIZE];        ///<Nonce
   uint32_t kissCode;                           ///<Kiss code
   NTS_CLIENT_PRIVATE_CONTEXT                   ///<Application specific context
};


//NTS client related functions
error_t ntsClientInit(NtsClientContext *context);

error_t ntsClientRegisterTlsInitCallback(NtsClientContext *context,
   NtsClientTlsInitCallback callback);

error_t ntsClientRegisterRandCallback(NtsClientContext *context,
   NtsClientRandCallback callback);

error_t ntsClientSetTimeout(NtsClientContext *context, systime_t timeout);

error_t ntsClientBindToInterface(NtsClientContext *context,
   NetInterface *interface);

error_t ntsClientSetServerAddr(NtsClientContext *context,
   const IpAddr *serverIpAddr, uint16_t serverPort);

error_t ntsClientGetTimestamp(NtsClientContext *context,
   NtpTimestamp *timestamp);

uint32_t ntsClientGetKissCode(NtsClientContext *context);

void ntsClientDeinit(NtsClientContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
