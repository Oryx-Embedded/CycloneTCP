/**
 * @file lldp_rx_fsm.h
 * @brief LLDP receive state machine
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2024 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.4.2
 **/

#ifndef _LLDP_RX_FSM_H
#define _LLDP_RX_FSM_H

//Dependencies
#include "core/net.h"
#include "lldp/lldp.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief LLDP receive states
 **/

typedef enum
{
   LLDP_STATE_LLDP_WAIT_PORT_OPERATIONAL = 0,
   LLDP_STATE_DELETE_AGED_INFO           = 1,
   LLDP_STATE_RX_LLDP_INITIALIZE         = 2,
   LLDP_STATE_RX_WAIT_FOR_FRAME          = 3,
   LLDP_STATE_RX_FRAME                   = 4,
   LLDP_STATE_DELETE_INFO                = 5,
   LLDP_STATE_UPDATE_INFO                = 6
} LldpRxState;


//LLDP related functions
void lldpInitRxFsm(LldpPortEntry *port);
void lldpRxFsm(LldpPortEntry *port);
void lldpChangeRxState(LldpPortEntry *port, LldpRxState newState);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
