/**
 * @file rza2_eth1_driver.c
 * @brief RZ/A2 Ethernet MAC driver (ETHERC0 instance)
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
#include "iodefine.h"
#include "cpg_iobitmask.h"
#include "gpio_iobitmask.h"
#include "etherc_iobitmask.h"
#include "edmac_iobitmask.h"
#include "r_intc_lld_rza2m.h"
#include "core/net.h"
#include "drivers/mac/rza2_eth1_driver.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 32
#pragma location = RZA2_ETH1_RAM_SECTION
static uint8_t txBuffer[RZA2_ETH1_TX_BUFFER_COUNT][RZA2_ETH1_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 32
#pragma location = RZA2_ETH1_RAM_SECTION
static uint8_t rxBuffer[RZA2_ETH1_RX_BUFFER_COUNT][RZA2_ETH1_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 32
#pragma location = RZA2_ETH1_RAM_SECTION
static Rza2Eth1TxDmaDesc txDmaDesc[RZA2_ETH1_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 32
#pragma location = RZA2_ETH1_RAM_SECTION
static Rza2Eth1RxDmaDesc rxDmaDesc[RZA2_ETH1_RX_BUFFER_COUNT];

//ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[RZA2_ETH1_TX_BUFFER_COUNT][RZA2_ETH1_TX_BUFFER_SIZE]
   __attribute__((aligned(32), section(RZA2_ETH1_RAM_SECTION)));
//Receive buffer
static uint8_t rxBuffer[RZA2_ETH1_RX_BUFFER_COUNT][RZA2_ETH1_RX_BUFFER_SIZE]
   __attribute__((aligned(32), section(RZA2_ETH1_RAM_SECTION)));
//Transmit DMA descriptors
static Rza2Eth1TxDmaDesc txDmaDesc[RZA2_ETH1_TX_BUFFER_COUNT]
   __attribute__((aligned(32), section(RZA2_ETH1_RAM_SECTION)));
//Receive DMA descriptors
static Rza2Eth1RxDmaDesc rxDmaDesc[RZA2_ETH1_RX_BUFFER_COUNT]
   __attribute__((aligned(32), section(RZA2_ETH1_RAM_SECTION)));

#endif

//Current transmit descriptor
static uint_t txIndex;
//Current receive descriptor
static uint_t rxIndex;


/**
 * @brief RZ/A2 Ethernet MAC driver (ETHERC0 instance)
 **/

const NicDriver rza2Eth1Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   rza2Eth1Init,
   rza2Eth1Tick,
   rza2Eth1EnableIrq,
   rza2Eth1DisableIrq,
   rza2Eth1EventHandler,
   rza2Eth1SendPacket,
   rza2Eth1UpdateMacAddrFilter,
   rza2Eth1UpdateMacConfig,
   rza2Eth1WritePhyReg,
   rza2Eth1ReadPhyReg,
   TRUE,
   TRUE,
   TRUE,
   TRUE
};


