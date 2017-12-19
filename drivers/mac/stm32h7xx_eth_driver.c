/**
 * @file stm32h7xx_eth_driver.c
 * @brief STM32H743/753 Ethernet MAC controller
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
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include "stm32h7xx.h"
#include "core/net.h"
#include "drivers/mac/stm32h7xx_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
#pragma location = ".ram_no_cache"
static uint8_t txBuffer[STM32H7XX_ETH_TX_BUFFER_COUNT][STM32H7XX_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
#pragma location = ".ram_no_cache"
static uint8_t rxBuffer[STM32H7XX_ETH_RX_BUFFER_COUNT][STM32H7XX_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
#pragma location = ".ram_no_cache"
static Stm32h7xxTxDmaDesc txDmaDesc[STM32H7XX_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
#pragma location = ".ram_no_cache"
static Stm32h7xxRxDmaDesc rxDmaDesc[STM32H7XX_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[STM32H7XX_ETH_TX_BUFFER_COUNT][STM32H7XX_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(".ram_no_cache")));
//Receive buffer
static uint8_t rxBuffer[STM32H7XX_ETH_RX_BUFFER_COUNT][STM32H7XX_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(".ram_no_cache")));
//Transmit DMA descriptors
static Stm32h7xxTxDmaDesc txDmaDesc[STM32H7XX_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(".ram_no_cache")));
//Receive DMA descriptors
static Stm32h7xxRxDmaDesc rxDmaDesc[STM32H7XX_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4), __section__(".ram_no_cache")));

#endif

//Current transmit descriptor
static uint_t txIndex;
//Current receive descriptor
static uint_t rxIndex;


/**
 * @brief STM32H743/753 Ethernet MAC driver
 **/

