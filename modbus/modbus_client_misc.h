/**
 * @file modbus_client_misc.h
 * @brief Helper functions for Modbus/TCP client
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

#ifndef _MODBUS_CLIENT_MISC_H
#define _MODBUS_CLIENT_MISC_H

//Dependencies
#include "core/net.h"
#include "modbus/modbus_client.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//Modbus/TCP client related functions
error_t modbusClientTransaction(ModbusClientContext *context);
error_t modbusClientCheckResp(ModbusClientContext *context);

error_t modbusClientFormatMbapHeader(ModbusClientContext *context,
   size_t length);

error_t modbusClientParseMbapHeader(ModbusClientContext *context);

void *modbusClientGetRequestPdu(ModbusClientContext *context);
void *modbusClientGetResponsePdu(ModbusClientContext *context, size_t *length);

error_t modbusClientCheckTimeout(ModbusClientContext *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
