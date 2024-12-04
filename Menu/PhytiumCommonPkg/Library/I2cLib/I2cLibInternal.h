/** @file
  I2C information and resource.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef I2C_LIB_INTRENAL_H_
#define I2C_LIB_INTRENAL_H_

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/I2cLib.h>

/*
 * Registers offset
 */
#define  IC_CON              0x0 /* i2c control register */
#define  IC_CON_SD              0x0040
#define  IC_CON_RE              0x0020
#define  IC_CON_10BITADDRMASTER 0x0010
#define  IC_CON_10BITADDR_SLAVE 0x0008
#define  IC_CON_SPD_MSK         0x0006
#define  IC_CON_SPD_SS          0x0002
#define  IC_CON_SPD_FS          0x0004
#define  IC_CON_SPD_HS          0x0006
#define  IC_CON_MM              0x0001
#define  IC_TAR              0x4 /* i2c target address register */
#define  IC_SAR              0x8 /* i2c slave address register */
#define  IC_DATA_CMD         0x10  /* i2c data buffer and command register */
#define  IC_CMD                 0x0100
#define  IC_STOP                0x0200
#define  IC_SS_SCL_HCNT      0x14
#define  IC_SS_SCL_LCNT      0x18
#define  IC_FS_SCL_HCNT      0x1c
#define  IC_FS_SCL_LCNT      0x20
#define  IC_HS_SCL_HCNT      0x24
#define  IC_HS_SCL_LCNT      0x28
#define  IC_INTR_STAT        0x2c  /* i2c interrupt status register */
#define  IC_GEN_CALL            0x0800
#define  IC_START_DET           0x0400
#define  IC_STOP_DET            0x0200
#define  IC_ACTIVITY            0x0100
#define  IC_RX_DONE             0x0080
#define  IC_TX_ABRT             0x0040
#define  IC_RD_REQ              0x0020
#define  IC_TX_EMPTY            0x0010
#define  IC_TX_OVER             0x0008
#define  IC_RX_FULL             0x0004
#define  IC_RX_OVER             0x0002
#define  IC_RX_UNDER            0x0001
#define  IC_INTR_MASK        0x30
#define  IC_RAW_INTR_STAT    0x34
#define  IC_RX_TL            0x38  /* fifo receive threshold register */
#define  IC_TX_TL            0x3c  /* fifo transmit threshold register */
#define  IC_TL0                 0x00
#define  IC_TL1                 0x01
#define  IC_TL2                 0x02
#define  IC_TL3                 0x03
#define  IC_TL4                 0x04
#define  IC_TL5                 0x05
#define  IC_TL6                 0x06
#define  IC_TL7                 0x07
#define  IC_CLR_INTR         0x40
#define  IC_CLR_RX_UNDER     0x44
#define  IC_CLR_RX_OVER      0x48
#define  IC_CLR_TX_OVER      0x4c
#define  IC_CLR_RD_REQ       0x50
#define  IC_CLR_TX_ABRT      0x54
#define  IC_CLR_RX_DONE      0x58
#define  IC_CLR_ACTIVITY     0x5c
#define  IC_CLR_STOP_DET     0x60
#define  IC_CLR_START_DET    0x64
#define  IC_CLR_GEN_CALL     0x68
#define  IC_ENABLE           0x6c  /* i2c enable register */
#define  IC_ENABLE_0B           0x0001
#define  IC_STATUS           0x70  /* i2c status register */
#define  IC_STATUS_SA           0x0040
#define  IC_STATUS_MA           0x0020
#define  IC_STATUS_RFF          0x0010
#define  IC_STATUS_RFNE         0x0008
#define  IC_STATUS_TFE          0x0004
#define  IC_STATUS_TFNF         0x0002
#define  IC_STATUS_ACT          0x0001
#define  IC_TXFLR            0x74
#define  IC_RXFLR            0x78
#define  IC_SDA_HOLD         0x7c
#define  IC_TX_ABRT_SOURCE   0x80
#define  IC_ENABLE_STATUS    0x9c
#define  IC_FS_SPKLEN        0xa0
#define  IC_HS_SPKLEN        0xa4
#define  IC_COMP_PARAM_1     0xf4
#define  IC_COMP_VERSION     0xf8
#define  IC_COMP_TYPE        0xfc

/* Worst case timeout for 1 byte is kept as 2ms */
#define I2C_BYTE_TIMEOUT       (2)
#define I2C_STOPDET_TIMEOUT    (2)
#define I2C_BYTE_TIMEOUT_BUSY  (I2C_BYTE_TIMEOUT * 16)
/* wait time uint 1us,it's a reference value and
can be changed as required*/
#define I2C_WAIT_TIME          1000
#define I2C_FLUSH_WAIT_TIME    10
#define I2C_WRITE_WAIT_TIME    100000


/* Speed Selection */
#define I2C_MAX_SPEED          3400000
#define I2C_FAST_SPEED         400000
#define I2C_STANDARD_SPEED     100000

#endif /* I2C_LIB_INTRENAL_H_ */
