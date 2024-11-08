/** @file
*  OemMiscLib.c
*
*  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <CpuConfigNVDataStruc.h>
#include <NVParamDef.h>
#include <PiPei.h>
#include <Uefi.h>
#include <Library/AmpereCpuLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/OemMiscLib.h>
#include <Library/PrintLib.h>
#include <Guid/PlatformInfoHob.h>

#include "IniParserUtil.h"
#include <Protocol/BlockIo.h>
#define EFI_FILE_NAME L"\\conf.ini"

/**
  Get the conf.ini file path from a media device

**/
EFI_DEVICE_PATH_PROTOCOL *
ExpandMediaDeviceFilePath (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                Handle;
  EFI_HANDLE                *BlockIoHandles;
  EFI_HANDLE                *SimpleFileSystemHandles;
  EFI_BLOCK_IO_PROTOCOL     *BlockIo;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  UINTN                     NumberBlockIoHandles;
  UINTN                     NumberSimpleFileSystemHandles;
  UINTN                     Index;
  UINTN                     TempSize;
  UINTN                     Size;
  VOID                      *Buffer;

  //
  // Step 1. Get the device path
  //
  Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiBlockIoProtocolGuid,
                 NULL,
                 &NumberBlockIoHandles,
                 &BlockIoHandles
                 );

  if (EFI_ERROR (Status)) {
    //
    // This is not an error, just an informative condition.
    //
    DEBUG ((
      DEBUG_ERROR,
      "%a: %g: %r\n",
      __func__,
      gEfiBlockIoProtocolGuid,
      Status
      ));
    return NULL;
  }
  ASSERT (NumberBlockIoHandles > 0);

  for (Index = 0; Index < NumberBlockIoHandles; Index++) {
    //
    // Get the device path of SimpleFileSystem handle
    //
    TempDevicePath = DevicePathFromHandle (BlockIoHandles[Index]);
  }

  //
  // Step 2. Check whether the device is connected
  //
  Status = gBS->LocateDevicePath (
                 &gEfiBlockIoProtocolGuid,
                 &TempDevicePath,
                 &Handle
                 );
  ASSERT_EFI_ERROR (Status);

  gBS->ConnectController (Handle, NULL, NULL, TRUE);

  //
  // Issue a dummy read to the device to check for media change.
  // When the removable media is changed, any Block IO read/write will
  // cause the BlockIo protocol be reinstalled and EFI_MEDIA_CHANGED is
  // returned. After the Block IO protocol is reinstalled, subsequent
  // Block IO read/write will success.
  //
  Status = gBS->HandleProtocol (
                 Handle,
                 &gEfiBlockIoProtocolGuid,
                 (VOID **)&BlockIo
                 );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  Buffer = AllocatePool (BlockIo->Media->BlockSize);
  if (Buffer != NULL) {
    BlockIo->ReadBlocks (
               BlockIo,
               BlockIo->Media->MediaId,
               0,
               BlockIo->Media->BlockSize,
               Buffer
               );
    FreePool (Buffer);
  }

  //
  // Step 3. Detect the ASpeedAst2600Gop.efi file from device
  //
  FilePath = NULL;
  Size = GetDevicePathSize (TempDevicePath) - END_DEVICE_PATH_LENGTH;
  gBS->LocateHandleBuffer (
         ByProtocol,
         &gEfiSimpleFileSystemProtocolGuid,
         NULL,
         &NumberSimpleFileSystemHandles,
         &SimpleFileSystemHandles
         );
  for (Index = 0; Index < NumberSimpleFileSystemHandles; Index++) {
    //
    // Get the device path size of SimpleFileSystem handle
    //
    TempDevicePath = DevicePathFromHandle (SimpleFileSystemHandles[Index]);
    TempSize = GetDevicePathSize (TempDevicePath) - END_DEVICE_PATH_LENGTH;
    //
    // Check whether the device path of boot option is part of the SimpleFileSystem handle's device path
    //
    if ((Size <= TempSize) && (CompareMem (TempDevicePath, TempDevicePath, Size) == 0)) {
      FilePath = FileDevicePath (SimpleFileSystemHandles[Index], EFI_FILE_NAME);
    }
  }

  if (BlockIoHandles != NULL) {
    FreePool (BlockIoHandles);
  }

  if (SimpleFileSystemHandles != NULL) {
    FreePool (SimpleFileSystemHandles);
  }

  return FilePath;
}

/**
  load conf.ini into memory

**/
EFI_STATUS
LoadFileFromExpandMediaDevice (
 VOID
 )
{
  EFI_STATUS                 Status;
  EFI_DEVICE_PATH_PROTOCOL   *FilePath;
  VOID                       *FileBuffer;
  UINTN                      FileBufferSize;
  EFI_HANDLE                 Handle;
  EFI_LOAD_FILE_PROTOCOL     *LoadFileProtocol;

  Handle = NULL;
  FileBufferSize = 0;
  FileBuffer = NULL;

  //
  // Get the file path
  //
  FilePath = ExpandMediaDeviceFilePath ();
  if (FilePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Attempt to access the file via LoadFile interfface
  //
  Status = gBS->HandleProtocol (
		  Handle,
		  &gEfiLoadFileProtocolGuid,
		  (VOID **)&LoadFileProtocol
		  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a[%d]: Get Load File Protocol failed!\n",
      __func__,
      __LINE__
      ));
	
    return Status;
  }

  Status = LoadFileProtocol->LoadFile (
		 LoadFileProtocol,
		 FilePath,
		 TRUE,
		 &FileBufferSize,
		 FileBuffer
		 );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    FileBuffer = AllocatePages (BufferSize),
    if (FileBuffer == NULL) {
      DEBUG ((
        DEBUG_ERROR,
        "%a[%d]: Allocate Pages failed!\n",
        __func__,
        __LINE__
        ));

      return EFI_OUT_OF_RESOURCES;
    } else {
      Status = LoadFileProtocol->LoadFile (
		   LoadFileProtocol,
		   FilePath,
		   TRUE,
		   &FileBufferSize,
		   FileBuffer
		   );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "%a[%d]: Load File failed!\n",
          __func__,
          __LINE__
          ));

        return Status;
      }
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
Handler (
  IN       VOID  *User,
  IN CONST CHAR8 *Section,
  IN CONST CHAR8 *Name,
  IN CONST CHAR8 *Value
  )
{
  Struct GlobalConfig *Pconfig = (struct GlobalConfig *)User;

  #define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

  if (MATCH("devicetree", "name")) {
    Pconfig->Name = strdup (Value);
  } else if (MATCH("devicetree", "addr")) {
    Pconfig->Addr = strtoul (Value, NULL, 16);
  } else {
    return EFI_SUCCESS;
  }

  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
IniParserLib (
  VOID
  )
{

  LoadFileFromExpandMediaDevice ();
#if 0
  CHAR ReadBuffer[1024];

  //
  // We have a limitation for flash DMMR mode. It can only read out data
  // below 16MB. But it is enough for a configuration file.
  //
  if (mango_load_conf_sd(read_buf, sizeof(read_buf))) {
    if (mango_load_conf_spi(read_buf, sizeof(read_buf)))
                        return -1;
        }


        if (ini_parse_string((const char*)read_buf, handler, &config) < 0
            || config.name == NULL)
                return -1;
#endif
        return EFI_SUCCESS;
}

