/** @file
  This file implements basic functions of I3C bus communication.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "I3cLib.h"

/**
  Disable the i3c controller.

  @param[in]  Bus  A pointer to I3C_BUS_INFO structure.

**/
VOID
DisableI3cContrller (
  IN I3C_BUS_INFO  *Bus
  )
{
  MmioWrite32 (Bus->BusAddress + CTRL, CTRL_DEV_DIS);
}

/**
  Wait the operation finish on the bus.

  @param[in]  Bus      Address of an I3C_BUS_INFO structure.

  @retval EFI_SUCCESS  The data tansfer successfully.
  @retval EFI_TIMEOUT  Some error occurred causing a timeout.

**/
EFI_STATUS
WaitForTransferFinish (
  IN I3C_BUS_INFO  *Bus
  )
{
  UINT32  Timeout;

  Timeout = I3C_DELAY_TIMEOUT;

  while (!(MmioRead32 (Bus->BusAddress + MST_STATUS0) & MST_STATUS0_IDLE) ||
    !(MmioRead32 (Bus->BusAddress + MST_STATUS0) & MST_STATUS0_CMDD_EMP)) {
    MicroSecondDelay (I3C_DELAY);
    if (Timeout-- == 0) {
      return EFI_TIMEOUT;
    }
  }

  return EFI_SUCCESS;
}

/**
  Performs parity checks on address.

  @param[in]  CheckNumber  Address to be checked .

  @retval Value            When represent an address in binary,return 0 if they
                           have an even number of ones and 1 if they have an odd
                           number of ones.

**/
UINT32
AddressParityCheck (
  IN UINT32  CheckNumber
  )
{
  UINT32  ParityCheck;
  UINT16  Value;

  Value = 0;
  ParityCheck = 0;

  ParityCheck = CheckNumber - ((CheckNumber >> 1) & 0x55);
  ParityCheck = (ParityCheck & 0x33) + ((ParityCheck >> 2) & 0x33);

  Value = (ParityCheck + (ParityCheck >> 4)) & 0x0F;
  return Value;
}

/**
  Generate the value to be filled into the register based on the slave devices address.

  @param[in]  SlaveAddress  Address of slave devices.

  @retval DevIdRR0          The value is filled into DevIdxRR0 register.

**/
UINT32
PrepareRR0DevAddress (
  UINT32  SlaveAddress
  )
{
  UINT32  DevIdxRR0;
  DevIdxRR0 = (SlaveAddress << 1) & 0xff;

  /* RR0[7:1] = addr[6:0] */
  DevIdxRR0 |= (SlaveAddress & GENMASK (6, 0)) << 1;

  /* RR0[15:13] = addr[9:7] */
  DevIdxRR0 |= (SlaveAddress & GENMASK (9, 7)) << 6;

  /* RR0[0] = ~XOR(addr[6:0]) */
  if ((AddressParityCheck (SlaveAddress & 0x7f) & 1) == 0)
    DevIdxRR0 |= 1;

  return DevIdxRR0;
}

/**
  Fill register with correct salve address.

  @param[in]  Bus              Address of an I3C_BUS_INFO structure.
  @param[in]  SlaveAddress     Address of the slave device.

**/
VOID
I3cXferInit (
  IN I3C_BUS_INFO  *Bus,
  IN UINT16        SlaveAddress
  )
{
  UINT32  DevIDxRR0;

  DevIDxRR0 = PrepareRR0DevAddress (SlaveAddress);

  if (Bus->TransMode == I3C_MODE) {
    MmioWrite32 (Bus->BusAddress + DEV_ID_RR0 (1), (DevIDxRR0 | DEV_ID_RR0_IS_I3C));
  } else {
    MmioWrite32 (Bus->BusAddress + DEV_ID_RR0 (1), DevIDxRR0);
  }

}

