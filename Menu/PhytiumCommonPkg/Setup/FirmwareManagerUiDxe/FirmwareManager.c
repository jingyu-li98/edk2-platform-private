#include "PasswordConfigData.h"
/** @file
The functions for phytium firmware manager menu.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/CustomizedDisplayLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>

#include "FirmwareManager.h"
#include "FirmwareManagerFormGuid.h"

#define EFI_BIOS_HEAD_SIGNATURE                  SIGNATURE_32 ('B', 'I', 'O', 'S')

EFI_GUID                    mFirmwareManagerGuid = FIRMWARE_MANAGER_FORMSET_GUID;
EFI_NORFLASH_DRV_PROTOCOL                *mFlash = NULL;

extern UINT8 FirmwareManagerUiBin[];
EFI_HII_HANDLE gHiiHandle;

HII_VENDOR_DEVICE_PATH  mFirmwareManagerHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    {0x50648efe, 0x8ff7, 0x11ee, {0x8a, 0xe9, 0xb7, 0xcf, 0xb7, 0x79, 0x62, 0x4c}}
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

FIRMWARE_MANAGER_CALLBACK_DATA gFirmwareManagerPrivate = {
  FIRMWARE_MANAGER_CALLBACK_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    FirmwareManagerExtractConfig,
    FirmwareManagerRouteConfig,
    FirmwareManagerCallback
  }
};


/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Request         A null-terminated Unicode string in <ConfigRequest> format.
  @param Progress        On return, points to a character in the Request string.
                         Points to the string's null terminator if request was successful.
                         Points to the most recent '&' before the first failing name/value
                         pair (or the beginning of the string if the failure is in the
                         first name/value pair) if the request was not successful.
  @param Results         A null-terminated Unicode string in <ConfigAltResp> format which
                         has all values filled in for the names in the Request string.
                         String to be allocated by the called function.

  @retval  EFI_SUCCESS            The Results is filled with the requested values.
  @retval  EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval  EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
FirmwareManagerExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                     Status;
  FIRMWARE_MANAGER_CALLBACK_DATA  *Private;
  EFI_STRING                     ConfigRequestHdr;
  EFI_STRING                     ConfigRequest;
  BOOLEAN                        AllocatedRequest;
  UINTN                          Size;
  UINTN                          BufferSize;

  Status = EFI_SUCCESS;

  BufferSize = sizeof (FIRMWARE_MANAGER_DATA);
  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Private = FIRMWARE_MANAGER_CALLBACK_DATA_FROM_THIS (This);
  *Progress = Request;
  if ((Request != NULL) && !HiiIsConfigHdrMatch (Request, &mFirmwareManagerGuid, FIRMWARE_MANAGER_VARIABLE)) {
    DEBUG((DEBUG_ERROR,"%a(),%d,HiiIsConfigHdrMatch not match\n",__FUNCTION__,__LINE__));
    return EFI_NOT_FOUND;
  }
  ConfigRequestHdr = NULL;
  ConfigRequest  = NULL;
  AllocatedRequest = FALSE;

  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //

  ConfigRequest = Request;
  if ((Request == NULL) || (StrStr (Request, L"OFFSET") == NULL)) {
    //
    // Request has no request element, construct full request string.
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&mFirmwareManagerGuid, FIRMWARE_MANAGER_VARIABLE, Private->DriverHandle);
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    FreePool (ConfigRequestHdr);
  }
  Status = gHiiConfigRouting->BlockToConfig (
                                gHiiConfigRouting,
                                ConfigRequest,
                                (UINT8 *) &Private->FirmwareManagerData,
                                sizeof (FIRMWARE_MANAGER_DATA),
                                Results,
                                Progress
                                );
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
    ConfigRequest = NULL;
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return Status;
}

/**
  This function processes the results of changes in configuration.

  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Configuration   A null-terminated Unicode string in <ConfigResp> format.
  @param Progress        A pointer to a string filled in with the offset of the most
                         recent '&' before the first failing name/value pair (or the
                         beginning of the string if the failure is in the first
                         name/value pair) or the terminating NULL if all was successful.

  @retval  EFI_SUCCESS            The Results is processed successfully.
  @retval  EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.
**/
EFI_STATUS
EFIAPI
FirmwareManagerRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STATUS                       Status;
  UINTN                            Buffsize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *ConfigRouting;
  FIRMWARE_MANAGER_CALLBACK_DATA    *Private;

  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Private = FIRMWARE_MANAGER_CALLBACK_DATA_FROM_THIS (This);
  *Progress = Configuration;
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **)&ConfigRouting
                  );
  if(EFI_ERROR (Status)) {
    return Status;
  }
  if (!HiiIsConfigHdrMatch (Configuration, &mFirmwareManagerGuid, FIRMWARE_MANAGER_VARIABLE)) {
    return EFI_NOT_FOUND;
  }
  Buffsize = sizeof (FIRMWARE_MANAGER_DATA);
  Status = ConfigRouting->ConfigToBlock (
                            ConfigRouting,
                            Configuration,
                            (UINT8 *) &Private->FirmwareManagerData,
                            &Buffsize,
                            Progress
                            );

  return Status;
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
  CHAR16      Space[]  = L"                 ";
  UINTN       Columns;
  UINTN       Rows;
  UINTN       StringLen;
  CHAR16      *WarningString;


  UINTN       VariableBase;
  UINT32      VariableSize;
  VariableBase = PcdGet64(PcdFlashNvStorageVariableBase64);
  VariableSize = PcdGet32(PcdFlashNvStorageVariableSize);

  TempBuffer = NULL;
  TempBuffer = AllocatePool (SIZE_64KB);
  Status = EFI_SUCCESS;
  Count  = (Size / SIZE_64KB);
  StringLen = StrLen(Space);
  
  WarningString = HiiGetString (gHiiHandle, STRING_TOKEN (STR_UPDATING_BIOS_WARNING), NULL);

  CreatePopUp (
    EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE,
    NULL,
    WarningString,
    Space,
    NULL
    );

  gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE);
  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &Columns, &Rows);
  Columns   = (Columns - StringLen) / 2;
  Rows    = (Rows - (1 + 3)) / 2 + 1;
  //Rows    = (Rows - (1 + 3)) / 2 + 2;
  gST->ConOut->SetCursorPosition (gST->ConOut, Columns, Rows);
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
    if(((VariableBase / SIZE_64KB) <= Index) && (Index < ((VariableBase + VariableSize) / SIZE_64KB)))
        continue;

    if (TempBuffer != NULL) {
      mFlash->Read (Address + Index * SIZE_64KB, TempBuffer, SIZE_64KB);
      if (CompareMem (TempBuffer, Buffer + Index * SIZE_64KB, SIZE_64KB) == 0) {
        gST->ConOut->SetCursorPosition (gST->ConOut, Columns, Rows);
        Print(L"%s %02d%%%",String, ((Index + 1) * 100) / Count);
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
    gST->ConOut->SetCursorPosition (gST->ConOut, Columns, Rows);
    Print(L"%s %02d%%%",String, ((Index + 1) * 100) / Count);
  }

ProExit:
  if (TempBuffer != NULL) {
    FreePool (TempBuffer);
  }
  return Status;
}

