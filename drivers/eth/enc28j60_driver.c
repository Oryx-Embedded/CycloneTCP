/**
 * @file enc28j60_driver.c
 * @brief ENC28J60 Ethernet controller
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
#include <limits.h>
#include "core/net.h"
#include "drivers/eth/enc28j60_driver.h"
#include "debug.h"


/**
 * @brief ENC28J60 driver
 **/

const NicDriver enc28j60Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   enc28j60Init,
   enc28j60Tick,
   enc28j60EnableIrq,
   enc28j60DisableIrq,
   enc28j60EventHandler,
   enc28j60SendPacket,
   enc28j60SetMulticastFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief ENC28J60 controller initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t enc28j60Init(NetInterface *interface)
{
   uint8_t revisionId;
   Enc28j60Context *context;

   //Debug message
   TRACE_INFO("Initializing ENC28J60 Ethernet controller...\r\n");

   //Initialize SPI
   interface->spiDriver->init();
   //Initialize external interrupt line
   interface->extIntDriver->init();

   //Issue a system reset
   enc28j60SoftReset(interface);

   //After issuing the reset command, wait at least 1ms in firmware
   //for the device to be ready
   sleep(10);

   //Point to the driver context
   context = (Enc28j60Context *) interface->nicContext;

   //Initialize driver specific variables
   context->currentBank = UINT16_MAX;
   context->nextPacket = ENC28J60_RX_BUFFER_START;

   //Allocate RX buffer
   context->rxBuffer = memPoolAlloc(ETH_MAX_FRAME_SIZE);
   //Failed to allocate memory?
   if(context->rxBuffer == NULL)
      return ERROR_OUT_OF_MEMORY;

   //Read silicon revision ID
   revisionId = enc28j60ReadReg(interface, ENC28J60_REG_EREVID);

   //Debug message
   TRACE_INFO("ENC28J60 revision ID: 0x%02X\r\n", revisionId);

   //Disable CLKOUT output
   enc28j60WriteReg(interface, ENC28J60_REG_ECOCON, 0x00);

   //Set the MAC address
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR1, interface->macAddr.b[0]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR2, interface->macAddr.b[1]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR3, interface->macAddr.b[2]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR4, interface->macAddr.b[3]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR5, interface->macAddr.b[4]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR6, interface->macAddr.b[5]);

   //Set receive buffer location
   enc28j60WriteReg(interface, ENC28J60_REG_ERXSTL, LSB(ENC28J60_RX_BUFFER_START));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXSTH, MSB(ENC28J60_RX_BUFFER_START));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXNDL, LSB(ENC28J60_RX_BUFFER_STOP));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXNDH, MSB(ENC28J60_RX_BUFFER_STOP));

   //The ERXRDPT register defines a location within the FIFO
   //where the receive hardware is forbidden to write to
   enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTL, LSB(ENC28J60_RX_BUFFER_STOP));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTH, MSB(ENC28J60_RX_BUFFER_STOP));

   //Configure the receive filters
   enc28j60WriteReg(interface, ENC28J60_REG_ERXFCON, ERXFCON_UCEN |
      ERXFCON_CRCEN | ERXFCON_HTEN | ERXFCON_BCEN);

   //Initialize the hash table
   enc28j60WriteReg(interface, ENC28J60_REG_EHT0, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT1, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT2, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT3, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT4, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT5, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT6, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT7, 0x00);

   //Pull the MAC out of reset
   enc28j60WriteReg(interface, ENC28J60_REG_MACON2, 0x00);

   //Enable the MAC to receive frames
   enc28j60WriteReg(interface, ENC28J60_REG_MACON1,
      MACON1_TXPAUS | MACON1_RXPAUS | MACON1_MARXEN);

   //Enable automatic padding to at least 60 bytes, always append a valid CRC
   //and check frame length. MAC can operate in half-duplex or full-duplex mode
#if (ENC28J60_FULL_DUPLEX_SUPPORT == ENABLED)
   enc28j60WriteReg(interface, ENC28J60_REG_MACON3, MACON3_PADCFG(1) |
      MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX);
