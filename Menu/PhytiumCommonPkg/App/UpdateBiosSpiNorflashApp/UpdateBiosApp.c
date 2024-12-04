/** @file
Update Bios operation.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Guid/FileInfo.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/PhytiumPowerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SpiNorFlashProtocol.h>

CHAR16 *BiosNames[2] = {L"BIOS.ROM" , L"BIOS.FD"};

#define EFI_BIOS_HEAD_SIGNATURE SIGNATURE_32 ('B', 'I', 'O', 'S')
#define EFI_BIOS_VENDOR         SIGNATURE_32 ('P', 'H', 'Y', 'T')

typedef struct {
  UINT8     Year;
  UINT8     Month;
  UINT8     Day;
  UINT8     Hour;
  UINT8     Minute;
  UINT8     Second;
} BIOS_RELEASE_TIME;

typedef struct {
  UINT32               Signature;
  UINT32               BiosVendor;
  UINT16               BiosMajorVersion;
  UINT16               BiosMinorVersion;
  BIOS_RELEASE_TIME    Time;
  UINT8                BiosCheckSum[16];
} EFI_UPDATE_FIRMWARE_HEADER;


EFI_HII_HANDLE              gUpdateBiosHandle;
EFI_NORFLASH_DRV_PROTOCOL   *mFlash = NULL;

/**
  Show  copy rights and warning.

  @retval    Null.
**/
EFI_STATUS
ShowCopyRightsAndWarning (
  VOID
  )
{
  UINTN   Columns, Rows;
  UINTN   StringLen;
  UINTN   Index;
  CHAR16  *String[] = {
    L"************************************************************\n",
    L"*                  Phytium Flash Update                    *\n",
    L"*        Copyright(C) 2015-2023, Phytium Co.,Ltd.          *\n",
    L"*                  All rights reserved                     *\n",
    L"************************************************************\n"
  };

  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &Columns, &Rows);
  StringLen = StrLen(String[0]);
  Columns   = (Columns - StringLen) / 2;
  Rows      = 0;
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->EnableCursor (gST->ConOut, FALSE);
  for (Index = 0; Index < 5; Index ++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, Columns, Rows ++);
    Print (String[Index]);
  }
  Print(L"\n");

  return EFI_SUCCESS;
}

/**
  Read file from file system.

  @param[in]    RootFile      A pointer to EFI_FILE_PROTOCOL.
  @param[in]    FilePathName  File name to read.
  @param[out]   OutFileData   File data after reading.
  @param[out]   OutFileSize   File size.

  @retval       EFI_OUT_OF_RESOURCES
                EFI_SUCCESS
**/
STATIC
EFI_STATUS
ReadFileInFs (
  IN  EFI_FILE_PROTOCOL  *RootFile,
  IN  CHAR16             *FilePathName,
  OUT UINT8              **OutFileData,
  OUT UINTN              *OutFileSize
  )
{
  EFI_STATUS          Status;
  EFI_FILE_PROTOCOL   *File;
  EFI_FILE_INFO       *FileInfo;
  UINTN               FileInfoSize;
  UINT8               *FileData;
  UINTN               FileSize;

  File = NULL;
  FileSize = 0;
  Status = RootFile->Open (
                       RootFile,
                       &File,
                       FilePathName,
                       EFI_FILE_MODE_READ,
                       0
                       );
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_INFO, "LXQ FILE NOT FOUND!!!\n"));
    return EFI_NOT_FOUND;
    //goto ProcExit;
  }

  FileInfo = NULL;
  FileInfoSize = 0;
  Status = File->GetInfo (
                   File,
                   &gEfiFileInfoGuid,
                   &FileInfoSize,
                   FileInfo
                   );
  if(Status == EFI_BUFFER_TOO_SMALL) {
    FileInfo = AllocatePool(FileInfoSize);
    if(FileInfo == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      Status = File->GetInfo (
                       File,
                       &gEfiFileInfoGuid,
                       &FileInfoSize,
                       FileInfo
                       );
    }
  }
  if(EFI_ERROR(Status)) {
    goto ProcExit;
  }

  if(FileInfo->Attribute & EFI_FILE_DIRECTORY) {
    Status = EFI_INVALID_PARAMETER;
    goto ProcExit;
  }

  FileSize = (UINTN)FileInfo->FileSize;
  FileData = AllocatePages (EFI_SIZE_TO_PAGES(FileSize));
  if(FileData == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }

  Status = File->Read(File, &FileSize, FileData);
  if(EFI_ERROR(Status)) {
    goto ProcExit;;
  }

  *OutFileData = FileData;
  *OutFileSize = FileSize;

