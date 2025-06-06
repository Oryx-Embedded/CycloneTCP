/**
 * @file cc33xx_driver.h
 * @brief CC3300/CC3301 Wi-Fi controller
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

#ifndef _CC33XX_DRIVER_H
#define _CC33XX_DRIVER_H

//Dependencies
#include "core/nic.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//CC33xx driver (STA mode)
extern const NicDriver cc33xxStaDriver;
//CC33xx driver (AP mode)
extern const NicDriver cc33xxApDriver;

//CC33xx related functions
error_t cc33xxInit(NetInterface *interface);

void cc33xxTick(NetInterface *interface);

void cc33xxEnableIrq(NetInterface *interface);
void cc33xxDisableIrq(NetInterface *interface);
void cc33xxEventHandler(NetInterface *interface);

error_t cc33xxSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t cc33xxUpdateMacAddrFilter(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
