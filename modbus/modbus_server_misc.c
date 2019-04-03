/**
 * @file modbus_server_misc.c
 * @brief Helper functions for Modbus/TCP server
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
#include "modbus/modbus_server_pdu.h"
#include "modbus/modbus_server_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (MODBUS_SERVER_SUPPORT == ENABLED)


/**
 * @brief Handle periodic operations
 * @param[in] context Pointer to the Modbus/TCP server context
 **/

void modbusServerTick(ModbusServerContext *context)
{
   uint_t i;
   systime_t time;
   ModbusClientConnection *connection;

   //Get current time
   time = osGetSystemTime();

   //Loop through the connection table
   for(i = 0; i < MODBUS_SERVER_MAX_CONNECTIONS; i++)
   {
      //Point to the current entry
      connection = &context->connection[i];

      //Check the state of the current connection
      if(connection->state != MODBUS_CONNECTION_STATE_CLOSED)
      {
         //Disconnect inactive client after idle timeout
         if(timeCompare(time, connection->timestamp + MODBUS_SERVER_TIMEOUT) >= 0)
         {
            //Debug message
            TRACE_INFO("Modbus server: Closing inactive connection...\r\n");
            //Close the Modbus/TCP connection
            modbusServerCloseConnection(connection);
         }
      }
   }
}


/**
 * @brief Accept connection request
 * @param[in] context Pointer to the Modbus/TCP server context
 **/

void modbusServerAcceptConnection(ModbusServerContext *context)
{
   uint_t i;
   Socket *socket;
   IpAddr clientIpAddr;
   uint16_t clientPort;
   ModbusClientConnection *connection;

   //Accept incoming connection
   socket = socketAccept(context->socket, &clientIpAddr, &clientPort);

   //Make sure the socket handle is valid
   if(socket != NULL)
   {
      //Force the socket to operate in non-blocking mode
      socketSetTimeout(socket, 0);

      //Initialize pointer
      connection = NULL;

      //Loop through the connection table
      for(i = 0; i < MODBUS_SERVER_MAX_CONNECTIONS; i++)
      {
         //Check the state of the current connection
         if(context->connection[i].state == MODBUS_CONNECTION_STATE_CLOSED)
         {
            //The current entry is free
            connection = &context->connection[i];
            break;
         }
      }

      //If the connection table runs out of space, then the client's connection
      //request is rejected
      if(connection != NULL)
      {
         //Debug message
         TRACE_INFO("Modbus Server: Connection established with client %s port %"
            PRIu16 "...\r\n", ipAddrToString(&clientIpAddr, NULL), clientPort);

         //Clear the structure describing the connection
         memset(connection, 0, sizeof(ModbusClientConnection));

         //Attach Modbus/TCP server context
         connection->context = context;
         //Save socket handle
         connection->socket = socket;
         //Initialize time stamp
         connection->timestamp = osGetSystemTime();

         //Wait for incoming Modbus requests
         connection->state = MODBUS_CONNECTION_STATE_RECEIVING;
      }
      else
      {
         //Debug message
         TRACE_INFO("Modbus Server: Connection refused with client %s port %"
            PRIu16 "...\r\n", ipAddrToString(&clientIpAddr, NULL), clientPort);

         //The Modbus/TCP server cannot accept the incoming connection request
         socketClose(socket);
      }
   }
}


/**
 * @brief Connection event handler
 * @param[in] connection Pointer to the client connection
 **/

void modbusServerCloseConnection(ModbusClientConnection *connection)
{
   //Valid connection?
   if(connection != NULL)
   {
      //Debug message
      TRACE_INFO("Modbus Server: Closing connection...\r\n");

      //Valid socket handle?
      if(connection->socket != NULL)
      {
         //Close TCP socket
         socketClose(connection->socket);
         connection->socket = NULL;
      }

      //Mark the connection as closed
      connection->state = MODBUS_CONNECTION_STATE_CLOSED;
   }
}


/**
 * @brief Connection event handler
 * @param[in] context Pointer to the Modbus/TCP server context
 * @param[in] connection Pointer to the client connection
 **/

