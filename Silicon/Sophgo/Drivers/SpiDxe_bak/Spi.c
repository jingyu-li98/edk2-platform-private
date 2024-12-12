/** @file
 *  SPI DW Controller.
 *
 *  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Protocol/FdtClient.h>
#include "Spi.h"

SPI_MASTER *mSpiMasterInstance;

STATIC
UINT32
SpiMmioRead (
  IN SOPHGO_SPI_DRIVER  *SpiDriver,
  IN UINT32              Offset
  )
{
  ASSERT ((Offset & 3) == 0);

  return MmioRead32 ((UINTN)(SpiDriver->RegBase + Offset));
}

STATIC
VOID
SpiMmioWrite (
  IN SOPHGO_SPI_DRIVER  *SpiDriver,
  IN UINT32              Offset,
  IN UINT32              Data
  )
{
  ASSERT ((Offset & 3) == 0);

  MemoryFence ();

  MmioWrite32 ((UINTN)(SpiDriver->RegBase + Offset), Data);
}

STATIC
VOID
SpiEnableChip (
  IN SOPHGO_SPI_DRIVER *SpiDriver,
  IN BOOLEAN           Enable
  )
{
  SpiMmioWrite (SpiDriver, DW_SPI_SSIENR, (Enable ? 1 : 0));
}

STATIC
EFI_STATUS
SpifmcWaitInt (
  IN UINTN   SpiBase,
  IN UINT8   IntType
  )
{
  UINT32  Stat;

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
  IN UINTN   SpiBase
  )
{
  UINT32 Register;

  Register = MmioRead32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR));
  Register &= ~(SPIFMC_TRAN_CSR_TRAN_MODE_MASK
           | SPIFMC_TRAN_CSR_CNTNS_READ
           | SPIFMC_TRAN_CSR_FAST_MODE
           | SPIFMC_TRAN_CSR_BUS_WIDTH_2_BIT
           | SPIFMC_TRAN_CSR_BUS_WIDTH_4_BIT
           | SPIFMC_TRAN_CSR_DMA_EN
           | SPIFMC_TRAN_CSR_ADDR_BYTES_MASK
           | SPIFMC_TRAN_CSR_WITH_CMD
           | SPIFMC_TRAN_CSR_FIFO_TRG_LVL_MASK);

  return Register;
}

/*
 * Return the max entries we can fill into tx fifo
 */
STATIC
UINT32
SpiTxMax (
  IN SOPHGO_SPI_DRIVER *SpiDriver
  )
{
  UINT32 TxRoom,
  UINT32 RxTxGap;

  TxRoom = SpiDriver->FifoLength - SpiMmioRead (SpiDriver, DW_SPI_TXFLR);

  //
  // Another concern is about the tx/rx mismatch, we
  // though to use (SpiDriver->FifoLength - RxFlr - Txflr) as
  // one maximum value for tx, but it doesn't cover the
  // data which is out of tx/rx fifo and inside the
  // shift registers. So a control from sw point of view is taken.
  //
  RxTxGap = SpiDriver->FifoLength - (SpiDriver->RxLength - SpiDriver->TxLength);
  
  return min3((UINT32)SpiDriver->TxLength, TxRoom, RxTxGap);
}

/*
 * Return the max entries we should read out of rx fifo
 */
STATIC
u32
SpiRxMax (
  IN SOPHGO_SPI_DRIVER *SpiDriver
  )
{
  return MIN (SpiDriver->RxLength, SpiMmioRead (SpiDriver, DW_SPI_RXFLR));
}

