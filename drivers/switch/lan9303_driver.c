/**
 * @file lan9303_driver.c
 * @brief LAN9303 3-port Ethernet switch
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
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
 * @version 1.9.6
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "drivers/switch/lan9303_driver.h"
#include "debug.h"


/**
 * @brief LAN9303 Ethernet switch driver
 **/

const PhyDriver lan9303PhyDriver =
{
   lan9303Init,
   lan9303Tick,
   lan9303EnableIrq,
   lan9303DisableIrq,
   lan9303EventHandler,
   lan9303TagFrame,
   lan9303UntagFrame
};


/**
 * @brief LAN9303 Ethernet switch initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t lan9303Init(NetInterface *interface)
{
   uint_t port;
   uint32_t value;

   //Debug message
   TRACE_INFO("Initializing LAN9303...\r\n");

   //Chip-level reset/configuration completion can be determined by first
   //polling the BYTE_TEST register
   do
   {
      //Read BYTE_TEST register
      value = lan9303ReadSysReg(interface, LAN9303_BYTE_TEST);

      //The returned data is invalid until the serial interface reset is
      //complete
   } while(value != LAN9303_BYTE_TEST_DEFAULT);

   //The completion of the entire chip-level reset must then be determined
   //by polling the READY bit of the HW_CFG register
   do
   {
      //Read HW_CFG register
      value = lan9303ReadSysReg(interface, LAN9303_HW_CFG);

      //When set, the READY bit indicates that the reset has completed and
      //the device is ready to be accessed
   } while((value & LAN9303_HW_CFG_DEVICE_READY) == 0);

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Special VLAN tagging mode?
   if(interface->port != 0)
   {
      //Enable special VLAN tagging mode
      lan9303WriteSwitchReg(interface, LAN9303_SWE_INGRSS_PORT_TYP,
         LAN9303_SWE_INGRSS_PORT_TYP_PORT0);

      //Configure egress VLAN tagging rules
      lan9303WriteSwitchReg(interface, LAN9303_BM_EGRSS_PORT_TYPE,
         LAN9303_BM_EGRSS_PORT_TYPE_PORT0_CPU);

      //Configure port mirroring
      lan9303WriteSwitchReg(interface, LAN9303_SWE_PORT_MIRROR,
         LAN9303_SWE_PORT_MIRROR_RX_MIRRORING_FILT_EN |
         LAN9303_SWE_PORT_MIRROR_SNIFFER_PORT0 |
         LAN9303_SWE_PORT_MIRROR_MIRRORED_PORT2 |
         LAN9303_SWE_PORT_MIRROR_MIRRORED_PORT1 |
         LAN9303_SWE_PORT_MIRROR_RX_MIRRORING_EN);

      //Configure port state
      lan9303WriteSwitchReg(interface, LAN9303_SWE_PORT_STATE,
         LAN9303_SWE_PORT_STATE_PORT2_LISTENING |
         LAN9303_SWE_PORT_STATE_PORT1_LISTENING |
         LAN9303_SWE_PORT_STATE_PORT0_FORWARDING);
   }
   else
#endif
   {
      //Disable special VLAN tagging mode
      lan9303WriteSwitchReg(interface, LAN9303_SWE_INGRSS_PORT_TYP, 0);

      //Revert to default configuration
      lan9303WriteSwitchReg(interface, LAN9303_BM_EGRSS_PORT_TYPE, 0);
      lan9303WriteSwitchReg(interface, LAN9303_SWE_PORT_MIRROR, 0);
      lan9303WriteSwitchReg(interface, LAN9303_SWE_PORT_STATE, 0);
   }

   //Configure port 0 receive parameters
   lan9303WriteSwitchReg(interface, LAN9303_MAC_RX_CFG(0),
      LAN9303_MAC_RX_CFG_REJECT_MAC_TYPES | LAN9303_MAC_RX_CFG_RX_EN);

   //Configure port 0 transmit parameters
   lan9303WriteSwitchReg(interface, LAN9303_MAC_TX_CFG(0),
      LAN9303_MAC_TX_CFG_IFG_CONFIG_DEFAULT | LAN9303_MAC_TX_CFG_TX_PAD_EN |
      LAN9303_MAC_TX_CFG_TX_EN);

   //Loop through ports
   for(port = LAN9303_PORT1; port <= LAN9303_PORT2; port++)
   {
      //Debug message
      TRACE_DEBUG("Port %u:\r\n", port);
      //Dump PHY registers for debugging purpose
      lan9303DumpPhyReg(interface, port);
   }

   //Force the TCP/IP stack to poll the link state at startup
   interface->phyEvent = TRUE;
   //Notify the TCP/IP stack of the event
   osSetEvent(&netEvent);

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Get link state
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @return Link state
 **/