void modbusServerProcessConnectionEvents(ModbusServerContext *context,
   ModbusClientConnection *connection)
{
   error_t error;
   size_t n;

   //Initialize status code
   error = NO_ERROR;

   //Update time stamp
   connection->timestamp = osGetSystemTime();

   //Check the state of the connection
   if(connection->state == MODBUS_CONNECTION_STATE_RECEIVING)
   {
      //Receive Modbus request
      if(connection->requestAduPos < sizeof(ModbusHeader))
      {
         //Receive more data
         error = socketReceive(connection->socket,
            connection->requestAdu + connection->requestAduPos,
            sizeof(ModbusHeader) - connection->requestAduPos, &n, 0);

         //Check status code
         if(!error)
         {
            //Advance data pointer
            connection->requestAduPos += n;

            //MBAP header successfully received?
            if(connection->requestAduPos >= sizeof(ModbusHeader))
            {
               //Parse MBAP header
               error = modbusServerParseMbapHeader(connection);
            }
         }
      }
      else if(connection->requestAduPos < connection->requestAduLen)
      {
         //Receive more data
         error = socketReceive(connection->socket,
            connection->requestAdu + connection->requestAduPos,
            connection->requestAduLen - connection->requestAduPos, &n, 0);

         //Check status code
         if(!error)
         {
            //Advance data pointer
            connection->requestAduPos += n;

            //Modbus request successfully received?
            if(connection->requestAduPos >= connection->requestAduLen)
            {
               //Check unit identifier
               if(context->settings.unitId == 0 ||
                  context->settings.unitId == 255 ||
                  context->settings.unitId == connection->requestUnitId)
               {
                  //Process Modbus request
                  error = modbusServerProcessRequest(connection);
               }
            }
         }
      }
      else
      {
         //Just for sanity
         error = ERROR_WRONG_STATE;
      }
   }
   else if(connection->state == MODBUS_CONNECTION_STATE_SENDING)
   {
      //Send Modbus response
      if(connection->responseAduPos < connection->responseAduLen)
      {
         //Send more data
         error = socketSend(connection->socket,
            connection->responseAdu + connection->responseAduPos,
            connection->responseAduLen - connection->responseAduPos,
            &n, SOCKET_FLAG_NO_DELAY);

         //Check status code
         if(!error)
         {
            //Advance data pointer
            connection->responseAduPos += n;

            //Modbus response successfully sent?
            if(connection->responseAduPos >= connection->responseAduLen)
            {
               //Flush receive buffer
               connection->requestAduLen = 0;
               connection->requestAduPos = 0;

               //Wait for the next Modbus request
               connection->state = MODBUS_CONNECTION_STATE_RECEIVING;
            }
         }
      }
      else
      {
         //Just for sanity
         error = ERROR_WRONG_STATE;
      }
   }
   else
   {
      //Invalid state
      error = ERROR_WRONG_STATE;
   }

   //Any communication error?
   if(error != NO_ERROR && error != ERROR_TIMEOUT)
   {
      //Close the Modbus/TCP connection
      modbusServerCloseConnection(connection);
   }
}


/**
 * @brief Parse request MBAP header
 * @param[in] connection Pointer to the client connection
 * @return Error code
 **/

