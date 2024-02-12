/**
 * @file pic32cz_eth_driver.c
 * @brief PIC32CZ CA80/CA90 Gigabit Ethernet MAC driver
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2024 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.4.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include <limits.h>
#include "pic32c.h"
#include "core/net.h"
#include "drivers/mac/pic32cz_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//TX buffer
#pragma data_alignment = 8
#pragma location = PIC32CZ_ETH_RAM_SECTION
static uint8_t txBuffer[PIC32CZ_ETH_TX_BUFFER_COUNT][PIC32CZ_ETH_TX_BUFFER_SIZE];
//RX buffer
#pragma data_alignment = 8
#pragma location = PIC32CZ_ETH_RAM_SECTION
static uint8_t rxBuffer[PIC32CZ_ETH_RX_BUFFER_COUNT][PIC32CZ_ETH_RX_BUFFER_SIZE];
//TX buffer descriptors
#pragma data_alignment = 8
#pragma location = PIC32CZ_ETH_RAM_SECTION
static Pic32czTxBufferDesc txBufferDesc[PIC32CZ_ETH_TX_BUFFER_COUNT];
//RX buffer descriptors
#pragma data_alignment = 8
#pragma location = PIC32CZ_ETH_RAM_SECTION
static Pic32czRxBufferDesc rxBufferDesc[PIC32CZ_ETH_RX_BUFFER_COUNT];

//Dummy TX buffer
#pragma data_alignment = 8
#pragma location = PIC32CZ_ETH_RAM_SECTION
static uint8_t dummyTxBuffer[PIC32CZ_ETH_DUMMY_BUFFER_COUNT][PIC32CZ_ETH_DUMMY_BUFFER_SIZE];
//Dummy RX buffer
#pragma data_alignment = 8
#pragma location = PIC32CZ_ETH_RAM_SECTION
static uint8_t dummyRxBuffer[PIC32CZ_ETH_DUMMY_BUFFER_COUNT][PIC32CZ_ETH_DUMMY_BUFFER_SIZE];
//Dummy TX buffer descriptors
#pragma data_alignment = 8
#pragma location = PIC32CZ_ETH_RAM_SECTION
static Pic32czTxBufferDesc dummyTxBufferDesc[PIC32CZ_ETH_DUMMY_BUFFER_COUNT];
//Dummy RX buffer descriptors
#pragma data_alignment = 8
#pragma location = PIC32CZ_ETH_RAM_SECTION
static Pic32czRxBufferDesc dummyRxBufferDesc[PIC32CZ_ETH_DUMMY_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//TX buffer
static uint8_t txBuffer[PIC32CZ_ETH_TX_BUFFER_COUNT][PIC32CZ_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(PIC32CZ_ETH_RAM_SECTION)));
//RX buffer
static uint8_t rxBuffer[PIC32CZ_ETH_RX_BUFFER_COUNT][PIC32CZ_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(PIC32CZ_ETH_RAM_SECTION)));
//TX buffer descriptors
static Pic32czTxBufferDesc txBufferDesc[PIC32CZ_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(PIC32CZ_ETH_RAM_SECTION)));
//RX buffer descriptors
static Pic32czRxBufferDesc rxBufferDesc[PIC32CZ_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(PIC32CZ_ETH_RAM_SECTION)));

//Dummy TX buffer
static uint8_t dummyTxBuffer[PIC32CZ_ETH_DUMMY_BUFFER_COUNT][PIC32CZ_ETH_DUMMY_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(PIC32CZ_ETH_RAM_SECTION)));
//Dummy RX buffer
static uint8_t dummyRxBuffer[PIC32CZ_ETH_DUMMY_BUFFER_COUNT][PIC32CZ_ETH_DUMMY_BUFFER_SIZE]
   __attribute__((aligned(8), __section__(PIC32CZ_ETH_RAM_SECTION)));
//Dummy TX buffer descriptors
static Pic32czTxBufferDesc dummyTxBufferDesc[PIC32CZ_ETH_DUMMY_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(PIC32CZ_ETH_RAM_SECTION)));
//Dummy RX buffer descriptors
static Pic32czRxBufferDesc dummyRxBufferDesc[PIC32CZ_ETH_DUMMY_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(PIC32CZ_ETH_RAM_SECTION)));

#endif

//TX buffer index
static uint_t txBufferIndex;
//RX buffer index
static uint_t rxBufferIndex;


/**
 * @brief PIC32CZ Ethernet MAC driver
 **/