/**
  Configure the control register and fill the CMD register according to the content.

  @param[in]  Bus      Address of an I3C_BUS_INFO structure.
  @param[in]  Xfer     Address of an I3C_XFER structure.

  @retval EFI_SUCCESS  The data tansfer successfully.
  @retval EFI_TIMEOUT  Some error occurred causing a timeout.

**/
EFI_STATUS
I3cXferTransfer (
  IN I3C_BUS_INFO  *Bus,
  IN I3C_XFER      *Xfer
  )
{
  EFI_STATUS  Status;
  UINT32  Ctrl;
  UINT32  Index1;
  UINT32  Index2;
  UINT32  Timeout;
  UINT32  ReadCount;
  UINT32  WriteCount;
  UINT32  RxFiFo[RX_FIFO_MAX_32_BIT];
  UINT32  TxFiFo[TX_FIFO_MAX_32_BIT];

  Ctrl    = 0;
  Index1  = 0;
  Index2  = 0;
  Timeout = I3C_DELAY_TIMEOUT;
  Status = EFI_SUCCESS;
  SetMem (TxFiFo, TX_FIFO_MAX_32_BIT, 0);

  Ctrl = Bus->BusMode;
  Ctrl |= CTRL_MST_ACK | CTRL_MCS_EN | CTRL_DEV_EN;
  MmioWrite32 (Bus->BusAddress + CTRL, Ctrl);

  for (Index1 = 0; Index1 < Xfer->Number; Index1++) {
    CopyMem (TxFiFo, Xfer->Cmds[Index1].TxBuf, Xfer->Cmds[Index1].TxLen);

    //
    //Counts the number of writes acorrding to 4-byte alignment
    //
    if (Xfer->Cmds[Index1].TxLen % 4) {
      WriteCount = (Xfer->Cmds[Index1].TxLen / 4) + 1;
    } else {
        WriteCount = Xfer->Cmds[Index1].TxLen / 4;
    }

    //
    //write data to TX_FIFO
    //
    for (Index2 = 0; Index2 < WriteCount; Index2++) {

      //
      //Determine if there is full before writing
      //
      while (MmioRead32 (Bus->BusAddress + MST_STATUS0) & MST_STATUS0_TX_FULL) {
        MicroSecondDelay (I3C_DELAY);
        if (Timeout-- == 0) {
          DEBUG((DEBUG_ERROR, "Wait TX isn't full when write timeout\n"));
          Status = EFI_TIMEOUT;
          goto Out;
        }
      }
      MmioWrite32 (Bus->BusAddress + TX_FIFO, TxFiFo[Index2]);
    }

    MmioWrite32 (Bus->BusAddress + CMD1_FIFO, Xfer->Cmds[Index1].Cmd1);
    MmioWrite32 (Bus->BusAddress + CMD0_FIFO, Xfer->Cmds[Index1].Cmd0);

    Ctrl |= CTRL_MCS;
    MmioWrite32 (Bus->BusAddress + CTRL, Ctrl);

    if (Xfer->Cmds[Index1].Cmd0 & CMD0_FIFO_RNW) {
      //
      //Determine if there is data and bus if idle before reading
      //
      while ((MmioRead32 (Bus->BusAddress + MST_STATUS0) & MST_STATUS0_RX_EMP) ||
        !(MmioRead32 (Bus->BusAddress + MST_STATUS0) & MST_STATUS0_IDLE)) {
        MicroSecondDelay (I3C_DELAY);
        if (Timeout-- == 0) {
          DEBUG((DEBUG_ERROR, "Wait RX isn't empty when read timeout\n"));
          Status = EFI_TIMEOUT;
          goto Out;
        }
      }
      //
      //Counts the number of reads acorrding to 4-byte alignment
      //
      if (Xfer->Cmds[Index1].RxLen % 4) {
        ReadCount = (Xfer->Cmds[Index1].RxLen / 4) + 1;
      } else {
          ReadCount = Xfer->Cmds[Index1].RxLen / 4;
      }
      //
      //read data from RX_FIFO
      //
      for (Index2 = 0; Index2 < ReadCount; Index2++) {
        RxFiFo[Index2] = MmioRead32 (Bus->BusAddress + RX_FIFO);
      }

      CopyMem (Xfer->Cmds[Index1].RxBuf, RxFiFo, Xfer->Cmds[Index1].RxLen);

      while (!(MmioRead32 (Bus->BusAddress + MST_STATUS0) & MST_STATUS0_RX_EMP)) {
        MmioRead32 (Bus->BusAddress + RX_FIFO);
      }
    }
  }

  Status = WaitForTransferFinish (Bus);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Wait bus IDLE after transmission time out!\n"));
    goto Out;
  }

Out:
  return Status;
}


