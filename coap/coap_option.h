/**
 * @file coap_option.h
 * @brief CoAP option formatting and parsing
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

#ifndef _COAP_OPTION_H
#define _COAP_OPTION_H

//Dependencies
#include "core/net.h"
#include "coap/coap_common.h"
#include "coap/coap_message.h"

//Option delta encoding
#define COAP_OPT_DELTA_8_BITS         13
#define COAP_OPT_DELTA_16_BITS        14
#define COAP_OPT_DELTA_RESERVED       15
#define COAP_OPT_DELTA_MINUS_8_BITS   13
#define COAP_OPT_DELTA_MINUS_16_BITS  269

//Option length encoding
#define COAP_OPT_LEN_8_BITS        13
#define COAP_OPT_LEN_16_BITS       14
#define COAP_OPT_LEN_RESERVED      15
#define COAP_OPT_LEN_MINUS_8_BITS  13
#define COAP_OPT_LEN_MINUS_16_BITS 269

//Default Max-Age option value
#define COAP_DEFAULT_MAX_AGE 60

//Test whether an option is critical
#define COAP_IS_OPTION_CRITICAL(num) (((num) & 0x01U) ? TRUE : FALSE)
//Test whether an option is unsafe to forward
#define COAP_IS_OPTION_UNSAFE(num) (((num) & 0x02U) ? TRUE : FALSE)

//Set block number
#define COAP_SET_BLOCK_NUM(value, n) value = ((value) & 0x0FU) | ((n) << 4U)
//Set More flag
#define COAP_SET_BLOCK_M(value, m) value = ((value) & ~0x08U) | (((m) << 3U) & 0x08U)
//Set block size
#define COAP_SET_BLOCK_SZX(value, s) value = ((value) & ~0x07U) | ((s) & 0x07U)

//Get block number
#define COAP_GET_BLOCK_NUM(value) ((value) >> 4U)
//Get More flag
#define COAP_GET_BLOCK_M(value) (((value) >> 3U) & 0x01U)
//Get block size
#define COAP_GET_BLOCK_SZX(value) ((value) & 0x07U)

//Get block size (in bytes)
#define COAP_GET_BLOCK_SIZE(value) (16U << ((value) & 0x07U))
//Get block position from the beginning of the resource (in bytes)
#define COAP_GET_BLOCK_POS(value) (((value) & ~0x0FU) << ((value) & 0x07U))

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief CoAP options
 **/

typedef enum
{
   COAP_OPT_IF_MATCH       = 1,  //RFC 7252
   COAP_OPT_URI_HOST       = 3,  //RFC 7252
   COAP_OPT_ETAG           = 4,  //RFC 7252
   COAP_OPT_IF_NONE_MATCH  = 5,  //RFC 7252
   COAP_OPT_OBSERVE        = 6,  //RFC 7641
   COAP_OPT_URI_PORT       = 7,  //RFC 7252
   COAP_OPT_LOCATION_PATH  = 8,  //RFC 7252
   COAP_OPT_URI_PATH       = 11, //RFC 7252
   COAP_OPT_CONTENT_FORMAT = 12, //RFC 7252
   COAP_OPT_MAX_AGE        = 14, //RFC 7252
   COAP_OPT_URI_QUERY      = 15, //RFC 7252
   COAP_OPT_ACCEPT         = 17, //RFC 7252
   COAP_OPT_LOCATION_QUERY = 20, //RFC 7252
   COAP_OPT_BLOCK2         = 23, //RFC 7959
   COAP_OPT_BLOCK1         = 27, //RFC 7959
   COAP_OPT_SIZE2          = 28, //RFC 7959
   COAP_OPT_PROXY_URI      = 35, //RFC 7252
   COAP_OPT_PROXY_SCHEME   = 39, //RFC 7252
   COAP_OPT_SIZE1          = 60, //RFC 7252
   COAP_OPT_NO_RESPONSE    = 258 //RFC 7967
} CoapOptionNumber;


/**
 * @brief CoAP option formats
 **/

