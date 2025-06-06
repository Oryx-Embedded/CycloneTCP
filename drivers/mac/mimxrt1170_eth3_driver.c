/**
 * @file mimxrt1170_eth3_driver.c
 * @brief NXP i.MX RT1170 Gigabit Ethernet MAC driver (ENET_QOS instance)
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
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include "fsl_device_registers.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "core/net.h"
#include "drivers/mac/mimxrt1170_eth3_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
#pragma location = MIMXRT1170_ETH3_RAM_SECTION
static uint8_t txBuffer[MIMXRT1170_ETH3_TX_BUFFER_COUNT][MIMXRT1170_ETH3_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
#pragma location = MIMXRT1170_ETH3_RAM_SECTION
static uint8_t rxBuffer[MIMXRT1170_ETH3_RX_BUFFER_COUNT][MIMXRT1170_ETH3_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 8
#pragma location = MIMXRT1170_ETH3_RAM_SECTION
static Mimxrt1170Eth3TxDmaDesc txDmaDesc[MIMXRT1170_ETH3_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 8
#pragma location = MIMXRT1170_ETH3_RAM_SECTION
static Mimxrt1170Eth3RxDmaDesc rxDmaDesc[MIMXRT1170_ETH3_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[MIMXRT1170_ETH3_TX_BUFFER_COUNT][MIMXRT1170_ETH3_TX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(MIMXRT1170_ETH3_RAM_SECTION)));
//Receive buffer
static uint8_t rxBuffer[MIMXRT1170_ETH3_RX_BUFFER_COUNT][MIMXRT1170_ETH3_RX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(MIMXRT1170_ETH3_RAM_SECTION)));
//Transmit DMA descriptors
static Mimxrt1170Eth3TxDmaDesc txDmaDesc[MIMXRT1170_ETH3_TX_BUFFER_COUNT]
   __attribute__((aligned(8), __section__(MIMXRT1170_ETH3_RAM_SECTION)));
//Receive DMA descriptors
static Mimxrt1170Eth3RxDmaDesc rxDmaDesc[MIMXRT1170_ETH3_RX_BUFFER_COUNT]
   __attribute__((aligned(8), __section__(MIMXRT1170_ETH3_RAM_SECTION)));

#endif

//Current transmit descriptor
static uint_t txIndex;
//Current receive descriptor
static uint_t rxIndex;


/**
 * @brief i.MX RT1170 Ethernet MAC driver (ENET_QOS instance)
 **/

