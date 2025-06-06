/**
 * @file nts_debug.h
 * @brief Data logging functions for debugging purpose (NTS)
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

#ifndef _NTS_DEBUG_H
#define _NTS_DEBUG_H

//Dependencies
#include "core/net.h"
#include "nts/nts_common.h"
#include "debug.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Parameter value/name binding
 **/

typedef struct
{
   uint_t value;
   const char_t *name;
} NtsParamName;


//Check current trace level
#if (NTS_TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   void ntsDumpNtsKeRecords(const uint8_t *records, size_t length);
   void ntsDumpNtsKeRecord(const NtsKeRecord *record, size_t length);
#else
   #define ntsDumpNtsKeRecords(records, length)
   #define ntsDumpNtsKeRecord(record, length)
#endif

void ntsDumpNtsNextProtoNegoRecord(const uint8_t *body, size_t length);
void ntsDumpAeadAlgoNegoRecord(const uint8_t *body, size_t length);
void ntsDumpNtpv4ServerNegoRecord(const uint8_t *body, size_t length);
void ntsDumpNtpv4PortNegoRecord(const uint8_t *body, size_t length);
void ntsDumpErrorRecord(const uint8_t *body, size_t length);
void ntsDumpWarningRecord(const uint8_t *body, size_t length);

const char_t *ntsGetParamName(uint_t value, const NtsParamName *paramList,
   size_t paramListLen);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
