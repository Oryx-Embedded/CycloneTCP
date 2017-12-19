/**
 * @file net.c
 * @brief TCP/IP stack core
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

//Switch to the appropriate trace level
#define TRACE_LEVEL ETH_TRACE_LEVEL

//Dependencies
#include <stdlib.h>
#include "core/net.h"
#include "core/socket.h"
#include "core/tcp_timer.h"
#include "core/ethernet.h"
#include "ipv4/arp.h"
#include "ipv4/ipv4.h"
#include "ipv4/ipv4_routing.h"
#include "ipv4/igmp.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_routing.h"
#include "ipv6/mld.h"
#include "ipv6/ndp.h"
#include "ipv6/ndp_router_adv.h"
#include "dhcp/dhcp_client.h"
#include "dhcp/dhcp_server.h"
#include "dns/dns_cache.h"
#include "dns/dns_client.h"
#include "mdns/mdns_client.h"
#include "mdns/mdns_responder.h"
#include "mdns/mdns_common.h"
#include "dns_sd/dns_sd.h"
#include "netbios/nbns_client.h"
#include "netbios/nbns_responder.h"
#include "netbios/nbns_common.h"
#include "dns_sd/dns_sd.h"
#include "mibs/mib2_module.h"
#include "str.h"
#include "debug.h"

#if (WEB_SOCKET_SUPPORT == ENABLED)
   #include "web_socket/web_socket.h"
#endif

//TCP/IP stack handle
OsTask *netTaskHandle;
//Mutex preventing simultaneous access to the TCP/IP stack
OsMutex netMutex;
//Event object to receive notifications from device drivers
OsEvent netEvent;
//Network interfaces
NetInterface netInterface[NET_INTERFACE_COUNT];

//TCP/IP process state
static bool_t netTaskRunning;
//Timestamp
static systime_t netTimestamp;
//Pseudo-random number generator state
static uint32_t prngState = 0;

//Mutex to prevent simultaneous access to the callback table
static OsMutex callbackTableMutex;
//Table that holds the registered user callbacks
static LinkChangeCallbackDesc callbackTable[NET_CALLBACK_TABLE_SIZE];

//Check TCP/IP stack configuration
#if (NET_STATIC_OS_RESOURCES == ENABLED)

//Task responsible for handling TCP/IP events
static OsTask netTaskInstance;
static uint_t netTaskStack[NET_TASK_STACK_SIZE];

#endif


/**
 * @brief TCP/IP stack initialization
 * @return Error code
 **/

error_t netInit(void)
{
   error_t error;
   uint_t i;
   NetInterface *interface;

   //The TCP/IP process is currently suspended
   netTaskRunning = FALSE;
   //Get current time
   netTimestamp = osGetSystemTime();

   //Create a mutex to prevent simultaneous access to the TCP/IP stack
   if(!osCreateMutex(&netMutex))
   {
      //Failed to create mutex
      return ERROR_OUT_OF_RESOURCES;
   }

   //Create a event object to receive notifications from device drivers
   if(!osCreateEvent(&netEvent))
   {
      //Failed to create mutex
      return ERROR_OUT_OF_RESOURCES;
   }

   //Memory pool initialization
   error = memPoolInit();
   //Any error to report?
   if(error)
      return error;

   //Clear configuration data for each interface
   memset(netInterface, 0, sizeof(netInterface));

   //Loop through network interfaces
   for(i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Point to the current interface
      interface = &netInterface[i];

      //Default interface name
      sprintf(interface->name, "eth%u", i);

      //Zero-based index
      interface->index = i;
      //Unique number identifying the interface
      interface->id = i;
      //Default PHY address
      interface->phyAddr = UINT8_MAX;
   }

   //Create a mutex to prevent simultaneous access to the callback table
   if(!osCreateMutex(&callbackTableMutex))
   {
      //Failed to create mutex
      return ERROR_OUT_OF_RESOURCES;
   }

   //Initialize callback table
   memset(callbackTable, 0, sizeof(callbackTable));

   //Socket related initialization
   error = socketInit();
   //Any error to report?
   if(error)
      return error;

#if (WEB_SOCKET_SUPPORT == ENABLED)
   //WebSocket related initialization
   webSocketInit();
#endif

#if (IPV4_SUPPORT == ENABLED && IPV4_ROUTING_SUPPORT == ENABLED)
   //Initialize IPv4 routing table
   error = ipv4InitRouting();
   //Any error to report?
   if(error)
      return error;
#endif

#if (IPV6_SUPPORT == ENABLED && IPV6_ROUTING_SUPPORT == ENABLED)
   //Initialize IPv6 routing table
   error = ipv6InitRouting();
   //Any error to report?
   if(error)
      return error;
#endif

#if (UDP_SUPPORT == ENABLED)
   //UDP related initialization
   error = udpInit();
   //Any error to report?
   if(error)
      return error;
#endif

#if (TCP_SUPPORT == ENABLED)
   //TCP related initialization
   error = tcpInit();
   //Any error to report?
   if(error)
      return error;
#endif

#if (DNS_CLIENT_SUPPORT == ENABLED || MDNS_CLIENT_SUPPORT == ENABLED || \
   NBNS_CLIENT_SUPPORT == ENABLED)
   //DNS cache initialization
   error = dnsInit();
   //Any error to report?
   if(error)
      return error;