#else
   enc28j60WriteReg(interface, ENC28J60_REG_MACON3, MACON3_PADCFG(1) |
      MACON3_TXCRCEN | MACON3_FRMLNEN);
#endif

   //When the medium is occupied, the MAC will wait indefinitely for it to
   //become free when attempting to transmit
   enc28j60WriteReg(interface, ENC28J60_REG_MACON4, MACON4_DEFER);

   //Maximum frame length that can be received or transmitted (1518 bytes)
   enc28j60WriteReg(interface, ENC28J60_REG_MAMXFLL, LSB(1518));
   enc28j60WriteReg(interface, ENC28J60_REG_MAMXFLH, MSB(1518));

   //Configure the back-to-back inter-packet gap register
#if (ENC28J60_FULL_DUPLEX_SUPPORT == ENABLED)
   enc28j60WriteReg(interface, ENC28J60_REG_MABBIPG, 0x15);
#else
   enc28j60WriteReg(interface, ENC28J60_REG_MABBIPG, 0x12);
#endif

   //Configure the non-back-to-back inter-packet gap register
   enc28j60WriteReg(interface, ENC28J60_REG_MAIPGL, 0x12);
   enc28j60WriteReg(interface, ENC28J60_REG_MAIPGH, 0x0C);

   //Collision window register
   enc28j60WriteReg(interface, ENC28J60_REG_MACLCON2, 63);

   //Set the PHY to the proper duplex mode
#if (ENC28J60_FULL_DUPLEX_SUPPORT == ENABLED)
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHCON1, PHCON1_PDPXMD);
#else
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHCON1, 0x0000);
#endif

   //Disable half-duplex loopback in PHY
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHCON2, PHCON2_HDLDIS);

   //LEDA displays link status and LEDB displays TX/RX activity
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHLCON,
      PHLCON_LACFG(4) | PHLCON_LBCFG(7) | PHLCON_LFRQ(0) | PHLCON_STRCH);

   //Clear interrupt flags
   enc28j60WriteReg(interface, ENC28J60_REG_EIR, 0x00);

   //Configure interrupts as desired
   enc28j60WriteReg(interface, ENC28J60_REG_EIE, EIE_INTIE |
      EIE_PKTIE | EIE_LINKIE | EIE_TXIE | EIE_TXERIE);

   //Configure PHY interrupts as desired
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHIE,
      PHIE_PLNKIE | PHIE_PGEIE);

   //Set RXEN to enable reception
   enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_RXEN);

   //Dump registers for debugging purpose
   enc28j60DumpReg(interface);
   enc28j60DumpPhyReg(interface);

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Force the TCP/IP stack to poll the link state at startup
   interface->nicEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief ENC28J60 timer handler
 * @param[in] interface Underlying network interface
 **/