const NicDriver mimxrt1170Eth3Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   mimxrt1170Eth3Init,
   mimxrt1170Eth3Tick,
   mimxrt1170Eth3EnableIrq,
   mimxrt1170Eth3DisableIrq,
   mimxrt1170Eth3EventHandler,
   mimxrt1170Eth3SendPacket,
   mimxrt1170Eth3UpdateMacAddrFilter,
   mimxrt1170Eth3UpdateMacConfig,
   mimxrt1170Eth3WritePhyReg,
   mimxrt1170Eth3ReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief i.MX RT1170 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mimxrt1170Eth3Init(NetInterface *interface)
{
   error_t error;
   uint32_t temp;

   //Debug message
   TRACE_INFO("Initializing i.MX RT1170 Ethernet MAC (ENET_QOS)...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable ENET_QOS peripheral clock
   CLOCK_EnableClock(kCLOCK_Enet_Qos);

   //GPIO configuration
   mimxrt1170Eth3InitGpio(interface);

   //Perform a software reset
   ENET_QOS->DMA_MODE |= ENET_QOS_DMA_MODE_SWR_MASK;
   //Wait for the reset to complete
   while((ENET_QOS->DMA_MODE & ENET_QOS_DMA_MODE_SWR_MASK) != 0)
   {
   }

   //Adjust MDC clock range depending on CSR frequency
   ENET_QOS->MAC_MDIO_ADDRESS = ENET_QOS_MAC_MDIO_ADDRESS_CR(7);

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

   //Use default MAC configuration
   ENET_QOS->MAC_CONFIGURATION = ENET_QOS_MAC_CONFIGURATION_GPSLCE_MASK |
      ENET_QOS_MAC_CONFIGURATION_DO_MASK;

   //Set the maximum packet size that can be accepted
   temp = ENET_QOS->MAC_EXT_CONFIGURATION & ~ENET_QOS_MAC_EXT_CONFIGURATION_GPSL_MASK;
   ENET_QOS->MAC_EXT_CONFIGURATION = temp | MIMXRT1170_ETH3_RX_BUFFER_SIZE;

   //Configure MAC address filtering
   mimxrt1170Eth3UpdateMacAddrFilter(interface);

   //Disable flow control
   ENET_QOS->MAC_TX_FLOW_CTRL_Q[0] = 0;
   ENET_QOS->MAC_RX_FLOW_CTRL = 0;

   //Enable the first RX queue
   ENET_QOS->MAC_RXQ_CTRL[0] = ENET_QOS_MAC_RXQ_CTRL_RXQ0EN(2);

   //Configure DMA operating mode
   ENET_QOS->DMA_MODE = ENET_QOS_DMA_MODE_INTM(0) | ENET_QOS_DMA_MODE_DSPW(0);
   //Configure system bus mode
   ENET_QOS->DMA_SYSBUS_MODE |= ENET_QOS_DMA_SYSBUS_MODE_AAL_MASK;

   //The DMA takes the descriptor table as contiguous
   ENET_QOS->DMA_CH[0].DMA_CHX_CTRL = ENET_QOS_DMA_CHX_CTRL_DSL(0);
   //Configure TX features
   ENET_QOS->DMA_CH[0].DMA_CHX_TX_CTRL = ENET_QOS_DMA_CHX_TX_CTRL_TxPBL(32);

   //Configure RX features
   ENET_QOS->DMA_CH[0].DMA_CHX_RX_CTRL = ENET_QOS_DMA_CHX_RX_CTRL_RxPBL(32) |
      ENET_QOS_DMA_CHX_RX_CTRL_RBSZ_13_y(MIMXRT1170_ETH3_RX_BUFFER_SIZE / 8);

   //Enable store and forward mode for transmission
   ENET_QOS->MTL_QUEUE[0].MTL_TXQX_OP_MODE |= ENET_QOS_MTL_TXQX_OP_MODE_TQS(7) |
      ENET_QOS_MTL_TXQX_OP_MODE_TXQEN(2) | ENET_QOS_MTL_TXQX_OP_MODE_TSF_MASK;

   //Enable store and forward mode for reception
   ENET_QOS->MTL_QUEUE[0].MTL_RXQX_OP_MODE |= ENET_QOS_MTL_RXQX_OP_MODE_RQS(7) |
      ENET_QOS_MTL_RXQX_OP_MODE_RSF_MASK;

   //Initialize DMA descriptor lists
   mimxrt1170Eth3InitDmaDesc(interface);

   //Prevent interrupts from being generated when the statistic counters reach
   //half their maximum value
   ENET_QOS->MAC_MMC_TX_INTERRUPT_MASK = 0xFFFFFFFF;
   ENET_QOS->MAC_MMC_RX_INTERRUPT_MASK = 0xFFFFFFFF;
   ENET_QOS->MAC_MMC_IPC_RX_INTERRUPT_MASK = 0xFFFFFFFF;

   //Disable MAC interrupts
   ENET_QOS->MAC_INTERRUPT_ENABLE = 0;

   //Enable the desired DMA interrupts
   ENET_QOS->DMA_CH[0].DMA_CHX_INT_EN = ENET_QOS_DMA_CHX_INT_EN_NIE_MASK |
      ENET_QOS_DMA_CHX_INT_EN_RIE_MASK | ENET_QOS_DMA_CHX_INT_EN_TIE_MASK;

   //Set priority grouping (3 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(MIMXRT1170_ETH3_IRQ_PRIORITY_GROUPING);

   //Configure ENET_QOS interrupt priority
   NVIC_SetPriority(ENET_QOS_IRQn, NVIC_EncodePriority(MIMXRT1170_ETH3_IRQ_PRIORITY_GROUPING,
      MIMXRT1170_ETH3_IRQ_GROUP_PRIORITY, MIMXRT1170_ETH3_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   ENET_QOS->MAC_CONFIGURATION |= ENET_QOS_MAC_CONFIGURATION_TE_MASK |
      ENET_QOS_MAC_CONFIGURATION_RE_MASK;

   //Enable DMA transmission and reception
   ENET_QOS->DMA_CH[0].DMA_CHX_TX_CTRL |= ENET_QOS_DMA_CHX_TX_CTRL_ST_MASK;
   ENET_QOS->DMA_CH[0].DMA_CHX_RX_CTRL |= ENET_QOS_DMA_CHX_RX_CTRL_SR_MASK;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void mimxrt1170Eth3InitGpio(NetInterface *interface)
{
//MIMXRT1170-EVK evaluation board?
#if defined(USE_MIMXRT1170_EVK)
   uint32_t temp;
   gpio_pin_config_t pinConfig;
   clock_root_config_t rootConfig = {0};
#if 0
   clock_sys_pll1_config_t sysPll1Config = {0};

   //Initialize system PLL1
   sysPll1Config.pllDiv2En = true;
   CLOCK_InitSysPll1(&sysPll1Config);
#endif

   //Generate 125MHz root clock
   rootConfig.clockOff = false;
   rootConfig.mux = kCLOCK_ENET_QOS_ClockRoot_MuxSysPll1Div2;
   rootConfig.div = 4;
   CLOCK_SetRootClock(kCLOCK_Root_Enet_Qos, &rootConfig);

#if 0
   //Initialize PLL PFD3 (528*18/24 = 396MHz)
   CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 24);

   //Generate 198MHz bus clock
   rootConfig.clockOff = false;
   rootConfig.mux = kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3;
   rootConfig.div = 2;
   CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootConfig);
#endif

   //Select RGMII interface mode
   temp = IOMUXC_GPR->GPR6 & ~IOMUXC_GPR_GPR6_ENET_QOS_INTF_SEL_MASK;
   IOMUXC_GPR->GPR6 = temp | IOMUXC_GPR_GPR6_ENET_QOS_INTF_SEL(1);

   //ENET_QOS_TX_CLK is driven by ENET_QOS_CLK_ROOT
   IOMUXC_GPR->GPR6 |= IOMUXC_GPR_GPR6_ENET_QOS_CLKGEN_EN_MASK;
   //Enable ENET_QOS_TX_CLK output
   IOMUXC_GPR->GPR6 |= IOMUXC_GPR_GPR6_ENET_QOS_RGMII_EN_MASK;

   //Enable IOMUXC clock
   CLOCK_EnableClock(kCLOCK_Iomuxc);

   //Configure GPIO_DISP_B1_00 pin as ENET_QOS_RX_EN
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_00_ENET_QOS_RX_EN, 0);

   //Set GPIO_DISP_B1_00 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_00_ENET_QOS_RX_EN,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(2) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_01 pin as ENET_QOS_RX_CLK
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_01_ENET_QOS_RX_CLK, 0);

   //Set GPIO_DISP_B1_01 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_01_ENET_QOS_RX_CLK,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(2) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_02 pin as ENET_QOS_RX_DATA00
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_02_ENET_QOS_RX_DATA00, 0);

   //Set GPIO_DISP_B1_02 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_02_ENET_QOS_RX_DATA00,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(2) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_03 pin as ENET_QOS_RX_DATA01
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_03_ENET_QOS_RX_DATA01, 0);

   //Set GPIO_DISP_B1_03 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_03_ENET_QOS_RX_DATA01,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(2) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_04 pin as ENET_QOS_RX_DATA02
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_04_ENET_QOS_RX_DATA02, 0);

   //Set GPIO_DISP_B1_04 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_04_ENET_QOS_RX_DATA02,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(2) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_05 pin as ENET_QOS_RX_DATA03
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_05_ENET_QOS_RX_DATA03, 0);

   //Set GPIO_DISP_B1_05 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_05_ENET_QOS_RX_DATA03,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(2) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_06 pin as ENET_QOS_TX_DATA03
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_06_ENET_QOS_TX_DATA03, 0);

   //Set GPIO_DISP_B1_06 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_06_ENET_QOS_TX_DATA03,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(3) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_07 pin as ENET_QOS_TX_DATA02
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_07_ENET_QOS_TX_DATA02, 0);

   //Set GPIO_DISP_B1_07 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_07_ENET_QOS_TX_DATA02,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(3) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_08 pin as ENET_QOS_TX_DATA01
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_08_ENET_QOS_TX_DATA01, 0);

   //Set GPIO_DISP_B1_08 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_08_ENET_QOS_TX_DATA01,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(3) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_09 pin as ENET_QOS_TX_DATA00
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_09_ENET_QOS_TX_DATA00, 0);

   //Set GPIO_DISP_B1_09 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_09_ENET_QOS_TX_DATA00,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(3) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_10 pin as ENET_QOS_TX_EN
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_10_ENET_QOS_TX_EN, 0);

   //Set GPIO_DISP_B1_10 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_10_ENET_QOS_TX_EN,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(3) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B1_11 pin as ENET_QOS_TX_CLK
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_11_ENET_QOS_TX_CLK, 0);

   //Set GPIO_DISP_B1_11 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B1_11_ENET_QOS_TX_CLK,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(3) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_EMC_B2_19 pin as ENET_QOS_MDC
   IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_19_ENET_QOS_MDC, 0);

   //Set GPIO_EMC_B2_19 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_B2_19_ENET_QOS_MDC,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(3) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_EMC_B2_20 pin as ENET_QOS_MDIO
   IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_20_ENET_QOS_MDIO, 0);

   //Set GPIO_EMC_B2_20 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_B2_20_ENET_QOS_MDIO,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PULL(1) |
      IOMUXC_SW_PAD_CTL_PAD_PDRV(0));

   //Configure GPIO_DISP_B2_13 pin as GPIO11_IO14
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_13_GPIO11_IO14, 0);

   //Set GPIO_DISP_B2_13 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_13_GPIO11_IO14,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PUS(0) |
      IOMUXC_SW_PAD_CTL_PAD_PUE(0) |
      IOMUXC_SW_PAD_CTL_PAD_DSE(1) |
      IOMUXC_SW_PAD_CTL_PAD_SRE(0));

   //Configure GPIO_DISP_B2_12 pin as GPIO11_IO13
   IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_12_GPIO11_IO13, 0);

   //Set GPIO_DISP_B2_12 pad properties
   IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_12_GPIO11_IO13,
      IOMUXC_SW_PAD_CTL_PAD_DWP_LOCK(0) |
      IOMUXC_SW_PAD_CTL_PAD_DWP(0) |
      IOMUXC_SW_PAD_CTL_PAD_ODE(0) |
      IOMUXC_SW_PAD_CTL_PAD_PUS(0) |
      IOMUXC_SW_PAD_CTL_PAD_PUE(0) |
      IOMUXC_SW_PAD_CTL_PAD_DSE(1) |
      IOMUXC_SW_PAD_CTL_PAD_SRE(0));

   //Configure ENET_QOS_RST as an output
   pinConfig.direction = kGPIO_DigitalOutput;
   pinConfig.outputLogic = 0;
   pinConfig.interruptMode = kGPIO_NoIntmode;
   GPIO_PinInit(GPIO11, 14, &pinConfig);

   //Configure ENET_QOS_INT as an input
   pinConfig.direction = kGPIO_DigitalInput;
   pinConfig.outputLogic = 0;
   pinConfig.interruptMode = kGPIO_NoIntmode;
   GPIO_PinInit(GPIO11, 13, &pinConfig);

   //Reset PHY transceiver (hard reset)
   GPIO_PinWrite(GPIO11, 14, 0);
   sleep(10);
   GPIO_PinWrite(GPIO11, 14, 1);
   sleep(10);
