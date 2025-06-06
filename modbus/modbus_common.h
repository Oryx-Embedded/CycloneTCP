/**
 * @file modbus_common.h
 * @brief Definitions common to Modbus/TCP client and server
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

#ifndef _MODBUS_COMMON_H
#define _MODBUS_COMMON_H

//Dependencies
#include "core/net.h"

//Modbus/TCP port number
#define MODBUS_TCP_PORT 502
//Secure Modbus/TCP port number
#define MODBUS_TCP_SECURE_PORT 802

//Modbus protocol identifier
#define MODBUS_PROTOCOL_ID 0
//Default unit identifier
#define MODBUS_DEFAULT_UNIT_ID 255

//Maximum size of Modbus PDU
#define MODBUS_MAX_PDU_SIZE 253
//Maximum size of Modbus/TCP ADU
#define MODBUS_MAX_ADU_SIZE 260

//Function code mask
#define MODBUS_FUNCTION_CODE_MASK 0x7F
//Exception response mask
#define MODBUS_EXCEPTION_MASK 0x80

//Set coil value
#define MODBUS_SET_COIL(a, n) ((a)[(n) / 8] |= (1 << ((n) % 8)))
//Reset coil value
#define MODBUS_RESET_COIL(a, n) ((a)[(n) / 8] &= ~(1 << ((n) % 8)))
//Test coil value
#define MODBUS_TEST_COIL(a, n) ((a[(n) / 8] >> ((n) % 8)) & 1)

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Modbus functions codes
 **/

typedef enum
{
   MODBUS_FUNCTION_READ_COILS                = 1,
   MODBUS_FUNCTION_READ_DISCRETE_INPUTS      = 2,
   MODBUS_FUNCTION_READ_HOLDING_REGS         = 3,
   MODBUS_FUNCTION_READ_INPUT_REGS           = 4,
   MODBUS_FUNCTION_WRITE_SINGLE_COIL         = 5,
   MODBUS_FUNCTION_WRITE_SINGLE_REG          = 6,
   MODBUS_FUNCTION_READ_EXCEPTION_STATUS     = 7,
   MODBUS_FUNCTION_DIAGNOSTICS               = 8,
   MODBUS_FUNCTION_GET_COMM_EVENT_COUNTER    = 11,
   MODBUS_FUNCTION_GET_COMM_EVENT_LOG        = 12,
   MODBUS_FUNCTION_WRITE_MULTIPLE_COILS      = 15,
   MODBUS_FUNCTION_WRITE_MULTIPLE_REGS       = 16,
   MODBUS_FUNCTION_REPORT_SLAVE_ID           = 17,
   MODBUS_FUNCTION_READ_FILE_RECORD          = 20,
   MODBUS_FUNCTION_WRITE_FILE_RECORD         = 21,
   MODBUS_FUNCTION_MASK_WRITE_REG            = 22,
   MODBUS_FUNCTION_READ_WRITE_MULTIPLE_REGS  = 23,
   MODBUS_FUNCTION_READ_FIFO_QUEUE           = 24,
   MODBUS_FUNCTION_ENCAPSULATED_IF_TRANSPORT = 43
} ModbusFunctionCode;


/**
 * @brief Modbus exception codes
 **/

typedef enum
{
   MODBUS_EXCEPTION_ILLEGAL_FUNCTION                = 1,
   MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS            = 2,
   MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE              = 3,
   MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE            = 4,
   MODBUS_EXCEPTION_ACKNOWLEDGE                     = 5,
   MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY               = 6,
   MODBUS_EXCEPTION_MEMORY_PARITY_ERROR             = 8,
   MODBUS_EXCEPTION_GATEWAY_PATH_UNAVAILABLE        = 10,
   MODBUS_EXCEPTION_GATEWAY_NO_RESPONSE_FROM_TARGET = 11
} ModbusExceptionCode;


/**
 * @brief Coil states
 **/

typedef enum
{
   MODBUS_COIL_STATE_OFF = 0x0000,
   MODBUS_COIL_STATE_ON  = 0xFF00
} ModbusCoilState;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma pack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief MBAP header (Modbus Application Protocol)
 **/

typedef __packed_struct
{
   uint16_t transactionId; //0-1
   uint16_t protocolId;    //2-3
   uint16_t length;        //4-5
   uint8_t unitId;         //6
   uint8_t pdu[];          //7
} ModbusHeader;