void enc28j60Tick(NetInterface *interface)
{
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void enc28j60EnableIrq(NetInterface *interface)
{
   //Enable interrupts
   interface->extIntDriver->enableIrq();
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void enc28j60DisableIrq(NetInterface *interface)
{
   //Disable interrupts
   interface->extIntDriver->disableIrq();
}


/**
 * @brief ENC28J60 interrupt service routine
 * @param[in] interface Underlying network interface
 * @return TRUE if a higher priority task must be woken. Else FALSE is returned
 **/

bool_t enc28j60IrqHandler(NetInterface *interface)
{
   bool_t flag;
   uint8_t status;

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Clear the INTIE bit, immediately after an interrupt event
   enc28j60ClearBit(interface, ENC28J60_REG_EIE, EIE_INTIE);

   //Read interrupt status register
   status = enc28j60ReadReg(interface, ENC28J60_REG_EIR);

   //Link status change?
   if(status & EIR_LINKIF)
   {
      //Disable LINKIE interrupt
      enc28j60ClearBit(interface, ENC28J60_REG_EIE, EIE_LINKIE);

      //Set event flag
      interface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Packet received?
   if(status & EIR_PKTIF)
   {
      //Disable PKTIE interrupt
      enc28j60ClearBit(interface, ENC28J60_REG_EIE, EIE_PKTIE);

      //Set event flag
      interface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Packet transmission complete?
   if(status & (EIR_TXIF | EIE_TXERIE))
   {
      //Clear interrupt flags
      enc28j60ClearBit(interface, ENC28J60_REG_EIR, EIR_TXIF | EIE_TXERIE);

      //Notify the TCP/IP stack that the transmitter is ready to send
      flag |= osSetEventFromIsr(&interface->nicTxEvent);
   }

   //Once the interrupt has been serviced, the INTIE bit
   //is set again to re-enable interrupts
   enc28j60SetBit(interface, ENC28J60_REG_EIE, EIE_INTIE);

   //A higher priority task must be woken?
   return flag;
}


/**
 * @brief ENC28J60 event handler
 * @param[in] interface Underlying network interface
 **/

void enc28j60EventHandler(NetInterface *interface)
{
   error_t error;
   uint16_t status;
   uint16_t value;

   //Read interrupt status register
   status = enc28j60ReadReg(interface, ENC28J60_REG_EIR);

   //Check whether the link state has changed
   if(status & EIR_LINKIF)
   {
      //Clear PHY interrupts flags
      enc28j60ReadPhyReg(interface, ENC28J60_PHY_REG_PHIR);
      //Clear interrupt flag
      enc28j60ClearBit(interface, ENC28J60_REG_EIR, EIR_LINKIF);
      //Read PHY status register
      value = enc28j60ReadPhyReg(interface, ENC28J60_PHY_REG_PHSTAT2);

      //Check link state
      if(value & PHSTAT2_LSTAT)
      {
         //Link speed
         interface->linkSpeed = NIC_LINK_SPEED_10MBPS;

#if (ENC28J60_FULL_DUPLEX_SUPPORT == ENABLED)
         //Full-duplex mode
         interface->duplexMode = NIC_FULL_DUPLEX_MODE;
#else
         //Half-duplex mode
         interface->duplexMode = NIC_HALF_DUPLEX_MODE;
#endif
         //Link is up
         interface->linkState = TRUE;
      }
      else
      {
         //Link is down
         interface->linkState = FALSE;
      }

      //Process link state change event
      nicNotifyLinkChange(interface);
   }

   //Check whether a packet has been received?
   if(status & EIR_PKTIF)
   {
      //Clear interrupt flag
      enc28j60ClearBit(interface, ENC28J60_REG_EIR, EIR_PKTIF);

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = enc28j60ReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }

   //Re-enable LINKIE and PKTIE interrupts
   enc28j60SetBit(interface, ENC28J60_REG_EIE, EIE_LINKIE | EIE_PKTIE);
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t enc28j60SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
{
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > 1536)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the link is up before transmitting the frame
   if(!interface->linkState)
   {
      //The transmitter can accept another packet
      osSetEventFromIsr(&interface->nicTxEvent);
      //Drop current packet
      return NO_ERROR;
   }

   //It is recommended to reset the transmit logic before
   //attempting to transmit a packet
   enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_TXRST);
   enc28j60ClearBit(interface, ENC28J60_REG_ECON1, ECON1_TXRST);

   //Interrupt flags should be cleared after the reset is completed
   enc28j60ClearBit(interface, ENC28J60_REG_EIR, EIR_TXIF | EIR_TXERIF);

   //Set transmit buffer location
   enc28j60WriteReg(interface, ENC28J60_REG_ETXSTL, LSB(ENC28J60_TX_BUFFER_START));
   enc28j60WriteReg(interface, ENC28J60_REG_ETXSTH, MSB(ENC28J60_TX_BUFFER_START));

   //Point to start of transmit buffer
   enc28j60WriteReg(interface, ENC28J60_REG_EWRPTL, LSB(ENC28J60_TX_BUFFER_START));
   enc28j60WriteReg(interface, ENC28J60_REG_EWRPTH, MSB(ENC28J60_TX_BUFFER_START));

   //Copy the data to the transmit buffer
   enc28j60WriteBuffer(interface, buffer, offset);

   //ETXND should point to the last byte in the data payload
   enc28j60WriteReg(interface, ENC28J60_REG_ETXNDL, LSB(ENC28J60_TX_BUFFER_START + length));
   enc28j60WriteReg(interface, ENC28J60_REG_ETXNDH, MSB(ENC28J60_TX_BUFFER_START + length));

   //Start transmission
   enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_TXRTS);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Receive a packet
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t enc28j60ReceivePacket(NetInterface *interface)
{
   error_t error;
   uint16_t n;
   uint16_t status;
   Enc28j60Context *context;

   //Point to the driver context
   context = (Enc28j60Context *) interface->nicContext;

   //Any packet pending in the receive buffer?
   if(enc28j60ReadReg(interface, ENC28J60_REG_EPKTCNT))
   {
      //Point to the start of the received packet
      enc28j60WriteReg(interface, ENC28J60_REG_ERDPTL, LSB(context->nextPacket));
      enc28j60WriteReg(interface, ENC28J60_REG_ERDPTH, MSB(context->nextPacket));

      //Read the first two bytes, which are the address of the next packet
      enc28j60ReadBuffer(interface, (uint8_t *) &context->nextPacket, sizeof(uint16_t));
      //Get the length of the received frame in bytes
      enc28j60ReadBuffer(interface, (uint8_t *) &n, sizeof(uint16_t));
      //Read the receive status vector (RSV)
      enc28j60ReadBuffer(interface, (uint8_t *) &status, sizeof(uint16_t));

      //Make sure no error occurred
      if(status & RSV_RECEIVED_OK)
      {
         //Limit the number of data to read
         n = MIN(n, ETH_MAX_FRAME_SIZE);
         //Read the Ethernet frame
         enc28j60ReadBuffer(interface, context->rxBuffer, n);
         //Valid packet received
         error = NO_ERROR;
      }
      else
      {
         //The received packet contains an error
         error = ERROR_INVALID_PACKET;
      }

      //Advance the ERXRDPT pointer, taking care to wrap back at the
      //end of the received memory buffer
      if(context->nextPacket == ENC28J60_RX_BUFFER_START)
      {
         enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTL, LSB(ENC28J60_RX_BUFFER_STOP));
         enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTH, MSB(ENC28J60_RX_BUFFER_STOP));
      }
      else
      {
         enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTL, LSB(context->nextPacket - 1));
         enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTH, MSB(context->nextPacket - 1));
      }

      //Decrement the packet counter
      enc28j60SetBit(interface, ENC28J60_REG_ECON2, ECON2_PKTDEC);
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Check whether a valid packet has been received
   if(!error)
   {
      //Pass the packet to the upper layer
      nicProcessPacket(interface, context->rxBuffer, n);
   }

   //Return status code
   return error;
}


