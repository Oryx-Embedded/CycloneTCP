/**
 * @file tm4c129_eth_driver.c
 * @brief Tiva TM4C129 Ethernet controller
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

//TM4C1294NCPDT device?
#if defined(PART_TM4C1294NCPDT)
   #include "tm4c1294ncpdt.h"
//TM4C129XNCZAD device?
#elif defined(PART_TM4C129XNCZAD)
   #include "tm4c129xnczad.h"
#endif

//Dependencies
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_emac.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "core/net.h"
#include "drivers/mac/tm4c129_eth_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
static uint8_t txBuffer[TM4C129_ETH_TX_BUFFER_COUNT][TM4C129_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
static uint8_t rxBuffer[TM4C129_ETH_RX_BUFFER_COUNT][TM4C129_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
static Tm4c129TxDmaDesc txDmaDesc[TM4C129_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
static Tm4c129RxDmaDesc rxDmaDesc[TM4C129_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[TM4C129_ETH_TX_BUFFER_COUNT][TM4C129_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Receive buffer
static uint8_t rxBuffer[TM4C129_ETH_RX_BUFFER_COUNT][TM4C129_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Transmit DMA descriptors
static Tm4c129TxDmaDesc txDmaDesc[TM4C129_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//Receive DMA descriptors
static Tm4c129RxDmaDesc rxDmaDesc[TM4C129_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//Pointer to the current TX DMA descriptor
static Tm4c129TxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static Tm4c129RxDmaDesc *rxCurDmaDesc;


/**
 * @brief Tiva TM4C129 Ethernet MAC driver
 **/

