/** @file
 *
 *  The definitions of SPI NOR Flash commands and registers are from Linux Kernel.
 *
 *  Copyright (c) 2023, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __NOR_FLASH_PROTOCOL_H__
#define __NOR_FLASH_PROTOCOL_H__

#include <Include/Spi.h>

#define SPI_NOR_MAX_ID_LEN      6

//
// Flash opcodes.
//
#define SPINOR_OP_WRDI          0x04    /* Write disable */
#define SPINOR_OP_WREN          0x06    /* Write enable */
#define SPINOR_OP_RDSR          0x05    /* Read status register */
#define SPINOR_OP_WRSR          0x01    /* Write status register 1 byte */
#define SPINOR_OP_READ          0x03    /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST     0x0b    /* Read data bytes (high frequency) */

#define SPINOR_OP_PP            0x02    /* Page program (up to 256 bytes) */
#define SPINOR_OP_SE            0xd8    /* Sector erase (usually 64KiB) */
#define SPINOR_OP_RDID          0x9f    /* Read JEDEC ID */
#define SPINOR_OP_RDCR          0x35    /* Read configuration register */

//
// 4-byte address opcodes.
//
#define SPINOR_OP_READ_4B       0x13    /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST_4B  0x0c    /* Read data bytes (high frequency) */
#define SPINOR_OP_PP_4B         0x12    /* Page program (up to 256 bytes) */
#define SPINOR_OP_SE_4B         0xdc    /* Sector erase (usually 64KiB) */
#define SPINOR_OP_EN4B          0xb7    /* Enter 4-byte mode */
#define SPINOR_OP_EX4B          0xe9    /* Exit 4-byte mode */

//
// Status Register bits.
//
#define SR_WIP                  BIT0  /* Write in progress */
#define SR_WEL                  BIT1  /* Write enable latch */


extern EFI_GUID  gSophgoNorFlashProtocolGuid;

typedef struct _SOPHGO_NOR_FLASH_PROTOCOL SOPHGO_NOR_FLASH_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_GET_FLASH_ID)(
  IN  SPI_NOR                          *Nor,
  IN  BOOLEAN                           UseInRuntime
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_READ_DATA)(
  IN  SPI_NOR                          *Nor,
  IN  UINTN                            FlashAddress,
  IN  UINTN                            LengthInBytes,
  OUT UINT8                            *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_READ_STATUS)(
  IN  SPI_NOR                          *Nor,
  OUT UINT8                            *FlashStatus
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_WRITE_STATUS)(
  IN SPI_NOR                          *Nor,
  IN UINT8                            *FlashStatus,
  IN UINTN                            LengthInBytes
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_WRITE_DATA)(
  IN SPI_NOR                          *Nor,
  IN UINTN                            FlashAddress,
  IN UINTN                            LengthInBytes,
  IN UINT8                            *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_ERASE)(
  IN SPI_NOR                          *Nor,
  IN UINTN                            FlashAddress,
  IN UINTN                            Length
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_INIT)(
  IN SOPHGO_NOR_FLASH_PROTOCOL       *This,
  IN SPI_NOR                         *Nor
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_LOAD_IMAGE)(
  IN SPI_NOR                         *Nor,
  IN UINTN                           PartitionTableAddr,
  IN CONST UINT8                     *ImageName
  );

typedef
EFI_STATUS
(EFIAPI *SG_NOR_FLASH_PROTOCOL_BUILD_DEVICE_PATH)(
  // IN  SOPHGO_NOR_FLASH_PROTOCOL      *This,
  OUT EFI_DEVICE_PATH_PROTOCOL       **DevicePath
  );

struct _SOPHGO_NOR_FLASH_PROTOCOL {
  SG_NOR_FLASH_PROTOCOL_GET_FLASH_ID       GetFlashid;
  SG_NOR_FLASH_PROTOCOL_READ_DATA          ReadData;
  SG_NOR_FLASH_PROTOCOL_READ_STATUS        ReadStatus;
  SG_NOR_FLASH_PROTOCOL_WRITE_STATUS       WriteStatus;
  SG_NOR_FLASH_PROTOCOL_WRITE_DATA         WriteData;
  SG_NOR_FLASH_PROTOCOL_ERASE              Erase;
  SG_NOR_FLASH_PROTOCOL_INIT               Init;
  SG_NOR_FLASH_PROTOCOL_LOAD_IMAGE         LoadImage;
  SG_NOR_FLASH_PROTOCOL_BUILD_DEVICE_PATH  BuildDevicePath;
};

#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH                  Vendor;
  UINT8                               Index;
  EFI_DEVICE_PATH_PROTOCOL            End;
} NOR_FLASH_DEVICE_PATH;
#pragma pack()

typedef struct {
  SOPHGO_NOR_FLASH_PROTOCOL  NorFlashProtocol;
  UINTN                      Signature;
  EFI_HANDLE                 Handle;
  NOR_FLASH_DEVICE_PATH      *DevicePath;
  // EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
} NOR_FLASH_INSTANCE;

#endif // __NOR_FLASH_PROTOCOL_H__