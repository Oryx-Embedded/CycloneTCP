/**
 * @file upd60611_driver.h
 * @brief uPD60611 Ethernet PHY transceiver
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

#ifndef _UPD60611_DRIVER_H
#define _UPD60611_DRIVER_H

//Dependencies
#include "core/nic.h"

//PHY address
#ifndef UPD60611_PHY_ADDR
   #define UPD60611_PHY_ADDR 0
#elif (UPD60611_PHY_ADDR < 0 || UPD60611_PHY_ADDR > 31)
   #error UPD60611_PHY_ADDR parameter is not valid
#endif

//uPD60611 registers
#define UPD60611_PHY_REG_BMCR       0x00
#define UPD60611_PHY_REG_BMSR       0x01
#define UPD60611_PHY_REG_PHYIDR1    0x02
#define UPD60611_PHY_REG_PHYIDR2    0x03
#define UPD60611_PHY_REG_ANAR       0x04
#define UPD60611_PHY_REG_ANLPAR     0x05
#define UPD60611_PHY_REG_ANER       0x06
#define UPD60611_PHY_REG_ANNPTR     0x07
#define UPD60611_PHY_REG_SRR        0x10
#define UPD60611_PHY_REG_MCSR       0x11
#define UPD60611_PHY_REG_SMR        0x12
#define UPD60611_PHY_REG_EBSR       0x13
#define UPD60611_PHY_REG_BER        0x17
#define UPD60611_PHY_REG_FEQMR      0x18
#define UPD60611_PHY_REG_DCSR       0x19
#define UPD60611_PHY_REG_DCR        0x1A
#define UPD60611_PHY_REG_SCSIR      0x1B
#define UPD60611_PHY_REG_ISR        0x1D
#define UPD60611_PHY_REG_IER        0x1E
#define UPD60611_PHY_REG_PSCSR      0x1F

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
#define BMSR_AN_COMPLETE           (1 << 5)
#define BMSR_REMOTE_FAULT          (1 << 4)
#define BMSR_AN_ABLE               (1 << 3)
#define BMSR_LINK_STATUS           (1 << 2)
#define BMSR_JABBER_DETECT         (1 << 1)
#define BMSR_EXTENDED_CAP          (1 << 0)

//ANAR register
#define ANAR_NP                    (1 << 15)
#define ANAR_RF                    (1 << 13)
#define ANAR_PAUSE1                (1 << 11)
#define ANAR_PAUSE0                (1 << 10)
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
#define ANNPTR_TOGGLE              (1 << 11)
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

//MCSR register
#define MCSR_EDPWRDOWN             (1 << 13)
#define MCSR_FARLOOPBACK           (1 << 9)
#define MCSR_FASTEST               (1 << 8)
#define MCSR_AUTOMDIX_EN           (1 << 7)
#define MCSR_MDI MODE              (1 << 6)
#define MCSR_FORCE_GOOD_LINK       (1 << 2)
#define MCSR_ENERGYON              (1 << 1)

//SMR register
#define SMR_FX_MODE                (1 << 10)
#define SMR_PHY_MODE3              (1 << 8)
#define SMR_PHY_MODE2              (1 << 7)
#define SMR_PHY_MODE1              (1 << 6)
#define SMR_PHY_MODE0              (1 << 5)
#define SMR_PHY_ADD_DEV1           (1 << 4)
#define SMR_PHY_ADD_DEV0           (1 << 3)
#define SMR_PHY_ADD_MOD2           (1 << 2)
#define SMR_PHY_ADD_MOD1           (1 << 1)
#define SMR_PHY_ADD_MOD0           (1 << 0)

//EBSR register
#define EBSR_T_EL_BUF_OVF          (1 << 7)
#define EBSR_T_EL_BUF_UDF          (1 << 6)
#define EBSR_R_EL_BUF_OVF          (1 << 5)
#define EBSR_R_EL_BUF_UDF          (1 << 4)

//BER register
#define BER_LNK_OK                 (1 << 15)
#define BER_CNT_LNK_EN             (1 << 14)
#define BER_CNT_TRIG2              (1 << 13)
#define BER_CNT_TRIG1              (1 << 12)
#define BER_CNT_TRIG0              (1 << 11)
#define BER_WINDOW3                (1 << 10)
#define BER_WINDOW2                (1 << 9)
#define BER_WINDOW1                (1 << 8)
#define BER_WINDOW0                (1 << 7)
#define BER_COUNT6                 (1 << 6)
#define BER_COUNT5                 (1 << 5)
#define BER_COUNT4                 (1 << 4)
#define BER_COUNT3                 (1 << 3)
#define BER_COUNT2                 (1 << 2)
#define BER_COUNT1                 (1 << 1)
#define BER_COUNT0                 (1 << 0)

//DCSR register
#define DCSR_DIAG_INIT             (1 << 14)
#define DCSR_ADC_MAX_VALUE5        (1 << 13)
#define DCSR_ADC_MAX_VALUE4        (1 << 12)
#define DCSR_ADC_MAX_VALUE3        (1 << 11)
#define DCSR_ADC_MAX_VALUE2        (1 << 10)
#define DCSR_ADC_MAX_VALUE1        (1 << 9)
#define DCSR_ADC_MAX_VALUE0        (1 << 8)
#define DCSR_DIAG_DONE             (1 << 7)
#define DCSR_DIAG_POL              (1 << 6)
#define DCSR_DIAG_SEL_LINE         (1 << 5)
#define DCSR_PW_DIAG4              (1 << 4)
#define DCSR_PW_DIAG3              (1 << 3)
#define DCSR_PW_DIAG2              (1 << 2)
#define DCSR_PW_DIAG1              (1 << 1)
#define DCSR_PW_DIAG0              (1 << 0)

//DCR register
#define DCR_CNT_WINDOW7            (1 << 15)
#define DCR_CNT_WINDOW6            (1 << 14)
#define DCR_CNT_WINDOW5            (1 << 13)
#define DCR_CNT_WINDOW4            (1 << 12)
#define DCR_CNT_WINDOW3            (1 << 11)
#define DCR_CNT_WINDOW2            (1 << 10)
#define DCR_CNT_WINDOW1            (1 << 9)
#define DCR_CNT_WINDOW0            (1 << 8)
#define DCR_DIAGCNT7               (1 << 7)
#define DCR_DIAGCNT6               (1 << 6)
#define DCR_DIAGCNT5               (1 << 5)
#define DCR_DIAGCNT4               (1 << 4)
#define DCR_DIAGCNT3               (1 << 3)
#define DCR_DIAGCNT2               (1 << 2)
#define DCR_DIAGCNT1               (1 << 1)
#define DCR_DIAGCNT0               (1 << 0)

//SCSIR register
#define SCSIR_SWRST_FAST           (1 << 12)
#define SCSIR_SQEOFF               (1 << 11)
#define SCSIR_FEFIEN               (1 << 5)
#define SCSIR_XPOL                 (1 << 4)

//ISR register
#define ISR_BER                    (1 << 10)
#define ISR_FEQ                    (1 << 9)
#define ISR_ENERGYON               (1 << 7)
#define ISR_AN_COMPLETE            (1 << 6)
#define ISR_REMOTE_FAULT           (1 << 5)
#define ISR_LINK_DOWN              (1 << 4)
#define ISR_AN_LP_ACK              (1 << 3)
#define ISR_PD_FAULT               (1 << 2)
#define ISR_AN_PAGE_RECEIVED       (1 << 1)

//IER register
#define IER_BER                    (1 << 10)
#define IER_FEQ                    (1 << 9)
#define IER_ENERGYON               (1 << 7)
#define IER_AN_COMPLETE            (1 << 6)
#define IER_REMOTE_FAULT           (1 << 5)
#define IER_LINK_DOWN              (1 << 4)
#define IER_AN_LP_ACK              (1 << 3)
#define IER_PD_FAULT               (1 << 2)
#define IER_AN_PAGE_RECEIVED       (1 << 1)

//PSCSR register
#define PSCSR_AUTODONE             (1 << 12)
#define PSCSR_ENABLE_4B5B          (1 << 6)
#define PSCSR_HCDSPEED2            (1 << 4)
#define PSCSR_HCDSPEED1            (1 << 3)
#define PSCSR_HCDSPEED0            (1 << 2)
#define PSCSR_RX_DV_J2T            (1 << 1)
#define PSCSR_SCRAMBLE_DIS         (1 << 0)

//Speed indication
#define PSCSR_HCDSPEED_MASK        (7 << 2)
#define PSCSR_HCDSPEED_10BT        (1 << 2)
#define PSCSR_HCDSPEED_100BTX      (2 << 2)
#define PSCSR_HCDSPEED_10BT_FD     (5 << 2)
#define PSCSR_HCDSPEED_100BTX_FD   (6 << 2)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//uPD60611 Ethernet PHY driver
extern const PhyDriver upd60611PhyDriver;

//uPD60611 related functions
error_t upd60611Init(NetInterface *interface);

void upd60611Tick(NetInterface *interface);

void upd60611EnableIrq(NetInterface *interface);
void upd60611DisableIrq(NetInterface *interface);

void upd60611EventHandler(NetInterface *interface);

void upd60611WritePhyReg(NetInterface *interface, uint8_t address, uint16_t data);
uint16_t upd60611ReadPhyReg(NetInterface *interface, uint8_t address);

void upd60611DumpPhyReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