const NicDriver tm4c129EthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   tm4c129EthInit,
   tm4c129EthTick,
   tm4c129EthEnableIrq,
   tm4c129EthDisableIrq,
   tm4c129EthEventHandler,
   tm4c129EthSendPacket,
   tm4c129EthSetMulticastFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief Tiva TM4C129 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t tm4c129EthInit(NetInterface *interface)
{
   //Debug message
   TRACE_INFO("Initializing Tiva TM4C129 Ethernet controller...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable Ethernet controller clock
   SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
   //Reset Ethernet controller
   SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);
   //Wait for the reset to complete
   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0));

   //Enable internal PHY clock
   SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);
   //Reset internal PHY
   SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);
   //Wait for the reset to complete
   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EPHY0));

   //GPIO configuration
   tm4c129EthInitGpio(interface);

   //Perform a software reset
   EMAC0_DMABUSMOD_R |= EMAC_DMABUSMOD_SWR;
   //Wait for the reset to complete
   while(EMAC0_DMABUSMOD_R & EMAC_DMABUSMOD_SWR);

   //Adjust MDC clock range depending on SYSCLK frequency
   EMAC0_MIIADDR_R = EMAC_MIIADDR_CR_100_150;

   //Reset PHY transceiver
   tm4c129EthWritePhyReg(EPHY_BMCR, EPHY_BMCR_MIIRESET);
   //Wait for the reset to complete
   while(tm4c129EthReadPhyReg(EPHY_BMCR) & EPHY_BMCR_MIIRESET);

   //Dump PHY registers for debugging purpose
   tm4c129EthDumpPhyReg();

   //Configure LED0, LED1 and LED2
   tm4c129EthWritePhyReg(EPHY_LEDCFG, EPHY_LEDCFG_LED0_TX |
      EPHY_LEDCFG_LED1_RX | EPHY_LEDCFG_LED2_LINK);

   //Configure PHY interrupts as desired
   tm4c129EthWritePhyReg(EPHY_MISR1, EPHY_MISR1_LINKSTATEN);
   //Enable PHY interrupts
   tm4c129EthWritePhyReg(EPHY_SCR, EPHY_SCR_INTEN);

   //Use default MAC configuration
   EMAC0_CFG_R = EMAC_CFG_DRO;

   //Set the MAC address
   EMAC0_ADDR0L_R = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   EMAC0_ADDR0H_R = interface->macAddr.w[2];

   //Initialize hash table
   EMAC0_HASHTBLL_R = 0;
   EMAC0_HASHTBLH_R = 0;

   //Configure the receive filter
   EMAC0_FRAMEFLTR_R = EMAC_FRAMEFLTR_HPF | EMAC_FRAMEFLTR_HMC;
   //Disable flow control
   EMAC0_FLOWCTL_R = 0;
   //Enable store and forward mode
   EMAC0_DMAOPMODE_R = EMAC_DMAOPMODE_RSF | EMAC_DMAOPMODE_TSF;

   //Configure DMA bus mode
   EMAC0_DMABUSMOD_R = EMAC_DMABUSMOD_AAL | EMAC_DMABUSMOD_USP | EMAC_DMABUSMOD_RPBL_1 |
      EMAC_DMABUSMOD_PR_1_1 | EMAC_DMABUSMOD_PBL_1 | EMAC_DMABUSMOD_ATDS;

   //Initialize DMA descriptor lists
   tm4c129EthInitDmaDesc(interface);

   //Prevent interrupts from being generated when the transmit statistic
   //counters reach half their maximum value
   EMAC0_MMCTXIM_R = EMAC_MMCTXIM_OCTCNT | EMAC_MMCTXIM_MCOLLGF |
      EMAC_MMCTXIM_SCOLLGF | EMAC_MMCTXIM_GBF;

   //Prevent interrupts from being generated when the receive statistic
   //counters reach half their maximum value
   EMAC0_MMCRXIM_R = EMAC_MMCRXIM_UCGF | EMAC_MMCRXIM_ALGNERR |
      EMAC_MMCRXIM_CRCERR | EMAC_MMCRXIM_GBF;

   //Disable MAC interrupts
   EMAC0_IM_R = EMAC_IM_TSI | EMAC_IM_PMT;
   //Enable the desired DMA interrupts
   EMAC0_DMAIM_R = EMAC_DMAIM_NIE | EMAC_DMAIM_RIE | EMAC_DMAIM_TIE;
   //Enable PHY interrupts
   EMAC0_EPHYIM_R = EMAC_EPHYIM_INT;

   //Set priority grouping (3 bits for pre-emption priority, no bits for subpriority)
   IntPriorityGroupingSet(TM4C129_ETH_IRQ_PRIORITY_GROUPING);
   //Configure Ethernet interrupt priority
   IntPrioritySet(INT_EMAC0, TM4C129_ETH_IRQ_PRIORITY);

   //Enable MAC transmission and reception
   EMAC0_CFG_R |= EMAC_CFG_TE | EMAC_CFG_RE;
   //Enable DMA transmission and reception
   EMAC0_DMAOPMODE_R |= EMAC_DMAOPMODE_ST | EMAC_DMAOPMODE_SR;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


//DK-TM4C129X or EK-TM4C1294XL evaluation board?
#if defined(USE_DK_TM4C129X) || defined(USE_EK_TM4C1294XL)

/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

void tm4c129EthInitGpio(NetInterface *interface)
{
//DK-TM4C129X evaluation board?
#if defined(USE_DK_TM4C129X)
   //Enable GPIO clocks
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);

   //Select the relevant alternate function for PF1, PK4 and PK6
   GPIOPinConfigure(GPIO_PF1_EN0LED2);
   GPIOPinConfigure(GPIO_PK4_EN0LED0);
   GPIOPinConfigure(GPIO_PK6_EN0LED1);

   //Configure Ethernet LED pins for proper operation
   GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_1);
   GPIOPinTypeEthernetLED(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_6);

//EK-TM4C1294XL evaluation board?
#elif defined(USE_EK_TM4C1294XL)
   //Enable GPIO clock
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

   //Select the relevant alternate function for PF0 and PF4
   GPIOPinConfigure(GPIO_PF0_EN0LED0);
   GPIOPinConfigure(GPIO_PF4_EN0LED1);

   //Configure Ethernet LED pins for proper operation
   GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
#endif
}

