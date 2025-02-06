/** @file
  Platform flash device access library for Marvell Armada 7k8k Platforms

  Copyright (c) 2018 Marvell International Ltd.<BR>

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

#include <Include/Spifmc.h>
#include <Include/SpiNorFlash.h>

STATIC SPI_NOR                             *Nor;
STATIC SOPHGO_NOR_FLASH_PROTOCOL           *NorFlashProtocol;
STATIC SOPHGO_SPI_MASTER_PROTOCOL          *SpiMasterProtocol;

STATIC
EFI_STATUS
NorFlashProbe (
  VOID
  )
{
  EFI_STATUS  Status;

  //
  // Read Nor Flash ID
  //
  Status = NorFlashProtocol->GetFlashid (Nor, TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Read Nor flash ID failed!\n",
      __func__
      ));
    return EFI_NOT_FOUND;
  }

  //
  // Initialize Nor Flash
  //
  Status = NorFlashProtocol->Init (NorFlashProtocol, Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Cannot initialize flash device\n",
      __func__
      ));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
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
  EFI_STATUS  Status;
  UINTN       BlockSize;
  UINTN       Count;
  UINTN       Index;

  BlockSize = Nor->Info->SectorSize;
  Count = Length / BlockSize;

  //
  // Verify Firmware data
  //
  if (FlashAddressType != FlashAddressTypeAbsoluteAddress) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): only FlashAddressTypeAbsoluteAddress supported\n",
      __func__
      ));

    return EFI_INVALID_PARAMETER;
  }

  if (FirmwareType != PlatformFirmwareTypeSystemFirmware) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): only PlatformFirmwareTypeSystemFirmware supported\n",
      __func__
      ));

    return EFI_INVALID_PARAMETER;
  }

  //
  // Locate SPI Master protocol
  //
  Status = gBS->LocateProtocol (
                  &gSophgoSpiMasterProtocolGuid,
                  NULL,
                  (VOID *)&SpiMasterProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Cannot locate SPI Master protocol\n",
      __func__
      ));
    return Status;
  }

  //
  // Locate Nor Flash protocol
  //
  Status = gBS->LocateProtocol (
                  &gSophgoNorFlashProtocolGuid,
                  NULL,
                  (VOID *)&NorFlashProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Cannot locate Nor Flash protocol\n",
      __func__
      ));
    return Status;
  }

  //
  // Setup and probe Nor flash
  //
  Nor = SpiMasterProtocol->SetupDevice (
                  SpiMasterProtocol,
                  Nor,
		  0
                  );

  if (Nor == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Nor Flash not found!\n",
      __func__
      ));
    Status = EFI_NOT_FOUND;
    goto Error;
  }

  Status = NorFlashProbe ();
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Error while performing Nor flash probe [Status=%r]\n",
      __func__,
      Status
      ));
    goto Error;
  }

  //
  // Update firmware image in flash
  //
  DEBUG ((DEBUG_INFO, "%a Progress=%x.\n", __func__, Progress));

  //
  // make sure FlashAddress & Length is BlockSize align
  //
  if (((FlashAddress % BlockSize) != 0)
	|| ((Length % BlockSize) != 0) ) {
    Count ++;
    FlashAddress = FlashAddress & (~ (BlockSize - 1));
  }
  //
  // Erase & Write
  //
  for (Index = 0; Index < Count; Index ++) {
    if (Progress != NULL) {
      Progress (StartPercentage +
	       ((Index * (EndPercentage - StartPercentage)) / Count));
    }

    Status = NorFlashProtocol->Erase (
		    Nor,
		    FlashAddress + Index * BlockSize,
		    BlockSize
		    );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a(): Erase Flash Failed!\n",
        __func__
        ));
      goto Error;
    }

    Status = NorFlashProtocol->WriteData (
		    Nor,
		    FlashAddress + Index * BlockSize,
		    BlockSize,
		    (UINT8 *)Buffer + Index * BlockSize
		    );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a(): Write Flash Failed!\n",
        __func__
        ));
      goto Error;
    }
  }

  if (Progress != NULL) {
    Progress (EndPercentage);
  }

  Status = EFI_SUCCESS;

Error:
  if (Nor) {
    SpiMasterProtocol->FreeDevice (Nor);
  }

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
