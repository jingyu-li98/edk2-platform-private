/** @file
 *
 *  Copyright (c) 2023, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include "NorFlashDxe.h"

NOR_FLASH_INSTANCE         *mNorFlashInstance;
SOPHGO_SPI_MASTER_PROTOCOL *SpiMasterProtocol;

STATIC
EFI_STATUS
SpiNorWriteEnable (
  IN SPI_NOR  *Nor
  )
{
  EFI_STATUS Status;

  Status = SpiMasterProtocol->WriteRegister (Nor, SPINOR_OP_WREN, NULL, 0);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
      "%a: SPINor error while write enable\n",
      __func__
      ));
  }

  return Status;
}

STATIC
EFI_STATUS
SpiNorWriteDisable (
  IN SPI_NOR  *Nor
  )
{
  EFI_STATUS Status;

  Status = SpiMasterProtocol->WriteRegister (Nor, SPINOR_OP_WRDI, NULL, 0);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
      "%a: SPINor error while write disable\n",
      __func__
      ));
  }

  return Status;
}

EFI_STATUS
EFIAPI
SpiNorGetFlashId (
  IN SPI_NOR     *Nor,
  IN BOOLEAN     UseInRuntime
  )
{
  UINT8      Id[NOR_FLASH_MAX_ID_LEN];
  EFI_STATUS Status;

  Status = SpiMasterProtocol->ReadRegister (Nor, SPINOR_OP_RDID, SPI_NOR_MAX_ID_LEN, Id);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ReadId: Spi error while reading id\n"
      ));
    return Status;
  }

  Status = NorFlashGetInfo (Id, &Nor->Info, UseInRuntime);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
      "%a: Unrecognized JEDEC Id bytes: 0x%02x%02x%02x\n",
      __func__,
      Id[0],
      Id[1],
      Id[2]));
    return Status;
  }

  NorFlashPrintInfo (Nor->Info);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiNorReadData (
  IN  SPI_NOR   *Nor,
  IN  UINTN      FlashOffset,
  IN  UINTN      Length,
  OUT UINT8      *Buffer
  )
{
  // UINTN RetLenth;
  // EFI_STATUS Status;
  UINTN       Index;
  UINTN       Addr;
  UINTN       PageOffset;
  UINTN       PageRemain;
  EFI_STATUS  Status;

  if (Length == 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Length is Zero!\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (Buffer == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Buffer is NULL!\n",
      __func__
      ));
    return EFI_BAD_BUFFER_SIZE;
  }

  //
  // read data from flash memory by PAGE
  //
  DEBUG ((
    DEBUG_INFO,
    "%a[%d] Read progress:       ",
    __func__,
    __LINE__
    ));
  for (Index = 0; Index < Length; Index += PageRemain) {
    Addr = FlashOffset + Index;

    PageOffset = IS_POW2 (Nor->Info->PageSize) ? (Addr & (Nor->Info->PageSize - 1)) : (Addr % Nor->Info->PageSize);

    PageRemain = MIN (Nor->Info->PageSize - PageOffset, Length - Index);

    Status = SpiMasterProtocol->Read (Nor, Addr, PageRemain, Buffer + Index);
    if (EFI_ERROR(Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Read Data from flash memory - %r!\n",
        __func__,
        Status
        ));
        return Status;
      }

    DEBUG ((
      DEBUG_INFO,
      "\b\b\b%2ld%%",
      (Index + PageRemain) * 100 / Length
      ));
  }
  DEBUG ((DEBUG_INFO, "\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiNorReadStatus (
  IN SPI_NOR     *Nor,
  IN UINT8       *Sr
  )
{
  EFI_STATUS Status;

  Status = SpiMasterProtocol->ReadRegister (Nor, SPINOR_OP_RDSR, 1, Sr);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Read the Status Register - %r\n",
      __func__,
      Status
      ));
  }

  return Status;
}

/**
  Wait for a predefined amount of time for the flash to be ready,
  or timeout occurs.

**/
EFI_STATUS
SpiNorWaitTillReady (
  IN SPI_NOR *Nor
  )
{
  UINT32 WaitTime;

  WaitTime = 0;

  while (1) {
    gBS->Stall (100000);

    //
    // Query the Status Register to see if the flash is ready for new commands.
    //
    SpiNorReadStatus (Nor, Nor->BounceBuf);

    if (!(Nor->BounceBuf[0] & SR_WIP) || WaitTime > 30) {
      return EFI_SUCCESS;
    }

    WaitTime ++;
  }

  return EFI_TIMEOUT;
}

