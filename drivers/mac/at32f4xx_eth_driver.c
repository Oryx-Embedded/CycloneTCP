/**
 * @file at32f4xx_eth_driver.c
 * @brief Artery AT32F4 Ethernet MAC driver
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

//Device-specific definitions
#if defined(AT32F407RGT7) || defined(AT32F407VGT7) || defined(AT32F407RCT7) || \
   defined(AT32F407VCT7) || defined(AT32F407VET7) || defined(AT32F407RET7) || \
   defined(AT32F407AVCT7) || defined(AT32F407AVGT7)
   #include "at32f403a_407.h"
#elif defined(AT32F437RCT7) || defined(AT32F437RGT7) || defined(AT32F437RMT7) || \
   defined(AT32F437VCT7) || defined(AT32F437VGT7) || defined(AT32F437VMT7) || \
   defined(AT32F437ZCT7) || defined(AT32F437ZGT7) || defined(AT32F437ZMT7)
   #include "at32f435_437.h"
#endif

//Dependencies
#include "core/net.h"
#include "drivers/mac/at32f4xx_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
static uint8_t txBuffer[AT32F4XX_ETH_TX_BUFFER_COUNT][AT32F4XX_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
static uint8_t rxBuffer[AT32F4XX_ETH_RX_BUFFER_COUNT][AT32F4XX_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
static At32f4xxTxDmaDesc txDmaDesc[AT32F4XX_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
static At32f4xxRxDmaDesc rxDmaDesc[AT32F4XX_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[AT32F4XX_ETH_TX_BUFFER_COUNT][AT32F4XX_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Receive buffer
static uint8_t rxBuffer[AT32F4XX_ETH_RX_BUFFER_COUNT][AT32F4XX_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Transmit DMA descriptors
static At32f4xxTxDmaDesc txDmaDesc[AT32F4XX_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//Receive DMA descriptors
static At32f4xxRxDmaDesc rxDmaDesc[AT32F4XX_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//Pointer to the current TX DMA descriptor
static At32f4xxTxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static At32f4xxRxDmaDesc *rxCurDmaDesc;


/**
 * @brief AT32F4 Ethernet MAC driver
 **/