/**
  Encapsulate the packet used for communication from the device into I3C_XFER
  structure,then start the transfer.

  @param[in]  Bus              Address of an I3C_BUS_INFO structure.
  @param[in]  SlaveAddress     Address of the device on the I3C bus.
  @param[in]  SlaveSubAddress  Sub-Address of the device on the I3C bus.
  @param[in]  I3cPrivexfer     Address of an I3cPrivexfer structure.
  @param[in]  XferNumber       The number of I3c priver xfer.

  @retval EFI_SUCCESS          The data tansfer successfully.
  @retval EFI_TIMEOUT          Some error occurred causing a timeout.

**/
EFI_STATUS
I3cMasterPriveXfers (
  IN I3C_BUS_INFO   *Bus,
  IN UINT16          SlaveAddress,
  IN UINT16          SlaveSubAddress,
  IN I3C_PRIVE_XFER  *I3cPrivexfer,
  IN UINT32          XferNumber
  )
{
  EFI_STATUS  Status;
  I3C_XFER    Xfer;
  UINT32      Index;
  UINT32      Size;

  Status = EFI_SUCCESS;
  Size = XferNumber * sizeof (I3C_CMD) + sizeof (UINT32);

  SetMem (&Xfer, Size, 0);

  I3cXferInit (Bus, SlaveAddress);

  Xfer.Number = XferNumber;
  for (Index = 0; Index < XferNumber; Index++) {
    if (I3cPrivexfer[Index].Rnw) {
      Xfer.Cmds[Index].Cmd0 |= CMD0_FIFO_RNW;
      Xfer.Cmds[Index].RxBuf = I3cPrivexfer[Index].DataIn;
      Xfer.Cmds[Index].RxLen = I3cPrivexfer[Index].Lenth;
    } else {
        Xfer.Cmds[Index].TxBuf = I3cPrivexfer[Index].DataOut;
        Xfer.Cmds[Index].TxLen = I3cPrivexfer[Index].Lenth;
    }

   if (Bus->SubSCBA == SubAddress16Bit) {
        Xfer.Cmds[Index].Cmd0 |= CMD0_FIFO_SBCA;
    }

    Xfer.Cmds[Index].Cmd0 |= CMD0_FIFO_DEV_ADDR (SlaveAddress) | CMD0_FIFO_PL_LEN (I3cPrivexfer[Index].Lenth) | CMD0_FIFO_PRIV_XMIT_MODE (Bus->XMITMode);
    Xfer.Cmds[Index].Cmd1 = SlaveSubAddress;
  }

  Status = I3cXferTransfer (Bus, &Xfer);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "I3cXferTransfer failed!\n"));
  }

  DisableI3cContrller (Bus);
  return Status;
}


/**
  Send the CCC command to slave devices.

  @param[in]  Bus           Address of an I3C_BUS_INFO structure.
  @param[in]  CCC           CCC number.
  @param[in]  SlaveAddress  Slave address when direct mode.
  @param[in]  Length        Length of data.
  @param[in]  Mode          Broadcast or direct mode.
  @param[in]  Buffer        A buffer uesd to store CCC data.

  @retval EFI_SUCCESS       The send CCC successfully.
  @retval EFI_TIMEOUT       Some error occurred causing a timeout.

**/
EFI_STATUS
I3cSendCCC (
  IN I3C_BUS_INFO  *Bus,
  IN UINT8         CCC,
  IN UINT16        SlaveAddress,
  IN UINT8         Length,
  IN UINT8         Mode,
  IN UINT8         *Buffer
  )
{
  EFI_STATUS  Status;
  I3C_XFER    *Xfer;
  UINT32      Size;

  Status = EFI_SUCCESS;
  Size = 1 * sizeof (I3C_CMD) + sizeof (UINT32);
  Xfer = NULL;

  Xfer = AllocatePages (1);
  if (Xfer == NULL) {
    DEBUG ((DEBUG_ERROR, "I3cXfer allocate failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  SetMem (Xfer, Size, 0);

  Xfer->Number = 1;
  Xfer->Cmds[0].TxBuf = Buffer;
  Xfer->Cmds[0].TxLen = Length;
  Xfer->Cmds[0].Cmd1 = CCC | Mode;
  Xfer->Cmds[0].Cmd0 = CMD0_FIFO_IS_CCC | CMD0_FIFO_DEV_ADDR (SlaveAddress) | CMD0_FIFO_PL_LEN (Length);

  Status = I3cXferTransfer (Bus, Xfer);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "send CCC errorr!\n"));
    goto Out;
  }

Out:
  FreePages (Xfer, 1);
  return Status;
}