/**
 * @brief Configure multicast MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t enc28j60SetMulticastFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint8_t hashTable[8];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating ENC28J60 hash table...\r\n");

   //Clear hash table
   memset(hashTable, 0, sizeof(hashTable));

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
         crc = enc28j60CalcCrc(&entry->addr, sizeof(MacAddr));
         //Calculate the corresponding index in the table
         k = (crc >> 23) & 0x3F;
         //Update hash table contents
         hashTable[k / 8] |= (1 << (k % 8));
      }
   }

   //Write the hash table to the ENC28J60 controller
   enc28j60WriteReg(interface, ENC28J60_REG_EHT0, hashTable[0]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT1, hashTable[1]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT2, hashTable[2]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT3, hashTable[3]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT4, hashTable[4]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT5, hashTable[5]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT6, hashTable[6]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT7, hashTable[7]);

   //Debug message
   TRACE_DEBUG("  EHT0 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT0));
   TRACE_DEBUG("  EHT1 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT1));
   TRACE_DEBUG("  EHT2 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT2));
   TRACE_DEBUG("  EHT3 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT3));
   TRACE_DEBUG("  EHT0 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT4));
   TRACE_DEBUG("  EHT1 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT5));
   TRACE_DEBUG("  EHT2 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT6));
   TRACE_DEBUG("  EHT3 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT7));

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief ENC28J60 controller reset
 * @param[in] interface Underlying network interface
 **/

