/**
 * @file ksz8851_driver.h
 * @brief KSZ8851 Ethernet controller
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

#ifndef _KSZ8851_DRIVER_H
#define _KSZ8851_DRIVER_H

//SPI interface support
#ifndef KSZ8851_SPI_SUPPORT
   #define KSZ8851_SPI_SUPPORT ENABLED
#elif (KSZ8851_SPI_SUPPORT != ENABLED && KSZ8851_SPI_SUPPORT != DISABLED)
   #error KSZ8851_SPI_SUPPORT parameter is not valid
#endif

//KSZ8851 data register
#ifndef KSZ8851_DATA_REG
   #define KSZ8851_DATA_REG *((volatile uint16_t *) 0x60000000)
#endif

//KSZ8851 command register
#ifndef KSZ8851_CMD_REG
   #define KSZ8851_CMD_REG *((volatile uint16_t *) 0x60000004)
#endif

//Device ID
#define KSZ8851_REV_A2_ID        0x8870
#define KSZ8851_REV_A3_ID        0x8872

//SPI command set
#define KSZ8851_CMD_RD_REG       0x00
#define KSZ8851_CMD_WR_REG       0x40
#define KSZ8851_CMD_RD_FIFO      0x80
#define KSZ8851_CMD_WR_FIFO      0xC0

//Byte enable bits
#if (KSZ8851_SPI_SUPPORT == ENABLED)
   #define KSZ8851_CMD_B0        0x04
   #define KSZ8851_CMD_B1        0x08
   #define KSZ8851_CMD_B2        0x10
   #define KSZ8851_CMD_B3        0x20
#else
   #define KSZ8851_CMD_B0        0x1000
   #define KSZ8851_CMD_B1        0x2000
   #define KSZ8851_CMD_B2        0x4000
   #define KSZ8851_CMD_B3        0x8000
#endif

//KSZ8851 registers
#define KSZ8851_REG_CCR          0x08
#define KSZ8851_REG_MARL         0x10
#define KSZ8851_REG_MARM         0x12
#define KSZ8851_REG_MARH         0x14
#define KSZ8851_REG_OBCR         0x20
#define KSZ8851_REG_EEPCR        0x22
#define KSZ8851_REG_MBIR         0x24
#define KSZ8851_REG_GRR          0x26
#define KSZ8851_REG_WFCR         0x2A
#define KSZ8851_REG_WF0CRC0      0x30
#define KSZ8851_REG_WF0CRC1      0x32
#define KSZ8851_REG_WF0BM0       0x34
#define KSZ8851_REG_WF0BM1       0x36
#define KSZ8851_REG_WF0BM2       0x38
#define KSZ8851_REG_WF0BM3       0x3A
#define KSZ8851_REG_WF1CRC0      0x40
#define KSZ8851_REG_WF1CRC1      0x42
#define KSZ8851_REG_WF1BM0       0x44
#define KSZ8851_REG_WF1BM1       0x46
#define KSZ8851_REG_WF1BM2       0x48
#define KSZ8851_REG_WF1BM3       0x4A
#define KSZ8851_REG_WF2CRC0      0x50
#define KSZ8851_REG_WF2CRC1      0x52
#define KSZ8851_REG_WF2BM0       0x54
#define KSZ8851_REG_WF2BM1       0x56
#define KSZ8851_REG_WF2BM2       0x58
#define KSZ8851_REG_WF2BM3       0x5A
#define KSZ8851_REG_WF3CRC0      0x60
#define KSZ8851_REG_WF3CRC1      0x62
#define KSZ8851_REG_WF3BM0       0x64
#define KSZ8851_REG_WF3BM1       0x66
#define KSZ8851_REG_WF3BM2       0x68
#define KSZ8851_REG_WF3BM3       0x6A
#define KSZ8851_REG_TXCR         0x70
#define KSZ8851_REG_TXSR         0x72
#define KSZ8851_REG_RXCR1        0x74
#define KSZ8851_REG_RXCR2        0x76
#define KSZ8851_REG_TXMIR        0x78
#define KSZ8851_REG_RXFHSR       0x7C
#define KSZ8851_REG_RXFHBCR      0x7E
#define KSZ8851_REG_TXQCR        0x80
#define KSZ8851_REG_RXQCR        0x82
#define KSZ8851_REG_TXFDPR       0x84
#define KSZ8851_REG_RXFDPR       0x86
#define KSZ8851_REG_RXDTTR       0x8C
#define KSZ8851_REG_RXDBCTR      0x8E
#define KSZ8851_REG_IER          0x90
#define KSZ8851_REG_ISR          0x92
#define KSZ8851_REG_RXFCTR       0x9C
#define KSZ8851_REG_TXNTFSR      0x9E
#define KSZ8851_REG_MAHTR0       0xA0
#define KSZ8851_REG_MAHTR1       0xA2
#define KSZ8851_REG_MAHTR2       0xA4
#define KSZ8851_REG_MAHTR3       0xA6
#define KSZ8851_REG_FCLWR        0xB0
#define KSZ8851_REG_FCHWR        0xB2
#define KSZ8851_REG_FCOWR        0xB4
#define KSZ8851_REG_CIDER        0xC0
#define KSZ8851_REG_CGCR         0xC6
#define KSZ8851_REG_IACR         0xC8
#define KSZ8851_REG_IADLR        0xD0
#define KSZ8851_REG_IADHR        0xD2
#define KSZ8851_REG_PMECR        0xD4
#define KSZ8851_REG_GSWUTR       0xD6
#define KSZ8851_REG_PHYRR        0xD8
#define KSZ8851_REG_P1MBCR       0xE4
#define KSZ8851_REG_P1MBSR       0xE6
#define KSZ8851_REG_PHY1ILR      0xE8
#define KSZ8851_REG_PHY1IHR      0xEA
#define KSZ8851_REG_P1ANAR       0xEC
#define KSZ8851_REG_P1ANLPR      0xEE
#define KSZ8851_REG_P1SCLMD      0xF4
#define KSZ8851_REG_P1CR         0xF6
#define KSZ8851_REG_P1SR         0xF8

//CCR register
#define CCR_BUS_ENDIAN_MODE      0x0400
#define CCR_EEPROM_PRESENCE      0x0200
#define CCR_SPI_MODE             0x0100
#define CCR_8_BIT_DATA_BUS       0x0080
#define CCR_16_BIT_DATA_BUS      0x0040
#define CCR_32_BIT_DATA_BUS      0x0020
#define CCR_BUS_SHARED_MODE      0x0010
#define CCR_128_PIN_PACKAGE      0x0008
#define CCR_48_PIN_PACKAGE       0x0002
#define CCR_32_PIN_PACKAGE       0x0001

//OBCR register
#define OBCR_OUT_DRIVE_STRENGTH  0x0040
#define OBCR_SPI_SO_DELAY2       0x0020
#define OBCR_SPI_SO_DELAY1       0x0010
#define OBCR_SPI_SO_DELAY0       0x0008
#define OBCR_BUS_CLOCK_SEL       0x0004
#define OBCR_BUS_CLOCK_DIV1      0x0002
#define OBCR_BUS_CLOCK_DIV0      0x0001

//EEPCR register
#define EEPCR_EESA               0x0010
#define EEPCR_EESB               0x0008
#define EEPCR_EECB2              0x0004
#define EEPCR_EECB1              0x0002
#define EEPCR_EECB0              0x0001

//MBIR register
#define MBIR_TXMBF               0x1000
#define MBIR_TXMBFA              0x0800
#define MBIR_TXMBFC2             0x0400
#define MBIR_TXMBFC1             0x0200
#define MBIR_TXMBFC0             0x0100
#define MBIR_RXMBF               0x0010
#define MBIR_RXMBFA              0x0008
#define MBIR_RXMBFC2             0x0004
#define MBIR_RXMBFC1             0x0002
#define MBIR_RXMBFC0             0x0001

//GRR register
#define GRR_QMU_MODULE_SOFT_RST  0x0002
#define GRR_GLOBAL_SOFT_RST      0x0001

//WFCR register
#define WFCR_MPRXE               0x0080
#define WFCR_WF3E                0x0008
#define WFCR_WF2E                0x0004
#define WFCR_WF1E                0x0002
#define WFCR_WF0E                0x0001

//TXCR register
#define TXCR_TCGICMP             0x0100
#define TXCR_TCGUDP              0x0080
#define TXCR_TCGTCP              0x0040
#define TXCR_TCGIP               0x0020
#define TXCR_FTXQ                0x0010
#define TXCR_TXFCE               0x0008
#define TXCR_TXPE                0x0004
#define TXCR_TXCE                0x0002
#define TXCR_TXE                 0x0001

//TXSR register
#define TXSR_TXLC                0x2000
#define TXSR_TXMC                0x1000
#define TXSR_TXFID5              0x0020
#define TXSR_TXFID4              0x0010
#define TXSR_TXFID3              0x0008
#define TXSR_TXFID2              0x0004
#define TXSR_TXFID1              0x0002
#define TXSR_TXFID0              0x0001

//RXCR1 register
#define RXCR1_FRXQ               0x8000
#define RXCR1_RXUDPFCC           0x4000
#define RXCR1_RXTCPFCC           0x2000
#define RXCR1_RXIPFCC            0x1000
#define RXCR1_RXPAFMA            0x0800
#define RXCR1_RXFCE              0x0400
#define RXCR1_RXEFE              0x0200
#define RXCR1_RXMAFMA            0x0100
#define RXCR1_RXBE               0x0080
#define RXCR1_RXME               0x0040
#define RXCR1_RXUE               0x0020
#define RXCR1_RXAE               0x0010
#define RXCR1_RXINVF             0x0002
#define RXCR1_RXE                0x0001

//RXCR2 register
#define RXCR2_SRDBL2             0x0080
#define RXCR2_SRDBL1             0x0040
#define RXCR2_SRDBL0             0x0020
#define RXCR2_IUFFP              0x0010
#define RXCR2_RXIUFCEZ           0x0008
#define RXCR2_UDPLFE             0x0004
#define RXCR2_RXICMPFCC          0x0002
#define RXCR2_RXSAF              0x0001

//TXMIR register
#define TXMIR_TXMA_MASK          0x1FFF

//RXFHSR register
#define RXFHSR_RXFV              0x8000
#define RXFHSR_RXICMPFCS         0x2000
#define RXFHSR_RXIPFCS           0x1000
#define RXFHSR_RXTCPFCS          0x0800
#define RXFHSR_RXUDPFCS          0x0400
#define RXFHSR_RXBF              0x0080
#define RXFHSR_RXMF              0x0040
#define RXFHSR_RXUF              0x0020
#define RXFHSR_RXMR              0x0010
#define RXFHSR_RXFT              0x0008
#define RXFHSR_RXFTL             0x0004
#define RXFHSR_RXRF              0x0002
#define RXFHSR_RXCE              0x0001

//RXFHBCR register
#define RXFHBCR_RXBC_MASK        0x0FFF

//TXQCR register
#define TXQCR_AETFE              0x0004
#define TXQCR_TXQMAM             0x0002
#define TXQCR_METFE              0x0001

//RXQCR register
#define RXQCR_RXDTTS             0x1000
#define RXQCR_RXDBCTS            0x0800
#define RXQCR_RXFCTS             0x0400
#define RXQCR_RXIPHTOE           0x0200
#define RXQCR_RXDTTE             0x0080
#define RXQCR_RXDBCTE            0x0040
#define RXQCR_RXFCTE             0x0020
#define RXQCR_ADRFE              0x0010
#define RXQCR_SDA                0x0008
#define RXQCR_RRXEF              0x0001

//TXFDPR register
#define TXFDPR_TXFPAI            0x4000

//RXFDPR register
#define RXFDPR_RXFPAI            0x4000

//IER register
#define IER_LCIE                 0x8000
#define IER_TXIE                 0x4000
#define IER_RXIE                 0x2000
#define IER_RXOIE                0x0800
#define IER_TXPSIE               0x0200
#define IER_RXPSIE               0x0100
#define IER_TXSAIE               0x0040
#define IER_RXWFDIE              0x0020
#define IER_RXMPDIE              0x0010
#define IER_LDIE                 0x0008
#define IER_EDIE                 0x0004
#define IER_SPIBEIE              0x0002
#define IER_DEDIE                0x0001

//ISR register
#define ISR_LCIS                 0x8000
#define ISR_TXIS                 0x4000
#define ISR_RXIS                 0x2000
#define ISR_RXOIS                0x0800
#define ISR_TXPSIS               0x0200
#define ISR_RXPSIS               0x0100
#define ISR_TXSAIS               0x0040
#define ISR_RXWFDIS              0x0020
#define ISR_RXMPDIS              0x0010
#define ISR_LDIS                 0x0008
#define ISR_EDIS                 0x0004
#define ISR_SPIBEIS              0x0002

//CGCR register
#define CGCR_LEDSEL0             0x0200

//IACR register
#define IACR_READ_ENABLE         0x1000
#define IACR_TABLE_SELECT1       0x0800
#define IACR_TABLE_SELECT0       0x0400

//PMECR register
#define PMECR_PME_DELAY_EN       0x4000
#define PMECR_PME_POLARITY       0x1000
#define PMECR_PME_WUP_FRAME_EN   0x0800
#define PMECR_PME_MAGIC_EN       0x0400
#define PMECR_PME_LINK_UP_EN     0x0200
#define PMECR_PME_ENERGY_EN      0x0100
#define PMECR_AUTO_WUP_EN        0x0080
#define PMECR_WUP_NORMAL_OP_MODE 0x0040
#define PMECR_WUP_FROM_WUP_FRAME 0x0020
#define PMECR_WUP_FROM_MAGIC     0x0010
#define PMECR_WUP_FROM_LINK_UP   0x0008
#define PMECR_WUP_FROM_ENERGY    0x0004
#define PMECR_PWR_MODE1          0x0002
#define PMECR_PWR_MODE0          0x0001

//PHYRR register
#define PHYRR_PHY_RESET          0x0001

//P1MBCR register
#define P1MBCR_LOCAL_LOOPBACK    0x4000
#define P1MBCR_FORCE_100         0x2000
#define P1MBCR_AN_ENABLE         0x1000
#define P1MBCR_RESTART_AN        0x0200
#define P1MBCR_FORCE_FULL_DUPLEX 0x0100
#define P1MBCR_HP_MDIX           0x0020
#define P1MBCR_FORCE_MDIX        0x0010
#define P1MBCR_DISABLE_MDIX      0x0008
#define P1MBCR_DISABLE_TRANSMIT  0x0002
#define P1MBCR_DISABLE_LED       0x0001

//P1MBSR register
#define P1MBSR_T4_CAPABLE        0x8000
#define P1MBSR_100_FD_CAPABLE    0x4000
#define P1MBSR_100_CAPABLE       0x2000
#define P1MBSR_10_FD_CAPABLE     0x1000
#define P1MBSR_10_CAPABLE        0x0800
#define P1MBSR_PREAMBLE_SUPPR    0x0040
#define P1MBSR_AN_COMPLETE       0x0020
#define P1MBSR_AN_CAPABLE        0x0008
#define P1MBSR_LINK_STATUS       0x0004
#define P1MBSR_JABBER_TEST       0x0002
#define P1MBSR_EXTENDED_CAPABLE  0x0001

//P1ANAR register
#define P1ANAR_NEXT_PAGE         0x8000
#define P1ANAR_REMOTE_FAULT      0x2000
#define P1ANAR_ADV_PAUSE         0x0400
#define P1ANAR_ADV_100_FD        0x0100
#define P1ANAR_ADV_100           0x0080
#define P1ANAR_ADV_10_FD         0x0040
#define P1ANAR_ADV_10            0x0020
#define P1ANAR_SELECTOR_FIELD4   0x0010
#define P1ANAR_SELECTOR_FIELD3   0x0008
#define P1ANAR_SELECTOR_FIELD2   0x0004
#define P1ANAR_SELECTOR_FIELD1   0x0002
#define P1ANAR_SELECTOR_FIELD0   0x0001

//P1ANLPR register
#define P1ANLPR_NEXT_PAGE        0x8000
#define P1ANLPR_LP_ACK           0x4000
#define P1ANLPR_REMOTE_FAULT     0x2000
#define P1ANLPR_ADV_PAUSE        0x0400
#define P1ANLPR_ADV_100_FD       0x0100
#define P1ANLPR_ADV_100          0x0080
#define P1ANLPR_ADV_10_FD        0x0040
#define P1ANLPR_ADV_10           0x0020

//P1SCLMD register
#define P1SCLMD_VCT_RESULT1      0x4000
#define P1SCLMD_VCT_RESULT0      0x2000
#define P1SCLMD_VCT_EN           0x1000
#define P1SCLMD_FORCE_LNK        0x0800
#define P1SCLMD_REMOTE_LOOPBACK  0x0200

//P1CR register
#define P1CR_LED_OFF             0x8000
#define P1CR_TX_DISABLE          0x4000
#define P1CR_RESTART_AN          0x2000
#define P1CR_DISABLE_AUTO_MDIX   0x0400
#define P1CR_FORCE_MDIX          0x0200
#define P1CR_AN_ENABLE           0x0080
#define P1CR_FORCE_SPEED         0x0040
#define P1CR_FORCE_DUPLEX        0x0020
#define P1CR_ADV_PAUSE           0x0010
#define P1CR_ADV_100_FD          0x0008
#define P1CR_ADV_100             0x0004
#define P1CR_ADV_10_FD           0x0002
#define P1CR_ADV_10              0x0001

//P1SR register
#define P1SR_HP_MDIX             0x8000
#define P1SR_REVERSED_POLARITY   0x2000
#define P1SR_OPERATION_SPEED     0x0400
#define P1SR_OPERATION_DUPLEX    0x0200
#define P1SR_MDIX_STATUS         0x0080
#define P1SR_AN_DONE             0x0040
#define P1SR_LINK_GOOD           0x0020
#define P1SR_PARTNER_ADV_PAUSE   0x0010
#define P1SR_PARTNER_ADV_100_FD  0x0008
#define P1SR_PARTNER_ADV_100     0x0004
#define P1SR_PARTNER_ADV_10_FD   0x0002
#define P1SR_PARTNER_ADV_10      0x0001

//Transmit control word
#define TX_CTRL_TXIC             0x8000
#define TX_CTRL_TXFID            0x003F

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief TX packet header
 **/

