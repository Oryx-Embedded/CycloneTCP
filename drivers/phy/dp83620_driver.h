/**
 * @file dp83620_driver.h
 * @brief DP83620 Ethernet PHY transceiver
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

#ifndef _DP83620_DRIVER_H
#define _DP83620_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef DP83620_PHY_ADDR
   #define DP83620_PHY_ADDR 1
#elif (DP83620_PHY_ADDR < 0 || DP83620_PHY_ADDR > 31)
   #error DP83620_PHY_ADDR parameter is not valid
#endif

//DP83620 registers
#define DP83620_PHY_REG_BMCR       0x00
#define DP83620_PHY_REG_BMSR       0x01
#define DP83620_PHY_REG_PHYIDR1    0x02
#define DP83620_PHY_REG_PHYIDR2    0x03
#define DP83620_PHY_REG_ANAR       0x04
#define DP83620_PHY_REG_ANLPAR     0x05
#define DP83620_PHY_REG_ANER       0x06
#define DP83620_PHY_REG_ANNPTR     0x07
#define DP83620_PHY_REG_PHYSTS     0x10
#define DP83620_PHY_REG_MICR       0x11
#define DP83620_PHY_REG_MISR       0x12
#define DP83620_PHY_REG_PAGESEL    0x13

//Extended registers (page 0)
#define DP83620_PHY_REG_FCSCR      0x14
#define DP83620_PHY_REG_RECR       0x15
#define DP83620_PHY_REG_PCSR       0x16
#define DP83620_PHY_REG_RBR        0x17
#define DP83620_PHY_REG_LEDCR      0x18
#define DP83620_PHY_REG_PHYCR      0x19
#define DP83620_PHY_REG_10BTSCR    0x1A
#define DP83620_PHY_REG_CDCTRL1    0x1B
#define DP83620_PHY_REG_PHYCR2     0x1C
#define DP83620_PHY_REG_EDCR       0x1D
#define DP83620_PHY_REG_PCFCR      0x1F

//Extended registers (page 1)
#define DP83620_PHY_REG_SD_CNFG    0x1E

//Extended registers (page 2)
#define DP83620_PHY_REG_LEN100_DET 0x14
#define DP83620_PHY_REG_FREQ100    0x15
#define DP83620_PHY_REG_TDR_CTRL   0x16
#define DP83620_PHY_REG_TDR_WIN    0x17
#define DP83620_PHY_REG_TDR_PEAK   0x18
#define DP83620_PHY_REG_TDR_THR    0x19
#define DP83620_PHY_REG_VAR_CTRL   0x1A
#define DP83620_PHY_REG_VAR_DAT    0x1B
#define DP83620_PHY_REG_LQMR       0x1D
#define DP83620_PHY_REG_LQDR       0x1E
#define DP83620_PHY_REG_LQMR2      0x1F

//Extended registers (page 5)
#define DP83620_PHY_REG_PSF_CFG    0x18

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
#define BMCR_UNIDIRECTIONAL_EN     (1 << 5)

//BMSR register
#define BMSR_100BT4                (1 << 15)
#define BMSR_100BTX_FD             (1 << 14)
#define BMSR_100BTX                (1 << 13)
#define BMSR_10BT_FD               (1 << 12)
#define BMSR_10BT                  (1 << 11)
#define BMSR_UNIDIRECTIONAL_ABLE   (1 << 7)
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
#define MISR_LQ_INT_EN             (1 << 7)
#define MISR_ED_INT_EN             (1 << 6)
#define MISR_LINK_INT_EN           (1 << 5)
#define MISR_SPD_INT_EN            (1 << 4)
#define MISR_DUP_INT_EN            (1 << 3)
#define MISR_ANC_INT_EN            (1 << 2)
#define MISR_FHF_INT_EN            (1 << 1)
#define MISR_RHF_INT_EN            (1 << 0)

//PAGESEL register
#define PAGESEL_PAGE_SEL2          (1 << 2)
#define PAGESEL_PAGE_SEL1          (1 << 1)
#define PAGESEL_PAGE_SEL0          (1 << 0)

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
#define PCSR_AUTO_CROSSOVER        (1 << 15)
#define PCSR_FREE_CLK              (1 << 11)
#define PCSR_TQ_EN                 (1 << 10)
#define PCSR_SD_FORCE_PMA          (1 << 9)
#define PCSR_SD_OPTION             (1 << 8)
#define PCSR_DESC_TIME             (1 << 7)
#define PCSR_FX_EN                 (1 << 6)
#define PCSR_FORCE_100_OK          (1 << 5)
#define PCSR_FEFI_EN               (1 << 3)
#define PCSR_NRZI_BYPASS           (1 << 2)
#define PCSR_SCRAM_BYPASS          (1 << 1)
#define PCSR_DESCRAM_BYPASS        (1 << 0)

//RBR register
#define RBR_RMII_MASTER            (1 << 14)
#define RBR_DIS_TX_OPT             (1 << 13)
#define RBR_PMD_LOOP               (1 << 8)
#define RBR_SCMII_RX               (1 << 7)
#define RBR_SCMII_TX               (1 << 6)
#define RBR_RMII_MODE              (1 << 5)
#define RBR_RMII_REV1_0            (1 << 4)
#define RBR_RX_OVF_STS             (1 << 3)
#define RBR_RX_UNF_STS             (1 << 2)
#define RBR_ELAST_BUF1             (1 << 1)
#define RBR_ELAST_BUF0             (1 << 0)

//LEDCR register
#define LEDCR_DIS_SPDLED           (1 << 11)
#define LEDCR_DIS_LNKLED           (1 << 10)
#define LEDCR_DIS_ACTLED           (1 << 9)
#define LEDCR_LEDACT_RX            (1 << 8)
#define LEDCR_BLINK_FREQ1          (1 << 7)
#define LEDCR_BLINK_FREQ0          (1 << 6)
#define LEDCR_DRV_SPDLED           (1 << 5)
#define LEDCR_DRV_LNKLED           (1 << 4)
#define LEDCR_DRV_ACTLED           (1 << 3)
#define LEDCR_SPDLED               (1 << 2)
#define LEDCR_LNKLED               (1 << 1)
#define LEDCR_ACTLED               (1 << 0)

#define LEDCR_BLINK_FREQ_6HZ       (0 << 6)
#define LEDCR_BLINK_FREQ_12HZ      (1 << 6)
#define LEDCR_BLINK_FREQ_24HZ      (2 << 6)
#define LEDCR_BLINK_FREQ_48HZ      (3 << 6)

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
#define _10BTSCR_AUTOPOL_DIS       (1 << 3)
#define _10BTSCR_10BT_SCALE_MSB    (1 << 2)
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
#define CDCTRL1_MII_CLOCK_EN       (1 << 6)
#define CDCTRL1_BIST_CONT          (1 << 5)
#define CDCTRL1_CDPATTEN_10        (1 << 4)
#define CDCTRL1_MDIO_PULL_EN       (1 << 3)
#define CDCTRL1_PATT_GAP_10M       (1 << 2)
#define CDCTRL1_CDPATTSEL1         (1 << 1)
#define CDCTRL1_CDPATTSEL0         (1 << 0)

//PHYCR2 register
#define PHYCR2_SYNC_ENET_EN        (1 << 13)
#define PHYCR2_CLK_OUT RXCLK       (1 << 12)
#define PHYCR2_BC_WRITE            (1 << 11)
#define PHYCR2_PHYTER_COMP         (1 << 10)
#define PHYCR2_SOFT_RESET          (1 << 9)
#define PHYCR2_CLK_OUT_DIS         (1 << 1)

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

//PCFCR register
#define PCFCR_PCF_STS_ERR          (1 << 15)
#define PCFCR_PCF_STS_OK           (1 << 14)
#define PCFCR_PCF_DA_SEL           (1 << 8)
#define PCFCR_PCF_INT_CTL1         (1 << 7)
#define PCFCR_PCF_INT_CTL0         (1 << 6)
#define PCFCR_PCF_BC_DIS           (1 << 5)
#define PCFCR_PCF_BUF3             (1 << 4)
#define PCFCR_PCF_BUF2             (1 << 3)
#define PCFCR_PCF_BUF1             (1 << 2)
#define PCFCR_PCF_BUF0             (1 << 1)
#define PCFCR_PCF_EN               (1 << 0)

//SD_CNFG register
#define SD_CNFG_SD_TIME            (1 << 8)

//LEN100_DET register
#define LEN100_DET_CABLE_LEN7      (1 << 7)
#define LEN100_DET_CABLE_LEN6      (1 << 6)
#define LEN100_DET_CABLE_LEN5      (1 << 5)
#define LEN100_DET_CABLE_LEN4      (1 << 4)
#define LEN100_DET_CABLE_LEN3      (1 << 3)
#define LEN100_DET_CABLE_LEN2      (1 << 2)
#define LEN100_DET_CABLE_LEN1      (1 << 1)
#define LEN100_DET_CABLE_LEN0      (1 << 0)

//FREQ100 register
#define FREQ100_SAMPLE_FREQ        (1 << 15)
#define FREQ100_SEL_FC             (1 << 8)
#define FREQ100_FREQ_OFFSET7       (1 << 7)
#define FREQ100_FREQ_OFFSET6       (1 << 6)
#define FREQ100_FREQ_OFFSET5       (1 << 5)
#define FREQ100_FREQ_OFFSET4       (1 << 4)
#define FREQ100_FREQ_OFFSET3       (1 << 3)
#define FREQ100_FREQ_OFFSET2       (1 << 2)
#define FREQ100_FREQ_OFFSET1       (1 << 1)
#define FREQ100_FREQ_OFFSET0       (1 << 0)

//TDR_CTRL register
#define TDR_CTRL_TDR_ENABLE        (1 << 15)
#define TDR_CTRL_TDR_100MB         (1 << 14)
#define TDR_CTRL_TX_CHANNEL        (1 << 13)
#define TDR_CTRL_RX_CHANNEL        (1 << 12)
#define TDR_CTRL_SEND_TDR          (1 << 11)
#define TDR_CTRL_TDR_WIDTH2        (1 << 10)
#define TDR_CTRL_TDR_WIDTH1        (1 << 9)
#define TDR_CTRL_TDR_WIDTH0        (1 << 8)
#define TDR_CTRL_TDR_MIN_MODE      (1 << 7)
#define TDR_CTRL_RX_THRESHOLD5     (1 << 5)
#define TDR_CTRL_RX_THRESHOLD4     (1 << 4)
#define TDR_CTRL_RX_THRESHOLD3     (1 << 3)
#define TDR_CTRL_RX_THRESHOLD2     (1 << 2)
#define TDR_CTRL_RX_THRESHOLD1     (1 << 1)
#define TDR_CTRL_RX_THRESHOLD0     (1 << 0)

//TDR_WIN register
#define TDR_WIN_TDR_START7         (1 << 15)
#define TDR_WIN_TDR_START6         (1 << 14)
#define TDR_WIN_TDR_START5         (1 << 13)
#define TDR_WIN_TDR_START4         (1 << 12)
#define TDR_WIN_TDR_START3         (1 << 11)
#define TDR_WIN_TDR_START2         (1 << 10)
#define TDR_WIN_TDR_START1         (1 << 9)
#define TDR_WIN_TDR_START0         (1 << 8)
#define TDR_WIN_TDR_STOP7          (1 << 7)
#define TDR_WIN_TDR_STOP6          (1 << 6)
#define TDR_WIN_TDR_STOP5          (1 << 5)
#define TDR_WIN_TDR_STOP4          (1 << 4)
#define TDR_WIN_TDR_STOP3          (1 << 3)
#define TDR_WIN_TDR_STOP2          (1 << 2)
#define TDR_WIN_TDR_STOP1          (1 << 1)
#define TDR_WIN_TDR_STOP0          (1 << 0)

//TDR_PEAK register
#define TDR_PEAK_TDR_PEAK5         (1 << 13)
#define TDR_PEAK_TDR_PEAK4         (1 << 12)
#define TDR_PEAK_TDR_PEAK3         (1 << 11)
#define TDR_PEAK_TDR_PEAK2         (1 << 10)
#define TDR_PEAK_TDR_PEAK1         (1 << 9)
#define TDR_PEAK_TDR_PEAK0         (1 << 8)
#define TDR_PEAK_TDR_PEAK_TIME7    (1 << 7)
#define TDR_PEAK_TDR_PEAK_TIME6    (1 << 6)
#define TDR_PEAK_TDR_PEAK_TIME5    (1 << 5)
#define TDR_PEAK_TDR_PEAK_TIME4    (1 << 4)
#define TDR_PEAK_TDR_PEAK_TIME3    (1 << 3)
#define TDR_PEAK_TDR_PEAK_TIME2    (1 << 2)
#define TDR_PEAK_TDR_PEAK_TIME1    (1 << 1)
#define TDR_PEAK_TDR_PEAK_TIME0    (1 << 0)

//TDR_THR register
#define TDR_THR_TDR_THR_MET        (1 << 8)
#define TDR_THR_TDR_THR_TIME7      (1 << 7)
#define TDR_THR_TDR_THR_TIME6      (1 << 6)
#define TDR_THR_TDR_THR_TIME5      (1 << 5)
#define TDR_THR_TDR_THR_TIME4      (1 << 4)
#define TDR_THR_TDR_THR_TIME3      (1 << 3)
#define TDR_THR_TDR_THR_TIME2      (1 << 2)
#define TDR_THR_TDR_THR_TIME1      (1 << 1)
#define TDR_THR_TDR_THR_TIME0      (1 << 0)

//VAR_CTRL register
#define VAR_CTRL_VAR_RDY           (1 << 15)
#define VAR_CTRL_VAR_FREEZE        (1 << 3)
#define VAR_CTRL_VAR_TIMER1        (1 << 2)
#define VAR_CTRL_VAR_TIMER0        (1 << 1)
#define VAR_CTRL_VAR_ENABLE        (1 << 0)

//LQMR register
#define LQMR_LQM_ENABLE            (1 << 15)
#define LQMR_RESTART_ON_FC         (1 << 14)
#define LQMR_RESTART_ON_FREQ       (1 << 13)
#define LQMR_RESTART_ON_DBLW       (1 << 12)
#define LQMR_RESTART_ON_DAGC       (1 << 11)
#define LQMR_RESTART_ON_C1         (1 << 10)
#define LQMR_FC_HI_WARN            (1 << 9)
#define LQMR_FC_LO_WARN            (1 << 8)
#define LQMR_FREQ_HI_WARN          (1 << 7)
#define LQMR_FREQ_LO_WARN          (1 << 6)
#define LQMR_DBLW_HI_WARN          (1 << 5)
#define LQMR_DBLW_LO_WARN          (1 << 4)
#define LQMR_DAGC_HI_WARN          (1 << 3)
#define LQMR_DAGC_LO_WARN          (1 << 2)
#define LQMR_C1_HI_WARN            (1 << 1)
#define LQMR_C1_LO_WARN            (1 << 0)

//LQDR register
#define LQDR_SAMPLE_PARAM          (1 << 13)
#define LQDR_WRITE_LQ_THR          (1 << 12)
#define LQDR_LQ_PARAM_SEL2         (1 << 11)
#define LQDR_LQ_PARAM_SEL1         (1 << 10)
#define LQDR_LQ_PARAM_SEL0         (1 << 9)
#define LQDR_LQ_THR_SEL            (1 << 8)
#define LQDR_LQ_THR_DATA7          (1 << 7)
#define LQDR_LQ_THR_DATA6          (1 << 6)
#define LQDR_LQ_THR_DATA5          (1 << 5)
#define LQDR_LQ_THR_DATA4          (1 << 4)
#define LQDR_LQ_THR_DATA3          (1 << 3)
#define LQDR_LQ_THR_DATA2          (1 << 2)
#define LQDR_LQ_THR_DATA1          (1 << 1)
#define LQDR_LQ_THR_DATA0          (1 << 0)

//LQMR2 register
#define LQMR2_RESTART_ON_VAR       (1 << 10)
#define LQMR2_VAR_HI_WARN          (1 << 1)

//PSF_CFG register
#define PSF_CFG_MAC_SRC_ADD1       (1 << 12)
#define PSF_CFG_MAC_SRC_ADD0       (1 << 11)
#define PSF_CFG_MIN_PRE2           (1 << 10)
#define PSF_CFG_MIN_PRE1           (1 << 9)
#define PSF_CFG_MIN_PRE0           (1 << 8)
#define PSF_CFG_PSF_ENDIAN         (1 << 7)
#define PSF_CFG_PSF_IPV4           (1 << 6)
#define PSF_CFG_PSF_PCF_RD         (1 << 5)
#define PSF_CFG_PSF_ERR_EN         (1 << 4)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//DP83620 Ethernet PHY driver
extern const PhyDriver dp83620PhyDriver;

//DP83620 related functions
error_t dp83620Init(NetInterface *interface);

void dp83620Tick(NetInterface *interface);

void dp83620EnableIrq(NetInterface *interface);
void dp83620DisableIrq(NetInterface *interface);

void dp83620EventHandler(NetInterface *interface);

void dp83620WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t dp83620ReadPhyReg(NetInterface *interface, uint8_t address);

void dp83620DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