const NicDriver at32f4xxEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   at32f4xxEthInit,
   at32f4xxEthTick,
   at32f4xxEthEnableIrq,
   at32f4xxEthDisableIrq,
   at32f4xxEthEventHandler,
   at32f4xxEthSendPacket,
   at32f4xxEthUpdateMacAddrFilter,
   at32f4xxEthUpdateMacConfig,
   at32f4xxEthWritePhyReg,
   at32f4xxEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief AT32F4 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t at32f4xxEthInit(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing AT32F4 Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   at32f4xxEthInitGpio(interface);

   //Enable Ethernet MAC clock
   crm_periph_clock_enable(CRM_EMAC_PERIPH_CLOCK, TRUE);
   crm_periph_clock_enable(CRM_EMACTX_PERIPH_CLOCK, TRUE);
   crm_periph_clock_enable(CRM_EMACRX_PERIPH_CLOCK, TRUE);

   //Reset Ethernet MAC peripheral
   crm_periph_reset(CRM_EMAC_PERIPH_RESET, TRUE);
   crm_periph_reset(CRM_EMAC_PERIPH_RESET, FALSE);

   //Perform a software reset
   EMAC_DMA->bm |= EMAC_DMA_BM_SWR;
   //Wait for the reset to complete
   while((EMAC_DMA->bm & EMAC_DMA_BM_SWR) != 0)
   {
   }

   //Adjust MDC clock range depending on HCLK frequency
   EMAC->miiaddr = EMAC_MIIADDR_CR_DIV_124;

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
   EMAC->ctrl = EMAC_CTRL_RESERVED15 | EMAC_CTRL_DRO;

   //Set the MAC address of the station
   EMAC->a0l = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   EMAC->a0h = interface->macAddr.w[2] | EMAC_A0H_AE;

   //The MAC supports 3 additional addresses for unicast perfect filtering
   EMAC->a1l = 0;
   EMAC->a1h = 0;
   EMAC->a2l = 0;
   EMAC->a2h = 0;
   EMAC->a3l = 0;
   EMAC->a3h = 0;

   //Initialize hash table
   EMAC->htl = 0;
   EMAC->hth = 0;

   //Configure the receive filter
   EMAC->frmf = EMAC_FRMF_HPF | EMAC_FRMF_HMC;
   //Disable flow control
   EMAC->fctrl = 0;
   //Enable store and forward mode
   EMAC_DMA->opm = EMAC_DMA_OPM_RSF | EMAC_DMA_OPM_TSF;

   //Configure DMA bus mode
   EMAC_DMA->bm = EMAC_DMA_BM_AAB | EMAC_DMA_BM_USP | EMAC_DMA_BM_RDP_32 |
      EMAC_DMA_BM_PR_1_1 | EMAC_DMA_BM_PBL_32;

   //Initialize DMA descriptor lists
   at32f4xxEthInitDmaDesc(interface);

   //Prevent interrupts from being generated when the transmit statistic
   //counters reach half their maximum value
   EMAC_MMC->tim = EMAC_MMC_TIM_TGFCIM | EMAC_MMC_TIM_TMCGFCIM |
      EMAC_MMC_TIM_TSCGFCIM;

   //Prevent interrupts from being generated when the receive statistic
   //counters reach half their maximum value
   EMAC_MMC->rim = EMAC_MMC_RIM_RUGFCIM | EMAC_MMC_RIM_RAEFACIM |
      EMAC_MMC_RIM_RCEFCIM;

   //Disable MAC interrupts
   EMAC->imr = EMAC_IMR_TIM | EMAC_IMR_PIM;
   //Enable the desired DMA interrupts
   EMAC_DMA->ie = EMAC_DMA_IE_NIE | EMAC_DMA_IE_RIE | EMAC_DMA_IE_TIE;

   //Set priority grouping (4 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(AT32F4XX_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(EMAC_IRQn, NVIC_EncodePriority(AT32F4XX_ETH_IRQ_PRIORITY_GROUPING,
      AT32F4XX_ETH_IRQ_GROUP_PRIORITY, AT32F4XX_ETH_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   EMAC->ctrl |= EMAC_CTRL_TE | EMAC_CTRL_RE;
   //Enable DMA transmission and reception
   EMAC_DMA->opm |= EMAC_DMA_OPM_SSTC | EMAC_DMA_OPM_SSR;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void at32f4xxEthInitGpio(NetInterface *interface)
{
//AT-START-F407 evaluation board?
#if defined(AT_START_F407_V1)
   gpio_init_type gpio_init_struct;

   //Enable IOMUX clock
   crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);

   //Enable GPIO clocks
   crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
   crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
   crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
   crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);

   //Remap EMAC pins
   gpio_pin_remap_config(EMAC_MUX, TRUE);

   //Select RMII interface mode
   gpio_pin_remap_config(MII_RMII_SEL_GMUX, TRUE);

   //Get default parameters
   gpio_default_para_init(&gpio_init_struct);

   //Configure CLKOUT (PA8) as an output
   gpio_init_struct.gpio_pins = GPIO_PINS_8;
   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
   gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
   gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
   gpio_init(GPIOA, &gpio_init_struct);

   //Configure CLKOUT pin to output SCLK/8 clock (25MHz)
   crm_clock_out_set(CRM_CLKOUT_SCLK);
   crm_clkout_div_set(CRM_CLKOUT_DIV_8);

   //Configure RMII pins
   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
   gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
   gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;

   //Configure EMAC_RMII_REF_CLK (PA1) and EMAC_MDIO (PA2)
   gpio_init_struct.gpio_pins = GPIO_PINS_1 | GPIO_PINS_2;
   gpio_init(GPIOA, &gpio_init_struct);

   //Configure EMAC_RMII_TX_EN (PB11), EMAC_RMII_TXD0 (PB12) and
   //EMAC_RMII_TXD1 (PB13)
   gpio_init_struct.gpio_pins = GPIO_PINS_11 | GPIO_PINS_12 | GPIO_PINS_13;
   gpio_init(GPIOB, &gpio_init_struct);

   //Configure EMAC_MDC (PC1)
   gpio_init_struct.gpio_pins = GPIO_PINS_1;
   gpio_init(GPIOC, &gpio_init_struct);

   //Configure EMAC_RMII_CRS_DV (PD8), EMAC_RMII_RXD0 (PD9) and
   //EMAC_RMII_RXD1 (PD10)
   gpio_init_struct.gpio_pins = GPIO_PINS_8 | GPIO_PINS_9 | GPIO_PINS_10;
   gpio_init(GPIOD, &gpio_init_struct);

   //Configure PHY_RST (PC8)
   gpio_init_struct.gpio_pins = GPIO_PINS_8;
   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
   gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
   gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
   gpio_init(GPIOC, &gpio_init_struct);

   //Reset PHY transceiver
   gpio_bits_write(GPIOC, GPIO_PINS_8, FALSE);
   sleep(10);
   gpio_bits_write(GPIOC, GPIO_PINS_8, TRUE);
   sleep(10);