#endif
}


/**
 * @brief Initialize buffer descriptors
 * @param[in] interface Underlying network interface
 **/

void mimxrt1170Eth3InitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < MIMXRT1170_ETH3_TX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the application
      txDmaDesc[i].tdes0 = 0;
      txDmaDesc[i].tdes1 = 0;
      txDmaDesc[i].tdes2 = 0;
      txDmaDesc[i].tdes3 = 0;
   }

   //Initialize TX descriptor index
   txIndex = 0;

   //Initialize RX DMA descriptor list
   for(i = 0; i < MIMXRT1170_ETH3_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = (uint32_t) rxBuffer[i];
      rxDmaDesc[i].rdes1 = 0;
      rxDmaDesc[i].rdes2 = 0;
      rxDmaDesc[i].rdes3 = ENET_RDES3_OWN | ENET_RDES3_IOC | ENET_RDES3_BUF1V;
   }

   //Initialize RX descriptor index
   rxIndex = 0;

   //Start location of the TX descriptor list
   ENET_QOS->DMA_CH[0].DMA_CHX_TXDESC_LIST_ADDR = (uint32_t) &txDmaDesc[0];
   //Length of the transmit descriptor ring
   ENET_QOS->DMA_CH[0].DMA_CHX_TXDESC_RING_LENGTH = MIMXRT1170_ETH3_TX_BUFFER_COUNT - 1;

   //Start location of the RX descriptor list
   ENET_QOS->DMA_CH[0].DMA_CHX_RXDESC_LIST_ADDR = (uint32_t) &rxDmaDesc[0];
   //Length of the receive descriptor ring
   ENET_QOS->DMA_CH[0].DMA_CHX_RXDESC_RING_LENGTH = MIMXRT1170_ETH3_RX_BUFFER_COUNT - 1;
}