/**
 * @brief Read Coils request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;     //0
   uint16_t startingAddr;    //1-2
   uint16_t quantityOfCoils; //3-4
} ModbusReadCoilsReq;


/**
 * @brief Read Coils response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode; //0
   uint8_t byteCount;    //1
   uint8_t coilStatus[]; //2
} ModbusReadCoilsResp;


/**
 * @brief Read Discrete Inputs request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;      //0
   uint16_t startingAddr;     //1-2
   uint16_t quantityOfInputs; //3-4
} ModbusReadDiscreteInputsReq;


/**
 * @brief Read Discrete Inputs response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;  //0
   uint8_t byteCount;     //1
   uint8_t inputStatus[]; //2
} ModbusReadDiscreteInputsResp;


/**
 * @brief Read Holding Registers request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;    //0
   uint16_t startingAddr;   //1-2
   uint16_t quantityOfRegs; //3-4
} ModbusReadHoldingRegsReq;


/**
 * @brief Read Holding Registers response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode; //0
   uint8_t byteCount;    //1
   uint16_t regValue[];  //2
} ModbusReadHoldingRegsResp;


/**
 * @brief Read Holding Input request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;    //0
   uint16_t startingAddr;   //1-2
   uint16_t quantityOfRegs; //3-4
} ModbusReadInputRegsReq;


/**
 * @brief Read Holding Input response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode; //0
   uint8_t byteCount;    //1
   uint16_t regValue[];  //2
} ModbusReadInputRegsResp;


/**
 * @brief Write Single Coil request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;  //0
   uint16_t outputAddr;   //1-2
   uint16_t outputValue;  //3-4
} ModbusWriteSingleCoilReq;


/**
 * @brief Write Single Coil response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;  //0
   uint16_t outputAddr;   //1-2
   uint16_t outputValue;  //3-4
} ModbusWriteSingleCoilResp;


/**
 * @brief Write Single Register request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode; //0
   uint16_t regAddr;     //1-2
   uint16_t regValue;    //3-4
} ModbusWriteSingleRegReq;


/**
 * @brief Write Single Register response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode; //0
   uint16_t regAddr;     //1-2
   uint16_t regValue;    //3-4
} ModbusWriteSingleRegResp;


/**
 * @brief Write Multiple Coils request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;       //0
   uint16_t startingAddr;      //1-2
   uint16_t quantityOfOutputs; //3-4
   uint8_t byteCount;          //5
   uint8_t outputValue[];      //6
} ModbusWriteMultipleCoilsReq;


/**
 * @brief Write Multiple Coils response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;       //0
   uint16_t startingAddr;      //1-2
   uint16_t quantityOfOutputs; //3-4
} ModbusWriteMultipleCoilsResp;


/**
 * @brief Write Multiple Registers request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;    //0
   uint16_t startingAddr;   //1-2
   uint16_t quantityOfRegs; //3-4
   uint8_t byteCount;       //5
   uint16_t regValue[];     //6
} ModbusWriteMultipleRegsReq;


/**
 * @brief Write Multiple Registers response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;     //0
   uint16_t startingAddr;    //1-2
   uint16_t quantityOfRegs;  //3-4
} ModbusWriteMultipleRegsResp;


/**
 * @brief Mask Write Register request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;   //0
   uint16_t referenceAddr; //1-2
   uint16_t andMask;       //3-4
   uint16_t orMask;        //5-6
} ModbusMaskWriteRegReq;


/**
 * @brief Mask Write Register response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;   //0
   uint16_t referenceAddr; //1-2
   uint16_t andMask;       //3-4
   uint16_t orMask;        //5-6
} ModbusMaskWriteRegResp;


/**
 * @brief Read/Write Multiple Registers request PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;       //0
   uint16_t readStartingAddr;  //1-2
   uint16_t quantityToRead;    //3-4
   uint16_t writeStartingAddr; //5-6
   uint16_t quantityToWrite;   //7-8
   uint8_t writeByteCount;     //9
   uint16_t writeRegValue[];   //10
} ModbusReadWriteMultipleRegsReq;


/**
 * @brief Read/Write Multiple Registers response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;    //0
   uint8_t readByteCount;   //1
   uint16_t readRegValue[]; //2
} ModbusReadWriteMultipleRegsResp;


/**
 * @brief Exception response PDU
 **/

typedef __packed_struct
{
   uint8_t functionCode;  //0
   uint8_t exceptionCode; //1
} ModbusExceptionResp;


//CC-RX, CodeWarrior or Win32 compiler?
#if defined(__CCRX__)
   #pragma unpack
#elif defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