//AT-START-F437 evaluation board?
#elif defined(AT_START_F437_V1)
   gpio_init_type gpio_init_struct;

   //Enable SCFG clock
   crm_periph_clock_enable(CRM_SCFG_PERIPH_CLOCK, TRUE);

   //Enable GPIO clocks
   crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
   crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
   crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);
   crm_periph_clock_enable(CRM_GPIOE_PERIPH_CLOCK, TRUE);
   crm_periph_clock_enable(CRM_GPIOG_PERIPH_CLOCK, TRUE);

   //Select RMII interface mode
   scfg_emac_interface_set(SCFG_EMAC_SELECT_RMII);

   //Get default parameters
   gpio_default_para_init(&gpio_init_struct);

   //Configure CLKOUT1 (PA8) as an output
   gpio_init_struct.gpio_pins = GPIO_PINS_8;
   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
   gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
   gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
   gpio_init(GPIOA, &gpio_init_struct);

   //Remap CLKOUT1 pin
   gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE8, GPIO_MUX_0);

   //Configure CLKOUT1 pin to output PLLCLK/10 clock (25MHz)
   crm_clock_out1_set(CRM_CLKOUT1_PLL);
   crm_clkout_div_set(CRM_CLKOUT_INDEX_1, CRM_CLKOUT_DIV1_5, CRM_CLKOUT_DIV2_2);

   //Configure RMII pins
   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
   gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
   gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;

   //Configure EMAC_RMII_REF_CLK (PA1) and EMAC_MDIO (PA2)
   gpio_init_struct.gpio_pins = GPIO_PINS_1 | GPIO_PINS_2;
   gpio_init(GPIOA, &gpio_init_struct);

   //Configure EMAC_MDC (PC1)
   gpio_init_struct.gpio_pins = GPIO_PINS_1;
   gpio_init(GPIOC, &gpio_init_struct);

   //Configure EMAC_RMII_CRS_DV (PD8), EMAC_RMII_RXD0 (PD9) and
   //EMAC_RMII_RXD1 (PD10)
   gpio_init_struct.gpio_pins = GPIO_PINS_8 | GPIO_PINS_9 | GPIO_PINS_10;
   gpio_init(GPIOD, &gpio_init_struct);

   //Configure EMAC_RMII_TX_EN (PG11), EMAC_RMII_TXD0 (PG13) and
   //EMAC_RMII_TXD1 (PG14)
   gpio_init_struct.gpio_pins = GPIO_PINS_11 | GPIO_PINS_13 | GPIO_PINS_14;
   gpio_init(GPIOG, &gpio_init_struct);

   //Remap Ethernet pins
   gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE1, GPIO_MUX_11);
   gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE2, GPIO_MUX_11);
   gpio_pin_mux_config(GPIOC, GPIO_PINS_SOURCE1, GPIO_MUX_11);
   gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE8, GPIO_MUX_11);
   gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE9, GPIO_MUX_11);
   gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE10, GPIO_MUX_11);
   gpio_pin_mux_config(GPIOG, GPIO_PINS_SOURCE11, GPIO_MUX_11);
   gpio_pin_mux_config(GPIOG, GPIO_PINS_SOURCE13, GPIO_MUX_11);
   gpio_pin_mux_config(GPIOG, GPIO_PINS_SOURCE14, GPIO_MUX_11);

   //Configure PHY_RST (PE15)
   gpio_init_struct.gpio_pins = GPIO_PINS_15;
   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
   gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
   gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
   gpio_init(GPIOE, &gpio_init_struct);

   //Configure PHY_PD (PG15)
   gpio_init_struct.gpio_pins = GPIO_PINS_15;
   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
   gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
   gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
   gpio_init(GPIOG, &gpio_init_struct);

   //Exit power down mode
   gpio_bits_write(GPIOG, GPIO_PINS_15, FALSE);

   //Reset PHY transceiver
   gpio_bits_write(GPIOE, GPIO_PINS_15, FALSE);
   sleep(10);
   gpio_bits_write(GPIOE, GPIO_PINS_15, TRUE);
   sleep(10);
