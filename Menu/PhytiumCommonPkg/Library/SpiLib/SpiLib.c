/** @file
  Spi Base function Library

  This lib provides commands that can operate transfer base function interface.

  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpiLibInternal.h"

/**
  Enable the spi controller.

  @param[in]  SpiControllerAddress    Controller address.

**/
VOID
EnableController (
  IN UINT64  SpiControllerAddress
  )
{
  MmioWrite32(SpiControllerAddress + SPI_SSIENR, SPI_MASTER_ENABLE);
}

/**
  Disable the spi controller.

  @param[in]  SpiControllerAddress    Controller address.

**/
VOID
DisableController (
  IN UINT64  SpiControllerAddress
  )
{
  MmioWrite32(SpiControllerAddress + SPI_SSIENR, SPI_MASTER_DISABLE);
}
/**
  Wait for bus idle and tx fifo is empty.

  @param[in]  SpiMaster    A pointer to SPI_MASTER

  @retval  EFI_SUCCESS   Transfer successfully.
  @retval  EFI_TIMEOUT   Transfer failure,timeout.

**/
EFI_STATUS
SpiWaitForBb (
  IN SPI_MASTER  *SpiMaster
  )
{
  EFI_STATUS Status;
  UINT32  TimeOut;

  Status = EFI_SUCCESS;
  TimeOut = SPI_TIMEOUT;

  while((MmioRead32(SpiMaster->ControllerAddress + SPI_SR) & SR_BUSY) ||
       !(MmioRead32(SpiMaster->ControllerAddress + SPI_SR) & SR_TF_EMPT) ||
       !(MmioRead32(SpiMaster->ControllerAddress + SPI_SR) & SR_TF_NOT_FULL)){

      MicroSecondDelay(BUS_BUSY_TIMEOUT);
      if (TimeOut-- == 0) {
        DEBUG((DEBUG_ERROR,"the spi bus is not idle or tx fifo is not empty\n"));
        return EFI_TIMEOUT;
       }
    }

  return Status;
}


/**
  Wait for RX Fifo data.

  @param[in]  SpiMaster    A pointer to SPI_MASTER

  @retval  EFI_SUCCESS   Transfer successfully.
  @retval  EFI_TIMEOUT   Transfer failure,timeout.

**/
EFI_STATUS
SpiWaitForData (
  IN SPI_MASTER *SpiMaster
  )
{
  EFI_STATUS Status;
  UINT32  TimeOut;

  Status = EFI_SUCCESS;
  TimeOut = SPI_TIMEOUT;

  while(!(MmioRead32(SpiMaster->ControllerAddress + SPI_SR) & SR_RF_NOT_EMPT)){

      MicroSecondDelay(RX_TIMEOUT);
      if (TimeOut-- == 0) {
        DEBUG((DEBUG_ERROR,"the rtx fifo is empty\n"));
        return EFI_TIMEOUT;
       }
    }

  return Status;
}


/**
  Set the baud rate.

  @param[in] SpiMaster  A pointer to the SPI Slave device.
  @param[in] Speed      The speed will be set,the unit is KHz.

**/
VOID
SpiSetBaudRate (
  IN SPI_MASTER  *SpiMaster,
  IN UINT32      Speed
  )
{
  UINT16  SCKDV;

  if (Speed > SpiMaster->MaxFreq)
    Speed = SpiMaster->MaxFreq;
  /* Disable controller before writing control registers */

  /* clk_div doesn't support odd number */
  SCKDV = SpiMaster->CoreClock / Speed;
  SCKDV = (SCKDV + 1) & 0xfffe;
  MmioWrite32(SpiMaster->ControllerAddress + SPI_BAUDR, SCKDV);

  /* Enable controller after writing control registers */
}


/**
  Enable the chip select.

  @param[in] Slave  A pointer to the SPI Slave device.

**/
VOID
SpiActivateCs (
  IN SPI_DEVICE  *Slave
  )
{
  UINT32 Cs;
  UINT32 CsLevel;

  Cs = 0;
  CsLevel = 0;

  //
  //chip select SPI_SER[3:0]
  //
  Cs = (1 << Slave->Cs);
  MmioWrite32 (Slave->HostRegisterBaseAddress + SPI_SER, Cs);

  //
  //chip select SPI_CS_REG[7:4]
  //
  Cs = 1 << (Slave->Cs + 4);
  MmioWrite32 (Slave->HostRegisterBaseAddress + SPI_CS_REG, Cs);

  //
  //Output low level CS SPI_CS_REG[3:0], write 1
  //
  CsLevel = MmioRead32(Slave->HostRegisterBaseAddress + SPI_CS_REG);
  CsLevel |= (1 << Slave->Cs);
  MmioWrite32 (Slave->HostRegisterBaseAddress + SPI_CS_REG, CsLevel);

}