STATIC
EFI_STATUS
SpiWriter (
  IN SOPHGO_SPI_DRIVER *SpiDriver,
  IN VOID              *TxBuffer,
  IN UINT16            TxLength,
  IN UINT8             Nbytes
  )
{
  UINT32 Max;
  UINT16 TxW;

  // Max = Length;
  Max = SpiTxMax (SpiDriver);
  TxW = 0;

  while (Max --) {
    //
    // Set the tx word if the transfer's original "tx" is not null
    //
    if ((TxBuffer) {
      if (Nbytes == 1) {
        TxW = *(UINT8 *)(TxBuffer);
      } else if (Nbytes == 2) {
	TxW = *(UINT16 *)(TxBuffer);
      } else {
	TxW = *(UINT32 *)(TxBuffer);
      }

      DEBUG ((
	DEBUG_INFO,
	"%a: txw: %x tx len: %d\n",
	__func__,
        TxW,
        SpiMmioRead (SpiDriver, DW_SPI_TXFLR)
	));
      TxBuffer += Nbytes;
    } 

    SpiMmioWrite (SpiDriver, DW_SPI_DR, TxW);
    -- TxLength;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SpiReader (
  IN SOPHGO_SPI_DRIVER *SpiDriver,
  IN VOID              *RxBuffer,
  IN UINT16            RxLength,
  IN UINT8             Nbytes
  )
{
  UINT32 Max;
  UINT16 RxW;

  // Max = RxLength;
  Max = SpiRxMax (SpiDrvier);

  while (Max--) {
    RxW = SpiMmioRead (SpiDriver, DW_SPI_DR)
    if (RxBuffer) {
      if (n_bytes == 1) {
        *(UINT8 *)(RxBuf) = Rxw;
      } else if (Nbytes == 2) {
        *(UINT16 *)(RxBuf) = RxW;
      } else {
        *(UINT32 *)(RxBuf) = RxW;
      }
    
      DEBUG ((
        DEBUG_INFO,
        "rxw: %x tx len: %d\n",
        RxW,
        SpiMmioRead (SpiDriver, DW_SPI_RXFLR)
        );

      RxBuffer += Nbytes;
    }
    -- RxLength;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SpiLoopback (
  IN SOPHGO_SPI_DRIVER *SpiDrvier
  )
{
  UINT32 Max;
  UINT16 TxW;
  UINT16 RxW;

  Max = Length;
	timer_meter_start();

  while (Max > 0) {
    //
    // Set the tx word if the transfer's original "tx" is not null
    //
    if ((SPI_READ(DW_SPI_RISR) & SPI_RISR_TXOIR) != SPI_RISR_TXOIR) {
      timer_meter_start();

      if (dws->n_bytes == 1) {
        txw = *(u8 *)(dws->tx);
      } else {
	txw = *(u16 *)(dws->tx);
      }

      SPI_WRITE(DW_SPI_DR, txw);
      dws->tx += dws->n_bytes;
      udelay(10);

      rxw = SPI_READ(DW_SPI_DR);
      if (dws->n_bytes == 1) {
        *(u8 *)(dws->rx) = rxw;
      } else {
        *(u16 *)(dws->rx) = rxw;
      }

      dws->rx += dws->n_bytes;
      Max--;
    } else if (timer_meter_get_ms() < 100) {
      udelay(10); /* wait FIFO exit overflow status */
    } else {
      DEBUG ((
        DEBUG_INFO,
	"%a: SPI TX timeout\n",
	__func__
	));
      return EFI_TIMEOUT;
    }
    MemoryFence ();
  }

  return EFI_SUCCESS;
}

VOID
SpiInit (
  IN UINT8 Nbytes
  )
{
  UINT32 Value;
  UINT32 SpiMode;

  Ctrl0 = 0;
  SpiMode = 3;
  SpiEnableChip (FALSE);
  Value = SpiMmioRead (SpiDriver, DW_SPI_CTRL0);
  Value &= ~(DW_PSSI_CTRL0_TMOD_MASK
           | DW_PSSI_CTRL0_SCPOL
           | DW_PSSI_CTRL0_SCPHA
	   | DW_PSSI_CTRL0_FRF_MASK
           | DW_PSSI_CTRL0_DFS_MASK);
  if (Nbytes == 1) {
    SpiMmioWrite (SpiDriver,
		  DW_SPI_CTRL0,
		  Value
		  | DW_SPI_CTRL0_TMOD_TR
		  | (SpiMode << 6)
		  | DW_SPI_CTRL0_FRF_MOTO_SPI
		  | 0x7 // 8-bit serial data transfer 
		  ); /* Set to SPI frame format */
  } else if (Nbytes == 2) {
    SpiMmioWrite (SpiDriver, DW_SPI_CTRL0,
		  Value
		  | DW_SPI_CTRL0_TMOD_TR
		  | (SpiMode << 6)
		  | SPI_FRF_SPI
		  | 0xF // 16-bit serial data transfer
		);
  }

  SpiMmioWrite (SpiDriver, DW_SPI_BAUDR, SPI_BAUDR_DIV);
  SpiMmioWrite (SpiDriver, DW_SPI_TXFLTR, 4);
  SpiMmioWrite (SpiDriver, DW_SPI_RXFLTR, 4);
  SpiMmioWrite (SpiDriver, DW_SPI_SER, 0x1); /* enable slave 1 device */

  DEBUG ((
    DEBUG_INFO,
    "%a: CS %x\n",
    __func__,
    SpiMmioRead (SpiDriver, DW_SPI_SER)
    ));

  DEBUG ((
    DEBUG_INFO,
    "%a: SPI mode: %d\n",
    __func__,
    (SpiMmioRead (SpiDriver, DW_SPI_CTRL0) >> 6) & 0x3
    ));

  SpiEnableChip (TRUE);
}

EFI_STATUS
EFIAPI
SpifmcRead (
  IN  SPI_NOR *Nor,
  IN  UINTN   From,
  IN  UINTN   Length,
  OUT UINT8   *Buffer
  )
{
  INT32      XferSize;
  INT32      Offset;
  INT32      Index;
  UINTN      SpiBase;
  UINT32     Register;
  EFI_STATUS Status;

  SpiBase = Nor->SpiBase;
  Offset = 0;

  Register = SpifmcInitReg (SpiBase);
  Register |= (Nor->AddrNbytes) << SPIFMC_TRAN_CSR_ADDR_BYTES_SHIFT;
  Register |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_8_BYTE;
  Register |= SPIFMC_TRAN_CSR_WITH_CMD;
  Register |= SPIFMC_TRAN_CSR_TRAN_MODE_RX;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);
  MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Nor->ReadOpcode);

  //
  // This is a workaround.
  // For Length<=SPIFMC_MAX_FIFO_DEPTH, we have to modify the Length manually.
  // Although we set the TRAN_CSR_FIFO_TRG_LVL_1_BYTE, not
  // TRAN_CSR_FIFO_TRG_LVL_8_BYTE, we cannot wait for the INT_RD_FIFO to be set
  // when the Length equals one byte.
  //
  Length = MAX (SPIFMC_MAX_FIFO_DEPTH, Length);

  for (Index = Nor->AddrNbytes - 1; Index >= 0; Index --) {
    MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), (From >> Index * 8) & 0xff);
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_INT_STS), 0);
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_NUM), Length);
  Register |= SPIFMC_TRAN_CSR_GO_BUSY;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Register);

  Status = SpifmcWaitInt (SpiBase, SPIFMC_INT_RD_FIFO);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Wait Read FIFO Done %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  while (Offset < Length) {
    XferSize = MIN (SPIFMC_MAX_FIFO_DEPTH, Length - Offset);

    while ((MmioRead32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT)) & 0xf) != XferSize)
      ;

    for (Index = 0; Index < XferSize; Index++) {
      Buffer[Index + Offset] = MmioRead8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT));
    }

    Offset += XferSize;
  }

  Status = SpifmcWaitInt (SpiBase, SPIFMC_INT_TRAN_DONE);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Wait Transfer Done %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpifmcDmmrRead (
  IN  SPI_NOR *Nor,
  IN  UINTN   From,
  IN  UINTN   Length,
  OUT UINT8   *Buffer
  )
{
  UINT32     Register;
  UINTN      SpiBase;

  SpiBase = Nor->SpiBase;

  Register = SpifmcInitReg (SpiBase);
  Register |= (Nor->AddrNbytes) << SPIFMC_TRAN_CSR_ADDR_BYTES_SHIFT;
  Register |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_8_BYTE;
  Register |= SPIFMC_TRAN_CSR_WITH_CMD;
  Register |= SPIFMC_TRAN_CSR_TRAN_MODE_RX;
  if (Nor->AddrNbytes == 4) {
    Register |= SPIFMC_TRAN_CSR_ADDR4B;
    Register |= SPIFMC_TRAN_CSR_CMD4B;
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Register);

  //
  // enable DMMR (Direct Memory Mapping Read)
  //
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_DMMR), 1);

  //
  // Read the data
  //
  CopyMem (Buffer, (UINTN *)(SpiBase + From), Length);

  //
  // disable DMMR (Direct Memory Mapping Read)
  //
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_DMMR), 0);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpifmcWrite (
  IN  SPI_NOR     *Nor,
  IN  UINTN       To,
  IN  UINTN       Length,
  IN  CONST UINT8 *Buffer
  )
{
  INT32      Index;
  UINTN      SpiBase;
  INT32      Offset;
  INT32      XferSize;
  UINT32     Register;
  UINT32     WaitTime;
  EFI_STATUS Status;

  SpiBase     = Nor->SpiBase;
  Offset      = 0;
  WaitTime    = 0;

  Register = SpifmcInitReg (SpiBase);
  Register |= Nor->AddrNbytes << SPIFMC_TRAN_CSR_ADDR_BYTES_SHIFT;
  Register |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_8_BYTE;
  Register |= SPIFMC_TRAN_CSR_WITH_CMD;
  Register |= SPIFMC_TRAN_CSR_TRAN_MODE_TX;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);
  MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Nor->ProgramOpcode);

  for (Index = Nor->AddrNbytes - 1; Index >= 0; Index--) {
    MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), (To >> Index * 8) & 0xff);
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_INT_STS), 0);
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_NUM), Length);
  Register |= SPIFMC_TRAN_CSR_GO_BUSY;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Register);

  while ((MmioRead32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT)) & 0xf) != 0)
    ;

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

  while (Offset < Length) {
    XferSize = MIN (SPIFMC_MAX_FIFO_DEPTH, Length - Offset);

    while ((MmioRead32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT)) & 0xf) != 0) {
      WaitTime ++;
      gBS->Stall (10);
      if (WaitTime > 30000) {
        DEBUG ((
          DEBUG_ERROR,
          "%a: Wait FIFO empty timeout!\n",
          __func__
          ));
        return EFI_TIMEOUT;
      }
    }

    for (Index = 0; Index < XferSize; Index++) {
      MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Buffer[Index + Offset]);
    }

    Offset += XferSize;
  }

  Status = SpifmcWaitInt (SpiBase, SPIFMC_INT_TRAN_DONE);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Wait Transfer Done %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpifmcErase (
  IN  SPI_NOR *Nor,
  IN  UINTN   Offs
  )
{
  INT32      Index;
  UINTN      SpiBase;
  UINT32     Register;
  EFI_STATUS Status;

  SpiBase = Nor->SpiBase;

  Register = SpifmcInitReg (SpiBase);
  Register |= Nor->AddrNbytes << SPIFMC_TRAN_CSR_ADDR_BYTES_SHIFT;
  Register |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_1_BYTE;
  Register |= SPIFMC_TRAN_CSR_WITH_CMD;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);
  MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), Nor->EraseOpcode);

  for (Index = Nor->AddrNbytes - 1; Index >= 0; Index--) {
    MmioWrite8 ((UINTN)(SpiBase + SPIFMC_FIFO_PORT), (Offs >> Index * 8) & 0xff);
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_INT_STS), 0);
  Register |= SPIFMC_TRAN_CSR_GO_BUSY;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Register);

  Status = SpifmcWaitInt (SpiBase, SPIFMC_INT_TRAN_DONE);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Wait Transfer Done %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_FIFO_PT), 0);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpifmcInit (
  IN  SPI_NOR *Nor
  )
{
  UINTN      SpiBase;
  UINT32     Register;

  SpiBase = Nor->SpiBase;

  //
  // disable DMMR (Direct Memory Mapping Read)
  //
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_DMMR), 0);

  //
  // Soft reset
  //
  MmioWrite32 (SpiBase + SPIFMC_CTRL, MmioRead32 ((UINTN)(SpiBase + SPIFMC_CTRL)) | SPIFMC_CTRL_SRST | 0x3);

  //
  // Hardware CE contrl, soft reset cannot change the register
  //
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_CE_CTRL), 0);

  Register = Nor->AddrNbytes << SPIFMC_TRAN_CSR_ADDR_BYTES_SHIFT;
  Register |= SPIFMC_TRAN_CSR_FIFO_TRG_LVL_4_BYTE;
  Register |= SPIFMC_TRAN_CSR_WITH_CMD;
  MmioWrite32 ((UINTN)(SpiBase + SPIFMC_TRAN_CSR), Register);

  return EFI_SUCCESS;
}

