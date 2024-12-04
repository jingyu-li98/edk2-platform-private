/** @file
The functions for phytium advanced menu.

Copyright (c) 2023-2024, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/CustomizedDisplayLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/HobLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/Smbios.h>
#include <Protocol/PciIo.h>

#include "AdvancedConfig.h"
#include "AdvancedConfigFormGuid.h"
#include "DeviceInfo.h"
#include "PasswordConfigData.h"
#include "X100Config.h"

EFI_GUID mAdvancedConfigGuid = ADVANCEDCONFIG_FORMSET_GUID;
ADVANCED_CONFIG_CALLBACK_DATA   gAdvancedConfigPrivate;
BOOLEAN  mFirstEnterAdvancedConfigForm = FALSE;

extern UINT8 AdvancedConfigUiBin[];

HII_VENDOR_DEVICE_PATH  mAdvancedConfigHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    {0xbf30b1b6, 0xea98, 0x11eb, {0x89, 0xb2, 0xe7, 0x04, 0x9c, 0x42, 0xfb, 0xa8}}
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

ADVANCED_CONFIG_CALLBACK_DATA gAdvancedConfigPrivate = {
  ADVANCED_CONFIG_CALLBACK_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    AdvancedConfigExtractConfig,
    AdvancedConfigRouteConfig,
    AdvancedConfigCallback
  }
};

/**
  Set hii strings.

  @param  HiiHandle    Hii handle.
  @param  StrRef       String ID.
  @param  sFormat      Format string.
  @param  ...          Variable argument list whose contents are accessed
                           based on the format string specified by Format.

  @retval     StringId.
**/
EFI_STRING_ID
SetHiiString (
  IN EFI_HII_HANDLE   HiiHandle,
  IN EFI_STRING_ID    StrRef,
  IN CHAR16           *sFormat,
  ...
  )
{
  STATIC CHAR16 s[1024];
  VA_LIST  Marker;
  EFI_STRING_ID    StringId;

  VA_START (Marker, sFormat);
  UnicodeVSPrint (s, sizeof (s),  sFormat, Marker);
  VA_END (Marker);
  StringId = HiiSetString (HiiHandle, StrRef, s, NULL);

  return StringId;
}

