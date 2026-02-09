/**
 * @file if_mib_module.h
 * @brief Interfaces Group MIB module
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2026 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.6.0
 **/

#ifndef _IF_MIB_MODULE_H
#define _IF_MIB_MODULE_H

//Dependencies
#include "mibs/mib_common.h"

//Interfaces Group MIB module support
#ifndef IF_MIB_SUPPORT
   #define IF_MIB_SUPPORT DISABLED
#elif (IF_MIB_SUPPORT != ENABLED && IF_MIB_SUPPORT != DISABLED)
   #error IF_MIB_SUPPORT parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Interface types
 **/

typedef enum
{
   IF_MIB_IF_TYPE_OTHER           = 1,
   IF_MIB_IF_TYPE_ETHERNET_CSMACD = 6,
   IF_MIB_IF_TYPE_PROP_PTP_SERIAL = 22,
   IF_MIB_IF_TYPE_PPP             = 23,
   IF_MIB_IF_TYPE_SOFT_LOOPBACK   = 24,
   IF_MIB_IF_TYPE_SLIP            = 28,
   IF_MIB_IF_TYPE_RS232           = 33,
   IF_MIB_IF_TYPE_PARA            = 34,
   IF_MIB_IF_TYPE_IEEE_802_11     = 71,
   IF_MIB_IF_TYPE_HDLC            = 118,
   IF_MIB_IF_TYPE_TUNNEL          = 131,
   IF_MIB_IF_TYPE_L2_VLAN         = 135,
   IF_MIB_IF_TYPE_USB             = 160,
   IF_MIB_IF_TYPE_PLC             = 174,
   IF_MIB_IF_TYPE_BRIDGE          = 209,
   IF_MIB_IF_TYPE_IEEE_802_15_4   = 259
} IfMibIfType;


/**
 * @brief The desired state of the interface
 **/

typedef enum
{
   IF_MIB_IF_ADMIN_STATUS_UP      = 1,
   IF_MIB_IF_ADMIN_STATUS_DOWN    = 2,
   IF_MIB_IF_ADMIN_STATUS_TESTING = 3
} IfMibIfAdminStatus;


/**
 * @brief The operational state of the interface
 **/

typedef enum
{
   IF_MIB_IF_OPER_STATUS_UP      = 1,
   IF_MIB_IF_OPER_STATUS_DOWN    = 2,
   IF_MIB_IF_OPER_STATUS_TESTING = 3
} IfMibIfOperStatus;


/**
 * @brief Enable linkUp/linkDown traps
 **/

typedef enum
{
   IF_MIB_IF_LINK_UP_DOWN_TRAP_ENABLED  = 1,
   IF_MIB_IF_LINK_UP_DOWN_TRAP_DISABLED = 2
} IfMibIfLinkUpDownEnable;


/**
 * @brief Address type
 **/

typedef enum
{
   IF_MIB_RCV_ADDRESS_TYPE_OTHER        = 1,
   IF_MIB_RCV_ADDRESS_TYPE_VOLATILE     = 2,
   IF_MIB_RCV_ADDRESS_TYPE_NON_VOLATILE = 3
} IfMibRcvAddressType;


/**
 * @brief Interfaces Group MIB base
 **/

typedef struct
{
   uint32_t ifTableLastChange;
   uint32_t ifStackLastChange;
} IfMibBase;


//Interfaces Group MIB related constants
extern IfMibBase ifMibBase;
extern const MibObject ifMibObjects[];
extern const MibModule ifMibModule;

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
