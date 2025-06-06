/**
 * @file w3150a_driver.c
 * @brief WIZnet W3150A+ Ethernet controller
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
#include "drivers/eth/w3150a_driver.h"
#include "debug.h"


/**
 * @brief W3150A+ driver
 **/

const NicDriver w3150aDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   w3150aInit,
   w3150aTick,
   w3150aEnableIrq,
   w3150aDisableIrq,
   w3150aEventHandler,
   w3150aSendPacket,
   w3150aUpdateMacAddrFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   TRUE
};


/**
 * @brief W3150A+ controller initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t w3150aInit(NetInterface *interface)
{
   uint8_t value;

   //Debug message
   TRACE_INFO("Initializing W3150A+ Ethernet controller...\r\n");

   //Initialize SPI interface
   interface->spiDriver->init();

   //Initialize external interrupt line driver
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->init();
   }

   //Perform software reset
   w3150aWriteReg8(interface, W3150A_MR, W3150A_MR_RST);

   //Wait for reset completion
   do
   {
      //Read mode register
      value = w3150aReadReg8(interface, W3150A_MR);

      //The RST bit is automatically cleared after reset completion
   } while((value & W3150A_MR_RST) != 0);

   //Set the MAC address of the station
   w3150aWriteReg8(interface, W3150A_SHAR0, interface->macAddr.b[0]);
   w3150aWriteReg8(interface, W3150A_SHAR1, interface->macAddr.b[1]);
   w3150aWriteReg8(interface, W3150A_SHAR2, interface->macAddr.b[2]);
   w3150aWriteReg8(interface, W3150A_SHAR3, interface->macAddr.b[3]);
   w3150aWriteReg8(interface, W3150A_SHAR4, interface->macAddr.b[4]);
   w3150aWriteReg8(interface, W3150A_SHAR5, interface->macAddr.b[5]);

   //Set TX buffer size
   w3150aWriteReg8(interface, W3150A_TMSR,
      W3150A_TMSR_SOCKET0_8KB | W3150A_TMSR_SOCKET1_1KB |
      W3150A_TMSR_SOCKET2_1KB | W3150A_TMSR_SOCKET3_1KB);

   //Set RX buffer size
   w3150aWriteReg8(interface, W3150A_RMSR,
      W3150A_RMSR_SOCKET0_8KB | W3150A_RMSR_SOCKET1_1KB |
      W3150A_RMSR_SOCKET2_1KB | W3150A_RMSR_SOCKET3_1KB);

   //Configure socket 0 in MACRAW mode
   w3150aWriteReg8(interface, W3150A_S0_MR, W3150A_Sn_MR_PROTOCOL_MACRAW);

   //Open socket 0
   w3150aWriteReg8(interface, W3150A_S0_CR, W3150A_Sn_CR_OPEN);

   //Wait for command completion
   do
   {
      //Read status register
      value = w3150aReadReg8(interface, W3150A_S0_SR);

      //Check the status of the socket
   } while(value != W3150A_Sn_SR_SOCK_MACRAW);

   //Enable socket 0 interrupts
   w3150aWriteReg8(interface, W3150A_IMR, W3150A_IMR_IM_IR0);

   //Perform custom configuration
   w3150aInitHook(interface);

   //Dump registers for debugging purpose
   w3150aDumpReg(interface);

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
 * @brief W3150A+ custom configuration
 * @param[in] interface Underlying network interface
 **/

__weak_func void w3150aInitHook(NetInterface *interface)
{
}


/**
 * @brief W3150A+ timer handler
 * @param[in] interface Underlying network interface
 **/

__weak_func void w3150aTick(NetInterface *interface)
{
   //Check link state
   if(!interface->linkState)
   {
      //Link is always up
      interface->linkState = TRUE;
      //Process link state change event
      nicNotifyLinkChange(interface);
   }
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void w3150aEnableIrq(NetInterface *interface)
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

void w3150aDisableIrq(NetInterface *interface)
{
   //Disable interrupts
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->disableIrq();
   }
}


