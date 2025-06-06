/**
 * @file lan8651_driver.c
 * @brief LAN8651 10Base-T1S Ethernet controller
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
#include "core/net.h"
#include "drivers/eth/lan8651_driver.h"
#include "debug.h"


/**
 * @brief LAN8651 driver
 **/

const NicDriver lan8651Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   lan8651Init,
   lan8651Tick,
   lan8651EnableIrq,
   lan8651DisableIrq,
   lan8651EventHandler,
   lan8651SendPacket,
   lan8651UpdateMacAddrFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief LAN8651 controller initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t lan8651Init(NetInterface *interface)
{
   uint32_t value;

   //Debug message
   TRACE_INFO("Initializing LAN8651 Ethernet controller...\r\n");

   //Initialize SPI interface
   interface->spiDriver->init();

   //Initialize external interrupt line driver
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->init();
   }

   //Issue a device reset
   lan8651WriteReg(interface, LAN8651_OA_RESET, LAN8651_OA_RESET_SWRESET);

   //Wait for the reset to complete
   do
   {
      //Read reset control and status register
      value = lan8651ReadReg(interface, LAN8651_OA_RESET);

      //The SWRESET self-clears when the reset finishes
   } while((value & LAN8651_OA_RESET_SWRESET) != 0);

   //Read the STATUS0 register and confirm that the RESETC field is 1
   do
   {
      //Read the status register 0
      value = lan8651ReadReg(interface, LAN8651_OA_STATUS0);

      //Check the value of the RESETC bit
   } while((value & LAN8651_OA_STATUS0_RESETC) == 0);

   //Write 1 to the RESETC field in the STATUS0 register to clear this field
   lan8651WriteReg(interface, LAN8651_OA_STATUS0, LAN8651_OA_STATUS0_RESETC);

   //Dump MMS0 registers for debugging purpose
   TRACE_DEBUG("MMS0 registers:\r\n");
   lan8651DumpReg(interface, LAN8651_MMS_STD, 0, 16);

   //Configuration process
   lan8651Config(interface);

#if (LAN8651_PLCA_SUPPORT == ENABLED)
   //Set PLCA burst
   lan8651WriteReg(interface, LAN8651_PLCA_BURST,
      LAN8651_PLCA_BURST_MAXBC_DEFAULT | LAN8651_PLCA_BURST_BTMR_DEFAULT);

   //Set PLCA node count and local ID
   lan8651WriteReg(interface, LAN8651_PLCA_CTRL1,
      ((LAN8651_NODE_COUNT << 8) & LAN8651_PLCA_CTRL1_NCNT) |
      (LAN8651_LOCAL_ID & LAN8651_PLCA_CTRL1_ID));

   //Enable PLCA
   lan8651WriteReg(interface, LAN8651_PLCA_CTRL0, LAN8651_PLCA_CTRL0_EN);
#else
   //Disable PLCA
   lan8651WriteReg(interface, LAN8651_PLCA_CTRL0, 0);
#endif

   //Perform custom configuration
   lan8651InitHook(interface);

   //Configure MAC address filtering
   lan8651UpdateMacAddrFilter(interface);

   //Configure the receive filter
   lan8651WriteReg(interface, LAN8651_MAC_NCFGR, LAN8651_MAC_NCFGR_MAXFS |
      LAN8651_MAC_NCFGR_MTIHEN);

   //Configure the SPI protocol engine
   lan8651WriteReg(interface, LAN8651_OA_CONFIG0,
      LAN8651_OA_CONFIG0_RFA_CSARFE | LAN8651_OA_CONFIG0_TXCTHRESH_16_CREDITS |
      LAN8651_OA_CONFIG0_CPS_64_BYTES);

   //When the MAC is configured, write 1 to the SYNC field in the CONFIG0
   //register to indicate that the MAC configuration is complete
   value = lan8651ReadReg(interface, LAN8651_OA_CONFIG0);
   value |= LAN8651_OA_CONFIG0_SYNC;
   lan8651WriteReg(interface, LAN8651_OA_CONFIG0, value);

   //Enable TX and RX
   value = lan8651ReadReg(interface, LAN8651_MAC_NCR);
   value |= LAN8651_MAC_NCR_TXEN | LAN8651_MAC_NCR_RXEN;
   lan8651WriteReg(interface, LAN8651_MAC_NCR, value);

   //Accept any packets from the upper layer
   osSetEvent(&interface->nicTxEvent);

   //Force the TCP/IP stack to poll the status at startup
   interface->nicEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief LAN8651 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void lan8651InitHook(NetInterface *interface)
{
}


