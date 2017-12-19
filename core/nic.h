/**
 * @file nic.h
 * @brief Network interface controller abstraction layer
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

#ifndef _NIC_H
#define _NIC_H

//Dependencies
#include "core/net.h"

//Tick interval to handle NIC periodic operations
#ifndef NIC_TICK_INTERVAL
   #define NIC_TICK_INTERVAL 1000
#elif (NIC_TICK_INTERVAL < 10)
   #error NIC_TICK_INTERVAL parameter is not valid
#endif

//Maximum duration a write operation may block
#ifndef NIC_MAX_BLOCKING_TIME
   #define NIC_MAX_BLOCKING_TIME INFINITE_DELAY
#elif (NIC_MAX_BLOCKING_TIME < 0)
   #error NIC_MAX_BLOCKING_TIME parameter is not valid
#endif

//Size of the NIC driver context
#ifndef NIC_CONTEXT_SIZE
   #define NIC_CONTEXT_SIZE 16
#elif (NIC_CONTEXT_SIZE < 1)
   #error NIC_CONTEXT_SIZE parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief NIC types
 **/

typedef enum
{
   NIC_TYPE_ETHERNET = 0, ///<Ethernet interface
   NIC_TYPE_PPP      = 1, ///<PPP interface
   NIC_TYPE_6LOWPAN  = 2  ///<6LoWPAN interface
} NicType;


/**
 * @brief Link state
 **/

typedef enum
{
   NIC_LINK_STATE_DOWN = 0,
   NIC_LINK_STATE_UP   = 1,
   NIC_LINK_STATE_AUTO = 2
} NicLinkState;


/**
 * @brief Link speed
 **/

typedef enum
{
   NIC_LINK_SPEED_UNKNOWN = 0,
   NIC_LINK_SPEED_10MBPS  = 10000000,
   NIC_LINK_SPEED_100MBPS = 100000000,
   NIC_LINK_SPEED_1GBPS   = 1000000000
} NicLinkSpeed;


/**
 * @brief Duplex mode
 **/

typedef enum
{
   NIC_UNKNOWN_DUPLEX_MODE = 0,
   NIC_HALF_DUPLEX_MODE    = 1,
   NIC_FULL_DUPLEX_MODE    = 2
} NicDuplexMode;


//NIC abstraction layer
typedef error_t (*NicInit)(NetInterface *interface);
typedef void (*NicTick)(NetInterface *interface);
typedef void (*NicEnableIrq)(NetInterface *interface);
typedef void (*NicDisableIrq)(NetInterface *interface);
typedef void (*NicEventHandler)(NetInterface *interface);
typedef error_t (*NicSendPacket)(NetInterface *interface, const NetBuffer *buffer, size_t offset);
typedef error_t (*NicSetMulticastFilter)(NetInterface *interface);
typedef error_t (*NicUpdateMacConfig)(NetInterface *interface);
typedef void (*NicWritePhyReg)(uint8_t phyAddr, uint8_t regAddr, uint16_t data);
typedef uint16_t (*NicReadPhyReg)(uint8_t phyAddr, uint8_t regAddr);

//PHY abstraction layer
typedef error_t (*PhyInit)(NetInterface *interface);
typedef void (*PhyTick)(NetInterface *interface);
typedef void (*PhyEnableIrq)(NetInterface *interface);
typedef void (*PhyDisableIrq)(NetInterface *interface);
typedef void (*PhyEventHandler)(NetInterface *interface);

//SPI abstraction layer
typedef error_t (*SpiInit)(void);
typedef error_t (*SpiSetMode)(uint_t mode);
typedef error_t (*SpiSetBitrate)(uint_t bitrate);
typedef void (*SpiAssertCs)(void);
typedef void (*SpiDeassertCs)(void);
typedef uint8_t (*SpiTransfer)(uint8_t data);

//UART abstraction layer
typedef error_t (*UartInit)(void);
typedef void (*UartEnableIrq)(void);
typedef void (*UartDisableIrq)(void);
typedef void (*UartStartTx)(void);

//External interrupt line abstraction layer
typedef error_t (*ExtIntInit)(void);
typedef void (*ExtIntEnableIrq)(void);
typedef void (*ExtIntDisableIrq)(void);


/**
 * @brief NIC driver
 **/

typedef struct
{
   NicType type;
   size_t mtu;
   NicInit init;
   NicTick tick;
   NicEnableIrq enableIrq;
   NicDisableIrq disableIrq;
   NicEventHandler eventHandler;
   NicSendPacket sendPacket;
   NicSetMulticastFilter setMulticastFilter;
   NicUpdateMacConfig updateMacConfig;
   NicWritePhyReg writePhyReg;
   NicReadPhyReg readPhyReg;
   bool_t autoPadding;
   bool_t autoCrcCalc;
   bool_t autoCrcVerif;
   bool_t autoCrcStrip;
   //bool_t autoIpv4ChecksumCalc;
   //bool_t autoIpv4ChecksumVerif;
   //bool_t autoIpv6ChecksumCalc;
   //bool_t autoIpv6ChecksumVerif;
   //bool_t autoIcmpChecksumCalc;
   //bool_t autoIcmpChecksumVerif;
   //bool_t autoTcpChecksumCalc;
   //bool_t autoTcpChecksumVerif;
   //bool_t autoUdpChecksumCalc;
   //bool_t autoUdpChecksumVerif;
} NicDriver;


/**
 * @brief PHY driver
 **/

typedef struct
{
   PhyInit init;
   PhyTick tick;
   PhyEnableIrq enableIrq;
   PhyDisableIrq disableIrq;
   PhyEventHandler eventHandler;
} PhyDriver;


/**
 * @brief SPI driver
 **/

typedef struct
{
   SpiInit init;
   SpiSetMode setMode;
   SpiSetBitrate setBitrate;
   SpiAssertCs assertCs;
   SpiDeassertCs deassertCs;
   SpiTransfer transfer;
} SpiDriver;


/**
 * @brief UART driver
 **/

typedef struct
{
   UartInit init;
   UartEnableIrq enableIrq;
   UartDisableIrq disableIrq;
   UartStartTx startTx;
} UartDriver;


/**
 * @brief External interrupt line driver
 **/

typedef struct
{
   ExtIntInit init;
   ExtIntEnableIrq enableIrq;
   ExtIntDisableIrq disableIrq;
} ExtIntDriver;


//Tick counter to handle periodic operations
extern systime_t nicTickCounter;

//NIC abstraction layer
void nicTick(NetInterface *interface);

error_t nicSendPacket(NetInterface *interface, const NetBuffer *buffer, size_t offset);
error_t nicSetMulticastFilter(NetInterface *interface);

void nicProcessPacket(NetInterface *interface, void *packet, size_t length);
void nicNotifyLinkChange(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