/**
 * @brief i.MX RT1170 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void mimxrt1170Eth3Tick(NetInterface *interface)
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

void mimxrt1170Eth3EnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(ENET_QOS_IRQn);

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

void mimxrt1170Eth3DisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(ENET_QOS_IRQn);

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
 * @brief Ethernet MAC interrupt
 **/

void ENET_QOS_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = ENET_QOS->DMA_CH[0].DMA_CHX_STAT;

   //Packet transmitted?
   if((status & ENET_QOS_DMA_CHX_STAT_TI_MASK) != 0)
   {
      //Clear TI interrupt flag
      ENET_QOS->DMA_CH[0].DMA_CHX_STAT = ENET_QOS_DMA_CHX_STAT_TI_MASK;

      //Check whether the TX buffer is available for writing
      if((txDmaDesc[txIndex].tdes3 & ENET_TDES3_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & ENET_QOS_DMA_CHX_STAT_RI_MASK) != 0)
   {
      //Clear RI interrupt flag
      ENET_QOS->DMA_CH[0].DMA_CHX_STAT = ENET_QOS_DMA_CHX_STAT_RI_MASK;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   ENET_QOS->DMA_CH[0].DMA_CHX_STAT = ENET_QOS_DMA_CHX_STAT_NIS_MASK;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief i.MX RT1170 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void mimxrt1170Eth3EventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = mimxrt1170Eth3ReceivePacket(interface);

      //No more data in the receive buffer?
   } while(error != ERROR_BUFFER_EMPTY);
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

error_t mimxrt1170Eth3SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > MIMXRT1170_ETH3_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txDmaDesc[txIndex].tdes3 & ENET_TDES3_OWN) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txIndex], buffer, offset, length);

   //Set the start address of the buffer
   txDmaDesc[txIndex].tdes0 = (uint32_t) txBuffer[txIndex];
   //Write the number of bytes to send
   txDmaDesc[txIndex].tdes2 = ENET_TDES2_IOC | (length & ENET_TDES2_B1L);
   //Give the ownership of the descriptor to the DMA
   txDmaDesc[txIndex].tdes3 = ENET_TDES3_OWN | ENET_TDES3_FD | ENET_TDES3_LD;

   //Data synchronization barrier
   __DSB();

   //Clear TBU flag to resume processing
   ENET_QOS->DMA_CH[0].DMA_CHX_STAT = ENET_QOS_DMA_CHX_STAT_TBU_MASK;
   //Instruct the DMA to poll the transmit descriptor list
   ENET_QOS->DMA_CH[0].DMA_CHX_TXDESC_TAIL_PTR = 0;

   //Increment index and wrap around if necessary
   if(++txIndex >= MIMXRT1170_ETH3_TX_BUFFER_COUNT)
   {
      txIndex = 0;
   }

   //Check whether the next buffer is available for writing
   if((txDmaDesc[txIndex].tdes3 & ENET_TDES3_OWN) == 0)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
   }

   //Data successfully written
   return NO_ERROR;
}