/**
  Find and create information in uni file.

  @param[in]    StringToken1    String token 1.
  @param[in]    StringToken2    String token 2.

**/
VOID
InvalidInformation (
  IN UINT16  StringToken1,
  IN UINT16  StringToken2
  )
{
  CHAR16                          Str1[64];
  CHAR16                          Str2[64];
  CHAR16                          *HiiFormatString;
  EFI_INPUT_KEY                   InputKey;
  UINTN                           EventIndex;

  DEBUG((DEBUG_INFO, "%a()\n", __FUNCTION__));

  HiiFormatString = HiiGetString (gHiiHandle, StringToken1, NULL);
  UnicodeSPrint (Str1, sizeof (Str1), L"%s", HiiFormatString);
  if (HiiFormatString != NULL) {
    FreePool (HiiFormatString);
  }
  HiiFormatString = HiiGetString (gHiiHandle, StringToken2, NULL);
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
}

/**
  Clear the Display card buffer

  @param  NULL

  @retval NULL

**/

VOID ClearScreen(VOID)
{

  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsOutput;
  EFI_STATUS                            Status;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Background = {0x00,0x00,0x00,0x00};

  //
  // Get current video resolution and text mode 
  //
  Status = gBS->HandleProtocol (
                  gST->ConsoleOutHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID**)&GraphicsOutput
                  );
  if (EFI_ERROR (Status)) {
    GraphicsOutput = NULL;
    return;
  }
  DEBUG ((EFI_D_INFO, "HorizontalResolution %d,VerticalResolution, %d \n",GraphicsOutput->Mode->Info->HorizontalResolution,GraphicsOutput->Mode->Info->VerticalResolution));

  Status = GraphicsOutput->Blt(
                     GraphicsOutput,
                     &Background,
                     EfiBltVideoFill,
                     0,
                     0,
                     0,
                     0,
                     GraphicsOutput->Mode->Info->HorizontalResolution,
                     GraphicsOutput->Mode->Info->VerticalResolution,
                     0
                     );


}

