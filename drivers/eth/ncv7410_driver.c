/**
 * @file ncv7410_driver.c
 * @brief Onsemi NCV7410 10Base-T1S Ethernet controller
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
#include "drivers/eth/ncv7410_driver.h"
#include "debug.h"


/**
 * @brief NCV7410 driver
 **/

const NicDriver ncv7410Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   ncv7410Init,
   ncv7410Tick,
   ncv7410EnableIrq,
   ncv7410DisableIrq,
   ncv7410EventHandler,
   ncv7410SendPacket,
   ncv7410UpdateMacAddrFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief NCV7410 controller initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t ncv7410Init(NetInterface *interface)
{
   uint32_t value;

   //Debug message
   TRACE_INFO("Initializing NCV7410 Ethernet controller...\r\n");

   //Initialize SPI interface
   interface->spiDriver->init();

   //Initialize external interrupt line driver
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->init();
   }

   //Issue a device reset
   ncv7410WriteReg(interface, NCV7410_RESET, NCV7410_RESET_RESET);

   //Wait for the reset to complete
   do
   {
      //Read reset control and status register
      value = ncv7410ReadReg(interface, NCV7410_RESET);

      //The RESET self-clears when the reset finishes
   } while((value & NCV7410_RESET_RESET) != 0);

   //Read the STATUS0 register and confirm that the RESETC field is 1
   do
   {
      //Read the status register 0
      value = ncv7410ReadReg(interface, NCV7410_STATUS0);

      //Check the value of the RESETC bit
   } while((value & NCV7410_STATUS0_RESETC) == 0);

   //Write 1 to the RESETC field in the STATUS0 register to clear this field
   ncv7410WriteReg(interface, NCV7410_STATUS0, NCV7410_STATUS0_RESETC);

   //Dump MMS0 registers for debugging purpose
   TRACE_DEBUG("MMS0 registers:\r\n");
   ncv7410DumpReg(interface, NCV7410_MMS_STD, 0, 16);

   //Configure DIO LEDs
   ncv7410WriteReg(interface, NCV7410_DIOCFG,
      NCV7410_DIOCFG_SLR1 | NCV7410_DIOCFG_FN1_LED_RX |
      NCV7410_DIOCFG_VAL1 | NCV7410_DIOCFG_SLR0 |
      NCV7410_DIOCFG_FN0_LED_TX | NCV7410_DIOCFG_VAL0);

   //Perform custom configuration
   ncv7410InitHook(interface);

   //Configure the MAC for calculating and appending the FCS
   value = ncv7410ReadReg(interface, NCV7410_MACCTRL0);
   value |= NCV7410_MACCTRL0_FCSA;
   ncv7410WriteReg(interface, NCV7410_MACCTRL0, value);

   //Use factory preprogrammed MAC address?
   if(macCompAddr(&interface->macAddr, &MAC_UNSPECIFIED_ADDR))
   {
      //Read PHYID register
      value = ncv7410ReadReg(interface, NCV7410_PHYID);
      //The OUI field records the 22 MSB's of the OUI in reverse order
      value = reverseInt32(value) << 2;

      //Save the OUI
      interface->macAddr.b[0] = value & 0xFF;
      interface->macAddr.b[1] = (value >> 8) & 0xFF;
      interface->macAddr.b[2] = (value >> 16) & 0xFF;

      //Read MACID0 register
      value = ncv7410ReadReg(interface, NCV7410_MACID0);

      //Save the lower 16 bits of the unique MAC address
      interface->macAddr.b[5] = value & 0xFF;
      interface->macAddr.b[4] = (value >> 8) & 0xFF;

      //Read MACID1 register
      value = ncv7410ReadReg(interface, NCV7410_MACID1);

      //Save the upper 8 bits of the unique MAC address
      interface->macAddr.b[3] = value & 0xFF;

      //Generate the 64-bit interface identifier
      macAddrToEui64(&interface->macAddr, &interface->eui64);
   }

   //Configure MAC address filtering
   ncv7410UpdateMacAddrFilter(interface);

   //Configure the SPI protocol engine
   ncv7410WriteReg(interface, NCV7410_CONFIG0, NCV7410_CONFIG0_CSARFE |
      NCV7410_CONFIG0_ZARFE | NCV7410_CONFIG0_TXCTHRESH_16_CREDITS |
      NCV7410_CONFIG0_CPS_64_BYTES);

   //When the MAC is configured, write 1 to the SYNC field in the CONFIG0
   //register to indicate that the MAC configuration is complete
   value = ncv7410ReadReg(interface, NCV7410_CONFIG0);
   value |= NCV7410_CONFIG0_SYNC;
   ncv7410WriteReg(interface, NCV7410_CONFIG0, value);

   //Enable TX and RX
   value = ncv7410ReadReg(interface, NCV7410_MACCTRL0);
   value |= NCV7410_MACCTRL0_TXEN | NCV7410_MACCTRL0_RXEN;
   ncv7410WriteReg(interface, NCV7410_MACCTRL0, value);

   //Enable the physical link
   ncv7410WriteReg(interface, NCV7410_PHYCTRL, NCV7410_PHYCTRL_LCTL);

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
 * @brief NCV7410 custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void ncv7410InitHook(NetInterface *interface)
{
#if (NCV7410_PLCA_SUPPORT == ENABLED)
   //Set PLCA burst
   ncv7410WriteReg(interface, NCV7410_PLCABURST,
      NCV7410_PLCABURST_MAXBC_DEFAULT | NCV7410_PLCABURST_BTMR_DEFAULT);

   //Set PLCA node count and local ID
   ncv7410WriteReg(interface, NCV7410_PLCACTRL1,
      ((NCV7410_NODE_COUNT << 8) & NCV7410_PLCACTRL1_NCNT) |
      (NCV7410_LOCAL_ID & NCV7410_PLCACTRL1_ID));

   //Enable PLCA
   ncv7410WriteReg(interface, NCV7410_PLCACTRL0, NCV7410_PLCACTRL0_EN);
#else
   //Disable PLCA
   ncv7410WriteReg(interface, NCV7410_PLCACTRL0, 0);
#endif
}


