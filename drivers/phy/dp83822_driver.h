/**
 * @file dp83822_driver.h
 * @brief DP83822 Ethernet PHY transceiver
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

#ifndef _DP83822_DRIVER_H
#define _DP83822_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef DP83822_PHY_ADDR
   #define DP83822_PHY_ADDR 1
#elif (DP83822_PHY_ADDR < 0 || DP83822_PHY_ADDR > 31)
   #error DP83822_PHY_ADDR parameter is not valid
#endif

//DP83822 registers
#define DP83822_PHY_REG_BMCR       0x00
#define DP83822_PHY_REG_BMSR       0x01
#define DP83822_PHY_REG_PHYIDR1    0x02
#define DP83822_PHY_REG_PHYIDR2    0x03
#define DP83822_PHY_REG_ANAR       0x04
#define DP83822_PHY_REG_ANLPAR     0x05
#define DP83822_PHY_REG_ANER       0x06
#define DP83822_PHY_REG_ANNPTR     0x07
#define DP83822_PHY_REG_ANLNPTR    0x08
#define DP83822_PHY_REG_CR1        0x09
#define DP83822_PHY_REG_CR2        0x0A
#define DP83822_PHY_REG_CR3        0x0B
#define DP83822_PHY_REG_REGCR      0x0D
#define DP83822_PHY_REG_ADDAR      0x0E
#define DP83822_PHY_REG_FLDS       0x0F
#define DP83822_PHY_REG_PHYSTS     0x10
#define DP83822_PHY_REG_PHYSCR     0x11
#define DP83822_PHY_REG_MISR1      0x12
#define DP83822_PHY_REG_MISR2      0x13
#define DP83822_PHY_REG_FCSCR      0x14
#define DP83822_PHY_REG_RECR       0x15
#define DP83822_PHY_REG_BISCR      0x16
#define DP83822_PHY_REG_RCSR       0x17
#define DP83822_PHY_REG_LEDCR      0x18
#define DP83822_PHY_REG_PHYCR      0x19
#define DP83822_PHY_REG_10BTSCR    0x1A
#define DP83822_PHY_REG_BICSR1     0x1B
#define DP83822_PHY_REG_BICSR2     0x1C
#define DP83822_PHY_REG_CDCR       0x1E
#define DP83822_PHY_REG_PHYRCR     0x1F

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

//ANLNPTR register
#define ANLNPTR_NP                 (1 << 15)
#define ANLNPTR_ACK                (1 << 14)
#define ANLNPTR_MP                 (1 << 13)
#define ANLNPTR_ACK2               (1 << 12)
#define ANLNPTR_TOGGLE             (1 << 11)
#define ANLNPTR_MESSAGE10          (1 << 10)
#define ANLNPTR_MESSAGE9           (1 << 9)
#define ANLNPTR_MESSAGE8           (1 << 8)
#define ANLNPTR_MESSAGE7           (1 << 7)
#define ANLNPTR_MESSAGE6           (1 << 6)
#define ANLNPTR_MESSAGE5           (1 << 5)
#define ANLNPTR_MESSAGE4           (1 << 4)
#define ANLNPTR_MESSAGE3           (1 << 3)
#define ANLNPTR_MESSAGE2           (1 << 2)
#define ANLNPTR_MESSAGE1           (1 << 1)
#define ANLNPTR_MESSAGE0           (1 << 0)

//CR1 register
#define CR1_RMII_ENHANCED_MODE     (1 << 9)
#define CR1_TDR_AUTO_RUN           (1 << 8)
#define CR1_LINK_LOSS_RECOVERY     (1 << 7)
#define CR1_FAST_AUTO_MDIX         (1 << 6)
#define CR1_ROBUST_AUTO_MDIX       (1 << 5)
#define CR1_FAST_AN_EN             (1 << 4)
#define CR1_FAST_AN_SEL1           (1 << 3)
#define CR1_FAST_AN_SEL0           (1 << 2)
#define CR1_FAST_RX_DV_DETECT      (1 << 1)

//CR2 register
#define CR2_FORCE_FAR_END_LINK_DROP (1 << 15)
#define CR2_100BFX_EN               (1 << 14)
#define CR2_ASY_LINK_UP_IN_PD       (1 << 6)
#define CR2_EXT_FULL_DUPLEX_ABLE    (1 << 5)
#define CR2_ENHANCED_LED_LINK       (1 << 4)
#define CR2_ISOLATE_MII             (1 << 3)
#define CR2_RX_ER_DURING_IDLE       (1 << 2)
#define CR2_ODD_NIBBLE_DETECT_DIS   (1 << 1)
#define CR2_RMII_RX_CLK             (1 << 0)

//CR3 register
#define CR3_DESCR_FAST_LINK_DOWN    (1 << 10)
#define CR_POLARITY_SWAP            (1 << 6)
#define CR_MDIX_SWAP                (1 << 5)
#define CR_FAST_LINK_DOWN_MODE3     (1 << 3)
#define CR_FAST_LINK_DOWN_MODE2     (1 << 2)
#define CR_FAST_LINK_DOWN_MODE1     (1 << 1)
#define CR_FAST_LINK_DOWN_MODE0     (1 << 0)

//REGCR register
#define REGCR_EXTENDED_REG_CMD1     (1 << 15)
#define REGCR_EXTENDED_REG_CMD0     (1 << 14)
#define REGCR_DEVAD4                (1 << 4)
#define REGCR_DEVAD3                (1 << 3)
#define REGCR_DEVAD2                (1 << 2)
#define REGCR_DEVAD1                (1 << 1)
#define REGCR_DEVAD0                (1 << 0)

//FLDS register
#define FLDS_FAST_LINK_DOWN_STATUS4 (1 << 8)
#define FLDS_FAST_LINK_DOWN_STATUS3 (1 << 7)
#define FLDS_FAST_LINK_DOWN_STATUS2 (1 << 6)
#define FLDS_FAST_LINK_DOWN_STATUS1 (1 << 5)
#define FLDS_FAST_LINK_DOWN_STATUS0 (1 << 4)

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

//PHYSCR register
#define PHYSCR_DISABLE_PLL          (1 << 15)
#define PHYSCR_POWER_SAVE_MODE_EN   (1 << 14)
#define PHYSCR_POWER_SAVE_MODE1     (1 << 13)
#define PHYSCR_POWER_SAVE_MODE0     (1 << 12)
#define PHYSCR_SCRAMBLER_BYPASS     (1 << 11)
#define PHYSCR_LOOPBACK_FIFO_DEPTH1 (1 << 9)
#define PHYSCR_LOOPBACK_FIFO_DEPTH0 (1 << 8)
#define PHYSCR_COL_FULL_DUPLEX_EN   (1 << 4)
#define PHYSCR_INT_POLARITY         (1 << 3)
#define PHYSCR_TEST_INT             (1 << 2)
#define PHYSCR_INT_EN               (1 << 1)
#define PHYSCR_INT_OE               (1 << 0)

//MISR1 register
#define MISR1_LQ_INT                (1 << 15)
#define MISR1_ED_INT                (1 << 14)
#define MISR1_LINK_INT              (1 << 13)
#define MISR1_SPD_INT               (1 << 12)
#define MISR1_DUP_INT               (1 << 11)
#define MISR1_ANC_INT               (1 << 10)
#define MISR1_FHF_INT               (1 << 9)
#define MISR1_RHF_INT               (1 << 8)
#define MISR1_LQ_INT_EN             (1 << 7)
#define MISR1_ED_INT_EN             (1 << 6)
#define MISR1_LINK_INT_EN           (1 << 5)
#define MISR1_SPD_INT_EN            (1 << 4)
#define MISR1_DUP_INT_EN            (1 << 3)
#define MISR1_ANC_INT_EN            (1 << 2)
#define MISR1_FHF_INT_EN            (1 << 1)
#define MISR1_RHF_INT_EN            (1 << 0)

//MISR2 register
#define MISR2_EEE_ERROR_INT         (1 << 15)
#define MISR2_AN_ERROR_INT          (1 << 14)
#define MISR2_PR_INT                (1 << 13)
#define MISR2_FIFO_OF_UF_INT        (1 << 12)
#define MISR2_MDI_CHANGE_INT        (1 << 11)
#define MISR2_SLEEP_MODE_INT        (1 << 10)
#define MISR2_POL_CHANGE_INT        (1 << 9)
#define MISR2_JABBER_DETECT_INT     (1 << 8)
#define MISR2_EEE_ERROR_INT_EN      (1 << 7)
#define MISR2_AN_ERROR_INT_EN       (1 << 6)
#define MISR2_PR_INT_EN             (1 << 5)
#define MISR2_FIFO_OF_UF_INT_EN     (1 << 4)
#define MISR2_MDI_CHANGE_INT_EN     (1 << 3)
#define MISR2_SLEEP_MODE_INT_EN     (1 << 2)
#define MISR2_POL_CHANGE_INT_EN     (1 << 1)
#define MISR2_JABBER_DETECT_INT_EN  (1 << 0)

//FCSCR register
#define FCSCR_FCSCNT7               (1 << 7)
#define FCSCR_FCSCNT6               (1 << 6)
#define FCSCR_FCSCNT5               (1 << 5)
#define FCSCR_FCSCNT4               (1 << 4)
#define FCSCR_FCSCNT3               (1 << 3)
#define FCSCR_FCSCNT2               (1 << 2)
#define FCSCR_FCSCNT1               (1 << 1)
#define FCSCR_FCSCNT0               (1 << 0)

//RECR register
#define RECR_RXERCNT7               (1 << 7)
#define RECR_RXERCNT6               (1 << 6)
#define RECR_RXERCNT5               (1 << 5)
#define RECR_RXERCNT4               (1 << 4)
#define RECR_RXERCNT3               (1 << 3)
#define RECR_RXERCNT2               (1 << 2)
#define RECR_RXERCNT1               (1 << 1)
#define RECR_RXERCNT0               (1 << 0)

//BISCR register
#define BISCR_ERROR_COUNTER_MODE    (1 << 14)
#define BISCR_PRBS_CHECK            (1 << 13)
#define BISCR_PACKET_GEN_EN         (1 << 12)
#define BISCR_PRBS_CHECK_LOCK_SYNC  (1 << 11)
#define BISCR_PRBS_CHECK_SYNC_LOSS  (1 << 10)
#define BISCR_PACKET_GEN_STATUS     (1 << 9)
#define BISCR_POWER_MODE            (1 << 8)
#define BISCR_TX_MII_LOOPBACK       (1 << 6)
#define BISCR_LOOPBACK_MODE4        (1 << 4)
#define BISCR_LOOPBACK_MODE3        (1 << 3)
#define BISCR_LOOPBACK_MODE2        (1 << 2)
#define BISCR_LOOPBACK_MODE1        (1 << 1)
#define BISCR_LOOPBACK_MODE0        (1 << 0)

//RCSR register
#define RCSR_RGMII_RX_CLK_SHIFT     (1 << 12)
#define RCSR_RGMII_TX_CLK_SHIFT     (1 << 11)
#define RCSR_RGMII_TX_SYNCED        (1 << 10)
#define RCSR_RGMII_MODE             (1 << 9)
#define RCSR_RMII_TX_CLOCK_SHIFT    (1 << 8)
#define RCSR_RMII_CLK_SEL           (1 << 7)
#define RCSR_RMII_ASYNC_FIFO_BP     (1 << 6)
#define RCSR_RMII_MODE              (1 << 5)
#define RCSR_RMII_REV_SEL           (1 << 4)
#define RCSR_RMII_OVF_STATUS        (1 << 3)
#define RCSR_RMII_UNF_STATUS        (1 << 2)
#define RCSR_ELAST_BUFFER_SIZE1     (1 << 1)
#define RCSR_ELAST_BUFFER_SIZE0     (1 << 0)

//LEDCR register
#define LEDCR_BLINK_RATE1           (1 << 10)
#define LEDCR_BLINK_RATE0           (1 << 9)
#define LEDCR_LED_0_POLARITY        (1 << 7)
#define LEDCR_DRIVE_LED_0           (1 << 4)
#define LEDCR_LED_0_ON_OFF          (1 << 1)

//PHYCR register
#define PHYCR_MDIX_EN               (1 << 15)
#define PHYCR_FORCE_MDIX            (1 << 14)
#define PHYCR_PAUSE_RX_STATUS       (1 << 13)
#define PHYCR_PAUSE_TX_STATUS       (1 << 12)
#define PHYCR_MII_LINK_STATUS       (1 << 11)
#define PHYCR_BP_LED_STRETCH        (1 << 7)
#define PHYCR_LED_CONFIG            (1 << 5)
#define PHYCR_PHY_ADDR4             (1 << 4)
#define PHYCR_PHY_ADDR3             (1 << 3)
#define PHYCR_PHY_ADDR2             (1 << 2)
#define PHYCR_PHY_ADDR1             (1 << 1)
#define PHYCR_PHY_ADDR0             (1 << 0)

//10BTSCR register
#define _10BTSCR_RX_THRESHOLD_EN    (1 << 13)
#define _10BTSCR_SQUELCH3           (1 << 12)
#define _10BTSCR_SQUELCH2           (1 << 11)
#define _10BTSCR_SQUELCH1           (1 << 10)
#define _10BTSCR_SQUELCH0           (1 << 9)
#define _10BTSCR_NLP_DIS            (1 << 7)
#define _10BTSCR_POLARITY_STATUS    (1 << 4)
#define _10BTSCR_JABBER_DIS         (1 << 0)

//BICSR1 register
#define BICSR1_BIST_ERROR_COUNT7    (1 << 15)
#define BICSR1_BIST_ERROR_COUNT6    (1 << 14)
#define BICSR1_BIST_ERROR_COUNT5    (1 << 13)
#define BICSR1_BIST_ERROR_COUNT4    (1 << 12)
#define BICSR1_BIST_ERROR_COUNT3    (1 << 11)
#define BICSR1_BIST_ERROR_COUNT2    (1 << 10)
#define BICSR1_BIST_ERROR_COUNT1    (1 << 9)
#define BICSR1_BIST_ERROR_COUNT0    (1 << 8)
#define BICSR1_BIST_IPG_LENGTH7     (1 << 7)
#define BICSR1_BIST_IPG_LENGTH6     (1 << 6)
#define BICSR1_BIST_IPG_LENGTH5     (1 << 5)
#define BICSR1_BIST_IPG_LENGTH4     (1 << 4)
#define BICSR1_BIST_IPG_LENGTH3     (1 << 3)
#define BICSR1_BIST_IPG_LENGTH2     (1 << 2)
#define BICSR1_BIST_IPG_LENGTH1     (1 << 1)
#define BICSR1_BIST_IPG_LENGTH0     (1 << 0)

//BICSR2 register
#define BICSR2_BIST_PACKET_LENGTH10 (1 << 10)
#define BICSR2_BIST_PACKET_LENGTH9  (1 << 9)
#define BICSR2_BIST_PACKET_LENGTH8  (1 << 8)
#define BICSR2_BIST_PACKET_LENGTH7  (1 << 7)
#define BICSR2_BIST_PACKET_LENGTH6  (1 << 6)
#define BICSR2_BIST_PACKET_LENGTH5  (1 << 5)
#define BICSR2_BIST_PACKET_LENGTH4  (1 << 4)
#define BICSR2_BIST_PACKET_LENGTH3  (1 << 3)
#define BICSR2_BIST_PACKET_LENGTH2  (1 << 2)
#define BICSR2_BIST_PACKET_LENGTH1  (1 << 1)
#define BICSR2_BIST_PACKET_LENGTH0  (1 << 0)

//CDCR register
#define CDCR_CABLE_DIAG_START       (1 << 15)
#define CDCR_CABLE_DIAG_STATUS      (1 << 1)
#define CDCR_CABLE_DIAG_TEST_FAIL   (1 << 0)

//PHYRCR register
#define PHYRCR_SOFT_RESET           (1 << 15)
#define PHYRCR_DIGITAL_RESTART      (1 << 14)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//DP83822 Ethernet PHY driver
extern const PhyDriver dp83822PhyDriver;

//DP83822 related functions
error_t dp83822Init(NetInterface *interface);

void dp83822Tick(NetInterface *interface);

void dp83822EnableIrq(NetInterface *interface);
void dp83822DisableIrq(NetInterface *interface);

void dp83822EventHandler(NetInterface *interface);

void dp83822WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t dp83822ReadPhyReg(NetInterface *interface, uint8_t address);

void dp83822DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