/**
  Initialize advanced configuration memu. If a menu loaded by the driver, the
  guid that needs to be loaded is matched.
**/
VOID
InitAdvancedConfigMenu (
  VOID
  )
{
  VOID                *StartOpCodeHandle;
  VOID                *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL  *StartGuidLabel;
  EFI_IFR_GUID_LABEL  *EndGuidLabel;
  EFI_HII_HANDLE      Handle;
  EFI_IFR_FORM_SET    *Buffer;
  UINTN               BufferSize;
  UINT8               ClassGuidNum;
  EFI_GUID            *ClassGuid;
  UINTN               TempSize;
  UINT8               *Ptr;
  EFI_STRING          String;
  EFI_HII_HANDLE      *HiiHandles;
  EFI_GUID            FormSetGuid;
  EFI_STRING_ID       Token;
  EFI_STRING_ID       TokenHelp;
  EFI_STATUS          Status;
  UINT32              Index;

  TempSize =0;
  BufferSize = 0;
  Buffer = NULL;
  Handle = gAdvancedConfigPrivate.HiiHandle;
  //
  // Allocate space for creation of UpdateData Buffer
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);
  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartGuidLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
                                            StartOpCodeHandle,
                                            &gEfiIfrTianoGuid,
                                            NULL,
                                            sizeof (EFI_IFR_GUID_LABEL)
                                            );
  StartGuidLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartGuidLabel->Number       = LABEL_FORM_ADVANCEDCONFIG_START;
  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  EndGuidLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
                                          EndOpCodeHandle,
                                          &gEfiIfrTianoGuid,
                                          NULL,
                                          sizeof (EFI_IFR_GUID_LABEL)
                                          );
  EndGuidLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndGuidLabel->Number       = LABEL_FORM_ADVANCEDCONFIG_END;

  //
  //Init Front Page form
  //
  if (IsX100Existed() == 1) {
    DEBUG ((DEBUG_INFO, "X100 existed!Create X100 Config Menu\n"));
    /*CreateX100ConfigMenu (
      gAdvancedConfigPrivate.HiiHandle,
      StartOpCodeHandle
      );sunrui - X100 Config */
  }
  //
  // Get all the Hii handles
  //
  HiiHandles = HiiGetHiiHandles (NULL);
  ASSERT (HiiHandles != NULL);

  //
  // Search for formset of each class type
  //
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    Status = HiiGetFormSetFromHiiHandle(HiiHandles[Index], &Buffer,&BufferSize);
    if (EFI_ERROR (Status)) {
      continue;
    }
    Ptr = (UINT8 *)Buffer;
    while (TempSize < BufferSize)  {
      TempSize += ((EFI_IFR_OP_HEADER *) Ptr)->Length;
      if (((EFI_IFR_OP_HEADER *) Ptr)->Length <= OFFSET_OF (EFI_IFR_FORM_SET, Flags)) {
        Ptr += ((EFI_IFR_OP_HEADER *) Ptr)->Length;
        continue;
      }

      ClassGuidNum = (UINT8) (((EFI_IFR_FORM_SET *)Ptr)->Flags & 0x3);
      ClassGuid = (EFI_GUID *) (VOID *)(Ptr + sizeof (EFI_IFR_FORM_SET));
      while (ClassGuidNum-- > 0) {
        if (CompareGuid (&gEfiHiiPlatformSetupFormsetGuid, ClassGuid)== 0) {
          ClassGuid++;
          continue;
        }

        String = HiiGetString (HiiHandles[Index], ((EFI_IFR_FORM_SET *)Ptr)->FormSetTitle, NULL);
        if (String == NULL) {
          String = HiiGetString (Handle, STRING_TOKEN (STR_MISSING_STRING), NULL);
          ASSERT (String != NULL);
        }
        Token = HiiSetString (Handle, 0, String, NULL);
        FreePool (String);

        String = HiiGetString (HiiHandles[Index], ((EFI_IFR_FORM_SET *)Ptr)->Help, NULL);
        if (String == NULL) {
          String = HiiGetString (Handle, STRING_TOKEN (STR_MISSING_STRING), NULL);
          ASSERT (String != NULL);
        }
        TokenHelp = HiiSetString (Handle, 0, String, NULL);
        FreePool (String);

        FormSetGuid = ((EFI_IFR_FORM_SET *)Ptr)->Guid;
        if (CompareGuid (&gPcieConfigFormSetGuid, &FormSetGuid) ||
            CompareGuid (&gCpuConfigFormSetGuid, &FormSetGuid)  ||
            CompareGuid (&gBmcInfoFormsetGuid, &FormSetGuid)   ||
            CompareGuid (&gFeatureFormsetGuid, &FormSetGuid)  ||
            CompareGuid (&gRasConfigFormsetGuid, &FormSetGuid)  ||
            CompareGuid (&gPasswordConfigFormSetGuid, &FormSetGuid) ) {
          HiiCreateGotoExOpCode (
            StartOpCodeHandle,
            0,
            Token,
            TokenHelp,
            0,
            (EFI_QUESTION_ID) (Index + ADVANCED_KEY_OFFSET),
            0,
            &FormSetGuid,
            0
          );
        }
        break;
      }

      Ptr += ((EFI_IFR_OP_HEADER *) Ptr)->Length;
    }
    FreePool (Buffer);
    Buffer = NULL;
    TempSize = 0;
    BufferSize = 0;
  }
  HiiUpdateForm (
    Handle,
    NULL,
    FORM_ADVANCEDCONFIG_ID,
    StartOpCodeHandle,
    EndOpCodeHandle);
  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);
}

