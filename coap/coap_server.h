/**
 * @file coap_server.h
 * @brief CoAP server
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2026 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.6.6
 **/

#ifndef _COAP_SERVER_H
#define _COAP_SERVER_H

//Dependencies
#include "core/net.h"
#include "coap/coap_common.h"
#include "coap/coap_message.h"
#include "coap/coap_option.h"

//CoAP server support
#ifndef COAP_SERVER_SUPPORT
   #define COAP_SERVER_SUPPORT ENABLED
#elif (COAP_SERVER_SUPPORT != ENABLED && COAP_SERVER_SUPPORT != DISABLED)
   #error COAP_SERVER_SUPPORT parameter is not valid
#endif

//DTLS-secured CoAP support
#ifndef COAP_SERVER_DTLS_SUPPORT
   #define COAP_SERVER_DTLS_SUPPORT DISABLED
#elif (COAP_SERVER_DTLS_SUPPORT != ENABLED && COAP_SERVER_DTLS_SUPPORT != DISABLED)
   #error COAP_SERVER_DTLS_SUPPORT parameter is not valid
#endif

//Observable resource support
#ifndef COAP_SERVER_OBSERVE_SUPPORT
   #define COAP_SERVER_OBSERVE_SUPPORT DISABLED
#elif (COAP_SERVER_OBSERVE_SUPPORT != ENABLED && COAP_SERVER_OBSERVE_SUPPORT != DISABLED)
   #error COAP_SERVER_OBSERVE_SUPPORT parameter is not valid
#endif

//Stack size required to run the CoAP server
#ifndef COAP_SERVER_STACK_SIZE
   #define COAP_SERVER_STACK_SIZE 650
#elif (COAP_SERVER_STACK_SIZE < 1)
   #error COAP_SERVER_STACK_SIZE parameter is not valid
#endif

//DTLS server tick interval
#ifndef COAP_SERVER_TICK_INTERVAL
   #define COAP_SERVER_TICK_INTERVAL 250
#elif (COAP_SERVER_TICK_INTERVAL < 100)
   #error COAP_SERVER_TICK_INTERVAL parameter is not valid
#endif

//DTLS session timeout
#ifndef COAP_SERVER_SESSION_TIMEOUT
   #define COAP_SERVER_SESSION_TIMEOUT 120000
#elif (COAP_SERVER_SESSION_TIMEOUT < 0)
   #error COAP_SERVER_SESSION_TIMEOUT parameter is not valid
#endif

//Maximum number of retransmissions
#ifndef COAP_SERVER_MAX_RETRANSMIT
   #define COAP_SERVER_MAX_RETRANSMIT 4
#elif (COAP_SERVER_MAX_RETRANSMIT < 1)
   #error COAP_SERVER_MAX_RETRANSMIT parameter is not valid
#endif

//Initial retransmission timeout (minimum)
#ifndef COAP_SERVER_ACK_TIMEOUT_MIN
   #define COAP_SERVER_ACK_TIMEOUT_MIN 2000
#elif (COAP_SERVER_ACK_TIMEOUT_MIN < 1000)
   #error COAP_SERVER_ACK_TIMEOUT_MIN
#endif

//Initial retransmission timeout (maximum)
#ifndef COAP_SERVER_ACK_TIMEOUT_MAX
   #define COAP_SERVER_ACK_TIMEOUT_MAX 3000
#elif (COAP_SERVER_ACK_TIMEOUT_MAX < COAP_SERVER_ACK_TIMEOUT_MIN)
   #error COAP_SERVER_ACK_TIMEOUT_MAX
#endif

//Minimum time interval between non-confirmable notifications
#ifndef COAP_SERVER_MIN_NON_CONFIRMABLE_NOTIF_INTERVAL
   #define COAP_SERVER_MIN_NON_CONFIRMABLE_NOTIF_INTERVAL 3000
#elif (COAP_SERVER_MIN_NON_CONFIRMABLE_NOTIF_INTERVAL < 0)
   #error COAP_SERVER_MIN_NON_CONFIRMABLE_NOTIF_INTERVAL
#endif

//Maximum time interval between confirmable notifications
#ifndef COAP_SERVER_MAX_CONFIRMABLE_NOTIF_INTERVAL
   #define COAP_SERVER_MAX_CONFIRMABLE_NOTIF_INTERVAL 60000
#elif (COAP_SERVER_MAX_CONFIRMABLE_NOTIF_INTERVAL < 1000)
   #error COAP_SERVER_MAX_CONFIRMABLE_NOTIF_INTERVAL
#endif

//Size of buffer used for input/output operations
#ifndef COAP_SERVER_BUFFER_SIZE
   #define COAP_SERVER_BUFFER_SIZE 2048
#elif (COAP_SERVER_BUFFER_SIZE < 1)
   #error COAP_SERVER_BUFFER_SIZE parameter is not valid
#endif

