/**
 * @file ftp_client_transport.h
 * @brief Transport protocol abstraction layer
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

#ifndef _FTP_CLIENT_TRANSPORT_H
#define _FTP_CLIENT_TRANSPORT_H

//Dependencies
#include "core/net.h"
#include "ftp/ftp_client.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//FTP client related functions
error_t ftpClientOpenChannel(FtpClientContext *context,
   FtpClientChannel *channel, size_t txBufferSize, size_t rxBufferSize);

error_t ftpClientOpenSecureChannel(FtpClientContext *context,
   FtpClientChannel *channel, size_t txBufferSize, size_t rxBufferSize);

error_t ftpClientEstablishSecureChannel(FtpClientChannel *channel);

error_t ftpClientShutdownChannel(FtpClientChannel *channel);

void ftpClientCloseChannel(FtpClientChannel *channel);

error_t ftpClientWriteChannel(FtpClientChannel *channel, const void *data,
   size_t length, size_t *written, uint_t flags);

error_t ftpClientReadChannel(FtpClientChannel *channel, void *data,
   size_t size, size_t *received, uint_t flags);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
