/**
 * @file snmp_agent.h
 * @brief SNMP agent (Simple Network Management Protocol)
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
 * @version 1.7.6
 **/

#ifndef _SNMP_AGENT_H
#define _SNMP_AGENT_H

//Dependencies
#include "core/net.h"
#include "snmp/snmp_common.h"
#include "snmp/snmp_usm.h"
#include "mibs/mib_common.h"

//SNMP agent support
#ifndef SNMP_AGENT_SUPPORT
   #define SNMP_AGENT_SUPPORT DISABLED
#elif (SNMP_AGENT_SUPPORT != ENABLED && SNMP_AGENT_SUPPORT != DISABLED)
   #error SNMP_AGENT_SUPPORT parameter is not valid
#endif

//Stack size required to run the SNMP agent
#ifndef SNMP_AGENT_STACK_SIZE
   #define SNMP_AGENT_STACK_SIZE 550
#elif (SNMP_AGENT_STACK_SIZE < 1)
   #error SNMP_AGENT_STACK_SIZE parameter is not valid
#endif

//Priority at which the SNMP agent should run
#ifndef SNMP_AGENT_PRIORITY
   #define SNMP_AGENT_PRIORITY OS_TASK_PRIORITY_NORMAL
#endif

//Maximum number of users
#ifndef SNMP_AGENT_MAX_USER_COUNT
   #define SNMP_AGENT_MAX_USER_COUNT 4
#elif (SNMP_AGENT_MAX_USER_COUNT < 1)
   #error SNMP_AGENT_MAX_USER_COUNT parameter is not valid
#endif

//Maximum number of MIBs
#ifndef SNMP_AGENT_MAX_MIB_COUNT
   #define SNMP_AGENT_MAX_MIB_COUNT 4
#elif (SNMP_AGENT_MAX_MIB_COUNT < 1)
   #error SNMP_AGENT_MAX_MIB_COUNT parameter is not valid
#endif


/**
 * @brief Random data generation callback function
 **/

typedef error_t (*SnmpAgentRandCallback)(uint8_t *data, size_t length);


/**
 * @brief SNMP agent settings
 **/

typedef struct
{
   NetInterface *interface;                        ///<Network interface to configure
   SnmpVersion versionMin;                         ///<Minimum version accepted by the SNMP agent
   SnmpVersion versionMax;                         ///<Maximum version accepted by the SNMP agent
   uint16_t port;                                  ///<SNMP port number
   uint16_t trapPort;                              ///<SNMP trap port number
   SnmpAgentRandCallback randCallback;             ///<Random data generation callback function
} SnmpAgentSettings;


/**
 * @brief SNMP agent context
 **/

typedef struct
{
   SnmpAgentSettings settings;                           ///<SNMP agent settings
   OsMutex mutex;                                        ///<Mutex preventing simultaneous access to SNMP agent context
   uint8_t enterpriseOid[SNMP_MAX_OID_SIZE];             ///<Enterprise OID
   size_t enterpriseOidLen;                              ///<Length of the enterprise OID
   SnmpUserInfo userTable[SNMP_AGENT_MAX_USER_COUNT];    ///<List of users
   const MibModule *mibModule[SNMP_AGENT_MAX_MIB_COUNT]; ///<MIB modules
   uint_t mibModuleCount;                                ///<Number of MIB modules
   Socket *socket;                                       ///<Underlying socket
   IpAddr remoteIpAddr;                                  ///<IP address of the remote SNMP engine
   uint16_t remotePort;                                  ///<Source port used by the remote SNMP engine
   SnmpMessage request;                                  ///<SNMP request message
   SnmpMessage response;                                 ///<SNMP response message
   const SnmpUserInfo *user;                             ///<Security profile of current user
#if (SNMP_V3_SUPPORT == ENABLED)
   uint8_t contextEngine[SNMP_MAX_CONTEXT_ENGINE_SIZE];  ///<Context engine identifier
   size_t contextEngineLen;                              ///<Length of the context engine identifier
   char_t contextName[SNMP_MAX_CONTEXT_NAME_LEN + 1];    ///<Context name
   systime_t systemTime;                                 ///<System time
   int32_t engineBoots;                                  ///<Number of times that the SNMP engine has rebooted
   int32_t engineTime;                                   ///<SNMP engine time
   uint64_t salt;                                        ///<Integer initialized to a random value at boot time
   uint8_t privParameters[8];                            ///<Privacy parameters
#endif
} SnmpAgentContext;


/**
 * @brief Object descriptor
 **/

typedef struct
{
   uint8_t oid[SNMP_MAX_OID_SIZE];
   size_t oidLen;
} SnmpTrapObject;


//SNMP agent related functions
void snmpAgentGetDefaultSettings(SnmpAgentSettings *settings);
error_t snmpAgentInit(SnmpAgentContext *context, const SnmpAgentSettings *settings);
error_t snmpAgentStart(SnmpAgentContext *context);

error_t snmpAgentLoadMib(SnmpAgentContext *context, const MibModule *module);
error_t snmpAgentUnloadMib(SnmpAgentContext *context, const MibModule *module);

error_t snmpAgentSetEngineBoots(SnmpAgentContext *context, int32_t engineBoots);
error_t snmpAgentGetEngineBoots(SnmpAgentContext *context, int32_t *engineBoots);

error_t snmpAgentSetEnterpriseOid(SnmpAgentContext *context,
   const uint8_t *enterpriseOid, size_t enterpriseOidLen);

error_t snmpAgentSetContextEngine(SnmpAgentContext *context,
   const void *contextEngine, size_t contextEngineLen);

error_t snmpAgentSetContextName(SnmpAgentContext *context,
   const char_t *contextName);

error_t snmpAgentCreateCommunity(SnmpAgentContext *context,
   const char_t *community, SnmpAccess mode);

error_t snmpAgentDeleteCommunity(SnmpAgentContext *context,
   const char_t *community);

error_t snmpAgentCreateUser(SnmpAgentContext *context,
   const char_t *username, SnmpAccess mode, SnmpKeyFormat keyFormat,
   SnmpAuthProtocol authProtocol, const void *authKey,
   SnmpPrivProtocol privProtocol, const void *privKey);

error_t snmpAgentDeleteUser(SnmpAgentContext *context, const char_t *username);

error_t snmpAgentSendTrap(SnmpAgentContext *context, const IpAddr *destIpAddr,
   SnmpVersion version, const char_t *username, uint_t genericTrapType,
   uint_t specificTrapCode, const SnmpTrapObject *objectList, uint_t objectListSize);

void snmpAgentTask(SnmpAgentContext *context);

#endif