/**
  The method of reading slave devices data through I3C bus.
  Before calling this fuction ,you need to call I3cInit() to initialize
  the configuration.

  @param[in] Bus              A pointer I3C_BUS_INFO structure.
  @param[in] SlaveAddress     Slave devices' address.
  @param[in] SlaveSubAddress  SubAddress of devices.
  @param[in, out] Buffer      A buffer uesd to store read data.
  @param[in] Length           Number of bytes to be read.

  @retval EFI_SUCCESS         The data tansfer successfully.
  @retval EFI_TIMEOUT         Some error occurred causing a timeout.

**/
EFI_STATUS
I3cRead (
  IN I3C_BUS_INFO  *Bus,
  IN UINT32        SlaveAddress,
  IN UINT16        SlaveSubAddress,
  IN OUT UINT8     *Buffer,
  IN UINT32        Length
  )
{
  I3C_PRIVE_XFER  SpdI3cPrivexfer;
  EFI_STATUS      Status;
  UINT32          SpdXferNumber;

  Status = EFI_SUCCESS;

  if (Length > RX_FIFO_MAX_8_BIT) {
    DEBUG((DEBUG_ERROR, "The number of bytes currently read is greater than the maximum tansmission!\n"));
    return EFI_INVALID_PARAMETER;
  }

  SpdXferNumber = 1;

  SetMem (&SpdI3cPrivexfer, SpdXferNumber * sizeof (I3C_PRIVE_XFER), 0);

  SpdI3cPrivexfer.Rnw = CMD0_FIFO_RNW;
  SpdI3cPrivexfer.Lenth = Length;
  SpdI3cPrivexfer.DataIn = Buffer;

  Status = I3cMasterPriveXfers (Bus, SlaveAddress, SlaveSubAddress, &SpdI3cPrivexfer, SpdXferNumber);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "I3c read data error!\n"));
    goto Out;
  }

Out:
  return Status;
}


/**
  The method of writing data to slave devices through I3C bus.
  Before calling this fuction ,you need to call I3cInit() to initialize
  the configuration.

  @param[in] Bus              A pointer I3C_BUS_INFO structure.
  @param[in] SlaveAddress     Slave devices' address.
  @param[in] SlaveSubAddress  SubAddress of devices.
  @param[in] Buffer           A buffer uesd to store writeen data.
  @param[in] Length           Number of bytes to be written.

  @retval EFI_SUCCESS         The data tansfer successfully.
  @retval EFI_TIMEOUT         Some error occurred causing a timeout.

**/
EFI_STATUS
I3cWrite (
  IN I3C_BUS_INFO  *Bus,
  IN UINT32        SlaveAddress,
  IN UINT16        SlaveSubAddress,
  IN UINT8        *Buffer,
  IN UINT32        Length
  )
{
  I3C_PRIVE_XFER  SpdI3cPrivexfer;
  UINT32          SpdXferNumber;
  EFI_STATUS      Status;
  UINT8           TxBuffer[TX_FIFO_MAX_8_BIT];

  Status = EFI_SUCCESS;

  if (Length > TX_FIFO_MAX_8_BIT) {
    DEBUG((DEBUG_ERROR, "The number of bytes currently write is greater than the maximum tansmission!\n"));
    return EFI_INVALID_PARAMETER;
  }

  SpdXferNumber = 1;

  SetMem (&SpdI3cPrivexfer, SpdXferNumber * sizeof (I3C_PRIVE_XFER), 0);
  SetMem (TxBuffer, TX_FIFO_MAX_8_BIT, 0);

  //
  // When use i3c mode to write data to devices,we should ues 00XMIT mode
  // for must devices, so we need put SlaveSubAddress in the front postsion
  // of written Buffer[].
  //
  if (Bus->TransMode == I3C_MODE) {
    if (Bus->SubSCBA == SubAddress8Bit) {
      TxBuffer[0] = SlaveSubAddress;
      CopyMem(&TxBuffer[1], Buffer, Length);
      SpdI3cPrivexfer.Lenth = (Length + 1);
    } else if (Bus->SubSCBA == SubAddress16Bit) {
        TxBuffer[0] = (SlaveSubAddress & 0xff);
        TxBuffer[1] = ((SlaveSubAddress >> 8) & 0xff);
        CopyMem(&TxBuffer[2], Buffer, Length);
        SpdI3cPrivexfer.Lenth = (Length + 2);
    }
  } else if (Bus->TransMode == I2C_MODE) {
      CopyMem(TxBuffer, Buffer, Length);
      SpdI3cPrivexfer.Lenth = Length;
  }

  SpdI3cPrivexfer.DataOut = TxBuffer;

  Status = I3cMasterPriveXfers (Bus, SlaveAddress, SlaveSubAddress, &SpdI3cPrivexfer, SpdXferNumber);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "I3c write data error!\n"));
    goto Out;
  }