ProcExit:
  if (EFI_ERROR(Status) && FileData != NULL) {
    FreePages(FileData, EFI_SIZE_TO_PAGES(FileSize));
  }
  if (FileInfo != NULL) {
    FreePool(FileInfo);
  }
  if (File != NULL) {
    File->Close(File);
  }
  return Status;
}

/**
  Read bios form file system.

  @param[in]    BiosFileName    Bios file name to read.
  @param[out]   BiosData        A pointer to bios data after reading.
  @param[out]   BiosSize        Bios data size.

  @retval       EFI_NOT_FOUND
                EFI_SUCCESS
**/
EFI_STATUS
ReadBiosFdInFs (
  IN  CHAR16  *BiosFileName,
  OUT UINT8   **BiosData,
  OUT UINTN   *BiosSize
  )
{
  EFI_STATUS                       Status;
  UINTN                            Index;
  UINTN                            HandleCount;
  EFI_HANDLE                       *Handles;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *FS;
  EFI_FILE_PROTOCOL                *RootFile;
  EFI_DEVICE_PATH_PROTOCOL         *DevPath;
  BOOLEAN                          IsUsb;

  Handles = NULL;
  RootFile = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );
  if(EFI_ERROR (Status) || HandleCount == 0) {
    Status = EFI_NOT_FOUND;
    goto ProcExit;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    IsUsb = FALSE;
    Status = gBS->HandleProtocol (
                    Handles[Index],
                    &gEfiDevicePathProtocolGuid,
                    (VOID**)&DevPath
                    );
    if (!EFI_ERROR(Status)) {
      while (!IsDevicePathEnd (DevPath)) {
        if ((DevicePathType (DevPath) == MESSAGING_DEVICE_PATH) &&
           (DevicePathSubType (DevPath) == MSG_USB_DP)) {
           IsUsb = TRUE;
           break;
        }
        DevPath = NextDevicePathNode (DevPath);
      }
    }

    if (!IsUsb) {
      continue;
    }

    Status = gBS->HandleProtocol (
                    Handles[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID**)&FS
                    );
    ASSERT (!EFI_ERROR (Status));

    if (RootFile != NULL) {
      RootFile->Close(RootFile);
      RootFile = NULL;
    }
    Status = FS->OpenVolume (FS, &RootFile);
    if(EFI_ERROR (Status)) {
      DEBUG((DEBUG_ERROR, "OpenRootFile Error:%r\n", Status));
      continue;
    }

    Status = ReadFileInFs (RootFile, BiosFileName, BiosData, BiosSize);
    if(!EFI_ERROR(Status)) {
      break;
    }
  }

  if(Index >= HandleCount) {
    Status = EFI_NOT_FOUND;
    goto ProcExit;
  }

ProcExit:
  if (Handles != NULL) {
    FreePool (Handles);
  }
  if (RootFile != NULL) {
    RootFile->Close (RootFile);
  }
  return Status;
}

/**
  Find and create information in uni file.

  @param[in]    StringToken1    String token 1.
  @param[in]    StringToken2    String token 2.

  @retval       TRUE
**/
BOOLEAN
InvalidInformation (
  IN UINT16  StringToken1,
  IN UINT16  StringToken2
  )
{
  CHAR16         Str1[64];
  CHAR16         Str2[64];
  CHAR16         *HiiFormatString;
  EFI_INPUT_KEY  InputKey;
  UINTN          EventIndex;

  DEBUG((DEBUG_INFO, "%a()\n", __FUNCTION__));

  HiiFormatString = HiiGetString (gUpdateBiosHandle, StringToken1, NULL);
  UnicodeSPrint (Str1, sizeof (Str1), L"%s", HiiFormatString);
  if (HiiFormatString != NULL) {
    FreePool (HiiFormatString);
  }
  HiiFormatString = HiiGetString (gUpdateBiosHandle, StringToken2, NULL);
  UnicodeSPrint (Str2, sizeof (Str2), L"%s", HiiFormatString);
  if (HiiFormatString != NULL) {
    FreePool (HiiFormatString);
  }

  CreatePopUp (
    EFI_LIGHTGRAY | EFI_BACKGROUND_RED,
    NULL,
    Str1,
    Str2,
    NULL
    );

  while (1) {
    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &EventIndex);
    gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);
    if (InputKey.ScanCode == SCAN_NULL && InputKey.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      break;
    }
  }

  return TRUE;
}