#endif
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void at32f4xxEthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < AT32F4XX_ETH_TX_BUFFER_COUNT; i++)
   {
      //Use chain structure rather than ring structure
      txDmaDesc[i].tdes0 = EMAC_TDES0_IC | EMAC_TDES0_TCH;
      //Initialize transmit buffer size
      txDmaDesc[i].tdes1 = 0;
      //Transmit buffer address
      txDmaDesc[i].tdes2 = (uint32_t) txBuffer[i];
      //Next descriptor address
      txDmaDesc[i].tdes3 = (uint32_t) &txDmaDesc[i + 1];
   }

   //The last descriptor is chained to the first entry
   txDmaDesc[i - 1].tdes3 = (uint32_t) &txDmaDesc[0];
   //Point to the very first descriptor
   txCurDmaDesc = &txDmaDesc[0];

   //Initialize RX DMA descriptor list
   for(i = 0; i < AT32F4XX_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = EMAC_RDES0_OWN;
      //Use chain structure rather than ring structure
      rxDmaDesc[i].rdes1 = EMAC_RDES1_RCH | (AT32F4XX_ETH_RX_BUFFER_SIZE & EMAC_RDES1_RBS1);
      //Receive buffer address
      rxDmaDesc[i].rdes2 = (uint32_t) rxBuffer[i];
      //Next descriptor address
      rxDmaDesc[i].rdes3 = (uint32_t) &rxDmaDesc[i + 1];
   }

   //The last descriptor is chained to the first entry
   rxDmaDesc[i - 1].rdes3 = (uint32_t) &rxDmaDesc[0];
   //Point to the very first descriptor
   rxCurDmaDesc = &rxDmaDesc[0];

   //Start location of the TX descriptor list
   EMAC_DMA->tdladdr = (uint32_t) txDmaDesc;
   //Start location of the RX descriptor list
   EMAC_DMA->rdladdr = (uint32_t) rxDmaDesc;
}


/**
 * @brief AT32F4 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void at32f4xxEthTick(NetInterface *interface)
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

void at32f4xxEthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(EMAC_IRQn);

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

void at32f4xxEthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(EMAC_IRQn);

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
 * @brief AT32F4 Ethernet MAC interrupt service routine
 **/

