/**
 * @file enc28j60_driver.h
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

#ifndef _ENC28J60_DRIVER_H
#define _ENC28J60_DRIVER_H

//Full-duplex support
#ifndef ENC28J60_FULL_DUPLEX_SUPPORT
   #define ENC28J60_FULL_DUPLEX_SUPPORT ENABLED
#elif (ENC28J60_FULL_DUPLEX_SUPPORT != ENABLED && ENC28J60_FULL_DUPLEX_SUPPORT != DISABLED)
   #error ENC28J60_FULL_DUPLEX_SUPPORT parameter is not valid
#endif

//Silicon revision identifiers
#define ENC28J60_REV_B1 0x02
#define ENC28J60_REV_B4 0x04
#define ENC28J60_REV_B5 0x05
#define ENC28J60_REV_B7 0x06

//Receive and transmit buffers
#define ENC28J60_RX_BUFFER_START 0x0000
#define ENC28J60_RX_BUFFER_STOP  0x17FF
#define ENC28J60_TX_BUFFER_START 0x1800
#define ENC28J60_TX_BUFFER_STOP  0x1FFF

//SPI command set
#define ENC28J60_CMD_RCR 0x00
#define ENC28J60_CMD_RBM 0x3A
#define ENC28J60_CMD_WCR 0x40
#define ENC28J60_CMD_WBM 0x7A
#define ENC28J60_CMD_BFS 0x80
#define ENC28J60_CMD_BFC 0xA0
#define ENC28J60_CMD_SRC 0xFF

//ENC28J60 register types
#define ETH_REG_TYPE 0x0000
#define MAC_REG_TYPE 0x1000
#define MII_REG_TYPE 0x2000
#define PHY_REG_TYPE 0x3000

//ENC28J60 banks
#define BANK_0 0x0000
#define BANK_1 0x0100
#define BANK_2 0x0200
#define BANK_3 0x0300

//Related masks
#define REG_TYPE_MASK 0xF000
#define REG_BANK_MASK 0x0F00
#define REG_ADDR_MASK 0x001F

//Bank 0 registers
#define ENC28J60_REG_ERDPTL      (ETH_REG_TYPE | BANK_0 | 0x00)
#define ENC28J60_REG_ERDPTH      (ETH_REG_TYPE | BANK_0 | 0x01)
#define ENC28J60_REG_EWRPTL      (ETH_REG_TYPE | BANK_0 | 0x02)
#define ENC28J60_REG_EWRPTH      (ETH_REG_TYPE | BANK_0 | 0x03)
#define ENC28J60_REG_ETXSTL      (ETH_REG_TYPE | BANK_0 | 0x04)
#define ENC28J60_REG_ETXSTH      (ETH_REG_TYPE | BANK_0 | 0x05)
#define ENC28J60_REG_ETXNDL      (ETH_REG_TYPE | BANK_0 | 0x06)
#define ENC28J60_REG_ETXNDH      (ETH_REG_TYPE | BANK_0 | 0x07)
#define ENC28J60_REG_ERXSTL      (ETH_REG_TYPE | BANK_0 | 0x08)
#define ENC28J60_REG_ERXSTH      (ETH_REG_TYPE | BANK_0 | 0x09)
#define ENC28J60_REG_ERXNDL      (ETH_REG_TYPE | BANK_0 | 0x0A)
#define ENC28J60_REG_ERXNDH      (ETH_REG_TYPE | BANK_0 | 0x0B)
#define ENC28J60_REG_ERXRDPTL    (ETH_REG_TYPE | BANK_0 | 0x0C)
#define ENC28J60_REG_ERXRDPTH    (ETH_REG_TYPE | BANK_0 | 0x0D)
#define ENC28J60_REG_ERXWRPTL    (ETH_REG_TYPE | BANK_0 | 0x0E)
#define ENC28J60_REG_ERXWRPTH    (ETH_REG_TYPE | BANK_0 | 0x0F)
#define ENC28J60_REG_EDMASTL     (ETH_REG_TYPE | BANK_0 | 0x10)
#define ENC28J60_REG_EDMASTH     (ETH_REG_TYPE | BANK_0 | 0x11)
#define ENC28J60_REG_EDMANDL     (ETH_REG_TYPE | BANK_0 | 0x12)
#define ENC28J60_REG_EDMANDH     (ETH_REG_TYPE | BANK_0 | 0x13)
#define ENC28J60_REG_EDMADSTL    (ETH_REG_TYPE | BANK_0 | 0x14)
#define ENC28J60_REG_EDMADSTH    (ETH_REG_TYPE | BANK_0 | 0x15)
#define ENC28J60_REG_EDMACSL     (ETH_REG_TYPE | BANK_0 | 0x16)
#define ENC28J60_REG_EDMACSH     (ETH_REG_TYPE | BANK_0 | 0x17)
#define ENC28J60_REG_EIE         (ETH_REG_TYPE | BANK_0 | 0x1B)
#define ENC28J60_REG_EIR         (ETH_REG_TYPE | BANK_0 | 0x1C)
#define ENC28J60_REG_ESTAT       (ETH_REG_TYPE | BANK_0 | 0x1D)
#define ENC28J60_REG_ECON2       (ETH_REG_TYPE | BANK_0 | 0x1E)
#define ENC28J60_REG_ECON1       (ETH_REG_TYPE | BANK_0 | 0x1F)

//Bank 1 registers
#define ENC28J60_REG_EHT0        (ETH_REG_TYPE | BANK_1 | 0x00)
#define ENC28J60_REG_EHT1        (ETH_REG_TYPE | BANK_1 | 0x01)
#define ENC28J60_REG_EHT2        (ETH_REG_TYPE | BANK_1 | 0x02)
#define ENC28J60_REG_EHT3        (ETH_REG_TYPE | BANK_1 | 0x03)
#define ENC28J60_REG_EHT4        (ETH_REG_TYPE | BANK_1 | 0x04)
#define ENC28J60_REG_EHT5        (ETH_REG_TYPE | BANK_1 | 0x05)
#define ENC28J60_REG_EHT6        (ETH_REG_TYPE | BANK_1 | 0x06)
#define ENC28J60_REG_EHT7        (ETH_REG_TYPE | BANK_1 | 0x07)
#define ENC28J60_REG_EPMM0       (ETH_REG_TYPE | BANK_1 | 0x08)
#define ENC28J60_REG_EPMM1       (ETH_REG_TYPE | BANK_1 | 0x09)
#define ENC28J60_REG_EPMM2       (ETH_REG_TYPE | BANK_1 | 0x0A)
#define ENC28J60_REG_EPMM3       (ETH_REG_TYPE | BANK_1 | 0x0B)
#define ENC28J60_REG_EPMM4       (ETH_REG_TYPE | BANK_1 | 0x0C)
#define ENC28J60_REG_EPMM5       (ETH_REG_TYPE | BANK_1 | 0x0D)
#define ENC28J60_REG_EPMM6       (ETH_REG_TYPE | BANK_1 | 0x0E)
#define ENC28J60_REG_EPMM7       (ETH_REG_TYPE | BANK_1 | 0x0F)
#define ENC28J60_REG_EPMCSL      (ETH_REG_TYPE | BANK_1 | 0x10)
#define ENC28J60_REG_EPMCSH      (ETH_REG_TYPE | BANK_1 | 0x11)
#define ENC28J60_REG_EPMOL       (ETH_REG_TYPE | BANK_1 | 0x14)
#define ENC28J60_REG_EPMOH       (ETH_REG_TYPE | BANK_1 | 0x15)
#define ENC28J60_REG_EWOLIE      (ETH_REG_TYPE | BANK_1 | 0x16)
#define ENC28J60_REG_EWOLIR      (ETH_REG_TYPE | BANK_1 | 0x17)
#define ENC28J60_REG_ERXFCON     (ETH_REG_TYPE | BANK_1 | 0x18)
#define ENC28J60_REG_EPKTCNT     (ETH_REG_TYPE | BANK_1 | 0x19)

//Bank 2 registers
#define ENC28J60_REG_MACON1      (MAC_REG_TYPE | BANK_2 | 0x00)
#define ENC28J60_REG_MACON2      (MAC_REG_TYPE | BANK_2 | 0x01)
#define ENC28J60_REG_MACON3      (MAC_REG_TYPE | BANK_2 | 0x02)
#define ENC28J60_REG_MACON4      (MAC_REG_TYPE | BANK_2 | 0x03)
#define ENC28J60_REG_MABBIPG     (MAC_REG_TYPE | BANK_2 | 0x04)
#define ENC28J60_REG_MAIPGL      (MAC_REG_TYPE | BANK_2 | 0x06)
#define ENC28J60_REG_MAIPGH      (MAC_REG_TYPE | BANK_2 | 0x07)
#define ENC28J60_REG_MACLCON1    (MAC_REG_TYPE | BANK_2 | 0x08)
#define ENC28J60_REG_MACLCON2    (MAC_REG_TYPE | BANK_2 | 0x09)
#define ENC28J60_REG_MAMXFLL     (MAC_REG_TYPE | BANK_2 | 0x0A)
#define ENC28J60_REG_MAMXFLH     (MAC_REG_TYPE | BANK_2 | 0x0B)
#define ENC28J60_REG_MAPHSUP     (MAC_REG_TYPE | BANK_2 | 0x0D)
#define ENC28J60_REG_MICON       (MII_REG_TYPE | BANK_2 | 0x11)
#define ENC28J60_REG_MICMD       (MII_REG_TYPE | BANK_2 | 0x12)
#define ENC28J60_REG_MIREGADR    (MII_REG_TYPE | BANK_2 | 0x14)
#define ENC28J60_REG_MIWRL       (MII_REG_TYPE | BANK_2 | 0x16)
#define ENC28J60_REG_MIWRH       (MII_REG_TYPE | BANK_2 | 0x17)
#define ENC28J60_REG_MIRDL       (MII_REG_TYPE | BANK_2 | 0x18)
#define ENC28J60_REG_MIRDH       (MII_REG_TYPE | BANK_2 | 0x19)

//Bank 3 registers
#define ENC28J60_REG_MAADR5      (MAC_REG_TYPE | BANK_3 | 0x00)
#define ENC28J60_REG_MAADR6      (MAC_REG_TYPE | BANK_3 | 0x01)
#define ENC28J60_REG_MAADR3      (MAC_REG_TYPE | BANK_3 | 0x02)
#define ENC28J60_REG_MAADR4      (MAC_REG_TYPE | BANK_3 | 0x03)
#define ENC28J60_REG_MAADR1      (MAC_REG_TYPE | BANK_3 | 0x04)
#define ENC28J60_REG_MAADR2      (MAC_REG_TYPE | BANK_3 | 0x05)
#define ENC28J60_REG_EBSTSD      (ETH_REG_TYPE | BANK_3 | 0x06)
#define ENC28J60_REG_EBSTCON     (ETH_REG_TYPE | BANK_3 | 0x07)
#define ENC28J60_REG_EBSTCSL     (ETH_REG_TYPE | BANK_3 | 0x08)
#define ENC28J60_REG_EBSTCSH     (ETH_REG_TYPE | BANK_3 | 0x09)
#define ENC28J60_REG_MISTAT      (MII_REG_TYPE | BANK_3 | 0x0A)
#define ENC28J60_REG_EREVID      (ETH_REG_TYPE | BANK_3 | 0x12)
#define ENC28J60_REG_ECOCON      (ETH_REG_TYPE | BANK_3 | 0x15)
#define ENC28J60_REG_EFLOCON     (ETH_REG_TYPE | BANK_3 | 0x17)
#define ENC28J60_REG_EPAUSL      (ETH_REG_TYPE | BANK_3 | 0x18)
#define ENC28J60_REG_EPAUSH      (ETH_REG_TYPE | BANK_3 | 0x19)

//PHY registers
#define ENC28J60_PHY_REG_PHCON1  (PHY_REG_TYPE | 0x00)
#define ENC28J60_PHY_REG_PHSTAT1 (PHY_REG_TYPE | 0x01)
#define ENC28J60_PHY_REG_PHID1   (PHY_REG_TYPE | 0x02)
#define ENC28J60_PHY_REG_PHID2   (PHY_REG_TYPE | 0x03)
#define ENC28J60_PHY_REG_PHCON2  (PHY_REG_TYPE | 0x10)
#define ENC28J60_PHY_REG_PHSTAT2 (PHY_REG_TYPE | 0x11)
#define ENC28J60_PHY_REG_PHIE    (PHY_REG_TYPE | 0x12)
#define ENC28J60_PHY_REG_PHIR    (PHY_REG_TYPE | 0x13)
#define ENC28J60_PHY_REG_PHLCON  (PHY_REG_TYPE | 0x14)

//EIE register
#define EIE_INTIE         (1 << 7)
#define EIE_PKTIE         (1 << 6)
#define EIE_DMAIE         (1 << 5)
#define EIE_LINKIE        (1 << 4)
#define EIE_TXIE          (1 << 3)
#define EIE_WOLIE         (1 << 2)
#define EIE_TXERIE        (1 << 1)
#define EIE_RXERIE        (1 << 0)

//EIR register
#define EIR_PKTIF         (1 << 6)
#define EIR_DMAIF         (1 << 5)
#define EIR_LINKIF        (1 << 4)
#define EIR_TXIF          (1 << 3)
#define EIR_WOLIF         (1 << 2)
#define EIR_TXERIF        (1 << 1)
#define EIR_RXERIF        (1 << 0)

//ESTAT register
#define ESTAT_INT         (1 << 7)
#define ESTAT_LATECOL     (1 << 4)
#define ESTAT_RXBUSY      (1 << 2)
#define ESTAT_TXABRT      (1 << 1)
#define ESTAT_CLKRDY      (1 << 0)

//ECON2 register
#define ECON2_AUTOINC     (1 << 7)
#define ECON2_PKTDEC      (1 << 6)
#define ECON2_PWRSV       (1 << 5)
#define ECON2_VRPS        (1 << 3)

//ECON1 register
#define ECON1_TXRST       (1 << 7)
#define ECON1_RXRST       (1 << 6)
#define ECON1_DMAST       (1 << 5)
#define ECON1_CSUMEN      (1 << 4)
#define ECON1_TXRTS       (1 << 3)
#define ECON1_RXEN        (1 << 2)
#define ECON1_BSEL1       (1 << 1)
#define ECON1_BSEL0       (1 << 0)

//ERXFCON register
#define ERXFCON_UCEN      (1 << 7)
#define ERXFCON_ANDOR     (1 << 6)
#define ERXFCON_CRCEN     (1 << 5)
#define ERXFCON_PMEN      (1 << 4)
#define ERXFCON_MPEN      (1 << 3)
#define ERXFCON_HTEN      (1 << 2)
#define ERXFCON_MCEN      (1 << 1)
#define ERXFCON_BCEN      (1 << 0)

//MACON1 register
#define MACON1_LOOPBK     (1 << 4)
#define MACON1_TXPAUS     (1 << 3)
#define MACON1_RXPAUS     (1 << 2)
#define MACON1_PASSALL    (1 << 1)
#define MACON1_MARXEN     (1 << 0)

//MACON2 register
#define MACON2_MARST      (1 << 7)
#define MACON2_RNDRST     (1 << 6)
#define MACON2_MARXRST    (1 << 3)
#define MACON2_RFUNRST    (1 << 2)
#define MACON2_MATXRST    (1 << 1)
#define MACON2_TFUNRST    (1 << 0)

//MACON3 register
#define MACON3_PADCFG2    (1 << 7)
#define MACON3_PADCFG1    (1 << 6)
#define MACON3_PADCFG0    (1 << 5)
#define MACON3_TXCRCEN    (1 << 4)
#define MACON3_PHDRLEN    (1 << 3)
#define MACON3_HFRMLEN    (1 << 2)
#define MACON3_FRMLNEN    (1 << 1)
#define MACON3_FULDPX     (1 << 0)

#define MACON3_PADCFG(x) ((x) << 5)

//MACON4 register
#define MACON4_DEFER      (1 << 6)
#define MACON4_BPEN       (1 << 5)
#define MACON4_NOBKOFF    (1 << 4)
#define MACON4_LONGPRE    (1 << 1)
#define MACON4_PUREPRE    (1 << 0)

//MAPHSUP register
#define MAPHSUP_RSTINTFC  (1 << 7)
#define MAPHSUP_RSTRMII   (1 << 3)

//MICON register
#define MICON_RSTMII      (1 << 7)

//MICMD register
#define MICMD_MIISCAN     (1 << 1)
#define MICMD_MIIRD       (1 << 0)

//EBSTCON register
#define EBSTCON_PSV2      (1 << 7)
#define EBSTCON_PSV1      (1 << 6)
#define EBSTCON_PSV0      (1 << 5)
#define EBSTCON_PSEL      (1 << 4)
#define EBSTCON_TMSEL1    (1 << 3)
#define EBSTCON_TMSEL0    (1 << 2)
#define EBSTCON_TME       (1 << 1)
#define EBSTCON_BISTST    (1 << 0)

//MISTAT register
#define MISTAT_NVALID     (1 << 2)
#define MISTAT_SCAN       (1 << 1)
#define MISTAT_BUSY       (1 << 0)

//ECOCON register
#define ECOCON_COCON2     (1 << 2)
#define ECOCON_COCON1     (1 << 1)
#define ECOCON_COCON0     (1 << 0)

//EFLOCON register
#define EFLOCON_FULDPXS   (1 << 2)
#define EFLOCON_FCEN1     (1 << 1)
#define EFLOCON_FCEN0     (1 << 0)

//PHCON1 register
#define PHCON1_PRST       (1 << 15)
#define PHCON1_PLOOPBK    (1 << 14)
#define PHCON1_PPWRSV     (1 << 11)
#define PHCON1_PDPXMD     (1 << 8)

//PHSTAT1 register
#define PHSTAT1_PFDPX     (1 << 12)
#define PHSTAT1_PHDPX     (1 << 11)
#define PHSTAT1_LLSTAT    (1 << 2)
#define PHSTAT1_JBSTAT    (1 << 1)

//PHCON2 register
#define PHCON2_FRCLINK    (1 << 14)
#define PHCON2_TXDIS      (1 << 13)
#define PHCON2_JABBER     (1 << 10)
#define PHCON2_HDLDIS     (1 << 8)

//PHSTAT2 register
#define PHSTAT2_TXSTAT    (1 << 13)
#define PHSTAT2_RXSTAT    (1 << 12)
#define PHSTAT2_COLSTAT   (1 << 11)
#define PHSTAT2_LSTAT     (1 << 10)
#define PHSTAT2_DPXSTAT   (1 << 9)
#define PHSTAT2_PLRITY    (1 << 4)

//PHIE register
#define PHIE_PLNKIE       (1 << 4)
#define PHIE_PGEIE        (1 << 1)

//PHIR register
#define PHIR_PLNKIF       (1 << 4)
#define PHIR_PGIF         (1 << 2)

//PHLCON register
#define PHLCON_LACFG3     (1 << 11)
#define PHLCON_LACFG2     (1 << 10)
#define PHLCON_LACFG1     (1 << 9)
#define PHLCON_LACFG0     (1 << 8)
#define PHLCON_LBCFG3     (1 << 7)
#define PHLCON_LBCFG2     (1 << 6)
#define PHLCON_LBCFG1     (1 << 5)
#define PHLCON_LBCFG0     (1 << 4)
#define PHLCON_LFRQ1      (1 << 3)
#define PHLCON_LFRQ0      (1 << 2)
#define PHLCON_STRCH      (1 << 1)

#define PHLCON_LACFG(x)  ((x) << 8)
#define PHLCON_LBCFG(x)  ((x) << 4)
#define PHLCON_LFRQ(x)   ((x) << 2)

//Per-packet control byte
#define TX_CTRL_PHUGEEN   (1 << 3)
#define TX_CTRL_PPADEN    (1 << 2)
#define TX_CTRL_PCRCEN    (1 << 1)
#define TX_CTRL_POVERRIDE (1 << 0)

//Receive status vector
#define RSV_VLAN_TYPE           0x4000
#define RSV_UNKNOWN_OPCODE      0x2000
#define RSV_PAUSE_CONTROL_FRAME 0x1000
#define RSV_CONTROL_FRAME       0x0800
#define RSV_DRIBBLE_NIBBLE      0x0400
#define RSV_BROADCAST_PACKET    0x0200
#define RSV_MULTICAST_PACKET    0x0100
#define RSV_RECEIVED_OK         0x0080
#define RSV_LENGTH_OUT_OF_RANGE 0x0040
#define RSV_LENGTH_CHECK_ERROR  0x0020
#define RSV_CRC_ERROR           0x0010
#define RSV_CARRIER_EVENT       0x0004
#define RSV_DROP_EVENT          0x0001

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief ENC28J60 driver context
 **/

