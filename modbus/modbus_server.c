/**
 * @file modbus_server.c
 * @brief Modbus/TCP server
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL MODBUS_TRACE_LEVEL

//Dependencies
#include "modbus/modbus_server.h"
#include "modbus/modbus_server_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (MODBUS_SERVER_SUPPORT == ENABLED)


/**
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains Modbus/TCP server settings
 **/

void modbusServerGetDefaultSettings(ModbusServerSettings *settings)
{
   //The Modbus/TCP server is not bound to any interface
   settings->interface = NULL;

   //Modbus/TCP port number
   settings->port = MODBUS_TCP_PORT;
   //Default unit identifier
   settings->unitId = MODBUS_DEFAULT_UNIT_ID;

   //Lock Modbus table callback function
   settings->lockCallback = NULL;
   //Unlock Modbus table callback function
   settings->unlockCallback = NULL;
   //Get coil state callback function
   settings->readCoilCallback = NULL;
   //Set coil state callback function
   settings->writeCoilCallback = NULL;
   //Get register value callback function
   settings->readRegCallback = NULL;
   //Set register value callback function
   settings->writeRegValueCallback = NULL;
   //PDU processing callback
   settings->processPduCallback = NULL;
}


/**
 * @brief Initialize Modbus/TCP server context
 * @param[in] context Pointer to the Modbus/TCP server context
 * @param[in] settings Modbus/TCP server specific settings
 * @return Error code
 **/

error_t modbusServerInit(ModbusServerContext *context,
   const ModbusServerSettings *settings)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing Modbus/TCP server...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;

   //Clear Modbus/TCP server context
   memset(context, 0, sizeof(ModbusServerContext));

   //Save user settings
   context->settings = *settings;

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
      if(context->socket == NULL)
      {
         //Report an error
         error = ERROR_OPEN_FAILED;
         //Exit immediately
         break;
      }

      //Associate the socket with the relevant interface
      error = socketBindToInterface(context->socket, settings->interface);
      //Any error to report?
      if(error)
         break;

      //The Modbus/TCP server listens for connection requests on port 502
      error = socketBind(context->socket, &IP_ADDR_ANY, settings->port);
      //Any error to report?
      if(error)
         break;

      //Place socket in listening state
      error = socketListen(context->socket, 0);
      //Any error to report?
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
 * @brief Start Modbus/TCP server
 * @param[in] context Pointer to the Modbus/TCP server context
 * @return Error code
 **/

error_t modbusServerStart(ModbusServerContext *context)
{
   OsTask *task;

   //Debug message
   TRACE_INFO("Starting Modbus/TCP server...\r\n");

   //Make sure the Modbus/TCP server context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Create the Modbus/TCP server task
   task = osCreateTask("Modbus/TCP Server", (OsTaskCode) modbusServerTask,
      context, MODBUS_SERVER_STACK_SIZE, MODBUS_SERVER_PRIORITY);

   //Unable to create the task?
   if(task == OS_INVALID_HANDLE)
      return ERROR_OUT_OF_RESOURCES;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Modbus/TCP server task
 * @param[in] context Pointer to the Modbus/TCP server context
 **/

void modbusServerTask(ModbusServerContext *context)
{
   error_t error;
   uint_t i;
   ModbusClientConnection *connection;

#if (NET_RTOS_SUPPORT == ENABLED)
   //Process events
   while(1)
   {
#endif
      //Clear event descriptor set
      memset(context->eventDesc, 0, sizeof(context->eventDesc));

      //Specify the events the application is interested in
      for(i = 0; i < MODBUS_SERVER_MAX_CONNECTIONS; i++)
      {
         //Point to the structure describing the current connection
         connection = &context->connection[i];

         //Check the state of the connection
         if(connection->state == MODBUS_CONNECTION_STATE_RECEIVING)
         {
            //Wait for data to be available for reading
            context->eventDesc[i].socket = connection->socket;
            context->eventDesc[i].eventMask = SOCKET_EVENT_RX_READY;
         }
         else if(connection->state == MODBUS_CONNECTION_STATE_SENDING)
         {
            //Wait until there is more room in the send buffer
            context->eventDesc[i].socket = connection->socket;
            context->eventDesc[i].eventMask = SOCKET_EVENT_TX_READY;
         }
      }

      //The Modbus/TCP server listens for connection requests on port 502
      context->eventDesc[i].socket = context->socket;
      context->eventDesc[i].eventMask = SOCKET_EVENT_RX_READY;

      //Wait for one of the set of sockets to become ready to perform I/O
      error = socketPoll(context->eventDesc, MODBUS_SERVER_MAX_CONNECTIONS + 1,
         &context->event, MODBUS_SERVER_TICK_INTERVAL);

      //Verify status code
      if(!error)
      {
         //Event-driven processing
         for(i = 0; i < MODBUS_SERVER_MAX_CONNECTIONS; i++)
         {
            //Point to the structure describing the current connection
            connection = &context->connection[i];

            //Loop through active connections only
            if(connection->state == MODBUS_CONNECTION_STATE_RECEIVING ||
               connection->state == MODBUS_CONNECTION_STATE_SENDING)
            {
               //Check whether the socket is ready to perform I/O
               if(context->eventDesc[i].eventFlags != 0)
               {
                  //Connection event handler
                  modbusServerProcessConnectionEvents(context, connection);
               }
            }
         }

         //Any connection request received on port 502?
         if(context->eventDesc[i].eventFlags != 0)
         {
            //Accept connection request
            modbusServerAcceptConnection(context);
         }
      }

      //Handle periodic operations
      modbusServerTick(context);

#if (NET_RTOS_SUPPORT == ENABLED)
   }
#endif
}

#endif