typedef __start_packed struct
{
   uint16_t controlWord;
   uint16_t byteCount;
} __end_packed Ksz8851TxHeader;


/**
 * @brief RX packet header
 **/

typedef __start_packed struct
{
   uint16_t statusWord;
   uint16_t byteCount;
} __end_packed Ksz8851RxHeader;


/**
 * @brief KSZ8851 driver context
 **/

typedef struct
{
   uint_t frameId;    ///<Identify a frame and its associated status
   uint8_t *txBuffer; ///<Transmit buffer
   uint8_t *rxBuffer; ///<Receive buffer
} Ksz8851Context;


//KSZ8851 driver
extern const NicDriver ksz8851Driver;

//KSZ8851 related functions
error_t ksz8851Init(NetInterface *interface);

void ksz8851Tick(NetInterface *interface);

void ksz8851EnableIrq(NetInterface *interface);
void ksz8851DisableIrq(NetInterface *interface);
bool_t ksz8851IrqHandler(NetInterface *interface);
void ksz8851EventHandler(NetInterface *interface);

error_t ksz8851SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset);

error_t ksz8851ReceivePacket(NetInterface *interface);

error_t ksz8851SetMulticastFilter(NetInterface *interface);

void ksz8851WriteReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t ksz8851ReadReg(NetInterface *interface, uint8_t address);

void ksz8851WriteFifo(NetInterface *interface, const uint8_t *data, size_t length);
void ksz8851ReadFifo(NetInterface *interface, uint8_t *data, size_t length);

void ksz8851SetBit(NetInterface *interface, uint8_t address, uint16_t mask);
void ksz8851ClearBit(NetInterface *interface, uint8_t address, uint16_t mask);

uint32_t ksz8851CalcCrc(const void *data, size_t length);

void ksz8851DumpReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
