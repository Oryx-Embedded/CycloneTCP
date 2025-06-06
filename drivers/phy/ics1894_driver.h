/**
 * @file ics1894_driver.h
 * @brief ICS1894-32 Ethernet PHY driver
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

#ifndef _ICS1894_DRIVER_H
#define _ICS1894_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef ICS1894_PHY_ADDR
   #define ICS1894_PHY_ADDR 5
#elif (ICS1894_PHY_ADDR < 0 || ICS1894_PHY_ADDR > 31)
   #error ICS1894_PHY_ADDR parameter is not valid
#endif

//ICS1894 PHY registers
#define ICS1894_BMCR                          0x00
#define ICS1894_BMSR                          0x01
#define ICS1894_PHYID1                        0x02
#define ICS1894_PHYID2                        0x03
#define ICS1894_ANAR                          0x04
#define ICS1894_ANLPAR                        0x05
#define ICS1894_ANER                          0x06
#define ICS1894_ANNPT                         0x07
#define ICS1894_ANNPLPAR                      0x08
#define ICS1894_ECR1                          0x10
#define ICS1894_QPDSR                         0x11
#define ICS1894_10BT_OP                       0x12
#define ICS1894_ECR2                          0x13
#define ICS1894_ECR3                          0x14
#define ICS1894_ECR4                          0x15
#define ICS1894_ECR5                          0x16
#define ICS1894_ECR6                          0x17
#define ICS1894_ECR7                          0x18

//Basic Control register
#define ICS1894_BMCR_RESET                    0x8000
#define ICS1894_BMCR_LOOPBACK                 0x4000
#define ICS1894_BMCR_SPEED_SEL                0x2000
#define ICS1894_BMCR_AN_EN                    0x1000
#define ICS1894_BMCR_POWER_DOWN               0x0800
#define ICS1894_BMCR_ISOLATE                  0x0400
#define ICS1894_BMCR_RESTART_AN               0x0200
#define ICS1894_BMCR_DUPLEX_MODE              0x0100
#define ICS1894_BMCR_COL_TEST                 0x0080

//Basic Status register
#define ICS1894_BMSR_100BT4                   0x8000
#define ICS1894_BMSR_100BTX_FD                0x4000
#define ICS1894_BMSR_100BTX_HD                0x2000
#define ICS1894_BMSR_10BT_FD                  0x1000
#define ICS1894_BMSR_10BT_HD                  0x0800
#define ICS1894_BMSR_MF_PREAMBLE_SUPPR        0x0040
#define ICS1894_BMSR_AN_COMPLETE              0x0020
#define ICS1894_BMSR_REMOTE_FAULT             0x0010
#define ICS1894_BMSR_AN_CAPABLE               0x0008
#define ICS1894_BMSR_LINK_STATUS              0x0004
#define ICS1894_BMSR_JABBER_DETECT            0x0002
#define ICS1894_BMSR_EXTENDED_CAPABLE         0x0001

//PHY Identifier 1 register
#define ICS1894_PHYID1_OUI_MSB                0xFFFF
#define ICS1894_PHYID1_OUI_MSB_DEFAULT        0x0015

//PHY Identifier 2 register
#define ICS1894_PHYID2_OUI_LSB                0xFC00
#define ICS1894_PHYID2_OUI_LSB_DEFAULT        0xF400
#define ICS1894_PHYID2_MODEL_NUM              0x03F0
#define ICS1894_PHYID2_MODEL_NUM_DEFAULT      0x0050
#define ICS1894_PHYID2_REVISION_NUM           0x000F
#define ICS1894_PHYID2_REVISION_NUM_DEFAULT   0x0000

//Auto-Negotiation Advertisement register
#define ICS1894_ANAR_NEXT_PAGE                0x8000
#define ICS1894_ANAR_REMOTE_FAULT             0x2000
#define ICS1894_ANAR_100BT4                   0x0200
#define ICS1894_ANAR_100BTX_FD                0x0100
#define ICS1894_ANAR_100BTX_HD                0x0080
#define ICS1894_ANAR_10BT_FD                  0x0040
#define ICS1894_ANAR_10BT_HD                  0x0020
#define ICS1894_ANAR_SELECTOR                 0x001F
#define ICS1894_ANAR_SELECTOR_DEFAULT         0x0001

//Auto-Negotiation Link Partner Ability register
#define ICS1894_ANLPAR_NEXT_PAGE              0x8000
#define ICS1894_ANLPAR_ACK                    0x4000
#define ICS1894_ANLPAR_REMOTE_FAULT           0x2000
#define ICS1894_ANLPAR_100BT4                 0x0200
#define ICS1894_ANLPAR_100BTX_FD              0x0100
#define ICS1894_ANLPAR_100BTX_HD              0x0080
#define ICS1894_ANLPAR_10BT_FD                0x0040
#define ICS1894_ANLPAR_10BT_HD                0x0020
#define ICS1894_ANLPAR_SELECTOR               0x001F

//Auto-Negotiation Expansion register
#define ICS1894_ANER_PAR_DETECT_FAULT         0x0010
#define ICS1894_ANER_LP_NEXT_PAGE_ABLE        0x0008
#define ICS1894_ANER_NEXT_PAGE_ABLE           0x0004
#define ICS1894_ANER_PAGE_RECEIVED            0x0002
#define ICS1894_ANER_LP_AN_ABLE               0x0001

//Auto-Negotiation Next Page Transmit register
#define ICS1894_ANNPT_NEXT_PAGE               0x8000
#define ICS1894_ANNPT_MSG_PAGE                0x2000
#define ICS1894_ANNPT_ACK2                    0x1000
#define ICS1894_ANNPT_TOGGLE                  0x0800
#define ICS1894_ANNPT_MESSAGE                 0x07FF

//Auto-Negotiation Next Page Link Partner Ability register
#define ICS1894_ANNPLPAR_NEXT_PAGE            0x8000
#define ICS1894_ANNPLPAR_MSG_PAGE             0x2000
#define ICS1894_ANNPLPAR_ACK2                 0x1000
#define ICS1894_ANNPLPAR_TOGGLE               0x0800
#define ICS1894_ANNPLPAR_MESSAGE              0x07FF

//Extended Control 1 register
#define ICS1894_ECR1_CMD_OVERRIDE_WR_EN       0x8000
#define ICS1894_ECR1_PHY_ADDR                 0x07C0
#define ICS1894_ECR1_STREAM_CIPHER_TEST_MODE  0x0020
#define ICS1894_ECR1_NRZ_NRZI_ENCODING        0x0008
#define ICS1894_ECR1_TRANSMIT_INVALID_CODES   0x0004
#define ICS1894_ECR1_STREAM_CIPHER_DIS        0x0001

//Quick Poll Detailed Status register
#define ICS1894_QPDSR_DATA_RATE               0x8000
#define ICS1894_QPDSR_DATA_RATE_10MBPS        0x0000
#define ICS1894_QPDSR_DATA_RATE_100MBPS       0x8000
#define ICS1894_QPDSR_DUPLEX                  0x4000
#define ICS1894_QPDSR_DUPLEX_HD               0x0000
#define ICS1894_QPDSR_DUPLEX_FD               0x4000
#define ICS1894_QPDSR_AN_PROGRESS_MONITOR     0x3800
#define ICS1894_QPDSR_100BTX_SIGNAL_LOST      0x0400
#define ICS1894_QPDSR_100_PLL_LOCK_ERROR      0x0200
#define ICS1894_QPDSR_FALSE_CARRIER_DETECT    0x0100
#define ICS1894_QPDSR_INVALID_SYMBOL_DETECT   0x0080
#define ICS1894_QPDSR_HALT_SYMBOL_DETECT      0x0040
#define ICS1894_QPDSR_PREMATURE_END_DETECT    0x0020
#define ICS1894_QPDSR_AN_COMPLETE             0x0010
#define ICS1894_QPDSR_100BTX_SIGNAL_DETECT    0x0008
#define ICS1894_QPDSR_JABBER_DETECT           0x0004
#define ICS1894_QPDSR_REMOTE_FAULT            0x0002
#define ICS1894_QPDSR_LINK_STATUS             0x0001

//10Base-T Operations register
#define ICS1894_10BT_OP_REMOTE_JABBER_DETECT  0x8000
#define ICS1894_10BT_OP_POLARITY_REV          0x4000
#define ICS1894_10BT_OP_DATA_BUS_MODE         0x3000
#define ICS1894_10BT_OP_DATA_BUS_MODE_MII     0x0000
#define ICS1894_10BT_OP_DATA_BUS_MODE_SI      0x1000
#define ICS1894_10BT_OP_DATA_BUS_MODE_RMII    0x2000
#define ICS1894_10BT_OP_AMDIXEN               0x0800
#define ICS1894_10BT_OP_RXTRI                 0x0400
#define ICS1894_10BT_OP_REGEN                 0x0200
#define ICS1894_10BT_OP_TM_SWITCH             0x0100
#define ICS1894_10BT_OP_JABBER_INHIBIT        0x0020
#define ICS1894_10BT_OP_AUTO_POLARITY_INHIBIT 0x0008
#define ICS1894_10BT_OP_SQE_TEST_INHIBIT      0x0004
#define ICS1894_10BT_OP_LINK_LOSS_INHIBIT     0x0002
#define ICS1894_10BT_OP_SQUELCH_INHIBIT       0x0001

//Extended Control 2 register
#define ICS1894_ECR2_NODE_MODE                0x8000
#define ICS1894_ECR2_HW_SW_MODE_SPEED_SEL     0x4000
#define ICS1894_ECR2_REMOTE_FAULT             0x2000
#define ICS1894_ECR2_REGISTER_BANK_SEL        0x1800
#define ICS1894_ECR2_AMDIX_EN                 0x0200
#define ICS1894_ECR2_MDI_MODE                 0x0100
#define ICS1894_ECR2_TPTRI                    0x0080
#define ICS1894_ECR2_AUTO_100BTX_PD           0x0001

//Extended Control 3 register
#define ICS1894_ECR3_STR_ENHANCE              0x8000
#define ICS1894_ECR3_FAST_OFF                 0x4000
#define ICS1894_ECR3_LED4_MODE                0x3000
#define ICS1894_ECR3_LED4_MODE_RX             0x0000
#define ICS1894_ECR3_LED4_MODE_COL            0x1000
#define ICS1894_ECR3_LED4_MODE_FD             0x2000
#define ICS1894_ECR3_LED4_MODE_OFF            0x3000
#define ICS1894_ECR3_LED3_MODE                0x0E00
#define ICS1894_ECR3_LED3_MODE_LINK           0x0000
#define ICS1894_ECR3_LED3_MODE_ACT            0x0200
#define ICS1894_ECR3_LED3_MODE_TX             0x0400
#define ICS1894_ECR3_LED3_MODE_RX             0x0600
#define ICS1894_ECR3_LED3_MODE_COL            0x0800
#define ICS1894_ECR3_LED3_MODE_100_10         0x0A00
#define ICS1894_ECR3_LED3_MODE_FD             0x0C00
#define ICS1894_ECR3_LED3_MODE_OFF            0x0E00
#define ICS1894_ECR3_LED2_MODE                0x01C0
#define ICS1894_ECR3_LED2_MODE_LINK           0x0000
#define ICS1894_ECR3_LED2_MODE_ACT            0x0040
#define ICS1894_ECR3_LED2_MODE_TX             0x0080
#define ICS1894_ECR3_LED2_MODE_RX             0x00C0
#define ICS1894_ECR3_LED2_MODE_COL            0x0100
#define ICS1894_ECR3_LED2_MODE_100_10         0x0140
#define ICS1894_ECR3_LED2_MODE_FD             0x0180
#define ICS1894_ECR3_LED2_MODE_OFF            0x01C0
#define ICS1894_ECR3_LED1_MODE                0x0038
#define ICS1894_ECR3_LED1_MODE_LINK           0x0000
#define ICS1894_ECR3_LED1_MODE_ACT            0x0008
#define ICS1894_ECR3_LED1_MODE_TX             0x0010
#define ICS1894_ECR3_LED1_MODE_RX             0x0018
#define ICS1894_ECR3_LED1_MODE_COL            0x0020
#define ICS1894_ECR3_LED1_MODE_100_10         0x0028
#define ICS1894_ECR3_LED1_MODE_FD             0x0030
#define ICS1894_ECR3_LED1_MODE_OFF            0x0038
#define ICS1894_ECR3_LED0_MODE                0x0007
#define ICS1894_ECR3_LED0_MODE_LINK           0x0000
#define ICS1894_ECR3_LED0_MODE_ACT            0x0001
#define ICS1894_ECR3_LED0_MODE_TX             0x0002
#define ICS1894_ECR3_LED0_MODE_RX             0x0003
#define ICS1894_ECR3_LED0_MODE_COL            0x0004
#define ICS1894_ECR3_LED0_MODE_100_10         0x0005
#define ICS1894_ECR3_LED0_MODE_FD             0x0006
#define ICS1894_ECR3_LED0_MODE_LINK_STAT      0x0007

//Extended Control 4 register
#define ICS1894_ECR4_RXER_CNT                 0xFFFF

//Extended Control 5 register
#define ICS1894_ECR5_INT_OUT_EN               0x8000
#define ICS1894_ECR5_INT_FLAG_RD_CLEAR_EN     0x4000
#define ICS1894_ECR5_Int_pol                  0x2000
#define ICS1894_ECR5_INT_FLAG_AUTO_CLEAR_EN   0x1000
#define ICS1894_ECR5_INT_FLAG_RE_SETUP_EN     0x0800
#define ICS1894_ECR5_DPD_WU_INT_EN            0x0400
#define ICS1894_ECR5_DPD_INT_EN               0x0200
#define ICS1894_ECR5_AN_COMPLETE_INT_EN       0x0100
#define ICS1894_ECR5_JABBER_INT_EN            0x0080
#define ICS1894_ECR5_RX_ERROR_INT_EN          0x0040
#define ICS1894_ECR5_PAGE_RECEIVED_INT_EN     0x0020
#define ICS1894_ECR5_PAR_DETECT_FAULT_INT_EN  0x0010
#define ICS1894_ECR5_LP_ACK_INT_EN            0x0008
#define ICS1894_ECR5_LINK_DOWN_INT_EN         0x0004
#define ICS1894_ECR5_REMOTE_FAULT_INT_EN      0x0002
#define ICS1894_ECR5_LINK_UP_INT_EN           0x0001

//Extended Control 6 register
#define ICS1894_ECR6_DPD_WU_INT               0x0400
#define ICS1894_ECR6_DPD_INT                  0x0200
#define ICS1894_ECR6_AN_COMPLETE_INT          0x0100
#define ICS1894_ECR6_JABBER_INT               0x0080
#define ICS1894_ECR6_RX_ERROR_INT             0x0040
#define ICS1894_ECR6_PAGE_RECEIVED_INT        0x0020
#define ICS1894_ECR6_PAR_DETECT_FAULT_INT     0x0010
#define ICS1894_ECR6_LP_ACK_INT               0x0008
#define ICS1894_ECR6_LINK_DOWN_INT            0x0004
#define ICS1894_ECR6_REMOTE_FAULT_INT         0x0002
#define ICS1894_ECR6_LINK_UP_INT              0x0001

//Extended Control 7 register
#define ICS1894_ECR7_FIFO_HALF                0xF000
#define ICS1894_ECR7_DPD_EN                   0x0100
#define ICS1894_ECR7_TPLL_10_100_DPD_EN       0x0080
#define ICS1894_ECR7_RX_100_DPD_EN            0x0040
#define ICS1894_ECR7_ADMIX_TX_DPD_EN          0x0020
#define ICS1894_ECR7_CDR_100_CDR_DPD_EN       0x0010

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//ICS1894 Ethernet PHY driver
extern const PhyDriver ics1894PhyDriver;

//ICS1894 related functions
error_t ics1894Init(NetInterface *interface);
void ics1894InitHook(NetInterface *interface);

void ics1894Tick(NetInterface *interface);

void ics1894EnableIrq(NetInterface *interface);
void ics1894DisableIrq(NetInterface *interface);

void ics1894EventHandler(NetInterface *interface);

void ics1894WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data);

uint16_t ics1894ReadPhyReg(NetInterface *interface, uint8_t address);

void ics1894DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
