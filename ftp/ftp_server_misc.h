/**
 * @file ftp_server_misc.h
 * @brief FTP server (miscellaneous functions)
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.8.0
 **/

#ifndef _FTP_SERVER_MISC_H
#define _FTP_SERVER_MISC_H

//Dependencies
#include "ftp/ftp_server.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//FTP server related functions
uint16_t ftpServerGetPassivePort(FtpServerContext *context);

void ftpServerCloseConnection(FtpServerContext *context,
   FtpClientConnection *connection);

FtpClientConnection *ftpServerAcceptControlConnection(FtpServerContext *context);
void ftpServerCloseControlConnection(FtpClientConnection *connection);

error_t ftpServerOpenDataConnection(FtpServerContext *context,
   FtpClientConnection *connection);

void ftpServerAcceptDataConnection(FtpClientConnection *connection);
void ftpServerCloseDataConnection(FtpClientConnection *connection);

error_t ftpServerGetPath(FtpClientConnection *connection,
   const char_t *inputPath, char_t *outputPath, size_t maxLen);

uint_t ftpServerGetFilePermissions(FtpServerContext *context,
   FtpClientConnection *connection, const char_t *path);

const char_t *ftpServerStripRootDir(FtpServerContext *context, const char_t *path);
const char_t *ftpServerStripHomeDir(FtpClientConnection *connection, const char_t *path);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