/**
 * @brief RZ/A2 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t rza2Eth1Init(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing RZ/A2 Ethernet MAC (ETHERC0)...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //Enable the circuits shared by the Ethernet controllers and DMA controllers
   CPG.STBCR6.BYTE &= ~CPG_STBCR6_MSTP62;
   //Enable channel 0 Ethernet controller and channel 0 DMA controller
   CPG.STBCR6.BYTE &= ~CPG_STBCR6_MSTP65;

   //GPIO configuration
   rza2Eth1InitGpio(interface);

   //Reset EDMAC0 module
   EDMAC0.EDMR.LONG |= EDMAC_EDMR_SWR;
   //Wait for the reset to complete
   sleep(10);

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

   //Initialize DMA descriptor lists
   rza2Eth1InitDmaDesc(interface);

   //Maximum frame length that can be accepted
   ETHERC0.RFLR.LONG = RZA2_ETH1_RX_BUFFER_SIZE;
   //Set default inter packet gap (96-bit time)
   ETHERC0.IPGR.LONG = 0x14;

   //Set the upper 32 bits of the MAC address
   ETHERC0.MAHR.LONG = (interface->macAddr.b[0] << 24) | (interface->macAddr.b[1] << 16) |
      (interface->macAddr.b[2] << 8) | interface->macAddr.b[3];

   //Set the lower 16 bits of the MAC address
   ETHERC0.MALR.LONG = (interface->macAddr.b[4] << 8) | interface->macAddr.b[5];

   //Select little endian mode and set descriptor length (16 bytes)
   EDMAC0.EDMR.LONG = EDMAC_EDMR_DE | EDMAC_EDMR_DL_16;
   //Use store and forward mode
   EDMAC0.TFTR.LONG = 0;
   //Set transmit FIFO size (2048 bytes) and receive FIFO size (4096 bytes)
   EDMAC0.FDR.LONG = EDMAC_FDR_TFD_2048 | EDMAC_FDR_RFD_4096;
   //Enable continuous reception of multiple frames
   EDMAC0.RMCR.LONG = EDMAC_RMCR_RNR;
   //Select write-back complete interrupt mode and enable transmit interrupts
   EDMAC0.TRIMD.LONG = EDMAC_TRIMD_TIM | EDMAC_TRIMD_TIS;

   //Disable all ETHERC interrupts
   ETHERC0.ECSIPR.LONG = 0;
   //Enable the desired EDMAC interrupts
   EDMAC0.EESIPR.LONG = EDMAC_EESIPR_TWBIP | EDMAC_EESIPR_FRIP;

   //Register interrupt handler
   R_INTC_RegistIntFunc(INTC_ID_ETHER_EINT1, rza2Eth1IrqHandler);
   //Configure interrupt priority
   R_INTC_SetPriority(INTC_ID_ETHER_EINT1, RZA2_ETH1_IRQ_PRIORITY);

   //Enable transmission and reception
   ETHERC0.ECMR.LONG |= ETHERC_ECMR_TE | ETHERC_ECMR_RE;

   //Instruct the DMA to poll the receive descriptor list
   EDMAC0.EDRRR.LONG = EDMAC_EDRRR_RR;

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void rza2Eth1InitGpio(NetInterface *interface)
{
//RZ/A2-EVK evaluation board?
#if defined(USE_RZA2_EVK)
   //Unlock PFS registers
   GPIO.PWPR.BIT.B0WI = 0;
   GPIO.PWPR.BIT.PFSWE = 1;

   //Select RMII interface mode
   GPIO.PFENET.BIT.PHYMODE0 = 0;

   //Configure RMII0_TXD_EN (P6_1)
   GPIO.P61PFS.BIT.PSEL = 7;
   PORT6.PMR.BIT.PMR1 = 1;
   PORT6.DSCR.BIT.DSCR1 = 1;

   //Configure RMII0_TXD0 (P6_2)
   GPIO.P62PFS.BIT.PSEL = 7;
   PORT6.PMR.BIT.PMR2 = 1;
   PORT6.DSCR.BIT.DSCR2 = 1;

   //Configure RMII0_TXD1 (P6_3)
   GPIO.P63PFS.BIT.PSEL = 7;
   PORT6.PMR.BIT.PMR3 = 1;
   PORT6.DSCR.BIT.DSCR3 = 1;

   //Configure REF50CK0 (PE_0)
   GPIO.PE0PFS.BIT.PSEL = 7;
   PORTE.PMR.BIT.PMR0 = 1;
   PORTE.DSCR.BIT.DSCR0 = 1;

   //Configure RMII0_RXD0 (PE_1)
   GPIO.PE1PFS.BIT.PSEL = 7;
   PORTE.PMR.BIT.PMR1 = 1;
   PORTE.DSCR.BIT.DSCR1 = 1;

   //Configure RMII0_RXD1 (PE_2)
   GPIO.PE2PFS.BIT.PSEL = 7;
   PORTE.PMR.BIT.PMR2 = 1;
   PORTE.DSCR.BIT.DSCR2 = 1;

   //Configure RMII0_RXER (PE_3)
   GPIO.PE3PFS.BIT.PSEL = 7;
   PORTE.PMR.BIT.PMR3 = 1;
   PORTE.DSCR.BIT.DSCR3 = 1;

   //Configure RMII0_CRS_DV (PE_4)
   GPIO.PE4PFS.BIT.PSEL = 7;
   PORTE.PMR.BIT.PMR4 = 1;
   PORTE.DSCR.BIT.DSCR4 = 1;

   //Configure ET0_MDC (PE_5)
   GPIO.PE5PFS.BIT.PSEL = 1;
   PORTE.PMR.BIT.PMR5 = 1;
   PORTE.DSCR.BIT.DSCR5 = 1;

   //Configure ET0_MDIO (PE_6)
   GPIO.PE6PFS.BIT.PSEL = 1;
   PORTE.PMR.BIT.PMR6 = 1;
   PORTE.DSCR.BIT.DSCR6 = 1;

   //Lock PFS registers
   GPIO.PWPR.BIT.PFSWE = 0;
   GPIO.PWPR.BIT.B0WI = 1;
#endif
}


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void rza2Eth1InitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX descriptors
   for(i = 0; i < RZA2_ETH1_TX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the application
      txDmaDesc[i].td0 = 0;
      //Transmit buffer length
      txDmaDesc[i].td1 = 0;
      //Transmit buffer address
      txDmaDesc[i].td2 = RZA2_ETH1_GET_PHYSICAL_ADDR(txBuffer[i]);
      //Clear padding field
      txDmaDesc[i].padding = 0;
   }

   //Mark the last descriptor entry with the TDLE flag
   txDmaDesc[i - 1].td0 |= EDMAC_TD0_TDLE;
   //Initialize TX descriptor index
   txIndex = 0;

   //Initialize RX descriptors
   for(i = 0; i < RZA2_ETH1_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rd0 = EDMAC_RD0_RACT;
      //Receive buffer length
      rxDmaDesc[i].rd1 = (RZA2_ETH1_RX_BUFFER_SIZE << 16) & EDMAC_RD1_RBL;
      //Receive buffer address
      rxDmaDesc[i].rd2 = RZA2_ETH1_GET_PHYSICAL_ADDR(rxBuffer[i]);
      //Clear padding field
      rxDmaDesc[i].padding = 0;
   }

   //Mark the last descriptor entry with the RDLE flag
   rxDmaDesc[i - 1].rd0 |= EDMAC_RD0_RDLE;
   //Initialize RX descriptor index
   rxIndex = 0;

   //Start address of the TX descriptor list
   EDMAC0.TDLAR.LONG = RZA2_ETH1_GET_PHYSICAL_ADDR(txDmaDesc);
   //Start address of the RX descriptor list
   EDMAC0.RDLAR.LONG = RZA2_ETH1_GET_PHYSICAL_ADDR(rxDmaDesc);
}


/**
 * @brief RZ/A2 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to handle periodic
 * operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void rza2Eth1Tick(NetInterface *interface)
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

void rza2Eth1EnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   R_INTC_Enable(INTC_ID_ETHER_EINT1);

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

void rza2Eth1DisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   R_INTC_Disable(INTC_ID_ETHER_EINT1);

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
 * @brief RZ/A2 Ethernet MAC interrupt service routine
 * @param[in] intSense Unused parameter
 **/

