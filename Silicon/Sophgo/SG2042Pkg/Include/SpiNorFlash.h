/** @file
 *
 *  Copyright (c) 2023, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/
#include <Include/Spi.h>
#ifndef __NOR_FLASH_PROTOCOL_H__
#define __NOR_FLASH_PROTOCOL_H__

typedef struct _SOPHGO_NOR_FLASH_PROTOCOL SOPHGO_NOR_FLASH_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_GET_FLASH_ID)(
  IN  SPI_NOR                          *Nor,
  IN  BOOLEAN UseInRuntime
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_READ_DATA)(
  IN  SPI_NOR                          *Nor,
  IN  UINT32                           FlashAddress,
  IN  UINT32                           LengthInBytes,
  OUT UINT8                            *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_READ_STATUS)(
  IN  SPI_NOR                          *Nor,
  OUT UINT8                            *FlashStatus
  );

/**
  Write the flash status register.

  This routine must be called at or below TPL_N OTIFY.
  This routine writes the flash part status register.

  @param[in] This           Pointer to an SPI_NOR_FLASH_PROTOCOL data
                            structure.
  @param[in] LengthInBytes  Number of status bytes to write.
  @param[in] FlashStatus    Pointer to a buffer containing the new status.

  @retval EFI_SUCCESS           The status write was successful.
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate the write buffer.

**/
typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_WRITE_STATUS)(
  IN SPI_NOR                          *Nor,
  IN UINT8                            *FlashStatus,
  IN UINT32                           LengthInBytes
  );

/**
  Write data to the SPI flash.

  This routine must be called at or below TPL_NOTIFY.
  This routine breaks up the write operation as necessary to write the data to
  the SPI part.

  @param[in] This           Pointer to an SPI_NOR_FLASH_PROTOCOL data
                            structure.
  @param[in] FlashAddress   Address in the flash to start writing
  @param[in] LengthInBytes  Write length in bytes
  @param[in] Buffer         Address of a buffer containing the data

  @retval EFI_SUCCESS            The data was written successfully.
  @retval EFI_INVALID_PARAMETER  Buffer is NULL, or
                                 FlashAddress >= This->FlashSize, or
                                 LengthInBytes > This->FlashSize - FlashAddress
  @retval EFI_OUT_OF_RESOURCES   Insufficient memory to copy buffer.

**/
typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_WRITE_DATA)(
  IN SPI_NOR                          *Nor,
  IN UINT32                           FlashAddress,
  IN UINT32                           LengthInBytes,
  IN UINT8                            *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_ERASE)(
  IN SPI_NOR                          *Nor,
  IN UINT32                           FlashAddress,
  IN UINT32                           BlockCount
  );

///
/// The SPI_NOR_FLASH_PROTOCOL exists in the SPI peripheral layer.
/// This protocol manipulates the SPI NOR flash parts using a common set of
/// commands. The board layer provides the interconnection and configuration
/// details for the SPI NOR flash part. The SPI NOR flash driver uses this
/// configuration data to expose a generic interface which provides the
/// following APls:
/// * Read manufacture and device ID
/// * Read data
/// * Read data using low frequency
/// * Read status
/// * Write data
/// * Erase 4 KiB blocks
/// * Erase 32 or 64 KiB blocks
/// * Write status
/// The SPI_NOR_FLASH_PROTOCOL also exposes some APls to set the security
/// features on the legacy SPI flash controller.
///
struct _SOPHGO_NOR_FLASH_PROTOCOL {
  ///
  /// Read the 3 byte manufacture and device ID from the SPI flash.
  ///
  SG_NOR_FLASH_PROTOCOL_GET_FLASH_ID    GetFlashid;

  ///
  /// Read data from the SPI flash.
  ///
  SG_NOR_FLASH_PROTOCOL_READ_DATA       ReadData;

  ///
  /// Read the flash status register.
  ///
  SG_NOR_FLASH_PROTOCOL_READ_STATUS     ReadStatus;

  ///
  /// Write the flash status register.
  ///
  SG_NOR_FLASH_PROTOCOL_WRITE_STATUS    WriteStatus;

  ///
  /// Write data to the SPI flash.
  ///
  SG_NOR_FLASH_PROTOCOL_WRITE_DATA      WriteData;

  ///
  /// Efficiently erases one or more 4KiB regions in the SPI flash.
  ///
  SG_NOR_FLASH_PROTOCOL_ERASE           Erase;
};

extern EFI_GUID  gSophgoNorFlashProtocolGuid;


typedef struct {
  SOPHGO_NOR_FLASH_PROTOCOL  NorFlashProtocol;
  UINTN                      Signature;
  EFI_HANDLE                 Handle;
} NOR_FLASH_INSTANCE;


#endif // __NOR_FLASH_PROTOCOL_H__