/**
 * @brief NCV7410 timer handler
 * @param[in] interface Underlying network interface
 **/

void ncv7410Tick(NetInterface *interface)
{
   uint32_t value;
   bool_t linkState;

   //Read PHY status register
   value = ncv7410ReadReg(interface, NCV7410_PHYSTATUS);
   //Retrieve current link state
   linkState = (value & NCV7410_PHYSTATUS_LKST) ? TRUE : FALSE;

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

void ncv7410EnableIrq(NetInterface *interface)
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

void ncv7410DisableIrq(NetInterface *interface)
{
   //Disable interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief NCV7410 interrupt service routine
 * @param[in] interface Underlying network interface
 * @return TRUE if a higher priority task must be woken. Else FALSE is returned
 **/

bool_t ncv7410IrqHandler(NetInterface *interface)
{
   //When the SPI host detects an asserted IRQn from the MACPHY, it should
   //initiate a data chunk transfer to obtain the current data footer
   interface->nicEvent = TRUE;

   //Notify the TCP/IP stack of the event
   return osSetEventFromIsr(&netEvent);
}


/**
 * @brief NCV7410 event handler
 * @param[in] interface Underlying network interface
 **/

void ncv7410EventHandler(NetInterface *interface)
{
   uint32_t status;

   //Read buffer status register
   status = ncv7410ReadReg(interface, NCV7410_BUFSTS);

   //Process all the data chunks currently available
   while((status & NCV7410_BUFSTS_RCA) != 0)
   {
      //Read incoming packet
      ncv7410ReceivePacket(interface);

      //Read buffer status register
      status = ncv7410ReadReg(interface, NCV7410_BUFSTS);
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

error_t ncv7410SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   static uint8_t chunk[NCV7410_CHUNK_PAYLOAD_SIZE + 4];
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
   status = ncv7410ReadReg(interface, NCV7410_BUFSTS);
   //Get the number of data chunks available in the transmit buffer
   n = (status & NCV7410_BUFSTS_TXC) >> 8;

   //Check the number of transmit credits available
   if(length <= (n * NCV7410_CHUNK_PAYLOAD_SIZE))
   {
      //A data transaction consists of multiple chunks
      for(i = 0; i < length; i += n)
      {
         //The default size of the data chunk payload is 64 bytes
         n = MIN(length - i, NCV7410_CHUNK_PAYLOAD_SIZE);

         //Set up a data transfer
         header = NCV7410_TX_HEADER_DNC | NCV7410_TX_HEADER_NORX |
            NCV7410_TX_HEADER_DV;

         //Start of packet?
         if(i == 0)
         {
            //The SPI host shall set the SV bit when the beginning of an
            //Ethernet frame is present in the current transmit data chunk
            //payload
            header |= NCV7410_TX_HEADER_SV;
         }

         //End of packet?
         if((i + n) == length)
         {
            //The SPI host shall set the EV bit when the end of an Ethernet
            //frame is present in the current transmit data chunk payload
            header |= NCV7410_TX_HEADER_EV;

            //When EV is 1, the EBO field shall contain the byte offset into
            //the transmit data chunk payload that points to the last byte of
            //the Ethernet frame to transmit
            header |= ((n - 1) << 8) & NCV7410_TX_HEADER_EBO;
         }

         //The parity bit is calculated over the transmit data header
         if(ncv7410CalcParity(header) != 0)
         {
            header |= NCV7410_CTRL_HEADER_P;
         }

         //A chunk is composed of 4 bytes of overhead plus the configured
         //payload size
         STORE32BE(header, chunk);

         //Copy data chunk payload
         netBufferRead(chunk + 4, buffer, offset + i, n);

         //Pad frames shorter than the data chunk payload
         if(n < NCV7410_CHUNK_PAYLOAD_SIZE)
         {
            osMemset(chunk + 4 + n, 0, NCV7410_CHUNK_PAYLOAD_SIZE - n);
         }

         //Pull the CS pin low
         interface->spiDriver->assertCs();

         //Perform data transfer
         for(j = 0; j < (NCV7410_CHUNK_PAYLOAD_SIZE + 4); j++)
         {
            chunk[j] = interface->spiDriver->transfer(chunk[j]);
         }

         //Terminate the operation by raising the CS pin
         interface->spiDriver->deassertCs();

         //Receive data chunks consist of the receive data chunk payload followed
         //by a 4-byte footer
         footer = LOAD32BE(chunk + NCV7410_CHUNK_PAYLOAD_SIZE);

         //The RCA field indicates the number of receive data chunks available
         if((footer & NCV7410_RX_FOOTER_RCA) != 0)
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

error_t ncv7410ReceivePacket(NetInterface *interface)
{
   static uint8_t buffer[NCV7410_ETH_RX_BUFFER_SIZE];
   static uint8_t chunk[NCV7410_CHUNK_PAYLOAD_SIZE + 4];
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
      if((length + NCV7410_CHUNK_PAYLOAD_SIZE) > NCV7410_ETH_RX_BUFFER_SIZE)
      {
         error = ERROR_BUFFER_OVERFLOW;
         break;
      }

      //The SPI host sets NORX to 0 to indicate that it accepts and process
      //any receive frame data within the current chunk
      header = NCV7410_TX_HEADER_DNC;

      //The parity bit is calculated over the transmit data header
      if(ncv7410CalcParity(header) != 0)
      {
         header |= NCV7410_CTRL_HEADER_P;
      }

      //Transmit data chunks consist of a 4-byte header followed by the
      //transmit data chunk payload,
      STORE32BE(header, chunk);

      //Clear data chunk payload
      osMemset(chunk + 4, 0, NCV7410_CHUNK_PAYLOAD_SIZE);

      //Pull the CS pin low
      interface->spiDriver->assertCs();

      //Perform data transfer
      for(i = 0; i < (NCV7410_CHUNK_PAYLOAD_SIZE + 4); i++)
      {
         chunk[i] = interface->spiDriver->transfer(chunk[i]);
      }

      //Terminate the operation by raising the CS pin
      interface->spiDriver->deassertCs();

      //Receive data chunks consist of the receive data chunk payload followed
      //by a 4-byte footer
      footer = LOAD32BE(chunk + NCV7410_CHUNK_PAYLOAD_SIZE);

      //When the DV bit is 0, the SPI host ignores the chunk payload
      if((footer & NCV7410_RX_FOOTER_DV) == 0)
      {
         error = ERROR_BUFFER_EMPTY;
         break;
      }

      //When the SV bit is 1, the beginning of an Ethernet frame is present in
      //the current transmit data chunk payload
      if(length == 0)
      {
         if((footer & NCV7410_RX_FOOTER_SV) == 0)
         {
            error = ERROR_INVALID_PACKET;
            break;
         }
      }
      else
      {
         if((footer & NCV7410_RX_FOOTER_SV) != 0)
         {
            error = ERROR_INVALID_PACKET;
            break;
         }
      }

      //When EV is 1, the EBO field contains the byte offset into the
      //receive data chunk payload that points to the last byte of the
      //received Ethernet frame
      if((footer & NCV7410_RX_FOOTER_EV) != 0)
      {
         n = ((footer & NCV7410_RX_FOOTER_EBO) >> 8) + 1;
      }
      else
      {
         n = NCV7410_CHUNK_PAYLOAD_SIZE;
      }

      //Copy data chunk payload
      osMemcpy(buffer + length, chunk, n);
      //Adjust the length of the packet
      length += n;

      //When the EV bit is 1, the end of an Ethernet frame is present in the
      //current receive data chunk payload
      if((footer & NCV7410_RX_FOOTER_EV) != 0)
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

error_t ncv7410UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t j;
   uint32_t value;
   bool_t acceptMulticast;
   MacAddr unicastMacAddr[3];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating MAC filter...\r\n");

   //Set the lower 32 bits of the station MAC address
   ncv7410WriteReg(interface, NCV7410_ADDRFILT0L,
      (interface->macAddr.b[2] << 24) | (interface->macAddr.b[3] << 16) |
      (interface->macAddr.b[4] << 8) | interface->macAddr.b[5]);

   //Set the upper 16 bits of the station MAC address
   ncv7410WriteReg(interface, NCV7410_ADDRFILT0H, NCV7410_ADDRFILTnH_EN |
      (interface->macAddr.b[0] << 8) | interface->macAddr.b[1]);

   //The MAC supports 3 additional addresses for unicast perfect filtering
   unicastMacAddr[0] = MAC_UNSPECIFIED_ADDR;
   unicastMacAddr[1] = MAC_UNSPECIFIED_ADDR;
   unicastMacAddr[2] = MAC_UNSPECIFIED_ADDR;

   //This flag will be set if multicast addresses should be accepted
   acceptMulticast = FALSE;

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
            //Accept multicast addresses
            acceptMulticast = TRUE;
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
      ncv7410WriteReg(interface, NCV7410_ADDRFILT1L,
         (unicastMacAddr[0].b[2] << 24) | (unicastMacAddr[0].b[3] << 16) |
         (unicastMacAddr[0].b[4] << 8) | unicastMacAddr[0].b[5]);

      //Set the upper 16 bits of the MAC address
      ncv7410WriteReg(interface, NCV7410_ADDRFILT1H, NCV7410_ADDRFILTnH_EN |
         (unicastMacAddr[0].b[0] << 8) | unicastMacAddr[0].b[1]);
   }
   else
   {
      ncv7410WriteReg(interface, NCV7410_ADDRFILT1L, 0);
      ncv7410WriteReg(interface, NCV7410_ADDRFILT1H, 0);
   }

   //Configure the second unicast address filter
   if(j >= 2)
   {
      //Set the lower 32 bits of the MAC address
      ncv7410WriteReg(interface, NCV7410_ADDRFILT2L,
         (unicastMacAddr[1].b[2] << 24) | (unicastMacAddr[1].b[3] << 16) |
         (unicastMacAddr[1].b[4] << 8) | unicastMacAddr[1].b[5]);

      //Set the upper 16 bits of the MAC address
      ncv7410WriteReg(interface, NCV7410_ADDRFILT2H, NCV7410_ADDRFILTnH_EN |
         (unicastMacAddr[1].b[0] << 8) | unicastMacAddr[1].b[1]);
   }
   else
   {
      ncv7410WriteReg(interface, NCV7410_ADDRFILT2L, 0);
      ncv7410WriteReg(interface, NCV7410_ADDRFILT2H, 0);
   }

   //Configure the third unicast address filter
   if(j >= 3)
   {
      //Set the lower 32 bits of the MAC address
      ncv7410WriteReg(interface, NCV7410_ADDRFILT3L,
         (unicastMacAddr[2].b[2] << 24) | (unicastMacAddr[2].b[3] << 16) |
         (unicastMacAddr[2].b[4] << 8) | unicastMacAddr[2].b[5]);

      //Set the upper 16 bits of the MAC address
      ncv7410WriteReg(interface, NCV7410_ADDRFILT3H, NCV7410_ADDRFILTnH_EN |
         (unicastMacAddr[2].b[0] << 8) | unicastMacAddr[2].b[1]);
   }
   else
   {
      ncv7410WriteReg(interface, NCV7410_ADDRFILT3L, 0);
      ncv7410WriteReg(interface, NCV7410_ADDRFILT3H, 0);
   }

   //Read MACCTRL0 register
   value = ncv7410ReadReg(interface, NCV7410_MACCTRL0);

   //Disable broadcast filter
   value &= ~NCV7410_MACCTRL0_BCSF;
   //Enable destination address filter
   value |= NCV7410_MACCTRL0_ADRF;

   //Enable or disable the reception of multicast frames
   if(acceptMulticast)
   {
      //Disable multicast filter
      value &= ~NCV7410_MACCTRL0_MCSF;
   }
   else
   {
      //Enable multicast filter
      value |= NCV7410_MACCTRL0_MCSF;
   }

   //Update MACCTRL0 register
   ncv7410WriteReg(interface, NCV7410_MACCTRL0, value);

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

void ncv7410WriteReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint32_t data)
{
   uint32_t header;

   //Set up a register write operation
   header = NCV7410_CTRL_HEADER_WNR | NCV7410_CTRL_HEADER_AID;
   //The MMS field selects the specific register memory map to access
   header |= (mms << 24) & NCV7410_CTRL_HEADER_MMS;
   //Address of the first register to access
   header |= (address << 8) & NCV7410_CTRL_HEADER_ADDR;
   //Specifies the number of registers to write
   header |= (0 << 1) & NCV7410_CTRL_HEADER_LEN;

   //The parity bit is calculated over the control command header
   if(ncv7410CalcParity(header) != 0)
   {
      header |= NCV7410_CTRL_HEADER_P;
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

uint32_t ncv7410ReadReg(NetInterface *interface, uint8_t mms,
   uint16_t address)
{
   uint32_t data;
   uint32_t header;

   //Set up a register read operation
   header = NCV7410_CTRL_HEADER_AID;
   //The MMS field selects the specific register memory map to access
   header |= (mms << 24) & NCV7410_CTRL_HEADER_MMS;
   //Address of the first register to access
   header |= (address << 8) & NCV7410_CTRL_HEADER_ADDR;
   //Specifies the number of registers to read
   header |= (0 << 1) & NCV7410_CTRL_HEADER_LEN;

   //The parity bit is calculated over the control command header
   if(ncv7410CalcParity(header) != 0)
   {
      header |= NCV7410_CTRL_HEADER_P;
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

void ncv7410DumpReg(NetInterface *interface, uint8_t mms, uint16_t address,
   uint_t num)
{
   uint_t i;

   //Loop through registers
   for(i = 0; i < num; i++)
   {
      //Display current register
      TRACE_DEBUG("0x%02" PRIX16 ": 0x%08" PRIX32 "\r\n", address + i,
         ncv7410ReadReg(interface, mms, address + i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Calculate parity bit over a 32-bit data
 * @param[in] data 32-bit bit stream
 * @return Odd parity bit computed over the supplied data
 **/

uint32_t ncv7410CalcParity(uint32_t data)
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
