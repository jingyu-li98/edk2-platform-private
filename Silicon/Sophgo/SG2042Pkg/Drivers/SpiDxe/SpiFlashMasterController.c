/** @file
 *  SPI Flash Master Controller (SPIFMC)
 *
 *  Copyright (c) 2023, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include "SpiFlashMasterController.h"
#include <Library/DebugLib.h>

STATIC
EFI_STATUS
SpifmcWaitInt (
  IN UINT32  SpiBase,
  IN UINT8   IntType
  )
{
  UINT32 Stat;

  while (1) {
    Stat = MmioRead32 ((UINTN)(SpiBase + SPIFMC_INT_STS));
    if (Stat & IntType) {
      return EFI_SUCCESS;
    }
  }

  return EFI_TIMEOUT;
}

STATIC
UINT32
SpifmcInitReg (
  UINT32 SpiBase
  )
{
  UINT32 Reg;

  Reg = MmioRead32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR));
  Reg &= ~(SPIFMC_TRAN_CSR_TRAN_MODE_MASK
           | SPIFMC_TRAN_CSR_CNTNS_READ
           | SPIFMC_TRAN_CSR_FAST_MODE
           | SPIFMC_TRAN_CSR_BUS_WIDTH_2_BIT
           | SPIFMC_TRAN_CSR_BUS_WIDTH_4_BIT
           | SPIFMC_TRAN_CSR_DMA_EN
           | SPIFMC_TRAN_CSR_ADDR_BYTES_MASK
           | SPIFMC_TRAN_CSR_WITH_CMD
           | SPIFMC_TRAN_CSR_FIFO_TRG_LVL_MASK);

  return Reg;
}

/*
 * SpifmcReadRegister is a workaround function:
 * AHB bus could only do 32-bit access to SPIFMC fifo,
 * so cmd without 3-byte addr will leave 3-byte data in fifo.
 * Set TX to mark that these 3-byte data would be sent out.
 */