SPI_NOR *
EFIAPI
SpiMasterSetupSlave (
  IN SOPHGO_SPI_MASTER_PROTOCOL *This,
  IN SPI_NOR                    *Nor
  )
{
#if 0
  FDT_CLIENT_PROTOCOL    *FdtClient;
  INT32                  Node;
  CONST INT32            *RegProp;
  UINT32                 RegSize;
  EFI_STATUS             Status;
#endif


  if (!Nor) {
    Nor = AllocateZeroPool (sizeof(SPI_NOR));
    if (!Nor) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Cannot allocate memory\n",
        __func__
        ));
      return NULL;
    }
  }
#if 0
  Status = gBS->LocateProtocol (
		  &gFdtClientProtocolGuid,
		  NULL,
		  (VOID **)&FdtClient
		  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "Failed to locate FDT client protocol: %r\n",
      Status));

    return NULL;
  }

  for (Status = FdtClient->FindCompatibleNode (
                                     FdtClient,
                                     "sophgo,spifmc",
                                     &Node
                                     );
       !EFI_ERROR (Status);
       Status = FdtClient->FindNextCompatibleNode (
                                     FdtClient,
                                     "sophgo,spifmc",
                                     Node,
                                     &Node
                                     ))
  {
    Status = FdtClient->GetNodeProperty (
                      FdtClient,
                      Node,
                      "reg",
                      (CONST VOID **)&RegProp,
                      &RegSize
                      );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Get reg failed (Status == %r)\n",
        __func__,
        Status
        ));
      return NULL;
    } else {
      Nor->SpiBase = SwapBytes64 (((CONST UINT64 *) RegProp)[0]);
    }
  }
