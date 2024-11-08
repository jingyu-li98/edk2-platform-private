/** @file
*  OemMiscLib.c
*
*  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadFile.h>
#include <Protocol/DevicePath.h>

#include "IniParserUtil.h"
#include <Protocol/BlockIo.h>
#define EFI_FILE_NAME L"\\conf.ini"

typedef struct {
  CHAR8 *Name;
  UINTN Addr;
} TEST_CONFIG;

TEST_CONFIG *Config;
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
 IN VOID *FileBuffer
 )
{
  EFI_STATUS                 Status;
  EFI_DEVICE_PATH_PROTOCOL   *FilePath;
  UINTN                      FileBufferSize;
  EFI_HANDLE                 Handle;
  EFI_LOAD_FILE_PROTOCOL     *LoadFileProtocol;

  Handle = NULL;
  FileBufferSize = 0;
  //FileBuffer = NULL;

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
    FileBuffer = AllocatePages (FileBufferSize);
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
#if 1
STATIC
CHAR8 *
IniStrDup (
  IN CONST CHAR8  *String
  )
{
  return AllocateCopyPool (AsciiStrSize (String), String);
}
#endif
STATIC
INT32
TestHandler (
  IN       VOID  *User,
  IN CONST CHAR8 *Section,
  IN CONST CHAR8 *Name,
  IN CONST CHAR8 *Value
  )
{
  Config = (TEST_CONFIG *)User;

  #define MATCH(S, N) (AsciiStrCmp (Section, S) == 0 && AsciiStrCmp (Name, N) == 0)

  if (MATCH("devicetree", "name")) {
    Config->Name = IniStrDup (Value);
  } else {
    return 0;
  }

  return -1;
}

EFI_STATUS
IniParser (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  
  CONST CHAR8 *Header;
  CONST CHAR8 *Tail;
  CHAR8 *Eof;
  VOID  *FileBuffer;

  FileBuffer = NULL;
  Header = "[sophgo-config]";
  Tail = "[eof]";

  LoadFileFromExpandMediaDevice (FileBuffer);
#if 0

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
  if (AsciiStrnCmp(Header, (CONST CHAR8 *)FileBuffer, AsciiStrLen(Header))) {
    DEBUG ((
      DEBUG_ERROR,
      "conf.ini should start with \"%s\"\n",
      Header
      ));
			   ;
    return EFI_INVALID_PARAMETER;
  }

  Eof = AsciiStrStr((CONST CHAR8 *)FileBuffer, Tail);

  if (!Eof) {
    DEBUG ((
      DEBUG_ERROR,
      "conf.ini should terminated by \"%s\"\n",
      Tail
      ));
			   ;
    return EFI_INVALID_PARAMETER;
  }

  if (IniParseString((CONST CHAR8 *)FileBuffer, TestHandler, &Config) < 0
            || Config->Name == NULL) {
                return EFI_INVALID_PARAMETER;
  
  } 

  return EFI_SUCCESS;
}