void enc28j60SoftReset(NetInterface *interface)
{
   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode
   interface->spiDriver->transfer(ENC28J60_CMD_SRC);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Bank selection
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 **/

void enc28j60SelectBank(NetInterface *interface, uint16_t address)
{
   uint16_t bank;
   Enc28j60Context *context;

   //Point to the driver context
   context = (Enc28j60Context *) interface->nicContext;

   //Get the bank number from the specified address
   bank = address & REG_BANK_MASK;

   //Rewrite the bank number only if a change is detected
   if(bank != context->currentBank)
   {
      //Select specified bank
      switch(bank)
      {
      case BANK_0:
         //Select bank 0
         enc28j60ClearBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL1 | ECON1_BSEL0);
         break;
      case BANK_1:
         //Select bank 1
         enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL0);
         enc28j60ClearBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL1);
         break;
      case BANK_2:
         //Select bank 2
         enc28j60ClearBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL0);
         enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL1);
         break;
      case BANK_3:
         //Select bank 3
         enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_BSEL1 | ECON1_BSEL0);
         break;
      default:
         //Invalid bank
         break;
      }

      //Save bank number
      context->currentBank = bank;
   }
}


/**
 * @brief Write ENC28J60 register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @param[in] data Register value
 **/

void enc28j60WriteReg(NetInterface *interface, uint16_t address, uint8_t data)
{
   //Make sure the corresponding bank is selected
   enc28j60SelectBank(interface, address);

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode and register address
   interface->spiDriver->transfer(ENC28J60_CMD_WCR | (address & REG_ADDR_MASK));
   //Write register value
   interface->spiDriver->transfer(data);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Read ENC28J60 register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @return Register value
 **/

uint8_t enc28j60ReadReg(NetInterface *interface, uint16_t address)
{
   uint16_t data;

   //Make sure the corresponding bank is selected
   enc28j60SelectBank(interface, address);

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode and register address
   interface->spiDriver->transfer(ENC28J60_CMD_RCR | (address & REG_ADDR_MASK));

   //When reading MAC or MII registers, a dummy byte is first shifted out
   if((address & REG_TYPE_MASK) != ETH_REG_TYPE)
      interface->spiDriver->transfer(0x00);

   //Read register contents
   data = interface->spiDriver->transfer(0x00);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();

   //Return register contents
   return data;
}


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void enc28j60WritePhyReg(NetInterface *interface, uint16_t address, uint16_t data)
{
   //Write register address
   enc28j60WriteReg(interface, ENC28J60_REG_MIREGADR, address & REG_ADDR_MASK);

   //Write the lower 8 bits
   enc28j60WriteReg(interface, ENC28J60_REG_MIWRL, LSB(data));
   //Write the upper 8 bits
   enc28j60WriteReg(interface, ENC28J60_REG_MIWRH, MSB(data));

   //Wait until the PHY register has been written
   while(enc28j60ReadReg(interface, ENC28J60_REG_MISTAT) & MISTAT_BUSY);
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t enc28j60ReadPhyReg(NetInterface *interface, uint16_t address)
{
   uint16_t data;

   //Write register address
   enc28j60WriteReg(interface, ENC28J60_REG_MIREGADR, address & REG_ADDR_MASK);

   //Start read operation
   enc28j60WriteReg(interface, ENC28J60_REG_MICMD, MICMD_MIIRD);
   //Wait for the read operation to complete
   while(enc28j60ReadReg(interface, ENC28J60_REG_MISTAT) & MISTAT_BUSY);
   //Clear command register
   enc28j60WriteReg(interface, ENC28J60_REG_MICMD, 0);

   //Read the lower 8 bits
   data = enc28j60ReadReg(interface, ENC28J60_REG_MIRDL);
   //Read the upper 8 bits
   data |= enc28j60ReadReg(interface, ENC28J60_REG_MIRDH) << 8;

   //Return register contents
   return data;
}


/**
 * @brief Write SRAM buffer
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to be written
 * @param[in] offset Offset to the first data byte
 **/

void enc28j60WriteBuffer(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
{
   uint_t i;
   size_t j;
   size_t n;
   uint8_t *p;

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode
   interface->spiDriver->transfer(ENC28J60_CMD_WBM);
   //Write per-packet control byte
   interface->spiDriver->transfer(0x00);

   //Loop through data chunks
   for(i = 0; i < buffer->chunkCount; i++)
   {
      //Is there any data to copy from the current chunk?
      if(offset < buffer->chunk[i].length)
      {
         //Point to the first byte to be read
         p = (uint8_t *) buffer->chunk[i].address + offset;
         //Compute the number of bytes to copy at a time
         n = buffer->chunk[i].length - offset;

         //Copy data to SRAM buffer
         for(j = 0; j < n; j++)
            interface->spiDriver->transfer(p[j]);

         //Process the next block from the start
         offset = 0;
      }
      else
      {
         //Skip the current chunk
         offset -= buffer->chunk[i].length;
      }
   }

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Read SRAM buffer
 * @param[in] interface Underlying network interface
 * @param[in] data Buffer where to store the incoming data
 * @param[in] length Number of data to read
 **/

void enc28j60ReadBuffer(NetInterface *interface,
   uint8_t *data, size_t length)
{
   size_t i;

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode
   interface->spiDriver->transfer(ENC28J60_CMD_RBM);

   //Copy data from SRAM buffer
   for(i = 0; i < length; i++)
      data[i] = interface->spiDriver->transfer(0x00);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Set bit field
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @param[in] mask Bits to set in the target register
 **/

void enc28j60SetBit(NetInterface *interface, uint16_t address, uint16_t mask)
{
   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode and register address
   interface->spiDriver->transfer(ENC28J60_CMD_BFS | (address & REG_ADDR_MASK));
   //Write bit mask
   interface->spiDriver->transfer(mask);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Clear bit field
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @param[in] mask Bits to clear in the target register
 **/

void enc28j60ClearBit(NetInterface *interface, uint16_t address, uint16_t mask)
{
   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode and register address
   interface->spiDriver->transfer(ENC28J60_CMD_BFC | (address & REG_ADDR_MASK));
   //Write bit mask
   interface->spiDriver->transfer(mask);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief CRC calculation using the polynomial 0x4C11DB7
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t enc28j60CalcCrc(const void *data, size_t length)
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
   return crc;
}


/**
 * @brief Dump registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void enc28j60DumpReg(NetInterface *interface)
{
#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   uint8_t i;
   uint8_t bank;
   uint16_t address;

   //Display header
   TRACE_DEBUG("    Bank 0  Bank 1  Bank 2  Bank 3\r\n");

   //Loop through register addresses
   for(i = 0; i < 32; i++)
   {
      //Display register address
      TRACE_DEBUG("%02" PRIX8 ": ", i);

      //Loop through bank numbers
      for(bank = 0; bank < 4; bank++)
      {
         //Format register address
         address = (bank << 8) | i;

         //MAC and MII registers require a specific read sequence
         if(address >= 0x200 && address <= 0x219)
            address |= MAC_REG_TYPE;
         else if(address >= 0x300 && address <= 0x305)
            address |= MAC_REG_TYPE;
         else if(address == 0x30A)
            address |= MAC_REG_TYPE;

         //Display register contents
         TRACE_DEBUG("0x%02" PRIX8 "    ", enc28j60ReadReg(interface, address));
      }

      //Jump to the following line
      TRACE_DEBUG("\r\n");
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
#endif
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void enc28j60DumpPhyReg(NetInterface *interface)
{
#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIX8 ": 0x%04" PRIX16 "\r\n", i, enc28j60ReadPhyReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
#endif
}