#endif

   //Initialize tick counters
   nicTickCounter = 0;

#if (PPP_SUPPORT == ENABLED)
   pppTickCounter = 0;
#endif
#if (IPV4_SUPPORT == ENABLED && ETH_SUPPORT == ENABLED)
   arpTickCounter = 0;
#endif
#if (IPV4_SUPPORT == ENABLED && IPV4_FRAG_SUPPORT == ENABLED)
   ipv4FragTickCounter = 0;
#endif
#if (IPV4_SUPPORT == ENABLED && IGMP_SUPPORT == ENABLED)
   igmpTickCounter = 0;
#endif
#if (IPV4_SUPPORT == ENABLED && AUTO_IP_SUPPORT == ENABLED)
   autoIpTickCounter = 0;
#endif
#if (IPV4_SUPPORT == ENABLED && DHCP_CLIENT_SUPPORT == ENABLED)
   dhcpClientTickCounter = 0;
#endif
#if (IPV4_SUPPORT == ENABLED && DHCP_SERVER_SUPPORT == ENABLED)
   dhcpServerTickCounter = 0;
#endif
#if (IPV6_SUPPORT == ENABLED && IPV6_FRAG_SUPPORT == ENABLED)
   ipv6FragTickCounter = 0;
#endif
#if (IPV6_SUPPORT == ENABLED && MLD_SUPPORT == ENABLED)
   mldTickCounter = 0;
#endif
#if (IPV6_SUPPORT == ENABLED && NDP_SUPPORT == ENABLED)
   ndpTickCounter = 0;
#endif
#if (IPV6_SUPPORT == ENABLED && NDP_ROUTER_ADV_SUPPORT == ENABLED)
   ndpRouterAdvTickCounter = 0;
#endif
#if (IPV6_SUPPORT == ENABLED && DHCPV6_CLIENT_SUPPORT == ENABLED)
   dhcpv6ClientTickCounter = 0;
#endif
#if (TCP_SUPPORT == ENABLED)
   tcpTickCounter = 0;
#endif
#if (DNS_CLIENT_SUPPORT == ENABLED || MDNS_CLIENT_SUPPORT == ENABLED || \
   NBNS_CLIENT_SUPPORT == ENABLED)
   dnsTickCounter = 0;
#endif
#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   mdnsResponderTickCounter = 0;
#endif
#if (DNS_SD_SUPPORT == ENABLED)
   dnsSdTickCounter = 0;
#endif

#if (NET_STATIC_OS_RESOURCES == ENABLED)
   //Create a task to handle TCP/IP events
   osCreateStaticTask(&netTaskInstance, "TCP/IP Stack", (OsTaskCode) netTask,
      NULL, netTaskStack, NET_TASK_STACK_SIZE, NET_TASK_PRIORITY);