#endif


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void tm4c129EthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < TM4C129_ETH_TX_BUFFER_COUNT; i++)
   {
      //Use chain structure rather than ring structure
      txDmaDesc[i].tdes0 = EMAC_TDES0_IC | EMAC_TDES0_TCH;
      //Initialize transmit buffer size
      txDmaDesc[i].tdes1 = 0;
      //Transmit buffer address
      txDmaDesc[i].tdes2 = (uint32_t) txBuffer[i];
      //Next descriptor address
      txDmaDesc[i].tdes3 = (uint32_t) &txDmaDesc[i + 1];
      //Reserved fields
      txDmaDesc[i].tdes4 = 0;
      txDmaDesc[i].tdes5 = 0;
      //Transmit frame time stamp
      txDmaDesc[i].tdes6 = 0;
      txDmaDesc[i].tdes7 = 0;
   }

   //The last descriptor is chained to the first entry
   txDmaDesc[i - 1].tdes3 = (uint32_t) &txDmaDesc[0];
   //Point to the very first descriptor
   txCurDmaDesc = &txDmaDesc[0];

   //Initialize RX DMA descriptor list
   for(i = 0; i < TM4C129_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = EMAC_RDES0_OWN;
      //Use chain structure rather than ring structure
      rxDmaDesc[i].rdes1 = EMAC_RDES1_RCH | (TM4C129_ETH_RX_BUFFER_SIZE & EMAC_RDES1_RBS1);
      //Receive buffer address
      rxDmaDesc[i].rdes2 = (uint32_t) rxBuffer[i];
      //Next descriptor address
      rxDmaDesc[i].rdes3 = (uint32_t) &rxDmaDesc[i + 1];
      //Extended status
      rxDmaDesc[i].rdes4 = 0;
      //Reserved field
      rxDmaDesc[i].rdes5 = 0;
      //Receive frame time stamp
      rxDmaDesc[i].rdes6 = 0;
      rxDmaDesc[i].rdes7 = 0;
   }

   //The last descriptor is chained to the first entry
   rxDmaDesc[i - 1].rdes3 = (uint32_t) &rxDmaDesc[0];
   //Point to the very first descriptor
   rxCurDmaDesc = &rxDmaDesc[0];

   //Start location of the TX descriptor list
   EMAC0_TXDLADDR_R = (uint32_t) txDmaDesc;
   //Start location of the RX descriptor list
   EMAC0_RXDLADDR_R = (uint32_t) rxDmaDesc;
}


/**
 * @brief TM4C129 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void tm4c129EthTick(NetInterface *interface)
{
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void tm4c129EthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   IntEnable(INT_EMAC0);
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void tm4c129EthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   IntDisable(INT_EMAC0);
}


/**
 * @brief TM4C129 Ethernet MAC interrupt service routine
 **/

