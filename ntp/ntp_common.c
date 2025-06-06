/**
 * @file ntp_common.c
 * @brief Definitions common to NTP client and server
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

//Switch to the appropriate trace level
#define TRACE_LEVEL NTP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "ntp/ntp_common.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SNTP_CLIENT_SUPPORT == ENABLED || NTS_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Search a NTP packet for a given extension
 * @param[in] extensions Pointer to the NTP extensions
 * @param[in] length Length of the NTP extensions, in bytes
 * @param[in] type Extension type
 * @param[in] index Extension occurrence index
 * @return If the specified extension is found, a pointer to the corresponding
 *   extension is returned. Otherwise NULL pointer is returned
 **/

const NtpExtension *ntpGetExtension(const uint8_t *extensions, size_t length,
   uint16_t type, uint_t index)
{
   uint_t k;
   size_t i;
   size_t n;
   const NtpExtension *extension;

   //Initialize occurrence index
   k = 0;

   //Parse extension fields
   for(i = 0; i < length; i += n)
   {
      //Malformed extension?
      if(length < sizeof(NtpExtension))
         break;

      //Point to the current extension
      extension = (NtpExtension *) (extensions + i);
      //Retrieve the length of the extension
      n = ntohs(extension->length);

      //Malformed extension?
      if(n < sizeof(NtpExtension) || (i + n) > length)
         break;

      //Matching extension type?
      if(ntohs(extension->fieldType) == type)
      {
         //Matching occurrence found?
         if(k++ == index)
         {
            return extension;
         }
      }
   }

   //The specified extension type was not found
   return NULL;
}

#endif
