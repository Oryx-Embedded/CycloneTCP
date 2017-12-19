/**
 * @file discard.h
 * @brief Discard protocol
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

#ifndef _DISCARD_H
#define _DISCARD_H

//Dependencies
#include "core/net.h"
#include "core/socket.h"

//Stack size required to run the discard service
#ifndef DISCARD_SERVICE_STACK_SIZE
   #define DISCARD_SERVICE_STACK_SIZE 600
#elif (DISCARD_SERVICE_STACK_SIZE < 1)
   #error DISCARD_SERVICE_STACK_SIZE parameter is not valid
#endif

//Priority at which the discard service should run
#ifndef DISCARD_SERVICE_PRIORITY
   #define DISCARD_SERVICE_PRIORITY OS_TASK_PRIORITY_NORMAL
#endif

//Size of the buffer for input/output operations
#ifndef DISCARD_BUFFER_SIZE
   #define DISCARD_BUFFER_SIZE 1500
#elif (DISCARD_BUFFER_SIZE < 1)
   #error DISCARD_BUFFER_SIZE parameter is not valid
#endif

//Maximum time the TCP discard server will wait before closing the connection
#ifndef DISCARD_TIMEOUT
   #define DISCARD_TIMEOUT 20000
#elif (DISCARD_TIMEOUT < 1)
   #error DISCARD_TIMEOUT parameter is not valid
#endif

//Discard service port
#define DISCARD_PORT 9

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief Discard service context
 **/

typedef struct
{
   Socket *socket;
   char_t buffer[DISCARD_BUFFER_SIZE];
} DiscardServiceContext;


//TCP discard service related functions
error_t tcpDiscardStart(void);
void tcpDiscardListenerTask(void *param);
void tcpDiscardConnectionTask(void *param);

//UDP discard service related functions
error_t udpDiscardStart(void);
void udpDiscardTask(void *param);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