EFI_STATUS
EFIAPI
SpifmcReadRegister (
  IN  SPI_NOR *Nor,
  IN  UINT8   Opcode,
  IN  UINTN   Length
  OUT UINT8   *Buffer,
  )
{
  UINTN      Index;
  UINT32     Reg;
  UINT32     SpiBase;
  EFI_STATUS Status;

  SpiBase = Nor->SpiBase;

  Reg = SpifmcInitReg (SpiBase);
  Reg |= SPIFMC_TRAN_CSR_BUS_WIDTH_1_BIT;
  Reg |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_1_BYTE;
  Reg |= SPIFMC_TRAN_CSR_WITH_CMD;
  Reg |= SPIFMC_TRAN_CSR_TRAN_MODE_RX | SPIFMC_TRAN_CSR_TRAN_MODE_TX;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);
  MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Opcode);

  for (Index = 0; Index < Length; Index++) {
    MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), 0);
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_INT_STS), 0);
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_NUM), Length);
  Reg |= SPIFMC_TRAN_CSR_GO_BUSY;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Reg);

  Status = SpifmcWaitInt (SpiBase, SPIFMC_INT_TRAN_DONE);
  if (Status) {
    return Status;
  }

  while (Length--) {
    *Buffer++ = MmioRead8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT));
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpifmcWriteRegister (
  IN SPI_NOR      *Nor,
  IN UINT8        Opcode,
  IN CONST UINT8 *Buffer,
  IN UINTN        Length
  )
{
  UINTN      Index;
  UINT32     Reg;
  UINT32     SpiBase;

  SpiBase = Nor->SpiBase;

  Reg = SpifmcInitReg (SpiBase);
  Reg |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_1_BYTE;
  Reg |= SPIFMC_TRAN_CSR_WITH_CMD;

  /*
   * If write values to the Status Register,
   * configure TRAN_CSR register as the same as SpifmcReadReg.
   */
  if (Opcode == SPINOR_OP_WRSR) {
    Reg |= SPIFMC_TRAN_CSR_TRAN_MODE_RX | SPIFMC_TRAN_CSR_TRAN_MODE_TX;
    MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_NUM), Length);
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);
  MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Opcode);

  for (Index = 0; Index < Length; Index++) {
    MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Buffer[Index]);
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_INT_STS), 0);
  Reg |= SPIFMC_TRAN_CSR_GO_BUSY;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Reg);
  SpifmcWaitInt (SpiBase, SPIFMC_INT_TRAN_DONE);
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpifmcRead (
  IN  SPI_NOR *Nor,
  IN  UINT32  From,
  IN  UINTN   Length,
  OUT UINT8   *Buffer
  )
{

  INT32      XferSize;
  INT32      Offset;
  UINTN      Index;
  UINT32     Reg;
  UINT32     SpiBase;

  SpiBase = Nor->SpiBase;
  Offset = 0;

  Reg = SpifmcInitReg (SpiBase);
  Reg |= (Nor->AddrNbytes) << SPIFMC_TRAN_CSR_ADDR_BYTES_SHIFT;
  Reg |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_8_BYTE;
  Reg |= SPIFMC_TRAN_CSR_WITH_CMD;
  Reg |= SPIFMC_TRAN_CSR_TRAN_MODE_RX;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);
  MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Nor->ReadOpcode);

  for (Index = Nor->AddrNbytes - 1; Index >= 0; Index--) {
    MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), (From >> Index * 8) & 0xff);
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_INT_STS), 0);
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_NUM), Length);
  Reg |= SPIFMC_TRAN_CSR_GO_BUSY;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Reg);
  SpifmcWaitInt (SpiBase, SPIFMC_INT_RD_FIFO);

  while (Offset < Length) {
    XferSize = MIN (SPIFMC_MAX_FIFO_DEPTH, Length - Offset);

    while ((MmioRead32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT)) & 0xf) != XferSize)
      ;

    for (Index = 0; Index < XferSize; Index++) {
      Buffer[Index + Offset] = MmioRead8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT));
    }

    Offset += XferSize;
  }

  SpifmcWaitInt (SpiBase, SPIFMC_INT_TRAN_DONE);
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpifmcWrite (
  IN  SPI_NOR     *Nor,
  IN  UINT32      To,
  IN  UINTN       Length,
  IN  CONST UINT8 *Buffer
  )
{
  UINTN      Index;
  INT32      Offset;
  INT32      XferSize;
  UINT32     Reg;
  UINT32     Wait;
  UINT32     SpiBase;

  SpiBase = Nor->SpiBase;
  Offset = 0;

  Reg = SpifmcInitReg (SpiBase);
  Reg |= Nor->AddrNbytes << SPIFMC_TRAN_CSR_ADDR_BYTES_SHIFT;
  Reg |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_8_BYTE;
  Reg |= SPIFMC_TRAN_CSR_WITH_CMD;
  Reg |= SPIFMC_TRAN_CSR_TRAN_MODE_TX;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);
  MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Nor->ProgramOpcode);

  for (Index = Nor->AddrNbytes - 1; Index >= 0; Index--) {
    MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), (To >> Index * 8) & 0xff);
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_INT_STS), 0);
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_NUM), Length);
  Reg |= SPIFMC_TRAN_CSR_GO_BUSY;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Reg);

  while ((MmioRead32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT)) & 0xf) != 0)
    ;

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

  while (Offset < Length) {
    XferSize = MIN (SPIFMC_MAX_FIFO_DEPTH, Length - Offset);

    Wait = 0;
    while ((MmioRead32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT)) & 0xf) != 0) {
      Wait ++;
      gBS->Stall (10);
      if (Wait > 30000) {
        DEBUG ((DEBUG_ERROR, "%a: Wait to write FIFO timeout.\n", __func__));
        return EFI_TIMEOUT;
      }
    }

    for (Index = 0; Index < XferSize; Index++)
      MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Buffer[Index + Offset]);

      Offset += XferSize;
    }

    SpifmcWaitInt (SpiBase, SPIFMC_INT_TRAN_DONE);
    MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpifmcErase (
  IN  SPI_NOR *Nor,
  IN  UINT32 Offs
  )
{
  UINTN      Index;
  UINT32     Reg;
  UINT32     SpiBase;

  SpiBase = Nor->SpiBase;

  Reg = SpifmcInitReg (SpiBase);
  Reg |= Nor->AddrNbytes << SPIFMC_TRAN_CSR_ADDR_BYTES_SHIFT;
  Reg |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_1_BYTE;
  Reg |= SPIFMC_TRAN_CSR_WITH_CMD;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);
  MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Nor->EraseOpcode);

  for (Index = Nor->AddrNbytes - 1; Index >= 0; Index--) {
    MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), (Offs >> Index * 8) & 0xff);
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_INT_STS), 0);
  Reg |= SPIFMC_TRAN_CSR_GO_BUSY;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Reg);
  SpifmcWaitInt (SpiBase, SPIFMC_INT_TRAN_DONE);
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

  return EFI_SUCCESS;
}



EFI_STATUS
SpiMasterInitProtocol (
  IN SOPHGO_SPI_MASTER_PROTOCOL *SpiMasterProtocol,
  IN SPI_NOR  *Nor
  )
{
  Nor->SpiBase = SPIFMC_BASE;

  if (PcdGet32 (PcdCpuRiscVMmuMaxSatpMode) > 0UL){
    for (INT32 I = 39; I < 64; I++) {
      if (Nor->SpiBase & (1ULL << 38)) {
        Nor->SpiBase |= (1ULL << I);
      } else {
        Nor->SpiBase &= ~(1ULL << I);
      }
    }
  }

  SpiMasterProtocol->ReadRegister   = SpifmcReadRegister;
  SpiMasterProtocol->WriteRegister  = SpifmcWriteRegister;
  SpiMasterProtocol->Read           = SpifmcRead;
  SpiMasterProtocol->Write          = SpifmcWrite;
  SpiMasterProtocol->Erase          = SpifmcErase;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MangoSpifmcEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS  Status;

  mSpiMasterInstance = AllocateRuntimeZeroPool (sizeof (SPI_MASTER));
  if (mSpiMasterInstance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  EfiInitializeLock (&mSpiMasterInstance->Lock, TPL_NOTIFY);

  SpiMasterInitProtocol (&mSpiMasterInstance->SpiMasterProtocol);

  mSpiMasterInstance->Signature = SPI_MASTER_SIGNATURE;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &(mSpiMasterInstance->Handle),
                  &gMarvellSpiMasterProtocolGuid,
                  &(mSpiMasterInstance->SpiMasterProtocol),
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    FreePool (mSpiMasterInstance);
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