/**
  Update bios through qspi flash.

  @param[in]    Address    Address to update.
  @param[in]    Buffer     A pointer to update bios data.
  @param[in]    Size       Size of update bios to update.
  @param[in]    String     Start info to print.

  @retval       EFI_SUCCESS    Success.
                Other          Failed.
**/
EFI_STATUS
UpdateSpi (
  IN UINTN                     Address,
  IN UINT8                     *Buffer,
  IN UINTN                     Size,
  IN CHAR16                    *String
  )
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       Count;
  VOID        *TempBuffer;


  //UINTN       VariableBase;
  //UINT32      VariableSize;
  //VariableBase = PcdGet64(PcdFlashNvStorageVariableBase64);
  //VariableSize = PcdGet32(PcdFlashNvStorageVariableSize);


  TempBuffer = NULL;
  TempBuffer = AllocatePool (SIZE_64KB);
  Status = EFI_SUCCESS;
  Count  = (Size / SIZE_64KB);
  //
  // make sure Address & Size is 64k align
  //
  if (((Address % SIZE_64KB) >=0) || ((Size % SIZE_64KB) >=0) ) {
    Count ++;
    Address = Address & (~0xFFFF);
  }

  Print (L"%s%02d%%%", String, 1);
  for (Index = 0; Index < Count; Index ++) {
    //Skip the Variable range
    //if(((VariableBase / SIZE_64KB) <= Index) && (Index < ((VariableBase + VariableSize) / SIZE_64KB)))
    //    continue;

    if (TempBuffer != NULL) {
      mFlash->Read (Address + Index * SIZE_64KB, TempBuffer, SIZE_64KB);
      if (CompareMem (TempBuffer, Buffer + Index * SIZE_64KB, SIZE_64KB) == 0) {
        Print (L"\r%s %02d%%",String, ((Index + 1) * 100) / Count);
        continue;
      }
    }
    Status = mFlash->Erase (Address + Index * SIZE_64KB, SIZE_64KB);
    if (EFI_ERROR(Status)) {
      Print(L"\r%s Fail!\n",String);
      goto ProExit;
    }
    Status = mFlash->Write (Address + Index * SIZE_64KB, Buffer + Index * SIZE_64KB, SIZE_64KB);
    if (EFI_ERROR(Status)) {
      Print(L"\r%s Fail!\n",String);
      goto ProExit;
    }
    Print(L"\r%s %02d%%",String, ((Index + 1) * 100) / Count);
  }
  Print(L"\r%s Success!\n",String);

ProExit:
  if (TempBuffer != NULL) {
    FreePool (TempBuffer);
  }
  return Status;
}

/**
  Check the header of bios image.

  @param[in]    Buffer    A pointer to update bios data.
  @param[in]    FileSize  Size of bios.

  @retval    EFI_SUCCESS    Success.
             Other          Failed.
**/
EFI_STATUS
CheckBiosImage (
  IN UINT8        *Buffer,
  IN UINTN        FileSize
  )
{
  EFI_STATUS    Status;
  UINT64        SignOffset;
  UINT32        Signature;
  UINTN         Md5ContextSize;
  VOID          *Md5Ctx;
  UINT8         HashValue[16];
  UINT8         MD5Value[16];
  //
  // check BIOS.Signature.
  // BIOS header Signature offset is 0
  //
  Status = EFI_SUCCESS;
  SignOffset = 0;
  Signature  = *(UINT32*)(Buffer + SignOffset);
  SignOffset = 0x12;

  CopyMem(MD5Value, (Buffer + SignOffset), 16);
  if (Signature != EFI_BIOS_HEAD_SIGNATURE) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG ((DEBUG_ERROR , "Bios Signature isn't right ,please check the bios if correct or not !\n"));
    InvalidInformation (STRING_TOKEN(STR_CHECK_BIOS_SIGNATURE_ERROR), STRING_TOKEN(STR_PRESS_ENTER_CONTINUE));
    goto ProExit;
  }

  Md5ContextSize = Md5GetContextSize ();
  Md5Ctx = AllocatePool (Md5ContextSize);
  if (Md5Ctx == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  if (!Md5Init (Md5Ctx)) {
    goto ProExit;
  }
  if (!Md5Update (Md5Ctx,(Buffer+0x30), FileSize - 0x30)) {
    goto ProExit;
  }
  if (Md5Final (Md5Ctx, HashValue)) {
    Status = EFI_SUCCESS;
  }

  if (CompareMem(HashValue, MD5Value, 16) == 0) {
    Status = EFI_SUCCESS;
  } else {
    DEBUG ((DEBUG_ERROR,"the HashValue is not identical\n"));
    Status = EFI_INVALID_PARAMETER;
    InvalidInformation (STRING_TOKEN(STR_CHECK_BIOS_MD5_ERROR), STRING_TOKEN(STR_PRESS_ENTER_CONTINUE));
    goto ProExit;
  }

ProExit:
  return Status;
}