const NicDriver stm32h7xxEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   stm32h7xxEthInit,
   stm32h7xxEthTick,
   stm32h7xxEthEnableIrq,
   stm32h7xxEthDisableIrq,
   stm32h7xxEthEventHandler,
   stm32h7xxEthSendPacket,
   stm32h7xxEthSetMulticastFilter,
   stm32h7xxEthUpdateMacConfig,
   stm32h7xxEthWritePhyReg,
   stm32h7xxEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief STM32H743/753 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t stm32h7xxEthInit(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing STM32H7xx Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   stm32h7xxEthInitGpio(interface);

   //Enable Ethernet MAC clock
   __HAL_RCC_ETH1MAC_CLK_ENABLE();
   __HAL_RCC_ETH1TX_CLK_ENABLE();
   __HAL_RCC_ETH1RX_CLK_ENABLE();

   //Reset Ethernet MAC peripheral
   __HAL_RCC_ETH1MAC_FORCE_RESET();
   __HAL_RCC_ETH1MAC_RELEASE_RESET();

   //Perform a software reset
   ETH->DMAMR |= ETH_DMAMR_SWR;
   //Wait for the reset to complete
   while(ETH->DMAMR & ETH_DMAMR_SWR);

   //Adjust MDC clock range depending on HCLK frequency
   ETH->MACMDIOAR = ETH_MACMDIOAR_CR_DIV124;

   //PHY transceiver initialization
   error = interface->phyDriver->init(interface);
   //Failed to initialize PHY transceiver?
   if(error)
      return error;

   //Use default MAC configuration
   ETH->MACCR = ETH_MACCR_DO;

   //Set the MAC address
   ETH->MACA0LR = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ETH->MACA0HR = interface->macAddr.w[2];

   //Initialize hash table
   ETH->MACHT0R = 0;
   ETH->MACHT1R = 0;

   //Configure the receive filter
   ETH->MACPFR = ETH_MACPFR_HPF | ETH_MACPFR_HMC;

   //Disable flow control
   ETH->MACTFCR = 0;
   ETH->MACRFCR = 0;

   //Configure DMA operating mode
   ETH->DMAMR = ETH_DMAMR_INTM_0 | ETH_DMAMR_PR_1_1;
   //Configure system bus mode
   ETH->DMASBMR |= ETH_DMASBMR_AAL;
   //The DMA takes the descriptor table as contiguous
   ETH->DMACCR = ETH_DMACCR_DSL_0BIT;

   //Configure TX features
   ETH->DMACTCR = ETH_DMACTCR_TPBL_1PBL;

   //Configure RX features
   ETH->DMACRCR = ETH_DMACRCR_RPBL_1PBL;
   ETH->DMACRCR |= (STM32H7XX_ETH_RX_BUFFER_SIZE << 1) & ETH_DMACRCR_RBSZ;

   //Enable store and forward mode
   ETH->MTLTQOMR |= ETH_MTLTQOMR_TSF;
   ETH->MTLRQOMR |= ETH_MTLRQOMR_RSF;

   //Initialize DMA descriptor lists
   stm32h7xxEthInitDmaDesc(interface);

   //Prevent interrupts from being generated when the transmit statistic
   //counters reach half their maximum value
   ETH->MMCTIMR = ETH_MMCTIMR_TXLPITRCIM | ETH_MMCTIMR_TXLPIUSCIM |
      ETH_MMCTIMR_TXGPKTIM | ETH_MMCTIMR_TXMCOLGPIM | ETH_MMCTIMR_TXSCOLGPIM;

   //Prevent interrupts from being generated when the receive statistic
   //counters reach half their maximum value
   ETH->MMCRIMR = ETH_MMCRIMR_RXLPITRCIM | ETH_MMCRIMR_RXLPIUSCIM |
      ETH_MMCRIMR_RXUCGPIM | ETH_MMCRIMR_RXALGNERPIM | ETH_MMCRIMR_RXCRCERPIM;

   //Disable MAC interrupts
   ETH->MACIER = 0;
   //Enable the desired DMA interrupts
   ETH->DMACIER = ETH_DMACIER_NIE | ETH_DMACIER_RIE | ETH_DMACIER_TIE;

   //Set priority grouping (4 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(STM32H7XX_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(ETH_IRQn, NVIC_EncodePriority(STM32H7XX_ETH_IRQ_PRIORITY_GROUPING,
      STM32H7XX_ETH_IRQ_GROUP_PRIORITY, STM32H7XX_ETH_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   ETH->MACCR |= ETH_MACCR_TE | ETH_MACCR_RE;

   //Enable DMA transmission and reception
   ETH->DMACTCR |= ETH_DMACTCR_ST;
   ETH->DMACRCR |= ETH_DMACRCR_SR;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


//STM32F743I-EVAL or Nucleo-H743I evaluation board?
#if defined(USE_STM32H743I_EVAL) || defined(USE_STM32H7XX_NUCLEO_144)

/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

void stm32h7xxEthInitGpio(NetInterface *interface)
{
   uint32_t temp;
   GPIO_InitTypeDef GPIO_InitStructure;

//STM32F743I-EVAL evaluation board?
#if defined(USE_STM32H743I_EVAL)
   //Enable SYSCFG clock
   __HAL_RCC_SYSCFG_CLK_ENABLE();

   //Enable GPIO clocks
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();

   //Select RMII interface mode
   temp = SYSCFG->PMCR & ~SYCFG_PMCR_EPIS_SEL;
   SYSCFG->PMCR = temp | SYSCFG_PMCR_EPIS_SEL_2;

   //Configure RMII pins
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStructure.Alternate = GPIO_AF11_ETH;

   //Configure ETH_RMII_REF_CLK (PA1), ETH_MDIO (PA2) and ETH_RMII_CRS_DV (PA7)
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   //Configure ETH_MDC (PC1), ETH_RMII_RXD0 (PC4) and ETH_RMII_RXD1 (PC5)
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

   //Configure RMII_TX_EN (PG11), ETH_RMII_TXD1 (PG12) and ETH_RMII_TXD0 (PG13)
   GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

//Nucleo-H743I evaluation board?
#elif defined(USE_STM32H7XX_NUCLEO_144)
   //Enable SYSCFG clock
   __HAL_RCC_SYSCFG_CLK_ENABLE();

   //Enable GPIO clocks
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();

   //Select RMII interface mode
   temp = SYSCFG->PMCR & ~SYCFG_PMCR_EPIS_SEL;
   SYSCFG->PMCR = temp | SYSCFG_PMCR_EPIS_SEL_2;

   //Configure RMII pins
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStructure.Alternate = GPIO_AF11_ETH;

   //Configure ETH_RMII_REF_CLK (PA1), ETH_MDIO (PA2) and ETH_RMII_CRS_DV (PA7)
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   //Configure ETH_RMII_TXD1 (PB13)
   GPIO_InitStructure.Pin = GPIO_PIN_13;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

   //Configure ETH_MDC (PC1), ETH_RMII_RXD0 (PC4) and ETH_RMII_RXD1 (PC5)
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

   //Configure RMII_TX_EN (PG11), ETH_RMII_TXD0 (PG13)
   GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_13;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
#endif
}

#endif


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void stm32h7xxEthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < STM32H7XX_ETH_TX_BUFFER_COUNT; i++)
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
   for(i = 0; i < STM32H7XX_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = (uint32_t) rxBuffer[i];
      rxDmaDesc[i].rdes1 = 0;
      rxDmaDesc[i].rdes2 = 0;
      rxDmaDesc[i].rdes3 = ETH_RDES3_OWN | ETH_RDES3_IOC | ETH_RDES3_BUF1V;
   }

   //Initialize RX descriptor index
   rxIndex = 0;

   //Start location of the TX descriptor list
   ETH->DMACTDLAR = (uint32_t) &txDmaDesc[0];
   //Length of the transmit descriptor ring
   ETH->DMACTDRLR = STM32H7XX_ETH_TX_BUFFER_COUNT - 1;

   //Start location of the RX descriptor list
   ETH->DMACRDLAR = (uint32_t) &rxDmaDesc[0];
   //Length of the receive descriptor ring
   ETH->DMACRDRLR = STM32H7XX_ETH_RX_BUFFER_COUNT - 1;
}