void EMAC_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = EMAC_DMA->sts;

   //Packet transmitted?
   if((status & EMAC_DMA_STS_TI) != 0)
   {
      //Clear TI interrupt flag
      EMAC_DMA->sts = EMAC_DMA_STS_TI;

      //Check whether the TX buffer is available for writing
      if((txCurDmaDesc->tdes0 & EMAC_TDES0_OWN) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & EMAC_DMA_STS_RI) != 0)
   {
      //Clear RI interrupt flag
      EMAC_DMA->sts = EMAC_DMA_STS_RI;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   EMAC_DMA->sts = EMAC_DMA_STS_NIS;

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief AT32F4 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void at32f4xxEthEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = at32f4xxEthReceivePacket(interface);

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

error_t at32f4xxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > AT32F4XX_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txCurDmaDesc->tdes0 & EMAC_TDES0_OWN) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead((uint8_t *) txCurDmaDesc->tdes2, buffer, offset, length);

   //Write the number of bytes to send
   txCurDmaDesc->tdes1 = length & EMAC_TDES1_TBS1;
   //Set LS and FS flags as the data fits in a single buffer
   txCurDmaDesc->tdes0 |= EMAC_TDES0_LS | EMAC_TDES0_FS;
   //Give the ownership of the descriptor to the DMA
   txCurDmaDesc->tdes0 |= EMAC_TDES0_OWN;

   //Clear TBUS flag to resume processing
   EMAC_DMA->sts = EMAC_DMA_STS_TBU;
   //Instruct the DMA to poll the transmit descriptor list
   EMAC_DMA->tpd = 0;

   //Point to the next descriptor in the list
   txCurDmaDesc = (At32f4xxTxDmaDesc *) txCurDmaDesc->tdes3;

   //Check whether the next buffer is available for writing
   if((txCurDmaDesc->tdes0 & EMAC_TDES0_OWN) == 0)
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

error_t at32f4xxEthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxCurDmaDesc->rdes0 & EMAC_RDES0_OWN) == 0)
   {
      //FS and LS flags should be set
      if((rxCurDmaDesc->rdes0 & EMAC_RDES0_FS) != 0 &&
         (rxCurDmaDesc->rdes0 & EMAC_RDES0_LS) != 0)
      {
         //Make sure no error occurred
         if((rxCurDmaDesc->rdes0 & EMAC_RDES0_ES) == 0)
         {
            //Retrieve the length of the frame
            n = (rxCurDmaDesc->rdes0 & EMAC_RDES0_FL) >> 16;
            //Limit the number of data to read
            n = MIN(n, AT32F4XX_ETH_RX_BUFFER_SIZE);

            //Additional options can be passed to the stack along with the packet
            ancillary = NET_DEFAULT_RX_ANCILLARY;

            //Pass the packet to the upper layer
            nicProcessPacket(interface, (uint8_t *) rxCurDmaDesc->rdes2, n,
               &ancillary);

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

      //Give the ownership of the descriptor back to the DMA
      rxCurDmaDesc->rdes0 = EMAC_RDES0_OWN;
      //Point to the next descriptor in the list
      rxCurDmaDesc = (At32f4xxRxDmaDesc *) rxCurDmaDesc->rdes3;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RBUS flag to resume processing
   EMAC_DMA->sts = EMAC_DMA_STS_RBU;
   //Instruct the DMA to poll the receive descriptor list
   EMAC_DMA->rpd = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t at32f4xxEthUpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   uint_t k;
   uint32_t crc;
   uint32_t hashTable[2];
   MacAddr unicastMacAddr[3];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Set the MAC address of the station
   EMAC->a0l = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   EMAC->a0h = interface->macAddr.w[2] | EMAC_A0H_AE;

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
            //Compute CRC over the current MAC address
            crc = at32f4xxEthCalcCrc(&entry->addr, sizeof(MacAddr));

            //The upper 6 bits in the CRC register are used to index the
            //contents of the hash table
            k = (crc >> 26) & 0x3F;

            //Update hash table contents
            hashTable[k / 32] |= (1 << (k % 32));
         }
         else
         {
            //Up to 3 additional MAC addresses can be specified
            if(j < 3)
            {
               //Save the unicast address
               unicastMacAddr[j++] = entry->addr;
            }
         }
      }
   }

   //Configure the first unicast address filter
   if(j >= 1)
   {
      //When the AE bit is set, the entry is used for perfect filtering
      EMAC->a1l = unicastMacAddr[0].w[0] | (unicastMacAddr[0].w[1] << 16);
      EMAC->a1h = unicastMacAddr[0].w[2] | EMAC_A1H_AE;
   }
   else
   {
      //When the AE bit is cleared, the entry is ignored
      EMAC->a1l = 0;
      EMAC->a1h = 0;
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      //When the AE bit is set, the entry is used for perfect filtering
      EMAC->a2l = unicastMacAddr[1].w[0] | (unicastMacAddr[1].w[1] << 16);
      EMAC->a2h = unicastMacAddr[1].w[2] | EMAC_A2H_AE;
   }
   else
   {
      //When the AE bit is cleared, the entry is ignored
      EMAC->a2l = 0;
      EMAC->a2h = 0;
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      //When the AE bit is set, the entry is used for perfect filtering
      EMAC->a3l = unicastMacAddr[2].w[0] | (unicastMacAddr[2].w[1] << 16);
      EMAC->a3h = unicastMacAddr[2].w[2] | EMAC_A3H_AE;
   }
   else
   {
      //When the AE bit is cleared, the entry is ignored
      EMAC->a3l = 0;
      EMAC->a3h = 0;
   }

   //Configure the multicast hash table
   EMAC->htl = hashTable[0];
   EMAC->hth = hashTable[1];

   //Debug message
   TRACE_DEBUG("  EMAC->htl = %08" PRIX32 "\r\n", EMAC->htl);
   TRACE_DEBUG("  EMAC->hth = %08" PRIX32 "\r\n", EMAC->hth);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t at32f4xxEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = EMAC->ctrl;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= EMAC_CTRL_FES;
   }
   else
   {
      config &= ~EMAC_CTRL_FES;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= EMAC_CTRL_DM;
   }
   else
   {
      config &= ~EMAC_CTRL_DM;
   }

   //Update MAC configuration register
   EMAC->ctrl = config;

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