const NicDriver pic32czEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   pic32czEthInit,
   pic32czEthTick,
   pic32czEthEnableIrq,
   pic32czEthDisableIrq,
   pic32czEthEventHandler,
   pic32czEthSendPacket,
   pic32czEthUpdateMacAddrFilter,
   pic32czEthUpdateMacConfig,
   pic32czEthWritePhyReg,
   pic32czEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief PIC32CZ Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t pic32czEthInit(NetInterface *interface)
{
   error_t error;
   volatile uint32_t status;

   //Debug message
   TRACE_INFO("Initializing PIC32CZ Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable CLK_GMAC_TX core clock
   GCLK_REGS->GCLK_PCHCTRL[ETH_GCLK_ID_TX] = GCLK_PCHCTRL_GEN_GCLK2 |
      GCLK_PCHCTRL_CHEN_Msk;

   //Wait for synchronization
   while((GCLK_REGS->GCLK_PCHCTRL[ETH_GCLK_ID_TX] & GCLK_PCHCTRL_CHEN_Msk) == 0)
   {
   }

   //Enable CLK_GMAC_TSU core clock
   GCLK_REGS->GCLK_PCHCTRL[ETH_GCLK_ID_TSU] = GCLK_PCHCTRL_GEN_GCLK2 |
      GCLK_PCHCTRL_CHEN_Msk;

   //Wait for synchronization
   while((GCLK_REGS->GCLK_PCHCTRL[ETH_GCLK_ID_TSU] & GCLK_PCHCTRL_CHEN_Msk) == 0)
   {
   }

   //Enable ETH bus clocks (CLK_GMAC_APB and CLK_GMAC_AXI)
   MCLK_REGS->MCLK_CLKMSK[ETH_MCLK_ID_APB / 32] |= (1U << (ETH_MCLK_ID_APB % 32));
   MCLK_REGS->MCLK_CLKMSK[ETH_MCLK_ID_AXI / 32] |= (1U << (ETH_MCLK_ID_AXI % 32));

   //Enable ETH module
   ETH_REGS->ETH_CTRLA = ETH_CTRLA_ENABLE_Msk;

   //Wait for synchronization
   while(ETH_REGS->ETH_SYNCB != 0)
   {
   }

   //Disable transmit and receive circuits
   ETH_REGS->ETH_NCR = 0;

   //GPIO configuration
   pic32czEthInitGpio(interface);

   //Configure MDC clock speed
   ETH_REGS->ETH_NCFGR = ETH_NCFGR_DBW(1) | ETH_NCFGR_CLK(6);
   //Enable management port (MDC and MDIO)
   ETH_REGS->ETH_NCR |= ETH_NCR_MPE_Msk;

   //Valid Ethernet PHY or switch driver?
   if(interface->phyDriver != NULL)
   {
      //Ethernet PHY initialization
      error = interface->phyDriver->init(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      //Ethernet switch initialization
      error = interface->switchDriver->init(interface);
   }
   else
   {
      //The interface is not properly configured
      error = ERROR_FAILURE;
   }

   //Any error to report?
   if(error)
   {
      return error;
   }

   //Set the MAC address of the station
   ETH_REGS->SA[0].ETH_SAB = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ETH_REGS->SA[0].ETH_SAT = interface->macAddr.w[2];

   //The MAC supports 3 additional addresses for unicast perfect filtering
   ETH_REGS->SA[1].ETH_SAB = 0;
   ETH_REGS->SA[2].ETH_SAB = 0;
   ETH_REGS->SA[3].ETH_SAB = 0;

   //Initialize hash table
   ETH_REGS->ETH_HRB = 0;
   ETH_REGS->ETH_HRT = 0;

   //Configure the receive filter
   ETH_REGS->ETH_NCFGR |= ETH_NCFGR_MAXFS_Msk | ETH_NCFGR_MTIHEN_Msk;

   //DMA configuration
   ETH_REGS->ETH_DCFGR = ETH_DCFGR_DRBS(PIC32CZ_ETH_RX_BUFFER_SIZE / 64) |
      ETH_DCFGR_TXPBMS_Msk | ETH_DCFGR_RXBMS(3) | ETH_DCFGR_FBLDO(4);

   ETH_REGS->ETH_RBQSZ[0] = ETH_RBQSZ_RXBUFSZ(PIC32CZ_ETH_DUMMY_BUFFER_SIZE / 64);
   ETH_REGS->ETH_RBQSZ[1] = ETH_RBQSZ_RXBUFSZ(PIC32CZ_ETH_DUMMY_BUFFER_SIZE / 64);
   ETH_REGS->ETH_RBQSZ[2] = ETH_RBQSZ_RXBUFSZ(PIC32CZ_ETH_DUMMY_BUFFER_SIZE / 64);
   ETH_REGS->ETH_RBQSZ[3] = ETH_RBQSZ_RXBUFSZ(PIC32CZ_ETH_DUMMY_BUFFER_SIZE / 64);
   ETH_REGS->ETH_RBQSZ[4] = ETH_RBQSZ_RXBUFSZ(PIC32CZ_ETH_DUMMY_BUFFER_SIZE / 64);

   //Initialize buffer descriptors
   pic32czEthInitBufferDesc(interface);

   //Clear transmit status register
   ETH_REGS->ETH_TSR = ETH_TSR_HRESP_Msk | ETH_TSR_UND_Msk |
      ETH_TSR_TXCOMP_Msk | ETH_TSR_TFC_Msk | ETH_TSR_TXGO_Msk |
      ETH_TSR_RLE_Msk | ETH_TSR_COL_Msk | ETH_TSR_UBR_Msk;

   //Clear receive status register
   ETH_REGS->ETH_RSR = ETH_RSR_HNO_Msk | ETH_RSR_RXOVR_Msk |
      ETH_RSR_REC_Msk | ETH_RSR_BNA_Msk;

   //First disable all ETH interrupts
   ETH_REGS->ETH_IDR = 0xFFFFFFFF;
   ETH_REGS->ETH_IDRQ[0] = 0xFFFFFFFF;
   ETH_REGS->ETH_IDRQ[1] = 0xFFFFFFFF;
   ETH_REGS->ETH_IDRQ[2] = 0xFFFFFFFF;
   ETH_REGS->ETH_IDRQ[3] = 0xFFFFFFFF;
   ETH_REGS->ETH_IDRQ[4] = 0xFFFFFFFF;

   //Only the desired ones are enabled
   ETH_REGS->ETH_IER = ETH_IER_HRESP_Msk | ETH_IER_ROVR_Msk |
      ETH_IER_TCOMP_Msk | ETH_IER_TFC_Msk | ETH_IER_RLEX_Msk |
      ETH_IER_TUR_Msk | ETH_IER_RXUBR_Msk | ETH_IER_RCOMP_Msk;

   //Read ETH_ISR register to clear any pending interrupt
   status = ETH_REGS->ETH_ISR;
   status = ETH_REGS->ETH_ISRQ[0];
   status = ETH_REGS->ETH_ISRQ[1];
   status = ETH_REGS->ETH_ISRQ[2];
   status = ETH_REGS->ETH_ISRQ[3];
   status = ETH_REGS->ETH_ISRQ[4];
   (void) status;

   //Set priority grouping (3 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(PIC32CZ_ETH_IRQ_PRIORITY_GROUPING);

   //Configure ETH interrupt priority
   NVIC_SetPriority(ETH_PRI_Q_0_IRQn, NVIC_EncodePriority(PIC32CZ_ETH_IRQ_PRIORITY_GROUPING,
      PIC32CZ_ETH_IRQ_GROUP_PRIORITY, PIC32CZ_ETH_IRQ_SUB_PRIORITY));

   //Enable the ETH to transmit and receive data
   ETH_REGS->ETH_NCR |= ETH_NCR_TXEN_Msk | ETH_NCR_RXEN_Msk;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void pic32czEthInitGpio(NetInterface *interface)
{
//PIC32CZ CA80/CA90 Curiosity Ultra evaluation board?
#if defined(USE_PIC32CZ_CA80_CURIOSITY_ULTRA) || \
   defined(USE_PIC32CZ_CA90_CURIOSITY_ULTRA)
   uint32_t temp;

   //Enable PORT bus clocks (CLK_PORT_APB and CLK_PORT_AHB)
   MCLK_REGS->MCLK_CLKMSK[PORT_MCLK_ID_APB / 32] |= (1U << (PORT_MCLK_ID_APB % 32));
   MCLK_REGS->MCLK_CLKMSK[PORT_MCLK_ID_AHB / 32] |= (1U << (PORT_MCLK_ID_AHB % 32));

   //Configure GTX1 (PA0)
   PORT_REGS->GROUP[0].PORT_PINCFG[0] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[0].PORT_PMUX[0] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[0].PORT_PMUX[0] = temp | PORT_PMUX_PMUXE(MUX_PA00K_ETH_TXD1);

   //Configure GTX0 (PA1)
   PORT_REGS->GROUP[0].PORT_PINCFG[1] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[0].PORT_PMUX[0] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[0].PORT_PMUX[0] = temp | PORT_PMUX_PMUXO(MUX_PA01K_ETH_TXD0);

   //Configure GTXEN (PA2)
   PORT_REGS->GROUP[0].PORT_PINCFG[2] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[0].PORT_PMUX[1] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[0].PORT_PMUX[1] = temp | PORT_PMUX_PMUXE(MUX_PA02K_ETH_TXEN);

   //Configure GMDC (PA3)
   PORT_REGS->GROUP[0].PORT_PINCFG[3] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[0].PORT_PMUX[1] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[0].PORT_PMUX[1] = temp | PORT_PMUX_PMUXO(MUX_PA03K_ETH_MDC);

   //Configure GMDIO (PA4)
   PORT_REGS->GROUP[0].PORT_PINCFG[4] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[0].PORT_PMUX[2] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[0].PORT_PMUX[2] = temp | PORT_PMUX_PMUXE(MUX_PA04K_ETH_MDIO);

   //Configure GRXDV (PA5)
   PORT_REGS->GROUP[0].PORT_PINCFG[5] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[0].PORT_PMUX[2] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[0].PORT_PMUX[2] = temp | PORT_PMUX_PMUXO(MUX_PA05K_ETH_RXDV);

   //Configure GRXER (PA6)
   PORT_REGS->GROUP[0].PORT_PINCFG[6] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[0].PORT_PMUX[3] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[0].PORT_PMUX[3] = temp | PORT_PMUX_PMUXE(MUX_PA06K_ETH_RXER);

   //Configure TXCK (PA21)
   PORT_REGS->GROUP[0].PORT_PINCFG[21] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[0].PORT_PMUX[10] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[0].PORT_PMUX[10] = temp | PORT_PMUX_PMUXO(MUX_PA21K_ETH_TX_CLK);

   //Configure GTXER (PD2)
   PORT_REGS->GROUP[3].PORT_PINCFG[2] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[1] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[1] = temp | PORT_PMUX_PMUXE(MUX_PD02K_ETH_TXER);

   //Configure GTX3 (PD3)
   PORT_REGS->GROUP[3].PORT_PINCFG[3] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[1] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[1] = temp | PORT_PMUX_PMUXO(MUX_PD03K_ETH_TXD3);

   //Configure GTX2 (PD4)
   PORT_REGS->GROUP[3].PORT_PINCFG[4] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[2] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[2] = temp | PORT_PMUX_PMUXE(MUX_PD04K_ETH_TXD2);

   //Configure GTXCK (PD5)
   PORT_REGS->GROUP[3].PORT_PINCFG[5] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[2] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[2] = temp | PORT_PMUX_PMUXO(MUX_PD05L_ETH_GTX_CLK);

   //Configure GRX3 (PD6)
   PORT_REGS->GROUP[3].PORT_PINCFG[6] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[3] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[3] = temp | PORT_PMUX_PMUXE(MUX_PD06K_ETH_RXD3);

   //Configure GRX2 (PD7)
   PORT_REGS->GROUP[3].PORT_PINCFG[7] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[3] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[3] = temp | PORT_PMUX_PMUXO(MUX_PD07K_ETH_RXD2);

   //Configure GCOL (PD8)
   PORT_REGS->GROUP[3].PORT_PINCFG[8] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[4] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[4] = temp | PORT_PMUX_PMUXE(MUX_PD08K_ETH_COL);

   //Configure GCRS (PD9)
   PORT_REGS->GROUP[3].PORT_PINCFG[9] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[4] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[4] = temp | PORT_PMUX_PMUXO(MUX_PD09K_ETH_CRS);

   //Configure GCOL (PD10)
   PORT_REGS->GROUP[3].PORT_PINCFG[10] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[5] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[5] = temp | PORT_PMUX_PMUXE(MUX_PD10K_ETH_RXD1);

   //Configure GCRS (PD11)
   PORT_REGS->GROUP[3].PORT_PINCFG[11] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[5] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[5] = temp | PORT_PMUX_PMUXO(MUX_PD11K_ETH_RXD0);

   //Configure GRXCK (PD12)
   PORT_REGS->GROUP[3].PORT_PINCFG[12] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[6] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[6] = temp | PORT_PMUX_PMUXE(MUX_PD12L_ETH_RX_CLK);

   //Configure GTX7 (PD14)
   PORT_REGS->GROUP[3].PORT_PINCFG[14] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[7] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[7] = temp | PORT_PMUX_PMUXE(MUX_PD14K_ETH_TXD7);

   //Configure GTX6 (PD15)
   PORT_REGS->GROUP[3].PORT_PINCFG[15] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[7] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[7] = temp | PORT_PMUX_PMUXO(MUX_PD15K_ETH_TXD6);

   //Configure GTX5 (PD16)
   PORT_REGS->GROUP[3].PORT_PINCFG[16] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[8] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[8] = temp | PORT_PMUX_PMUXE(MUX_PD16K_ETH_TXD5);

   //Configure GTX4 (PD17)
   PORT_REGS->GROUP[3].PORT_PINCFG[17] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[8] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[8] = temp | PORT_PMUX_PMUXO(MUX_PD17K_ETH_TXD4);

   //Configure GRX7 (PD18)
   PORT_REGS->GROUP[3].PORT_PINCFG[18] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[9] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[9] = temp | PORT_PMUX_PMUXE(MUX_PD18K_ETH_RXD7);

   //Configure GRX6 (PD19)
   PORT_REGS->GROUP[3].PORT_PINCFG[19] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[9] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[9] = temp | PORT_PMUX_PMUXO(MUX_PD19K_ETH_RXD6);

   //Configure GRX4 (PD20)
   PORT_REGS->GROUP[3].PORT_PINCFG[20] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[10] & ~PORT_PMUX_PMUXE_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[10] = temp | PORT_PMUX_PMUXE(MUX_PD20K_ETH_RXD4);

   //Configure GRX5 (PD21)
   PORT_REGS->GROUP[3].PORT_PINCFG[21] |= PORT_PINCFG_PMUXEN_Msk;
   temp = PORT_REGS->GROUP[3].PORT_PMUX[10] & ~PORT_PMUX_PMUXO_Msk;
   PORT_REGS->GROUP[3].PORT_PMUX[10] = temp | PORT_PMUX_PMUXO(MUX_PD21K_ETH_RXD5);

   //Select GMII operation mode
   ETH_REGS->ETH_CTRLB |= ETH_CTRLB_GMIIEN_Msk | ETH_CTRLB_GBITCLKREQ_Msk;

   //Wait for synchronization
   while(ETH_REGS->ETH_SYNCB != 0)
   {
   }

   //Configure CLK125_EN strapping pin
   PORT_REGS->GROUP[0].PORT_PINCFG[5] |= PORT_PINCFG_PULLEN_Msk;
   PORT_REGS->GROUP[0].PORT_OUTCLR = PORT_PA05;

   //Configure MODE3 strapping pin
   PORT_REGS->GROUP[3].PORT_PINCFG[6] |= PORT_PINCFG_PULLEN_Msk;
   PORT_REGS->GROUP[3].PORT_OUTCLR = PORT_PD06;

   //Configure MODE2 strapping pin
   PORT_REGS->GROUP[3].PORT_PINCFG[7] |= PORT_PINCFG_PULLEN_Msk;
   PORT_REGS->GROUP[3].PORT_OUTCLR = PORT_PD07;

   //Configure MODE1 strapping pin
   PORT_REGS->GROUP[3].PORT_PINCFG[10] |= PORT_PINCFG_PULLEN_Msk;
   PORT_REGS->GROUP[3].PORT_OUTCLR = PORT_PD10;

   //Configure MODE0 strapping pin
   PORT_REGS->GROUP[3].PORT_PINCFG[11] |= PORT_PINCFG_PULLEN_Msk;
   PORT_REGS->GROUP[3].PORT_OUTSET = PORT_PD11;

   //Configure PHYAD2 strapping pin
   PORT_REGS->GROUP[3].PORT_PINCFG[12] |= PORT_PINCFG_PULLEN_Msk;
   PORT_REGS->GROUP[3].PORT_OUTSET = PORT_PD12;

   //Configure PHY_RESET (PB23) as an output
   PORT_REGS->GROUP[1].PORT_DIRSET = PORT_PB23;

   //Reset PHY transceiver
   PORT_REGS->GROUP[1].PORT_OUTCLR = PORT_PB23;
   sleep(10);
   PORT_REGS->GROUP[1].PORT_OUTSET = PORT_PB23;
   sleep(10);
#endif
}


/**
 * @brief Initialize buffer descriptors
 * @param[in] interface Underlying network interface
 **/

void pic32czEthInitBufferDesc(NetInterface *interface)
{
   uint_t i;
   uint32_t address;

   //Initialize TX buffer descriptors
   for(i = 0; i < PIC32CZ_ETH_TX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current TX buffer
      address = (uint32_t) txBuffer[i];
      //Write the address to the descriptor entry
      txBufferDesc[i].address = address;
      //Initialize status field
      txBufferDesc[i].status = ETH_TX_USED;
   }

   //Mark the last descriptor entry with the wrap flag
   txBufferDesc[i - 1].status |= ETH_TX_WRAP;
   //Initialize TX buffer index
   txBufferIndex = 0;

   //Initialize RX buffer descriptors
   for(i = 0; i < PIC32CZ_ETH_RX_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current RX buffer
      address = (uint32_t) rxBuffer[i];
      //Write the address to the descriptor entry
      rxBufferDesc[i].address = address & ETH_RX_ADDRESS;
      //Clear status field
      rxBufferDesc[i].status = 0;
   }

   //Mark the last descriptor entry with the wrap flag
   rxBufferDesc[i - 1].address |= ETH_RX_WRAP;
   //Initialize RX buffer index
   rxBufferIndex = 0;

   //Initialize dummy TX buffer descriptors
   for(i = 0; i < PIC32CZ_ETH_DUMMY_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current TX buffer
      address = (uint32_t) dummyTxBuffer[i];
      //Write the address to the descriptor entry
      dummyTxBufferDesc[i].address = address;
      //Initialize status field
      dummyTxBufferDesc[i].status = ETH_TX_USED;
   }

   //Mark the last descriptor entry with the wrap flag
   dummyTxBufferDesc[i - 1].status |= ETH_TX_WRAP;

   //Initialize dummy RX buffer descriptors
   for(i = 0; i < PIC32CZ_ETH_DUMMY_BUFFER_COUNT; i++)
   {
      //Calculate the address of the current RX buffer
      address = (uint32_t) dummyRxBuffer[i];
      //Write the address to the descriptor entry
      dummyRxBufferDesc[i].address = (address & ETH_RX_ADDRESS) | ETH_RX_OWNERSHIP;
      //Clear status field
      dummyRxBufferDesc[i].status = 0;
   }

   //Mark the last descriptor entry with the wrap flag
   dummyRxBufferDesc[i - 1].address |= ETH_RX_WRAP;

   //Start location of the TX descriptor list
   ETH_REGS->ETH_TBQB = (uint32_t) txBufferDesc;
   ETH_REGS->ETH_TBPQB[0] = (uint32_t) dummyTxBufferDesc;
   ETH_REGS->ETH_TBPQB[1] = (uint32_t) dummyTxBufferDesc;
   ETH_REGS->ETH_TBPQB[2] = (uint32_t) dummyTxBufferDesc;
   ETH_REGS->ETH_TBPQB[3] = (uint32_t) dummyTxBufferDesc;
   ETH_REGS->ETH_TBPQB[4] = (uint32_t) dummyTxBufferDesc;

   //Start location of the RX descriptor list
   ETH_REGS->ETH_RBQB = (uint32_t) rxBufferDesc;
   ETH_REGS->ETH_RBPQB[0] = (uint32_t) dummyRxBufferDesc;
   ETH_REGS->ETH_RBPQB[1] = (uint32_t) dummyRxBufferDesc;
   ETH_REGS->ETH_RBPQB[2] = (uint32_t) dummyRxBufferDesc;
   ETH_REGS->ETH_RBPQB[3] = (uint32_t) dummyRxBufferDesc;
   ETH_REGS->ETH_RBPQB[4] = (uint32_t) dummyRxBufferDesc;
}


/**
 * @brief PIC32CZ Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void pic32czEthTick(NetInterface *interface)
{
   //Valid Ethernet PHY or switch driver?
   if(interface->phyDriver != NULL)
   {
      //Handle periodic operations
      interface->phyDriver->tick(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      //Handle periodic operations
      interface->switchDriver->tick(interface);
   }
   else
   {
      //Just for sanity
   }
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void pic32czEthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(ETH_PRI_Q_0_IRQn);

   //Valid Ethernet PHY or switch driver?
   if(interface->phyDriver != NULL)
   {
      //Enable Ethernet PHY interrupts
      interface->phyDriver->enableIrq(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      //Enable Ethernet switch interrupts
      interface->switchDriver->enableIrq(interface);
   }
   else
   {
      //Just for sanity
   }
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void pic32czEthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(ETH_PRI_Q_0_IRQn);

   //Valid Ethernet PHY or switch driver?
   if(interface->phyDriver != NULL)
   {
      //Disable Ethernet PHY interrupts
      interface->phyDriver->disableIrq(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      //Disable Ethernet switch interrupts
      interface->switchDriver->disableIrq(interface);
   }
   else
   {
      //Just for sanity
   }
}


/**
 * @brief PIC32CZ Ethernet MAC interrupt service routine
 **/

void ETH_PRI_Q_0_Handler(void)
{
   bool_t flag;
   volatile uint32_t isr;
   volatile uint32_t tsr;
   volatile uint32_t rsr;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Each time the software reads ETH_ISR, it has to check the contents
   //of ETH_TSR, ETH_RSR and ETH_NSR
   isr = ETH_REGS->ETH_ISR;
   tsr = ETH_REGS->ETH_TSR;
   rsr = ETH_REGS->ETH_RSR;

   //Clear interrupt flags
   ETH_REGS->ETH_ISR = isr;

   //Packet transmitted?
   if((tsr & (ETH_TSR_HRESP_Msk | ETH_TSR_UND_Msk |
      ETH_TSR_TXCOMP_Msk | ETH_TSR_TFC_Msk | ETH_TSR_TXGO_Msk |
      ETH_TSR_RLE_Msk | ETH_TSR_COL_Msk | ETH_TSR_UBR_Msk)) != 0)
   {
      //Only clear TSR flags that are currently set
      ETH_REGS->ETH_TSR = tsr;

      //Check whether the TX buffer is available for writing
      if((txBufferDesc[txBufferIndex].status & ETH_TX_USED) != 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((rsr & (ETH_RSR_HNO_Msk | ETH_RSR_RXOVR_Msk | ETH_RSR_REC_Msk |
      ETH_RSR_BNA_Msk)) != 0)
   {
      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief PIC32CZ Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void pic32czEthEventHandler(NetInterface *interface)
{
   error_t error;
   uint32_t rsr;

   //Read receive status
   rsr = ETH_REGS->ETH_RSR;

   //Packet received?
   if((rsr & (ETH_RSR_HNO_Msk | ETH_RSR_RXOVR_Msk | ETH_RSR_REC_Msk |
      ETH_RSR_BNA_Msk)) != 0)
   {
      //Only clear RSR flags that are currently set
      ETH_REGS->ETH_RSR = rsr;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = pic32czEthReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @param[in] ancillary Additional options passed to the stack along with
 *   the packet
 * @return Error code
 **/

error_t pic32czEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   static uint32_t temp[PIC32CZ_ETH_TX_BUFFER_SIZE / 4];
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > PIC32CZ_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txBufferDesc[txBufferIndex].status & ETH_TX_USED) == 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead(temp, buffer, offset, length);
   osMemcpy(txBuffer[txBufferIndex], temp, (length + 3) & ~3UL);

   //Set the necessary flags in the descriptor entry
   if(txBufferIndex < (PIC32CZ_ETH_TX_BUFFER_COUNT - 1))
   {
      //Write the status word
      txBufferDesc[txBufferIndex].status = ETH_TX_LAST |
         (length & ETH_TX_LENGTH);

      //Point to the next buffer
      txBufferIndex++;
   }
   else
   {
      //Write the status word
      txBufferDesc[txBufferIndex].status = ETH_TX_WRAP | ETH_TX_LAST |
         (length & ETH_TX_LENGTH);

      //Wrap around
      txBufferIndex = 0;
   }

   //Data synchronization barrier
   __DSB();

   //Set the TSTART bit to initiate transmission
   ETH_REGS->ETH_NCR |= ETH_NCR_TSTART_Msk;

   //Check whether the next buffer is available for writing
   if((txBufferDesc[txBufferIndex].status & ETH_TX_USED) != 0)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Receive a packet
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t pic32czEthReceivePacket(NetInterface *interface)
{
   static uint32_t temp[ETH_MAX_FRAME_SIZE / 4];
   error_t error;
   uint_t i;
   uint_t j;
   uint_t sofIndex;
   uint_t eofIndex;
   size_t n;
   size_t size;
   size_t length;

   //Initialize variables
   size = 0;
   sofIndex = UINT_MAX;
   eofIndex = UINT_MAX;

   //Search for SOF and EOF flags
   for(i = 0; i < PIC32CZ_ETH_RX_BUFFER_COUNT; i++)
   {
      //Point to the current entry
      j = rxBufferIndex + i;

      //Wrap around to the beginning of the buffer if necessary
      if(j >= PIC32CZ_ETH_RX_BUFFER_COUNT)
      {
         j -= PIC32CZ_ETH_RX_BUFFER_COUNT;
      }

      //No more entries to process?
      if((rxBufferDesc[j].address & ETH_RX_OWNERSHIP) == 0)
      {
         //Stop processing
         break;
      }

      //A valid SOF has been found?
      if((rxBufferDesc[j].status & ETH_RX_SOF) != 0)
      {
         //Save the position of the SOF
         sofIndex = i;
      }

      //A valid EOF has been found?
      if((rxBufferDesc[j].status & ETH_RX_EOF) != 0 && sofIndex != UINT_MAX)
      {
         //Save the position of the EOF
         eofIndex = i;
         //Retrieve the length of the frame
         size = rxBufferDesc[j].status & ETH_RX_LENGTH;
         //Limit the number of data to read
         size = MIN(size, ETH_MAX_FRAME_SIZE);
         //Stop processing since we have reached the end of the frame
         break;
      }
   }

   //Determine the number of entries to process
   if(eofIndex != UINT_MAX)
   {
      j = eofIndex + 1;
   }
   else if(sofIndex != UINT_MAX)
   {
      j = sofIndex;
   }
   else
   {
      j = i;
   }

   //Total number of bytes that have been copied from the receive buffer
   length = 0;

   //Process incoming frame
   for(i = 0; i < j; i++)
   {
      //Any data to copy from current buffer?
      if(eofIndex != UINT_MAX && i >= sofIndex && i <= eofIndex)
      {
         //Calculate the number of bytes to read at a time
         n = MIN(size, PIC32CZ_ETH_RX_BUFFER_SIZE);
         //Copy data from receive buffer
         osMemcpy((uint8_t *) temp + length, rxBuffer[rxBufferIndex], (n + 3) & ~3UL);
         //Update byte counters
         length += n;
         size -= n;
      }

      //Mark the current buffer as free
      rxBufferDesc[rxBufferIndex].address &= ~ETH_RX_OWNERSHIP;

      //Point to the following entry
      rxBufferIndex++;

      //Wrap around to the beginning of the buffer if necessary
      if(rxBufferIndex >= PIC32CZ_ETH_RX_BUFFER_COUNT)
      {
         rxBufferIndex = 0;
      }
   }

   //Any packet to process?
   if(length > 0)
   {
      NetRxAncillary ancillary;

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_RX_ANCILLARY;

      //Pass the packet to the upper layer
      nicProcessPacket(interface, (uint8_t *) temp, length, &ancillary);
      //Valid packet received
      error = NO_ERROR;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t pic32czEthUpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   uint_t k;
   uint8_t *p;
   uint32_t hashTable[2];
   MacAddr unicastMacAddr[3];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Set the MAC address of the station
   ETH_REGS->SA[0].ETH_SAB = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ETH_REGS->SA[0].ETH_SAT = interface->macAddr.w[2];

   //The MAC supports 3 additional addresses for unicast perfect filtering
   unicastMacAddr[0] = MAC_UNSPECIFIED_ADDR;
   unicastMacAddr[1] = MAC_UNSPECIFIED_ADDR;
   unicastMacAddr[2] = MAC_UNSPECIFIED_ADDR;

   //The hash table is used for multicast address filtering
   hashTable[0] = 0;
   hashTable[1] = 0;

   //The MAC address filter contains the list of MAC addresses to accept
   //when receiving an Ethernet frame
   for(i = 0, j = 0; i < MAC_ADDR_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->macAddrFilter[i];

      //Valid entry?
      if(entry->refCount > 0)
      {
         //Multicast address?
         if(macIsMulticastAddr(&entry->addr))
         {
            //Point to the MAC address
            p = entry->addr.b;

            //Apply the hash function
            k = (p[0] >> 6) ^ p[0];
            k ^= (p[1] >> 4) ^ (p[1] << 2);
            k ^= (p[2] >> 2) ^ (p[2] << 4);
            k ^= (p[3] >> 6) ^ p[3];
            k ^= (p[4] >> 4) ^ (p[4] << 2);
            k ^= (p[5] >> 2) ^ (p[5] << 4);

            //The hash value is reduced to a 6-bit index
            k &= 0x3F;

            //Update hash table contents
            hashTable[k / 32] |= (1 << (k % 32));
         }
         else
         {
            //Up to 3 additional MAC addresses can be specified
            if(j < 3)
            {
               //Save the unicast address
               unicastMacAddr[j] = entry->addr;
            }
            else
            {
               //Point to the MAC address
               p = entry->addr.b;

               //Apply the hash function
               k = (p[0] >> 6) ^ p[0];
               k ^= (p[1] >> 4) ^ (p[1] << 2);
               k ^= (p[2] >> 2) ^ (p[2] << 4);
               k ^= (p[3] >> 6) ^ p[3];
               k ^= (p[4] >> 4) ^ (p[4] << 2);
               k ^= (p[5] >> 2) ^ (p[5] << 4);

               //The hash value is reduced to a 6-bit index
               k &= 0x3F;

               //Update hash table contents
               hashTable[k / 32] |= (1 << (k % 32));
            }

            //Increment the number of unicast addresses
            j++;
         }
      }
   }

   //Configure the first unicast address filter
   if(j >= 1)
   {
      //The address is activated when SAT register is written
      ETH_REGS->SA[1].ETH_SAB = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
      ETH_REGS->SA[1].ETH_SAT = unicastMacAddr[0].w[2];
   }
   else
   {
      //The address is deactivated when SAB register is written
      ETH_REGS->SA[1].ETH_SAB = 0;
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      //The address is activated when SAT register is written
      ETH_REGS->SA[2].ETH_SAB = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
      ETH_REGS->SA[2].ETH_SAT = unicastMacAddr[1].w[2];
   }
   else
   {
      //The address is deactivated when SAB register is written
      ETH_REGS->SA[2].ETH_SAB = 0;
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      //The address is activated when SAT register is written
      ETH_REGS->SA[3].ETH_SAB = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
      ETH_REGS->SA[3].ETH_SAT = unicastMacAddr[2].w[2];
   }
   else
   {
      //The address is deactivated when SAB register is written
      ETH_REGS->SA[3].ETH_SAB = 0;
   }

   //The perfect MAC filter supports only 3 unicast addresses
   if(j >= 4)
   {
      ETH_REGS->ETH_NCFGR |= ETH_NCFGR_UNIHEN_Msk;
   }
   else
   {
      ETH_REGS->ETH_NCFGR &= ~ETH_NCFGR_UNIHEN_Msk;
   }

   //Configure the multicast hash table
   ETH_REGS->ETH_HRB = hashTable[0];
   ETH_REGS->ETH_HRT = hashTable[1];

   //Debug message
   TRACE_DEBUG("  HRB = %08" PRIX32 "\r\n", ETH_REGS->ETH_HRB);
   TRACE_DEBUG("  HRT = %08" PRIX32 "\r\n", ETH_REGS->ETH_HRT);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t pic32czEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read network configuration register
   config = ETH_REGS->ETH_NCFGR;

   //1000BASE-T operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_1GBPS)
   {
      config |= ETH_NCFGR_GIGE_Msk;
      config &= ~ETH_NCFGR_SPD_Msk;
   }
   //100BASE-TX operation mode?
   else if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config &= ~ETH_NCFGR_GIGE_Msk;
      config |= ETH_NCFGR_SPD_Msk;
   }
   //10BASE-T operation mode?
   else
   {
      config &= ~ETH_NCFGR_GIGE_Msk;
      config &= ~ETH_NCFGR_SPD_Msk;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= ETH_NCFGR_FD_Msk;
   }
   else
   {
      config &= ~ETH_NCFGR_FD_Msk;
   }

   //Write configuration value back to NCFGR register
   ETH_REGS->ETH_NCFGR = config;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write PHY register
 * @param[in] opcode Access type (2 bits)
 * @param[in] phyAddr PHY address (5 bits)
 * @param[in] regAddr Register address (5 bits)
 * @param[in] data Register value
 **/

void pic32czEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Set up a write operation
      temp = ETH_MAN_CLTTO_Msk | ETH_MAN_OP(1) | ETH_MAN_WTN(2);
      //PHY address
      temp |= ETH_MAN_PHYA(phyAddr);
      //Register address
      temp |= ETH_MAN_REGA(regAddr);
      //Register value
      temp |= ETH_MAN_DATA(data);

      //Start a write operation
      ETH_REGS->ETH_MAN = temp;
      //Wait for the write to complete
      while((ETH_REGS->ETH_NSR & ETH_NSR_IDLE_Msk) == 0)
      {
      }
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
   }
}


/**
 * @brief Read PHY register
 * @param[in] opcode Access type (2 bits)
 * @param[in] phyAddr PHY address (5 bits)
 * @param[in] regAddr Register address (5 bits)
 * @return Register value
 **/

uint16_t pic32czEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Set up a read operation
      temp = ETH_MAN_CLTTO_Msk | ETH_MAN_OP(2) | ETH_MAN_WTN(2);
      //PHY address
      temp |= ETH_MAN_PHYA(phyAddr);
      //Register address
      temp |= ETH_MAN_REGA(regAddr);

      //Start a read operation
      ETH_REGS->ETH_MAN = temp;
      //Wait for the read to complete
      while((ETH_REGS->ETH_NSR & ETH_NSR_IDLE_Msk) == 0)
      {
      }

      //Get register value
      data = ETH_REGS->ETH_MAN & ETH_MAN_DATA_Msk;
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
      data = 0;
   }

   //Return the value of the PHY register
   return data;
}
