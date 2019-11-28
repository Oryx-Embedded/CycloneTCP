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
 * @version 1.9.6
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL MODBUS_TRACE_LEVEL

//Dependencies
#include "modbus/modbus_server.h"
#include "modbus/modbus_server_transport.h"
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

#if (MODBUS_SERVER_TLS_SUPPORT == ENABLED)
   //TLS initialization callback function
   settings->tlsInitCallback = NULL;
#endif

   //Lock Modbus table callback function
   settings->lockCallback = NULL;
   //Unlock Modbus table callback function
   settings->unlockCallback = NULL;
   //Get coil state callback function
   settings->readCoilCallback = NULL;
   //Get discrete input state callback function
   settings->readDiscreteInputCallback = NULL;
   //Set coil state callback function
   settings->writeCoilCallback = NULL;
   //Get register value callback function
   settings->readRegCallback = NULL;
   //Get holding register value callback function
   settings->readHoldingRegCallback = NULL;
   //Get input register value callback function
   settings->readInputRegCallback = NULL;
   //Set register value callback function
   settings->writeRegCallback = NULL;
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

      //Force the socket to operate in non-blocking mode
      error = socketSetTimeout(context->socket, 0);
      //Any error to report?
      if(error)
         break;

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

#if (MODBUS_SERVER_TLS_SUPPORT == ENABLED && TLS_TICKET_SUPPORT == ENABLED)
      //Initialize ticket encryption context
      error = tlsInitTicketContext(&context->tlsTicketContext);
      //Any error to report?
      if(error)
         return error;
#endif

      //End of exception handling block
   } while(0);

   //Check status code
   if(error)
   {
      //Clean up side effects
      modbusServerDeinit(context);
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
   systime_t timeout;
   ModbusClientConnection *connection;
   SocketEventDesc eventDesc[MODBUS_SERVER_MAX_CONNECTIONS + 1];

#if (NET_RTOS_SUPPORT == ENABLED)
   //Task prologue
   osEnterTask();

   //Process events
   while(1)
   {
#endif
      //Set polling timeout
      timeout = MODBUS_SERVER_TICK_INTERVAL;

      //Clear event descriptor set
      memset(eventDesc, 0, sizeof(eventDesc));

      //Specify the events the application is interested in
      for(i = 0; i < MODBUS_SERVER_MAX_CONNECTIONS; i++)
      {
         //Point to the structure describing the current connection
         connection = &context->connection[i];

         //Check the state of the connection
         if(connection->state == MODBUS_CONNECTION_STATE_CONNECT_TLS)
         {
#if (MODBUS_SERVER_TLS_SUPPORT == ENABLED)
            //Any data pending in the send buffer?
            if(tlsIsTxReady(connection->tlsContext))
            {
               //Wait until there is more room in the send buffer
               eventDesc[i].socket = connection->socket;
               eventDesc[i].eventMask = SOCKET_EVENT_TX_READY;
            }
            else
            {
               //Wait for data to be available for reading
               eventDesc[i].socket = connection->socket;
               eventDesc[i].eventMask = SOCKET_EVENT_RX_READY;
            }
#endif
         }
         else if(connection->state == MODBUS_CONNECTION_STATE_RECEIVE)
         {
#if (MODBUS_SERVER_TLS_SUPPORT == ENABLED)
            //Any data available in the receive buffer?
            if(connection->tlsContext != NULL &&
               tlsIsRxReady(connection->tlsContext))
            {
               //No need to poll the underlying socket for incoming traffic
               eventDesc[i].eventFlags |= SOCKET_EVENT_RX_READY;
               timeout = 0;
            }
            else
#endif
            {
               //Wait for data to be available for reading
               eventDesc[i].socket = connection->socket;
               eventDesc[i].eventMask = SOCKET_EVENT_RX_READY;
            }
         }
         else if(connection->state == MODBUS_CONNECTION_STATE_SEND ||
            connection->state == MODBUS_CONNECTION_STATE_SHUTDOWN_TLS)
         {
            //Wait until there is more room in the send buffer
            eventDesc[i].socket = connection->socket;
            eventDesc[i].eventMask = SOCKET_EVENT_TX_READY;
         }
         else if(connection->state == MODBUS_CONNECTION_STATE_SHUTDOWN_TX)
         {
            //Wait for the FIN to be acknowledged
            eventDesc[i].socket = connection->socket;
            eventDesc[i].eventMask = SOCKET_EVENT_TX_SHUTDOWN;
         }
         else if(connection->state == MODBUS_CONNECTION_STATE_SHUTDOWN_RX)
         {
            //Wait for a FIN to be received
            eventDesc[i].socket = connection->socket;
            eventDesc[i].eventMask = SOCKET_EVENT_RX_SHUTDOWN;
         }
         else
         {
            //Just for sanity
         }
      }

      //The Modbus/TCP server listens for connection requests on port 502
      eventDesc[i].socket = context->socket;
      eventDesc[i].eventMask = SOCKET_EVENT_RX_READY;

      //Wait for one of the set of sockets to become ready to perform I/O
      error = socketPoll(eventDesc, MODBUS_SERVER_MAX_CONNECTIONS + 1,
         &context->event, timeout);

      //Check status code
      if(error == NO_ERROR || error == ERROR_TIMEOUT)
      {
         //Event-driven processing
         for(i = 0; i < MODBUS_SERVER_MAX_CONNECTIONS; i++)
         {
            //Point to the structure describing the current connection
            connection = &context->connection[i];

            //Loop through active connections only
            if(connection->state != MODBUS_CONNECTION_STATE_CLOSED)
            {
               //Check whether the socket is ready to perform I/O
               if(eventDesc[i].eventFlags != 0)
               {
                  //Connection event handler
                  modbusServerProcessConnectionEvents(context, connection);
               }
            }
         }

         //Any connection request received on port 502?
         if(eventDesc[i].eventFlags != 0)
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


/**
 * @brief Release Modbus/TCP server context
 * @param[in] context Pointer to the Modbus/TCP server context
 **/

void modbusServerDeinit(ModbusServerContext *context)
{
   uint_t i;

   //Make sure the Modbus/TCP server context is valid
   if(context != NULL)
   {
      //Loop through the connection table
      for(i = 0; i < MODBUS_SERVER_MAX_CONNECTIONS; i++)
      {
         //Close client connection
         modbusServerCloseConnection(&context->connection[i]);
      }

      //Close listening socket
      socketClose(context->socket);

#if (MODBUS_SERVER_TLS_SUPPORT == ENABLED && TLS_TICKET_SUPPORT == ENABLED)
      //Release ticket encryption context
      tlsFreeTicketContext(&context->tlsTicketContext);
#endif

      //Free previously allocated resources
      osDeleteEvent(&context->event);

      //Clear Modbus/TCP server context
      memset(context, 0, sizeof(ModbusServerContext));
   }
}

#endif
