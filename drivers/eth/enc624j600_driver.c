/**
 * @file enc624j600_driver.c
 * @brief ENC624J600/ENC424J600 Ethernet controller
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
#include "core/net.h"
#include "drivers/eth/enc624j600_driver.h"
#include "debug.h"


/**
 * @brief ENC624J600 driver
 **/

const NicDriver enc624j600Driver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   enc624j600Init,
   enc624j600Tick,
   enc624j600EnableIrq,
   enc624j600DisableIrq,
   enc624j600EventHandler,
   enc624j600SendPacket,
   enc624j600SetMulticastFilter,
   NULL,
   NULL,
   NULL,
   TRUE,
   TRUE,
   TRUE,
   FALSE
};


/**
 * @brief ENC624J600 controller initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t enc624j600Init(NetInterface *interface)
{
   Enc624j600Context *context;

   //Debug message
   TRACE_INFO("Initializing ENC624J600 Ethernet controller...\r\n");

   //Initialize SPI
   interface->spiDriver->init();
   //Initialize external interrupt line
   interface->extIntDriver->init();

   //Point to the driver context
   context = (Enc624j600Context *) interface->nicContext;

   //Initialize driver specific variables
   context->nextPacket = ENC624J600_RX_BUFFER_START;

   //Allocate RX buffer
   context->rxBuffer = memPoolAlloc(ETH_MAX_FRAME_SIZE);
   //Failed to allocate memory?
   if(context->rxBuffer == NULL)
      return ERROR_OUT_OF_MEMORY;

   //Issue a system reset
   enc624j600SoftReset(interface);

   //Disable CLKOUT output
   enc624j600WriteReg(interface, ENC624J600_REG_ECON2, ECON2_ETHEN | ECON2_STRCH);

   //Optionally set the station MAC address
   if(macCompAddr(&interface->macAddr, &MAC_UNSPECIFIED_ADDR))
   {
      //Use the factory preprogrammed station address
      interface->macAddr.w[0] = enc624j600ReadReg(interface, ENC624J600_REG_MAADR1);
      interface->macAddr.w[1] = enc624j600ReadReg(interface, ENC624J600_REG_MAADR2);
      interface->macAddr.w[2] = enc624j600ReadReg(interface, ENC624J600_REG_MAADR3);

      //Generate the 64-bit interface identifier
      macAddrToEui64(&interface->macAddr, &interface->eui64);
   }
   else
   {
      //Override the factory preprogrammed address
      enc624j600WriteReg(interface, ENC624J600_REG_MAADR1, interface->macAddr.w[0]);
      enc624j600WriteReg(interface, ENC624J600_REG_MAADR2, interface->macAddr.w[1]);
      enc624j600WriteReg(interface, ENC624J600_REG_MAADR3, interface->macAddr.w[2]);
   }

   //Set receive buffer location
   enc624j600WriteReg(interface, ENC624J600_REG_ERXST, ENC624J600_RX_BUFFER_START);
   //Program the tail pointer ERXTAIL to the last even address of the buffer
   enc624j600WriteReg(interface, ENC624J600_REG_ERXTAIL, ENC624J600_RX_BUFFER_STOP);

   //Configure the receive filters
   enc624j600WriteReg(interface, ENC624J600_REG_ERXFCON, ERXFCON_HTEN |
      ERXFCON_CRCEN | ERXFCON_RUNTEN | ERXFCON_UCEN | ERXFCON_BCEN);

   //Initialize the hash table
   enc624j600WriteReg(interface, ENC624J600_REG_EHT1, 0x0000);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT2, 0x0000);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT3, 0x0000);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT4, 0x0000);

   //All short frames will be zero-padded to 60 bytes and a valid CRC is then appended
   enc624j600WriteReg(interface, ENC624J600_REG_MACON2,
      MACON2_DEFER | MACON2_PADCFG0 | MACON2_TXCRCEN | MACON2_R1);

   //Program the MAMXFL register with the maximum frame length to be accepted
   enc624j600WriteReg(interface, ENC624J600_REG_MAMXFL, 1518);

   //PHY initialization
   enc624j600WritePhyReg(interface, ENC624J600_PHY_REG_PHANA, PHANA_ADPAUS0 |
      PHANA_AD100FD | PHANA_AD100 | PHANA_AD10FD | PHANA_AD10 | PHANA_ADIEEE0);

   //Clear interrupt flags
   enc624j600WriteReg(interface, ENC624J600_REG_EIR, 0x0000);

   //Configure interrupts as desired
   enc624j600WriteReg(interface, ENC624J600_REG_EIE, EIE_INTIE |
      EIE_LINKIE | EIE_PKTIE | EIE_TXIE | EIE_TXABTIE);

   //Set RXEN to enable reception
   enc624j600SetBit(interface, ENC624J600_REG_ECON1, ECON1_RXEN);

   //Dump registers for debugging purpose
   enc624j600DumpReg(interface);
   enc624j600DumpPhyReg(interface);

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
 * @brief ENC624J600 timer handler
 * @param[in] interface Underlying network interface
 **/