/**
  Press Enter to reboot.
**/
VOID
PressKeytoReset (
  VOID
  )
{
  CHAR16                          Str1[64];
  EFI_INPUT_KEY                   InputKey;
  UINTN                           EventIndex;
  CHAR16                          *UpdateSuccString;

  //UnicodeSPrint (Str1, sizeof (Str1), L"Updating bios Sucess, Press Enter to reboot system");
  UnicodeSPrint (Str1, sizeof (Str1), L"                                                  ");
  UpdateSuccString = HiiGetString (gHiiHandle, STRING_TOKEN (STR_BIOS_UPDATE_SUCC), NULL);
  CreatePopUp (
    EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE,
    NULL,
    Str1,
    UpdateSuccString,
    NULL
    );

  while (1) {
    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &EventIndex);
    gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);
    if (InputKey.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      break;
    }
  }

  DEBUG ((DEBUG_INFO , "The ENTER key is pressed, next clear screen and reboot!\n"));
  
  ClearScreen();
  SystemReboot();
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
  //DEBUG((DEBUG_INFO,"the MD5Value0 1  is %x%x",MD5Value[0],MD5Value[1]));
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
  //DEBUG((DEBUG_INFO,"the HashValue 1  is %x%x",HashValue[0],HashValue[1]));

  if (CompareMem(HashValue, MD5Value, 16) == 0) {
    //DEBUG((DEBUG_INFO,"the HashValue is identical\n"));
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
  Update bios from file.

  @param[in]    FilePath    A pointer to update bios data file path.

  @retval    FALSE    Failed.
**/
BOOLEAN
EFIAPI
UpdateFromFile(
  IN EFI_DEVICE_PATH_PROTOCOL    *FilePath
)
{
  VOID                    *FileBuffer;
  UINTN                   FileSize;
  UINT32                  AuthStat;
  EFI_STATUS              Status;
  UINT8                   *Biosaddress;
  UINTN                   VariableBase;
  UINT32                  VariableSize;
  CHAR16                *UpdatingBiosString;

  Status = gBS->LocateProtocol (&gSpiNorFlashProtocolGuid, NULL, (VOID*) &mFlash);
  if (EFI_ERROR(Status)) {
    Print(L"  Locate SpiFlashProtocol failed!\n");
    return FALSE;
  }
  FileBuffer = GetFileBufferByFilePath(FALSE, FilePath, &FileSize, &AuthStat);
  Status = CheckBiosImage (FileBuffer, FileSize);

  if (Status == EFI_SUCCESS) {
    Biosaddress = FileBuffer + 0x30;
    FileSize = FileSize - 0x30;
    UpdatingBiosString = HiiGetString (gHiiHandle, STRING_TOKEN (STR_UPDATING_BIOS), NULL);
    Status = UpdateSpi (
              0,
              Biosaddress,
              FileSize,
              UpdatingBiosString
              );

    VariableBase = PcdGet64(PcdFlashNvStorageVariableBase64);
    VariableSize = PcdGet32(PcdFlashNvStorageVariableSize);
    //Do not erase use data
    /*
    Status = mFlash->Erase (
                  VariableBase,
                  VariableSize
                  );
    */
    PressKeytoReset();
  }
  FreePool(FileBuffer);
  return FALSE;
}
/**
  This call back function is registered with Device Info infomation formset.
  When user selects a boot option, this call back function will
  be triggered. The boot option is saved for later processing.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Action          Specifies the type of action taken by the browser.
  @param QuestionId      A unique value which is sent to the original exporting driver
                         so that it can identify the type of data to expect.
  @param Type            The type of value for the question.
  @param Value           A pointer to the data being sent to the original exporting driver.
  @param ActionRequest   On return, points to the action requested by the callback function.

  @retval  EFI_SUCCESS           The callback successfully handled the action.
  @retval  EFI_INVALID_PARAMETER The setup browser call this function with invalid parameters.

**/
EFI_STATUS
EFIAPI
FirmwareManagerCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_DEVICE_PATH_PROTOCOL        *File;
  EFI_STATUS                       Status;
  UINTN             VarSize;
  UINT8             UserPriv;
  
  Status = EFI_SUCCESS;
  VarSize = sizeof (UINT8);

   Status = gRT->GetVariable (
                  PASSWORD_PRIV,
                  &gPasswordPrivGuid,
                  NULL,
                  &VarSize,
                  &UserPriv
                  );

  if (Action == EFI_BROWSER_ACTION_CHANGING) {
    if (QuestionId == UPDATEBIOS_KEY) {
      //enter FileExplorer.
      if (Status == EFI_NOT_FOUND) 
      {
        Status = ChooseFile (NULL, NULL, UpdateFromFile, &File);
      }
      else
      {
        if( UserPriv == 0 ){
            Status = ChooseFile (NULL, NULL, UpdateFromFile, &File);
        }else{
            InvalidInformation (STRING_TOKEN(STR_NO_RIGHT_ERROR), STRING_TOKEN(STR_PRESS_ENTER_CONTINUE));
            Status = EFI_NOT_FOUND;
        }
      }
    }
  }
  return Status;
}