/**
  Disable the chip select.

  @param[in] Slave  A pointer to the SPI Slave device.

**/
VOID
SpiDeactivateCs (
  IN SPI_DEVICE *Slave
  )
{
  UINT32 CsSet;
  UINT32 CsLevelSet;

  CsSet = 0;
  CsLevelSet = 0;

  //
  //Output high level CS SPI_CS_REG[3:0],wtite 0
  //
  CsLevelSet = MmioRead32(Slave->HostRegisterBaseAddress + SPI_CS_REG);
  CsLevelSet &= ~(0x1 << Slave->Cs);
  MmioWrite32 (Slave->HostRegisterBaseAddress + SPI_CS_REG, CsLevelSet);

  //
  //cancel chip select SPI_CS_REG[7:4]
  //
  CsSet = MmioRead32(Slave->HostRegisterBaseAddress + SPI_CS_REG);
  CsSet &= ~(1 << (Slave->Cs + 4));
  MmioWrite32 (Slave->HostRegisterBaseAddress + SPI_CS_REG, CsSet);

  //
  //chip select SPI_SER[3:0]
  //
  CsSet &= ~(1 << Slave->Cs);
  MmioWrite32 (Slave->HostRegisterBaseAddress + SPI_SER, CsSet);

}

/**
  Set up the baud rate and config the control regsiter.

  @param[in] SpiMaster  A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.
  @param[in] Slave      A pointer to the SPI Slave device.

**/
VOID
SpiSetupTransfer (
  IN SPI_MASTER  *SpiMaster,
  IN SPI_DEVICE  *Slave
  )
{
  UINT32  Ctrl0;

  Ctrl0 = 0;

  DisableController(SpiMaster->ControllerAddress);

  Ctrl0 = SpiMaster->BitsPerData - 1;//DFS filed3:0

  SpiSetBaudRate (SpiMaster, Slave->XferSpeed);

  //TMode filed 9:8
  switch (SpiMaster->Transfermode) {
  case CTRLR0_TMOD_TR:
    break;
  case CTRLR0_TMOD_TO:
    Ctrl0 |= (CTRLR0_TMOD_TO << CTRLR0_TMOD_SHIFT);
    break;
  case CTRLR0_TMOD_RO:
    Ctrl0 |= (CTRLR0_TMOD_RO << CTRLR0_TMOD_SHIFT);
    break;
  case CTRLR0_TMOD_EPROMREAD:
    Ctrl0 |= (CTRLR0_TMOD_EPROMREAD << CTRLR0_TMOD_SHIFT);
    break;
  }

  switch (Slave->Mode) {
  case SpiMode0:
    break;
  case SpiMode1:
    Ctrl0 |= CTRLR0_MODE_SCPH;
    break;
  case SpiMode2:
    Ctrl0 |= CTRLR0_MODE_SCPOL;
    break;
  case SpiMode3:
    Ctrl0 |= CTRLR0_MODE_SCPOL;
    Ctrl0 |= CTRLR0_MODE_SCPH;
    break;
  }

  /*
    CFS filed 15:10
    SRL   11
    SLOVE 10
    SCPL  7
    SCPOH 6
  */
  Ctrl0 |= (CTRLR0_CFS_DAFULT << CTRLR0_CFS_SHIFT) | CTRLR0_SLVOE;

  MmioWrite32 (SpiMaster->ControllerAddress + SPI_CTRLR0, Ctrl0);

  /* Mask all interrupt*/
  MmioWrite32(SpiMaster->ControllerAddress + SPI_IMR, 0);
  MmioWrite32(SpiMaster->ControllerAddress + SPI_TXFTLR, SPI_TXFTLR_DEFAULT);
  MmioWrite32(SpiMaster->ControllerAddress + SPI_RXFTLR, SPI_RXFTLR_DEFAULT);

  EnableController(SpiMaster->ControllerAddress);
}


/**
  Return the max entries we can fill into tx fifo

  @param[in] SpiMaster  A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.

**/
UINT32
TxMax(
  IN SPI_MASTER  *SpiMaster
  )
{
  UINT32 TxLeft;
  UINT32 TxRoom;
  UINT32 RxtxGap;

  TxLeft = (SpiMaster->TxEnd - SpiMaster->Tx) / (SpiMaster->BitsPerData >> 3);
  TxRoom = SpiMaster->FifoLen - MmioRead32(SpiMaster->ControllerAddress + SPI_TXFLR);

  /*
   * Another concern is about the tx/rx mismatch, we
   * thought about using (priv->fifo_len - rxflr - txflr) as
   * one maximum value for tx, but it doesn't cover the
   * data which is out of tx/rx fifo and inside the
   * shift registers. So a control from sw point of
   * view is taken.
   */
  RxtxGap = ((SpiMaster->RxEnd - SpiMaster->Rx) - (SpiMaster->TxEnd - SpiMaster->Tx)) /
    (SpiMaster->BitsPerData >> 3);

  return MIN3(TxLeft, TxRoom, (UINT32)(SpiMaster->FifoLen - RxtxGap));
}