/**
 * @brief W3150A+ interrupt service routine
 * @param[in] interface Underlying network interface
 * @return TRUE if a higher priority task must be woken. Else FALSE is returned
 **/

bool_t w3150aIrqHandler(NetInterface *interface)
{
   bool_t flag;
   uint16_t n;
   uint8_t isr;

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read socket interrupt register
   isr = w3150aReadReg8(interface, W3150A_IR);
   //Disable interrupts to release the interrupt line
   w3150aWriteReg8(interface, W3150A_IMR, 0);

   //Socket 0 interrupt?
   if((isr & W3150A_IR_S0_INT) != 0)
   {
      //Read socket 0 interrupt register
      isr = w3150aReadReg8(interface, W3150A_S0_IR);

      //Packet transmission complete?
      if((isr & W3150A_Sn_IR_SEND_OK) != 0)
      {
         //Get the amount of free memory available in the TX buffer
         n = w3150aReadReg16(interface, W3150A_S0_TX_FSR0);

         //Check whether the TX buffer is available for writing
         if(n >= ETH_MAX_FRAME_SIZE)
         {
            //The transmitter can accept another packet
            osSetEvent(&interface->nicTxEvent);
         }
      }

      //Packet received?
      if((isr & W3150A_Sn_IR_RECV) != 0)
      {
         //Set event flag
         interface->nicEvent = TRUE;
         //Notify the TCP/IP stack of the event
         flag |= osSetEventFromIsr(&netEvent);
      }

      //Clear interrupt flags
      w3150aWriteReg8(interface, W3150A_S0_IR, isr &
         (W3150A_Sn_IR_SEND_OK | W3150A_Sn_IR_RECV));
   }

   //Re-enable interrupts once the interrupt has been serviced
   w3150aWriteReg8(interface, W3150A_IMR, W3150A_IMR_IM_IR0);

   //A higher priority task must be woken?
   return flag;
}


/**
 * @brief W3150A+ event handler
 * @param[in] interface Underlying network interface
 **/