void at32f4xxEthWritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_WRITE)
   {
      //Take care not to alter MDC clock configuration
      temp = EMAC->miiaddr & EMAC_MIIADDR_CR;
      //Set up a write operation
      temp |= EMAC_MIIADDR_MW | EMAC_MIIADDR_MB;
      //PHY address
      temp |= (phyAddr << 11) & EMAC_MIIADDR_PA;
      //Register address
      temp |= (regAddr << 6) & EMAC_MIIADDR_MII;

      //Data to be written in the PHY register
      EMAC->miidt = data & EMAC_MIIDT_MD;

      //Start a write operation
      EMAC->miiaddr = temp;
      //Wait for the write to complete
      while((EMAC->miiaddr & EMAC_MIIADDR_MB) != 0)
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

uint16_t at32f4xxEthReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;
   uint32_t temp;

   //Valid opcode?
   if(opcode == SMI_OPCODE_READ)
   {
      //Take care not to alter MDC clock configuration
      temp = EMAC->miiaddr & EMAC_MIIADDR_CR;
      //Set up a read operation
      temp |= EMAC_MIIADDR_MB;
      //PHY address
      temp |= (phyAddr << 11) & EMAC_MIIADDR_PA;
      //Register address
      temp |= (regAddr << 6) & EMAC_MIIADDR_MII;

      //Start a read operation
      EMAC->miiaddr = temp;
      //Wait for the read to complete
      while((EMAC->miiaddr & EMAC_MIIADDR_MB) != 0)
      {
      }

      //Get register value
      data = EMAC->miidt & EMAC_MIIDT_MD;
   }
   else
   {
      //The MAC peripheral only supports standard Clause 22 opcodes
      data = 0;
   }

   //Return the value of the PHY register
   return data;
}


/**
 * @brief CRC calculation
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t at32f4xxEthCalcCrc(const void *data, size_t length)
{
   uint_t i;
   uint_t j;
   uint32_t crc;
   const uint8_t *p;

   //Point to the data over which to calculate the CRC
   p = (uint8_t *) data;
   //CRC preset value
   crc = 0xFFFFFFFF;

   //Loop through data
   for(i = 0; i < length; i++)
   {
      //The message is processed bit by bit
      for(j = 0; j < 8; j++)
      {
         //Update CRC value
         if((((crc >> 31) ^ (p[i] >> j)) & 0x01) != 0)
         {
            crc = (crc << 1) ^ 0x04C11DB7;
         }
         else
         {
            crc = crc << 1;
         }
      }
   }

   //Return CRC value
   return ~crc;
}
