/**
 * @file dm9000_driver.h
 * @brief DM9000A/B Ethernet controller
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

#ifndef _DM9000_DRIVER_H
#define _DM9000_DRIVER_H

//Dependencies
#include "core/ethernet.h"

//Loopback mode
#ifndef DM9000_LOOPBACK_MODE
   #define DM9000_LOOPBACK_MODE DISABLED
#elif (DM9000_LOOPBACK_MODE != ENABLED && DM9000_LOOPBACK_MODE != DISABLED)
   #error DM9000_LOOPBACK_MODE parameter is not valid
#endif

//DM9000 index register
#ifndef DM9000_INDEX_REG
   #define DM9000_INDEX_REG *((volatile uint16_t *) 0x30000000)
#endif

//DM9000 data register
#ifndef DM9000_DATA_REG
   #define DM9000_DATA_REG *((volatile uint16_t *) 0x30001000)
#endif

//DM9000 bus timing
#define AT91C_SMC2_NWS_2        (2 << 0)
#define AT91C_SMC2_TDF_2        (2 << 8)
#define AT91C_SMC2_BAT_16       (1 << 12)
#define AT91C_SMC2_DRP_STANDARD (0 << 15)
#define AT91C_SMC2_RWSETUP_1    (1 << 24)
#define AT91C_SMC2_RWHOLD_1     (1 << 28)

//DM9000 identifiers
#define DM9000_VID             0x0A46
#define DM9000_PID             0x9000
#define DM9000A_CHIP_REV       0x19
#define DM9000B_CHIP_REV       0x1A

//DM9000 registers
#define DM9000_REG_NCR         0x00
#define DM9000_REG_NSR         0x01
#define DM9000_REG_TCR         0x02
#define DM9000_REG_TSR1        0x03
#define DM9000_REG_TSR2        0x04
#define DM9000_REG_RCR         0x05
#define DM9000_REG_RSR         0x06
#define DM9000_REG_ROCR        0x07
#define DM9000_REG_BPTR        0x08
#define DM9000_REG_FCTR        0x09
#define DM9000_REG_FCR         0x0A
#define DM9000_REG_EPCR        0x0B
#define DM9000_REG_EPAR        0x0C
#define DM9000_REG_EPDRL       0x0D
#define DM9000_REG_EPDRH       0x0E
#define DM9000_REG_WCR         0x0F
#define DM9000_REG_PAR0        0x10
#define DM9000_REG_PAR1        0x11
#define DM9000_REG_PAR2        0x12
#define DM9000_REG_PAR3        0x13
#define DM9000_REG_PAR4        0x14
#define DM9000_REG_PAR5        0x15
#define DM9000_REG_MAR0        0x16
#define DM9000_REG_MAR1        0x17
#define DM9000_REG_MAR2        0x18
#define DM9000_REG_MAR3        0x19
#define DM9000_REG_MAR4        0x1A
#define DM9000_REG_MAR5        0x1B
#define DM9000_REG_MAR6        0x1C
#define DM9000_REG_MAR7        0x1D
#define DM9000_REG_GPCR        0x1E
#define DM9000_REG_GPR         0x1F
#define DM9000_REG_TRPAL       0x22
#define DM9000_REG_TRPAH       0x23
#define DM9000_REG_RWPAL       0x24
#define DM9000_REG_RWPAH       0x25
#define DM9000_REG_VIDL        0x28
#define DM9000_REG_VIDH        0x29
#define DM9000_REG_PIDL        0x2A
#define DM9000_REG_PIDH        0x2B
#define DM9000_REG_CHIPR       0x2C
#define DM9000_REG_TCR2        0x2D
#define DM9000_REG_OCR         0x2E
#define DM9000_REG_SMCR        0x2F
#define DM9000_REG_ETXCSR      0x30
#define DM9000_REG_TCSCR       0x31
#define DM9000_REG_RCSCSR      0x32
#define DM9000_REG_MPAR        0x33
#define DM9000_REG_LEDCR       0x34
#define DM9000_REG_BUSCR       0x38
#define DM9000_REG_INTCR       0x39
#define DM9000_REG_SCCR        0x50
#define DM9000_REG_RSCCR       0x51
#define DM9000_REG_MRCMDX      0xF0
#define DM9000_REG_MRCMDX1     0xF1
#define DM9000_REG_MRCMD       0xF2
#define DM9000_REG_MRRL        0xF4
#define DM9000_REG_MRRH        0xF5
#define DM9000_REG_MWCMDX      0xF6
#define DM9000_REG_MWCMD       0xF8
#define DM9000_REG_MWRL        0xFA
#define DM9000_REG_MWRH        0xFB
#define DM9000_REG_TXPLL       0xFC
#define DM9000_REG_TXPLH       0xFD
#define DM9000_REG_ISR         0xFE
#define DM9000_REG_IMR         0xFF

//DM9000 PHY registers
#define DM9000_PHY_REG_BMCR    0x00
#define DM9000_PHY_REG_BMSR    0x01
#define DM9000_PHY_REG_PHYIDR1 0x02
#define DM9000_PHY_REG_PHYIDR2 0x03
#define DM9000_PHY_REG_ANAR    0x04
#define DM9000_PHY_REG_ANLPAR  0x05
#define DM9000_PHY_REG_ANER    0x06
#define DM9000_PHY_REG_DSCR    0x10
#define DM9000_PHY_REG_DSCSR   0x11
#define DM9000_PHY_REG_10BTCSR 0x12
#define DM9000_PHY_REG_PWDOR   0x13
#define DM9000_PHY_REG_SCR     0x14
#define DM9000_PHY_REG_DSP     0x1B
#define DM9000_PHY_REG_PSCR    0x1D

//NCR register
#define NCR_WAKEEN         (1 << 6)
#define NCR_FCOL           (1 << 4)
#define NCR_FDX            (1 << 3)
#define NCR_LBK            (3 << 1)
#define NCR_RST            (1 << 0)

//NSR register
#define NSR_SPEED          (1 << 7)
#define NSR_LINKST         (1 << 6)
#define NSR_WAKEST         (1 << 5)
#define NSR_TX2END         (1 << 3)
#define NSR_TX1END         (1 << 2)
#define NSR_RXOV           (1 << 1)

//TCR register
#define TCR_TJDIS          (1 << 6)
#define TCR_EXCECM         (1 << 5)
#define TCR_PAD_DIS2       (1 << 4)
#define TCR_CRC_DIS2       (1 << 3)
#define TCR_PAD_DIS1       (1 << 2)
#define TCR_CRC_DIS1       (1 << 1)
#define TCR_TXREQ          (1 << 0)

//TSR1 and TSR2 registers
#define TSR_TJTO           (1 << 7)
#define TSR_LC             (1 << 6)
#define TSR_NC             (1 << 5)
#define TSR_LCOL           (1 << 4)
#define TSR_COL            (1 << 3)
#define TSR_EC             (1 << 2)

//RCR register
#define RCR_WTDIS          (1 << 6)
#define RCR_DIS_LONG       (1 << 5)
#define RCR_DIS_CRC        (1 << 4)
#define RCR_ALL            (1 << 3)
#define RCR_RUNT           (1 << 2)
#define RCR_PRMSC          (1 << 1)
#define RCR_RXEN           (1 << 0)

//RSR register
#define RSR_RF             (1 << 7)
#define RSR_MF             (1 << 6)
#define RSR_LCS            (1 << 5)
#define RSR_RWTO           (1 << 4)
#define RSR_PLE            (1 << 3)
#define RSR_AE             (1 << 2)
#define RSR_CE             (1 << 1)
#define RSR_FOE            (1 << 0)

//ROCR register
#define ROCR_ROC           (127 << 0)
#define ROCR_RXFU          (1 << 7)

//BPTR register
#define BPTR_BPHW          (15 << 4)
#define BPTR_JPT           (15 << 0)

//FCTR register
#define FCTR_HWOT          (15 << 4)
#define FCTR_LWOT          (15 << 0)

//FCR register
#define FCR_TXP0           (1 << 7)
#define FCR_TXPF           (1 << 6)
#define FCR_TXPEN          (1 << 5)
#define FCR_BKPA           (1 << 4)
#define FCR_BKPM           (1 << 3)
#define FCR_RXPS           (1 << 2)
#define FCR_RXPCS          (1 << 1)
#define FCR_FLCE           (1 << 0)

//EPCR register
#define EPCR_REEP          (1 << 5)
#define EPCR_WEP           (1 << 4)
#define EPCR_EPOS          (1 << 3)
#define EPCR_ERPRR         (1 << 2)
#define EPCR_ERPRW         (1 << 1)
#define EPCR_ERRE          (1 << 0)

//EPAR register
#define EPAR_PHY_ADR       (3 << 6)
#define EPAR_EROA          (31 << 0)

//WCR register
#define WCR_LINKEN         (1 << 5)
#define WCR_SAMPLEEN       (1 << 4)
#define WCR_MAGICEN        (1 << 3)
#define WCR_LINKST         (1 << 2)
#define WCR_SAMPLEST       (1 << 1)
#define WCR_MAGICST        (1 << 0)

//GPCR register
#define GPCR_GPC6          (1 << 6)
#define GPCR_GPC5          (1 << 5)
#define GPCR_GPC4          (1 << 4)
#define GPCR_GPC3          (1 << 3)
#define GPCR_GPC2          (1 << 2)
#define GPCR_GPC1          (1 << 1)

//GPR register
#define GPR_GPO6           (1 << 6)
#define GPR_GPO5           (1 << 5)
#define GPR_GPO4           (1 << 4)
#define GPR_GPIO3          (1 << 3)
#define GPR_GPIO2          (1 << 2)
#define GPR_GPIO1          (1 << 1)
#define GPR_PHYPD          (1 << 0)

//TCR2 register
#define TCR2_LED           (1 << 7)
#define TCR2_RLCP          (1 << 6)
#define TCR2_DTU           (1 << 5)
#define TCR2_ONEPM         (1 << 4)
#define TCR2_IFGS          (15 << 0)

//OCR register
#define OCR_SCC            (3 << 6)
#define OCR_SOE            (1 << 4)
#define OCR_SCS            (1 << 3)
#define OCR_PHYOP          (7 << 0)

//SMCR register
#define SMCR_SM_EN         (1 << 7)
#define SMCR_FLC           (1 << 2)
#define SMCR_FB1           (1 << 1)
#define SMCR_FB0           (1 << 0)

//ETXCSR register
#define ETXCSR_ETE         (1 << 7)
#define ETXCSR_ETS2        (1 << 6)
#define ETXCSR_ETS1        (1 << 5)
#define ETXCSR_ETT         (3 << 0)

//TCSCR register
#define TCSCR_UDPCSE       (1 << 2)
#define TCSCR_TCPCSE       (1 << 1)
#define TCSCR_IPCSE        (1 << 0)

//RCSCSR register
#define RCSCSR_UDPS        (1 << 7)
#define RCSCSR_TCPS        (1 << 6)
#define RCSCSR_IPS         (1 << 5)
#define RCSCSR_UDPP        (1 << 4)
#define RCSCSR_TCPP        (1 << 3)
#define RCSCSR_IPP         (1 << 2)
#define RCSCSR_RCSEN       (1 << 1)
#define RCSCSR_DCSE        (1 << 0)

//MPAR register
#define MPAR_ADR_EN        (1 << 7)
#define MPAR_EPHYADR       (31 << 0)

//LEDC register
#define LEDCR_GPIO         (1 << 1)
#define LEDCR_MII          (1 << 0)

//BUSCR register
#define BUSCR_CURR         (3 << 5)
#define BUSCR_EST          (1 << 3)
#define BUSCR_IOW_SPIKE    (1 << 1)
#define BUSCR_IOR_SPIKE    (1 << 0)

//INTCR register
#define INTCR_INT_TYPE     (1 << 1)
#define INTCR_INT_POL      (1 << 0)

//SCCR register
#define SCCR_DIS_CLK       (1 << 0)

//ISR register
#define ISR_IOMODE         (1 << 7)
#define ISR_LNKCHG         (1 << 5)
#define ISR_UDRUN          (1 << 4)
#define ISR_ROO            (1 << 3)
#define ISR_ROS            (1 << 2)
#define ISR_PT             (1 << 1)
#define ISR_PR             (1 << 0)

//IMR register
#define IMR_PAR            (1 << 7)
#define IMR_LNKCHGI        (1 << 5)
#define IMR_UDRUNI         (1 << 4)
#define IMR_ROOI           (1 << 3)
#define IMR_ROI            (1 << 2)
#define IMR_PTI            (1 << 1)
#define IMR_PRI            (1 << 0)

//PHY BMCR register
#define BMCR_RST           (1 << 15)
#define BMCR_LOOPBACK      (1 << 14)
#define BMCR_SPEED_SEL     (1 << 13)
#define BMCR_AN_EN         (1 << 12)
#define BMCR_PD            (1 << 11)
#define BMCR_ISOLATE       (1 << 10)
#define BMCR_RESTART_AN    (1 << 9)
#define BMCR_DUPLEX_MODE   (1 << 8)
#define BMCR_COL_TEST      (1 << 7)

//Loopback mode
#define DM9000_LBK_NORMAL  (0 << 1)
#define DM9000_LBK_MAC     (1 << 1)
#define DM9000_LBK_PHY     (2 << 1)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief DM9000 driver context
 **/

typedef struct
{
   uint_t queuedPackets; ///<Number of packets in transmission buffer
   uint8_t *txBuffer;    ///<Transmit buffer
   uint8_t *rxBuffer;    ///<Receive buffer
} Dm9000Context;


//DM9000 driver
extern const NicDriver dm9000Driver;

//DM9000 related functions
error_t dm9000Init(NetInterface *interface);

void dm9000Tick(NetInterface *interface);

void dm9000EnableIrq(NetInterface *interface);
void dm9000DisableIrq(NetInterface *interface);
bool_t dm9000IrqHandler(NetInterface *interface);
void dm9000EventHandler(NetInterface *interface);

error_t dm9000SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset);

error_t dm9000ReceivePacket(NetInterface *interface);

error_t dm9000SetMulticastFilter(NetInterface *interface);

void dm9000WriteReg(uint8_t address, uint8_t data);
uint8_t dm9000ReadReg(uint8_t address);

void dm9000WritePhyReg(uint8_t address, uint16_t data);
uint16_t dm9000ReadPhyReg(uint8_t address);

uint32_t dm9000CalcCrc(const void *data, size_t length);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