/**
 * @brief STM32H743/753 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void stm32h7xxEthTick(NetInterface *interface)
{
   //Handle periodic operations
   interface->phyDriver->tick(interface);
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void stm32h7xxEthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(ETH_IRQn);
   //Enable Ethernet PHY interrupts
   interface->phyDriver->enableIrq(interface);
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void stm32h7xxEthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(ETH_IRQn);
   //Disable Ethernet PHY interrupts
   interface->phyDriver->disableIrq(interface);
}


/**
 * @brief STM32H743/753 Ethernet MAC interrupt service routine
 **/

void ETH_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Enter interrupt service routine
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = ETH->DMACSR;

   //A packet has been transmitted?
   if(status & ETH_DMACSR_TI)
   {
      //Clear TI interrupt flag
      ETH->DMACSR = ETH_DMACSR_TI;

      //Check whether the TX buffer is available for writing
      if(!(txDmaDesc[txIndex].tdes3 & ETH_TDES3_OWN))
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //A packet has been received?
   if(status & ETH_DMACSR_RI)
   {
      //Disable RIE interrupt
      ETH->DMACIER &= ~ETH_DMACIER_RIE;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   ETH->DMACSR = ETH_DMACSR_NIS;

   //Leave interrupt service routine
   osExitIsr(flag);
}


/**
 * @brief STM32H743/753 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void stm32h7xxEthEventHandler(NetInterface *interface)
{
   error_t error;

   //Packet received?
   if(ETH->DMACSR & ETH_DMACSR_RI)
   {
      //Clear interrupt flag
      ETH->DMACSR = ETH_DMACSR_RI;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = stm32h7xxEthReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }

   //Re-enable DMA interrupts
   ETH->DMACIER |= ETH_DMACIER_NIE | ETH_DMACIER_RIE | ETH_DMACIER_TIE;
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t stm32h7xxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
{
   static uint8_t temp[STM32H7XX_ETH_TX_BUFFER_SIZE];
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > STM32H7XX_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if(txDmaDesc[txIndex].tdes3 & ETH_TDES3_OWN)
      return ERROR_FAILURE;

   //Copy user data to the transmit buffer
   netBufferRead(temp, buffer, offset, length);
   memcpy(txBuffer[txIndex], temp, (length + 3) & ~3UL);

   //Set the start address of the buffer
   txDmaDesc[txIndex].tdes0 = (uint32_t) txBuffer[txIndex];
   //Write the number of bytes to send
   txDmaDesc[txIndex].tdes2 = ETH_TDES2_IOC | (length & ETH_TDES2_B1L);
   //Give the ownership of the descriptor to the DMA
   txDmaDesc[txIndex].tdes3 = ETH_TDES3_OWN | ETH_TDES3_FD | ETH_TDES3_LD;

   //Data synchronization barrier
   __DSB();

   //Clear TBU flag to resume processing
   ETH->DMACSR = ETH_DMACSR_TBU;
   //Instruct the DMA to poll the transmit descriptor list
   ETH->DMACTDTPR = 0;

   //Increment index and wrap around if necessary
   if(++txIndex >= STM32H7XX_ETH_TX_BUFFER_COUNT)
      txIndex = 0;

   //Check whether the next buffer is available for writing
   if(!(txDmaDesc[txIndex].tdes3 & ETH_TDES3_OWN))
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

error_t stm32h7xxEthReceivePacket(NetInterface *interface)
{
   static uint8_t temp[STM32H7XX_ETH_RX_BUFFER_SIZE];
   error_t error;
   size_t n;

   //The current buffer is available for reading?
   if(!(rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_OWN))
   {
      //FD and LD flags should be set
      if((rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_FD) && (rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_LD))
      {
         //Make sure no error occurred
         if(!(rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_ES))
         {
            //Retrieve the length of the frame
            n = rxDmaDesc[rxIndex].rdes3 & ETH_RDES3_PL;
            //Limit the number of data to read
            n = MIN(n, STM32H7XX_ETH_RX_BUFFER_SIZE);

            //Copy data from the receive buffer
            memcpy(temp, (uint8_t *) rxBuffer[rxIndex], (n + 3) & ~3UL);

            //Pass the packet to the upper layer
            nicProcessPacket(interface, temp, n);

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
      rxDmaDesc[rxIndex].rdes3 = ETH_RDES3_OWN | ETH_RDES3_IOC | ETH_RDES3_BUF1V;

      //Increment index and wrap around if necessary
      if(++rxIndex >= STM32H7XX_ETH_RX_BUFFER_COUNT)
         rxIndex = 0;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RBU flag to resume processing
   ETH->DMACSR = ETH_DMACSR_RBU;
   //Instruct the DMA to poll the receive descriptor list
   ETH->DMACRDTPR = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure multicast MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t stm32h7xxEthSetMulticastFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint32_t hashTable[2];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating STM32H7xx hash table...\r\n");

   //Clear hash table
   hashTable[0] = 0;
   hashTable[1] = 0;

   //The MAC filter table contains the multicast MAC addresses
   //to accept when receiving an Ethernet frame
   for(i = 0; i < MAC_MULTICAST_FILTER_SIZE; i++)
   {
      //Point to the current entry
      entry = &interface->macMulticastFilter[i];

      //Valid entry?
      if(entry->refCount > 0)
      {
         //Compute CRC over the current MAC address
         crc = stm32h7xxEthCalcCrc(&entry->addr, sizeof(MacAddr));

         //The upper 6 bits in the CRC register are used to index the
         //contents of the hash table
         k = (crc >> 26) & 0x3F;

         //Update hash table contents
         hashTable[k / 32] |= (1 << (k % 32));
      }
   }

   //Write the hash table
   ETH->MACHT0R = hashTable[0];
   ETH->MACHT1R = hashTable[1];

   //Debug message
   TRACE_DEBUG("  MACHT0R = %08" PRIX32 "\r\n", ETH->MACHT0R);
   TRACE_DEBUG("  MACHT1R = %08" PRIX32 "\r\n", ETH->MACHT1R);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t stm32h7xxEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;

   //Read current MAC configuration
   config = ETH->MACCR;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
      config |= ETH_MACCR_FES;
   else
      config &= ~ETH_MACCR_FES;

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
      config |= ETH_MACCR_DM;
   else
      config &= ~ETH_MACCR_DM;

   //Update MAC configuration register
   ETH->MACCR = config;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @param[in] data Register value
 **/

void stm32h7xxEthWritePhyReg(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
   uint32_t value;

   //Take care not to alter MDC clock configuration
   value = ETH->MACMDIOAR & ETH_MACMDIOAR_CR;
   //Set up a write operation
   value |= ETH_MACMDIOAR_MOC_WR | ETH_MACMDIOAR_MB;
   //PHY address
   value |= (phyAddr << 21) & ETH_MACMDIOAR_PA;
   //Register address
   value |= (regAddr << 16) & ETH_MACMDIOAR_RDA;

   //Data to be written in the PHY register
   ETH->MACMDIODR = data & ETH_MACMDIODR_MD;

   //Start a write operation
   ETH->MACMDIOAR = value;
   //Wait for the write to complete
   while(ETH->MACMDIOAR & ETH_MACMDIOAR_MB);
}


/**
 * @brief Read PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t stm32h7xxEthReadPhyReg(uint8_t phyAddr, uint8_t regAddr)
{
   uint32_t value;

   //Take care not to alter MDC clock configuration
   value = ETH->MACMDIOAR & ETH_MACMDIOAR_CR;
   //Set up a read operation
   value |= ETH_MACMDIOAR_MOC_RD | ETH_MACMDIOAR_MB;
   //PHY address
   value |= (phyAddr << 21) & ETH_MACMDIOAR_PA;
   //Register address
   value |= (regAddr << 16) & ETH_MACMDIOAR_RDA;

   //Start a read operation
   ETH->MACMDIOAR = value;
   //Wait for the read to complete
   while(ETH->MACMDIOAR & ETH_MACMDIOAR_MB);

   //Return PHY register contents
   return ETH->MACMDIODR & ETH_MACMDIODR_MD;
}


/**
 * @brief CRC calculation
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t stm32h7xxEthCalcCrc(const void *data, size_t length)
{
   uint_t i;
   uint_t j;

   //Point to the data over which to calculate the CRC
   const uint8_t *p = (uint8_t *) data;
   //CRC preset value
   uint32_t crc = 0xFFFFFFFF;

   //Loop through data
   for(i = 0; i < length; i++)
   {
      //The message is processed bit by bit
      for(j = 0; j < 8; j++)
      {
         //Update CRC value
         if(((crc >> 31) ^ (p[i] >> j)) & 0x01)
            crc = (crc << 1) ^ 0x04C11DB7;
         else
            crc = crc << 1;
      }
   }

   //Return CRC value
   return ~crc;
}