//Maximum size of observable resources
#ifndef COAP_SERVER_MAX_OBS_RESOURCE_SIZE
   #define COAP_SERVER_MAX_OBS_RESOURCE_SIZE 512
#elif (COAP_SERVER_MAX_OBS_RESOURCE_SIZE < 1)
   #error COAP_SERVER_MAX_OBS_RESOURCE_SIZE parameter is not valid
#endif

//Maximum size of the cookie secret
#ifndef COAP_SERVER_MAX_COOKIE_SECRET_SIZE
   #define COAP_SERVER_MAX_COOKIE_SECRET_SIZE 32
#elif (COAP_SERVER_MAX_COOKIE_SECRET_SIZE < 1)
   #error COAP_SERVER_MAX_COOKIE_SECRET_SIZE parameter is not valid
#endif

//Maximum length of URI
#ifndef COAP_SERVER_MAX_URI_LEN
   #define COAP_SERVER_MAX_URI_LEN 128
#elif (COAP_SERVER_MAX_URI_LEN < 1)
   #error COAP_SERVER_MAX_URI_LEN parameter is not valid
#endif

//Priority at which the CoAP server should run
#ifndef COAP_SERVER_PRIORITY
   #define COAP_SERVER_PRIORITY OS_TASK_PRIORITY_NORMAL
#endif

//Application specific context
#ifndef COAP_SERVER_CONTEXT_PRIVATE
   #define COAP_SERVER_CONTEXT_PRIVATE
#endif

//DTLS supported?
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   #include "core/crypto.h"
   #include "tls/tls.h"
   #include "tls/tls_ticket.h"
#endif

//Forward declaration of CoapServerContext structure
struct _CoapServerContext;
#define CoapServerContext struct _CoapServerContext

//Forward declaration of CoapDtlsSession structure
struct _CoapDtlsSession;
#define CoapDtlsSession struct _CoapDtlsSession

//Forward declaration of CoapResource structure
struct _CoapResource;
#define CoapResource struct _CoapResource

//Forward declaration of CoapObserver structure
struct _CoapObserver;
#define CoapObserver struct _CoapObserver

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Observer states
 **/

typedef enum
{
   COAP_OBSERVER_STATE_UNREGISTERED = 0,
   COAP_OBSERVER_STATE_REGISTERED   = 1,
   COAP_OBSERVER_STATE_UPDATING     = 2
} CoapObserverState;


/**
 * @brief UDP initialization callback function
 **/

typedef error_t (*CoapServerUdpInitCallback)(CoapServerContext *context,
   Socket *socket);


//DTLS supported?
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)

/**
 * @brief DTLS initialization callback
 **/

typedef error_t (*CoapServerDtlsInitCallback)(CoapServerContext *context,
   TlsContext *dtlsContext);

#endif


/**
 * @brief CoAP request callback function
 **/

typedef error_t (*CoapServerRequestCallback)(CoapServerContext *context,
   CoapCode method, const char_t *uri);


/**
 * @brief Observe callback function
 **/

typedef error_t (*CoapServerObserveCallback)(CoapServerContext *context,
   CoapObserver *observer, CoapResource *resource);


/**
 * @brief CoAP server settings
 **/

typedef struct
{
   OsTaskParameters task;                       ///<Task parameters
   NetContext *netContext;                      ///<TCP/IP stack context
   NetInterface *interface;                     ///<Underlying network interface
   uint16_t port;                               ///<CoAP port number
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   uint_t numSessions;                          ///<Maximum number of DTLS sessions
   CoapDtlsSession *sessions;                   ///<DTLS sessions
#endif
#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   uint_t numResources;                         ///<Maximum number of observable resources
   CoapResource *resources;                     ///<Observable resources
   uint_t numObservers;                         ///<Maximum number of observers
   CoapObserver *observers;                     ///<Observers
#endif
   CoapServerUdpInitCallback udpInitCallback;   ///<UDP initialization callback
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   CoapServerDtlsInitCallback dtlsInitCallback; ///<DTLS initialization callback
#endif
   CoapServerRequestCallback requestCallback;   ///<CoAP request callback
#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   CoapServerObserveCallback observeCallback;   ///<Observe callback
#endif
} CoapServerSettings;


/**
 * @brief CoAP server context
 **/

