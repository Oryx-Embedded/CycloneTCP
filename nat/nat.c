/**
 * @file nat.c
 * @brief NAT (IP Network Address Translator)
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
 * @section Description
 *
 * Network Address Translation (NAT) is a method of mapping one IP address
 * space to another by modifying network address information in the IP header
 * of packets. Refer to the following RFCs for complete details:
 * - RFC 2663: NAT Terminology and Considerations
 * - RFC 3022: Traditional IP Network Address Translator (Traditional NAT)
 * - RFC 4787: NAT Behavioral Requirements for Unicast UDP
 * - RFC 5382: NAT Behavioral Requirements for TCP
 * - RFC 5508: NAT Behavioral Requirements for ICMP
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.2
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NAT_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "nat/nat.h"
#include "nat/nat_misc.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (NAT_SUPPORT == ENABLED)


/**
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains NAT settings
 **/

void natGetDefaultSettings(NatSettings *settings)
{
   //Public interface
   settings->publicInterface = NULL;
   //Index of the public IP address to use
   settings->publicIpAddrIndex = 0;

   //Private interfaces
   settings->numPrivateInterfaces = 0;

   //Port redirection rules
   settings->numPortFwdRules = 0;
   settings->portFwdRules = NULL;

   //NAT sessions (initiated from a private host)
   settings->numSessions = 0;
   settings->sessions = NULL;
}


/**
 * @brief NAT initialization
 * @param[in] context Pointer to the NAT context
 * @param[in] settings NAT specific settings
 * @return Error code
 **/