/**
  Return the max entries we should read out of rx fifo

  @param[in] ThSpiMasteris  A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.

**/
UINT32
RxMax(
  IN SPI_MASTER *SpiMaster
  )
{
  UINT32 RxLeft;

  RxLeft = (SpiMaster->RxEnd - SpiMaster->Rx) / (SpiMaster->BitsPerData >> 3);

  return MIN(RxLeft, MmioRead32(SpiMaster->ControllerAddress + SPI_RXFLR));
}

/**
  Write data to the Data Register

  @param[in] ThSpiMasteris  A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.

**/
VOID
WriteTxDataFifo(
  IN SPI_MASTER *SpiMaster
  )
{
  UINT32 TxFifoMax;
  UINT32 TxData;

  TxFifoMax = TxMax(SpiMaster);
  TxData = 0xFFFF;

  while (TxFifoMax--) {
  /* Set the tx word if the transfer's original "tx" is not null */
    if (SpiMaster->TxEnd - SpiMaster->Length) {
      if (SpiMaster->BitsPerData == 8)
        TxData = *(UINT8 *)(SpiMaster->Tx);
      else
        TxData = *(UINT16 *)(SpiMaster->Tx);
    }
    MmioWrite32(SpiMaster->ControllerAddress + SPI_DR, TxData);
    SpiMaster->Tx += SpiMaster->BitsPerData >> 3;
  }

}

/**
  Read data from the Data Register

  @param[in] SpiMaster  A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.

**/
VOID
ReadRxDataFifo(
  IN SPI_MASTER *SpiMaster
  )
{
  UINT32 RxFifoMax;
  UINT16 RxData;

  RxFifoMax = RxMax(SpiMaster);
  RxData = 0xFFFF;

  while (RxFifoMax--) {
    RxData = MmioRead32(SpiMaster->ControllerAddress + SPI_DR);
    /* Care about rx if the transfer's original "rx" is not null */
    if (SpiMaster->RxEnd - SpiMaster->Length) {
      if (SpiMaster->BitsPerData == 8)
        *(UINT8 *)(SpiMaster->Rx) = RxData;
      else
        *(UINT16 *)(SpiMaster->Rx) = RxData;
      }
    SpiMaster->Rx += SpiMaster->BitsPerData >> 3;
  }

}

/**
  Transfer the data by poll mode.

  @param[in] SpiMaster  A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.

  @retval EFI_SUCCESS   Transfer successfully.
  @retval EFI_TIMEOUT   Transfer failure,timeout.

**/
EFI_STATUS
EFIAPI
PollTransfer (
  IN SPI_MASTER *SpiMaster
  )
{
  UINT32 Status;

  Status = EFI_SUCCESS;

  do {

    WriteTxDataFifo(SpiMaster);

    Status = SpiWaitForData(SpiMaster);
    if (EFI_ERROR(Status)){
      return Status;
    }

    ReadRxDataFifo(SpiMaster);

    Status = SpiWaitForBb(SpiMaster);
    if (EFI_ERROR(Status)){
      return Status;
    }

  } while (SpiMaster->RxEnd > SpiMaster->Rx);

  return Status;
}
/**
  To send or obtain data from a slave device.

  Enable or disable the chip selection based an the flag.After configuring the control register and
  baud rate,begin the poll mode transfer.

  @param[in] SpiMaster  A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.
  @param[in] Slave      A pointer to the SPI Slave device.
  @param[in] Flag       A flag indicating the start at or end of a transmission.

  @retval EFI_SUCCESS   Transfer successfully.
  @retval EFI_TIMEOUT   Transfer failure,timeout.

**/
EFI_STATUS
EFIAPI
SpiBaseTransfer (
  IN  SPI_MASTER  *SpiMaster,
  IN  SPI_DEVICE  *Slave,
  IN  UINTN       Flag
  )
{
  EFI_STATUS  Status;

  SpiSetupTransfer(SpiMaster,Slave);

  if ((Flag & SPI_TRANSFER_BEGIN) != 0) {
    SpiActivateCs (Slave);
  }

  //
  // After SpiActivateCs setup controller again
  //
  SpiSetupTransfer(SpiMaster,Slave);

  Status = PollTransfer(SpiMaster);

  if (EFI_ERROR(Status)){
    FreePool (Slave);
    return Status;
  }

  if ((Flag & SPI_TRANSFER_END) != 0) {
    SpiDeactivateCs (Slave);
  }

  return EFI_SUCCESS;
}
