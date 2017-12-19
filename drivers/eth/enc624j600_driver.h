/**
 * @file enc624j600_driver.h
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

#ifndef _ENC624J600_DRIVER_H
#define _ENC624J600_DRIVER_H

//Receive and transmit buffers
#define ENC624J600_TX_BUFFER_START 0x0000
#define ENC624J600_TX_BUFFER_STOP  0x17FE
#define ENC624J600_RX_BUFFER_START 0x1800
#define ENC624J600_RX_BUFFER_STOP  0x5FFE

//SPI command set
#define ENC624J600_CMD_B0SEL       0xC0 //Bank 0 Select
#define ENC624J600_CMD_B1SEL       0xC2 //Bank 1 Select
#define ENC624J600_CMD_B2SEL       0xC4 //Bank 2 Select
#define ENC624J600_CMD_B3SEL       0xC6 //Bank 3 Select
#define ENC624J600_CMD_SETETHRST   0xCA //System Reset
#define ENC624J600_CMD_FCDISABLE   0xE0 //Flow Control Disable
#define ENC624J600_CMD_FCSINGLE    0xE2 //Flow Control Single
#define ENC624J600_CMD_FCMULTIPLE  0xE4 //Flow Control Multiple
#define ENC624J600_CMD_FCCLEAR     0xE6 //Flow Control Clear
#define ENC624J600_CMD_SETPKTDEC   0xCC //Decrement Packet Counter
#define ENC624J600_CMD_DMASTOP     0xD2 //DMA Stop
#define ENC624J600_CMD_DMACKSUM    0xD8 //DMA Start Checksum
#define ENC624J600_CMD_DMACKSUMS   0xDA //DMA Start Checksum with Seed
#define ENC624J600_CMD_DMACOPY     0xDC //DMA Start Copy
#define ENC624J600_CMD_DMACOPYS    0xDE //DMA Start Copy and Checksum with Seed
#define ENC624J600_CMD_SETTXRTS    0xD4 //Request Packet Transmission
#define ENC624J600_CMD_ENABLERX    0xE8 //Enable RX
#define ENC624J600_CMD_DISABLERX   0xEA //Disable RX
#define ENC624J600_CMD_SETEIE      0xEC //Enable Interrupts
#define ENC624J600_CMD_CLREIE      0xEE //Disable Interrupts
#define ENC624J600_CMD_RBSEL       0xC8 //Read Bank Select
#define ENC624J600_CMD_WGPRDPT     0x60 //Write EGPRDPT
#define ENC624J600_CMD_RGPRDPT     0x62 //Read EGPRDPT
#define ENC624J600_CMD_WRXRDPT     0x64 //Write ERXRDPT
#define ENC624J600_CMD_RRXRDPT     0x66 //Read ERXRDPT
#define ENC624J600_CMD_WUDARDPT    0x68 //Write EUDARDPT
#define ENC624J600_CMD_RUDARDPT    0x6A //Read EUDARDPT
#define ENC624J600_CMD_WGPWRPT     0x6C //Write EGPWRPT
#define ENC624J600_CMD_RGPWRPT     0x6E //Read EGPWRPT
#define ENC624J600_CMD_WRXWRPT     0x70 //Write ERXWRPT
#define ENC624J600_CMD_RRXWRPT     0x72 //Read ERXWRPT
#define ENC624J600_CMD_WUDAWRPT    0x74 //Write EUDAWRPT
#define ENC624J600_CMD_RUDAWRPT    0x76 //Read EUDAWRPT
#define ENC624J600_CMD_RCR         0x00 //Read Control Register
#define ENC624J600_CMD_WCR         0x40 //Write Control Register
#define ENC624J600_CMD_RCRU        0x20 //Read Control Register Unbanked
#define ENC624J600_CMD_WCRU        0x22 //Write Control Register Unbanked
#define ENC624J600_CMD_BFS         0x80 //Bit Field Set
#define ENC624J600_CMD_BFC         0xA0 //Bit Field Clear
#define ENC624J600_CMD_BFSU        0x24 //Bit Field Set Unbanked
#define ENC624J600_CMD_BFCU        0x26 //Bit Field Clear Unbanked
#define ENC624J600_CMD_RGPDATA     0x28 //Read EGPDATA
#define ENC624J600_CMD_WGPDATA     0x2A //Write EGPDATA
#define ENC624J600_CMD_RRXDATA     0x2C //Read ERXDATA
#define ENC624J600_CMD_WRXDATA     0x2E //Write ERXDATA
#define ENC624J600_CMD_RUDADATA    0x30 //Read EUDADATA
#define ENC624J600_CMD_WUDADATA    0x32 //Write EUDADATA

//ENC624J600 registers
#define ENC624J600_REG_ETXST       0x00
#define ENC624J600_REG_ETXLEN      0x02
#define ENC624J600_REG_ERXST       0x04
#define ENC624J600_REG_ERXTAIL     0x06
#define ENC624J600_REG_ERXHEAD     0x08
#define ENC624J600_REG_EDMAST      0x0A
#define ENC624J600_REG_EDMALEN     0x0C
#define ENC624J600_REG_EDMADST     0x0E
#define ENC624J600_REG_EDMACS      0x10
#define ENC624J600_REG_ETXSTAT     0x12
#define ENC624J600_REG_ETXWIRE     0x14
#define ENC624J600_REG_EUDAST      0x16
#define ENC624J600_REG_EUDAND      0x18
#define ENC624J600_REG_ESTAT       0x1A
#define ENC624J600_REG_EIR         0x1C
#define ENC624J600_REG_ECON1       0x1E
#define ENC624J600_REG_EHT1        0x20
#define ENC624J600_REG_EHT2        0x22
#define ENC624J600_REG_EHT3        0x24
#define ENC624J600_REG_EHT4        0x26
#define ENC624J600_REG_EPMM1       0x28
#define ENC624J600_REG_EPMM2       0x2A
#define ENC624J600_REG_EPMM3       0x2C
#define ENC624J600_REG_EPMM4       0x2E
#define ENC624J600_REG_EPMCS       0x30
#define ENC624J600_REG_EPMO        0x32
#define ENC624J600_REG_ERXFCON     0x34
#define ENC624J600_REG_MACON1      0x40
#define ENC624J600_REG_MACON2      0x42
#define ENC624J600_REG_MABBIPG     0x44
#define ENC624J600_REG_MAIPG       0x46
#define ENC624J600_REG_MACLCON     0x48
#define ENC624J600_REG_MAMXFL      0x4A
#define ENC624J600_REG_MICMD       0x52
#define ENC624J600_REG_MIREGADR    0x54
#define ENC624J600_REG_MAADR3      0x60
#define ENC624J600_REG_MAADR2      0x62
#define ENC624J600_REG_MAADR1      0x64
#define ENC624J600_REG_MIWR        0x66
#define ENC624J600_REG_MIRD        0x68
#define ENC624J600_REG_MISTAT      0x6A
#define ENC624J600_REG_EPAUS       0x6C
#define ENC624J600_REG_ECON2       0x6E
#define ENC624J600_REG_ERXWM       0x70
#define ENC624J600_REG_EIE         0x72
#define ENC624J600_REG_EIDLED      0x74
#define ENC624J600_REG_EGPDATA     0x80
#define ENC624J600_REG_ERXDATA     0x82
#define ENC624J600_REG_EUDADATA    0x84
#define ENC624J600_REG_EGPRDPT     0x86
#define ENC624J600_REG_EGPWRPT     0x88
#define ENC624J600_REG_ERXRDPT     0x8A
#define ENC624J600_REG_ERXWRPT     0x8C
#define ENC624J600_REG_EUDARDPT    0x8E
#define ENC624J600_REG_EUDAWRPT    0x90

//ENC624J600 PHY registers
#define ENC624J600_PHY_REG_PHCON1  0x00
#define ENC624J600_PHY_REG_PHSTAT1 0x01
#define ENC624J600_PHY_REG_PHANA   0x04
#define ENC624J600_PHY_REG_PHANLPA 0x05
#define ENC624J600_PHY_REG_PHANE   0x06
#define ENC624J600_PHY_REG_PHCON2  0x11
#define ENC624J600_PHY_REG_PHSTAT2 0x1B
#define ENC624J600_PHY_REG_PHSTAT3 0x1F

//ESTAT register
#define ESTAT_INT                  0x8000
#define ESTAT_FCIDLE               0x4000
#define ESTAT_RXBUSY               0x2000
#define ESTAT_CLKRDY               0x1000
#define ESTAT_R11                  0x0800
#define ESTAT_PHYDPX               0x0400
#define ESTAT_R9                   0x0200
#define ESTAT_PHYLNK               0x0100
#define ESTAT_PKTCNT               0x00FF

//EIR register
#define EIR_CRYPTEN                0x8000
#define EIR_MODEXIF                0x4000
#define EIR_HASHIF                 0x2000
#define EIR_AESIF                  0x1000
#define EIR_LINKIF                 0x0800
#define EIR_R10                    0x0400
#define EIR_R9                     0x0200
#define EIR_R8                     0x0100
#define EIR_R7                     0x0080
#define EIR_PKTIF                  0x0040
#define EIR_DMAIF                  0x0020
#define EIR_R4                     0x0010
#define EIR_TXIF                   0x0008
#define EIR_TXABTIF                0x0004
#define EIR_RXABTIF                0x0002
#define EIR_PCFULIF                0x0001

//ECON1 register
#define ECON1_MODEXST              0x8000
#define ECON1_HASHEN               0x4000
#define ECON1_HASHOP               0x2000
#define ECON1_HASHLST              0x1000
#define ECON1_AESST                0x0800
#define ECON1_AESOP1               0x0400
#define ECON1_AESOP0               0x0200
#define ECON1_PKTDEC               0x0100
#define ECON1_FCOP1                0x0080
#define ECON1_FCOP0                0x0040
#define ECON1_DMAST                0x0020
#define ECON1_DMACPY               0x0010
#define ECON1_DMACSSD              0x0008
#define ECON1_DMANOCS              0x0004
#define ECON1_TXRTS                0x0002
#define ECON1_RXEN                 0x0001

//ETXSTAT register
#define ETXSTAT_R12                0x1000
#define ETXSTAT_R11                0x0800
#define ETXSTAT_LATECOL            0x0400
#define ETXSTAT_MAXCOL             0x0200
#define ETXSTAT_EXDEFER            0x0100
#define ETXSTAT_DEFER              0x0080
#define ETXSTAT_R6                 0x0040
#define ETXSTAT_R5                 0x0020
#define ETXSTAT_CRCBAD             0x0010
#define ETXSTAT_COLCNT             0x000F

//ERXFCON register
#define ERXFCON_HTEN               0x8000
#define ERXFCON_MPEN               0x4000
#define ERXFCON_NOTPM              0x1000
#define ERXFCON_PMEN3              0x0800
#define ERXFCON_PMEN2              0x0400
#define ERXFCON_PMEN1              0x0200
#define ERXFCON_PMEN0              0x0100
#define ERXFCON_CRCEEN             0x0080
#define ERXFCON_CRCEN              0x0040
#define ERXFCON_RUNTEEN            0x0020
#define ERXFCON_RUNTEN             0x0010
#define ERXFCON_UCEN               0x0008
#define ERXFCON_NOTMEEN            0x0004
#define ERXFCON_MCEN               0x0002
#define ERXFCON_BCEN               0x0001

//MACON1 register
#define MACON1_R15                 0x8000
#define MACON1_R14                 0x4000
#define MACON1_R11                 0x0800
#define MACON1_R10                 0x0400
#define MACON1_R9                  0x0200
#define MACON1_R8                  0x0100
#define MACON1_LOOPBK              0x0010
#define MACON1_R3                  0x0008
#define MACON1_RXPAUS              0x0004
#define MACON1_PASSALL             0x0002
#define MACON1_R0                  0x0001

//MACON2 register
#define MACON2_DEFER               0x4000
#define MACON2_BPEN                0x2000
#define MACON2_NOBKOFF             0x1000
#define MACON2_R9                  0x0200
#define MACON2_R8                  0x0100
#define MACON2_PADCFG2             0x0080
#define MACON2_PADCFG1             0x0040
#define MACON2_PADCFG0             0x0020
#define MACON2_TXCRCEN             0x0010
#define MACON2_PHDREN              0x0008
#define MACON2_HFRMEN              0x0004
#define MACON2_R1                  0x0002
#define MACON2_FULDPX              0x0001

//MABBIPG register
#define MABBIPG_BBIPG              0x007F

//MAIPG register
#define MAIPG_R14                  0x4000
#define MAIPG_R13                  0x2000
#define MAIPG_R12                  0x1000
#define MAIPG_R11                  0x0800
#define MAIPG_R10                  0x0400
#define MAIPG_R9                   0x0200
#define MAIPG_R8                   0x0100
#define MAIPG_IPG                  0x007F

//MACLCON register
#define MACLCON_R13                0x2000
#define MACLCON_R12                0x1000
#define MACLCON_R11                0x0800
#define MACLCON_R10                0x0400
#define MACLCON_R9                 0x0200
#define MACLCON_R8                 0x0100
#define MACLCON_MAXRET             0x000F

//MICMD register
#define MICMD_MIISCAN              0x0002
#define MICMD_MIIRD                0x0001

//MIREGADR register
#define MIREGADR_R12               0x1000
#define MIREGADR_R11               0x0800
#define MIREGADR_R10               0x0400
#define MIREGADR_R9                0x0200
#define MIREGADR_R8                0x0100
#define MIREGADR_PHREG             0x001F

//MISTAT register
#define MISTAT_R3                  0x0008
#define MISTAT_NVALID              0x0004
#define MISTAT_SCAN                0x0002
#define MISTAT_BUSY                0x0001

//ECON2 register
#define ECON2_ETHEN                0x8000
#define ECON2_STRCH                0x4000
#define ECON2_TXMAC                0x2000
#define ECON2_SHA1MD5              0x1000
#define ECON2_COCON3               0x0800
#define ECON2_COCON2               0x0400
#define ECON2_COCON1               0x0200
#define ECON2_COCON0               0x0100
#define ECON2_AUTOFC               0x0080
#define ECON2_TXRST                0x0040
#define ECON2_RXRST                0x0020
#define ECON2_ETHRST               0x0010
#define ECON2_MODLEN1              0x0008
#define ECON2_MODLEN0              0x0004
#define ECON2_AESLEN1              0x0002
#define ECON2_AESLEN0              0x0001

//ERXWM register
#define ERXWM_RXFWM                0xFF00
#define ERXWM_RXEWM                0x00FF

//EIE register
#define EIE_INTIE                  0x8000
#define EIE_MODEXIE                0x4000
#define EIE_HASHIE                 0x2000
#define EIE_AESIE                  0x1000
#define EIE_LINKIE                 0x0800
#define EIE_R10                    0x0400
#define EIE_R9                     0x0200
#define EIE_R8                     0x0100
#define EIE_R7                     0x0080
#define EIE_PKTIE                  0x0040
#define EIE_DMAIE                  0x0020
#define EIE_R4                     0x0010
#define EIE_TXIE                   0x0008
#define EIE_TXABTIE                0x0004
#define EIE_RXABTIE                0x0002
#define EIE_PCFULIE                0x0001

//EIDLED register
#define EIDLED_LACFG3              0x8000
#define EIDLED_LACFG2              0x4000
#define EIDLED_LACFG1              0x2000
#define EIDLED_LACFG0              0x1000
#define EIDLED_LBCFG3              0x0800
#define EIDLED_LBCFG2              0x0400
#define EIDLED_LBCFG1              0x0200
#define EIDLED_LBCFG0              0x0100
#define EIDLED_DEVID               0x00FF

//PHCON1 register
#define PHCON1_PRST                0x8000
#define PHCON1_PLOOPBK             0x4000
#define PHCON1_SPD100              0x2000
#define PHCON1_ANEN                0x1000
#define PHCON1_PSLEEP              0x0800
#define PHCON1_RENEG               0x0200
#define PHCON1_PFULDPX             0x0100

//PHSTAT1 register
#define PHSTAT1_FULL100            0x4000
#define PHSTAT1_HALF100            0x2000
#define PHSTAT1_FULL10             0x1000
#define PHSTAT1_HALF10             0x0800
#define PHSTAT1_ANDONE             0x0020
#define PHSTAT1_LRFAULT            0x0010
#define PHSTAT1_ANABLE             0x0008
#define PHSTAT1_LLSTAT             0x0004
#define PHSTAT1_EXTREGS            0x0001

//PHANA register
#define PHANA_ADNP                 0x8000
#define PHANA_ADFAULT              0x2000
#define PHANA_ADPAUS1              0x0800
#define PHANA_ADPAUS0              0x0400
#define PHANA_AD100FD              0x0100
#define PHANA_AD100                0x0080
#define PHANA_AD10FD               0x0040
#define PHANA_AD10                 0x0020
#define PHANA_ADIEEE4              0x0010
#define PHANA_ADIEEE3              0x0008
#define PHANA_ADIEEE2              0x0004
#define PHANA_ADIEEE1              0x0002
#define PHANA_ADIEEE0              0x0001

//PHANLPA register
#define PHANLPA_LPNP               0x8000
#define PHANLPA_LPACK              0x4000
#define PHANLPA_LPFAULT            0x2000
#define PHANLPA_LPPAUS1            0x0800
#define PHANLPA_LPPAUS0            0x0400
#define PHANLPA_LP100T4            0x0200
#define PHANLPA_LP100FD            0x0100
#define PHANLPA_LP100              0x0080
#define PHANLPA_LP10FD             0x0040
#define PHANLPA_LP10               0x0020
#define PHANLPA_LPIEEE             0x001F
#define PHANLPA_LPIEEE4            0x0010
#define PHANLPA_LPIEEE3            0x0008
#define PHANLPA_LPIEEE2            0x0004
#define PHANLPA_LPIEEE1            0x0002
#define PHANLPA_LPIEEE0            0x0001

//PHANE register
#define PHANE_PDFLT                0x0010
#define PHANE_LPARCD               0x0002
#define PHANE_LPANABL              0x0001

//PHCON2 register
#define PHCON2_EDPWRDN             0x2000
#define PHCON2_EDTHRES             0x0800
#define PHCON2_FRCLNK              0x0004
#define PHCON2_EDSTAT              0x0002

//PHSTAT2 register
#define PHSTAT2_PLRITY             0x0010

//PHSTAT3 register
#define PHSTAT3_SPDDPX2            0x0010
#define PHSTAT3_SPDDPX1            0x0008
#define PHSTAT3_SPDDPX0            0x0004

//Receive status vector
#define RSV_UNICAST_FILTER         0x00100000
#define RSV_PATTERN_MATCH_FILTER   0x00080000
#define RSV_MAGIC_PACKET_FILTER    0x00040000
#define RSV_HASH_FILTER            0x00020000
#define RSV_NOT_ME_FILTER          0x00010000
#define RSV_RUNT_FILTER            0x00008000
#define RSV_VLAN_TYPE              0x00004000
#define RSV_UNKNOWN_OPCODE         0x00002000
#define RSV_PAUSE_CONTROL_FRAME    0x00001000
#define RSV_CONTROL_FRAME          0x00000800
#define RSV_DRIBBLE_NIBBLE         0x00000400
#define RSV_BROADCAST_PACKET       0x00000200
#define RSV_MULTICAST_PACKET       0x00000100
#define RSV_RECEIVED_OK            0x00000080
#define RSV_LENGTH_OUT_OF_RANGE    0x00000040
#define RSV_LENGTH_CHECK_ERROR     0x00000020
#define RSV_CRC_ERROR              0x00000010
#define RSV_CARRIER_EVENT          0x00000004
#define RSV_PACKET_IGNORED         0x00000001

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief ENC624J600 driver context
 **/

