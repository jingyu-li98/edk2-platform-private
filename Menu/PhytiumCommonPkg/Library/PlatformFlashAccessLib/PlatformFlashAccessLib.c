/** @file
Platform flash device access library.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PlatformFlashAccessLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/SpiNorFlashProtocol.h>


STATIC EFI_NORFLASH_DRV_PROTOCOL *mFlash;

/**
  Update firmware data to flash.

  @param[in]  Address  The address of flash device to be accessed.
  @param[in]  Buffer   The pointer to the data buffer.
  @param[in]  Length   The length of data buffer in bytes.
  @param[in]  String   The Pointer to string to be printed.

  @retval  EFI_SUCCESS  The operation returns successfully.
**/
EFI_STATUS
UpdateFlash (
  IN UINTN                  Address,
  IN UINT8                  *Buffer,
  IN UINTN                  Length,
  IN CHAR16                 *String
)
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       Count;
  UINTN       RemainingBytes;
  VOID        *TempBuffer;

  TempBuffer = AllocatePool (SIZE_64KB);
  Status = EFI_SUCCESS;
  RemainingBytes = Length;
  Count  = (Length / SIZE_64KB);

  //
  // make sure Address & Length is 64k aligned
  //
  if (((Address % SIZE_64KB) != 0) || ((Length % SIZE_64KB) != 0)) {
    Count++;
    Address = Address & (~0xFFFF);
  }

  Print (L"%s%02d%%%", String, 1);
  for (Index = 0; Index < Count; Index++) {
    if (TempBuffer != NULL) {
      Status = mFlash->Read (
                 Address + Index * SIZE_64KB,
                 TempBuffer,
                 SIZE_64KB
                 );
      if (EFI_ERROR (Status)) {
        Print (L"\r%s Fail!\n", String);
        goto ProExit;
      }

      if (CompareMem (
            TempBuffer,
            Buffer + Index * SIZE_64KB,
            SIZE_64KB
            ) == 0) {
        Print (L"\r%s %02d%", String, ((Index + 1) * 100) / Count);
        continue;
      }
    }

    Status = mFlash->Erase (Address + Index * SIZE_64KB, SIZE_64KB);
    if (EFI_ERROR (Status)) {
      Print (L"\r%s Fail!\n", String);
      goto ProExit;
    }

    if ((RemainingBytes > 0) && (RemainingBytes < SIZE_64KB)) {
      CopyMem (
        TempBuffer,
        Buffer + Index * SIZE_64KB,
        RemainingBytes
        );
    } else {
      CopyMem (
        TempBuffer,
        Buffer + Index * SIZE_64KB,
        SIZE_64KB
        );
    }

    Status = mFlash->Write (
                       Address + Index * SIZE_64KB,
                       TempBuffer,
                       SIZE_64KB
                       );
    if (EFI_ERROR(Status)) {
      Print (L"\r%s Fail!\n", String);
      goto ProExit;
    }
    Print (L"\r%s %02d%", String, ((Index + 1) * 100) / Count);
    RemainingBytes -= SIZE_64KB;
  }
  Print (L"\r%s Success!\n", String);

ProExit:
  if (TempBuffer != NULL) {
    FreePool (TempBuffer);
  }

  return Status;
}


