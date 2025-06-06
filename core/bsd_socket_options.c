/**
 * @file bsd_socket_options.c
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

//Switch to the appropriate trace level
#define TRACE_LEVEL BSD_SOCKET_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/bsd_socket.h"
#include "core/bsd_socket_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (BSD_SOCKET_SUPPORT == ENABLED)

//Dependencies
#include "core/bsd_socket_options.h"


/**
 * @brief Set SO_REUSEADDR option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetSoReuseAddrOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option specifies whether the socket can be bound to an address
      //which is already in use
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_REUSE_ADDR;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_REUSE_ADDR;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Set SO_BROADCAST option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetSoBroadcastOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies whether transmission and receipt of broadcast
      //messages are allowed
      socketEnableBroadcast(socket, *optval);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Set SO_SNDTIMEO option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetSoSndTimeoOption(Socket *socket, const struct timeval *optval,
   socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(struct timeval))
   {
      //If the specified value is of zero, I/O operations shall not time out
      if(optval->tv_sec == 0 && optval->tv_usec == 0)
      {
         socketSetTimeout(socket, INFINITE_DELAY);
      }
      else
      {
         socketSetTimeout(socket, optval->tv_sec * 1000 + optval->tv_usec / 1000);
      }

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Set SO_RCVTIMEO option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetSoRcvTimeoOption(Socket *socket, const struct timeval *optval,
   socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(struct timeval))
   {
      //If the specified value is of zero, I/O operations shall not time out
      if(optval->tv_sec == 0 && optval->tv_usec == 0)
      {
         socketSetTimeout(socket, INFINITE_DELAY);
      }
      else
      {
         socketSetTimeout(socket, optval->tv_sec * 1000 + optval->tv_usec / 1000);
      }

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Set SO_SNDBUF option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetSoSndBufOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED)
//Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Adjust the size of the send buffer
      socketSetTxBufferSize(socket, *optval);
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set SO_RCVBUF option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetSoRcvBufOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Adjust the size of the receive buffer
      socketSetRxBufferSize(socket, *optval);
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set SO_KEEPALIVE option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetSoKeepAliveOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies whether TCP keep-alive is enabled
      socketEnableKeepAlive(socket, *optval);
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP keep-alive is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set SO_NO_CHECK option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetSoNoCheckOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (UDP_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option allows UDP checksum generation to be bypassed
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_UDP_NO_CHECKSUM;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_UDP_NO_CHECKSUM;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_TOS option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpTosOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Save ToS value
      socket->tos = *optval & 0xFF;
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_TTL option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpTtlOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies the TTL value associated with an IPv4 socket
      //for unicast traffic
      socket->ttl = *optval;

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_MULTICAST_IF option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpMulticastIfOption(Socket *socket,
   const struct in_addr *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IN_ADDR))
   {
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_MULTICAST_TTL option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpMulticastTtlOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies the TTL value associated with an IPv4 socket
      //for multicast traffic
      socket->multicastTtl = *optval;

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_MULTICAST_LOOP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpMulticastLoopOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //For a socket that has joined one or more multicast groups, this option
      //controls whether it will receive a copy of outgoing packets sent to
      //those multicast groups
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV4_MULTICAST_LOOP;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV4_MULTICAST_LOOP;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_ADD_MEMBERSHIP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpAddMembershipOption(Socket *socket,
   const struct ip_mreq *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IP_MREQ))
   {
      error_t error;
      IpAddr groupAddr;

      //Copy IPv4 address
      groupAddr.length = sizeof(Ipv4Addr);
      groupAddr.ipv4Addr = optval->imr_multiaddr.s_addr;

      //Join the specified multicast group
      error = socketJoinMulticastGroup(socket, &groupAddr);

      //Check status code
      if(!error)
      {
         //Successful processing
         ret = SOCKET_SUCCESS;
      }
      else
      {
         //The multicast group cannot be joined
         socketSetErrnoCode(socket, EINVAL);
         ret = SOCKET_ERROR;
      }
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_DROP_MEMBERSHIP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpDropMembershipOption(Socket *socket,
   const struct ip_mreq *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IP_MREQ))
   {
      IpAddr groupAddr;

      //Copy group address
      groupAddr.length = sizeof(Ipv4Addr);
      groupAddr.ipv4Addr = optval->imr_multiaddr.s_addr;

      //Leave the specified multicast group
      socketLeaveMulticastGroup(socket, &groupAddr);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_BLOCK_SOURCE option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpBlockSourceOption(Socket *socket,
   const struct ip_mreq_source *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IP_MREQ_SOURCE))
   {
      error_t error;
      IpAddr groupAddr;
      IpAddr srcAddr;

      //Copy group address
      groupAddr.length = sizeof(Ipv4Addr);
      groupAddr.ipv4Addr = optval->imr_multiaddr.s_addr;

      //Copy source address
      srcAddr.length = sizeof(Ipv4Addr);
      srcAddr.ipv4Addr = optval->imr_sourceaddr.s_addr;

      //Block specific source for specific group
      error = socketBlockMulticastSource(socket, &groupAddr, &srcAddr);

      //Check status code
      if(!error)
      {
         //Successful processing
         ret = SOCKET_SUCCESS;
      }
      else
      {
         //The source address cannot be blocked
         socketSetErrnoCode(socket, EINVAL);
         ret = SOCKET_ERROR;
      }
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_UNBLOCK_SOURCE option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpUnblockSourceOption(Socket *socket,
   const struct ip_mreq_source *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IP_MREQ_SOURCE))
   {
      IpAddr groupAddr;
      IpAddr srcAddr;

      //Copy group address
      groupAddr.length = sizeof(Ipv4Addr);
      groupAddr.ipv4Addr = optval->imr_multiaddr.s_addr;

      //Copy source address
      srcAddr.length = sizeof(Ipv4Addr);
      srcAddr.ipv4Addr = optval->imr_sourceaddr.s_addr;

      //Unblock specific source for specific group
      socketUnblockMulticastSource(socket, &groupAddr, &srcAddr);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_ADD_SOURCE_MEMBERSHIP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpAddSourceMembershipOption(Socket *socket,
   const struct ip_mreq_source *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IP_MREQ_SOURCE))
   {
      error_t error;
      IpAddr groupAddr;
      IpAddr srcAddr;

      //Copy group address
      groupAddr.length = sizeof(Ipv4Addr);
      groupAddr.ipv4Addr = optval->imr_multiaddr.s_addr;

      //Copy source address
      srcAddr.length = sizeof(Ipv4Addr);
      srcAddr.ipv4Addr = optval->imr_sourceaddr.s_addr;

      //Accept specific source for specific group
      error = socketAddMulticastSource(socket, &groupAddr, &srcAddr);

      //Check status code
      if(!error)
      {
         //Successful processing
         ret = SOCKET_SUCCESS;
      }
      else
      {
         //The source address cannot be accepted
         socketSetErrnoCode(socket, EINVAL);
         ret = SOCKET_ERROR;
      }
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_DROP_SOURCE_MEMBERSHIP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpDropSourceMembershipOption(Socket *socket,
   const struct ip_mreq_source *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IP_MREQ_SOURCE))
   {
      IpAddr groupAddr;
      IpAddr srcAddr;

      //Copy group address
      groupAddr.length = sizeof(Ipv4Addr);
      groupAddr.ipv4Addr = optval->imr_multiaddr.s_addr;

      //Copy source address
      srcAddr.length = sizeof(Ipv4Addr);
      srcAddr.ipv4Addr = optval->imr_sourceaddr.s_addr;

      //Drop specific source for specific group
      socketDropMulticastSource(socket, &groupAddr, &srcAddr);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set MCAST_JOIN_GROUP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetMcastJoinGroupOption(Socket *socket,
   const struct group_req *optval, socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(GROUP_REQ))
   {
      error_t error;
      IpAddr groupAddr;

#if (IPV4_SUPPORT == ENABLED)
      //IPv4 address?
      if(optval->gr_group.ss_family == AF_INET)
      {
         //Point to the IPv4 address information
         SOCKADDR_IN *sa = (SOCKADDR_IN *) &optval->gr_group;

         //Copy group address
         groupAddr.length = sizeof(Ipv4Addr);
         groupAddr.ipv4Addr = sa->sin_addr.s_addr;
      }
      else
#endif
#if (IPV6_SUPPORT == ENABLED)
      //IPv6 address?
      if(optval->gr_group.ss_family == AF_INET6)
      {
         //Point to the IPv6 address information
         SOCKADDR_IN6 *sa = (SOCKADDR_IN6 *) &optval->gr_group;

         //Copy group address
         groupAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&groupAddr.ipv6Addr, sa->sin6_addr.s6_addr);
      }
      else
#endif
      //Invalid address?
      {
         //Report an error
         socketSetErrnoCode(socket, EINVAL);
         return SOCKET_ERROR;
      }

      //Join the specified multicast group
      error = socketJoinMulticastGroup(socket, &groupAddr);

      //Check status code
      if(!error)
      {
         //Successful processing
         ret = SOCKET_SUCCESS;
      }
      else
      {
         //The multicast group cannot be joined
         socketSetErrnoCode(socket, EINVAL);
         ret = SOCKET_ERROR;
      }
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Set MCAST_LEAVE_GROUP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetMcastLeaveGroupOption(Socket *socket,
   const struct group_req *optval, socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(GROUP_REQ))
   {
      IpAddr groupAddr;

#if (IPV4_SUPPORT == ENABLED)
      //IPv4 address?
      if(optval->gr_group.ss_family == AF_INET)
      {
         //Point to the IPv4 address information
         SOCKADDR_IN *sa = (SOCKADDR_IN *) &optval->gr_group;

         //Copy group address
         groupAddr.length = sizeof(Ipv4Addr);
         groupAddr.ipv4Addr = sa->sin_addr.s_addr;
      }
      else
#endif
#if (IPV6_SUPPORT == ENABLED)
      //IPv6 address?
      if(optval->gr_group.ss_family == AF_INET6)
      {
         //Point to the IPv6 address information
         SOCKADDR_IN6 *sa = (SOCKADDR_IN6 *) &optval->gr_group;

         //Copy group address
         groupAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&groupAddr.ipv6Addr, sa->sin6_addr.s6_addr);
      }
      else
#endif
      //Invalid address?
      {
         //Report an error
         socketSetErrnoCode(socket, EINVAL);
         return SOCKET_ERROR;
      }

      //Leave the specified multicast group
      socketLeaveMulticastGroup(socket, &groupAddr);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Set MCAST_BLOCK_SOURCE option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetMcastBlockSourceOption(Socket *socket,
   const struct group_source_req *optval, socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(GROUP_SOURCE_REQ))
   {
      error_t error;
      IpAddr groupAddr;
      IpAddr srcAddr;

#if (IPV4_SUPPORT == ENABLED)
      //IPv4 address?
      if(optval->gsr_group.ss_family == AF_INET &&
         optval->gsr_source.ss_family == AF_INET)
      {
         //Point to the IPv4 address information
         SOCKADDR_IN *sa1 = (SOCKADDR_IN *) &optval->gsr_group;
         SOCKADDR_IN *sa2 = (SOCKADDR_IN *) &optval->gsr_source;

         //Copy group address
         groupAddr.length = sizeof(Ipv4Addr);
         groupAddr.ipv4Addr = sa1->sin_addr.s_addr;

         //Copy source address
         srcAddr.length = sizeof(Ipv4Addr);
         srcAddr.ipv4Addr = sa2->sin_addr.s_addr;
      }
      else
#endif
#if (IPV6_SUPPORT == ENABLED)
      //IPv6 address?
      if(optval->gsr_group.ss_family == AF_INET6 &&
         optval->gsr_source.ss_family == AF_INET6)
      {
         //Point to the IPv6 address information
         SOCKADDR_IN6 *sa1 = (SOCKADDR_IN6 *) &optval->gsr_group;
         SOCKADDR_IN6 *sa2 = (SOCKADDR_IN6 *) &optval->gsr_source;

         //Copy group address
         groupAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&groupAddr.ipv6Addr, sa1->sin6_addr.s6_addr);

         //Copy source address
         srcAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&srcAddr.ipv6Addr, sa2->sin6_addr.s6_addr);
      }
      else
#endif
      //Invalid address?
      {
         //Report an error
         socketSetErrnoCode(socket, EINVAL);
         return SOCKET_ERROR;
      }

      //Block specific source for specific group
      error = socketBlockMulticastSource(socket, &groupAddr, &srcAddr);

      //Check status code
      if(!error)
      {
         //Successful processing
         ret = SOCKET_SUCCESS;
      }
      else
      {
         //The source address cannot be blocked
         socketSetErrnoCode(socket, EINVAL);
         ret = SOCKET_ERROR;
      }
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}

/**
 * @brief Set MCAST_UNBLOCK_SOURCE option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetMcastUnblockSourceOption(Socket *socket,
   const struct group_source_req *optval, socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(GROUP_SOURCE_REQ))
   {
      IpAddr groupAddr;
      IpAddr srcAddr;

#if (IPV4_SUPPORT == ENABLED)
      //IPv4 address?
      if(optval->gsr_group.ss_family == AF_INET &&
         optval->gsr_source.ss_family == AF_INET)
      {
         //Point to the IPv4 address information
         SOCKADDR_IN *sa1 = (SOCKADDR_IN *) &optval->gsr_group;
         SOCKADDR_IN *sa2 = (SOCKADDR_IN *) &optval->gsr_source;

         //Copy group address
         groupAddr.length = sizeof(Ipv4Addr);
         groupAddr.ipv4Addr = sa1->sin_addr.s_addr;

         //Copy source address
         srcAddr.length = sizeof(Ipv4Addr);
         srcAddr.ipv4Addr = sa2->sin_addr.s_addr;
      }
      else
#endif
#if (IPV6_SUPPORT == ENABLED)
      //IPv6 address?
      if(optval->gsr_group.ss_family == AF_INET6 &&
         optval->gsr_source.ss_family == AF_INET6)
      {
         //Point to the IPv6 address information
         SOCKADDR_IN6 *sa1 = (SOCKADDR_IN6 *) &optval->gsr_group;
         SOCKADDR_IN6 *sa2 = (SOCKADDR_IN6 *) &optval->gsr_source;

         //Copy group address
         groupAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&groupAddr.ipv6Addr, sa1->sin6_addr.s6_addr);

         //Copy source address
         srcAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&srcAddr.ipv6Addr, sa2->sin6_addr.s6_addr);
      }
      else
#endif
      //Invalid address?
      {
         //Report an error
         socketSetErrnoCode(socket, EINVAL);
         return SOCKET_ERROR;
      }

      //Unblock specific source for specific group
      socketUnblockMulticastSource(socket, &groupAddr, &srcAddr);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Set MCAST_JOIN_SOURCE_GROUP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetMcastJoinSourceGroupOption(Socket *socket,
   const struct group_source_req *optval, socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(GROUP_SOURCE_REQ))
   {
      error_t error;
      IpAddr groupAddr;
      IpAddr srcAddr;

#if (IPV4_SUPPORT == ENABLED)
      //IPv4 address?
      if(optval->gsr_group.ss_family == AF_INET &&
         optval->gsr_source.ss_family == AF_INET)
      {
         //Point to the IPv4 address information
         SOCKADDR_IN *sa1 = (SOCKADDR_IN *) &optval->gsr_group;
         SOCKADDR_IN *sa2 = (SOCKADDR_IN *) &optval->gsr_source;

         //Copy group address
         groupAddr.length = sizeof(Ipv4Addr);
         groupAddr.ipv4Addr = sa1->sin_addr.s_addr;

         //Copy source address
         srcAddr.length = sizeof(Ipv4Addr);
         srcAddr.ipv4Addr = sa2->sin_addr.s_addr;
      }
      else
#endif
#if (IPV6_SUPPORT == ENABLED)
      //IPv6 address?
      if(optval->gsr_group.ss_family == AF_INET6 &&
         optval->gsr_source.ss_family == AF_INET6)
      {
         //Point to the IPv6 address information
         SOCKADDR_IN6 *sa1 = (SOCKADDR_IN6 *) &optval->gsr_group;
         SOCKADDR_IN6 *sa2 = (SOCKADDR_IN6 *) &optval->gsr_source;

         //Copy group address
         groupAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&groupAddr.ipv6Addr, sa1->sin6_addr.s6_addr);

         //Copy source address
         srcAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&srcAddr.ipv6Addr, sa2->sin6_addr.s6_addr);
      }
      else
#endif
      //Invalid address?
      {
         //Report an error
         socketSetErrnoCode(socket, EINVAL);
         return SOCKET_ERROR;
      }

      //Accept specific source for specific group
      error = socketAddMulticastSource(socket, &groupAddr, &srcAddr);

      //Check status code
      if(!error)
      {
         //Successful processing
         ret = SOCKET_SUCCESS;
      }
      else
      {
         //The source address cannot be accepted
         socketSetErrnoCode(socket, EINVAL);
         ret = SOCKET_ERROR;
      }
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Set MCAST_LEAVE_SOURCE_GROUP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetMcastLeaveSourceGroupOption(Socket *socket,
   const struct group_source_req *optval, socklen_t optlen)
{
   int_t ret;

   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(GROUP_SOURCE_REQ))
   {
      IpAddr groupAddr;
      IpAddr srcAddr;

#if (IPV4_SUPPORT == ENABLED)
      //IPv4 address?
      if(optval->gsr_group.ss_family == AF_INET &&
         optval->gsr_source.ss_family == AF_INET)
      {
         //Point to the IPv4 address information
         SOCKADDR_IN *sa1 = (SOCKADDR_IN *) &optval->gsr_group;
         SOCKADDR_IN *sa2 = (SOCKADDR_IN *) &optval->gsr_source;

         //Copy group address
         groupAddr.length = sizeof(Ipv4Addr);
         groupAddr.ipv4Addr = sa1->sin_addr.s_addr;

         //Copy source address
         srcAddr.length = sizeof(Ipv4Addr);
         srcAddr.ipv4Addr = sa2->sin_addr.s_addr;
      }
      else
#endif
#if (IPV6_SUPPORT == ENABLED)
      //IPv6 address?
      if(optval->gsr_group.ss_family == AF_INET6 &&
         optval->gsr_source.ss_family == AF_INET6)
      {
         //Point to the IPv6 address information
         SOCKADDR_IN6 *sa1 = (SOCKADDR_IN6 *) &optval->gsr_group;
         SOCKADDR_IN6 *sa2 = (SOCKADDR_IN6 *) &optval->gsr_source;

         //Copy group address
         groupAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&groupAddr.ipv6Addr, sa1->sin6_addr.s6_addr);

         //Copy source address
         srcAddr.length = sizeof(Ipv6Addr);
         ipv6CopyAddr(&srcAddr.ipv6Addr, sa2->sin6_addr.s6_addr);
      }
      else
#endif
      //Invalid address?
      {
         //Report an error
         socketSetErrnoCode(socket, EINVAL);
         return SOCKET_ERROR;
      }

      //Drop specific source for specific group
      socketDropMulticastSource(socket, &groupAddr, &srcAddr);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Set IP_DONTFRAG option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpDontFragOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option can be used to set the "don't fragment" flag on IP packets
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV4_DONT_FRAG;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV4_DONT_FRAG;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_PKTINFO option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpPktInfoOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option allows an application to enable or disable the return of
      //packet information
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV4_PKT_INFO;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV4_PKT_INFO;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_RECVTOS option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpRecvTosOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option allows an application to enable or disable the return of
      //ToS header field on received datagrams
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV4_RECV_TOS;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV4_RECV_TOS;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IP_RECVTTL option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpRecvTtlOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option allows an application to enable or disable the return of
      //TTL header field on received datagrams
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV4_RECV_TTL;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV4_RECV_TTL;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_TCLASS option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6TrafficClassOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Save Traffic Class value
      socket->tos = *optval & 0xFF;
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_UNICAST_HOPS option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6UnicastHopsOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies the TTL value associated with an IPv6 socket
      //for unicast traffic
      socket->ttl = *optval;

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_MULTICAST_IF option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6MulticastIfOption(Socket *socket,
   const struct in_addr *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IN_ADDR))
   {
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_MULTICAST_HOPS option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6MulticastHopsOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies the TTL value associated with an IPv6 socket
      //for multicast traffic
      socket->multicastTtl = *optval;

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_MULTICAST_LOOP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6MulticastLoopOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //For a socket that has joined one or more multicast groups, this option
      //controls whether it will receive a copy of outgoing packets sent to
      //those multicast groups
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV6_MULTICAST_LOOP;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV6_MULTICAST_LOOP;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_ADD_MEMBERSHIP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6AddMembershipOption(Socket *socket,
   const struct ipv6_mreq *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IPV6_MREQ))
   {
      error_t error;
      IpAddr groupAddr;

      //Copy group address
      groupAddr.length = sizeof(Ipv6Addr);
      ipv6CopyAddr(&groupAddr.ipv6Addr, optval->ipv6mr_multiaddr.s6_addr);

      //Join the specified multicast group
      error = socketJoinMulticastGroup(socket, &groupAddr);

      //Check status code
      if(!error)
      {
         //Successful processing
         ret = SOCKET_SUCCESS;
      }
      else
      {
         //The multicast group cannot be joined
         socketSetErrnoCode(socket, EINVAL);
         ret = SOCKET_ERROR;
      }
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_DROP_MEMBERSHIP option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6DropMembershipOption(Socket *socket,
   const struct ipv6_mreq *optval, socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(IPV6_MREQ))
   {
      IpAddr groupAddr;

      //Copy group address
      groupAddr.length = sizeof(Ipv6Addr);
      ipv6CopyAddr(&groupAddr.ipv6Addr, optval->ipv6mr_multiaddr.s6_addr);

      //Leave the specified multicast group
      socketLeaveMulticastGroup(socket, &groupAddr);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_V6ONLY option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6OnlyOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option indicates if a socket created for the AF_INET6 address
      //family is restricted to IPv6 communications only
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV6_ONLY;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV6_ONLY;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_DONTFRAG option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6DontFragOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //This option defines a mechanism to turn off the automatic inserting
      //of a fragment header for UDP and raw sockets (refer to RFC 3542,
      //section 11.2)
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV6_DONT_FRAG;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV6_DONT_FRAG;
      }
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_PKTINFO option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6PktInfoOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option allows an application to enable or disable the return of
      //packet information
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV6_PKT_INFO;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV6_PKT_INFO;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_RECVTCLASS option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6RecvTrafficClassOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option allows an application to enable or disable the return of
      //Traffic Class header field on received datagrams
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV6_RECV_TRAFFIC_CLASS;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV6_RECV_TRAFFIC_CLASS;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set IPV6_RECVHOPLIMIT option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetIpv6RecvHopLimitOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //This option allows an application to enable or disable the return of
      //Hop Limit header field on received datagrams
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_IPV6_RECV_HOP_LIMIT;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_IPV6_RECV_HOP_LIMIT;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set TCP_NODELAY option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetTcpNoDelayOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //The option enables or disables the Nagle algorithm for TCP sockets
      if(*optval != 0)
      {
         socket->options |= SOCKET_OPTION_TCP_NO_DELAY;
      }
      else
      {
         socket->options &= ~SOCKET_OPTION_TCP_NO_DELAY;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set TCP_MAXSEG option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetTcpMaxSegOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Set the maximum segment size for outgoing TCP packets. If this option
      //is set before connection establishment, it also change the MSS value
      //announced to the other end in the initial SYN packet
      socketSetMaxSegmentSize(socket, *optval);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set TCP_KEEPIDLE option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetTcpKeepIdleOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Convert the time interval to milliseconds
      socket->keepAliveIdle = *optval * 1000;
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP keep-alive is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set TCP_KEEPINTVL option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetTcpKeepIntvlOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Convert the time interval to milliseconds
      socket->keepAliveInterval = *optval * 1000;
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP keep-alive is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Set TCP_KEEPCNT option
 * @param[in] socket Handle referencing the socket
 * @param[in] optval A pointer to the buffer in which the value for the
 *   requested option is specified
 * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval
 *   parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketSetTcpKeepCntOption(Socket *socket, const int_t *optval,
   socklen_t optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Check the length of the option
   if(optlen >= (socklen_t) sizeof(int_t))
   {
      //Save parameter value
      socket->keepAliveMaxProbes = *optval;
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP keep-alive is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get SO_REUSEADDR option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoReuseAddrOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies whether the socket can be bound to an address
      //which is already in use
      if((socket->options & SOCKET_OPTION_REUSE_ADDR) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Get SO_TYPE option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoTypeOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Return the type of the socket
      if(socket->type == SOCKET_TYPE_STREAM)
      {
         *optval = SOCK_STREAM;
      }
      else if(socket->type == SOCKET_TYPE_DGRAM)
      {
         *optval = SOCK_DGRAM;
      }
      else
      {
         *optval = SOCK_RAW;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Get SO_ERROR option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoErrorOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Return the error code
      *optval = socket->errnoCode;
      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Clear error status
      socket->errnoCode = 0;

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Get SO_BROADCAST option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoBroadcastOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies whether transmission and receipt of broadcast
      //messages are allowed
      if((socket->options & SOCKET_OPTION_BROADCAST) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Get SO_SNDTIMEO option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoSndTimeoOption(Socket *socket, struct timeval *optval,
   socklen_t *optlen)
{
   int_t ret;

   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(struct timeval))
   {
      //Return the timeout value
      if(socket->timeout == INFINITE_DELAY)
      {
         optval->tv_sec = 0;
         optval->tv_usec = 0;
      }
      else
      {
         optval->tv_sec = socket->timeout / 1000;
         optval->tv_usec = (socket->timeout % 1000) * 1000;
      }

      //Return the actual length of the option
      *optlen = sizeof(struct timeval);
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Get SO_RCVTIMEO option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoRcvTimeoOption(Socket *socket, struct timeval *optval,
   socklen_t *optlen)
{
   int_t ret;

   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(struct timeval))
   {
      //Return the timeout value
      if(socket->timeout == INFINITE_DELAY)
      {
         optval->tv_sec = 0;
         optval->tv_usec = 0;
      }
      else
      {
         optval->tv_sec = socket->timeout / 1000;
         optval->tv_usec = (socket->timeout % 1000) * 1000;
      }

      //Return the actual length of the option
      *optlen = sizeof(struct timeval);
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }

   //Return status code
   return ret;
}


/**
 * @brief Get SO_SNDBUF option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoSndBufOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Return the size of the send buffer
      *optval = socket->txBufferSize;
      //Return the actual length of the option
      *optlen = sizeof(int_t);
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get SO_RCVBUF option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoRcvBufOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Return the size of the receive buffer
      *optval = socket->rxBufferSize;
      //Return the actual length of the option
      *optlen = sizeof(int_t);
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get SO_KEEPALIVE option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoKeepAliveOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies whether TCP keep-alive is enabled
      *optval = socket->keepAliveEnabled;
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP keep-alive is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get SO_NO_CHECK option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetSoNoCheckOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (UDP_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option allows UDP checksum generation to be bypassed
      if((socket->options & SOCKET_OPTION_UDP_NO_CHECKSUM) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IP_TOS option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpTosOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Return ToS value
      *optval = socket->tos;
      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IP_TTL option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpTtlOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Return TTL value
      *optval = socket->ttl;
      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IP_MULTICAST_TTL option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpMulticastTtlOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Return TTL value for multicast packets
      *optval = socket->multicastTtl;
      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IP_MULTICAST_LOOP option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpMulticastLoopOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //For a socket that has joined one or more multicast groups, this option
      //controls whether it will receive a copy of outgoing packets sent to
      //those multicast groups
      if((socket->options & SOCKET_OPTION_IPV4_MULTICAST_LOOP) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IP_DONTFRAG option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpDontFragOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option can be used to set the "don't fragment" flag on IP packets
      if((socket->options & SOCKET_OPTION_IPV4_DONT_FRAG) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IP_PKTINFO option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpPktInfoOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option allows an application to enable or disable the return of
      //packet information
      if((socket->options & SOCKET_OPTION_IPV4_PKT_INFO) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IP_RECVTOS option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpRecvTosOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option allows an application to enable or disable the return of
      //ToS header field on received datagrams
      if((socket->options & SOCKET_OPTION_IPV4_RECV_TOS) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IP_RECVTTL option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpRecvTtlOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV4_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option allows an application to enable or disable the return of
      //TTL header field on received datagrams
      if((socket->options & SOCKET_OPTION_IPV4_RECV_TTL) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv4 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IPV6_TCLASS option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpv6TrafficClassOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Return Traffic Class value
      *optval = socket->tos;
      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IPV6_UNICAST_HOPS option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpv6UnicastHopsOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies the TTL value associated with an IPv6 socket
      //for unicast traffic
      *optval = socket->ttl;

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IPV6_MULTICAST_HOPS option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpv6MulticastHopsOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option specifies the TTL value associated with an IPv6 socket
      //for multicast traffic
      *optval = socket->multicastTtl;

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IPV6_MULTICAST_LOOP option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpv6MulticastLoopOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //For a socket that has joined one or more multicast groups, this option
      //controls whether it will receive a copy of outgoing packets sent to
      //those multicast groups
      if((socket->options & SOCKET_OPTION_IPV6_MULTICAST_LOOP) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}

/**
 * @brief Get IPV6_V6ONLY option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpv6OnlyOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option indicates if a socket created for the AF_INET6 address
      //family is restricted to IPv6 communications only
      if((socket->options & SOCKET_OPTION_IPV6_ONLY) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IPV6_DONTFRAG option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpv6DontFragOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option defines a mechanism to turn off the automatic inserting
      //of a fragment header for UDP and raw sockets (refer to RFC 3542,
      //section 11.2)
      if((socket->options & SOCKET_OPTION_IPV6_DONT_FRAG) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IPV6_PKTINFO option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpv6PktInfoOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option allows an application to enable or disable the return of
      //packet information
      if((socket->options & SOCKET_OPTION_IPV6_PKT_INFO) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IPV6_RECVTCLASS option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpv6RecvTrafficClassOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option allows an application to enable or disable the return of
      //Traffic Class header field on received datagrams
      if((socket->options & SOCKET_OPTION_IPV6_RECV_TRAFFIC_CLASS) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get IPV6_RECVHOPLIMIT option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetIpv6RecvHopLimitOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (IPV6_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //This option allows an application to enable or disable the return of
      //Hop Limit header field on received datagrams
      if((socket->options & SOCKET_OPTION_IPV6_RECV_HOP_LIMIT) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //IPv6 is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get TCP_NODELAY option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetTcpNoDelayOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //The option enables or disables the Nagle algorithm for TCP sockets
      if((socket->options & SOCKET_OPTION_TCP_NO_DELAY) != 0)
      {
         *optval = TRUE;
      }
      else
      {
         *optval = FALSE;
      }

      //Return the actual length of the option
      *optlen = sizeof(int_t);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get TCP_MAXSEG option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetTcpMaxSegOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);

      //Return the maximum segment size for outgoing TCP packets
      if(socket->state == TCP_STATE_CLOSED ||
         socket->state == TCP_STATE_LISTEN)
      {
         *optval = socket->mss;
      }
      else
      {
         *optval = socket->smss;
      }

      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get TCP_KEEPIDLE option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetTcpKeepIdleOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Convert the time interval to seconds
      *optval = socket->keepAliveIdle / 1000;
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP keep-alive is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif

   //Return status code
   return ret;
}


/**
 * @brief Get TCP_KEEPINTVL option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetTcpKeepIntvlOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Convert the time interval to seconds
      *optval = socket->keepAliveInterval / 1000;
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP keep-alive is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif


   //Return status code
   return ret;
}


/**
 * @brief Get TCP_KEEPCNT option
 * @param[in] socket Handle referencing the socket
 * @param[out] optval A pointer to the buffer in which the value for the
 *   requested option is to be returned
 * @param[in,out] optlen The size, in bytes, of the buffer pointed to by the
 *   optval parameter
 * @return Error code (SOCKET_SUCCESS or SOCKET_ERROR)
 **/

int_t socketGetTcpKeepCntOption(Socket *socket, int_t *optval,
   socklen_t *optlen)
{
   int_t ret;

#if (TCP_SUPPORT == ENABLED && TCP_KEEP_ALIVE_SUPPORT == ENABLED)
   //Check the length of the option
   if(*optlen >= (socklen_t) sizeof(int_t))
   {
      //Return parameter value
      *optval = socket->keepAliveMaxProbes;
      //Successful processing
      ret = SOCKET_SUCCESS;
   }
   else
   {
      //The option length is not valid
      socketSetErrnoCode(socket, EFAULT);
      ret = SOCKET_ERROR;
   }
#else
   //TCP keep-alive is not supported
   socketSetErrnoCode(socket, ENOPROTOOPT);
   ret = SOCKET_ERROR;
#endif


   //Return status code
   return ret;
}

#endif