typedef struct
{
   uint16_t nextPacket; ///<Next packet in the receive buffer
   uint8_t *rxBuffer;   ///<Receive buffer
} Enc624j600Context;


//ENC624J600 driver
extern const NicDriver enc624j600Driver;

//ENC624J600 related functions
error_t enc624j600Init(NetInterface *interface);

void enc624j600Tick(NetInterface *interface);

void enc624j600EnableIrq(NetInterface *interface);
void enc624j600DisableIrq(NetInterface *interface);
bool_t enc624j600IrqHandler(NetInterface *interface);
void enc624j600EventHandler(NetInterface *interface);

error_t enc624j600SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset);

error_t enc624j600ReceivePacket(NetInterface *interface);

error_t enc624j600SetMulticastFilter(NetInterface *interface);
void enc624j600UpdateMacConfig(NetInterface *interface);

error_t enc624j600SoftReset(NetInterface *interface);

void enc624j600WriteReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t enc624j600ReadReg(NetInterface *interface, uint8_t address);

void enc624j600WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t enc624j600ReadPhyReg(NetInterface *interface, uint8_t address);

void enc624j600WriteBuffer(NetInterface *interface,
   uint8_t opcode, const NetBuffer *buffer, size_t offset);

void enc624j600ReadBuffer(NetInterface *interface,
   uint8_t opcode, uint8_t *data, size_t length);

void enc624j600SetBit(NetInterface *interface, uint8_t address, uint16_t mask);
void enc624j600ClearBit(NetInterface *interface, uint8_t address, uint16_t mask);

uint32_t enc624j600CalcCrc(const void *data, size_t length);

void enc624j600DumpReg(NetInterface *interface);
void enc624j600DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
