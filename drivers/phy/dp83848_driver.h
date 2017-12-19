/**
 * @file dp83848_driver.h
 * @brief DP83848 Ethernet PHY transceiver
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

#ifndef _DP83848_DRIVER_H
#define _DP83848_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef DP83848_PHY_ADDR
   #define DP83848_PHY_ADDR 1
#elif (DP83848_PHY_ADDR < 0 || DP83848_PHY_ADDR > 31)
   #error DP83848_PHY_ADDR parameter is not valid
#endif

//DP83848 registers
#define DP83848_PHY_REG_BMCR       0x00
#define DP83848_PHY_REG_BMSR       0x01
#define DP83848_PHY_REG_PHYIDR1    0x02
#define DP83848_PHY_REG_PHYIDR2    0x03
#define DP83848_PHY_REG_ANAR       0x04
#define DP83848_PHY_REG_ANLPAR     0x05
#define DP83848_PHY_REG_ANER       0x06
#define DP83848_PHY_REG_ANNPTR     0x07
#define DP83848_PHY_REG_PHYSTS     0x10
#define DP83848_PHY_REG_MICR       0x11
#define DP83848_PHY_REG_MISR       0x12
#define DP83848_PHY_REG_FCSCR      0x14
#define DP83848_PHY_REG_RECR       0x15
#define DP83848_PHY_REG_PCSR       0x16
#define DP83848_PHY_REG_RBR        0x17
#define DP83848_PHY_REG_LEDCR      0x18
#define DP83848_PHY_REG_PHYCR      0x19
#define DP83848_PHY_REG_10BTSCR    0x1A
#define DP83848_PHY_REG_CDCTRL1    0x1B
#define DP83848_PHY_REG_EDCR       0x1D

//BMCR register
#define BMCR_RESET                 (1 << 15)
#define BMCR_LOOPBACK              (1 << 14)
#define BMCR_SPEED_SEL             (1 << 13)
#define BMCR_AN_EN                 (1 << 12)
#define BMCR_POWER_DOWN            (1 << 11)
#define BMCR_ISOLATE               (1 << 10)
#define BMCR_RESTART_AN            (1 << 9)
#define BMCR_DUPLEX_MODE           (1 << 8)
#define BMCR_COL_TEST              (1 << 7)

//BMSR register
#define BMSR_100BT4                (1 << 15)
#define BMSR_100BTX_FD             (1 << 14)
#define BMSR_100BTX                (1 << 13)
#define BMSR_10BT_FD               (1 << 12)
#define BMSR_10BT                  (1 << 11)
#define BMSR_NO_PREAMBLE           (1 << 6)
#define BMSR_AN_COMPLETE           (1 << 5)
#define BMSR_REMOTE_FAULT          (1 << 4)
#define BMSR_AN_ABLE               (1 << 3)
#define BMSR_LINK_STATUS           (1 << 2)
#define BMSR_JABBER_DETECT         (1 << 1)
#define BMSR_EXTENDED_CAP          (1 << 0)

//ANAR register
#define ANAR_NP                    (1 << 15)
#define ANAR_RF                    (1 << 13)
#define ANAR_ASM_DIR               (1 << 11)
#define ANAR_PAUSE                 (1 << 10)
#define ANAR_100BT4                (1 << 9)
#define ANAR_100BTX_FD             (1 << 8)
#define ANAR_100BTX                (1 << 7)
#define ANAR_10BT_FD               (1 << 6)
#define ANAR_10BT                  (1 << 5)
#define ANAR_SELECTOR4             (1 << 4)
#define ANAR_SELECTOR3             (1 << 3)
#define ANAR_SELECTOR2             (1 << 2)
#define ANAR_SELECTOR1             (1 << 1)
#define ANAR_SELECTOR0             (1 << 0)

//ANLPAR register
#define ANLPAR_NP                  (1 << 15)
#define ANLPAR_ACK                 (1 << 14)
#define ANLPAR_RF                  (1 << 13)
#define ANLPAR_ASM_DIR             (1 << 11)
#define ANLPAR_PAUSE               (1 << 10)
#define ANLPAR_100BT4              (1 << 9)
#define ANLPAR_100BTX_FD           (1 << 8)
#define ANLPAR_100BTX              (1 << 7)
#define ANLPAR_10BT_FD             (1 << 6)
#define ANLPAR_10BT                (1 << 5)
#define ANLPAR_SELECTOR4           (1 << 4)
#define ANLPAR_SELECTOR3           (1 << 3)
#define ANLPAR_SELECTOR2           (1 << 2)
#define ANLPAR_SELECTOR1           (1 << 1)
#define ANLPAR_SELECTOR0           (1 << 0)

//ANER register
#define ANER_PDF                   (1 << 4)
#define ANER_LP_NP_ABLE            (1 << 3)
#define ANER_NP_ABLE               (1 << 2)
#define ANER_PAGE_RX               (1 << 1)
#define ANER_LP_AN_ABLE            (1 << 0)

//ANNPTR register
#define ANNPTR_NP                  (1 << 15)
#define ANNPTR_MP                  (1 << 13)
#define ANNPTR_ACK2                (1 << 12)
#define ANNPTR_TOG_TX              (1 << 11)
#define ANNPTR_CODE10              (1 << 10)
#define ANNPTR_CODE9               (1 << 9)
#define ANNPTR_CODE8               (1 << 8)
#define ANNPTR_CODE7               (1 << 7)
#define ANNPTR_CODE6               (1 << 6)
#define ANNPTR_CODE5               (1 << 5)
#define ANNPTR_CODE4               (1 << 4)
#define ANNPTR_CODE3               (1 << 3)
#define ANNPTR_CODE2               (1 << 2)
#define ANNPTR_CODE1               (1 << 1)
#define ANNPTR_CODE0               (1 << 0)

//PHYSTS register
#define PHYSTS_MDIX_MODE           (1 << 14)
#define PHYSTS_RX_ERROR_LATCH      (1 << 13)
#define PHYSTS_POLARITY_STATUS     (1 << 12)
#define PHYSTS_FALSE_CARRIER_SENSE (1 << 11)
#define PHYSTS_SIGNAL_DETECT       (1 << 10)
#define PHYSTS_DESCRAMBLER_LOCK    (1 << 9)
#define PHYSTS_PAGE_RECEIVED       (1 << 8)
#define PHYSTS_MII_INTERRUPT       (1 << 7)
#define PHYSTS_REMOTE_FAULT        (1 << 6)
#define PHYSTS_JABBER_DETECT       (1 << 5)
#define PHYSTS_AN_COMPLETE         (1 << 4)
#define PHYSTS_LOOPBACK_STATUS     (1 << 3)
#define PHYSTS_DUPLEX_STATUS       (1 << 2)
#define PHYSTS_SPEED_STATUS        (1 << 1)
#define PHYSTS_LINK_STATUS         (1 << 0)

//MICR register
#define MICR_TINT                  (1 << 2)
#define MICR_INTEN                 (1 << 1)
#define MICR_INT_OE                (1 << 0)

//MISR register
#define MISR_ED_INT                (1 << 14)
#define MISR_LINK_INT              (1 << 13)
#define MISR_SPD_INT               (1 << 12)
#define MISR_DUP_INT               (1 << 11)
#define MISR_ANC_INT               (1 << 10)
#define MISR_FHF_INT               (1 << 9)
#define MISR_RHF_INT               (1 << 8)
#define MISR_ED_INT_EN             (1 << 6)
#define MISR_LINK_INT_EN           (1 << 5)
#define MISR_SPD_INT_EN            (1 << 4)
#define MISR_DUP_INT_EN            (1 << 3)
#define MISR_ANC_INT_EN            (1 << 2)
#define MISR_FHF_INT_EN            (1 << 1)
#define MISR_RHF_INT_EN            (1 << 0)

//FCSCR register
#define FCSCR_FCSCNT7              (1 << 7)
#define FCSCR_FCSCNT6              (1 << 6)
#define FCSCR_FCSCNT5              (1 << 5)
#define FCSCR_FCSCNT4              (1 << 4)
#define FCSCR_FCSCNT3              (1 << 3)
#define FCSCR_FCSCNT2              (1 << 2)
#define FCSCR_FCSCNT1              (1 << 1)
#define FCSCR_FCSCNT0              (1 << 0)

//RECR register
#define RECR_RXERCNT7              (1 << 7)
#define RECR_RXERCNT6              (1 << 6)
#define RECR_RXERCNT5              (1 << 5)
#define RECR_RXERCNT4              (1 << 4)
#define RECR_RXERCNT3              (1 << 3)
#define RECR_RXERCNT2              (1 << 2)
#define RECR_RXERCNT1              (1 << 1)
#define RECR_RXERCNT0              (1 << 0)

//PCSR register
#define PCSR_TQ_EN                 (1 << 10)
#define PCSR_SD_FORCE_PMA          (1 << 9)
#define PCSR_SD_OPTION             (1 << 8)
#define PCSR_DESC_TIME             (1 << 7)
#define PCSR_FORCE_100_OK          (1 << 5)
#define PCSR_NRZI_BYPASS           (1 << 2)

//RBR register
#define RBR_RMII_MODE              (1 << 5)
#define RBR_RMII_REV1_0            (1 << 4)
#define RBR_RX_OVF_STS             (1 << 3)
#define RBR_RX_UNF_STS             (1 << 2)
#define RBR_ELAST_BUF1             (1 << 1)
#define RBR_ELAST_BUF0             (1 << 0)

//LEDCR register
#define LEDCR_DRV_SPDLED           (1 << 5)
#define LEDCR_DRV_LNKLED           (1 << 4)
#define LEDCR_DRV_ACTLED           (1 << 3)
#define LEDCR_SPDLED               (1 << 2)
#define LEDCR_LNKLED               (1 << 1)
#define LEDCR_ACTLED               (1 << 0)

//PHYCR register
#define PHYCR_MDIX_EN              (1 << 15)
#define PHYCR_FORCE_MDIX           (1 << 14)
#define PHYCR_PAUSE_RX             (1 << 13)
#define PHYCR_PAUSE_TX             (1 << 12)
#define PHYCR_BIST_FE              (1 << 11)
#define PHYCR_PSR_15               (1 << 10)
#define PHYCR_BIST_STATUS          (1 << 9)
#define PHYCR_BIST_START           (1 << 8)
#define PHYCR_BP_STRETCH           (1 << 7)
#define PHYCR_LED_CNFG1            (1 << 6)
#define PHYCR_LED_CNFG0            (1 << 5)
#define PHYCR_PHYADDR4             (1 << 4)
#define PHYCR_PHYADDR3             (1 << 3)
#define PHYCR_PHYADDR2             (1 << 2)
#define PHYCR_PHYADDR1             (1 << 1)
#define PHYCR_PHYADDR0             (1 << 0)

//10BTSCR register
#define _10BTSCR_10BT_SERIAL       (1 << 15)
#define _10BTSCR_SQUELCH2          (1 << 11)
#define _10BTSCR_SQUELCH1          (1 << 10)
#define _10BTSCR_SQUELCH0          (1 << 9)
#define _10BTSCR_LOOPBACK_10_DIS   (1 << 8)
#define _10BTSCR_LP_DIS            (1 << 7)
#define _10BTSCR_FORCE_LINK_10     (1 << 6)
#define _10BTSCR_POLARITY          (1 << 4)
#define _10BTSCR_HEARTBEAT_DIS     (1 << 1)
#define _10BTSCR_JABBER_DIS        (1 << 0)

//CDCTRL1 register
#define CDCTRL1_BIST_ERROR_COUNT7  (1 << 15)
#define CDCTRL1_BIST_ERROR_COUNT6  (1 << 14)
#define CDCTRL1_BIST_ERROR_COUNT5  (1 << 13)
#define CDCTRL1_BIST_ERROR_COUNT4  (1 << 12)
#define CDCTRL1_BIST_ERROR_COUNT3  (1 << 11)
#define CDCTRL1_BIST_ERROR_COUNT2  (1 << 10)
#define CDCTRL1_BIST_ERROR_COUNT1  (1 << 9)
#define CDCTRL1_BIST_ERROR_COUNT0  (1 << 8)
#define CDCTRL1_BIST_CONT_MODE     (1 << 5)
#define CDCTRL1_CDPATTEN_10        (1 << 4)
#define CDCTRL1_10MEG_PATT_GAP     (1 << 2)
#define CDCTRL1_CDPATTSEL1         (1 << 1)
#define CDCTRL1_CDPATTSEL0         (1 << 0)

//EDCR register
#define EDCR_ED_EN                 (1 << 15)
#define EDCR_ED_AUTO_UP            (1 << 14)
#define EDCR_ED_AUTO_DOWN          (1 << 13)
#define EDCR_ED_MAN                (1 << 12)
#define EDCR_ED_BURST_DIS          (1 << 11)
#define EDCR_ED_PWR_STATE          (1 << 10)
#define EDCR_ED_ERR_MET            (1 << 9)
#define EDCR_ED_DATA_MET           (1 << 8)
#define EDCR_ED_ERR_COUNT3         (1 << 7)
#define EDCR_ED_ERR_COUNT2         (1 << 6)
#define EDCR_ED_ERR_COUNT1         (1 << 5)
#define EDCR_ED_ERR_COUNT0         (1 << 4)
#define EDCR_ED_DATA_COUNT3        (1 << 3)
#define EDCR_ED_DATA_COUNT2        (1 << 2)
#define EDCR_ED_DATA_COUNT1        (1 << 1)
#define EDCR_ED_DATA_COUNT0        (1 << 0)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//DP83848 Ethernet PHY driver
extern const PhyDriver dp83848PhyDriver;

//DP83848 related functions
error_t dp83848Init(NetInterface *interface);

void dp83848Tick(NetInterface *interface);

void dp83848EnableIrq(NetInterface *interface);
void dp83848DisableIrq(NetInterface *interface);

void dp83848EventHandler(NetInterface *interface);

void dp83848WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t dp83848ReadPhyReg(NetInterface *interface, uint8_t address);

void dp83848DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