#else
  Nor->SpiBase = SPIFMC_BASE;
#endif

  Nor->BounceBufSize = SIZE_4KB;
  Nor->BounceBuf = AllocateZeroPool (Nor->BounceBufSize);
  if (!Nor->BounceBuf) {
    return NULL;
  }


  SpifmcInit (Nor);

  DEBUG ((
    DEBUG_VERBOSE,
    "%a[%d] SPI Base Address = 0x%llx\n",
    __func__,
    __LINE__,
    Nor->SpiBase
    ));

  return Nor;
}

EFI_STATUS
EFIAPI
SpiMasterFreeSlave (
  IN SPI_NOR *Nor
  )
{
  FreePool (Nor);

  // FreePool (Nor->BounceBuf);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpifmcEntryPoint (
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

  mSpiMasterInstance->SpiMasterProtocol.ReadRegister   = SpifmcReadRegister;
  mSpiMasterInstance->SpiMasterProtocol.WriteRegister  = SpifmcWriteRegister;
  mSpiMasterInstance->SpiMasterProtocol.Write          = SpifmcWrite;
  mSpiMasterInstance->SpiMasterProtocol.Erase          = SpifmcErase;
  mSpiMasterInstance->SpiMasterProtocol.SetupDevice    = SpiMasterSetupSlave;
  mSpiMasterInstance->SpiMasterProtocol.FreeDevice     = SpiMasterFreeSlave;

  if (FixedPcdGetBool (PcdSpifmcDmmrEnable)) {
    mSpiMasterInstance->SpiMasterProtocol.Read           = SpifmcDmmrRead;
  } else {
    mSpiMasterInstance->SpiMasterProtocol.Read           = SpifmcRead;
  }

  mSpiMasterInstance->Signature = SPI_MASTER_SIGNATURE;

  Status = gBS->InstallMultipleProtocolInterfaces (
                    &(mSpiMasterInstance->Handle),
                    &gSophgoSpiMasterProtocolGuid,
                    &(mSpiMasterInstance->SpiMasterProtocol),
                    NULL
                    );
  if (EFI_ERROR (Status)) {
    FreePool (mSpiMasterInstance);
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