/**
  Perform flash write operation with progress indicator.  The start and end
  completion percentage values are passed into this function.  If the requested
  flash write operation is broken up, then completion percentage between the
  start and end values may be passed to the provided Progress function.  The
  caller of this function is required to call the Progress function for the
  start and end completion percentage values.  This allows the Progress,
  StartPercentage, and EndPercentage parameters to be ignored if the requested
  flash write operation can not be broken up

  @param[in] FirmwareType      The type of firmware.
  @param[in] FlashAddress      The address of flash device to be accessed.
  @param[in] FlashAddressType  The type of flash device address.
  @param[in] Buffer            The pointer to the data buffer.
  @param[in] Length            The length of data buffer in bytes.
  @param[in] Progress          A function used report the progress of the
                               firmware update.  This is an optional parameter
                               that may be NULL.
  @param[in] StartPercentage   The start completion percentage value that may
                               be used to report progress during the flash
                               write operation.
  @param[in] EndPercentage     The end completion percentage value that may
                               be used to report progress during the flash
                               write operation.

  @retval EFI_SUCCESS           The operation returns successfully.
  @retval EFI_WRITE_PROTECTED   The flash device is read only.
  @retval EFI_UNSUPPORTED       The flash device access is unsupported.
  @retval EFI_INVALID_PARAMETER The input parameter is not valid.
**/
EFI_STATUS
EFIAPI
PerformFlashWriteWithProgress (
  IN PLATFORM_FIRMWARE_TYPE                         FirmwareType,
  IN EFI_PHYSICAL_ADDRESS                           FlashAddress,
  IN FLASH_ADDRESS_TYPE                             FlashAddressType,
  IN VOID                                           *Buffer,
  IN UINTN                                          Length,
  IN EFI_FIRMWARE_MANAGEMENT_UPDATE_IMAGE_PROGRESS  Progress,        OPTIONAL
  IN UINTN                                          StartPercentage,
  IN UINTN                                          EndPercentage
  )
{
  UINTN               Index;
  UINTN               SectorNum;
  EFI_STATUS          Status;

  mFlash = NULL;
  // Verify Firmware data
  if (FlashAddressType != FlashAddressTypeAbsoluteAddress) {
    DEBUG ((DEBUG_ERROR,
      "%a: only FlashAddressTypeAbsoluteAddress supported\n",
      __FUNCTION__));

    return EFI_INVALID_PARAMETER;
  }

  if (FirmwareType != PlatformFirmwareTypeSystemFirmware) {
    DEBUG ((DEBUG_ERROR,
      "%a: only PlatformFirmwareTypeSystemFirmware supported\n",
      __FUNCTION__));

    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gSpiNorFlashProtocolGuid,
                  NULL,
                  (VOID **)&mFlash);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "Cannot locate NorFlash protocol.\n"
      ));
    return Status;
  }

  Status = mFlash->Initialization ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((DEBUG_INFO, "%a Progress=%x.\n", __func__, Progress));

  //
  // Erase & Write
  //
  SectorNum = Length / SIZE_64KB;
  if (Progress != NULL) {
    for (Index = 0; Index < SectorNum; Index++){
      Progress (StartPercentage + ((Index * (EndPercentage - StartPercentage)) / SectorNum));
    }
  }

  Status = UpdateFlash (
             FlashAddress,
             Buffer,
             Length,
             L"Updating firmware......."
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UpdateFlash Failed!\n"));
    goto Error;
  }

  if (Progress != NULL) {
    Progress (EndPercentage);
  }

  DEBUG ((DEBUG_INFO, "UpdateFlash Success!\n"));

Error:

  return Status;
}

/**
  Perform flash write operation.

  @param[in] FirmwareType      The type of firmware.
  @param[in] FlashAddress      The address of flash device to be accessed.
  @param[in] FlashAddressType  The type of flash device address.
  @param[in] Buffer            The pointer to the data buffer.
  @param[in] Length            The length of data buffer in bytes.

  @retval EFI_SUCCESS           The operation returns successfully.
  @retval EFI_WRITE_PROTECTED   The flash device is read only.
  @retval EFI_UNSUPPORTED       The flash device access is unsupported.
  @retval EFI_INVALID_PARAMETER The input parameter is not valid.
**/
EFI_STATUS
EFIAPI
PerformFlashWrite (
  IN PLATFORM_FIRMWARE_TYPE       FirmwareType,
  IN EFI_PHYSICAL_ADDRESS         FlashAddress,
  IN FLASH_ADDRESS_TYPE           FlashAddressType,
  IN VOID                         *Buffer,
  IN UINTN                        Length
  )
{
  return PerformFlashWriteWithProgress (
           FirmwareType,
           FlashAddress,
           FlashAddressType,
           Buffer,
           Length,
           NULL,
           0,
           0
           );
}

/**
  Platform Flash Access Lib Constructor.

  @param[in]  ImageHandle       The firmware allocated handle for the EFI image.
  @param[in]  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS  Constructor returns successfully.
**/
EFI_STATUS
EFIAPI
PlatformFlashAccessLibEntryPoint (
  VOID
  )
{
  return EFI_SUCCESS;
}