/**
 * @brief LAN8651 controller configuration
 * @param[in] interface Underlying network interface
 **/

void lan8651Config(NetInterface *interface)
{
   int8_t value1;
   int8_t value2;
   uint16_t value3;
   uint16_t value4;
   uint16_t value5;
   uint16_t value6;
   uint16_t value7;
   int8_t offset1;
   int8_t offset2;
   uint16_t param1;
   uint16_t param2;
   uint16_t param3;
   uint16_t param4;
   uint16_t param5;

   //The configuration process begins with reading some registers
   value1 = lan8651ReadIndirectReg(interface, 0x04);
   value2 = lan8651ReadIndirectReg(interface, 0x08);
   value3 = lan8651ReadReg(interface, 0x04, 0x0084);
   value4 = lan8651ReadReg(interface, 0x04, 0x008A);
   value5 = lan8651ReadReg(interface, 0x04, 0x00AD);
   value6 = lan8651ReadReg(interface, 0x04, 0x00AE);
   value7 = lan8651ReadReg(interface, 0x04, 0x00AF);

   //Calculation of configuration offset 1
   if((value1 & 0x10) != 0)
   {
      offset1 = value1 | 0xE0;
   }
   else
   {
      offset1 = value1;
   }

   //Calculation of configuration offset 2
   if((value2 & 0x10) != 0)
   {
      offset2 = value2 | 0xE0;
   }
   else
   {
      offset2 = value2;
   }

   //Calculation of configuration parameters
   param1 = (value3 & 0xF) | (((9 + offset1) << 10) | ((14 + offset1) << 4));
   param2 = (value4 & 0x3FF) | ((40 + offset2) << 10);
   param3 = (value5 & 0xC0C0) | (((5 + offset1) << 8) | (9 + offset1));
   param4 = (value6 & 0xC0C0) | (((9 + offset1) << 8) | (14 + offset1));
   param5 = (value7 & 0xC0C0) | (((17 + offset1) << 8) | (22 + offset1));

   //The configuration parameters, along with other constant values are then
   //written to the device
   lan8651WriteReg(interface, 0x04, 0x0091, 0x9660);
   lan8651WriteReg(interface, 0x04, 0x0081, 0x00C0);
   lan8651WriteReg(interface, 0x01, 0x0077, 0x0028);
   lan8651WriteReg(interface, 0x04, 0x0043, 0x00FF);
   lan8651WriteReg(interface, 0x04, 0x0044, 0xFFFF);
   lan8651WriteReg(interface, 0x04, 0x0045, 0x0000);
   lan8651WriteReg(interface, 0x04, 0x0053, 0x00FF);
   lan8651WriteReg(interface, 0x04, 0x0054, 0xFFFF);
   lan8651WriteReg(interface, 0x04, 0x0055, 0x0000);
   lan8651WriteReg(interface, 0x04, 0x0040, 0x0002);
   lan8651WriteReg(interface, 0x04, 0x0050, 0x0002);
   lan8651WriteReg(interface, 0x04, 0x00D0, 0x5F21);
   lan8651WriteReg(interface, 0x04, 0x0084, param1);
   lan8651WriteReg(interface, 0x04, 0x008A, param2);
   lan8651WriteReg(interface, 0x04, 0x00E9, 0x9E50);
   lan8651WriteReg(interface, 0x04, 0x00F5, 0x1CF8);
   lan8651WriteReg(interface, 0x04, 0x00F4, 0xC020);
   lan8651WriteReg(interface, 0x04, 0x00F8, 0x9B00);
   lan8651WriteReg(interface, 0x04, 0x00F9, 0x4E53);
   lan8651WriteReg(interface, 0x04, 0x00AD, param3);
   lan8651WriteReg(interface, 0x04, 0x00AE, param4);
   lan8651WriteReg(interface, 0x04, 0x00AF, param5);
   lan8651WriteReg(interface, 0x04, 0x00B0, 0x0103);
   lan8651WriteReg(interface, 0x04, 0x00B1, 0x0910);
   lan8651WriteReg(interface, 0x04, 0x00B2, 0x1D26);
   lan8651WriteReg(interface, 0x04, 0x00B3, 0x002A);
   lan8651WriteReg(interface, 0x04, 0x00B4, 0x0103);
   lan8651WriteReg(interface, 0x04, 0x00B5, 0x070D);
   lan8651WriteReg(interface, 0x04, 0x00B6, 0x1720);
   lan8651WriteReg(interface, 0x04, 0x00B7, 0x0027);
   lan8651WriteReg(interface, 0x04, 0x00B8, 0x0509);
   lan8651WriteReg(interface, 0x04, 0x00B9, 0x0E13);
   lan8651WriteReg(interface, 0x04, 0x00BA, 0x1C25);
   lan8651WriteReg(interface, 0x04, 0x00BB, 0x002B);
}


