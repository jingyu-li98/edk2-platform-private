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
SOPHGO_NOR_FLASH_PROTOCOL  *NorFlashProtocol;
UINT32                      mNorFlashDeviceCount;

STATIC NOR_FLASH_DEVICE_PATH mDevicePathTemplate = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8)(OFFSET_OF (NOR_FLASH_DEVICE_PATH, End)),
        (UINT8)(OFFSET_OF (NOR_FLASH_DEVICE_PATH, End) >> 8)
      }
    },
    // { 0x0,                               0x0, 0x0, { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }
    //   },                                                             // GUID ... NEED TO BE FILLED
    EFI_CALLER_ID_GUID
  },
  0, // Index
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    { sizeof (EFI_DEVICE_PATH_PROTOCOL), 0 }
  }
};   // DevicePath

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
      "%a: SpiNor error while write enable\n",
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
      "%a: SpiNor error while write disable\n",
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
      "%a: Flash is not ready for new commands - %r\n",
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
SpiNorReadData (
  IN  SPI_NOR   *Nor,
  IN  UINTN      FlashOffset,
  IN  UINTN      Length,
  OUT UINT8      *Buffer
  )
{
  UINTN       Index;
  UINTN       Address;
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
  // DEBUG ((
  //   DEBUG_INFO,
  //   "%a\tRead progress:       ",
  //   __func__
  //   ));
  for (Index = 0; Index < Length; Index += PageRemain) {
    Address = FlashOffset + Index;
    PageOffset = IS_POW2 (Nor->Info->PageSize) ? (Address & (Nor->Info->PageSize - 1)) : (Address % Nor->Info->PageSize);
    PageRemain = MIN (Nor->Info->PageSize - PageOffset, Length - Index);
    DEBUG ((DEBUG_WARN, "%a: Address=0x%lx\tPageRemain=0x%lx\tPageOffset=0x%lx\tIndex=0x%lx\tLength=0x%lx\n",__func__, Address, PageRemain, PageOffset, Index, Length));
    Status = SpiMasterProtocol->Read (Nor, Address, PageRemain, Buffer + Index);
    if (EFI_ERROR(Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Read Data from flash memory - %r!\n",
        __func__,
        Status
        ));
        return Status;
      }

    // Status = SpiNorWaitTillReady (Nor);
    // if (EFI_ERROR (Status)) {
    //   DEBUG ((
    //     DEBUG_ERROR,
    //     "%a: Flash is not ready for new commands - %r\n",
    //     __func__,
    //     Status
    //     ));
    //   return Status;
    // }

    // DEBUG ((
    //   DEBUG_INFO,
    //   "\b\b\b%2ld%%",
    //   (Index + PageRemain) * 100 / Length
    //   ));
  }
  // DEBUG ((DEBUG_INFO, "\n"));

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
  UINTN       Address;
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
  // Write data by PAGE
  //
  // DEBUG ((
  //   DEBUG_INFO,
  //   "%a\tWrite progress:       ",
  //   __func__
  //   ));
  for (Index = 0; Index < Length; Index += PageRemain) {
    Address = FlashOffset + Index;
    PageOffset = IS_POW2 (Nor->Info->PageSize) ? (Address & (Nor->Info->PageSize - 1)) : (Address % Nor->Info->PageSize);
    PageRemain = MIN (Nor->Info->PageSize - PageOffset, Length - Index);

    DEBUG ((DEBUG_WARN, "%a[%d] Length=0x%lx ==> Index=0x%lx ==> Address=%0xlx ==> PageRemain=0x%lx \n", __func__, __LINE__, Length, Index, Address, PageRemain));
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

    Status = SpiMasterProtocol->Write (Nor, Address, PageRemain, Buffer + Index);
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
          "%a: Flash is not ready for new commands - %r\n",
          __func__,
          Status
          ));
      return Status;
    }

    // DEBUG ((
    //   DEBUG_INFO,
    //   "\b\b\b%2ld%%",
    //   (Index + PageRemain) * 100 / Length
    //   ));
  }
  // DEBUG ((DEBUG_INFO, "\n"));

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
  UINT32     ErasedSectors;
  INT32      Index;
  UINTN      Address;
  UINTN      EraseSize;
  EFI_STATUS Status;

  if (Length == 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Length is Zero!\n",
      __func__
    ));
    return EFI_INVALID_PARAMETER;
  }

  Address = FlashOffset;

  if (Nor->Info->Flags & NOR_FLASH_ERASE_4K) {
    EraseSize = SIZE_4KB;
  } else {
    EraseSize = Nor->Info->SectorSize;
  }

  if ((FlashOffset % EraseSize) != 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: <flash offset addr> is not aligned erase sector size (0x%x)!\n",
      __func__,
      EraseSize
      ));
    return EFI_INVALID_PARAMETER;
  }



  //
  // Erase Sector
  //
  ErasedSectors = (Length + EraseSize - 1) / EraseSize;
  DEBUG ((
      DEBUG_INFO,
      "%a: Start erasing %d sectors, each %d bytes...\nErase progress:       ",
      __func__,
      ErasedSectors,
      EraseSize
      ));
  for (Index = 0; Index < ErasedSectors; Index++) {
    Address += Index * EraseSize;
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

    Status = SpiNorWaitTillReady (Nor);
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Flash is not ready for new commands - %r\n",
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
GetPartitionInfo (
  IN     SPI_NOR       *Nor,
  IN     UINTN         PartitionTableAddr,
  IN OUT PART_INFO     *PartInfo
  )
{
  EFI_STATUS Status;

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
      "%a: Bad partition magic\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
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
    Status = GetPartitionInfo (Nor, PartitionTableAddr, PartInfo);

    PartitionTableAddr += sizeof (PART_INFO);

  } while (!(!EFI_ERROR (Status) && CompareMem (PartInfo->Name, ImageName, AsciiStrLen ((CONST CHAR8 *)ImageName))));

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

  Nor->ReadOpcode    = SPINOR_OP_READ; // Low Frequency
  Nor->ProgramOpcode = SPINOR_OP_PP;
  Nor->EraseOpcode   = SPINOR_OP_SE;

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

/**
  Build the device path for the SPI Nor Flash.

  @param[in]  This           Pointer to the EFI_MMC_HOST_PROTOCOL instance.
  @param[out] DevicePath     Pointer to the location to store the newly created device path.

  @retval EFI_SUCCESS        The device path is built successfully.

**/
EFI_STATUS
EFIAPI
SpiNorBuildDevicePath (
  // IN  SOPHGO_NOR_FLASH_PROTOCOL   *This,
  OUT EFI_DEVICE_PATH_PROTOCOL    **DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL *NewDevicePathNode;
  EFI_GUID DevicePathGuid = EFI_CALLER_ID_GUID;

  DEBUG ((
    DEBUG_INFO,
    "SpiNor: %a\n",
    __func__
    ));

  NewDevicePathNode = CreateDeviceNode (HARDWARE_DEVICE_PATH, HW_VENDOR_DP, sizeof (VENDOR_DEVICE_PATH));
  CopyGuid (&((VENDOR_DEVICE_PATH*)NewDevicePathNode)->Guid, &DevicePathGuid);
  *DevicePath = NewDevicePathNode;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiNorEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS            Status;
  NOR_FLASH_DEVICE_PATH *NorFlashDevicePath;
  UINT32                Index;
  // SPI_NOR               *NorFlash;

  Index = 0;

  //
  // Locate SPI Master protocol
  //
  DEBUG ((DEBUG_INFO, "%a[%d] Locate SPI Master protocol\n", __func__, __LINE__));
  Status = gBS->LocateProtocol (
                  &gSophgoSpiMasterProtocolGuid,
                  NULL,
                  (VOID **)&SpiMasterProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
        DEBUG_ERROR,
        "%a: Cannot locate SPI Master protocol\n",
        __func__
        ));
    return Status;
  }

  // //
  // // Locate SPI Nor Flash protocol
  // //
  // DEBUG ((DEBUG_INFO, "%a[%d] Locate SPI Nor Flash protocol\n", __func__, __LINE__));
  // Status = gBS->LocateProtocol (
  //                 &gSophgoNorFlashProtocolGuid,
  //                 NULL,
  //                 (VOID **)&NorFlashProtocol
  //                 );
  // if (EFI_ERROR (Status)) {
  //   DEBUG ((
  //       DEBUG_ERROR,
  //       "%a: Cannot locate SPI Nor Flash protocol\n",
  //       __func__
  //       ));
  //   return Status;
  // }

  // //
  // // Setup SPI Flash Master Controller
  // //
  // NorFlash = NULL;
  // DEBUG ((DEBUG_INFO, "%a[%d] Setup SPI Flash Master Controller\n", __func__, __LINE__));
  // NorFlash = SpiMasterProtocol->SetupDevice (SpiMasterProtocol,
  //                                 NorFlash);
  // if (NorFlash == NULL) {
  //   DEBUG ((
  //       DEBUG_ERROR,
  //       "%a: Cannot allocate SPI Nor Flash device!\n",
  //       __func__
  //       ));
  //   return EFI_ABORTED;
  // }

  // DEBUG ((DEBUG_INFO, "%a[%d] Initialize SPIFMC\n", __func__, __LINE__));
  // Status = SpiMasterProtocol->Init (NorFlash);
  // if (EFI_ERROR (Status)) {
  //    DEBUG ((
  //       DEBUG_ERROR,
  //       "%a: Error while performing SPI flash probe!\n",
  //       __func__
  //       ));
  //   goto FlashProbeError;
  // }

  // //
  // // Read SPI Nor Flash ID
  // //
  // DEBUG ((DEBUG_INFO, "%a[%d] Read SPI Nor Flash ID\n", __func__, __LINE__));
  // Status = SpiNorGetFlashId (NorFlash, FALSE);
  // if (EFI_ERROR (Status)) {
  //    DEBUG ((
  //       DEBUG_ERROR,
  //       "%a: Read SPI Nor flash ID failed!\n",
  //       __func__
  //       ));
  //   return EFI_NOT_FOUND;
  // }

  // //
  // // Initialize SPI Nor Flash
  // //
  // DEBUG ((DEBUG_INFO, "%a[%d] Initialize SPI Nor Flash\n", __func__, __LINE__));
  // Status = SpiNorInit (NorFlashProtocol, NorFlash);
  // if (EFI_ERROR(Status)) {
  //   DEBUG ((
  //       DEBUG_ERROR,
  //       "%a: Initialize Nor Flash device failed!\n",
  //       __func__
  //       ));
  //   return EFI_DEVICE_ERROR;
  // }

  //
  // Initialize Nor Flash Instance
  //
  mNorFlashInstance = AllocateRuntimeZeroPool (sizeof (NOR_FLASH_INSTANCE));
  if (mNorFlashInstance == NULL) {
    DEBUG((
      DEBUG_ERROR,
      "SpiNor: Cannot allocate memory\n"
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Create DevicePath for SPI Nor Flash
  //
  // for (Index = 0; Index < mNorFlashDeviceCount; Index++) {
    mNorFlashInstance->NorFlashProtocol.Init            = SpiNorInit;
    mNorFlashInstance->NorFlashProtocol.GetFlashid      = SpiNorGetFlashId;
    mNorFlashInstance->NorFlashProtocol.ReadData        = SpiNorReadData;
    mNorFlashInstance->NorFlashProtocol.WriteData       = SpiNorWriteData;
    mNorFlashInstance->NorFlashProtocol.ReadStatus      = SpiNorReadStatus;
    mNorFlashInstance->NorFlashProtocol.WriteStatus     = SpiNorWriteStatus;
    mNorFlashInstance->NorFlashProtocol.Erase           = SpiNorErase;
    mNorFlashInstance->NorFlashProtocol.LoadImage       = SpiNorLoadImage;
    // mNorFlashInstance->NorFlashProtocol.BuildDevicePath = SpiNorBuildDevicePath;

    mNorFlashInstance->Signature = NOR_FLASH_SIGNATURE;

    NorFlashDevicePath = AllocateCopyPool (
                 sizeof (NOR_FLASH_DEVICE_PATH),
                 (VOID *)&mDevicePathTemplate
                 );
    if (NorFlashDevicePath == NULL) {
      DEBUG ((DEBUG_ERROR, "[%a]:[%dL] AllocatePool failed!\n", __func__, __LINE__));
      return EFI_OUT_OF_RESOURCES;
    }

    NorFlashDevicePath->Index = (UINT8) Index;

    mNorFlashInstance->DevicePath = NorFlashDevicePath;

    // CopyGuid (&mNorFlashInstance->DevicePath.Vendor.Guid, &gEfiCallerIdGuid);

    Status = gBS->InstallMultipleProtocolInterfaces (
                    &(mNorFlashInstance->Handle),
                    &gSophgoNorFlashProtocolGuid,
                    &(mNorFlashInstance->NorFlashProtocol),
                    &gEfiDevicePathProtocolGuid,
                    (EFI_DEVICE_PATH_PROTOCOL *)NorFlashDevicePath,
                  NULL
                    );
    if (EFI_ERROR (Status)) {
      DEBUG((
        DEBUG_ERROR,
        "SpiNor: Cannot install SPI flash protocol\n"
        ));
      goto ErrorInstallProto;
    }
  // }

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

// FlashProbeError:
//   SpiMasterProtocol->FreeDevice (NorFlash);

ErrorInstallProto:
  FreePool (mNorFlashInstance);

  return EFI_SUCCESS;
}
