/**
 * @file ar8031_driver.h
 * @brief AR8031 Gigabit Ethernet PHY transceiver
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

#ifndef _AR8031_DRIVER_H
#define _AR8031_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef AR8031_PHY_ADDR
   #define AR8031_PHY_ADDR 0
#elif (AR8031_PHY_ADDR < 0 || AR8031_PHY_ADDR > 31)
   #error AR8031_PHY_ADDR parameter is not valid
#endif

//AR8031 registers
#define AR8031_PHY_REG_BMCR              0x00
#define AR8031_PHY_REG_BMSR              0x01
#define AR8031_PHY_REG_PHYIDR1           0x02
#define AR8031_PHY_REG_PHYIDR2           0x03
#define AR8031_PHY_REG_ANAR              0x04
#define AR8031_PHY_REG_ANLPAR            0x05
#define AR8031_PHY_REG_ANER              0x06
#define AR8031_PHY_REG_ANNPTR            0x07
#define AR8031_PHY_REG_LPNPAR            0x08
#define AR8031_PHY_REG_1000BT_CTRL       0x09
#define AR8031_PHY_REG_1000BT_STATUS     0x0A
#define AR8031_PHY_REG_MMD_CTRL          0x0D
#define AR8031_PHY_REG_MMD_DATA          0x0E
#define AR8031_PHY_REG_EXT_STATUS        0x0F
#define AR8031_PHY_REG_FUNCTION_CTRL     0x10
#define AR8031_PHY_REG_PHY_STATUS        0x11
#define AR8031_PHY_REG_INT_EN            0x12
#define AR8031_PHY_REG_INT_STATUS        0x13
#define AR8031_PHY_REG_SMART_SPEED       0x14
#define AR8031_PHY_REG_CDT_CTRL          0x16
#define AR8031_PHY_REG_LED_CTRL          0x18
#define AR8031_PHY_REG_MAN_LED_OVERRIDE  0x19
#define AR8031_PHY_REG_CDT_STATUS        0x1C
#define AR8031_PHY_REG_DBG_PORT          0x1D
#define AR8031_PHY_REG_DBG_PORT2         0x1E
#define AR8031_PHY_REG_CHIP_CONFIG       0x1F

//BMCR register
#define BMCR_RESET                       (1 << 15)
#define BMCR_LOOPBACK                    (1 << 14)
#define BMCR_SPEED_SEL_LSB               (1 << 13)
#define BMCR_AN_EN                       (1 << 12)
#define BMCR_POWER_DOWN                  (1 << 11)
#define BMCR_ISOLATE                     (1 << 10)
#define BMCR_RESTART_AN                  (1 << 9)
#define BMCR_DUPLEX_MODE                 (1 << 8)
#define BMCR_COL_TEST                    (1 << 7)
#define BMCR_SPEED_SEL_MSB               (1 << 6)

//BMSR register
#define BMSR_100BT4                      (1 << 15)
#define BMSR_100BTX_FD                   (1 << 14)
#define BMSR_100BTX_HD                   (1 << 13)
#define BMSR_10BT_FD                     (1 << 12)
#define BMSR_10BT_HD                     (1 << 11)
#define BMSR_100BT2_FD                   (1 << 10)
#define BMSR_100BT2_HD                   (1 << 9)
#define BMSR_EXTENDED_STATUS             (1 << 8)
#define BMSR_NO_PREAMBLE                 (1 << 6)
#define BMSR_AN_COMPLETE                 (1 << 5)
#define BMSR_REMOTE_FAULT                (1 << 4)
#define BMSR_AN_ABLE                     (1 << 3)
#define BMSR_LINK_STATUS                 (1 << 2)
#define BMSR_JABBER_DETECT               (1 << 1)
#define BMSR_EXTENDED_CAP                (1 << 0)

//ANAR register
#define ANAR_NEXT_PAGE                   (1 << 15)
#define ANAR_ACK                         (1 << 14)
#define ANAR_REMOTE_FAULT                (1 << 13)
#define ANAR_XNP_ABLE                    (1 << 12)
#define ANAR_ASYMMETRIC_PAUSE            (1 << 11)
#define ANAR_PAUSE                       (1 << 10)
#define ANAR_100BT4                      (1 << 9)
#define ANAR_100BTX_FD                   (1 << 8)
#define ANAR_100BTX_HD                   (1 << 7)
#define ANAR_10BT_FD                     (1 << 6)
#define ANAR_10BT_HD                     (1 << 5)
#define ANAR_SELECTOR4                   (1 << 4)
#define ANAR_SELECTOR3                   (1 << 3)
#define ANAR_SELECTOR2                   (1 << 2)
#define ANAR_SELECTOR1                   (1 << 1)
#define ANAR_SELECTOR0                   (1 << 0)

//ANLPAR register
#define ANLPAR_NEXT_PAGE                 (1 << 15)
#define ANLPAR_ACK                       (1 << 14)
#define ANLPAR_REMOTE_FAULT              (1 << 13)
#define ANLPAR_ASYMMETRIC_PAUSE          (1 << 11)
#define ANLPAR_PAUSE                     (1 << 10)
#define ANLPAR_100BT4                    (1 << 9)
#define ANLPAR_100BTX_FD                 (1 << 8)
#define ANLPAR_100BTX_HD                 (1 << 7)
#define ANLPAR_10BT_FD                   (1 << 6)
#define ANLPAR_10BT_HD                   (1 << 5)
#define ANLPAR_SELECTOR4                 (1 << 4)
#define ANLPAR_SELECTOR3                 (1 << 3)
#define ANLPAR_SELECTOR2                 (1 << 2)
#define ANLPAR_SELECTOR1                 (1 << 1)
#define ANLPAR_SELECTOR0                 (1 << 0)

//ANER register
#define ANER_PAR_DET_FAULT               (1 << 4)
#define ANER_LP_NEXT_PAGE_ABLE           (1 << 3)
#define ANER_NEXT_PAGE_ABLE              (1 << 2)
#define ANER_PAGE_RECEIVED               (1 << 1)
#define ANER_LP_AN_ABLE                  (1 << 0)

//ANNPTR register
#define ANNPTR_NEXT_PAGE                 (1 << 15)
#define ANNPTR_MSG_PAGE                  (1 << 13)
#define ANNPTR_ACK2                      (1 << 12)
#define ANNPTR_TOGGLE                    (1 << 11)
#define ANNPTR_MESSAGE10                 (1 << 10)
#define ANNPTR_MESSAGE9                  (1 << 9)
#define ANNPTR_MESSAGE8                  (1 << 8)
#define ANNPTR_MESSAGE7                  (1 << 7)
#define ANNPTR_MESSAGE6                  (1 << 6)
#define ANNPTR_MESSAGE5                  (1 << 5)
#define ANNPTR_MESSAGE4                  (1 << 4)
#define ANNPTR_MESSAGE3                  (1 << 3)
#define ANNPTR_MESSAGE2                  (1 << 2)
#define ANNPTR_MESSAGE1                  (1 << 1)
#define ANNPTR_MESSAGE0                  (1 << 0)

//LPNPAR register
#define LPNPAR_NEXT_PAGE                 (1 << 15)
#define LPNPAR_MSG_PAGE                  (1 << 13)
#define LPNPAR_ACK2                      (1 << 12)
#define LPNPAR_TOGGLE                    (1 << 11)
#define LPNPAR_MESSAGE10                 (1 << 10)
#define LPNPAR_MESSAGE9                  (1 << 9)
#define LPNPAR_MESSAGE8                  (1 << 8)
#define LPNPAR_MESSAGE7                  (1 << 7)
#define LPNPAR_MESSAGE6                  (1 << 6)
#define LPNPAR_MESSAGE5                  (1 << 5)
#define LPNPAR_MESSAGE4                  (1 << 4)
#define LPNPAR_MESSAGE3                  (1 << 3)
#define LPNPAR_MESSAGE2                  (1 << 2)
#define LPNPAR_MESSAGE1                  (1 << 1)
#define LPNPAR_MESSAGE0                  (1 << 0)

//1000BT_CTRL register
#define _1000BT_CTRL_TEST_MODE2          (1 << 15)
#define _1000BT_CTRL_TEST_MODE1          (1 << 14)
#define _1000BT_CTRL_TEST_MODE0          (1 << 13)
#define _1000BT_CTRL_MS_MAN_CONF_EN      (1 << 12)
#define _1000BT_CTRL_MS_MAN_CONF_VAL     (1 << 11)
#define _1000BT_CTRL_PORT_TYPE           (1 << 10)
#define _1000BT_CTRL_1000BT_FD           (1 << 9)
#define _1000BT_CTRL_1000BT_HD           (1 << 8)

//1000BT_STATUS register
#define _1000BT_STATUS_MS_CONF_FAULT     (1 << 15)
#define _1000BT_STATUS_MS_CONF_RES       (1 << 14)
#define _1000BT_STATUS_LOC_REC_STATUS    (1 << 13)
#define _1000BT_STATUS_REM_REC_STATUS    (1 << 12)
#define _1000BT_STATUS_LP_1000BT_FD      (1 << 11)
#define _1000BT_STATUS_LP_1000BT_HD      (1 << 10)
#define _1000BT_STATUS_IDLE_ERR_CTR7     (1 << 7)
#define _1000BT_STATUS_IDLE_ERR_CTR6     (1 << 6)
#define _1000BT_STATUS_IDLE_ERR_CTR5     (1 << 5)
#define _1000BT_STATUS_IDLE_ERR_CTR4     (1 << 4)
#define _1000BT_STATUS_IDLE_ERR_CTR3     (1 << 3)
#define _1000BT_STATUS_IDLE_ERR_CTR2     (1 << 2)
#define _1000BT_STATUS_IDLE_ERR_CTR1     (1 << 1)
#define _1000BT_STATUS_IDLE_ERR_CTR0     (1 << 0)

//MMD_CTRL register
#define MMD_CTRL_FUNCTION1               (1 << 15)
#define MMD_CTRL_FUNCTION0               (1 << 14)
#define MMD_CTRL_DEVAD4                  (1 << 4)
#define MMD_CTRL_DEVAD3                  (1 << 3)
#define MMD_CTRL_DEVAD2                  (1 << 2)
#define MMD_CTRL_DEVAD1                  (1 << 1)
#define MMD_CTRL_DEVAD0                  (1 << 0)

//EXT_STATUS register
#define EXT_STATUS_1000BX_FD             (1 << 15)
#define EXT_STATUS_1000BX_HD             (1 << 14)
#define EXT_STATUS_1000BT_FD             (1 << 13)
#define EXT_STATUS_1000BT_HD             (1 << 12)

//FUNCTION register
#define FUNCTION_ASSERT_CRS_ON_TX        (1 << 11)
#define FUNCTION_FORCE_LINK              (1 << 10)
#define FUNCTION_MDI_CROSSOVER_MODE1     (1 << 6)
#define FUNCTION_MDI_CROSSOVER_MODE0     (1 << 5)
#define FUNCTION_SQE_TEST                (1 << 2)
#define FUNCTION_POLARITY_REVERSAL       (1 << 1)
#define FUNCTION_DISABLE_JABBER          (1 << 0)

//PHY_STATUS register
#define PHY_STATUS_SPEED1                (1 << 15)
#define PHY_STATUS_SPEED0                (1 << 14)
#define PHY_STATUS_DUPLEX                (1 << 13)
#define PHY_STATUS_PAGE_RECEIVED         (1 << 12)
#define PHY_STATUS_SPEED_DUPLEX_RESOLVED (1 << 11)
#define PHY_STATUS_LINK                  (1 << 10)
#define PHY_STATUS_MDI_CROSSOVER_STATUS  (1 << 6)
#define PHY_STATUS_WIRESPEED_DOWNGRADE   (1 << 5)
#define PHY_STATUS_TX_PAUSE_ENABLED      (1 << 3)
#define PHY_STATUS_RX_PAUSE_ENABLED      (1 << 2)
#define PHY_STATUS_POLARITY              (1 << 1)
#define PHY_STATUS_JABBER                (1 << 0)

//Speed
#define PHY_STATUS_SPEED_MASK            (3 << 14)
#define PHY_STATUS_SPEED_10              (0 << 14)
#define PHY_STATUS_SPEED_100             (1 << 14)
#define PHY_STATUS_SPEED_1000            (2 << 14)

//INT_EN register
#define INT_EN_AN_ERROR                  (1 << 15)
#define INT_EN_SPEED_CHANGED             (1 << 14)
#define INT_EN_PAGE_RECEIVED             (1 << 12)
#define INT_EN_LINK_FAIL                 (1 << 11)
#define INT_EN_LINK_SUCCESS              (1 << 10)
#define INT_EN_FAST_LINK_DOWN1           (1 << 9)
#define INT_EN_LINK_FAIL_BX              (1 << 8)
#define INT_EN_LINK_SUCCESS_BX           (1 << 7)
#define INT_EN_FAST_LINK_DOWN0           (1 << 6)
#define INT_EN_WIRESPEED_DOWNGRADE       (1 << 5)
#define INT_EN_10MS_PTP                  (1 << 4)
#define INT_EN_RX_PTP                    (1 << 3)
#define INT_EN_TX_PTP                    (1 << 2)
#define INT_EN_POLARITY_CHANGED          (1 << 1)
#define INT_EN_WOL_PTP                   (1 << 0)

//INT_STATUS register
#define INT_STATUS_AN_ERROR              (1 << 15)
#define INT_STATUS_SPEED_CHANGED         (1 << 14)
#define INT_STATUS_PAGE_RECEIVED         (1 << 12)
#define INT_STATUS_LINK_FAIL             (1 << 11)
#define INT_STATUS_LINK_SUCCESS          (1 << 10)
#define INT_STATUS_FAST_LINK_DOWN1       (1 << 9)
#define INT_STATUS_LINK_FAIL_BX          (1 << 8)
#define INT_STATUS_LINK_SUCCESS_BX       (1 << 7)
#define INT_STATUS_FAST_LINK_DOWN0       (1 << 6)
#define INT_STATUS_WIRESPEED_DOWNGRADE   (1 << 5)
#define INT_STATUS_10MS_PTP              (1 << 4)
#define INT_STATUS_RX_PTP                (1 << 3)
#define INT_STATUS_TX_PTP                (1 << 2)
#define INT_STATUS_POLARITY_CHANGED      (1 << 1)
#define INT_STATUS_WOL_PTP               (1 << 0)

//SMART_SPEED register
#define SMART_SPEED_EN                   (1 << 5)
#define SMART_SPEED_RETRY_LIMIT2         (1 << 4)
#define SMART_SPEED_RETRY_LIMIT1         (1 << 3)
#define SMART_SPEED_RETRY_LIMIT0         (1 << 2)
#define SMART_SPEED_TIMER                (1 << 1)

//CDT_CTRL register
#define CDT_CTRL_MDI_PAIR_SELECT1        (1 << 9)
#define CDT_CTRL_MDI_PAIR_SELECT0        (1 << 8)
#define CDT_CTRL_ENABLE_TEST             (1 << 0)

//LED_CTRL register
#define LED_CTRL_DISABLE_LED             (1 << 15)
#define LED_CTRL_LED_ON_TIME2            (1 << 14)
#define LED_CTRL_LED_ON_TIME1            (1 << 13)
#define LED_CTRL_LED_ON_TIME0            (1 << 12)
#define LED_CTRL_LED_OFF_TIME2           (1 << 10)
#define LED_CTRL_LED_OFF_TIME1           (1 << 9)
#define LED_CTRL_LED_OFF_TIME0           (1 << 8)
#define LED_CTRL_LED_LINK_CTRL1          (1 << 4)
#define LED_CTRL_LED_LINK_CTRL0          (1 << 3)
#define LED_CTRL_LED_ACT_CTRL            (1 << 1)

//MAN_LED_OVERRIDE register
#define MAN_LED_OVERRIDE_LED_ACT_CTRL    (1 << 12)
#define MAN_LED_OVERRIDE_LED_LINK_CTRL1  (1 << 7)
#define MAN_LED_OVERRIDE_LED_LINK_CTRL0  (1 << 6)
#define MAN_LED_OVERRIDE_LED_RX_CTRL1    (1 << 3)
#define MAN_LED_OVERRIDE_LED_RX_CTRL0    (1 << 2)
#define MAN_LED_OVERRIDE_LED_TX_CTRL1    (1 << 1)
#define MAN_LED_OVERRIDE_LED_TX_CTRL0    (1 << 0)

//CDT_STATUS register
#define CDT_STATUS_STATUS1               (1 << 9)
#define CDT_STATUS_STATUS0               (1 << 8)
#define CDT_STATUS_DELTA_TIME7           (1 << 7)
#define CDT_STATUS_DELTA_TIME6           (1 << 6)
#define CDT_STATUS_DELTA_TIME5           (1 << 5)
#define CDT_STATUS_DELTA_TIME4           (1 << 4)
#define CDT_STATUS_DELTA_TIME3           (1 << 3)
#define CDT_STATUS_DELTA_TIME2           (1 << 2)
#define CDT_STATUS_DELTA_TIME1           (1 << 1)
#define CDT_STATUS_DELTA_TIME0           (1 << 0)

//CHIP_CONF register
#define CHIP_CONFIG_BT_BX_REG_SEL        (1 << 15)
#define CHIP_CONFIG_SMII_IMP_50_75_AUTO  (1 << 14)
#define CHIP_CONFIG_SGMII_RXIMP_50_75    (1 << 13)
#define CHIP_CONFIG_SGMII_TXIMP_50_75    (1 << 12)
#define CHIP_CONFIG_PRIORITY_SEL         (1 << 10)
#define CHIP_CONFIG_FIBER_MODE_AUTO      (1 << 8)
#define CHIP_CONFIG_MODE_CFG_QUAL3       (1 << 7)
#define CHIP_CONFIG_MODE_CFG_QUAL2       (1 << 6)
#define CHIP_CONFIG_MODE_CFG_QUAL1       (1 << 5)
#define CHIP_CONFIG_MODE_CFG_QUAL0       (1 << 4)
#define CHIP_CONFIG_MODE_CFG3            (1 << 3)
#define CHIP_CONFIG_MODE_CFG2            (1 << 2)
#define CHIP_CONFIG_MODE_CFG1            (1 << 1)
#define CHIP_CONFIG_MODE_CFG0            (1 << 0)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//AR8031 Ethernet PHY driver
extern const PhyDriver ar8031PhyDriver;

//AR8031 related functions
error_t ar8031Init(NetInterface *interface);

void ar8031Tick(NetInterface *interface);

void ar8031EnableIrq(NetInterface *interface);
void ar8031DisableIrq(NetInterface *interface);

void ar8031EventHandler(NetInterface *interface);

void ar8031WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t ar8031ReadPhyReg(NetInterface *interface, uint8_t address);

void ar8031DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