/**
 * @brief LAN8651 timer handler
 * @param[in] interface Underlying network interface
 **/

void lan8651Tick(NetInterface *interface)
{
   uint32_t value;
   bool_t linkState;

#if (LAN8651_PLCA_SUPPORT == ENABLED)
   //Read PLCA status register
   value = lan8651ReadReg(interface, LAN8651_PLCA_STS);

   //The PST field indicates that the PLCA reconciliation sublayer is active
   //and a BEACON is being regularly transmitted or received
   linkState = (value & LAN8651_PLCA_STS_PST) ? TRUE : FALSE;
#else
   //Link status indication is not supported
   linkState = TRUE;
#endif

   //Link up event?
   if(linkState && !interface->linkState)
   {
      //The PHY is only able to operate in 10 Mbps mode
      interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
      interface->duplexMode = NIC_HALF_DUPLEX_MODE;

      //Update link state
      interface->linkState = TRUE;

      //Process link state change event
      nicNotifyLinkChange(interface);
   }
   //Link down event?
   else if(!linkState && interface->linkState)
   {
      //Update link state
      interface->linkState = FALSE;

      //Process link state change event
      nicNotifyLinkChange(interface);
   }
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void lan8651EnableIrq(NetInterface *interface)
{
   //Enable interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->enableIrq();
   }
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void lan8651DisableIrq(NetInterface *interface)
{
   //Disable interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief LAN8651 interrupt service routine
 * @param[in] interface Underlying network interface
 * @return TRUE if a higher priority task must be woken. Else FALSE is returned
 **/

bool_t lan8651IrqHandler(NetInterface *interface)
{
   //When the SPI host detects an asserted IRQn from the MACPHY, it should
   //initiate a data chunk transfer to obtain the current data footer
   interface->nicEvent = TRUE;

   //Notify the TCP/IP stack of the event
   return osSetEventFromIsr(&netEvent);
}


/**
 * @brief LAN8651 event handler
 * @param[in] interface Underlying network interface
 **/

void lan8651EventHandler(NetInterface *interface)
{
   uint32_t status;

   //Process all the data chunks
   do
   {
      //Read incoming packet
      lan8651ReceivePacket(interface);

      //Read buffer status register
      status = lan8651ReadReg(interface, LAN8651_OA_BUFSTS);

      //Any data chunk available to the host MCU for reading?
   } while((status & LAN8651_OA_BUFSTS_RCA) != 0);
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

error_t lan8651SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   static uint8_t chunk[LAN8651_CHUNK_PAYLOAD_SIZE + 4];
   size_t i;
   size_t j;
   size_t n;
   size_t length;
   uint32_t status;
   uint32_t header;
   uint32_t footer;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Read buffer status register
   status = lan8651ReadReg(interface, LAN8651_OA_BUFSTS);
   //Get the number of data chunks available in the transmit buffer
   n = (status & LAN8651_OA_BUFSTS_TXC) >> 8;

   //Check the number of transmit credits available
   if(length <= (n * LAN8651_CHUNK_PAYLOAD_SIZE))
   {
      //A data transaction consists of multiple chunks
      for(i = 0; i < length; i += n)
      {
         //The default size of the data chunk payload is 64 bytes
         n = MIN(length - i, LAN8651_CHUNK_PAYLOAD_SIZE);

         //Set up a data transfer
         header = LAN8651_TX_HEADER_DNC | LAN8651_TX_HEADER_NORX |
            LAN8651_TX_HEADER_DV;

         //Start of packet?
         if(i == 0)
         {
            //The SPI host shall set the SV bit when the beginning of an
            //Ethernet frame is present in the current transmit data chunk
            //payload
            header |= LAN8651_TX_HEADER_SV;
         }

         //End of packet?
         if((i + n) == length)
         {
            //The SPI host shall set the EV bit when the end of an Ethernet
            //frame is present in the current transmit data chunk payload
            header |= LAN8651_TX_HEADER_EV;

            //When EV is 1, the EBO field shall contain the byte offset into
            //the transmit data chunk payload that points to the last byte of
            //the Ethernet frame to transmit
            header |= ((n - 1) << 8) & LAN8651_TX_HEADER_EBO;
         }

         //The parity bit is calculated over the transmit data header
         if(lan8651CalcParity(header) != 0)
         {
            header |= LAN8651_CTRL_HEADER_P;
         }

         //A chunk is composed of 4 bytes of overhead plus the configured
         //payload size
         STORE32BE(header, chunk);

         //Copy data chunk payload
         netBufferRead(chunk + 4, buffer, offset + i, n);

         //Pad frames shorter than the data chunk payload
         if(n < LAN8651_CHUNK_PAYLOAD_SIZE)
         {
            osMemset(chunk + 4 + n, 0, LAN8651_CHUNK_PAYLOAD_SIZE - n);
         }

         //Pull the CS pin low
         interface->spiDriver->assertCs();

         //Perform data transfer
         for(j = 0; j < (LAN8651_CHUNK_PAYLOAD_SIZE + 4); j++)
         {
            chunk[j] = interface->spiDriver->transfer(chunk[j]);
         }

         //Terminate the operation by raising the CS pin
         interface->spiDriver->deassertCs();

         //Receive data chunks consist of the receive data chunk payload followed
         //by a 4-byte footer
         footer = LOAD32BE(chunk + LAN8651_CHUNK_PAYLOAD_SIZE);

         //The RCA field indicates the number of receive data chunks available
         if((footer & LAN8651_RX_FOOTER_RCA) != 0)
         {
            //Some data chunks are available for reading
            interface->nicEvent = TRUE;
            //Notify the TCP/IP stack of the event
            osSetEvent(&netEvent);
         }
      }
   }
   else
   {
      //No sufficient credits available
   }

   //The transmitter can accept another packet
   osSetEvent(&interface->nicTxEvent);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Receive a packet
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t lan8651ReceivePacket(NetInterface *interface)
{
   static uint8_t buffer[LAN8651_ETH_RX_BUFFER_SIZE];
   static uint8_t chunk[LAN8651_CHUNK_PAYLOAD_SIZE + 4];
   error_t error;
   size_t i;
   size_t n;
   size_t length;
   uint32_t header;
   uint32_t footer;

   //Initialize variable
   length = 0;

   //A data transaction consists of multiple chunks
   while(1)
   {
      //Check the length of the received packet
      if((length + LAN8651_CHUNK_PAYLOAD_SIZE) > LAN8651_ETH_RX_BUFFER_SIZE)
      {
         error = ERROR_BUFFER_OVERFLOW;
         break;
      }

      //The SPI host sets NORX to 0 to indicate that it accepts and process
      //any receive frame data within the current chunk
      header = LAN8651_TX_HEADER_DNC;

      //The parity bit is calculated over the transmit data header
      if(lan8651CalcParity(header) != 0)
      {
         header |= LAN8651_CTRL_HEADER_P;
      }

      //Transmit data chunks consist of a 4-byte header followed by the
      //transmit data chunk payload,
      STORE32BE(header, chunk);

      //Clear data chunk payload
      osMemset(chunk + 4, 0, LAN8651_CHUNK_PAYLOAD_SIZE);

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Perform data transfer
      for(i = 0; i < (LAN8651_CHUNK_PAYLOAD_SIZE + 4); i++)
      {
         chunk[i] = interface->spiDriver->transfer(chunk[i]);
      }

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();

      //Receive data chunks consist of the receive data chunk payload followed
      //by a 4-byte footer
      footer = LOAD32BE(chunk + LAN8651_CHUNK_PAYLOAD_SIZE);

      //When the DV bit is 0, the SPI host ignores the chunk payload
      if((footer & LAN8651_RX_FOOTER_DV) == 0)
      {
         error = ERROR_BUFFER_EMPTY;
         break;
      }

      //When the SV bit is 1, the beginning of an Ethernet frame is present in
      //the current transmit data chunk payload
      if(length == 0)
      {
         if((footer & LAN8651_RX_FOOTER_SV) == 0)
         {
            error = ERROR_INVALID_PACKET;
            break;
         }
      }
      else
      {
         if((footer & LAN8651_RX_FOOTER_SV) != 0)
         {
            error = ERROR_INVALID_PACKET;
            break;
         }
      }

      //When EV is 1, the EBO field contains the byte offset into the
      //receive data chunk payload that points to the last byte of the
      //received Ethernet frame
      if((footer & LAN8651_RX_FOOTER_EV) != 0)
      {
         n = ((footer & LAN8651_RX_FOOTER_EBO) >> 8) + 1;
      }
      else
      {
         n = LAN8651_CHUNK_PAYLOAD_SIZE;
      }

      //Copy data chunk payload
      osMemcpy(buffer + length, chunk, n);
      //Adjust the length of the packet
      length += n;

      //When the EV bit is 1, the end of an Ethernet frame is present in the
      //current receive data chunk payload
      if((footer & LAN8651_RX_FOOTER_EV) != 0)
      {
         NetRxAncillary ancillary;

         //Additional options can be passed to the stack along with the packet
         ancillary = NET_DEFAULT_RX_ANCILLARY;
         //Pass the packet to the upper layer
         nicProcessPacket(interface, buffer, length, &ancillary);

         //Successful processing
         error = NO_ERROR;
         break;
      }
   }

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t lan8651UpdateMacAddrFilter(NetInterface *interface)
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

   //Set the lower 32 bits of the station MAC address
   lan8651WriteReg(interface, LAN8651_MAC_SAB1,
      (interface->macAddr.b[3] << 24) | (interface->macAddr.b[2] << 16) |
      (interface->macAddr.b[1] << 8) | interface->macAddr.b[0]);

   //Set the upper 16 bits of the station MAC address
   lan8651WriteReg(interface, LAN8651_MAC_SAT1,
      (interface->macAddr.b[5] << 8) | interface->macAddr.b[4]);

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
               unicastMacAddr[j++] = entry->addr;
            }
         }
      }
   }

   //Configure the first unicast address filter
   if(j >= 1)
   {
      //Set the lower 32 bits of the MAC address
      lan8651WriteReg(interface, LAN8651_MAC_SAB2,
         (unicastMacAddr[0].b[3] << 24) | (unicastMacAddr[0].b[2] << 16) |
         (unicastMacAddr[0].b[1] << 8) | unicastMacAddr[0].b[0]);

      //The address is activated when SAT register is written
      lan8651WriteReg(interface, LAN8651_MAC_SAT2,
         (unicastMacAddr[0].b[5] << 8) | unicastMacAddr[0].b[4]);
   }
   else
   {
      //The address is deactivated when SAB register is written
      lan8651WriteReg(interface, LAN8651_MAC_SAB2, 0);
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      //Set the lower 32 bits of the MAC address
      lan8651WriteReg(interface, LAN8651_MAC_SAB3,
         (unicastMacAddr[1].b[3] << 24) | (unicastMacAddr[1].b[2] << 16) |
         (unicastMacAddr[1].b[1] << 8) | unicastMacAddr[1].b[0]);

      //The address is activated when SAT register is written
      lan8651WriteReg(interface, LAN8651_MAC_SAT3,
         (unicastMacAddr[1].b[5] << 8) | unicastMacAddr[1].b[4]);
   }
   else
   {
      //The address is deactivated when SAB register is written
      lan8651WriteReg(interface, LAN8651_MAC_SAB3, 0);
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      //Set the lower 32 bits of the MAC address
      lan8651WriteReg(interface, LAN8651_MAC_SAB4,
         (unicastMacAddr[2].b[3] << 24) | (unicastMacAddr[2].b[2] << 16) |
         (unicastMacAddr[2].b[1] << 8) | unicastMacAddr[2].b[0]);

      //The address is activated when SAT register is written
      lan8651WriteReg(interface, LAN8651_MAC_SAT4,
         (unicastMacAddr[2].b[5] << 8) | unicastMacAddr[2].b[4]);
   }
   else
   {
      //The address is deactivated when SAB register is written
      lan8651WriteReg(interface, LAN8651_MAC_SAB4, 0);
   }

   //Configure the multicast hash table
   lan8651WriteReg(interface, LAN8651_MAC_HRB, hashTable[0]);
   lan8651WriteReg(interface, LAN8651_MAC_HRT, hashTable[1]);

   //Debug message
   TRACE_DEBUG("  HRB = %08" PRIX32 "\r\n", hashTable[0]);
   TRACE_DEBUG("  HRT = %08" PRIX32 "\r\n", hashTable[1]);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Write register
 * @param[in] interface Underlying network interface
 * @param[in] mms Register memory map to access
 * @param[in] address Register address
 * @param[in] data Register value
 **/