error_t natInit(NatContext *context, const NatSettings *settings)
{
   uint_t i;

   //Debug message
   TRACE_INFO("Initializing NAT...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;

   //Check parameters
   if(settings->publicInterface == NULL || settings->numPrivateInterfaces == 0)
      return ERROR_INVALID_PARAMETER;

   //Save public interface
   context->publicInterface = settings->publicInterface;
   //Index of the public IP address to use
   context->publicIpAddrIndex = settings->publicIpAddrIndex;

   //Save the number of private interfaces
   context->numPrivateInterfaces = settings->numPrivateInterfaces;

   //Save private interfaces
   for(i = 0; i < context->numPrivateInterfaces; i++)
   {
      context->privateInterfaces[i] = settings->privateInterfaces[i];
   }

   //Save port redirection rules
   context->numPortFwdRules = settings->numPortFwdRules;
   context->portFwdRules = settings->portFwdRules;

   //Loop through the list of port redirection rules
   for(i = 0; i < context->numPortFwdRules; i++)
   {
      //Initialize the current entry
      osMemset(&context->portFwdRules[i], 0, sizeof(NatPortFwdRule));
   }

   //NAT sessions (initiated from a private host)
   context->numSessions = settings->numSessions;
   context->sessions = settings->sessions;

   //Loop through the NAT sessions
   for(i = 0; i < context->numSessions; i++)
   {
      //Initialize the current entry
      osMemset(&context->sessions[i], 0, sizeof(NatSession));
   }

   //Attach the NAT context
   netContext.natContext = context;

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Specify the NAT public interface
 * @param[in] context Pointer to the NAT context
 * @param[in] publicInterface NAT public interface
 * @return Error code
 **/


error_t natSetPublicInterface(NatContext *context,
   NetInterface *publicInterface)
{
   uint_t i;

   //Check parameters
   if(context == NULL || publicInterface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Save public interface
   context->publicInterface = publicInterface;

   //Loop through the NAT sessions
   for(i = 0; i < context->numSessions; i++)
   {
      //Terminate session
      context->sessions[i].protocol = IPV4_PROTOCOL_NONE;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Sucessful processing
   return NO_ERROR;
}


/**
 * @brief Add port redirection rule
 * @param[in] context Pointer to the NAT context
 * @param[in] index Zero-based index identifying a given entry
 * @param[in] protocol Transport protocol (IPV4_PROTOCOL_TCP or IPV4_PROTOCOL_UDP)
 * @param[in] publicPort Public port to be redirected
 * @param[in] privateInterface Destination interface
 * @param[in] privateIpAddr Destination IP address
 * @param[in] privatePort Destination port
 * @return Error code
 **/

error_t natSetPortFwdRule(NatContext *context, uint_t index,
   Ipv4Protocol protocol, uint16_t publicPort, NetInterface *privateInterface,
   Ipv4Addr privateIpAddr, uint16_t privatePort)
{
   error_t error;

   //Add port redirection rule
   error = natSetPortRangeFwdRule(context, index, protocol, publicPort,
      publicPort, privateInterface, privateIpAddr, privatePort);

   //Return status code
   return error;
}


/**
 * @brief Add port range redirection rule
 * @param[in] context Pointer to the NAT context
 * @param[in] index Zero-based index identifying a given entry
 * @param[in] protocol Transport protocol (IPV4_PROTOCOL_TCP or IPV4_PROTOCOL_UDP)
 * @param[in] publicPortMin Public port range to be redirected (lower value)
 * @param[in] publicPortMax Public port range to be redirected (upper value)
 * @param[in] privateInterface Destination interface
 * @param[in] privateIpAddr Destination IP address
 * @param[in] privatePortMin Destination port (lower value)
 * @return Error code
 **/

error_t natSetPortRangeFwdRule(NatContext *context, uint_t index,
   Ipv4Protocol protocol, uint16_t publicPortMin, uint16_t publicPortMax,
   NetInterface *privateInterface, Ipv4Addr privateIpAddr,
   uint16_t privatePortMin)
{
   NatPortFwdRule *rule;

   //Check parameters
   if(context == NULL || privateInterface == NULL)
      return ERROR_INVALID_PARAMETER;

   //The implementation limits the number of port redirection rules
   if(index >= context->numPortFwdRules)
      return ERROR_INVALID_PARAMETER;

   //Only TCP and UDP traffic can be forwarded by the NAT
   if(protocol != IPV4_PROTOCOL_TCP && protocol != IPV4_PROTOCOL_UDP)
      return ERROR_INVALID_PROTOCOL;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Point to the specified rule
   rule = &context->portFwdRules[index];

   //Update port redirection rule
   rule->protocol = protocol;
   rule->publicPortMin = publicPortMin;
   rule->publicPortMax = publicPortMax;
   rule->privateInterface = privateInterface;
   rule->privateIpAddr = privateIpAddr;
   rule->privatePortMin = privatePortMin;
   rule->privatePortMax = privatePortMin + publicPortMax - publicPortMin;

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Sucessful processing
   return NO_ERROR;
}


/**
 * @brief Remove port redirection rule
 * @param[in] context Pointer to the NAT context
 * @param[in] index Zero-based index identifying a given entry
 * @return Error code
 **/

error_t natClearPortFwdRule(NatContext *context, uint_t index)
{
   //Make sure the NAT context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //The implementation limits the number of port redirection rules
   if(index >= context->numPortFwdRules)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Invalidate the specified rule
   context->portFwdRules[index].protocol = IPV4_PROTOCOL_NONE;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Start NAT operation
 * @param[in] context Pointer to the NAT context
 * @return Error code
 **/

error_t natStart(NatContext *context)
{
   error_t error;

   //Make sure the NAT context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Starting NAT...\r\n");

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Check the operational state of the NAT
   if(!context->running)
   {
      //Start NAT operation
      context->running = TRUE;

      //Successful processing
      error = NO_ERROR;
   }
   else
   {
      //The DHCP client is already running
      error = ERROR_ALREADY_RUNNING;
   }


   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief Stop NAT operation
 * @param[in] context Pointer to the NAT context
 * @return Error code
 **/

error_t natStop(NatContext *context)
{
   uint_t i;

   //Make sure the NAT context is valid
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Stopping NAT...\r\n");

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Check whether the NAT is running
   if(context->running)
   {
      //Loop through the NAT sessions
      for(i = 0; i < context->numSessions; i++)
      {
         //Terminate session
         context->sessions[i].protocol = IPV4_PROTOCOL_NONE;
      }

      //Stop NAT operation
      context->running = FALSE;
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Release NAT context
 * @param[in] context Pointer to the NAT context
 **/

void natDeinit(NatContext *context)
{
   //Make sure the NAT context is valid
   if(context != NULL)
   {
      //Clear NAT context
      osMemset(context, 0, sizeof(NatContext));
   }
}

#endif
