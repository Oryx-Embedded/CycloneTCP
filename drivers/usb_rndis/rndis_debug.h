/**
 * @file rndis_debug.h
 * @brief RNDIS (Remote Network Driver Interface Specification)
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Pro.
 *
 * This software is provided under a commercial license. You may
 * use this software under the conditions stated in the license
 * terms. This source code cannot be redistributed.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.9.2
 **/

#ifndef _RNDIS_DEBUG_H
#define _RNDIS_DEBUG_H

//Dependencies
#include "rndis.h"


/**
 * @brief Value/name pair
 **/

typedef struct
{
   uint32_t value;
   const char_t *name;
} RndisValueName;


//RNDIS related functions
error_t rndisDumpMsg(const RndisMsg *message, size_t length);
error_t rndisDumpPacketMsg(const RndisPacketMsg *message, size_t length);
error_t rndisDumpInitializeMsg(const RndisInitializeMsg *message, size_t length);
error_t rndisDumpHaltMsg(const RndisHaltMsg *message, size_t length);
error_t rndisDumpQueryMsg(const RndisQueryMsg *message, size_t length);
error_t rndisDumpSetMsg(const RndisSetMsg *message, size_t length);
error_t rndisDumpResetMsg(const RndisResetMsg *message, size_t length);
error_t rndisDumpIndicateStatusMsg(const RndisIndicateStatusMsg *message, size_t length);
error_t rndisDumpKeepAliveMsg(const RndisKeepAliveMsg *message, size_t length);

error_t rndisDumpInitializeCmplt(const RndisInitializeCmplt *message, size_t length);
error_t rndisDumpQueryCmplt(const RndisQueryCmplt *message, size_t length);
error_t rndisDumpSetCmplt(const RndisSetCmplt *message, size_t length);
error_t rndisDumpResetCmplt(const RndisResetCmplt *message, size_t length);
error_t rndisDumpKeepAliveCmplt(const RndisKeepAliveCmplt *message, size_t length);

const char_t *rndisFindName(uint32_t value, const RndisValueName *table, size_t size);

#endif