EFI_STATUS
EFIAPI
SpiNorWriteStatus (
  IN SPI_NOR     *Nor,
  IN UINT8       *Sr,
  IN UINTN       Length
  )
{
  EFI_STATUS Status;

  Status = SpiNorWriteEnable (Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Write Enable - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  Status = SpiMasterProtocol->WriteRegister (Nor, SPINOR_OP_WRSR, Sr, Length);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Write Register - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  Status = SpiNorWaitTillReady (Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR, 
      "%a: flash is not ready for new commands - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  //
  // Write disable
  //
  Status = SpiNorWriteDisable (Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR, 
      "%a: Write Disable - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiNorWriteData (
  IN SPI_NOR     *Nor,
  IN UINTN       FlashOffset,
  IN UINTN       Length,
  IN UINT8       *Buffer
  )
{
  UINTN       Index;
  UINTN       Addr;
  UINTN       PageOffset;
  UINTN       PageRemain;
  EFI_STATUS  Status;

  if (Length == 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Length is Zero!\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (Buffer == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Buffer is NULL!\n",
      __func__
      ));
    return EFI_BAD_BUFFER_SIZE;
  }

  Status = SpiNorWriteEnable (Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Write Enable - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  //
  // Write data by PAGE
  //
  DEBUG ((
    DEBUG_INFO,
    "%a[%d] Write progress:       ",
    __func__,
    __LINE__
    ));
  for (Index = 0; Index < Length; Index += PageRemain) {
    Addr = FlashOffset + Index;

    PageOffset = IS_POW2 (Nor->Info->PageSize) ? (Addr & (Nor->Info->PageSize - 1)) : (Addr % Nor->Info->PageSize);

    PageRemain = MIN (Nor->Info->PageSize - PageOffset, Length - Index);
    // DEBUG ((DEBUG_WARN, "%a[%d] - Length=0x%lx == > Index[0x%lx] ==> PageRemain=0x%lx\n", __func__, __LINE__, Length, Index, PageRemain));

    Status = SpiMasterProtocol->Write (Nor, Addr, PageRemain, Buffer + Index);
    if (EFI_ERROR (Status)) {
      DEBUG ((
          DEBUG_ERROR,
          "%a: Write Data - %r\n",
          __func__,
          Status
          ));
      return Status;
    }

    Status = SpiNorWaitTillReady (Nor);
    if (EFI_ERROR (Status)) {
      DEBUG ((
          DEBUG_ERROR, 
          "%a: flash is not ready for new commands - %r\n",
          __func__,
          Status
          ));
      return Status;
    }

    DEBUG ((
      DEBUG_INFO,
      "\b\b\b%2ld%%",
      (Index + PageRemain) * 100 / Length
      ));
  }
  DEBUG ((DEBUG_INFO, "\n"));

  Status = SpiNorWriteDisable (Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR, 
      "%a: Write Disable - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiNorErase (
  IN SPI_NOR    *Nor,
  IN UINTN      FlashOffset,
  IN UINTN      Length
  )
{
  EFI_STATUS Status;
  UINT32     ErasedSectors;
  INT32      Index;
  UINTN      Address;

  Address = FlashOffset;

  if (Length == 0) {
    DEBUG ((DEBUG_ERROR,
      "%a: Length is Zero!\n",
      __func__
    ));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Write enable
  //
  Status = SpiNorWriteEnable (Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Write Enable - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  //
  // Erase Sector
  //
  ErasedSectors = Length / Nor->Info->SectorSize + 1;
  DEBUG ((
      DEBUG_INFO,
      "%a[%d] Erase progress:       ",
      __func__,
      __LINE__
      ));
  for (Index = 0; Index < ErasedSectors; Index++) {
    Address += Index * Nor->Info->SectorSize;

    Status = SpiMasterProtocol->Erase (Nor, Address);
    if (EFI_ERROR (Status)) {
      DEBUG ((
          DEBUG_ERROR,
          "%a: Erase Sector - %r\n",
          __func__,
          Status
          ));
      return Status;
    }

    // for (Index = Nor->AddrNbytes - 1; Index >= 0; Index--) {
    //   // DEBUG ((DEBUG_INFO, "\n%a[%d] Index=%d, Addr=0x%lx, Addr & 0xff=0x%lx-----\n", __func__, __LINE__, Index, Addr, Addr & 0xff));
    //   Nor->BounceBuf[Index] = Addr & 0xff;
    //   Addr >>= 8;
    //   // DEBUG ((DEBUG_INFO, "\n%a[%d] Index=%d, Addr=0x%lx, Nor->BounceBuf[Index]=0x%lx-----\n", __func__, __LINE__, Index, Addr, Nor->BounceBuf[Index]));
    // }
    // DEBUG ((DEBUG_INFO, "%a[%d] Write Register -----\n", __func__, __LINE__));
    // Status = SpiMasterProtocol->WriteRegister (Nor, Nor->EraseOpcode, Nor->BounceBuf, Nor->AddrNbytes);
    // if (EFI_ERROR (Status)) {
    //   DEBUG ((
    //     DEBUG_ERROR,
    //     "%a: Write Register - %r\n",
    //     __func__,
    //     Status
    //     ));
    //   return Status;
    // }

    Status = SpiNorWaitTillReady (Nor);
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: flash is not ready for new commands - %r\n",
        __func__,
        Status
        ));
      return Status;
    }

    DEBUG ((
      DEBUG_INFO,
      "\b\b\b%2ld%%",
      (Index + 1) * 100 / ErasedSectors
      ));
  }
  DEBUG ((DEBUG_INFO, "\n"));

  //
  // Write disable
  //
  Status = SpiNorWriteDisable (Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR, 
      "%a: Write Disable - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetPartitionInfoByImageName (
  IN     SPI_NOR       *Nor,
  IN     UINTN         PartitionTableAddr,
  IN     CONST UINT8   *ImageName,
  IN OUT PART_INFO     *PartInfo
  )
{
  EFI_STATUS Status;

  do {
    Status = SpiNorReadData (Nor, PartitionTableAddr, sizeof (PART_INFO), (UINT8 *)PartInfo);
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Read Partition Info - %r\n",
        __func__,
        Status
        ));
      return Status;
    }

    if (PartInfo->Magic != DPT_MAGIC) {
      DEBUG ((
        DEBUG_ERROR,
        "%a[%d] Bad partition magic\n",
        __func__,
        __LINE__));
      return EFI_INVALID_PARAMETER;
    }

    PartitionTableAddr += sizeof (PART_INFO);

  } while (!EFI_ERROR (Status) && CompareMem (PartInfo->Name, ImageName, AsciiStrLen ((CONST CHAR8 *)ImageName)));

  return Status;
}

EFI_STATUS
EFIAPI
SpiNorLoadImage (
  IN SPI_NOR     *Nor,
  IN UINTN       PartitionTableAddr,
  IN CONST UINT8 *ImageName
  )
{
  EFI_STATUS Status;
  PART_INFO  *PartInfo;

  PartInfo = AllocateZeroPool (sizeof (PART_INFO));
  if (!PartInfo) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = GetPartitionInfoByImageName (Nor, PartitionTableAddr, ImageName, PartInfo);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Failed to get %s partition info\n",
      __func__,
      ImageName
      ));
    return Status;
  }

  DEBUG ((
    DEBUG_INFO,
    "%a: Load %s image from spi flash 0x%x to memory 0x%lx size %d\n",
    __func__,
    ImageName,
    PartInfo->Offset,
    PartInfo->TargetMemAddr,
    PartInfo->Size
    ));

  Status = SpiNorReadData (Nor, PartInfo->Offset, PartInfo->Size, (UINT8 *)PartInfo->TargetMemAddr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
      "%a: Failed to load %s image form spi flash\n",
      __func__,
      ImageName
      ));
    return Status;
  }

  FreePool (PartInfo);
  // flush_dcache_range(ZSBL_BASE, PartInfo->size);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiNorInit (
  IN SOPHGO_NOR_FLASH_PROTOCOL *This,
  IN SPI_NOR                   *Nor
  )
{
  EFI_STATUS Status;

  Nor->AddrNbytes= (Nor->Info->Flags & NOR_FLASH_4B_ADDR) ? 4 : 3;

  Status = SpiNorWriteEnable (Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Write Enable - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  Nor->ReadOpcode = SPINOR_OP_READ; // Low Frequency
  Nor->ProgramOpcode = SPINOR_OP_PP;
  Nor->EraseOpcode = SPINOR_OP_SE;

  if (Nor->AddrNbytes == 4) {
    //
    // Enter 4-byte mode
    //
    Status = SpiMasterProtocol->WriteRegister (Nor, SPINOR_OP_EN4B, NULL, 0);
    if (EFI_ERROR (Status)) {
      DEBUG((
        DEBUG_ERROR,
        "%a: Enter 4-byte mode - %r\n",
        __func__,
        Status
        ));
      return Status;
    }

    Nor->ReadOpcode    = SPINOR_OP_READ_4B;
    Nor->ProgramOpcode = SPINOR_OP_PP_4B;
    Nor->EraseOpcode   = SPINOR_OP_SE_4B;
  }

  //
  // Initialize flash status register
  //
  Status = SpiNorWriteStatus (Nor, Nor->BounceBuf, 1);
  if (EFI_ERROR (Status)) {
    DEBUG((
      DEBUG_ERROR,
      "%a: Initialize status register - %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiNorEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (
                &gSophgoSpiMasterProtocolGuid,
                NULL,
                (VOID **)&SpiMasterProtocol
                );

  if (EFI_ERROR (Status)) {
    DEBUG((
      DEBUG_ERROR,
      "SpiNor: Cannot locate SPI Master protocol\n"
      ));
    return EFI_DEVICE_ERROR;
  }

  mNorFlashInstance = AllocateRuntimeZeroPool (sizeof (NOR_FLASH_INSTANCE));

  if (mNorFlashInstance == NULL) {
    DEBUG((
      DEBUG_ERROR,
      "SpiNor: Cannot allocate memory\n"
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  mNorFlashInstance->NorFlashProtocol.Init            = SpiNorInit;
  mNorFlashInstance->NorFlashProtocol.GetFlashid      = SpiNorGetFlashId;
  mNorFlashInstance->NorFlashProtocol.ReadData        = SpiNorReadData;
  mNorFlashInstance->NorFlashProtocol.WriteData       = SpiNorWriteData;
  mNorFlashInstance->NorFlashProtocol.ReadStatus      = SpiNorReadStatus;
  mNorFlashInstance->NorFlashProtocol.WriteStatus     = SpiNorWriteStatus;
  mNorFlashInstance->NorFlashProtocol.Erase           = SpiNorErase;
  mNorFlashInstance->NorFlashProtocol.LoadImage       = SpiNorLoadImage;

  mNorFlashInstance->Signature = NOR_FLASH_SIGNATURE;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &(mNorFlashInstance->Handle),
                  &gSophgoNorFlashProtocolGuid,
                  &(mNorFlashInstance->NorFlashProtocol),
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG((
      DEBUG_ERROR,
      "SpiNor: Cannot install SPI flash protocol\n"
      ));
    goto ErrorInstallProto;
  }

  // //
  // // Register for the virtual address change event
  // //
  // Status = gBS->CreateEventEx (EVT_NOTIFY_SIGNAL,
  //                 TPL_NOTIFY,
  //                 NorFlashVirtualNotifyEvent,
  //                 NULL,
  //                 &gEfiEventVirtualAddressChangeGuid,
  //                 &mNorFlashVirtualAddrChangeEvent);
  // if (EFI_ERROR (Status)) {
  //   DEBUG ((DEBUG_ERROR, "%a: Failed to register VA change event\n", __func__));
  //   goto ErrorCreateEvent;
  // }

  return EFI_SUCCESS;

// ErrorCreateEvent:
//   gBS->UninstallMultipleProtocolInterfaces (&mNorFlashInstance->Handle,
//     &gSophgoNorFlashProtocolGuid,
//     NULL);

ErrorInstallProto:
  FreePool (mNorFlashInstance);

  return EFI_SUCCESS;
}