void enc624j600Tick(NetInterface *interface)
{
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void enc624j600EnableIrq(NetInterface *interface)
{
   //Enable interrupts
   interface->extIntDriver->enableIrq();
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void enc624j600DisableIrq(NetInterface *interface)
{
   //Disable interrupts
   interface->extIntDriver->disableIrq();
}


/**
 * @brief ENC624J600 interrupt service routine
 * @param[in] interface Underlying network interface
 * @return TRUE if a higher priority task must be woken. Else FALSE is returned
 **/

bool_t enc624j600IrqHandler(NetInterface *interface)
{
   bool_t flag;
   uint16_t status;

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Clear the INTIE bit, immediately after an interrupt event
   enc624j600ClearBit(interface, ENC624J600_REG_EIE, EIE_INTIE);

   //Read interrupt status register
   status = enc624j600ReadReg(interface, ENC624J600_REG_EIR);

   //Link status change?
   if(status & EIR_LINKIF)
   {
      //Disable LINKIE interrupt
      enc624j600ClearBit(interface, ENC624J600_REG_EIE, EIE_LINKIE);

      //Set event flag
      interface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Packet received?
   if(status & EIR_PKTIF)
   {
      //Disable PKTIE interrupt
      enc624j600ClearBit(interface, ENC624J600_REG_EIE, EIE_PKTIE);

      //Set event flag
      interface->nicEvent = TRUE;
      //Notify the TCP/IP stack of the event
      flag |= osSetEventFromIsr(&netEvent);
   }

   //Packet transmission complete?
   if(status & (EIR_TXIF | EIR_TXABTIF))
   {
      //Clear interrupt flags
      enc624j600ClearBit(interface, ENC624J600_REG_EIR, EIR_TXIF | EIR_TXABTIF);

      //Notify the TCP/IP stack that the transmitter is ready to send
      flag |= osSetEventFromIsr(&interface->nicTxEvent);
   }

   //Once the interrupt has been serviced, the INTIE bit
   //is set again to re-enable interrupts
   enc624j600SetBit(interface, ENC624J600_REG_EIE, EIE_INTIE);

   //A higher priority task must be woken?
   return flag;
}


/**
 * @brief ENC624J600 event handler
 * @param[in] interface Underlying network interface
 **/

void enc624j600EventHandler(NetInterface *interface)
{
   error_t error;
   uint16_t status;
   uint16_t value;

   //Read interrupt status register
   status = enc624j600ReadReg(interface, ENC624J600_REG_EIR);

   //Check whether the link state has changed
   if(status & EIR_LINKIF)
   {
      //Clear interrupt flag
      enc624j600ClearBit(interface, ENC624J600_REG_EIR, EIR_LINKIF);
      //Read Ethernet status register
      value = enc624j600ReadReg(interface, ENC624J600_REG_ESTAT);

      //Check link state
      if(value & ESTAT_PHYLNK)
      {
         //Read PHY status register 3
         value = enc624j600ReadPhyReg(interface, ENC624J600_PHY_REG_PHSTAT3);

         //Get current speed
         if(value & PHSTAT3_SPDDPX1)
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
         else
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;

         //Determine the new duplex mode
         if(value & PHSTAT3_SPDDPX2)
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
         else
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;

         //Link is up
         interface->linkState = TRUE;

         //Update MAC configuration parameters for proper operation
         enc624j600UpdateMacConfig(interface);
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
      enc624j600ClearBit(interface, ENC624J600_REG_EIR, EIR_PKTIF);

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = enc624j600ReceivePacket(interface);

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }

   //Re-enable LINKIE and PKTIE interrupts
   enc624j600SetBit(interface, ENC624J600_REG_EIE, EIE_LINKIE | EIE_PKTIE);
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t enc624j600SendPacket(NetInterface *interface,
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

   //Ensure that the transmitter is ready to send
   if(enc624j600ReadReg(interface, ENC624J600_REG_ECON1) & ECON1_TXRTS)
      return ERROR_FAILURE;

   //Point to the SRAM buffer
   enc624j600WriteReg(interface, ENC624J600_REG_EGPWRPT, ENC624J600_TX_BUFFER_START);
   //Copy the packet to the SRAM buffer
   enc624j600WriteBuffer(interface, ENC624J600_CMD_WGPDATA, buffer, offset);

   //Program ETXST to the start address of the packet
   enc624j600WriteReg(interface, ENC624J600_REG_ETXST, ENC624J600_TX_BUFFER_START);
   //Program ETXLEN with the length of data copied to the memory
   enc624j600WriteReg(interface, ENC624J600_REG_ETXLEN, length);

   //Clear TXIF and TXABTIF interrupt flags
   enc624j600ClearBit(interface, ENC624J600_REG_EIR, EIR_TXIF | EIR_TXABTIF);
   //Set the TXRTS bit to initiate transmission
   enc624j600SetBit(interface, ENC624J600_REG_ECON1, ECON1_TXRTS);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Receive a packet
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t enc624j600ReceivePacket(NetInterface *interface)
{
   error_t error;
   uint16_t n;
   uint32_t status;
   Enc624j600Context *context;

   //Point to the driver context
   context = (Enc624j600Context *) interface->nicContext;

   //Verify that a packet is waiting by ensuring that PKTCNT is non-zero
   if(enc624j600ReadReg(interface, ENC624J600_REG_ESTAT) & ESTAT_PKTCNT)
   {
      //Point to the next packet
      enc624j600WriteReg(interface, ENC624J600_REG_ERXRDPT, context->nextPacket);

      //Read the first two bytes, which are the address of the next packet
      enc624j600ReadBuffer(interface, ENC624J600_CMD_RRXDATA,
         (uint8_t *) &context->nextPacket, sizeof(uint16_t));

      //Get the length of the received frame in bytes
      enc624j600ReadBuffer(interface, ENC624J600_CMD_RRXDATA,
         (uint8_t *) &n, sizeof(uint16_t));

      //Read the receive status vector (RSV)
      enc624j600ReadBuffer(interface, ENC624J600_CMD_RRXDATA,
         (uint8_t *) &status, sizeof(uint32_t));

      //Make sure no error occurred
      if(status & RSV_RECEIVED_OK)
      {
         //Limit the number of data to read
         n = MIN(n, ETH_MAX_FRAME_SIZE);
         //Read the Ethernet frame
         enc624j600ReadBuffer(interface, ENC624J600_CMD_RRXDATA, context->rxBuffer, n);
         //Valid packet received
         error = NO_ERROR;
      }
      else
      {
         //The received packet contains an error
         error = ERROR_INVALID_PACKET;
      }

      //Update the ERXTAIL pointer value to the point where the packet
      //has been processed, taking care to wrap back at the end of the
      //received memory buffer
      if(context->nextPacket == ENC624J600_RX_BUFFER_START)
         enc624j600WriteReg(interface, ENC624J600_REG_ERXTAIL, ENC624J600_RX_BUFFER_STOP);
      else
         enc624j600WriteReg(interface, ENC624J600_REG_ERXTAIL, context->nextPacket - 2);

      //Set PKTDEC to decrement the PKTCNT bits
      enc624j600SetBit(interface, ENC624J600_REG_ECON1, ECON1_PKTDEC);
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

error_t enc624j600SetMulticastFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint16_t hashTable[4];
   MacFilterEntry *entry;

   //Debug message
   TRACE_DEBUG("Updating ENC624J600 hash table...\r\n");

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
         crc = enc624j600CalcCrc(&entry->addr, sizeof(MacAddr));
         //Calculate the corresponding index in the table
         k = (crc >> 23) & 0x3F;
         //Update hash table contents
         hashTable[k / 16] |= (1 << (k % 16));
      }
   }

   //Write the hash table to the ENC624J600 controller
   enc624j600WriteReg(interface, ENC624J600_REG_EHT1, hashTable[0]);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT2, hashTable[1]);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT3, hashTable[2]);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT4, hashTable[3]);

   //Debug message
   TRACE_DEBUG("  EHT1 = %04" PRIX16 "\r\n", enc624j600ReadReg(interface, ENC624J600_REG_EHT1));
   TRACE_DEBUG("  EHT2 = %04" PRIX16 "\r\n", enc624j600ReadReg(interface, ENC624J600_REG_EHT2));
   TRACE_DEBUG("  EHT3 = %04" PRIX16 "\r\n", enc624j600ReadReg(interface, ENC624J600_REG_EHT3));
   TRACE_DEBUG("  EHT4 = %04" PRIX16 "\r\n", enc624j600ReadReg(interface, ENC624J600_REG_EHT4));

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Adjust MAC configuration parameters for proper operation
 * @param[in] interface Underlying network interface
 **/

void enc624j600UpdateMacConfig(NetInterface *interface)
{
   uint16_t duplexMode;

   //Determine the new duplex mode by reading the PHYDPX bit
   duplexMode = enc624j600ReadReg(interface, ENC624J600_REG_ESTAT) & ESTAT_PHYDPX;

   //Full-duplex mode?
   if(duplexMode)
   {
      //Configure the FULDPX bit to match the current duplex mode
      enc624j600WriteReg(interface, ENC624J600_REG_MACON2, MACON2_DEFER |
         MACON2_PADCFG2 | MACON2_PADCFG0 | MACON2_TXCRCEN | MACON2_R1 | MACON2_FULDPX);
      //Configure the Back-to-Back Inter-Packet Gap register
      enc624j600WriteReg(interface, ENC624J600_REG_MABBIPG, 0x15);
   }
   //Half-duplex mode?
   else
   {
      //Configure the FULDPX bit to match the current duplex mode
      enc624j600WriteReg(interface, ENC624J600_REG_MACON2, MACON2_DEFER |
         MACON2_PADCFG2 | MACON2_PADCFG0 | MACON2_TXCRCEN | MACON2_R1);
      //Configure the Back-to-Back Inter-Packet Gap register
      enc624j600WriteReg(interface, ENC624J600_REG_MABBIPG, 0x12);
   }
}


/**
 * @brief Reset ENC624J600 controller
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t enc624j600SoftReset(NetInterface *interface)
{
   //Wait for the SPI interface to be ready
   do
   {
      //Write 0x1234 to EUDAST
      enc624j600WriteReg(interface, ENC624J600_REG_EUDAST, 0x1234);
      //Read back register and check contents
   } while(enc624j600ReadReg(interface, ENC624J600_REG_EUDAST) != 0x1234);

   //Poll CLKRDY and wait for it to become set
   while(!(enc624j600ReadReg(interface, ENC624J600_REG_ESTAT) & ESTAT_CLKRDY));

   //Issue a system reset command by setting ETHRST
   enc624j600SetBit(interface, ENC624J600_REG_ECON2, ECON2_ETHRST);
   //Wait at least 25us for the reset to take place
   sleep(1);

   //Read EUDAST to confirm that the system reset took place.
   //EUDAST should have reverted back to its reset default
   if(enc624j600ReadReg(interface, ENC624J600_REG_EUDAST) != 0x0000)
      return ERROR_FAILURE;

   //Wait at least 256us for the PHY registers and PHY
   //status bits to become available
   sleep(1);

   //The controller is now ready to accept further commands
   return NO_ERROR;
}


/**
 * @brief Write ENC624J600 register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @param[in] data Register value
 **/

void enc624j600WriteReg(NetInterface *interface, uint8_t address, uint16_t data)
{
   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode
   interface->spiDriver->transfer(ENC624J600_CMD_WCRU);
   //Write register address
   interface->spiDriver->transfer(address);
   //Write register value
   interface->spiDriver->transfer(LSB(data));
   interface->spiDriver->transfer(MSB(data));

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Read ENC624J600 register
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @return Register value
 **/

uint16_t enc624j600ReadReg(NetInterface *interface, uint8_t address)
{
   uint16_t data;

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode
   interface->spiDriver->transfer(ENC624J600_CMD_RCRU);
   //Write register address
   interface->spiDriver->transfer(address);
   //Read the lower 8 bits of data
   data = interface->spiDriver->transfer(0x00);
   //Read the upper 8 bits of data
   data |= interface->spiDriver->transfer(0x00) << 8;

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

void enc624j600WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data)
{
   //Write the address of the PHY register to write to
   enc624j600WriteReg(interface, ENC624J600_REG_MIREGADR, MIREGADR_R8 | address);
   //Write the 16 bits of data into the MIWR register
   enc624j600WriteReg(interface, ENC624J600_REG_MIWR, data);

   //Wait until the PHY register has been written
   while(enc624j600ReadReg(interface, ENC624J600_REG_MISTAT) & MISTAT_BUSY);
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t enc624j600ReadPhyReg(NetInterface *interface, uint8_t address)
{
   //Write the address of the PHY register to read from
   enc624j600WriteReg(interface, ENC624J600_REG_MIREGADR, MIREGADR_R8 | address);
   //Start read operation
   enc624j600WriteReg(interface, ENC624J600_REG_MICMD, MICMD_MIIRD);

   //Wait at least 25.6us before polling the BUSY bit
   usleep(100);
   //Wait for the read operation to complete
   while(enc624j600ReadReg(interface, ENC624J600_REG_MISTAT) & MISTAT_BUSY);

   //Clear command register
   enc624j600WriteReg(interface, ENC624J600_REG_MICMD, 0x00);

   //Return register contents
   return enc624j600ReadReg(interface, ENC624J600_REG_MIRD);
}


/**
 * @brief Write SRAM buffer
 * @param[in] interface Underlying network interface
 * @param[in] opcode SRAM buffer operation
 * @param[in] buffer Multi-part buffer containing the data to be written
 * @param[in] offset Offset to the first data byte
 **/

void enc624j600WriteBuffer(NetInterface *interface,
   uint8_t opcode, const NetBuffer *buffer, size_t offset)
{
   uint_t i;
   size_t j;
   size_t n;
   uint8_t *p;

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode
   interface->spiDriver->transfer(opcode);

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
 * @param[in] opcode SRAM buffer operation
 * @param[in] data Buffer where to store the incoming data
 * @param[in] length Number of data to read
 **/

void enc624j600ReadBuffer(NetInterface *interface,
   uint8_t opcode, uint8_t *data, size_t length)
{
   size_t i;

   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode
   interface->spiDriver->transfer(opcode);

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

void enc624j600SetBit(NetInterface *interface, uint8_t address, uint16_t mask)
{
   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode
   interface->spiDriver->transfer(ENC624J600_CMD_BFSU);
   //Write register address
   interface->spiDriver->transfer(address);
   //Write bit mask
   interface->spiDriver->transfer(LSB(mask));
   interface->spiDriver->transfer(MSB(mask));

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief Clear bit field
 * @param[in] interface Underlying network interface
 * @param[in] address Register address
 * @param[in] mask Bits to clear in the target register
 **/

void enc624j600ClearBit(NetInterface *interface, uint8_t address, uint16_t mask)
{
   //Pull the CS pin low
   interface->spiDriver->assertCs();

   //Write opcode
   interface->spiDriver->transfer(ENC624J600_CMD_BFCU);
   //Write register address
   interface->spiDriver->transfer(address);
   //Write bit mask
   interface->spiDriver->transfer(LSB(mask));
   interface->spiDriver->transfer(MSB(mask));

   //Terminate the operation by raising the CS pin
   interface->spiDriver->deassertCs();
}


/**
 * @brief CRC calculation using the polynomial 0x4C11DB7
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t enc624j600CalcCrc(const void *data, size_t length)
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

void enc624j600DumpReg(NetInterface *interface)
{
#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   uint8_t i;
   uint8_t bank;
   uint16_t address;

   //Display header
   TRACE_DEBUG("    Bank 0  Bank 1  Bank 2  Bank 3  Unbanked\r\n");

   //Loop through register addresses
   for(i = 0; i < 32; i += 2)
   {
      //Display register address
      TRACE_DEBUG("%02" PRIX8 ": ", i);

      //Loop through bank numbers
      for(bank = 0; bank < 5; bank++)
      {
         //Format register address
         address = 0x7E00 | (bank << 5) | i;
         //Display register contents
         TRACE_DEBUG("0x%04" PRIX16 "  ", enc624j600ReadReg(interface, address));
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

void enc624j600DumpPhyReg(NetInterface *interface)
{
#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIX8 ": 0x%04" PRIX16 "\r\n", i, enc624j600ReadPhyReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
#endif
}