Out:
  return Status;
}


/**
  Initialize and config the bus controller according to the patameters.

  @param[in, out]  Bus         A pointer to I3C_BUS_INFO structure.
  @param[in]  I3cBusAddress    I3C bus address.
  @param[in]  I3cBusSpeed      Bus frequency during communication,unit:1MHz.
  @param[in]  I3cBusTransMode  Mode of this communication,I2C_MODE or I3C_MODE.
  @param[in]  I3cBusXMITMode   XMITMode of this communication,XMIT_BURST_STATIC_SUBADDR,
                               XMIT_SINGLE_INC_SUBADDR,XMIT_SINGLE_STATIC_SUBADDR,
                               XMIT_BURST_WITHOUT_SUBADDR.
  @param[in]  SubAddressBits   Bist of subaddress to slave deives,SUBADDRESS_8Bit or
                               SUBADDRESS_16Bit.

  @retval I3C_BUS_INFO         A pointer to configured I3C_BUS_INFO structure

**/
I3C_BUS_INFO *
I3cInit (
  IN  OUT I3C_BUS_INFO  *Bus,
  IN UINT64             I3cBusAddress,
  IN UINT32             Speed,
  IN UINT8              TransMode,
  IN UINT8              XMITMode,
  IN UINT8              SubAddressBits
  )
{
  UINT32  DeviceID;
  UINT32  Prescl0;
  UINT16  SclClkI3c;
  UINT16  SclClkI2c;
  UINT32  SysClkFreq;

  DeviceID = MmioRead32 (I3cBusAddress + DEV_ID);
  if (DeviceID != DEV_ID_I3C_MASTER) {
    DEBUG ((DEBUG_INFO, "i3c master dev id:0x%x error!\n", DeviceID));
  }

  Bus->BusMode    = I3cBusModeMixedFast;
  Bus->BusAddress = I3cBusAddress;
  Bus->TransMode  = TransMode;
  Bus->XMITMode   = XMITMode;
  Bus->SubSCBA    = SubAddressBits;

  if (TransMode == I3C_MODE) {
    Bus->SclRate.I3c = Speed;
  } else {
    Bus->SclRate.I2c = Speed;
  }

  //
  // 1.Fill the work frequncy
  //
  SysClkFreq = PcdGet32 (PcdSysClkFreq);
  SclClkI3c = ((SysClkFreq / (Bus->SclRate.I3c * 4)) - 1);
  Prescl0 = PRESCL_CTRL0_I3C (SclClkI3c);
  SclClkI2c = ((SysClkFreq / (Bus->SclRate.I2c * 5)) - 1);
  Prescl0 |= PRESCL_CTRL0_I2C (SclClkI2c);
  MmioWrite32 (I3cBusAddress + PRESCL_CTRL0, Prescl0);

  //
  // 2.Mask and disable all interrupt
  //
  MmioWrite32 (I3cBusAddress + MST_IDR, 0x7FFFF);
  MmioWrite32 (I3cBusAddress + MST_IMR, 0x7FFFF);

  //
  // 3. Clean dev_active register and enable
  //
  MmioWrite32 (I3cBusAddress + DEVS_CTRL, DEVS_CTRL_DEV_CLR_ALL); /*deactivate all possiable device*/
  MmioWrite32 (I3cBusAddress + DEVS_CTRL, DEVS_CTRL_DEVS_ACTIVE_MASK); /*active all possiable device*/

  return Bus;
}