/**
  Entry of the app.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
UpdateBiosAppEntry (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  UINT8                    *BiosData;
  UINTN                    BiosSize;
  UINT8                    *Biosaddress;
  UINTN                    Biossize;
  EFI_STATUS               Status;
  UINTN                    Attribute;
  UINTN                    VariableBase;
  UINT32                   VariableSize;

  BiosData = NULL;
  BiosSize= 0;
  Biosaddress = NULL;
  Biossize = 0;

  VariableBase = PcdGet64(PcdFlashNvStorageVariableBase64);
  VariableSize = PcdGet32(PcdFlashNvStorageVariableSize);

  //
  // turn off the watchdog timer
  //
  gBS->SetWatchdogTimer (0, 0, 0, NULL);

  Status = gBS->LocateProtocol (&gSpiNorFlashProtocolGuid, NULL, (VOID*) &mFlash);
  if (EFI_ERROR(Status)) {
    Print(L"  Locate SpiFlashProtocol failed!\n");
    goto ProcExit;
  }

  Attribute = EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK;
  gST->ConOut->SetAttribute (gST->ConOut, Attribute);

  gUpdateBiosHandle = HiiAddPackages (
                      &gEfiCallerIdGuid,
                      gImageHandle,
                      UpdateBiosAppStrings,
                      NULL
                      );
  ASSERT (gUpdateBiosHandle != NULL);

  DEBUG((DEBUG_INFO, "UpdateBiosAppEntry\n"));
  Status = ReadBiosFdInFs (BiosNames[1], &BiosData, &BiosSize);
  if (EFI_ERROR(Status)) {
    InvalidInformation (STRING_TOKEN(STR_UPDATE_BIOS_ERROR_INFO), STRING_TOKEN(STR_PRESS_ENTER_CONTINUE));
    DEBUG((DEBUG_ERROR, "%s not found\n", BiosNames[1]));
    goto ProcExit;
  }
  ShowCopyRightsAndWarning ();
  Print(L"  Do not power off or restart,during bios upgrade process\n");
  Print(L"  Reading file.................%r\n", Status);
  Print(L"  BiosSize:%x\n", BiosSize);
  DEBUG((DEBUG_INFO, "InputFD(%X,%X)\n", BiosData, BiosSize));
  //
  //check the bios head
  //
  Status = CheckBiosImage (BiosData, BiosSize);
  if (Status == EFI_SUCCESS) {
    Biosaddress = BiosData + 0x30;
    Biossize    = BiosSize - 0x30;
    Status = UpdateSpi (
                 0,
                 Biosaddress,
                 Biossize,
                 L"  Updating bios."
                 );
    //clear variable range
    ///*
    Print(L"  Erase Variable Range\n");
    Status = mFlash->Erase (
                     VariableBase,
                     VariableSize
                     );
    //*/
  }else{
    goto ProcExit;
  }


ProcExit:
  if (BiosData != NULL) {
    FreePages (BiosData, EFI_SIZE_TO_PAGES (BiosSize));
  }
  Attribute = EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK;
  gST->ConOut->SetAttribute (gST->ConOut, Attribute);
  if (!EFI_ERROR (Status)) {
    Print(L"\nBIOS has been updated, system will reboot now!\n");
    // stall 2s
    gBS->Stall (2000 * 1000);
    SystemReboot ();
  }
  HiiRemovePackages (gUpdateBiosHandle);
  return Status;
}