/**
  Install Firmware manager config Menu driver.

  @param[in] ImageHandle     The image handle.
  @param[in] SystemTable     The system table.

  @retval  EFI_SUCEESS  Install Boot manager menu success.
  @retval  Other        Return error status.
**/
EFI_STATUS
EFIAPI
InstallFirmwareManagerForm (
  IN OUT FIRMWARE_MANAGER_CALLBACK_DATA   *PrivateData
  )
{
  EFI_STATUS  Status;
  //
  // Install Device Path Protocol and Config Access protocol to driver handle
  //
  PrivateData->DriverHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &PrivateData->DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mFirmwareManagerHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &PrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  //
  // Publish our HII data
  //
  PrivateData->HiiHandle = HiiAddPackages (
                                       &mFirmwareManagerGuid,
                                       PrivateData->DriverHandle,
                                       FirmwareManagerUiBin,
                                       FirmwareManagerUiStrings,
                                       NULL
                                       );
  ASSERT (PrivateData->HiiHandle != NULL);
  gHiiHandle = PrivateData->HiiHandle;

  return EFI_SUCCESS;
}

/**
  Unloads the form and its installed protocol.

  @param[in]  ImageHandle      Handle that identifies the image to be unloaded.
  @param[in]  SystemTable      The system table.

  @retval EFI_SUCCESS      The image has been unloaded.
**/
EFI_STATUS
EFIAPI
UninstallFirmwareManagerForm (
  IN OUT FIRMWARE_MANAGER_CALLBACK_DATA   *PrivateData
  )
{
  EFI_STATUS    Status;
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  PrivateData->DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mFirmwareManagerHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &PrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  HiiRemovePackages (PrivateData->HiiHandle);

  return EFI_SUCCESS;
}