#else
   //Create a task to handle TCP/IP events
   netTaskHandle = osCreateTask("TCP/IP Stack", (OsTaskCode) netTask,
      NULL, NET_TASK_STACK_SIZE, NET_TASK_PRIORITY);

   //Unable to create the task?
   if(netTaskHandle == OS_INVALID_HANDLE)
      return ERROR_OUT_OF_RESOURCES;
#endif

#if (NET_RTOS_SUPPORT == DISABLED)
   //The TCP/IP process is now running
   netTaskRunning = TRUE;
#endif

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Set MAC address
 * @param[in] interface Pointer to the desired network interface
 * @param[in] macAddr MAC address
 * @return Error code
 **/

error_t netSetMacAddr(NetInterface *interface, const MacAddr *macAddr)
{
   //Check parameters
   if(interface == NULL || macAddr == NULL)
      return ERROR_INVALID_PARAMETER;

#if (ETH_SUPPORT == ENABLED)
   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Set MAC address
   interface->macAddr = *macAddr;

   //Generate the 64-bit interface identifier
   macAddrToEui64(macAddr, &interface->eui64);

   //Release exclusive access
   osReleaseMutex(&netMutex);
#endif

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve MAC address
 * @param[in] interface Pointer to the desired network interface
 * @param[out] macAddr MAC address
 * @return Error code
 **/

error_t netGetMacAddr(NetInterface *interface, MacAddr *macAddr)
{
   //Check parameters
   if(interface == NULL || macAddr == NULL)
      return ERROR_INVALID_PARAMETER;

#if (ETH_SUPPORT == ENABLED)
   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Get MAC address
   *macAddr = interface->macAddr;
   //Release exclusive access
   osReleaseMutex(&netMutex);
#endif

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set EUI-64 interface identifier
 * @param[in] interface Pointer to the desired network interface
 * @param[in] eui64 Interface identifier
 * @return Error code
 **/

error_t netSetEui64(NetInterface *interface, const Eui64 *eui64)
{
   //Check parameters
   if(interface == NULL || eui64 == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set interface identifier
   interface->eui64 = *eui64;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve EUI-64 interface identifier
 * @param[in] interface Pointer to the desired network interface
 * @param[out] eui64 Interface identifier
 * @return Error code
 **/

error_t netGetEui64(NetInterface *interface, Eui64 *eui64)
{
   //Check parameters
   if(interface == NULL || eui64 == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Get interface identifier
   *eui64 = interface->eui64;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set interface identifier
 * @param[in] interface Pointer to the desired network interface
 * @param[in] id Unique number identifying the interface
 * @return Error code
 **/

error_t netSetInterfaceId(NetInterface *interface, uint32_t id)
{
   //Check parameters
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set interface identifier
   interface->id = id;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set interface name
 * @param[in] interface Pointer to the desired network interface
 * @param[in] name NULL-terminated string that contains the interface name
 * @return Error code
 **/

error_t netSetInterfaceName(NetInterface *interface, const char_t *name)
{
   //Check parameters
   if(interface == NULL || name == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Set interface name
   strSafeCopy(interface->name, name, NET_MAX_IF_NAME_LEN);

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set host name
 * @param[in] interface Pointer to the desired network interface
 * @param[in] name NULL-terminated string that contains the host name
 * @return Error code
 **/

error_t netSetHostname(NetInterface *interface, const char_t *name)
{
   //Check parameters
   if(interface == NULL || name == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Set host name
   strSafeCopy(interface->hostname, name, NET_MAX_HOSTNAME_LEN);

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set proxy server
 * @param[in] interface Pointer to the desired network interface
 * @param[in] name Proxy server name
 * @param[in] port Proxy server port
 * @return Error code
 **/

error_t netSetProxy(NetInterface *interface, const char_t *name, uint16_t port)
{
   //Check parameters
   if(interface == NULL || name == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Set proxy server name
   strSafeCopy(interface->proxyName, name, NET_MAX_PROXY_NAME_LEN);
   //Set proxy server port
   interface->proxyPort = port;

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set Ethernet MAC driver
 * @param[in] interface Pointer to the desired network interface
 * @param[in] driver Ethernet MAC driver
 * @return Error code
 **/

error_t netSetDriver(NetInterface *interface, const NicDriver *driver)
{
   //Check parameters
   if(interface == NULL || driver == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set Ethernet MAC driver
   interface->nicDriver = driver;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set Ethernet PHY driver
 * @param[in] interface Pointer to the desired network interface
 * @param[in] driver Ethernet PHY driver (can be NULL for MAC + PHY controller)
 * @return Error code
 **/

error_t netSetPhyDriver(NetInterface *interface, const PhyDriver *driver)
{
   //Check parameters
   if(interface == NULL || driver == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set Ethernet PHY driver
   interface->phyDriver = driver;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set Ethernet PHY address
 * @param[in] interface Pointer to the desired network interface
 * @param[in] phyAddr PHY address
 * @return Error code
 **/

error_t netSetPhyAddr(NetInterface *interface, uint8_t phyAddr)
{
   //Make sure the network interface is valid
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Make sure the PHY address is valid
   if(phyAddr >= 32)
      return ERROR_OUT_OF_RANGE;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set PHY address
   interface->phyAddr = phyAddr;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set SPI driver
 * @param[in] interface Pointer to the desired network interface
 * @param[in] driver Underlying SPI driver
 * @return Error code
 **/

error_t netSetSpiDriver(NetInterface *interface, const SpiDriver *driver)
{
   //Check parameters
   if(interface == NULL || driver == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set SPI driver
   interface->spiDriver = driver;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set UART driver
 * @param[in] interface Pointer to the desired network interface
 * @param[in] driver Underlying UART driver
 * @return Error code
 **/

error_t netSetUartDriver(NetInterface *interface, const UartDriver *driver)
{
   //Check parameters
   if(interface == NULL || driver == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set UART driver
   interface->uartDriver = driver;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set external interrupt line driver
 * @param[in] interface Pointer to the desired network interface
 * @param[in] driver Underlying SPI driver
 * @return Error code
 **/

error_t netSetExtIntDriver(NetInterface *interface, const ExtIntDriver *driver)
{
   //Check parameters
   if(interface == NULL || driver == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Set external interrupt line driver
   interface->extIntDriver = driver;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set link state (for virtual drivers only)
 * @param[in] interface Pointer to the desired network interface
 * @param[in] linkState Link state
 * @return Error code
 **/

error_t netSetLinkState(NetInterface *interface, NicLinkState linkState)
{
   //Make sure the network interface is valid
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Link state changed?
   if(linkState != interface->linkState)
   {
      //Update link state
      interface->linkState = linkState;
      //Process link state change event
      nicNotifyLinkChange(interface);
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Get link state
 * @param[in] interface Pointer to the desired network interface
 * @return Link state
 **/

bool_t netGetLinkState(NetInterface *interface)
{
   bool_t linkState;

   //Make sure the network interface is valid
   if(interface == NULL)
      return FALSE;

   //Get exclusive access
   osAcquireMutex(&netMutex);
   //Retrieve link state
   linkState = interface->linkState;
   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return link state
   return linkState;
}


/**
 * @brief Configure network interface
 * @param[in] interface Network interface to configure
 * @return Error code
 **/

error_t netConfigInterface(NetInterface *interface)
{
   error_t error;

   //Make sure the network interface is valid
   if(interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Get exclusive access
   osAcquireMutex(&netMutex);

   //Disable hardware interrupts
   interface->nicDriver->disableIrq(interface);

   //Start of exception handling block
   do
   {
      //Receive notifications when the transmitter is ready to send
      if(!osCreateEvent(&interface->nicTxEvent))
      {
         //Failed to create event object
         error = ERROR_OUT_OF_RESOURCES;
         //Stop immediately
         break;
      }

      //Network controller initialization
      error = interface->nicDriver->init(interface);
      //Any error to report?
      if(error)
         break;

#if (ETH_SUPPORT == ENABLED)
      //Ethernet related initialization
      error = ethInit(interface);
      //Any error to report?
      if(error)
         break;
#endif

#if (IPV4_SUPPORT == ENABLED)
      //IPv4 initialization
      error = ipv4Init(interface);
      //Any error to report?
      if(error)
         break;

#if (ETH_SUPPORT == ENABLED)
      //ARP cache initialization
      error = arpInit(interface);
      //Any error to report?
      if(error)
         break;
#endif

#if (IGMP_SUPPORT == ENABLED)
      //IGMP related initialization
      error = igmpInit(interface);
      //Any error to report?
      if(error)
         break;

      //Join the all-systems group
      error = ipv4JoinMulticastGroup(interface, IGMP_ALL_SYSTEMS_ADDR);
      //Any error to report?
      if(error)
         break;
#endif

#if (NBNS_CLIENT_SUPPORT == ENABLED || NBNS_RESPONDER_SUPPORT == ENABLED)
      //NetBIOS Name Service related initialization
      error = nbnsInit(interface);
      //Any error to report?
      if(error)
         break;
#endif
#endif

#if (IPV6_SUPPORT == ENABLED)
      //IPv6 initialization
      error = ipv6Init(interface);
      //Any error to report?
      if(error)
         break;

#if (NDP_SUPPORT == ENABLED)
      //NDP related initialization
      error = ndpInit(interface);
      //Any error to report?
      if(error)
         break;
#endif

#if (MLD_SUPPORT == ENABLED)
      //MLD related initialization
      error = mldInit(interface);
      //Any error to report?
      if(error)
         break;
#endif

      //Join the All-Nodes multicast address
      error = ipv6JoinMulticastGroup(interface, &IPV6_LINK_LOCAL_ALL_NODES_ADDR);
      //Any error to report?
      if(error)
         break;
#endif

#if (MDNS_CLIENT_SUPPORT == ENABLED || MDNS_RESPONDER_SUPPORT == ENABLED)
      //mDNS related initialization
      error = mdnsInit(interface);
      //Any error to report?
      if(error)
         break;
#endif

      //End of exception handling block
   } while(0);

   //Check status code
   if(!error)
   {
      //The network interface is now fully configured
      interface->configured = TRUE;

      //Check whether the TCP/IP process is running
      if(netTaskRunning)
      {
         //Interrupts can be safely enabled
         interface->nicDriver->enableIrq(interface);
      }
   }
   else
   {
      //Clean up side effects before returning
      osDeleteEvent(&interface->nicTxEvent);
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Return status code
   return error;
}


/**
 * @brief TCP/IP events handling
 **/

void netTask(void)
{
   uint_t i;
   bool_t status;
   systime_t time;
   systime_t timeout;
   NetInterface *interface;

#if (NET_RTOS_SUPPORT == ENABLED)
   //Get exclusive access
   osAcquireMutex(&netMutex);

   //The TCP/IP process is now running
   netTaskRunning = TRUE;

   //Loop through network interfaces
   for(i = 0; i < NET_INTERFACE_COUNT; i++)
   {
      //Point to the current network interface
      interface = &netInterface[i];

      //Check whether the interface is fully configured
      if(interface->configured)
      {
         //Interrupts can be safely enabled
         interface->nicDriver->enableIrq(interface);
      }
   }

   //Release exclusive access
   osReleaseMutex(&netMutex);

   //Main loop
   while(1)
   {
#endif
      //Get current time
      time = osGetSystemTime();

      //Compute the maximum blocking time when waiting for an event
      if(timeCompare(time, netTimestamp) < 0)
         timeout = netTimestamp - time;
      else
         timeout = 0;

      //Receive notifications when a frame has been received, or the
      //link state of any network interfaces has changed
      status = osWaitForEvent(&netEvent, timeout);

      //Check whether the specified event is in signaled state
      if(status)
      {
         //Get exclusive access
         osAcquireMutex(&netMutex);

         //Process events
         for(i = 0; i < NET_INTERFACE_COUNT; i++)
         {
            //Point to the current network interface
            interface = &netInterface[i];

            //Check whether a NIC event is pending
            if(interface->nicEvent)
            {
               //Acknowledge the event by clearing the flag
               interface->nicEvent = FALSE;

               //Disable hardware interrupts
               interface->nicDriver->disableIrq(interface);
               //Handle NIC events
               interface->nicDriver->eventHandler(interface);
               //Re-enable hardware interrupts
               interface->nicDriver->enableIrq(interface);
            }

            //Check whether a PHY event is pending
            if(interface->phyEvent)
            {
               //Acknowledge the event by clearing the flag
               interface->phyEvent = FALSE;

               //Disable hardware interrupts
               interface->nicDriver->disableIrq(interface);
               //Handle PHY events
               interface->phyDriver->eventHandler(interface);
               //Re-enable hardware interrupts
               interface->nicDriver->enableIrq(interface);
            }
         }

         //Release exclusive access
         osReleaseMutex(&netMutex);
      }

      //Check current time
      if(timeCompare(time, netTimestamp) > 0)
      {
         //Get exclusive access
         osAcquireMutex(&netMutex);
         //Handle periodic operations
         netTick();
         //Release exclusive access
         osReleaseMutex(&netMutex);

         //Next event
         netTimestamp = time + NET_TICK_INTERVAL;
      }
#if (NET_RTOS_SUPPORT == ENABLED)
   }
#endif
}


/**
 * @brief Manage TCP/IP timers
 **/

void netTick(void)
{
   uint_t i;

   //Increment tick counter
   nicTickCounter += NET_TICK_INTERVAL;

   //Handle periodic operations such as polling the link state
   if(nicTickCounter >= NIC_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            nicTick(&netInterface[i]);
      }

      //Reset tick counter
      nicTickCounter = 0;
   }

#if (PPP_SUPPORT == ENABLED)
   //Increment tick counter
   pppTickCounter += NET_TICK_INTERVAL;

   //Manage PPP related timers
   if(pppTickCounter >= PPP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            pppTick(&netInterface[i]);
      }

      //Reset tick counter
      pppTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && ETH_SUPPORT == ENABLED)
   //Increment tick counter
   arpTickCounter += NET_TICK_INTERVAL;

   //Manage ARP cache
   if(arpTickCounter >= ARP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            arpTick(&netInterface[i]);
      }

      //Reset tick counter
      arpTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && IPV4_FRAG_SUPPORT == ENABLED)
   //Increment tick counter
   ipv4FragTickCounter += NET_TICK_INTERVAL;

   //Handle IPv4 fragment reassembly timeout
   if(ipv4FragTickCounter >= IPV4_FRAG_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            ipv4FragTick(&netInterface[i]);
      }

      //Reset tick counter
      ipv4FragTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && IGMP_SUPPORT == ENABLED)
   //Increment tick counter
   igmpTickCounter += NET_TICK_INTERVAL;

   //Handle IGMP related timers
   if(igmpTickCounter >= IGMP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            igmpTick(&netInterface[i]);
      }

      //Reset tick counter
      igmpTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && AUTO_IP_SUPPORT == ENABLED)
   //Increment tick counter
   autoIpTickCounter += NET_TICK_INTERVAL;

   //Handle Auto-IP related timers
   if(autoIpTickCounter >= AUTO_IP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         autoIpTick(netInterface[i].autoIpContext);

      //Reset tick counter
      autoIpTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && DHCP_CLIENT_SUPPORT == ENABLED)
   //Increment tick counter
   dhcpClientTickCounter += NET_TICK_INTERVAL;

   //Handle DHCP client related timers
   if(dhcpClientTickCounter >= DHCP_CLIENT_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         dhcpClientTick(netInterface[i].dhcpClientContext);

      //Reset tick counter
      dhcpClientTickCounter = 0;
   }
#endif

#if (IPV4_SUPPORT == ENABLED && DHCP_SERVER_SUPPORT == ENABLED)
   //Increment tick counter
   dhcpServerTickCounter += NET_TICK_INTERVAL;

   //Handle DHCP server related timers
   if(dhcpServerTickCounter >= DHCP_SERVER_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         dhcpServerTick(netInterface[i].dhcpServerContext);

      //Reset tick counter
      dhcpServerTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && IPV6_FRAG_SUPPORT == ENABLED)
   //Increment tick counter
   ipv6FragTickCounter += NET_TICK_INTERVAL;

   //Handle IPv6 fragment reassembly timeout
   if(ipv6FragTickCounter >= IPV6_FRAG_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            ipv6FragTick(&netInterface[i]);
      }

      //Reset tick counter
      ipv6FragTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && MLD_SUPPORT == ENABLED)
   //Increment tick counter
   mldTickCounter += NET_TICK_INTERVAL;

   //Handle MLD related timers
   if(mldTickCounter >= MLD_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            mldTick(&netInterface[i]);
      }

      //Reset tick counter
      mldTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && NDP_SUPPORT == ENABLED)
   //Increment tick counter
   ndpTickCounter += NET_TICK_INTERVAL;

   //Handle NDP related timers
   if(ndpTickCounter >= NDP_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Make sure the interface has been properly configured
         if(netInterface[i].configured)
            ndpTick(&netInterface[i]);
      }

      //Reset tick counter
      ndpTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && NDP_ROUTER_ADV_SUPPORT == ENABLED)
   //Increment tick counter
   ndpRouterAdvTickCounter += NET_TICK_INTERVAL;

   //Handle RA service related timers
   if(ndpRouterAdvTickCounter >= NDP_ROUTER_ADV_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         ndpRouterAdvTick(netInterface[i].ndpRouterAdvContext);

      //Reset tick counter
      ndpRouterAdvTickCounter = 0;
   }
#endif

#if (IPV6_SUPPORT == ENABLED && DHCPV6_CLIENT_SUPPORT == ENABLED)
   //Increment tick counter
   dhcpv6ClientTickCounter += NET_TICK_INTERVAL;

   //Handle DHCPv6 client related timers
   if(dhcpv6ClientTickCounter >= DHCPV6_CLIENT_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         dhcpv6ClientTick(netInterface[i].dhcpv6ClientContext);

      //Reset tick counter
      dhcpv6ClientTickCounter = 0;
   }
#endif

#if (TCP_SUPPORT == ENABLED)
   //Increment tick counter
   tcpTickCounter += NET_TICK_INTERVAL;

   //Manage TCP related timers
   if(tcpTickCounter >= TCP_TICK_INTERVAL)
   {
      //TCP timer handler
      tcpTick();
      //Reset tick counter
      tcpTickCounter = 0;
   }
#endif

#if (DNS_CLIENT_SUPPORT == ENABLED || MDNS_CLIENT_SUPPORT == ENABLED || \
   NBNS_CLIENT_SUPPORT == ENABLED)
   //Increment tick counter
   dnsTickCounter += NET_TICK_INTERVAL;

   //Manage DNS cache
   if(dnsTickCounter >= DNS_TICK_INTERVAL)
   {
      //DNS timer handler
      dnsTick();
      //Reset tick counter
      dnsTickCounter = 0;
   }
#endif

#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   //Increment tick counter
   mdnsResponderTickCounter += NET_TICK_INTERVAL;

   //Manage mDNS probing and announcing
   if(mdnsResponderTickCounter >= MDNS_RESPONDER_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         mdnsResponderTick(netInterface[i].mdnsResponderContext);

      //Reset tick counter
      mdnsResponderTickCounter = 0;
   }
#endif

#if (DNS_SD_SUPPORT == ENABLED)
   //Increment tick counter
   dnsSdTickCounter += NET_TICK_INTERVAL;

   //Manage DNS-SD probing and announcing
   if(dnsSdTickCounter >= DNS_SD_TICK_INTERVAL)
   {
      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
         dnsSdTick(netInterface[i].dnsSdContext);

      //Reset tick counter
      dnsSdTickCounter = 0;
   }
#endif
}


/**
 * @brief Get default network interface
 * @return Pointer to the default network interface to be used
 **/

NetInterface *netGetDefaultInterface(void)
{
   //Default network interface
   return &netInterface[0];
}


/**
 * @brief Seed pseudo-random number generator
 * @param[in] seed An integer value to be used as seed by the pseudo-random number generator
 * @return Error code
 **/

error_t netInitRand(uint32_t seed)
{
   //Seed the pseudo-random number generator
   prngState += seed;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Get a random value
 * @return Random value
 **/

uint32_t netGetRand(void)
{
   uint32_t value;

   //Use a linear congruential generator (LCG) to update the state of the PRNG
   prngState *= 1103515245;
   prngState += 12345;
   value = (prngState >> 16) & 0x07FF;

   prngState *= 1103515245;
   prngState += 12345;
   value <<= 10;
   value |= (prngState >> 16) & 0x03FF;

   prngState *= 1103515245;
   prngState += 12345;
   value <<= 10;
   value |= (prngState >> 16) & 0x03FF;

   //Return the random value
   return value;
}


/**
 * @brief Get a random value in the specified range
 * @param[in] min Lower bound
 * @param[in] max Upper bound
 * @return Random value in the specified range
 **/

int32_t netGetRandRange(int32_t min, int32_t max)
{
   int32_t value;

   //Valid parameters?
   if(max > min)
   {
      //Pick up a random value in the given range
      value = min + (netGetRand() % (max - min + 1));
   }
   else
   {
      //Use default value
      value = min;
   }

   //Return the random value
   return value;
}


/**
 * @brief Register link change callback
 * @param[in] interface Underlying network interface
 * @param[in] callback Callback function to be called when the link state changed
 * @param[in] params Callback function parameter (optional)
 * @param[out] cookie Identifier that can be used to unregister the callback function
 * @return Error code
 **/

error_t netAttachLinkChangeCallback(NetInterface *interface,
   LinkChangeCallback callback, void *params, uint_t *cookie)
{
   uint_t i;
   LinkChangeCallbackDesc *entry;

   //Acquire exclusive access to the callback table
   osAcquireMutex(&callbackTableMutex);

   //Loop through the table
   for(i = 0; i < NET_CALLBACK_TABLE_SIZE; i++)
   {
      //Point to the current entry
      entry = &callbackTable[i];

      //Check whether the entry is currently in used
      if(entry->callback == NULL)
      {
         //Create a new entry
         entry->interface = interface;
         entry->callback = callback;
         entry->params = params;
         //We are done
         break;
      }
   }

   //Release exclusive access to the callback table
   osReleaseMutex(&callbackTableMutex);

   //Failed to attach the specified user callback?
   if(i >= NET_CALLBACK_TABLE_SIZE)
      return ERROR_OUT_OF_RESOURCES;

   //Return a cookie that can be used later to unregister the callback
   if(cookie != NULL)
      *cookie = i;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Unregister link change callback
 * @param[in] cookie Identifier specifying the callback to be unregistered
 * @return Error code
 **/

error_t netDetachLinkChangeCallback(uint_t cookie)
{
   //Make sure the cookie is valid
   if(cookie >= NET_CALLBACK_TABLE_SIZE)
      return ERROR_INVALID_PARAMETER;

   //Acquire exclusive access to the callback table
   osAcquireMutex(&callbackTableMutex);
   //Unregister user callback
   callbackTable[cookie].callback = NULL;
   //Release exclusive access to the callback table
   osReleaseMutex(&callbackTableMutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Invoke link change callback
 * @param[in] interface Underlying network interface
 * @param[in] linkState Link state
 **/

void netInvokeLinkChangeCallback(NetInterface *interface, bool_t linkState)
{
   uint_t i;
   LinkChangeCallbackDesc *entry;

   //Acquire exclusive access to the callback table
   osAcquireMutex(&callbackTableMutex);

   //Loop through the table
   for(i = 0; i < NET_CALLBACK_TABLE_SIZE; i++)
   {
      //Point to the current entry
      entry = &callbackTable[i];

      //Any registered callback?
      if(entry->callback != NULL)
      {
         //Check whether the network interface matches the current entry
         if(entry->interface == NULL || entry->interface == interface)
         {
            //Invoke user callback function
            entry->callback(interface, linkState, entry->params);
         }
      }
   }

   //Release exclusive access to the callback table
   osReleaseMutex(&callbackTableMutex);
}
