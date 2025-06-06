/**
 * @file syslog_client_misc.c
 * @brief Helper functions for Syslog client
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
#define TRACE_LEVEL SYSLOG_TRACE_LEVEL

//Dependencies
#include "syslog/syslog_client.h"
#include "syslog/syslog_client_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SYSLOG_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Format timestamp
 * @param[in] time Unix timestamp
 * @param[out] buffer Buffer where to store the formatted string
 * @return Length of the formatted string
 **/

size_t syslogClientFormatTimestamp(time_t time, char_t *buffer)
{
   size_t n;
   DateTime dateTime;

   //Abbreviated months
   static const char_t months[13][4] =
   {
      "   ",
      "Jan",
      "Feb",
      "Mar",
      "Apr",
      "May",
      "Jun",
      "Jul",
      "Aug",
      "Sep",
      "Oct",
      "Nov",
      "Dec"
   };

   //Convert Unix timestamp to date
   convertUnixTimeToDate(time, &dateTime);

   //The format of the timestamp field is Mmm dd hh:mm:ss. If the day of the
   //month is less than 10, then it must be represented as a space and then the
   //number (refer to RFC 3164, section 4.1.2)
   n = osSprintf(buffer, "%s %2" PRIu8 " %02" PRIu8 ":%02" PRIu8 ":%02" PRIu8,
      months[dateTime.month], dateTime.day, dateTime.hours, dateTime.minutes,
      dateTime.seconds);

   //Return the length of the formatted string
   return n;
}

#endif