void rza2Eth1IrqHandler(uint32_t intSense)
{
   bool_t flag;
   uint32_t status;

   //Interrupt service routine prologue
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read interrupt status register
   status = EDMAC0.EESR.LONG;

   //Packet transmitted?
   if((status & EDMAC_EESR_TWB) != 0)
   {
      //Clear TWB interrupt flag
      EDMAC0.EESR.LONG = EDMAC_EESR_TWB;

      //Check whether the TX buffer is available for writing
      if((txDmaDesc[txIndex].td0 & EDMAC_TD0_TACT) == 0)
      {
         //Notify the TCP/IP stack that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }

   //Packet received?
   if((status & EDMAC_EESR_FR) != 0)
   {
      //Clear FR interrupt flag
      EDMAC0.EESR.LONG = EDMAC_EESR_FR;

      //Set event flag
      nicDriverInterface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Interrupt service routine epilogue
   osExitIsr(flag);
}


/**
 * @brief RZ/A2 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void rza2Eth1EventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = rza2Eth1ReceivePacket(interface);

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

error_t rza2Eth1SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   //Retrieve the length of the packet
   size_t length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > RZA2_ETH1_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if((txDmaDesc[txIndex].td0 & EDMAC_TD0_TACT) != 0)
   {
      return ERROR_FAILURE;
   }

   //Copy user data to the transmit buffer
   netBufferRead(txBuffer[txIndex], buffer, offset, length);

   //Write the number of bytes to send
   txDmaDesc[txIndex].td1 = (length << 16) & EDMAC_TD1_TBL;

   //Check current index
   if(txIndex < (RZA2_ETH1_TX_BUFFER_COUNT - 1))
   {
      //Give the ownership of the descriptor to the DMA engine
      txDmaDesc[txIndex].td0 = EDMAC_TD0_TACT | EDMAC_TD0_TFP_SOF |
         EDMAC_TD0_TFP_EOF | EDMAC_TD0_TWBI;

      //Point to the next descriptor
      txIndex++;
   }
   else
   {
      //Give the ownership of the descriptor to the DMA engine
      txDmaDesc[txIndex].td0 = EDMAC_TD0_TACT | EDMAC_TD0_TDLE |
         EDMAC_TD0_TFP_SOF | EDMAC_TD0_TFP_EOF | EDMAC_TD0_TWBI;

      //Wrap around
      txIndex = 0;
   }

   //Instruct the DMA to poll the transmit descriptor list
   EDMAC0.EDTRR.LONG = EDMAC_EDTRR_TR;

   //Check whether the next buffer is available for writing
   if((txDmaDesc[txIndex].td0 & EDMAC_TD0_TACT) == 0)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
   }

   //Successful write operation
   return NO_ERROR;
}


/**
 * @brief Receive a packet
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t rza2Eth1ReceivePacket(NetInterface *interface)
{
   static uint32_t temp[RZA2_ETH1_RX_BUFFER_SIZE / 4];
   error_t error;
   size_t n;
   NetRxAncillary ancillary;

   //Current buffer available for reading?
   if((rxDmaDesc[rxIndex].rd0 & EDMAC_RD0_RACT) == 0)
   {
      //SOF and EOF flags should be set
      if((rxDmaDesc[rxIndex].rd0 & EDMAC_RD0_RFP_SOF) != 0 &&
         (rxDmaDesc[rxIndex].rd0 & EDMAC_RD0_RFP_EOF) != 0)
      {
         //Make sure no error occurred
         if((rxDmaDesc[rxIndex].rd0 & (EDMAC_RD0_RFS_MASK & ~EDMAC_RD0_RFS_RMAF)) == 0)
         {
            //Retrieve the length of the frame
            n = rxDmaDesc[rxIndex].rd1 & EDMAC_RD1_RFL;
            //Limit the number of data to read
            n = MIN(n, RZA2_ETH1_RX_BUFFER_SIZE);

            //Copy data from the receive buffer
            osMemcpy(temp, rxBuffer[rxIndex], n);

            //Additional options can be passed to the stack along with the packet
            ancillary = NET_DEFAULT_RX_ANCILLARY;

            //Pass the packet to the upper layer
            nicProcessPacket(interface, (uint8_t *) temp, n, &ancillary);

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

      //Check current index
      if(rxIndex < (RZA2_ETH1_RX_BUFFER_COUNT - 1))
      {
         //Give the ownership of the descriptor back to the DMA
         rxDmaDesc[rxIndex].rd0 = EDMAC_RD0_RACT;
         //Point to the next descriptor
         rxIndex++;
      }
      else
      {
         //Give the ownership of the descriptor back to the DMA
         rxDmaDesc[rxIndex].rd0 = EDMAC_RD0_RACT | EDMAC_RD0_RDLE;
         //Wrap around
         rxIndex = 0;
      }

      //Instruct the DMA to poll the receive descriptor list
      EDMAC0.EDRRR.LONG = EDMAC_EDRRR_RR;
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

error_t rza2Eth1UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   bool_t acceptMulticast;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Promiscuous mode?
   if(interface->promiscuous)
   {
      //Accept all frames regardless of their destination address
      ETHERC0.ECMR.LONG |= ETHERC_ECMR_PRM;
   }
   else
   {
      //Disable promiscuous mode
      ETHERC0.ECMR.LONG &= ~ETHERC_ECMR_PRM;

      //Set the upper 32 bits of the MAC address
      ETHERC0.MAHR.LONG = (interface->macAddr.b[0] << 24) | (interface->macAddr.b[1] << 16) |
         (interface->macAddr.b[2] << 8) | interface->macAddr.b[3];

      //Set the lower 16 bits of the MAC address
      ETHERC0.MALR.LONG = (interface->macAddr.b[4] << 8) | interface->macAddr.b[5];

      //This flag will be set if multicast addresses should be accepted
      acceptMulticast = FALSE;

      //The MAC address filter contains the list of MAC addresses to accept
      //when receiving an Ethernet frame
      for(i = 0; i < MAC_ADDR_FILTER_SIZE; i++)
      {
         //Valid entry?
         if(interface->macAddrFilter[i].refCount > 0)
         {
            //Accept multicast addresses
            acceptMulticast = TRUE;
            //We are done
            break;
         }
      }

      //Enable or disable the reception of multicast frames
      if(acceptMulticast || interface->acceptAllMulticast)
      {
         EDMAC0.EESR.LONG |= EDMAC_EESR_RMAF;
      }
      else
      {
         EDMAC0.EESR.LONG &= ~EDMAC_EESR_RMAF;
      }
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t rza2Eth1UpdateMacConfig(NetInterface *interface)
{
   uint32_t mode;

   //Read ETHERC mode register
   mode = ETHERC0.ECMR.LONG;

   //10BASE-T or 100BASE-TX operation mode?
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      mode |= ETHERC_ECMR_RTM;
   }
   else
   {
      mode &= ~ETHERC_ECMR_RTM;
   }

   //Half-duplex or full-duplex mode?
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      mode |= ETHERC_ECMR_DM;
   }
   else
   {
      mode &= ~ETHERC_ECMR_DM;
   }

   //Update ETHERC mode register
   ETHERC0.ECMR.LONG = mode;

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

void rza2Eth1WritePhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr, uint16_t data)
{
   //Synchronization pattern
   rza2Eth1WriteSmi(SMI_SYNC, 32);
   //Start of frame
   rza2Eth1WriteSmi(SMI_START, 2);
   //Set up a write operation
   rza2Eth1WriteSmi(opcode, 2);
   //Write PHY address
   rza2Eth1WriteSmi(phyAddr, 5);
   //Write register address
   rza2Eth1WriteSmi(regAddr, 5);
   //Turnaround
   rza2Eth1WriteSmi(SMI_TA, 2);
   //Write register value
   rza2Eth1WriteSmi(data, 16);
   //Release MDIO
   rza2Eth1ReadSmi(1);
}


/**
 * @brief Read PHY register
 * @param[in] opcode Access type (2 bits)
 * @param[in] phyAddr PHY address (5 bits)
 * @param[in] regAddr Register address (5 bits)
 * @return Register value
 **/

uint16_t rza2Eth1ReadPhyReg(uint8_t opcode, uint8_t phyAddr,
   uint8_t regAddr)
{
   uint16_t data;

   //Synchronization pattern
   rza2Eth1WriteSmi(SMI_SYNC, 32);
   //Start of frame
   rza2Eth1WriteSmi(SMI_START, 2);
   //Set up a read operation
   rza2Eth1WriteSmi(opcode, 2);
   //Write PHY address
   rza2Eth1WriteSmi(phyAddr, 5);
   //Write register address
   rza2Eth1WriteSmi(regAddr, 5);
   //Turnaround to avoid contention
   rza2Eth1ReadSmi(1);
   //Read register value
   data = rza2Eth1ReadSmi(16);
   //Force the PHY to release the MDIO pin
   rza2Eth1ReadSmi(1);

   //Return PHY register contents
   return data;
}


/**
 * @brief SMI write operation
 * @param[in] data Raw data to be written
 * @param[in] length Number of bits to be written
 **/

void rza2Eth1WriteSmi(uint32_t data, uint_t length)
{
   //Skip the most significant bits since they are meaningless
   data <<= 32 - length;

   //Configure MDIO as an output
   ETHERC0.PIR.LONG |= ETHERC_PIR_MMD;

   //Write the specified number of bits
   while(length--)
   {
      //Write MDIO
      if((data & 0x80000000) != 0)
      {
         ETHERC0.PIR.LONG |= ETHERC_PIR_MDO;
      }
      else
      {
         ETHERC0.PIR.LONG &= ~ETHERC_PIR_MDO;
      }

      //Assert MDC
      usleep(1);
      ETHERC0.PIR.LONG |= ETHERC_PIR_MDC;
      //Deassert MDC
      usleep(1);
      ETHERC0.PIR.LONG &= ~ETHERC_PIR_MDC;

      //Rotate data
      data <<= 1;
   }
}


/**
 * @brief SMI read operation
 * @param[in] length Number of bits to be read
 * @return Data resulting from the MDIO read operation
 **/

uint32_t rza2Eth1ReadSmi(uint_t length)
{
   uint32_t data = 0;

   //Configure MDIO as an input
   ETHERC0.PIR.LONG &= ~ETHERC_PIR_MMD;

   //Read the specified number of bits
   while(length--)
   {
      //Rotate data
      data <<= 1;

      //Assert MDC
      ETHERC0.PIR.LONG |= ETHERC_PIR_MDC;
      usleep(1);
      //Deassert MDC
      ETHERC0.PIR.LONG &= ~ETHERC_PIR_MDC;
      usleep(1);

      //Check MDIO state
      if((ETHERC0.PIR.LONG & ETHERC_PIR_MDI) != 0)
      {
         data |= 0x01;
      }
   }

   //Return the received data
   return data;
}
