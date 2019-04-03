/**
 * @file http_common.h
 * @brief Definitions common to HTTP client and server
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

#ifndef _HTTP_COMMON_H
#define _HTTP_COMMON_H

//Dependencies
#include "core/net.h"

//HTTP port number
#define HTTP_PORT 80
//HTTPS port number (HTTP over TLS)
#define HTTPS_PORT 443

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief HTTP version numbers
 **/

typedef enum
{
   HTTP_VERSION_0_9 = 0x0009,
   HTTP_VERSION_1_0 = 0x0100,
   HTTP_VERSION_1_1 = 0x0101
} HttpVersion;


/**
 * @brief HTTP methods
 **/

typedef enum
{
   HTTP_METHOD_GET     = 0,
   HTTP_METHOD_HEAD    = 1,
   HTTP_METHOD_POST    = 2,
   HTTP_METHOD_PUT     = 3,
   HTTP_METHOD_DELETE  = 4,
   HTTP_METHOD_TRACE   = 5,
   HTTP_METHOD_CONNECT = 6,
   HTTP_METHOD_PATCH   = 7
} HttpMethod;


/**
 * @brief HTTP authentication schemes
 **/

typedef enum
{
   HTTP_AUTH_MODE_NONE   = 0,
   HTTP_AUTH_MODE_BASIC  = 1,
   HTTP_AUTH_MODE_DIGEST = 2
} HttpAuthMode;


//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
