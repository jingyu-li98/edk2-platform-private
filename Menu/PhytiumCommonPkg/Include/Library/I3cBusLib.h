/** @file
  This file implements basic functions of using I3C bus communication.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef I3C_BUS_LIB_H_
#define I3C_BUS_LIB_H_

#define CMD0_FIFO_RNW    1

/*I3C CCC CMD code*/
#define ENEC             0x00
#define DISEC            0x01
#define SETAASA          0x29
#define ENTDAA           0x07
#define RSTDAA           0x06
#define SETHID           0x61
#define DISEC            0x01

#define WAIT_MS_TIMEOUT  1000
#define ERROR            1
#define I3C_MODE         1
#define I2C_MODE         0
#define DISABLE          0x01
#define ENABLE           0x00
#define NVMOFFSET        0x80

#define CCC_BROADCAST_MODE  0x0
#define CCC_DIRECT_MODE     0x80

#define RX_FIFO_MAX_8_BIT  124
#define TX_FIFO_MAX_8_BIT  124
#define RX_FIFO_MAX_32_BIT  31
#define TX_FIFO_MAX_32_BIT  31

#define I3C_CMD_FIFO_MAX    256
typedef enum {
  I3cBusModePure            = 0x0, //Pure Bus Mode
  I3cBusModeInvalid         = 0x1, //Invalid Config
  I3cBusModeMixedFast       = 0x2, //Mixed Fast Bus Mode
  I3cBusModeMixedSlowLimit  = 0x3  //Mixed Slow/Limited Bus Mode
} BUS_MODE;

typedef enum {
  SubAddress8Bit  = 8,
  SubAddress16Bit = 16
} SUB_SCBA;

typedef struct {
  INT32  I2c;
  INT32  I3c;
} SCL_RATE;

typedef enum {
  XmitBurstStaticSubaddr  = 0, //Burst Transfer With Static Register SubAddress
  XmitSingleIncSubaddr    = 1, //Single Data Byte Transfers With Auto Incremented Register SubAddress
  XmitSingleStaticSubaddr = 2, //Single Data Byte Transfers Without Auto Incremented Register SubAddress
  XmitBurstWithoutSubaddr = 3  //Burst Transfer Without Register SubAddress
} XMIT_MODE;


typedef struct {
  UINT64    BusAddress;
  UINT32    SysClk;
  SCL_RATE  SclRate;
  UINT8     TransMode; ///0 I2c mode  , 1 I3c mode
  UINT8     BusMode; ///< The enumeration value from BUS_MODE.  BIT1-0  CRTL(0x10)
  UINT8     XMITMode; ///< The enumeration value from XMIT_MODE.  BIT1-0  CRTL(0x10)
  UINT8     SubSCBA;  //Sub8Bit   Sub16Bit
} I3C_BUS_INFO;


typedef struct {
  UINT32  Rnw;
  UINT16  Lenth;
  UINT8   *DataIn;
  UINT8   *DataOut;
} I3C_PRIVE_XFER;


typedef struct I3C_CMD {
  UINT32  Cmd0;
  UINT32  Cmd1;
  UINT32  TxLen;
  UINT8   *TxBuf;
  UINT32  RxLen;
  UINT8   *RxBuf;
  UINT32  Error;
} I3C_CMD;

typedef struct {
  UINT32   Number;
  I3C_CMD  Cmds[I3C_CMD_FIFO_MAX];
} I3C_XFER;


EFI_STATUS
I3cMasterPriveXfers (
  IN I3C_BUS_INFO    *Bus,
  IN UINT16           SlaveAddress,
  IN UINT16           SlaveSubAddress,
  IN I3C_PRIVE_XFER  *I3cPrivexfer,
  IN UINT32           XferNumber
  );


EFI_STATUS
I3cSendCCC (
  IN I3C_BUS_INFO  *Bus,
  IN UINT8         CCC,
  IN UINT16        SlaveAddress,
  IN UINT8         Length,
  IN UINT8         Mode,
  IN UINT8        *Buffer
  );


I3C_BUS_INFO *
I3cInit (
  IN OUT I3C_BUS_INFO  *Bus,
  IN UINT64            I3cBusAddress,
  IN UINT32            Speed,
  IN UINT8             TransMode,
  IN UINT8             XMITMode,
  IN UINT8             SubAddressBits
  );


EFI_STATUS
I3cWrite (
  IN I3C_BUS_INFO  *Bus,
  IN UINT32        SlaveAddress,
  IN UINT16        SlaveSubAddress,
  IN UINT8         *Buffer,
  IN UINT32        Length
  );


EFI_STATUS
I3cRead (
  IN I3C_BUS_INFO  *Bus,
  IN UINT32        SlaveAddress,
  IN UINT16        SlaveSubAddress,
  IN OUT UINT8     *Buffer,
  IN UINT32        Length
  );

#endif  /* I3C_BUS_LIB_H_ */