void EMAC0_Handler(void)
{
   bool_t flag;
   uint32_t status;

   //Enter interrupt service routine
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read PHY status register
   status = EMAC0_EPHYRIS_R;

   //PHY interrupt?
   if(status & EMAC_EPHYRIS_INT)
   {
      //Disable PHY interrupt
      EMAC0_EPHYIM_R &= ~EMAC_EPHYIM_INT;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Read DMA status register
   status = EMAC0_DMARIS_R;

   //A packet has been transmitted?
   if(status & EMAC_DMARIS_TI)
   {
      //Clear TI interrupt flag
      EMAC0_DMARIS_R = EMAC_DMARIS_TI;

      //Check whether the TX buffer is available for writing
      if(!(txCurDmaDesc->tdes0 & EMAC_TDES0_OWN))
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //A packet has been received?
   if(status & EMAC_DMARIS_RI)
   {
      //Disable RIE interrupt
      EMAC0_DMAIM_R &= ~EMAC_DMAIM_RIE;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Clear NIS interrupt flag
   EMAC0_DMARIS_R = EMAC_DMARIS_NIS;

   //Leave interrupt service routine
   osExitIsr(flag);
}


/**
 * @brief TM4C129 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void tm4c129EthEventHandler(NetInterface *interface)
{
   error_t error;
   uint32_t status;

   //PHY interrupt?
   if(EMAC0_EPHYRIS_R & EMAC_EPHYRIS_INT)
   {
      //Clear PHY interrupt flag
      EMAC0_EPHYMISC_R = EMAC_EPHYMISC_INT;
      //Read PHY interrupt status register
      status = tm4c129EthReadPhyReg(EPHY_MISR1);

      //Check whether the link state has changed?
      if(status & EPHY_MISR1_LINKSTAT)
      {
         //Read BMSR register
         status = tm4c129EthReadPhyReg(EPHY_BMSR);

         //Check whether the link is up
         if(status & EPHY_BMSR_LINKSTAT)
         {
            //Read PHY status register
            status = tm4c129EthReadPhyReg(EPHY_STS);

            //Check current speed
            if(status & EPHY_STS_SPEED)
            {
               //10BASE-T operation
               interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
               //Update MAC configuration
               EMAC0_CFG_R &= ~EMAC_CFG_FES;
            }
            else
            {
               //100BASE-TX operation
               interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
               //Update MAC configuration
               EMAC0_CFG_R |= EMAC_CFG_FES;
            }

            //Check current duplex mode
            if(status & EPHY_STS_DUPLEX)
            {
               //Full-Duplex mode
               interface->duplexMode = NIC_FULL_DUPLEX_MODE;
               //Update MAC configuration
               EMAC0_CFG_R |= EMAC_CFG_DUPM;
            }
            else
            {
               //Half-Duplex mode
               interface->duplexMode = NIC_HALF_DUPLEX_MODE;
               //Update MAC configuration
               EMAC0_CFG_R &= ~EMAC_CFG_DUPM;
            }

            //Update link state
            interface->linkState = TRUE;
         }
         else
         {
            //Update link state
            interface->linkState = FALSE;
         }

         //Process link state change event
         nicNotifyLinkChange(interface);
      }
   }

   //Packet received?
   if(EMAC0_DMARIS_R & EMAC_DMARIS_RI)
   {
      //Clear interrupt flag
      EMAC0_DMARIS_R = EMAC_DMARIS_RI;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = tm4c129EthReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }

   //Re-enable DMA interrupts
   EMAC0_DMAIM_R |= EMAC_DMAIM_NIE | EMAC_DMAIM_RIE | EMAC_DMAIM_TIE;
   //Re-enable PHY interrupts
   EMAC0_EPHYIM_R |= EMAC_EPHYIM_INT;
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t tm4c129EthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > TM4C129_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if(txCurDmaDesc->tdes0 & EMAC_TDES0_OWN)
      return ERROR_FAILURE;

   //Copy user data to the transmit buffer
   netBufferRead((uint8_t *) txCurDmaDesc->tdes2, buffer, offset, length);

   //Write the number of bytes to send
   txCurDmaDesc->tdes1 = length & EMAC_TDES1_TBS1;
   //Set LS and FS flags as the data fits in a single buffer
   txCurDmaDesc->tdes0 |= EMAC_TDES0_LS | EMAC_TDES0_FS;
   //Give the ownership of the descriptor to the DMA
   txCurDmaDesc->tdes0 |= EMAC_TDES0_OWN;

   //Clear TU flag to resume processing
   EMAC0_DMARIS_R = EMAC_DMARIS_TU;
   //Instruct the DMA to poll the transmit descriptor list
   EMAC0_TXPOLLD_R = 0;

   //Point to the next descriptor in the list
   txCurDmaDesc = (Tm4c129TxDmaDesc *) txCurDmaDesc->tdes3;

   //Check whether the next buffer is available for writing
   if(!(txCurDmaDesc->tdes0 & EMAC_TDES0_OWN))
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

error_t tm4c129EthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;

   //The current buffer is available for reading?
   if(!(rxCurDmaDesc->rdes0 & EMAC_RDES0_OWN))
   {
      //FS and LS flags should be set
      if((rxCurDmaDesc->rdes0 & EMAC_RDES0_FS) && (rxCurDmaDesc->rdes0 & EMAC_RDES0_LS))
      {
         //Make sure no error occurred
         if(!(rxCurDmaDesc->rdes0 & EMAC_RDES0_ES))
         {
            //Retrieve the length of the frame
            n = (rxCurDmaDesc->rdes0 & EMAC_RDES0_FL) >> 16;
            //Limit the number of data to read
            n = MIN(n, TM4C129_ETH_RX_BUFFER_SIZE);

            //Pass the packet to the upper layer
            nicProcessPacket(interface, (uint8_t *) rxCurDmaDesc->rdes2, n);

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
      rxCurDmaDesc = (Tm4c129RxDmaDesc *) rxCurDmaDesc->rdes3;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Clear RU flag to resume processing
   EMAC0_DMARIS_R = EMAC_DMARIS_RU;
   //Instruct the DMA to poll the receive descriptor list
   EMAC0_RXPOLLD_R = 0;

   //Return status code
   return error;
}


/**
 * @brief Configure multicast MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t tm4c129EthSetMulticastFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint32_t hashTable[2];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating Tiva TM4C129 hash table...\r\n");

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
         crc = tm4c129EthCalcCrc(&entry->addr, sizeof(MacAddr));

         //The upper 6 bits in the CRC register are used to index the
         //contents of the hash table
         k = (crc >> 26) & 0x3F;

         //Update hash table contents
         hashTable[k / 32] |= (1 << (k % 32));
      }
   }

   //Write the hash table
   EMAC0_HASHTBLL_R = hashTable[0];
   EMAC0_HASHTBLH_R = hashTable[1];

   //Debug message
   TRACE_DEBUG("  HASHTBLL = %08" PRIX32 "\r\n", EMAC0_HASHTBLL_R);
   TRACE_DEBUG("  HASHTBLH = %08" PRIX32 "\r\n", EMAC0_HASHTBLH_R);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write PHY register
 * @param[in] regAddr Register address
 * @param[in] data Register value
 **/

void tm4c129EthWritePhyReg(uint8_t regAddr, uint16_t data)
{
   uint32_t value;

   //Take care not to alter MDC clock configuration
   value = EMAC0_MIIADDR_R & EMAC_MIIADDR_CR_M;
   //Set up a write operation
   value |= EMAC_MIIADDR_MIIW | EMAC_MIIADDR_MIIB;
   //The address of the integrated PHY is 0
   value |= (0 << EMAC_MIIADDR_PLA_S) & EMAC_MIIADDR_PLA_M;
   //Register address
   value |= (regAddr << EMAC_MIIADDR_MII_S) & EMAC_MIIADDR_MII_M;

   //Data to be written in the PHY register
   EMAC0_MIIDATA_R = data & EMAC_MIIDATA_DATA_M;

   //Start a write operation
   EMAC0_MIIADDR_R = value;
   //Wait for the write to complete
   while(EMAC0_MIIADDR_R & EMAC_MIIADDR_MIIB);
}


/**
 * @brief Read PHY register
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t tm4c129EthReadPhyReg(uint8_t regAddr)
{
   uint32_t value;

   //Take care not to alter MDC clock configuration
   value = EMAC0_MIIADDR_R & EMAC_MIIADDR_CR_M;
   //Set up a read operation
   value |= EMAC_MIIADDR_MIIB;
   //The address of the integrated PHY is 0
   value |= (0 << EMAC_MIIADDR_PLA_S) & EMAC_MIIADDR_PLA_M;
   //Register address
   value |= (regAddr << EMAC_MIIADDR_MII_S) & EMAC_MIIADDR_MII_M;

   //Start a read operation
   EMAC0_MIIADDR_R = value;
   //Wait for the read to complete
   while(EMAC0_MIIADDR_R & EMAC_MIIADDR_MIIB);

   //Return PHY register contents
   return EMAC0_MIIDATA_R & EMAC_MIIDATA_DATA_M;
}


/**
 * @brief Dump PHY registers for debugging purpose
 **/

void tm4c129EthDumpPhyReg(void)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i, tm4c129EthReadPhyReg(i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief CRC calculation
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t tm4c129EthCalcCrc(const void *data, size_t length)
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
