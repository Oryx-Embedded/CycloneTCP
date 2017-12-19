/**
 * @file st802rt1a_driver.h
 * @brief ST802RT1A Ethernet PHY transceiver
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

#ifndef _ST802RT1A_DRIVER_H
#define _ST802RT1A_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#define ST802RT1A_PHY_ADDR 1

//ST802RT1A registers
#define ST802RT1A_PHY_REG_RN00    0x00
#define ST802RT1A_PHY_REG_RN01    0x01
#define ST802RT1A_PHY_REG_RN02    0x02
#define ST802RT1A_PHY_REG_RN03    0x03
#define ST802RT1A_PHY_REG_RN04    0x04
#define ST802RT1A_PHY_REG_RN05    0x05
#define ST802RT1A_PHY_REG_RN06    0x06
#define ST802RT1A_PHY_REG_RN07    0x07
#define ST802RT1A_PHY_REG_RN08    0x08
#define ST802RT1A_PHY_REG_RN10    0x10
#define ST802RT1A_PHY_REG_RN11    0x11
#define ST802RT1A_PHY_REG_RN12    0x12
#define ST802RT1A_PHY_REG_RN13    0x13
#define ST802RT1A_PHY_REG_RN14    0x14
#define ST802RT1A_PHY_REG_RN18    0x18
#define ST802RT1A_PHY_REG_RN19    0x19
#define ST802RT1A_PHY_REG_RN1B    0x1B
#define ST802RT1A_PHY_REG_RN1C    0x1C
#define ST802RT1A_PHY_REG_RN1E    0x1E
#define ST802RT1A_PHY_REG_RN1F    0x1F
#define ST802RT1A_PHY_REG_RS1B    0x1B

//RN00 register
#define RN00_SOFT_RESET           (1 << 15)
#define RN00_LOCAL_LOOPBACK       (1 << 14)
#define RN00_SPEED_SEL            (1 << 13)
#define RN00_AN_EN                (1 << 12)
#define RN00_POWER_DOWN           (1 << 11)
#define RN00_ISOLATE              (1 << 10)
#define RN00_RESTART_AN           (1 << 9)
#define RN00_DUPLEX_MODE          (1 << 8)
#define RN00_COL_TEST             (1 << 7)

//RN01 register
#define RN01_100BT4               (1 << 15)
#define RN01_100BTX_FD            (1 << 14)
#define RN01_100BTX               (1 << 13)
#define RN01_10BT_FD              (1 << 12)
#define RN01_10BT                 (1 << 11)
#define RN01_NO_PREAMBLE          (1 << 6)
#define RN01_AN_COMPLETE          (1 << 5)
#define RN01_REMOTE_FAULT         (1 << 4)
#define RN01_AN_ABLE              (1 << 3)
#define RN01_LINK_STATUS          (1 << 2)
#define RN01_JABBER_DETECT        (1 << 1)
#define RN01_EXTENDED_CAP         (1 << 0)

//RN04 register
#define RN04_NP                   (1 << 15)
#define RN04_RF                   (1 << 13)
#define RN04_ASYM_PAUSE           (1 << 11)
#define RN04_PAUSE                (1 << 10)
#define RN04_100BT4               (1 << 9)
#define RN04_100BTX_FD            (1 << 8)
#define RN04100BTX                (1 << 7)
#define RN04_10BT_FD              (1 << 6)
#define RN04_10BT                 (1 << 5)
#define RN04_SELECTOR4            (1 << 4)
#define RN04_SELECTOR3            (1 << 3)
#define RN04_SELECTOR2            (1 << 2)
#define RN04_SELECTOR1            (1 << 1)
#define RN04_SELECTOR0            (1 << 0)

//RN05 register
#define RN05_NP                   (1 << 15)
#define RN05_ACK                  (1 << 14)
#define RN05_RF                   (1 << 13)
#define RN05_ASYM_PAUSE           (1 << 11)
#define RN05_PAUSE                (1 << 10)
#define RN05_100BT4               (1 << 9)
#define RN05_100BTX_FD            (1 << 8)
#define RN05_100BTX               (1 << 7)
#define RN05_10BT_FD              (1 << 6)
#define RN05_10BT                 (1 << 5)
#define RN05_SELECTOR4            (1 << 4)
#define RN05_SELECTOR3            (1 << 3)
#define RN05_SELECTOR2            (1 << 2)
#define RN05_SELECTOR1            (1 << 1)
#define RN05_SELECTOR0            (1 << 0)

//RN06 register
#define RN06_PD_FAULT             (1 << 4)
#define RN06_LP_NP_ABLE           (1 << 3)
#define RN06_NP_ABLE              (1 << 2)
#define RN06_PAGE_RCVD            (1 << 1)
#define RN06_LP_AN_ABLE           (1 << 0)

//RN07 register
#define RN07_NP                   (1 << 15)
#define RN07_MP                   (1 << 13)
#define RN07_ACK2                 (1 << 12)
#define RN07_TOGGLE               (1 << 11)
#define RN07_CODE10               (1 << 10)
#define RN07_CODE9                (1 << 9)
#define RN07_CODE8                (1 << 8)
#define RN07_CODE7                (1 << 7)
#define RN07_CODE6                (1 << 6)
#define RN07_CODE5                (1 << 5)
#define RN07_CODE4                (1 << 4)
#define RN07_CODE3                (1 << 3)
#define RN07_CODE2                (1 << 2)
#define RN07_CODE1                (1 << 1)
#define RN07_CODE0                (1 << 0)

//RN08 register
#define RN08_NP                   (1 << 15)
#define RN08_ACK                  (1 << 14)
#define RN08_MP                   (1 << 13)
#define RN08_ACK2                 (1 << 12)
#define RN08_TOGGLE               (1 << 11)
#define RN08_CODE10               (1 << 10)
#define RN08_CODE9                (1 << 9)
#define RN08_CODE8                (1 << 8)
#define RN08_CODE7                (1 << 7)
#define RN08_CODE6                (1 << 6)
#define RN08_CODE5                (1 << 5)
#define RN08_CODE4                (1 << 4)
#define RN08_CODE3                (1 << 3)
#define RN08_CODE2                (1 << 2)
#define RN08_CODE1                (1 << 1)
#define RN08_CODE0                (1 << 0)

//RN10 register
#define RN10_MII_EN               (1 << 9)
#define RN10_FEF_EN               (1 << 5)
#define RN10_FIFO_EXT             (1 << 2)
#define RN10_RMII_OOBS            (1 << 1)

//RN11 register
#define RN11_FX_MODE              (1 << 10)
#define RN11_SPEED                (1 << 9)
#define RN11_DUPLEX               (1 << 8)
#define RN11_PAUSE                (1 << 7)
#define RN11_AN_COMPLETE_INT      (1 << 6)
#define RN11_REMOTE_FAULT_INT     (1 << 5)
#define RN11_LINK_DOWN_INT        (1 << 4)
#define RN11_AN_LCW_RCVD_INT      (1 << 3)
#define RN11_PD_FAULT_INT         (1 << 2)
#define RN11_PG_RCVD_INT          (1 << 1)
#define RN11_RX_FUL_INT           (1 << 0)

//RN12 register
#define RN12_INT_OE_N             (1 << 8)
#define RN12_INT_EN               (1 << 7)
#define RN12_AN_COMPLETE_EN       (1 << 6)
#define RN12_REMOTE_FAULT_EN      (1 << 5)
#define RN12_LINK_DOWN_EN         (1 << 4)
#define RN12_AN_LCW_RCVD_EN       (1 << 3)
#define RN12_PD_FAULT_EN          (1 << 2)
#define RN12_PG_RCVD_EN           (1 << 1)
#define RN12_RX_FULL_EN           (1 << 0)

//RN13 register
#define RN13_RX_ERR_COUNTER_DIS   (1 << 13)
#define RN13_AN_COMPLETE          (1 << 12)
#define RN13_DC_REST_EN           (1 << 8)
#define RN13_NRZ_CONV_EN          (1 << 7)
#define RN13_TX_ISOLATE           (1 << 5)
#define RN13_CMODE2               (1 << 4)
#define RN13_CMODE1               (1 << 3)
#define RN13_CMODE0               (1 << 2)
#define RN13_MLT3_DIS             (1 << 1)
#define RN13_SCRAMBLER_DIS        (1 << 0)

#define RN13_CMODE_MASK           (7 << 2)
#define RN13_CMODE_AN             (0 << 2)
#define RN13_CMODE_10BT           (1 << 2)
#define RN13_CMODE_100BTX         (2 << 2)
#define RN13_CMODE_10BT_FD        (5 << 2)
#define RN13_CMODE_100BTX_FD      (6 << 2)
#define RN13_CMODE_TX_ISOLATE     (7 << 2)

//RN14 register
#define RN14_PHY_ADDR4            (1 << 7)
#define RN14_PHY_ADDR3            (1 << 6)
#define RN14_PHY_ADDR2            (1 << 5)
#define RN14_PHY_ADDR1            (1 << 4)
#define RN14_PHY_ADDR0            (1 << 3)
#define RN14_NO_PREAMBLE          (1 << 1)

//RN18 register
#define RN18_JABBER_DIS           (1 << 15)
#define RN18_MDIO_PS              (1 << 4)

//RN19 register
#define RN19_AN_COMPLETE          (1 << 15)
#define RN19_AN_ACK               (1 << 14)
#define RN19_AN_DETECT            (1 << 13)
#define RN19_LP_AN_ABLE_DETECT    (1 << 12)
#define RN19_AN_PAUSE             (1 << 11)
#define RN19_AN_HCD2              (1 << 10)
#define RN19_AN_HCD1              (1 << 9)
#define RN19_AN_HCD0              (1 << 8)
#define RN19_PD_FAULT             (1 << 7)
#define RN19_REMOTE_FAULT         (1 << 6)
#define RN19_PAGE_RCVD            (1 << 5)
#define RN19_LP_AN_ABLE           (1 << 4)
#define RN19_SP100                (1 << 3)
#define RN19_LINK_STATUS          (1 << 2)
#define RN19_AN_EN                (1 << 1)
#define RN19_JABBER_DETECT        (1 << 0)

//RN1B register
#define RN1B_LED_MODE             (1 << 9)
#define RN1B_10BT_ECHO_DIS        (1 << 7)
#define RN1B_MI_SQE_DIS           (1 << 3)

//RN1C register
#define RN1C_MDIX_STATUS          (1 << 13)
#define RN1C_MDIX_SWAP            (1 << 12)
#define RN1C_MDIX_DIS             (1 << 11)
#define RN1C_JABBER_DETECT        (1 << 9)
#define RN1C_POLARITY_CHANGED     (1 << 8)

//RN1E
#define RN1E_HCD_100BTX_FD        (1 << 15)
#define RN1E_HCD_100BT4           (1 << 14)
#define RN1E_HCD_100BTX           (1 << 13)
#define RN1E_HCD_10BT_FD          (1 << 12)
#define RN1E_HCD_10BT             (1 << 11)
#define RN1E_AN_RESTART           (1 << 8)
#define RN1E_AN_COMPLETE          (1 << 7)
#define RN1E_AN_ACK_COMPLETE      (1 << 6)
#define RN1E_AN_ACK               (1 << 5)
#define RN1E_AN_ABLE              (1 << 4)
#define RN1E_SUPER_ISOLATE        (1 << 3)

//RN1F register
#define RN1F_SHADOW_REG_EN        (1 << 7)

//RS1B
#define RS1B_MLT3_DETECT          (1 << 15)
#define RS1B_TX_CABLE_LEN2        (1 << 14)
#define RS1B_TX_CABLE_LEN1        (1 << 13)
#define RS1B_TX_CABLE_LEN0        (1 << 12)
#define RS1B_LED_TEST_CTRL        (1 << 10)
#define RS1B_DESCRAMBLER_LOCKED   (1 << 9)
#define RS1B_FALSE_CARRIER_DETECT (1 << 8)
#define RS1B_BAD_ESD_DETECT       (1 << 7)
#define RS1B_RX_ERROR_DETECT      (1 << 6)
#define RS1B_LOCK_ERROR_DETECT    (1 << 4)
#define RS1B_MLT3_ERROR_DETECT    (1 << 3)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//ST802RT1A Ethernet PHY driver
extern const PhyDriver st802rt1aPhyDriver;

//ST802RT1A related functions
error_t st802rt1aInit(NetInterface *interface);

void st802rt1aTick(NetInterface *interface);

void st802rt1aEnableIrq(NetInterface *interface);
void st802rt1aDisableIrq(NetInterface *interface);

void st802rt1aEventHandler(NetInterface *interface);

void st802rt1aWritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t st802rt1aReadPhyReg(NetInterface *interface, uint8_t address);

void st802rt1aDumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