void lan8651WriteReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint32_t data)
{
   uint32_t header;

   //Set up a register write operation
   header = LAN8651_CTRL_HEADER_WNR | LAN8651_CTRL_HEADER_AID;
   //The MMS field selects the specific register memory map to access
   header |= (mms << 24) & LAN8651_CTRL_HEADER_MMS;
   //Address of the first register to access
   header |= (address << 8) & LAN8651_CTRL_HEADER_ADDR;
   //Specifies the number of registers to write
   header |= (0 << 1) & LAN8651_CTRL_HEADER_LEN;

   //The parity bit is calculated over the control command header
   if(lan8651CalcParity(header) != 0)
   {
      header |= LAN8651_CTRL_HEADER_P;
   }

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write control command header
   interface->spiDriver->transfer((header >> 24) & 0xFF);
   interface->spiDriver->transfer((header >> 16) & 0xFF);
   interface->spiDriver->transfer((header >> 8) & 0xFF);
   interface->spiDriver->transfer(header & 0xFF);

   //Write data
   interface->spiDriver->transfer((data >> 24) & 0xFF);
   interface->spiDriver->transfer((data >> 16) & 0xFF);
   interface->spiDriver->transfer((data >> 8) & 0xFF);
   interface->spiDriver->transfer(data & 0xFF);

   //Send 32 bits of dummy data at the end of the control write command
   interface->spiDriver->transfer(0x00);
   interface->spiDriver->transfer(0x00);
   interface->spiDriver->transfer(0x00);
   interface->spiDriver->transfer(0x00);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Read register
 * @param[in] interface Underlying network interface
 * @param[in] mms Register memory map to access
 * @param[in] address Register address
 * @return Register value
 **/

uint32_t lan8651ReadReg(NetInterface *interface, uint8_t mms,
   uint16_t address)
{
   uint32_t data;
   uint32_t header;

   //Set up a register read operation
   header = LAN8651_CTRL_HEADER_AID;
   //The MMS field selects the specific register memory map to access
   header |= (mms << 24) & LAN8651_CTRL_HEADER_MMS;
   //Address of the first register to access
   header |= (address << 8) & LAN8651_CTRL_HEADER_ADDR;
   //Specifies the number of registers to read
   header |= (0 << 1) & LAN8651_CTRL_HEADER_LEN;

   //The parity bit is calculated over the control command header
   if(lan8651CalcParity(header) != 0)
   {
      header |= LAN8651_CTRL_HEADER_P;
   }

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write control command header
   interface->spiDriver->transfer((header >> 24) & 0xFF);
   interface->spiDriver->transfer((header >> 16) & 0xFF);
   interface->spiDriver->transfer((header >> 8) & 0xFF);
   interface->spiDriver->transfer(header & 0xFF);

   //Discard the echoed control header
   interface->spiDriver->transfer(0x00);
   interface->spiDriver->transfer(0x00);
   interface->spiDriver->transfer(0x00);
   interface->spiDriver->transfer(0x00);

   //Read data
   data = interface->spiDriver->transfer(0x00) << 24;
   data |= interface->spiDriver->transfer(0x00) << 16;
   data |= interface->spiDriver->transfer(0x00) << 8;
   data |= interface->spiDriver->transfer(0x00);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();

   //Return register value
   return data;
}


/**
 * @brief Dump registers for debugging purpose
 * @param[in] interface Underlying network interface
 * @param[in] mms Register memory map to access
 * @param[in] address Start address
 * @param[in] num Number of registers to dump
 **/

void lan8651DumpReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint_t num)
{
   uint_t i;

   //Loop through registers
   for(i = 0; i < num; i++)
   {
      //Display current register
      TRACE_DEBUG("0x%02" PRIX16 ": 0x%08" PRIX32 "\r\n", address + i,
         lan8651ReadReg(interface, mms, address + i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Write MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @param[in] data MMD register value
 **/

void lan8651WriteMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr, uint16_t data)
{
   //Select register operation
   lan8651WriteReg(interface, LAN8651_MMDCTRL,
      LAN8651_MMDCTRL_FNCTN_ADDR | (devAddr & LAN8651_MMDCTRL_DEVAD));

   //Write MMD register address
   lan8651WriteReg(interface, LAN8651_MMDAD, regAddr);

   //Select data operation
   lan8651WriteReg(interface, LAN8651_MMDCTRL,
      LAN8651_MMDCTRL_FNCTN_DATA_NO_POST_INC | (devAddr & LAN8651_MMDCTRL_DEVAD));

   //Write the content of the MMD register
   lan8651WriteReg(interface, LAN8651_MMDAD, data);
}


/**
 * @brief Read MMD register
 * @param[in] interface Underlying network interface
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return MMD register value
 **/

uint16_t lan8651ReadMmdReg(NetInterface *interface, uint8_t devAddr,
   uint16_t regAddr)
{
   //Select register operation
   lan8651WriteReg(interface, LAN8651_MMDCTRL,
      LAN8651_MMDCTRL_FNCTN_ADDR | (devAddr & LAN8651_MMDCTRL_DEVAD));

   //Write MMD register address
   lan8651WriteReg(interface, LAN8651_MMDAD, regAddr);

   //Select data operation
   lan8651WriteReg(interface, LAN8651_MMDCTRL,
      LAN8651_MMDCTRL_FNCTN_DATA_NO_POST_INC | (devAddr & LAN8651_MMDCTRL_DEVAD));

   //Read the content of the MMD register
   return lan8651ReadReg(interface, LAN8651_MMDAD);
}


/**
 * @brief Read indirect register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @return Indirect register value
 **/

int8_t lan8651ReadIndirectReg(NetInterface *interface, uint8_t address)
{
   //Specify the address of the register to read
   lan8651WriteMmdReg(interface, 0x04, 0x00D8, address);
   lan8651WriteMmdReg(interface, 0x04, 0x00DA, 0x0002);

   //Read the content of the register
   return lan8651ReadMmdReg(interface, 0x04, 0x00D9);
}


/**
 * @brief Calculate parity bit over a 32-bit data
 * @param[in] data 32-bit bit stream
 * @return Odd parity bit computed over the supplied data
 **/

uint32_t lan8651CalcParity(uint32_t data)
{
   //Calculate the odd parity bit computed over the supplied bit stream
   data ^= data >> 1;
   data ^= data >> 2;
   data ^= data >> 4;
   data ^= data >> 8;
   data ^= data >> 16;

   //Return '1' when the number of bits set to one in the supplied bit
   //stream is even (resulting in an odd number of ones when the parity is
   //included), otherwise return '0'
   return ~data & 0x01;
}
