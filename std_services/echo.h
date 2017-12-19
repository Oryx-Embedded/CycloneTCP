/**
 * @file echo.h
 * @brief Echo protocol
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

#ifndef _ECHO_H
#define _ECHO_H

//Dependencies
#include "core/net.h"
#include "core/socket.h"

//Stack size required to run the echo service
#ifndef ECHO_SERVICE_STACK_SIZE
   #define ECHO_SERVICE_STACK_SIZE 600
#elif (ECHO_SERVICE_STACK_SIZE < 1)
   #error ECHO_SERVICE_STACK_SIZE parameter is not valid
#endif

//Priority at which the echo service should run
#ifndef ECHO_SERVICE_PRIORITY
   #define ECHO_SERVICE_PRIORITY OS_TASK_PRIORITY_NORMAL
#endif

//Size of the buffer for input/output operations
#ifndef ECHO_BUFFER_SIZE
   #define ECHO_BUFFER_SIZE 1500
#elif (ECHO_BUFFER_SIZE < 1)
   #error ECHO_BUFFER_SIZE parameter is not valid
#endif

//Maximum time the TCP echo server will wait before closing the connection
#ifndef ECHO_TIMEOUT
   #define ECHO_TIMEOUT 20000
#elif (ECHO_TIMEOUT < 1)
   #error ECHO_TIMEOUT parameter is not valid
#endif

//Echo service port
#define ECHO_PORT 7

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief Echo service context
 **/

typedef struct
{
   Socket *socket;
   char_t buffer[ECHO_BUFFER_SIZE];
} EchoServiceContext;


//TCP echo service related functions
error_t tcpEchoStart(void);
void tcpEchoListenerTask(void *param);
void tcpEchoConnectionTask(void *param);

//UDP echo service related functions
error_t udpEchoStart(void);
void udpEchoTask(void *param);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