typedef enum
{
   COAP_OPT_FORMAT_EMPTY  = 0, ///<Zero-length sequence of bytes
   COAP_OPT_FORMAT_OPAQUE = 1, ///<Opaque sequence of bytes
   COAP_OPT_FORMAT_UINT   = 2, ///<Non-negative integer
   COAP_OPT_FORMAT_STRING = 3  ///<UTF-8 string
} CoapOptionFormat;


/**
 * @brief Observe option
 **/

typedef enum
{
   COAP_OBSERVE_REGISTER    = 0,
   COAP_OBSERVE_DEREGISTER  = 1
} CoapObserveOption;


/**
 * @brief Content-Format option
 **/

typedef enum
{
   COAP_CONTENT_FORMAT_TEXT_PLAIN       = 0,
   COAP_CONTENT_FORMAT_APP_LINK_FORMAT  = 40,
   COAP_CONTENT_FORMAT_APP_XML          = 41,
   COAP_CONTENT_FORMAT_APP_OCTET_STREAM = 42,
   COAP_CONTENT_FORMAT_APP_EXI          = 47,
   COAP_CONTENT_FORMAT_APP_JSON         = 50
} CoapContentFormat;


/**
 * @brief Block size parameter
 **/

typedef enum
{
   COAP_BLOCK_SIZE_16       = 0,
   COAP_BLOCK_SIZE_32       = 1,
   COAP_BLOCK_SIZE_64       = 2,
   COAP_BLOCK_SIZE_128      = 3,
   COAP_BLOCK_SIZE_256      = 4,
   COAP_BLOCK_SIZE_512      = 5,
   COAP_BLOCK_SIZE_1024     = 6,
   COAP_BLOCK_SIZE_RESERVED = 7,
} CoapBlockSize;


/**
 * @brief CoAP option
 **/

typedef struct
{
   uint16_t delta;
   uint16_t number;
   size_t length;
   const uint8_t *value;
} CoapOption;


/**
 * @brief CoAP option parameters
 **/

typedef struct
{
   uint16_t number;         ///<Option number
   bool_t critical;         ///<Critical property
   bool_t unsafe;           ///<Unsafe property
   bool_t noCacheKey;       ///<NoCacheKey property
   bool_t repeatable;       ///<Repeatable option
   const char_t *name;      ///<Option name
   CoapOptionFormat format; ///<Option format
   uint16_t minLength;      ///<Minimum acceptable length
   uint16_t maxLength;      ///<Maximum acceptable length
} CoapOptionParameters;


//CoAP option related functions
error_t coapParseOptions(const uint8_t *p, size_t length, size_t *consumed);

error_t coapParseOption(const uint8_t *p, size_t length,
   uint16_t prevOptionNum, CoapOption *option, size_t *consumed);

error_t coapFormatOption(uint8_t *p, uint16_t prevOptionNum,
   CoapOption *option, size_t *written);

error_t coapSetOption(CoapMessage *message, uint16_t optionNum,
   uint_t optionIndex, const uint8_t *optionValue, size_t optionLen);

error_t coapSetUintOption(CoapMessage *message, uint16_t optionNum,
   uint_t optionIndex, uint32_t optionValue);

error_t coapGetOption(const CoapMessage *message, uint16_t optionNum,
   uint_t optionIndex, const uint8_t **optionValue, size_t *optionLen);

error_t coapGetUintOption(const CoapMessage *message, uint16_t optionNum,
   uint_t optionIndex, uint32_t *optionValue);

error_t coapDeleteOption(CoapMessage *message, uint16_t optionNum,
   uint_t optionIndex);

error_t coapSplitRepeatableOption(CoapMessage *message, uint16_t optionNum,
   const char_t *optionValue, char_t separator);

error_t coapJoinRepeatableOption(const CoapMessage *message,
   uint16_t optionNum, char_t *optionValue, size_t maxLen, char_t separator);

const CoapOptionParameters *coapGetOptionParameters(uint16_t optionNum);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