typedef struct
{
   uint16_t currentBank; ///<Current bank
   uint16_t nextPacket;  ///<Next packet in the receive buffer
   uint8_t *rxBuffer;    ///<Receive buffer
} Enc28j60Context;


//ENC28J60 driver
extern const NicDriver enc28j60Driver;

//ENC28J60 related functions
error_t enc28j60Init(NetInterface *interface);

void enc28j60Tick(NetInterface *interface);

void enc28j60EnableIrq(NetInterface *interface);
void enc28j60DisableIrq(NetInterface *interface);
bool_t enc28j60IrqHandler(NetInterface *interface);
void enc28j60EventHandler(NetInterface *interface);

error_t enc28j60SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset);

error_t enc28j60ReceivePacket(NetInterface *interface);

error_t enc28j60SetMulticastFilter(NetInterface *interface);

void enc28j60SoftReset(NetInterface *interface);
void enc28j60SelectBank(NetInterface *interface, uint16_t address);

void enc28j60WriteReg(NetInterface *interface, uint16_t address, uint8_t data);
uint8_t enc28j60ReadReg(NetInterface *interface, uint16_t address);

void enc28j60WritePhyReg(NetInterface *interface, uint16_t address, uint16_t data);
uint16_t enc28j60ReadPhyReg(NetInterface *interface, uint16_t address);

void enc28j60WriteBuffer(NetInterface *interface,
   const NetBuffer *buffer, size_t offset);

void enc28j60ReadBuffer(NetInterface *interface,
   uint8_t *data, size_t length);

void enc28j60SetBit(NetInterface *interface, uint16_t address, uint16_t mask);
void enc28j60ClearBit(NetInterface *interface, uint16_t address, uint16_t mask);

uint32_t enc28j60CalcCrc(const void *data, size_t length);

void enc28j60DumpReg(NetInterface *interface);
void enc28j60DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
