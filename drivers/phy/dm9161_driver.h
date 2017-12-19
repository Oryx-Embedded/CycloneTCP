/**
 * @file dm9161_driver.h
 * @brief DM9161 Ethernet PHY transceiver
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

#ifndef _DM9161_DRIVER_H
#define _DM9161_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef DM9161_PHY_ADDR
   #define DM9161_PHY_ADDR 0
#elif (DM9161_PHY_ADDR < 0 || DM9161_PHY_ADDR > 31)
   #error DM9161_PHY_ADDR parameter is not valid
#endif

//DM9161 registers
#define DM9161_PHY_REG_BMCR      0x00
#define DM9161_PHY_REG_BMSR      0x01
#define DM9161_PHY_REG_PHYIDR1   0x02
#define DM9161_PHY_REG_PHYIDR2   0x03
#define DM9161_PHY_REG_ANAR      0x04
#define DM9161_PHY_REG_ANLPAR    0x05
#define DM9161_PHY_REG_ANER      0x06
#define DM9161_PHY_REG_DSCR      0x10
#define DM9161_PHY_REG_DSCSR     0x11
#define DM9161_PHY_REG_10BTCSR   0x12
#define DM9161_PHY_REG_MDINTR    0x15
#define DM9161_PHY_REG_RECR      0x16
#define DM9161_PHY_REG_DISCR     0x17
#define DM9161_PHY_REG_RLSR      0x18

//BMCR register
#define BMCR_RESET               (1 << 15)
#define BMCR_LOOPBACK            (1 << 14)
#define BMCR_SPEED_SEL           (1 << 13)
#define BMCR_AN_EN               (1 << 12)
#define BMCR_POWER_DOWN          (1 << 11)
#define BMCR_ISOLATE             (1 << 10)
#define BMCR_RESTART_AN          (1 << 9)
#define BMCR_DUPLEX_MODE         (1 << 8)
#define BMCR_COL_TEST            (1 << 7)

//BMSR register
#define BMSR_100BT4              (1 << 15)
#define BMSR_100BTX_FD           (1 << 14)
#define BMSR_100BTX              (1 << 13)
#define BMSR_10BT_FD             (1 << 12)
#define BMSR_10BT                (1 << 11)
#define BMSR_NO_PREAMBLE         (1 << 6)
#define BMSR_AN_COMPLETE         (1 << 5)
#define BMSR_REMOTE_FAULT        (1 << 4)
#define BMSR_AN_ABLE             (1 << 3)
#define BMSR_LINK_STATUS         (1 << 2)
#define BMSR_JABBER_DETECT       (1 << 1)
#define BMSR_EXTENDED_CAP        (1 << 0)

//ANAR register
#define ANAR_NP                  (1 << 15)
#define ANAR_ACK                 (1 << 14)
#define ANAR_RF                  (1 << 13)
#define ANAR_FCS                 (1 << 10)
#define ANAR_100BT4              (1 << 9)
#define ANAR_100BTX_FD           (1 << 8)
#define ANAR_100BTX              (1 << 7)
#define ANAR_10BT_FD             (1 << 6)
#define ANAR_10BT                (1 << 5)
#define ANAR_SELECTOR4           (1 << 4)
#define ANAR_SELECTOR3           (1 << 3)
#define ANAR_SELECTOR2           (1 << 2)
#define ANAR_SELECTOR1           (1 << 1)
#define ANAR_SELECTOR0           (1 << 0)

//ANLPAR register
#define ANLPAR_NP                (1 << 15)
#define ANLPAR_ACK               (1 << 14)
#define ANLPAR_RF                (1 << 13)
#define ANLPAR_FCS               (1 << 10)
#define ANLPAR_100BT4            (1 << 9)
#define ANLPAR_100BTX_FD         (1 << 8)
#define ANLPAR_100BTX            (1 << 7)
#define ANLPAR_10BT_FD           (1 << 6)
#define ANLPAR_10BT              (1 << 5)
#define ANLPAR_SELECTOR4         (1 << 4)
#define ANLPAR_SELECTOR3         (1 << 3)
#define ANLPAR_SELECTOR2         (1 << 2)
#define ANLPAR_SELECTOR1         (1 << 1)
#define ANLPAR_SELECTOR0         (1 << 0)

//ANER register
#define ANER_PDF                 (1 << 4)
#define ANER_LP_NP_ABLE          (1 << 3)
#define ANER_NP_ABLE             (1 << 2)
#define ANER_PAGE_RX             (1 << 1)
#define ANER_LP_AN_ABLE          (1 << 0)

//DSCR register
#define DSCR_BP_4B5B             (1 << 15)
#define DSCR_BP_SCR              (1 << 14)
#define DSCR_BP_ALIGN            (1 << 13)
#define DSCR_BP_ADPOK            (1 << 12)
#define DSCR_REPEATER            (1 << 11)
#define DSCR_TX                  (1 << 10)
#define DSCR_FEF                 (1 << 9)
#define DSCR_RMII_EN             (1 << 8)
#define DSCR_F_LINK_100          (1 << 7)
#define DSCR_SPLED_CTL           (1 << 6)
#define DSCR_COLLED_CTL          (1 << 5)
#define DSCR_RPDCTR_EN           (1 << 4)
#define DSCR_SMRST               (1 << 3)
#define DSCR_MFPSC               (1 << 2)
#define DSCR_SLEEP               (1 << 1)
#define DSCR_RLOUT               (1 << 0)

//DSCSR register
#define DSCSR_100FDX             (1 << 15)
#define DSCSR_100HDX             (1 << 14)
#define DSCSR_10FDX              (1 << 13)
#define DSCSR_10HDX              (1 << 12)
#define DSCSR_PHYADR4            (1 << 8)
#define DSCSR_PHYADR3            (1 << 7)
#define DSCSR_PHYADR2            (1 << 6)
#define DSCSR_PHYADR1            (1 << 5)
#define DSCSR_PHYADR0            (1 << 4)
#define DSCSR_ANMB3              (1 << 3)
#define DSCSR_ANMB2              (1 << 2)
#define DSCSR_ANMB1              (1 << 1)
#define DSCSR_ANMB0              (1 << 0)

//10BTCSR register
#define _10BTCSR_LP_EN           (1 << 14)
#define _10BTCSR_HBE             (1 << 13)
#define _10BTCSR_SQUELCH         (1 << 12)
#define _10BTCSR_JABEN           (1 << 11)
#define _10BTCSR_10BT_SER        (1 << 10)
#define _10BTCSR_POLR            (1 << 0)

//MDINTR register
#define MDINTR_INTR_PEND         (1 << 15)
#define MDINTR_FDX_MASK          (1 << 11)
#define MDINTR_SPD_MASK          (1 << 10)
#define MDINTR_LINK_MASK         (1 << 9)
#define MDINTR_INTR_MASK         (1 << 8)
#define MDINTR_FDX_CHANGE        (1 << 4)
#define MDINTR_SPD_CHANGE        (1 << 3)
#define MDINTR_LINK_CHANGE       (1 << 2)
#define MDINTR_INTR_STATUS       (1 << 0)

//RLSR register
#define RLSR_LH_LEDST            (1 << 13)
#define RLSR_LH_CSTS             (1 << 12)
#define RLSR_LH_RMII             (1 << 11)
#define RLSR_LH_SCRAM            (1 << 10)
#define RLSR_LH_REPTR            (1 << 9)
#define RLSR_LH_TSTMOD           (1 << 8)
#define RLSR_LH_OP2              (1 << 7)
#define RLSR_LH_OP1              (1 << 6)
#define RLSR_LH_OP0              (1 << 5)
#define RLSR_LH_PH4              (1 << 4)
#define RLSR_LH_PH3              (1 << 3)
#define RLSR_LH_PH2              (1 << 2)
#define RLSR_LH_PH1              (1 << 1)
#define RLSR_LH_PH0              (1 << 0)

//Auto-negotiation state machine
#define DSCSR_ANMB_MASK                      0x000F
#define DSCSR_ANMB_IDLE                      0x0000
#define DSCSR_ANMB_ABILITY_MATCH             0x0001
#define DSCSR_ANMB_ACK_MATCH                 0x0002
#define DSCSR_ANMB_ACK_MATCH_FAILED          0x0003
#define DSCSR_ANMB_CONSIST_MATCH             0x0004
#define DSCSR_ANMB_CONSIST_MATCH_FAILED      0x0005
#define DSCSR_ANMB_SIGNAL_LINK_READY         0x0006
#define DSCSR_ANMB_SIGNAL_LINK_READY_FAILED  0x0007
#define DSCSR_ANMB_AN_SUCCESS                0x0008

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//DM9161 Ethernet PHY driver
extern const PhyDriver dm9161PhyDriver;

//DM9161 related functions
error_t dm9161Init(NetInterface *interface);

void dm9161Tick(NetInterface *interface);

void dm9161EnableIrq(NetInterface *interface);
void dm9161DisableIrq(NetInterface *interface);

void dm9161EventHandler(NetInterface *interface);

void dm9161WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t dm9161ReadPhyReg(NetInterface *interface, uint8_t address);

void dm9161DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