bool_t lan9303GetLinkState(NetInterface *interface, uint8_t port)
{
   uint16_t status;
   bool_t linkState;

   //Check port number
   if(port >= LAN9303_PORT1 && port <= LAN9303_PORT2)
   {
      //Get exclusive access
      osAcquireMutex(&netMutex);
      //Read status register
      status = lan9303ReadPhyReg(interface, port, LAN9303_BMSR);
      //Release exclusive access
      osReleaseMutex(&netMutex);

      //Retrieve current link state
      linkState = (status & LAN9303_BMSR_LINK_STATUS) ? TRUE : FALSE;
   }
   else
   {
      //The specified port number is not valid
      linkState = FALSE;
   }

   //Return link status
   return linkState;
}


/**
 * @brief LAN9303 timer handler
 * @param[in] interface Underlying network interface
 **/

void lan9303Tick(NetInterface *interface)
{
   uint_t port;
   uint16_t status;
   bool_t linkState;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Special VLAN tagging mode?
   if(interface->port != 0)
   {
      uint_t i;
      NetInterface *virtualInterface;

      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Point to the current interface
         virtualInterface = &netInterface[i];

         //Check whether the current virtual interface is attached to the
         //physical interface
         if(virtualInterface == interface || virtualInterface->parent == interface)
         {
            //The VLAN identifier is used to indicate the source/destination port
            port = virtualInterface->port;

            //Valid port?
            if(port >= LAN9303_PORT1 && port <= LAN9303_PORT2)
            {
               //Read status register
               status = lan9303ReadPhyReg(interface, port, LAN9303_BMSR);

               //Retrieve current link state
               linkState = (status & LAN9303_BMSR_LINK_STATUS) ? TRUE : FALSE;

               //Link up or link down event?
               if(linkState != virtualInterface->linkState)
               {
                  //Set event flag
                  interface->phyEvent = TRUE;
                  //Notify the TCP/IP stack of the event
                  osSetEvent(&netEvent);
               }
            }
         }
      }
   }
   else