/**
  Get X100 configuration data from variable and set brawser data.

  @param[in]  CallbackData    A pointer to ADVANCED_CONFIG_CALLBACK_DATA.

  @retval     EFI_SUCCESS     Success.
  @retval     Other           Failed.
**/
EFI_STATUS
GetX100ConfigData (
  IN ADVANCED_CONFIG_CALLBACK_DATA  *CallbackData
  )
{
  EFI_STATUS        Status;
  X100_CONFIG       X100ConfigData;
  UINTN             VarSize;
  UINT8             EdpLight;

  Status = EFI_SUCCESS;

  GetX100BaseInfo (CallbackData->HiiHandle);

  VarSize = sizeof(X100_CONFIG);
  Status = gRT->GetVariable (
                  VAR_X100_CONFIG,
                  &gAdvancedX100VarGuid,
                  NULL,
                  &VarSize,
                  &X100ConfigData
                  );
  if (Status == EFI_SUCCESS) {
    CopyMem (&CallbackData->AdvancedConfigData.X100Config, &X100ConfigData, VarSize);
    Status = gRT->GetVariable (
                    VAR_X100_EDP_LIGHT,
                    &gAdvancedX100EdpLightVarGuid,
                    NULL,
                    &VarSize,
                    &EdpLight
                    );
    if (Status == EFI_SUCCESS) {
      CallbackData->AdvancedConfigData.X100EdpLight = EdpLight;
      Status = HiiSetBrowserData (
                 &mAdvancedConfigGuid,
                 ADVANCED_CONFIG_VARIABLE,
                 sizeof (ADVANCED_CONFIG_DATA),
                 (UINT8 *) &CallbackData->AdvancedConfigData,
                 NULL
                 );
    }
  }

  return Status;
}

/**
  Update X100 configuration data from variable.

  @param[in]  CallbackData    A pointer to ADVANCED_CONFIG_CALLBACK_DATA.

  @retval     EFI_SUCCESS     Success.
  @retval     Other           Failed.
**/
EFI_STATUS
UpdateX100Config (
  IN ADVANCED_CONFIG_CALLBACK_DATA  *CallbackData
  )
{
  EFI_STATUS   Status;
  X100_CONFIG  X100ConfigData;
  UINTN        VarSize;
  UINT8        EdpLight;

  Status = EFI_SUCCESS;
  VarSize = sizeof(X100_CONFIG);
  Status = gRT->GetVariable (
                  VAR_X100_CONFIG,
                  &gAdvancedX100VarGuid,
                  NULL,
                  &VarSize,
                  &X100ConfigData
                  );
  if (Status == EFI_SUCCESS) {
    CopyMem (&CallbackData->AdvancedConfigData.X100Config, &X100ConfigData, VarSize);
  }

  Status = gRT->GetVariable (
                  VAR_X100_EDP_LIGHT,
                  &gAdvancedX100EdpLightVarGuid,
                  NULL,
                  &VarSize,
                  &EdpLight
                  );
  if (Status == EFI_SUCCESS) {
    CallbackData->AdvancedConfigData.X100EdpLight = EdpLight;
  }

  return Status;
}

