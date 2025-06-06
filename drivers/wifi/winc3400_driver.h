/**
 * @file winc3400_driver.h
 * @brief WINC3400 Wi-Fi controller
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

#ifndef _WINC3400_DRIVER_H
#define _WINC3400_DRIVER_H

//Dependencies
#include "core/nic.h"

//TX buffer size
#ifndef WINC3400_TX_BUFFER_SIZE
   #define WINC3400_TX_BUFFER_SIZE 1600
#elif (WINC3400_TX_BUFFER_SIZE != 1600)
   #error WINC3400_TX_BUFFER_SIZE parameter is not valid
#endif

//RX buffer size
#ifndef WINC3400_RX_BUFFER_SIZE
   #define WINC3400_RX_BUFFER_SIZE 1600
#elif (WINC3400_RX_BUFFER_SIZE != 1600)
   #error WINC3400_RX_BUFFER_SIZE parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//WINC3400 driver
extern const NicDriver winc3400Driver;

//WINC3400 related functions
error_t winc3400Init(NetInterface *interface);

void winc3400Tick(NetInterface *interface);

void winc3400EnableIrq(NetInterface *interface);
void winc3400DisableIrq(NetInterface *interface);
bool_t winc3400IrqHandler(void);
void winc3400EventHandler(NetInterface *interface);

error_t winc3400SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t winc3400UpdateMacAddrFilter(NetInterface *interface);

void winc3400AppWifiEvent(uint8_t msgType, void *msg);
void winc3400AppEthEvent(uint8_t msgType, void *msg, void *ctrlBuf);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