#endif
   {
      //Initialize link state
      linkState = FALSE;

      //Loop through ports
      for(port = LAN9303_PORT1; port <= LAN9303_PORT2; port++)
      {
         //Read status register
         status = lan9303ReadPhyReg(interface, port, LAN9303_BMSR);

         //Retrieve current link state
         if(status & LAN9303_BMSR_LINK_STATUS)
            linkState = TRUE;
      }

      //Link up or link down event?
      if(linkState != interface->linkState)
      {
         //Set event flag
         interface->phyEvent = TRUE;
         //Notify the TCP/IP stack of the event
         osSetEvent(&netEvent);
      }
   }
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void lan9303EnableIrq(NetInterface *interface)
{
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void lan9303DisableIrq(NetInterface *interface)
{
}


/**
 * @brief LAN9303 event handler
 * @param[in] interface Underlying network interface
 **/

void lan9303EventHandler(NetInterface *interface)
{
   uint_t port;
   uint16_t status;
   bool_t linkState;

#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   //Special VLAN tagging mode?
   if(interface->port != 0)
   {
      uint_t i;
      NetInterface *virtualInterface;

      //Loop through network interfaces
      for(i = 0; i < NET_INTERFACE_COUNT; i++)
      {
         //Point to the current interface
         virtualInterface = &netInterface[i];

         //Check whether the current virtual interface is attached to the
         //physical interface
         if(virtualInterface == interface || virtualInterface->parent == interface)
         {
            //The VLAN identifier is used to indicate the source/destination port
            port = virtualInterface->port;

            //Valid port?
            if(port >= LAN9303_PORT1 && port <= LAN9303_PORT2)
            {
               //Read status register
               status = lan9303ReadPhyReg(interface, port, LAN9303_BMSR);

               //Retrieve current link state
               linkState = (status & LAN9303_BMSR_LINK_STATUS) ? TRUE : FALSE;

               //Link up event?
               if(linkState && !virtualInterface->linkState)
               {
                  //Adjust MAC configuration parameters for proper operation
                  interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                  interface->duplexMode = NIC_FULL_DUPLEX_MODE;
                  interface->nicDriver->updateMacConfig(interface);

                  //Read PHY special control/status register
                  status = lan9303ReadPhyReg(interface, port, LAN9303_PSCSR);

                  //Check current operation mode
                  switch(status & LAN9303_PSCSR_SPEED)
                  {
                  //10BASE-T half-duplex
                  case LAN9303_PSCSR_SPEED_10BT_HD:
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_10MBPS;
                     virtualInterface->duplexMode = NIC_HALF_DUPLEX_MODE;
                     break;
                  //10BASE-T full-duplex
                  case LAN9303_PSCSR_SPEED_10BT_FD:
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_10MBPS;
                     virtualInterface->duplexMode = NIC_FULL_DUPLEX_MODE;
                     break;
                  //100BASE-TX half-duplex
                  case LAN9303_PSCSR_SPEED_100BTX_HD:
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                     virtualInterface->duplexMode = NIC_HALF_DUPLEX_MODE;
                     break;
                  //100BASE-TX full-duplex
                  case LAN9303_PSCSR_SPEED_100BTX_FD:
                     virtualInterface->linkSpeed = NIC_LINK_SPEED_100MBPS;
                     virtualInterface->duplexMode = NIC_FULL_DUPLEX_MODE;
                     break;
                  //Unknown operation mode
                  default:
                     //Debug message
                     TRACE_WARNING("Invalid operation mode!\r\n");
                     break;
                  }

                  //Update link state
                  virtualInterface->linkState = TRUE;

                  //Process link state change event
                  nicNotifyLinkChange(virtualInterface);
               }
               //Link down event
               else if(!linkState && virtualInterface->linkState)
               {
                  //Update link state
                  virtualInterface->linkState = FALSE;

                  //Process link state change event
                  nicNotifyLinkChange(virtualInterface);
               }
            }
         }
      }
   }
   else
#endif
   {
      //Initialize link state
      linkState = FALSE;

      //Loop through ports
      for(port = LAN9303_PORT1; port <= LAN9303_PORT2; port++)
      {
         //Read status register
         status = lan9303ReadPhyReg(interface, port, LAN9303_BMSR);

         //Retrieve current link state
         if(status & LAN9303_BMSR_LINK_STATUS)
            linkState = TRUE;
      }

      //Link up event?
      if(linkState)
      {
         //Adjust MAC configuration parameters for proper operation
         interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
         interface->duplexMode = NIC_FULL_DUPLEX_MODE;
         interface->nicDriver->updateMacConfig(interface);

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


/**
 * @brief Add special VLAN tag to Ethernet frame
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the payload
 * @param[in,out] offset Offset to the first payload byte
 * @param[in] port Switch port identifier
 * @param[in,out] type Ethernet type
 * @return Error code
 **/

error_t lan9303TagFrame(NetInterface *interface, NetBuffer *buffer,
   size_t *offset, uint8_t port, uint16_t *type)
{
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   error_t error;
   VlanTag *vlanTag;

   //Valid port?
   if(port >= LAN9303_PORT1 && port <= LAN9303_PORT2)
   {
      //Is there enough space for the VLAN tag?
      if(*offset >= sizeof(VlanTag))
      {
         //Make room for the VLAN tag
         *offset -= sizeof(VlanTag);
         //Point to the special VLAN tag
         vlanTag = netBufferAt(buffer, *offset);

         //Bits 0 and 1 of the VID field specify the destination port
         vlanTag->tci = LAN9303_VLAN_ID_ENCODE(port);
         //The EtherType field indicates which protocol is encapsulated in the payload
         vlanTag->type = htons(*type);

         //A distinct Ethertype has been allocated for use in the TPID field
         *type = ETH_TYPE_VLAN;

         //Successful processing
         error = NO_ERROR;
      }
      else
      {
         //Report an error
         error = ERROR_INVALID_PARAMETER;
      }
   }
   else
   {
      //Invalid port
      error = ERROR_WRONG_IDENTIFIER;
   }

   //Return status code
   return error;
#else
   //VLAN tagging mode is not implemented
   return NO_ERROR;
#endif
}


/**
 * @brief Decode special VLAN tag from incoming Ethernet frame
 * @param[in] interface Underlying network interface
 * @param[in,out] frame Pointer to the received Ethernet frame
 * @param[in,out] length Length of the frame, in bytes
 * @param[out] port Switch port identifier
 * @return Error code
 **/

error_t lan9303UntagFrame(NetInterface *interface, uint8_t **frame,
   size_t *length, uint8_t *port)
{
#if (ETH_PORT_TAGGING_SUPPORT == ENABLED)
   error_t error;
   EthHeader *header;
   VlanTag *vlanTag;

   //Point to the Ethernet frame header
   header = (EthHeader *) *frame;

   //Check whether VLAN tagging is used
   if(ntohs(header->type) == ETH_TYPE_VLAN)
   {
      //Valid Ethernet frame received?
      if(*length >= (sizeof(EthHeader) + sizeof(VlanTag)))
      {
         //Point to the special VLAN tag
         vlanTag = (VlanTag *) header->data;

         //Bits 0 and 1 of the VID field specify the source port
         *port = LAN9303_VLAN_ID_DECODE(vlanTag->tci);

         //Fix the value of the EtherType field
         header->type = vlanTag->type;
         //Strip VLAN tag from Ethernet frame
         memmove(*frame + sizeof(VlanTag), *frame, sizeof(EthHeader));

         //Point to the Ethernet frame header
         *frame += sizeof(VlanTag);
         //Adjust the length of the frame
         *length -= sizeof(VlanTag);

         //Successful processing
         error = NO_ERROR;
      }
      else
      {
         //Drop the received frame
         error = ERROR_INVALID_LENGTH;
      }
   }
   else
   {
      //If the interface is configured to accept VLAN-tagged frames, then
      //drop the incoming Ethernet frame
      error = ERROR_WRONG_IDENTIFIER;
   }

   //Return status code
   return error;
#else
   //VLAN tagging mode is not implemented
   return NO_ERROR;
#endif
}


/**
 * @brief Write PHY register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @param[in] data Register value
 **/

void lan9303WritePhyReg(NetInterface *interface, uint8_t port,
   uint8_t address, uint16_t data)
{
   //Write the specified PHY register
   interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE, port, address, data);
}


/**
 * @brief Read PHY register
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t lan9303ReadPhyReg(NetInterface *interface, uint8_t port,
   uint8_t address)
{
   //Read the specified PHY register
   return interface->nicDriver->readPhyReg(SMI_OPCODE_READ, port, address);
}


/**
 * @brief Dump PHY registers for debugging purpose
 * @param[in] interface Underlying network interface
 * @param[in] port Port number
 **/

void lan9303DumpPhyReg(NetInterface *interface, uint8_t port)
{
   uint8_t i;

   //Loop through PHY registers
   for(i = 0; i < 32; i++)
   {
      //Display current PHY register
      TRACE_DEBUG("%02" PRIu8 ": 0x%04" PRIX16 "\r\n", i,
         lan9303ReadPhyReg(interface, port, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Write system CSR register
 * @param[in] interface Underlying network interface
 * @param[in] address System register address
 * @param[in] data Register value
 **/

void lan9303WriteSysReg(NetInterface *interface, uint16_t address,
   uint32_t data)
{
   uint8_t phyAddr;
   uint8_t regAddr;

   //PHY address bit 4 is 1 for SMI commands. PHY address 3:0 form system
   //register address bits 9:6
   phyAddr = 0x10 | ((address >> 6) & 0x0F);

   //Register address field forms register address bits 5:1
   regAddr = (address >> 1) & 0x1F;

   //Write the low word of the SMI register
   interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE, phyAddr, regAddr,
      data & 0xFFFF);

   //Write the high word of the SMI register
   interface->nicDriver->writePhyReg(SMI_OPCODE_WRITE, phyAddr, regAddr + 1,
      (data >> 16) & 0xFFFF);
}


/**
 * @brief Read system CSR register
 * @param[in] interface Underlying network interface
 * @param[in] address System register address
 * @return Register value
 **/

uint32_t lan9303ReadSysReg(NetInterface *interface, uint16_t address)
{
   uint8_t phyAddr;
   uint8_t regAddr;
   uint32_t data;

   //PHY address bit 4 is 1 for SMI commands. PHY address 3:0 form system
   //register address bits 9:6
   phyAddr = 0x10 | ((address >> 6) & 0x0F);

   //Register address field forms register address bits 5:1
   regAddr = (address >> 1) & 0x1F;

   //Read the low word of the SMI register
   data = interface->nicDriver->readPhyReg(SMI_OPCODE_READ, phyAddr, regAddr);

   //Read the high word of the SMI register
   data |= interface->nicDriver->readPhyReg(SMI_OPCODE_READ, phyAddr,
      regAddr + 1) << 16;

   //Return register value
   return data;
}


/**
 * @brief Dump system CSR registers for debugging purpose
 * @param[in] interface Underlying network interface
 **/

void lan9303DumpSysReg(NetInterface *interface)
{
   uint16_t i;

   //Loop through system registers
   for(i = 80; i < 512; i += 4)
   {
      //Display current system register
      TRACE_DEBUG("0x%03" PRIX16 ": 0x%08" PRIX32 "\r\n", i,
         lan9303ReadSysReg(interface, i));
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");
}


/**
 * @brief Write switch fabric CSR register
 * @param[in] interface Underlying network interface
 * @param[in] address Switch fabric register address
 * @param[in] data Register value
 **/

void lan9303WriteSwitchReg(NetInterface *interface, uint16_t address,
   uint32_t data)
{
   uint32_t value;

   //To perform a write to an individual switch fabric register, the desired
   //data must first be written into the SWITCH_CSR_DATA register
   lan9303WriteSysReg(interface, LAN9303_SWITCH_CSR_DATA, data);

   //Set up a write operation
   value = LAN9303_SWITCH_CSR_CMD_BUSY | LAN9303_SWITCH_CSR_CMD_BE;
   //Set register address
   value |= address & LAN9303_SWITCH_CSR_CMD_ADDR;

   //The write cycle is initiated by performing a single write to the
   //SWITCH_CSR_CMD register
   lan9303WriteSysReg(interface, LAN9303_SWITCH_CSR_CMD, value);

   //The completion of the write cycle is indicated by the clearing of the
   //CSR_BUSY bit
   do
   {
      //Read SWITCH_CSR_CMD register
      value = lan9303ReadSysReg(interface, LAN9303_SWITCH_CSR_CMD);

      //Poll CSR_BUSY bit
   } while(value & LAN9303_SWITCH_CSR_CMD_BUSY);
}


/**
 * @brief Read switch fabric CSR register
 * @param[in] interface Underlying network interface
 * @param[in] address Switch fabric register address
 * @return Register value
 **/

uint32_t lan9303ReadSwitchReg(NetInterface *interface, uint16_t address)
{
   uint32_t value;

   //Set up a read operation
   value = LAN9303_SWITCH_CSR_CMD_BUSY | LAN9303_SWITCH_CSR_CMD_READ |
      LAN9303_SWITCH_CSR_CMD_BE;

   //Set register address
   value |= address & LAN9303_SWITCH_CSR_CMD_ADDR;

   //To perform a read of an individual switch fabric register, the read cycle
   //must be initiated by performing a single write to the SWITCH_CSR_CMD
   //register
   lan9303WriteSysReg(interface, LAN9303_SWITCH_CSR_CMD, value);

   //Valid data is available for reading when the CSR_BUSY bit is cleared
   do
   {
      //Read SWITCH_CSR_CMD register
      value = lan9303ReadSysReg(interface, LAN9303_SWITCH_CSR_CMD);

      //Poll CSR_BUSY bit
   } while(value & LAN9303_SWITCH_CSR_CMD_BUSY);

   //Read data from the SWITCH_CSR_DATA register
   return lan9303ReadSysReg(interface, LAN9303_SWITCH_CSR_DATA);
}
