/**
 * @file w5100_driver.h
 * @brief WIZnet W5100 Ethernet controller
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

#ifndef _W5100_DRIVER_H
#define _W5100_DRIVER_H

//Dependencies
#include "core/nic.h"

//TX buffer size
#ifndef W5100_ETH_TX_BUFFER_SIZE
   #define W5100_ETH_TX_BUFFER_SIZE 1536
#elif (W5100_ETH_TX_BUFFER_SIZE != 1536)
   #error W5100_ETH_TX_BUFFER_SIZE parameter is not valid
#endif

//RX buffer size
#ifndef W5100_ETH_RX_BUFFER_SIZE
   #define W5100_ETH_RX_BUFFER_SIZE 1536
#elif (W5100_ETH_RX_BUFFER_SIZE != 1536)
   #error W5100_ETH_RX_BUFFER_SIZE parameter is not valid
#endif

//Control byte
#define W5100_CTRL_READ              0x0F
#define W5100_CTRL_WRITE             0xF0

//W5100 Common registers
#define W5100_MR                     0x00
#define W5100_GAR0                   0x01
#define W5100_GAR1                   0x02
#define W5100_GAR2                   0x03
#define W5100_GAR3                   0x04
#define W5100_SUBR0                  0x05
#define W5100_SUBR1                  0x06
#define W5100_SUBR2                  0x07
#define W5100_SUBR3                  0x08
#define W5100_SHAR0                  0x09
#define W5100_SHAR1                  0x0A
#define W5100_SHAR2                  0x0B
#define W5100_SHAR3                  0x0C
#define W5100_SHAR4                  0x0D
#define W5100_SHAR5                  0x0E
#define W5100_SIPR0                  0x0F
#define W5100_SIPR1                  0x10
#define W5100_SIPR2                  0x11
#define W5100_SIPR3                  0x12
#define W5100_IR                     0x15
#define W5100_IMR                    0x16
#define W5100_RTR0                   0x17
#define W5100_RTR1                   0x18
#define W5100_RCR                    0x19
#define W5100_RMSR                   0x1A
#define W5100_TMSR                   0x1B
#define W5100_PATR0                  0x1C
#define W5100_PATR1                  0x1D
#define W5100_PTIMER                 0x28
#define W5100_PMAGIC                 0x29
#define W5100_UIPR0                  0x2A
#define W5100_UIPR1                  0x2B
#define W5100_UIPR2                  0x2C
#define W5100_UIPR3                  0x2D
#define W5100_UPORT0                 0x2E
#define W5100_UPORT1                 0x2F

//W5100 Socket registers
#define W5100_S0_MR                  0x0400
#define W5100_S0_CR                  0x0401
#define W5100_S0_IR                  0x0402
#define W5100_S0_SR                  0x0403
#define W5100_S0_PORT0               0x0404
#define W5100_S0_PORT1               0x0405
#define W5100_S0_DHAR0               0x0406
#define W5100_S0_DHAR1               0x0407
#define W5100_S0_DHAR2               0x0408
#define W5100_S0_DHAR3               0x0409
#define W5100_S0_DHAR4               0x040A
#define W5100_S0_DHAR5               0x040B
#define W5100_S0_DIPR0               0x040C
#define W5100_S0_DIPR1               0x040D
#define W5100_S0_DIPR2               0x040E
#define W5100_S0_DIPR3               0x040F
#define W5100_S0_DPORT0              0x0410
#define W5100_S0_DPORT1              0x0411
#define W5100_S0_MSSR0               0x0412
#define W5100_S0_MSSR1               0x0413
#define W5100_S0_PROTO               0x0414
#define W5100_S0_TOS                 0x0415
#define W5100_S0_TTL                 0x0416
#define W5100_S0_TX_FSR0             0x0420
#define W5100_S0_TX_FSR1             0x0421
#define W5100_S0_TX_RD0              0x0422
#define W5100_S0_TX_RD1              0x0423
#define W5100_S0_TX_WR0              0x0424
#define W5100_S0_TX_WR1              0x0425
#define W5100_S0_RX_RSR0             0x0426
#define W5100_S0_RX_RSR1             0x0427
#define W5100_S0_RX_RD0              0x0428
#define W5100_S0_RX_RD1              0x0429
#define W5100_S1_MR                  0x0500
#define W5100_S1_CR                  0x0501
#define W5100_S1_IR                  0x0502
#define W5100_S1_SR                  0x0503
#define W5100_S1_PORT0               0x0504
#define W5100_S1_PORT1               0x0505
#define W5100_S1_DHAR0               0x0506
#define W5100_S1_DHAR1               0x0507
#define W5100_S1_DHAR2               0x0508
#define W5100_S1_DHAR3               0x0509
#define W5100_S1_DHAR4               0x050A
#define W5100_S1_DHAR5               0x050B
#define W5100_S1_DIPR0               0x050C
#define W5100_S1_DIPR1               0x050D
#define W5100_S1_DIPR2               0x050E
#define W5100_S1_DIPR3               0x050F
#define W5100_S1_DPORT0              0x0510
#define W5100_S1_DPORT1              0x0511
#define W5100_S1_MSSR0               0x0512
#define W5100_S1_MSSR1               0x0513
#define W5100_S1_PROTO               0x0514
#define W5100_S1_TOS                 0x0515
#define W5100_S1_TTL                 0x0516
#define W5100_S1_TX_FSR0             0x0520
#define W5100_S1_TX_FSR1             0x0521
#define W5100_S1_TX_RD0              0x0522
#define W5100_S1_TX_RD1              0x0523
#define W5100_S1_TX_WR0              0x0524
#define W5100_S1_TX_WR1              0x0525
#define W5100_S1_RX_RSR0             0x0526
#define W5100_S1_RX_RSR1             0x0527
#define W5100_S1_RX_RD0              0x0528
#define W5100_S1_RX_RD1              0x0529
#define W5100_S2_MR                  0x0600
#define W5100_S2_CR                  0x0601
#define W5100_S2_IR                  0x0602
#define W5100_S2_SR                  0x0603
#define W5100_S2_PORT0               0x0604
#define W5100_S2_PORT1               0x0605
#define W5100_S2_DHAR0               0x0606
#define W5100_S2_DHAR1               0x0607
#define W5100_S2_DHAR2               0x0608
#define W5100_S2_DHAR3               0x0609
#define W5100_S2_DHAR4               0x060A
#define W5100_S2_DHAR5               0x060B
#define W5100_S2_DIPR0               0x060C
#define W5100_S2_DIPR1               0x060D
#define W5100_S2_DIPR2               0x060E
#define W5100_S2_DIPR3               0x060F
#define W5100_S2_DPORT0              0x0610
#define W5100_S2_DPORT1              0x0611
#define W5100_S2_MSSR0               0x0612
#define W5100_S2_MSSR1               0x0613
#define W5100_S2_PROTO               0x0614
#define W5100_S2_TOS                 0x0615
#define W5100_S2_TTL                 0x0616
#define W5100_S2_TX_FSR0             0x0620
#define W5100_S2_TX_FSR1             0x0621
#define W5100_S2_TX_RD0              0x0622
#define W5100_S2_TX_RD1              0x0623
#define W5100_S2_TX_WR0              0x0624
#define W5100_S2_TX_WR1              0x0625
#define W5100_S2_RX_RSR0             0x0626
#define W5100_S2_RX_RSR1             0x0627
#define W5100_S2_RX_RD0              0x0628
#define W5100_S2_RX_RD1              0x0629
#define W5100_S3_MR                  0x0700
#define W5100_S3_CR                  0x0701
#define W5100_S3_IR                  0x0702
#define W5100_S3_SR                  0x0703
#define W5100_S3_PORT0               0x0704
#define W5100_S3_PORT1               0x0705
#define W5100_S3_DHAR0               0x0706
#define W5100_S3_DHAR1               0x0707
#define W5100_S3_DHAR2               0x0708
#define W5100_S3_DHAR3               0x0709
#define W5100_S3_DHAR4               0x070A
#define W5100_S3_DHAR5               0x070B
#define W5100_S3_DIPR0               0x070C
#define W5100_S3_DIPR1               0x070D
#define W5100_S3_DIPR2               0x070E
#define W5100_S3_DIPR3               0x070F
#define W5100_S3_DPORT0              0x0710
#define W5100_S3_DPORT1              0x0711
#define W5100_S3_MSSR0               0x0712
#define W5100_S3_MSSR1               0x0713
#define W5100_S3_PROTO               0x0714
#define W5100_S3_TOS                 0x0715
#define W5100_S3_TTL                 0x0716
#define W5100_S3_TX_FSR0             0x0720
#define W5100_S3_TX_FSR1             0x0721
#define W5100_S3_TX_RD0              0x0722
#define W5100_S3_TX_RD1              0x0723
#define W5100_S3_TX_WR0              0x0724
#define W5100_S3_TX_WR1              0x0725
#define W5100_S3_RX_RSR0             0x0726
#define W5100_S3_RX_RSR1             0x0727
#define W5100_S3_RX_RD0              0x0728
#define W5100_S3_RX_RD1              0x0729

//W5100 Socket register access macros
#define W5100_Sn_MR(n)               (0x0400 + ((n) * 0x0100))
#define W5100_Sn_CR(n)               (0x0401 + ((n) * 0x0100))
#define W5100_Sn_IR(n)               (0x0402 + ((n) * 0x0100))
#define W5100_Sn_SR(n)               (0x0403 + ((n) * 0x0100))
#define W5100_Sn_PORT0(n)            (0x0404 + ((n) * 0x0100))
#define W5100_Sn_PORT1(n)            (0x0405 + ((n) * 0x0100))
#define W5100_Sn_DHAR0(n)            (0x0406 + ((n) * 0x0100))
#define W5100_Sn_DHAR1(n)            (0x0407 + ((n) * 0x0100))
#define W5100_Sn_DHAR2(n)            (0x0408 + ((n) * 0x0100))
#define W5100_Sn_DHAR3(n)            (0x0409 + ((n) * 0x0100))
#define W5100_Sn_DHAR4(n)            (0x040A + ((n) * 0x0100))
#define W5100_Sn_DHAR5(n)            (0x040B + ((n) * 0x0100))
#define W5100_Sn_DIPR0(n)            (0x040C + ((n) * 0x0100))
#define W5100_Sn_DIPR1(n)            (0x040D + ((n) * 0x0100))
#define W5100_Sn_DIPR2(n)            (0x040E + ((n) * 0x0100))
#define W5100_Sn_DIPR3(n)            (0x040F + ((n) * 0x0100))
#define W5100_Sn_DPORT0(n)           (0x0410 + ((n) * 0x0100))
#define W5100_Sn_DPORT1(n)           (0x0411 + ((n) * 0x0100))
#define W5100_Sn_MSSR0(n)            (0x0412 + ((n) * 0x0100))
#define W5100_Sn_MSSR1(n)            (0x0413 + ((n) * 0x0100))
#define W5100_Sn_PROTO(n)            (0x0414 + ((n) * 0x0100))
#define W5100_Sn_TOS(n)              (0x0415 + ((n) * 0x0100))
#define W5100_Sn_TTL(n)              (0x0416 + ((n) * 0x0100))
#define W5100_Sn_TX_FSR0(n)          (0x0420 + ((n) * 0x0100))
#define W5100_Sn_TX_FSR1(n)          (0x0421 + ((n) * 0x0100))
#define W5100_Sn_TX_RD0(n)           (0x0422 + ((n) * 0x0100))
#define W5100_Sn_TX_RD1(n)           (0x0423 + ((n) * 0x0100))
#define W5100_Sn_TX_WR0(n)           (0x0424 + ((n) * 0x0100))
#define W5100_Sn_TX_WR1(n)           (0x0425 + ((n) * 0x0100))
#define W5100_Sn_RX_RSR0(n)          (0x0426 + ((n) * 0x0100))
#define W5100_Sn_RX_RSR1(n)          (0x0427 + ((n) * 0x0100))
#define W5100_Sn_RX_RD0(n)           (0x0428 + ((n) * 0x0100))
#define W5100_Sn_RX_RD1(n)           (0x0429 + ((n) * 0x0100))

//TX and RX buffers
#define W5100_TX_BUFFER              0x4000
#define W5100_RX_BUFFER              0x6000

//Mode register
#define W5100_MR_RST                 0x80
#define W5100_MR_PB                  0x10
#define W5100_MR_PPPOE               0x08
#define W5100_MR_AI                  0x02
#define W5100_MR_IND                 0x01

//Interrupt register
#define W5100_IR_CONFLICT            0x80
#define W5100_IR_UNREACH             0x40
#define W5100_IR_PPPOE               0x20
#define W5100_IR_S3_INT              0x08
#define W5100_IR_S2_INT              0x04
#define W5100_IR_S1_INT              0x02
#define W5100_IR_S0_INT              0x01

//Interrupt Mask register
#define W5100_IMR_IM_IR7             0x80
#define W5100_IMR_IM_IR6             0x40
#define W5100_IMR_IM_IR5             0x20
#define W5100_IMR_IM_IR3             0x08
#define W5100_IMR_IM_IR2             0x04
#define W5100_IMR_IM_IR1             0x02
#define W5100_IMR_IM_IR0             0x01

//RX Memory Size register
#define W5100_RMSR_SOCKET3           0xC0
#define W5100_RMSR_SOCKET3_1KB       0x00
#define W5100_RMSR_SOCKET3_2KB       0x40
#define W5100_RMSR_SOCKET3_4KB       0x80
#define W5100_RMSR_SOCKET3_8KB       0xC0
#define W5100_RMSR_SOCKET2           0x30
#define W5100_RMSR_SOCKET2_1KB       0x00
#define W5100_RMSR_SOCKET2_2KB       0x10
#define W5100_RMSR_SOCKET2_4KB       0x20
#define W5100_RMSR_SOCKET2_8KB       0x30
#define W5100_RMSR_SOCKET1           0x0C
#define W5100_RMSR_SOCKET1_1KB       0x00
#define W5100_RMSR_SOCKET1_2KB       0x04
#define W5100_RMSR_SOCKET1_4KB       0x08
#define W5100_RMSR_SOCKET1_8KB       0x0C
#define W5100_RMSR_SOCKET0           0x03
#define W5100_RMSR_SOCKET0_1KB       0x00
#define W5100_RMSR_SOCKET0_2KB       0x01
#define W5100_RMSR_SOCKET0_4KB       0x02
#define W5100_RMSR_SOCKET0_8KB       0x03

//TX Memory Size register
#define W5100_TMSR_SOCKET3           0xC0
#define W5100_TMSR_SOCKET3_1KB       0x00
#define W5100_TMSR_SOCKET3_2KB       0x40
#define W5100_TMSR_SOCKET3_4KB       0x80
#define W5100_TMSR_SOCKET3_8KB       0xC0
#define W5100_TMSR_SOCKET2           0x30
#define W5100_TMSR_SOCKET2_1KB       0x00
#define W5100_TMSR_SOCKET2_2KB       0x10
#define W5100_TMSR_SOCKET2_4KB       0x20
#define W5100_TMSR_SOCKET2_8KB       0x30
#define W5100_TMSR_SOCKET1           0x0C
#define W5100_TMSR_SOCKET1_1KB       0x00
#define W5100_TMSR_SOCKET1_2KB       0x04
#define W5100_TMSR_SOCKET1_4KB       0x08
#define W5100_TMSR_SOCKET1_8KB       0x0C
#define W5100_TMSR_SOCKET0           0x03
#define W5100_TMSR_SOCKET0_1KB       0x00
#define W5100_TMSR_SOCKET0_2KB       0x01
#define W5100_TMSR_SOCKET0_4KB       0x02
#define W5100_TMSR_SOCKET0_8KB       0x03

//Socket n Mode register
#define W5100_Sn_MR_MULTI            0x80
#define W5100_Sn_MR_ND               0x20
#define W5100_Sn_MR_MC               0x20
#define W5100_Sn_MR_PROTOCOL         0x0F
#define W5100_Sn_MR_PROTOCOL_CLOSED  0x00
#define W5100_Sn_MR_PROTOCOL_TCP     0x01
#define W5100_Sn_MR_PROTOCOL_UDP     0x02
#define W5100_Sn_MR_PROTOCOL_IPRAW   0x03
#define W5100_Sn_MR_PROTOCOL_MACRAW  0x04
#define W5100_Sn_MR_PROTOCOL_PPPOE   0x05

//Socket n Command register
#define W5100_Sn_CR_OPEN             0x01
#define W5100_Sn_CR_LISTEN           0x02
#define W5100_Sn_CR_CONNECT          0x04
#define W5100_Sn_CR_DISCON           0x08
#define W5100_Sn_CR_CLOSE            0x10
#define W5100_Sn_CR_SEND             0x20
#define W5100_Sn_CR_SEND_MAC         0x21
#define W5100_Sn_CR_SEND_KEEP        0x22
#define W5100_Sn_CR_RECV             0x40

//Socket n Interrupt register
#define W5100_Sn_IR_SEND_OK          0x10
#define W5100_Sn_IR_TIMEOUT          0x08
#define W5100_Sn_IR_RECV             0x04
#define W5100_Sn_IR_DISCON           0x02
#define W5100_Sn_IR_CON              0x01

//Socket n Status register
#define W5100_Sn_SR_SOCK_CLOSED      0x00
#define W5100_Sn_SR_SOCK_ARP_1       0x11
#define W5100_Sn_SR_SOCK_INIT        0x13
#define W5100_Sn_SR_SOCK_LISTEN      0x14
#define W5100_Sn_SR_SOCK_SYNSENT     0x15
#define W5100_Sn_SR_SOCK_SYNRECV     0x16
#define W5100_Sn_SR_SOCK_ESTABLISHED 0x17
#define W5100_Sn_SR_SOCK_FIN_WAIT    0x18
#define W5100_Sn_SR_SOCK_CLOSING     0x1A
#define W5100_Sn_SR_SOCK_TIME_WAIT   0x1B
#define W5100_Sn_SR_SOCK_CLOSE_WAIT  0x1C
#define W5100_Sn_SR_SOCK_LAST_ACK    0x1D
#define W5100_Sn_SR_SOCK_ARP_2       0x21
#define W5100_Sn_SR_SOCK_UDP         0x22
#define W5100_Sn_SR_SOCK_ARP_3       0x31
#define W5100_Sn_SR_SOCK_IPRAW       0x32
#define W5100_Sn_SR_SOCK_MACRAW      0x42
#define W5100_Sn_SR_SOCK_PPPOE       0x5F

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//W5100 driver
extern const NicDriver w5100Driver;

//W5100 related functions
error_t w5100Init(NetInterface *interface);
void w5100InitHook(NetInterface *interface);

void w5100Tick(NetInterface *interface);

void w5100EnableIrq(NetInterface *interface);
void w5100DisableIrq(NetInterface *interface);
bool_t w5100IrqHandler(NetInterface *interface);
void w5100EventHandler(NetInterface *interface);

error_t w5100SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary);

error_t w5100ReceivePacket(NetInterface *interface);

error_t w5100UpdateMacAddrFilter(NetInterface *interface);

void w5100WriteReg8(NetInterface *interface, uint16_t address, uint8_t data);
uint8_t w5100ReadReg8(NetInterface *interface, uint16_t address);

void w5100WriteReg16(NetInterface *interface, uint16_t address, uint16_t data);
uint16_t w5100ReadReg16(NetInterface *interface, uint16_t address);

void w5100WriteData(NetInterface *interface, const uint8_t *data,
   size_t length);

void w5100ReadData(NetInterface *interface, uint8_t *data, size_t length);

void w5100WriteBuffer(NetInterface *interface, uint16_t offset,
   const uint8_t *data, size_t length);

void w5100ReadBuffer(NetInterface *interface, uint16_t address, uint8_t *data,
   size_t length);

void w5100DumpReg(NetInterface *interface);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
