/**
 * @file modbus_server.h
 * @brief Modbus/TCP server
 *
 * @section License
 *
 * Copyright (C) 2010-2018 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.8.6
 **/

#ifndef _MODBUS_SERVER_H
#define _MODBUS_SERVER_H

//Dependencies
#include "core/net.h"
#include "modbus/modbus_common.h"

//Modbus/TCP server support
#ifndef MODBUS_SERVER_SUPPORT
   #define MODBUS_SERVER_SUPPORT ENABLED
#elif (MODBUS_SERVER_SUPPORT != ENABLED && MODBUS_SERVER_SUPPORT != DISABLED)
   #error MODBUS_SERVER_SUPPORT parameter is not valid
#endif

//Stack size required to run the Modbus/TCP server
#ifndef MODBUS_SERVER_STACK_SIZE
   #define MODBUS_SERVER_STACK_SIZE 650
#elif (MODBUS_SERVER_STACK_SIZE < 1)
   #error MODBUS_SERVER_STACK_SIZE parameter is not valid
#endif

//Priority at which the Modbus/TCP server should run
#ifndef MODBUS_SERVER_PRIORITY
   #define MODBUS_SERVER_PRIORITY OS_TASK_PRIORITY_NORMAL
#endif

//Maximum number of simultaneous connections
#ifndef MODBUS_SERVER_MAX_CONNECTIONS
   #define MODBUS_SERVER_MAX_CONNECTIONS 2
#elif (MODBUS_SERVER_MAX_CONNECTIONS < 1)
   #error MODBUS_SERVER_MAX_CONNECTIONS parameter is not valid
#endif

//Maximum time the server will wait before closing the connection
#ifndef MODBUS_SERVER_TIMEOUT
   #define MODBUS_SERVER_TIMEOUT 60000
#elif (MODBUS_SERVER_TIMEOUT < 1000)
   #error MODBUS_SERVER_TIMEOUT parameter is not valid
#endif

//Modbus/TCP server tick interval
#ifndef MODBUS_SERVER_TICK_INTERVAL
   #define MODBUS_SERVER_TICK_INTERVAL 1000
#elif (MODBUS_SERVER_TICK_INTERVAL < 100)
   #error MODBUS_SERVER_TICK_INTERVAL parameter is not valid
#endif

//Forward declaration of ModbusServerContext structure
struct _ModbusServerContext;
#define ModbusServerContext struct _ModbusServerContext

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief Modbus/TCP connection state
 **/

typedef enum
{
   MODBUS_CONNECTION_STATE_CLOSED    = 0,
   MODBUS_CONNECTION_STATE_RECEIVING = 1,
   MODBUS_CONNECTION_STATE_SENDING   = 2,
} ModbusConnectionState;


/**
 * @brief Lock Modbus table callback function
 **/

typedef void (*ModbusServerLockCallback)(void);


/**
 * @brief Unlock Modbus table callback function
 **/

typedef void (*ModbusServerUnlockCallback)(void);


/**
 * @brief Get coil state callback function
 **/

typedef error_t (*ModbusServerReadCoilCallback)(uint16_t address,
   bool_t *state);


/**
 * @brief Set coil state callback function
 **/

typedef error_t (*ModbusServerWriteCoilCallback)(uint16_t address,
   bool_t state, bool_t commit);


/**
 * @brief Get register value callback function
 **/

typedef error_t (*ModbusServerReadRegCallback)(uint16_t address,
   uint16_t *value);


/**
 * @brief Set register value callback function
 **/

typedef error_t (*ModbusServerWriteRegCallback)(uint16_t address,
   uint16_t value, bool_t commit);


/**
 * @brief Modbus/TCP server settings
 **/

typedef struct
{
   NetInterface *interface;                            ///<Underlying network interface
   uint16_t port;                                      ///<Modbus/TCP port number
   uint8_t unitId;                                     ///<Unit identifier
   ModbusServerLockCallback lockCallback;              ///<Lock Modbus table callback function
   ModbusServerUnlockCallback unlockCallback;          ///<Unlock Modbus table callback function
   ModbusServerReadCoilCallback readCoilCallback;      ///<Get coil state callback function
   ModbusServerWriteCoilCallback writeCoilCallback;    ///<Set coil state callback function
   ModbusServerReadRegCallback readRegCallback;        ///<Get register value callback function
   ModbusServerWriteRegCallback writeRegValueCallback; ///<Set register value callback function
} ModbusServerSettings;


/**
 * @brief Modbus/TCP client connection
 **/

typedef struct
{
   ModbusConnectionState state;              ///<Connection state
   ModbusServerContext *context;             ///<Modbus/TCP server context
   Socket *socket;                           ///<Underlying socket
   systime_t timestamp;                      ///<Time stamp
   uint8_t requestAdu[MODBUS_MAX_ADU_SIZE];  ///<Request ADU
   size_t requestAduLen;                     ///<Length of the request ADU, in bytes
   size_t requestAduPos;                     ///<Current position in the request ADU
   uint8_t requestUnitId;                    ///<Unit identifier
   uint8_t responseAdu[MODBUS_MAX_ADU_SIZE]; ///<Response ADU
   size_t responseAduLen;                    ///<Length of the response ADU, in bytes
   size_t responseAduPos;                    ///<Current position in the response ADU
} ModbusClientConnection;


/**
 * @brief Modbus/TCP server context
 **/

struct _ModbusServerContext
{
   ModbusServerSettings settings;                                    ///<User settings
   OsEvent event;                                                    ///<Event object used to poll the sockets
   Socket *socket;                                                   ///<Listening socket
   ModbusClientConnection connection[MODBUS_SERVER_MAX_CONNECTIONS]; ///<Client connections
   SocketEventDesc eventDesc[MODBUS_SERVER_MAX_CONNECTIONS + 1];     ///<The events the application is interested in
};


//Modbus/TCP server related functions
void modbusServerGetDefaultSettings(ModbusServerSettings *settings);

error_t modbusServerInit(ModbusServerContext *context,
   const ModbusServerSettings *settings);

error_t modbusServerStart(ModbusServerContext *context);

void modbusServerTask(ModbusServerContext *context);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
