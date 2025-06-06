/**
 * @file bsd_socket_options.h
 * @brief BSD socket options
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

#ifndef _BSD_SOCKET_OPTIONS_H
#define _BSD_SOCKET_OPTIONS_H

//Dependencies
#include "core/net.h"
#include "core/bsd_socket.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//BSD socket related functions
int_t socketSetSoReuseAddrOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetSoBroadcastOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetSoSndTimeoOption(Socket *socket, const struct timeval *optval,
   socklen_t optlen);

int_t socketSetSoRcvTimeoOption(Socket *socket, const struct timeval *optval,
   socklen_t optlen);

int_t socketSetSoSndBufOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetSoRcvBufOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetSoKeepAliveOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetSoNoCheckOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpTosOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpTtlOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpMulticastIfOption(Socket *socket,
   const struct in_addr *optval, socklen_t optlen);

int_t socketSetIpMulticastTtlOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpMulticastLoopOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpAddMembershipOption(Socket *socket,
   const struct ip_mreq *optval, socklen_t optlen);

int_t socketSetIpDropMembershipOption(Socket *socket,
   const struct ip_mreq *optval, socklen_t optlen);

int_t socketSetIpBlockSourceOption(Socket *socket,
   const struct ip_mreq_source *optval, socklen_t optlen);

int_t socketSetIpUnblockSourceOption(Socket *socket,
   const struct ip_mreq_source *optval, socklen_t optlen);

int_t socketSetIpAddSourceMembershipOption(Socket *socket,
   const struct ip_mreq_source *optval, socklen_t optlen);

int_t socketSetIpDropSourceMembershipOption(Socket *socket,
   const struct ip_mreq_source *optval, socklen_t optlen);

int_t socketSetMcastJoinGroupOption(Socket *socket,
   const struct group_req *optval, socklen_t optlen);

int_t socketSetMcastLeaveGroupOption(Socket *socket,
   const struct group_req *optval, socklen_t optlen);

int_t socketSetMcastBlockSourceOption(Socket *socket,
   const struct group_source_req *optval, socklen_t optlen);

int_t socketSetMcastUnblockSourceOption(Socket *socket,
   const struct group_source_req *optval, socklen_t optlen);

int_t socketSetMcastJoinSourceGroupOption(Socket *socket,
   const struct group_source_req *optval, socklen_t optlen);

int_t socketSetMcastLeaveSourceGroupOption(Socket *socket,
   const struct group_source_req *optval, socklen_t optlen);

int_t socketSetIpDontFragOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpPktInfoOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpRecvTosOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpRecvTtlOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpv6TrafficClassOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpv6UnicastHopsOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpv6MulticastIfOption(Socket *socket,
   const struct in_addr *optval, socklen_t optlen);

int_t socketSetIpv6MulticastHopsOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpv6MulticastLoopOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpv6AddMembershipOption(Socket *socket,
   const struct ipv6_mreq *optval, socklen_t optlen);

int_t socketSetIpv6DropMembershipOption(Socket *socket,
   const struct ipv6_mreq *optval, socklen_t optlen);

int_t socketSetIpv6OnlyOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpv6DontFragOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpv6PktInfoOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpv6RecvTrafficClassOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetIpv6RecvHopLimitOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetTcpNoDelayOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetTcpMaxSegOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetTcpKeepIdleOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetTcpKeepIntvlOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketSetTcpKeepCntOption(Socket *socket, const int_t *optval,
   socklen_t optlen);

int_t socketGetSoReuseAddrOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetSoTypeOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetSoErrorOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetSoBroadcastOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetSoSndTimeoOption(Socket *socket, struct timeval *optval,
   socklen_t *optlen);

int_t socketGetSoRcvTimeoOption(Socket *socket, struct timeval *optval,
   socklen_t *optlen);

int_t socketGetSoSndBufOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetSoRcvBufOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetSoKeepAliveOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetSoNoCheckOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpTosOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpTtlOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpMulticastTtlOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpMulticastLoopOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpDontFragOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpPktInfoOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpRecvTosOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpRecvTtlOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpv6TrafficClassOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpv6UnicastHopsOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpv6MulticastHopsOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpv6MulticastLoopOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpv6OnlyOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpv6DontFragOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpv6PktInfoOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpv6RecvTrafficClassOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetIpv6RecvHopLimitOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetTcpNoDelayOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetTcpMaxSegOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetTcpKeepIdleOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetTcpKeepIntvlOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

int_t socketGetTcpKeepCntOption(Socket *socket, int_t *optval,
   socklen_t *optlen);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