/**
  Get advanced configuration data from broswer and set it to variable.

  @param[in]  This          A pointer to EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  CallbackData  A pointer to ADVANCED_CONFIG_CALLBACK_DATA.

  @retval     EFI_SUCCESS   Success.
  @retval     Other         Failed.
**/
EFI_STATUS
AdvancedConfigSetupConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  ADVANCED_CONFIG_CALLBACK_DATA          *CallbackData
  )
{
  EFI_STATUS            Status;
  ADVANCED_CONFIG_DATA  AdvancedData;
  //
  // Retrive uncommitted data from Browser
  //
  if (!HiiGetBrowserData (&mAdvancedConfigGuid, ADVANCED_CONFIG_VARIABLE, sizeof (ADVANCED_CONFIG_DATA), (UINT8 *) &AdvancedData)) {
    return EFI_NOT_FOUND;
  }
  CopyMem (&CallbackData->AdvancedConfigData.X100Config, &AdvancedData.X100Config, sizeof(X100_CONFIG));
  Status = EFI_SUCCESS;
  Status = gRT->SetVariable (
                  VAR_X100_CONFIG,
                  &gAdvancedX100VarGuid,
                  PLATFORM_SETUP_VARIABLE_FLAG,
                  sizeof (X100_CONFIG),
                  &CallbackData->AdvancedConfigData.X100Config
                  );
  CallbackData->AdvancedConfigData.X100EdpLight = AdvancedData.X100EdpLight;
  Status = gRT->SetVariable (
                  VAR_X100_EDP_LIGHT,
                  &gAdvancedX100EdpLightVarGuid,
                  PLATFORM_SETUP_VARIABLE_FLAG,
                  sizeof (UINT8),
                  &CallbackData->AdvancedConfigData.X100EdpLight
                  );

  Status = gRT->SetVariable (
                VAR_ADVANCED_CONFIG_NAME,
                &gAdvancedConfigVarGuid,
                PLATFORM_SETUP_VARIABLE_FLAG,
                sizeof (ADVANCED_CONFIG_DATA),
                &AdvancedData
                );

  return Status;
}

/**
  Get Privilege configuration data from variable and set brawser data.

  @param[in]  CallbackData    A pointer to ADVANCED_CONFIG_CALLBACK_DATA.

  @retval     EFI_SUCCESS     Success.
  @retval     Other           Failed.
**/
EFI_STATUS
GetPrivData (
  IN ADVANCED_CONFIG_CALLBACK_DATA  *CallbackData
  )
{
  EFI_STATUS                          Status;
  UINTN                               VarSize;
  UINT8                               UserPriv;
  ADVANCED_CONFIG_DATA                AdvancedConfigData;

  Status = EFI_SUCCESS;
  VarSize = sizeof (UINT8);

  Status = gRT->GetVariable (
                  PASSWORD_PRIV,
                  &gPasswordPrivGuid,
                  NULL,
                  &VarSize,
                  &UserPriv
                  );

  VarSize = sizeof (ADVANCED_CONFIG_DATA);
  Status = gRT->GetVariable (
                VAR_ADVANCED_CONFIG_NAME,
                &gAdvancedConfigVarGuid,
                NULL,
                &VarSize,
                &AdvancedConfigData
                );

  if (Status == EFI_SUCCESS) {
    CallbackData->AdvancedConfigData.UserPriv = UserPriv;
    CallbackData->AdvancedConfigData.AutoReset = AdvancedConfigData.AutoReset;
    Status = HiiSetBrowserData (
                &mAdvancedConfigGuid,
                ADVANCED_CONFIG_VARIABLE,
                sizeof (ADVANCED_CONFIG_DATA),
                (UINT8 *) &CallbackData->AdvancedConfigData,
                NULL
                );
  }

  return Status;
}

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
AdvancedConfigExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                     Status;
  ADVANCED_CONFIG_CALLBACK_DATA  *Private;
  EFI_STRING                     ConfigRequestHdr;
  EFI_STRING                     ConfigRequest;
  BOOLEAN                        AllocatedRequest;
  UINTN                          Size;
  UINTN                          BufferSize;

  Status = EFI_SUCCESS;

  BufferSize = sizeof (ADVANCED_CONFIG_DATA);
  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Private = ADVANCED_CONFIG_CALLBACK_DATA_FROM_THIS (This);
  UpdateX100Config (Private);
  *Progress = Request;
  if ((Request != NULL) && !HiiIsConfigHdrMatch (Request, &mAdvancedConfigGuid, ADVANCED_CONFIG_VARIABLE)) {
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
    ConfigRequestHdr = HiiConstructConfigHdr (&mAdvancedConfigGuid, ADVANCED_CONFIG_VARIABLE, Private->DriverHandle);
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
                                (UINT8 *) &Private->AdvancedConfigData,
                                sizeof (ADVANCED_CONFIG_DATA),
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
AdvancedConfigRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STATUS                       Status;
  UINTN                            Buffsize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *ConfigRouting;
  ADVANCED_CONFIG_CALLBACK_DATA    *Private;

  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Private = ADVANCED_CONFIG_CALLBACK_DATA_FROM_THIS (This);
  *Progress = Configuration;
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **)&ConfigRouting
                  );
  if(EFI_ERROR (Status)) {
    return Status;
  }
  if (!HiiIsConfigHdrMatch (Configuration, &mAdvancedConfigGuid, ADVANCED_CONFIG_VARIABLE)) {
    return EFI_NOT_FOUND;
  }
  AdvancedConfigSetupConfig (This, Private);
  Buffsize = sizeof (ADVANCED_CONFIG_DATA);
  Status = ConfigRouting->ConfigToBlock (
                            ConfigRouting,
                            Configuration,
                            (UINT8 *) &Private->AdvancedConfigData,
                            &Buffsize,
                            Progress
                            );

  return Status;
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
AdvancedConfigCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  ADVANCED_CONFIG_CALLBACK_DATA  *Private;
  EFI_STATUS Status;
  Status = EFI_SUCCESS;

  Private = ADVANCED_CONFIG_CALLBACK_DATA_FROM_THIS (This);
  if (Action == EFI_BROWSER_ACTION_FORM_OPEN) {
      if (QuestionId == FORM_ADVANCEDCONFIG_OPEN) {
  //      if (mFirstEnterAdvancedConfigForm == FALSE) {
          InitAdvancedConfigMenu ();
  //        mFirstEnterAdvancedConfigForm = TRUE;
  //      }
        Status = GetPrivData(Private);
      }
      else if (QuestionId == FORM_X100CONFIG_OPEN) {
        GetX100ConfigData (Private);
      }
      else if (QuestionId == FORM_PCIEDEVICEINFO_OPEN) {
        GetPcieDeviceInfo (Private->HiiHandle);
      }
      else if (QuestionId == FORM_USBDEVICEINFO_OPEN) {
        GetUsbDeviceInfo (Private->HiiHandle);
        GetUsbControllerInfo (Private->HiiHandle);
      }
      else if (QuestionId == FORM_HDDINFO_OPEN) {
        GetHddDeviceInfo (Private->HiiHandle);
      }
  }
  return Status;
}

