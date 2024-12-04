/** @file
  Spi Base function Library

  This lib provides commands that can operate transfer base function interface.

  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef SPI_LIB_INTERNAL_H_
#define SPI_LIB_INTERNAL_H_

#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/Bits.h>
#include <Library/TimerLib.h>
#include <Library/SpiLib.h>

#include <Protocol/SpiMaster.h>

/* Register offsets */
#define SPI_CTRLR0          0x00
#define CTRLR0_DFS_MASK         GENMASK(3, 0)
#define CTRLR0_FRF_MASK         GENMASK(5, 4)
#define CTRLR0_FRF_SPI          0x0
#define CTRLR0_FRF_SSP          0x1
#define CTRLR0_FRF_MICROWIRE    0x2
#define CTRLR0_FRF_RESV         0x3
#define CTRLR0_MODE_MASK        GENMASK(7, 6)
#define CTRLR0_MODE_SCPH        BIT(6)
#define CTRLR0_MODE_SCPOL       BIT(7)
#define CTRLR0_TMOD_MASK        GENMASK(9, 8)
#define CTRLR0_TMOD_TR          0x0     /* xmit & recv */
#define CTRLR0_TMOD_TO          0x1     /* xmit only */
#define CTRLR0_TMOD_RO          0x2     /* recv only */
#define CTRLR0_TMOD_EPROMREAD   0x3 /* eeprom read mode */
#define CTRLR0_TMOD_SHIFT       8

#define CTRLR0_SLVOE            BIT(10)
#define CTRLR0_SRL              BIT(11)
#define CTRLR0_CFS_MASK         GENMASK(15, 12)
#define CTRLR0_CFS_DAFULT       0x7
#define CTRLR0_CFS_SHIFT        12
#define CTRLR0_DFS_32_MASK      GENMASK(20, 16)
#define CTRLR0_SPI_FRF_MASK     GENMASK(22, 21)
#define CTRLR0_SPI_FRF_BYTE     0x0
#define CTRLR0_SPI_FRF_DUAL     0x1
#define CTRLR0_SPI_FRF_QUAD     0x2

#define SPI_CTRLR1          0x04  /*the value of this register is the amount of data continuously accepted whenTMOP=10 or TMOD=11 */

#define SPI_SSIENR          0x08
#define SPI_MASTER_ENABLE   0x1
#define SPI_MASTER_DISABLE  0x0

#define SPI_MWCR            0x0c
#define SPI_MWCR_MHS        BIT(2)
#define SPI_MWCR_MDD        BIT(1)
#define SPI_MWCR_MWMOD      BIT(0)

#define SPI_SER             0x10
#define SPI_BAUDR           0x14
#define SPI_TXFTLR          0x18
#define SPI_TXFTLR_DEFAULT  0xf
#define SPI_RXFTLR          0x1c
#define SPI_RXFTLR_DEFAULT  0x10
#define SPI_TXFLR           0x20
#define SPI_RXFLR           0x24

#define SPI_SR              0x28
#define SR_MASK             GENMASK(6, 0)  /* cover 7 bits */
#define SR_BUSY             BIT(0)
#define SR_TF_NOT_FULL      BIT(1)
#define SR_TF_EMPT          BIT(2)
#define SR_RF_NOT_EMPT      BIT(3)
#define SR_RF_FULL          BIT(4)
#define SR_TX_ERR           BIT(5)
#define SR_DCOL             BIT(6)

#define SPI_IMR             0x2c
#define SPI_INTERRUPT_MASK  0xff
#define SPI_ISR             0x30
#define SPI_RISR            0x34
#define SPI_TXOICR          0x38
#define SPI_RXOICR          0x3c
#define SPI_RXUICR          0x40
#define SPI_MSTICR          0x44
#define SPI_ICR             0x48
#define SPI_DMACR           0x4c
#define SPI_DMATDLR         0x50
#define SPI_DMARDLR         0x54
#define SPI_IDR             0x58
#define SPI_VERSION         0x5c
#define SPI_DR              0x60

#define SPI_CS_REG          0x100
#define SPI_CS_REG_LOW      0x0
#define SPI_CS_REG_HIGH     0x1


#define RX_TIMEOUT          1000  /* timeout in ms */
#define BUS_BUSY_TIMEOUT    1000  /* timeout in ms */


#define SPI_TIMEOUT                     100
#define CONTROLLER_SHIFT_BIT            12

#define MIN3(A,B,C)  MIN(A,MIN(B,C))

#endif