struct _CoapServerContext
{
   NetContext *netContext;                                   ///<TCP/IP stack context
   NetInterface *interface;                                  ///<Underlying network interface
   uint16_t port;                                            ///<CoAP port number
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   uint_t numSessions;                                       ///<Maximum number of DTLS sessions
   CoapDtlsSession *sessions;                                ///<DTLS sessions
#endif
#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   uint_t numResources;                                      ///<Maximum number of observable resources
   CoapResource *resources;                                  ///<List of observable resources
   uint_t numObservers;                                      ///<Maximum number of observers
   CoapObserver *observers;                                  ///<List of registered observers
#endif
   CoapServerUdpInitCallback udpInitCallback;                ///<UDP initialization callback
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   CoapServerDtlsInitCallback dtlsInitCallback;              ///<DTLS initialization callback
#endif
   CoapServerRequestCallback requestCallback;                ///<CoAP request callback
#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   CoapServerObserveCallback observeCallback;                ///<Observe callback
#endif
   bool_t running;                                           ///<Operational state of the CoAP server
   bool_t stop;                                              ///<Stop request
   OsMutex mutex;                                            ///<Mutex preventing simultaneous access to the context
   OsEvent event;                                            ///<Event object used to poll the underlying socket
   OsTaskParameters taskParams;                              ///<Task parameters
   OsTaskId taskId;                                          ///<Task identifier
   Socket *socket;                                           ///<Underlying socket
   NetInterface *localInterface;                             ///<Network interface the CoAP request was received on
   IpAddr localIpAddr;                                       ///<Destination IP address of the received CoAP request
   IpAddr remoteIpAddr;                                      ///<Source IP address of the received CoAP request
   uint16_t remotePort;                                      ///<Source port of the received CoAP request
   uint8_t buffer[COAP_SERVER_BUFFER_SIZE];                  ///<Memory buffer for input/output operations
   size_t bufferLen;                                         ///<Length of the buffer, in bytes
   char_t uri[COAP_SERVER_MAX_URI_LEN + 1];                  ///<Resource identifier
   CoapMessage request;                                      ///<CoAP request message
   CoapMessage response;                                     ///<CoAP response message
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   uint8_t cookieSecret[COAP_SERVER_MAX_COOKIE_SECRET_SIZE]; ///<Cookie secret
   size_t cookieSecretLen;                                   ///<Length of the cookie secret, in bytes
#endif
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED && TLS_TICKET_SUPPORT == ENABLED)
   TlsTicketContext dtlsTicketContext;                       ///<DTLS ticket encryption context
#endif
#if (COAP_SERVER_OBSERVE_SUPPORT == ENABLED)
   uint16_t mid;                                             ///<Message identifier
#endif
   COAP_SERVER_CONTEXT_PRIVATE                               ///<Application specific context
};


/**
 * @brief DTLS session
 **/

struct _CoapDtlsSession
{
   CoapServerContext *context; ///<Pointer to the CoAP server context
   NetInterface *interface;    ///<Underlying network interface
   IpAddr serverIpAddr;        ///<Server's IP address
   IpAddr clientIpAddr;        ///<Client's IP address
   uint16_t clientPort;        ///<Client's port
#if (COAP_SERVER_DTLS_SUPPORT == ENABLED)
   TlsContext *dtlsContext;    ///<DTLS context
#endif
   systime_t timestamp;        ///<Timestamp to manage timeout
};


/**
 * @brief Observable resource
 **/

struct _CoapResource
{
   char_t uri[COAP_SERVER_MAX_URI_LEN + 1];         ///<Resource identifier
   uint8_t data[COAP_SERVER_MAX_OBS_RESOURCE_SIZE]; ///<Resource state
   size_t dataLen;                                  ///<Length of the resource state
   uint32_t seqNum;                                 ///<Sequence number
};


/**
 * @brief Observer
 **/

struct _CoapObserver
{
   CoapObserverState state;           ///Observer state
   CoapResource *resource;            ///<Registered resource
   NetInterface *interface;           ///<Underlying network interface
   IpAddr serverIpAddr;               ///<Server's IP address
   IpAddr clientIpAddr;               ///<Client's IP address
   uint16_t clientPort;               ///<Client's port
   CoapMessageType type;              ///<Message type
   uint16_t mid;                      ///<Message identifier
   uint8_t token[COAP_MAX_TOKEN_LEN]; ///<Token
   size_t tokenLen;                   ///<Length of the token
   uint_t retransmitCount;            ///<Retransmission counter
   systime_t retransmitTimestamp;     ///<Time at which the last message was sent
   systime_t retransmitTimeout;       ///<Retransmission timeout
   systime_t ackTimestamp;            ///<Time at which the last acknowledgement was received
   bool_t changed;                    ///<The resource state has changed
};


//CoAP server related functions
void coapServerGetDefaultSettings(CoapServerSettings *settings);

error_t coapServerInit(CoapServerContext *context,
   const CoapServerSettings *settings);

error_t coapServerSetCookieSecret(CoapServerContext *context,
   const uint8_t *cookieSecret, size_t cookieSecretLen);

error_t coapServerStart(CoapServerContext *context);
error_t coapServerStop(CoapServerContext *context);

error_t coapServerCreateResource(CoapServerContext *context, const char_t *uri);

error_t coapServerUpdateResource(CoapServerContext *context, const char_t *uri,
   const void *data, size_t length);

error_t coapServerDeleteResource(CoapServerContext *context, const char_t *uri);

void coapServerTask(CoapServerContext *context);

void coapServerDeinit(CoapServerContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