void w3150aEventHandler(NetInterface *interface)
{
   error_t error;

   //Process all pending packets
   do
   {
      //Read incoming packet
      error = w3150aReceivePacket(interface);

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

error_t w3150aSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   static uint8_t temp[W3150A_ETH_TX_BUFFER_SIZE];
   uint16_t n;
   size_t length;

   //Retrieve the length of the packet
   length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > ETH_MAX_FRAME_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Get the amount of free memory available in the TX buffer
   n = w3150aReadReg16(interface, W3150A_S0_TX_FSR0);

   //Make sure the TX buffer is available for writing
   if(n < length)
      return ERROR_FAILURE;

   //Copy user data
   netBufferRead(temp, buffer, offset, length);

   //Write packet data
   w3150aWriteData(interface, temp, length);

   //Get the amount of free memory available in the TX buffer
   n = w3150aReadReg16(interface, W3150A_S0_TX_FSR0);

   //Check whether the TX buffer is available for writing
   if(n >= ETH_MAX_FRAME_SIZE)
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

error_t w3150aReceivePacket(NetInterface *interface)
{
   static uint8_t temp[W3150A_ETH_RX_BUFFER_SIZE];
   error_t error;
   size_t length;

   //Get the amount of data in the RX buffer
   length = w3150aReadReg16(interface, W3150A_S0_RX_RSR0);

   //Any packet pending in the receive buffer?
   if(length > 0)
   {
      //Read packet header
      w3150aReadData(interface, temp, 2);

      //Retrieve the length of the received packet
      length = LOAD16BE(temp);

      //Ensure the packet size is acceptable
      if(length >= 2 && length <= (ETH_MAX_FRAME_SIZE + 2))
      {
         //Read packet data
         w3150aReadData(interface, temp, length - 2);
         //Successful processing
         error = NO_ERROR;
      }
      else
      {
         //The packet length is not valid
         error = ERROR_INVALID_LENGTH;
      }
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Check whether a valid packet has been received
   if(!error)
   {
      NetRxAncillary ancillary;

      //Additional options can be passed to the stack along with the packet
      ancillary = NET_DEFAULT_RX_ANCILLARY;

      //Pass the packet to the upper layer
      nicProcessPacket(interface, temp, length, &ancillary);
   }

   //Return status code
   return error;
}


/**
 * @brief Configure MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t w3150aUpdateMacAddrFilter(NetInterface *interface)
{
   //Not implemented
   return NO_ERROR;
}


/**
 * @brief Write 8-bit register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @param[in] data Register value
 **/

void w3150aWriteReg8(NetInterface *interface, uint16_t address, uint8_t data)
{
   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Control phase
   interface->spiDriver->transfer(W3150A_CTRL_WRITE);

   //Address phase
   interface->spiDriver->transfer(MSB(address));
   interface->spiDriver->transfer(LSB(address));

   //Data phase
   interface->spiDriver->transfer(data);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Read 8-bit register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @return Register value
 **/

uint8_t w3150aReadReg8(NetInterface *interface, uint16_t address)
{
   uint8_t data;

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Control phase
   interface->spiDriver->transfer(W3150A_CTRL_READ);

   //Address phase
   interface->spiDriver->transfer(MSB(address));
   interface->spiDriver->transfer(LSB(address));

   //Data phase
   data = interface->spiDriver->transfer(0x00);

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();

   //Return register value
   return data;
}


/**
 * @brief Write 16-bit register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @param[in] data Register value
 **/

void w3150aWriteReg16(NetInterface *interface, uint16_t address, uint16_t data)
{
   //Write upper byte
   w3150aWriteReg8(interface, address, MSB(data));
   //Write lower byte
   w3150aWriteReg8(interface, address + 1, LSB(data));
}


/**
 * @brief Read 16-bit register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @return Register value
 **/

uint16_t w3150aReadReg16(NetInterface *interface, uint16_t address)
{
   uint16_t data;
   uint16_t data2;

   //It is recommended to read all 16-bit registers twice or more until
   //getting the same value
   do
   {
      //Read upper byte
      data = w3150aReadReg8(interface, address) << 8;
      //Read lower byte
      data |= w3150aReadReg8(interface, address + 1);

      //Read upper byte
      data2 = w3150aReadReg8(interface, address) << 8;
      //Read lower byte
      data2 |= w3150aReadReg8(interface, address + 1);

      //Compare 16-bit values
   } while(data != data2);

   //Return register value
   return data;
}


/**
 * @brief Write data
 * @param[in] interface Underlying network interface
 * @param[in] data Pointer to the data being written
 * @param[in] length Number of data to write
 **/

void w3150aWriteData(NetInterface *interface, const uint8_t *data,
   size_t length)
{
   size_t p;
   size_t size;
   size_t offset;
   uint8_t value;

   //Read TX memory size register
   value = w3150aReadReg8(interface, W3150A_TMSR);

   //Get TX buffer size
   switch(value & W3150A_TMSR_SOCKET0)
   {
   case W3150A_TMSR_SOCKET0_1KB:
      size = 1024;
      break;
   case W3150A_TMSR_SOCKET0_2KB:
      size = 2048;
      break;
   case W3150A_TMSR_SOCKET0_4KB:
      size = 4096;
      break;
   default:
      size = 8192;
      break;
   }

   //Get TX write pointer
   p = w3150aReadReg16(interface, W3150A_S0_TX_WR0);

   //Retrieve current offset
   offset = p & (size - 1);

   //Check whether the data crosses buffer boundaries
   if((offset + length) < size)
   {
      //Write data
      w3150aWriteBuffer(interface, W3150A_TX_BUFFER + offset, data, length);
   }
   else
   {
      //Write the first part of the data
      w3150aWriteBuffer(interface, W3150A_TX_BUFFER + offset, data,
         size - offset);

      //Wrap around to the beginning of the circular buffer
      w3150aWriteBuffer(interface, W3150A_TX_BUFFER,
         data + size - offset, offset + length - size);
   }

   //Increment TX write pointer
   w3150aWriteReg16(interface, W3150A_S0_TX_WR0, p + length);

   //Start packet transmission
   w3150aWriteReg8(interface, W3150A_S0_CR, W3150A_Sn_CR_SEND);
}


/**
 * @brief Read data
 * @param[in] interface Underlying network interface
 * @param[out] data Buffer where to store the incoming data
 * @param[in] length Number of data to read
 **/

void w3150aReadData(NetInterface *interface, uint8_t *data, size_t length)
{
   size_t p;
   size_t size;
   size_t offset;
   uint8_t value;

   //Read RX memory size register
   value = w3150aReadReg8(interface, W3150A_RMSR);

   //Get RX buffer size
   switch(value & W3150A_RMSR_SOCKET0)
   {
   case W3150A_RMSR_SOCKET0_1KB:
      size = 1024;
      break;
   case W3150A_RMSR_SOCKET0_2KB:
      size = 2048;
      break;
   case W3150A_RMSR_SOCKET0_4KB:
      size = 4096;
      break;
   default:
      size = 8192;
      break;
   }

   //Get RX read pointer
   p = w3150aReadReg16(interface, W3150A_S0_RX_RD0);

   //Retrieve current offset
   offset = p & (size - 1);

   //Check whether the data crosses buffer boundaries
   if((offset + length) < size)
   {
      //Read data
      w3150aReadBuffer(interface, W3150A_RX_BUFFER + offset, data, length);
   }
   else
   {
      //Read the first part of the data
      w3150aReadBuffer(interface, W3150A_RX_BUFFER + offset, data,
         size - offset);

      //Wrap around to the beginning of the circular buffer
      w3150aReadBuffer(interface, W3150A_RX_BUFFER,
         data + size - offset, offset + length - size);
   }

   //Increment RX read pointer
   w3150aWriteReg16(interface, W3150A_S0_RX_RD0, p + length);

   //Complete the processing of the receive data
   w3150aWriteReg8(interface, W3150A_S0_CR, W3150A_Sn_CR_RECV);
}


/**
 * @brief Write TX buffer
 * @param[in] interface Underlying network interface
 * @param[in] address Buffer address
 * @param[in] data Pointer to the data being written
 * @param[in] length Number of data to write
 **/

void w3150aWriteBuffer(NetInterface *interface, uint16_t address,
   const uint8_t *data, size_t length)
{
   size_t i;

   //W3150A+ operates in unit of 32-bit stream
   for(i = 0; i < length; i++)
   {
      //The unit of 32-bit stream is composed of a 1-byte opcode field,
      //a 2-byte address field and a 1-byte data field
      w3150aWriteReg8(interface, address + i, data[i]);
   }
}


/**
 * @brief Read RX buffer
 * @param[in] interface Underlying network interface
 * @param[in] address Buffer address
 * @param[out] data Buffer where to store the incoming data
 * @param[in] length Number of data to read
 **/

void w3150aReadBuffer(NetInterface *interface, uint16_t address, uint8_t *data,
   size_t length)
{
   size_t i;

   //W3150A+ operates in unit of 32-bit stream
   for(i = 0; i < length; i++)
   {
      //The unit of 32-bit stream is composed of a 1-byte opcode field,
      //a 2-byte address field and a 1-byte data field
      data[i] = w3150aReadReg8(interface, address + i);
   }
}


/**
 * @brief Dump registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void w3150aDumpReg(NetInterface *interface)
{
   uint16_t i;

   //Loop through registers
   for(i = 0; i < 64; i++)
   {
      //Display current host MAC register
      TRACE_DEBUG("%02" PRIX16 ": 0x%02" PRIX8 "\r\n", i,
         w3150aReadReg8(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}