/**
 * @brief Receive a packet
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mimxrt1170Eth3ReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_OWN) == 0)
   {
      //FD and LD flags should be set
      if((rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_FD) != 0 &&
         (rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_LD) != 0)
      {
         //Make sure no error occurred
         if((rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_ES) == 0)
         {
            //Retrieve the length of the frame
            n = rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_PL;
            //Limit the number of data to read
            n = MIN(n, MIMXRT1170_ETH3_RX_BUFFER_SIZE);

            //Additional options can be passed to the stack along with the packet
            ancillary = NET_DEFAULT_RX_ANCILLARY;

            //Pass the packet to the upper layer
            nicProcessPacket(interface, rxBuffer[rxIndex], n, &ancillary);

            //Valid packet received
            error = NO_ERROR;
         }
         else
         {
            //The received packet contains an error
            error = ERROR_INVALID_PACKET;
         }
      }
      else
      {
         //The packet is not valid
         error = ERROR_INVALID_PACKET;
      }

      //Set the start address of the buffer
      rxDmaDesc[rxIndex].rdes0 = (uint32_t) rxBuffer[rxIndex];
      //Give the ownership of the descriptor back to the DMA
      rxDmaDesc[rxIndex].rdes3 = ENET_RDES3_OWN | ENET_RDES3_IOC | ENET_RDES3_BUF1V;

      //Increment index and wrap around if necessary
      if(++rxIndex >= MIMXRT1170_ETH3_RX_BUFFER_COUNT)
      {
         rxIndex = 0;
      }
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RBU flag to resume processing
   ENET_QOS->DMA_CH[0].DMA_CHX_STAT = ENET_QOS_DMA_CHX_STAT_RBU_MASK;
   //Instruct the DMA to poll the receive descriptor list
   ENET_QOS->DMA_CH[0].DMA_CHX_RXDESC_TAIL_PTR = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mimxrt1170Eth3UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Configure the receive filter
   ENET_QOS->MAC_PACKET_FILTER = 0;

   //Set the MAC address of the station
   ENET_QOS->MAC_ADDRESS[0].LOW = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ENET_QOS->MAC_ADDRESS[0].HIGH = interface->macAddr.w[2];

   //The MAC address filter contains the list of MAC addresses to accept
   //when receiving an Ethernet frame
   for(i = 0, j = 1; i < MAC_ADDR_FILTER_SIZE && j < 64; i++)
   {
      //Point to the current entry
      entry = &interface->macAddrFilter[i];

      //Valid entry?
      if(entry->refCount > 0)
      {
         //When the AE bit is set, the entry is used for perfect filtering
         ENET_QOS->MAC_ADDRESS[j].LOW = entry->addr.w[0] | (entry->addr.w[1] << 16);
         ENET_QOS->MAC_ADDRESS[j].HIGH = entry->addr.w[2] | ENET_QOS_HIGH_AE_MASK;

         //Next entry
         j++;
      }
   }

   //Clear unused entries
   while(j < 64)
   {
      //When the AE bit is cleared, the entry is ignored
      ENET_QOS->MAC_ADDRESS[j].LOW = 0;
      ENET_QOS->MAC_ADDRESS[j].HIGH = 0;

      //Next entry
      j++;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t mimxrt1170Eth3UpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = ENET_QOS->MAC_CONFIGURATION;

   //1000BASE-T operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_1GBPS)
   {
      config &= ~ENET_QOS_MAC_CONFIGURATION_PS_MASK;
      config &= ~ENET_QOS_MAC_CONFIGURATION_FES_MASK;
   }
   //100BASE-TX operation mode?
   else if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= ENET_QOS_MAC_CONFIGURATION_PS_MASK;
      config |= ENET_QOS_MAC_CONFIGURATION_FES_MASK;
   }
   //10BASE-T operation mode?
   else
   {
      config |= ENET_QOS_MAC_CONFIGURATION_PS_MASK;
      config &= ~ENET_QOS_MAC_CONFIGURATION_FES_MASK;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= ENET_QOS_MAC_CONFIGURATION_DM_MASK;
   }
   else
   {
      config &= ~ENET_QOS_MAC_CONFIGURATION_DM_MASK;
   }

   //Update MAC configuration register
   ENET_QOS->MAC_CONFIGURATION = config;

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

void mimxrt1170Eth3WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = ENET_QOS->MAC_MDIO_ADDRESS & ENET_QOS_MAC_MDIO_ADDRESS_CR_MASK;

      //Set up a write operation
      temp |= ENET_QOS_MAC_MDIO_ADDRESS_GOC_0_MASK |
         ENET_QOS_MAC_MDIO_ADDRESS_GB_MASK;

      //PHY address
      temp |= ENET_QOS_MAC_MDIO_ADDRESS_PA(phyAddr);
      //Register address
      temp |= ENET_QOS_MAC_MDIO_ADDRESS_RDA(regAddr);

      //Data to be written in the PHY register
      ENET_QOS->MAC_MDIO_DATA = data & ENET_QOS_MAC_MDIO_DATA_GD_MASK;

      //Start a write operation
      ENET_QOS->MAC_MDIO_ADDRESS = temp;
      //Wait for the write to complete
      while((ENET_QOS->MAC_MDIO_ADDRESS & ENET_QOS_MAC_MDIO_ADDRESS_GB_MASK) != 0)
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

uint16_t mimxrt1170Eth3ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = ENET_QOS->MAC_MDIO_ADDRESS & ENET_QOS_MAC_MDIO_ADDRESS_CR_MASK;

      //Set up a read operation
      temp |= ENET_QOS_MAC_MDIO_ADDRESS_GOC_1_MASK |
         ENET_QOS_MAC_MDIO_ADDRESS_GOC_0_MASK | ENET_QOS_MAC_MDIO_ADDRESS_GB_MASK;

      //PHY address
      temp |= ENET_QOS_MAC_MDIO_ADDRESS_PA(phyAddr);
      //Register address
      temp |= ENET_QOS_MAC_MDIO_ADDRESS_RDA(regAddr);

      //Start a read operation
      ENET_QOS->MAC_MDIO_ADDRESS = temp;
      //Wait for the read to complete
      while((ENET_QOS->MAC_MDIO_ADDRESS & ENET_QOS_MAC_MDIO_ADDRESS_GB_MASK) != 0)
      {
      }

      //Get register value
      data = ENET_QOS->MAC_MDIO_DATA & ENET_QOS_MAC_MDIO_DATA_GD_MASK;
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
      data = 0;
   }

   //Return the value of the PHY register
   return data;
}