error_t modbusServerParseMbapHeader(ModbusClientConnection *connection)
{
   size_t n;
   ModbusHeader *requestHeader;

   //Sanity check
   if(connection->requestAduPos < sizeof(ModbusHeader))
      return ERROR_INVALID_LENGTH;

   //Point to the beginning of the request ADU
   requestHeader = (ModbusHeader *) connection->requestAdu;

   //The length field is a byte count of the following fields, including
   //the unit identifier and data fields
   n = ntohs(requestHeader->length);

   //Malformed Modbus request?
   if(n < sizeof(uint8_t))
      return ERROR_INVALID_LENGTH;

   //Retrieve the length of the PDU
   n -= sizeof(uint8_t);

   //Debug message
   TRACE_DEBUG("\r\nModbus Server: ADU received (%" PRIuSIZE " bytes)...\r\n",
      sizeof(ModbusHeader) + n);

   //Dump MBAP header
   TRACE_DEBUG("  Transaction ID = %" PRIu16 "\r\n", ntohs(requestHeader->transactionId));
   TRACE_DEBUG("  Protocol ID = %" PRIu16 "\r\n", ntohs(requestHeader->protocolId));
   TRACE_DEBUG("  Length = %" PRIu16 "\r\n", ntohs(requestHeader->length));
   TRACE_DEBUG("  Unit ID = %" PRIu16 "\r\n", requestHeader->unitId);

   //Check protocol identifier
   if(ntohs(requestHeader->protocolId) != MODBUS_PROTOCOL_ID)
      return ERROR_WRONG_IDENTIFIER;

   //The length of the Modbus PDU is limited to 253 bytes
   if(n > MODBUS_MAX_PDU_SIZE)
      return ERROR_INVALID_LENGTH;

   //Save unit identifier
   connection->requestUnitId = requestHeader->unitId;
   //Compute the length of the request ADU
   connection->requestAduLen = sizeof(ModbusHeader) + n;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Format response MBAP header
 * @param[in] connection Pointer to the client connection
 * @param[in] length Length of the PDU, in bytes
 * @return Error code
 **/

error_t modbusServerFormatMbapHeader(ModbusClientConnection *connection,
   size_t length)
{
   ModbusHeader *requestHeader;
   ModbusHeader *responseHeader;

   //Sanity check
   if(connection->requestAduPos < sizeof(ModbusHeader))
      return ERROR_INVALID_LENGTH;

   //Point to the beginning of the request ADU
   requestHeader = (ModbusHeader *) connection->requestAdu;
   //Point to the beginning of the response ADU
   responseHeader = (ModbusHeader *) connection->responseAdu;

   //Format MBAP header
   responseHeader->transactionId = requestHeader->transactionId;
   responseHeader->protocolId = requestHeader->protocolId;
   responseHeader->length = htons(length + sizeof(uint8_t));
   responseHeader->unitId = requestHeader->unitId;

   //Compute the length of the response ADU
   connection->responseAduLen = sizeof(ModbusHeader) + length;

   //Debug message
   TRACE_DEBUG("Modbus Server: Sending ADU (%" PRIuSIZE " bytes)...\r\n",
      connection->responseAduLen);

   //Dump MBAP header
   TRACE_DEBUG("  Transaction ID = %" PRIu16 "\r\n", ntohs(responseHeader->transactionId));
   TRACE_DEBUG("  Protocol ID = %" PRIu16 "\r\n", ntohs(responseHeader->protocolId));
   TRACE_DEBUG("  Length = %" PRIu16 "\r\n", ntohs(responseHeader->length));
   TRACE_DEBUG("  Unit ID = %" PRIu16 "\r\n", responseHeader->unitId);

   //Rewind to the beginning of the transmit buffer
   connection->responseAduPos = 0;
   //Send the response ADU to the client
   connection->state = MODBUS_CONNECTION_STATE_SENDING;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve request PDU
 * @param[in] connection Pointer to the client connection
 * @param[out] length Length request the request PDU, in bytes
 * @return Pointer to the request PDU
 **/

void *modbusServerGetRequestPdu(ModbusClientConnection *connection,
   size_t *length)
{
   uint8_t *requestPdu;

   //Point to the request PDU
   requestPdu = connection->requestAdu + sizeof(ModbusHeader);

   //Retrieve the length of the PDU
   if(connection->requestAduLen >= sizeof(ModbusHeader))
      *length = connection->requestAduLen - sizeof(ModbusHeader);
   else
      *length = 0;

   //Return a pointer to the request PDU
   return requestPdu;
}


/**
 * @brief Retrieve response PDU
 * @param[in] connection Pointer to the client connection
 * @return Pointer to the response PDU
 **/

void *modbusServerGetResponsePdu(ModbusClientConnection *connection)
{
   //Point to the response PDU
   return connection->responseAdu + sizeof(ModbusHeader);
}


/**
 * @brief Lock Modbus table
 * @param[in] context Pointer to the Modbus/TCP server context
 **/

void modbusServerLock(ModbusServerContext *context)
{
   //Any registered callback?
   if(context->settings.lockCallback != NULL)
   {
      //Invoke user callback function
      context->settings.lockCallback();
   }
}


/**
 * @brief Unlock Modbus table
 * @param[in] context Pointer to the Modbus/TCP server context
 **/

void modbusServerUnlock(ModbusServerContext *context)
{
   //Any registered callback?
   if(context->settings.lockCallback != NULL)
   {
      //Invoke user callback function
      context->settings.unlockCallback();
   }
}


/**
 * @brief Get coil state
 * @param[in] context Pointer to the Modbus/TCP server context
 * @param[in] address Coil address
 * @param[out] state Current coil state
 * @return Error code
 **/

error_t modbusServerReadCoil(ModbusServerContext *context, uint16_t address,
   bool_t *state)
{
   error_t error;

   //Any registered callback?
   if(context->settings.readCoilCallback!= NULL)
   {
      //Invoke user callback function
      error = context->settings.readCoilCallback(address, state);
   }
   else
   {
      //Report an error
      error = ERROR_INVALID_ADDRESS;
   }

   //Return status code
   return error;
}


/**
 * @brief Set coil state
 * @param[in] context Pointer to the Modbus/TCP server context
 * @param[in] address Address of the coil
 * @param[in] state Desired coil state
 * @param[in] commit This flag indicates the current phase (validation phase
 *   or write phase if the validation was successful)
 * @return Error code
 **/

error_t modbusServerWriteCoil(ModbusServerContext *context, uint16_t address,
   bool_t state, bool_t commit)
{
   error_t error;

   //Any registered callback?
   if(context->settings.writeCoilCallback!= NULL)
   {
      //Invoke user callback function
      error = context->settings.writeCoilCallback(address, state, commit);
   }
   else
   {
      //Report an error
      error = ERROR_INVALID_ADDRESS;
   }

   //Return status code
   return error;
}


/**
 * @brief Get register value
 * @param[in] context Pointer to the Modbus/TCP server context
 * @param[in] address Register address
 * @param[out] value Current register value
 * @return Error code
 **/

error_t modbusServerReadReg(ModbusServerContext *context, uint16_t address,
   uint16_t *value)
{
   error_t error;

   //Any registered callback?
   if(context->settings.readRegCallback!= NULL)
   {
      //Invoke user callback function
      error = context->settings.readRegCallback(address, value);
   }
   else
   {
      //Report an error
      error = ERROR_INVALID_ADDRESS;
   }

   //Return status code
   return error;
}


/**
 * @brief Set register value
 * @param[in] context Pointer to the Modbus/TCP server context
 * @param[in] address Register address
 * @param[in] value Desired register value
 * @param[in] commit This flag indicates the current phase (validation phase
 *   or write phase if the validation was successful)
 * @return Error code
 **/

error_t modbusServerWriteReg(ModbusServerContext *context, uint16_t address,
   uint16_t value, bool_t commit)
{
   error_t error;

   //Any registered callback?
   if(context->settings.writeRegValueCallback!= NULL)
   {
      //Invoke user callback function
      error = context->settings.writeRegValueCallback(address, value, commit);
   }
   else
   {
      //Report an error
      error = ERROR_INVALID_ADDRESS;
   }

   //Return status code
   return error;
}


/**
 * @brief Translate exception code
 * @param[in] status Status code
 * @return Exception code
 **/

ModbusExceptionCode modbusServerTranslateExceptionCode(error_t status)
{
   ModbusExceptionCode exceptionCode;

   //Check status code
   switch(status)
   {
   case ERROR_INVALID_FUNCTION_CODE:
      //The function code received in the query is not an allowable action
      //for the server
      exceptionCode = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
      break;
   case ERROR_INVALID_ADDRESS:
      //The data address received in the query is not an allowable address
      //for the server
      exceptionCode = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
      break;
   case ERROR_INVALID_VALUE:
      //A value contained in the query data field is not an allowable value
      //for the server
      exceptionCode = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
      break;
   case ERROR_DEVICE_BUSY:
      //The client should retransmit the message later when the server is free
      exceptionCode = MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY;
      break;
   default:
      //An unrecoverable error occurred while the server was attempting to
      //perform the requested action
      exceptionCode = MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
      break;
   }

   //Return exception code
   return exceptionCode;
}

#endif