/**
  Install advanced config Menu driver.

  @param[in] ImageHandle     The image handle.
  @param[in] SystemTable     The system table.

  @retval  EFI_SUCEESS  Install Boot manager menu success.
  @retval  Other        Return error status.
**/
EFI_STATUS
EFIAPI
AdvancedConfigUiConstructor (
  IN EFI_HANDLE          ImageHandle,
  IN EFI_SYSTEM_TABLE    *SystemTable
  )
{
  EFI_STATUS  Status;
  //
  // Install Device Path Protocol and Config Access protocol to driver handle
  //
  gAdvancedConfigPrivate.DriverHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gAdvancedConfigPrivate.DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mAdvancedConfigHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &gAdvancedConfigPrivate.ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  //
  // Publish our HII data
  //
  gAdvancedConfigPrivate.HiiHandle = HiiAddPackages (
                                       &mAdvancedConfigGuid,
                                       gAdvancedConfigPrivate.DriverHandle,
                                       AdvancedConfigUiBin,
                                       AdvancedConfigUiStrings,
                                       NULL
                                       );
  ASSERT (gAdvancedConfigPrivate.HiiHandle != NULL);

  return EFI_SUCCESS;
}

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle      Handle that identifies the image to be unloaded.
  @param[in]  SystemTable      The system table.

  @retval EFI_SUCCESS      The image has been unloaded.
**/
EFI_STATUS
EFIAPI
AdvancedConfigUiDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS    Status;
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  gAdvancedConfigPrivate.DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mAdvancedConfigHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &gAdvancedConfigPrivate.ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  HiiRemovePackages (gAdvancedConfigPrivate.HiiHandle);

  return EFI_SUCCESS;
}
